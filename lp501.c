/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: lp501.c
**
**  Description:
**      Perform simulation of CDC 6600 501 line printer.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

/*
**      Function codes
**
**      ----------------------------------
**      |  Equip select  |   function    |
**      ----------------------------------
**      11              6 5             0
**
**      06x0 = Select printer
**      06x1 = Advance Paper
**      06x3 = Advance to selected line
**      06x4 = Top of form (VFU channel 8)
**      06x6 = End of data
**      06x7 = Status request
**      061x = Select VFU format channel (X = 1 - 6)
**
**      x = printer unit # on channel
*/
#define FcPrintUnitMask         07707

#define FcPrintSelect           00600
#define FcPrintSelPrinter       00000
#define FcPrintAdvancePaper     00601
#define FcPrintAdvance          00001
#define FcPrintMoveChannel7     00603
#define FcPrintAdvanceToLine    00003
#define FcPrintMoveTOF          00604
#define FcPrintEject            00004
#define FcPrintAutoEject        00605
#define FcPrintAutoTOF          00005
#define FcPrintEndOfData        00606
#define FcPrintDone             00006
#define FcPrintStatusReq        00607
#define FcPrintReturnStatus     00007
#define FcPrintSelectVFUChan1   00610
#define FcPrintSelVFUChan1      00010
#define FcPrintSelectVFUChan2   00620
#define FcPrintSelVFUChan2      00020
#define FcPrintSelectVFUChan3   00630
#define FcPrintSelVFUChan3      00030
#define FcPrintSelectVFUChan4   00640
#define FcPrintSelVFUChan4      00040
#define FcPrintSelectVFUChan5   00650
#define FcPrintSelVFUChan5      00050
#define FcPrintSelectVFUChan6   00660
#define FcPrintSelVFUChan6      00060


#define Fc6681DevStatusReq      01300
#define FcControllerOutputEna   01600

/*
**  Unknown functions.
*/
#define FcPrintUnknown02        00002
#define FcPrintUnknown11        00011
#define FcPrintUnknown12        00012
#define FcPrintUnknown22        00022
#define FcPrintUnknown24        00024
#define FcPrintUnknown56        00056
#define FcPrintUnknown66        00066

/*
**      Status reply
**
**      xxx1 = Not Index Mark
**      xxx2 = Wait Character Mark
**      xxx4 = Holding Row Count
**      xx1x = Paper advancing
**      xx2x = Not Ready
**       = Ready
**
*/
#define StPrintReady            00001

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus lp501Func(PpWord funcCode);
static void lp501Io(void);
static void lp501Activate(void);
static void lp501Disconnect(void);
static void lp501Load(Dev3kSlot *up, char *fn);


