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
#include "const.h"
#include "types.h"
#include "proto.h"
#include "dd60.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define LineBufSize     64
#define DefaultInterval 0.06
#define HersheyMaxSegments 45

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

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
static void INLINE windowShowLine (void);
static void windowStoreChar (char c, int x, int y, int dx);
static void getCharWidths (FontInfo *f, char *s);
static void windowTextPlot(int xPos, int yPos, char ch, u8 fontSize);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
extern Display *disp;
extern XrmDatabase XrmDb;
extern const char * const consoleHersheyGlyphs[64];
extern bool emulationActive;
extern bool hersheyMode;
extern int scaleX;
extern int scaleY;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static bool keyboardTrue;
static int currentFont = -1;
static FontInfo *currentFontInfo;
static int currentX = -1;
static int currentY = -1;
static int currentXOffset = 0;
static FontInfo smallFont;
static FontInfo mediumFont;
static FontInfo largeFont;
static Window window;
static Pixmap pixmap;
static GC wgc, pgc;
static unsigned long fg, bg, pfg, pbg;
static XKeyboardControl kbPrefs;

/*
**  These two together make up the line buffering machinery used
**  to detect bold (intensified) text.  The XLineBuf vector is
**  separate because it (as a vector) is an argument to an Xlib
**  function.
*/
static CharData lineBuf[LineBufSize];
static XTextItem XLineBuf[LineBufSize];
static int lineBufCnt, xpos, xstart, ypos;

/*
**  These variables are used to handle dot mode intensification.
*/
static int lastDotX, lastDotY = -1;
static int dotRepeats = 0;
static const i8 dotdx[] = { 0, 1, 0, 1, -1, -1,  0, -1,  1 };
static const i8 dotdy[] = { 0, 0, 1, 1, -1,  0, -1,  1, -1 };

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

void windowInit(void);
void windowClose(void);
void windowSetKeyboardTrue (bool flag);
int windowInput(void);
void windowShowDisplay (void);
void windowSetX (int x);
void windowSetY (int y);
void windowSetMode (int mode);
void windowProcessChar (int ch);

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

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
    **  Initialize the text display structure and timestamp
    */
    for (rc = 0; rc < LineBufSize; rc++)
        {
        XLineBuf[rc].chars = &lineBuf[rc].c;
        XLineBuf[rc].nchars = 1;
        XLineBuf[rc].font = None;
        }
    
    /*
    **  Do common X initialization
    */
    dtXinit ();

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
    XDEBUG(disp, "got background");
    fg = WhitePixel(disp, screen);
    XDEBUG(disp, "got foreground");

    window = XCreateSimpleWindow (disp, DefaultRootWindow(disp),
        10, 10, XSize, YSize, 5, fg, bg);
    XDEBUG(disp, "created window");

    /*
    **  Create a pixmap for background image generation.
    */
    pixmap = XCreatePixmap (disp, window, XSize, YSize, 1);
    XDEBUG(disp, "created pixmap");

    /*
    **  Set window and icon titles.
    */
    XSetStandardProperties (disp, window, DtCyberVersion, DtCyberVersion,
        None, NULL, 0, NULL);
    XDEBUG(disp, "set window properties");

    /*
    **  Create the graphics contexts for window and pixmap.
    */
    wgc = XCreateGC (disp, window, 0, 0);
    XDEBUG(disp, "created window GC");
    pgc = XCreateGC (disp, pixmap, 0, 0);
    XDEBUG(disp, "created pixmap GC");

    /*
    **  Initialize the pixmap.
    */
    XSetForeground (disp, pgc, bg);
    XDEBUG(disp, "set pixmap bg");
    XFillRectangle (disp, pixmap, pgc, 0, 0, XSize, YSize);
    XDEBUG(disp, "cleared pixmap");

    /*
    **  We don't want to get Expose events, otherwise every XCopyArea will generate one,
    **  and the event queue will fill up. This application will discard them anyway, but
    **  it is better not to generate them in the first place.
    */
    XSetGraphicsExposures(disp, wgc, FALSE);
    XDEBUG(disp, "set window exposure events off");

    /*
    **  Load three Cyber fonts, normal and bold flavors
    */
    smallFont.normalId = XLoadFont(disp, "-*-seymour-medium-*-*-*-8-*-*-*-*-*-*-*\0");
    XDEBUG(disp, "loaded small font");
    smallFont.boldId = XLoadFont(disp, "-*-seymour-bold-*-*-*-8-*-*-*-*-*-*-*\0");
    XDEBUG(disp, "loaded bold small font");
    mediumFont.normalId = XLoadFont(disp, "-*-seymour-medium-*-*-*-16-*-*-*-*-*-*-*\0");
    XDEBUG(disp, "loaded medium font");
    mediumFont.boldId = XLoadFont(disp, "-*-seymour-bold-*-*-*-16-*-*-*-*-*-*-*\0");
    XDEBUG(disp, "loaded bold medium font");
    largeFont.normalId = XLoadFont(disp, "-*-seymour-medium-*-*-*-32-*-*-*-*-*-*-*\0");
    XDEBUG(disp, "loaded large font");
    largeFont.boldId = XLoadFont(disp, "-*-seymour-bold-*-*-*-32-*-*-*-*-*-*-*\0");
    XDEBUG(disp, "loaded bold large font");
    getCharWidths (&smallFont, "\001");
    XDEBUG(disp, "queried small font");
    getCharWidths (&mediumFont, "\001");
    XDEBUG(disp, "queried medium font");
    getCharWidths (&largeFont, "\001");
    XDEBUG(disp, "queried large font");

    /*
    **  Setup fore- and back-ground colors.
    */
    XGetWindowAttributes(disp,window,&a);
    XDEBUG(disp, "get window attributes");
    XAllocNamedColor(disp, a.colormap, fgcolor, &b, &c);
    XDEBUG(disp, "allocate foreground color");
    fg=b.pixel;
    XAllocNamedColor(disp, a.colormap, bgcolor, &b, &c);
    XDEBUG(disp, "allocate background color");
    bg=b.pixel;
    XSetBackground(disp, wgc, bg);
    XDEBUG(disp, "set window background");
    XSetForeground(disp, wgc, fg);
    XDEBUG(disp, "set window foreground");

    /*
    **  Ditto for pixmap
    */
    pfg = WhitePixel(disp, screen);
    XDEBUG(disp, "get pixmap white");
    pbg = BlackPixel(disp, screen);
    XDEBUG(disp, "get pixmap black");
    XSetBackground(disp, pgc, pbg);
    XDEBUG(disp, "set pixmap background");
    XSetForeground(disp, pgc, pfg);
    XDEBUG(disp, "set pixmap foreground");

    /*
    **  Create mappings of some ALT-key combinations to strings.
    */
    modList[0] = XK_Meta_L;
