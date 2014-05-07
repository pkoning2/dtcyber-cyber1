/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2010, Tom Hunter, Paul Koning (see license.txt)
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
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include <errno.h>

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define niuFirstStation    1
#define STAT2IDX(s) ((s) - niuFirstStation)
#define IDX2STAT(i) ((i) + niuFirstStation)

#define NetBufSize              256
#define SendBufSize             2048

/* -ext- words for operator box */
#define resett 0170000      /* keepalive (should show up every 10 seconds) */
#define op_call 04000       /* operator call bit (sent with resett) */
#define mode3   02000       /* mode 3 (operator box indicator light) */
#define resett_m 0770000    /* mask to trim out resett modifier flags */
#define resetb 0130000      /* disable (at system shutdown */
#define ext    0600000

/* -echo- words */
#define echotype 0300160    /* -echo- code to inquire about terminal type */

/* keepalive timeout -- use a conservative value. */
#define ALERT_TIMEOUT           25          /* seconds */

/* program to for operator box actions */
#define OPERBOX_EXEC            "./operbox"

/* file to write alert message to */
#define OPERBOX_MSGFILE         "/tmp/operbox.txt"

/* Argument codes for the operator box program */
#define OPERBOX_UP              0
#define OPERBOX_DOWN            1
#define OPERBOX_CRASH           2
#define OPERBOX_OPCALL          3
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
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct siteParam
    {
    u32     port;
    int     terms;
    int     first;
    const char *siteName;
    void    *statusBuf;
    bool    local;
    NetPortSet niuPorts;
    } SiteParam;

