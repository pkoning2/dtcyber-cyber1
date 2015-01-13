/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: lp1612.c
**
**  Description:
**      Perform simulation of CDC 6600 1612 line printer.
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
#include <time.h>
#include <errno.h>
#include <string.h>
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
**      06x1 = Single space
**      06x2 = Double space
**      06x3 = Move paper to format channel 7
**      06x4 = Move paper to top of form
**      06x5 = Print
**      06x6 = Suppress line advance after next print
**      06x7 = Status request
**
**      x = printer unit # on channel
*/
#define FcPrintUnitMask         07707

#define FcPrintSelect           00600
#define FcPrintSingleSpace      00601
#define FcPrintDoubleSpace      00602
#define FcPrintMoveChannel7     00603
#define FcPrintMoveTOF          00604
#define FcPrintPrint            00605
#define FcPrintSuppressLF       00606
#define FcPrintStatusReq        00607

/*
**      Status reply
**
**      0000 = Not Ready
**      4000 = Ready
**
*/
#define StPrintReady            04000
#define StPrintNotReady         00000

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
static FcStatus lp1612Func(PpWord funcCode);
static void lp1612Io(void);
static void lp1612Activate(void);
static void lp1612Disconnect(void);
static void lp1612Load(DevSlot *dp, int unitNo, char *fn);


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
**  Purpose:        Initialise 1612 line printer.
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
void lp1612Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;
    char fname[80];

    (void)eqNo;
    (void)deviceName;

    dp = channelAttach(channelNo, eqNo, DtLp1612);

    dp->activate = lp1612Activate;
    dp->disconnect = lp1612Disconnect;
    dp->func = lp1612Func;
    dp->io = lp1612Io;
    dp->load = lp1612Load;
    dp->selectedUnit = unitNo;

    /*
    **  Open the device file.
    */
    sprintf(fname, "LP1612_C%02o_U%o", channelNo, unitNo);
    dp->fcb[unitNo] = fopen(fname, "w+t");
    if (dp->fcb[unitNo] == NULL)
        {
        fprintf(stderr, "Failed to open %s\n", fname);
        exit(1);
        }

    /*
    **  Print a friendly message.
    */
    printf("LP1612 initialised on channel %o unit %o\n", channelNo, unitNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform load/unload on printer.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp1612Load(DevSlot *dp, int unitNo, char *fn)
    {
    FILE *fcb;
    time_t currentTime;
    struct tm t;
    char fname[80];
    char fnameNew[80];
    static char msgBuf[80];

    if (fn != NULL)
        {
        opSetMsg ("$LOAD NOT SUPPORTED ON LP1612");
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
    **  Rename the device file to the format "LP1612_yyyymmdd_hhmmss".
    */
    sprintf(fname, "LP1612_C%02o_U%o", dp->channel->id, unitNo);

    time(&currentTime);
    localtime_r(&currentTime, &t);
    sprintf(fnameNew, "LP1612_%04d%02d%02d_%02d%02d%02d",
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
    sprintf (msgBuf, "LP1612 unloaded to %s", fnameNew);
    opSetMsg (msgBuf);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 1612 line printer.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus lp1612Func(PpWord funcCode)
    {
    FILE *fcb = activeDevice->fcb[0];

    switch (funcCode & FcPrintUnitMask)
        {
    default:
        return(FcDeclined);

    case FcPrintSelect:
        fprintf(fcb, ">>>>>>> select <<<<<<<<<<\n");
        break;

    case FcPrintSingleSpace:
        fprintf(fcb, ">>>>>>> single space <<<<<<<<<<\n");
        break;

    case FcPrintDoubleSpace:
        fprintf(fcb, ">>>>>>> double space <<<<<<<<<<\n");
        break;

    case FcPrintMoveChannel7:
        fprintf(fcb, ">>>>>>> move to channel 7 <<<<<<<<<<\n");
        break;

    case FcPrintMoveTOF:
        fprintf(fcb, ">>>>>>> move to TOF <<<<<<<<<<\n");
        break;

    case FcPrintPrint:
        fprintf(fcb, ">>>>>>> print <<<<<<<<<<\n");
        break;

    case FcPrintSuppressLF:
        fprintf(fcb, ">>>>>>> suppress LF <<<<<<<<<<\n");
        break;

    case FcPrintStatusReq:
        activeChannel->status = StPrintReady;
        break;
        }

    activeDevice->fcode = funcCode;
    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 1612 line printer.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void lp1612Io(void)
    {
    FILE *fcb = activeDevice->fcb[0];

    switch (activeDevice->fcode & FcPrintUnitMask)
        {
    default:
    case FcPrintSelect:
    case FcPrintSingleSpace:
    case FcPrintDoubleSpace:
    case FcPrintMoveChannel7:
    case FcPrintMoveTOF:
    case FcPrintPrint:
    case FcPrintSuppressLF:
        if (activeChannel->full)
            {
            fputc(extBcdToAscii[activeChannel->data & 077], fcb);
            activeChannel->full = FALSE;
            }
        break;

    case FcPrintStatusReq:
        activeChannel->data = activeChannel->status;
        activeChannel->full = TRUE;
        activeDevice->fcode = 0;
        activeChannel->status = 0;
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
static void lp1612Activate(void)
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
static void lp1612Disconnect(void)
    {
    }

/*---------------------------  End Of File  ------------------------------*/
