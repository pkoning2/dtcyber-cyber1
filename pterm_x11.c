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
#define XSize           (512 + 2 * DisplayMargin)
#define YSize           (512 + 2 * DisplayMargin)

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
static void setColors (u8 wemode);
static void drawChar (Drawable d, GC gc, int snum, int cnum);
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

/* data for plato font, set 0. */
const u16 plato_m0[] = {
    0x0000, 0x0000, 0x0330, 0x0330, 0x0000, 0x0000, 0x0000, 0x0000, // :
    0x0060, 0x0290, 0x0290, 0x0290, 0x0290, 0x01e0, 0x0010, 0x0000, // a
    0x1ff0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x00c0, 0x0000, // b
    0x00c0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0210, 0x0120, 0x0000, // c
    0x00c0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x1ff0, 0x0000, // d
    0x00c0, 0x01a0, 0x0290, 0x0290, 0x0290, 0x0290, 0x0190, 0x0000, // e
    0x0000, 0x0000, 0x0210, 0x0ff0, 0x1210, 0x1000, 0x0800, 0x0000, // f
    0x01a8, 0x0254, 0x0254, 0x0254, 0x0254, 0x0194, 0x0208, 0x0000, // g
    0x1000, 0x1ff0, 0x0100, 0x0200, 0x0200, 0x0200, 0x01f0, 0x0000, // h
    0x0000, 0x0000, 0x0210, 0x13f0, 0x0010, 0x0000, 0x0000, 0x0000, // i
    0x0000, 0x0002, 0x0202, 0x13fc, 0x0000, 0x0000, 0x0000, 0x0000, // j
    0x1010, 0x1ff0, 0x0080, 0x0140, 0x0220, 0x0210, 0x0010, 0x0000, // k
    0x0000, 0x0000, 0x1010, 0x1ff0, 0x0010, 0x0000, 0x0000, 0x0000, // l
    0x03f0, 0x0200, 0x0200, 0x01f0, 0x0200, 0x0200, 0x01f0, 0x0000, // m
    0x0200, 0x03f0, 0x0100, 0x0200, 0x0200, 0x0200, 0x01f0, 0x0000, // n
    0x00c0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x00c0, 0x0000, // o
    0x03fe, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x00c0, 0x0000, // p
    0x00c0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x03fe, 0x0000, // q
    0x0200, 0x03f0, 0x0100, 0x0200, 0x0200, 0x0200, 0x0100, 0x0000, // r
    0x0120, 0x0290, 0x0290, 0x0290, 0x0290, 0x0290, 0x0060, 0x0000, // s
    0x0200, 0x0200, 0x1fe0, 0x0210, 0x0210, 0x0210, 0x0000, 0x0000, // t
    0x03e0, 0x0010, 0x0010, 0x0010, 0x0010, 0x03e0, 0x0010, 0x0000, // u
    0x0200, 0x0300, 0x00c0, 0x0030, 0x00c0, 0x0300, 0x0200, 0x0000, // v
    0x03e0, 0x0010, 0x0020, 0x01c0, 0x0020, 0x0010, 0x03e0, 0x0000, // w
    0x0200, 0x0210, 0x0120, 0x00c0, 0x00c0, 0x0120, 0x0210, 0x0000, // x
    0x0382, 0x0044, 0x0028, 0x0010, 0x0020, 0x0040, 0x0380, 0x0000, // y
    0x0310, 0x0230, 0x0250, 0x0290, 0x0310, 0x0230, 0x0000, 0x0000, // z
    0x0010, 0x07e0, 0x0850, 0x0990, 0x0a10, 0x07e0, 0x0800, 0x0000, // 0
    0x0000, 0x0000, 0x0410, 0x0ff0, 0x0010, 0x0000, 0x0000, 0x0000, // 1
    0x0000, 0x0430, 0x0850, 0x0890, 0x0910, 0x0610, 0x0000, 0x0000, // 2
    0x0000, 0x0420, 0x0810, 0x0910, 0x0910, 0x06e0, 0x0000, 0x0000, // 3
    0x0000, 0x0080, 0x0180, 0x0280, 0x0480, 0x0ff0, 0x0080, 0x0000, // 4
    0x0000, 0x0f10, 0x0910, 0x0910, 0x0920, 0x08c0, 0x0000, 0x0000, // 5
    0x0000, 0x03e0, 0x0510, 0x0910, 0x0910, 0x00e0, 0x0000, 0x0000, // 6
    0x0000, 0x0800, 0x0830, 0x08c0, 0x0b00, 0x0c00, 0x0000, 0x0000, // 7
    0x0000, 0x06e0, 0x0910, 0x0910, 0x0910, 0x06e0, 0x0000, 0x0000, // 8
    0x0000, 0x0700, 0x0890, 0x0890, 0x08a0, 0x07c0, 0x0000, 0x0000, // 9
    0x0000, 0x0080, 0x0080, 0x03e0, 0x0080, 0x0080, 0x0000, 0x0000, // +
    0x0000, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000, 0x0000, // -
    0x0000, 0x0240, 0x0180, 0x0660, 0x0180, 0x0240, 0x0000, 0x0000, // *
    0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0000, // /
    0x0000, 0x0000, 0x0000, 0x0000, 0x07e0, 0x0810, 0x1008, 0x0000, // (
    0x1008, 0x0810, 0x07e0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // )
    0x0640, 0x0920, 0x0920, 0x1ff0, 0x0920, 0x0920, 0x04c0, 0x0000, // $
    0x0000, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0000, 0x0000, // =
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // space
    0x0000, 0x0000, 0x0034, 0x0038, 0x0000, 0x0000, 0x0000, 0x0000, // ,
    0x0000, 0x0000, 0x0030, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, // .
    0x0000, 0x0080, 0x0080, 0x02a0, 0x0080, 0x0080, 0x0000, 0x0000, // divide
    0x0000, 0x0000, 0x0000, 0x0000, 0x1ff8, 0x1008, 0x1008, 0x0000, // [
    0x1008, 0x1008, 0x1ff8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // ]
    0x0c20, 0x1240, 0x0c80, 0x0100, 0x0260, 0x0490, 0x0860, 0x0000, // %
    0x0000, 0x0000, 0x0240, 0x0180, 0x0180, 0x0240, 0x0000, 0x0000, // multiply
    0x0080, 0x0140, 0x0220, 0x0770, 0x0140, 0x0140, 0x0140, 0x0000, // assign
    0x0000, 0x0000, 0x0000, 0x1c00, 0x0000, 0x0000, 0x0000, 0x0000, // '
    0x0000, 0x0000, 0x1c00, 0x0000, 0x1c00, 0x0000, 0x0000, 0x0000, // "
    0x0000, 0x0000, 0x0000, 0x1f90, 0x0000, 0x0000, 0x0000, 0x0000, // !
    0x0000, 0x0000, 0x0334, 0x0338, 0x0000, 0x0000, 0x0000, 0x0000, // ;
    0x0000, 0x0080, 0x0140, 0x0220, 0x0410, 0x0000, 0x0000, 0x0000, // <
    0x0000, 0x0000, 0x0410, 0x0220, 0x0140, 0x0080, 0x0000, 0x0000, // >
    0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, // _
    0x0000, 0x0c00, 0x1000, 0x10d0, 0x1100, 0x0e00, 0x0000, 0x0000, // ?
    0x1c1c, 0x1224, 0x0948, 0x0490, 0x0220, 0x0140, 0x0080, 0x0000, // arrow
    0x0000, 0x0000, 0x0000, 0x000a, 0x0006, 0x0000, 0x0000, 0x0000, // cedilla
};

