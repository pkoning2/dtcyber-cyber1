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

#define BLKMAX      330     // max 320 bytes of data plus header plus a few
#define MAXADDR     07777   // max Doelz "network" address (really connection number)

/*
**  Function codes.
*/
#define FcDlzOff                00000
#define FcDlzOn                 00001

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
    int         curBlkSiz;
    int         curBlkType;
    int         curBlkOff;
    u8          curBlk[BLKMAX];
    bool        active;
} LinkParam;

typedef struct connParam
{
    LinkParam   *link;
    struct connParam *next;     // Next connection with pending input data
    u16         addr;
} ConnParam;

    
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
u16 doelzPort;
u16 doelzConns;
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
static ConnParam *first, *last;         // list heads for pending input list
static int incnt;                       // byte index of next input byte of "first"
static u8 outbuf[BLKMAX];
static int outcnt;                      // byte index of next output byte in outbuf
static int ppwc;                        // count of pp words in outbuf

#if !defined(_WIN32)
static pthread_t doelz_thread;
#endif


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

    // We use two channels, one for input, one for output.
    // The input channel number is given by what is normally
    // the unit number in the cyber.ini file.
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
        // Connect the input channel to the input device, because there are NO
        // functions on the input channel, so the normal way for that association
        // to be made doesn't work here.
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
    
    if (activeChannel->full)
        return;

    if (first == NULL)
    {
        // If no data, the channel disconnects (!)
        activeChannel->active = FALSE;
        return;
    }

    return; // for now
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
    if ((ppwc & 1) == 0)
    {
        // Even word.  Note that we have the even words ending on byte boundaries,
        // which means that the first word skips the first 4 bits of the first byte.
        // That is done to keep the payload (starting at word 2) aligned on byte
        // boundaries.  The payload is often byte-oriented, for example in supervisory
        // messages, and doing it this way makes picking things apart easier.
        outbuf[outcnt++] |= d >> 8;
        outbuf[outcnt++] = d;
    }
    else
    {
        // Odd word
        outbuf[outcnt++] = d >> 4;
        outbuf[outcnt] = d << 4;
    }
    ppwc++;
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
    
#ifdef DEBUG
    printf ("doelz: output block len %d (%d ppwords), addr %04o",
            outcnt, ppwc, addr);
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
    
    // Use the length to sort out what's what.  Note that data messages
    // contain a 12 bit address plus some number of 20 bit data words, which
    // means the length cannot match the magic numbers for abort, key, or 
    // supervisory messages.
    if (ppwc == 1)
    {
        // Abort
#ifdef DEBUG
        printf ("doelz: abort from host\n");
#endif
    }
    else if (ppwc == 7)
    {
        // Key
#ifdef DEBUG
        printf ("doelz: key %04o\n", (outbuf[1] << 4) + (outbuf[2] >> 4));
#endif
    }
    else if (ppwc == 11)
    {
        if (addr != 1)
        {
            logError(LogErrorLocation, "Doelz channel %02o - supervisory block wrong address %04o",
                     activeChannel->id, addr);
            return;
        }
#ifdef DEBUG
        printf ("doelz: supervisory message code %d\n", outbuf[5]);
#endif
    }
    else
    {
        // Data block -- note that data block length cannot be 7
    }
    outcnt = ppwc = 0;
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
        NULL,                                       // no security attribute 
        0,                                          // default stack size 
        (LPTHREAD_START_ROUTINE)doelzThread, 
        (LPVOID)0,                                  // thread parameter 
        0,                                          // not suspended 
        &dwThreadId);                               // returns thread ID 

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
            //usleep(10000000);
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
