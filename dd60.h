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
*/
#define Dd60KeyXon      070         /* start display */
#define Dd60KeyXoff     071         /* stop display */

#endif  /* DD60_H */
