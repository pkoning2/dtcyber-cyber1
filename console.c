/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: console.c
**
**  Description:
**      Perform simulation of CDC 6612 console.
**
**--------------------------------------------------------------------------
*/

/*
**  General documentation about the display emulation:
**
**  This is the emulation of the Cyber console.  It attempts to provide
**  a couple of services beyond just basic display emulation.
**
**  The mechanisms described below are partly implemented in this
**  file, and partly in the graphics application that connects to
**  DtCyber via a TCP connection.  
**
**  The classic display is a CRT with relatively long persistence, which 
**  is refreshed frequently.  Data "decays" if not refreshed.
**  On the other hand, the emulation uses a PC/workstation graphics
**  mechanism, which doesn't directly support decay.  Instead, we have
**  graphics that persist until erased.  So the display emulation has
**  to erase what it has, then redraw from new data periodically.
**  If this is done without any synchronization with the display data
**  stream, the result is ugly flicker.
**
**  Another aspect that we would like to emulate is "intensify" or
**  "brightness control".  The Cyber display code would make things
**  bright ("bold") by displaying the same data several times in a row.
**  If we can detect that, we can switch to a bold font, or otherwise
**  emulate the brighter/bolder display.  This is reasonably easy
**  if you look for repeats in the data stream, but again it doesn't
**  draw consistently if there isn't some sort of synchronization.
**
**  There is no explicit synchronization mechanism in the original
**  hardware; you would simply send data whenever you'd want, and it
**  would get displayed.  But in practice, display I/O follows a 
**  definite pattern.  The emulation here takes advantage of those
**  usage patterns.
**
**  Except for a few places in deadstart (the CP/PP test display
**  for example), there is always checking for keyboard input going
**  on while output is being displayed.  The basic pattern looks
**  like this:
**      1. Check for keyboard input
**      2. Display the screen content
**      3. Go back to 1
**  A few programs, like WRM, are a bit more complex:
**      1. Check for keyboard input
**      2. Display a portion of the screen content
**      3. Check for keyboard input
**      4. Display some more
**      ...
**      n. Go back to 1
**  This seems to be the practice when the total amount of display 
**  work is "large".
**
**  The display emulation defines the notion of a "display cycle".
**  That is its best attempt to capture one program cycle as just
**  described.  
**
**  A basic cycle is simply the output data between two consecutive
**  keyboard reads.  That handles the basic pattern but not the WRM
**  pattern.
**
**  To handle the WRM pattern we try to find the point at which the
**  data repeats.  The approach used here, which seems to work
**  reasonably well, is to look at the Set X and Set Y operations in
**  the data stream.  We do a checksum of those values (using the
**  Fletcher checksum, which is cheap).  The checksum is saved at each
**  keyboard read and then reinitialized.  At each keyboard read we
**  look backwards through the saved checksums to find one that
**  matches the most recent checksum.  The data range between those
**  two points then defines a cycle.
**
**  If we identify a cycle, only that cycle worth of data is displayed
**  when it's time to show new data.  This also optimizes graphics
**  processing; if some data is sent to the console channel many
**  times from one screen update to another, only the last cycle will
**  be processed by the graphics machinery, not all of the redundant
**  copies.  This makes a detectable difference.
**
**  To support remote DD60 displays, we allow each display connection
**  to indicate how often it wants to get new data.  It will get a new
**  cycle worth of data is (a) the requested amount of time has
**  elapsed, or (b) there has been keyboard input since the last
**  update. 
**
**  Clearly, the cycle finding machinery will sometimes not find a
**  cycle, for example when elements of the display are moving.
**  That's ok; if that happens, more output will be processed than is
**  strictly necessary, but nothing will be lost.
**
**  A note on "bold" output:  
**
**  There are two cases: bold text, and bold dots.  Bold text appears
**  in a lot of places.  The default number of repeats for text isn't
**  necessarily one; for medium and large text, the hardware required
**  several repeats to get normal intensity.  So the definition of
**  "bold" is: 3 times for small, 7 times for medium, or 15 times
**  for large text.
**
**  For dot mode output, each repeat up to 9 makes the dot somewhat
**  larger, by turning on one of the 8 adjacent dots.  This is used in
**  the star field display in LUNAR.
**
**  Detection of repeated output relies on the fact that repeats are
**  not done randomly.  Dot mode repeats are done in direct sequence
**  (a given coordinate is hammered N times).  Text mode repeats are
**  normally done by sending a line or part of a line multiple times.
**  In a few cases, such as the STEP and UNLOCK display elements,
**  those two are displayed repeated as pairs.
**
**  To handle that last case, and also to optimize graphics
**  processing, the graphics machinery buffers a line worth of text,
**  including replacing of space skipped over with a Set X by spaces.
**  It then looks for a repeat, and finally it sends the entire
**  buffered line to the graphics machinery in the OS.
**
**  The data stream from here to the display program runs over a TCP
**  connection, by default on port 5007.  The output data stream is a
**  sequence of data blocks for display cycles, spaced at intervals
**  requested by the display program.  (This allows slow update when
**  low performance connections are used.)  The input data stream is a
**  sequence of keyboard codes as well as occasional control codes.
**
**  Multiple displays are allowed.  Each display gets the full output,
**  subject to the rate limiting it requests.  Each display program
**  can supply input, and the Cyber sees the combination of all
**  input.  (So if two people type simultaneously, things may get
**  confusing.  Then again, you'll see it happen.  And if you're
**  running STARWAR, you will actually get exactly what you want!)
**
**  As a special case, a display can request a cycle interval of zero,
**  which means it will get the full output stream without any cycles
**  omitted.  This is intended for display programs that emulate the
**  CRT decay explicitly, in the fashion of Phil Budne's GT40
**  emulator. 
**
**  The output stream is a byte stream.  If the high order bit is
**  clear, the byte is a character, in display code.  If the high
**  order bit is set, you're looking at a one or two byte control
**  sequence.  The defined control codes are listed in dd60.h
**
**  Except when full output (cycle interval of zero) is selected, each
**  block of data (a cycle) will always start with the necessary
**  control codes to initialize the state: set x and y, set screen and
**  mode, set keyboard mode.
**
**  Input is a sequence of bytes, coded as follows:
**      2/0, 6/key              key down
**      2/1, 6/key              key up (in "true keyboard" mode)
**      2/2, 6/rate             fast refresh rate
**      2/3, 6/rate             slow refresh rate
**
**  The rate requests normally will appear at initial connect only,
**  but they are in principle legal anytime.  A rate of zero means
**  full output.  Otherwise, for "fast rate" the delay between frames
**  is specified in 20 ms increments; for "slow rate" the delay is
**  specified in seconds.
**
**  Key code 70 is "stop output" and 71 is "start output".  "Stop"
**  suspend all output data independent of the rate requests.
**
**  Keyboard input comes in two modes, called "true" and "easy".
**
**  True mode emulates the behavior of the original keyboard, which
**  had no rollover and would OR two keys together if they were
**  pressed at the same time.  Usually this is not all that helpful.
**  But it is what the STARWAR program requires.
**
**  Easy mode is modern style keyboard input with rollover.  Real
**  rollover doesn't exist in the 6612, but we emulate it by inserting
**  a key data word of 0 in between each regular keycode.  The PPU
**  will interpret that as an "all keys are up" condition, so it will
**  see the code after that as a new independent keypress.  "Easy"
**  mode is the default; you can switch modes via the operator
**  interface.  Note that the keyboard mode is global; it affects all
**  connected displays.  This is actually useful if you want to play
**  STARWAR with two people at different locations...
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include "const.h"
#include "types.h"
#include "proto.h"
#include "dd60.h"
#include <time.h>

/*
**  -----------------
**  Private Constants
**  -----------------
*/

