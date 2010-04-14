/*--------------------------------------------------------------------------
**
**  Copyright (c) 2009-2010, Paul Koning (see license.txt)
**
**  Name: pni.c
**
**  Description:
**      Simulation of the PLATO "PNI" subsystem.
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
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#endif
#include <errno.h>
#include "plato.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define RETOK       MINUS1
#define RETINVREQ   0
#define RETNOSOCK   1
#define RETNODATA   2
#define RETLONG     3
#define RETNULL     4
#define RETERROR(x) (1000 + x)
#define RETERRNO    RETERROR(errno)

#define MAXNET      4096
#define SIMNAM      0       /* Which NAM site is the simulated one */

#define FLOW_XOFF   1       /* XOFF from terminal */
#define FLOW_TCP    2       /* dtSend backpressure */
#define FLOW_DOPTS  4       /* Need to send PTS to Framat  */
#define FLOW_DOABT  8       /* Need to send ABT to Framat  */

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define STAT2IDX(s) ((s) - pniFirstStation)
#define IDX2STAT(i) ((i) + pniFirstStation)
#ifdef DEBUG
#define DEBUGPRINT(fmt, ...) printf ("%s " fmt, dtNowString (), ## __VA_ARGS__ )
#else
#define DEBUGPRINT(fmt, ...)
#endif

#define WELCOME_MSG \
    "\033\002"      /* Enter PLATO mode */                              \
    "\033\014"  /* Full screen erase */                                 \
    "\033\024"  /* Mode rewrite */                                      \
    "\037"      /* Mode 3 (text plotting) */                            \
    "\033\062\044\140\044\100" /* Load X and Y 128 */                   \
    "Press  NEXT  to begin"
#define NOPLATO_MSG \
    "\033\002"      /* Enter PLATO mode */                              \
    "\033\014"  /* Full screen erase */                                 \
    "\033\024"  /* Mode rewrite */                                      \
    "\037"      /* Mode 3 (text plotting) */                            \
    "\033\062\044\140\044\100" /* Load X and Y 128 */                   \
    "PLATO not active"
#define OFF_MSG \
    "\033\002"      /* Enter PLATO mode */                              \
    "\033a\100\174\177\177" /* Foreground color yellow */               \
    "\033b\100\100\100\100" /* Background color black */                \
    "\033\024"  /* Mode rewrite */                                      \
    "\037"      /* Mode 3 (text plotting) */                            \
    "\033\062\056\140\044\110" /* Load Y 448, X 136 */                  \
    "                              "                                    \
    "\033\062\055\160\044\110" /* Load Y 432, X 136 */                  \
    "          plato  off          "                                    \
    "\033\062\055\140\044\110" /* Load Y 416, X 136 */                  \
    "                              "                                    \
    "\035"      /* Mode 1 (line drawing) */                             \
    "\055\140\053\130" /* Y 416, X 376 */                               \
    "\056\160\053\130" /* Y 464, X 376 */                               \
    "\056\160\044\110" /* Y 464, X 136 */                               \
    "\055\140\044\110" /* Y 416, X 136 */                               \
    "\055\140\053\130" /* Y 416, X 376 */                               \
    
/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
enum EscState { norm, esc, key3, xlow, xhigh, ylow, yhigh };

typedef struct siteParam
{
    u32     port;
    int     terms;
    int     first;
    const char *siteName;
    void    *statusBuf;
    bool    local;
    NetPortSet pniPorts;
} SiteParam;

typedef struct portParam
{
    NetFet      *np;
    SiteParam   *sp;
    enum EscState escState;
    int         keyAsm;
    int         flowFlags;
    bool        newKbd;
    bool        sendLogout;
    bool        forceLogout;
} PortParam;


/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static int rcb (CpWord *buf, struct Io *iop, int buflen, CpWord *dest);
static void storeKey (int key, int station);
static void storeChar (int c, char **pp);
static bool framatReq (int req, int station);
static void pniWelcome (NetFet *np, int stat, void *arg);
static bool pniSendstr (int stat, const char *p, int len);
static void pniUpdateStatus (SiteParam *sp);
static bool pniLoggedIn (int stat);
static struct stbank * pniStationBank (int stat);
static void pniActivateStation (int stat);

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
static struct PniPtr apni;
static bool pniActive = FALSE;
static CpWord *akb;
static struct stbank *ast;
static CpWord *aasccon;
static CpWord *abt;
static CpWord *afpnib;
static CpWord *afpniio;
static int fpnilen;
static CpWord *apnifb;
static CpWord *apnifio;
static int pniflen;
static int pniFirstStation;
static int pniStations;        // Count of stations actually enabled by PLATO
static int lastStation;
static CpWord *netbuf;
static char *ascbuf;
static PortParam *portVector;
static int lastInPort;
static int pniActiveConns;
static int sites;
static SiteParam *siteVector;

