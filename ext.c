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

#define RETOK       MINUS1
#define RETINVREQ   0
#define RETNOSOCK   1
#define RETNODATA   2
#define RETLONG     3
#define RETNULL     4
#define RETERROR(x) (1000 + x)
#define RETERRNO    RETERROR(errno)

#define MAXENV      500
#define MAXNET      4096

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
    char reqstr[11];
    char resultstr[MAXENV];
    char *p = reqstr;
    int i;
    char c;
    CpWord result;
    
    if (reqp == NULL)
    {
        return 0;
    }
//    DEBUGPRINT ("req %llo, reqp %p, reqp[1] %llo\n", req, reqp, reqp[1]);
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
**                              42/0, 6/function, 12/1
**                              6/mode, 22/0, 32/socknum
**                              60/reqdependent...
**
**                              Function codes:
**                                  0: reset
**                                  1: close ("shutdown" on the connection)
**                                  2: get socket
**                                  3: bind
**                                  4: listen
**                                  5: accept
**                                  6: connect
**                                  7: read
**                                  8: write
**                                  9: reset all
**
**                              For the "get socket" request, socknum must
**                              be zero, and the socket number obtained is 
**                              returned in word 1.  For "reset all" it is 
**                              unused.
**                              For all other request, socknum must be the 
**                              number of a socket obtained from "get socket" 
**                              or "accept".
**
**                              "socknum" is not actually a socket number;
**                              instead, it is a pointer to a NetFet.
**
**                              "mode" is 0 for text, 1 for binary.
**                              This applies only to read and write calls.
**                              Other values are reserved.
**
**                              See below for reqdependent fields.
**
**  Returns:        Operation status.  -1 for success, >= 0 for error.
**                  For host OS errors, status is 1000 + errno value.
**
**------------------------------------------------------------------------*/
static struct sockaddr_in addr;
static CpWord sockOp (CpWord req)
{
    // Check for a 5 word request buffer always (even though most requests require less)
    CpWord *reqp = cpuAccessMem (req, 5);
    NetFet *fet, *dataFet;
    int retval;
    int true_opt = 1;
    int mode;
    int buflen;
    CpWord *bufp;
    CpWord d;
    unsigned char *cp;
    int c;
    int ic, oc;
    int shift;
    int pc;
    int i, pcnt;
    socklen_t sl;
    int charset;
    char resultstr[MAXNET];
    
    if (reqp == NULL)
    {
        return 0;
    }
    mode = ((reqp[1] >> 54) & Mask6);
    fet = (NetFet *) (u32) (reqp[1] & 0xffffffff);
    DEBUGPRINT ("socket req %llo %llo %llo %llo %llo\n", 
                reqp[0], reqp[1], reqp[2], reqp[3], reqp[4]);
    switch ((*reqp >> 12) & Mask6)
    {
    case 0:
        // reset
        // no request dependent fields.
        if (fet != NULL)
        {
            dtCloseFet (fet, TRUE);
            free (fet);
            return RETOK;
        }
        return RETNOSOCK;
    case 1:
        // close
        // no request dependent fields.
        if (fet != NULL)
        {
            dtCloseFet (fet, FALSE);
            free (fet);
            return RETOK;
        }
        return RETNOSOCK;
    case 2:
        // socket (get a NetFet)
        // no request dependent fields.
        // Note that socket number (word 1) must be zero coming in,
        // and the NetFet address we obtained is returned there.
        if (fet == NULL)
        {
            fet = malloc (sizeof (*fet));
            if (fet == NULL)
            {
                return RETERROR (ENOMEM);
            }
            if ((u64)fet & 0xf000000000000000ULL)
            {
                printf ("HELP... FET pointers larger than 60 bits...\n");
            }
            if (dtInitFet (fet, MAXNET) != 0)
            {
                free (fet);
                return RETERRNO;
            }
            reqp[1] = (CpWord) (u32) fet;
            return RETOK;
        }
        return RETINVREQ;
    case 3:
        // bind + listen
        // request dependent fields:
        // 2: 28/0, 32/address
        // 3: 44/0, 16/port
        // 4: 44/0, 32/backlog
        if (fet == 0)
        {
            return RETNOSOCK;
        }
        if (dtBind (fet, htonl (reqp[2]), reqp[3], reqp[4]) != 0)
        {
            return RETERRNO;
        }
        return RETOK;
    case 4:
        // unused (was "listen")
        return RETINVREQ;
    case 5:
        // accept
        // no request dependent fields inbound.
        // On return, 3rd word is the data socket, if the accept worked.
        // The IP address is returned in the 4th word.
        if (fet == 0)
        {
            return RETNOSOCK;
        }
        reqp[2] = reqp[3] = 0;
        dataFet = dtAccept (fet, MAXNET);
        if (dataFet == NULL)
        {
            if (errno == EAGAIN)
            {
                return RETNODATA;
            }
            return RETERRNO;
        }
        if ((u64)dataFet & 0xf000000000000000ULL)
        {
            printf ("HELP... FET pointers larger than 60 bits...\n");
        }
        reqp[2] = (CpWord) (u32) dataFet;
        reqp[3] = ntohl (dataFet->from.s_addr);
        return RETOK;
    case 6:
        // connect
        // request dependent fields:
        // 2: 28/0, 32/address
        // 3: 44/0, 16/port
        if (fet == 0)
        {
            return RETNOSOCK;
        }
        if (dtConnect (fet, htonl (reqp[2]), reqp[3]) != 0)
        {
            return RETERRNO;
        }
        return RETOK;
    case 7:
        // read
        // request dependent fields:
        // 2: 1/ecs, 35/0, 24/bufaddr
        // 3: 60/bufwords
        // 4: 60/retbufchars
        // The amount of data read is returned in the 5th word of the
        // request buffer.  The amount is in 8-bit bytes for binary mode,
        // or 6-bit characters in text mode.
        //
        // mode (from word 1) is 1 for binary, 0 for PLATO text.
        // In text mode, multiple lines may be returned, with line
        // terminators in the usual form.  The character count includes
        // line terminator characters, including padding to the next 
        // word boundary.  If the last line is not a full line, the
        // character count will include all characters but there will
        // not be a line terminator at the end of the last line.
        // In other words, if char[count-1] != 0 then the last line
        // is a partial line.
        if (fet == 0)
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
        retval = dtRead (fet, 0);
        if (retval < 0)
        {
            return RETERRNO;
        }
        if (dtEmpty (fet))
        {
            return RETNODATA;
        }
        DEBUGPRINT ("dtRead %d, %d bytes buffered\n", retval, dtFetData (fet));
        c = dtReado (fet);
        if (mode == 1)
        {
            shift = 60 - 8;
        }
        else
        {
            shift = 60 - 6;
        }
        d = 0;
        oc = 0;
        for (ic = 0; ; ic++)
        {
            if (buflen == 0)
            {
                break;
            }
            if (mode == 1)
            {
                // binary
                if (shift == -4)
                {
                    if (buflen == 1)
                    {
                        // Quit if there's not room for both pieces
                        break;
                    }
                    d |= c >> 4;
                    *bufp++ = d;
                    buflen--;
                    d = ((CpWord) (c & 0x0f)) << 56;
                    shift = 56 - 8;
                }
                else
                {
                    d |= ((CpWord) (c)) << shift;
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
                if (c == '\r')
                {
                    c = dtReado (fet);
                    if (c < 0)
                    {
                        break;
                    }
                    continue;
                }
                else if (c == '\n')
                {
                    // End of line
                    *bufp++ = d;
                    buflen--;
                    d = 0;
                    shift = 60 - 6;
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
                    pc = asciiToPlatoString[c];
                    if (pc < 0)
                    {
                        // char with no PLATO equivalent
                        c = dtReado (fet);
                        if (c < 0)
                        {
                            break;
                        }
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
                c = dtReado (fet);
                if (c < 0)
                {
                    break;
                }
            }
        }
        if (buflen > 0 && shift < 60 - 8)
        {
            // There is a partially assembled word left, and we have room
            *bufp = d;
        }
        DEBUGPRINT ("chars to Cyber: %d\n", oc);
        reqp[4] = oc;
        return RETOK;
    case 8:
        // write
        // request dependent fields:
        // 2: 1/ecs, 35/0, 24/bufaddr
        // 3: 60/chars
        // chars is 8 bit bytes in binary mode, 6 bit chars in text mode.  
        // Char countincludes the end of line code (at least one 00 char) 
        // if last line ends in end of line.
        ic = reqp[3];
        if (ic > MAXNET)
        {
            return RETLONG;
        }
        if (mode == 0)
        {
            buflen = (ic + 9) / 10;
            shift = 60 - 6;
        }
        else
        {
            buflen = ic / 15;
            i = ic - buflen * 15;
            buflen = 2 * buflen;
            if (i > 7)
            {
                buflen += 2;
            }
            else if (i > 0)
            {
                buflen++;
            }
            shift = 60 - 8;
        }
        bufp = cpuAccessMem (reqp[2], buflen);
        if (bufp == NULL)
        {
            return RETINVREQ;
        }
        cp = (unsigned char *) resultstr;
        charset = 0;
        d = *bufp++;
        for (i = 0; i < ic; i++)
        {
            if (mode == 1)
            {
                // Binary mode
                if (shift == -4)
                {
                    *cp = d << 4;
                    d = *bufp++;
                    *cp++ |= (d >> 56) & 0x0f;
                    shift = 60 - 8 - 4;
                }
                else
                {
                    *cp++ = (d >> shift) & 0xff;
                    if (shift == 0)
                    {
                        shift = 60 - 8;
                        d = *bufp++;
                    }
                    else
                    {
                        shift -= 8;
                    }
                }
            }
            else
            {
                // PLATO string
                c = (d >> shift) & Mask6;
                //DEBUGPRINT ("word %p, %llo, char num %d, char %d, shift %d, ", bufp, d, i, c, shift);
                shift -= 6;
                if (shift < 0)
                {
                    d = *bufp++;
                    shift = 60 - 6;
                }
                if (c == 0)
                {
                    // end of line
                    *cp++ = '\n';
                    // Adjust input char count by number of pad zeroes skipped
                    i += (shift + 6) / 6;
                    // Advance to the next word (skip pad zeroes in this word)
                    shift = 60 - 6;
                    charset = 0;
                    d = *bufp++;
                    //DEBUGPRINT ("newline, now word %p, %llo, chars left %d\n", bufp, d, i);
                }
                else
                {
                    // TODO: postfix (autobackspacing) accent handling;
                    // those need to translate the preceding letter.
                    if (c == 070)
                    {
                        charset |= 1;
                    }
                    else if (c == 076)
                    {
                        charset |= 2;
                    }
                    else
                    {
                        c = platoStringToAscii[charset][c];
                        //DEBUGPRINT ("charset %d, ascii %d\n", charset, c);
                        charset = 0;
                        if (c != 001)
                        {
                            *cp++ = c;
                        }
                    }
                }
            }
        }
        oc = cp - (unsigned char *) resultstr;
        dtSend (fet, resultstr, oc);
        DEBUGPRINT ("dtSend (%d, ptr, %d)\n", fet, oc);
        return RETOK;
    case 9:
        // Reset all -- takes a buffer full of socket numbers
        // request dependent fields:
        // 2: 1/ecs, 35/0, 24/bufaddr
        // 3: 60/bufwords
        // Note that fet (word 1) is not used.
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
            fet = (NetFet *) (u32) *bufp;
            *bufp++ = 0;
            if (fet != 0)
            {
                DEBUGPRINT ("closing socket %d at index %d\n", fet, i + 1);
                dtCloseFet (fet, TRUE);
                free (fet);
            }
        }
        return RETOK;
            
    default:
        return RETINVREQ;
    }
    
}

/*--------------------------------------------------------------------------
**  Purpose:        Get information about a terminal connection
**
**  Parameters:     Name        Description.
**                  req         Pointer to request (1/ecsflag, 35/, 24/address)
**
**                              48/0, 12/2
**                              60/port number
**                              60/port type
**
**                              Port type is 0 for NIU, 1 for NPU.
**                              Port number is 0 based, relative to
**                              the ports maintained by NIU and NPU.
**
**  Returns:        IP address for connection (right justified, network order)
**                  0 if no connection
**                  -1 if error (port number or port type out of range)
**
**------------------------------------------------------------------------*/
static CpWord termConnOp (CpWord req)
{
    CpWord *reqp = cpuAccessMem (req, 3);
    
    if (reqp == NULL)
    {
        return 0;
    }
//    DEBUGPRINT ("req %llo, reqp %p, reqp[1] %llo\n", req, reqp, reqp[1]);
    switch (reqp[2])
    {
    case 0:
        return niuConn (reqp[1]);
    case 1:
        return npuConn (reqp[1]);
    default:
        return MINUS1;
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
    case 2:
        retval = termConnOp (req);
        break;
    default:
        retval = RETINVREQ;
    }
    if (retval != RETNODATA)
    {
        DEBUGPRINT ("req %llo, reqp %p, *reqp %llo\n", req, reqp, *reqp);
        DEBUGPRINT ("return value %llo\n", retval);
    }
    return retval;
}

