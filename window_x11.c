/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
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

// Size of the window and pixmap.
// This is: a screen high with marging top and botton, and two screens
// wide with margins top and bottom, and 20b space in between.
#define XSize           (02020 + 2 * DisplayMargin)
#define YSize           (01000 + 2 * DisplayMargin)

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define XADJUST(x) ((x) + DisplayMargin)
#define YADJUST(y) ((y) + DisplayMargin)

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct dispList
    {
    u16             xPos;           /* horizontal position */
    u16             yPos;           /* vertical position */
    u8              fontSize;       /* size of font */
    char            ch;             /* character to be displayed */
    } DispList;

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
static void INLINE dflush (void);
static void dput (char c, int x, int y, int dx);
static void getCharWidths (FontInfo *f, char *s);
static void windowInput(void);
static void showDisplay (void);
static void sum (u16 x);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
extern Display *disp;
extern XrmDatabase XrmDb;
extern Window ptermWindow;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static i8 currentFont = -1;
static FontInfo *currentFontInfo;
static i16 currentX = -1;
static i16 currentY = -1;
static DispList display[ListSize];
static int listGet, listPut, prevPut, listPutAtGetChar;
static u32 s1,s2;
static u32 s1list[MaxPolls], s2list[MaxPolls];
static int listPutsAtGetChar[MaxPolls];
static int sumListGet, sumListPut;
static char keybuf[KeyBufSize];
static u32 keyListPut, keyListGet;
static FontInfo smallFont;
static FontInfo mediumFont;
static FontInfo largeFont;
static FontInfo smallOperFont;
static FontInfo mediumOperFont;
static Window window;
static XTextItem dbuf[DisplayBufSize];
static char dchars[DisplayBufSize];
static u8 dhits[DisplayBufSize];
static int dcnt, xpos, xstart, ypos;
static Pixmap pixmap;
static GC wgc, pgc;
static u64 lastDisplayUs;
static unsigned long fg, bg, pfg, pbg;
static bool displayOff = FALSE;
static const i8 dotdx[] = { 0, 1, 0, 1, -1, -1,  0, -1,  1 };
static const i8 dotdy[] = { 0, 0, 1, 1, -1,  0, -1,  1, -1 };
static XKeyboardControl kbPrefs;
static bool keyboardTrue, keyboardSendUp;
static bool platoActive;

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
extern bool platoKeypress (XKeyEvent *kp, int stat);
extern void ptermInput(XEvent *event);
extern void ptermXinit(void);

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
    **  Create display list pool.
    */
    listGet = listPut = 0;
    sumListGet = sumListPut = 0;
    s1 = s2 = 0;
    
    /*
    **  Initialize the text display structure and timestamp
    */
    
    for (rc = 0; rc < DisplayBufSize; rc++)
        {
        dbuf[rc].chars = dchars + rc;
        dbuf[rc].nchars = 1;
        dbuf[rc].font = None;
        }
    
    /*
    **  Initialize the input list
    */
    keyListGet = keyListPut = 0;
    
    /*
    **  Do common X initialization
    */
    ptermXinit ();

    screen = DefaultScreen(disp);

    /*
    **  Look for resources
    */
    if (XrmDb == NULL)
    {
        XrmInitialize ();
        XrmDb = XrmGetFileDatabase ("/usr/lib/X11/app-defaults/Dtcyber");
        if (XrmDb == NULL)
        {
            fprintf (stderr, "no app default database for dtcyber\n");
        }
    }
    
    if (XrmGetResource (XrmDb, "dtcyber.foreground",
                        "Dtcyber.Foreground", type, &value))
    {
        strncpy (fgcolor, value.addr, (int) value.size);
    }
    else
    {
        strcpy (fgcolor, "green");
    }
    if (XrmGetResource (XrmDb, "dtcyber.background",
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
    bg = BlackPixel(disp, screen);
    fg = WhitePixel(disp, screen);

    window = XCreateSimpleWindow (disp, DefaultRootWindow(disp),
        10, 10, XSize, YSize, 5, fg, bg);

    /*
    **  Create a pixmap for background image generation.
    */
    pixmap = XCreatePixmap (disp, window, XSize, YSize, 1);

    /*
    **  Set window and icon titles.
    */
    XSetStandardProperties (disp, window, DtCyberVersion, DtCyberVersion,
        None, NULL, 0, NULL);

    /*
    **  Create the graphics contexts for window and pixmap.
    */
    wgc = XCreateGC (disp, window, 0, 0);
    pgc = XCreateGC (disp, pixmap, 0, 0);

    /*
    **  Initialize the pixmap.
    */
    XSetForeground (disp, pgc, bg);
    XFillRectangle (disp, pixmap, pgc, 0, 0, XSize, YSize);

    /*
    **  We don't want to get Expose events, otherwise every XCopyArea will generate one,
    **  and the event queue will fill up. This application will discard them anyway, but
    **  it is better not to generate them in the first place.
    */
    XSetGraphicsExposures(disp, wgc, FALSE);

    /*
    **  Load three Cyber fonts, normal and bold flavors
    */
    smallFont.normalId = XLoadFont(disp, "-*-seymour-medium-*-*-*-8-*-*-*-*-*-*-*\0");
    smallFont.boldId = XLoadFont(disp, "-*-seymour-bold-*-*-*-8-*-*-*-*-*-*-*\0");
    mediumFont.normalId = XLoadFont(disp, "-*-seymour-medium-*-*-*-16-*-*-*-*-*-*-*\0");
    mediumFont.boldId = XLoadFont(disp, "-*-seymour-bold-*-*-*-16-*-*-*-*-*-*-*\0");
    largeFont.normalId = XLoadFont(disp, "-*-seymour-medium-*-*-*-32-*-*-*-*-*-*-*\0");
    largeFont.boldId = XLoadFont(disp, "-*-seymour-bold-*-*-*-32-*-*-*-*-*-*-*\0");
    smallOperFont.normalId = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-12-*-*-*-*-*-*-*\0");
    smallOperFont.boldId = XLoadFont(disp, "-*-lucidatypewriter-bold-*-*-*-12-*-*-*-*-*-*-*\0");
    mediumOperFont.normalId = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-17-*-*-*-*-*-*-*\0");
    mediumOperFont.boldId = XLoadFont(disp, "-*-lucidatypewriter-bold-*-*-*-17-*-*-*-*-*-*-*\0");
    getCharWidths (&smallFont, "\001");
    getCharWidths (&mediumFont, "\001");
    getCharWidths (&largeFont, "\001");
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
    **  Ditto for pixmap
    */
    pfg = WhitePixel(disp, screen);
    pbg = BlackPixel(disp, screen);
    XSetBackground(disp, pgc, pbg);
    XSetForeground(disp, pgc, pfg);

    /*
    **  Create mappings of some ALT-key combinations to strings.
    */
    modList[0] = XK_Meta_L;
    XRebindKeysym(disp, '0', modList, 1, "$0", 2);
    XRebindKeysym(disp, '1', modList, 1, "$1", 2);
    XRebindKeysym(disp, '2', modList, 1, "$2", 2);
    XRebindKeysym(disp, '3', modList, 1, "$3", 2);
    XRebindKeysym(disp, '4', modList, 1, "$4", 2);
    XRebindKeysym(disp, '5', modList, 1, "$5", 2);
    XRebindKeysym(disp, '6', modList, 1, "$6", 2);
    XRebindKeysym(disp, '7', modList, 1, "$7", 2);
    XRebindKeysym(disp, '8', modList, 1, "$8", 2);
    XRebindKeysym(disp, '9', modList, 1, "$9", 2);
    XRebindKeysym(disp, 'c', modList, 1, "$c", 2);
    XRebindKeysym(disp, 'C', modList, 1, "$C", 2);
    XRebindKeysym(disp, 'e', modList, 1, "$e", 2);
    XRebindKeysym(disp, 'E', modList, 1, "$e", 2);
    XRebindKeysym(disp, 'j', modList, 1, "$j", 2);
    XRebindKeysym(disp, 'J', modList, 1, "$j", 2);
    XRebindKeysym(disp, 'x', modList, 1, "$x", 2);
    XRebindKeysym(disp, 'X', modList, 1, "$x", 2);
    XRebindKeysym(disp, 'o', modList, 1, "$o", 2);
    XRebindKeysym(disp, 'O', modList, 1, "$o", 2);
    XRebindKeysym(disp, 'p', modList, 1, "$p", 2);
    XRebindKeysym(disp, 'P', modList, 1, "$p", 2);
    XRebindKeysym(disp, 'q', modList, 1, "$q", 2);
    XRebindKeysym(disp, 'Q', modList, 1, "$q", 2);
    XRebindKeysym(disp, 's', modList, 1, "$s", 2);
    XRebindKeysym(disp, 'S', modList, 1, "$s", 2);
    XRebindKeysym(disp, XK_Scroll_Lock, 0, 0, "$t", 2);
    
    /*
    **  Initialise input.
    */
    wmhints.flags = InputHint;
    wmhints.input = True;
    XSetWMHints(disp, window, &wmhints);
    XSelectInput (disp, window, KeyPressMask | StructureNotifyMask);

    /*
    **  We like to be on top.
    */
    XMapRaised (disp, window);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Get width of medium operator display font.
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
**  Purpose:        Set keyboard emulation to "true" or "easy".
**
**  Parameters:     TRUE for accurate, FALSE for easy.
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
void windowSetKeyboardTrue (bool flag)
    {
    if ((keyboardTrue = flag))
        {
        kbPrefs.auto_repeat_mode = AutoRepeatModeOff;
        XChangeKeyboardControl(disp, KBAutoRepeatMode, &kbPrefs);
        XSelectInput (disp, window, KeyPressMask | KeyReleaseMask | StructureNotifyMask);
        }
    else
        {
        kbPrefs.auto_repeat_mode = AutoRepeatModeDefault;
        XChangeKeyboardControl(disp, KBAutoRepeatMode, &kbPrefs);
        XSelectInput (disp, window, KeyPressMask | StructureNotifyMask);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set font size.
**                  functions.
**
**  Parameters:     Name        Description.
**                  size        font size in points.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetFont(u8 font)
    {
    currentFont = font;
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
void windowCheckOutput(void)
    {
    struct timeval tm;
    int i;
    u64 us;
    
    us = rtcMicroSec ();
    if (us == 0)
        {
        gettimeofday (&tm, NULL);
        us = tm.tv_sec * ULL(1000000) + tm.tv_usec;
        }
    
    // Check if it's time for another display update
    if (us - lastDisplayUs > RefreshInterval)
        {
        lastDisplayUs = us;
        
        // If a keyboard poll has been done since the last display update,
        // display up to that point (since it's a cycle point of the
        // display refresh).  Otherwise (strange code that doesn't do
        // any polling) display the whole current display list.
        if (listPutAtGetChar >= 0)
            {
            prevPut = listPutAtGetChar;
            }
        else
            {
            prevPut = listPut;
            }

        showDisplay ();
        windowInput();

        // This next line is necessary at least on NetBSD, because
        // that uses the pth version of pthreads, which is non-preemptive,
        // so other threads like the socket listener threads in mux6676
        // and niu don't get a chance to run; we're CPU bound here.
        //
        // Don't do it on Linux because Linux has kernel based threads,
#if !defined(__linux__)
        sched_yield();
#endif
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set X coordinate.
**
**  Parameters:     Name        Description.
**                  x           horizontal coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetX(u16 x)
    {
    currentX = x;
    sum (x);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set Y coordinate.
**
**  Parameters:     Name        Description.
**                  y           vertical coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetY(u16 y)
    {
    currentY = 0777 - y;
    sum (y);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Queue characters.
**
**  Parameters:     Name        Description.
**                  ch          character to be queued.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowQueue(char ch)
    {
    DispList *elem;
    u32 nextput;
    
    if (   currentX == -1
        || currentY == -1
        || currentFont == -1)
        {
        return;
        }


    if (ch != 0)
        {
        nextput = listPut + 1;
        if (nextput == ListSize)
            {
            nextput = 0;
            }
        if (nextput == listGet)
            {
            return;
            }
        elem = display + listPut;
        listPut = nextput;
        elem->ch = ch;
        elem->fontSize = currentFont;
        elem->xPos = currentX;
        elem->yPos = currentY;
        }
    currentX += currentFont;

    }

/*--------------------------------------------------------------------------
**  Purpose:        Indicate that operator mode is finished.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowOperEnd(void)
    {
    currentX = currentY = currentFont = listPutAtGetChar = -1;
    listGet = listPut;
    if (keyboardTrue)
        {
        kbPrefs.auto_repeat_mode = AutoRepeatModeOff;
        XChangeKeyboardControl(disp, KBAutoRepeatMode, &kbPrefs);
        XSync(disp, 0);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Poll the keyboard
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void windowGetChar(void)
    {
    int nextget;
    struct timeval tm;
    int us;
    int nextput, i, j, k;
    
    // Remember the x/y sum for this poll cycle
    nextput = sumListPut + 1;
    if (nextput == MaxPolls)
        {
        nextput = 0;
        }
    if (nextput != sumListGet)
        {
        s1list[sumListPut] = s1;
        s2list[sumListPut] = s2;
        listPutsAtGetChar[sumListPut] = listPutAtGetChar = listPut;
        i = sumListGet;
        j = -1;
        while (i != sumListPut)
            {
            if (s1 == s1list[i] && s2 == s2list[i])
                {
                j = listPutsAtGetChar[i];
                k = i;
                }
            i++;
            if (i == MaxPolls)
                {
                i = 0;
                }
            }
        s1 = s2 = 0;
        if (j != -1)
            {
            k++;
            if (k == MaxPolls)
                {
                k = 0;
                }
            listGet = j;
            sumListGet = k;
            }
        sumListPut = nextput;
        }
    
    windowCheckOutput();
    
    if (keyboardSendUp || keyListGet == keyListPut)
        {
        ppKeyIn = 0;
        keyboardSendUp = FALSE;
        return;
        }

    nextget = keyListGet + 1;
    if (nextget == KeyBufSize)
        {
        nextget = 0;
        }
    ppKeyIn = keybuf[keyListGet];
    keyListGet = nextget;
    if (!keyboardTrue)
        {
        // We're not doing the precise emulation, instead doing
        // regular key rollover.  So ignore key up events,
        // and send a zero code (all up) in between each key code.
        if (ppKeyIn & 0200)
            {
            ppKeyIn = 0;
            }
        else
            {
            keyboardSendUp = TRUE;
            }
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
    XFreeGC (disp, pgc);
    XFreePixmap (disp, pixmap);
    XDestroyWindow (disp, window);
    XCloseDisplay (disp);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Flush pending characters to the bitmap.
**
**  Parameters:     None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void INLINE dflush (void)
    {
    if (dcnt != 0)
        {
        XDrawText(disp, pixmap, pgc, XADJUST (xstart),
                  YADJUST (ypos), dbuf, dcnt);
        dcnt = 0;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Sum coordinates
**
**  Parameters:     x or y
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void sum (u16 x)
    {
    // This is a Fletcher checsum of the 16 bit values passed in
    s1 += x;
    s2 += s1;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Put a character to the display, buffering a line
**					at a time so we can do bold handling etc.
**
**  Parameters:     Name        Description.
**					c			character
**					x			x position
**					y			y position
**					dx			current font size
**
**  Returns:        Nothing.
**
**  Note that the character codes are ASCII when in the operator
**  window, and display code when doing normal console display.
**
**------------------------------------------------------------------------*/
static void dput (char c, int x, int y, int dx)
    {
    int dindx;
    int fontId;
    
    // Center the character on the supplied x/y.
    x -= currentFontInfo->bwidth / 2;
    y += currentFontInfo->height / 2;
    
    // Check for intensify
    dindx = (x - xstart) / dx;
    if (y == ypos &&
        x < xpos && x >= xstart &&
            dindx * dx == x - xstart &&
            dchars[dindx] == c)
        {
        if (++dhits[dindx] >= dx / 2)
            {
            dbuf[dindx].font = currentFontInfo->boldId;
            if (dbuf[dindx + 1].font == None)
                {
                dbuf[dindx + 1].font = currentFontInfo->normalId;
                }
            }
        return;
        }
    if (dcnt == DisplayBufSize ||
        y != ypos ||
        x < xpos || 
        dindx >= DisplayBufSize ||
        dindx * dx != x - xstart)
        {
        dflush ();
        xpos = xstart = x;
        ypos = y;
        }
    /*
    ** If we're skipping to a spot further down this line,
    ** space fill the range in between.
    */
    fontId = currentFontInfo->normalId;
    for ( ; xpos < x; xpos += dx)
        {
        dbuf[dcnt].delta = currentFontInfo->pad;
        dbuf[dcnt].font = fontId;
        fontId = None;
        dhits[dcnt] = 1;
        if (opActive)
            {
            dchars[dcnt++] = ' ';
            }
        else
            {
            dchars[dcnt++] = 055;
            }
        }
    
    dbuf[dcnt].delta = currentFontInfo->pad;
    if (dcnt == 0 ||
        dbuf[dcnt - 1].font == currentFontInfo->boldId)
        {
        dbuf[dcnt].font = currentFontInfo->normalId;
        }
    else
        {
        dbuf[dcnt].font = None;
        }
    dhits[dcnt] = 1;
    dchars[dcnt++] = c;
    xpos += dx;
    }

static void getCharWidths (FontInfo *f, char *s)
    {
    int t;
    XCharStruct cs;

    XTextExtents(XQueryFont (disp, f->normalId), s, 1, &t, &t, &t, &cs);
    f->width = cs.width + f->pad;
    f->bwidth = cs.rbearing;
    f->height = cs.ascent;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Window input event processor.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowInput(void)
    {
    KeySym key;
    XEvent event;
    char text[30];
    u8 c;
    int len;
    u32 nextput;


    /*
    **  Process any X11 events.
    */
    while (XEventsQueued(disp, QueuedAfterFlush))
        {
        XNextEvent(disp, &event);

        if (((XAnyEvent *) &event)->window == ptermWindow)
            {
            ptermInput (&event);
            continue;
            }
        switch (event.type)
            {
        case MappingNotify:
            XRefreshKeyboardMapping ((XMappingEvent *)&event);
            break;

        case KeyRelease:
            // Note that we only get key release events in "true" 
            // keyboard mode.  The code is mostly common with 
            // keypresses.
        case KeyPress:
            if (platoActive && !opActive &&
                event.type == KeyPress &&
                platoKeypress ((XKeyEvent *)&event, 0))
                {
                // If it's a valid Plato keypress, we're done now.
                return;
                }
            else
                {
                len = XLookupString ((XKeyEvent *)&event, text, 10, &key, 0);
                if (platoActive && !opActive && len == 1)
                    {
                    // Unrecognized non-Alt keypresses in Plato mode are ignored.
                    break;
                    }
                c = text[0];
                }
            if (len == 1)
                {
                if (c > 127)
                    {
                    break;
                    }
                nextput = keyListPut + 1;
                if (event.type == KeyRelease)
                    {
                    c |= 0200;
                    }
                if (nextput == KeyBufSize)
                    nextput = 0;
                if (nextput != keyListGet)
                    {
                    keybuf[keyListPut] = c;
                    keyListPut = nextput;
                    }
                }
            else if (event.type == KeyPress && len == 2 && text[0] == '$')
                {
                switch (text[1])
                    {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    traceMask ^= (1 << (text[1] - '0'));
                    traceStop ();
                    break;

                case 'c':
                    traceMask ^= TraceCpu0;
                    traceStop ();
                    break;

                case 'C':
                    traceMask ^= TraceCpu1;
                    traceStop ();
                    break;

                case 'e':
                    traceMask ^= TraceEcs;
                    traceStop ();
                    break;

                case 'j':
                    traceMask ^= TraceXj;
                    traceStop ();
                    break;

                case 'x':
                    if (traceMask == 0)
                        {
                        traceMask = ~0;
                        }
                    else
                        {
                        traceMask = 0;
                        traceStop ();
                        }
                    break;

                case 'o':
                    opActive = TRUE;
                    currentX = currentY = currentFont = listPutAtGetChar = -1;
                    listGet = listPut;
                    kbPrefs.auto_repeat_mode = AutoRepeatModeDefault;
                    XChangeKeyboardControl(disp, KBAutoRepeatMode, &kbPrefs);
                    break;
                case 'p':
                    if (niuPresent ())
                    {
                        platoActive = !platoActive;
                    }
                    break;
                case 'q':
                    displayOff = FALSE;
                    break;
                case 's':
                    displayOff = TRUE;
                    break;
                case 't':
                    // this is useful in Plato mode when Alt-S and Alt-Q
                    // have other meanings.
                    displayOff = !displayOff;
                    break;
                    }
                }
            break;

            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Display current list.
**
**  Parameters:     None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void showDisplay (void)
    {
    static int refreshCount = 0;
    char str[2] = " ";
    DispList *curr;
    DispList *end;
    u8 oldFont = 0;
    int dotx, doty, doti;
    
    currentFontInfo = &smallFont;
    oldFont = FontSmall;

    listPutAtGetChar = -1;
    sumListGet = sumListPut;
    
    if (displayOff)
        {
        listGet = prevPut;
        return;
        }

#if CcDebug == 1
    {
        char buf[160];

        /*
        **  Display P registers of PPUs and CPU and current trace mask.
        */
        XSetFont(disp, pgc, smallOperFont.normalId);
        sprintf(buf, "Refresh: %-10d  PP P-reg: %04o %04o %04o %04o %04o %04o %04o %04o %04o %04o   CPU P-reg: %06o",
                refreshCount++,
                ppu[0].regP, ppu[1].regP, ppu[2].regP, ppu[3].regP, ppu[4].regP,
                ppu[5].regP, ppu[6].regP, ppu[7].regP, ppu[8].regP, ppu[9].regP,
                cpu[0].regP); 

        if (cpuCount > 1)
            {
            sprintf(buf + strlen(buf), " %06o", cpu[1].regP);
            }
            
        sprintf(buf + strlen(buf),
                "   Trace: %c%c%c%c%c%c%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c%c%c%c%c  %c",
                (traceMask >> 0) & 1 ? '0' : '_',
                (traceMask >> 1) & 1 ? '1' : '_',
                (traceMask >> 2) & 1 ? '2' : '_',
                (traceMask >> 3) & 1 ? '3' : '_',
                (traceMask >> 4) & 1 ? '4' : '_',
                (traceMask >> 5) & 1 ? '5' : '_',
                (traceMask >> 6) & 1 ? '6' : '_',
                (traceMask >> 7) & 1 ? '7' : '_',
                (traceMask >> 8) & 1 ? '8' : '_',
                (traceMask >> 9) & 1 ? '9' : '_',
                (traceMask & TraceCpu0) ? 'c' : '_',
                (traceMask & TraceCpu1) ? 'C' : '_',
                (traceMask & TraceEcs) ? 'E' : '_',
                (traceMask & TraceXj) ? 'J' : '_',
                (chTraceMask >> 0) & 1 ? '0' : '_',
                (chTraceMask >> 1) & 1 ? '1' : '_',
                (chTraceMask >> 2) & 1 ? '2' : '_',
                (chTraceMask >> 3) & 1 ? '3' : '_',
                (chTraceMask >> 4) & 1 ? '4' : '_',
                (chTraceMask >> 5) & 1 ? '5' : '_',
                (chTraceMask >> 6) & 1 ? '6' : '_',
                (chTraceMask >> 7) & 1 ? '7' : '_',
                (chTraceMask >> 8) & 1 ? '8' : '_',
                (chTraceMask >> 9) & 1 ? '9' : '_',
                (chTraceMask >> 10) & 1 ? 'A' : '_',
                (chTraceMask >> 11) & 1 ? 'B' : '_',
                (platoActive) ? 'P' : ' ');

        XDrawString(disp, pixmap, pgc, 10, 10, buf, strlen(buf));
    }
#endif

    XSetFont(disp, pgc, currentFontInfo->normalId);

    /*
    **  Draw display list in pixmap.
    */
    end = display + prevPut;
    curr = display + listGet;
        
    ypos = ~curr->yPos;     // Make sure this looks like a fresh line
    doty = 07777;
    
    for (;;)
    {
        /*
        **  Check for wrap and done
        */
        if (curr == display + ListSize)
            curr = display;
        if (curr == end)
            break;

        /*
        **  Setup new font if necessary.
        */
        if (oldFont != curr->fontSize)
        {
            dflush ();
            oldFont = curr->fontSize;

            switch (oldFont)
            {
            case FontSmall:
                if (opActive)
                    {
                    currentFontInfo = &smallOperFont;
                    }
                else
                    {
                    currentFontInfo = &smallFont;
                    }
                XSetFont(disp, pgc, currentFontInfo->normalId);
                break;

            case FontMedium:
                if (opActive)
                    {
                    currentFontInfo = &mediumOperFont;
                    }
                else
                    {
                    currentFontInfo = &mediumFont;
                    }
                XSetFont(disp, pgc, currentFontInfo->normalId);
                break;
    
            case FontLarge:
                currentFontInfo = &largeFont;
                XSetFont(disp, pgc, currentFontInfo->normalId);
                break;
            }
        }

        /*
        **  Draw dot or character.
        */
        if (curr->fontSize == FontDot)
        {
            dflush ();
            if (curr->xPos == dotx && curr->yPos == doty &&
                doti < sizeof (dotdx) - 1)
                {
                doti++;
                }
            else
                {
                dotx = curr->xPos;
                doty = curr->yPos;
                doti = 0;
                }
            XDrawPoint(disp, pixmap, pgc, 
                       XADJUST (curr->xPos + dotdx[doti]),
                       YADJUST (curr->yPos + dotdy[doti]));
        }
        else
        {
            str[0] = curr->ch;
            dput (curr->ch, curr->xPos, curr->yPos, currentFontInfo->width);
        }
        curr++;
    }

    dflush ();
    listGet = end - display;

    /*
    **  Update display from pixmap.
    */
    XCopyPlane(disp, pixmap, window, wgc, 0, 0, XSize, YSize, 0, 0, 1);

    /*
    **  Erase pixmap for next round.
    */
    XSetForeground (disp, pgc, pbg);
    XFillRectangle (disp, pixmap, pgc, 0, 0, XSize, YSize);
    XSetForeground (disp, pgc, pfg);
    }

/*---------------------------  End Of File  ------------------------------*/
