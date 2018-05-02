/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2018, Tom Hunter, Paul Koning, Steve Zoppi
**
**  Name: dtnetsubs.c
**
**  Description:
**      Common platform-dependent routines relating to networking.
**
**--------------------------------------------------------------------------
*/

// ============================================================================
// declarations
// ============================================================================

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
#include "dtnetsubs.h"
#include <time.h>
#include <sys/types.h>

// Some versions of Windows don't define these, so make up some numbers
#ifndef ENOBUFS
#define ENOBUFS 55
#endif
#ifndef ENOTCONN
#define ENOTCONN 57
#endif

#if defined(_WIN32)
    #include <winsock.h>
    #include <process.h>
#else
    #include <poll.h>
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
#define dtNC -1   /* connFd value for "socket was closed" */

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
static dtThreadFun (dtThread, param);
static dtThreadFun (dtDataThread, param);
static dtThreadFun (dtSendThread, param);
static int dtBindSocket  (in_addr_t host, int port, int backlog);
static NetFet * dtAcceptSocket (int connFd, NetPortSet *ps);
static void dtCloseSocket (int connFd, bool hard);
static int dtGetw (NetFet *fet, void *buf, int len, bool read);
static int dtSendo (NetFet *fet, u8 byte);
static NetFet * dtNewFet (int fd, NetPortSet *ps, bool listen);
static void dtActivateFet2 (NetFet *fet);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
NetPortSet *connlist;
void (*updateConnections) (void) = NULL;
int dtErrno;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static NetPortSet *connend;
#if defined(_WIN32)
static unsigned long false_opt = 0;
static unsigned long true_opt = 1;
#else
static pthread_attr_t dt_tattr;
static const int true_opt = 1;
#endif
static bool dtInited;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialize dtnetsubs global state
**
**  Parameters:     none
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void dtInit (void)
    {
    if (!dtInited)
        {
        dtInited = TRUE;
#if !defined(_WIN32)
        pthread_attr_init (&dt_tattr);
        pthread_attr_setdetachstate (&dt_tattr, PTHREAD_CREATE_JOINABLE);
#endif
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Establish an outbound connection
**
**  Parameters:     Name        Description.
**                  ps          Pointer to NetPortSet to use
**                  host        Address of host to connect to
**                  port        Port number to connect to
**
**  Returns:        Pointer to NetFet, or NULL if failure
**
**------------------------------------------------------------------------*/
NetFet * dtConnect (NetPortSet *ps, in_addr_t host, int port)
    {
    struct sockaddr_in server;
    int connFd, retval;
    NetFet *fet;
    
    /*
    **  Create TCP socket
    */
    connFd = socket(AF_INET, SOCK_STREAM, 0);
    if (connFd < 0)
        {
#if !defined(_WIN32)
        perror ("dtConnect: Can't create socket");
#endif
        dtErrno = errno;
        return NULL;
        }

    /*
    **  Set non-blocking state of the socket so we don't block on the
    **  connect () call.
    */
#if defined(_WIN32)
    // This works, but then changing it back to blocking
    // later on does not.  I can't figure out why not.
    // So turn this code off for now.
    //ioctlsocket (connFd, FIONBIO, &true_opt);
#else
    fcntl (connFd, F_SETFL, O_NONBLOCK);
#endif

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = host;
    server.sin_port = htons(port);

    retval = connect (connFd, (struct sockaddr *) &server, sizeof (server));

    if (retval < 0 && errno != EINPROGRESS)
        {
#if !defined(_WIN32)
        fprintf (stderr, "dtConnect: Can't connect to %08x %d, errno %d\n", 
                 host, port, errno);
        perror ("dtConnect");
#endif
        dtErrno = errno;
        dtCloseSocket (connFd, TRUE);
        return NULL;
        }

    /*
    **  Now that the connect is underway, set the socket back to blocking.
    **
    **  We always want the data socket to be blocking since it will
    **  be served by separate threads.
    */
#if defined(_WIN32)
    // For some reason this does nothing.
    ioctlsocket (connFd, FIONBIO, &false_opt);
#else
    fcntl (connFd, F_SETFL, 0);
#endif

    fet = dtNewFet (connFd, ps, FALSE);
    if (fet == NULL)
        {
        // Failed to allocate FET, return failure
        return NULL;
        }
    
    fet->from = server.sin_addr;
    fet->fromPort = server.sin_port;
    return fet;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Create a thread.
**
**  Parameters:     Name        Description.
**                  fp          Pointer to thread function
**                  param       thread function parameter
**                  id          Pointer to where to return thread id, 
**                              or NULL not to (makes thread detached)
**
**  Returns:        0 if ok, != 0 if not
**
**------------------------------------------------------------------------*/
int dtCreateThread (dtThreadFunPtr (fp), void *param, pthread_t *id)
#if defined(_WIN32)
    {
    uintptr_t retval;
    HANDLE hThread;

    /*
    **  Create thread.
    */
    retval = _beginthreadex (NULL, 0, fp, param, 0, NULL);
    if (retval == -1L)
        {
        dtErrno = errno;
        return 1;
        }
    hThread = (HANDLE) retval;
    if (id != NULL)
        {
        *id = hThread;
        }
    else
        {
        CloseHandle (hThread);
        }
    return 0;
    }
#else
    {
    int rc;
    pthread_t dt_thread;

    /*
    **  Create POSIX thread with detacted attribut.
    */
    rc = pthread_create(&dt_thread, &dt_tattr, fp, param);
    if (rc < 0)
        {
        fprintf (stderr, "Failed to create thread:\n %s\n", strerror (rc));
        dtErrno = rc;
        return rc;
        }
    if (id != NULL)
        {
        *id = dt_thread;
        }
    else
        {
        pthread_detach (dt_thread);
        }
    return 0;
    }
#endif

/*--------------------------------------------------------------------------
**  Purpose:        Initialize a portset, which is the structure that
**                  deals with a set of related connections.
**
**  Parameters:     Name        Description.
**                  ps          Pointer to NetPortSet to use
**
**  Returns:        Nothing.  Errors are fatal (process exits with status 1).
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
**      callArg     void * argument to pass to the above, if needed
**      dataCallBack pointer to function to be called when new data
**                  appears, or NULL if none is needed
**      dataCallArg void * argument to pass to the above, if needed
**      localOnly   TRUE to listen only to local connections (127.0.0.1),
**                  FALSE to allow connections from anywhere.
**      ringSize    FET receive buffer size for each port.
**      sendringSize FET transmit buffer size for each port.
**
**  The other fields of the NetPortSet struct are filled in by this function,
**  or by the thread it creates that actually does the listening to
**  new connections.
**
**  Each time a new connection is seen, curPorts is incremented and one of
**  the PortVec entries is loaded with the data for the new connection.
**  That data is the socket fd, and the remote IP address.
**
**  If a callback function pointer is specified, that callback
**  function is called with the NetFet entry that was filled in, the
**  portVec array index where that entry lives, and the callArg value.
**  The callback function is also called when the NetFet is closed;
**  the two cases are distinguished by the fact that the connFd is
**  non-zero if the connection is open, and zero if it is closed.
**
**  Similarly, if a data callback function pointer is specified, that
**  callback function is called whenever the receive thread receives
**  more data.  The callback function is called with the FET pointer,
**  the count of received bytes, and the dataCallArg value.  The data
**  callback function is also called when the connection closes.  Note
**  that this is not quite the same thing as the (connection)
**  callback, which is called when the NetFet is closed, presumably in
**  response to the connection loss.
**
**  Note that there currently is no "Close Portset" function, so the
**  memory allocated by this function constitutes a (small) memory leak.
**
**------------------------------------------------------------------------*/
void dtInitPortset (NetPortSet *ps)
    {
#if defined(_WIN32)
    static bool firstMux = TRUE;
    WORD versionRequested;
    WSADATA wsaData;
    int err;
#endif

    /*
    **  Initialize dtnetsubs if needed
    */
    dtInit ();
    
    /*
    **  Sanity check: if there is a send ring, there must also be a
    **  receive ring.
    */
    if (ps->sendRingSize != 0 && ps->ringSize == 0)
        {
#if !defined(_WIN32)
        fprintf (stderr, "Must have receive ring if send ring is used\n");
#endif
        exit (1);
        }
    
    /*
    **  Initialize the NetPortSet structure part way
    */
    ps->curPorts = 0;
    ps->portVec = calloc (1, sizeof(NetFet *) * ps->maxPorts);
    if (ps->portVec == NULL)
        {
#if !defined(_WIN32)
        fprintf (stderr, "Failed to allocate *NetFet[%d] vector\n",
                 ps->maxPorts);
#endif
        exit (1);
        }
    
    /*
    **  Maintain a linked list of known portsets.
    */
    ps->next = NULL;
    if (connend != NULL)
        {
        connend->next = ps;
        }
    else
        {
        connlist = ps;
        }
    connend = ps;
        
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
        fprintf (stderr, "\r\nError in WSAStartup: %d\r\n", err);
#endif
            exit (1);
            }
        }
#endif
    if (ps->portNum != 0)
        {
        if (dtCreateThread (dtThread, ps, NULL))
            {
#if !defined(_WIN32)
            fprintf (stderr, "Could not start listen thread for %d\n", ps->portNum);
#endif
            exit (1);
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close a network port
**
**  Parameters:     Name        Description.
**                  np          Pointer to NetFet being closed
**                  hard        TRUE for reset, FALSE for shutdown
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void dtClose (NetFet *fet, bool hard)
    {
    NetPortSet *ps;
    bool useThread = FALSE;

    /*
    **  Return success if already closed.
    */
    if (!dtActive (fet))
        {
        return;
        }

    /*
    **  Remember if we have threads.  This affects where the Fet is freed.
    */
    useThread = fet->sendend != fet->first;

    /*
    **  Mark the FET as closing down.
    */
    if (hard || fet->sendend == fet->sendfirst)
        {
        fet->closing = 2;
        }
    else
        {
        fet->closing = 1;
        }
    
    /*
    **  Call the connection callback for the portset, and remove this
    **  FET from the portset.
    */
    ps = fet->ps;
    fet->connected = FALSE;
    /*
    **  Do this before the callback so the callback function has the
    **  correct number of open connections.
    */
    ps->curPorts--;
    if (ps->callBack != NULL)
        {
        (*ps->callBack) (fet, fet->psIndex, ps->callArg);
        }
    ps->portVec[fet->psIndex] = NULL;
    
    if (updateConnections != NULL)
        {
        (*updateConnections) ();
        }

    /*
    **  Close the socket for this connection, if hard reset.
    */
    if (fet->closing != 1)
        {
        dtCloseSocket (fet->connFd, hard);
        fet->connFd = dtNC;        /* Indicate connection is closed */
        }

    /*
    **  If we have threads, tell the threads to go away.  (If there is
    **  no send thread, that part is a NOP).  The receive thread will
    **  free the FET.
    */
    if (useThread)
        {
        sem_post (ssemp (fet));
        sem_post (rsemp (fet));
        }

    /*
    **  If there are no threads, (listen FET), free the FET here.
    */
    if (!useThread)
        {
        free (fet);
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
    static char ts[64], us[12];
#ifdef _WIN32
    SYSTEMTIME tv;
    
    GetLocalTime (&tv);
    GetDateFormatA (LOCALE_SYSTEM_DEFAULT, 0, &tv, 
                    "yy'/'MM'/'dd' '", &ts[0], 10);
    GetTimeFormatA (LOCALE_SYSTEM_DEFAULT, 0, &tv, 
                    "HH'.'mm'.'ss", &ts[9], 10);
    sprintf (us, ".%03d.", tv.wMilliseconds);
#else
    struct timeval tv;
    struct tm tmbuf;

    gettimeofday (&tv, NULL);
    strftime (ts, sizeof (ts) - 1, "%y/%m/%d %H.%M.%S.", 
              localtime_r (&tv.tv_sec, &tmbuf));
    sprintf (us, "%06d.", (int)tv.tv_usec);
#endif
    strcat (ts, us);
    return ts;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Read one byte from the network buffer, with index
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  out         Pointer to int which is the current "out" 
**                              index.  If passed as -1, the fet's out 
**                              index is used.  The updated out index is
**                              passed back.
**
**  Returns:        -1 if no data available, or the next byte as
**                  an unsigned value.
**
**------------------------------------------------------------------------*/
int dtReadoi (NetFet *fet, int *outidx)
    {
    u8 *in, *out, *nextout;
    u8 b;

    /*
    **  Copy the pointers, since they are volatile.
    */
    in = (u8 *) (fet->in);
    if (*outidx < 0)
        {
        out = (u8 *) (fet->out);
        }
    else
        {
        out = fet->first + *outidx;
        if (out >= fet->end)
            {
            return -1;
            }
        }
    
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
    *outidx = nextout - fet->first;
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
**  Purpose:        Send a TLV formatted data item
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  tag         tag (a.k.a., type) code
**                  len         data length
**                  value       buffer holding the data
**
**  Returns:        -1 if ok, 0 if ok but full, >0 if nothing was sent. 
**
**------------------------------------------------------------------------*/
int dtSendTlv (NetFet *fet, int tag, int len, const void *value)
    {
    int retval;
    
    if (fet->sendfirst == fet->sendend)
        {
#if !defined(_WIN32)
        fprintf (stderr, "dtSendTlv called without send buffer allocated\n");
#endif
        return ENOBUFS;
        }
    
    if (tag > 255 || len > 255)
        {
#if !defined(_WIN32)
        fprintf (stderr, "dtSendTlv: bad tag/len %d %d\n", tag, len);
#endif
        return EINVAL;
        }
    if (len + 2 > dtSendFree (fet))
        {
        return 1;
        }
    
    dtSendo (fet, tag);
    dtSendo (fet, len);
    retval = dtSend (fet, value, len);

    return retval;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a buffer
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  len         data length
**                  buf         buffer holding the data
**
**  Returns:        -1 if ok, 0 if ok but full, >0 if nothing was sent. 
**                  Note that this is an all or nothing send; unlike socket
**                  send calls there isn't a partial send.
**
**------------------------------------------------------------------------*/
int dtSend (NetFet *fet, const void *buf, int len)
    {
    int size;
    u8 *in, *out, *nextin;
    
    /*
    ** Reject calls when connection is not open.
    */
    if (!dtConnected (fet) || fet->connFd == dtNC)
        {
        return ENOTCONN;
        }
    
    if (fet->sendfirst == fet->sendend)
        {
#if !defined(_WIN32)
        fprintf (stderr, "dtSend called without send buffer allocated\n");
#endif
        return ENOBUFS;
        }
    
    if (len > dtSendFree (fet))
        {
        return EAGAIN;
        }

    /*
    **  Copy the pointers, since they are volatile.
    */
    in = nextin = (u8 *) (fet->sendin);
    out = (u8 *) (fet->sendout);

    while (len > 0)
        {
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
            size = fet->sendend - in;
            if (out == fet->sendfirst)
                {
                size--;
                }
            }
        if (size > len)
            {
            size = len;
            }
        
        memcpy (in, buf, size);
        buf = (u8 *) buf + size;
        len -= size;
        nextin = in + size;
        if (nextin == fet->sendend)
            {
            nextin = fet->sendfirst;
            }
        in = nextin;
        }

    /*
    ** Update the IN pointer, then wake the send thread.
    */
    fet->sendin = in;
    if (ssemp (fet) != NULL)
    {
        sem_post (ssemp (fet));
    }
    
    if (dtSendFree (fet) == 0)
        {
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
**                  ps          Pointer to NetPortSet to use
**                  host        IP address to listen to (0.0.0.0 or 127.0.0.1)
**                  port        Port number to connect to
**                  backlog     Listen backlog
**
**  Returns:        NetFet pointer (listen FET)
**
**------------------------------------------------------------------------*/
NetFet * dtBind  (NetPortSet *ps, in_addr_t host, int port, int backlog)
    {
    int connFd;
    NetFet *fet;
    
    connFd = dtBindSocket (host, port, backlog);
    if (connFd < 0)
        {
#if !defined(_WIN32)
        perror ("dtBind: Can't create socket");
#endif
        dtErrno = errno;
        return NULL;
        }

    /*
    **  Set non-blocking state of the socket.
    */
#if defined(_WIN32)
    ioctlsocket (connFd, FIONBIO, &true_opt);
#else
    fcntl (connFd, F_SETFL, O_NONBLOCK);
#endif

    fet = dtNewFet (connFd, ps, TRUE);
    if (fet == NULL)
        {
        // Failed to allocate FET, return failure
        return NULL;
        }
    
    /*
    **  Remember what we're listening to (local address, not remote address
    **  as is the case for data sockets).
    */
    fet->from.s_addr = host;
    fet->fromPort = htons (port);
    
    return fet;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Accept a connection
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer, for a bound/listening fet
**                  ps          Pointer to NetPortSet to use
**
**  Returns:        NetFet pointer for connection, if there is one.
**                  NULL if no connection, or allocation failed.
**                  (dtError is 0 in the first case, non-zero in the second)
**
**------------------------------------------------------------------------*/
NetFet * dtAccept (NetFet *fet, NetPortSet *ps)
    {
    
    /*
    **  Accept a connection.
    */
    if (!fet->listen)
        {
        dtErrno = EINVAL;
        return NULL;
        }
    
    return dtAcceptSocket (fet->connFd, ps);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Allocate a NetFet
**
**  Parameters:     Name        Description.
**                  connFd      File descriptor for connection socket
**                  ps          Pointer to NetPortSet to use
**                  listen      TRUE if this is a listen socket
**
**  Returns:        Pointer to NetFet, or NULL on failure (dtErrno set)
**
**------------------------------------------------------------------------*/
NetFet * dtNewFet (int connFd, NetPortSet *ps, bool listen)
    {
    NetFet *fet;
    int rsize, tsize, fsize;
    int rc;
    int psi = -1;

    /*
    **  Find an empty slot in the portVec.
    */
    for (psi = 0; psi < ps->maxPorts && ps->portVec[psi] != NULL; psi++) ;
    if (psi == ps->maxPorts)
        {
        dtErrno = ENOBUFS;
        return NULL;
        }
    
    if (listen)
        {
        rsize = tsize = 0;
        }
    else
        {
        rsize = ps->ringSize;
        tsize = ps->sendRingSize;

        /*
        **  Set Keepalive if not a listen socket
        */
        setsockopt(connFd, SOL_SOCKET, SO_KEEPALIVE,
                   (char *) &true_opt, sizeof (true_opt));
        }
    
    /*
    **  Figure out the FET size, and allocate it.
    */
    fsize = sizeof (NetFet) + rsize + tsize;
    fet = (NetFet *) malloc (fsize);
    if (fet == NULL)
        {
        dtErrno = ENOMEM;
        return NULL;
        }
    memset (fet, 0, sizeof (NetFet));

#ifdef __APPLE__
    setsockopt(connFd, SOL_SOCKET, SO_NOSIGPIPE,
               (char *)&true_opt, sizeof(true_opt));
#endif
    fet->connFd = connFd;
    fet->listen = listen;
    fet->connected = TRUE;
    
    /*
    **  Initialize the ring pointers.  The receive and transmit rings
    **  go after the base NetFet in the same memory block.
    */
    fet->in = fet->out = fet->first = (u8 *) fet + sizeof (NetFet);
    fet->sendin = fet->sendout = fet->sendfirst = fet->end = fet->first + rsize;
    fet->sendend = fet->sendfirst + tsize;
    
#ifdef __APPLE__
    /* Semaphores are initialized in the thread that uses them. */
    rsemp (fet) = NULL;
    ssemp (fet) = NULL;
#else
    sem_init (rsemp (fet), 0, 0);
    sem_init (ssemp (fet), 0, 0);
#endif
    
    /*
    **  Remember which portset this NetFet belongs to.
    */
    fet->ps = ps;
    fet->psIndex = psi;
    ps->portVec[psi] = fet;
    if (!listen)
        {
        ps->curPorts++;
        }
    
    /*
    **  Create the data receive and transmit threads, if needed
    */
    if (rsize > 0)
        {
        rc = dtCreateThread (dtDataThread, fet, NULL);
        if (rc != 0)
            {
            dtErrno = rc;
            fet->sendend = fet->first;      /* No threads yet */
            dtClose (fet, TRUE);
            return NULL;
            }
        }
    if (tsize > 0)
        {
        rc = dtCreateThread (dtSendThread, fet, &fet->sendThread);
        if (rc != 0)
            {
            dtErrno = rc;
            fet->sendend = fet->sendfirst;  /* No send thread */
            dtClose (fet, TRUE);
            return NULL;
            }
        }

    /* 
    ** Tell connection watcher (like operators) about the new connection
    */
    if (updateConnections != NULL)
        {
        (*updateConnections) ();
        }

    /*
    **  Final action (portset connection callback) is done from the
    **  receive data thread handler if there is a receive thread.  If
    **  not, do it here.
    */
    if (rsize == 0)
        {
        dtActivateFet2 (fet);
        }

    return fet;
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
**
**  Returns:        >=0 if ok, value is count of bytes received
**                  -1 if disconnected
**                  -2 if some other error
**
**  This function waits for data or error.
**
**------------------------------------------------------------------------*/
static int dtRead (NetFet *fet)
    {
    int i;
    u8 *in, *out, *nextin;
    int size;
    
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

    i = recv (fet->connFd, in, size, MSG_NOSIGNAL);
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
static dtThreadFun (dtThread, param)
    {
    NetPortSet *ps = (NetPortSet *) param;
    in_addr_t host;
    int port;
    int listenFd;
    
    /*
    **  Start listening.
    */
    if (ps->localOnly)
        {
        host = inet_addr("127.0.0.1");
        }
    else
        {
        host = inet_addr("0.0.0.0");
        }
    port = ps->portNum;

    /*
    **  Create a socket, bind, and listen. 
    */
    listenFd = dtBindSocket (host, port, 5);
    if (listenFd < 0)
        {
#if !defined(_WIN32)
        perror ("dtThread: Can't create socket");
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
            dtCloseSocket (listenFd, TRUE);
            ThreadReturn;
            }
        
        /*
        **  Wait for a connection.  The things we need to do about it
        **  are all in dtAcceptSocket.
        */
        (void) dtAcceptSocket (listenFd, ps);
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
static dtThreadFun (dtDataThread, param)
    {
    NetFet *np = (NetFet *) param;
    NetPortSet *ps = np->ps;
    int bytes = 0;
#ifndef _WIN32
    struct pollfd pfd;
#ifdef __APPLE__
    char semname[64];

    /* Initialize the semaphore */
    sprintf (semname, "/dtrsem_%p", np);
    rsemp (np) = sem_open (semname, O_CREAT, 0600, 0);
    if (rsemp (np) == (void *) SEM_FAILED)
    {
        perror ("sem_open failed");
        exit (1);
    }
#endif
    
    /*
    **  Wait for socket ready or error, then do the final connection
    **  activation.
    **  If an error occurs, close the socket instead (which will generate 
    **  a close callback).  This way, the callback will occur when the
    **  connection is actually up, as opposed to immediately after an outgoing
    **  connection request is issued.
    */
    pfd.fd = np->connFd;
    pfd.events = POLLOUT;
    poll (&pfd, 1, -1);
    if (pfd.revents & (POLLHUP | POLLNVAL | POLLERR))
        {
        np->closing = 2;
        /*
        ** Fall through into main loop, which will exit because
        ** we marked the FET for close, and from there into the cleanup.
        */
        }
    else
        {
        dtActivateFet2 (np);
        }
#else
    dtActivateFet2 (np);
#endif
    
    while (1)
        {
        /*
        **  If DtCyber is closing down, close the socket
        **  and exit the thread.
        */
        if (!emulationActive)
            {
            break;
            }
        
        /*
        **  If the socket is closed now, exit.
        */
        if (!dtActive (np))
            {
            break;
            }
        
        if (!(np->connected))
            {
            /*
            **  If connection was closed by other end earlier, sleep
            **  a second to let other threads see it and deal with it.
            */
#if defined(_WIN32)
            Sleep (1000);
#else
            usleep (10000000);
#endif
            }
        else
            {
            /*
            **  Do we have any data?
            */
            bytes = dtRead (np);

            if (bytes == -1)
                {
                /*
                **  Indicate connection closed by other end.
                */
                np->connected = FALSE;
                }
            else if (bytes == 0)
                {
                /*
                **  Buffer is full.
                **  Sleep 10 ms to let other threads see it and deal with it.
                **  Note: we want to sleep only briefly, otherwise when there
                **  is a flood of traffic we may keep falling behind.
                */
#if defined(_WIN32)
                Sleep (10);
#else
                usleep (100000);
#endif
                }
            else if (bytes < 0)
                {
                /*
                **  Handle errors.
                */
                break;
                }
            }

        /*
        **  Note that the callback also happens (with byte count -1)
        **  if the connection is lost.  Some clients of this API use
        **  that case rather than (or in addition to) the connection
        **  change callback.
        */
        if (ps->dataCallBack != NULL)
            {
            (*ps->dataCallBack) (np, bytes, ps->dataCallArg);
            }
        }

    /*
    **  If this FET isn't marked for close yet, mark it.  Note the actual
    **  close call is made in the main thread, which will also allow
    **  this thread to free the FET.  It is done that way to avoid
    **  race conditions around freeing the FET.
    */
    if (np->closing == 0)
        {
        /*
        **  Tell send thread, if there is one, to quit right now.  
        */
        np->closing = 2;
        sem_post (ssemp (np));
        }

    /*
    **  Wait for the send thread to exit, if we have one. 
    */
    if (np->sendend != np->sendfirst)
        {
        pthread_join (np->sendThread, NULL);
        }

    /*
    **  Wait for dtClose to say that the FET can be freed.
    */
    sem_wait (rsemp (np));

    /*
    **  Free the semaphores, then the FET memory.
    */
#if !defined(__APPLE__)
    sem_destroy (rsemp (np));
    sem_destroy (ssemp (np));
#else
    sem_close (rsemp (np));
    sem_unlink (semname);
#endif
    
    free (np);

    ThreadReturn;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Thread for sending pending transmit data on a connection
**
**  Parameters:     Name        Description.
**                  param       Pointer to NetFet to use
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static dtThreadFun (dtSendThread, param)
    {
    NetFet *np = (NetFet *) param;
    u8 *in, *out, *nextout;
    int size;
    int bytes;
    int ret;
#ifdef __APPLE__
    char semname[64];

    /* Initialize the semaphore */
    sprintf (semname, "/dtssem_%p", np);
    ssemp (np) = sem_open (semname, O_CREAT, 0600, 0);
    if (ssemp (np) == (void *) SEM_FAILED)
    {
        perror ("sem_open failed");
        exit (1);
    }

    /* Mac does not have MSG_NOSIGNAL, so turn the SIGPIPE signal off */
    signal (SIGPIPE, SIG_IGN);
#endif
    
    while (1)
        {
        /*
        **  If DtCyber is closing down, or the socket is closed,
        **  exit the thread.
        */
        if (!emulationActive || np->closing == 2)
            {
            break;
            }
        
        /*
        **  Do we have any data?
        */
        if (dtSendEmpty (np))
            {
            /*
            **  If we have a soft close (shutdown) pending, and we no
            **  longer have data to send, do the actual shutdown.
            */
            if (np->closing == 1)
                {
                dtCloseSocket (np->connFd, FALSE);
                np->connFd = dtNC;         /* Indicate socket is closed */
                np->closing = 2;
                break;
                }
            
            /*
            **  If there is no data, wait on the semaphore (which is
            **  signaled by the main thread when data is put into the
            **  send ring).  Then go back to the top of the loop to
            **  check for reasons to exit.
            */
            ret = sem_wait (ssemp (np));
            if (ret < 0)
                {
                perror ("sem_wait");
                }
            continue;
            }
        
        /*
        **  Copy the pointers, since they are volatile.
        */
        in = (u8 *) (np->sendin);
        out = (u8 *) (np->sendout);

        if (out < in)
            {
            /*
            **  If the in pointer is beyond the out pointer, we can
            **  send the data in between.
            */
            size = in - out;
            }
        else
            {
            /*
            **  Otherwise, we send from the current out pointer to the
            **  end of the buffer.
            */
            size = np->sendend - out;
            }
        bytes = send (np->connFd, out, size, MSG_NOSIGNAL);
            
        /*
        **  Handle errors.
        */
        if (bytes < 0)
            {
            break;
            }
        else
            {
            nextout = out + bytes;
            if (nextout == np->sendend)
                {
                nextout = np->sendfirst;
                }
            np->sendout = nextout;
            }
        }

#if defined(__APPLE__)
    sem_close (ssemp (np));
    sem_unlink (semname);
#endif

    ThreadReturn;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Open a network socket to listen
**
**  Parameters:     Name        Description.
**                  host        IP address to listen to (0.0.0.0 or 127.0.0.1)
**                  port        Port number to connect to
**                  backlog     Listen backlog
**
**  Returns:        Socket number, or -1 if error.
**
**------------------------------------------------------------------------*/
static int dtBindSocket (in_addr_t host, int port, int backlog)
    {
    struct sockaddr_in server;
    int connFd;
    
    /*
    **  Create TCP socket
    */
    connFd = socket(AF_INET, SOCK_STREAM, 0);
    if (connFd < 0)
        {
#if !defined(_WIN32)
        perror ("dtBind: Can't create socket");
#endif
        dtErrno = errno;
        return -1;
        }

    /*
    **  Set reuse address
    */
    setsockopt(connFd, SOL_SOCKET, SO_REUSEADDR,
               (char *) &true_opt, sizeof (true_opt));
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = host;
    server.sin_port = htons (port);

    if (bind (connFd, (struct sockaddr *) &server, sizeof (server)) < 0)
        {
        dtCloseSocket (connFd, TRUE);
        dtErrno = errno;
        return -1;
        }
    if (listen (connFd, backlog) < 0)
        {
        dtCloseSocket (connFd, TRUE);
        dtErrno = errno;
        return -1;
        }

    return connFd;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Listen for a connection
**
**  Parameters:     Name        Description.
**                  connFd      Socket number of listen socket
**                  ps          NetPortSet to use
**
**  Returns:        NetFet pointer for connection, if there is one.
**                  NULL if no connection, or allocation failed.
**                  (dtError is 0 in the first case, non-zero in the second)
**
**------------------------------------------------------------------------*/
NetFet * dtAcceptSocket (int connFd, NetPortSet *ps)
    {
    NetFet *acceptFet;
    struct sockaddr_in from;
    socklen_t fromLen;
    
    fromLen = sizeof (from);
    connFd = accept (connFd, (struct sockaddr *) &from, &fromLen);
    if (connFd < 0)
        {
        dtErrno = errno;
        return NULL;
        }

    /*
    **  Listen socket may be nonblocking, and data socket inherits that.
    **  We always want the data socket to be blocking since it will
    **  be served by separate threads.
    */
#if defined(_WIN32)
    ioctlsocket (connFd, FIONBIO, &false_opt);
#else
    fcntl (connFd, F_SETFL, 0);
#endif

    acceptFet = dtNewFet (connFd, ps, FALSE);
    if (acceptFet == NULL)
        {
        dtCloseSocket (connFd, TRUE);
        return NULL;
        }
    
    /*
    **  Save relevant data in new FET
    */
    acceptFet->from = from.sin_addr;
    acceptFet->fromPort = from.sin_port;
    
    return acceptFet;
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
    (void) hard;   //  Nop Operation; Suppress C4100 Compiler Warning
    closesocket (connFd);
#else
    if (!hard)
        {
        shutdown (connFd, SHUT_RDWR);
        }
    close (connFd);
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
**  Purpose:        Last steps to activate the NetFet
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**
**  Returns:        Nothing. 
**
**------------------------------------------------------------------------*/
static void dtActivateFet2 (NetFet *fet)
    {
    NetPortSet *ps = fet->ps;
    
    if (ps->callBack != NULL)
        {
        (*ps->callBack) (fet, fet->psIndex, ps->callArg);
        }
    
    }

/*--------------------------------------------------------------------------
**  Purpose:        Write one byte to the network send buffer
**
**  Parameters:     Name        Description.
**                  fet         NetFet pointer
**                  byte        Byte to send
**
**  Returns:        -1 if it worked, 0 if not.
**
**      Note that this function does not wake the send thread.  It is 
**      intended for cases like dtSendTlv where several pieces are sent,
**      and the final one of those will wake the send thread.
**
**------------------------------------------------------------------------*/
static int dtSendo (NetFet *fet, u8 byte)
    {
    u8 *in, *out, *nextin;
    
    /*
    **  Copy the pointers, since they are volatile.
    */
    in = (u8 *) (fet->sendin);
    out = (u8 *) (fet->sendout);
    nextin = in + 1;
    if (nextin == fet->sendend)
        {
        nextin = fet->sendfirst;
        }

    if (nextin == out)
        {
        /*
        **  We're full
        */
        return 0;
        }
    *in = byte;
    fet->sendin = nextin;
    return -1;
    }

/*---------------------------  End Of File  ------------------------------*/
