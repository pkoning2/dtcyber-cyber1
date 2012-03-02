/*--------------------------------------------------------------------------
**
**  Copyright (c) 2005-2007, Paul Koning, Tom Hunter
**
**  Name: blackbox.c
**
**  Description:
**      Perform simulation of the CIU "black box"
**
**  Usage:
**      This little tool can be run when PLATO is down (for example for backups).
**      It acts like the CUI "down tapes" or "black box", periodically sending
**      a fixed message to all connected terminals.
**      In this case, the message is supplied as the command argument,
**      which normally means it has to be quoted.  If no argument is given,
**      the message is "PLATO is down for the moment".
**      The message will repeat every 5 seconds until the program is 
**      terminated.
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
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "const.h"
#include "types.h"
#include "proto.h"

long platoConns = 128;
long platoPort = 5004;
long asciiConns = 64;
long asciiPort = 8005;
bool emulationActive = TRUE;

int currentY = 128;
char *msg;

static NetPortSet niuPorts;
static NetPortSet niuAsciiPorts;

static void niuSendstr(int stat, const char *p);
static void niuSendWord(int stat, int word);
static void niuAsciiWelcome(NetFet *np, int stat, void *arg);
static void niuRemoteWelcome(NetFet *np, int stat, void *arg);
static void bbSendNiu(int stat);
static void bbSendAscii(int stat);

int main (int argc, char **argv)
    {
    int stat, i, detflag = 0;
    NetFet *np;
    char msg2[200];
    
    if (argc < 2)
        {
        msg = strdup ("PLATO is down for the moment");
        }
    else
        {
        i = 1;
        if (argc > 2)
            {
            if (strcmp (argv[1], "-d") != 0)
                {
                printf ("usage: blackbox [ [ -d ] message ]\n");
                exit (1);
                }
            i = 2;
            detflag = 1;
            }
        msg = strdup (argv[i]);
        }
    
    /*
    **  Create the threads which will deal with TCP connections.
    */
    niuPorts.portNum = platoPort;
    niuPorts.maxPorts = platoConns;
    niuPorts.localOnly = FALSE;
    niuPorts.callBack = niuRemoteWelcome;
    niuAsciiPorts.portNum = asciiPort;
    niuAsciiPorts.maxPorts = asciiConns;
    niuAsciiPorts.localOnly = FALSE;
    niuAsciiPorts.callBack = niuAsciiWelcome;
    
    dtInitPortset (&niuPorts, 1024);
    dtInitPortset (&niuAsciiPorts, 1024);

    printf ("Current message is: '%s'\n", msg);
    if (!detflag)
        {
        printf ("Enter a new message at any time, or Ctrl-D to stop blackbox.\n");
        fcntl (fileno (stdin), F_SETFL, O_NONBLOCK);
        }
    
    for (;;)
        {
        /*
        **  Read and discard any input; also handle disconnects.
        */
        for (;;)
            {
            np = dtFindInput (&niuPorts, 0);
            if (np == NULL)
                {
                break;
                }
            i = dtRead  (np, &niuPorts, -1);
            if (i < 0)
                {
                dtClose (np, &niuPorts, TRUE);
                }
            }
        for (;;)
            {
            np = dtFindInput (&niuAsciiPorts, 0);
            if (np == NULL)
                {
                break;
                }
            i = dtRead  (np, &niuPorts, -1);
            if (i < 0)
                {
                dtClose (np, &niuAsciiPorts, TRUE);
                }
            }
        for (stat = 0; stat < platoConns; stat++)
            {
            bbSendNiu (stat);
            }
        for (stat = 0; stat < asciiConns; stat++)
            {
            bbSendAscii (stat);
            }
        currentY -= 32;
        if (currentY < 64)
            {
            currentY = 512 - 64;
            }

        /*
        **  Check for a new message, or exit.
        */
        sleep (5);
        if (!detflag)
            {
            i = read (fileno (stdin), msg2, sizeof (msg2));
            if (i == 0)
                {
                return 0;
                }
            else if (i > 0)
                {
                free (msg);
                msg2[i - 1] ='\0';
                msg = strdup (msg2);
                }
            }
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
static void niuSendstr(int stat, const char *p)
    {
    int cc = 2;
    int w = 017720;
    bool shift = FALSE;
    int c;
    
    if (&(niuPorts.portVec + stat)->connFd == 0)
        {
        return;
        }
    
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
                    niuSendWord (stat, 1);                    
                    w = 1;
                    }
                w = (w << 6 | 021);
                if (++cc == 3)
                    {
                    cc = 0;
                    niuSendWord (stat, w);
                    niuSendWord (stat, 1);                    
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
                niuSendWord (stat, 1);                    
                w = 1;
                }
            w = (w << 6 | 020);
            if (++cc == 3)
                {
                cc = 0;
                niuSendWord (stat, w);
                niuSendWord (stat, 1);                    
                w = 1;
                }
            shift = FALSE;
            }
        w = (w << 6 | asciiToCdc[c]);
        if (++cc == 3)
            {
            cc = 0;
            niuSendWord (stat, w);
            niuSendWord (stat, 1);                    
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
        niuSendWord (stat, 1);                    
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
static void niuSendWord(int stat, int word)
    {
    NetFet *fet;
    u8 data[3];

    fet = niuPorts.portVec + stat;
    if (fet->connFd == 0)
        {
        return;
        }
    
    data[0] = word >> 12;
    data[1] = ((word >> 6) & 077) | 0200;
    data[2] = (word & 077) | 0300;

    dtSend(fet, &niuPorts, data, 3);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle connect/disconnect
**
**  Parameters:     Name        Description.
**                  np          NetFet pointer
**                  stat        station number
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuRemoteWelcome(NetFet *np, int stat, void *arg)
    {
    if (np->connFd == 0)
        {
        /*
        **  Connection was dropped.
        */
        return;
        }

    /*
    **  New connection for this port.  Send a message right now
    */
    bbSendNiu (stat);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle connect/disconnect
**
**  Parameters:     Name        Description.
**                  np          NetFet pointer
**                  stat        station number
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void niuAsciiWelcome(NetFet *np, int stat, void *arg)
    {
    if (np->connFd == 0)
        {
        /*
        **  Connection was dropped.
        */
        return;
        }

    /*
    **  New connection for this port.  Send a message right now
    */
    bbSendAscii (stat);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send the message to a station, NIU format
**
**  Parameters:     Name        Description.
**                  stat        station number
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void bbSendNiu (int stat)
    {
    niuSendWord (stat, 0100033);            /* mode 3, mode rewrite, screen */
    niuSendWord (stat, 0201000 + currentY); /* load Y = currentY */
    niuSendWord (stat, 0200060);            /* load X = 48 */
    niuSendstr (stat, msg);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send the message to a station, ASCII format
**
**  Parameters:     Name        Description.
**                  stat        station number
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void bbSendAscii (int stat)
    {
    NetFet *fet;
    const char *modeStr = "\033\002"    // Enter PLATO mode
                          "\033\014"    // Full erase
                          "\033\024"    // Mode rewrite
                          "\0332";      // Load coordinate
    char setY[2];
    const char *setX = "\041\120";      // X = 48
    
    fet = niuAsciiPorts.portVec + stat;
    if (fet->connFd == 0)
        {
        return;
        }

    dtSend(fet, &niuAsciiPorts, modeStr, strlen (modeStr));
    setY[0] = (currentY >> 5) | 040;
    setY[1] = (currentY & 037) | 0140;
    dtSend (fet, &niuAsciiPorts, setY, 2);
    dtSend (fet, &niuAsciiPorts, setX, 2);
    
    dtSend (fet, &niuAsciiPorts, msg, strlen (msg));
    }
