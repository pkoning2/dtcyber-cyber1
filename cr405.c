/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: cr405b.c
**
**  Description:
**      Perform simulation of channel-connected CDC 405-B card reader.
**      It does not use a 3000 series channel converter.
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

/*
**  CDC 405 card reader function and status codes.
**  
**      Function codes
**
**      ----------------------------------
**      |  Equip select  |   function    |
**      ----------------------------------
**      11              6 5             0
**
**      0700 = Deselect
**      0701 = Gate Card to Secondary bin
**      0702 = Read Non-stop
**      0704 = Status request
**
**      Note: To read one card, execute successive S702 and
**      S704 functions.
**      One column of card data per 12-bit data word.
*/
#define FcCr405Deselect         00700
#define FcCr405GateToSec        00701
#define FcCr405ReadNonStop      00702
#define FcCr405StatusReq        00704

/*
**      Status reply
**
**      0000 = Ready
**      0001 = Not ready
**      0002 = End of file
**      0004 = Compare error
**
*/
#define StCr405Ready            00000
#define StCr405NotReady         00001
#define StCr405EOF              00002
#define StCr405CompareErr       00004

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
static FcStatus cr405Func(ChSlot *activeChannel, DevSlot *activeDevice,
                          PpWord funcCode);
static void cr405Io(ChSlot *activeChannel, DevSlot *activeDevice);
static void cr405Activate(ChSlot *activeChannel, DevSlot *activeDevice);
static void cr405Disconnect(ChSlot *activeChannel, DevSlot *activeDevice);

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
static u16 retryOpen = 0;
static char fname[80];
static FILE *fcb;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise card reader.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitCount   number of units to initialise
**                  channelNo   channel number the device is attached to
**                  deviceName  optional device file name
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void cr405Init(u8 eqNo, u8 unitCount, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;

    (void)eqNo;
    (void)unitCount;
    (void)deviceName;

    dp = channelAttach(channelNo, eqNo, DtCr405);

    dp->activate = cr405Activate;
    dp->disconnect = cr405Disconnect;
    dp->func = cr405Func;
    dp->io = cr405Io;
    dp->selectedUnit = 0;

    sprintf(fname, "CR405_C%02o", channelNo);

    /*
    **  Print a friendly message.
    */
    printf("CR405 initialised on channel %o\n", channelNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 405 card reader.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus cr405Func(ChSlot *activeChannel, DevSlot *activeDevice,
                          PpWord funcCode)
    {
    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case FcCr405Deselect:
    case FcCr405GateToSec:
        logError(LogErrorLocation, "channel %02o - unsupported function code: %4.4o", activeChannel->id, (u32)funcCode);
        break;

    case FcCr405ReadNonStop:
        activeDevice->fcode = funcCode;
        break;

    case FcCr405StatusReq:
        activeDevice->fcode = funcCode;
        if (activeDevice->fcb[0] == NULL)
            {
            retryOpen = 0;
            fcb = fopen(fname, "rb");
            if (fcb != NULL)
                {
                activeDevice->fcb[0] = fcb;
                activeDevice->status = StCr405Ready;
                }
            else
                {
                activeDevice->status = StCr405NotReady;
                }
            }

        break;
        }

    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 405 card reader.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void cr405Io(ChSlot *activeChannel, DevSlot *activeDevice)
    {
    int len;

    switch (activeDevice->fcode)
        {
    default:
    case FcCr405Deselect:
    case FcCr405GateToSec:
        logError(LogErrorLocation, "channel %02o - unsupported function code: %4.4o", activeChannel->id, activeDevice->fcode);
        break;

    case FcCr405StatusReq:
        activeChannel->data = activeDevice->status;
        activeChannel->full = TRUE;
        activeDevice->fcode = 0;
        break;

    case FcCr405ReadNonStop:
        if (activeChannel->full)
            {
            break;
            }

        if (activeDevice->fcb[0] == NULL && retryOpen++ > 100)
            {
            retryOpen = 0;
            fcb = fopen(fname, "rb");
            if (fcb != NULL)
                {
                activeDevice->fcb[0] = fcb;
                activeDevice->status = StCr405Ready;
                }
            else
                {
                activeDevice->status = StCr405NotReady;
                }
            }

        if (activeDevice->fcb[0] == NULL)
            {
            activeDevice->status = StCr405NotReady;
            break;
            }

        len = fread(&activeChannel->data, sizeof(u16), 1, activeDevice->fcb[0]);
        if (len != 1)
            {
            fclose(activeDevice->fcb[0]);
            remove(fname);
            retryOpen = 0;
            activeDevice->fcb[0] = NULL;
            activeChannel->data = 0;
            activeChannel->full = FALSE;
            }
        else
            {
            activeChannel->data &= Mask12;
            activeChannel->full = TRUE;
            }
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
static void cr405Activate(ChSlot *activeChannel, DevSlot *activeDevice)
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
static void cr405Disconnect(ChSlot *activeChannel, DevSlot *activeDevice)
    {
    }

/*---------------------------  End Of File  ------------------------------*/
