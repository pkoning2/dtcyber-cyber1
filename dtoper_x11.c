/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: window_x11.c
**
**  Description:
**      Simulate CDC 6612 console display on X11R6.
**
**--------------------------------------------------------------------------
*/

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
#include <sys/time.h>
#include <pthread.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define ListSize        10000
#define KeyBufSize	    50
#define MaxPolls        10      // number of poll cycles we track
#define DisplayBufSize	64
#define DisplayMargin	20

// Size of the window.
// This is: a screen high and wide, with marging top and botton
#define XSize           (02000 + 2 * DisplayMargin)
#define YSize           (01000 + 2 * DisplayMargin)

#define DtOperVersion   "DtCyber Operator Window"

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define XADJUST(x) ((x) + DisplayMargin)
#define YADJUST(y) (01000 - (y) + DisplayMargin)

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct fontInfo
    {
    int             normalId;       /* horizontal position */
    int             boldId;         /* size of font */
    int             width;          /* character (full) width in pixels */
    int             bwidth;         /* character body width in pixels */
    int             height;         /* character height (ascent) in pixels */
    int             pad;            /* extra padding in between chars */
    } FontInfo;
/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void getCharWidths (FontInfo *f, char *s);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
extern Display *disp;
extern XrmDatabase XrmDb;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static FontInfo *currentFontInfo;
static FontInfo smallOperFont;
static FontInfo mediumOperFont;
static Window window;
static GC wgc;
static unsigned long fg, bg;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/* These functions are declared extern here rather than in proto.h,
** otherwise proto.h would have to #include X.h...
*/
extern void dtXinit(void);

/*--------------------------------------------------------------------------
**  Purpose:        Initialize the display window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowInit(void)
    {
    int rc;
    KeySym modList[2];
    XWMHints wmhints;
    int screen;
    XWindowAttributes a;
    XColor b,c;
    XrmValue value;
    char *type[20];
    char fgcolor[40], bgcolor[40];
    
    /*
    **  Do common X initialization
    */
    dtXinit ();

    screen = DefaultScreen(disp);

    /*
    **  Look for resources
    */
    if (XrmGetResource (XrmDb, "dtcyber.operforeground",
                        "Dtcyber.Operforeground", type, &value))
        {
        strncpy (fgcolor, value.addr, (int) value.size);
        }
    else
        {
        strcpy (fgcolor, "green");
        }
    if (XrmGetResource (XrmDb, "dtcyber.operbackground",
                        "Dtcyber.Operbackground", type, &value))
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
    bg = BlackPixel(disp, screen);
    fg = WhitePixel(disp, screen);

    window = XCreateSimpleWindow (disp, DefaultRootWindow(disp),
                                  10, 10, XSize, YSize, 5, fg, bg);

    /*
    **  Set window and icon titles.
    */
    XSetStandardProperties (disp, window, DtOperVersion, DtOperVersion,
                            None, NULL, 0, NULL);

    /*
    **  Create the graphics contexts for window.
    */
    wgc = XCreateGC (disp, window, 0, 0);

    /*
    **  We don't want to get Expose events, otherwise every XCopyArea will generate one,
    **  and the event queue will fill up. This application will discard them anyway, but
    **  it is better not to generate them in the first place.
    */
    XSetGraphicsExposures(disp, wgc, FALSE);

    /*
    **  Load fonts (two sizes, two flavors).
    */
    smallOperFont.normalId = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-*-120-*-*-*-*-*-*\0");
    smallOperFont.boldId = XLoadFont(disp, "-*-lucidatypewriter-bold-*-*-*-*-120-*-*-*-*-*-*\0");
    mediumOperFont.normalId = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-*-180-*-*-*-*-*-*\0");
    mediumOperFont.boldId = XLoadFont(disp, "-*-lucidatypewriter-bold-*-*-*-*-180-*-*-*-*-*-*\0");
    smallOperFont.pad = 1;
    mediumOperFont.pad = 2;
    getCharWidths (&smallOperFont, "A");
    getCharWidths (&mediumOperFont, "A");

    /*
    **  Setup fore- and back-ground colors.
    */
    XGetWindowAttributes(disp,window,&a);
    XAllocNamedColor(disp, a.colormap, fgcolor, &b, &c);
    fg=b.pixel;
    XAllocNamedColor(disp, a.colormap, bgcolor, &b, &c);
    bg=b.pixel;
    XSetBackground(disp, wgc, bg);
    XSetForeground(disp, wgc, fg);

    /*
    **  Initialise input.
    */
    wmhints.flags = InputHint;
    wmhints.input = True;
    XSetWMHints(disp, window, &wmhints);
    XSelectInput (disp, window, KeyPressMask | ExposureMask |
                  StructureNotifyMask);

    /*
    **  We like to be on top.
    */
    XMapRaised (disp, window);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Get width of operator display font.
