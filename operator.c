/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
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
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "const.h"
#include "types.h"
#include "proto.h"
#include "operator.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define OpCmdSize       64
#define NetBufSize      256
#define StatusLineMax   255
#define StatusSys       0       /* Line number of operator system status */
#define StatusHdr       1       /* Line number of status header line */
#define StatusFirstDev  2       /* Line number of first status line */
#define StatusMsgPfx    15      /* Number of prefix bytes in status buffer */
#define StatusMsgMax    50      /* Max length of status message */

#define StatusDevs      0       /* Currently displaying device status */
#define StatusConns     1       /* Currently displaying connections */

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
    const char *text;
    } OpMsg;

typedef struct
    {
    int     line;
    int     len;
    char    buf[StatusMsgPfx + StatusMsgMax + 1];
    } StatusData;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static int opRequest(NetFet *np);
static ThreadFunRet opThread (void *param);
static void opSetup(NetFet *np, int index);
static void opSendStatus (StatusData *sd);
static void opUpdateSysStatus (void);

static void opCmdShutdown(char *cmdParams);
static void opCmdLoad(char *cmdParams);
static void opCmdUnload(char *cmdParams);
static void opDumpCpu (char *cmdParams);
static void opDumpCMem (char *cmdParams);
static void opDumpEcs (char *cmdParams);
static void opDumpPpu (char *cmdParams);
static void opDisPpu (char *cmdParams);
static void opKeyboard (char *cmdParams);
static void opDebug(char *cmdParams);
static void opLock(char *cmdParams);
static void opUnlock(char *cmdParams);
static void opDeadstart(char *cmdParams);
static void opDevlist(char *cmdParams);
static void opConnlist(char *cmdParams);
#if CcDebug == 1
static void opTracePpu(char *cmdParams);
static void opTraceCh(char *cmdParams);
static void opTraceCp(char *cmdParams);
static void opTraceCpu(char *cmdParams);
static void opTraceXj(char *cmdParams);
static void opTraceEcs(char *cmdParams);
static void opUntrace(char *cmdParams);
static void opUntracePpu(char *cmdParams);
static void opUntraceCh(char *cmdParams);
static void opUntraceCpu(char *cmdParams);
static void opUntraceXj(char *cmdParams);
static void opUntraceEcs(char *cmdParams);
static void opResetTrace(char *cmdParams);
#endif
static void operUpdateConnections (void);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
long opPort;
long opConns;
bool debugDisplay = FALSE;
bool opDebugging = FALSE;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static char tlvBuf[OpCmdSize];
#define cmdBuf (tlvBuf + 2)
static NetPortSet opPorts;
static int statusType = StatusDevs;

