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

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define platoPort   5004

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
static void ptermConnect (const char *hostname, const char *portnum);
static int ptermCheckInput(void);
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
extern bool tracePterm;
extern u8 wemode;
extern volatile bool ptermActive;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static int obytes;
static u32 currOutput;
static int connFd;
static u16 currInput;
static u8 ibytes;      // how many bytes have been assembled into currInput (0..2)
static niuProcessOutput *outh;

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

int main (int argc, char **argv)
{
    int ibytes = 0;
    u32 niuwd;
    int d;
    char name[100];
    
    if (argc < 2)
    {
        printf ("usage: pterm hostname [ portnum ]\n");
        exit (1);
    }
    if (argc > 2)
    {
        ptermConnect (argv[1], argv[2]);
    }
    else
    {
        ptermConnect (argv[1], NULL);
    }

#if defined(_WIN32)
    /*
    **  Get our instance
    */
    hInstance = GetModuleHandle(NULL);
#endif

    sprintf (name, "Plato terminal -- %s", argv[1]);
    ptermInit (name, TRUE);
    while (ptermActive)
    {
        d = ptermCheckInput ();
        if (d >= 0)
        {
            switch (ibytes)
            {
            case 0:
                if (d & 0200)
                {
                    printf ("Plato output out of sync byte 0: %03o\n", d);
                    continue;
                }
                niuwd = d << 12;
                ibytes = 1;
                break;
            case 1:
                if ((d & 0300) != 0200)
                {
                    printf ("Plato output out of sync byte 1: %03o\n", d);
                    continue;
                }
                niuwd |= ((d & 077) << 6);
                ibytes = 2;
                break;
            case 2:
                if ((d & 0300) != 0300)
                {
                    printf ("Plato output out of sync byte 2: %03o\n", d);
                    continue;
                }
                niuwd |= (d & 077);
                ibytes = 0;
                (*outh) (1, niuwd);
                break;
            }
        }
#if !defined(_WIN32)
        ptermWindowInput ();
#endif
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
    
    data[0] = key >> 7;
    data[1] = 0200 | key;

    if (tracePterm)
    {
        fprintf (traceF, "key to plato %03o\n", key);
    }
    send(connFd, data, 2, 0);
}

/*--------------------------------------------------------------------------
**  Purpose:        Set handler address for station output
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
    outh = h;
}

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/
/*--------------------------------------------------------------------------
**  Purpose:        Initialise Pterm
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ptermConnect (const char *hostname, const char *portnum)
{
    struct hostent *hp;
    struct sockaddr_in server;
    int port = platoPort;
    
#if defined(_WIN32)
    WORD versionRequested;
    WSADATA wsaData;
    int err;

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
#endif
    /*
    **  Create TCP socket and bind to specified port.
    */
    connFd = socket(AF_INET, SOCK_STREAM, 0);
    if (connFd < 0)
    {
        printf("pterm: Can't create socket\n");
        return;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    hp = gethostbyname (hostname);
    if (hp->h_length == 0)
    {
        printf ("pterm: unrecognized hostname %s\n", hostname);
        return;
    }
    if (portnum != NULL)
    {
        port = atoi (portnum);
    }
    memcpy (&server.sin_addr, hp->h_addr, sizeof (server.sin_addr));
    server.sin_port = htons(port);

    if (connect (connFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("pterm: Can't connect to %s %d\n", hostname, port);
        return;
    }
    ptermActive = TRUE;
}

/*--------------------------------------------------------------------------
**  Purpose:        Check for input.
**
**  Parameters:     Name        Description.
**
**  Returns:        data byte received, or -1 if there is no data.
**
**------------------------------------------------------------------------*/
static int ptermCheckInput(void)
{
    int i;
    fd_set readFds;
    fd_set exceptFds;
    struct timeval timeout;
    u8 data;

    FD_ZERO(&readFds);
    FD_ZERO(&exceptFds);
    FD_SET(connFd, &readFds);
    FD_SET(connFd, &exceptFds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    select(connFd + 1, &readFds, NULL, &exceptFds, &timeout);
    if (FD_ISSET(connFd, &readFds))
    {
        i = recv(connFd, &data, 1, 0);
        if (i == 1)
        {
            return(data);
        }
        else
        {
#if defined(_WIN32)
            closesocket(connFd);
#else
            close(connFd);
#endif
            ptermActive = FALSE;
            printf("pterm: Connection dropped\n");
            return(-1);
        }
    }
    else if (FD_ISSET(connFd, &exceptFds))
    {
#if defined(_WIN32)
        closesocket(connFd);
#else
        close(connFd);
#endif
        ptermActive = FALSE;
        printf("pterm: Connection dropped\n");
        return(-1);
    }
    else
    {
        return(-1);
    }
}

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
    KeySym ks;
    char text[30];
    int len;
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
                len = XLookupString (kp, text, 10, &ks, 0);
                if (len == 1)
                {
                    if (text[0] == '\032')  // control-Z : exit
                    {
                        close(connFd);
                        ptermActive = FALSE;
                        return;
                    }
                    else if (text[0] == '\035') // control-] : trace
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
