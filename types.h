#ifndef TYPES_H
#define TYPES_H
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: types.h
**
**  Description:
**      This file defines global types.
**
**--------------------------------------------------------------------------
*/

/*
**  -----------------------
**  Public Type Definitions
**  -----------------------
*/

/*
**  Basic types used in emulation.
*/
#if defined(_WIN32)
    /*
    **  MS Win32 systems
    */
    typedef signed char  i8;
    typedef signed short i16;
    typedef signed long  i32;
    typedef signed __int64 i64;
    typedef unsigned char  u8;
    typedef unsigned short u16;
    typedef unsigned long  u32;
    typedef unsigned __int64 u64;
    #define FMT60_020o "%020I64o"
#elif defined (__GNUC__)
    #if defined(__alpha__) || defined(__powerpc64__) || (defined(__sparc64__) && defined(__arch64__))
        /*
        **  64 bit systems
        */
        typedef signed char i8;
        typedef signed short i16;
        typedef signed int i32;
        typedef unsigned long int i64;
        typedef unsigned char u8;
        typedef unsigned short u16;
        typedef unsigned int u32;
        typedef unsigned long int u64;
        #define FMT60_020o "%020lo"
    #elif defined(__i386__) || defined(__powerpc__) || defined(__sparc__) || defined(__hppa__)
        /*
        **  32 bit systems
        */
        typedef signed char i8;
        typedef signed short i16;
        typedef signed int i32;
        typedef unsigned long long int i64;
        typedef unsigned char u8;
        typedef unsigned short u16;
        typedef unsigned int u32;
        typedef unsigned long long int u64;
        #define FMT60_020o "%020llo"
    #else
        #error "Unable to determine size of basic data types"
    #endif
#else
    #error "Unable to determine size of basic data types"
#endif

#if (!defined(__cplusplus) && !defined(bool) && !defined(CURSES) && !defined(CURSES_H) && !defined(_CURSES_H))
    typedef int bool;
#endif

typedef u16 PpWord;                     /* 12 bit PP word */
typedef u8 PpByte;                      /* 6 bit PP word */
typedef u64 CpWord;                     /* 60 bit CPU word */

/*
**  Function code processing status.
*/
typedef enum {FcDeclined, FcAccepted, FcProcessed} FcStatus;

/*
**  Device descriptor.
*/                                        
typedef struct
    {
    char            id[8];              /* device id */
    void            (*init)(u8 eqNo, u8 unitCount, u8 channelNo, char *deviceName);
    } DevDesc;
                                        
/*
**  Device control block.
*/                                        
typedef struct devSlot                  
    {                                   
    struct devSlot  *next;              /* next device attached to this channel */
    struct chSlot   *channel;           /* channel this device is attached to */
    u8              devType;            /* attached device type */
    void            *context[MaxUnits]; /* device specific context data */
    PpWord          status;             /* device status */
    PpWord          fcode;              /* device function code */
    i8              selectedUnit;       /* selected unit */
    PpWord          recordLength;       /* length of read record */
    FILE            *fcb[MaxUnits];     /* unit data file control block */
    FILE            *log[MaxUnits];     /* unit log file control block */
    void            (*activate)(void);  /* channel activation function */        
    void            (*disconnect)(void);/* channel deactivation function */
    FcStatus        (*func)(PpWord);    /* function request handler */
    void            (*io)(void);        /* output request handler */
    } DevSlot;                          
                                        
/*
**  Channel control block.
*/                                        
typedef struct chSlot                          
    {                                   
    u8              id;                 /* channel number */
    bool            active;             /* channel active flag */
    bool            full;               /* channel full flag */
    bool            discAfterInput;     /* disconnect channel after input flag */
    PpWord          data;               /* channel data */
    PpWord          status;             /* channel status */
    DevSlot         *firstDevice;       /* linked list of devices attached to this channel */
    DevSlot         *ioDevice;          /* device which deals with current function */
    } ChSlot;                           
                                        
/*
**  PPU control block.
*/                                        
typedef struct                          
    {                                   
    u8              id;                 /* PP number */
    bool            stopped;            /* PP stopped */
    u32             regA;               /* Register A (18 bit) */
    PpWord          regP;               /* Program counter (12 bit) */
    ChSlot          *channel;           /* associated channel (-1 is none) */
    u8              ioWaitType;         /* Indicates what kind of I/O we wait for */
    PpWord          mem[PpMemSize];     /* PP memory */
    } PpSlot;                           

/*
**  CPU control block.
*/                                        
typedef struct                          
    {                                   
    u32             regA[010];          /* Address registers (18 bit) */
    u32             regB[010];          /* Index registers (18 bit) */
    CpWord          regX[010];          /* Data registers (60 bit) */
    u32             regP;               /* Program counter */
    u32             regRaCm;            /* Reference address CM */
    u32             regFlCm;            /* Field length CM */
    u32             regRaEcs;           /* Reference address ECS */
    u32             regFlEcs;           /* Field length ECS */
    u32             regMa;              /* Monitor address */
    bool            monitorMode;        /* Monitor mode bit */
    u32             regSpare;           /* Reserved */
    u32             exitMode;           /* CPU exit mode (24 bit) */
    u8              exitCondition;      /* Recorded exit conditions since XJ */
    } CpuContext;

/*---------------------------  End Of File  ------------------------------*/
#endif /* TYPES_H */

