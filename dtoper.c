/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: dtoper.c
**
**  Description:
**      Provide operator interface for CDC 6600 emulation.  This file contains
**      the common parsing and display code for the user interface side.
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
#include "operator.h"
#include "proto.h"
#if defined(_WIN32)
#include <windows.h>
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
#define OpCmdSize   64
#define OpStatSize  64
#define OpDataSize  257
#define CmdX        0020
#define CmdY        0060
#define StatusX     01040
#define StatusY     0740
#define StatusLines 32
#define NetBufSize  1024

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define opWidth(x) ((x == FontSmall) ? smallFontWidth : mediumFontWidth)

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct opCmd
{
    char            *name;               /* command name */
    void            (*handler)(char *cmdParams);
} OpCmd;

typedef struct opMsg
    {
    u16     x;
    u16     y;
    u8      fontSize;
    u8      bold;
    char    *text;
    } OpMsg;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void opSendString (OpMsg *m, bool blank);
static void opRequest(void);
static int opScanCmd (void);

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
bool emulationActive = TRUE;

/*
**  ----------------
**  Public Functions
**  ----------------
*/
extern char opWindowInput(void);
extern void windowInit(void);
extern void windowClose(void);
extern void windowSendString (int x, int y, int font,
                              bool bold, const char *str, bool blank);
extern int windowGetOperFontWidth(int font);

void opDisplay(void);

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static NetFet fet;
static char cmdBuf[OpCmdSize + 1];
static int cmdLen = 0;
static char userKey;
static int ppKeyIn;
static char nextKey = 0;
static u8 dataBuf[OpDataSize + 1];

static char **syntax;
static int syntaxCnt = 0;
static OpMsg *messages;
static int msgCount = 0;

static char *status[StatusLines];

static OpMsg cmdEcho = { CmdX, CmdY, 0020, 0, cmdBuf };
static OpMsg errmsg = { 0020, 0014, 0020, 0, NULL };   // pointer filled in
static char errmsgBuf[OpCmdSize + 1];

static char msgBuf[80];
static int smallFontWidth;
static int mediumFontWidth;
static bool initDone = FALSE;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

