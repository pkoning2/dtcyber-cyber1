// BUGS
//input: after connection drop, I see input, with crud (3777 or 3377) as data.
//    output: assembly looks ok but no bytes come out on telnet.

/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: niu.c
**
**  Description:
**      Perform simulation of Plato NIU (terminal controller)
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
#define NiuFirstStation         1           // site 0 station 1

/*
**  Function codes.
*/
#define FcNiuOutput             00000
#define FcNiuInput              00040

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
typedef struct portParam
{
    int         connFd;
    u16         currInput;
    u8          ibytes;      // how many bytes have been assembled into currInput (0..2)
    bool        active;
} PortParam;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus niuInFunc(PpWord funcCode);
static void niuInIo(void);
static FcStatus niuOutFunc(PpWord funcCode);
static void niuOutIo(void);
static void niuActivate(void);
static void niuDisconnect(void);
static void niuCreateThread(void);
static int niuCheckInput(PortParam *portVector);
#if defined(_WIN32)
static void niuThread(void *param);
#else
static void *niuThread(void *param);
#endif

/*
**  ----------------
**  Public Variables
**  ----------------
*/
u16 platoPort;
u16 platoConns;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static DevSlot *in;
static DevSlot *out;
static PortParam *portVector;
static int currInPort;
static int lastInPort;
static int obytes;
static u32 currOutput;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise NIU
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
void niuInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
{
    u8 i;
    PortParam *mp;

    (void)eqNo;
    (void)deviceName;

    if (in != NULL)
    {
        fprintf (stderr, "Multiple NIUs not supported\n");
        exit (1);
    }
    
    // We use two channels, one for input, one for output.
    // The input channel number is given by what is normally
    // the unit number in the cyber.ini file.
    in = channelAttach(unitNo, DtNiu);
    in->activate = niuActivate;
    in->disconnect = niuDisconnect;
    in->func = niuInFunc;
    in->io = niuInIo;
    out = channelAttach(channelNo, DtNiu);
    out->activate = niuActivate;
    out->disconnect = niuDisconnect;
    out->func = niuOutFunc;
    out->io = niuOutIo;

    mp = portVector = calloc(1, sizeof(PortParam) * platoConns);
    if (portVector == NULL)
    {
        fprintf(stderr, "Failed to allocate NIU context block\n");
        exit(1);
    }

    /*
    **  Initialise port control blocks.
    */
    for (i = 0; i < platoConns; i++)
    {
        mp->active = FALSE;
        mp->connFd = 0;
        mp->ibytes = 0;
        mp++;
    }

    currInPort = -1;
    lastInPort = 0;
    
    /*
    **  Create the thread which will deal with TCP connections.
    */
    niuCreateThread();

    /*
    **  Print a friendly message.
    */
    printf("NIU initialised with input channel %o and output channel %o\n", unitNo, channelNo);
}

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on NIU input channel.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus niuInFunc(PpWord funcCode)
{
    switch (funcCode)
    {
    default:
        return(FcDeclined);

    case FcNiuInput:
        currInPort = -1;
        break;
    }

    activeDevice->fcode = funcCode;
    return(FcAccepted);
}

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on NIU output channel.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus niuOutFunc(PpWord funcCode)
{
    switch (funcCode)
    {
    default:
        return(FcDeclined);

    case FcNiuOutput:
        obytes = 0;
        break;
    }

    activeDevice->fcode = funcCode;
    return(FcAccepted);
}

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on NIU input channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void niuInIo(void)
{
    int port;
    PortParam *mp;
    u8 in;
    
    if (activeDevice->fcode != FcNiuInput ||
        activeChannel->full)
        return;
    
    if (currInPort < 0)
    {
        // We're at the first of the two-word input sequence; find a
        // port with data.
        port = lastInPort;
        for (;;)
        {
            if (++port == platoConns)
                port = 0;
            mp = portVector + port;
            if (mp->active)
            {
                /*
                **  Port with active TCP connection.
                */
                if ((in = niuCheckInput(mp)) > 0)
                {
                    // Connection has data -- assemble it and see if we have
                    // a complete input word
                    if (mp->ibytes != 0)
                    {
                        if ((in & 0200) == 0)
                        {
                            // Sequence error, drop the byte
                            printf ("niu input sequence error, first byte %03o, port %d\n",
                                    in, port);
                            continue;
                        }
                        mp->currInput |= (in & 0177);
                        currInPort =  lastInPort = port;
                        activeChannel->data = 04000 + currInPort + NiuFirstStation;
                        activeChannel->full = TRUE;
                        return;
                    }
                    else
                    {
                        // first byte of key data, save it for later
                        if ((in & 370) != 0)
                        {
                            // sequence error, drop the byte
                            printf ("niu input sequence error, second byte %03o, port %d\n",
                                    in, port);
                            continue;
                        }
                        mp->currInput = in << 7;
                        mp->ibytes = 1;
                    }
                }
            }
            if (port == lastInPort)
                return;         // No input, leave channel empty
        }
    }
    // We have a current port, so we already sent the port number;
    // now send its data.
    mp = portVector + currInPort;
    activeChannel->data = mp->currInput;
    activeChannel->full = TRUE;
    mp->ibytes = 0;
}

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on NIU output channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void niuOutIo(void)
{
    PpWord d;
    PortParam *mp;
    u8 data[3];
    int port;
    
    if (activeDevice->fcode != FcNiuOutput ||
        !activeChannel->full)
        return;
    
    /*
    **  Output data.
    */
    activeChannel->full = FALSE;
    d = activeChannel->data;
    if (obytes == 0)
    {
        // first word of the triple
        if ((d & 06000) != 04000)
        {
            printf ("niu output out of sync, first word %04o\n", d);
            return;
        }
        currOutput = (d & 1777) << 9;
        obytes = 1;
        return;
    }
    if (obytes == 1)
    {
        // second word of the triple
        if ((d & 06001) != 0)
        {
            printf ("niu output out of sync, second word %04o\n", d);
            return;
        }
        currOutput |= d >> 1;
        obytes = 2;
        return;
    }
    // Third word of the triple
    if ((d & 04000) != 0)
    {
        printf ("niu output out of sync, third word %04o\n", d);
        return;
    }
    port = (d & 01777) - NiuFirstStation;
    
    // Now that we have a complete output triple, discard it
    // if it's for a station number out of range (larger than
    // what we're configured to support) or without an 
    // active TCP connections
    obytes = 0;
    if (port >= platoConns || port < 0)
        return;
    mp = portVector + port;
    if (!mp->active)
        return;
    data[0] = currOutput >> 12;
    data[1] = ((currOutput >> 6) & 077) | 0200;
    data[2] = (currOutput & 077) | 0300;
    send(mp->connFd, data, 3, 0);
}

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void niuActivate(void)
{
}