/*
**  CDC 6612 console functions and status codes.
*/
#define Fc6612Sel64CharLeft     07000
#define Fc6612Sel32CharLeft     07001
#define Fc6612Sel16CharLeft     07002

#define Fc6612Sel512DotsLeft    07010
#define Fc6612Sel512DotsRight   07110
#define Fc6612SelKeyIn          07020

#define Fc6612Sel64CharRight    07100
#define Fc6612Sel32CharRight    07101
#define Fc6612Sel16CharRight    07102

#define OffLeftScreen           0
#define OffRightScreen          01020

#define DispBufSize             20000   /* Display buffer size */
#define KeyBufSize	            50      /* Input buffer size */
#define MaxPolls                5       /* Number of poll cycles we track */
#define NetBufSize              256

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
typedef struct portParam
    {
    u32         interval;       /* frame interval, in microseconds */
    u32         lastFrame;      /* time when previous frame was sent */
    bool        stopped;        /* TRUE if output stopped */
    bool        sendNow;        /* TRUE to force sending of next frame */
    } PortParam;

typedef struct pollData
{
    int         sum1;           /* Fletcher checksum accumulators */
    int         sum2;
    int         start;          /* First data byte in display ring */
    int         end;            /* Last + 1 data byte in display ring */
} PollData;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static FcStatus consoleFunc(PpWord funcCode);
static void consoleIo(void);
static void consoleActivate(void);
static void consoleDisconnect(void);

