/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
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
static char asciiToConsole[128] =
    {
    /* 00-07 */ 0,      0,      0,      0,      0,      0,      0,      0,
    /* 08-0F */ 061,    0,      060,    0,      0,      060,    0,      0,
    /* 10-17 */ 0,      0,      0,      0,      0,      0,      0,      0,
    /* 18-1F */ 0,      0,      0,      0,      0,      0,      0,      0,
    /* 20-27 */ 062,    0,      0,      0,      0,      0,      0,      0,
    /* 28-2F */ 051,    052,    047,    045,    056,    046,    057,    050,
    /* 30-37 */ 033,    034,    035,    036,    037,    040,    041,    042,
    /* 38-3F */ 043,    044,    0,      0,      0,      054,    0,      0,
    /* 40-47 */ 0,      01,     02,     03,     04,     05,     06,     07,
    /* 48-4F */ 010,    011,    012,    013,    014,    015,    016,    017,
    /* 50-57 */ 020,    021,    022,    023,    024,    025,    026,    027,
    /* 58-5F */ 030,    031,    032,    053,    0,      055,    0,      0,
    /* 60-67 */ 0,      01,     02,     03,     04,     05,     06,     07,
    /* 68-6F */ 010,    011,    012,    013,    014,    015,    016,    017,
    /* 70-77 */ 020,    021,    022,    023,    024,    025,    026,    027,
    /* 78-7F */ 030,    031,    032,    0,      0,      0,      0,      0
    };

//static char consoleToAscii[64] =
char consoleToAscii[64] =
    {
    /* 00-07 */ 0,      'A',    'B',    'C',    'D',    'E',    'F',    'G',
    /* 10-17 */ 'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    /* 20-27 */ 'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    /* 30-37 */ 'X',    'Y',    'Z',    '0',    '1',    '2',    '3',    '4',
    /* 40-47 */ '5',    '6',    '7',    '8',    '9',    '+',    '-',    '*',
    /* 50-57 */ '/',    '(',    ')',    ' ',    '=',    ' ',    ',',    '.',
    /* 60-67 */  0,      0,      0,      0,      0,      0,      0,      0,
    /* 70-77 */  0,      0,      0,      0,      0,      0,      0,      0
    };

static u8 currentFont;
static u16 currentOffset;
static bool emptyDrop = FALSE;

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

    dp = channelAttach(channelNo, DtConsole);

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
    u8 c0;
    u8 c1;
    u8 ch;

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
            emptyDrop = FALSE;

            c0 = consoleToAscii[(activeChannel->data >> 6) & Mask6];
            c1 = consoleToAscii[(activeChannel->data >> 0) & Mask6];

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
                windowQueue(c0);
                windowQueue(c1);
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc6612Sel512DotsLeft:
    case Fc6612Sel512DotsRight:
        if (activeChannel->full)
            {
            emptyDrop = FALSE;

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
            windowGetChar();
            activeChannel->data = asciiToConsole[ppKeyIn];
            activeChannel->full = TRUE;
            activeChannel->status = 0;
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
    emptyDrop = TRUE;
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
    if (emptyDrop)
        {
        windowUpdate();
        emptyDrop = FALSE;
        }
    }

/*---------------------------  End Of File  ------------------------------*/
