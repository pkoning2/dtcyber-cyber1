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
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
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

#ifdef __APPLE__
#define MODMASK 8192    // Option key
#else
#define MODMASK Mod1Mask
#endif

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
#define YADJUST(y) (YSize - 1 - DisplayMargin - (y))

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
static void drawChar (Drawable d, GC gc, int x, int y, int snum, int cnum);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
Display *disp;
XrmDatabase XrmDb;
Window ptermWindow;

extern bool tracePterm;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static bool ptermActive = FALSE;
static int fontId;
static Pixmap pixmap;
static GC wgc, pgc;
static unsigned long fg, bg, pfg, pbg;
static u8 wemode;       // local copy of most recently set wemode
static Cursor curs;
static bool touchEnabled;
static int sts;

// X gc function codes for a given W/E mode:
static const int WeFunc[] = 
{ GXcopy,
  GXcopy,
  GXandInverted,
  GXor 
};

static XRectangle platoRect[] = 
{ { XADJUST (0), YADJUST (511), 512, 512 },
  { 0, YSize, 512, 32 } };

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

bool platoKeypress (XKeyEvent *kp, int stat);
void ptermInput(XEvent *event);
void ptermXinit(void);

/*--------------------------------------------------------------------------
**  Purpose:        Common X initialization for DtCyber and Pterm
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermXinit (void)
{
    char *home;
    char *xf;
    XrmDatabase xdef, appdef;
    
    /*
    **  Open the X11 display.
    */
    disp = XOpenDisplay (0);
    if (disp == NULL)
    {
        fprintf(stderr, "Could not open display\n");
        exit(1);
    }
    
    XrmInitialize ();
    appdef = XrmGetFileDatabase ("/usr/lib/X11/app-defaults/Dtcyber");
    if (appdef == NULL)
    {
        fprintf (stderr, "no app default database for dtcyber\n");
    }
    home = getenv ("HOME");
    xf = malloc (strlen (home) + strlen ("/.Xdefaults") + 1);
    strcpy (xf, home);
    strcat (xf, "/.Xdefaults");
    xdef = XrmGetFileDatabase (xf);
    free (xf);
    if (xdef != NULL)
    {
        XrmCombineDatabase (appdef, &xdef, FALSE);
    }
    else
    {
        xdef = appdef;
    }
    XrmDb = xdef;
}

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
    XrmValue value;
    char *type[20];
    char fgcolor[40], bgcolor[40];

    if (disp == NULL)
    {
        ptermXinit ();
    }
    
    screen = DefaultScreen(disp);

    /*
    **  Look for resources
    */
    if (XrmGetResource (XrmDb, "dtcyber.platoforeground",
                        "Dtcyber.PlatoForeground", type, &value))
    {
        strncpy (fgcolor, value.addr, (int) value.size);
    }
    else
    {
        strcpy (fgcolor, "#ff9000");
    }
    if (XrmGetResource (XrmDb, "dtcyber.platobackground",
                        "Dtcyber.PlatoBackground", type, &value))
    {
        strncpy (bgcolor, value.addr, (int) value.size);
    }
    else if (XrmGetResource (XrmDb, "dtcyber.background",
                        "Dtcyber.Background", type, &value))
    {
        strncpy (bgcolor, value.addr, (int) value.size);
    }
    else
    {
        strcpy (bgcolor, "#000000");
    }
    
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
    XGetWindowAttributes (disp, ptermWindow, &a);
    sts = XAllocNamedColor (disp, a.colormap, fgcolor, &b, &c);
    fg = b.pixel;
    sts = XAllocNamedColor (disp, a.colormap, bgcolor, &b, &c);
    bg = b.pixel;
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
    ptermSetWeMode (1);

    /*
    **  Set clipping rectangles
    **  The display gets one, the PLATO screen.
    **  The bitmap gets a second one for the loadable chars bitmap.
    **  (Painting of the trace marker is handled separately.)
    */
    XSetClipRectangles (disp, wgc, 0, 0, platoRect, 1, YXSorted);
    XSetClipRectangles (disp, pgc, 0, 0, platoRect, 2, YXSorted);
    
    /*
    **  Initialise input.
    */
    wmhints.flags = InputHint;
    wmhints.input = True;
    XSetWMHints (disp, ptermWindow, &wmhints);
    XSelectInput (disp, ptermWindow,
                  ExposureMask | KeyPressMask | StructureNotifyMask);

    /*
    **  Find the "hand" cursor
    */
    curs = XCreateFontCursor(disp, XC_hand2);
    
    /*
    **  Load Plato font
    */
    fontId = XLoadFont(disp, "-*-plato-medium-*-*-*-8-*-*-*-*-*-*-*\0");
    XSetFont(disp, wgc, fontId);
    XSetFont(disp, pgc, fontId);

    /*
    **  Should we be on top?  Probably not...
    */
