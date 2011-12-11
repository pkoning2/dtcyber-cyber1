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
#define DtCyberVersion          "Desktop CYBER 2.2 ALPHA 3 (GPK)"

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
#define DtTpm                   15
#define DtDdp                   16
#define DtNiu                   17
#define DtNpu                   18
#define DtMt679                 19

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
#define MaxUnits2               020
#define MaxEquipment            010
#define MaxDeadStart            0200
#define MaxChannels             040

#define NEWLINE                 "\n"

#define FontLarge               32
#define FontMedium              16
#define FontSmall               8
#define FontDot                 0

#define RefreshInterval         100000          /* microseconds */

#define TraceCh                 (1 << 27)       /* Some channel is being traced */
#define TraceEcs                (1 << 28)
#define TraceCpu0               (1 << 29)
#define TraceCpu1               (TraceCpu0 << 1)
#define TraceCpu(n)             (TraceCpu0 << (n))
#define TraceXj                 (1 << 31)

/*
**  TCP port numbers
*/
#define DefTelnetPort   5000        /* mux6676 default port */
#define DefTpmuxPort    5002        /* tpmux default port */
#define DefNiuPort      5004        /* Plato NIU default port */
#define DefOpPort       5006        /* Operator interface default port */
#define DefDd60Port     5007        /* DD60 interface default port */
#define DefDoelzPort    5008        /* Default Doelz net port */

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
#define Mask2                   03
#define Mask3                   07
#define Mask4                   017
#define Mask5                   037
#define Mask6                   077
#define Mask7                   0177
#define Mask8                   0377
#define Mask9                   0777
#define Mask10                  01777
#define Mask11                  03777
#define Mask12                  07777
#define Mask15                  077777
#define Mask18                  0777777
#define Mask21                  07777777
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
**  Other
*/
#define MINUS1                  ((~1) & Mask60)

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

/* 
**  Macros for enabling dual CPU emulation.  These are constructed
**  so the case when dual CPU emulation is turned off do NOT get the
**  added (even if small) overhead of passing a cpu context pointer
**  to all the emulation primitives.
*/
#ifdef DUAL_CPU
#define CPUVARG         CpuContext *activeCpu
#define CPUVARGS1(x)    CpuContext *activeCpu, x
#define CPUVARGS2(x, y) CpuContext *activeCpu, x, y
#define CPUVARGS3(x, y, z) CpuContext *activeCpu, x, y, z
#define CPUARG          activeCpu
#define CPUARGS1(x)     activeCpu, x
#define CPUARGS2(x, y)  activeCpu, x, y
#define CPUARGS3(x, y, z) activeCpu, x, y, z
#else
#define CPUVARG         void
#define CPUVARGS1(x)    x
#define CPUVARGS2(x, y) x, y
#define CPUVARGS3(x, y, z) x, y, z
#define CPUARG
#define CPUARGS1(x)     x
#define CPUARGS2(x, y)  x, y
#define CPUARGS3(x, y, z) x, y, z
#define cpuStep         cpuStepAll
#endif

/*---------------------------  End Of File  ------------------------------*/
#endif /* CONST_H */

