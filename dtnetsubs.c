/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, 2004, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: dtnetsubs.c
**
**  Description:
**      Common platform-dependent routines relating to networking.
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
#include <time.h>
#include <sys/types.h>
#if defined(_WIN32)
#include <winsock.h>
#else
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#endif

/*
**  -----------------
**  Private Constants
**  -----------------
*/

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define locDtFetData(fet) \
    ((in >= out) ? in - out                     \
     : (fet)->end - out + in - (fet)->first)

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static ThreadFunRet dtThread (void *param);
static void dtCloseSocket (int connFd);

/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/
#if !defined(_WIN32)
static pthread_t dt_thread;
#endif
fd_set nullSet;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Establish an outbound connection
**
**  Parameters:     Name        Description.
**                  connFd      Pointer to socket file number
**                  hostname    Host to connect to (name or address)
**                  port        Port number to connect to
**
**  Returns:        0 if ok, or -1 if error.
**
**------------------------------------------------------------------------*/
int dtConnect (int *connFd, const char *hostname, int port)
    {
    struct hostent *hp;
    struct sockaddr_in server;
    
#if defined(_WIN32)
    WORD versionRequested;
    WSADATA wsaData;
    int err;

    /*
    **  Select WINSOCK 1.1.
    */ 
    versionRequested = MAKEWORD(1, 1);
 
    err = WSAStartup(versionRequested, &wsaData);
    if (err != 0)
        {
#if !defined(_WIN32)
        fprintf(stderr, "\r\nError in WSAStartup: %d\r\n", err);
#endif
        return -1;
        }
#endif
    /*
    **  Create TCP socket and bind to specified port.
    */
    *connFd = socket(AF_INET, SOCK_STREAM, 0);
    if (*connFd < 0)
        {
#if !defined(_WIN32)
        fprintf(stderr, "dtConnect: Can't create socket\n");
#endif
        return -1;
        }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    hp = gethostbyname (hostname);
    if (hp == NULL || hp->h_length == 0)
        {
#if !defined(_WIN32)
        fprintf (stderr, "dtConnect: unrecognized hostname %s\n", 
                 hostname);
#endif
        return -1;
        }
    memcpy (&server.sin_addr, hp->h_addr, sizeof (server.sin_addr));
    server.sin_port = htons(port);

    if (connect (*connFd, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
#if !defined(_WIN32)
        fprintf(stderr, "dtConnect: Can't connect to %s %d\n", 
                hostname, port);
#endif
        return -1;
        }
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Create a thread.
**
**  Parameters:     Name        Description.
**                  fp          Pointer to thread function
**                  param       thread function parameter
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dtCreateThread (ThreadFunRet (*fp)(void *), void *param)
    {
#if defined(_WIN32)
    DWORD dwThreadId; 
    HANDLE hThread;

    /*
    **  Create thread.
    */
    hThread = CreateThread( 
        NULL,                                       /* no security attribute */
        0,                                          /* default stack size */
        (LPTHREAD_START_ROUTINE)fp, 
        (LPVOID)param,                              /* thread parameter */
        0,                                          /* not suspended */
        &dwThreadId);                               /* returns thread ID */

    if (hThread == NULL)
        {
        exit(1);
        }
#else
    int rc;

    /*
    **  Create POSIX thread with default attributes.
    */
    rc = pthread_create(&dt_thread, NULL, fp, param);
    if (rc < 0)
        {
        fprintf(stderr, "Failed to create thread\n");
        exit(1);
        }
#endif
    }

/*--------------------------------------------------------------------------
**  Purpose:        Create thread which will deal with all TCP
**                  connections.
**
**  Parameters:     Name        Description.
**                  ps          Pointer to NetPortSet to use
**                  ringSize    FET buffer size for each port
**
**  Returns:        Nothing.
**
**  This function provides a common way for handling inbound connections.
**  The necessary parameters are all in the NetPortSet structure.
**  The following fields must be set before calling this function:
**      maxPorts    total number of ports (connections) to allow
**      portNum     TCP port number to listen to
**      callBack    pointer to function to be called when a new connection
**                  appears, or NULL if none is needed
**      localOnly   TRUE to listen only to local connections (127.0.0.1),
**                  FALSE to allow connections from anywhere.
**
**  The other fields of the NetPortSet struct are filled in by this function,
**  or by the thread it creates that actually does the listening to
**  new connections.
**  Each time a new connection is seen, curPorts is incremented, activeSet
**  is adjusted to include the new socket fd into the set, and one of
**  the PortVec entries is loaded with the data for the new connection.
**  That data is the socket fd, and the remote IP address.
**
**  If a callback function pointer is specified, that callback function
**  is called with the NetPort entry that was filled in, and the
**  portVec array index where that entry lives.  The callback function
**  is also called when the NetPort is closed; the two cases are
**  distinguished by the fact that the connFd is non-zero if the connection
**  is open, and zero if it is closed.
**------------------------------------------------------------------------*/
void dtCreateListener(NetPortSet *ps, int ringSize)
    {
#if defined(_WIN32)
    static bool firstMux = TRUE;
    WORD versionRequested;
    WSADATA wsaData;
    int err;
#else
    int rc;
#endif
    int i;

    /*
    **  Initialize the NetPortSet structure part way
    */
    ps->curPorts = 0;
    FD_ZERO(&ps->activeSet);
    ps->maxFd = 0;
    ps->portVec = calloc(1, sizeof(NetPort) * ps->maxPorts);
    if (ps->portVec == NULL)
        {
#if !defined(_WIN32)
        fprintf(stderr, "Failed to allocate NetPort[%d] vector\n",
                ps->maxPorts);
#endif
        exit(1);
        }
    for (i = 0; i < ps->maxPorts; i++)
        {
        dtInitFet (&ps->portVec[i].fet, ringSize);
        }
    
#if defined(_WIN32)
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
#if !defined(_WIN32)
        fprintf(stderr, "\r\nError in WSAStartup: %d\r\n", err);
#endif
            exit(1);
            }
        }
#endif
    dtCreateThread (dtThread, ps);
    FD_ZERO(&nullSet);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close an inbound network port
**
**  Parameters:     Name        Description.
**                  np          Pointer to NetPort being closed
**                  ps          Pointer to NetPortSet that np belongs to
**
**  Returns:        Nothing.
**
**  This function is used to close sockets for inbound connections -- those
**  that were created by the dtCreateListener mechanism.
**------------------------------------------------------------------------*/
void dtClose (NetPort *np, NetPortSet *ps)
    {
    int fd, i, j;
    NetPort *t;
    
    fd = np->fet.connFd;
    FD_CLR(fd, &ps->activeSet);
    dtCloseFet (&np->fet);
    if (ps->callBack != NULL)
        {
        (*ps->callBack) (np, np - ps->portVec);
        }
    if (fd == ps->maxFd)
        {
        j = 0;
        for (i = 0; i < ps->maxPorts; i++)
            {
            t = ps->portVec + i;
            if (t->fet.connFd > j)
                {
                j = t->fet.connFd;
                }
            }
        ps->maxFd = j;
        }
    ps->curPorts--;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Find a NetPort with data waiting
**
**  Parameters:     Name        Description.
**                  ps          Pointer to NetPortSet
**                  time        Timeout in ms, or 0 to return immediately
**                              if no data
**
**  Returns:        pointer to NetPort with data, NULL if timeout
**
**  This function waits for received data or for a change of status
**  on any of the ports associated with the supplied NetPortSet.
**  Any status change (disconnect) is handled directly.
**  If data is received, a pointer to one of the NetPorts that has data
**  is returned.
**  If no data is received within the timeout period, NULL is returned.
**
**------------------------------------------------------------------------*/
NetPort * dtFindInput (NetPortSet *ps, int time)
    {
    int i;
    fd_set readFds;
    fd_set exceptFds;
    struct timeval timeout;
    NetPort *np;

    readFds = ps->activeSet;
    exceptFds = ps->activeSet;
    timeout.tv_sec = time / 1000;
    timeout.tv_usec = (time * 1000) % 1000000;
    
    for (;;)
        {
        i = select(ps->maxFd + 1, &readFds, NULL, &exceptFds, &timeout);
        if (i <= 0)
            {
            return NULL;
            }
        if (memcmp (&exceptFds, &nullSet, sizeof (fd_set)) != 0)
            {
            /*
            **  Some exceptions were found, handle them here.
            */
            np = ps->portVec;
            for (i = 0; i < ps->maxPorts; i++)
                {
                if (dtActive (&np->fet) &&
                    FD_ISSET (np->fet.connFd, &exceptFds))
                    {
                    dtClose (np, ps);
                    }
                np++;
                }
            }
        if (memcmp (&readFds, &nullSet, sizeof (fd_set)) != 0)
            {
            /*
            **  Some read data was seen, find which port
            */
            np = ps->portVec;
            for (i = 0; i < ps->maxPorts; i++)
                {
                if (dtActive (&np->fet) &&
                    FD_ISSET (np->fet.connFd, &readFds))
                    {
                    return np;
                    }
                np++;
                }
            }
        return NULL;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Format a timestamp
**
**  Parameters:     Name        Description.
**
**  Returns:        pointer to (static) string
**
**------------------------------------------------------------------------*/
const char *dtNowString (void)
    {
    static char ts[40];
    time_t t;

    time (&t);
    strftime (ts, sizeof (ts) - 1, "%y/%m/%d %H.%M.%S.", localtime (&t));
    return ts;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read more data from the network
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  time        Max time to wait in ms; 0 means do not
**                              wait, -1 means wait indefinitely
**
**  Returns:        < 0 if error, 0 if ok
**
**------------------------------------------------------------------------*/
int dtRead (NetFet *fet, int time)
    {
    int connFd = fet->connFd;
    int i;
    fd_set readFds;
    fd_set exceptFds;
    struct timeval timeout;
    u8 *in, *out, *nextin;
    int size;
    
    if (time != -1)
        {
        FD_ZERO(&readFds);
        FD_ZERO(&exceptFds);
        FD_SET(connFd, &readFds);
        FD_SET(connFd, &exceptFds);

        timeout.tv_sec = time / 1000;
        timeout.tv_usec = (time * 1000) % 1000000;

        select(connFd + 1, &readFds, NULL, &exceptFds, &timeout);
        if (!FD_ISSET(connFd, &readFds) &&
            !FD_ISSET(connFd, &exceptFds))
            {
            return(0);
            }
        }
    /*
    **  Copy the pointers, since they are volatile.
    */
    in = (u8 *) (fet->in);
    out = (u8 *) (fet->out);

    if (in < out)
        {
        /*
        **  If the out pointer is beyond the in pointer, we can
        **  fill the space in between, leaving one free word
        */
        size = out - in - 1;
        }
    else
        {
        /*
        **  Otherwise, we read from the current in pointer to the
        **  end of the buffer -- except if the out pointer is right
        **  at the start of the buffer, in which case we have to 
        **  leave the last word unused.
        */
        size = fet->end - in;
        if (out == fet->first)
            {
            size--;
            }
        }
    i = recv(fet->connFd, in, size, MSG_NOSIGNAL);
    if (i > 0)
        {
        nextin = in + i;
        if (nextin == fet->end)
            {
            nextin = fet->first;
            }
        fet->in = nextin;
        return 0;
        }
    else
        {
        return i;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read one byte from the network buffer
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**
**  Returns:        -1 if no data available, or the next byte as
**                  an unsigned value.
**
**------------------------------------------------------------------------*/
int dtReado (NetFet *fet)
    {
    u8 *in, *out, *nextout;
    u8 b;
    
    /*
    **  Copy the pointers, since they are volatile.
    */
    in = (u8 *) (fet->in);
    out = (u8 *) (fet->out);

    if (out == in)
        {
        return -1;
        }
    nextout = out + 1;
    if (nextout == fet->end)
        {
        nextout = fet->first;
        }
    b = *out;
    fet->out = nextout;
    return b;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read a specified number of bytes from the network buffer
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  buf         Data buffer pointer
**                  len         Number of bytes to read
**
**  Returns:        -1 if <len> bytes not currently available, 0 if ok
**
**------------------------------------------------------------------------*/
int dtReadw (NetFet *fet, void *buf, int len)
    {
    u8 *in, *out;
    u8 *to = (u8 *) buf;
    int left;
    
    /*
    **  Copy the pointers, since they are volatile.
    */
    in = (u8 *) (fet->in);
    out = (u8 *) (fet->out);

    if (locDtFetData (fet) < len)
        {
        return -1;
        }

    /*
    **  We now know we have enough data to satisfy the request.
    **  See how many bytes there are between the current "out"
    **  pointer and the end of the ring.  If that's less than the
    **  amount requested, we have to move two pieces.  If it's exactly
    **  equal, we only have to move one piece, but the "out" pointer
    **  has to wrap back to "first".
    */
    left = fet->end - out;
    
    if (left <= len)
        {
        /*
        **  We'll exhaust the data from here to end of ring, so copy that
        **  first and wrap "out" back to the start of the ring.
        */
        memcpy (to, out, left);
        to += left;
        len -= left;
        fet->out = out = fet->first;
        /*
        **  If the data to end of ring was exactly the amount we wanted,
        **  we're done now.
        */
        if (len == 0)
            {
            return 0;
            }
        }
    /*
    **  At this point we only have one piece left to move, and we know
    **  that it will NOT take us all the way to the end of the ring
    **  buffer, so we don't need a wrap check on "out" here.
    */
    memcpy (to, out, len);
    fet->out = out + len;
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read a TLV formatted data item from the network buffer
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  buf         Data buffer pointer
**                  len         Buffer length
**
**  Returns:        Data length of TLV item received
**                  -1 if no data
**                  -2 if data is too long for the buffer
**
**------------------------------------------------------------------------*/
int dtReadtlv (NetFet *fet, void *buf, int len)
    {
    int datalen;
    u8 *in, *out, *p;
    
    /*
    **  Copy the pointers, since they are volatile.
    */
    in = (u8 *) (fet->in);
    out = (u8 *) (fet->out);

    /*
    **  First check if we have enough data in the ring to find the
    **  length field.
    */
    if (locDtFetData (fet) < 2)
        {
        return -1;
        }
    
    /* Compute the address of the length field */
    p = out + 1;
    if (p == fet->end)
        {
        p = fet->first;
        }
    datalen = *p;
    if (datalen + 2 > len)
        {
        return -2;
        }
    if (dtReadw (fet, buf, datalen + 2) < 0)
        {
        datalen = -1;
        }
    return datalen;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Allocate a buffer and initialize the FET pointers
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  bufsiz      Size of buffer to allocate
**
**  Returns:        Nothing.  If malloc fails, abort.
**
**------------------------------------------------------------------------*/
void dtInitFet (NetFet *fet, int bufsiz)
    {
    u8 *buf;
    
    buf = (u8 *) malloc (bufsiz);
    if (buf == NULL)
        {
#if !defined(_WIN32)
        fprintf (stderr, "dtInitFet: failed to allocate %d byte buffer\n", 
                 bufsiz);
#endif
        exit (1);
        }
    fet->connFd = 0;            /* Mark FET not open yet */
    fet->first = buf;
    fet->in = buf;
    fet->out = buf;
    fet->end = buf + bufsiz;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close the NetFet and reset the FET pointers
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**
**  Returns:        Nothing. 
**
**------------------------------------------------------------------------*/
void dtCloseFet (NetFet *fet)
    {
    dtCloseSocket (fet->connFd);
    fet->connFd = 0;            /* Mark FET not open anymore */
    fet->in = fet->first;
    fet->out = fet->first;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a TLV formatted data item
**
**  Parameters:     Name        Description.
**                  connFd      socket
**                  tag         tag (a.k.a., type) code
**                  len         data length
**                  value       buffer holding the data
**
**  Returns:        Nothing. 
**
**------------------------------------------------------------------------*/
void dtSendTlv (int connFd, int tag, int len, const void *value)
    {
    u8  tl[2];
    
    if (tag > 255 || len > 255)
        {
#if !defined(_WIN32)
        fprintf (stderr, "dtSendTlv: bad tag/len %d %d\n", tag, len);
#endif
        return;
        }
    
    tl[0] = tag;
    tl[1] = len;
    send (connFd, tl, 2, MSG_NOSIGNAL);
    if (len > 0)
        {
        send (connFd, value, len, MSG_NOSIGNAL);
        }
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        TCP thread.
**
**  Parameters:     Name        Description.
**                  param       Pointer to NetPortSet to use
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
#if defined(_WIN32)
static void dtThread(void *param)
#else
static void *dtThread(void *param)
#endif
    {
    NetPortSet *ps = (NetPortSet *) param;
    NetPort *np;
    int listenFd, connFd;
    struct sockaddr_in server;
    struct sockaddr_in from;
    int fromLen;
    int true_opt = 1;
    
    /*
    **  Create TCP socket and bind to specified port.
    */
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
        {
#if !defined(_WIN32)
        fprintf(stderr, "dtThread: Can't create socket\n");
#endif
        ThreadReturn;
        }

    /*
    **  Initialize the NetPortSet structure the rest of the way
    */
    ps->listenFd = listenFd;

    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR,
               (char *)&true_opt, sizeof(true_opt));
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    if (ps->localOnly)
        {
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        }
    else
        {
        server.sin_addr.s_addr = inet_addr("0.0.0.0");
        }
    server.sin_port = htons(ps->portNum);

    if (bind(listenFd, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
#if !defined(_WIN32)
        fprintf(stderr, "dtThread: Can't bind to socket\n");
#endif
        ThreadReturn;
        }

    if (listen(listenFd, 5) < 0)
        {
#if !defined(_WIN32)
        fprintf(stderr, "dtThread: Can't listen\n");
#endif
        ThreadReturn;
        }

    while (1)
        {
        /*
        **  If DtCyber is closing down, close the listener socket
        **  and exit the thread.
        **
        **  Note that closing the data sockets is the responsibility
        **  of the code that uses those sockets.
        */
        if (!emulationActive)
            {
            dtCloseSocket (listenFd);
            ThreadReturn;
            }
        
        /*
        **  Do we have any free ports?  If not, sleep a bit
        **  and look again.
        */
        if (ps->curPorts == ps->maxPorts)
            {
#if defined(_WIN32)
            Sleep(1000);
#else
            /* usleep(10000000); */
            sleep(1);
#endif
            continue;
            }
            
        /*
        **  Wait for a connection.
        */
        fromLen = sizeof(from);
        connFd = accept(ps->listenFd, (struct sockaddr *)&from, &fromLen);

        /*
        **  Set Keepalive, non-blocking socket, and no signals.
        **
        **  Non-blocking ensures that we won't block if some network
        **  link is having trouble.  Ideally that's detected and handled
        **  more elegantly, but at least no one else will suffer even
        **  without any special handling.
        */
        setsockopt(connFd, SOL_SOCKET, SO_KEEPALIVE,
                   (char *)&true_opt, sizeof(true_opt));
#if defined(_WIN32)
        ioctlsocket (connFd, FIONBIO, &true_opt);
#else
        fcntl (connFd, F_SETFL, O_NONBLOCK);
#endif
#ifdef __APPLE__
        setsockopt(connFd, SOL_SOCKET, SO_NOSIGPIPE,
                   (char *)&true_opt, sizeof(true_opt));
#endif

        /*
        **  Find a free slot in the NetPort vector.
        **  Note that there definitely is one, because we checked
        **  current vs. max count earlier.
        */
        np = ps->portVec;
        while (dtActive (&np->fet))
            {
            np++;
            }
        
        /*
        **  Update max fd number.  
        */
        if (ps->maxFd < connFd)
            {
            ps->maxFd = connFd;
            }
        
        /*
        **  Mark connection as active.
        */
        ps->curPorts++;
        np->fet.connFd = connFd;
        np->from = from.sin_addr;
        FD_SET(connFd, &ps->activeSet);
        if (ps->callBack != NULL)
            {
            (*ps->callBack) (np, np - ps->portVec);
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close a network socket
**
**  Parameters:     Name        Description.
**                  connFd      socket fd to close
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dtCloseSocket (int connFd)
    {
#if defined(_WIN32)
    closesocket(connFd);
#else
    close(connFd);
#endif
    }


/*---------------------------  End Of File  ------------------------------*/
