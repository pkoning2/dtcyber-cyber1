/*--------------------------------------------------------------------------
**
**  Copyright (c) 2005, Paul Koning, Tom Hunter
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
#include "const.h"
#include "types.h"
#include "proto.h"

ChSlot ic, oc;
DevSlot id, od;
ChSlot *activeChannel;
DevSlot *activeDevice;
u32 rtcClock = 0;
bool emulationActive = TRUE;

long platoLocalConns = 31;
long platoLocalPort = 5005;
long platoConns = 96;
long platoPort = 5004;

DevSlot *channelAttach (u8 channelNo, u8 eqNo, u8 devType)
    {
    if (channelNo == 1)
        {
        activeDevice = &id;
        }
    else
        {
        activeDevice = &od;
        }
    activeDevice->devType = devType;
    activeDevice->eqNo = eqNo;
    return activeDevice;
    }


void * opInitStatus (const char *type, int ch, int un)
    {
    return NULL;
    }

void opSetStatus (void *buf, const char *msg)
    {
    }

int main (int argc, char **argv)
    {
    int stat;
    char *msg;
    
    if (argc < 2)
        {
        msg = "PLATO is down for the moment";
        }
    else
        {
        msg = argv[1];
        }
    
    ic.id = 1;
    oc.id = 2;
    id.channel = &ic;
    od.channel = &oc;
    
    niuInit (0, 1, 2, NULL);
    activeDevice = &id;
    (*id.func) (041);
    activeDevice = &od;
    (*od.func) (0);
    
    for (;;)
        {
        /*
        **  Read and discard any input; this also causes niu.c to
        **  handle disconnects.
        */
        for (;;)
            {
            ic.full = FALSE;
            activeDevice = &id;
            activeChannel = &ic;
            (*id.io) ();
            if (!ic.full)
                {
                break;
                }
            }
        for (stat = 1; stat < platoLocalConns + platoConns + 32; stat++)
            {
            niuSendWord (stat, 0100033);        /* mode 3, mode rewrite, screen */
            niuSendWord (stat, 0201400);        /* load Y = 256 */
            niuSendWord (stat, 0200060);        /* load X = 48 */
            niuSendstr (stat, msg);
            }
        sleep (5);
        }
    }
