#define DEBUG 1
/*--------------------------------------------------------------------------
**
**  Copyright (c) 2009, Paul Koning (see license.txt)
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

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#ifdef DEBUG
#define DEBUGPRINT printf
#else
#define DEBUGPRINT(fmt, ...)
#endif

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
enum EscState { norm, esc, key3, xlow, xhigh, ylow, yhigh };

typedef struct portParam
{
    NetFet      *np;
    enum EscState escState;
    int         keyAsm;
    int         flowFlags;
    bool        newKbd;
    bool        loggedIn;
    bool        sendLogout;
    bool        forceLogout;
} PortParam;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static int rcb (CpWord *buf, CpWord *iop, int buflen, CpWord *dest);
static void storeKey (int key, int station);
static bool storeChar (int c, char **pp);
static bool framatReq (int req, int station);
static void pniWelcome(NetFet *np, int stat);
static bool pniSendstr(int stat, const char *p, int len);
static void pniUpdateStatus (void);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
long pniPort;
long pniConns;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static struct PniPtr apni;
static bool pniActive = FALSE;
static CpWord *akb;
static CpWord *aasccon;
static CpWord *abt;
static CpWord *afpnib;
static CpWord *afpniio;
static int fpnilen;
static CpWord *apnifb;
static CpWord *apnifio;
static int pniflen;
static int firstStation;
static int stations;
static int lastStation;
static CpWord *netbuf;
static char *ascbuf;
static PortParam *portVector;
static NetPortSet pniPorts;
static int lastInPort;
static void *statusBuf;
static int pniActiveConns;

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
const int asc2plato2[128] = {
  -1, 0032, 0030, 0033, 0021, 0072,   -1, 0022, 
0023, 0014, 0065, 0025, 0035, 0026, 0070, 0075, 
  -1,01606, 0031,01607, 0062,   -1, 0073, 0020, 
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
**  Public Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise PNI
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
// pniOp (external operation function code 3)
//
// This turns PNI emulation on or off.
//
// Request format:
//      48/0, 12/3
//      60/PNI initialization pointer
//
// The PNI initialization pointer is the ECS address of the PNI 
// pointers block, or 0 to turn off PNI emulation.

CpWord pniOp (CpWord req)
{
    CpWord *reqp = cpuAccessMem (req, 2);
    CpWord *pnii;
    int i, len;
    PortParam *pp;
    
    if (reqp[1] == 0)
    {
        if (pniActive)
        {
            pp = portVector;
            for (i = 0; i < stations; i++)
            {
                if (pp->np != NULL)
                {
                    dtClose (pp->np, &pniPorts, TRUE);
                }
                pp++;
            }
            pniActive = FALSE;
            if (netbuf != NULL)
            {
                free (netbuf);
                free (ascbuf);
                ascbuf = NULL;
                netbuf = NULL;
            }
            dtClosePortset (&pniPorts);
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
    firstStation = ((apni.sd[SIMNAM].site >> 12) & Mask12) << 5;
    stations = (apni.sd[SIMNAM].site & Mask12) << 5;
    DEBUGPRINT ("pni on, first station %d, count %d\n", firstStation, stations);
    if (stations <= 0 || pniConns == 0)
    {
        return RETINVREQ;
    }
    if (stations > pniConns)
    {
        fprintf (stderr, "Warning: PNI On with more stations than configured: %d %d\n", stations, pniConns);
        stations = pniConns;
    }
    lastStation = firstStation + stations;
    akb = cpuAccessMem ((1ULL << 59) | apni.akb, lastStation * NKEYLTH);
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

    portVector = calloc(1, sizeof(PortParam) * stations);
    if (portVector == NULL)
    {
        fprintf(stderr, "Failed to allocate PNI context block\n");
        free (netbuf);
        free (ascbuf);
        return RETERROR (ENOMEM);
    }

    lastInPort = 0;
    
    /*
    **  Create the thread which will deal with TCP connections.
    */
    pniPorts.portNum = pniPort;
    pniPorts.maxPorts = stations;
    pniPorts.localOnly = FALSE;
    pniPorts.callBack = pniWelcome;
    pniPorts.kind = "PNI";
    
    dtInitPortset (&pniPorts, MAXNET);

    /*
    **  Allocate the operator status buffer
    */
    if (statusBuf == NULL)
    {
        statusBuf = opInitStatus ("PNI", 0, 0);
        pniUpdateStatus ();
    }

    /*
    **  Print a friendly message.
    */
    printf("PNI initialised, stations %d through %d\n", firstStation, lastStation - 1);
    
    pniActive = TRUE;
    return RETOK;
}

void initPni (void)
{
}

