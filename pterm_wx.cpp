/////////////////////////////////////////////////////////////////////////////
// Name:        pterm_wx.cpp
// Purpose:     pterm interface to wxWindows 
// Author:      Paul Koning
// Modified by:
// Created:     03/26/2005
// Copyright:   (c) Paul Koning
// Licence:     DtCyber license
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define DEFAULTHOST "cyberserv.org"
#define BufSiz      256
#define RINGSIZE    5000
#define RINGXOFF1   1000
#define RINGXOFF2   1100
#define RINGXON1    400
#define RINGXON2    200
#define xonkey      01606
#define xofkey      01607

#define niuRingCount ((niuIn >= niuOut) ? (niuIn - niuOut) : (RINGSIZE + niuIn - niuOut))

#define KeyBufSize	    50
#define DisplayMargin	8

#define CSETS           8       // leave room for the PPT multiple sets

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for the character sets (ROM and loadable)
#define XSize           ((512 + 2 * DisplayMargin) * scale)
#define YSize           ((512 + 2 * DisplayMargin+128) * scale)
#define YPMSize         ((512 + 2 * DisplayMargin + CSETS * 16) * scale)
#define ScreenSize      ((512+128) * scale)

// Size of frame decoration, used to adjust the frame size
#define XFrameAdj       2
#define YFrameAdj       68

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

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/colordlg.h"

extern "C"
{
#if defined(_WIN32)
#include <winsock.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#endif
#include "const.h"
#include "types.h"
#include "proto.h"
#include "pterm.h"
#include "ptermversion.h"
}
    
// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources)
#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__) || defined(__WXMGL__) || defined(__WXX11__)
    #include "mondrian.xpm"
#endif

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

class PtermFrame;

extern "C" {
extern FILE *traceF;
extern char traceFn[];
extern bool tracePterm;
extern u8 wemode;
extern bool emulationActive;
extern const char *hostName;
int scale = 1;
}

// ----------------------------------------------------------------------------
// local variables
// ----------------------------------------------------------------------------

static PtermFrame *pterm_frame;
static int obytes;
static u32 currOutput;
static NetFet fet;
static u16 currInput;
static u32 niuRing[RINGSIZE];
static int niuIn, niuOut;

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Pterm processing thread
class PtermThread : public wxThread
{
public:
    // override base class virtuals
    // ----------------------------
    PtermThread () : wxThread () {}
    virtual ExitCode Entry (void);
};

// Define a new application type, each program should derive a class from wxApp
class PtermApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit () returns false, the application terminates)
    virtual bool OnInit (void);
    virtual int OnExit (void);

    PtermThread *m_thread;
};

class PtermCanvas;

// Define a new frame type: this is going to be our main frame
class PtermFrame : public wxFrame
{
public:
    // ctor(s)
    PtermFrame(const wxString& title, const wxPoint& pos, const wxSize& size,
            long style = wxDEFAULT_FRAME_STYLE);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void PrepareDC(wxDC& dc);

    // PLATO drawing primitives
    void ptermDrawChar (int x, int y, int snum, int cnum);
    void ptermDrawPoint (int x, int y);
    void ptermDrawLine(int x1, int y1, int x2, int y2, bool nomutex = FALSE);
    void ptermFullErase (void);
    void ptermSetName (const char *winName);
    void ptermLoadChar (int snum, int cnum, const u16 *data, bool init = FALSE);
    void ptermSetClip (bool enable);
    
    wxColour    m_colorForeground,    // these are _text_ colors
                m_colorBackground;
    wxBrush     m_backgroundBrush;
    wxBitmap    m_bitmap;
    wxPen       m_foregroundPen;
    wxPen       m_backgroundPen;
    PtermCanvas *m_canvas;
    wxMemoryDC  *m_memDC;

private:
    void drawChar (wxDC &dc, int x, int y, int snum, int cnum);

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// define a scrollable canvas for drawing onto
class PtermCanvas: public wxScrolledWindow
{
public:
    PtermCanvas (PtermFrame *parent);