/* data for plato font, set 1. */
const u16 plato_m1[] = {
    0x0500, 0x0500, 0x1fc0, 0x0500, 0x1fc0, 0x0500, 0x0500, 0x0000, // #
    0x07f0, 0x0900, 0x1100, 0x1100, 0x1100, 0x0900, 0x07f0, 0x0000, // A
    0x1ff0, 0x1210, 0x1210, 0x1210, 0x1210, 0x0e10, 0x01e0, 0x0000, // B
    0x07c0, 0x0820, 0x1010, 0x1010, 0x1010, 0x1010, 0x0820, 0x0000, // C
    0x1ff0, 0x1010, 0x1010, 0x1010, 0x1010, 0x0820, 0x07c0, 0x0000, // D
    0x1ff0, 0x1110, 0x1110, 0x1110, 0x1010, 0x1010, 0x1010, 0x0000, // E
    0x1ff0, 0x1100, 0x1100, 0x1100, 0x1000, 0x1000, 0x1000, 0x0000, // F
    0x07c0, 0x0820, 0x1010, 0x1010, 0x1090, 0x1090, 0x08e0, 0x0000, // G
    0x1ff0, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x1ff0, 0x0000, // H
    0x0000, 0x1010, 0x1010, 0x1ff0, 0x1010, 0x1010, 0x0000, 0x0000, // I
    0x0020, 0x0010, 0x1010, 0x1010, 0x1fe0, 0x1000, 0x1000, 0x0000, // J
    0x1ff0, 0x0080, 0x0100, 0x0280, 0x0440, 0x0820, 0x1010, 0x0000, // K
    0x1ff0, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0000, // L
    0x1ff0, 0x0800, 0x0400, 0x0200, 0x0400, 0x0800, 0x1ff0, 0x0000, // M
    0x1ff0, 0x0800, 0x0600, 0x0100, 0x00c0, 0x0020, 0x1ff0, 0x0000, // N
    0x07c0, 0x0820, 0x1010, 0x1010, 0x1010, 0x0820, 0x07c0, 0x0000, // O
    0x1ff0, 0x1100, 0x1100, 0x1100, 0x1100, 0x1100, 0x0e00, 0x0000, // P
    0x07c0, 0x0820, 0x1010, 0x1018, 0x1014, 0x0824, 0x07c0, 0x0000, // Q
    0x1ff0, 0x1100, 0x1100, 0x1180, 0x1140, 0x1120, 0x0e10, 0x0000, // R
    0x0e20, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x08e0, 0x0000, // S
    0x1000, 0x1000, 0x1000, 0x1ff0, 0x1000, 0x1000, 0x1000, 0x0000, // T
    0x1fe0, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x1fe0, 0x0000, // U
    0x1800, 0x0700, 0x00c0, 0x0030, 0x00c0, 0x0700, 0x1800, 0x0000, // V
    0x1fe0, 0x0010, 0x0020, 0x03c0, 0x0020, 0x0010, 0x1fe0, 0x0000, // W
    0x1830, 0x0440, 0x0280, 0x0100, 0x0280, 0x0440, 0x1830, 0x0000, // X
    0x1800, 0x0400, 0x0200, 0x01f0, 0x0200, 0x0400, 0x1800, 0x0000, // Y
    0x1830, 0x1050, 0x1090, 0x1110, 0x1210, 0x1410, 0x1830, 0x0000, // Z
    0x0000, 0x1000, 0x2000, 0x2000, 0x1000, 0x1000, 0x2000, 0x0000, // ~
    0x0000, 0x0000, 0x1000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, // dieresis
    0x0000, 0x1000, 0x2000, 0x4000, 0x2000, 0x1000, 0x0000, 0x0000, // circumflex
    0x0000, 0x0000, 0x0000, 0x1000, 0x2000, 0x4000, 0x0000, 0x0000, // acute
    0x0000, 0x4000, 0x2000, 0x1000, 0x0000, 0x0000, 0x0000, 0x0000, // grave
    0x0000, 0x0100, 0x0300, 0x07f0, 0x0300, 0x0100, 0x0000, 0x0000, // uparrow
    0x0080, 0x0080, 0x0080, 0x0080, 0x03e0, 0x01c0, 0x0080, 0x0000, // rightarrow 
    0x0000, 0x0040, 0x0060, 0x07f0, 0x0060, 0x0040, 0x0000, 0x0000, // downarrow
    0x0080, 0x01c0, 0x03e0, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000, // leftarrow
    0x0000, 0x0080, 0x0100, 0x0100, 0x0080, 0x0080, 0x0100, 0x0000, // low tilde
    0x1010, 0x1830, 0x1450, 0x1290, 0x1110, 0x1010, 0x1010, 0x0000, // Sigma
    0x0030, 0x00d0, 0x0310, 0x0c10, 0x0310, 0x00d0, 0x0030, 0x0000, // Delta
    0x0000, 0x0380, 0x0040, 0x0040, 0x0040, 0x0380, 0x0000, 0x0000, // union
    0x0000, 0x01c0, 0x0200, 0x0200, 0x0200, 0x01c0, 0x0000, 0x0000, // intersect
    0x0000, 0x0000, 0x0000, 0x0080, 0x0f78, 0x1004, 0x1004, 0x0000, // {
    0x1004, 0x1004, 0x0f78, 0x0080, 0x0000, 0x0000, 0x0000, 0x0000, // }
    0x00e0, 0x0d10, 0x1310, 0x0c90, 0x0060, 0x0060, 0x0190, 0x0000, // &
    0x0150, 0x0160, 0x0140, 0x01c0, 0x0140, 0x0340, 0x0540, 0x0000, // not equal
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // space
    0x0000, 0x0000, 0x0000, 0x1ff0, 0x0000, 0x0000, 0x0000, 0x0000, // |
    0x0000, 0x0c00, 0x1200, 0x1200, 0x0c00, 0x0000, 0x0000, 0x0000, // degree
    0x0000, 0x02a0, 0x02a0, 0x02a0, 0x02a0, 0x02a0, 0x0000, 0x0000, // equiv
    0x01e0, 0x0210, 0x0210, 0x01a0, 0x0060, 0x0090, 0x0310, 0x0000, // alpha
    0x0002, 0x03fc, 0x0510, 0x0910, 0x0910, 0x0690, 0x0060, 0x0000, // beta
    0x0000, 0x0ce0, 0x1310, 0x1110, 0x0890, 0x0460, 0x0000, 0x0000, // delta
    0x0000, 0x1030, 0x0cc0, 0x0300, 0x00c0, 0x0030, 0x0000, 0x0000, // lambda
    0x0002, 0x0002, 0x03fc, 0x0010, 0x0010, 0x03e0, 0x0010, 0x0000, // mu
    0x0100, 0x0200, 0x03f0, 0x0200, 0x03f0, 0x0200, 0x0400, 0x0000, // pi
    0x0006, 0x0038, 0x00e0, 0x0110, 0x0210, 0x0220, 0x01c0, 0x0000, // rho
    0x00e0, 0x0110, 0x0210, 0x0310, 0x02e0, 0x0200, 0x0200, 0x0000, // sigma
    0x01e0, 0x0210, 0x0010, 0x00e0, 0x0010, 0x0210, 0x01e0, 0x0000, // omega
    0x0220, 0x0220, 0x0520, 0x0520, 0x08a0, 0x08a0, 0x0000, 0x0000, // less/equal
    0x0000, 0x08a0, 0x08a0, 0x0520, 0x0520, 0x0220, 0x0220, 0x0000, // greater/equal
    0x07c0, 0x0920, 0x1110, 0x1110, 0x1110, 0x0920, 0x07c0, 0x0000, // theta
    0x01e0, 0x0210, 0x04c8, 0x0528, 0x05e8, 0x0220, 0x01c0, 0x0000, // @
    0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0000, // backslash
    0x01e0, 0x0210, 0x0210, 0x01e0, 0x0290, 0x0290, 0x01a0, 0x0000, // oe
};

