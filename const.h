#ifndef CONST_H

#define CONST_H

/*--------------------------------------------------------------------------

**

**  Copyright (c) 2003, Tom Hunter (see license.txt)

**

**  Name: const.h

**

**  Description:

**      This file defines public constants and macros

**

**--------------------------------------------------------------------------

*/



/*

**  ----------------

**  Public Constants

**  ----------------

*/

#define DtCyberVersion          "Desktop CYBER 2.0 BETA 1"



#ifndef NULL                    

#define NULL                    ((void *) 0)

#endif                          

                                

#ifndef FALSE

#define FALSE                   0

#endif



#ifndef TRUE

#define TRUE                    (!FALSE)

#endif



/*

**  Conditional compiles:

**  ====================

*/



/*

**  CMU option enable.

*/

#define CcCMU                   1



/*

**  Large screen support.

*/

#define CcLargeWin32Screen      0



/*

**  Debug support

*/

#ifndef CcDebug

#define CcDebug                 0

#endif



/*

**  Device types.

*/

#define DtNone                  0

#define DtDeadStartPanel        1

#define DtMt607                 2

#define DtMt669                 3

#define DtDd6603                4

#define DtDd844                 5

#define DtCr405                 6

#define DtLp1612                7

#define DtRtc                   10

#define DtConsole               11

#define DtMux6676               12

#define DtCr3447                13

#define DtDdc6681               14

#define DtDdp                   15

#define DtNiu                   16

#define DtLp3000                17



/*

**  Special channels.

*/

#define ChClock                 014

#define ChInterlock             015

#define ChStatusAndControl      016

#define ChMaintenance           017



/*

**  Misc constants.

*/

#define PpMemSize               010000

                                

#define MaxUnits                010

#define MaxDevices              10

#define MaxDeadStart            16

#define MaxChannels             040



#define NEWLINE                 "\n"



#define FontLarge               32

#define FontMedium              16

#define FontSmall               8

#define FontDot                 0



#define RefreshInterval         100000  /* microseconds */



/*

**  PP wait for I/O types

*/

#define WaitNone                0x00

                                

#define WaitInOne               0x01

#define WaitInMany              0x02

#define WaitIn                  (WaitInOne | WaitInMany)

                                

#define WaitOutOne              0x04

#define WaitOutMany             0x08

#define WaitOut                 (WaitOutOne | WaitOutMany)

                                

#define WaitMany                (WaitInMany | WaitOutMany)

                                

#define WaitHung                0x20



/*

**  Bit masks.

*/

#define Mask3                   07

#define Mask4                   017

#define Mask6                   077

#define Mask9                   0777

#define Mask10                  01777

#define Mask11                  03777

#define Mask12                  07777

#define Mask15                  077777

#define Mask18                  0777777

#define Mask24                  077777777

#define Mask24Ecs               077777700

#define Mask30                  07777777777

#define Mask48                  000007777777777777777

#define Mask50                  000037777777777777777

#define Mask60                  077777777777777777777

#define MaskCoeff               000007777777777777777

#define MaskExp                 037770000000000000000

#define MaskNormalize           000004000000000000000



/*

**  Sign extension and overflow.

*/

#define Overflow12              010000



#define Sign18                  0400000

#define Overflow18              01000000



#define Sign48                  000004000000000000000



#define Sign60                  040000000000000000000

#define Overflow60              0100000000000000000000



#define SignExtend18To60        077777777777777000000



#define NegativeZero            077777777777777777777



/*

**  ----------------------

**  Public Macro Functions

**  ----------------------

*/

#if 0

#define ppAbort(pars) \

    { \

    fprintf(stderr, "PP abort in %s at line %d : ", __FILE__, __LINE__); \

    fprintf pars; \

    fprintf(stderr, NEWLINE); \

    dumpAll(); \

    exit(1); \

    } 

#else

#define ppAbort(pars) \

    { \

    fprintf(stdout, "PP abort in %s at line %d : ", __FILE__, __LINE__); \

    fprintf pars; \

    fprintf(stdout, NEWLINE); \

    } 

#endif







/*---------------------------  End Of File  ------------------------------*/

#endif /* CONST_H */



