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
#define NiuLocalStations        32          // range reserved for local stations
#define NiuLocalBufSize         50          // size of local input buffer

//#define DEBUG
//#define TRACE
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
#define STATIONS                (NiuLocalStations + platoConns)

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct portParam
{
    int         connFd;
    u32         currOutput;
    struct in_addr from;
    u16         currInput;
    u8          ibytes;      // how many bytes have been assembled into currInput (0..2)
    bool        active;
} PortParam;

typedef struct localRing
{
    u8 buf[NiuLocalBufSize];
    int get;
    int put;
} LocalRing;

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
#define RETURN return
#else
static void *niuThread(void *param);
#define RETURN return 0
#endif
static void niuWelcome(int stat);
static void niuSendstr(int stat, const char *p);
static void niuSendWord(int stat, int word);
static void niuSend(int stat, int word);
static const char *ts_now (void);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
u16 platoPort;
u16 platoConns;
extern u32 sequence;

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
static LocalRing localInput[NiuLocalStations];
static niuProcessOutput *outputHandler[NiuLocalStations];
static u32 lastFrame;
bool frameStart;
#if !defined(_WIN32)
static pthread_t niu_thread;
#endif
static bool realTiming;
#ifdef TRACE
static FILE *niuF;
static bool traced=FALSE;
#endif

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

    if (in != NULL)
    {
        fprintf (stderr, "Multiple NIUs not supported\n");
        exit (1);
    }

    if (deviceName != NULL && deviceName[0] != ';')
    {
        if (strcmp (deviceName, "realtiming") == 0)
        {
            realTiming = TRUE;
        }
        else
        {
            fprintf (stderr, "Unrecognized NIU option '%s'\n", deviceName);
            exit (1);
        }
    }
    
    // We use two channels, one for input, one for output.
    // The input channel number is given by what is normally
    // the unit number in the cyber.ini file.
    in = channelAttach(unitNo, eqNo, DtNiu);
    in->activate = niuActivate;
    in->disconnect = niuDisconnect;
    in->func = niuInFunc;
    in->io = niuInIo;
    out = channelAttach(channelNo, eqNo, DtNiu);
    out->activate = niuActivate;
    out->disconnect = niuDisconnect;
    out->func = niuOutFunc;
    out->io = niuOutIo;

    mp = portVector = calloc(1, sizeof(PortParam) * STATIONS);
    if (portVector == NULL)
    {
        fprintf(stderr, "Failed to allocate NIU context block\n");
        exit(1);
    }

    /*
    **  Initialise port control blocks.
    */
    for (i = 0; i < STATIONS; i++)
    {
        mp->active = FALSE;
        mp->connFd = 0;
        mp->ibytes = 0;
        mp++;
    }

    for (i = 0; i < NiuLocalStations; i++)
    {
        localInput[i].get = localInput[i].put = 0;
    }
    
    currInPort = -1;
    lastInPort = 0;
    
    /*
    **  Create the thread which will deal with TCP connections.
    */
    niuCreateThread ();

    /*
    **  Fire up the 0-1 Plato terminal window
    */
    ptermInit ("Plato station 0-1", FALSE);

    /*
    **  Print a friendly message.
    */
    printf("NIU initialised with input channel %o and output channel %o\n", unitNo, channelNo);
}

