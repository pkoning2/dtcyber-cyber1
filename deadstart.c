/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
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
static FcStatus deadFunc(PpWord funcCode);
static void deadIo(void);
static void deadActivate(void);
static void deadDisconnect(void);

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
static u8 dsSequence;       /* deadstart sequencer */

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
    u8 pp;
    u8 ch;

    dp = channelAttach(0, DtDeadStartPanel);

    dp->activate = deadActivate;
    dp->disconnect = deadDisconnect;
    dp->func = deadFunc;
    dp->io = deadIo;
    dp->selectedUnit = 0;

    /*
    **  Set lower channels to active.
    */
    for (ch = 0; ch < channelCount; ch++)
        {
        if (ch < 014)
            {
            channel[ch].active = TRUE;
            }
        }

    /*
    **  Set special channels appropriately.
    */
//    channel[ChInterlock       ].active = FALSE;
    channel[ChInterlock       ].active = TRUE;
    channel[ChStatusAndControl].active = FALSE;
    channel[ChMaintenance     ].active = FALSE;

    /*
    **  Reset deadstart sequencer.
    */
    dsSequence = 0;

    for (pp = 0; pp < ppuCount; pp++)
        {
        /*
        **  Initialise PPU.
        */
        memset(ppu + pp, 0, sizeof(ppu[0]));
        ppu[pp].id = pp;

        /*
        **  Assign PPs to the corresponding channels.
        */
        ppu[pp].channel = channel + pp;
        channel[pp].active = TRUE;

        /*
        **  Set all PPs to INPUT (71) instruction.
        */
        ppu[pp].ioWaitType = WaitInMany;
        ppu[pp].stopped = TRUE;

        /*
        **  Clear P registers and location zero of each PP.
        */
        ppu[pp].regP   = 0;
        ppu[pp].mem[0] = 0;

        /*
        **  Set all A registers to an input word count of 10000.
        */
        ppu[pp].regA = 010000;
        }

    /*
    **  Start load of PPU0.
    */
    channel[0].ioDevice = dp;
    channel[0].active = TRUE;
    channel[0].full = TRUE;
    channel[0].data = 0;
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
static FcStatus deadFunc(PpWord funcCode)
    {
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
static void deadIo(void)
    {
    activeChannel->data = deadstartPanel[dsSequence++] & Mask12;
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
static void deadActivate(void)
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
static void deadDisconnect(void)
    {
    }

/*---------------------------  End Of File  ------------------------------*/
