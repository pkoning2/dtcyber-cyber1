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
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define ListSize        5000

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
typedef struct dispList
    {
    u16             xPos;           /* horizontal position */
    u16             yPos;           /* vertical position */
    u8              fontSize;       /* size of font */
    char            ch;             /* character to be displayed */
    } DispList;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
void *windowThread(void *param);

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
static i16 currentX;
static i16 currentY;
static u16 oldCurrentY;
static DispList display[ListSize];
static u32 listEnd;
static Font hSmallFont;
static Font hMediumFont;
static Font hLargeFont;
static int width;
static int height;
static bool refresh = FALSE;
static pthread_mutex_t mutexDisplay;
static Display *disp;
static Window window;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Create POSIX thread which will deal with all X11
**                  functions.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowInit(void)
    {
    int rc;
    pthread_t thread;
    pthread_attr_t attr;

    /*
    **  Create display list pool.
    */
    listEnd = 0;

    /*
    **  Create a mutex to synchronise access to display list.
    */
    pthread_mutex_init(&mutexDisplay, NULL);

    /*
    **  Create POSIX thread with default attributes.
    */
    pthread_attr_init(&attr);
    rc = pthread_create(&thread, &attr, windowThread, NULL);
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
    currentX = x;
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

    if (   listEnd  >= ListSize
        || currentX == -1
        || currentY == -1)
        {
        return;
        }

    /*
    **  Protect display list.
    */
    pthread_mutex_lock (&mutexDisplay);

    if (ch != 0)
        {
        elem = display + listEnd++;
        elem->ch = ch;
        elem->fontSize = currentFont;
        elem->xPos = currentX;
        elem->yPos = currentY;
        }

    currentX += currentFont;

    /*
    **  Release display list.
    */
    pthread_mutex_unlock (&mutexDisplay);
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
**  Purpose:        Poll the keyboard (dummy for X11)
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void windowGetChar(void)
    {
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
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = window;
    ev.xclient.message_type = XA_INTEGER;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = 6600;
    XSendEvent(disp, window, False, NoEventMask, &ev);
    XSync(disp, 0);
    sleep(1);
    printf("Shutting down main thread\n");
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Windows thread.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void *windowThread(void *param)
    {
    GC gc;
    KeySym key;
    KeySym modList[2];
    Pixmap pixmap;
    XEvent event;
    XWMHints wmhints;
    int screen, depth;
    char text[30];
    unsigned long fg, bg;
    int len;
    XWindowAttributes a;
    XColor b,c;
    static int refreshCount = 0;
    char str[2] = " ";
    DispList *curr;
    DispList *end;
    u8 oldFont = 0;

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
    width = 1034;
    height = 750;

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
    **  Load three Cyber fonts.
    */
    hSmallFont = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-10-*-*-*-*-*-*-*\0");
    hMediumFont = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-14-*-*-*-*-*-*-*\0");
    hLargeFont = XLoadFont(disp, "-*-lucidatypewriter-medium-*-*-*-24-*-*-*-*-*-*-*\0");

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
    XSelectInput (disp, window, KeyPressMask | KeyReleaseMask | StructureNotifyMask);

    /*
    **  We like to be on top.
    */
    XMapRaised (disp, window);

    for (;;)
        {
        /*
        **  Process any X11 events.
        */
        while (XEventsQueued(disp, QueuedAfterReading))
            {
            XNextEvent(disp, &event);

            switch (event.type)
                {
            case ClientMessage:
                /*
                **  Shutdown requested.
                */
                if (   event.xclient.format    == 32
                    && event.xclient.data.l[0] == 6600)
                    {
                    printf("Shutting down window thread\n");
                    XFreeGC (disp, gc);
                    XFreePixmap (disp, pixmap);
                    XDestroyWindow (disp, window);
                    XCloseDisplay (disp);
                    pthread_exit(NULL);
                    return(NULL);
                    }

                break;

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
                    ppKeyIn = text[0];
                    usleep(5000);
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

            case KeyRelease:
                len = XLookupString((XKeyEvent *)&event, text, 10, &key, 0);
                if (len == 1)
                    {
                    switch (text[0])
                        {
                    default:
                        break;
                        }
                    }
                }
            }

        /*
        **  Process any refresh request.
        */
//        if (refresh)
            {
            XSetForeground (disp, gc, fg);

            XSetFont(disp, gc, hSmallFont);
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

            if (opActive)
                {
                static char opMessage[] = "Operator Interface Active";
                XSetFont(disp, gc, hLargeFont);
                oldFont = FontLarge;
                XDrawString(disp, pixmap, gc, 0, 256, opMessage, strlen(opMessage));
                }

            /*
            **  Protect display list.
            */
            pthread_mutex_lock (&mutexDisplay);

            /*
            **  Draw display list in pixmap.
            */
            curr = display;
            end = display + listEnd;

            for (curr = display; curr < end; curr++)
                {
                /*
                **  Setup new font if necessary.
                */
                if (oldFont != curr->fontSize)
                    {
                    oldFont = curr->fontSize;

                    switch (oldFont)
                        {
                    case FontSmall:
                        XSetFont(disp, gc, hSmallFont);
                        break;

                    case FontMedium:
                        XSetFont(disp, gc, hMediumFont);
                        break;
    
                    case FontLarge:
                        XSetFont(disp, gc, hLargeFont);
                        break;
                        }
                    }

                /*
                **  Draw dot or character.
                */
                if (curr->fontSize == FontDot)
                    {
                    XDrawPoint(disp, pixmap, gc, curr->xPos, (curr->yPos * 14) / 10 + 20);
                    }
                else
                    {
                    str[0] = consoleToAscii[curr->ch];
                    XDrawString(disp, pixmap, gc, curr->xPos, (curr->yPos * 14) / 10 + 20, str, 1);
                    }
                }

            listEnd = 0;
            currentX = -1;
            currentY = -1;
            refresh = FALSE;

            /*
            **  Release display list.
            */
            pthread_mutex_unlock (&mutexDisplay);

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
            }

        usleep(100000);
        }
    }

/*---------------------------  End Of File  ------------------------------*/
