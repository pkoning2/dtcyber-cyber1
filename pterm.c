/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, 2004, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: pterm.c
**
**  Description:
**      Mainline driver for Plato terminal emulation, connected via
**      TCP connection to DtCyber.
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
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#define PTERM_DEFINE_X
#endif
#include "ptermversion.h"
#include "pterm.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define DEFAULTHOST "cyberserv.org"
#define BufSiz      256
#define RINGSIZE    5000
#define RINGXOFF1   1000
#define RINGXOFF2   1100
#define RINGXON1    400
#define RINGXON2    200
#define xonkey      01606
#define xofkey      01607

#define niuRingCount ((niuIn >= niuOut) ? (niuIn - niuOut) : (RINGSIZE + niuIn - niuOut))

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
#if !defined(_WIN32)
static void ptermWindowInput(void);
#endif

/*
**  ----------------
**  Public Variables
**  ----------------
*/
#if !defined(_WIN32)
Display *disp;
XrmDatabase XrmDb;
#else
HINSTANCE hInstance;
#endif
extern FILE *traceF;
extern char traceFn[];
extern bool tracePterm;
extern u8 wemode;
extern bool emulationActive;
extern const char *hostName;
int scale = 1;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static int obytes;
static u32 currOutput;
static NetFet fet;
static u16 currInput;
static u32 niuRing[RINGSIZE];
static int niuIn, niuOut;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