static struct Io fpniio;
static struct Io pnifio;

// Original ASCII to PLATO key translation table
const int asc2plato[128] = {
0074, 0032, 0030, 0033, 0021, 0061,   -1, 0022, 
0023, 0065, 0014, 0025, 0035, 0026, 0070, 0075, 
  -1, 0072, 0031, 0020, 0062,   -1, 0073, 0060, 
0067, 0063, 0027,   -1, 0054, 0071, 0066, 0140, 
0100, 0176, 0177, 0056, 0044, 0045, 0012, 0053, 
0051, 0173, 0050, 0016, 0137, 0017, 0136, 0135, 
0000, 0001, 0002, 0003, 0004, 0005, 0006, 0007, 
0010, 0011, 0174, 0134, 0040, 0133, 0041, 0175, 
0052, 0141, 0142, 0143, 0144, 0145, 0146, 0147, 
0150, 0151, 0152, 0153, 0154, 0155, 0156, 0157, 
0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167, 
0170, 0171, 0172, 0042, 0055, 0043, 0015, 0046, 
0013, 0101, 0102, 0103, 0104, 0105, 0106, 0107, 
0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117, 
0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127, 
0130, 0131, 0132, 0024, 0047, 0034, 0057, 0064, 
};

// New ASCII to PLATO key translation table, for XON/XOFF support
// Note that XON and XOFF are handled separately and translate
// to "not used" (-1) here.
const int asc2plato2[128] = {
  -1, 0032, 0030, 0033, 0021, 0072,   -1, 0022, 
0023, 0014, 0065, 0025, 0035, 0026, 0070, 0075, 
  -1,   -1, 0031,   -1, 0062,   -1, 0073, 0020, 
0067, 0063, 0027,   -1, 0054, 0071, 0066, 0140, 
0100, 0176, 0177, 0056, 0044, 0045, 0012, 0047, 
0051, 0173, 0050, 0016, 0137, 0017, 0136, 0135, 
0000, 0001, 0002, 0003, 0004, 0005, 0006, 0007, 
0010, 0011, 0174, 0134, 0040, 0133, 0041, 0175, 
0052, 0141, 0142, 0143, 0144, 0145, 0146, 0147, 
0150, 0151, 0152, 0153, 0154, 0155, 0156, 0157, 
0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167, 
0170, 0171, 0172, 0042, 0055, 0043, 0015, 0046, 
0013, 0101, 0102, 0103, 0104, 0105, 0106, 0107, 
0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117, 
0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127, 
0130, 0131, 0132, 0024, 0053, 0034, 0057, 0064, 
};

/*
**--------------------------------------------------------------------------
**
**  Inline Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Store an ASCII character into the outbound network buffer
**
**  Parameters:     Name        Description.
**                  c           Character code
**                  pp          Pointer to character buffer
**
**  Returns:        nothing, but the buffer pointer is updated.
**
**  This function basically just stores what it is give, but if the
**  byte is 0xff (Telnet escape) it escapes it.
**
**------------------------------------------------------------------------*/
static inline void storeChar (int c, char **pp)
{
    char *p = *pp;
    
    *p = c;
    if (c == 0xff)
    {
        // Telnet escape
        *++p = c;
    }
    *pp = ++p;
}

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialize PNI
**
**  Parameters:     Name        Description.
**
**  Returns:        nothing
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
**
**  For PNI, the "plato" section is required.
**
**------------------------------------------------------------------------*/
void initPni (void)
{
    SiteParam *sp;
    long sectionPos;
    char *line;
    char *token;
    char *sitename;
    int i, j, terms, port;
    bool local;
    PortParam *pp;
    
    if (platoSection[0] == '\0')
    {
        return;
    }
    
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
        if (strncmp (line, "pni=,", 4) == 0)
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
        fprintf (stderr, "Failure allocating PNI site data vector\n");
        exit (1);
    }
    initRestorePosition (sectionPos);
    sp = siteVector;
    pniStations = 0;
    /*
    **  First PNI station is after last NIU station, rounded up to
    **  physical site boundary.
    */
    pniFirstStation = (niuStationEnd + 31) & (~31);
    while  ((line = initGetNextLine()) != NULL)
    {
        if (strncmp (line, "pni=,", 4) != 0)
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
        sp->first = IDX2STAT (pniStations);
        sp->port = port;
        sp->terms = terms;
        sp->local = local;
        sp->siteName = sitename;
        sp++;
        pniStations += terms;
    }

    portVector = pp = calloc(1, sizeof(PortParam) * pniStations);
    if (portVector == NULL)
    {
        fprintf(stderr, "Failed to allocate PNI context block\n");
        exit (1);
    }

    for (i = 0; i < sites; i++)
    {
        sp = siteVector + i;

        /*
        **  Create the threads which will deal with TCP connections.
        */
        sp->pniPorts.portNum = sp->port;
        sp->pniPorts.maxPorts = sp->terms;
        sp->pniPorts.localOnly = sp->local;
        sp->pniPorts.callBack = pniWelcome;
        sp->pniPorts.callArg = sp;
        sp->pniPorts.kind = sp->siteName;
        dtInitPortset (&(sp->pniPorts), MAXNET);
        /*
        **  Allocate the operator status buffer
        */
        if (sp->statusBuf == NULL)
        {
            sp->statusBuf = opInitStatus ("PNI", sp->first, sp->terms);
            pniUpdateStatus (sp);
        }
        /*
        **  Initialize some pointers in the PortParam blocks
        */
        for (j = 0; j < sp->terms; j++)
        {
            pp->sp = sp;
            pp->np = sp->pniPorts.portVec + j;
            pp++;
        }
    }
    
    /*
    **  Print a friendly message.
    */
    printf("PNI initialised, %d stations\n", pniStations);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process a PNI operation (external op function 3)