#if 0
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
#endif
    XRebindKeysym(disp, 'q', modList, 1, "$q", 2);
    XRebindKeysym(disp, 'Q', modList, 1, "$q", 2);
    XRebindKeysym(disp, 's', modList, 1, "$s", 2);
    XRebindKeysym(disp, 'S', modList, 1, "$s", 2);
    XRebindKeysym(disp, 'z', modList, 1, "$z", 2);
    XRebindKeysym(disp, 'Z', modList, 1, "$z", 2);
    XDEBUG(disp, "rebound keysyms (a pile of them)");
    
    /*
    **  Initialise input.
    */
    wmhints.flags = InputHint;
    wmhints.input = True;
    XSetWMHints(disp, window, &wmhints);
    XDEBUG(disp, "set window manager hints");
    XSelectInput (disp, window, KeyPressMask | StructureNotifyMask);
    XDEBUG(disp, "select input");

    /*
    **  We like to be on top.
    */
    XMapRaised (disp, window);
    XDEBUG(disp, "raise window");
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
    if (keyboardTrue == flag)
        {
        return;         /* already set, just exit */
        }
    
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

/*--------------------------------------------------------------------------
**  Purpose:        Window input event processor.
**
**  Parameters:     Name        Description.
**
**  Returns:        Key received, or 0.
**
**------------------------------------------------------------------------*/
int windowInput(void)
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

        switch (event.type)
            {
        case MappingNotify:
            XRefreshKeyboardMapping ((XMappingEvent *)&event);
            break;

        case KeyRelease:
            /*
            **  Note that we only get key release events in "true" 
            **  keyboard mode.  The code is mostly common with 
            **  keypresses.
            */
        case KeyPress:
            len = XLookupString ((XKeyEvent *)&event, text, 10, &key, 0);
            c = text[0];
            if (len == 1)
                {
                if (c > 127)
                    {
                    break;
                    }
                c = asciiToConsole[c];
                if (event.type == KeyRelease)
                    {
                    c |= 0200;
                    }
                return c;
                }
            else if (event.type == KeyPress && len == 2 && text[0] == '$')
                {
                switch (text[1])
                    {
#if 0
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
                    debugDisplay |= (traceMask != 0);
                    traceStop ();
                    break;

                case 'c':
                    traceMask ^= TraceCpu0;
                    debugDisplay |= (traceMask != 0);
                    traceStop ();
                    break;

                case 'C':
                    traceMask ^= TraceCpu1;
                    debugDisplay |= (traceMask != 0);
                    traceStop ();
                    break;

                case 'e':
                    traceMask ^= TraceEcs;
                    debugDisplay |= (traceMask != 0);
                    traceStop ();
                    break;

                case 'j':
                    traceMask ^= TraceXj;
                    debugDisplay |= (traceMask != 0);
                    traceStop ();
                    break;

                case 'x':
                    if (traceMask == 0 && chTraceMask == 0)
                        {
                        traceMask = ~0;
                        debugDisplay = TRUE;
                        }
                    else
                        {
                        traceMask = 0;
                        chTraceMask = 0;
                        traceStop ();
                        }
                    break;
#endif
                case 'q':
                    return Dd60KeyXon;
                case 's':
                    return Dd60KeyXoff;
                case 'z':
                    emulationActive = FALSE;
                    return Dd60KeyXoff;
                    }
                }
            break;

            }
        }
    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Show generated display data on the window.