int main (int argc, char **argv)
    {
    u8 ibuf[3];
    u32 niuwd;
    int i, j, k;
    char name[100];
    int port;
    int true_opt = 1;
    int next;
    
    argc--;
    argv++;
    if (argc > 0 && strcmp (argv[0], "-s") == 0)
        {
        argc--;
        argv++;
        scale = 2;
        }
    if (argc > 2)
        {
        printf ("usage: pterm [-s] [ hostname [ portnum ]]\n");
        exit (1);
        }
    if (argc > 1)
        {
        port = atoi (argv[1]);
        }
    else
        {
        port = DefNiuPort;
        }
    if (argc > 0)
        {
        hostName = argv[0];
        }
    else
        {
        hostName = DEFAULTHOST;
        }

    dtInitFet (&fet, BufSiz);
    if (dtConnect (&fet.connFd, hostName, port) < 0)
        {
        exit (1);
        }
    setsockopt (fet.connFd, SOL_SOCKET, SO_KEEPALIVE,
                (char *)&true_opt, sizeof(true_opt));
#if defined(_WIN32)
    ioctlsocket (fet.connFd, FIONBIO, &true_opt);
#else
    fcntl (fet.connFd, F_SETFL, O_NONBLOCK);
#endif
#ifdef __APPLE__
    setsockopt (fet.connFd, SOL_SOCKET, SO_NOSIGPIPE,
                (char *)&true_opt, sizeof(true_opt));
#endif

    emulationActive = TRUE;

#if defined(_WIN32)
    /*
    **  Get our instance
    */
    hInstance = GetModuleHandle(NULL);
#endif

    sprintf (name, "Pterm " PTERMVERSION ": %s", hostName);
    ptermInit (name, TRUE);
    while (emulationActive)
        {
        i = dtRead (&fet, 10);
        if (i < 0)
            {
            break;
            }
        
        for (;;)
        {
            /*
            **  Assemble words from the network buffer, all the
            **  while looking for "abort output" codes (word == 2).
            */
            next = niuIn + 1;
            if (next == RINGSIZE)
                {
                next = 0;
                }
            if (next == niuOut)
                {
                break;
                }
            if (dtFetData (&fet) < 3)
                {
                break;
                }
            i = dtReado (&fet);
            if (i & 0200)
                {
                printf ("Plato output out of sync byte 0: %03o\n", i);
                continue;
                }
    newj:
            j = dtReado (&fet);
            if ((j & 0300) != 0200)
                {
                printf ("Plato output out of sync byte 1: %03o\n", j);
                if ((j & 0200) == 0)
                    {
                    i = j;
                    goto newj;
                    }
                continue;
                }
            k = dtReado (&fet);
            if ((k & 0300) != 0300)
                {
                printf ("Plato output out of sync byte 2: %03o\n", k);
                if ((k & 0200) == 0)
                    {
                    i = k;
                    goto newj;
                    }
                continue;
                }
            niuwd = (i << 12) | ((j & 077) << 6) | (k & 077);
            if (niuwd == 2)
                {
                niuOut = niuIn;
                }
            niuRing[niuIn] = niuwd;
            niuIn = next;
            i = niuRingCount;
            if (i == RINGXOFF1 || i == RINGXOFF2)
                {
                ptermSendKey (xofkey);
#if 0
                printf ("off ");
                fflush (stdout);
#endif
                }
        }
#if 0
        if (niuRingCount > 0 &&
            ((niuRingCount % 100) == 0))
            {
            printf ("%d ", niuRingCount);
            fflush (stdout);
            }
#endif
        for (;;)
            {
            /*
            **  Process words until we hit some point that causes output delay.
            **  That way we see an abort code as soon as possible.
            */
#if !defined(_WIN32)
            ptermWindowInput ();
#endif
            if (niuIn == niuOut)
                {
                break;
                }
            next = niuOut + 1;
            if (next == RINGSIZE)
                {
                next = 0;
                }
            j = procNiuWord (niuRing[niuOut]);
            niuOut = next;
            i = niuRingCount;
            if (i == RINGXON1 || i == RINGXON2)
                {
                ptermSendKey (xonkey);
#if 0
                printf ("on ");
                fflush (stdout);
#endif
                }
            if (j)
                {
                break;
                }
            }
        }
    ptermClose ();
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process Plato mode input
**
**  Parameters:     Name        Description.
**                  key         Plato key code for station
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermSendKey(int key)
    {
    u8 data[2];
    
    /*
    **  If this is a "composite", recursively send the two pieces.
    */
    if ((key >> 9) != 0)
        {
        ptermSendKey (key >> 9);
        ptermSendKey (key & 0777);
        }
    else
        {
        data[0] = key >> 7;
        data[1] = 0200 | key;

        if (tracePterm)
            {
            fprintf (traceF, "key to plato %03o\n", key);
            }
        send(fet.connFd, data, 2, 0);
        }
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/
#if !defined(_WIN32)
/*--------------------------------------------------------------------------
**  Purpose:        Window input event processor.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ptermWindowInput(void)
    {
    XEvent event;
    XKeyEvent *kp;
    int key;
    KeySym ks;
    int savemode;
    
    /*
    **  Process any X11 events.
    */
    while (XEventsQueued(disp, QueuedAfterFlush))
        {
        XNextEvent(disp, &event);

        switch (event.type)
            {
        case MappingNotify:
            XRefreshKeyboardMapping ((XMappingEvent *)&event);
            break;

        case KeyPress:
            // Special case: check for Control/Z (disconnect)
            kp = (XKeyEvent *) &event;
            if (kp->state & ControlMask)
                {
                key = XKeycodeToKeysym (disp, kp->keycode, 0);
                if (key == XK_z)        // control-Z : exit
                    {
                    dtCloseFet(&fet);
                    emulationActive = FALSE;
                    return;
                    }
#if 0
                else if (key == XK_s || key == XK_q)
                    {
                    ptermSendKey (key == XK_s ? xofkey : xonkey);
                    return;
                    }
#endif
                else if (key == XK_bracketright)    // control-] : trace
                    {
                    tracePterm = !tracePterm;
                    ptermSetTrace (TRUE);
                    return;
                    }
                }
            // Fall through to default handler
        default:
            ptermInput (&event);
            break;
            }
        }
    }
#endif

/*---------------------------  End Of File  ------------------------------*/