**
**  Parameters:     None.
**
**  Returns:        Width.
**
**------------------------------------------------------------------------*/
int windowGetOperFontWidth(int font)
    {
    if (font == FontSmall)
        {
        return smallOperFont.width;
        }
    else
        {
        return mediumOperFont.width;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowClose(void)
    {
    XFreeGC (disp, wgc);
    XDestroyWindow (disp, window);
    XCloseDisplay (disp);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Window input event processor.
**
**  Parameters:     Name        Description.
**
**  Returns:        Key received, or 0.
**
**------------------------------------------------------------------------*/
char opWindowInput(void)
    {
    XEvent event;
    XKeyEvent *kp;
    KeySym key;
    char text[30];
    int len;
    
    /*
    **  Process X11 events until we find something interesting.
    **  Interesting == a keystroke or a need to refresh the screen.
    **  If there's nothing pending, just return.
    */
    while (XEventsQueued(disp, QueuedAfterFlush))
        {
        XNextEvent(disp, &event);

        switch (event.type)
            {
        case MappingNotify:
            XRefreshKeyboardMapping ((XMappingEvent *)&event);
            break;

        case KeyPress:
            len = XLookupString ((XKeyEvent *)&event, text, 10, &key, 0);
            if (len == 1)
                {
                return text[0];
                }
            break;

        case Expose:
            return 0;
            }
        }
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Display current list.
**
**  Parameters:     None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSendString (int x, int y, int font,
                       bool bold, const char *str, bool blank)
    {
    char cb[80];
    
    FontInfo *currentFontInfo;
    
    if (font == FontSmall)
        {
        currentFontInfo = &smallOperFont;
        }
    else
        {
        currentFontInfo = &mediumOperFont;
        }
    if (bold)
        {
        XSetFont(disp, wgc, currentFontInfo->boldId);
        }
    else
        {
        XSetFont(disp, wgc, currentFontInfo->normalId);
        }

    if (blank)
        {
        memset (cb, ' ', 64);
        memcpy (cb, str, strlen (str));
        XDrawImageString(disp, window, wgc, XADJUST(x), YADJUST(y), cb, 64);
        }
    else
        {
        XDrawString(disp, window, wgc, XADJUST(x), YADJUST(y), 
                    str, strlen (str));
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
**  Purpose:        Get width of characters in font
**
**  Parameters:     Name        Description.
**                  f           pointer to FontInfo struct
**                  s           pointer to char to query
**
**  Returns:        Nothing; the char size fields in *f are updated.
**
**------------------------------------------------------------------------*/
static void getCharWidths (FontInfo *f, char *s)
    {
    int t;
    XCharStruct cs;

    XTextExtents(XQueryFont (disp, f->normalId), s, 1, &t, &t, &t, &cs);
    f->width = cs.width + f->pad;
    f->bwidth = cs.rbearing;
    f->height = cs.ascent;
    }

/*---------------------------  End Of File  ------------------------------*/