void pniCheck (void)
{
    int len, i, j;
    int station;
    int opcode;
    CpWord oldout, hdr;
    char *p;
    CpWord *wp, w;
    PortParam *pp;
    NetFet *np;
    int port, key;
    char buf[2];
    
    if (!pniActive)
    {
        return;
    }
    fpniio.in = *afpniio;
    oldout = fpniio.out;
    while ((len = rcb (afpnib, (CpWord *)&fpniio, fpnilen, netbuf)) != 0)
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
        //  12/Station number
        hdr = netbuf[0];
        station = hdr & Mask12;
        opcode = (hdr >> 48) & Mask12;
        oldout = fpniio.out;
        DEBUGPRINT ("Message from framat, len %d, %llo, ptrs %llo %llo -> %llo\n", len, hdr, fpniio.in, oldout, fpniio.out);

        if (station < firstStation || station >= lastStation)
        {
            DEBUGPRINT ("Station %d out of range\n", station);
            continue;
        }
        pp = portVector + (station - firstStation);
        switch (opcode)
        {
        case 0:
            // Data message, send it to the terminal
            p = ascbuf;
            wp = netbuf + 1;
            for (i = 1; i < len; i += 2)
            {
                w = *wp++;
                DEBUGPRINT (" %015llx\n", w);
                if (storeChar (w >> 52, &p)) break;
                if (storeChar (w >> 44, &p)) break;
                if (storeChar (w >> 36, &p)) break;
                if (storeChar (w >> 28, &p)) break;
                if (storeChar (w >> 20, &p)) break;
                if (storeChar (w >> 12, &p)) break;
                if (storeChar (w >>  4, &p)) break;
                if (i + 1 == len) break;
                j = (w << 4) & 0x70;
                w = *wp++;
                DEBUGPRINT (" %015llx\n", w);
                j |= (w >> 56) & Mask4;
                if (storeChar (j, &p)) break;
                if (storeChar (w >> 48, &p)) break;
                if (storeChar (w >> 40, &p)) break;
                if (storeChar (w >> 32, &p)) break;
                if (storeChar (w >> 24, &p)) break;
                if (storeChar (w >> 16, &p)) break;
                if (storeChar (w >>  8, &p)) break;
                if (storeChar (w, &p)) break;
            }
            len = p - ascbuf;
            if (len > 0)
            {
                DEBUGPRINT ("Send %d bytes to terminal %d\n", len, station);
                if (!pniSendstr (station, ascbuf, len))
                {
                    pp->flowFlags |= FLOW_TCP;
                }
            }
            if (pp->flowFlags == 0)
            {
                // Send Permit to Send shortly
                pp->flowFlags |= FLOW_DOPTS;
            }
            break;
        case 1:
            // Abort output
            DEBUGPRINT ("Abort output, terminal %d\n", station);
            // Send Abort done
            framatReq (F_ABT, station);
            break;
        case 2:
        case 5:
            // Logout terminal
            DEBUGPRINT ("Log out terminal %d\n", station);
            pp->loggedIn = FALSE;
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
            break;
        default:
            DEBUGPRINT ("Invalid framat request code %04o, station %d\n", opcode, station);
        }
    }
    /*
    **  Scan the active connections, round robin, looking
    **  for one that has pending input. 
    */
    port = lastInPort;
    for (;;)
    {
        if (++port == stations)
        {
            /*
            **  Whenever we finish a scan pass through the ports
            **  (i.e., we've wrapped around) look for more data.
            */
            port = 0;
            for (;;)
            {
                np = dtFindInput (&pniPorts, 0);
                if (np == NULL)
                {
                    break;
                }
                i = dtRead  (np, &pniPorts, -1);
                if (i < 0)
                {
                    dtClose (np, &pniPorts, TRUE);
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
            storeKey (OFFKY2, port + firstStation);
            pp->sendLogout = FALSE;
            break;
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
            i = dtReadw (np, buf, 1);       /* Get a byte */
            if (i < 0)
            {
                if (port == lastInPort)
                {
                    key = -1;
                    break;                  /* no input anywhere */
                }
                break;                      /* we don't have enough data yet */
            }
            key = buf[0] & Mask7;
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
                    key = pp->keyAsm | ((key & 037) << 5);
                }
                else
                {
                    key = -1;
                }
                pp->escState = norm;
                break;
            }
            // If we're done with the esc sequence, done with this port
            if (pp->escState == norm)
            {
                break;
            }
                
        }
        if (pp->forceLogout)
        {
            /*
            **  If we're not logged out yet, send *offky2* which
            **  tells PLATO to log out this station.
            */
            storeKey (OFFKY2, port + firstStation);
            printf ("continuing to force logout port %d\n", port);
            break;
        }
        else if (key >= 0 && pp->escState == norm)
        {
            DEBUGPRINT ("pni key %04o\n", key);
            storeKey (key, port + firstStation);
            pp->keyAsm = 0;
        }
        if (port == lastInPort)
        {
            break;                      /* no input anywhere */
        }
    }
    lastInPort = port;
    for (port = 0; port < stations; port++)
    {
        pp = portVector + port;
        np = pp->np;
        if (pp->flowFlags == FLOW_DOPTS)
        {
            // Need to send PTS, and no flow-off flags are set
            if (framatReq (F_PTS, port + firstStation))
            {
                // Sent successfully, clear flag
                pp->flowFlags = 0;
            }
        }
    }
    
}