static const char *syntax[] = 
    /*
    ** Simple command decode table with DSD style completion.
    ** 7 means one or more octal digits.
    ** x means any character other than comma (case preserved).
    ** anything else means that character (case insensitive if letter).
    ** command is complete if matched to end of some entry.
    */
    {
    "END.\n",               /* This is handled locally at the other end */
    "SHUTDOWN.\n",
    "LOAD,7,7,x,W.\n",
    "LOAD,7,7,x\n",
    "UNLOAD,7,7.\n",
    "DUMP,CPU.\n",
    "DUMP,CM,7,7.\n",
    "DUMP,ECS,7,7.\n",
    "DUMP,PPU7.\n",
    "DISASSEMBLE,PPU7.\n",
    "SET,KEYBOARD=TRUE.\n",
    "SET,KEYBOARD=EASY.\n",
    "DEBUG,DISPLAY=ON.\n",
    "DEBUG,DISPLAY=OFF.\n",
    "LOCK.\n",
    "UNLOCK.\n",
    "DEADSTART.\n",
    "SHOW,DEVICES.\n",
    "SHOW,CONNECTIONS.\n",
#if CcDebug == 1
    "DEBUG,ON.\n",
    "DEBUG,OFF.\n",
    "TRACE,PPU7.\n",
    "TRACE,CHANNEL7.\n",
    "TRACE,CPU7.\n",
    "TRACE,CP7.\n",
    "TRACE,XJ.\n",
    "TRACE,ECS.\n",
    "UNTRACE,.\n",
    "UNTRACE,PPU7.\n",
    "UNTRACE,CHANNEL7.\n",
    "UNTRACE,CPU7.\n",
    "UNTRACE,XJ.\n",
    "UNTRACE,ECS.\n",
    "UNTRACE,RESET.\n",
#endif
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
    ** Note that the matches must in order from longest to shortest
    ** possible matching string.  The first match is used.
    */
    {
    { "SHUTDOWN.",                opCmdShutdown },
    { "LOAD,",                    opCmdLoad },
    { "UNLOAD,",                  opCmdUnload },
    { "DUMP,CPU.",                opDumpCpu },
    { "DUMP,CM,",                 opDumpCMem },
    { "DUMP,ECS,",                opDumpEcs },
    { "DUMP,PPU",                 opDumpPpu },
    { "DISASSEMBLE,PPU",          opDisPpu },
    { "SET,KEYBOARD=",            opKeyboard },
    { "DEBUG,",                   opDebug },
    { "LOCK.",                    opLock },
    { "UNLOCK.",                  opUnlock },
    { "DEADSTART.",               opDeadstart },
    { "SHOW,DEVICES.",            opDevlist },
    { "SHOW,CONNECTIONS.",        opConnlist },
#if CcDebug == 1
    { "TRACE,PPU",                opTracePpu },
    { "TRACE,CHANNEL",            opTraceCh },
    { "TRACE,CPU",                opTraceCpu },
    { "TRACE,CP",                 opTraceCp },
    { "TRACE,XJ.",                opTraceXj },
    { "TRACE,ECS.",               opTraceEcs },
    { "UNTRACE,.",                opUntrace },
    { "UNTRACE,PPU",              opUntracePpu },
    { "UNTRACE,CHANNEL",          opUntraceCh },
    { "UNTRACE,CPU",              opUntraceCpu },
    { "UNTRACE,XJ.",              opUntraceXj },
    { "UNTRACE,ECS.",             opUntraceEcs },
    { "UNTRACE,RESET.",           opResetTrace },
#endif
    { NULL,                       NULL },
    };

// Note: Y values of zero are filled in by opInit
static OpMsg msg[] =
    { { 0120, 0760, 0010, 0, "" },      /* Filled in */
      { 0120, 0730, 0020, 0, "OPERATOR INTERFACE" },
      { 0020, 0700, 0010, 0, "LOAD,CH,EQ,FILE    Load file for ch/eq, read-only." },
      { 0020,    0, 0010, 0, "LOAD,CH,EQ,FILE,W. Load file for ch/eq, read/write." },
      { 0020,    0, 0010, 0, "LOCK.              Disable SHUTDOWN." },
      { 0020,    0, 0010, 0, "UNLOCK.            Enable SHUTDOWN." },
      { 0020,    0, 0010, 0, "UNLOAD,CH,EQ.      Unload ch/eq." },
      { 0020,    0, 0010, 0, "DUMP,CPU.          Dump state of CPUs." },
      { 0020,    0, 0010, 0, "DUMP,CM,X,Y.       Dump CM from X to Y." },
      { 0020,    0, 0010, 0, "DUMP,ECS,X,Y.      Dump ECS from X to Y." },
      { 0020,    0, 0010, 0, "DUMP,PPUNN.        Dump specified PPU state." },
      { 0020,    0, 0010, 0, "DISASSEMBLE,PPUNN. Disassemble specified PPU." },
      { 0020,    0, 0010, 0, "SET,KEYBOARD=[TRUE,EASY]. Set rollover mode." },
      { 0020,    0, 0010, 0, "DEBUG,DISPLAY=[ON,OFF]. Turn CP/PP debug display on/off." },
      { 0020,    0, 0010, 0, "DEADSTART.         Deadstart the system." },
      { 0020,    0, 0010, 0, "SHOW,[DEVICES,CONNECTIONS]. Choose status display." },
#if CcDebug == 1
      { 0020,    0, 0010, 0, "DEBUG,[ON,OFF].    Enabled/disable debug commands." },
      { 0020,    0, 0010, 0, "TRACE,[CPUN,CPN,XJ,PPUN,CHANNELN,ECS]. Trace something." },
      { 0020,    0, 0010, 0, "UNTRACE,XXX        Stop trace of XXX." },
      { 0020,    0, 0010, 0, "UNTRACE,.          Stop all tracing." },
      { 0020,    0, 0010, 0, "UNTRACE,RESET.     Stop tracing, discard trace data." },
#endif
      { 0020,    0, 0010, 0, "" },      // blank line to separate these last two from the above
      { 0020,    0, 0010, 0, "END.               Exit operator mode." },
      { 0020,    0, 0010, 0, "SHUTDOWN.          Close DtCyber." },
      { 0, 0, 0, 0, NULL },
    };
static char msgBuf[80];
static const char *msgPtr;
static int statusLineCnt = StatusFirstDev;
static StatusData *statusLines[StatusLineMax];
static StatusData statusSys;
static StatusData statusHdr;
static int opActiveConns;
static bool opLocked = TRUE;

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
    int y = 0;
    OpMsg *m;

    /*
    **  Fill in the coordinates for the message data. 
    */
    m = msg;
    while (m->text != NULL)
        {
        if (m->y == 0)
            {
            y -= m->fontSize * 2 - 2;
            m->y = y;
            }
        else
            {
            y = m->y;
            }
        m++;
        }
    statusLines[StatusSys] = &statusSys;
    statusLines[StatusHdr] = &statusHdr;
    sprintf (statusHdr.buf, "\001Device status");
    statusHdr.len = strlen (statusHdr.buf);
    updateConnections = operUpdateConnections;
    
    opPorts.portNum = opPort;
    opPorts.maxPorts = opConns;
    opPorts.localOnly = TRUE;
    opPorts.callBack = opSetup;
    opPorts.kind = "oper";
    dtInitPortset (&opPorts, NetBufSize);
    dtCreateThread (opThread, 0);
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
    msgPtr = p;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set status text for operator
**
**  Parameters:     Name        Description.
**                  buf         pointer to buffer returned by opInitStatus
**                  msg         message text to set
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void opSetStatus (void *buf, const char *msg)
    {
    StatusData *sd = (StatusData *) buf;
    
    if (buf == NULL)
        {
        /*
        **  Checking for NULL here means the caller doesn't have to
        **  worry whether we ran out of space in opInitStatus.
        */
        return;
        }
    
    if (msg != NULL)
        {
        strncpy (sd->buf + StatusMsgPfx, msg, StatusMsgMax);
        sd->len = StatusMsgPfx + strlen (sd->buf + StatusMsgPfx);
        }
    else
        {
        sd->len = StatusMsgPfx;
        }
    if (statusType == StatusDevs)
        {
        opSendStatus (sd);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Initialize operator status buffer
**
**  Parameters:     Name        Description.
**                  type        Device type name
**                  ch          Channel number
**                  un          Unit number
**
**  Returns:        pointer to status buffer, to be passed to opSetStatus
**                  later, or NULL if no more room.
**
**------------------------------------------------------------------------*/
void * opInitStatus (const char *type, int ch, int un)
    {
    StatusData *sd;
    
    if (statusLineCnt == StatusLineMax)
        {
        return NULL;
        }
    sd = (StatusData *) calloc (1, sizeof(StatusData));
    if (sd == NULL)
        {
        return NULL;
        }
    statusLines[statusLineCnt] = sd;
    sd->line = statusLineCnt++;

    /*
    **  Form the status line prefix, including the line number byte
    **  The total length is 14 bytes
    */
    sprintf (sd->buf, "%c%-7s %02o %02o ", sd->line,
             type, ch, un);
    sd->len = StatusMsgPfx;
    return sd;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform pending operator requests
**
**  Parameters:     Name        Description.
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void operCheckRequests (void)
    {
    int i;
    NetFet *np;

    /*
    **  Look for commands from any of the connected 
    **  operator displays.
    */
    np = opPorts.portVec;
    for (i = 0; i < opConns; i++)
        {
        if (dtActive (np))
            {
            opRequest (np);
            }
        if (!emulationActive)
            {
            /* We just executed "shutdown" */
            return;
            }
        np++;
        }
        
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Operator command thread
**
**  Parameters:     Name        Description.
**                  param       unused
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static ThreadFunRet opThread (void *param)
    {
    int i;
    NetFet *np;
    
    printf ("operator thread running\n");
    
    for (;;)
        {
        if (!emulationActive)
            {
            /* We just executed "shutdown" */
            ThreadReturn;
            }
        /*
        **  Get any additional network data 
        */
        for (;;)
            {
            np = dtFindInput (&opPorts, 1000);
            if (np == NULL)
                {
                break;
                }
            i = dtRead  (np, -1);
            if (i < 0)
                {
                dtClose (np, &opPorts, TRUE);
                }
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process operator request
**
**  Parameters:     Name        Description.
**                  np          pointer to NetFet
**
**  Returns:        -1 if no command pending, 0 if command processed.
**
**------------------------------------------------------------------------*/
static int opRequest(NetFet *np)
    {
    OpCmd *cp;
    int cmdLen;
    int j;

    /*
    **  Try to receive a command TLV
    */
    cmdLen = dtReadtlv (np, tlvBuf, sizeof (tlvBuf));
    if (cmdLen < 0 || tlvBuf[0] != OpCommand)
        {
        return -1;
        }
    if (cmdLen == 0)
        {
        return 0;
        }
    
    cmdBuf[cmdLen++] = '\0';
    printf ("%s Operator command: %s\n", dtNowString (), cmdBuf);
    opSetMsg (NULL);
    for (cp = decode; cp->name != NULL; cp++)
        {
        j = strlen (cp->name);
        if (strncmp(cp->name, cmdBuf, j) == 0)
            {
            cp->handler(cmdBuf + j);
            break;
            }
        }
    if (cp->name == NULL)
        {
        opSetMsg ("$INVALID COMMAND.");
        }
    if (msgPtr != NULL)
        {
        dtSendTlv (np, &opPorts, OpReply, strlen (msgPtr), msgPtr);
        }
    else
        {
        dtSendTlv (np, &opPorts, OpReply, 0, NULL);
        }
    
    return 0;
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
    int i;
    NetFet *np;
    
    /*
    **  Process command.
    */
    if (opLocked)
        {
        opSetMsg ("NOT UNLOCKED.");
        return;
        }
    
    consoleSetKeyboardTrue (FALSE);
    emulationActive = FALSE;
    opSetMsg ("DtCyber shut down.");

    /* Send the shutdown reply message to all consoles */
    for (i = 0; i < opConns; i++)
        {
        np = opPorts.portVec + i;
        if (dtActive (np))
            {
            dtSendTlv (np, &opPorts, OpReply, strlen (msgPtr), msgPtr);
            }
        }

    printf("\nThanks for using %s - Goodbye for now.\n\n", DtCyberVersion);
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
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    ch = channel + chnum;
    dp = ch->firstDevice;
    while (dp != NULL)
        {
        if (dp->load)
            {
            activeDevice = dp;
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
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    ch = channel + chnum;
    dp = ch->firstDevice;
    while (dp != NULL)
        {
        if (dp->load)
            {
            activeDevice = dp;
            dp->load (dp, unit, NULL);
            }
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
    opSetMsg ("COMPLETED.");
    }


/*--------------------------------------------------------------------------
**  Purpose:        Dump a range of central memory.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opDumpCMem(char *cmdParams)
    {
    int     np, start, end;
    /*
    **  Process commands.
    */
    np = sscanf (cmdParams, "%o,%o", &start, &end);
    if (np != 2)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (start > end || end >= cpuMaxMemory)
        {
        opSetMsg ("$INVALID ADDRESS.");
        return;
        }
    dumpCpuMem (NULL, start, end, 0);
    opSetMsg ("COMPLETED.");
    }


/*--------------------------------------------------------------------------
**  Purpose:        Dump a range of ECS.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opDumpEcs(char *cmdParams)
    {
    int     np, start, end;
    /*
    **  Process commands.
    */
    np = sscanf (cmdParams, "%o,%o", &start, &end);
    if (np != 2)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (start > end || end >= ecsMaxMemory)
        {
        opSetMsg ("$INVALID ADDRESS.");
        return;
        }
    dumpEcs (NULL, start, end);
    opSetMsg ("COMPLETED.");
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
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (pp >= ppuCount)
        {
        opSetMsg ("$INVALID PPU NUMBER.");
        return;
        }
    dumpPpu (pp);
    opSetMsg ("COMPLETED.");
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
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (pp >= ppuCount)
        {
        opSetMsg ("$INVALID PPU NUMBER.");
        return;
        }
    dumpDisassemblePpu (pp);
    opSetMsg ("COMPLETED.");
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set keyboard mode to easy or true emulation
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opKeyboard(char *cmdParams)
    {
    /*
    **  Process commands.
    */
    if (strcmp (cmdParams, "EASY.") == 0)
        {
        consoleSetKeyboardTrue (FALSE);
        }
    else if (strcmp (cmdParams, "TRUE.") != 0)
        {
        opSetMsg ("$INVALID PARAMETER.");
        }
    else
        {
        consoleSetKeyboardTrue (TRUE);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Change debug flag, or debug trace display.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opDebug(char *cmdParams)
    {
    /*
    **  Process commands.
    */
    if (strcmp (cmdParams, "DISPLAY=ON.") == 0)
        {
        debugDisplay = TRUE;
        }
    else if (strcmp (cmdParams, "DISPLAY=OFF.") == 0)
        {
        debugDisplay = FALSE;
        }
#if CcDebug == 1
    else if (strcmp (cmdParams, "ON.") == 0)
        {
        opDebugging = TRUE;
        opUpdateSysStatus ();
        }
    else if (strcmp (cmdParams, "OFF.") == 0)
        {
        opDebugging = FALSE;
        opUntrace (cmdParams);
        opUpdateSysStatus ();
        }
#endif
    else
        {
        opSetMsg ("$INVALID PARAMETER.");
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set locked state.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opLock(char *cmdParams)
    {
    opLocked = TRUE;
    opUpdateSysStatus ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set unlocked state.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opUnlock(char *cmdParams)
    {
    opLocked = FALSE;
    opUpdateSysStatus ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Deadstart the system.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opDeadstart(char *cmdParams)
    {
    /*
    **  Process command.
    */
    if (opLocked)
        {
        opSetMsg ("NOT UNLOCKED.");
        return;
        }
    
    deadStart ();

    opUpdateSysStatus ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Show device status (default display).
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opDevlist(char *cmdParams)
    {
    int i, j;
    NetFet *np;
    char null[2];
    
    sprintf (statusHdr.buf + 1, "Device status");
    statusHdr.len = strlen (statusHdr.buf);
    statusType = StatusDevs;
    
    for (i = 0; i < opConns; i++)
        {
        np = opPorts.portVec + i;
        if (dtActive (np))
            {
            dtSendTlv (np, &opPorts, OpStatus, statusHdr.len, statusHdr.buf);
            for (j = 0; j < StatusLineMax; j++)
                {
                if (statusLines[j] != NULL)
                    {
                    dtSendTlv (np, &opPorts, OpStatus, statusLines[j]->len,
                               statusLines[j]->buf);
                    }
                else
                    {
                    null[0] = j;
                    dtSendTlv (np, &opPorts, OpStatus, 1, null);
                    }
                }
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Show connections status.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opConnlist(char *cmdParams)
    {
    int i, j;
    char cstat[50];
    NetFet *cp;
    NetFet *np;
    const char *kind;
    
    sprintf (statusHdr.buf + 1, "Connections");
    statusHdr.len = strlen (statusHdr.buf);
    statusType = StatusConns;
    
    for (i = 0; i < opConns; i++)
        {
        np = opPorts.portVec + i;
        if (dtActive (np))
            {
            dtSendTlv (np, &opPorts, OpStatus, statusHdr.len, statusHdr.buf);
            for (j = StatusFirstDev, cp = connlist.next; 
                 j < StatusLineMax && cp != &connlist;
                 j++, cp = cp->next)
                {
                if (cp->ps != NULL)
                    {
                    kind = cp->ps->kind;
                    }
                else
                    {
                    kind = "";
                    }
                if (cp->ownerInfo <= 0 || cp->ownerInfo > 1023)
                    {
                    sprintf (cstat, "%c%3d. %-7s %5lld to %s:%d", 
                             j, j - StatusFirstDev,
                             kind, cp->ownerInfo,
                             inet_ntoa (cp->from), ntohs (cp->fromPort));
                    }
                else
                    {
                    sprintf (cstat, "%c%3d. %-7s %2lld-%-2lld to %s:%d", 
                             j, j - StatusFirstDev,
                             kind, cp->ownerInfo / 32, cp->ownerInfo & 31,
                             inet_ntoa (cp->from), ntohs (cp->fromPort));
                    }
                dtSendTlv (np, &opPorts, OpStatus, strlen (cstat), cstat);
                }
            for ( ; j < StatusLineMax; j++)
                {
                cstat[0] = j;
                dtSendTlv (np, &opPorts, OpStatus, 1, cstat);
                }
            }
        }
    }

#if CcDebug == 1
/*--------------------------------------------------------------------------
**  Purpose:        Trace a PPU
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opTracePpu(char *cmdParams)
    {
    int     np, pp;
    /*
    **  Process commands.
    */
    if (!opDebugging)
        {
        opSetMsg ("$DEBUG MODE NOT SET.");
        return;
        }
    
    np = sscanf (cmdParams, "%o", &pp);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (pp >= ppuCount)
        {
        opSetMsg ("$INVALID PPU NUMBER.");
        return;
        }
    traceMask |= 1 << pp;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Trace a channel
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opTraceCh(char *cmdParams)
    {
    int     np, ch;
    /*
    **  Process commands.
    */
    if (!opDebugging)
        {
        opSetMsg ("$DEBUG MODE NOT SET.");
        return;
        }
    
    np = sscanf (cmdParams, "%o", &ch);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (ch >= channelCount)
        {
        opSetMsg ("$INVALID CHANNEL NUMBER.");
        return;
        }
    chTraceMask |= ULL(1) << ch;
    traceMask |= TraceCh;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Trace a control point
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opTraceCp(char *cmdParams)
    {
    int     np, cp;
    /*
    **  Process commands.
    */
    if (!opDebugging)
        {
        opSetMsg ("$DEBUG MODE NOT SET.");
        return;
        }
    
    np = sscanf (cmdParams, "%o", &cp);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (cp > 034 || cp < 1)
        {
        opSetMsg ("$INVALID CP NUMBER.");
        return;
        }
    traceMask |= TraceCpu0 | TraceCpu1;
    traceCp = cp;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Trace a CPU
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opTraceCpu(char *cmdParams)
    {
    int     np, cp;
    /*
    **  Process commands.
    */
    if (!opDebugging)
        {
        opSetMsg ("$DEBUG MODE NOT SET.");
        return;
        }
    
    np = sscanf (cmdParams, "%o", &cp);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (cp >= cpuCount)
        {
        opSetMsg ("$INVALID CPU NUMBER.");
        return;
        }
    traceMask |= TraceCpu (cp);
    traceCp = 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Trace exchange jumps
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opTraceXj(char *cmdParams)
    {
    if (!opDebugging)
        {
        opSetMsg ("$DEBUG MODE NOT SET.");
        return;
        }
    
    traceMask |= TraceXj;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Trace ECS references.
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opTraceEcs(char *cmdParams)
    {
    if (!opDebugging)
        {
        opSetMsg ("$DEBUG MODE NOT SET.");
        return;
        }
    
    traceMask |= TraceEcs;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Stop tracing a PPU
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opUntracePpu(char *cmdParams)
    {
    int     np, pp;
    /*
    **  Process commands.
    */
    np = sscanf (cmdParams, "%o", &pp);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (pp >= ppuCount)
        {
        opSetMsg ("$INVALID PPU NUMBER.");
        return;
        }
    traceMask &= ~(1 << pp);
    traceStop ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Stop tracing a channel
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opUntraceCh(char *cmdParams)
    {
    int     np, ch;
    /*
    **  Process commands.
    */
    np = sscanf (cmdParams, "%o", &ch);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (ch >= channelCount)
        {
        opSetMsg ("$INVALID CHANNEL NUMBER.");
        return;
        }
    chTraceMask &= ~(ULL(1) << ch);
    if (chTraceMask == 0)
        {
        traceMask &= ~TraceCh;
        }
    traceStop ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Stop tracing the CPU
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opUntraceCpu(char *cmdParams)
    {
    int     np, cp;
    /*
    **  Process commands.
    */
    np = sscanf (cmdParams, "%o", &cp);
    if (np != 1)
        {
        opSetMsg ("$INSUFFICENT PARAMETERS.");
        return;
        }
    if (cp >= cpuCount)
        {
        opSetMsg ("$INVALID CPU NUMBER.");
        return;
        }
    traceMask &= ~(TraceCpu (cp));
    traceCp = 0;
    traceStop ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Stop tracing exchange jumps
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opUntraceXj(char *cmdParams)
    {
    traceMask &= ~(TraceXj);
    traceStop ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Stop tracing ECS references
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opUntraceEcs(char *cmdParams)
    {
    traceMask &= ~(TraceEcs);
    traceStop ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Stop all tracing
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opUntrace(char *cmdParams)
    {
    traceMask = chTraceMask = traceCp = 0;
    traceStop ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Stop all tracing
**
**  Parameters:     Name        Description.
**                  cmdParams   Command parameters
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void opResetTrace(char *cmdParams)
    {
    traceMask = chTraceMask = traceCp = 0;
    traceReset ();
    }

#endif  // CcDebug=1

/*--------------------------------------------------------------------------
**  Purpose:        Send the initialization data to a new connection
**
**  Parameters:     Name        Description.
**                  np          NetFet pointer
**                  index       NetFet index
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void opSetup (NetFet *np, int index)
    {
    int i;
    const char **sp;
    OpMsg *msgp;
    char msgbuf[200];
    char hostbuf[100];
    char *p;
    
    if (np->connFd == 0)
        {
        opActiveConns--;
        opUpdateSysStatus ();
        printf("%s Operator connection dropped from %s\n",
               dtNowString (), inet_ntoa (np->from));
        return;
        }

    printf("%s Received operator connection from %s\n",
           dtNowString (), inet_ntoa (np->from));
    
    /*
    **  We have to send two sets of data:
    **      1. the fixed text (command list, version string etc.)
    **      2. the parse strings
    */
    sp = syntax;
    while (*sp != NULL)
        {
        dtSendTlv (np, &opPorts, OpSyntax, strlen (*sp), *sp);
        sp++;
        }
    msgp = msg;
    while (msgp->text != NULL)
        {
        p = msgbuf;
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
        *p++ = msgp->x & 0377;
        *p++ = msgp->x >> 8;
        *p++ = msgp->y & 0377;
        *p++ = msgp->y >> 8;
        *p++ = msgp->fontSize;
        *p++ = msgp->bold;
        if (msgp->y == 0760)
            {
            gethostname (hostbuf, sizeof (hostbuf) - 1);
            sprintf (p, DtCyberVersion " on %s", hostbuf);
            }
        else
            {
            strcpy (p, msgp->text);
            }
        dtSendTlv (np, &opPorts, OpText, p - msgbuf + strlen (p),
                   msgbuf);
        msgp++;
        }
    /*
    **  All done sending init data, so indicate that.
    */
    dtSendTlv (np, &opPorts, OpInitialized, 0, NULL);

    opActiveConns++;
    opUpdateSysStatus ();

    /*
    **  Status lines are not init data, so we can send them after
    **  saying "init done" -- send them now.
    */
    for (i = 0; i < StatusLineMax; i++)
        {
        if (statusLines[i] != NULL)
            {
            dtSendTlv (np, &opPorts, OpStatus, statusLines[i]->len,
                       statusLines[i]->buf);
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send a line of status data to all connections
**
**  Parameters:     Name        Description.
**                  sd          StatusData pointer
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void opSendStatus (StatusData *sd)
    {
    int i;
    NetFet *np;
    
    /*
    **  If operator subsystem not initialized yet, just exit.
    */
    if (opPorts.portVec == NULL)
        {
        return;
        }
    
    for (i = 0; i < opConns; i++)
        {
        np = opPorts.portVec + i;
        if (dtActive (np))
            {
            dtSendTlv (np, &opPorts, OpStatus, sd->len, sd->buf);
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Update the operator system status line
**
**  Parameters:     Name        Description.
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void opUpdateSysStatus (void)
    {
    if (!emulationActive)
        {
        sprintf (statusSys.buf + 1,
                 "DtCyber shut down");
        }
    else if (opActiveConns > 1)
        {
        sprintf (statusSys.buf + 1,
                 "%8s  %5s    %d operators",
                 (opLocked) ? "" : "UNLOCKED",
                 (opDebugging) ? "DEBUG" : "",
                 opActiveConns);
        }
    else
        {
        sprintf (statusSys.buf + 1,
                 "%8s  %5s",
                 (opLocked) ? "" : "UNLOCKED",
                 (opDebugging) ? "DEBUG" : "");
        }
    statusSys.len = strlen (statusSys.buf + 1) + 1;
    opSendStatus (&statusSys);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Update connections display, if active
**
**  Parameters:     Name        Description.
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static void operUpdateConnections (void)
    {
    if (statusType == StatusConns)
        {
        opConnlist (NULL);
        }
    }

/*---------------------------  End Of File  ------------------------------*/
