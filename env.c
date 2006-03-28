/*--------------------------------------------------------------------------
**
**  Copyright (c) 2006, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: env.c
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

/*--------------------------------------------------------------------------
**  Purpose:        Access some external information
**
**  Parameters:     Name        Description.
**                  req         Request (60 bits)
**
**  Returns:        result of the request (60 bits)
**
**------------------------------------------------------------------------*/
CpWord envOp (CpWord req)
    {
    char *p = reqstr;
    int i;
    char c;
    CpWord result;
    
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
    if (strcmp (reqstr, "HOSTNAME") == 0)
        {
        gethostname (resultstr, sizeof (resultstr) - 1);
        }
    else
        {
        return 0;
        }
    p = resultstr;
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
