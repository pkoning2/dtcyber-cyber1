/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2005, Tom Hunter, Paul Koning (see license.txt)
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
#include <string.h>
#include <errno.h>
#include "const.h"
#include "types.h"
#include "proto.h"
#include <time.h>
#include <sys/types.h>
#if defined(_WIN32)
	#include <winsock.h>
	//the following are a supreme hack.  Joe
	#define close(x)	closesocket(x)
	#define errno		WSAGetLastError()
	#define EAGAIN		WSAEWOULDBLOCK
	#define EINPROGRESS WSAEWOULDBLOCK
#else
	#include <sys/time.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
#endif

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

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

#define locDtFetData(fet)                                       \
    ((in >= out) ? in - out                                     \
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
static ThreadFunRet dtDataThread (void *param);
static void dtCloseSocket (int connFd, bool hard);
static int dtGetw (NetFet *fet, void *buf, int len, bool read);
static void dtSendPending (NetFet *fet, NetPortSet *ps);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
NetFet connlist = { &connlist, &connlist };  /* trailing fields default to 0 */
void (*updateConnections) (void) = NULL;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
#if !defined(_WIN32)
static pthread_t dt_thread;
#endif
static fd_set nullSet;

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
**                  fet         Pointer to NetFet
**                  ps          Pointer to NetPortSet to use (NULL not to)
**                  host        Address of host to connect to
**                  port        Port number to connect to
**
**  Returns:        0 if ok, or -1 if error.
**
**------------------------------------------------------------------------*/
int dtConnect (NetFet *fet, NetPortSet *ps, in_addr_t host, int port)
    {
    struct sockaddr_in server;
    int connFd, retval;
    
    /*
    **  Create TCP socket
    */
    connFd = socket(AF_INET, SOCK_STREAM, 0);
    if (connFd < 0)
        {
#if !defined(_WIN32)
        perror ("dtConnect: Can't create socket");
#endif
        return -1;
        }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = host;
    server.sin_port = htons(port);

    retval = connect (connFd, (struct sockaddr *)&server, sizeof(server));

//Joe comments: wonder if this code needs to be in a loop with a sleep since the documents i read re: blocking/non-blocking in windoze suggest this may be necessary,
//              or if a callback function needs to be implemented to handle it.  we'll see.

    if (retval < 0 && errno != EINPROGRESS)
        {
#if !defined(_WIN32)
        fprintf(stderr, "dtConnect: Can't connect to %08x %d, errno %d\n", 
                host, port, errno);
        perror ("Can't connect");
#endif
        close (connFd);
        return -1;
        }
    fet->from = server.sin_addr;
    fet->fromPort = server.sin_port;
    dtActivateFet (fet, ps, connFd);
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
        perror ("Failed to create thread");
        exit(1);
        }
#endif
    }

