/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter, Paul Koning (see license.txt)
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
#include <sys/time.h>
#if defined(_WIN32)
#include <winsock.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#endif

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define platoPort   5004

//#define DEBUG

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef void niuProcessOutput (int, u32);

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void ptermConnect (const char *hostname);
static int ptermCheckInput(void);
static void ptermWindowInput(void);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
Display *disp;

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
static bool active;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
extern void ptermInput(XEvent *event);

int main (int argc, char **argv)
{
    int ibytes = 0;
    u32 niuwd;
    int d;
    char name[100];
    
    if (argc < 2)
    {
        printf ("usage: pterm hostname\n");
        exit (1);
    }
    ptermConnect (argv[1]);
    sprintf (name, "Plato terminal -- %s", argv[1]);
    ptermInit (name);
    while (active)
    {
        d = ptermCheckInput ();
        if (d >= 0)
        {
#ifdef DEBUG
            printf ("from plato byte %d %03o\n", ibytes, d);
#endif
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
                procNiuWord (1, niuwd);
                break;
            }
        }
        ptermWindowInput ();
    }
    ptermClose ();
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
#ifdef DEBUG
            printf ("to plato %03o %03o\n", data[0], data[1]);
#endif
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
static void ptermConnect (const char *hostname)
{
    struct hostent *hp;
    struct sockaddr_in server;
    
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
    memcpy (&server.sin_addr, hp->h_addr, sizeof (server.sin_addr));
    server.sin_port = htons(platoPort);

    if (connect (connFd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("pterm: Can't connect to %s\n", hostname);
        return;
    }
    active = TRUE;
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
            active = FALSE;
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
        active = FALSE;
        printf("pterm: Connection dropped\n");
        return(-1);
    }
    else
    {
        return(-1);
    }
}

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

        default:
            ptermInput (&event);
            break;
        }
    }
}

/*---------------------------  End Of File  ------------------------------*/
