#ifndef TYPES_H
#define TYPES_H
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
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
    char            id[10];              /* device id */
    void            (*init)(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName);
    } DevDesc;

/*
**  Device control block.
*/                                        
typedef struct devSlot                  
    {                                   
    struct devSlot  *next;              /* next device attached to this channel or converter */
    struct chSlot   *channel;           /* channel this device is attached to */
    FILE            *fcb[MaxUnits];     /* unit data file control block */
    void            (*activate)(void);  /* channel activation function */        
    void            (*disconnect)(void);/* channel deactivation function */
    FcStatus        (*func)(PpWord);    /* function request handler */
    void            (*io)(void);        /* output request handler */
    void            (*load)(struct devSlot *, int, char *); /* load/unload request handler */
    void            *context[MaxUnits]; /* device specific context data */
    PpWord          status;             /* device status */
    PpWord          fcode;              /* device function code */
    PpWord          recordLength;       /* length of read record */
    u8              devType;            /* attached device type */
    u8              eqNo;               /* equipment number */
    i8              selectedUnit;       /* selected unit */
    } DevSlot;                          
                                        
/*
**  Channel control block.
*/                                        
typedef struct chSlot                          
    {                                   
    DevSlot         *firstDevice;       /* linked list of devices attached to this channel */
    DevSlot         *ioDevice;          /* device which deals with current function */
    PpWord          data;               /* channel data */
    PpWord          status;             /* channel status */
    bool            active;             /* channel active flag */
    bool            full;               /* channel full flag */
    bool            discAfterInput;     /* disconnect channel after input flag */
    u8              id;                 /* channel number */
    u8              delayStatus;        /* time to delay change of empty/full status */
    } ChSlot;                           
                                        
/*
**  PPU control block.
*/                                        
typedef struct                          
    {                                   
    ChSlot          *channel;           /* associated channel (-1 is none) */
    u32             regA;               /* register A (18 bit) */
    PpWord          regP;               /* program counter (12 bit) */
    PpWord          mem[PpMemSize];     /* PP memory */
    PpWord          delay;              /* time to delay before next instruction */
    bool            stopped;            /* PP stopped */
    u8              ioWaitType;         /* indicates what kind of I/O we wait for */
    u8              id;                 /* PP number */
    } PpSlot;                           

/*
**  CPU control block.
*/                                        
typedef struct                          
    {                                   
    CpWord          regX[010];          /* data registers (60 bit) */
    u32             regA[010];          /* address registers (18 bit) */
    u32             regB[010];          /* index registers (18 bit) */
    u32             regP;               /* program counter */
    u32             regRaCm;            /* reference address CM */
    u32             regFlCm;            /* field length CM */
    u32             regRaEcs;           /* reference address ECS */
    u32             regFlEcs;           /* field length ECS */
    u32             regMa;              /* monitor address */
    u32             regSpare;           /* reserved */
    u32             exitMode;           /* CPU exit mode (24 bit) */
    bool            monitorMode;        /* monitor mode bit */
    u8              exitCondition;      /* recorded exit conditions since XJ */
    } CpuContext;

/*---------------------------  End Of File  ------------------------------*/
#endif /* TYPES_H */

