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

#include <sys/types.h>
#if defined(_WIN32)
#include <winsock.h>
typedef u_long in_addr_t;
#define EINPROGRESS WSAEINPROGRESS
#else
#include <stdbool.h>
#include <netinet/in.h>
#define _POSIX_C_SOURCE_199309L
#include <unistd.h>
#endif

#ifdef _POSIX_ASYNCHRONOUS_IO
#if defined(__APPLE__)
#include <sys/aio.h>
#else
#include <aio.h>
#endif
#endif

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
#if !defined(__cplusplus)
	typedef int bool;
#endif
    typedef signed char  i8;
    typedef signed short i16;
    typedef signed long  i32;
    typedef signed __int64 i64;
    typedef unsigned char  u8;
    typedef unsigned short u16;
    typedef unsigned long  u32;
    typedef unsigned __int64 u64;
    #define FMT60_020o "%020I64o"
    #define FMT60_08o "%08I64o"
	typedef unsigned int socklen_t;
#elif defined (__GNUC__)
    #if defined(__LONG_MAX__)
        #if (__LONG_MAX__ == __INT_MAX__)
        #define LSIZE 32
        #else
        #define LSIZE 64
        #endif
    #elif defined(__alpha__) || defined(__powerpc64__) || (defined(__sparc64__) && defined(__arch64__))
    #define LSIZE 64
    #elif defined(__i386__) || defined(__powerpc__) || defined(__sparc__) || defined(__hppa__)
    #define LSIZE 32
    #else
        #error "Unable to determine size of basic data types"
    #endif
    #if (LSIZE == 64)
        /*
        **  64 bit systems
        */
        typedef signed char i8;
        typedef signed short i16;
        typedef signed int i32;
        typedef signed long int i64;
        typedef unsigned char u8;
        typedef unsigned short u16;
        typedef unsigned int u32;
        typedef unsigned long int u64;
        #define FMT60_020o "%020lo"
        #define FMT60_08o "%08lo"
    #else
        /*
        **  32 bit systems
        */
        typedef signed char i8;
        typedef signed short i16;
        typedef signed int i32;
        typedef signed long long int i64;
        typedef unsigned char u8;
        typedef unsigned short u16;
        typedef unsigned int u32;
        typedef unsigned long long int u64;
        #define FMT60_020o "%020llo"
        #define FMT60_08o "%08llo"
    #endif
#else
    #error "Unable to determine size of basic data types"
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
    void            *context[MaxUnits2];/* device specific context data */
    void            *controllerContext; /* controller specific context data */
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
    u8              delayDisconnect;    /* time to delay disconnect */
    } ChSlot;                           
                                        
/*
**  PPU control block.
*/                                        
typedef struct                          
    {                                   
    ChSlot          *channel;           /* associated channel (-1 is none) */
    u32             regA;               /* Register A (18 bit) */
    int             delay;              /* Time to delay before next instruction */
    PpWord          regP;               /* Program counter (12 bit) */
    PpWord          mem[PpMemSize];     /* PP memory */
    u16             ppMemStart;         /* Start of IAM/OAM for tracing */
    u16             ppMemLen;           /* Length of IAM/OAM for tracing */
    u8              ioWaitType;         /* Indicates what kind of I/O we wait for */
    u8              id;                 /* PP number */
    bool            stopped;            /* PP stopped */
    bool            traceLine;          /* Trace one line for this PP */
    bool            ioFlag;             /* Last instruction was an I/O */
    } PpSlot;                           