/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise 501 line printer.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      unit number
**                  channelNo   channel number the device is attached to
**                  deviceName  optional device file name
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void lp501Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    Dev3kSlot *up;
    char fname[80];

    (void)eqNo;
    (void)deviceName;

    up = dcc6681Attach(channelNo, unitNo, DtLp501);

    up->context = calloc(1, sizeof(int));
    up->activate = lp501Activate;
    up->disconnect = lp501Disconnect;
    up->func = lp501Func;
    up->io = lp501Io;
    up->load = lp501Load;

    /*
    **  Open the device file.
    */
    sprintf(fname, "LP501_C%02o_E%o", channelNo, unitNo);
    up->fcb = fopen(fname, "w");
    if (up->fcb == NULL)
        {
        fprintf(stderr, "Failed to open %s\n", fname);
        exit(1);
        }

    /*
    **  Print a friendly message.
    */
    printf("LP501 initialised on channel %o equipment %o\n", channelNo, unitNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on printer.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp501Load(Dev3kSlot *up, char *fn)
    {
    FILE *fcb;
    time_t currentTime;
    struct tm t;
    char fname[80];
    char fnameNew[80];
    static char msgBuf[80];

    if (fn != NULL)
        {
        opSetMsg ("$LOAD NOT SUPPORTED ON LP501");
        return;
        }
    
    /*
    **  Close the old device file.
    */
    fflush(up->fcb);
    fclose(up->fcb);
    up->fcb = NULL;

    /*
    **  Rename the device file to the format "LP501_yyyymmdd_hhmmss".
    */
    sprintf(fname, "LP501_C%02o_E%o", up->conv->channel->id, up->id);

    time(&currentTime);
    t = *localtime(&currentTime);
    sprintf(fnameNew, "LP501_%04d%02d%02d_%02d%02d%02d",
        t.tm_year + 1900,
        t.tm_mon + 1,
        t.tm_mday,
        t.tm_hour,
        t.tm_min,
        t.tm_sec);

    if (rename(fname, fnameNew) != 0)
        {
        sprintf (msgBuf, "$Rename error (%s to %s): %s",
                 fname, fnameNew, strerror(errno));
        opSetMsg(msgBuf);
        return;
        }

    /*
    **  Open the device file.
    */
    fcb = fopen(fname, "w");

    /*
    **  Check if the open succeeded.
    */
    if (fcb == NULL)
        {
        sprintf (msgBuf, "$Open error (%s): %s",
                 fname, strerror (errno));
        opSetMsg(msgBuf);
        return;
        }

    /*
    **  Setup status.
    */
    up->fcb = fcb;
    sprintf (msgBuf, "LP501 unloaded to %s", fnameNew);
    opSetMsg (msgBuf);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 501 line printer.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus lp501Func(PpWord funcCode)
    {
    FILE *fcb;
    fcb = activeUnit->fcb;

    /*
    **  Now process the printer function code.
    */
    switch (funcCode)
        {
    default:
//        return(FcDeclined);
        printf("Unknown printer function %04o\n", funcCode);
        return(FcProcessed);

    case FcPrintUnknown02:
    case FcPrintUnknown11:
    case FcPrintUnknown12:
    case FcPrintUnknown22:
    case FcPrintUnknown24:
    case FcPrintUnknown56:
    case FcPrintUnknown66:
        activeUnit->fcode = funcCode;
        return(FcProcessed);

    case FcPrintSelect:
    case FcPrintSelPrinter:
        break;

    case FcPrintAdvancePaper:
    case FcPrintAdvance:
        fprintf(fcb, "\n");
        break;

    case FcPrintMoveChannel7:
    case FcPrintAdvanceToLine:
        fprintf(fcb, "\n");
        break;

    case FcPrintMoveTOF:
    case FcPrintEject:
        fprintf(fcb, "\n");
        break;

    case FcPrintAutoEject:
    case FcPrintAutoTOF:
        fprintf(fcb, "\n");
        break;

    case FcPrintEndOfData:
    case FcPrintDone:
        fprintf(fcb, "\n");
        break;

    case FcPrintSelectVFUChan1:
    case FcPrintSelVFUChan1:
    case FcPrintSelectVFUChan2:
    case FcPrintSelVFUChan2:
    case FcPrintSelectVFUChan3:
    case FcPrintSelVFUChan3:
    case FcPrintSelectVFUChan4:
    case FcPrintSelVFUChan4:
    case FcPrintSelectVFUChan5:
    case FcPrintSelVFUChan5:
    case FcPrintSelectVFUChan6:
    case FcPrintSelVFUChan6:
        break;

    case FcControllerOutputEna:
        fprintf(fcb, "\n");
        activeUnit->fcode = funcCode;
        return(FcAccepted);

    case FcPrintStatusReq:
    case FcPrintReturnStatus:
    case Fc6681DevStatusReq:
        activeUnit->fcode = funcCode;
        return(FcAccepted);
        }
        
    return(FcProcessed);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 501 line printer.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp501Io(void)
    {
    FILE *fcb = activeUnit->fcb;

    /*
    **  Process printer I/O.
    */
    switch (activeUnit->fcode)
        {
    default:
        activeChannel->full = FALSE;
        break;

    case FcControllerOutputEna:
        if (activeChannel->full)
            {
            fputc(bcdToAscii[(activeChannel->data >> 6) & 077], fcb);
            fputc(bcdToAscii[activeChannel->data & 077], fcb);
            activeChannel->full = FALSE;
            }
        activeChannel->full = FALSE;
        break;

    case FcPrintStatusReq:
    case FcPrintReturnStatus:
    case Fc6681DevStatusReq:
        activeChannel->data = StPrintReady;
        activeChannel->full = TRUE;
        activeUnit->fcode = 0;
        break;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp501Activate(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle disconnecting of channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp501Disconnect(void)
    {
    }

/*---------------------------  End Of File  ------------------------------*/
