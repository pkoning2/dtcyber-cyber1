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
#define DisplayMargin	8

#define CSETS           8       // leave room for the PPT multiple sets

#ifdef __APPLE__
#define MODMASK 8192    // Option key
#else
#define MODMASK Mod1Mask
#endif

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for the character sets (ROM and loadable)
#define XSize           ((512 + 2 * DisplayMargin) * scale)
#define YSize           ((512 + 2 * DisplayMargin) * scale)
#define YPMSize         ((512 + 2 * DisplayMargin + CSETS * 16) * scale)

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define XADJUST(x) (((x) + DisplayMargin) * scale)
#define YADJUST(y) (YSize - (DisplayMargin + 1 + (y)) * scale)

// inverse mapping (for processing touch input)
#define XUNADJUST(x) ((x / scale) - DisplayMargin)
#define YUNADJUST(y) ((YSize - (y)) / scale - 1 - DisplayMargin)

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
extern u8 mode;
extern int scale;

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

/*
**  X gc function codes for a given W/E mode, used on the pixmap only.
**  (window mode is always GXcopy).  The modes are:
**  inverse, rewrite, erase, write.
**  The erasing modes don't have an "Inverse" in the function code because
**  they are handled by reversing the foreground and background colors.
*/
static const int WeFunc[] = 
{ GXcopy,
  GXcopy,
  GXand,
  GXor 
};

static XRectangle platoRect[2];