//    XMapRaised (disp, ptermWindow);
    XMapWindow (disp, ptermWindow);
    XSync (disp, FALSE);

    /*
    **  Now do common init stuff
    */
    ptermComInit ();
    ptermActive = TRUE;
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
    if (!ptermActive)
    {
        return;
    }
    
    ptermComClose ();
    XFreeCursor (disp, curs);
    XFreeGC (disp, wgc);
    XFreeGC (disp, pgc);
    XFreePixmap (disp, pixmap);
    XDestroyWindow (disp, ptermWindow);
}


/*--------------------------------------------------------------------------
**  Purpose:        Set W/E mode
**
**  Parameters:     Name        Description.
**                  we          mode byte
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermSetWeMode (u8 we)
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
    wemode = we;
}

/*--------------------------------------------------------------------------
**  Purpose:        Draw one character
**
**  Parameters:     Name        Description.
**                  x           X coordinate to draw
**                  y           Y coordinate to draw
**                  snum        character set number
**                  cnum        character number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermDrawChar (int x, int y, int snum, int cnum)
{
    drawChar (ptermWindow, wgc, x, y, snum, cnum);
    drawChar (pixmap, pgc, x, y, snum, cnum);
}


/*--------------------------------------------------------------------------
**  Purpose:        Draw a point
**
**  Parameters:     Name        Description.
**                  x           X coordinate of point
**                  y           Y coordinate of point
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermDrawPoint (int x, int y)
{
    XDrawPoint (disp, pixmap, pgc, XADJUST (x), YADJUST (y));
    XDrawPoint (disp, ptermWindow, wgc, XADJUST (x), YADJUST (y));
}

/*--------------------------------------------------------------------------
**  Purpose:        Draw a line
**
**  Parameters:     Name        Description.
**                  x1          starting X coordinate of line
**                  y1          starting Y coordinate of line
**                  x2          ending X coordinate
**                  y2          ending Y coordinate
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermDrawLine(int x1, int y1, int x2, int y2)
{
    XDrawLine (disp, pixmap, pgc,
               XADJUST (x1), YADJUST (y1),
               XADJUST (x2), YADJUST (y2));
    XDrawLine (disp, ptermWindow, wgc, 
               XADJUST (x1), YADJUST (y1),
               XADJUST (x2), YADJUST (y2));
}


/*--------------------------------------------------------------------------
**  Purpose:        Process Plato full screen erase.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**
**------------------------------------------------------------------------*/
void ptermFullErase (void)
{
    u8 savemode = wemode;
    
    ptermSetWeMode (0);
    XFillRectangle (disp, ptermWindow, wgc,
                    DisplayMargin, DisplayMargin, 512, 512);
    XFillRectangle (disp, pixmap, pgc,
                    DisplayMargin, DisplayMargin, 512, 512);
    ptermSetWeMode (savemode);
    XFlush (disp);
}

