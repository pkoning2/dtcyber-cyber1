/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
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
**  Changed to use the "ASCII subset" glyphs from the NOS Compass Instant.
**  p. koning. 03.06.23.
*/
static char printerToAscii[] = {
    /* 00-07 */     ':',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
    /* 10-17 */     '8',    '9',    '0',    '=',    '"',    '@',    '%',    '[',
    /* 20-27 */     ' ',    '/',    'S',    'T',    'U',    'V',    'W',    'X',
    /* 30-37 */     'Y',    'Z',    ']',    ',',    '(',    '_',    '#',    '&',
    /* 40-47 */     '-',    'J',    'K',    'L',    'M',    'N',    'O',    'P',
    /* 50-57 */     'Q',    'R',    '!',    '$',    '*',   '\'',    '?',    '>',
    /* 60-67 */     '+',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
    /* 70-77 */     'H',    'I',    '<',    '.',    ')',   '\\',    '^',    ';'
    };

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

    dp = channelAttach(channelNo, DtLp1612);

    dp->activate = lp1612Activate;
    dp->disconnect = lp1612Disconnect;
    dp->func = lp1612Func;
    dp->io = lp1612Io;
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
**  Purpose:        Remove the paper (operator interface).
**
**  Parameters:     Name        Description.
**                  params      parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void lp1612RemovePaper(char *params)
    {
    DevSlot *dp;
    int numParam;
    int channelNo;
    int unitNo;
    FILE *fcb;
    time_t currentTime;
    struct tm t;
    char fname[80];
    char fnameNew[80];

    /*
    **  Operator inserted a new tape.
    */
    numParam = sscanf(params,"%o,%o",&channelNo, &unitNo);

    /*
    **  Check parameters.
    */
    if (numParam != 2)
        {
        printf("Not enough or invalid parameters\n");
        return;
        }

    if (channelNo < 0 || channelNo >= MaxChannels)
        {
        printf("Invalid channel no\n");
        return;
        }

    if (unitNo < 0 || unitNo >= MaxUnits)
        {
        printf("Invalid equipment no\n");
        return;
        }

    /*
    **  Locate the device control block.
    */
    dp = channelFindDevice((u8)channelNo, DtLp1612);
    if (dp == NULL)
        {
        printf("No 1612 printer on channel %o\n", channelNo);
        return;
        }

    /*
    **  Check if the unit is even configured.
    */
    if (dp->fcb[unitNo] == NULL)
        {
        printf("Equipment %d not allocated\n", unitNo);
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
    sprintf(fname, "LP1612_C%02o_E%o", channelNo, unitNo);

    time(&currentTime);
    t = *localtime(&currentTime);
    sprintf(fnameNew, "LP1612_%04d%02d%02d_%02d%02d%02d",
        t.tm_year + 1900,
        t.tm_mon + 1,
        t.tm_mday,
        t.tm_hour,
        t.tm_min,
        t.tm_sec);

    if (rename(fname, fnameNew) != 0)
        {
        printf("Could not rename %s to %s - %s\n", fname, fnameNew, strerror(errno));
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
        printf("Failed to open %s\n", fname);
        return;
        }

    printf("Paper removed from 1612 printer\n");

    /*
    **  Setup status.
    */
    dp->fcb[unitNo] = fcb;
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
            fputc(printerToAscii[activeChannel->data & 077], fcb);
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