/*--------------------------------------------------------------------------
**  Purpose:        Report if NIU is configured
**
**  Parameters:     Name        Description.
**
**  Returns:        TRUE if it is.
**
**------------------------------------------------------------------------*/
bool niuPresent(void)
{
    return (portVector != NULL);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process local Plato mode input
**
**  Parameters:     Name        Description.
**                  key         Plato key code for station
**                  stat        Station number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void niuLocalKey(u16 key, int stat)
{
    int nextput;
    LocalRing *rp;
    
    if (stat >= NiuLocalStations)
    {
        fprintf (stderr, "Local station number of of range: %d\n", stat);
        exit (1);
    }
    rp = &localInput[stat];
    
    nextput = rp->put + 1;
    if (nextput == NiuLocalBufSize)
        nextput = 0;

    if (nextput != rp->get)
    {
        rp->buf[rp->put] = key;
        rp->put = nextput;
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Set handler address for local station output
**
**  Parameters:     Name        Description.
**                  h           Output handler function
**                  stat        Station number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void niuSetOutputHandler (niuProcessOutput *h, int stat)
{
    if (stat >= NiuLocalStations)
    {
        fprintf (stderr, "Local station number of of range: %d\n", stat);
        exit (1);
    }
    outputHandler[stat] = h;
}

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/
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
    int in;
    int nextget;
    LocalRing *rp;
    
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
            if (++port >= STATIONS)
                port = 0;
            if (port < NiuLocalStations)
            {
                // check for local terminal input
                rp = &localInput[port];
                if (rp->get != rp->put)
                {
                    currInPort = lastInPort = port;
                    activeChannel->data = 04000 + currInPort;
                    activeChannel->full = TRUE;
                    return;
                }
                if (port == lastInPort)
                    return;         // No input, leave channel empty
                continue;
            }
            mp = portVector + port;
            if (mp->active)
            {
                /*
                **  Port with active TCP connection.
                */
                if ((in = niuCheckInput(mp)) >= 0)
                {
#ifdef DEBUG
                    printf ("niu input byte %d %03o\n", mp->ibytes, in);
#endif
                    // Connection has data -- assemble it and see if we have
                    // a complete input word
                    if (mp->ibytes != 0)
                    {
                        if ((in & 0200) == 0)
                        {
                            // Sequence error, drop the byte
                            printf ("niu input sequence error, second byte %03o, port %d\n",
                                    in, port);
                            continue;
                        }
                        mp->currInput |= (in & 0177);
                        currInPort = lastInPort = port;
                        activeChannel->data = 04000 + currInPort;
                        activeChannel->full = TRUE;
                        return;
                    }
                    else
                    {
                        // first byte of key data, save it for later
                        if ((in & 370) != 0)
                        {
                            // sequence error, drop the byte
                            printf ("niu input sequence error, first byte %03o, port %d\n",
                                    in, port);
                            continue;
                        }
                        mp->currInput = in << 7;
                        mp->ibytes = 1;
                    }
                }
                else if (in == -2)
                {
                    // connection was dropped, generate an "offkey"
                    mp->currInput = 01753;  // offky2 -- immediate signoff
                    currInPort = lastInPort = port;
                    activeChannel->data = 04000 + currInPort;
                    activeChannel->full = TRUE;
                    return;
                }
            }
            if (port == lastInPort)
                return;         // No input, leave channel empty
        }
    }
    // We have a current port, so we already sent the port number;
    // now send its data.
    if (currInPort < NiuLocalStations)
    {
        // local input, send a keypress format input word
        rp = &localInput[currInPort];
        nextget = rp->get + 1;
        if (nextget == NiuLocalBufSize)
            nextget = 0;
        in = rp->buf[rp->get];
        rp->get = nextget;
        activeChannel->data = in << 1;
    }
    else
    {
        mp = portVector + currInPort;
        activeChannel->data = mp->currInput << 1;
        mp->ibytes = 0;
    }
    activeChannel->full = TRUE;
    currInPort = -1;
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
    int port;
    PortParam *mp;

    if (activeDevice->fcode != FcNiuOutput ||
        !activeChannel->full)
        return;
    
    /*
    **  Output data.
    */
    d = activeChannel->data;
    if (obytes == 0)
    {
        // first word of the triple.
        //
        // If we're doing real timing (60 frames per second), and
        // if this word is the first of a frame, don't acknowledge
        // it until it's a frame time later than the previous one.
        if (realTiming && frameStart)
        {
            if (rtcClock - lastFrame < ULL(16667))
            {
                return;
            }
            lastFrame = rtcClock;
        }
        frameStart = FALSE;

        activeChannel->full = FALSE;
        if ((d & 06000) != 04000)
        {
            printf ("niu output out of sync, first word %04o\n", d);
            return;
        }
        currOutput = (d & 01777) << 9;
        obytes = 1;
        return;
    }
    activeChannel->full = FALSE;
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

    // If end of frame bit is set, remember that so the
    // next output word is recognized as the start of a new frame.
    if ((d & 02000) != 0)
    {
        frameStart = TRUE;
#ifdef TRACE
        if (traced)
        {
            if (niuF == NULL)
            {
                niuF = fopen("niu.trc", "wt");
            }
            fprintf (niuF, "%06d frame end\n", sequence);
            traced = FALSE;
        }
#endif
    }

    port = (d & 01777);
    
    // Now that we have a complete output triple, discard it
    // if it's for a station number out of range (larger than
    // what we're configured to support) or without an 
    // active TCP connection.
    niuSend (port, currOutput);
    obytes = 0;

    if (frameStart)
    {
        // Frame just ended -- send pending output words
        for (port = 1; port < STATIONS; port++)
        {
            mp = portVector + port;
            if (mp->currOutput != 0)
            {
                niuSendWord (port, mp->currOutput);
                mp->currOutput = 0;
            }
        }
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
**  Purpose:        Create thread which will deal with all TCP
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

    /*
    **  Create POSIX thread with default attributes.
    */
    rc = pthread_create(&niu_thread, NULL, niuThread, 0);
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
    int i;
    int reuse = 1;
    
    /*
    **  Create TCP socket and bind to specified port.
    */
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0)
    {
        printf("niu: Can't create socket\n");
        RETURN;
    }

    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    server.sin_port = htons(platoPort);

    if (bind(listenFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("niu: Can't bind to socket\n");
        RETURN;
    }

    if (listen(listenFd, 5) < 0)
    {
        printf("niu: Can't listen\n");
        RETURN;
    }

    while (1)
    {
        /*
        **  Find a free port control block.
        */
        mp = portVector + NiuLocalStations;
        for (i = NiuLocalStations; i < STATIONS; i++)
        {
            if (!mp->active)
            {
                break;
            }
            mp += 1;
        }

        if (i == STATIONS)
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
        mp->from = from.sin_addr;
        i = mp - portVector;
        printf("%s niu: Received connection from %s for station %d-%d\n",
               ts_now (), inet_ntoa (from.sin_addr), i / 32, i % 32);
        niuWelcome (i);
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Check for input.
**
**  Parameters:     Name        Description.
**                  mp          pointer to PortParam struct for the connection.
**
**  Returns:        data byte received
**                  -1 if there is no data
**                  -2 if the connection was dropped
**
**------------------------------------------------------------------------*/
static int niuCheckInput(PortParam *mp)
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
            i = mp - portVector;
            printf("%s niu: Connection dropped from %s for station %d-%d\n",
                   ts_now (), inet_ntoa (mp->from),  i / 32, i % 32);
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
        i = mp - portVector;
        printf("%s niu: Connection dropped from %s for station %d-%d\n",
               ts_now (), inet_ntoa (mp->from),  i / 32, i % 32);
        return(-2);
    }
    else
    {
        return(-1);
    }
}


/*--------------------------------------------------------------------------
**  Purpose:        Send a welcome message to a station
**
**  Parameters:     Name        Description.
**                  stat        Station number
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuWelcome(int stat)
{
    char msg[100];
    
    sprintf (msg, "Connected to Plato station %d-%d", stat >> 5, stat & 037);
    niuSendWord (stat, 0100033);        // mode 3, mode rewrite, screen
    niuSendWord (stat, 0201200);        // load Y = 128
    niuSendWord (stat, 0200200);        // load X = 128
    niuSendstr (stat, msg);
}


/*--------------------------------------------------------------------------
**  Purpose:        Send a string to a station
**
**  Parameters:     Name        Description.
**                  stat        Station number
**                  p           pointer to ASCII string
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuSendstr(int stat, const char *p)
{
    int cc = 2;
    int w = 017720;
    bool shift = FALSE;
    char c;
    
    while ((c = *p++) != 0)
    {
        if (isupper (c))
        {
            c = tolower (c);
            if (!shift)
            {
                w = (w << 6 | 077);
                if (++cc == 3)
                {
                    cc = 0;
                    niuSendWord (stat, w);
                    w = 1;
                }
                w = (w << 6 | 021);
                if (++cc == 3)
                {
                    cc = 0;
                    niuSendWord (stat, w);
                    w = 1;
                }
                shift = TRUE;
            }
        }
        else if (shift)
        {
            w = (w << 6 | 077);
            if (++cc == 3)
            {
                cc = 0;
                niuSendWord (stat, w);
                w = 1;
            }
            w = (w << 6 | 020);
            if (++cc == 3)
            {
                cc = 0;
                niuSendWord (stat, w);
                w = 1;
            }
            shift = FALSE;
        }
        w = (w << 6 | asciiToCdc[c]);
        if (++cc == 3)
        {
            cc = 0;
            niuSendWord (stat, w);
            w = 1;
        }
    }
    if (cc > 0)
    {
        while (cc < 3)
        {
            w = (w << 6 | 077);
            cc++;
        }
        niuSendWord (stat, w);
    }
}


/*--------------------------------------------------------------------------
**  Purpose:        Store an output word to be sent for this stations
**
**  Parameters:     Name        Description.
**                  stat        Station number
**                  word        NIU data word
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuSend(int stat, int word)
{
    PortParam *mp;

    if (stat == 0 || stat >= STATIONS)
    {
        return;
    }
    mp = portVector + stat;
#if 0
    if (mp->currOutput)
    {
        printf ("niu: station %d frame word was %07o replaced by %07o\n",
                stat, mp->currOutput, word);
    }
#endif
    mp->currOutput = word;
}

/*--------------------------------------------------------------------------
**  Purpose:        Send an output word to a station
**
**  Parameters:     Name        Description.
**                  stat        Station number
**                  word        NIU data word
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuSendWord(int stat, int word)
{
    PortParam *mp;
    u8 data[3];

#ifdef TRACE
    if (stat > 0 && stat < STATIONS)
    {
        traced = TRUE;
        if (niuF == NULL)
        {
            niuF = fopen("niu.trc", "wt");
        }
        fprintf (niuF, "%06d %2d-%2d %07o\n", sequence, stat / 32, stat % 32, word);
    }
#endif

    if (stat < NiuLocalStations)
    {
        if (outputHandler[stat] != NULL)
        {
            (*outputHandler[stat]) (stat, word);
        }
    }
    else
    {
        if (stat >= STATIONS)
            return;
        mp = portVector + stat;
        if (!mp->active)
            return;
        data[0] = word >> 12;
        data[1] = ((word >> 6) & 077) | 0200;
        data[2] = (word & 077) | 0300;
#ifdef DEBUG
        printf ("niu output %03o %03o %03o\n",
                data[0], data[1], data[2]);
#endif
        send(mp->connFd, data, 3, 0);
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
static const char *ts_now (void)
{
    static char ts[40];
    time_t t;

    time (&t);
    strftime (ts, sizeof (ts) - 1, "%y/%m/%d %H.%M.%S.", localtime (&t));
    return ts;
}


/*---------------------------  End Of File  ------------------------------*/
