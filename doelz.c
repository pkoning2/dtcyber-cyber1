/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: doelz.c
**
**  Description:
**      Perform simulation of Plato Doelz net controller
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
#include <sys/types.h>
#if defined(_WIN32)
#include <winsock.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
/*
**  -----------------
**  Private Constants
**  -----------------
*/

#define DEBUG       1

#define BLKMAX      330     /* max 320 bytes of data plus header plus a few */
#define MAXADDR     07777   /* max Doelz "network" address (really connection number) */

/*
**  Function codes.
*/
#define FcDlzOff                00000
#define FcDlzOn                 00001

/*
**  Flags in messages
*/
#define SUPFLGS 0x80            /* flags in flags/address field of supervisory message */

/*
**  Supervisory message function codes
*/
#define ALCALM0     0            /* all call message (types 0 - 5) */
#define ALCALM5     5
#define ALCLRM0     8            /* all call response (types 8 - 13) */
#define ALCLRM5     13           /* all call response (types 8 - 13) */
#define ALCGRM      16           /* all call group response */
#define SOLR        17           /* solicit response */
#define ESTL        18           /* establish link */
#define NAV1        19           /* node available (1) */
#define NAV2        20           /* node available (2) */
#define BUSY        21           /* busy */
#define NO          22           /* no */
#define DRPL        23           /* drop link */
#define ABORTR      25           /* abort response */
#define ABORTM      32           /* abort */
#define PTS         36           /* permission to send */
#define ABORTR      25           /* abort response */
#define ABORTM      32           /* abort */
#define NCRDM       33           /* network control read */
#define NCWRM       35           /* net control write */
#define PTS         36           /* permission to send */
#define NCRSPM      37           /* network control response */

/*
**  Input flags
*/
#define DOALCALM    1           /* deliver ALCALM0 message to channel */

/*
**  Misc
*/
#define ALCALDELAY  ULL(45000)  /* delay (microsec) between ALCALM0 messages */

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
typedef struct doelzNode
    {
    u32         ipAddr;
    u16         tcpPort;
    char        rid[3 + 1];
    char        name[7 + 1];
    } DoelzNode;

typedef struct linkParam
    {
    DoelzNode   *node;
    int         connFd;
    int         curBlkSize;
    int         curBlkType;
    int         curBlkOff;
    u8          curBlk[BLKMAX];
    bool        active;
    } LinkParam;

typedef struct connParam
    {
    LinkParam   *link;
    struct connParam *next;     /* Next connection with pending input data */
    u16         addr;
    } ConnParam;

/* Abort message (outbound) or permission to send (inbound) */

typedef struct abortMsg
    {
    u8          addrhi;         /* high order address */
    u8          addrlo;         /* low order address */
    } AbortMsg;
#define PrmsMsg AbortMsg

/* Key message */

typedef struct keyMsg
    {
    u8          addrhi;         /* high order address */
    u8          addrlo;         /* low order address */
    u8          key[2];         /* key data (two bytes) */
    u8          pad[7];         /* unused key data field and pad */
    } KeyMsg;

/*
**  Supervisory message (not counting the 2 ppu words at the end in 
**  the received message format)
*/

typedef struct supMsg
    {
    u8          zero;           /* a byte of zero (high order sup msg ident) */
    u8          one;            /* byte of one (low order ident) */
    u8          flaghi;         /* flags and high order address */
    u8          addrlo;         /* low order address */
    u8          data[9];        /* data bytes 2 to 10 */
    u8          pad;
    } SupMsg;
#define supfun  data[1]         /* supervisory message function code */

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus doelzInFunc(PpWord funcCode);
static void doelzInIo(void);
static FcStatus doelzOutFunc(PpWord funcCode);
static void doelzOutIo(void);
static void doelzActivate(void);
static void doelzInDisconnect(void);
static void doelzOutDisconnect(void);
static void doelzCreateThread(void);
static int doelzCheckInput(LinkParam *linkVector);
#if defined(_WIN32)
static void doelzThread(void *param);
#define RETURN return
#else
static void *doelzThread(void *param);
#define RETURN return 0
#endif

/*
**  ----------------
**  Public Variables
**  ----------------
*/
long doelzPort;
long doelzConns;
int doelzNodeCnt;
DoelzNode *doelzNodes;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static DevSlot *in;
static DevSlot *out;
static LinkParam *linkVector;
static ConnParam *connTable[MAXADDR + 1];
static ConnParam *first, *last;         /* list heads for pending input list */
static int incnt;                       /* byte index of next input byte  */
static int inflags;                     /* various input state flags */
static int ppiwc;                       /* count of input pp words in block */

