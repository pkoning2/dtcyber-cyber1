/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: pterm_x11.c
**
**  Description:
**      Simulate Plato IV terminal on X11R6.
**
**--------------------------------------------------------------------------
*/

//#define DEBUG 

/*
**  -------------
**  Include Files
**  -------------
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <sys/time.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define KeyBufSize	    50
#define DisplayMargin	20

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for loadable characters.
#define XSize           (512 + 2 * DisplayMargin)
#define YSize           (512 + 2 * DisplayMargin)
#define YPMSize         (512 + 2 * DisplayMargin + 2 * 16)

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define XADJUST(x) ((x) + DisplayMargin)
#define YADJUST(y) (YSize - DisplayMargin - (y))

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef void niuProcessOutput (int, u32);

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void loadChar (const u16 *cdat, int snum, int cnum);
static void setWeMode (u8 wemode);
static void drawChar (Drawable d, GC gc, int snum, int cnum);
static void writeChar (int snum, int cnum, const u16 *data);
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
extern Display *disp;
Window ptermWindow;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static int fontId;
static u16 currentX;
static u16 currentY;
static u16 margin;
static Pixmap pixmap;
static GC wgc, pgc;
static unsigned long fg, bg, pfg, pbg;
static u8 mode;
static u8 wemode;
static u16 memaddr;
static u16 plato_m23[128 * 8];
static u16 memlpc;
static u8 currentCharset;
static bool uncover;

typedef void (*mptr)();

const mptr modePtr[] = 
{
    &mode0, &mode1, &mode2, &mode3,
    &mode4, &mode5, &mode6, &mode7
};


/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/
extern void niuSetOutputHandler (niuProcessOutput *h, int stat);

/*--------------------------------------------------------------------------
**  Purpose:        Initialize the Plato terminal window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermInit(const char *winName)
{
    int i;
    XWMHints wmhints;
    int screen;
    XWindowAttributes a;
    XColor b,c;

    /*
    **  Open the X11 display.
    */
    if (disp == NULL)
    {
        disp = XOpenDisplay (0);
    }
    if (disp == (Display *) NULL)
    {
        fprintf(stderr, "Could not open display\n");
        exit(1);
    }

    screen = DefaultScreen(disp);

    /*
    **  Create a window using the following hints.
    */
    bg = BlackPixel (disp, screen);
    fg = WhitePixel (disp, screen);

    ptermWindow = XCreateSimpleWindow (disp, DefaultRootWindow(disp),
                                       10, 10, XSize, YSize, 5, fg, bg);

    /*
    **  Create a pixmap for background image generation.
    */
    pixmap = XCreatePixmap (disp, ptermWindow, XSize, YPMSize, 1);

    /*
    **  Set window and icon titles.
    */
    XSetStandardProperties (disp, ptermWindow, winName, "Pterm",
                            None, NULL, 0, NULL);

    /*
    **  Create the graphics contexts for window and pixmap.
    */
    wgc = XCreateGC (disp, ptermWindow, 0, 0);
    pgc = XCreateGC (disp, pixmap, 0, 0);

    /*
    **  We don't want to get Expose events right now.
    */
    XSetGraphicsExposures (disp, wgc, FALSE);
    XSetGraphicsExposures (disp, pgc, FALSE);

    /*
    **  Setup fore- and back-ground colors.
    */
    XGetWindowAttributes (disp,ptermWindow,&a);
    XAllocNamedColor (disp, a.colormap,"orange",&b,&c);
    fg=b.pixel;
    bg = BlackPixel (disp, screen);
    XSetBackground (disp, wgc, bg);
    XSetForeground (disp, wgc, fg);

    /*
    **  Ditto for pixmap
    */
    pfg = WhitePixel (disp, screen);
    pbg = BlackPixel (disp, screen);
    XSetBackground (disp, pgc, pbg);
    XSetForeground (disp, pgc, pfg);

    /*
    **  Set line style
    */
    XSetLineAttributes (disp, wgc, 0, LineSolid, CapButt, JoinBevel);
    XSetLineAttributes (disp, pgc, 0, LineSolid, CapButt, JoinBevel);
    
    /*
    **  Initialize the window and pixmap.
    */
    XSetForeground (disp, wgc, bg);
    XFillRectangle (disp, ptermWindow, wgc, 0, 0, XSize, YSize);
    XSetForeground (disp, pgc, pbg);
    XFillRectangle (disp, pixmap, pgc, 0, 0, XSize, YPMSize);
    wemode = 1;
    setWeMode (wemode);

    /*
    **  Initialise input.
    */
    wmhints.flags = InputHint;
    wmhints.input = True;
    XSetWMHints (disp, ptermWindow, &wmhints);
    XSelectInput (disp, ptermWindow,
                  ExposureMask | KeyPressMask | StructureNotifyMask);

    /*
    **  Load Plato font
    */
    fontId = XLoadFont(disp, "-*-plato-medium-*-*-*-8-*-*-*-*-*-*-*\0");
    XSetFont(disp, wgc, fontId);
    XSetFont(disp, pgc, fontId);

    /*
    **  We like to be on top.
    */
    XMapRaised (disp, ptermWindow);
    XSync (disp, FALSE);

