/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: dd60.c
**
**  Description:
**      Simulate CDC 6612 console display, mainline and common code.
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
#if !defined (_WIN32)
#include <unistd.h>
#include <sys/time.h>
#endif
#include "const.h"
#include "types.h"
#include "proto.h"
#include "dd60.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define NetBufSize      2048
#define LineBufSize     64
#define DefaultInterval 0.06
/*
**  Screen related definitions
*/
#define DisplayMargin           20
#define OffLeftScreen           0
#define OffRightScreen          01020
#define TraceX                  (10 - DisplayMargin)
#define TraceY                  (10 - DisplayMargin)

/*
**  X and Y scale values used if scaled display enabled
*/
#define ScaleX              12
#define ScaleY              18

/*
**  Size of the window and pixmap.
**   This is: a screen high with marging top and botton, and two screens
**  wide with margins top and bottom, and 20b space in between.
*/
#define XSize       (01000 * scaleX / 10 + OffRightScreen + 2 * DisplayMargin)
#define YSize       (01000 * scaleY / 10 + 2 * DisplayMargin)

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define XADJUST(x) ((x) * scaleX / 10 + DisplayMargin + currentXOffset)
#define YADJUST(y) ((y) * scaleY / 10 + DisplayMargin)

/* turn on the next line to debug X startup troubles... */
//#define XDEBUG(disp,text) XSync(disp, 0); printf (text "\n")
#define XDEBUG(disp,text)

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/* Font metrics data for each font we use */
typedef struct fontInfo
    {
    int             normalId;       /* horizontal position */
    int             boldId;         /* size of font */
    int             width;          /* character (full) width in pixels */
    int             bwidth;         /* character body width in pixels */
    int             height;         /* character height (ascent) in pixels */
    int             pad;            /* extra padding in between chars */
    } FontInfo;

/* Data for buffered text in lineBuf */
typedef struct
    {
    char            c;              /* Character to display */
    u8              hits;           /* Number of repeats for this char */
    } CharData;
    
/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
/*
**  ----------------
**  Public Variables
**  ----------------
*/
#if defined(_WIN32)
HINSTANCE hInstance;
#endif
bool emulationActive = TRUE;
bool hersheyMode = FALSE;
int scaleX = 10;
int scaleY = 10;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static NetFet fet;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

extern void windowInit(void);
extern void windowClose(void);
extern void windowSetKeyboardTrue (bool flag);
extern int windowInput(void);
extern void windowShowDisplay (void);
extern void windowSetX (int x);
extern void windowSetY (int y);
extern void windowSetMode (int mode);
extern void windowProcessChar (int ch);


