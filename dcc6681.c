/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: dcc6681.c
**
**  Description:
**      Perform simulation of CDC 6681 data channel converter.
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
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#ifndef DEBUG
#define DEBUG 0
#endif

/*
**  Function codes.
*/
#define Fc6681Select            02000
#define Fc6681DeSelect          02100
#define Fc6681Connect1          01000
#define Fc6681Function          01100
#define Fc6681StatusReq         01200
#define Fc6681DevStatusReq      01300
#define Fc6681MasterClear       01700

#define Fc6681Connect4          04000
#define Fc6681Connect5          05000
#define Fc6681Connect6          06000
#define Fc6681Connect7          07000
#define Fc6681ConnectUnitMask   07000
#define Fc6681ConnectFuncMask   00777

#define Fc6681InputToEor        01400
#define Fc6681Input             01500
#define Fc6681Output            01600
#define Fc6681IoModeMask        07700
#define Fc6681IoIosMask         00070
#define Fc6681IoBcdMask         00001

/*
**      Status reply
*/
#define StFc6681Ready           00000

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
typedef struct dccControl
    {
    bool        configured;
    bool        selected;
    bool        connected;
    PpWord      ios;
    PpWord      bcd;
    } DccControl;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/

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
static DccControl dccMap[MaxChannels];

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise 6681 data channel converter.
**
**  Parameters:     Name        Description.
**                  channelNo   channel number the device is attached to
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dcc6681Init(u8 channelNo)
    {
    dccMap[channelNo].configured = TRUE;
    dccMap[channelNo].selected = TRUE;
    dccMap[channelNo].connected = FALSE;

    /*
    **  Print a friendly message.
    */
    printf("DCC6681 initialised on channel %o\n", channelNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 6681 data channel converter.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
FcStatus dcc6681Func(PpWord funcCode)
    {
    DccControl *mp = dccMap + activeChannel->id;

    if (!mp->configured)
        {
        return(FcDeclined);
        }

    switch (funcCode)
        {
    case Fc6681Select:
        mp->selected = TRUE;
        return(FcProcessed);
        
    case Fc6681DeSelect:
        mp->selected = FALSE;
        return(FcProcessed);
        
    case Fc6681Connect1:
    case Fc6681Function:
    case Fc6681StatusReq:
    case Fc6681DevStatusReq:
        activeDevice->fcode = funcCode;
        return(FcAccepted);

    case Fc6681MasterClear:
        return(FcProcessed);
        }

    switch (funcCode & Fc6681IoModeMask)
        {
    case Fc6681InputToEor:
    case Fc6681Input:
    case Fc6681Output:
        mp->ios = funcCode & Fc6681IoIosMask;
        mp->bcd = funcCode & Fc6681IoBcdMask;
        activeDevice->fcode = funcCode & Fc6681IoModeMask;
        return(FcDeclined);
        }

    switch (funcCode & Fc6681ConnectUnitMask)
        {
    case Fc6681Connect4:
    case Fc6681Connect5:
    case Fc6681Connect6:
    case Fc6681Connect7:
        activeDevice->selectedUnit = (funcCode & Fc6681ConnectUnitMask) >> 9;
        if (activeDevice->context[activeDevice->selectedUnit] == NULL)
            {
            ppAbort((stderr, "channel %02o - invalid select: %04o", activeChannel->id, (u32)funcCode));
            return(FcDeclined);
            }
        activeDevice->fcode        = funcCode & Fc6681ConnectFuncMask;
        if (DEBUG)
            {
            printf("Fc6681ConnectX %04o %o\n",funcCode, activeDevice->selectedUnit);
            }

        return(FcProcessed);
        }

    return(FcDeclined);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 6681 data channel converter.
**
**  Parameters:     Name        Description.
**
**  Returns:        TRUE if I/O has been completed here, FALSE otherwise.
**
**------------------------------------------------------------------------*/
bool dcc6681Io(void)
    {
    switch (activeDevice->fcode)
        {
    default:
        return(FALSE);

    case Fc6681Select:
    case Fc6681DeSelect:
    case Fc6681MasterClear:
    case Fc6681Connect4:
    case Fc6681Connect5:
    case Fc6681Connect6:
    case Fc6681Connect7:
        printf("unexpected IO for function %04o\n", activeDevice->fcode); 
        break;

    case Fc6681Connect1:
        if (activeChannel->full)
            {
            activeDevice->selectedUnit =
                (activeChannel->data & Fc6681ConnectUnitMask) >> 9;
            if (activeDevice->context[activeDevice->selectedUnit] == NULL)
                {
                ppAbort((stderr, "channel %02o - invalid select: %04o",
                     activeChannel->data, activeDevice->selectedUnit));
                return(FALSE);
                }
            if (DEBUG)
                {
                printf("Fc6681Connect1 %04o\n", activeChannel->data);
                }
            }

        activeChannel->full = FALSE;
        activeDevice->fcode = 0;
        break;

    case Fc6681Function:
        if (activeChannel->full)
            {
            if (DEBUG)
                {
                printf("Fc6681Function %04o\n", activeChannel->data);
                }
            }

        activeChannel->full = FALSE;
        activeDevice->fcode = 0;
        break;

    case Fc6681StatusReq:
        if (!activeChannel->full)
            {
            activeChannel->data = StFc6681Ready;
            activeChannel->full = TRUE;
            activeDevice->fcode = 0;
            }
        break;

    case Fc6681DevStatusReq:
        if (!activeChannel->full)
            {
            activeChannel->data = activeDevice->status;
            activeChannel->full = TRUE;
            activeDevice->fcode = 0;
            }
        break;
        }

    return(TRUE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dcc6681Activate(void)
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
static void dcc6681Disconnect(void)
    {
    }

/*---------------------------  End Of File  ------------------------------*/
