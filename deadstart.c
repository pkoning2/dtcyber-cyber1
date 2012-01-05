/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: deadstart.c
**
**  Description:
**      Perform simulation of CDC 6600 deadstart.
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
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

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
static FcStatus deadFunc(ChSlot *activeChannel, DevSlot *activeDevice,
                         PpWord funcCode);
static void deadIo(ChSlot *activeChannel, DevSlot *activeDevice);
static void deadActivate(ChSlot *activeChannel, DevSlot *activeDevice);
static void deadDisconnect(ChSlot *activeChannel, DevSlot *activeDevice);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
u16 deadstartPanel[MaxDeadStart];
u8 deadstartCount;


/*
**  -----------------
**  Private Variables
**  -----------------
*/
int dsSequence;       /* deadstart sequencer */

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Execute deadstart function.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void deadStart(void)
    {
    DevSlot *dp;
    int ch;

    dp = channelAttach(0, 0, DtDeadStartPanel);

    /*
    **  Set all channels to active and empty.
    */
    for (ch = 0; ch < chCount; ch++)
        {
        /*
        **  Don't do the special channels here, they are initialized 
        **  below, or in their specific init routines.
        */
        if (ch < ChClock || ch >= 020)
            {
            channel[ch].active = TRUE;
            channel[ch].full = FALSE;
            channel[ch].ioDevice = NULL;
            }
        }
    
    dp->activate = deadActivate;
    dp->disconnect = deadDisconnect;
    dp->func = deadFunc;
    dp->io = deadIo;
    dp->selectedUnit = 0;

    /*
    **  Set special channels appropriately.
    */
    channel[012               ].active = TRUE;
    channel[013               ].active = TRUE;
    channel[ChInterlock       ].active = TRUE;
    channel[ChStatusAndControl].active = FALSE;
    channel[ChMaintenance     ].active = FALSE;

    /*
    **  Reset deadstart sequencer.
    */
    dsSequence = -1;

    /*
    **  Start load of PPU0.
    */
    channel[0].ioDevice = dp;

    ppStart ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on deadstart pseudo device.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus deadFunc(ChSlot *activeChannel, DevSlot *activeDevice,
                         PpWord funcCode)
    {
    (void)activeChannel;
    (void)funcCode;

    return(FcDeclined);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on deadstart panel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void deadIo(ChSlot *activeChannel, DevSlot *activeDevice)
    {
    if (activeChannel->full ||
        !activeChannel->active)
        {
        return;
        }
    if (dsSequence < 0)
        {
        dsSequence++;
        activeChannel->data = 0;
        }
    else
        {
        activeChannel->data = deadstartPanel[dsSequence++] & Mask12;
        }
    if (dsSequence == deadstartCount)
        {
        activeChannel->active = FALSE;
        }
    else
        {
        activeChannel->full = TRUE;
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
static void deadActivate(ChSlot *activeChannel, DevSlot *activeDevice)
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
static void deadDisconnect(ChSlot *activeChannel, DevSlot *activeDevice)
    {
    }

/*---------------------------  End Of File  ------------------------------*/
