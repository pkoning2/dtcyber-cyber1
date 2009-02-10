#define DEBUG 1
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
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#endif
#include <errno.h>

#ifdef DEBUG
#define DEBUGPRINT printf
#else
#define DEBUGPRINT(fmt, ...)
#endif

#define RETOK       ((~1) & Mask60)
#define RETINVREQ   0
#define RETNOSOCK   1
#define RETNODATA   2
#define RETERRNO    (1000 + errno)

#define MAXIO       4096

static char reqstr[11];
static char resultstr[MAXIO];

/*--------------------------------------------------------------------------
**  Purpose:        Read an environment variable
**
**  Parameters:     Name        Description.
**                  req         Pointer to request (1/ecsflag, 35/, 24/address)
**
**                              48/0, 12/0
**                              60/envname
**
**                              The environment variable name is up to
**                              10 characters, display code.  It is
**                              translated to upper case ASCII for lookup.
**
**  Returns:        result of the request (60 bits): first 10 characters
**                  of lovariable value, in display code.  0 if not found.
**
**------------------------------------------------------------------------*/
static CpWord envOp (CpWord req)
{
    CpWord *reqp = cpuAccessMem (req, 2);
    CpWord *bufp;
    char *p = reqstr;
    int i;
    char c;
    CpWord result;
    
    if (reqp == NULL)
    {
        return 0;
    }
    DEBUGPRINT ("req %llo, reqp %p, reqp[1] %llo\n", req, reqp, reqp[1]);
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
**  Purpose:        Perform a socket operation.
**
**  Parameters:     Name        Description.
**                  req         Pointer to request (1/ecsflag, 35/, 24/address)
**
**                              36/0, 6/mode, 6/function, 12/1
**                              28/0, 32/socknum
**                              60/reqdependent...
**
**  Returns:        Operation status.  -1 for success, >= 0 for error.
**                  For host OS errors, status is 1000 + errno value.
**
**------------------------------------------------------------------------*/
static struct sockaddr_in addr;
static CpWord sockOp (CpWord req)
{
    CpWord *reqp = cpuAccessMem (req, 10);
    int socknum;
    int retval;
    int true_opt = 1;
    int mode;
    int buflen;
    CpWord *bufp;
    CpWord d;
    char *cp;
    int ic, oc;
    int shift;
    int pc;
    int i, pcnt;
    
    if (reqp == NULL)
    {
        return 0;
    }
    mode = ((*reqp >> 18) & Mask6);
    socknum = reqp[1];

    switch ((*reqp >> 12) & Mask6)
    {
    case 0:
        // reset
        if (socknum != 0)
        {
            if (close (socknum))
            {
                return RETERRNO;
            }
            return RETOK;
        }
        return RETNOSOCK;
    case 1:
        // close
        if (socknum != 0)
        {
            if (shutdown (socknum, SHUT_RDWR))
            {
                return RETERRNO;
            }
            return RETOK;
        }
        return RETNOSOCK;
    case 2:
        // socket (get a socket number)
        if (socknum == 0)
        {
            retval = socket (PF_INET, SOCK_STREAM, 0);
            if (retval > 0)
            {
                DEBUGPRINT ("new socket %d\n", retval);
                reqp[1] = retval;
#if defined(_WIN32)
                ioctlsocket (retval, FIONBIO, &true_opt);
#else
                fcntl (retval, F_SETFL, O_NONBLOCK);
#endif
#ifdef __APPLE__
                setsockopt (retval, SOL_SOCKET, SO_NOSIGPIPE,
                            (char *) &true_opt, sizeof (true_opt));
#endif
                return RETOK;
            }
            return RETERRNO;
        }
        return RETINVREQ;
    case 3:
        // bind
        // 2: 44/0, 16/port
        // 3: 28/0, 32/address
        if (socknum == 0)
        {
            return RETNOSOCK;
        }
        setsockopt (socknum, SOL_SOCKET, SO_REUSEADDR,
                   (char *) &true_opt, sizeof (true_opt));
        memset (&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl (reqp[3]);
        addr.sin_port = htons(reqp[2]);
        DEBUGPRINT ("bind to %08x:%d\n", ntohl (addr.sin_addr.s_addr), ntohs (addr.sin_port));
        if (bind (socknum, (struct sockaddr *) &addr, sizeof (addr)) < 0)
        {
            return RETOK;
        }
        return RETERRNO;
    case 4:
        // listen
        // 2: 44/0, 32/backlog
        if (socknum == 0)
        {
            return RETNOSOCK;
        }
        if (listen (socknum, reqp[2]) < 0)
        {
            return RETERRNO;
        }
        return RETOK;
    case 5:
        // accept
        // On return, 3rd word is the data socket, if the accept worked.
        // The IP address is returned in the 4th word.
        if (socknum == 0)
        {
            return RETNOSOCK;
        }
        reqp[2] = reqp[3] = 0;
        oc = sizeof (struct sockaddr);
        retval = accept (socknum, (struct sockaddr *) &addr, &oc);
        if (retval >= 0)
        {
            DEBUGPRINT ("accept returned socket %d, partner = %08x:%d\n", retval, ntohl (addr.sin_addr.s_addr), ntohs (addr.sin_port));
#if defined(_WIN32)
            ioctlsocket (retval, FIONBIO, &true_opt);
#else
            fcntl (retval, F_SETFL, O_NONBLOCK);
#endif
#ifdef __APPLE__
            setsockopt (retval, SOL_SOCKET, SO_NOSIGPIPE,
                        (char *) &true_opt, sizeof (true_opt));
#endif
            reqp[2] = retval;
            reqp[3] = ntohl (addr.sin_addr.s_addr);
            return RETOK;
        }
        if (errno == EAGAIN)
        {
            return RETNODATA;
        }
        return RETERRNO;
    case 6:
        // connect
        // 2: 44/0, 16/port
        // 3: 28/0, 32/address
        if (socknum == 0)
        {
            return RETNOSOCK;
        }
        memset (&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl (reqp[3]);
        addr.sin_port = htons(reqp[2]);
        if (connect (socknum, (struct sockaddr *) &addr, sizeof (addr)) < 0)
        {
            return RETOK;
        }
        return RETERRNO;
    case 7:
        // read
        // 2: 1/ecs, 35/0, 24/bufaddr
        // 3: 60/bufwords
        // 4: 60/retbufchars
        // The amount of data read is returned in the 5th word of the
        // request buffer.  The amount is in 8-bit bytes for binary mode,
        // or 6-bit characters in text mode.
        //
        // mode (from word 1) is 0 for binary, 1 for PLATO text.
        // In text mode, multiple lines may be returned, with line
        // terminators in the usual form.  The character count includes
        // line terminator characters, including padding to the next 
        // word boundary.  If the last line is not a full line, the
        // character count will include all characters but there will
        // not be a line terminator at the end of the last line.
        // In other words, if char[count-1] != 0 then the last line
        // is a partial line.
        if (socknum == 0)
        {
            return RETNOSOCK;
        }
        buflen = reqp[3];
        if (buflen <= 0)
        {
            return RETINVREQ;
        }
        bufp = cpuAccessMem (reqp[2], buflen);
        reqp[4] = 0;
        if (bufp == NULL)
        {
            return RETINVREQ;
        }
        retval = read (socknum, resultstr, MAXIO);
        if (retval < 0)
        {
            if (errno == EAGAIN)
            {
                return RETNODATA;
            }
            return RETERRNO;
        }
        resultstr[retval] = '\0';
        DEBUGPRINT ("read %d: %s\n", retval, resultstr);
        cp = resultstr;
        if (mode == 0)
        {
            shift = 60 - 8;
        }
        else
        {
            shift = 60 - 6;
        }
        d = 0;
        oc = 0;
        for (ic = 0; ic < retval; ic++)
        {
            if (buflen == 0)
            {
                break;
            }
            if (mode == 0)
            {
                // binary
                if (shift == -4)
                {
                    if (buflen == 1)
                    {
                        // Quit if there's not room for both pieces
                        break;
                    }
                    d |= *cp >> 4;
                    *bufp++ = d;
                    buflen--;
                    d = ((CpWord) (*cp & 0x0f)) << 56;
                    shift = 56 - 8;
                }
                else
                {
                    d |= ((CpWord) (*cp)) << shift;
                    if (shift == 0)
                    {
                        *bufp++ = d;
                        buflen--;
                        d = 0;
                        shift = 60 - 8;
                    }
                    else
                    {
                        shift -= 8;
                    }
                }
                oc++;
            }
            else
            {
                // PLATO text mode
                if (*cp == '\n' || *cp == '\r')
                {
                    // End of line
                    *bufp++ = d;
                    buflen--;
                    d = 0;
                    if (oc % 10 == 9)
                    {
                        if (buflen > 0)
                        {
                            *bufp++ = 0;
                            buflen--;
                            oc += 11;
                        }
                        else
                        {
                            oc++;
                        }
                    }
                    else
                    {
                        oc = ((oc + 10) / 10) * 10;
                    }
                }
                else
                {
                    pc = asciiToPlatoString[*cp];
                    if (pc < 0)
                    {
                        // char with no PLATO equivalent
                        continue;
                    }
                    // Check how many 6-bit codes we need
                    for (pcnt = 1; ; pcnt++)
                    {
                        if (((pc >> (6 * pcnt)) & Mask6) == 0)
                        {
                            break;
                        }
                    }
                    if (buflen == 1 && (oc % 10) > 10 - pcnt)
                    {
                        // Quit if no room
                        break;
                    }
                    for (i = pcnt - 1; i >= 0; i--)
                    {
                        d |= ((CpWord) ((pc >> (6 * i)) & Mask6)) << shift;
                        if (shift == 0)
                        {
                            *bufp++ = d;
                            buflen--;
                            d = 0;
                            shift = 60 - 6;
                        }
                        else
                        {
                            shift -= 6;
                        }
                        oc++;
                    }
                }
                cp++;
            }
        }
        DEBUGPRINT ("chars to Cyber: %d\n", oc);
        reqp[4] = oc;
        return RETOK;
    case 8:
        // write
        return 0;
    case 9:
        // Reset all -- takes a buffer full of socket numbers
        // 2: 1/ecs, 35/0, 24/bufaddr
        // 3: 60/bufwords
        buflen = reqp[3];
        if (buflen <= 0)
        {
            return RETINVREQ;
        }
        bufp = cpuAccessMem (reqp[2], buflen);
        if (bufp == NULL)
        {
            return RETINVREQ;
        }
        for (i = 0; i < buflen; i++)
        {
            socknum = *bufp;
            *bufp++ = 0;
            if (socknum != 0)
            {
                close (socknum);
            }
        }
        return RETOK;
            
    default:
        return RETINVREQ;
    }
    
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
    CpWord retval;
    
    DEBUGPRINT ("req %llo, reqp %p, *reqp %llo\n", req, reqp, *reqp);
    
    if (reqp == NULL)
    {
        return 0;
    }
    switch (*reqp & Mask12)
    {
    case 0:
        retval = envOp (req);
        break;
    case 1:
        retval = sockOp (req);
        break;
    default:
        retval = RETINVREQ;
    }
    DEBUGPRINT ("return value %llo\n", retval);

    return retval;
}

