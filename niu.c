/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2005, Tom Hunter, Paul Koning (see license.txt)
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
#include <string.h>
#include <ctype.h>
#include "const.h"
#include "types.h"
#include "proto.h"
#include <time.h>
#if defined(_WIN32)
#include <winsock.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#endif
#include <errno.h>

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define NiuLocalOffset          1           /* starting station for local */
#define NiuRemoteOffset         32          /* starting station for remotes */
#define NiuLocalBufSize         50          /* size of local input buffer */
#define NetBufSize              256

/* -ext- words for operator box */
#define resett 0170000      /* keepalive (should show up every 10 seconds) */
#define resetb 0130000      /* disable (at system shutdown */
#define ext    0600000

/* -echo- words */
#define echotype 0300160    /* -echo- code to inquire about terminal type */

/* keepalive timeout */
#define ALERT_TIMEOUT           15          /* seconds */

/* program to for operator box actions */
#define OPERBOX_EXEC            "./operbox"

/* Argument strings for the operator box program */
#define OPERBOX_UP              "up"
#define OPERBOX_DOWN            "down"
#define OPERBOX_CRASH           "crash"

/*
**  Function codes.
*/
/*
**  Input channel:
*/
#define FcNiuDeselectInput      00000
#define FcNiuSelectInput        00040
#define FcNiuSelectBlackBox     00041