**
**  Parameters:     Name        Description.
**                  req         Request word
**
**  Returns:        Status (-1 for ok, other values for errors)
**
**  This turns PNI emulation on or off.
** 
**  Request format:
**       48/0, 12/3
**       60/PNI initialization pointer
** 
**  The PNI initialization pointer is the ECS address of the PNI 
**  pointers block, or 0 to turn off PNI emulation.  If -0 is 
**  supplied, PNI is turned off but a "plato off" message is transmitted
**  to all connected stations.
**
**------------------------------------------------------------------------*/
CpWord pniOp (CpWord req)
{
    CpWord *reqp = cpuAccessMem (req, 2);
    CpWord *pnii;
    int i, stat, len;
    PortParam *pp;
    int stations;
    
    DEBUGPRINT ("PniOp, request is %020llo\n", reqp[1]);
    if (reqp[1] == 0 || reqp[1] == Mask60)
    {
        if (pniActive)
        {
            if (reqp[1] != 0)
            {
                // Off with message
                for (i = 0; i < pniStations; i++)
                {
                    pp = portVector + i;
                    if (pp->np != NULL)
                    {
                        DEBUGPRINT ("Sending offmsg to station %d\n", IDX2STAT (i));
                        pniSendstr (IDX2STAT (i), OFF_MSG, 0);
                    }
                }
            }
            pniActive = FALSE;
            if (netbuf != NULL)
            {
                free (netbuf);
                free (ascbuf);
                ascbuf = NULL;
                netbuf = NULL;
            }
            printf("PNI turned off\n");
        }
        return RETOK;
    }
    
    if (pniActive)
    {
        return RETINVREQ;
    }
    
    pnii = cpuAccessMem ((1ULL << 59) | reqp[1], 
                         sizeof (struct PniPtr) / sizeof (CpWord));
    if (pnii == NULL)
    {
        return RETINVREQ;
    }
    memcpy (&apni, pnii, sizeof (apni));

    // Convert ECS addresses to pointers, and extract fields
    pniFirstStation = ((apni.sd[SIMNAM].site >> 12) & Mask12) << 5;
    stations = (apni.sd[SIMNAM].site & Mask12) << 5;
    DEBUGPRINT ("pni on, first station %d, count %d\n", pniFirstStation, stations);
    if (stations <= 0 || pniStations == 0 || pniFirstStation < niuStationEnd)
    {
        return RETINVREQ;
    }
    if (stations > pniStations)
    {
        fprintf (stderr, "Warning: PNI On with more stations than configured: %d %d\n", stations, pniStations);
        stations = pniStations;
    }
    lastStation = pniFirstStation + stations;
    akb = cpuAccessMem ((1ULL << 59) | apni.akb, lastStation * NKEYLTH);
    ast = (struct stbank *) cpuAccessMem ((1ULL << 59) | apni.ast, 
                                          lastStation * sizeof (struct stbank) / sizeof (CpWord));
    aasccon = cpuAccessMem ((1ULL << 59) | apni.aasccon, lastStation);
    abt = cpuAccessMem ((1ULL << 59) | apni.abt, stations >> 5);  // one word per site
    fpnilen = (apni.sd[SIMNAM].fod >> 42) & Mask18;
    afpnib = cpuAccessMem ((1ULL << 59) | ((apni.sd[SIMNAM].fod >> 21) & Mask21), fpnilen);
    afpniio = cpuAccessMem ((1ULL << 59) | (apni.sd[SIMNAM].fod & Mask21), 2);
    pniflen = (apni.sd[SIMNAM].frb >> 42) & Mask18;
    apnifb = cpuAccessMem ((1ULL << 59) | ((apni.sd[SIMNAM].frb >> 21) & Mask21), pniflen);
    apnifio = cpuAccessMem ((1ULL << 59) | (apni.sd[SIMNAM].frb & Mask21), 2);
    if (akb == NULL || aasccon == NULL || abt == NULL ||
        afpnib == NULL || afpniio == NULL ||
        apnifb == NULL || apnifio == NULL)
    {
        return RETINVREQ;
    }
    len = pniflen;
    if (len < fpnilen)
    {
        len = fpnilen;
    }
    netbuf = (CpWord *) malloc (len * sizeof (CpWord));
    if (netbuf == NULL)
    {
        return RETERROR (ENOMEM);
    }
    ascbuf = (char *) malloc (len * 16);
    if (ascbuf == NULL)
    {
        free (netbuf);
        netbuf = NULL;
        return RETERROR (ENOMEM);
    }

    lastInPort = 0;
    pniActive = TRUE;

    // Send "Press NEXT to begin" to any connected terminals
    // and set their termid word because this is a new
    // PLATO load.
    for (stat = 0; stat < stations; stat++)
    {
        pp = portVector + stat;
        if (pp->np != NULL)
        {
            pniActivateStation (stat);
        }
    }

    /*
    **  Print a friendly message.
    */
    printf("PNI started, first station %d-%d, %d stations\n",
           pniFirstStation >> 5, pniFirstStation & 0x1f, stations);
    return RETOK;
}