/*
**  CPU control block.
*/                                        
typedef struct                          
    {                                   
    CpWord          regX[010];          /* Data registers (60 bit) */
    CpWord          opWord;             /* Current opcode word (60 bit) */
    u32             opAddress;          /* Current opcode address (18 bit) */
    u32             regA[010];          /* Address registers (18 bit) */
    u32             regB[010];          /* Index registers (18 bit) */
    u32             regP;               /* Program counter */
    u32             regRaCm;            /* Reference address CM */
    u32             regFlCm;            /* Field length CM */
    u32             regRaEcs;           /* Reference address ECS */
    u32             regFlEcs;           /* Field length ECS */
    u32             regMa;              /* Monitor address */
    u32             regSpare;           /* Reserved */
    u32             exitMode;           /* CPU exit mode (24 bit) */
    u32             exchangeTo;         /* Address to exchange to */
    u8              exitCondition;      /* Recorded exit conditions since XJ */
    u8              id;                 /* CPU number */
    bool            cpuStopped;         /* CPU stop flag */
    u8              opOffset;           /* Bit offset to current instruction */
    u8              opFm;               /* Opcode field (first 6 bits) */
    u8              opI;                /* I field of current instruction */
    u8              opJ;                /* J field of current instruction */
    u8              opK;                /* K field (first 3 bits only) */
    } CpuContext;
/*
**  Network "FET"
**
**  Ok, it's not quite a classic FET, but it's pretty similar and it
**  supports many of the same kinds of operations.
*/
typedef struct NetFet_s
    {
    struct NetFet_s *prev;              /* circular list pointers */
    struct NetFet_s *next;
    int         connFd;                 /* File descriptor for socket */
    u8          *first;                 /* Start of ring buffer */
    volatile u8 *in;                    /* Fill (write) pointer */
    volatile u8 *out;                   /* Empty (read) pointer */
    u8          *end;                   /* End of ring buffer + 1 */
    struct in_addr from;                /* remote IP address */
    int         fromPort;               /* remote TCP port number */
    u8          *sendData;              /* Pending data to send */
    int         sendCount;              /* Count of sendData bytes */
    int         sendBufCount;           /* Size of sendData realloc */
    struct NetPortSet_s *ps;            /* PortSet this belongs to */
    u64         ownerInfo;              /* Data supplied by socket owner */
    u8          inUse;                  /* In use flag */
    } NetFet;


/*
**  Callback function for new connection.
*/
typedef void (ConnCb) (NetFet *np, int portNum, void *arg);

/*
**  Tread function
*/
#if defined(_WIN32)
typedef void ThreadFunRet;
#define ThreadReturn return
#else
typedef void * ThreadFunRet;
#define ThreadReturn return 0
#endif

/*
**  Network port set.
*/
typedef struct NetPortSet_s
    {
    int         maxPorts;               /* total number of ports */
    volatile int curPorts;              /* number of ports currently active */
    NetFet      *portVec;               /* array of NetFets */
    int         listenFd;               /* listen socket fd */
    int         portNum;                /* TCP port number to listen to */
    fd_set      activeSet;              /* fd_set for active port fd's */
    fd_set      sendSet;                /* fd_set for fd's with send data */
    int         sendCount;              /* count of fd's in sendSet */
    int         maxFd;                  /* highest port fd value */
    ConnCb      *callBack;              /* function to call for new conn */
    void        *callArg;               /* argument to the above */
    const char  *kind;                  /* What is this portset for? */
    bool        localOnly;              /* TRUE to listen on 127.0.0.1 */
    bool        close;                  /* TRUE to shut down thread */
    } NetPortSet;

/*
**  Apple doesn't have a NOSIGNAL option on rcv/send, instead is has
**  SO_NOSIGPIPE as a setsockopt option.
**
**  Windows doesn't have either; it seems not to do SIGPIPE ever.
*/
#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

/*
**  Disk I/O control struct.
**
**  This support async I/O, if the host OS has it.
*/

typedef struct 
    {
#ifdef _POSIX_ASYNCHRONOUS_IO
    struct aiocb iocb;
#endif
    off_t       pos;
    void        *buf;
    int         fd;
    int         count;
    bool        ioPending;
    } DiskIO;

/*---------------------------  End Of File  ------------------------------*/
#endif /* TYPES_H */

