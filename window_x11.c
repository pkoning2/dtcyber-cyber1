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
#include <sys/time.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define ListSize        5000
#define KeyBufSize	50	// MUST be even
#define DisplayBufSize	64
#define DisplayMargin	20
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
    int             width;          /* character width in pixels */
    } FontInfo;
/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void dflush (void);
static void dput (char c, int x, int y, int dx);
static void getCharWidths (FontInfo *f);
void windowInput(void);
static void showDisplay (void);
/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 currentFont;
static FontInfo *currentFontInfo;
static i16 currentX;
static i16 currentY;
static u16 oldCurrentY;
static DispList display[ListSize];
static u32 listGet, listPut, prevPut;
static char keybuf[KeyBufSize];
static u32 keyListPut, keyListGet;
static FontInfo smallFont;
static FontInfo mediumFont;
static FontInfo largeFont;
static int width;
static int height;
static int depth;
static bool refresh = FALSE;
static Display *disp;
static Window window;
static XTextItem dbuf[DisplayBufSize];
static char dchars[DisplayBufSize];
static u8 dhits[DisplayBufSize];
static int dcnt, xpos, xstart, ypos;
static Pixmap pixmap;
static GC gc;
static struct timeval lastDisplay;
static unsigned long fg, bg;

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

    /*
    **  Create display list pool.
    */
    listGet = listPut = 0;

    /*
    **  Initialize the text display structure and timestamp
    */
    
    for (rc = 0; rc < DisplayBufSize; rc++)
        {
	dbuf[rc].chars = dchars + rc;
	dbuf[rc].nchars = 1;
	dbuf[rc].font = None;
	}
    gettimeofday (&lastDisplay, NULL);

    /*
    **  Initialize the input list
    */
    keyListGet = keyListPut = 0;
    
    /*
    **  Open the X11 display.
    */
    disp = XOpenDisplay(0);
    if (disp == (Display *) NULL)
        {
        fprintf(stderr, "Could not open display\n");
        exit(1);
        }

    screen = DefaultScreen(disp);

    /*
    **  Create a window using the following hints.
    */
    width = XADJUST (02020) + DisplayMargin;
    height = YADJUST (512) + DisplayMargin;

    bg = BlackPixel(disp, screen);
    fg = WhitePixel(disp, screen);

    window = XCreateSimpleWindow (disp, DefaultRootWindow(disp),
        10, 10, width, height, 5, fg, bg);

    /*
    **  Create a pixmap for background image generation.
    */
    depth = DefaultDepth (disp, screen);
    pixmap = XCreatePixmap (disp, window, width, height, depth);

    /*
    **  Set window and icon titles.
    */
    XSetStandardProperties (disp, window, DtCyberVersion, DtCyberVersion,
        None, NULL, 0, NULL);

    /*
    **  Create the graphics contexts for window and pixmap.
    */
    gc = XCreateGC (disp, window, 0, 0);

    /*
    **  We don't want to get Expose events, otherwise every XCopyArea will generate one,
    **  and the event queue will fill up. This application will discard them anyway, but
    **  it is better not to generate them in the first place.
    */
    XSetGraphicsExposures(disp, gc, FALSE);

    /*
    **  Load three Cyber fonts, normal and bold flavors
    */
    smallFont.normalId = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-10-*-*-*-*-*-*-*\0");
    smallFont.boldId = XLoadFont(disp, "-*-lucidatypewriter-bold-*-*-*-10-*-*-*-*-*-*-*\0");
    mediumFont.normalId = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-14-*-*-*-*-*-*-*\0");
    mediumFont.boldId = XLoadFont(disp, "-*-lucidatypewriter-bold-*-*-*-14-*-*-*-*-*-*-*\0");
    largeFont.normalId = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-24-*-*-*-*-*-*-*\0");
    largeFont.boldId = XLoadFont(disp, "-*-lucidatypewriter-bold-*-*-*-24-*-*-*-*-*-*-*\0");
    getCharWidths (&smallFont);
    getCharWidths (&mediumFont);
    getCharWidths (&largeFont);

    /*
    **  Setup fore- and back-ground colors.
    */
    XGetWindowAttributes(disp,window,&a);
    XAllocNamedColor(disp, a.colormap,"green",&b,&c);
    fg=b.pixel;
    bg = BlackPixel(disp, screen);
    XSetBackground(disp, gc, bg);
    XSetForeground(disp, gc, fg);

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
    XRebindKeysym(disp, 'C', modList, 1, "$c", 2);
    XRebindKeysym(disp, 'e', modList, 1, "$e", 2);
    XRebindKeysym(disp, 'E', modList, 1, "$e", 2);
    XRebindKeysym(disp, 'x', modList, 1, "$x", 2);
    XRebindKeysym(disp, 'X', modList, 1, "$x", 2);
    XRebindKeysym(disp, 'o', modList, 1, "$o", 2);
    XRebindKeysym(disp, 'O', modList, 1, "$o", 2);

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
**  Purpose:        Set X coordinate.
**
**  Parameters:     Name        Description.
**                  x           horinzontal coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetX(u16 x)
    {
    struct timeval tm;
    int us;

    currentX = x;
    gettimeofday (&tm, NULL);
    us = (tm.tv_sec - lastDisplay.tv_sec) * 1000000 +
         (tm.tv_usec - lastDisplay.tv_usec);
    // If we're overdue for a screen display, do it now.
    // This can happen if the PPU goes compute bound and
    // doesn't watch for input, but still does output,
    // for example in the NOS startup memory test.
    if (us > RefreshInterval * 15 / 10)
        {
        prevPut = listPut;
        showDisplay ();
        windowInput();
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set Y coordinate.
**
**  Parameters:     Name        Description.
**                  y           horinzontal coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetY(u16 y)
    {
    currentY = 0777 - y;
    if (oldCurrentY > currentY)
        {
        refresh = TRUE;
        }

    oldCurrentY = currentY;
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
        || currentY == -1)
        {
        return;
        }


    if (ch != 0)
        {
        nextput = listPut + 1;
        if (nextput == ListSize)
            nextput = 0;
        if (nextput == listGet)
            return;
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
**  Purpose:        Update window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowUpdate(void)
    {
    refresh = TRUE;
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
    
    if (traceMask & 2)
        fprintf (ppuTF[1], "key poll %02o\n", ppKeyIn);

    // We treat a keyboard poll as the end of a display refresh cycle.
    // If it's been long enough since the last one, call the displayer.
    // If not, advance the "get" pointer to where the "put" pointer was
    // at the start of the preceding cycle.
    gettimeofday (&tm, NULL);
    us = (tm.tv_sec - lastDisplay.tv_sec) * 1000000 +
         (tm.tv_usec - lastDisplay.tv_usec);
    if (us > RefreshInterval)
    {
        prevPut = listPut;
        showDisplay ();
        windowInput();
        
    }
    else
    {
        listGet = prevPut;
        prevPut = listPut;
    }
    
    if (keyListGet == keyListPut)
	return;

    nextget = keyListGet + 1;
    if (nextget == KeyBufSize)
	nextget = 0;
    ppKeyIn = keybuf[keyListGet];
    keyListGet = nextget;
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
    XFreeGC (disp, gc);
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

static void dflush (void)
    {
    if (dcnt != 0)
        {
        XDrawText(disp, pixmap, gc, XADJUST (xstart),
                  YADJUST (ypos), dbuf, dcnt);
        if (traceMask & 2)
            {
            int i;
                    
            dchars[dcnt] = '\0';
            fprintf (ppuTF[1], "text %03o %03o %s\n", xstart, ypos, dchars);
            for (i = 0; i < dcnt; i++)
                {
                fprintf (ppuTF[1], "%d ", dhits[i]);
                }
            fprintf (ppuTF[1], "\n");
            }
        dcnt = 0;
        }
    }

static void dput (char c, int x, int y, int dx)
    {
    int dindx = (x - xstart) / dx;
    int fontId;
    
    // Check for intensify
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
        dbuf[dcnt].delta = dx - currentFontInfo->width;
        dbuf[dcnt].font = fontId;
        fontId = None;
        dhits[dcnt] = 1;
        dchars[dcnt++] = ' ';
        }
    
    dbuf[dcnt].delta = dx - currentFontInfo->width;
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

static void getCharWidths (FontInfo *f)
    {
    int t;
    XCharStruct cs;

    XTextExtents(XQueryFont (disp, f->normalId), "A", 1, &t, &t, &t, &cs);
    f->width = cs.width;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Window input event processor.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowInput(void)
    {
    KeySym key;
    XEvent event;
    char text[30];
    int len;
    u32 nextput;


    /*
    **  Process any X11 events.
    */
    while (XEventsQueued(disp, QueuedAfterReading))
        {
        XNextEvent(disp, &event);

        switch (event.type)
            {
        case MappingNotify:
            XRefreshKeyboardMapping ((XMappingEvent *)&event);
            refresh = TRUE;
            break;

        case ConfigureNotify:
            if (event.xconfigure.width > width || event.xconfigure.height > height)
                {
                /*
                **  Reallocate pixmap only if it has grown.
                */
                width = event.xconfigure.width;
                height = event.xconfigure.height;
                XFreePixmap (disp, pixmap);
                pixmap = XCreatePixmap (disp, window, width, height, depth);
                }

            XFillRectangle (disp, pixmap, gc, 0, 0, width, height);
            refresh = TRUE;
            break;

        case KeyPress:
            len = XLookupString ((XKeyEvent *)&event, text, 10, &key, 0);
            if (len == 1)
                {
                nextput = keyListPut + 2;
                if (nextput == KeyBufSize)
                    nextput = 0;
                if (nextput != keyListGet)
                    {
                    keybuf[keyListPut] = text[0];
                    /*
                    ** Stick a null after the real character 
                    ** to represent "key up" after the key down.
                    ** Without this, NOS DSD loses many keystrokes.
                    */
                    keybuf[keyListPut + 1] = 0;
                    keyListPut = nextput;
                    }
                }
            else if (len == 2 && text[0] == '$')
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
                    break;

                case 'c':
                    traceMask ^= (1 << 14);
                    break;

                case 'e':
                    traceMask ^= (1 << 15);
                    break;

                case 'x':
                    if (traceMask == 0)
                        {
                        traceMask = ~0;
                        }
                    else
                        {
                        traceMask = 0;
                        }
                    break;

                case 'o':
                    opActive = TRUE;
                    break;
                    }
                }
            break;

            }
        }
    }

void showDisplay (void)
    {
    static int refreshCount = 0;
    char str[2] = " ";
    DispList *curr;
    DispList *end;
    u8 oldFont = 0;

    XSetForeground (disp, gc, fg);

    currentFontInfo = &smallFont;
    XSetFont(disp, gc, currentFontInfo->normalId);
    oldFont = FontSmall;

#if CcDebug == 1
    {
        char buf[160];

        /*
        **  Display P registers of PPUs and CPU and current trace mask.
        */
        sprintf(buf, "Refresh: %-10d  PP P-reg: %04o %04o %04o %04o %04o %04o %04o %04o %04o %04o   CPU P-reg: %06o",
                refreshCount++,
                ppu[0].regP, ppu[1].regP, ppu[2].regP, ppu[3].regP, ppu[4].regP,
                ppu[5].regP, ppu[6].regP, ppu[7].regP, ppu[8].regP, ppu[9].regP,
                cpu.regP); 

        sprintf(buf + strlen(buf), "   Trace: %c%c%c%c%c%c%c%c%c%c%c%c",
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
                (traceMask >> 14) & 1 ? 'C' : '_',
                (traceMask >> 15) & 1 ? 'E' : '_');

        XDrawString(disp, pixmap, gc, 0, 10, buf, strlen(buf));
    }
#endif

    /*
    **  Draw display list in pixmap.
    */
    end = display + prevPut;
    curr = display + listGet;
            
    ypos = ~curr->yPos;     // Make sure this looks like a fresh line
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
                currentFontInfo = &smallFont;
                XSetFont(disp, gc, currentFontInfo->normalId);
                break;

            case FontMedium:
                currentFontInfo = &mediumFont;
                XSetFont(disp, gc, currentFontInfo->normalId);
                break;
    
            case FontLarge:
                currentFontInfo = &largeFont;
                XSetFont(disp, gc, currentFontInfo->normalId);
                break;
            }
        }

        /*
        **  Draw dot or character.
        */
        if (curr->fontSize == FontDot)
        {
            dflush ();
            XDrawPoint(disp, pixmap, gc, curr->xPos,
                       YADJUST (curr->yPos));
        }
        else
        {
            str[0] = curr->ch;
            dput (curr->ch, curr->xPos, curr->yPos, curr->fontSize);
        }
        curr++;
    }

    dflush ();
    listGet = end - display;
    currentX = -1;
    currentY = -1;
    refresh = FALSE;

    /*
    **  Update display from pixmap.
    */
    XCopyArea(disp, pixmap, window, gc, 0, 0, width, height, 0, 0);

    /*
    **  Erase pixmap for next round.
    */
    XSetForeground (disp, gc, bg);
    XFillRectangle (disp, pixmap, gc, 0, 0, width, height);

    /*
    **  Make sure the updates make it to the X11 server.
    */
    XSync(disp, 0);
    gettimeofday (&lastDisplay, NULL);
    }

/*---------------------------  End Of File  ------------------------------*/
