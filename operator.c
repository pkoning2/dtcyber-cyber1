/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: operator.c
**
**  Description:
**      Provide operator interface for CDC 6600 emulation. This is required
**      to enable a human "operator" to change tapes, remove paper from the
**      printer, shutdown etc.
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

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define OpCmdSize 64
#define BoldMediumRepaints 8
#define TwinkleRate 2
#define CmdX 0020
#define CmdY 0100
/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

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
    int     x;
    int     y;
    int     fontSize;
    char    *text;
    } OpMsg;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
char *opGetString(char *inStr, char *outStr, int outSize);
static void opSendString (OpMsg *m);
static int opScanCmd (void);

static void opCmdEnd(char *cmdParams);
static void opCmdShutdown(char *cmdParams);
static void opCmdLoad(char *cmdParams);
static void opCmdUnload(char *cmdParams);
static void opDumpCpu (char *cmdParams);
static void opDumpPpu (char *cmdParams);
static void opDisPpu (char *cmdParams);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
bool opActive = FALSE;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static char cmdBuf[OpCmdSize];
static char *syntax[] = 
    /*
    ** Simple command decode table with DSD style completion.
    ** 7 means one or more octal digits.
    ** x means any character other than comma (case preserved).
    ** anything else means that character (case insensitive if letter).
    ** command is complete if matched to end of some entry.
    */
    {
    "END.\n",
    "SHUTDOWN.\n",
    "LOAD,7,7,x,W.\n",
    "LOAD,7,7,x\n",
    "UNLOAD,7,7.\n",
    "DUMP,CPU.\n",
    "DUMP,PPU7.\n",
    "DISASSEMBLE,PPU7.\n",
    NULL,
    };
static OpCmd decode[] = 
    /*
    ** Simple command decode table.
    ** This contains the portion of the command string that 
    ** distinguishes this command from others; that may be just
    ** the keyword, or it may be that plus some of the arguments.
    ** The action routine is handed the rest of the command line
    ** (everything past what matched here).
    ** Note that the matches must be unique -- the first match is used.
    */
    {
    "END.",                     opCmdEnd,
    "SHUTDOWN.",                opCmdShutdown,
    "LOAD,",                    opCmdLoad,
    "UNLOAD,",                  opCmdUnload,
    "DUMP,CPU",                 opDumpCpu,
    "DUMP,PPU",                 opDumpPpu,
    "DISASSEMBLE,PPU",          opDisPpu,
    NULL,                       NULL
    };

static OpMsg msg[] =
    { { 0120, 0700, 0020, "OPERATOR INTERFACE" },
      { 0760 - (sizeof(DtCyberVersion) * 010), 0760, 0010, DtCyberVersion },
      { 0020, 0640, 0010, "LOAD,CH,EQ,FILE    Load file for ch/eq, read-only." },
      { 0020, 0620, 0010, "LOAD,CH,EQ,FILE,W. Load file for ch/eq, read/write." },
      { 0020, 0600, 0010, "UNLOAD,CH,EQ.      Unload ch/eq." },
      { 0020, 0560, 0010, "DUMP,CPU.          Dump CPU state." },
      { 0020, 0540, 0010, "DUMP,PPUNN.        Dump specified PPU state." },
      { 0020, 0520, 0010, "DISASSEMBLE,PPUNN. Disassemble specified PPU." },
      { 0020, 0460, 0010, "END.               End operator mode." },
      { 0020, 0440, 0010, "SHUTDOWN.          Close DtCyber." },
      { CmdX, CmdY, 0020, cmdBuf },  // echo, MUST be last
      { 0, 0, 0, NULL },
    };
static OpMsg errmsg = { 0020, 0040, 0020, NULL };   // pointer filled in
static bool msgBold;
static bool complete;
static char msgBuf[80];
/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Operator interface initialisation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void opInit(void)
    {
    }


