/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: console.c
**
**  Description:
**      Perform simulation of CDC 6612 console.
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
**  CDC 6612 console functions and status codes.
*/
#define Fc6612Sel64CharLeft     07000
#define Fc6612Sel32CharLeft     07001
#define Fc6612Sel16CharLeft     07002

#define Fc6612Sel512DotsLeft    07010
#define Fc6612Sel512DotsRight   07110
#define Fc6612SelKeyIn          07020

#define Fc6612Sel64CharRight    07100
#define Fc6612Sel32CharRight    07101
#define Fc6612Sel16CharRight    07102

#define OffLeftScreen           0
#define OffRightScreen          01020

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
static FcStatus consoleFunc(PpWord funcCode);
static void consoleIo(void);
static void consoleActivate(void);
static void consoleDisconnect(void);

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
static u8 currentFont;
static u16 currentOffset;
static u64 keyMask;
static u8 currentKey;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise 6612 console.
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
void consoleInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;

    (void)eqNo;
    (void)unitNo;
    (void)deviceName;

    dp = channelAttach(channelNo, eqNo, DtConsole);

    dp->activate = consoleActivate;
    dp->disconnect = consoleDisconnect;
    dp->selectedUnit = 0;
    dp->func = consoleFunc;
    dp->io = consoleIo;

    /*
    **  Initialise (X)Windows environment.
    */
    windowInit();

    /*
    **  Print a friendly message.
    */
    printf("Console initialised on channel %o\n", channelNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 6612 console.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus consoleFunc(PpWord funcCode)
    {
    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case Fc6612Sel512DotsLeft:
        currentFont = FontDot;
        currentOffset = OffLeftScreen;
        windowSetFont(currentFont);
        break;

    case Fc6612Sel512DotsRight:
        currentFont = FontDot;
        currentOffset = OffRightScreen;
        windowSetFont(currentFont);
        break;

    case Fc6612Sel64CharLeft:
        currentFont = FontSmall;
        currentOffset = OffLeftScreen;
        windowSetFont(currentFont);
        break;

    case Fc6612Sel32CharLeft:
        currentFont = FontMedium;
        currentOffset = OffLeftScreen;
        windowSetFont(currentFont);
        break;

    case Fc6612Sel16CharLeft:
        currentFont = FontLarge;
        currentOffset = OffLeftScreen;
        windowSetFont(currentFont);
        break;

    case Fc6612Sel64CharRight:
        currentFont = FontSmall;
        currentOffset = OffRightScreen;
        windowSetFont(currentFont);
        break;

    case Fc6612Sel32CharRight:
        currentFont = FontMedium;
        currentOffset = OffRightScreen;
        windowSetFont(currentFont);
        break;

    case Fc6612Sel16CharRight:
        currentFont = FontLarge;
        currentOffset = OffRightScreen;
        windowSetFont(currentFont);
        break;

    case Fc6612SelKeyIn:
        break;
        }

    activeDevice->fcode = funcCode;
    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 6612 console.
**
**  Parameters:     Name        Description.
**                  device      Device control block
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void consoleIo(void)
    {
    u8 ch;
    int i;
    u64 m;
    
    switch (activeDevice->fcode)
        {
    default:
        break;

    case Fc6612Sel64CharLeft:
    case Fc6612Sel32CharLeft:
    case Fc6612Sel16CharLeft:
    case Fc6612Sel64CharRight:
    case Fc6612Sel32CharRight:
    case Fc6612Sel16CharRight:
        if (activeChannel->full)
            {
            ch = (u8)((activeChannel->data >> 6) & Mask6);

            if (ch >= 060)
                {
                if (ch >= 070)
                    {
                    /*
                    **  Vertical coordinate.
                    */
                    windowSetY((u16)(activeChannel->data & Mask9));
                    }
                else
                    {
                    /*
                    **  Horizontal coordinate.
                    */
                    windowSetX((u16)((activeChannel->data & Mask9) + currentOffset));
                    }
                }
            else
                {
                windowQueue((char)((activeChannel->data >> 6) & Mask6));
                windowQueue((char)((activeChannel->data >> 0) & Mask6));
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc6612Sel512DotsLeft:
    case Fc6612Sel512DotsRight:
        if (activeChannel->full)
            {
            ch = (u8)((activeChannel->data >> 6) & Mask6);

            if (ch >= 060)
                {
                if (ch >= 070)
                    {
                    /*
                    **  Vertical coordinate.
                    */
                    windowSetY((u16)(activeChannel->data & Mask9));
                    windowQueue('.');
                    }
                else
                    {
                    /*
                    **  Horizontal coordinate.
                    */
                    windowSetX((u16)((activeChannel->data & Mask9) + currentOffset));
                    }
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc6612SelKeyIn:
        if (!activeChannel->full)
            {
            /* For normal ("easy") keyboard mode we get one buffered keycode
            ** at a time.
            ** For true keyboard mode, we keep collecting buffered keycodes
            ** until we run out of news.  That way if two keys are pressed
            ** between the previous poll and the current one, we deliver
            ** those two keystrokes together rather than one at a time.
            */
            do
                {
                windowGetChar();
                ch = asciiToConsole[ppKeyIn & 0177];
                if (ppKeyIn == 0)
                    {
                    keyMask = ULL(0);
                    }
                else if (ppKeyIn == 0200)   /* 0200 means "no change" */
                    {
                    break;
                    }
                else if (ch != 0)
                    {
                    if (ppKeyIn & 0200)
                        {
                        keyMask &= ~(ULL(1) << ch);
                        }
                    else
                        {
                        keyMask |= ULL(1) << ch;
                        }
                    }
                }
            while (keyboardTrue);
                
            currentKey = 0;
            for (i = 0, m = 1; i < 64; i++, m <<= 1)
                {
                if (keyMask & m)
                    {
                    currentKey |= i;
                    }
                }
            activeChannel->data = currentKey;
            activeChannel->full = TRUE;
            activeChannel->status = 0;
            activeDevice->fcode = 0;
            ppKeyIn = 0;
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
static void consoleActivate(void)
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
static void consoleDisconnect(void)
    {
    }

/*---------------------------  End Of File  ------------------------------*/