static u8 outbuf[BLKMAX];
static int outcnt;                      /* byte index of next output byte in outbuf */
static int ppowc;                       /* count of pp words in outbuf */

static u32 lastAlCalm;

#if !defined(_WIN32)
static pthread_t doelz_thread;
#endif

static const SupMsg allCallMsg =
{
    0, 1, SUPFLGS, 0, 
    { 0, ALCALM0, 0, 0, 0, 0, 0, 0, 0 },
    0
};

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise Doelz
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      normally unit number, here abused as input channel.
**                  channelNo   output channel number.
**                  deviceName  optional device file name
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void doelzInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
{
    u8 i;
    LinkParam *mp;

    (void)eqNo;

    if (in != NULL)
    {
        fprintf (stderr, "Multiple Doelz devices not supported\n");
        exit (1);
    }

    /*
    **  We use two channels, one for input, one for output.
    **  The input channel number is given by what is normally
    **  the unit number in the cyber.ini file.
    */
    in = channelAttach(unitNo, eqNo, DtDoelz);
    in->activate = doelzActivate;
    in->disconnect = doelzInDisconnect;
    in->func = doelzInFunc;
    in->io = doelzInIo;
    out = channelAttach(channelNo, eqNo, DtDoelz);
    out->activate = doelzActivate;
    out->disconnect = doelzOutDisconnect;
    out->func = doelzOutFunc;
    out->io = doelzOutIo;

    mp = linkVector = calloc(1, sizeof(LinkParam) * doelzConns);
    if (linkVector == NULL)
    {
        fprintf(stderr, "Failed to allocate Doelz connection context block\n");
        exit(1);
    }

    /*
    **  Create the thread which will deal with TCP connections.
    */
    doelzCreateThread ();

    /*
    **  Print a friendly message.
    */
    printf("Doelz net initialised with input channel %o and output channel %o\n", unitNo, channelNo);
}

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on Doelz input channel.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus doelzInFunc(PpWord funcCode)
{
    return(FcDeclined);
}

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on Doelz output channel.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus doelzOutFunc(PpWord funcCode)
{
    switch (funcCode)
    {
        
    default:
        return(FcDeclined);

    case FcDlzOff:
        return(FcProcessed);
        
    case FcDlzOn:
        /*
        **  Connect the input channel to the input device, because there are NO
        **  functions on the input channel, so the normal way for that 
        **  association to be made doesn't work here.
        */
        in->channel->ioDevice = in;

        return(FcAccepted);
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on Doelz input channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void doelzInIo(void)
{
    int port;
    LinkParam *mp;
    PpWord d;
    u8 *p;
    int len;
    
    if (activeChannel->full)
        return;

    /*
    **  If we're not in the middle of a message and it has been
    **  long enough since the last one, send an All Call message
    **  to the PPU.  No, I don't know why.
    */
    if (incnt == 0 &&
        (inflags & DOALCALM) == 0 &&
        rtcClock - lastAlCalm > ALCALDELAY )
    {
        inflags |= DOALCALM;
    }
    
    /* If we need to send an All Call message, that takes precedence */
    if (inflags & DOALCALM)
    {
        p = (u8 *) &allCallMsg + incnt;
        len = sizeof (allCallMsg);
    }
    else if (first != NULL)
    {
        p = first->link->curBlk + incnt;
        len = first->link->curBlkSize;
    }
    else
    {
        /* If no data, the channel disconnects (!) */
        activeChannel->active = FALSE;
        return;
    }

    /*
    **  Output data.
    */
    if ((ppiwc & 1) == 0)
    {
        /*
        **  Even word.  Note that we have the even words ending on byte 
        **  boundaries, which means that the first word skips the first
        **  4 bits of the first byte.  That is done to keep the payload
        **  (starting at word 2) aligned on byte boundaries.  The payload
        **  is often byte-oriented, for example in supervisory messages,
        **  and doing it this way makes picking things apart easier.
        */
        d = p[incnt++] << 8;
        d |= p[incnt++];
    }
    else
    {
        /* Odd word */
        d = p[incnt++] << 4;
        d |= p[incnt] >> 4;
    }
    ppiwc++;
    activeChannel->data = d & Mask12;
    activeChannel->full = TRUE;
    if (incnt >= len)
    {
        activeChannel->discAfterInput = TRUE;
        incnt = 0;
        if (inflags & DOALCALM)
        {
            inflags &= ~DOALCALM;
        }
        else
        {
            first = first->next;
            if (first == NULL)
            {
                last == NULL;
            }
        }
    }
    
}

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on Doelz output channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void doelzOutIo(void)
{
    PpWord d;
    int port;
    struct timeval tm;
    u64 us;

    if (!activeChannel->full)
        return;
    
    /*
    **  If block is too long, disconnect and discard it.
    */
    if (outcnt >= BLKMAX - 1)
    {
        activeChannel->active = FALSE;
        activeChannel->full = FALSE;
        logError(LogErrorLocation, "Doelz channel %02o - output block too long",
                 activeChannel->id);
        outcnt = 0;
        return;
    }
    
    /*
    **  Output data.
    */
    d = activeChannel->data & Mask12;
    if ((ppowc & 1) == 0)
    {
        /*
        **  Even word.  Note that we have the even words ending on byte
        **  boundaries, which means that the first word skips the first
        **  4 bits of the first byte.  That is done to keep the payload
        **  (starting at word 2) aligned on byte boundaries.  The payload
        **  is often byte-oriented, for example in supervisory messages, 
        **  and doing it this way makes picking things apart easier.
        */
        outbuf[outcnt++] |= d >> 8;
        outbuf[outcnt++] = d;
    }
    else
    {
        /* Odd word */
        outbuf[outcnt++] = d >> 4;
        outbuf[outcnt] = d << 4;
    }
    ppowc++;
    activeChannel->full = FALSE;
}

/*--------------------------------------------------------------------------
**  Purpose:        Handle input or output channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void doelzActivate(void)
{
}

/*--------------------------------------------------------------------------
**  Purpose:        Handle disconnecting of input channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void doelzInDisconnect(void)
{
}

/*--------------------------------------------------------------------------
**  Purpose:        Handle disconnecting of output channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void doelzOutDisconnect(void)
{
    int addr = (outbuf[0] << 8) + outbuf[1];
    int i;
    AbortMsg *am;
    KeyMsg *km;
    SupMsg *sm;
    /* DataMsg *dm; */

#ifdef DEBUG
    printf ("doelz: output block len %d (%d ppwords), addr %04o",
            outcnt, ppowc, addr);
    for (i = 0; i < outcnt; i++)
    {
        if ((i % 16) == 0)
        {
            printf ("\n%03x: ", i);
        }
        printf (" %02x", outbuf[i]);
    }
    if ((outcnt % 16) != 0)
    {
        printf ("\n");
    }
    
#endif

    if (outcnt == 0)
    {
        return;
    }
    
    /*
    **  Use the length to sort out what's what.  Note that data messages
    **  contain a 12 bit address plus some number of 20 bit data words, which
    **  means the length cannot match the magic numbers for abort, key, or 
    **  supervisory messages.
    */
    if (ppowc == 1)
    {
        /* Abort */
#ifdef DEBUG
        printf ("doelz: abort from host\n");
#endif
    }
    else if (ppowc == 7)
    {
        /* Key */
#ifdef DEBUG
        km = (KeyMsg *) outbuf;
        printf ("doelz: key %04o\n", (km->key[0] << 4) + (km->key[1] >> 4));
#endif
    }
    else if (ppowc == 11)
    {
        if (addr != 1)
        {
            logError(LogErrorLocation, "Doelz channel %02o - supervisory block wrong address %04o",
                     activeChannel->id, addr);
            return;
        }
#ifdef DEBUG
        sm = (SupMsg *) outbuf;
        printf ("doelz: supervisory message code %d\n", sm->supfun);
#endif
    }
    else
    {
        /* Data block -- note that data block length cannot be 7 */
    }
    outcnt = ppowc = 0;
}

/*--------------------------------------------------------------------------
**  Purpose:        Create thread which will deal with all TCP
**                  connections.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void doelzCreateThread(void)
{
#if defined(_WIN32)
    static bool firstMux = TRUE;
    WORD versionRequested;
    WSADATA wsaData;
    int err;
    DWORD dwThreadId; 
    HANDLE hThread;

    if (firstMux)
    {
        firstMux = FALSE;

        /*
        **  Select WINSOCK 1.1.
        */ 
        versionRequested = MAKEWORD(1, 1);
 
        err = WSAStartup(versionRequested, &wsaData);
        if (err != 0)
        {
            fprintf(stderr, "\r\nError in WSAStartup: %d\r\n", err);
            exit(1);
        }
    }

    /*
    **  Create TCP thread.
    */
    hThread = CreateThread( 
        NULL,                                       /* no security attribute  */
        0,                                          /* default stack size  */
        (LPTHREAD_START_ROUTINE)doelzThread, 
        (LPVOID)0,                                  /* thread parameter  */
        0,                                          /* not suspended  */
        &dwThreadId);                               /* returns thread ID  */

    if (hThread == NULL)
    {
        fprintf(stderr, "Failed to create Doelz thread\n");
        exit(1);
    }