int main (int argc, char **argv)
    {
    u8 c;
    int data;
    int port;
    int i;
    double interval;
    int intervalCode;
    u8 initBuf[2];
    int readDelay;
    char opt;
    
#if !defined(_WIN32)
    for (;;)
        {
        opt = getopt (argc, argv, "hs");
        if (opt == (char) (-1))
            break;
        switch (opt)
            {
        case 'h':
            hersheyMode = TRUE;
            break;
        case 's':
            scaleX = ScaleX;
            scaleY = ScaleY;
            break;
            }
        }
    argc -= optind;
    argv += optind;
#else
    argc--; // temp
    argv++;
#endif

    if (argc > 2)
        {
        fprintf (stderr, "usage: dd60 [-hs] [ interval [ portnum ]]\n");
        exit (1);
        }
    if (argc > 1)
        {
        port = atoi (argv[1]);
        }
    else
        {
        port = DefDd60Port;
        }
    if (argc > 0)
        {
        interval = strtod (argv[0], NULL);
        if (interval < 0.02 || interval > 63.0)
            {
            fprintf (stderr, "interval value out of range\n");
            exit (1);
            }
        }
    else
        {
        interval = DefaultInterval;
        }

    /*
    **  Set the network read timeout appropriately for the
    **  display interval selected.  Keep it under 100 ms
    **  for decent keyboard responsiveness.
    */
    if (interval < 0.1)
        {
        readDelay = interval * 1000;
        }
    else
        {
        readDelay = 100;
        }

    /* 
    **  Convert the interval to an integer value in 20 ms units
    */
    intervalCode = interval / 0.02;
    
    if (intervalCode > 077)
        {
        /* If it doesn't fit, use the "slow" coding */
        intervalCode = Dd60SlowRate + (intervalCode / 50);
        }
    else
        {
        intervalCode += Dd60FastRate;
        }
    
    dtInitFet (&fet, NetBufSize);
    
    if (dtConnect (&fet.connFd, "localhost", port) < 0)
        {
        exit (1);
        }
    
#if defined(_WIN32)
    /*
    **  Get our instance
    */
    hInstance = GetModuleHandle(NULL);
#endif

    windowInit ();

    /*
    **  We're ready to go, so send the rate we want, and a
    **  "turn on display" code to DtCyber.
    */
    initBuf[0] = intervalCode;
    initBuf[1] = Dd60KeyXon;
    send (fet.connFd, initBuf, sizeof (initBuf), 0);

    while (emulationActive)
        {
        i = dtRead (&fet, readDelay);
        if (i < 0)
            {
            /* Connection went away... */
            emulationActive = FALSE;
            break;
            }
        else
            {
            /*
            **  We received some data, so process what we received
            */
            for (;;)
                {
                data = dtReado (&fet);
                if (data < 0)
                    {
                    break;
                    }
                if ((data & 0200) == 0)
                    {
                    windowProcessChar (data);
                    }
                else
                    {
                    switch (data & 0370)
                        {
                    case Dd60SetX:
                    case Dd60SetY:
                        /*
                        **  SetX and SetY are two bytes long, so get
                        **  the next byte and then proceed.  The second
                        **  byte should be here already, but if not, we
                        **  will wait for it.
                        */
                        for (;;)
                            {
                            i = dtReado (&fet);
                            if (i >= 0)
                                {
                                break;
                                }
                            dtRead (&fet, -1);
                            }
                        i |= ((data & 7) << 8);
                        if ((data & 0370) == Dd60SetX)
                            {
                            windowSetX (i);
                            }
                        else
                            {
                            windowSetY (i);
                            }
                        break;
                    case Dd60SetTrace:
                        windowSetMode (Dd60ScreenL | Dd60CharSmall);
                        windowSetX (TraceX);
                        windowSetY (0777 - TraceY);
                        break;
                    case Dd60SetKbTrue:
                        windowSetKeyboardTrue ((data & 1) != 0);
                        break;
                    case Dd60SetMode:
                        windowSetMode (data);
                        break;
                    case Dd60EndBlock:
                        windowShowDisplay ();
                        break;
                        }
                    }
                }
            }
        
        /*
        **  We've processed all pending data from the system;
        **  now look for keyboard data.
        */
#if !defined(_WIN32)
        c = windowInput ();
#else
        //tbd
#endif
        if (c != 0)
            {
            if (c & 0200)
                {
                /* Keyup code */
                c = Dd60KeyUp | (c & 077);
                }
            else
                {
                c = Dd60KeyDown | c;
                }
            send (fet.connFd, &c, 1, 0);
            }
        }
    windowClose ();
    return 0;
    }