const u16 *setPtr[] =
{
    plato_m0, plato_m1,
    plato_m23, plato_m23 + (64 * 8)
};

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
    pixmap = XCreatePixmap (disp, ptermWindow, XSize, YSize, 1);

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
    XSetForeground (disp, wgc, fg);
    XSetForeground (disp, pgc, pbg);
    XFillRectangle (disp, pixmap, pgc, 0, 0, XSize, YSize);
    XSetForeground (disp, pgc, pfg);

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

    /*
    **  We like to be on top.
    */
    XMapRaised (disp, ptermWindow);
    XSync (disp, FALSE);

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
**                  stat        Sstatio number
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
        XCopyPlane(disp, pixmap, ptermWindow, wgc, 0, 0, XSize, YSize, 0, 0, 1);
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
                XSetForeground (disp, wgc, bg);
                XFillRectangle (disp, ptermWindow, wgc, 0, 0, XSize, YSize);
                XSetForeground (disp, wgc, fg);
                XSetForeground (disp, pgc, pbg);
                XFillRectangle (disp, pixmap, pgc, 0, 0, XSize, YSize);
                XSetForeground (disp, pgc, pfg);
                XFlush (disp);
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
**  Purpose:        Write a character to the font area in the pixmap
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
    int i, j;
    int x = currentX;
    const u16 *cp;
    u16 col;
    u8 drawMode = wemode | 2;
    u8 eraseMode = (wemode ^ 1) | 2;
    
    cp = setPtr[snum] + (8 * cnum);
    for (i = 0; i < 8; i++)
    {
        col = *cp++;
        for (j = 0; j < 16; j++)
        {
            if (col & 1)
            {
                setColors (drawMode);
                XDrawPoint(disp, d, gc, XADJUST (x), YADJUST (currentY + j));
            }
            else if ((wemode & 2) == 0)
            {
                setColors (eraseMode);
                XDrawPoint(disp, d, gc, XADJUST (x), YADJUST (currentY + j));
            }
            col >>= 1;
        }
        x = (x + 1) & 0777;
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Set drawing color given current W/E mode
**
**  Parameters:     Name        Description.
**                  we          mode byte
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void setColors (u8 we)
{
    switch (we)
    {
    case 0:
        XSetBackground(disp, pgc, pfg);
        XSetBackground(disp, wgc, fg);
    case 2:
        XSetForeground(disp, pgc, pbg);
        XSetForeground(disp, wgc, bg);
        break;
    case 1:
        XSetBackground(disp, pgc, pbg);
        XSetBackground(disp, wgc, bg);
    case 3:
        XSetForeground(disp, pgc, pfg);
        XSetForeground(disp, wgc, fg);
        break;
    }
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
    setColors (wemode);
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
    setColors (wemode);
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