/*--------------------------------------------------------------------------
**  Purpose:        Write a (loadable set) character to the font storage 
**                  part of the pixmap
**
**  Parameters:     Name        Description.
**                  snum        character set number
**                  cnum        character number
**                  data        vector of 8 uint16s with column pattern
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermLoadChar (int snum, int cnum, const u16 *data)
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
    ptermSetWeMode (wemode);
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
        if (state & MODMASK)
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
    u8 savemode = wemode;

    switch (event->type)
    {
    case MappingNotify:
        XRefreshKeyboardMapping ((XMappingEvent *)event);
        break;

    case KeyPress:
        platoKeypress ((XKeyEvent *) event, 1);
        break;

    case Expose:
        ptermSetWeMode (0);
        XFillRectangle (disp, ptermWindow, wgc,
                        0, 0, XSize, YSize);
        XSetForeground (disp, wgc, fg);
        XSetBackground (disp, wgc, bg);
        XSetFunction (disp, wgc, GXcopy);
        XCopyPlane(disp, pixmap, ptermWindow, wgc, 
                   DisplayMargin, DisplayMargin,
                   512, 512, 
                   DisplayMargin, DisplayMargin, 1);
        // copy the trace marker
        if (tracePterm)
        {
            XSetClipMask (disp, wgc, None);
            XSetClipMask (disp, pgc, None);
            XCopyPlane(disp, pixmap, ptermWindow, wgc, 
                       DisplayMargin + 512, DisplayMargin - 16,
                       8, 16,
                       DisplayMargin + 512, DisplayMargin - 16, 1);
            XSetClipRectangles (disp, wgc, 0, 0, platoRect, 1, YXSorted);
            XSetClipRectangles (disp, pgc, 0, 0, platoRect, 2, YXSorted);
        }
        ptermSetWeMode (savemode);
        XSync (disp, FALSE);
        break;
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Enable or disable "touch" input
**
**  Parameters:     Name        Description.
**                  enable      true or false for enable or disable.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermTouchPanel(bool enable)
{
    if (enable)
    {
        XDefineCursor (disp, ptermWindow, curs);
    }
    else
    {
        XUndefineCursor (disp, ptermWindow);
    }
    touchEnabled = enable;
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
**                  x           X coordinate to draw
**                  y           Y coordinate to draw
**                  snum        character set number
**                  cnum        character number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void drawChar (Drawable d, GC gc, int x, int y, int snum, int cnum)
{
    int charX, charY;
    char c;
    
    if (x >= 1024)
    {
        // Special flag coordinate to write to the status field
        XSetClipMask (disp, wgc, None);
        XSetClipMask (disp, pgc, None);
    }
    
    if (snum < 2)
    {
        // "ROM" characters
        c = (snum * 64) + cnum;
        if (wemode & 2)
        {
            XDrawString(disp, d, gc, XADJUST (x & 1023),
                        YADJUST (y), &c, 1);
        }
        else
        {
            XDrawImageString(disp, d, gc, XADJUST (x),
                             YADJUST (y), &c, 1);
        }
    }
    else
    {
        charX = cnum * 8;
        charY = YSize + (snum - 2) * 16;
        XCopyPlane (disp, pixmap, d, gc, charX, charY, 8, 16, 
                    XADJUST (x), YADJUST (y) - 15, 1);
    }
    // Handle screen edge wraparound by recursion...
    if (x >= 1024)
    {
        // Restore normal clipping
        XSetClipRectangles (disp, wgc, 0, 0, platoRect, 1, YXSorted);
        XSetClipRectangles (disp, pgc, 0, 0, platoRect, 2, YXSorted);
    }
    else 
    {
        if (x > 512 - 8)
        {
            drawChar (d, gc, x - 512, y, snum, cnum);
        }
        if (y > 512 - 16)
        {
            drawChar (d, gc, x, y - 512, snum, cnum);
        }
    }
}

/*---------------------------  End Of File  ------------------------------*/