/* data for plato font, set 0. */
const unsigned short plato_m0[] = {
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
const unsigned short plato_m1[] = {
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
 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0000, // \
 0x01e0, 0x0210, 0x0210, 0x01e0, 0x0290, 0x0290, 0x01a0, 0x0000, // oe
};

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

bool platoKeypress (XKeyEvent *kp, int stat);
bool platoTouch (XButtonPressedEvent *bp, int stat);
void ptermInput(XEvent *event);
void dtXinit(void);
void ptermSetName (const char *winName);
void ptermLoadChar (int snum, int cnum, const u16 *data);
void ptermSetWeMode (u8 we);

/*--------------------------------------------------------------------------
**  Purpose:        Initialize the Plato terminal window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermInit(const char *winName, bool closeOk)
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
        dtXinit ();
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

    ptermSetName (winName);
    
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
    XSetLineAttributes (disp, wgc, (scale == 1) ? 0 : scale,
                        LineSolid, CapButt, JoinBevel);
    XSetLineAttributes (disp, pgc, (scale == 1) ? 0 : scale,
                        LineSolid, CapButt, JoinBevel);
    
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
    platoRect[0].x = XADJUST (0);
    platoRect[0].y = YADJUST (511);
    platoRect[0].width = 512 * scale;
    platoRect[0].height = 512 * scale;
    platoRect[1].x = 0;
    platoRect[1].y = YSize;
    platoRect[1].width = 512 * scale;
    platoRect[1].height = 16 * CSETS * scale;
    
    XSetClipRectangles (disp, wgc, 0, 0, platoRect, 1, YXSorted);
    XSetClipRectangles (disp, pgc, 0, 0, platoRect, 2, YXSorted);
    
    /*
    **  Initialise input.
    */
    wmhints.flags = InputHint;
    wmhints.input = True;
    XSetWMHints (disp, ptermWindow, &wmhints);
    XSelectInput (disp, ptermWindow,
                  ExposureMask | KeyPressMask |
                  ButtonPressMask | StructureNotifyMask);

    /*
    **  Find the "hand" cursor
    */
    curs = XCreateFontCursor(disp, XC_hand2);
    
    /*
    **  Load Plato ROM characters
    */
    for (i = 0; i < sizeof (plato_m0) / (sizeof (plato_m0[0]) * 8); i++)
        {
        ptermLoadChar (0, i, plato_m0 + (i * 8));
        ptermLoadChar (1, i, plato_m1 + (i * 8));
        }
    
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
**  Purpose:        Set window name
**
**  Parameters:     Name        Description.
**                  winName     name to set.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermSetName (const char *winName)
    {
    /*
    **  Set window and icon titles.
    */
    XSetStandardProperties (disp, ptermWindow, winName, "Pterm",
                            None, NULL, 0, NULL);
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
    /*
    **  We set the window foreground and background to the writing colors,
    **  or to the reverse for the erasing modes, and the writing mode is
    **  always GXcopy.
    **  The pixmap writing varies according to what the Plato mode is.
    */
    if ( (we & 1) == 0)    /* inverse or erase, not char mode */
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
    XSetFunction (disp, wgc, GXcopy);
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
    x = XADJUST (x);
    y = YADJUST (y);
    
    XDrawPoint (disp, pixmap, pgc, x, y);
    XDrawPoint (disp, ptermWindow, wgc, x, y);
    if (scale == 2)
        {
        XDrawPoint (disp, pixmap, pgc, x + 1, y);
        XDrawPoint (disp, ptermWindow, wgc, x + 1, y);
        XDrawPoint (disp, pixmap, pgc, x, y + 1);
        XDrawPoint (disp, ptermWindow, wgc, x, y + 1);
        XDrawPoint (disp, pixmap, pgc, x + 1, y + 1);
        XDrawPoint (disp, ptermWindow, wgc, x + 1, y + 1);
        }    
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
                    DisplayMargin * scale, DisplayMargin * scale,
                    512 * scale, 512 * scale);
    XFillRectangle (disp, pixmap, pgc,
                    DisplayMargin * scale, DisplayMargin * scale,
                    512 * scale, 512 * scale);
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
    int x = cnum * 8 * scale;
    int y = YSize + (snum * 16 + 15) * scale;
    u16 col;

    XSetForeground (disp, pgc, pbg);
    XSetFunction (disp, pgc, GXcopy);
    XFillRectangle (disp, pixmap, pgc, x, y - 15 * scale,
                    8 * scale, 16 * scale);
    XSetForeground (disp, pgc, pfg);

    for (i = 0; i < 8; i++)
        {
        col = *data++;
        for (j = 0; j < 16; j++)
            {
            if (col & 1)
                {
                XDrawPoint(disp, pixmap, pgc,
                           x, y - j * scale);
                if (scale == 2)
                    {
                    XDrawPoint(disp, pixmap, pgc,
                               x + 1, y - j * scale);
                    XDrawPoint(disp, pixmap, pgc,
                               x, y - j * scale + 1);
                    XDrawPoint(disp, pixmap, pgc,
                               x + 1, y - j * scale + 1);
                    }
                }
            col >>= 1;
            }
        x += scale;
        }
    ptermSetWeMode (wemode);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process XButtonPressedEvent for Plato touch panel
**
**  Parameters:     Name        Description.
**                  bp          XButtonPressedEvent for the touch
**                  stat        Station number
**
**  Returns:        TRUE if key event was a valid Plato touch input.
**
**------------------------------------------------------------------------*/
bool platoTouch (XButtonPressedEvent *bp, int stat)
    {
    int x, y;
    
    if (!touchEnabled)
        {
        return FALSE;
        }

    x = XUNADJUST (bp->x);
    y = YUNADJUST (bp->y);
    if (x < 0 || x > 511 ||
        y < 0 || y > 511)
        {
        return FALSE;
        }
    x /= 32;
    y /= 32;

    niuLocalKey (0x100 | (x << 4) | y, stat);
    return TRUE;
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
    bool ctrl = FALSE;
    
    if (state & ControlMask)
        {
        ctrl = TRUE;
        state &= ~ControlMask;
        }
    if (state & ShiftMask)
        {
        state &= ~ShiftMask;
        shift = 040;
        }
    key = XKeycodeToKeysym (disp, kp->keycode, 0);

    // ALT leftarrow is assignment arrow
    if ((state & MODMASK) != 0 && key == XK_Left)
        {
        pc = 015 | shift;       // assignment arrow
        niuLocalKey (pc, stat);
        return TRUE;
        }
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
            if (ctrl && !isalpha (key))
                {
                // control but not a letter -- 
                // translate to what a PLATO keyboard
                // would have on the shifted position for that key
                pc = asciiToPlato[key];
                shift = 040;
                }
            else
                {
                len = XLookupString (kp, text, 10, &ks, 0);
                if (len > 1)
                    {
                    return FALSE;
                    }
                pc = asciiToPlato[text[0]];
                }
            if (pc >= 0)
                {
                if (ctrl)
                    {
                    pc |= shift;
                    }
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
        pc = 023;       // erase
        break;
    case XK_Return:
    case XK_Linefeed:
    case XK_KP_Enter:
        pc = 026;       // next
        break;
    case XK_Home:
    case XK_KP_Home:
    case XK_F8:
        pc = 030;       // back
        break;
    case XK_Pause:
    case XK_Break:
    case XK_F10:
        pc = 032;       // stop
        break;
    case XK_Tab:
        pc = 014;       // tab
        break;
    case XK_KP_Add:
        if (ctrl)
            {
            pc = 056;   // Sigma
            }
        else
            {
            pc = 016;   // +
            }
        break;
    case XK_KP_Subtract:
        if (ctrl)
            {
            pc = 057;   // Delta
            }
        else
            {
            pc = 017;   // -
            }
        break;
    case XK_KP_Multiply:
    case XK_Delete:
    case XK_KP_Delete:
        pc = 012;       // multiply sign
        break;
    case XK_KP_Divide:
    case XK_Insert:
    case XK_KP_Insert:
        pc = 013;       // divide sign
        break;
    case XK_Left:
    case XK_KP_Left:
        pc = 0101;      // left arrow (a)
        break;
    case XK_Right:
    case XK_KP_Right:
        pc = 0104;      // right arrow (d)
        break;
    case XK_Up:
    case XK_KP_Up:
        pc = 0127;      // up arrow (w)
        break;
    case XK_Down:
    case XK_KP_Down:
        pc = 0130;      // down arrow (x)
        break;
    case XK_Page_Up:
        pc = 020;       // super
        break;
    case XK_Page_Down:
        pc = 021;       // sub
        break;
    case XK_F3:
        pc = 034;       // square
        break;
    case XK_F2:
        pc = 022;       // ans
        break;
    case XK_F1:
    case XK_F11:
        pc = 033;       // copy
        break;
    case XK_F9:
        pc = 031;       // data
        break;
    case XK_F5:
        pc = 027;       // edit
        break;
    case XK_F4:
        pc = 024;       // micro/font
        break;
    case XK_F6:
        pc = 025;       // help
        break;
    case XK_F7:
        pc = 035;       // lab
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

    case ButtonPress:
        platoTouch ((XButtonPressedEvent *) event, 1);
        break;
        
    case Expose:
        ptermSetWeMode (0);
        XFillRectangle (disp, ptermWindow, wgc,
                        0, 0, XSize, YSize);
        XSetForeground (disp, wgc, fg);
        XSetBackground (disp, wgc, bg);
        XSetFunction (disp, wgc, GXcopy);
        XCopyPlane(disp, pixmap, ptermWindow, wgc, 
                   DisplayMargin * scale, DisplayMargin * scale,
                   512 * scale, 512 * scale, 
                   DisplayMargin * scale, DisplayMargin * scale, 1);
        // copy the trace marker
        if (tracePterm)
            {
            XSetClipMask (disp, wgc, None);
            XSetClipMask (disp, pgc, None);
            XCopyPlane(disp, pixmap, ptermWindow, wgc, 
                       (DisplayMargin + 512) * scale,
                       (DisplayMargin - 16) * scale,
                       8 * scale, 16 * scale,
                       (DisplayMargin + 512) * scale,
                       (DisplayMargin - 16) * scale, 1);
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
    int charX, charY, sizeX, sizeY, screenX, screenY;
    char c;
    int savemode = wemode;

    if (x >= 1024)
        {
        // Special flag coordinate to write to the status field
        XSetClipMask (disp, wgc, None);
        XSetClipMask (disp, pgc, None);
        }
    
    charX = cnum * 8 * scale;
    charY = YSize + snum * 16 * scale;
    sizeX = 8;
    sizeY = 16;
    screenX = XADJUST (x & 1023);
    screenY = YADJUST (y) - 15 * scale;
    
    if (x < 0)
        {
        sizeX += x;
        charX -= x * scale;
        screenX = XADJUST (0);
        }
    if (y < 0)
        {
        sizeY += y;
        }
    XCopyPlane (disp, pixmap, d, gc, charX, charY,
                sizeX * scale, sizeY * scale, 
                screenX, screenY, 1);
	ptermSetWeMode (1);
    XCopyPlane (disp, pixmap, ptermWindow, wgc,
                screenX, screenY, sizeX * scale, sizeY * scale,
                screenX, screenY, 1);
	ptermSetWeMode (savemode);
    
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