/*--------------------------------------------------------------------------
**  Purpose:        Check for PNI things to do
**
**  Parameters:     None
**
**  Returns:        Nothing
**
**  This function does the periodic PNI things: check for input,
**  check for output, check to see if requests need to be sent to Framat.
**  If PNI is not active, we still check for input (discarding all of it).
**
**------------------------------------------------------------------------*/
void pniCheck (void)
{
    int len, i, j, snum;
    bool lastword;
    int station;
    int opcode;
    CpWord oldout, hdr;
    char *p;
    CpWord *wp, w;
    int shift;
    PortParam *pp;
    NetFet *np;
    int port, key;
    char buf[2];
    struct stbank *sb;
    NetPortSet *psp;
    SiteParam *sp;

    /*
    **  Scan the active connections, round robin, looking
    **  for one that has pending input. 
    */
    port = lastInPort;
    for (;;)
    {
        if (++port == pniStations)
        {
            /*
            **  Whenever we finish a scan pass through the ports
            **  (i.e., we've wrapped around) look for more data.
            */
            port = 0;
            for (snum = 0; snum < sites; snum++)
            {
                for (;;)
                {
                    sp = siteVector + snum;
                    psp = &(sp->pniPorts);
                    np = dtFindInput (psp, 0);
                    if (np == NULL)
                    {
                        break;
                    }
                    i = dtRead  (np, psp, -1);
                    if (i < 0)
                    {
                        dtClose (np, psp, TRUE);
                    }
                }
            }
        }

        pp = portVector + port;
        np = pp->np;
            
        if (pp->sendLogout)
        {
            /*
            **  If we disconnected without logging out, send
            **  *offky2* which tells PLATO to log out this
            **  station.
            */
            storeKey (OFFKY2, IDX2STAT (port));
            pp->sendLogout = FALSE;
            continue;
        }
        if (np == NULL || !dtActive (np))
        {
            if (port == lastInPort)
            {
                break;                      /* no input anywhere */
            }
            continue;
        }
        for (;;)
        {
            /*
            ** We'll try to process a whole escape sequence, if
            ** it's available, but if not we'll go on to another port.
            */
            i = dtReado (np);               /* Get a byte */
            if (i < 0)
            {
                key = -1;
                break;                      /* we don't have enough data yet */
            }
            key = i & Mask7;
            DEBUGPRINT ("pni input, char code %03o, state %d\n", key, (int) (pp->escState));
            switch (pp->escState)
            {
            case norm:
                if (key == 033)
                {
                    pp->escState = esc;
                    continue;
                }
                else if (pp->newKbd)
                {
                    if (key == 0x11)
                    {
                        // XON
                        pp->flowFlags &= ~FLOW_XOFF;
                        if (pp->flowFlags == 0)
                        {
                            pp->flowFlags = FLOW_DOPTS;
                        }
                    }
                    else if (key == 0x13)
                    {
                        // XOFF
                        pp->flowFlags |= FLOW_XOFF;
                    }
                    key = asc2plato2[key];
                }
                else
                {
                    key = asc2plato[key];
                }
                break;
            case esc:
                // Escape sequence.  There are only a few possibilities.
                switch (key)
                {
                case 4: 
                    key = 0x31;     // sub1
                    break;
                case 0x17:
                    key = 0x30;     // super1
                    break;
                case 0x1d:
                    key = 0x3c;     // square1
                    break;
                case 0x1f:
                    pp->escState = xlow;
                    continue;       // fine grid touch
                default:
                    if (key >= 0100)
                    {
                        pp->escState = key3;
                        pp->keyAsm = key & 077;
                        continue;
                    }
                    key = -1;
                    break;
                }
                pp->escState = norm;
                break;
            case key3:
                if (key >= 0100)
                {
                    key = ((key & 077) << 6) + pp->keyAsm;
                }
                else
                {
                    key = -1;
                }
                pp->escState = norm;
                break;
            case xlow:
                if (key >= 0100)
                {
                    pp->keyAsm = (key & 037) << 10;
                    pp->escState = xhigh;
                }
                else
                {
                    key = -1;
                    pp->escState = norm;
                }
                break;
            case xhigh:
                if (key >= 0100)
                {
                    pp->keyAsm |= (key & 037) << 15;
                    pp->escState = ylow;
                }
                else
                {
                    key = -1;
                    pp->escState = norm;
                }
                break;
            case ylow:
                if (key >= 0100)
                {
                    pp->keyAsm |= (key & 037);
                    pp->escState = yhigh;
                }
                else
                {
                    key = -1;
                    pp->escState = norm;
                }
                break;
            case yhigh:
                if (key >= 0100)
                {
                    // Form the coordinate data
                    key = pp->keyAsm | ((key & 037) << 5);
                    sb = pniStationBank (IDX2STAT (port));
                    if (sb != NULL)
                    {
                        // Merge the fine grid position data
                        sb->cwsinfo = (sb->cwsinfo & ~ULL (03777777)) | key;
                    }
                }
                key = -1;
                pp->escState = norm;
                break;
            }
            // If we're done with the esc sequence, done with this port
            if (pp->escState == norm)
            {
                break;
            }
        }
        if (key >= 0 && pp->escState == norm)
        {
            if (pp->forceLogout)
            {
                /*
                **  If we're not logged out yet, send *offky2* which
                **  tells PLATO to log out this station instead of
                **  the key that was typed
                */
                if (pniLoggedIn (IDX2STAT (port)))
                {
                    key = OFFKY2;
                    printf ("continuing to force logout port %d\n", IDX2STAT (port));
                }
                else
                {
                    pp->forceLogout = FALSE;
                }
            }
            DEBUGPRINT ("pni key %04o\n", key);
            storeKey (key, IDX2STAT (port));
            pp->keyAsm = 0;
        }
        if (port == lastInPort)
        {
            break;                      /* no input anywhere */
        }
    }
    lastInPort = port;

    if (!pniActive)
    {
        return;
    }

    /*
    **  Look for requests from Framat
    */
    memcpy (&fpniio, afpniio, sizeof (fpniio));
    oldout = fpniio.out;
    while ((len = rcb (afpnib, &fpniio, fpnilen, netbuf)) != 0)
    {
        //  Message header format
        //
        //  12/Function code
        //         0 - Data message
        //         1 - Output abort
        //         2 - Logout terminal
        //         3 - Return terminal type via echo key
        //         4 - Enable flow control and new keycode translation
        //         5 - Logout terminal immediately
        //  12/Message sequence number
        //  12/CPU word count of message (including header)
        //  12/PPU word count of message (including header)
        //     For data messages this is instead the byte count + 1
        //  12/Station number
        hdr = netbuf[0];
        station = hdr & Mask12;
        opcode = (hdr >> 48) & Mask12;
        DEBUGPRINT ("Message from framat, len %d, %llo, ptrs %llo %llo -> %llo\n", len, hdr, fpniio.in, oldout, fpniio.out);
        oldout = fpniio.out;

        if (station < pniFirstStation || station >= lastStation)
        {
            DEBUGPRINT ("Station %d out of range\n", station);
            continue;
        }
        pp = portVector + STAT2IDX (station);
        switch (opcode)
        {
        case 0:
            // Data message, send it to the terminal
            p = ascbuf;
            wp = netbuf + 1;
            shift = -8;
            len = ((hdr >> 12) & Mask12) - 1;
            for (i = 0; i < len; i++)
            {
                if (shift == -4)
                {
                    j = (w << 4) & 0x70;
                    w = *wp++;
                    DEBUGPRINT (" %015llx\n", w);
                    j |= (w >> 56) & Mask4;
                    shift = 48;
                }
                else
                {
                    if (shift == -8)
                    {
                        // Start a new CM word
                        w = *wp++;
                        shift = 52;
                        DEBUGPRINT (" %015llx\n", w);
                    }
                    j = (w >> shift) & Mask8;
                    shift -= 8;
                }
                storeChar (j, &p);
            }
            len = p - ascbuf;
            if (len > 0)
            {
                DEBUGPRINT ("Send %d bytes to terminal %d\n", len, station);
                if (!pniSendstr (station, ascbuf, len))
                {
                    pp->flowFlags |= FLOW_TCP;
                    DEBUGPRINT ("Flow set to off\n");
                }
            }
            // Send Permit to Send at next opportunity
            pp->flowFlags |= FLOW_DOPTS;
            break;
        case 1:
            // Abort output
            DEBUGPRINT ("Abort output, terminal %d\n", station);
            // Send Abort done
            pp->flowFlags |= FLOW_DOABT;
            break;
        case 2:
        case 5:
            // Logout terminal
            DEBUGPRINT ("Log out terminal %d\n", station);
            // Actually we do nothing here
            break;
        case 3:
            // Return terminal type
            DEBUGPRINT ("Return terminal type, station %d\n", station);
            // Tell PLATO that we have an ASCII terminal
            storeKey (0214, station);
            break;
        case 4:
            // Set new keycode conversion
            DEBUGPRINT ("Set new keycode conversion/flow control, station %d\n", station);
            pp->newKbd = TRUE;
            // Send "new kb" echo code to terminal?
            break;
        default:
            DEBUGPRINT ("Invalid framat request code %04o, station %d\n", opcode, station);
        }
    }
    /* 
    **  Write back the "out" pointer
    */
    afpniio[1] = fpniio.out;

    /*
    **  Send any necessary requests to Framat
    */
    for (port = 0; port < pniStations; port++)
    {
        pp = portVector + port;
        np = pp->np;
        if (pp->flowFlags != 0 && np != NULL)
        {
            // Some flags are set, figure out what to do
            if ((pp->flowFlags & FLOW_DOABT) != 0)
            {
                // Need to send ABT
                if (framatReq (F_ABT, IDX2STAT (port)))
                {
                    // Sent successfully, clear flag
                    pp->flowFlags &= ~FLOW_DOABT;
                }
            }
            if ((pp->flowFlags & FLOW_TCP) != 0 && np->sendCount == 0)
            {
                // Flow was off due to TCP and it's ready now
                pp->flowFlags &= ~FLOW_TCP;
            }
            if (pp->flowFlags == FLOW_DOPTS)
            {
                // Need to send PTS, and no flow-off flags are set
                if (framatReq (F_PTS, IDX2STAT (port)))
                {
                    // Sent successfully, clear flag
                    pp->flowFlags = 0;
                }
            }
        }
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Return connected IP address for a port
**
**  Parameters:     Name        Description.
**                  stat        Port number
**
**  Returns:        IP address, 0 if no connection, -1 if error.
**
**------------------------------------------------------------------------*/
CpWord pniConn (u32 stat)
{
    PortParam *mp;
    NetFet *fet;
    if (stat >= pniStations)
    {
        return MINUS1;
    }
    mp = portVector + STAT2IDX (stat);
    fet = mp->np;
    if (fet->connFd == 0)
    {
        return 0;
    }
    return ntohl (fet->from.s_addr);
}

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Read from the Framat to PNI request ring
**
**  Parameters:     Name        Description.
**                  buf         Buffer pointer
**                  iop         In/out pointers address
**                  buflen      Length of buffer
**                  dest        Where to write the data
**
**  Returns:        Count of words read (0 if nothing pending)
**
**  This function reads the entire block of pending data in the
**  Framat to PNI request ring.  Note that the dest buffer must
**  be big enough, i.e., it has to be sized to at least the ring
**  buffer size.  The "out" pointer is updated.
**
**------------------------------------------------------------------------*/
static int rcb (CpWord *buf, struct Io *iop, int buflen, CpWord *dest)
{
    int len, len2;
    CpWord hdr;
    
    if (iop->in == iop->out)
    {
        return 0;
    }
    hdr = *dest = buf[iop->out];

    //  MESSAGE HEADER FORMAT
    //
    //  12/FUNCTION CODE
    //         0 - DATA MESSAGE
    //         1 - OUTPUT ABORT
    //         2 - LOGOUT TERMINAL
    //         3 - RETURN TERMINAL TYPE VIA ECHO KEY
    //  12/MESSAGE SEQUENCE NUMBER
    //  12/CPU WORD COUNT OF MESSAGE (INCLUDING HEADER)
    //  12/PPU WORD COUNT OF MESSAGE (INCLUDING HEADER)
    //  12/STATION NUMBER

    len = (hdr >> 24) & Mask12;
    DEBUGPRINT ("header %020llo\n", hdr);
    if (len > 1 && len < buflen)
    {        
        // message has data, read it from circular buffer
        if (iop->out + len <= buflen)
        {
            // no wrap
            memcpy (dest, buf + iop->out, len * sizeof (CpWord));
            if (iop->out + len < buflen)
            {
                iop->out += len;
            }
            else
            {
                iop->out = 0;
            }
        }
        else
        {
            len2 = buflen - iop->out;
            memcpy (dest, buf + iop->out, len2 * sizeof (CpWord));
            dest += len2;
            len2 = len - len2;
            memcpy (dest, buf, len2 * sizeof (CpWord));
            iop->out = len2;
        }
    }
    else if (++iop->out == buflen)
    {
        iop->out = 0;
    }

    return len;
}

/*--------------------------------------------------------------------------
**  Purpose:        Store a key (in PLATO coding) in the PLATO key buffer
**
**  Parameters:     Name        Description.
**                  key         Key code
**                  stat        Station number
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static void storeKey (int key, int station)
{
    struct Keybuf *kp;
    int ppidx, cpidx;
    int word, shift;
    
    if (!pniActive)
    {
        return;
    }
    if (station < pniFirstStation || station >= lastStation)
    {
        DEBUGPRINT ("storeKey: station out of range, %d\n");
        return;
    }
    kp = (struct Keybuf *) (akb + (station * NKEYLTH));
    /*
    **  The key buffer uses in/out pointers, but not in the usual
    **  way.  The in pointer is incremented before the store,
    **  rather than afterward.
    */
    ppidx = (kp->buf[CKPPIDX] & Mask12) + 1;
    cpidx = kp->cpidx & Mask12;
    if (ppidx == IDXLIM)
    {
        ppidx = 0;
    }
    if (cpidx == ppidx)
    {
        // Key buffer is full.  Check for special keys
        if (key == OFFKY2 || key == ((kp->cpidx >> 12) & Mask11))
        {
            // Overwrite the last stored key
            ppidx = kp->buf[CKPPIDX] & Mask12;
        }
        else
        {
            // Ordinary key and no room, ignore
            return;
        }
    }
    word = ppidx / 5;
    shift = 48 - (ppidx % 5) * 12;
    kp->buf[word] = (kp->buf[word] & ~((CpWord) Mask12 << shift)) | ((CpWord) (key & Mask11) << (shift + 1));
    DEBUGPRINT ("stored key: %020llo pp index %d cp index %d, offset %d, shift %d\n", kp->buf[word], ppidx, cpidx, word, shift);
    kp->buf[CKPPIDX] = (kp->buf[CKPPIDX] & ~Mask12) | ppidx;

    // Now that we've stored the key, set the bit in the 
    // site/station bitmap.  We don't need the readback
    // check on the bottom bit as is done elsewhere because
    // nothing else can get in the middle of this...
    abt[station >> 5] |= 1 + (0x1ULL << ((station & 0x1f) + 16));
    DEBUGPRINT ("Key bitmap: %020llo\n", abt[station >> 5]);
}

/*--------------------------------------------------------------------------
**  Purpose:        Send a PNI to Framat request word
**
**  Parameters:     Name        Description.
**                  req         Request code
**                  stat        Station number
**
**  Returns:        TRUE if the request was stored
**                  FALSE if request ring buffer was full
**
**------------------------------------------------------------------------*/
static bool framatReq (int req, int station)
{
    int newin;
    
    memcpy (&pnifio, apnifio, sizeof (pnifio));
    newin = pnifio.in + 1;
    if (newin == pniflen)
    {
        newin = 0;
    }
    if (newin == pnifio.out)
    {
        // pni -> framat buffer is full
        DEBUGPRINT ("PNI to Framat request %d, station %d, buffer is full\n", req, station);
        return FALSE;
    }
    *(apnifb + pnifio.in) = ((CpWord) req << 48) | station | 000100050000ULL;
    DEBUGPRINT ("PNI to Framat req %020llo, station %d at offset %d\n", *(apnifb + pnifio.in), station, pnifio.in);
    *apnifio = newin;

    return TRUE;
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
static void pniUpdateStatus (SiteParam *sp)
{
    char msg[64];
    
    if (sp->pniPorts.curPorts == 0)
    {
        sprintf (msg, "site %s", sp->siteName);
    }
    else
    {
        sprintf (msg, "site %s, %d connection%s",
                 sp->siteName, sp->pniPorts.curPorts,
                 (sp->pniPorts.curPorts != 1) ? "s" : "");
    }
    opSetStatus (sp->statusBuf, msg);
}

/*--------------------------------------------------------------------------
**  Purpose:        Send a welcome message to a station
**
**  Parameters:     Name        Description.
**                  np          NetFet pointer
**                  stat        station number relative to start of PNI
**                  arg         generic argument: the SiteParam pointer
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void pniWelcome(NetFet *np, int stat, void *arg)
{
    PortParam *mp;
    SiteParam *sp = (SiteParam *) arg;
    
    stat += sp->first;
    mp = portVector + STAT2IDX (stat);
    mp->np = np;
    
    if (np->connFd == 0)
        {
        /*
        **  Connection was dropped.
        */
        printf("%s pni: Connection dropped from %s for station %d-%d\n",
               dtNowString (), inet_ntoa (np->from), 
               stat / 32, stat % 32);
        pniActiveConns--;
        pniUpdateStatus (sp);
        if (pniActive && pniLoggedIn (stat))
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
    printf("%s pni: Received connection from %s for station %d-%d\n",
           dtNowString (), inet_ntoa (np->from),
           stat / 32, stat % 32);

    np->ownerInfo = stat;
    pniActiveConns++;
    pniUpdateStatus (sp);
    
    if (!pniActive)
    {
        pniSendstr (stat, NOPLATO_MSG, 0);
        return;
    }

    /*
    **  If we're not logged out yet (i.e. PLATO dropped the *offky2*)
    **  that was sent when the connection dropped on this port the
    **  last time it had a connection), set a flag to keep working on
    **  it.  With that flag set, we'll send another *offky2* in the
    **  key input handler for each key entered by the user, until
    **  PLATO indicates that the logout has been done.
    */
    if (pniLoggedIn (stat))
    {
        printf ("need to force logout for port %d\n", stat);
        mp->forceLogout = TRUE;
    }

    pniActivateStation (STAT2IDX (stat));
} 

/*--------------------------------------------------------------------------
**  Purpose:        Send a string to a station
**
**  Parameters:     Name        Description.
**                  stat        Station number
**                  p           pointer to ASCII string
**                  len         string length (0 to use strlen)
**
**  Returns:        TRUE if more output is allowed.
**
**------------------------------------------------------------------------*/
static bool pniSendstr(int stat, const char *p, int len)
{
    PortParam *mp;
    NetFet *fet;

    if (len == 0)
    {
        len = strlen (p);
    }
    mp = portVector + STAT2IDX (stat);
    fet = mp->np;
    return (dtSend (fet, &(mp->sp->pniPorts), p, len) < 0);
}

/*--------------------------------------------------------------------------
**  Purpose:        Test if a station is logged in
**
**  Parameters:     Name        Description.
**                  stat        Station number
**
**  Returns:        TRUE if logged in
**
**------------------------------------------------------------------------*/
static bool pniLoggedIn (int stat)
{
    struct stbank *sb = pniStationBank (stat);
    
    return (sb != NULL && (sb->bankadd & Sign60) == 0);
}

/*--------------------------------------------------------------------------
**  Purpose:        Return a station bank pointer
**
**  Parameters:     Name        Description.
**                  stat        Station number
**
**  Returns:        NULL if invalid station number or lookup failure.
**
**------------------------------------------------------------------------*/
static struct stbank * pniStationBank (int stat)
{
    struct stbank *sb;

    sb = ast + stat;
    
    // Check that we're on the right station bank
    if ((sb->stflags & Mask18) != stat)
    {
        DEBUGPRINT ("station bank mismatch, station %d\n", stat);
        sb = NULL;
    }

    return sb;
}

/*--------------------------------------------------------------------------
**  Purpose:        Activate a station
**
**  Parameters:     Name        Description.
**                  stat        station number relative to start of PNI
**
**  Returns:        nothing
**
**  This routine does the things we want to do when a new connection is made.
**  It is separate from the "welcome" routine because we also do this for
**  existing connections when PNI is activated by PLATO.
**
**------------------------------------------------------------------------*/
static void pniActivateStation (int stat)
{
    PortParam *mp;
    char termname[10];
    CpWord termid;
    int i;
    
    mp = portVector + stat;
    sprintf (termname, "te%02x   ", stat + 1);
    stat = IDX2STAT (stat);
    termid = 0;
    for (i = 0; i < 7; i++)
    {
        termid |= (CpWord) asciiToCdc[termname[i]] << (54 - 6 * i);
    }
    *(aasccon + stat) = termid;

    /*
    **  Indicate flow is on and abort needed. 
    */
    mp->flowFlags = FLOW_DOABT;
    
    pniSendstr (stat, WELCOME_MSG, 0);
}