/*--------------------------------------------------------------------------
**  Purpose:        Handle disconnecting of channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void niuDisconnect(void)
{
}

/*--------------------------------------------------------------------------
**  Purpose:        Create WIN32 thread which will deal with all TCP
**                  connections.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void niuCreateThread(void)
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
        (LPTHREAD_START_ROUTINE)niuThread, 
        (LPVOID)0,                                  // thread parameter 
        0,                                          // not suspended 
        &dwThreadId);                               // returns thread ID 

    if (hThread == NULL)
    {
        fprintf(stderr, "Failed to create niu thread\n");
        exit(1);
    }
#else
    int rc;
    pthread_t thread;
    pthread_attr_t attr;

    /*
    **  Create POSIX thread with default attributes.
    */
    pthread_attr_init(&attr);
    rc = pthread_create(&thread, &attr, niuThread, 0);
    if (rc < 0)
    {
        fprintf(stderr, "Failed to create niu thread\n");
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
static void niuThread(void *param)
#else
    static void *niuThread(void *param)
#endif
{
    int listenFd;
    struct sockaddr_in server;
    struct sockaddr_in from;
    int fromLen;
    PortParam *mp;
    u8 i;

    /*
    **  Create TCP socket and bind to specified port.
    */
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
    {
        printf("niu: Can't create socket\n");
        return;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_port = htons(platoPort);

    if (bind(listenFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("niu: Can't bind to socket\n");
        return;
    }

    if (listen(listenFd, 5) < 0)
    {
        printf("niu: Can't listen\n");
        return;
    }

    while (1)
    {
        /*
        **  Find a free port control block.
        */
        mp = portVector;
        for (i = 0; i < platoConns; i++)
        {
            if (!mp->active)
            {
                break;
            }
            mp += 1;
        }

        if (i == platoConns)
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
        printf("niu: Received connection on port %d\n", mp - portVector);
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Check for input.
**
**  Parameters:     Name        Description.
**                  mp          pointer to PortParam struct for the connection.
**
**  Returns:        data byte received, or -1 if there is no data.
**
**------------------------------------------------------------------------*/
static int niuCheckInput(PortParam *mp)
{
    int i;
    fd_set readFds;
    fd_set exceptFds;
    struct timeval timeout;
    char data;

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
            printf("niu: Connection dropped on port %d\n", mp - portVector);
            return(-1);
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
        printf("niu: Connection dropped on port %d\n", mp - portVector);
        return(-1);
    }
    else
    {
        return(0);
    }
}

/*---------------------------  End Of File  ------------------------------*/