/*--------------------------------------------------------------------------
**  Purpose:        Operator interface activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void opRequest(void)
    {
    OpCmd *cp;
    char nextKey;
    int cmdLen;
    OpMsg *m;
    int i, j;
    int twRate = 0;     // number of repaints between twinkles
    int twPos = 0;      // Twinkle position mod 4

    cmdBuf[0] = '\0';
    cmdLen = 0;
    while (opActive)
        {
        for (m = msg; m->text; m++)
            {
            opSendString (m);
            }
        // Do this right after displaying all the message lines,
        // because the command line echo buffer is the last line
        // displayed so now is the time to highlight it.
        if (complete)
            {
            windowSetY (CmdY);
            windowSetFont (FontMedium);
            for (i = 0; i < BoldMediumRepaints - 1; i++)
                {
                for (j = twPos; j < cmdLen; j += 4)
                    {
                    windowSetX (CmdX + (FontMedium * j));
                    windowQueue (cmdBuf[j]);
                    }
                }
            }
        if (errmsg.text != NULL)
            {
            for (i = 0; i < ((msgBold) ? BoldMediumRepaints : 1); i++)
                {
                opSendString (&errmsg);
                }
            }
        if (nextKey)
            {
            ppKeyIn = nextKey;
            nextKey = 0;
            windowCheckOutput();
            }
        else
            {
            windowGetChar();
            }
        if (--twRate < 0)
            {
            twRate = TwinkleRate;
            twPos = (twPos + 1) & 3;
            }
        if (ppKeyIn == 0) 
            {
            usleep (RefreshInterval / 2);
            continue;
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
            complete = FALSE;
            }
        else if (ppKeyIn == 0177 || ppKeyIn == '\b')
            {
            if (cmdLen != 0)
                {
                cmdBuf[--cmdLen] = '\0';
                complete = FALSE;
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
                **  Find the command handler.
                */
                /*
                **  Complete command, find its processor
                */
                cmdBuf[--cmdLen] = '\0';
                for (cp = decode; cp->name != NULL; cp++)
                    {
                    j = strlen (cp->name);
                    if (strncmp(cp->name, cmdBuf, j) == 0)
                        {
                        cp->handler(cmdBuf + j);
                        if (!msgBold)
                            {
                            // Command did not produce an error message,
                            // so erase it because we're done with it.
                            cmdBuf[0] = '\0';
                            cmdLen = 0;
                            }
                        complete = FALSE;
                        break;
                        }
                    }
                if (cp->name == NULL)
                    {
                    opSetMsg ("$INVALID COMMAND");
                    }
                }
            else 
                {
                if (i == '\n')
                    {
                    complete = TRUE;
                    i = 0;
                    }
                else
                    {
                    complete = FALSE;
                    }
                nextKey = i;
                }
            }
        usleep (RefreshInterval / 2);
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
void opSetMsg (char *p)
    {
    if (p != NULL && *p == '$')
        {
        p++;
        msgBold = TRUE;
        }
    else
        {
        msgBold = FALSE;
        }
    errmsg.text = p;
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Send a string to the console.
**
**  Parameters:     Name        Description.
**                  m           message structure pointer
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static void opSendString (OpMsg *m)
    {
    char *c;
    
    windowSetX (m->x);
    windowSetY (m->y);
    windowSetFont (m->fontSize);
    for (c = m->text; *c; c++)
        {
        windowQueue (*c);
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
    char **c;
    int match = -1;
    
    for (c = syntax; *c != NULL; c++)
        {
        for (b = cmdBuf, p = *c; ; b++)
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

/*--------------------------------------------------------------------------
**  Purpose:        Parse command string and return the first string
**                  terminated by comma.
**
**  Parameters:     Name        Description.
**                  inStr       Input string
**                  outStr      Output string
**                  outSize     Size of output field
**
**  Returns:        Pointer to first character in input string after
**                  the extracted string..
**
**------------------------------------------------------------------------*/
char *opGetString(char *inStr, char *outStr, int outSize)
    {
    u8 pos = 0;
    u8 len = 0;

    /*
    **  Skip leading whitespace.
    */
    while (inStr[pos] != 0 && isspace(inStr[pos]))
        {
        pos += 1;
        }

    /*
    **  Return pointer to end of input string when there was only
    **  whitespace left.
    */
    if (inStr[pos] == 0)
        {
        *outStr = 0;
        return(inStr + pos);
        }

    /*
    **  Copy string to output buffer.
    */
    while (inStr[pos] != 0 && inStr[pos] != ',')
        {
        if (len >= outSize - 1)
            {
            return(NULL);
            }

        outStr[len++] = inStr[pos++];
        }

    outStr[len] = 0;

    /*
    **  Skip the comma.
    */
    pos += 1;

    return(inStr + pos);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate emulation.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdShutdown(char *cmdParams)
    {
    /*
    **  Process command.
    */
    opActive = FALSE;
    emulationActive = FALSE;

    printf("\nThanks for using %s - Goodbye for now.\n\n", DtCyberVersion);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate operator interface.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdEnd(char *cmdParams)
    {
    /*
    **  Process commands.
    */
    opActive = FALSE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Load new media (cards, tape, etc.)
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdLoad(char *cmdParams)
    {
    int chnum, unit, np;
    int rest;
    ChSlot *ch;
    DevSlot *dp;
    bool done = FALSE;
    
    rest = 0;
    np = sscanf (cmdParams, "%o,%o,%n", &chnum, &unit, &rest);
    if ((np != 3 && np != 2) || rest == 0)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS");
        return;
        }
    ch = channel + chnum;
    dp = ch->firstDevice;
    while (dp != NULL)
        {
        if (dp->load)
            {
            dp->load (dp, unit, cmdParams + rest);
            done = TRUE;
            }
        dp = dp->next;
        }
    if (!done)
        {
        sprintf (msgBuf, "$NOT SUPPORTED ON CH%02o EQ%02o",
                 chnum, unit);
        opSetMsg (msgBuf);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Unload media (cards, tape, etc.)
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opCmdUnload(char *cmdParams)
    {
    int chnum, unit, np;
    ChSlot *ch;
    DevSlot *dp;

    np = sscanf (cmdParams, "%o,%o", &chnum, &unit);
    if (np != 2)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS");
        return;
        }
    ch = channel + chnum;
    dp = ch->firstDevice;
    while (dp != NULL)
        {
        if (dp->load)
            dp->load (dp, unit, NULL);
        dp = dp->next;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Dump CPU state
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opDumpCpu(char *cmdParams)
    {
    /*
    **  Process commands.
    */
    dumpCpu ();
    }


/*--------------------------------------------------------------------------
**  Purpose:        Dump selected PPU state
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opDumpPpu(char *cmdParams)
    {
    int     np, pp;
    /*
    **  Process commands.
    */
    np = sscanf (cmdParams, "%o", &pp);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS");
        return;
        }
    dumpPpu (pp);
    }


/*--------------------------------------------------------------------------
**  Purpose:        Disassemble memory of selected PPU
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opDisPpu(char *cmdParams)
    {
    int     np, pp;
    /*
    **  Process commands.
    */
    np = sscanf (cmdParams, "%o", &pp);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS");
        return;
        }
    dumpDisassemblePpu (pp);
    }


/*---------------------------  End Of File  ------------------------------*/