static int rcb (CpWord *buf, CpWord *iop, int buflen, CpWord *dest)
{
    struct Io io;
    int len;
    CpWord hdr;
    
    memcpy (&io, iop, sizeof (io));
    if (io.in == io.out)
    {
        return 0;
    }
    hdr = *dest = buf[io.out];

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
        if (io.out + len <= buflen)
        {
            // no wrap
            memcpy (dest, buf + io.out, len * sizeof (CpWord));
            if (io.out + len < buflen)
            {
                io.out += len;
            }
            else
            {
                io.out = 0;
            }
        }
        else
        {
            memcpy (dest, buf + io.out, (buflen - io.out) * sizeof (CpWord));
            dest += buflen - io.out;
            len -= buflen - io.out;
            memcpy (dest, buf, len * sizeof (CpWord));
            io.out = len;
        }
    }
    else if (++io.out == buflen)
    {
        io.out = 0;
    }
    iop[1] = io.out;
    
    return len;
}

static void storeKey (int key, int station)
{
    struct Keybuf *kp;
    int ppidx, newppidx, cpidx;
    int word, shift;
    
    if (station < firstStation || station >= lastStation)
    {
        DEBUGPRINT ("storeKey: station out of range, %d\n");
        return;
    }
    
    kp = (struct Keybuf *) (akb + (station * NKEYLTH));
    ppidx = kp->buf[CKPPIDX] & Mask12;
    cpidx = kp->cpidx & Mask12;
    newppidx = ppidx + 1;
    if (newppidx == IDXLIM)
    {
        newppidx = 0;
    }
    if (cpidx == newppidx)
    {
        // Key buffer is full.  Check for special keys
        if (key == OFFKY2 || key == ((kp->cpidx >> 12) & Mask11))
        {
            // Overwrite the last stored key by backing up the pointers
            newppidx = ppidx;
            if (--ppidx < 0)
            {
                ppidx = IDXLIM - 1;
            }
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
    kp->buf[CKPPIDX] = (kp->buf[CKPPIDX] & ~Mask12) | newppidx;

    // Now that we've stored the key, set the bit in the 
    // site/station bitmap.  We don't need the readback
    // check on the bottom bit as is done elsewhere because
    // nothing else can get in the middle of this...
    abt[station >> 5] |= 1 + (0x1ULL << ((station & 0x1f) + 16));
    DEBUGPRINT ("Key bitmap: %020llo\n", abt[station >> 5]);
}

static bool storeChar (int c, char **pp)
{
    char *p = *pp;
    
    c &= Mask8;
    *p = c;
    if (c == 0xff)
    {
        // Telnet escape
        *++p = c;
    }
    if (c != 0)
    {
        *pp = ++p;
    }
    return (c == 0);
}

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
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void pniUpdateStatus (void)
{
    char msg[64];
    
    sprintf (msg, "%d connection%s", pniActiveConns,
             (pniActiveConns != 1) ? "s" : "");
    if (pniActiveConns == 0)
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
**                  np          NetFet pointer
**                  stat        station number relative to start of PNI
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void pniWelcome(NetFet *np, int stat)
{
    PortParam *mp;

    mp = portVector + stat;
    stat += firstStation;
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
        pniUpdateStatus ();
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
    printf("%s pni: Received connection from %s for station %d-%d\n",
           dtNowString (), inet_ntoa (np->from),
           stat / 32, stat % 32);

    np->ownerInfo = stat;
    pniActiveConns++;
    pniUpdateStatus ();

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

    /*
    **  Indicate flow is on and permit to send needed. 
    */
    mp->flowFlags = FLOW_DOPTS;
    
#define WELCOME_PFX \
    "\033\002"      /* Enter PLATO mode */                              \
        "\033\014"  /* Full screen erase */                             \
        "\033\024"  /* Mode rewrite */                                  \
        "\037"      /* Mode 3 (text plotting) */                        \
        "\033\062\044\140\044\100" /* Load X and Y 128 */

    pniSendstr (stat, WELCOME_PFX "Press  NEXT  to begin", 0);
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
    NetFet *fet;

    if (len == 0)
    {
        len = strlen (p);
    }
    fet = pniPorts.portVec + (stat - firstStation);
    return (dtSend(fet, &pniPorts, p, strlen (p)) < 0);
}

