/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, 2004, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: ptermcom.c
**
**  Description:
**      Common (target independent) code to simulate Plato IV terminal
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
#include <sys/types.h>
#if defined(_WIN32)
#include <winsock.h>
#else
#include <unistd.h>
#endif
#include "const.h"
#include "types.h"
#include "proto.h"
#include "ptermversion.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
//#define PPT

#ifdef PPT
#define TERMTYPE 9
#else
#define TERMTYPE 0
#endif

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define TRACEN(str) \
    if (tracePterm) \
    { \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc); \
    }

#define TRACE(str, arg) \
    if (tracePterm) \
    { \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg); \
    }

#define TRACE2(str, arg, arg2) \
    if (tracePterm) \
    { \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg, arg2); \
    }

#define TRACE3(str, arg, arg2, arg3) \
    if (tracePterm) \
    { \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg, arg2, arg3); \
    }

#define TRACE4(str, a, a2, a3, a4) \
    if (tracePterm) \
    { \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, a, a2, a3, a4); \
    }

#define TRACE6(str, a, a2, a3, a4, a5, a6) \
    if (tracePterm) \
    { \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, a, a2, a3, a4, a5, a6); \
    }

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void plotChar (u8 c);
static void mode0 (u32 d);
static void mode1 (u32 d);
static void mode2 (u32 d);
static void mode3 (u32 d);
static void mode4 (u32 d);
static void mode5 (u32 d);
static void mode6 (u32 d);
static void mode7 (u32 d);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
bool tracePterm = FALSE;
volatile bool ptermActive = FALSE;
u8 mode;
u8 wemode;
FILE *traceF;
char traceFn[20];
const char *hostName;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u16 currentX;
static u16 currentY;
static u16 margin = 0;
static u16 memaddr;
static u16 plato_m23[128 * 8];
static u16 memlpc;
static u8 currentCharset;
static bool uncover;
static int wc;
static int seq;

typedef void (*mptr)();

const mptr modePtr[] = 
{
    &mode0, &mode1, &mode2, &mode3,
    &mode4, &mode5, &mode6, &mode7
};

const char rom0char[] =
    ":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.÷[]%×«'\"!;<>+?»U";

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
extern void ptermSetName (const char *winName);
extern void ptermLoadChar (int snum, int cnum, const u16 *data);
extern void ptermSetWeMode (u8 we);

/*--------------------------------------------------------------------------
**  Purpose:        Common initialization for the Plato simulation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermComInit(void)
{
    /*
    **  Set trace file name
    */
    sprintf (traceFn, "pterm%d.trc", getpid ());
    
    /*
    **  Do some common state initialization.
    */
    ptermTouchPanel (FALSE);
}