    void OnDraw(wxDC &dc);
    void OnChar(wxKeyEvent& event);

private:
    PtermFrame *m_owner;

    DECLARE_EVENT_TABLE ()
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Pterm_Quit = 1,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Pterm_About = wxID_ABOUT
};

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
 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0000, /* \ */
 0x01e0, 0x0210, 0x0210, 0x01e0, 0x0290, 0x0290, 0x01a0, 0x0000, // oe
};


// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(PtermFrame, wxFrame)
    EVT_MENU(Pterm_Quit,  PtermFrame::OnQuit)
    EVT_MENU(Pterm_About, PtermFrame::OnAbout)
END_EVENT_TABLE ()

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp () which will return the reference of the right type (i.e. PtermApp and
// not wxApp)
IMPLEMENT_APP(PtermApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool PtermApp::OnInit (void)
{
    int port;
    int true_opt = 1;
#if defined(_WIN32)
    unsigned long ltrue_opt = 1;
#endif

    if (argc > 1 && strcmp (argv[1], "-s") == 0)
    {
        argv++;
        argc--;
        scale = 2;
    }
    
    if (argc > 3)
    {
        printf ("usage: pterm [-s] [ hostname [ portnum ]]\n");
        exit (1);
    }
    if (argc > 2)
    {
        port = atoi (argv[2]);
    }
    else
    {
        port = DefNiuPort;
    }
    if (argc > 1)
    {
        hostName = argv[1];
    }
    else
    {
        hostName = DEFAULTHOST;
    }

    dtInitFet (&fet, BufSiz);
    if (dtConnect (&fet.connFd, hostName, port) < 0)
        {
        return FALSE;
        }
    setsockopt (fet.connFd, SOL_SOCKET, SO_KEEPALIVE,
                (char *)&true_opt, sizeof(true_opt));
#if defined(_WIN32)
    ioctlsocket (fet.connFd, FIONBIO, &ltrue_opt);
#else
    fcntl (fet.connFd, F_SETFL, O_NONBLOCK);
#endif
#ifdef __APPLE__
    setsockopt (fet.connFd, SOL_SOCKET, SO_NOSIGPIPE,
                (char *)&true_opt, sizeof(true_opt));
#endif

    emulationActive = TRUE;

    // create the main application window
    pterm_frame = new PtermFrame(_T("Pterm"),
                                 wxDefaultPosition,
                                 wxSize (XSize + XFrameAdj, YSize + YFrameAdj));

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    pterm_frame->Show(TRUE);
    SetTopWindow(pterm_frame);

    ptermComInit ();
    
    // Create and start the network processing thread
    m_thread = new PtermThread;
    if (m_thread->Create () != wxTHREAD_NO_ERROR)
    {
        return FALSE;
    }
    m_thread->Run ();
    
    // success: wxApp::OnRun () will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
}

int PtermApp::OnExit (void)
{
    return 0;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
PtermFrame::PtermFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(NULL, -1, title, pos, size, style),
      m_bitmap (XSize, YPMSize, 1),
      m_foregroundPen (*wxRED, scale, wxSOLID),     // color will be overridden later
      m_backgroundPen (*wxBLACK, scale, wxSOLID)
{
    int i;
    
    // set the frame icon
    SetIcon(wxICON(mondrian));

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *menuFile = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Pterm_About, _T("&About..."), _T("Show about dialog"));

    menuFile->Append(Pterm_Quit, _T("E&xit\tCtrl-Z"), _T("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append(menuFile, _T("&File"));
    menuBar->Append(helpMenu, _T("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(_T("Not connected"), 1);
#endif // wxUSE_STATUSBAR

    m_colorForeground = wxColour (255, 144, 0);     // PLATO orange
    m_colorBackground = *wxBLACK;

    m_foregroundPen.SetColour (m_colorForeground);
    m_backgroundPen.SetColour (m_colorBackground);

    m_memDC = new wxMemoryDC ();
    m_memDC->SelectObject (m_bitmap);
    m_memDC->SetPen (*wxBLACK_PEN);
	m_memDC->SetBrush (*wxWHITE_BRUSH);
    m_memDC->Blit (0, 0, XSize, YPMSize, m_memDC, 0, 0, wxCLEAR);
    m_canvas = new PtermCanvas (this);

    /*
    **  Load Plato ROM characters
    */
    for (i = 0; i < sizeof (plato_m0) / (sizeof (plato_m0[0]) * 8); i++)
    {
        ptermLoadChar (0, i, plato_m0 + (i * 8), TRUE);
        ptermLoadChar (1, i, plato_m1 + (i * 8), TRUE);
    }
}


// event handlers

void PtermFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // TRUE is to force the frame to close
    Close(TRUE);
}

void PtermFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf (_T("PLATO terminal emulator %s.\n")
                _T("Copyright © 2005 by Paul Koning."),
                PTERMVERSION);

    wxMessageBox(msg, _T("About Pterm"), wxOK | wxICON_INFORMATION, this);
}

#if 0
wxColour PtermFrame::SelectColor (void)
{
    wxColour col;
    wxColourData data;
    wxColourDialog dialog(this, &data);

    if (dialog.ShowModal () == wxID_OK)
    {
        col = dialog.GetColourData ().GetColour ();
    }

    return col;
}
#endif

void PtermFrame::PrepareDC(wxDC& dc)
{
    dc.SetAxisOrientation (TRUE, FALSE);
    if (m_colorForeground.Ok ())
        dc.SetTextForeground (m_colorForeground);
    if (m_colorBackground.Ok ())
        dc.SetTextBackground (m_colorBackground);
    if (! m_backgroundBrush.Ok ()) 
        m_backgroundBrush.SetColour(m_colorBackground);
    if (m_backgroundBrush.Ok ())
        dc.SetBackground (m_backgroundBrush);
    if (! m_foregroundPen.Ok ())
        m_foregroundPen.SetColour (m_colorForeground);
    if (! m_backgroundPen.Ok ())
        m_backgroundPen.SetColour (m_colorBackground);
}

// ----------------------------------------------------------------------------
// PtermThread
// ----------------------------------------------------------------------------

PtermThread::ExitCode PtermThread::Entry (void)
{
    u32 niuwd;
    int i, j, k;
    int next;
    wxIdleEvent doit;

    while (emulationActive)
    {
        i = dtRead (&fet, 10);
        if (i < 0)
        {
            break;
        }
        
        for (;;)
        {
            /*
            **  Assemble words from the network buffer, all the
            **  while looking for "abort output" codes (word == 2).
            */
            next = niuIn + 1;
            if (next == RINGSIZE)
            {
                next = 0;
            }
            if (next == niuOut)
            {
                break;
            }
            if (dtFetData (&fet) < 3)
            {
                break;
            }
            i = dtReado (&fet);
            if (i & 0200)
            {
                printf ("Plato output out of sync byte 0: %03o\n", i);
                continue;
            }
    newj:
            j = dtReado (&fet);
            if ((j & 0300) != 0200)
            {
                printf ("Plato output out of sync byte 1: %03o\n", j);
                if ((j & 0200) == 0)
                {
                    i = j;
                    goto newj;
                }
                continue;
            }
            k = dtReado (&fet);
            if ((k & 0300) != 0300)
            {
                printf ("Plato output out of sync byte 2: %03o\n", k);
                if ((k & 0200) == 0)
                {
                    i = k;
                    goto newj;
                }
                continue;
            }
            niuwd = (i << 12) | ((j & 077) << 6) | (k & 077);
            if (niuwd == 2)
            {
                niuOut = niuIn;
            }
            niuRing[niuIn] = niuwd;
            niuIn = next;
            i = niuRingCount;
            if (i == RINGXOFF1 || i == RINGXOFF2)
            {
                ptermSendKey (xofkey);
            }
        }
        for (;;)
        {
            /*
            **  Process words until we hit some point that causes output delay.
            **  That way we see an abort code as soon as possible.
            */
            if (niuIn == niuOut)
            {
                break;
            }
            next = niuOut + 1;
            if (next == RINGSIZE)
            {
                next = 0;
            }
            j = procNiuWord (niuRing[niuOut]);
            niuOut = next;
            i = niuRingCount;
            if (i == RINGXON1 || i == RINGXON2)
            {
                ptermSendKey (xonkey);
            }
            if (j)
            {
                break;
            }
        }
        // Send a do-nothing event to the frame; that will wake up the main
        // thread and cause the display to be up to date.  If we don't do this,
        // the last bit of data won't show up until later.
        pterm_frame->AddPendingEvent (doit);
        Yield ();
    }
	return (ExitCode) 0;
}

// ----------------------------------------------------------------------------
// PtermCanvas
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them.
BEGIN_EVENT_TABLE(PtermCanvas, wxScrolledWindow)
    EVT_CHAR(PtermCanvas::OnChar)
END_EVENT_TABLE ()

PtermCanvas::PtermCanvas(PtermFrame *parent)
        : wxScrolledWindow(parent, -1, wxDefaultPosition, 
                           wxSize (XSize, YSize),
                           wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxClientDC dc(this);

    m_owner = parent;
    SetVirtualSize (wxSize (XSize, YSize));
    SetScrollbars (1, 1, XSize, YSize);
    dc.SetClippingRegion (XADJUST (0), YADJUST (511), ScreenSize, ScreenSize);
    SetFocus ();
}

void PtermCanvas::OnDraw(wxDC &dc)
{
    m_owner->PrepareDC (dc);
    dc.Clear ();
    dc.Blit (XADJUST (0), YADJUST (511), ScreenSize, ScreenSize,
             m_owner->m_memDC, XADJUST (0), YADJUST (511), wxCOPY);
//	dc.DrawBitmap (m_owner->m_bitmap, 0, 0);
}

void PtermCanvas::OnChar(wxKeyEvent& event)
{
    int key;
    int shift = 0;
    int pc = -1;
    bool ctrl = FALSE;

    if (event.m_controlDown)
    {
        ctrl = TRUE;
    }
    if (event.m_shiftDown)
    {
        shift = 040;
    }
    key = event.m_keyCode;
    if (tracePterm)
            {
            fprintf (traceF, "ctrl %d, shift %d, alt %d, key %d\n", event.m_controlDown, event.m_shiftDown, event.m_altDown, key);
            }
    if (key == (']' & 037))         // control-] : trace
    {
        tracePterm = !tracePterm;
        ptermSetTrace (TRUE);
        return;
    }

    if (key < sizeof (asciiToPlato) / sizeof (asciiToPlato[0]))
    {
        if (event.m_altDown)
        {
            pc = altKeyToPlato[key] | shift;
            
            if (pc >= 0)
            {
                ptermSendKey (pc);
                return;
            }
            else
            {
                event.Skip ();
                return;
            }
        }
        else
        {
            if (ctrl && !isalpha (key) && key >= 040)
            {
                // control but not a letter or ASCII control code -- 
                // translate to what a PLATO keyboard
                // would have on the shifted position for that key
                pc = asciiToPlato[key];
                shift = 040;
            }
            else if (ctrl || key > 040)
            {
                // If we see a char code in the control range, do the table
                // lookup only if the control key is actually down.  That way
                // we avoid doing table lookup for function keys such as 
                // Enter (Return) and Backspace.  Those are handled in the
                // switch statement below.
                pc = asciiToPlato[key];
            }
            if (pc >= 0)
            {
                if (ctrl)
                {
                    pc |= shift;
                }
                ptermSendKey (pc);
                return;
            }
        }
    }
    if (tracePterm)
            {
            fprintf (traceF, "key %d to switch\n",key);
            }
    switch (key)
    {
    case WXK_SPACE:
        pc = 0100;      // space
        break;
    case WXK_BACK:
        pc = 023;       // erase
        break;
    case WXK_RETURN:
        pc = 026;       // next
        break;
    case WXK_HOME:
    case WXK_F8:
        pc = 030;       // back
        break;
    case WXK_PAUSE:
    case WXK_F10:
        pc = 032;       // stop
        break;
    case WXK_TAB:
        pc = 014;       // tab
        break;
    case WXK_ADD:
        if (ctrl)
        {
            pc = 056;   // Sigma
        }
        else
        {
            pc = 016;   // +
        }
        break;
    case WXK_SUBTRACT:
        if (ctrl)
        {
            pc = 057;   // Delta
        }
        else
        {
            pc = 017;   // -
        }
        break;
    case WXK_MULTIPLY:
    case WXK_DELETE:
        pc = 012;       // multiply sign
        break;
    case WXK_DIVIDE:
    case WXK_INSERT:
        pc = 013;       // divide sign
        break;
    case WXK_LEFT:
        pc = 0101;      // left arrow (a)
        break;
    case WXK_RIGHT:
        pc = 0104;      // right arrow (d)
        break;
    case WXK_UP:
        pc = 0127;      // up arrow (w)
        break;
    case WXK_DOWN:
        pc = 0130;      // down arrow (x)
        break;
    case WXK_PRIOR:
        pc = 020;       // super
        break;
    case WXK_NEXT:
        pc = 021;       // sub
        break;
    case WXK_F3:
        pc = 034;       // square
        break;
    case WXK_F2:
        pc = 022;       // ans
        break;
    case WXK_F1:
    case WXK_F11:
        pc = 033;       // copy
        break;
    case WXK_F9:
        pc = 031;       // data
        break;
    case WXK_F5:
        pc = 027;       // edit
        break;
    case WXK_F4:
        pc = 024;       // micro/font
        break;
    case WXK_F6:
        pc = 025;       // help
        break;
    case WXK_F7:
        pc = 035;       // lab
        break;
    default:
        event.Skip ();
        return;
    }
	    if (tracePterm)
            {
            fprintf (traceF, "pc %o, shift %o, key %d\n", pc, shift, key);
            }

    pc |= shift;
    ptermSendKey (pc);
}


void PtermFrame::ptermDrawChar (int x, int y, int snum, int cnum)
{
    wxMutexGuiEnter ();
    
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    PrepareDC (dc);
    m_memDC->SetPen (*wxBLACK_PEN);
	m_memDC->SetBrush (*wxWHITE_BRUSH);
    drawChar (dc, x, y, snum, cnum);
    dc.EndDrawing ();
    wxMutexGuiLeave ();
}

void PtermFrame::ptermDrawPoint (int x, int y)
{
    wxMutexGuiEnter ();
    
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    PrepareDC (dc);
    x = XADJUST (x);
    y = YADJUST (y);
    if (wemode & 1)
    {
        // mode rewrite or write
        dc.SetPen (m_foregroundPen);
    }
    else
    {
        // mode inverse or erase
        dc.SetPen (m_backgroundPen);
    }
    dc.DrawPoint (x, y);
    m_memDC->SetPen (*wxBLACK_PEN);
	m_memDC->SetBrush (*wxWHITE_BRUSH);
    m_memDC->DrawPoint (x, y);
    if (scale == 2)
        {
        dc.DrawPoint (x + 1, y);
        m_memDC->DrawPoint (x + 1, y);
        dc.DrawPoint (x, y + 1);
        m_memDC->DrawPoint (x, y + 1);
        dc.DrawPoint (x + 1, y + 1);
        m_memDC->DrawPoint (x + 1, y + 1);
        }    
    dc.EndDrawing ();
    wxMutexGuiLeave ();
}

void PtermFrame::ptermDrawLine(int x1, int y1, int x2, int y2, bool nomutex)
{
    // the nomutex argument is passed as TRUE when this method is called from
    // the context of the main (GUI) thread.  That is the case for the 
    // ptermShowTrace function, because it executes as a consequence of a 
    // keystroke.  We don't want to wait for the GUI mutex then because we'd
    // wait forever.
    if (!nomutex)
    {
        wxMutexGuiEnter ();
    }
    
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    PrepareDC (dc);
    x1 = XADJUST (x1);
    y1 = YADJUST (y1);
    x2 = XADJUST (x2);
    y2 = YADJUST (y2);
    if (wemode & 1)
    {
        // mode rewrite or write
        dc.SetPen (m_foregroundPen);
    }
    else
    {
        // mode inverse or erase
        dc.SetPen (m_backgroundPen);
    }
    dc.DrawLine (x1, y1, x2, y2);
    m_memDC->SetPen (*wxBLACK_PEN);
	m_memDC->SetBrush (*wxWHITE_BRUSH);
    m_memDC->DrawLine (x1, y1, x2, y2);
    dc.EndDrawing ();
    if (!nomutex)
    {
        wxMutexGuiLeave ();
    }
}

void PtermFrame::ptermFullErase (void)
{
    wxMutexGuiEnter ();
    
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    PrepareDC (dc);
    dc.Blit (XADJUST (0), YADJUST (511), ScreenSize, ScreenSize, 
             m_memDC, XADJUST (0), YADJUST (511), wxCLEAR);
	m_memDC->SetBrush (*wxWHITE_BRUSH);
    m_memDC->Blit (XADJUST (0), YADJUST (511), ScreenSize, ScreenSize,
                   m_memDC, XADJUST (0), YADJUST (511), wxCLEAR);
    dc.EndDrawing ();
    wxMutexGuiLeave ();
}

void PtermFrame::ptermSetName (const char *winName)
{
    wxMutexGuiEnter ();
    SetStatusText(winName, 1);
    wxMutexGuiLeave ();
}

void PtermFrame::ptermLoadChar (int snum, int cnum, const u16 *data, bool init)
{
    int i, j;
    int x = cnum * 8 * scale;
    int y = YSize + (snum * 16 + 15) * scale;
    u16 col;

    // the init argument is passed as TRUE when this method is called from
    // the context of the main (GUI) thread.  That is the case for the 
    // program initialization, when we're loading the "rom" character patterns.
    // We don't want to wait for the GUI mutex then because we'd wait forever.
    if (!init)
    {
        wxMutexGuiEnter ();
    }

    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    m_memDC->SetPen (*wxWHITE_PEN);
	m_memDC->SetBrush (*wxWHITE_BRUSH);
    m_memDC->Blit (x, y - 15 * scale, 8 * scale, 16 * scale,
                   m_memDC, 0, 0, wxCLEAR);
    m_memDC->SetPen (*wxBLACK_PEN);

    for (i = 0; i < 8; i++)
    {
        col = *data++;
        for (j = 0; j < 16; j++)
        {
            if (col & 1)
            {
                m_memDC->DrawPoint(x, y - j * scale);
                if (scale == 2)
                {
                    m_memDC->DrawPoint(x + 1, y - j * scale);
                    m_memDC->DrawPoint(x, y - j * scale + 1);
                    m_memDC->DrawPoint(x + 1, y - j * scale + 1);
                }
            }
            col >>= 1;
        }
        x += scale;
    }
    dc.EndDrawing ();
    if (!init)
    {
        wxMutexGuiLeave ();
    }
}

void PtermFrame::ptermSetClip (bool enable)
{
    wxClientDC dc(m_canvas);

    dc.DestroyClippingRegion();
    if (enable)
    {
        dc.SetClippingRegion (XADJUST (0), YADJUST (511), ScreenSize, ScreenSize);
    }
}

static const int blitFunction[4] =
{ wxCOPY,               // mode inverse
  wxSRC_INVERT,         // mode rewrite
  wxAND,                // mode erase
  wxOR_INVERT           // mode write
};

void PtermFrame::drawChar (wxDC &dc, int x, int y, int snum, int cnum)
    {
    int charX, charY, sizeX, sizeY, screenX, screenY;

    charX = cnum * 8 * scale;
    charY = YSize + snum * 16 * scale;
    sizeX = 8;
    sizeY = 16;
    screenX = XADJUST (x);
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
    // Draw the character into the backing bitmap
    // For some mysterious reason, the COPY operation produces an inversion
    // in the monochrome bitmap, so the functions are the reverse of what
    // you might expect.
    m_memDC->Blit (screenX, screenY, sizeX * scale, sizeY * scale, 
                   m_memDC, charX, charY, blitFunction[wemode]);
    // Now copy the resulting state of the character area into the screen dc
    dc.Blit (screenX, screenY, sizeX * scale, sizeY * scale,
             m_memDC, screenX, screenY, wxCOPY);
    
    // Handle screen edge wraparound by recursion...
    if (x > 512 - 8)
        {
        drawChar (dc, x - 512, y, snum, cnum);
        }
    if (y > 512 - 16)
        {
        drawChar (dc, x, y - 512, snum, cnum);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process Plato mode input
**
**  Parameters:     Name        Description.
**                  key         Plato key code for station
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermSendKey(int key)
    {
    char data[2];
    
    /*
    **  If this is a "composite", recursively send the two pieces.
    */
    if ((key >> 9) != 0)
        {
        ptermSendKey (key >> 9);
        ptermSendKey (key & 0777);
        }
    else
        {
        data[0] = key >> 7;
        data[1] = 0200 | key;

        if (tracePterm)
            {
            fprintf (traceF, "key to plato %03o\n", key);
            }
        send(fet.connFd, data, 2, 0);
        }
    }

// C callable handlers

void ptermDrawChar (int x, int y, int snum, int cnum)
{
    pterm_frame->ptermDrawChar (x, y, snum, cnum);
}

void ptermDrawPoint (int x, int y)
{
    pterm_frame->ptermDrawPoint (x,y);
}

void ptermDrawLine(int x1, int y1, int x2, int y2)
{
    pterm_frame->ptermDrawLine (x1, y1, x2, y2);
}

void ptermFullErase (void)
{
    pterm_frame->ptermFullErase ();
}

void ptermSetName (const char *winName)
{
    pterm_frame->ptermSetName (winName);
}

void ptermLoadChar (int snum, int cnum, const u16 *data)
{
    pterm_frame->ptermLoadChar (snum, cnum, data);
}

void ptermSetWeMode (u8 we)
{
//    pterm_frame->ptermSetWeMode (we);
}

void ptermTouchPanel(bool enable)
{
}

void ptermShowTrace (bool enable)
{
    int savemode = wemode;
    
    if (enable)
    {
        wemode = 3;
    }
    else
    {
        wemode = 2;
    }
    pterm_frame->ptermSetClip (FALSE);
    pterm_frame->ptermDrawLine (-2, -2, 513, -2, TRUE);
    pterm_frame->ptermDrawLine (513, -2, 513, 513, TRUE);
    pterm_frame->ptermDrawLine (513, 513, -2, 513, TRUE);
    pterm_frame->ptermDrawLine (-2, 513, -2, -2, TRUE);
    pterm_frame->ptermDrawLine (-3, -3, 514, -3, TRUE);
    pterm_frame->ptermDrawLine (514, -3, 514, 514, TRUE);
    pterm_frame->ptermDrawLine (514, 514, -3, 514, TRUE);
    pterm_frame->ptermDrawLine (-3, 514, -3, -3, TRUE);
    pterm_frame->ptermSetClip (TRUE);
    wemode = savemode;
}

