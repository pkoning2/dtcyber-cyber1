/*--------------------------------------------------------------------------
**
**  Copyright (c) 2006, Paul Koning (see license.txt)
**
**  Name: ext.c
**
**  Description:
**      Access to the environment outside the emulator
**
**--------------------------------------------------------------------------
*/
#define DEBUG 1
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

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define RETOK       MINUS1
#define RETINVREQ   0
#define RETNOSOCK   1
#define RETNODATA   2
#define RETLONG     3
#define RETNULL     4
#define RETERROR(x) (1000 + x)
#define RETERRNO    RETERROR(dtErrno)

#define MAXENV      500
#define MAXNET      4096

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define socktofet(x) (extPorts.portVec[(x) - 1])
#define fettosock(x) ((x)->psIndex + 1)

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static CpWord envOp (CPUVARGS1 (CpWord req));
static CpWord sockOp (CPUVARGS1 (CpWord req));
static CpWord termConnOp (CPUVARGS1 (CpWord req));

/*
**  ----------------
**  Public Variables
**  ----------------
*/
long extSockets;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static NetPortSet extPorts;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialize external operations module
**
**  Parameters:     None
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void initExt (void)
{
    extPorts.maxPorts = extSockets;
    extPorts.kind = "socket";
    extPorts.ringSize = extPorts.sendRingSize = MAXNET;
    
    /*
    **  Initialize the portset, which among other things allocates
    **  a vector of NetFet pointers.
    */
    dtInitPortset (&extPorts);
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
CpWord extOp (CPUVARGS1 (CpWord req))
{
    CpWord *reqp = cpuAccessMem (CPUARGS2 (req, 1));
    CpWord retval;
    
    if (reqp == NULL)
    {
        return 0;
    }
    switch (*reqp & Mask12)
    {
    case 0:
        retval = envOp (CPUARGS1 (req));
        break;
    case 1:
        retval = sockOp (CPUARGS1 (req));
        break;
    case 2:
        retval = termConnOp (CPUARGS1 (req));
        break;
    case 3:
        retval = pniOp (CPUARGS1 (req));
        break;
    default:
        retval = RETINVREQ;
    }
    if (retval != RETNODATA)
    {
        DEBUGPRINT ("req %llo, reqp %p, reqp[]  %llo %llo %llo %llo %llo\n",
                    req, reqp, reqp[0], reqp[1], reqp[2], reqp[3], reqp[4]);
        DEBUGPRINT ("return value %llo\n", retval);
    }
    return retval;
}

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

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
static CpWord envOp (CPUVARGS1 (CpWord req))
{
    CpWord *reqp = cpuAccessMem (CPUARGS2 (req, 2));
    char reqstr[11];
    char resultstr[MAXENV];
    char *p = reqstr;
    int i;
    int c;
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
**                                  2: unused (reserved)
**                                  3: bind + listen
**                                  4: accept
**                                  5: connect
**                                  6: read
**                                  7: write
**                                  8: reset multiple sockets
**                                  9: reset all ext sockets
**                                 10: get socket buffer info
**
**                              For the "bind/listen" and "connect"
**                              requests, socknum must be zero, and
**                              the socket number obtained is returned
**                              in word 1.  For "accept", the listen
**                              socket number is passed in, and the
**                              resulting data socket number is passed
**                              out in word 2.  For "reset all" the
**                              socket number is unused.  For all
**                              other requests, socknum must be the
**                              number of a socket obtained from
**                              "connect" or "accept".
**
**                              "socknum" is not actually a socket number;
**                              instead, it is an index into our NetPortSet.
**
**                              "mode" is a bit mask:
**                                  4/, 1/no partials, 1/binary
**                                "binary" means 8-bit binary data is
**                                  transferred (otherwise translate ASCII
**                                  to display code and store in line format)
**                                  Applies to read and write functions.
**                                "no partials" means only full lines (text)
**                                  or full words (binary) are read.
**                                  Applies only to read functions.
**
**                              See below for reqdependent fields.
**
**  Returns:        Operation status.  -1 for success, >= 0 for error.
**                  For host OS errors, status is 1000 + errno value.
**                  Exception: for code 10, see below.
**
**------------------------------------------------------------------------*/
static CpWord sockOp (CPUVARGS1 (CpWord req))
{
    // Check for a 5 word request buffer always (even though most requests require less)
    CpWord *reqp = cpuAccessMem (CPUARGS2 (req, 5));
    NetFet *fet = NULL, *dataFet;
    int socknum;
    int retval;
    int mode;
    int modeflags;
    int buflen;
    CpWord *bufp;
    CpWord d;
    unsigned char *cp;
    int prev_c, c;
    int ic, oc;
    int shift;
    int pc;
    int i, idx, pcnt;
    int charset;
    char resultstr[MAXNET];
    int out, prev_out;
    int prev_oc;
    
    if (reqp == NULL)
    {
        return 0;
    }
    mode = ((reqp[1] >> 54) & Mask6);
    modeflags = mode >> 1;
    mode &= 1;
    socknum = reqp[1] & 0xffffffff;
    if (socknum > extSockets)
    {
        return RETINVREQ;
    }
    if (socknum != 0)
    {
        fet = socktofet (socknum);
    }
    
#if 0
    DEBUGPRINT ("socket req %llo %llo %llo %llo %llo\n", 
                reqp[0], reqp[1], reqp[2], reqp[3], reqp[4]);
#endif
    switch ((*reqp >> 12) & Mask6)
    {
    case 0:
        // reset
        // no request dependent fields.
        if (fet != NULL)
        {
            dtClose (fet, TRUE);
            return RETOK;
        }
        return RETNOSOCK;
    case 1:
        // close
        // no request dependent fields.
        if (fet != NULL)
        {
            dtClose (fet, FALSE);
            return RETOK;
        }
        return RETNOSOCK;
    case 2:
        return RETINVREQ;       // formerly Get Socket, now unused
    case 3:
        // bind + listen
        // request dependent fields:
        // 2: 28/0, 32/address
        // 3: 44/0, 16/port
        // 4: 44/0, 32/backlog
        if (fet != NULL)
        {
            return RETINVREQ;
        }
        fet = dtBind (&extPorts, htonl (reqp[2]), reqp[3], reqp[4]);
        if (fet == NULL)
        {
            return RETERRNO;
        }
        reqp[1] = fettosock (fet);
        return RETOK;
    case 4:
        // accept
        // no request dependent fields inbound.
        // On return, 3rd word is the data socket, if the accept worked.
        // The IP address is returned in the 4th word.
        if (fet == NULL)
        {
            return RETNOSOCK;
        }
        if (!fet->listen)
        {
            return RETINVREQ;
        }        
        reqp[2] = reqp[3] = 0;
        dataFet = dtAccept (fet, &extPorts);
        if (dataFet == NULL)
        {
            if (dtErrno == EAGAIN)
            {
                return RETNODATA;
            }
            return RETERRNO;
        }
        dataFet->ownerInfo = fet->ownerInfo;
        reqp[2] = fettosock (dataFet);
        reqp[3] = ntohl (dataFet->from.s_addr);
        return RETOK;
    case 5:
        // connect
        // request dependent fields:
        // 2: 28/0, 32/address
        // 3: 44/0, 16/port
        if (fet != NULL)
        {
            return RETINVREQ;
        }
        fet = dtConnect (&extPorts, htonl (reqp[2]), reqp[3]);
        if (fet == NULL)
        {
            return RETERRNO;
        }
        reqp[1] = fettosock (fet);
        return RETOK;
    case 6:
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
        // If the "no partials" bit is set in word 1, then normally only
        // full lines (text mode) or full words (binary mode) are returned.
        // For binary mode, that means an even number of words since bytes
        // are packed 7.5 per word.  The exceptions are:
        //  - If the connection has closed, the final data is delivered
        //    even if it is a partial line or word
        //  - In text mode, if a complete line is waiting to be received,
        //    but the buffer is not big enough for the line, part of it
        //    will be delivered, and the rest is available for the next
        //    receive call.  Note that if multiple lines are waiting,
        //    and there is space for some but not all lines, the full lines
        //    that fit will be delivered.  So the partial line case applies
        //    only if the line that's too long is the first line waiting.
        if (fet == NULL)
        {
            return RETNOSOCK;
        }
        buflen = reqp[3];
        if (buflen <= 0)
        {
            return RETINVREQ;
        }
        bufp = cpuAccessMem (CPUARGS2 (reqp[2], buflen));
        // DEBUGPRINT ("read mode %d to %06llo length %d\n", mode, reqp[2], buflen);
        reqp[4] = 0;
        if (bufp == NULL)
        {
            return RETINVREQ;
        }

        prev_c = prev_out = out = -1;
        c = dtReadoi (fet, &out);
        if (c < 0)
        {
            if (!dtConnected (fet))
            {
                dtClose (fet, TRUE);
            }
            return RETNODATA;
        }
        
        if (mode == 1)
        {
            shift = 60 - 8;
        }
        else
        {
            shift = 60 - 6;
        }
        d = 0;
        oc = prev_oc =  0;
        
        for (ic = 0; ; ic++)
        {
            // DEBUGPRINT (" byte %03o (%02x) shift %d\n", c, c, shift);
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
                    DEBUGPRINT (" %020llo\n", d);
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
                        DEBUGPRINT (" %020llo\n", d);
                        buflen--;
                        d = 0;
                        shift = 60 - 8;
                        prev_oc = oc + 1;
                        prev_out = out;
                    }
                    else
                    {
                        shift -= 8;
                    }
                }
                oc++;
                if (buflen == 0)
                {
                    break;
                }
                prev_c = c;
                c = dtReadoi (fet, &out);
                if (c < 0)
                {
                    if (!dtConnected (fet))
                    {
                        dtClose (fet, TRUE);
                    }
                    break;
                }
            }
            else
            {
                if (buflen == 0)
                {
                    break;
                }
                // PLATO text mode
                if (c == '\n' && prev_c == '\r')
                {
                    // treat \r\n as a single newline
                    prev_c = c;
                    c = dtReadoi (fet, &out);
                    if (c < 0)
                    {
                        if (!dtConnected (fet))
                        {
                            dtClose (fet, TRUE);
                        }
                        break;
                    }
                    continue;
                }
                else if (c == '\r' || c == '\n')
                {
                    // \r (optionally followed by \n) or \n by itself is newline
                    *bufp++ = d;
                    DEBUGPRINT (" %020llo\n", d);
                    buflen--;
                    d = 0;
                    shift = 60 - 6;
                    if (oc % 10 == 9)
                    {
                        if (buflen > 0)
                        {
                            *bufp++ = 0;
                            DEBUGPRINT (" %020llo\n", 0);
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
                    prev_oc = oc;
                    prev_out = out;
                }
                else
                {
                    pc = asciiToPlatoString[c];
                    if (pc < 0)
                    {
                        // char with no PLATO equivalent
                        prev_c = c;
                        c = dtReadoi (fet, &out);
                        if (c < 0)
                        {
                            if (!dtConnected (fet))
                            {
                                dtClose (fet, TRUE);
                            }
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
                            DEBUGPRINT (" %020llo\n", d);
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
                prev_c = c;
                c = dtReadoi (fet, &out);
                if (c < 0)
                {
                    if (!dtConnected (fet))
                    {
                        dtClose (fet, TRUE);
                    }
                    break;
                }
            }
        }
        if (buflen > 0 && shift < 60 - 8)
        {
            // There is a partially assembled word left, and we have room
            *bufp = d;
            DEBUGPRINT (" %020llo\n", d);
        }
        if (modeflags & 1)
        {
            // whole wordpairs/lines only
            oc = prev_oc;
            out = prev_out;
            if (oc == 0)
            {
                // If the connection is gone and the data that's
                // left is less than what we want to get right now,
                // return the error for the lost connection.
                if (fet->connFd == 0)
                {
                    return RETNULL;
                }
                return RETNODATA;
            }
        }
        DEBUGPRINT ("chars to Cyber: %d\n", oc);
        reqp[4] = oc;
        // Consume the characters that we processed, if any
        if (out >= 0)
        {
            dtUpdateOut (fet, out);
        }
        
        return RETOK;
    case 7:
        // write
        // request dependent fields:
        // 2: 1/ecs, 35/0, 24/bufaddr
        // 3: 60/chars
        // chars is 8 bit bytes in binary mode, 6 bit chars in text mode.  
        // Char count includes the end of line code (at least one 00 char) 
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
        bufp = cpuAccessMem (CPUARGS2 (reqp[2], buflen));
        DEBUGPRINT ("write mode %d from %06llo length %lld bytes (%d words)\n", mode, reqp[2], reqp[3], buflen);
        if (bufp == NULL)
        {
            return RETINVREQ;
        }
        cp = (unsigned char *) resultstr;
        charset = 0;
        d = *bufp++;
        DEBUGPRINT (" %020llo\n", d);
        for (i = 0; i < ic; i++)
        {
            if (mode == 1)
            {
                // Binary mode
                if (shift == -4)
                {
                    *cp = d << 4;
                    d = *bufp++;
                    DEBUGPRINT (" %020llo\n", d);
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
                        if (i < ic - 1)
                            DEBUGPRINT (" %020llo\n", d);
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
                    // end of line.  Use CRLF because that's the RFC convention
                    *cp++ = '\r';
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
        retval = dtSend (fet, resultstr, oc);
        DEBUGPRINT ("dtSend (%d, ptr, %d) returned %d\n", fet, oc, retval);
        if (retval > 0)
        {
            if (retval == EAGAIN)
            {
                return RETNODATA;
            }
            else
            {
                return RETERROR (retval);
            }
        }
        return RETOK;
    case 8:
        // Reset multiple -- takes a buffer full of socket numbers
        // request dependent fields:
        // 2: 1/ecs, 35/0, 24/bufaddr
        // 3: 60/bufwords
        // Note that fet (word 1) is not used.
        buflen = reqp[3];
        if (buflen <= 0)
        {
            return RETINVREQ;
        }
        bufp = cpuAccessMem (CPUARGS2 (reqp[2], buflen));
        if (bufp == NULL)
        {
            return RETINVREQ;
        }
        for (i = 0; i < buflen; i++)
        {
            idx = *bufp;
            *bufp++ = 0;
            if (idx != 0)
            {
                if (idx <= extSockets)
                {
                    fet = socktofet (idx);                
                    DEBUGPRINT ("closing fet %p at index %d\n", fet, i + 1);
                    dtClose (fet, TRUE);
                }
            }
        }
        return RETOK;
    case 9:
        // Reset all -- reset every socket controlled by ext.
        // No request dependent fields.
        for (i = 1; i <= extSockets; i++)
        {
            fet = socktofet (i);                
            DEBUGPRINT ("closing fet %p\n", fet);
            dtClose (fet, TRUE);
        }
        return RETOK;
    case 10:
        // get socket info
        // no request dependent fields.
        // This one has nonstandard return values: error is -1, success
        // is 24/, 18/sendcount, 18/receivecount
        if (fet != NULL)
        {
            return (dtSendData (fet) << 18) + dtFetData (fet);
        }
        return MINUS1;
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
**                              Port type is 0 for NIU, 1 for PNI.
**                              Port number is 0 based, relative to
**                              the ports maintained by NIU and PNI.
**
**  Returns:        IP address for connection (right justified, network order)
**                  0 if no connection
**                  -1 if error (port number or port type out of range)
**
**------------------------------------------------------------------------*/
static CpWord termConnOp (CPUVARGS1 (CpWord req))
{
    CpWord *reqp = cpuAccessMem (CPUARGS2 (req, 3));
    
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
        return pniConn (reqp[1]);
    default:
        return MINUS1;
    }
}

/*---------------------------  End Of File  ------------------------------*/


