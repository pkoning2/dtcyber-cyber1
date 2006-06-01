#ifndef DD60_H
#define DD60_H
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: const.h
**
**  Description:
**      This file defines constants for communication between the
**      console emulation in DtCyber and the display programs.
**
**--------------------------------------------------------------------------
*/

/*
**  ----------------
**  Public Constants
**  ----------------
*/

/*
**  Command codes
**
**  Each of this is a byte value with the high order bit set, and
**  the low 3 bits left available for variable data.
*/
#define Dd60SetX        0200
#define Dd60SetY        0210
#define Dd60SetTrace    0220        /* Position to start of trace data area */
#define Dd60SetKbTrue   0230
#define Dd60SetMode     0240        /* Mode and screen */
#define Dd60EndBlock    0250        /* End of a data block */

/*
**  Mode/screen codes
**
**  The SetMode command has a screen plus a char/dot code in the low bits.
*/
#define Dd60ScreenL     0
#define Dd60ScreenR     4
#define Dd60CharSmall   0
#define Dd60CharMedium  1
#define Dd60CharLarge   2
#define Dd60Dot         3

/*
**  Data type codes in upper 2 bits of incoming data (display to dtcyber)
*/
#define Dd60KeyDown     0000
#define Dd60KeyUp       0100
#define Dd60FastRate    0200
#define Dd60SlowRate    0300

/*
**  Special actions encoded as keycodes outside the normal keyboard range
**  (063-077) and KeyDown or KeyUp for up to 26 special codes.
*/
#define Dd60TraceCp0    0063        /* toggle CPU0 trace */
#define Dd60TraceCp1    0064        /* toggle CPU1 trace */
#define Dd60TraceEcs    0065        /* toggle ECS trace */
#define Dd60TraceXj     0066        /* toggle XJ trace */
#define Dd60TraceAll    0067        /* toggle all traces */
#define Dd60KeyXon      0070        /* start display */
#define Dd60KeyXoff     0071        /* stop display */
#define Dd60KeyTest     0072        /* test key (alt N) */
#define Dd60TracePp0    0163        /* toggle PP0 trace */
/*      ... through     0174        ** toggle PP11 trace */

/*
**  Screen related definitions
*/
#define DisplayMargin	        20
#define OffLeftScreen           0
#define OffRightScreen          01020
#define TraceX                  (10 - DisplayMargin)
#define TraceY                  (10 - DisplayMargin)

/*
**  Size of the window and pixmap.
**   This is: a screen high with marging top and botton, and two screens
**  wide with margins top and bottom, and 20b space in between.
*/
#define XSize       (01000 * scaleX / 10 + OffRightScreen + 2 * DisplayMargin)
#define YSize       (01000 * scaleY / 10 + 2 * DisplayMargin)

#define XADJUST(x) ((x) * scaleX / 10 + DisplayMargin + currentXOffset)
#define YADJUST(y) ((y) * scaleY / 10 + DisplayMargin)

#endif  /* DD60_H */