**
**  Parameters:     None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowShowDisplay (void)
    {
    /*
    **  Output any pending data from the line buffer
    */
    windowShowLine ();

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

/*--------------------------------------------------------------------------
**  Purpose:        Process a Set X command
**
**  Parameters:     Name        Description.
**                  x           New x position
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetX (int x)
    {
    currentX = x;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process a Set Y command
**
**  Parameters:     Name        Description.
**                  y           New y position
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetY (int y)
    {
    currentY = 0777 - y;
    if (currentFont == FontDot)
        {
        windowShowLine ();
        if (currentX == lastDotX && currentY == lastDotY &&
            dotRepeats < sizeof (dotdx) - 1)
            {
            dotRepeats++;
            }
        else
            {
            lastDotX = currentX;
            lastDotY = currentY;
            dotRepeats = 0;
            }
        XDrawPoint(disp, pixmap, pgc, 
                   XADJUST (currentX + dotdx[dotRepeats]),
                   YADJUST (currentY + dotdy[dotRepeats]));
        }
    
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process a Set X command
**
**  Parameters:     Name        Description.
**                  mode        New mode position
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetMode (int mode)
    {
    int newFont, newOffset;
    
    /*
    **  Handle left screen/right screen selection
    */
    if (mode & Dd60ScreenR)
        {
        newOffset = OffRightScreen;
        }
    else
        {
        newOffset = OffLeftScreen;
        }

    if (newOffset != currentXOffset)
        {
        /*
        **  Send out any buffered line of text.
        */
        windowShowLine ();
        currentXOffset = newOffset;
        }
        
    /*
    **  Setup new font if necessary.
    */
    switch (mode & 3)
        {
    case Dd60CharSmall:
        newFont = FontSmall;
        break;
    case Dd60CharMedium:
        newFont = FontMedium;
        break;
    case Dd60CharLarge:
        newFont = FontLarge;
        break;
    case Dd60Dot:
        newFont = FontDot;
        break;
        }

        if (currentFont != newFont)
        {
            windowShowLine ();
            currentFont = newFont;
            switch (newFont)
            {
            case FontSmall:
                currentFontInfo = &smallFont;
                XSetFont(disp, pgc, currentFontInfo->normalId);
                break;

            case FontMedium:
                currentFontInfo = &mediumFont;
                XSetFont(disp, pgc, currentFontInfo->normalId);
                break;
    
            case FontLarge:
                currentFontInfo = &largeFont;
                XSetFont(disp, pgc, currentFontInfo->normalId);
                break;
            }
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process a byte of character data
**
**  Parameters:     Name        Description.
**                  ch          character (display code)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowProcessChar (int ch)
    {
    /*
    **  If in dot mode, or no set mode received yet, just ignore the char
    */
    if (currentFont <= 0)
        {
        return;
        }
    if (hersheyMode)
        {
        windowTextPlot(XADJUST (currentX), YADJUST (currentY),
                       ch, currentFont);
        }
    else
        {
        windowStoreChar (ch, currentX, currentY,
                         currentFont);
        }
    currentX += currentFont;
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Flush pending characters in the line buffer to the bitmap.
**
**  Parameters:     None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void INLINE windowShowLine (void)
    {
    if (lineBufCnt != 0)
        {
        XDrawText(disp, pixmap, pgc, XADJUST (xstart),
                  YADJUST (ypos), XLineBuf, lineBufCnt);
        lineBufCnt = 0;
        ypos = -1;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Put a character to the display, buffering a line
**					at a time so we can do bold handling etc.
**
**  Parameters:     Name        Description.
**					c			character (display code)
**					x			x position
**					y			y position
**					dx			current font size
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowStoreChar (char c, int x, int y, int dx)
    {
    int dindx;
    int fontId;
    
    /*
    **  Center the character on the supplied x/y. 
    */
    x -= currentFontInfo->bwidth / 2;
    y += currentFontInfo->height / 2;
    
    /*
    **  Check for intensify
    */
    dindx = (x - xstart) / dx;
    if (y == ypos &&
        x < xpos && x >= xstart &&
            dindx * dx == x - xstart &&
            lineBuf[dindx].c == c)
        {
        if (++lineBuf[dindx].hits >= (dx / 2) - 1)
            {
            XLineBuf[dindx].font = currentFontInfo->boldId;
            if (XLineBuf[dindx + 1].font == None)
                {
                XLineBuf[dindx + 1].font = currentFontInfo->normalId;
                }
            }
        return;
        }
    if (lineBufCnt == LineBufSize ||
        y != ypos ||
        x < xpos || 
        dindx >= LineBufSize ||
        dindx * dx != x - xstart)
        {
        windowShowLine ();
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
        XLineBuf[lineBufCnt].delta = currentFontInfo->pad;
        XLineBuf[lineBufCnt].font = fontId;
        fontId = None;
        lineBuf[lineBufCnt].hits = 1;
        lineBuf[lineBufCnt++].c = 055;
        }
    
    XLineBuf[lineBufCnt].delta = currentFontInfo->pad;
    if (lineBufCnt == 0 ||
        XLineBuf[lineBufCnt - 1].font == currentFontInfo->boldId)
        {
        XLineBuf[lineBufCnt].font = currentFontInfo->normalId;
        }
    else
        {
        XLineBuf[lineBufCnt].font = None;
        }
    lineBuf[lineBufCnt].hits = 1;
    lineBuf[lineBufCnt++].c = c;
    xpos += dx;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Get font parameters
**
**  Parameters:     Name        Description.
**					f           FontInfo struct pointer
**					s           pointer to char to use to query for metrics
**
**  Returns:        Nothing.
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


/*--------------------------------------------------------------------------
**  Purpose:        Plot a character using the Hershey glyphs.
**
**  Parameters:     Name        Description.
**					xPos		x position
**					yPos		y position
**					ch			character (display code)
**					fontSize	current font size
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowTextPlot(int xPos, int yPos, char ch, u8 fontSize)
    {
    XPoint linesVec[HersheyMaxSegments];
    int segnum = 0;
    const unsigned char *glyph;
    int x, y;
    int charSize = fontSize / 8;
    
    xPos += 8;
    glyph = (const unsigned char *)(consoleHersheyGlyphs[ch]);

    if (*glyph != '\0') /* nonempty glyph */
        {
        glyph += 2;

        while (*glyph)
            {
            x = (int)glyph[0];

            if (x == (int)' ')
                {
                if (segnum != 0)
                    {
                    XDrawLines (disp, pixmap, pgc, linesVec,
                                segnum, CoordModeOrigin);
                    segnum = 0;
                    }
                }
            else
                {
                x = charSize * (x - (int)'R') + xPos;
                y = charSize * ((int)glyph[1] - (int)'R') + yPos;
                linesVec[segnum].x = x;
                linesVec[segnum].y = y;
                segnum++;
                }

            glyph += 2; /* on to next pair */
            }
        if (segnum != 0)
            {
            XDrawLines (disp, pixmap, pgc, linesVec,
                        segnum, CoordModeOrigin);
            }
        }
    }

/*---------------------------  End Of File  ------------------------------*/