/*--------------------------------------------------------------------------
**  Purpose:        Define Hershey glyphs.
**
**------------------------------------------------------------------------*/
const char * const consoleHersheyGlyphs[64] =
    {
#if 0
    /* 00 = ':' */ "PURPRQSQSPRP RRURVSVSURU",
    /* 01 = 'A' */ "MWRMNV RRMVV RPSTS",
    /* 02 = 'B' */ "MWOMOV ROMSMUNUPSQ ROQSQURUUSVOV",
    /* 03 = 'C' */ "MXVNTMRMPNOPOSPURVTVVU",
    /* 04 = 'D' */ "MWOMOV ROMRMTNUPUSTURVOV",
    /* 05 = 'E' */ "MWOMOV ROMUM ROQSQ ROVUV",
    /* 06 = 'F' */ "MVOMOV ROMUM ROQSQ",
    /* 07 = 'G' */ "MXVNTMRMPNOPOSPURVTVVUVR RSRVR",
    /* 10 = 'H' */ "MWOMOV RUMUV ROQUQ",
    /* 11 = 'I' */ "PTRMRV",
    /* 12 = 'J' */ "NUSMSTRVPVOTOS",
    /* 13 = 'K' */ "MWOMOV RUMOS RQQUV",
    /* 14 = 'L' */ "MVOMOV ROVUV",
    /* 15 = 'M' */ "LXNMNV RNMRV RVMRV RVMVV",
    /* 16 = 'N' */ "MWOMOV ROMUV RUMUV",
    /* 17 = 'O' */ "MXRMPNOPOSPURVSVUUVSVPUNSMRM",
    /* 20 = 'P' */ "MWOMOV ROMSMUNUQSROR",
    /* 21 = 'Q' */ "MXRMPNOPOSPURVSVUUVSVPUNSMRM RSTVW",
    /* 22 = 'R' */ "MWOMOV ROMSMUNUQSROR RRRUV",
    /* 23 = 'S' */ "MWUNSMQMONOOPPTRUSUUSVQVOU",
    /* 24 = 'T' */ "MWRMRV RNMVM",
    /* 25 = 'U' */ "MXOMOSPURVSVUUVSVM",
    /* 26 = 'V' */ "MWNMRV RVMRV",
    /* 27 = 'W' */ "LXNMPV RRMPV RRMTV RVMTV",
    /* 30 = 'X' */ "MWOMUV RUMOV",
    /* 31 = 'Y' */ "MWNMRQRV RVMRQ",
    /* 32 = 'Z' */ "MWUMOV ROMUM ROVUV",
    /* 33 = '0' */ "MWRMPNOPOSPURVTUUSUPTNRM",
    /* 34 = '1' */ "MWPORMRV",
    /* 35 = '2' */ "MWONQMSMUNUPTROVUV",
    /* 36 = '3' */ "MWONQMSMUNUPSQ RRQSQURUUSVQVOU",
    /* 37 = '4' */ "MWSMSV RSMNSVS",
    /* 40 = '5' */ "MWPMOQQPRPTQUSTURVQVOU RPMTM",
    /* 41 = '6' */ "MWTMRMPNOPOSPURVTUUSTQRPPQOS",
    /* 42 = '7' */ "MWUMQV ROMUM",
    /* 43 = '8' */ "MWQMONOPQQSQUPUNSMQM RQQOROUQVSVUUURSQ",
    /* 44 = '9' */ "MWUPTRRSPROPPNRMTNUPUSTURVPV",
    /* 45 = '+' */ "LXRNRV RNRVR",
    /* 46 = '-' */ "LXNRVR",
    /* 47 = '*' */ "MWRORU ROPUT RUPOT",
    /* 50 = '/' */ "MWVLNX",
    /* 51 = '(' */ "OUTKRNQQQSRVTY",
    /* 52 = ')' */ "OUPKRNSQSSRVPY",
    /* 53 = '$' */ "MWRKRX RUNSMQMONOPQQTRUSUUSVQVOU",
    /* 54 = '=' */ "LXNPVP RNTVT",
    /* 55 = ' ' */ "",
    /* 56 = ',' */ "PUSVRVRUSUSWRY",
    /* 57 = '.' */ "PURURVSVSURU",
    /* 60 = '#' */ "MXQLQY RTLTY ROQVQ ROTVT",
    /* 61 = '[' */ "",
    /* 62 = ']' */ "",
    /* 63 = '%' */ "",
    /* 64 = '"' */ "NVPMPQ RTMTQ",
    /* 65 = '_' */ "",
    /* 66 = '!' */ "PURMRR RSMSR RRURVSVSURU",
    /* 67 = '&' */ "LXVRURTSSURVOVNUNSORRQSPSNRMPMONOPQSSUUVVV",
    /* 70 = ''' */ "PTRMRQ",
    /* 71 = '?' */ "NWPNRMSMUNUPRQRRSRSQUP RRURVSVSURU",
    /* 72 = '<' */ "",
    /* 73 = '>' */ "",
    /* 74 = '@' */ "",
    /* 75 = '\' */ "",
    /* 76 = '^' */ "",
    /* 77 = ';' */ "PURPRQSQSPRP RSVRVRUSUSWRY",
#else
    /* 00 = ' ' */ "",
    /* 01 = 'A' */ "MWRMNV RRMVV RPSTS",
    /* 02 = 'B' */ "MWOMOV ROMSMUNUPSQ ROQSQURUUSVOV",
    /* 03 = 'C' */ "MXVNTMRMPNOPOSPURVTVVU",
    /* 04 = 'D' */ "MWOMOV ROMRMTNUPUSTURVOV",
    /* 05 = 'E' */ "MWOMOV ROMUM ROQSQ ROVUV",
    /* 06 = 'F' */ "MVOMOV ROMUM ROQSQ",
    /* 07 = 'G' */ "MXVNTMRMPNOPOSPURVTVVUVR RSRVR",
    /* 10 = 'H' */ "MWOMOV RUMUV ROQUQ",
    /* 11 = 'I' */ "PTRMRV",
    /* 12 = 'J' */ "NUSMSTRVPVOTOS",
    /* 13 = 'K' */ "MWOMOV RUMOS RQQUV",
    /* 14 = 'L' */ "MVOMOV ROVUV",
    /* 15 = 'M' */ "LXNMNV RNMRV RVMRV RVMVV",
    /* 16 = 'N' */ "MWOMOV ROMUV RUMUV",
    /* 17 = 'O' */ "MXRMPNOPOSPURVSVUUVSVPUNSMRM",
    /* 20 = 'P' */ "MWOMOV ROMSMUNUQSROR",
    /* 21 = 'Q' */ "MXRMPNOPOSPURVSVUUVSVPUNSMRM RSTVW",
    /* 22 = 'R' */ "MWOMOV ROMSMUNUQSROR RRRUV",
    /* 23 = 'S' */ "MWUNSMQMONOOPPTRUSUUSVQVOU",
    /* 24 = 'T' */ "MWRMRV RNMVM",
    /* 25 = 'U' */ "MXOMOSPURVSVUUVSVM",
    /* 26 = 'V' */ "MWNMRV RVMRV",
    /* 27 = 'W' */ "LXNMPV RRMPV RRMTV RVMTV",
    /* 30 = 'X' */ "MWOMUV RUMOV",
    /* 31 = 'Y' */ "MWNMRQRV RVMRQ",
    /* 32 = 'Z' */ "MWUMOV ROMUM ROVUV",
    /* 33 = '0' */ "MWRMPNOPOSPURVTUUSUPTNRM",
    /* 34 = '1' */ "MWPORMRV",
    /* 35 = '2' */ "MWONQMSMUNUPTROVUV",
    /* 36 = '3' */ "MWONQMSMUNUPSQ RRQSQURUUSVQVOU",
    /* 37 = '4' */ "MWSMSV RSMNSVS",
    /* 40 = '5' */ "MWPMOQQPRPTQUSTURVQVOU RPMTM",
    /* 41 = '6' */ "MWTMRMPNOPOSPURVTUUSTQRPPQOS",
    /* 42 = '7' */ "MWUMQV ROMUM",
    /* 43 = '8' */ "MWQMONOPQQSQUPUNSMQM RQQOROUQVSVUUURSQ",
    /* 44 = '9' */ "MWUPTRRSPROPPNRMTNUPUSTURVPV",
    /* 45 = '+' */ "LXRNRV RNRVR",
    /* 46 = '-' */ "LXNRVR",
    /* 47 = '*' */ "MWRORU ROPUT RUPOT",
    /* 50 = '/' */ "MWVLNX",
    /* 51 = '(' */ "OUTKRNQQQSRVTY",
    /* 52 = ')' */ "OUPKRNSQSSRVPY",
    /* 53 = ' ' */ "",
    /* 54 = '=' */ "LXNPVP RNTVT",
    /* 55 = ' ' */ "",
    /* 56 = ',' */ "PUSVRVRUSUSWRY",
    /* 57 = '.' */ "PURURVSVSURU",
    /* 60 = ' ' */ "",
    /* 61 = ' ' */ "",
    /* 62 = ' ' */ "",
    /* 63 = ' ' */ "",
    /* 64 = ' ' */ "",
    /* 65 = ' ' */ "",
    /* 66 = ' ' */ "",
    /* 67 = ' ' */ "",
    /* 70 = ' ' */ "",
    /* 71 = ' ' */ "",
    /* 72 = ' ' */ "",
    /* 73 = ' ' */ "",
    /* 74 = ' ' */ "",
    /* 75 = ' ' */ "",
    /* 76 = ' ' */ "",
    /* 77 = ' ' */ "",
#endif
    };


/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*---------------------------  End Of File  ------------------------------*/
