/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, 2004, Tom Hunter, Paul Koning (see license.txt)
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
#include <time.h>
#if defined(_WIN32)
#include <winsock.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
    bool        sendOffkey;
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

/*
**  ----------------
**  Public Variables
**  ----------------
*/
long platoPort;
long platoConns;
long platoLocalPort;
long platoLocalConns;
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
    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case FcNiuSelectInput:
    case FcNiuSelectBlackBox:
        currInPort = -1;
        break;

    case FcNiuDeselectInput:
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
    NetPortSet *ps;
    int i;
    int nextget;
    
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
                        portVector[NiuRemoteOffset + (np - niuPorts.portVec)].sendOffkey = TRUE;
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
                        portVector[NiuLocalOffset + (np - niuLocalPorts.portVec)].sendOffkey = TRUE;
                        dtClose (np, &niuLocalPorts);
                        }
                    }
                }

            mp = portVector + port;
            np = mp->np;
            
            if (mp->sendOffkey)
                {
                /* connection was dropped */
                mp->currInput[0] = 01753 >> 7;
                mp->currInput[1] = 01753 & 0177;  /* offky2 -- immediate signoff */
                mp->sendOffkey = FALSE;
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
        printf("%s niu: Connection dropped from %s for station %d-%d\n",
               dtNowString (), inet_ntoa (np->from), 
               stat / 32, stat % 32);
        niuActiveConns--;
        niuUpdateStatus ();
        return;
        }

    
    printf("%s niu: Received connection from %s for station %d-%d\n",
           dtNowString (), inet_ntoa (np->from),
           stat / 32, stat % 32);
    
    niuActiveConns++;
    niuUpdateStatus ();

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
void niuSendWord(int stat, int word)
    {
    int fd;
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

    if (stat < NiuRemoteOffset)
        {
        stat -= NiuLocalOffset;
        if (stat < 0 || stat >= platoLocalConns)
            {
            return;
            }
        fd = (niuLocalPorts.portVec + stat)->fet.connFd;
        }
    else
        {
        stat -= NiuRemoteOffset;
        if (stat >= platoConns)
            {
            return;
            }
        fd = (niuPorts.portVec + stat)->fet.connFd;
        }
    if (fd == 0)
        {
        return;
        }
    
    data[0] = word >> 12;
    data[1] = ((word >> 6) & 077) | 0200;
    data[2] = (word & 077) | 0300;
#ifdef DEBUG
    printf ("niu output %03o %03o %03o\n",
            data[0], data[1], data[2]);
#endif
    /*
    **  Note that we ignore the send status.  If the connection has
    **  gone away, an error will be returned which we ignore here.
    **  It will be detected on the next read, which gives us an 
    **  opportunity to send an "offkey" to PLATO.
    */
        send(fd, data, 3, MSG_NOSIGNAL);
    }


/*---------------------------  End Of File  ------------------------------*/