#if 0
// testing...
    int j;
    
    mode = 1;
    wemode = 1;
    setWeMode (wemode);
    for (i = 0; i < 0774; i += 01)
    {
        currentY = i;
        currentX = 0;
        mode1 (0777000 + (i));
    }
    for (i = 0; i < 63; i++)
    {
        for (j = 0; j < 8; j++)
        {
            plato_m23[j] = (i << 8) + i;
            plato_m23[j + 8] = (((i + j) << 8) + i + j);
        }
        writeChar (0, i, plato_m23);
        writeChar (1, i, plato_m23 + 8);
    }
#define pc(c) { \
        drawChar (ptermWindow, wgc, currentCharset, c); \
        drawChar (pixmap, pgc, currentCharset, c); \
        currentX = (currentX + 8) & 0777; \
    }
        
    for (wemode = 0; wemode < 4; wemode++)
    {
        setWeMode (wemode);
        currentX = 0;
        mode = 3;
        mode3(0777720);
        currentY = 400 - 80 * wemode;
        for (i = 0; i < 64; i++)
            pc (i);
        mode3(0777715);
        mode3(0777721);
        for (i = 0; i < 64; i++)
            pc (i);
        mode3(0777715);
        mode3(0777722);
        for (i = 0; i < 64; i++)
            pc (i);
        mode3(0777715);
        mode3(0777723);
        for (i = 0; i < 64; i++)
            pc (i);
        mode3(0777715);
    }
// end testing
#endif

    /*
    **  Register with NIU for local output.
    */
    niuSetOutputHandler (procNiuWord, 1);
}

