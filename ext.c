/*--------------------------------------------------------------------------
**
**  Copyright (c) 2006, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: ext.c
**
**  Description:
**      Access to the environment outside the emulator
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
#endif
#include <errno.h>

static char reqstr[11];
static char resultstr[500];

static CpWord envOp (CpWord req)
{
    CpWord *reqp = cpuAccessMem (req, 2);
    char *p = reqstr;
    int i;
    char c;
    CpWord result;
    
    if (reqp == NULL)
    {
        return 0;
    }
    printf ("req %llo, reqp %p, reqp[1] %llo\n", req, reqp, reqp[1]);
    req = reqp[1];
    for (i = 0; i < 10; i++)
    {
        c = (req >> 54) & 077;
        if (c == 0)
        {
            break;
        }
        *p++ = cdcToAscii[c];
        req <<= 6;
    }
    *p = '\0';
    p = getenv (reqstr);
    if (p == NULL)
    {
        /*
        **  Not in the environment, see if it's a name we know of.
        */
        if (strcmp (reqstr, "HOSTNAME") == 0)
        {
            gethostname (resultstr, sizeof (resultstr) - 1);
        }
        else
        {
            return 0;
        }
        p = resultstr;
    }
    result = 0;
    for (i = 0; i < 10; i++)
    {
        c = *p++;
        if (c == 0)
        {
            break;
        }
        result |= (CpWord) asciiToCdc[c] << (54 - 6 * i);
    }
    return result;
}

/*--------------------------------------------------------------------------
**  Purpose:        External operation
**
**  Parameters:     Name        Description.
**                  req         Pointer to request (1/ecsflag, 35/, 24/address)
**
**                              48/reqdependent, 12/request
**                              60/reqdependent ...
**
**  Returns:        result of the request (60 bits)
**
**------------------------------------------------------------------------*/
CpWord extOp (CpWord req)
{
    CpWord *reqp = cpuAccessMem (req, 1);
    
    printf ("req %llo, reqp %p, *reqp %llo\n", req, reqp, *reqp);
    
    if (reqp == NULL)
    {
        return 0;
    }
    switch (*reqp & Mask12)
    {
    case 0:
        return envOp (req);
//    case 1:
//        return sockOp (req);
    default:
        return 0;
    }
}

