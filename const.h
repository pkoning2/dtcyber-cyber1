#ifndef CONST_H
#define CONST_H
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
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
#define DtCyberVersion          "Desktop CYBER 2.1 BETA 0"

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
**  Macro for defining > 32 bit constants
*/
#if defined(__GNUC__)
#define ULL(x) x ## ULL
#else
#define ULL(x) ((u64)(x))
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
**  Hershey font support.
*/
#define CcHersheyFont           0

/*
**  Debug support
*/
#ifndef CcDebug
#define CcDebug                 1
#endif

/*
**  Device types.
*/
#define DtNone                  0
#define DtDeadStartPanel        1
#define DtMt607                 2
#define DtMt669                 3
#define DtDd6603                4
#define DtDd8xx                 5
#define DtCr405                 6
#define DtLp1612                7
#define DtLp5xx                 8
#define DtRtc                   9
#define DtConsole               10
#define DtMux6676               11
#define DtCp3446                12
#define DtCr3447                13
#define DtDcc6681               14
#define DtDdp                   15
#define DtNiu                   16
#define DtDd7155                17
#define DtDoelz                 18

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
#define MaxEquipment            010
#define MaxDeadStart            020
#define MaxChannels             040

#define NEWLINE                 "\n"

#define FontLarge               32
#define FontMedium              16
#define FontSmall               8
#define FontDot                 0

#define RefreshInterval         ULL(100000)  /* microseconds */

#define TraceEcs                (1 << 28)
#define TraceCpu0               (1 << 29)
#define TraceCpu1               (TraceCpu0 << 1)
#define TraceCpu(n)             (TraceCpu0 << (n))
#define TraceXj                 (1 << 31)

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
#define Mask48                  ULL(000007777777777777777)
#define Mask50                  ULL(000037777777777777777)
#define Mask60                  ULL(077777777777777777777)
#define MaskCoeff               ULL(000007777777777777777)
#define MaskExp                 ULL(037770000000000000000)
#define MaskNormalize           ULL(000004000000000000000)

/*
**  Sign extension and overflow.
*/
#define Overflow12              010000

#define Sign18                  0400000
#define Overflow18              01000000

#define Sign48                  ULL(000004000000000000000)

#define Sign60                  ULL(040000000000000000000)
#define Overflow60              ULL(0100000000000000000000)

#define SignExtend18To60        ULL(077777777777777000000)

#define NegativeZero            ULL(077777777777777777777)

/*
**  ----------------------
**  Public Macro Functions
**  ----------------------
*/

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

#define LogErrorLocation        __FILE__, __LINE__

/*---------------------------  End Of File  ------------------------------*/
#endif /* CONST_H */