int main (int argc, char **argv)
    {
    char c;
    int port;
    int i, j;
    OpMsg *msgp;
    u8 *p;
    char *cp;
    int true_opt = 1;
    
    if (argc > 2)
        {
        printf ("usage: dtoper [ portnum ]\n");
        exit (1);
        }
    if (argc > 1)
        {
        port = atoi (argv[1]);
        }
    else
        {
        port = DefOpPort;
        }
        
    dtInitFet (&fet, NetBufSize);
    
    if (dtConnect (&fet.connFd, "localhost", port) < 0)
        {
        exit (1);
        }

#if defined(_WIN32)
    ioctlsocket (fet.connFd, FIONBIO, &true_opt);
#else
    fcntl (fet.connFd, F_SETFL, O_NONBLOCK);
#endif
#ifdef __APPLE__
    setsockopt (fet.connFd, SOL_SOCKET, SO_NOSIGPIPE,
                (char *)&true_opt, sizeof(true_opt));
#endif

#if defined(_WIN32)
    /*
    **  Get our instance
    */
    hInstance = GetModuleHandle(NULL);
#endif

    windowInit ();
    
    while (emulationActive)
        {
        i = dtRead (&fet, 30);
        if (i < 0)
            {
#if defined(_WIN32)
            Sleep (30);
#else
            usleep (30000);
#endif
            }
        else
            {
            for (;;)
                {
                i = dtReadtlv (&fet, dataBuf, OpDataSize);
                if (i < 0)
                    {
                    if (i == -2)
                        {
                        opSetMsg ("$Disconnected");
                        }
                    break;
                    }
                p = dataBuf + 2;
                p[i] = '\0';        /* put in a string terminator */
                switch (dataBuf[0])
                    {
                case OpText:
                    /*
                    **  The fixed text comes across with the data in the
                    **  following format:
                    **
                    **      x coordinate, 2 bytes, little endian.
                    **      y coordinate, 2 bytes, little endian.
                    **      font size, one byte
                    **      bold flag, one byte
                    **      text, the remaining bytes (not null terminated)
                    */
                    messages = realloc (messages, (++msgCount) * sizeof (OpMsg));
                    msgp = &messages[msgCount - 1];
                    j = *p++;
                    j += *p++ << 8;
                    msgp->x = j;
                    j = *p++;
                    j += *p++ << 8;
                    msgp->y = j;
                    msgp->fontSize = *p++;
                    msgp->bold = *p++;
                    i -= 6;             /* adjust i to string length */
                    if (i == 0)
                        {
                        msgp->text = NULL;
                        }
                    else
                        {
                        msgp->text = (char *) malloc (i + 1);
                        strcpy (msgp->text, p);
                        }
                    break;
                case OpSyntax:
                    syntax = realloc (syntax, (++syntaxCnt) * sizeof (char *));
                    cp = syntax[syntaxCnt - 1] = (char *) malloc (i + 1);
                    strcpy (cp, p);
                    break;
                case OpStatus:
                    /*
                    **  Data format is:
                    **      line number, one byte
                    **      text, the remaining bytes
                    */
                    j = *p++;
                    if (i > OpStatSize + 1 || i == 0 || j >= StatusLines)
                        {
                        break;
                        }
                    i--;
                    if (status[j] == NULL)
                        {
                        status[j] = malloc(OpStatSize + 1);
                        }
                    cp = status[j];
                    strncpy (cp, p, OpStatSize);
                    break;
                case OpReply:
                    /*
                    **  A reply causes the following actions:
                    **      Post the response to the message line.
                    **      If the response is not an error (not bold)
                    **      erase the command.
                    **      In any case, mark the command "not complete".
                    */
                    opSetMsg (p);
                    if (!errmsg.bold)
                        {
                        cmdBuf[0] = '\0';
                        cmdLen = 0;
                        }
                    cmdEcho.bold = FALSE;
                    break;
                case OpInitialized:
                    initDone = TRUE;
                    break;
                    }
                }
            }
        /*
        **  We've processed all pending data from the system;
        **  now look for keyboard data -- but only after we have
        **  received all the initialization data.
        */
        if (initDone)
            {
            c = opWindowInput ();
            if (c != 0)
                {
                userKey = c;
                }
            if (userKey != 0)
                {
                do
                    {
                    opRequest ();
                    } while (nextKey != 0);
                }
            }
        opDisplay ();
        
        }
    windowClose ();
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Display the operator window content
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void opDisplay(void)
    {
    int i;

    smallFontWidth = windowGetOperFontWidth(FontSmall);
    mediumFontWidth = windowGetOperFontWidth(FontMedium);

    if (messages != NULL)
        {
        for (i = 0; i < msgCount; i++)
            {
            opSendString (&messages[i], FALSE);
            }
        }
    
    for (i = 0; i < StatusLines; i++)
        {
        if (status[i] != NULL)
            {
            windowSendString (StatusX, StatusY - 020 * i, 010, 
                              FALSE, status[i], TRUE);
            }
        }

    opSendString (&cmdEcho, TRUE);
    opSendString (&errmsg, TRUE);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Process a keystroke.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opRequest(void)
    {
    int i;

    if (nextKey)
        {
        ppKeyIn = nextKey;
        nextKey = 0;
        }
    else
        {
        ppKeyIn = userKey;
        userKey = '\0';
        }
    if (ppKeyIn <= 0) 
        {
        return;
        }
    opSetMsg (NULL);
    if (ppKeyIn == '\r')
        {
        ppKeyIn = '\n';
        }
    if (ppKeyIn == '[' || ppKeyIn == ('U' & 037))
        {
        cmdBuf[0] = '\0';
        cmdLen = 0;
        cmdEcho.bold = FALSE;
        }
    else if (ppKeyIn == 0177 || ppKeyIn == '\b')
        {
        if (cmdLen != 0)
            {
            cmdBuf[--cmdLen] = '\0';
            cmdEcho.bold = FALSE;
            }
        }
    else
        {
        cmdBuf[cmdLen++] = ppKeyIn;
        cmdBuf[cmdLen] = '\0';
        i = opScanCmd ();
        if (i < 0)
            {
            cmdBuf[--cmdLen] = '\0';
            }
        else if (ppKeyIn == '\n')
            {
            /*
            **  The command is complete, send it off.
            */
            cmdBuf[--cmdLen] = '\0';
            if (strcmp (cmdBuf, "END.") == 0)
                {
                emulationActive = FALSE;
                return;
                }
            dtSendTlv (fet.connFd, OpCommand, strlen (cmdBuf), cmdBuf);
            return;
            }
        else 
            {
            if (i == '\n')
                {
                cmdEcho.bold = TRUE;
                i = 0;
                }
            else
                {
                cmdEcho.bold = FALSE;
                }
            nextKey = i;
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set operator message
**
**  Parameters:     Name        Description.
**                  p           string pointer
**                              If string begins with $, make it bold
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void opSetMsg (const char *p)
    {
    if (p != NULL && *p == '$')
        {
        p++;
        errmsg.bold = TRUE;
        }
    else
        {
        if (p == NULL)
            {
            p = " ";
            }
        errmsg.bold = FALSE;
        }
    strncpy (errmsgBuf, p, OpCmdSize);
    errmsg.text = errmsgBuf;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a string to the console.
**
**  Parameters:     Name        Description.
**                  m           message structure pointer
**                  blank       TRUE if spaces should be output after string
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static void opSendString (OpMsg *m, bool blank)
    {
    if (m->text == NULL)
        {
        windowSendString (m->x, m->y, m->fontSize, m->bold, " ", blank);
        }
    else
        {
        windowSendString (m->x, m->y, m->fontSize, m->bold, m->text, blank);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Scan the command table to match against a partial command
**
**  Parameters:     Name        Description.
**                  none
**
**  Returns:        -1 for bad string.
**                  0 for multiple matches, different next character.
**                  c > 0 means next legal character is always c.
**
**------------------------------------------------------------------------*/
static int opScanCmd (void)
    {
    char *p, *b;
    int match = -1;
    int i;
    
    for (i = 0; i < syntaxCnt; i++)
        {
        for (b = cmdBuf, p = syntax[i]; ; b++)
            {
            if (*b == '\0')
                {
                if (match == -1 || match == *p)
                    {
                    match = *p;
                    if (match == 'x' || match == '7')
                        {
                        match = 0;
                        }
                    }
                else
                    {
                    match = 0;
                    }
                break;
                }
            else if (*p == '7')     // octal digit match
                {
                if (*b < '0' || *b > '7')
                    {
                    break;          // no match on this pattern
                    }
                if (b[1] != '\0' &&
                    (b[1] < '0' || b[1] > '7'))
                    {
                    p++;        // if next is not digit, advance pattern
                    }
                }
            else if (*p == 'x')
                {
                // 'x' matches any character other than the argument
                // separator (comma).
                if (*b == ',')
                    {
                    break;
                    }
                if (b[1] != '\0' &&
                    b[1] == p[1])
                    {
                    p++;        // if next input == next pattern, advance
                    }
                }
            else if (*p == '\0')
                {
                break;
                }
            else
                {
                if (isupper (*p))
                    {
                    *b = toupper (*b);
                    }
                if (*b != *p)
                    {
                    break;          // no match on this pattern
                    }
                p++;
                }
            }   
        }
    
    return match;
    }

/*---------------------------  End Of File  ------------------------------*/