/*--------------------------------------------------------------------------
**  Purpose:        Close window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermClose(void)
{
    XFreeGC (disp, wgc);
    XFreeGC (disp, pgc);
    XFreePixmap (disp, pixmap);
    XDestroyWindow (disp, ptermWindow);
}


/*--------------------------------------------------------------------------
**  Purpose:        Process XKeyEvent for Plato keyboard
**
**  Parameters:     Name        Description.
**                  kp          XKeyEvent for the keypress
**                  stat        Station number
**
**  Returns:        TRUE if key event was a valid Plato keycode.
**
**------------------------------------------------------------------------*/
bool platoKeypress (XKeyEvent *kp, int stat)
{
    int state = kp->state;
    int key;
    KeySym ks;
    u8 shift = 0;
    int pc;
    char text[30];
    int len;
    
    if (state & ControlMask)
    {
        return FALSE;
    }
    if (state & ShiftMask)
    {
        state &= ~ShiftMask;
        shift = 040;
    }
    key = XKeycodeToKeysym (disp, kp->keycode, 0);
    if (key < sizeof (asciiToPlato))
    {
        if (state & Mod1Mask)
        {
            pc = altKeyToPlato[key] | shift;
            if (pc >= 0)
            {
                niuLocalKey (pc, stat);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            len = XLookupString (kp, text, 10, &ks, 0);
            if (len > 1)
            {
                return FALSE;
            }
            pc = asciiToPlato[text[0]];
            if (pc >= 0)
            {
                niuLocalKey (pc, stat);
                return TRUE;
            }
            
        }
    }
    switch (key)
    {
    case XK_space:
    case XK_KP_Space:
        pc = 0100;      // space
        break;
    case XK_BackSpace:
    case XK_Delete:
    case XK_KP_Delete:
        pc = 023;       // erase
        break;
    case XK_Return:
    case XK_Linefeed:
    case XK_KP_Enter:
        pc = 026;       // next
        break;
    case XK_Home:
    case XK_KP_Home:
        pc = 030;       // back
        break;
    case XK_Pause:
    case XK_Break:
        pc = 032;       // stop
        break;
    case XK_Tab:
        pc = 014;       // tab
        break;
    case XK_KP_Add:
        pc = 016;       // +
        break;
    case XK_KP_Subtract:
        pc = 017;       // -
        break;
    case XK_KP_Multiply:
        pc = 012;       // multiply sign
        break;
    case XK_KP_Divide:
        pc = 013;       // divide sign
        break;
    case XK_Left:
    case XK_KP_Left:
        pc = 015;       // assignment arrow
        break;
    case XK_Up:
    case XK_KP_Up:
    case XK_Page_Up:
        pc = 020;       // super
        break;
    case XK_Down:
    case XK_KP_Down:
    case XK_Page_Down:
        pc = 021;       // sub
        break;
    default:
        return FALSE;
    }
    pc |= shift;
    niuLocalKey (pc, stat);
    return TRUE;
}

/*--------------------------------------------------------------------------
**  Purpose:        Window input event processor.
**
**  Parameters:     Name        Description.
**                  event       pointer to X event struct.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermInput(XEvent *event)
{
    switch (event->type)
    {
    case MappingNotify:
        XRefreshKeyboardMapping ((XMappingEvent *)event);
        break;

    case KeyPress:
        platoKeypress ((XKeyEvent *) event, 1);
        break;

    case Expose:
        XSetForeground (disp, wgc, fg);
        XSetFunction (disp, wgc, GXcopy);
        XCopyPlane(disp, pixmap, ptermWindow, wgc, 
                   DisplayMargin, DisplayMargin,
                   512, 512, 
                   DisplayMargin, DisplayMargin, 1);
        setWeMode (wemode);
        XSync (disp, FALSE);
        break;
    }
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
    
    if (stat != 1)
    {
        return;
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
            break;

        case 1:     // load mode
            mode = (d >> 3) & 7;
            wemode = (d >> 1) & 3;
#ifdef DEBUG
            printf ("load mode %d %d %d\n", mode, wemode, d & 1);
#endif
            if (d & 1)
            {
                // full screen erase
                setWeMode (0);
                XFillRectangle (disp, ptermWindow, wgc, 0, 0, XSize, YSize);
                XFillRectangle (disp, pixmap, pgc, 0, 0, XSize, YSize);
                setWeMode (wemode);
                XFlush (disp);
            }
            else
            {
                setWeMode (wemode);
            }
            break;
            
        case 2:     // load coordinate
#ifdef DEBUG
            printf ("load coord %d %d %d\n",
                    d & 0777, (d >> 9) & 1, (d >> 12) & 1);
#endif
            if (d & 01000)
            {
                currentY = d & 0777;
            }
            else
            {
                currentX = d & 0777;
                if (d & 010000)
                {
                    margin = currentX;
                }
            }
            break;
        case 3:     // echo
#ifdef DEBUG
            printf ("echo %03o\n", d & 0177);
#endif
            niuLocalKey ((d & 0177) + 0200, 1);
            break;
            
        case 4:     // load address
            memaddr = d & 077777;
            break;
            
        default:    // ignore
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
**  Purpose:        Draw a character
**
**  Parameters:     Name        Description.
**                  d           X Drawable
**                  gc          Graphics context
**                  snum        character set number
**                  cnum        character number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void drawChar (Drawable d, GC gc, int snum, int cnum)
{
    int charX, charY;
    char c;
    
    if (snum < 2)
    {
        // "ROM" characters
        c = (snum * 64) + cnum;
        if (wemode & 2)
        {
            XDrawString(disp, d, gc, XADJUST (currentX),
                        YADJUST (currentY), &c, 1);
        }
        else
        {
            XDrawImageString(disp, d, gc, XADJUST (currentX),
                             YADJUST (currentY), &c, 1);
        }
    }
    else
    {
        charX = cnum * 8;
        charY = YSize + (snum - 2) * 16;
        XCopyPlane (disp, pixmap, d, gc, charX, charY, 8, 16, 
                    XADJUST (currentX), YADJUST (currentY) - 15, 1);
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Write a (loadable set) character to the font storage 
**                  part of the pixmap
**
**  Parameters:     Name        Description.
**                  snum        character set number
**                  cnum        character number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void writeChar (int snum, int cnum, const u16 *data)
{
    int i, j;
    int x = cnum * 8;
    int y = YSize + (snum & 1) * 16 + 15;
    u16 col;

    XSetForeground (disp, pgc, pbg);
    XSetFunction (disp, pgc, GXcopy);
    XFillRectangle (disp, pixmap, pgc, x, y - 15, 8, 16);
    XSetForeground (disp, pgc, pfg);

    for (i = 0; i < 8; i++)
    {
        col = *data++;
        for (j = 0; j < 16; j++)
        {
            if (col & 1)
            {
                XDrawPoint(disp, pixmap, pgc, x, y - j);
            }
            col >>= 1;
        }
        x = (x + 1) & 0777;
    }
    setWeMode (wemode);
}

// X gc function codes for a given W/E mode:
static const int WeFunc[] = 
{ GXcopy,
  GXcopy,
  GXandInverted,
  GXor 
};

/*--------------------------------------------------------------------------
**  Purpose:        Set W/E mode
**
**  Parameters:     Name        Description.
**                  we          mode byte
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void setWeMode (u8 we)
{
    // Unfortunately we can't just do wemode == 0 with GXcopyInverted,
    // partly because that produces wrong colors when writing to the
    // (orange) window, and partly because XDrawImageString doesn't 
    // pay attention to the GC function.
    if (we == 0)
    {
        XSetBackground (disp, wgc, fg);
        XSetForeground (disp, wgc, bg);
        XSetBackground (disp, pgc, pfg);
        XSetForeground (disp, pgc, pbg);
    }
    else
    {
        XSetBackground (disp, wgc, bg);
        XSetForeground (disp, wgc, fg);
        XSetBackground (disp, pgc, pbg);
        XSetForeground (disp, pgc, pfg);
    }
    XSetFunction (disp, pgc, WeFunc[we]);
    XSetFunction (disp, wgc, WeFunc[we]);
}

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
    int x, y;
    
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
        default:
            break;
        }
    }
    else
    {
        drawChar (ptermWindow, wgc, currentCharset, c);
        drawChar (pixmap, pgc, currentCharset, c);
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
#ifdef DEBUG
    printf ("dot %d %d\n", x, y);
#endif
    XDrawPoint (disp, pixmap, pgc, XADJUST (x), YADJUST (y));
    XDrawPoint (disp, ptermWindow, wgc, XADJUST (x), YADJUST (y));
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
#ifdef DEBUG
    printf ("lineto %d %d\n", x, y);
#endif
    XDrawLine (disp, pixmap, pgc,
               XADJUST (currentX), YADJUST (currentY),
               XADJUST (x), YADJUST (y));
    XDrawLine (disp, ptermWindow, wgc, 
               XADJUST (currentX), YADJUST (currentY),
               XADJUST (x), YADJUST (y));
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
        return;
    }
    if (((d >> 16) & 3) == 0)
    {
        // load data
        plato_m23[memaddr] = d & 0xffff;
        if ((++memaddr & 7) == 0)
        {
            // character is done -- load it to display 
            ch = (memaddr / 8) - 1;
            writeChar (ch / 64, ch % 64, &plato_m23[memaddr - 8]);
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
#ifdef DEBUG
    printf ("char %02o %02o %02o\n", (d >> 12) & 077, (d >> 6) & 077, d & 077);
#endif
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
}

/*---------------------------  End Of File  ------------------------------*/