/*
**  Output channel:
*/
#define FcNiuSelectOutput       00000
#define FcNiuDeselectOutput     07000

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define STATIONS                (NiuRemoteOffset + platoConns)

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct portParam
    {
    NetPort     *np;
    u32         currOutput;
    u8          currInput[2];
    bool        loggedIn;
    bool        sendLogout;
    bool        forceLogout;
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
static void niuWelcome(NetPort *np, int stat);
static void niuLocalWelcome(NetPort *np, int stat);
static void niuRemoteWelcome(NetPort *np, int stat);
void niuSendstr(int stat, const char *p);
void niuSendWord(int stat, int word);
static void niuSend(int stat, int word);
static void niuUpdateStatus (void);
static void niuOpdata (int word);
#if !defined(_WIN32)
static ThreadFunRet niuThread (void *param);
static void niuCheckAlert (void);
static void niuDoAlert (const char *msg);
#endif

/*
**  ----------------
**  Public Variables
**  ----------------
*/
long platoPort;
long platoConns;
long platoLocalPort;
long platoLocalConns;
long niuOpstat;
extern u32 sequence;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static DevSlot *in;
static DevSlot *out;
static PortParam *portVector;
static NetPortSet niuPorts;
static NetPortSet niuLocalPorts;
static int currInPort;
static int lastInPort;
static int obytes;
static u32 currOutput;
static u32 lastFrame;
bool frameStart;
static bool realTiming;
#ifdef TRACE
static FILE *niuF;
static bool traced=FALSE;
#endif
static void *statusBuf;
static int niuActiveConns;
static u32 niuLastAlertReset;

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
    
    /*
    **  We use two channels, one for input, one for output.
    **  The input channel number is given by what is normally
    **  the unit number in the cyber.ini file.
    */
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

    /*
    **  We allow up to 31 local stations
    */
    if (platoLocalConns > NiuRemoteOffset - NiuLocalOffset)
        {
        platoLocalConns = NiuRemoteOffset - NiuLocalOffset;
        }
    
    portVector = calloc(1, sizeof(PortParam) * STATIONS);
    if (portVector == NULL)
        {
        fprintf(stderr, "Failed to allocate NIU context block\n");
        exit(1);
        }

    currInPort = -1;
    lastInPort = NiuLocalOffset;        /* Start first scan at 0-2 */
    
    /*
    **  Create the threads which will deal with TCP connections.
    */
    niuPorts.portNum = platoPort;
    niuPorts.maxPorts = platoConns;
    niuPorts.localOnly = FALSE;
    niuPorts.callBack = niuRemoteWelcome;
    niuLocalPorts.portNum = platoLocalPort;
    niuLocalPorts.maxPorts = platoLocalConns;
    niuLocalPorts.localOnly = TRUE;
    niuLocalPorts.callBack = niuLocalWelcome;
    
    dtCreateListener (&niuPorts, NetBufSize);
    dtCreateListener (&niuLocalPorts, NetBufSize);

    /*
    **  Allocate the operator status buffer
    */
    statusBuf = opInitStatus ("NIU", channelNo, 0);

    /*
    **  Start the operator alert box thread, if needed.
    **
    **  Note that this is currently only supported on Unix-like
    **  platforms because of the need for fork and exec.
    */
#if !defined(_WIN32)
    if (niuOpstat > 0)
        {
        dtCreateThread (niuThread, NULL);
        }
#endif

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
    PortParam *mp;
    int i;
    
    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case FcNiuSelectInput:
    case FcNiuSelectBlackBox:
        currInPort = -1;
        break;

    case FcNiuDeselectInput:
        /*
        **  When PIO initializes or drops, it sends a Deselect Input
        **  function to the NIU.  We then mark all stations as logged
        **  out, since clearly that is what they are right then.
        */
        for (i = 0; i < STATIONS; i++)
            {
            mp = portVector + i;
            mp->loggedIn = FALSE;
            mp->sendLogout = FALSE;
            mp->forceLogout = FALSE;
            }
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

    case FcNiuSelectOutput:
        obytes = 0;
        break;

    case FcNiuDeselectOutput:
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
    NetPort *np;
    int i;
    
    if ((activeDevice->fcode != FcNiuSelectBlackBox &&
         activeDevice->fcode != FcNiuSelectInput) ||
        activeChannel->full)
        return;
    
    if (currInPort < 0)
        {
        /*
        **  We're at the first of the two-word input sequence.
        **
        **  Scan the active connections, round robin, looking
        **  for one that has pending input.  We need two bytes of
        **  data to be waiting in order to proceed with a given
        **  connection.
        */
        port = lastInPort;
        for (;;)
            {
            if (++port == STATIONS)
                {
                /*
                **  Whenever we finish a scan pass through the ports
                **  (i.e., we've wrapped around) look for more data.
                */
                port = NiuLocalOffset;
                for (;;)
                    {
                    np = dtFindInput (&niuPorts, 0);
                    if (np == NULL)
                        {
                        break;
                        }
                    i = dtRead  (&np->fet, -1);
                    if (i < 0)
                        {
                        dtClose (np, &niuPorts);
                        }
                    }
                for (;;)
                    {
                    np = dtFindInput (&niuLocalPorts, 0);
                    if (np == NULL)
                        {
                        break;
                        }
                    i = dtRead  (&np->fet, -1);
                    if (i < 0)
                        {
                        dtClose (np, &niuLocalPorts);
                        }
                    }
                }

            mp = portVector + port;
            np = mp->np;
            
            if (mp->sendLogout)
                {
                /*
                **  If we disconnected without logging out, send
                **  *offky2* which tells PLATO to log out this
                **  station.
                */
                mp->currInput[0] = 01753 >> 7;
                mp->currInput[1] = 01753 & 0177;
                mp->sendLogout = FALSE;
                break;
                }
            if (np == NULL || !dtActive (&np->fet))
                {
                if (port == lastInPort)
                    {
                    return;                     /* no input anywhere */
                    }
                continue;
                }
            i = dtReadw (&np->fet, mp->currInput, 2);
            if (i < 0)
                {
                if (port == lastInPort)
                    {
                    return;                     /* no input anywhere */
                    }
                continue;                   /* we don't have 2 bytes yet */
                }
            if (mp->forceLogout)
                {
                /*
                **  If we're not logged out yet, send *offky2* which
                **  tells PLATO to log out this station.
                */
                mp->currInput[0] = 01753 >> 7;
                mp->currInput[1] = 01753 & 0177;
                printf ("continuing to force logout port %d\n", port);
                break;
                }
#ifdef DEBUG
            printf ("niu input %03o %03o\n", mp->currInput[0],
                    mp->currInput[1]);
#endif
            if ((mp->currInput[0] & 0370) != 0)
                {
                /* sequence error, drop the byte */
                printf ("niu input sequence error, first byte %03o, port %d\n",
                        mp->currInput[0], port);
                return;
                }
            if ((mp->currInput[1] & 0200) == 0)
                {
                /* Sequence error, drop the byte */
                printf ("niu input sequence error, second byte %03o, port %d\n",
                        mp->currInput[1], port);
                return;
                }
            break;
            }
        currInPort = lastInPort = port;
        activeChannel->data = 04000 + currInPort;
        activeChannel->full = TRUE;
#ifdef DEBUG
        printf ("niu input data (station number) %04o\n", 
                activeChannel->data);
#endif
        return;         /* return with data */
        }
    /*
    **  We have a current port, so we already sent the port number;
    **  now send its data.
    */
    mp = portVector + currInPort;
    activeChannel->data = ((mp->currInput[0] << 7) |
                           (mp->currInput[1] & 0177)) << 1;
#ifdef DEBUG
    printf ("niu input data (keycode) %04o\n", 
            activeChannel->data);
#endif
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

    if (activeDevice->fcode != FcNiuSelectOutput ||
        !activeChannel->full)
        return;
    
    /*
    **  Output data.
    */
    d = activeChannel->data;
    if (obytes == 0)
        {
        /*
        **  First word of the triple.
        ** 
        **  If we're doing real timing (60 frames per second), and
        **  if this word is the first of a frame, don't acknowledge
        **  it until it's a frame time later than the previous one.
        */
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
        /* Second word of the triple */
        if ((d & 06001) != 0)
            {
            printf ("niu output out of sync, second word %04o\n", d);
            return;
            }
        currOutput |= d >> 1;
        obytes = 2;
        return;
        }
    /* Third word of the triple */
    if ((d & 04000) != 0)
        {
        printf ("niu output out of sync, third word %04o\n", d);
        return;
        }

    /*
    **  If end of frame bit is set, remember that so the
    **  next output word is recognized as the start of a new frame.
    */
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
    
    /*
    **  Now that we have a complete output triple, discard it
    **  if it's for a station number out of range (larger than
    **  what we're configured to support) or without an 
    **  active TCP connection.
    */
    niuSend (port, currOutput);
    obytes = 0;

    if (frameStart)
        {
        /* Frame just ended -- send pending output words */
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
**  Purpose:        Update operator status
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void niuUpdateStatus (void)
    {
    char msg[64];
    
    sprintf (msg, "%d connection%s", niuActiveConns,
             (niuActiveConns != 1) ? "s" : "");
    if (niuActiveConns == 0)
        {
        opSetStatus (statusBuf, NULL);
        }
    else
        {
        opSetStatus (statusBuf, msg);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a welcome message to a station
**
**  Parameters:     Name        Description.
**                  np          NetPort pointer
**                  stat        station number
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuWelcome(NetPort *np, int stat)
    {
    PortParam *mp;

    mp = portVector + stat;
    mp->np = np;
    
    if (np->fet.connFd == 0)
        {
        /*
        **  Connection was dropped.
        */
        printf("%s niu: Connection dropped from %s for station %d-%d\n",
               dtNowString (), inet_ntoa (np->from), 
               stat / 32, stat % 32);
        niuActiveConns--;
        niuUpdateStatus ();
        if (mp->loggedIn)
            {
            /*
            **  If we're not logged out yet, set a flag to send
            **  *offky2*, which tells PLATO to log out this station.
            */
            mp->sendLogout = TRUE;
            }
        return;
        }

    /*
    **  New connection for this port.
    */
    printf("%s niu: Received connection from %s for station %d-%d\n",
           dtNowString (), inet_ntoa (np->from),
           stat / 32, stat % 32);
    
    niuActiveConns++;
    niuUpdateStatus ();

    /*
    **  If we're not logged out yet (i.e. PLATO dropped the *offky2*)
    **  that was sent when the connection dropped on this port the
    **  last time it had a connection), set a flag to keep working on
    **  it.  With that flag set, we'll send another *offky2* in the
    **  key input handler for each key entered by the user, until
    **  PLATO indicates that the logout has been done.
    */
    if (mp->loggedIn)
        {
        printf ("need to force logout for port %d\n", stat);
        mp->forceLogout = TRUE;
        }

    niuSendWord (stat, 0042000 + stat); /* NOP with station number in it */
    niuSendWord (stat, 0100033);        /* mode 3, mode rewrite, screen */
    niuSendWord (stat, 0201200);        /* load Y = 128 */
    niuSendWord (stat, 0200200);        /* load X = 128 */
    /*
    **  Display an appropriate message depending on whether the NIU
    **  is selected -- i.e., PIO is running.
    */
    if (in->fcode == FcNiuSelectInput)
        {
        niuSendstr (stat, "Press  NEXT  to begin");
        }
    else if (in->fcode != FcNiuSelectBlackBox)

        {
        niuSendstr (stat, "PLATO not active");
        }
    
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a welcome message to a local or remote station
**
**  Parameters:     Name        Description.
**                  np          NetPort pointer
**                  stat        NetPort index
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuLocalWelcome(NetPort *np, int stat)
    {
    niuWelcome (np, stat + NiuLocalOffset);
    }

static void niuRemoteWelcome(NetPort *np, int stat)
    {
    niuWelcome (np, stat + NiuRemoteOffset);
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
void niuSendstr(int stat, const char *p)
    {
    int cc = 2;
    int w = 017720;
    bool shift = FALSE;
    int c;
    
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
    if (word == 3)
        {
        /*
        **  NOP code 3 is sent by the formatter when PLATO tell it
        **  that the station is logging out.  It is actually sent
        **  just before the "Press NEXT to begin" string.  Until we see
        **  that, we'll have to keep forcing this station off if a new
        **  connection is assigned to it.
        **
        **  Note that usually a station is signed off as soon as the
        **  connection drops, but if PLATO manages to lose that key,
        **  this might not happen.  We do see that on rare occasions.
        **  To cure this, we now have this explicit handshake; PLATO
        **  has to tell us that the station has logged out.
        **
        **  If we believe that the station is logged in but PLATO
        **  knows it is logged out, then it will also send a NOP 3
        **  when it receives the *offky2*.  That way we have explicit
        **  resynchronization.
        */
        mp->loggedIn = FALSE;
        mp->forceLogout = FALSE;
        }
    else if (word == echotype)
        {
        mp->loggedIn = TRUE;
        }
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
void niuSendWord(int stat, int word)
    {
    NetFet *fet;
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

    /*
    **  If we have an operator station defined and this word is going there,
    **  take a look at it.  It may be the operator alert box control codes.
    */
    if (stat == niuOpstat)
        {
        niuOpdata (word);
        }
    
    if (stat < NiuRemoteOffset)
        {
        stat -= NiuLocalOffset;
        if (stat < 0 || stat >= platoLocalConns)
            {
            return;
            }
        fet = &(niuLocalPorts.portVec + stat)->fet;
        }
    else
        {
        stat -= NiuRemoteOffset;
        if (stat >= platoConns)
            {
            return;
            }
        fet = &(niuPorts.portVec + stat)->fet;
        }
    
    data[0] = word >> 12;
    data[1] = ((word >> 6) & 077) | 0200;
    data[2] = (word & 077) | 0300;
#ifdef DEBUG
    printf ("niu output %03o %03o %03o\n",
            data[0], data[1], data[2]);
#endif
    dtSend(fet, data, 3);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Do operator station specific processing
**
**  Parameters:     Name        Description.
**                  word        NIU data word
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuOpdata(int word)
    {
#if !defined(_WIN32)    /* only non-Windows for now */
    /*
    **  This is where we implement the special handling of the operator
    **  station.  We emulate the operator's alert box.  That reacts
    **  to -ext- command words.
    **  The specific command format can be seen by reading lesson "stats1".
    **  For now, we handle just two things:
    **  ext 0170000  keepalive
    **  ext 0130000  shutdown
    **
    **  Keepalive enables the status watcher, and resets the timeout.  If
    **  no keepalives are then seen for the timeout period, we issue an alert.
    **  Shutdown turns off the status watcher; this happens (in lesson stats1)
    **  in a controlled PLATO shutdown.
    */
    if (word == ext + resett)
        {
        if (niuLastAlertReset == 0)
            {
            /*
            **  First keepalive message since startup or since it was shut off,
            **  so report a startup.
            */
            niuDoAlert (OPERBOX_UP);
            }
        niuLastAlertReset = rtcClock;
        }
    else if (word == ext + resetb)
        {
        /*
        **  Report a shutdown
        */
        niuDoAlert (OPERBOX_DOWN);
        niuLastAlertReset = 0;
        }
#endif
    }


/*--------------------------------------------------------------------------
**  Purpose:        Operator alert box thread
**
**  Parameters:     Name        Description.
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
#if !defined(_WIN32)    /* only non-Windows for now */

#if defined(_WIN32)
static void niuThread(void *param)
#else
static void *niuThread(void *param)
#endif
    {
    while (emulationActive)
        {
        niuCheckAlert ();
        sleep (ALERT_TIMEOUT / 2);
        }

    /*
    **  If the alert wasn't shut off, raise the alarm now as part
    **  of DtCyber termination.
    */
    if (niuLastAlertReset != 0)
        {
        niuDoAlert (OPERBOX_CRASH);
        }
        ThreadReturn;
    }


/*--------------------------------------------------------------------------
**  Purpose:        Check for operator alert timeout
**
**  Parameters:     Name        Description.
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuCheckAlert (void)
    {
    if (niuLastAlertReset != 0 &&
        (rtcClock - niuLastAlertReset) / 1000000 > ALERT_TIMEOUT)
        {
        /* Only raise the alarm once */
        niuLastAlertReset = 0;
        niuDoAlert (OPERBOX_CRASH);
        }
    }


/*--------------------------------------------------------------------------
**  Purpose:        Issue the operator alarm
**
**  Parameters:     Name        Description.
**                  msg         A string to pass to the operbox program
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuDoAlert (const char *msg)
    {
    if (fork () == 0)
        {
        execlp (OPERBOX_EXEC, OPERBOX_EXEC, msg, NULL);
        
        /*
        **  Just exit if that didn't work.
        */
        exit (0);
        }
    }

#endif  /* !WIN32 */

/*---------------------------  End Of File  ------------------------------*/
