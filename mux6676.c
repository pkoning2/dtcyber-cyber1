/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: mux6676.c
**
**  Description:
**      Perform simulation of CDC 6676 data set controller (terminal mux).
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

/*
**  Function codes.
*/
#define Fc6676Output            00001
#define Fc6676Status            00002
#define Fc6676Input             00003

#define Fc6676UnitMask          07000
#define Fc6676UnitShift         9

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
    u8          id;
    bool        active;
    int         connFd;
    } PortParam;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus mux6676Func(PpWord funcCode);
static void mux6676Io(void);
static void mux6676Activate(void);
static void mux6676Disconnect(void);
static void mux6676CreateThread(DevSlot *dp);
static int mux6676CheckInput(PortParam *mp);
#if defined(_WIN32)
static void mux6676Thread(void *param);
#define RETURN return
#else
static void *mux6676Thread(void *param);
#define RETURN return 0
#endif

/*
**  ----------------
**  Public Variables
**  ----------------
*/
u16 telnetPort;
u16 telnetConns;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
#if !defined(_WIN32)
static pthread_t mux6676_thread;
#endif

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise 844 disk drive.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      unit number
**                  channelNo   channel number the device is attached to
**                  deviceName  optional device file name
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void mux6676Init(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;
    PortParam *mp;
    u8 i;

    (void)eqNo;
    (void)deviceName;

    dp = channelAttach(channelNo, DtMux6676);
    dp->activate = mux6676Activate;
    dp->disconnect = mux6676Disconnect;
    dp->func = mux6676Func;
    dp->io = mux6676Io;
    dp->selectedUnit = unitNo;

    mp = calloc(1, sizeof(PortParam) * telnetConns);
    if (mp == NULL)
        {
        fprintf(stderr, "Failed to allocate mux6676 context block\n");
        exit(1);
        }

    dp->context[unitNo] = mp;

    /*
    **  Initialise port control blocks.
    */
    for (i = 0; i < telnetConns; i++)
        {
        mp->active = FALSE;
        mp->connFd = 0;
        mp->id = i;
        mp += 1;
        }

    /*
    **  Create the thread which will deal with TCP connections.
    */
    mux6676CreateThread(dp);

    /*
    **  Print a friendly message.
    */
    printf("MUX6676 initialised on channel %o unit %o\n", channelNo, unitNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 6676 mux.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus mux6676Func(PpWord funcCode)
    {
    u8 unitNo;
    PortParam *mp;

    unitNo = (funcCode & Fc6676UnitMask) >> Fc6676UnitShift;
    mp = (PortParam *)activeDevice->context[unitNo];

    if (mp == NULL)
        {
        /*
        **  Unit not initialised.
        */
        return(FcDeclined);
        }

    activeDevice->selectedUnit = unitNo;
    funcCode &= ~Fc6676UnitMask;

    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case Fc6676Output:
    case Fc6676Status:
    case Fc6676Input:
        activeDevice->recordLength = 0;
        break;
        }

    activeDevice->fcode = funcCode;
    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 6676 mux.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mux6676Io(void)
    {
    u8 unitNo;
    PortParam *mp;
    PpWord function;
    u8 portNumber;
    char x;
    int in;

    unitNo = activeDevice->selectedUnit;
    mp = (PortParam *)activeDevice->context[unitNo];

    switch (activeDevice->fcode)
        {
    default:
        break;

    case Fc6676Output:
        if (activeChannel->full)
            {
            /*
            **  Output data.
            */
            activeChannel->full = FALSE;
            portNumber = (u8)activeDevice->recordLength++;
            if (portNumber < telnetConns)
                {
                mp += portNumber;
                if (mp->active)
                    {
                    /*
                    **  Port with active TCP connection.
                    */
                    function = activeChannel->data >> 9;
                    switch (function)
                        {
                    case 0:
                        break;

                    case 4:
                    case 6:
                        /*
                        **  Get data and strip off parity.
                        */
                        x = (activeChannel->data >> 1) & 0x7f;
                        send(mp->connFd, &x, 1, 0);
                        break;

                    default:
                        break;
                        }
                    }
                }
            }
        break;
        
    case Fc6676Input:
        if (!activeChannel->full)
            {
            activeChannel->data = 0;
            activeChannel->full = TRUE;
            portNumber = (u8)activeDevice->recordLength++;
            if (portNumber < telnetConns)
                {
                mp += portNumber;
                if (mp->active)
                    {
                    /*
                    **  Port with active TCP connection.
                    */
                    activeChannel->data |= 01000;
                    if ((in = mux6676CheckInput(mp)) > 0)
                        {
                        activeChannel->data |= ((in & 0x7F) << 1) | 04000;
                        }
                    }
                }
            }
        break;

    case Fc6676Status:
        activeChannel->data = 0;
        activeChannel->full = TRUE;
        break;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mux6676Activate(void)
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
static void mux6676Disconnect(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Create thread which will deal with all TCP
**                  connections.
**
**  Parameters:     Name        Description.
**                  dp          pointer to device descriptor
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mux6676CreateThread(DevSlot *dp)
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
        (LPTHREAD_START_ROUTINE)mux6676Thread, 
        (LPVOID)dp,                                 // thread parameter 
        0,                                          // not suspended 
        &dwThreadId);                               // returns thread ID 

    if (hThread == NULL)
        {
        fprintf(stderr, "Failed to create mux6676 thread\n");
        exit(1);
        }
#else
    int rc;

    /*
    **  Create POSIX thread with default attributes.
    */
    rc = pthread_create(&mux6676_thread, NULL, mux6676Thread, dp);
    if (rc < 0)
        {
        fprintf(stderr, "Failed to create mux6676 thread\n");
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
static void mux6676Thread(void *param)
#else
static void *mux6676Thread(void *param)
#endif
    {
    DevSlot *dp = (DevSlot *)param;
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
        printf("mux6676: Can't create socket\n");
        RETURN;
        }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_port = htons(telnetPort);

    if (bind(listenFd, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
        printf("mux6676: Can't bind to socket\n");
        RETURN;
        }

    if (listen(listenFd, 5) < 0)
        {
        printf("mux6676: Can't listen\n");
        RETURN;
        }

    while (1)
        {
        /*
        **  Find a free port control block.
        */
        mp = (PortParam *)dp->context[dp->selectedUnit];
        for (i = 0; i < telnetConns; i++)
            {
            if (!mp->active)
                {
                break;
                }
            mp += 1;
            }

        if (i == telnetConns)
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
        printf("mux6676: Received connection on port %d\n", mp->id);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Check for input.
**
**  Parameters:     Name        Description.
**                  mp          pointer to mux parameters.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static int mux6676CheckInput(PortParam *mp)
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
            printf("mux6676: Connection dropped on port %d\n", mp->id);
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
        printf("mux6676: Connection dropped on port %d\n", mp->id);
        return(-1);
        }
    else
        {
        return(0);
        }
    }

/*---------------------------  End Of File  ------------------------------*/
