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
#endif
#include "ptermversion.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define DEFAULTHOST "cyberserv.org"
#define BufSiz      256

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

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
#if !defined(_WIN32)

extern void ptermInput(XEvent *event);
#endif
extern void ptermInit(const char *winName, bool closeOk);
extern void ptermClose (void);
extern void procNiuWord (int stat, u32 d);
extern void ptermSetWeMode (u8 we);

int main (int argc, char **argv)
    {
    u8 ibuf[3];
    u32 niuwd;
    int i;
    char name[100];
    int port;
    int true_opt = 1;
    
    argc--;
    argv++;
    if (strcmp (argv[0], "-s") == 0)
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
#if !defined(_WIN32)
            ptermWindowInput ();
#endif
            i = dtReadw (&fet, ibuf, 3);
            if (i < 0)
                {
                break;              /* don't have 3 bytes yet */
                }
            if (ibuf[0] & 0200)
                {
                printf ("Plato output out of sync byte 0: %03o\n", ibuf[0]);
                break;
                }
            if ((ibuf[1] & 0300) != 0200)
                {
                printf ("Plato output out of sync byte 1: %03o\n", ibuf[1]);
                break;
                }
            if ((ibuf[2] & 0300) != 0300)
                {
                printf ("Plato output out of sync byte 2: %03o\n", ibuf[2]);
                break;
                }
            niuwd = (ibuf[0] << 12) | ((ibuf[1] & 077) << 6) | (ibuf[2] & 077);
            procNiuWord (1, niuwd);
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
**                  stat        Station number (ignored)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void niuLocalKey(u16 key, int stat)
    {
    u8 data[2];
    
#if 0   // this doesn't seem to work quite the way I want it to...
    if (key == 032 || key == 072)
        {
        /*
        **  STOP or SHIFT-STOP keys -- discard any pending
        **  data from the network.
        **
        **  Usually that doesn't matter much, but if there are -delay-
        **  words pending, there could be quite a lot of stuff backed
        **  up, and we want to abort all that delaying.
        */
        fet.out = fet.in;
        }
#endif

    data[0] = key >> 7;
    data[1] = 0200 | key;

    if (tracePterm)
        {
        fprintf (traceF, "key to plato %03o\n", key);
        }
    send(fet.connFd, data, 2, 0);
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
            // Special case: check for Control/D (disconnect)
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
                else if (key == XK_bracketright)    // control-] : trace
                    {
                    tracePterm = !tracePterm;
                    savemode = wemode;
                    if (!tracePterm)
                        {
                        wemode = 2;
                        fflush (traceF);
                        }
                    else
                        {
                        if (traceF == NULL)
                            {
                            traceF = fopen (traceFn, "w");
                            }
                        wemode = 3;
                        }
                    ptermSetWeMode (wemode);
                    // The 1024 is a strange hack to circumvent the
                    // screen edge wrap checking.
                    ptermDrawChar (1024 + 512, 512, 1, 024);
                    wemode = savemode;
                    ptermSetWeMode (wemode);
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