/*--------------------------------------------------------------------------
**  Purpose:        Initialize a portset, which is the structure that
**                  deals with a set of related connections.
**
**  Parameters:     Name        Description.
**                  ps          Pointer to NetPortSet to use
**                  ringSize    FET buffer size for each port
**
**  Returns:        Nothing.
**
**  This function provides a common way for handling connections.
**  It tracks sets of active file descriptors to wait on, and sets of
**  blocked descriptors whose sends have to be retried.
**  It also creates a thread to listen for incoming connections, if
**  this is a portset for which we want to do that.
**  The necessary parameters are all in the NetPortSet structure.
**
**  The following fields must be set before calling this function:
**      maxPorts    total number of ports (connections) to allow
**      portNum     TCP port number to listen to (0 to not listen)
**      callBack    pointer to function to be called when a new connection
**                  appears, or NULL if none is needed
**      localOnly   TRUE to listen only to local connections (127.0.0.1),
**                  FALSE to allow connections from anywhere.
**
**  The other fields of the NetPortSet struct are filled in by this function,
**  or by the thread it creates that actually does the listening to
**  new connections.
**
**  Each time a new connection is seen, curPorts is incremented, activeSet
**  is adjusted to include the new socket fd into the set, and one of
**  the PortVec entries is loaded with the data for the new connection.
**  That data is the socket fd, and the remote IP address.
**
**  If a callback function pointer is specified, that callback function
**  is called with the NetFet entry that was filled in, and the
**  portVec array index where that entry lives.  The callback function
**  is also called when the NetFet is closed; the two cases are
**  distinguished by the fact that the connFd is non-zero if the connection
**  is open, and zero if it is closed.
**
**------------------------------------------------------------------------*/
void dtInitPortset (NetPortSet *ps, int ringSize)
    {
#if defined(_WIN32)
    static bool firstMux = TRUE;
    WORD versionRequested;
    WSADATA wsaData;
    int err;
#endif
    int i;

    /*
    **  Initialize the NetPortSet structure part way
    */
    ps->curPorts = 0;
    FD_ZERO(&ps->activeSet);
    ps->sendCount = 0;
    FD_ZERO(&ps->sendSet);
    ps->maxFd = 0;
    ps->portVec = calloc(1, sizeof(NetFet) * ps->maxPorts);
    if (ps->portVec == NULL)
        {
#if !defined(_WIN32)
        fprintf(stderr, "Failed to allocate NetFet[%d] vector\n",
                ps->maxPorts);
#endif
        exit(1);
        }
    for (i = 0; i < ps->maxPorts; i++)
        {
        if (dtInitFet (&ps->portVec[i], ringSize) != 0)
            {
#if !defined(_WIN32)
            fprintf (stderr, "dtInitPortSet: failed to allocate %d byte buffer for FET\n", 
                     ringSize);
#endif
            exit (1);
            }
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
    if (ps->portNum != 0)
        {
        dtCreateThread (dtThread, ps);
        }
    FD_ZERO(&nullSet);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close a portset.
**
**  Parameters:     Name        Description.
**                  ps          Pointer to NetPortSet to use
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void dtClosePortset (NetPortSet *ps)
    {
    int i;
    NetFet *fet;

    for (i = 0; i < ps->maxPorts; i++)
        {
        fet = &ps->portVec[i];
        if (fet->first)
            {
            free (fet->first);
            fet->first = 0;
            }
        }
    if (ps->portNum != 0)
        {
        ps->close = TRUE;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close a network port
**
**  Parameters:     Name        Description.
**                  np          Pointer to NetFet being closed
**                  ps          Pointer to NetPortSet that np belongs to
**                  hard        TRUE for reset, FALSE for shutdown
**
**  Returns:        0 if ok
**                  -1 if send data is still pending and hard is FALSE.
**
**  This function is used to close sockets for inbound connections -- those
**  that were created by the dtCreateListener mechanism.
**------------------------------------------------------------------------*/
int dtClose (NetFet *np, NetPortSet *ps, bool hard)
    {
    int fd, i, j;
    NetFet *t;

    /*
    **  Return success if already closed.
    */
    if (!np->inUse)
        {
        return 0;
        }
    
    if (!hard && np->sendCount != 0)
        {
        /*
        ** First send any pending data (if possible).
        ** Return with failure if we can't send yet.
        */
        dtSendPending (np, ps);
        if (np->sendCount != 0)
            {
            return -1;
            }
        }
        
    fd = np->connFd;
    FD_CLR(fd, &ps->activeSet);
    if (np->sendCount != 0)
        {
        ps->sendCount--;
        FD_CLR (fd, &ps->sendSet);
        }
    dtCloseFet (np, hard);
    ps->curPorts--;
    if (ps->callBack != NULL)
        {
        (*ps->callBack) (np, np - ps->portVec, ps->callArg);
        }
    if (fd == ps->maxFd)
        {
        j = 0;
        for (i = 0; i < ps->maxPorts; i++)
            {
            t = ps->portVec + i;
            if (t->connFd > j)
                {
                j = t->connFd;
                }
            }
        ps->maxFd = j;
        }

    return 0;
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
    static char ts[64], us[12];
    struct timeval tv;

    gettimeofday (&tv, NULL);
    strftime (ts, sizeof (ts) - 1, "%y/%m/%d %H.%M.%S.", localtime (&tv.tv_sec));
    sprintf (us, "%06d.", tv.tv_usec);
    strcat (ts, us);
    return ts;
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
    return dtGetw (fet, buf, len, TRUE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Peek at the specified number of bytes in the network buffer
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  buf         Data buffer pointer
**                  len         Number of bytes to read
**
**  Returns:        -1 if <len> bytes not currently available, 0 if ok
**
**------------------------------------------------------------------------*/
int dtPeekw (NetFet *fet, void *buf, int len)
    {
    return dtGetw (fet, buf, len, FALSE);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read at most the specified number of bytes.
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  buf         Data buffer pointer
**                  len         Maximum number of bytes to read
**
**  Returns:        Number of bytes actually read
**
**------------------------------------------------------------------------*/
int dtReadmax (NetFet *fet, void *buf, int len)
{
    int actual;
    
    actual = dtFetData (fet);
    if (len < actual)
        {
        /*
        ** Don't read more than was requested.
        */
        actual = len;
        }
    
    if (actual > 0)
    {
        dtReadw (fet, buf, actual);
    }
    return actual;
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
**                  bufsiz      Size of buffer to allocate (0 not to)
**
**  Returns:        0 if ok, -1 if malloc failed
**
**------------------------------------------------------------------------*/
int dtInitFet (NetFet *fet, int bufsiz)
    {
    u8 *buf = NULL;

    if (bufsiz > 0)
        {
        buf = (u8 *) malloc (bufsiz);
        if (buf == NULL)
            {
            return -1;
            }
        }

    fet->prev = fet->next = NULL;
    fet->connFd = 0;                /* no socket yet */
    fet->first = buf;
    fet->in = buf;
    fet->out = buf;
    fet->end = buf + bufsiz;
    fet->sendData = NULL;
    fet->sendCount = fet->sendBufCount = 0;
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close the NetFet and reset the FET pointers
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  hard        TRUE for reset, FALSE for shutdown
**
**  Returns:        Nothing. 
**
**------------------------------------------------------------------------*/
void dtCloseFet (NetFet *fet, bool hard)
    {
    dtCloseSocket (fet->connFd, hard);
    /* Free any pending output data */
    if (fet->sendData != NULL)
        {
        free (fet->sendData);
        fet->sendData = NULL;
        fet->sendCount = fet->sendBufCount = 0;
        }
    fet->connFd = 0;            /* Mark FET not open anymore */
    fet->in = fet->first;
    fet->out = fet->first;
    fet->ownerInfo = 0;
    fet->inUse = 0;
    
    /* 
    ** Remove from the active list.
    ** If there is no link, this is a listen FET as opposed to a data
    ** connection FET, so skip the unlink.
    */
    if (fet->prev != NULL)
        {
        fet->prev->next = fet->next;
        fet->next->prev = fet->prev;
        fet->prev = fet->next = NULL;
        if (updateConnections != NULL)
            {
            (*updateConnections) ();
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a TLV formatted data item
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  ps          Pointer to NetPortSet to use
**                  tag         tag (a.k.a., type) code
**                  len         data length
**                  value       buffer holding the data
**
**  Returns:        -1 if ok, 0 if ok but full, >0 if nothing was sent. 
**
**------------------------------------------------------------------------*/
int dtSendTlv (NetFet *fet, NetPortSet *ps, 
               int tag, int len, const void *value)
    {
    u8  tl[2];
    int retval;
    
    if (tag > 255 || len > 255)
        {
#if !defined(_WIN32)
        fprintf (stderr, "dtSendTlv: bad tag/len %d %d\n", tag, len);
#endif
        return EINVAL;
        }
    
    tl[0] = tag;
    tl[1] = len;
    retval = dtSend (fet, ps, tl, 2);
    if (retval < 0)
        {
        retval = dtSend (fet, ps, value, -len);
        }
    return retval;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a buffer
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  ps          Pointer to NetPortSet to use
**                  len         data length
**                  buf         buffer holding the data
**
**  Returns:        -1 if ok, 0 if ok but full, >0 if nothing was sent. 
**                  Note that this is an all or nothing send; unlike socket
**                  send calls there isn't a partial send.
**
**------------------------------------------------------------------------*/
int dtSend (NetFet *fet, NetPortSet *ps, const void *buf, int len)
    {
    int connFd = fet->connFd;
    int sent = 0, pend, newcount;
    u8 *waitptr;
    
    /*
    ** Ignore calls when connection is not open.
    */
    if (connFd == 0)
        {
        return ENOTCONN;
        }
    
    /*
    ** First send any pending data (if possible).
    */
    dtSendPending (fet, ps);
    
    /*
    ** Only try to send new data if nothing is currently still pending.
    ** Otherwise return EAGAIN (indicating no data was sent).
    **
    ** For internal use: if len < 0, send unconditionally.  This is
    ** used in dtSendTlv, it is not for general use.
    */
    if (fet->sendCount == 0 || len < 0)
        {
        if (len < 0)
            {
            len = -len;
            }
        if (len > 0 && connFd != 0)
            {
            sent = send (connFd, buf, len, MSG_NOSIGNAL);
            if (sent < 0)
                {
                return errno;
                }
            }
        }
    else
        {
        return EAGAIN;
        }

    /*
    ** If some data was unsent, or if no send was done because we already
    ** had data pending, append the unsent data to the pending data.
    */
    pend = len - sent;
    if (pend > 0)
        {
        newcount = fet->sendCount + pend;
        if (newcount > fet->sendBufCount)
            {
            waitptr = realloc (fet->sendData, newcount);
            if (waitptr != NULL)
                {
                fet->sendBufCount = newcount;
                fet->sendData = waitptr;
                }
            }
        else
            {
            waitptr = fet->sendData;
            }
        if (waitptr != NULL)
            {
            if (fet->sendCount == 0)
                {
                /*
                ** Transition from nothing pending to something pending.
                ** Add this connection to the fd_set of connections that
                ** have pending data waiting for space.
                */
                FD_SET (connFd, &ps->sendSet);
                ps->sendCount++;
                }
            memcpy (waitptr + fet->sendCount, (u8 *)buf + sent, pend);
            fet->sendCount = newcount;
            }
        /*
        **  Indicate that this send worked but the next one will not.
        */
        return 0;
        }
    return -1;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Bind to address/port and listen for connections
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  host        IP address to listen to (0.0.0.0 or 127.0.0.1)
**                  port        Port number to connect to
**                  backlog     Listen backlog
**
**  Returns:        0 for ok, -1 for error
**
**------------------------------------------------------------------------*/
int dtBind  (NetFet *fet, in_addr_t host, int port, int backlog)
    {
    struct sockaddr_in server;
    int true_opt = 1;

    /*
    **  Create TCP socket
    */
    fet->connFd = socket(AF_INET, SOCK_STREAM, 0);
    if (fet->connFd < 0)
        {
#if !defined(_WIN32)
        perror ("dtBind: Can't create socket");
#endif
        return -1;
        }

    setsockopt(fet->connFd, SOL_SOCKET, SO_REUSEADDR,
               (char *)&true_opt, sizeof (true_opt));
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = host;
    server.sin_port = htons(port);

    if (bind (fet->connFd, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
        close (fet->connFd);
        return -1;
        }
    if (listen (fet->connFd, backlog) < 0)
        {
        close (fet->connFd);
        return -1;
        }    
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Accept a connection
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer, for a bound/listening fet
**                  acceptFet   NetFet for data connection
**
**  Returns:        File descriptor for data connection.
**                  0 if nothing was waiting.
**
**------------------------------------------------------------------------*/
int dtAccept (NetFet *fet, NetFet *acceptFet)
    {
    int connFd;
    struct sockaddr_in from;
    socklen_t fromLen;
    int true_opt = 1;
    
    /*
    **  Accept a connection.
    */
    fromLen = sizeof (from);
    connFd = accept (fet->connFd, (struct sockaddr *) &from, &fromLen);
    if (connFd < 0)
        {
        return 0;
        }

    /*
    **  Set Keepalive and no signals.
    */
    setsockopt(connFd, SOL_SOCKET, SO_KEEPALIVE,
               (char *)&true_opt, sizeof(true_opt));
#ifdef __APPLE__
    setsockopt(connFd, SOL_SOCKET, SO_NOSIGPIPE,
               (char *)&true_opt, sizeof(true_opt));
#endif

    /*
    **  Save relevant data in supplied FET
    */
    acceptFet->connFd = connFd;
    acceptFet->from = from.sin_addr;
    acceptFet->fromPort = from.sin_port;
    acceptFet->inUse = 1;
    
    return connFd;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Activate the NetFet
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  ps          Pointer to NetPortSet to use
**                  connFd      File descriptor for connection socket
**
**  Returns:        Nothing. 
**
**------------------------------------------------------------------------*/
void dtActivateFet (NetFet *fet, NetPortSet *ps, int connFd)
    {
    /*
    **  Set the FD number in the FET.
    */
    fet->connFd = connFd;
    fet->inUse = 1;

    /*
    **  Track this FET in the NetPortSet, if supplied
    */
    fet->ps = ps;
    FD_SET (fet->connFd, &ps->activeSet);
    ps->curPorts++;
    if (ps->maxFd < connFd)
        {
        ps->maxFd = connFd;
        }
        
    /*
    **  Create the data receive thread
    */
    dtCreateThread (dtDataThread, fet);

    if (ps->callBack != NULL)
        {
        (*ps->callBack) (fet, fet - ps->portVec, ps->callArg);
        }
    
    /* 
    ** Link this FET into the active list. 
    */
    fet->next = connlist.next;
    fet->prev = &connlist;
    fet->next->prev = fet;
    connlist.next = fet;
    if (updateConnections != NULL)
        {
        (*updateConnections) ();
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
**  Purpose:        Read more data from the network, if there is any
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  ps          Pointer to NetPortSet to use
**
**  Returns:        >=0 if ok, value is count of bytes received
**                  -1 if disconnected
**                  -2 if some other error
**
**  This function waits for data or error.
**
**------------------------------------------------------------------------*/
static int dtRead (NetFet *fet, NetPortSet *ps)
    {
    int i;
    u8 *in, *out, *nextin;
    int size;
    
    /*
    ** First send any pending data (if possible).
    */
    dtSendPending (fet, ps);
    
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
    if (size == 0)
        {
        /*
        ** No room for new data, so return but do not call that an error.
        */
        return 0;
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
        return i;
        }
    else
        {
        return(-1);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Thread for listening for inbound TCP connections.
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
    NetFet *np;
    int listenFd, connFd;
    struct sockaddr_in server;
    struct sockaddr_in from;
    socklen_t fromLen;
    int true_opt = 1;
    
    /*
    **  Create TCP socket
    */
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
        {
#if !defined(_WIN32)
        perror ("dtThread: Can't create socket");
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
        perror ("dtThread: Can't bind to socket");
#endif
        ThreadReturn;
        }

    if (listen(listenFd, 5) < 0)
        {
#if !defined(_WIN32)
        perror ("dtThread: Can't listen");
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
        if (!emulationActive || ps->close)
            {
            dtCloseSocket (listenFd, TRUE);
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
        **  Set Keepalive and no signals.
        */
        setsockopt(connFd, SOL_SOCKET, SO_KEEPALIVE,
                   (char *)&true_opt, sizeof(true_opt));
#ifdef __APPLE__
        setsockopt(connFd, SOL_SOCKET, SO_NOSIGPIPE,
                   (char *)&true_opt, sizeof(true_opt));
#endif

        /*
        **  Find a free slot in the NetFet vector.
        **  Note that there definitely is one, because we checked
        **  current vs. max count earlier.
        */
        np = ps->portVec;
        while (dtActive (np))
            {
            np++;
            }
        
        /*
        **  Mark connection as active.
        */
        np->from = from.sin_addr;
        np->fromPort = from.sin_port;
        dtActivateFet (np, ps, connFd);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Thread for listening for new data on a connection
**
**  Parameters:     Name        Description.
**                  param       Pointer to NetFet to use
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
#if defined(_WIN32)
static void dtDataThread(void *param)
#else
    static void *dtDataThread(void *param)
#endif
    {
    NetFet *np = (NetFet *) param;
    NetPortSet *ps = np->ps;
    int bytes;
    
    while (1)
        {
        /*
        **  If DtCyber is closing down, close the socket
        **  and exit the thread.
        */
        if (!emulationActive || ps->close)
            {
            break;
            }
        
        /*
        **  Do we have any data?
        */
        bytes = dtRead (np, ps);

        if (bytes == 0)
            {
            /*
            **  Buffer is full, sleep a while to let other threads empty it.
            */
#if defined(_WIN32)
            Sleep(1000);
#else
            /* usleep(10000000); */
            sleep(1);
#endif
            continue;
            }
            
        /*
        **  Handle errors.
        */
        if (bytes < 0)
            {
            break;
            }
        
        if (ps->dataCallBack)
            {
                (*ps->dataCallBack) (np, bytes, ps->dataCallArg);
            }
        }
    dtClose (np, ps, TRUE);
    ThreadReturn;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close a network socket
**
**  Parameters:     Name        Description.
**                  connFd      socket fd to close
**                  hard        TRUE for reset, FALSE for shutdown
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dtCloseSocket (int connFd, bool hard)
    {
#if defined(_WIN32)
    closesocket(connFd);
#else
    if (!hard)
        {
        shutdown (connFd, SHUT_RDWR);
        }
    close(connFd);
#endif
    }

/*--------------------------------------------------------------------------
**  Purpose:        Get the specified number of bytes from the buffer.
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  buf         Data buffer pointer
**                  len         Number of bytes to get
**                  read        FALSE if just peeking, TRUE to advance "out"
**
**  Returns:        -1 if <len> bytes not currently available, 0 if ok
**
**------------------------------------------------------------------------*/
static int dtGetw (NetFet *fet, void *buf, int len, bool read)
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
        out = fet->first;
        if (read)
            {
            fet->out = out;
            }
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
    if (read)
        {
        fet->out = out + len;
        }
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send pending data, if any
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  ps          Pointer to NetPortSet to use
**
**  Returns:        Nothing. 
**
**------------------------------------------------------------------------*/
void dtSendPending (NetFet *fet, NetPortSet *ps)
    {
    int connFd = fet->connFd;
    int sent = 0, pend;
//    u8 *waitptr;

    if (fet->sendCount == 0)
        {
        return;
        }
    sent = send (connFd, fet->sendData, fet->sendCount, MSG_NOSIGNAL);
    if (sent < 0)
        {
        if (errno != EAGAIN)
            {
            /*
            ** Some strange error.  Pretend the whole send worked,
            ** which results in the pending data being discarded.
            */
            sent = fet->sendCount;
            }
        else
            {
            sent = 0;
            }
        }
    pend = fet->sendCount = fet->sendCount - sent;
    
    if (pend == 0)
        {
        free (fet->sendData);
        fet->sendData = NULL;
        fet->sendBufCount = 0;
        ps->sendCount--;
        FD_CLR (fet->connFd, &ps->sendSet);
        }
    else
        {
        memmove (fet->sendData, fet->sendData + sent, pend);
        }
    }

/*---------------------------  End Of File  ------------------------------*/