static ThreadFunRet consoleThread (void *param);
static int consoleInput (NetPort *np);

static void consoleByte1 (int byte);
static void consoleByte2 (int byte1, int byte2);
static int consoleGetKey (void);
static void consoleQueueKey(char ch);
static void consolePoll (void);
static void consoleSetXY (int code, int data);
static void consoleSendOutput (int start, int end);
static void consoleReinitPoll (void);
static void consoleConnect(NetPort *np, int index);
static void consoleUpdateStatus (void);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
long dd60Port;
long dd60Conns;
bool keyboardTrue;

extern u8 rtcIncrement;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 currentFont;
static u64 keyMask;
static u8 currentKey;
static bool autoDate;
static int autoPos;
static u32 lastDisplayUs;
static void *statusBuf;
static int activeConns;

static NetPortSet consolePorts;
static PortParam *portVector;

/* Buffers and cycle finding data */

/* Ring buffer (CIO style) for display output */
static u8 displayRing[DispBufSize];
static int displayOut, displayIn;

/* Just a simple linear buffer for poll data */
static PollData pollBuffer[MaxPolls];
static int pollIn;
static PollData currentPoll;

/* Ring buffer for keyboard input */
static u8 keyRing[KeyBufSize];
static u32 keyIn, keyOut;
static bool keyboardSendUp;
static bool sendToAll;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Initialise 6612 console.
**
**  Parameters:     Name        Description.
**                  eqNo        equipment number
**                  unitNo      unit number
**                  channelNo   channel number the device is attached to
**                  deviceName  optional device file name
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void consoleInit(u8 eqNo, u8 unitNo, u8 channelNo, char *deviceName)
    {
    DevSlot *dp;
    int i;
    char *p;
    
    (void)eqNo;
    (void)unitNo;
    (void)deviceName;

    dp = channelAttach(channelNo, eqNo, DtConsole);

    dp->activate = consoleActivate;
    dp->disconnect = consoleDisconnect;
    dp->selectedUnit = 0;
    dp->func = consoleFunc;
    dp->io = consoleIo;

    portVector = calloc(1, sizeof(PortParam) * dd60Conns);
    if (portVector == NULL)
        {
        fprintf(stderr, "Failed to allocate console context block\n");
        exit(1);
        }

    /*
    **  Turn on auto date entry checking if an auto date string
    **  was specified.  That string is the pattern we look for
    **  as the prompt output.
    */
    autoDate = (strlen (autoDateString) != 0);
    
    /*
    **  Initialize ring pointers and other cycle related state.
    */
    displayOut = displayIn = 0;
    keyOut = keyIn = 0;
    pollIn = 0;
    consoleReinitPoll ();
    
    /*
    **  Load any specified typeahead ("auto" parameter)
    **  into the keyboard buffer
    */
    if (autoString[0] != '\0')
        {
        consoleQueueKey (0);            /* something for the PPU to eat */
        for (p = autoString; *p; p++)
            {
            if (*p == '%')
                {
                consoleQueueKey (060);      /* carriage return */
                }
            else
                {
                consoleQueueKey (asciiToConsole[*p]);
                }
            }
        }

    /*
    **  Allocate the operator status buffer
    */
    statusBuf = opInitStatus ("DD6612", channelNo, 0);

    /*
    **  Set up the TCP listener state.
    */
    consolePorts.portNum = dd60Port;
    consolePorts.maxPorts = dd60Conns;
    consolePorts.localOnly = TRUE;
    consolePorts.callBack = consoleConnect;
    dtCreateListener (&consolePorts, NetBufSize);
    dtCreateThread (consoleThread, 0);

    /*
    **  Print a friendly message.
    */
    printf("Console initialised on channel %o\n", channelNo);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Check whether it's time to do output
**
**  Parameters:     Name        Description.
**                  None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void consoleCheckOutput(void)
    {
    struct timeval tm;
    int i;
    u32 us;
    
    gettimeofday (&tm, NULL);
    us = tm.tv_sec * ULL(1000000) + tm.tv_usec;
    
    /* Check if it's time for another display update */
    if (us - lastDisplayUs > RefreshInterval)
        {
        /*
        **  It's been a while since the last time we sent
        **  output to the displays, which means we haven't seen
        **  a completed cycle.  A possible reason is no keyboard
        **  polling, or it might be that the display content is
        **  moving around.
        **
        **  If we have any polls at all, we send the data 
        **  corresponding to any completed polls.  If not, we
        **  simply send the entire buffer content.
        */
        if (displayIn == displayOut)
            {
            /* There was nothing at all to display... */
            lastDisplayUs = us;
            return;
            }
        if (pollIn == 0)
            {
            /* No completed polls, send the whole display ring */
#ifdef DEBUG
            printf ("sending all data %d to %d\n",
                    displayOut, displayIn);
#endif
            consoleSendOutput (displayOut, displayIn);
            
            /* Reinitialize the current poll data */
            consoleReinitPoll ();
            }
        else
            {
#ifdef DEBUG
            printf ("sending all polls %d to %d\n",
                    pollBuffer[0].start, currentPoll.start);
#endif
            consoleSendOutput (pollBuffer[0].start, 
                               currentPoll.start);
            pollIn = 0;
            }

        /*
        **  This next line is necessary at least on NetBSD, because
        **  that uses the pth version of pthreads, which is non-preemptive,
        **  so other threads like the socket listener threads in mux6676
        **  and niu don't get a chance to run; we're CPU bound here.
        ** 
        **  Don't do it on Linux because Linux has kernel based threads,
        */
#if !defined(__linux__)
        sched_yield();
#endif
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set keyboard emulation to "true" or "easy".
**
**  Parameters:     TRUE for accurate, FALSE for easy.
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
void consoleSetKeyboardTrue (bool flag)
    {
    if (keyboardTrue == flag)
        {
        return;         /* already set, just exit */
        }

    keyboardTrue = flag;
    
    consoleByte1 (Dd60SetKbTrue + keyboardTrue);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Execute function code on 6612 console.
**
**  Parameters:     Name        Description.
**                  funcCode    function code
**
**  Returns:        FcStatus
**
**------------------------------------------------------------------------*/
static FcStatus consoleFunc(PpWord funcCode)
    {
    switch (funcCode)
        {
    default:
        return(FcDeclined);

    case Fc6612Sel512DotsLeft:
    case Fc6612Sel512DotsRight:
        consoleByte1 (Dd60SetMode + Dd60Dot +
                      ((funcCode & 0100) >> (6 - 2)));  /* Screen into bit 2 */
        break;

    case Fc6612Sel64CharLeft:
    case Fc6612Sel32CharLeft:
    case Fc6612Sel16CharLeft:
    case Fc6612Sel64CharRight:
    case Fc6612Sel32CharRight:
    case Fc6612Sel16CharRight:
        consoleByte1 (Dd60SetMode +
                      ((funcCode & 0100) >> (6 - 2)) +  /* Screen into bit 2 */
                      (funcCode & 3));                  /* Char size to 0-1 */
        break;

    case Fc6612SelKeyIn:
        consoleByte1 (Dd60SetKbTrue + keyboardTrue);
        break;
        }

    activeDevice->fcode = funcCode;
    return(FcAccepted);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Perform I/O on 6612 console.
**
**  Parameters:     Name        Description.
**                  device      Device control block
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void consoleIo(void)
    {
    int ch;
    int i;
    u64 m;
    static char ts[40];
    time_t t;
    char *p;
    
    switch (activeDevice->fcode)
        {
    default:
        break;

    case Fc6612Sel64CharLeft:
    case Fc6612Sel32CharLeft:
    case Fc6612Sel16CharLeft:
    case Fc6612Sel64CharRight:
    case Fc6612Sel32CharRight:
    case Fc6612Sel16CharRight:
        if (activeChannel->full)
            {
            ch = (activeChannel->data >> 6) & Mask6;

            if (ch >= 060)
                {
                if (ch >= 070)
                    {
                    consoleSetXY (Dd60SetY, activeChannel->data);
                    }
                else
                    {
                    consoleSetXY (Dd60SetX, activeChannel->data);
                    }
                }
            else
                {
                consoleByte1 (ch);
                ch = activeChannel->data & Mask6;
                if (ch >= 060)
                    {
                    ch = 0;
                    }
                consoleByte1 (ch);

                /*
                **  Check for auto date entry.
                */
                if (autoDate)
                    {
                    /* 
                    **  See if medium char size, and text matches
                    **  next word of "enter date" message.
                    */
                    if ((activeDevice->fcode == Fc6612Sel32CharLeft ||
                         activeDevice->fcode == Fc6612Sel32CharRight) &&
                        ((activeChannel->data >> 6) & Mask6) == asciiToCdc[autoDateString[autoPos]] &&
                        (activeChannel->data & Mask6) == asciiToCdc[autoDateString[autoPos + 1]])
                        {
                        /*
                        **  It matches so far.  Let's see if we're done.
                        */
                        if (autoDateString[autoPos + 1] == 0 ||
                            autoDateString[autoPos + 2] == 0)
                            {
                            /*
                            **  Entire pattern matched, supply
                            **  auto date and time, provided that
                            **  there is no typeahead, and keyboard
                            **  is in "easy" mode.
                            */
                            autoDate = FALSE;
                            if (keyOut == keyIn && !keyboardTrue)
                                {
                                time (&t);
                                /* Note that DSD supplies punctuation */
                                strftime (ts, sizeof (ts) - 1,
                                          "%y%m%d\n%H%M%S\n", localtime (&t));
                                for (p = ts; *p; p++)
                                    {
                                    consoleQueueKey (asciiToConsole[*p]);
                                    }
                                }
                            }
                        else
                            {
                            /*
                            **  Partial match; advance the string pointer
                            */
                            autoPos += 2;
                            }
                        }
                    else
                        {
                        /*
                        **  No match, reset match position to start.
                        */
                        autoPos = 0;
                        }
                    }
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc6612Sel512DotsLeft:
    case Fc6612Sel512DotsRight:
        if (activeChannel->full)
            {
            ch = activeChannel->data;

            if (ch >= 06000)
                {
                if (ch >= 07000)
                    {
                    consoleSetXY (Dd60SetY, ch);
                    }
                else
                    {
                    consoleSetXY (Dd60SetX, ch);
                    }
                }

            activeChannel->full = FALSE;
            }
        break;

    case Fc6612SelKeyIn:
        /*
        **  The PPU asked for console input, so call this a "poll".
        */
        consolePoll ();

        if (!activeChannel->full)
            {
            /* For normal ("easy") keyboard mode we get one buffered keycode
            ** at a time.
            ** For true keyboard mode, we keep collecting buffered keycodes
            ** until we run out of news.  That way if two keys are pressed
            ** between the previous poll and the current one, we deliver
            ** those two keystrokes together rather than one at a time.
            */
            do
                {
                ch = consoleGetKey ();
                if (ch == 0)
                    {
                    keyMask = ULL(0);
                    }
                else if (ch == 0200)   /* 0200 means "no change" */
                    {
                    break;
                    }
                else 
                    {
                    if (ch & 0200)
                        {
                        keyMask &= ~(ULL(1) << (ch & 077));
                        }
                    else
                        {
                        keyMask |= ULL(1) << ch;
                        }
                    }
                }
            while (keyboardTrue);
                
            currentKey = 0;
            for (i = 0, m = 1; i < 64; i++, m <<= 1)
                {
                if (keyMask & m)
                    {
                    currentKey |= i;
                    }
                }
            activeChannel->data = currentKey;
            activeChannel->full = TRUE;
            activeChannel->status = 0;
            activeDevice->fcode = 0;
            }
        break;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle channel activation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void consoleActivate(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle disconnecting of channel.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void consoleDisconnect(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Console listener thread
**
**  Parameters:     Name        Description.
**                  param       unused
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
static ThreadFunRet consoleThread (void *param)
    {
    int i;
    NetPort *np;
    
    printf ("console thread running\n");
    
    for (;;)
        {
        np = dtFindInput (&consolePorts, 100);
        if (np == NULL)
            {
            continue;}
        
        i = consoleInput (np);
        if (i < 0)
            {
            dtClose (np, &consolePorts);
            }
        else if (!emulationActive)
            {
            /* We just executed "shutdown" */
            ThreadReturn;
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process console input
**
**  Parameters:     Name        Description.
**                  np          NetPort with input
**
**  Returns:        -1 for disconnect, 0 if command processed.
**
**------------------------------------------------------------------------*/
static int consoleInput (NetPort *np)
    {
    u8 buf;
    int i;
    int port;
    PortParam *mp;
    
    /*
    **  Try to receive a byte of data
    */
    i = recv (np->fet.connFd, &buf, 1, MSG_NOSIGNAL);
    if (i <= 0)
        {
        return -1;
        }
    
    /* Figure out which port (connection index) this is */
    port = np - consolePorts.portVec;
    mp = portVector + port;
    switch (buf & 0300)
        {
    case Dd60KeyDown:
        if (buf == Dd60KeyXoff)
            {
            mp->stopped = TRUE;
            }
        else if (buf == Dd60KeyXon)
            {
            mp->stopped = FALSE;
            mp->sendNow = TRUE;     /* Send new data ASAP */
            }
        else
            {
            consoleQueueKey (buf);
            }
        break;
    case Dd60KeyUp:
        consoleQueueKey ((buf & 077) | 0200);
        break;
    case Dd60FastRate:
        mp->interval = (buf & 077) * 20000;
        break;
    case Dd60SlowRate:
        mp->interval = (buf & 077) * 1000000;
        break;
        }
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Store one byte of display output
**
**  Parameters:     Name        Description.
**                  byte        Byte to store
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void consoleByte1 (int byte)
    {
    int nextin;
    
    nextin = displayIn + 1;
    if (nextin == DispBufSize)
        {
        nextin = 0;
        }
    if (nextin != displayOut)
        {
        displayRing[displayIn] = byte;
        displayIn = nextin;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Store two bytes of display output
**
**  Parameters:     Name        Description.
**                  byte1       First byte to store
**                  byte2       Second byte to store
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void consoleByte2 (int byte1, int byte2)
    {
    int nextin, nextin2;
    
    nextin = displayIn + 1;
    if (nextin == DispBufSize)
        {
        nextin = 0;
        }
    nextin2 = nextin + 1;
    if (nextin2 == DispBufSize)
        {
        nextin2 = 0;
        }

    if (nextin != displayOut && nextin2 != displayOut)
        {
        displayRing[displayIn] = byte1;
        displayRing[nextin] = byte2;
        displayIn = nextin2;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Get next keycode from buffer
**
**  Parameters:     Name        Description.
**
**  Returns:        keycode or 0 if nothing pending.
**                  keycode has 0200 bit set for key-up
**
**------------------------------------------------------------------------*/
static int consoleGetKey (void)
    {
    int nextout;
    struct timeval tm;
    u32 us;
    int nextin, i, j, k;
    int key;
    
    if (keyboardSendUp || keyOut == keyIn)
        {
        if (keyboardTrue)
            {
            // If we're in true keyboard mode, lack of news means
            // "no change to last input" rather than "all keys up"!
            key = 0200;
            }
        else
            {
            key = 0;
            }
        keyboardSendUp = FALSE;
        return key;
        }

    nextout = keyOut + 1;
    if (nextout == KeyBufSize)
        {
        nextout = 0;
        }
    key = keyRing[keyOut];
    sendToAll = TRUE;
    keyOut = nextout;
    if (!keyboardTrue)
        {
        // We're not doing the precise emulation, instead doing
        // regular key rollover.  So ignore key up events,
        // and send a zero code (all up) in between each key code.
        if (key & 0200)
            {
            key = 0;
            }
        else
            {
            keyboardSendUp = TRUE;
            }
        }
    return key;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Queue keyboard input.
**
**  Parameters:     Name        Description.
**                  ch          character to be queued (display code)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void consoleQueueKey(char ch)
    {
    int nextin;
    
    nextin = keyIn + 1;
    if (nextin == KeyBufSize)
        {
        nextin = 0;
        }
    if (nextin != keyOut)
        {
        keyRing[keyIn] = ch;
        keyIn = nextin;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Queue a set X/Y command
**
**  Parameters:     Name        Description.
**                  code        control code (Dd60SetX or Dd60SetY)
**                  data        channel data
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void consoleSetXY (int code, int data)
    {
    /*
    **  Accumulate a Fletcher checksum of the coordinates sent
    */
    currentPoll.sum1 += data;
    currentPoll.sum2 += currentPoll.sum1;
    
    data &= 0777;           /* Leave only the coordinate bits */
    consoleByte2 (code | (data >> 8), data & 0xff);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Record another poll
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void consolePoll (void)
    {
    int i, j, k;
    
    /* If the buffer is full, just exit. */
    if (pollIn == MaxPolls - 1)
        {
        return;
        }

    currentPoll.end = displayIn;

    /* 
    **  Scan the pollBuffer, backwards, to see if there is an entry
    **  with the same checksum as the currentPoll.  If yes, then
    **  the pollBuffer defines a display cycle starting at that entry.
    */
    for (i = pollIn - 1; i >= 0; i--)
        { 
        if (pollBuffer[i].sum1 == currentPoll.sum1 &&
            pollBuffer[i].sum2 == currentPoll.sum2)
            {
            /*
            **  We have a match.  Go send the output range of the
            **  cycle we found.  Then set the pollBuffer to empty.
            */
#ifdef DEBUG
            printf ("sending cycle %d to %d\n",
                    pollBuffer[i].start, currentPoll.start);
#endif
            consoleSendOutput (pollBuffer[i].start, currentPoll.start);
            pollIn = 0;
            break;
            }
        }

    pollBuffer[pollIn++] = currentPoll;

    /*
    **  Reinitialize currentPoll
    */
    consoleReinitPoll ();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Reinitialize the current poll data
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/

static void consoleReinitPoll (void)
    {
    currentPoll.sum1 = currentPoll.sum2 = 0;
    currentPoll.start = displayIn;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send selected range of displayRing
**
**  Parameters:     Name        Description.
**                  fd          file descriptor to send to
**                  start       displayRing index of first byte
**                  end         displayRing index of last byte + 1
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void consoleSendFrame (int fd, int start, int end)
    {
#ifdef DEBUG
    printf ("%d ", (start > end) ? end + DispBufSize - start : end - start);
    fflush (stdout);
#endif

    if (start > end)
        {
        send (fd, displayRing + start, DispBufSize - start, MSG_NOSIGNAL);
        if (end > 0)
            {
            send (fd, displayRing, end, MSG_NOSIGNAL);
            }
        }
    else
        {
        send (fd, displayRing + start, end - start, MSG_NOSIGNAL);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Send output to display programs.
**
**  Parameters:     Name        Description.
**                  start       displayRing index of first byte
**                  end         displayRing index of last byte + 1
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
static void consoleSendOutput (int start, int end)
    {
    PortParam *mp;
    NetPort *np;
    int i;
    struct timeval tm;
    u32 us;
    char buf[160];
    u8 endBlock = Dd60EndBlock;
    
    if (debugDisplay)
        {
        /*
        **  Display P registers of PPUs and CPU and current trace mask.
        */
        buf[0] = Dd60SetTrace;
        sprintf(buf + 1, "PP P-reg %04o %04o %04o %04o %04o %04o %04o %04o %04o %04o   CPU P-reg %06o",
                ppu[0].regP, ppu[1].regP, ppu[2].regP, ppu[3].regP, ppu[4].regP,
                ppu[5].regP, ppu[6].regP, ppu[7].regP, ppu[8].regP, ppu[9].regP,
                cpu[0].regP); 

        if (cpuCount > 1)
            {
            sprintf(buf + strlen(buf), " %06o", cpu[1].regP);
            }
            
        sprintf(buf + strlen(buf),
                "   Trace %c%c%c%c%c%c%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c%c%c%c%c",
                (traceMask >> 0) & 1 ? '0' : '-',
                (traceMask >> 1) & 1 ? '1' : '-',
                (traceMask >> 2) & 1 ? '2' : '-',
                (traceMask >> 3) & 1 ? '3' : '-',
                (traceMask >> 4) & 1 ? '4' : '-',
                (traceMask >> 5) & 1 ? '5' : '-',
                (traceMask >> 6) & 1 ? '6' : '-',
                (traceMask >> 7) & 1 ? '7' : '-',
                (traceMask >> 8) & 1 ? '8' : '-',
                (traceMask >> 9) & 1 ? '9' : '-',
                (traceMask & TraceCpu0) ? 'c' : '-',
                (traceMask & TraceCpu1) ? 'C' : '-',
                (traceMask & TraceEcs) ? 'E' : '-',
                (traceMask & TraceXj) ? 'J' : '-',
                (chTraceMask >> 0) & 1 ? '0' : '-',
                (chTraceMask >> 1) & 1 ? '1' : '-',
                (chTraceMask >> 2) & 1 ? '2' : '-',
                (chTraceMask >> 3) & 1 ? '3' : '-',
                (chTraceMask >> 4) & 1 ? '4' : '-',
                (chTraceMask >> 5) & 1 ? '5' : '-',
                (chTraceMask >> 6) & 1 ? '6' : '-',
                (chTraceMask >> 7) & 1 ? '7' : '-',
                (chTraceMask >> 8) & 1 ? '8' : '-',
                (chTraceMask >> 9) & 1 ? '9' : '-',
                (chTraceMask >> 10) & 1 ? 'A' : '-',
                (chTraceMask >> 11) & 1 ? 'B' : '-');
        for (i = 1; buf[i] != '\0'; i++)
            {
            buf[i] = asciiToCdc[buf[i]];
            }
        }

    gettimeofday (&tm, NULL);
    us = tm.tv_sec * ULL(1000000) + tm.tv_usec;
    
    for (i = 0; i < dd60Conns; i++)
        {
        mp = portVector + i;
        np = consolePorts.portVec + i;
        if (mp->stopped || !dtActive (&np->fet))
            {
            /*
            **  Always skip a display that is stopped or not connected.
            */
            continue;
            }
        if (sendToAll ||
            mp->sendNow ||
            us - mp->lastFrame >= mp->interval)
            {
            mp->sendNow = FALSE;
            mp->lastFrame = us;

            /* Start by sending the trace line, if enabled */
            if (debugDisplay)
                {
                send (np->fet.connFd, buf, strlen (buf), MSG_NOSIGNAL);
                }
            
            if (mp->interval == 0)
                {
                /* 
                **  This display wants all data, so send data
                **  starting with oldest (displayOut).
                */
                consoleSendFrame (np->fet.connFd, displayOut, end);
                }
            else
                {
                /*
                **  Typical case -- send only the cycle we found.
                */
                consoleSendFrame (np->fet.connFd, start, end);
                }
            /*
            **  Send "end of data block" marker
            */
            send (np->fet.connFd, &endBlock, 1, MSG_NOSIGNAL);
            }
        mp++;
        }
    /*
    **  Remember the current time as the last display output time.
    **  Also reset the display ring start of data pointer to be
    **  just after the last byte we sent.
    */
    lastDisplayUs = us;
    displayOut = end;
    sendToAll = FALSE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Handle console connect or disconnect
**
**  Parameters:     Name        Description.
**                  np          NetPort pointer
**                  index       NetPort index
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
static void consoleConnect (NetPort *np, int index)
    {
    PortParam *mp;

    mp = portVector + index;
    
    if (np->fet.connFd == 0)
        {
        printf("%s Console connection dropped from %s\n",
               dtNowString (), inet_ntoa (np->from));
        activeConns--;
        consoleUpdateStatus ();
        return;
        }

    printf("%s Received console connection from %s\n",
           dtNowString (), inet_ntoa (np->from));

    activeConns++;
    consoleUpdateStatus ();

    /*
    **  The console program should send us what it wants for
    **  the refresh interval.  Until we hear that, keep the display
    **  stopped so we don't swamp a slow link immediate after it
    **  connects.  The default interval set here (one second) is
    **  just an extra safeguard.
    */
    mp->stopped = TRUE;
    mp->interval = 1000000;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Update operator status
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void consoleUpdateStatus (void)
    {
    char msg[64];
    
    sprintf (msg, "%d connection%s", activeConns,
             (activeConns != 1) ? "s" : "");
    if (activeConns == 0)
        {
        opSetStatus (statusBuf, NULL);
        }
    else
        {
        opSetStatus (statusBuf, msg);
        }
    }


/*---------------------------  End Of File  ------------------------------*/
