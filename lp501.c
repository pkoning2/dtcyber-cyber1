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
static void lp501Load(DevSlot *dp, int unitNo, char *fn);


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
**  Character set translation table (question marks as placeholders for
**  scientific char set glyphs that could be mapped in any of several
**  different ways).
*/
static char printerToAscii[] =
    {
    /* 00-07 */     '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
    /* 10-17 */     '8',    '9',    ':',    '=',    '"',    '@',    ' ',    '[',
    /* 20-27 */     '+',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
    /* 30-37 */     'H',    'I',    '<',    '.',    ')',    '\\',    '^',    ';',
    /* 40-47 */     '-',    'J',    'K',    'L',    'M',    'N',    'O',    'P',
    /* 50-57 */     'Q',    'R',    '!',    '$',    '*',    0x27,    '?',    '>',
    /* 60-67 */     ' ',    '/',    'S',    'T',    'U',    'V',    'W',    'X',
    /* 70-77 */     'Y',    'Z',    ']',    ',',    '(',    '_',    '#',    '&'
    };

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
    DevSlot *dp;
    char fname[80];

    (void)eqNo;
    (void)deviceName;

    dp = channelAttach(channelNo, DtLp501);

    dp->context[unitNo] = calloc(1, sizeof(int));
    dp->activate = lp501Activate;
    dp->disconnect = lp501Disconnect;
    dp->func = lp501Func;
    dp->io = lp501Io;
    dp->load = lp501Load;
    dp->selectedUnit = unitNo;

    /*
    **  Open the device file.
    */
    sprintf(fname, "LP501_C%02o_E%o", channelNo, unitNo);
    dp->fcb[unitNo] = fopen(fname, "w");
    if (dp->fcb[unitNo] == NULL)
        {
        fprintf(stderr, "Failed to open %s\n", fname);
        exit(1);
        }

    /*
    **  Initialise DCC6681 on this channel.
    */
    dcc6681Init(channelNo);

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
static void lp501Load(DevSlot *dp, int unitNo, char *fn)
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
    
    if (unitNo < 0 || unitNo >= MaxUnits)
        {
        opSetMsg ("$INVALID EQUIPMENT NO");
        return;
        }

    /*
    **  Check if the unit is even configured.
    */
    if (dp->fcb[unitNo] == NULL)
        {
        opSetMsg ("$EQUIPMENT NOT ALLOCATED");
        return;
        }

    /*
    **  Close the old device file.
    */
    fflush(dp->fcb[unitNo]);
    fclose(dp->fcb[unitNo]);
    dp->fcb[unitNo] = NULL;

    /*
    **  Rename the device file to the format "LP501_yyyymmdd_hhmmss".
    */
    sprintf(fname, "LP501_C%02o_E%o", dp->channel->id, unitNo);

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
    dp->fcb[unitNo] = fcb;
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
    FcStatus st;

    /*
    **  Let the data channel converter have a pass over the function code first.
    */
    st = dcc6681Func(funcCode);
    if (st != FcDeclined)
        {
        return(st);
        }

    fcb = activeDevice->fcb[activeDevice->selectedUnit];

    /*
    **  Now process the printer function code.
    */
    switch (funcCode)
        {
    default:
//        return(FcDeclined);
        printf("Unknown printer function %04o\n", funcCode);
        return(FcAccepted);

    case FcPrintUnknown02:
    case FcPrintUnknown11:
    case FcPrintUnknown12:
    case FcPrintUnknown22:
    case FcPrintUnknown24:
    case FcPrintUnknown56:
    case FcPrintUnknown66:
        activeDevice->fcode = funcCode;
        return(FcAccepted);

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
        break;

    case FcPrintStatusReq:
    case FcPrintReturnStatus:
    case Fc6681DevStatusReq:
        break;
        }
        
    activeDevice->fcode = funcCode;
    return(FcAccepted);
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
    FILE *fcb = activeDevice->fcb[activeDevice->selectedUnit];

    /*
    **  Process any data channel converter I/O.
    */
    if (dcc6681Io())
        {
        return;
        }
    
    /*
    **  Process printer I/O.
    */
    switch (activeDevice->fcode)
        {
    default:
        activeChannel->full = FALSE;
        break;

    case FcControllerOutputEna:
        if (activeChannel->full)
            {
            fputc(printerToAscii[(activeChannel->data >> 6) & 077], fcb);
            fputc(printerToAscii[activeChannel->data & 077], fcb);
            activeChannel->full = FALSE;
            }
        activeChannel->full = FALSE;
        break;

    case FcPrintSelect:
    case FcPrintSelPrinter:
    case FcPrintAdvancePaper:
    case FcPrintAdvance:
    case FcPrintMoveChannel7:
    case FcPrintAdvanceToLine:
    case FcPrintMoveTOF:
    case FcPrintEject:
    case FcPrintAutoTOF:
    case FcPrintEndOfData:
    case FcPrintDone:
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
        activeChannel->full = FALSE;
        break;

    case FcPrintStatusReq:
    case FcPrintReturnStatus:
    case Fc6681DevStatusReq:
        activeChannel->data = StPrintReady;
        activeChannel->full = TRUE;
        activeDevice->fcode = 0;
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