/*--------------------------------------------------------------------------
**  Purpose:        Common termination for the Plato simulation.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermComClose(void)
{
}

/*--------------------------------------------------------------------------
**  Purpose:        Process NIU word
**
**  Parameters:     Name        Description.
**                  stat        Station number
**                  d           19-bit word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void procNiuWord (int stat, u32 d)
{
    mptr mp;
    char *msg = "";
    char name[100];
    
    if (stat != 1)
    {
        return;
    }

    seq++;
    if (tracePterm)
    {
        fprintf (traceF, "%07o ", d);
    }
	if ((d & 01700000) == 0)
    {
        // NOP command...
#ifdef PPT
        if (d & 1)
        {
            wc = (wc + 1) & 0177;
        }
#endif
    }
    else
    {
        wc = (wc + 1) & 0177;
    }
    
    if (d & 01000000)
    {
        mp = modePtr[mode];
        (*mp) (d);
    }
    else
    {
        switch ((d >> 15) & 7)
        {
        case 0:     // nop
            if ((d & 077000) == 042000)
            {
                // Special code to tell pterm the station number
                d &= 0777;
                if (hostName != NULL)
                {
                    sprintf (name, "Pterm " PTERMVERSION ": station %d-%d -- %s",
                             d >> 5, d & 31, hostName);
                }
                else
                {
                    sprintf (name, "Pterm " PTERMVERSION ": station %d-%d",
                             d >> 5, d & 31);
                }
                ptermSetName (name);
            }
            else if (d == 1)
            {
#if defined(_WIN32)
                Sleep(1000 / 60);
#else
                usleep(1000000 / 60);
#endif
            }
            
            TRACEN ("nop");
            break;

        case 1:     // load mode
            if ((d & 020000) != 0)
            {
                // load wc bit is set
                wc = (d >> 6) & 0177;
            }
            
            wemode = (d >> 1) & 3;
#ifdef PPT
            mode = (d >> 3) & 7;
#else
            mode = (d >> 3) & 3;
            if (wemode == 0)
            {
                wemode = 2;
            }
#endif
            if (d & 1)
            {
                // full screen erase
                ptermFullErase ();
            }
            ptermSetWeMode (wemode);
            // bit 15 set is DISable
            ptermTouchPanel ((d & 040000) == 0);
            TRACE4 ("load mode %d inhibit %d wemode %d screen %d",
                    mode, (d >> 14) & 1, wemode, (d & 1));
            break;
            
        case 2:     // load coordinate
            if (d & 01000)
            {
                currentY = d & 0777;
            }
            else
            {
                currentX = d & 0777;
#ifdef PPT
                if (d & 010000)
                {
                    margin = currentX;
                    msg = "margin";
                }
#endif
            }
            TRACE3 ("load coord %c %d %s",
                   (d & 01000) ? 'Y' : 'X', d & 0777, msg);
            break;
        case 3:     // echo
            // 160 is terminal type query
            if ((d & 0177) == 0160)
            {
                TRACE ("load echo termtype %d", TERMTYPE);
                niuLocalKey (0360 + TERMTYPE, stat);
                break;
            }
            TRACE ("load echo %d", d & 0177);
            niuLocalKey ((d & 0177) + 0200, stat);
            break;
            
        case 4:     // load address
#ifdef PPT
            memaddr = d & 077777;
#else
            memaddr = d & 01777;
#endif
            TRACE ("load address %o", memaddr);
            break;
            
#ifdef PPT
        case 5:     // SSF on PPT, Load Slide on PLATO IV
            switch ((d >> 10) & 037)
            {
#if 0
            case 1: // Touch panel control ?
                TRACE ("ssf touch %o", d);
                ptermTouchPanel ((d & 040) != 0);
                break;
#endif
            default:
                TRACE ("ssf %o", d);
                break;  // ignore
            }
            break;
#endif

        default:    // ignore
            TRACE ("ignored command word %07o", d);
            break;
        }
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
**  Purpose:        Draw one character
**
**  Parameters:     Name        Description.
**                  c           Character code
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void plotChar (u8 c)
{
    c &= 077;
    if (c == 077)
    {
        uncover = TRUE;
        return;
    }
    if (uncover)
    {
        uncover = FALSE;
        switch (c)
        {
        case 010:   // backspace
            currentX = (currentX - 8) & 0777;
            break;
        case 011:   // tab
            currentX = (currentX + 8) & 0777;
            break;
        case 012:   // linefeed
            currentY = (currentY - 16) & 0777;
            break;
        case 013:   // vertical tab
            currentY = (currentY + 16) & 0777;
            break;
        case 014:   // form feed
            currentX = 0;
            currentY = 496;
            break;
        case 015:   // carriage return
            currentX = margin;
            currentY = (currentY - 16) & 0777;
            break;
        case 016:   // superscript
            currentY = (currentY + 5) & 0777;
            break;
        case 017:   // subscript
            currentY = (currentY - 5) & 0777;
            break;
        case 020:   // select M0
        case 021:   // select M1
        case 022:   // select M2
        case 023:   // select M3
            currentCharset = c - 020;
            break;
#ifdef PPT
            // tbd: PPT only uncover codes, all the higher ones.
#endif
        default:
            break;
        }
    }
    else
    {
        ptermDrawChar (currentX, currentY, currentCharset, c);
        currentX = (currentX + 8) & 0777;
    }
}


/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 0 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mode0 (u32 d)
{
    int x, y;
    
    x = (d >> 9) & 0777;
    y = d & 0777;
    TRACE2 ("dot %d %d", x, y);
    ptermDrawPoint (x, y);
    currentX = x;
    currentY = y;
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 1 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mode1 (u32 d)
{
    int x, y;
    
    x = (d >> 9) & 0777;
    y = d & 0777;
    TRACE2 ("lineto %d %d", x, y);
    ptermDrawLine (currentX, currentY, x, y);
    currentX = x;
    currentY = y;
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 2 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mode2 (u32 d)
{
    int ch;
    
    if (memaddr >= 127 * 8)
    {
        TRACEN ("memdata, address out of range");
        return;
    }
    if (((d >> 16) & 3) == 0)
    {
        // load data
        TRACE2 ("memdata %06o to %04o", d & 0xffff, memaddr);
        plato_m23[memaddr] = d & 0xffff;
        if ((++memaddr & 7) == 0)
        {
            // character is done -- load it to display 
            ch = (memaddr / 8) - 1;
            ptermLoadChar (2 + (ch / 64), ch % 64, &plato_m23[memaddr - 8]);
        }
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 3 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mode3 (u32 d)
{
    TRACE6 ("char %02o %02o %02o (%c%c%c)",
           (d >> 12) & 077, (d >> 6) & 077, d & 077,
           rom0char[(d >> 12) & 077], 
           rom0char[(d >> 6) & 077], 
           rom0char[d & 077]);
    plotChar (d >> 12);
    plotChar (d >> 6);
    plotChar (d);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 4 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mode4 (u32 d)
{
    TRACE ("mode4 %06o", d);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 5 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mode5 (u32 d)
{
    TRACE ("mode5 %06o", d);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 6 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mode6 (u32 d)
{
    TRACE ("mode6 %06o", d);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 7 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void mode7 (u32 d)
{
    TRACE ("mode7 %06o", d);
}

/*---------------------------  End Of File  ------------------------------*/