#else
    int rc;

    /*
    **  Create POSIX thread with default attributes.
    */
    rc = pthread_create(&doelz_thread, NULL, doelzThread, 0);
    if (rc < 0)
    {
        fprintf(stderr, "Failed to create Doelz thread\n");
        exit(1);
    }
#endif
}

/*--------------------------------------------------------------------------
**  Purpose:        TCP thread.
**
**  Parameters:     Name        Description.
**                  mp          pointer to mux parameters.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
#if defined(_WIN32)
static void doelzThread(void *param)
#else
static void *doelzThread(void *param)
#endif
{
    int listenFd;
    struct sockaddr_in server;
    struct sockaddr_in from;
    int fromLen;
    LinkParam *mp;
    u8 i;
    int reuse = 1;

    /*
    **  Create TCP socket and bind to specified port.
    */
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
    {
        printf("doelz: Can't create socket\n");
        RETURN;
    }

    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_port = htons(doelzPort);

    if (bind(listenFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Doelz: Can't bind to socket\n");
        RETURN;
    }

    if (listen(listenFd, 5) < 0)
    {
        printf("Doelz: Can't listen\n");
        RETURN;
    }

    while (1)
    {
        /*
        **  Find a free port control block.
        */
        mp = linkVector;
        for (i = 0; i < doelzConns; i++)
        {
            if (!mp->active)
            {
                break;
            }
            mp += 1;
        }

        if (i == doelzConns)
        {
            /*
            **  No free port found - wait a bit and try again.
            */
#if defined(_WIN32)
            Sleep(1000);
#else
            /*usleep(10000000); */
            sleep(1);
#endif
            continue;
        }

        /*
        **  Wait for a connection.
        */
        fromLen = sizeof(from);
        mp->connFd = accept(listenFd, (struct sockaddr *)&from, &fromLen);

        /*
        **  Mark connection as active.
        */
        mp->active = TRUE;
        printf("doelz: Received connection on port %d\n", mp - linkVector);
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Check for input.
**
**  Parameters:     Name        Description.
**                  mp          pointer to LinkParam struct for the connection.
**
**  Returns:        data byte received
**                  -1 if there is no data
**                  -2 if the connection was dropped
**
**------------------------------------------------------------------------*/
static int doelzCheckInput(LinkParam *mp)
{
    int i;
    fd_set readFds;
    fd_set exceptFds;
    struct timeval timeout;
    u8 data;

    FD_ZERO(&readFds);
    FD_ZERO(&exceptFds);
    FD_SET(mp->connFd, &readFds);
    FD_SET(mp->connFd, &exceptFds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    select(mp->connFd + 1, &readFds, NULL, &exceptFds, &timeout);
    if (FD_ISSET(mp->connFd, &readFds))
    {
        i = recv(mp->connFd, &data, 1, 0);
        if (i == 1)
        {
            return(data);
        }
        else
        {
#if defined(_WIN32)
            closesocket(mp->connFd);
#else
            close(mp->connFd);
#endif
            mp->active = FALSE;
            printf("doelz: Connection dropped on port %d\n", mp - linkVector);
            return(-2);
        }
    }
    else if (FD_ISSET(mp->connFd, &exceptFds))
    {
#if defined(_WIN32)
        closesocket(mp->connFd);
#else
        close(mp->connFd);
#endif
        mp->active = FALSE;
        printf("doelz: Connection dropped on port %d\n", mp - linkVector);
        return(-2);
    }
    else
    {
        return(-1);
    }
}




/*---------------------------  End Of File  ------------------------------*/