typedef struct portParam
    {
    NetFet      *np;
    SiteParam   *sp;
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
static void niuWelcome(NetFet *np, int stat, void *);
void niuSendstr(int stat, const char *p);
void niuSendWord(int stat, int word);
static void niuSend(int stat, int word);
static void niuUpdateStatus (SiteParam *sp);
static void niuOpdata (int word);
#if !defined(_WIN32)
static dtThreadFun (niuThread, param);
static void niuCheckAlert (void);
#endif

/*
**  ----------------
**  Public Variables
**  ----------------
*/
int niuStations;
int niuStationEnd;
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
static u32 lastFrame;
bool frameStart;
static bool realTiming;
#ifdef TRACE
static FILE *niuF;
static bool traced=FALSE;
#endif
static int niuActiveConns;
static volatile u32 niuAlertCount;
static u32 niuLastAlertCount;
static u32 niuLastAlertFlags = 0;
static int niuOpstat;
static int sites;
static SiteParam *siteVector;

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
**  The network parameters are taken from the "plato" 
**  section in the cyber.ini file.  The lines in that section specify PLATO
**  terminal connection information, for both NIU and PNI.  The entries have
**  the following format:
**    type=name,count,port[,local]
**  where "type" is "pni" or "niu", "name" is a string identifying this group
**  of terminals (for example a PLATO site name).  "count" and "port" are decimal
**  integers, the number of terminals and the TCP listen port respectively.
**  "local" is omitted for a general access port, or the keyword "local" to have
**  the specified port number available only on the local address (127.0.0.1).
**  In addition, NIU initialization recognizes two additional entries in
**  the "plato" section:
**    realtiming        indicates that 1200 baud operation is simulated
**    operstation=n     set operator station to n
**  If these are omitted then high speed mode is used and no operator station
**  is defined.
**
**  If the "plato" section is omitted, a default configuration is supplied.
**
**------------------------------------------------------------------------*/
void niuInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    char *line;
    char *token;
    char *sitename;
    int i, j, terms, port;
    bool local;
    PortParam *mp;
    SiteParam *sp;
    long savedPos, sectionPos;
    
    (void)eqNo;

    if (in != NULL)
        {
        fprintf (stderr, "Multiple NIUs not supported\n");
        exit (1);
        }

    if (platoSection[0] != '\0')
        {
        savedPos = initSavePosition ();
        
        if (!initOpenSection(platoSection))
            {
            fprintf(stderr, "Required section [%s] not found in startup file\n", platoSection);
            exit(1);
            }
        sectionPos = initSavePosition ();

        /*
        **  Process options and count port number lines
        */
        sites = 0;
        while  ((line = initGetNextLine()) != NULL)
            {
            token = strtok(line, "=");
            if (strcmp (line, "realtiming") == 0)
                {
                realTiming = TRUE;
                continue;
                }
            else if (token != NULL && strcmp (token, "operstation") == 0)
                {
                token = strtok(NULL, "=");
                if (token != NULL)
                    {
                    niuOpstat = atoi (token);
                    }
                else
                    {
                    fprintf (stderr, "Invalid oper station\n");
                    }
                }
            else if (strcmp (token, "niu") == 0)
                {
                sites++;
                }
            }
        
        /*
        **  Allocate site data structures, then parse port number lines
        */
        siteVector = calloc (1, sites * sizeof (SiteParam));
        if (siteVector == NULL)
            {
            fprintf (stderr, "Failure allocating NIU site data vector\n");
            exit (1);
            }
        initRestorePosition (sectionPos);
        sp = siteVector;
        niuStations = 0;
        while  ((line = initGetNextLine()) != NULL)
            {
            if (strncmp (line, "niu=,", 4) != 0)
                {
                continue;
                }
            token = strtok(line + 4, ",\n");
            if (token == NULL)
                {
                fprintf (stderr, "Bad line in PLATO section: %s\n", line);
                continue;
                }
            sitename = strdup (token);
            token = strtok (NULL, ",\n");
            if (token == NULL)
                {
                fprintf (stderr, "Bad line in PLATO section: %s\n", line);
                continue;
                }
            terms = atoi (token);
            token = strtok (NULL, ",\n");
            if (token == NULL)
                {
                fprintf (stderr, "Bad line in PLATO section: %s\n", line);
                continue;
                }
            port = atoi (token);
            token = strtok (NULL, ",\n");
            local = FALSE;
            if (token != NULL)
                {
                /* Possible "local" keyword */
                if (strncmp (token, "local", 5) != 0)
                    {
                    fprintf (stderr, "Bad line in PLATO section: %s\n", line);
                    continue;
                    }
                local = TRUE;
                }
            sp->first = IDX2STAT (niuStations);
            sp->port = port;
            sp->terms = terms;
            sp->local = local;
            sp->siteName = sitename;
            sp++;
            niuStations += terms;
            }
        initRestorePosition (savedPos);
        }
    else
        {
        /*
        **  No PLATO section, supply defaults
        */
        sites = 2;
        siteVector = calloc (1, sites * sizeof (SiteParam));
        if (siteVector == NULL)
            {
            fprintf (stderr, "Failure allocating NIU site data vector\n");
            exit (1);
            }
        sp = siteVector;
        sp->first = IDX2STAT (0);
        sp->local = TRUE;
        sp->port = DefNiuPort + 1;
        sp->terms = 2;
        sp->siteName = "local";
        sp++;
        sp->first = IDX2STAT (2);
        sp->local = FALSE;
        sp->port = DefNiuPort;
        sp->terms = 4;
        sp->siteName = "remote";
        niuStations = 6;
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

    niuStationEnd = IDX2STAT (niuStations);
    portVector = calloc(1, sizeof(PortParam) * niuStations);
    if (portVector == NULL)
        {
        fprintf(stderr, "Failed to allocate NIU context block\n");
        exit(1);
        }

    mp = portVector;
    
    for (i = 0; i < sites; i++)
        {
        sp = siteVector + i;

        /*
        **  Create the threads which will deal with TCP connections.
        */
        sp->niuPorts.portNum = sp->port;
        sp->niuPorts.maxPorts = sp->terms;
        sp->niuPorts.localOnly = sp->local;
        sp->niuPorts.callBack = niuWelcome;
        sp->niuPorts.callArg = sp;
        sp->niuPorts.kind = sp->siteName;
        sp->niuPorts.ringSize = NetBufSize;
        sp->niuPorts.sendRingSize = SendBufSize;
        dtInitPortset (&(sp->niuPorts));
        /*
        **  Allocate the operator status buffer
        */
        if (sp->statusBuf == NULL)
        {
            sp->statusBuf = opInitStatus ("NIU", sp->first, sp->terms);
            niuUpdateStatus (sp);
        }

        /*
        **  Initialize some pointers in the PortParam blocks
        */
        for (j = 0; j < sp->terms; j++)
            {
            mp->sp = sp;
            mp->np = NULL;
            mp++;
            }
        }
    
    currInPort = -1;
    lastInPort = 0;        /* Start first scan at 0-1 */
    
    /*
    **  Start the operator alert box thread, if needed.
    **
    **  Note that this is currently only supported on Unix-like
    **  platforms because of the need for fork and exec.
    */
#if !defined(_WIN32)
    if (niuOpstat > 0)
        {
        if (dtCreateThread (niuThread, NULL, NULL))
            {
            exit (1);
            }
        }
#endif

    /*
    **  Print a friendly message.
    */
    printf("NIU initialised with input channel %o and output channel %o\n", unitNo, channelNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Return connected IP address for a station
**
**  Parameters:     Name        Description.
**                  stat        station number
**
**  Returns:        IP address, 0 if no connection, -1 if error.
**
**------------------------------------------------------------------------*/
CpWord niuConn (u32 stat)
    {
    PortParam *mp;
    NetFet *fet;

    stat = STAT2IDX (stat);
    if (stat >= niuStations)
        {
        return MINUS1;
        }
    mp = portVector + stat;
    fet = mp->np;
    if (!dtActive (fet))
        {
        return 0;
        }
    return ntohl (fet->from.s_addr);
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
#if !defined(_WIN32)    /* only non-Windows for now */
static const char *alert_msg[] = { "up", "down", "crash", "operator call" };
void niuDoAlert (int code)
    {
    const char *msg;
    FILE *msgf;
    u8 pp;
    
    msg = alert_msg[code];
    msgf = fopen (OPERBOX_MSGFILE, "wt");
    if (msgf == NULL)
        {
        logError (LogErrorLocation, "can't open alert message file");
        return;
        }
    fprintf (msgf, "Subject: PLATO %s\n", msg);
    fprintf (msgf, "PLATO %s at %s\n\n", msg, dtNowString ());
    fprintf (msgf, "niuLastAlertCount %d, niuAlertCount %d\n", 
             niuLastAlertCount, niuAlertCount);
    
    if (code == OPERBOX_CRASH)
        {
        /*
        **  Reset the status so next operbox keepalive is seen as "up".
        */
        niuAlertCount = 0;
        /*
        **  Get some debug data.
        */
        dumpCpuInfo (msgf);
        dumpCpuMem (msgf, 0, 010000, 0);
        for (pp = 0; pp < ppuCount; pp++)
            {
            fprintf (msgf, "\nPPU %o:\n", pp);
            dumpPpuInfo (msgf, pp);
            }
        }
    fclose (msgf);

    if (fork () == 0)
        {
		setsid ();
        execlp (OPERBOX_EXEC, OPERBOX_EXEC, msg, OPERBOX_MSGFILE, NULL);
        
        /*
        **  Just exit if exec didn't work.
        */
        _exit (0);
        }
    }
#endif  /* !WIN32 */

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
        for (i = 0; i < niuStations; i++)
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
    NetFet *np;
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
            if (++port == niuStations)
                {
                    port = 0;
                }
            mp = portVector + port;
            np = mp->np;
            if (!dtActive (np))
                {
                continue;
                }
            
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
            if (!dtActive (np))
                {
                if (port == lastInPort)
                    {
                    return;                     /* no input anywhere */
                    }
                continue;
                }
            i = dtReadw (np, mp->currInput, 2);
            if (i < 0)
                {
                if (!dtConnected (np))
                    {
                    dtClose (np, TRUE);
                    }
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
                printf ("continuing to force logout station %d\n", 
                        IDX2STAT (port));
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
        activeChannel->data = 04000 + (IDX2STAT (currInPort));
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
        for (port = 0; port < niuStations; port++)
            {
            mp = portVector + port;
            if (mp->currOutput != 0)
                {
                niuSendWord (IDX2STAT (port), mp->currOutput);
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
**                  sp          Pointer to SiteParam for this port
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void niuUpdateStatus (SiteParam *sp)
    {
    char msg[64];
    
    if (sp->niuPorts.curPorts == 0)
        {
        sprintf (msg, "site %s", sp->siteName);
        }
    else
        {
        sprintf (msg, "site %s, %d connection%s",
                 sp->siteName, sp->niuPorts.curPorts,
                 (sp->niuPorts.curPorts != 1) ? "s" : "");
        }
    opSetStatus (sp->statusBuf, msg);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a welcome message to a station
**
**  Parameters:     Name        Description.
**                  np          NetFet pointer
**                  stat        station number
**                  arg         generic argument: the SiteParam pointer
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuWelcome(NetFet *np, int stat, void *arg)
    {
    PortParam *mp;
    SiteParam *sp = (SiteParam *) arg;
    
    stat += sp->first;
    mp = portVector + STAT2IDX (stat);
    
    if (!dtConnected (np))
        {
        /*
        **  Connection was dropped.
        */
        mp->np = NULL;
        printf("%s niu: Connection dropped from %s for station %d-%d\n",
               dtNowString (), inet_ntoa (np->from), 
               stat / 32, stat % 32);
        niuActiveConns--;
        niuUpdateStatus (sp);
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
    mp->np = np;
    printf("%s niu: Received connection from %s for station %d-%d\n",
           dtNowString (), inet_ntoa (np->from),
           stat / 32, stat % 32);

    np->ownerInfo = stat;
    niuActiveConns++;
    niuUpdateStatus (sp);

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
        printf ("need to force logout for station %d\n", stat);
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

    stat = STAT2IDX (stat);
    if (stat < 0 || stat >= niuStations)
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
    int idx;
    PortParam *mp;
    NetFet *fet;
    u8 data[3];

    idx = STAT2IDX (stat);
#ifdef TRACE
    if (idx >= 0 && idx < niuStations)
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
    
    mp = portVector + idx;
    fet = mp->np;
    if (!dtActive (fet))
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
    dtSend (fet, data, 3);
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
    u32 newFlags;
    
    /*
    **  This is where we implement the special handling of the operator
    **  station.  We emulate the operator's alert box.  That reacts
    **  to -ext- command words.
    **  The specific command format can be seen by reading lesson "stats1".
    **  The ext codes are:
    **  ext 0170000  keepalive
    **  ext 0130000  shutdown
    **  keepalive ("resett") comes with two modifier flags:
    **  op.call  04000  activate operator call
    **  mode3    02000  "mode 3 lights ok" -- apparently this is some
    **                  sort of status warning light
    **  Keepalive enables the status watcher, and resets the timeout.  If
    **  no keepalives are then seen for the timeout period, we issue an alert.
    **  Shutdown turns off the status watcher; this happens (in lesson stats1)
    **  in a controlled PLATO shutdown.
    */
    if ((word & resett_m) == ext + resett)
        {
        if (niuAlertCount == 0)
            {
            /*
            **  First keepalive message since startup or since it was shut off,
            **  so report a startup.
            */
            niuDoAlert (OPERBOX_UP);
            }
        niuAlertCount++;
        newFlags = (~niuLastAlertFlags) & word & (~resett_m);
        if (newFlags & op_call)
            {
            /*
            **  Operator call bit changed to "on", send alert.
            */
            niuDoAlert (OPERBOX_OPCALL);
            }
        if ((word ^ niuLastAlertFlags) & mode3)
            {
            /*
            **  Mode 3 (operbox light) changed.  Update operator status.
            */
            opPlatoAlert = (word & mode3) != 0;
            opUpdateSysStatus ();
            }
        niuLastAlertFlags = word & (~resett_m);
        }
    else if (word == ext + resetb)
        {
        /*
        **  Report a shutdown
        */
        niuDoAlert (OPERBOX_DOWN);
        niuAlertCount = 0;
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
static dtThreadFun (niuThread, param)
    {
    int status;
    
    while (emulationActive)
        {
        niuCheckAlert ();
        /* Clean up any child processes that have exited.  */
        if (waitpid (-1, &status, WNOHANG) > 0)
            printf ("child exited\n");
        
        sleep (ALERT_TIMEOUT);
        }

    /*
    **  If the alert wasn't shut off, raise the alarm now as part
    **  of DtCyber termination.
    */
    if (niuAlertCount != 0)
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
    /*
    **  Raise the alarm if it is enabled, and no progress has 
    **  been seen since last time.  
    */
    if (niuAlertCount != 0)
        {
        if (niuAlertCount == niuLastAlertCount)
            {
            /* Only raise the alarm once */
            niuDoAlert (OPERBOX_CRASH);
            niuAlertCount = 0;
            }
        else
            {
            niuLastAlertCount = niuAlertCount;
            }
        }
    }

#endif  /* !WIN32 */

/*---------------------------  End Of File  ------------------------------*/
