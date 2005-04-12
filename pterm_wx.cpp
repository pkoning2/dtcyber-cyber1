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
#if 0
#define xonkey      01606
#define xofkey      01607
#endif

#define STATUS_TIP  0
#define STATUS_TRC  1
#define STATUS_CONN 2
#define STATUSPANES 3

#define niuRingCount ((niuIn >= niuOut) ? (niuIn - niuOut) : (RINGSIZE + niuIn - niuOut))

#define KeyBufSize      50
#define DisplayMargin   8

#define CSETS           8       // leave room for the PPT multiple sets

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for the character sets (ROM and loadable)
#define XSize           ((512 + 2 * DisplayMargin) * scale)
#define YSize           ((512 + 2 * DisplayMargin) * scale)
#define CharYSize       ((CSETS * 16) * scale)
#define ScreenSize      (512 * scale)

// Size of frame decoration, used to adjust the frame size
// These work for WinXP.  There should be a better way to do this...
#define XFrameAdj       8
#define YFrameAdj       74

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

#include "wx/clipbrd.h"
#include "wx/colordlg.h"
#include "wx/config.h"

extern "C"
{
#include <ctype.h>
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
    
#if wxUSE_UNICODE
#warning "Unicode support isn't fully baked yet"
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources)
#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__) || defined(__WXMGL__) || defined(__WXX11__)
#include "pterm_32.xpm"
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
    int scale = 1;
}

// ----------------------------------------------------------------------------
// local variables
// ----------------------------------------------------------------------------

class PtermApp;
static PtermApp *ptermApp;

#ifdef DEBUG
static wxLogWindow *logwindow;
#endif

static PtermFrame *pterm_frame;
static int obytes;
static u32 currOutput;
static NetFet fet;
static u16 currInput;
static u32 niuRing[RINGSIZE];
static volatile int niuIn, niuOut;
static char *hostName;

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

    wxColour    m_fgColor;
    wxColour    m_bgColor;
    wxConfig    *m_config;
    wxClipboard *m_clipboard;
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
    void OnQuit (wxCommandEvent& event);
    void OnAbout (wxCommandEvent& event);
    void OnPref (wxCommandEvent& event);
    void OnCopyScreen (wxCommandEvent &event);
    wxColour SelectColor (wxColour &initcol);

    void PrepareDC(wxDC& dc);

    // PLATO drawing primitives
    void ptermDrawChar (int x, int y, int snum, int cnum);
    void ptermDrawPoint (int x, int y);
    void ptermDrawLine(int x1, int y1, int x2, int y2);
    void ptermFullErase (void);
    void ptermSetName (const char *winName);
    void ptermSetStatus (const char *str);
    void ptermLoadChar (int snum, int cnum, const u16 *data);
    void ptermLoadRomChars (void);
    void ptermSetCharColors (wxColour &fb, wxColour &bg);
    void ptermSetSize (bool scale2);
    
    PtermThread *m_thread;
    wxBrush     m_backgroundBrush;
    wxBitmap    m_bitmap;
    wxPen       m_foregroundPen;
    wxPen       m_backgroundPen;
    PtermCanvas *m_canvas;
    wxMemoryDC  *m_memDC;
    
    // Character patterns are stored in three DCs because we want to
    // BLIT them in various ways, and the OR/AND type ops don't work
    // the way you'd want for color bitmaps.  Monochrome bitmaps would
    // be great, except that they don't work in Windows...
    //
    // So what we do is this:
    // m_charmap[i] contains the character patterns for mode i -- 
    // the right color pixels where we want color, and black where
    // we leave the screen untouched.
    //
    // m_charmap[4] contains a mask -- white for the character background,
    // black for the character foreground.  So we AND with that to erase the
    // character pixels (we need to do that for modes erase and write only).
    //
    // m_charDC[i] is the device context into which m_charmap[i] is mapped.
    wxBitmap    *m_charmap[5];
    wxMemoryDC  *m_charDC[5];

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

    void ptermTouchPanel(bool enable);

    void OnDraw (wxDC &dc);
    void OnChar (wxKeyEvent& event);
    void OnKeyDown (wxKeyEvent& event);
    void OnMouseDown (wxMouseEvent &event);
    void OnIdle (wxIdleEvent& event);
    
private:
    PtermFrame *m_owner;
    bool        m_touchEnabled;

    DECLARE_EVENT_TABLE ()
};

// define the preferences dialog
class PtermPrefdialog : public wxDialog
{
public:
    PtermPrefdialog (PtermFrame *parent, wxWindowID id, const wxString &title);
    
    void OnButton (wxCommandEvent& event);
    void OnCheckbox (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    wxBitmap        *m_fgBitmap;
    wxBitmap        *m_bgBitmap;
    wxBitmapButton  *m_fgButton;
    wxBitmapButton  *m_bgButton;
    wxButton        *m_okButton;
    wxButton        *m_cancelButton;
    wxButton        *m_resetButton;
    wxStaticText    *m_fgLabel;
    wxStaticText    *m_bgLabel;
    wxBoxSizer      *m_fgSizer;
    wxBoxSizer      *m_bgSizer;
    wxStaticBox     *m_colorsBox;
    wxStaticBoxSizer *m_colorsSizer;
    wxCheckBox      *m_scaleCheck;
    wxBoxSizer      *m_prefItems;
    wxBoxSizer      *m_prefButtons;
    wxBoxSizer      *m_dialogContent;

    wxColour        m_fgColor;
    wxColour        m_bgColor;
    bool            m_scale2;
    
private:
    void paintBitmap (wxBitmap *bm, wxColour &color, wxBitmapButton *to);
    
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
    Pterm_CopyScreen = 1,

    Pterm_Quit = wxID_EXIT,
#ifdef wxID_PREFERENCES
    Pterm_Pref = wxID_PREFERENCES,
#else
    Pterm_Pref,
#endif
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
    EVT_MENU(Pterm_Pref,  PtermFrame::OnPref)
    EVT_MENU(Pterm_CopyScreen, PtermFrame::OnCopyScreen)
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
    int r, g, b;
    wxString rgb;

    ptermApp = this;

#ifdef DEBUG
    logwindow = new wxLogWindow (NULL, "pterm log", TRUE, FALSE);
#endif

    if (argc > 1 && strcmp (wxString (argv[1]).mb_str (), "-s") == 0)
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
        port = atoi (wxString (argv[2]).mb_str ());
    }
    else
    {
        port = DefNiuPort;
    }
    if (argc > 1)
    {
        hostName = strdup (wxString (argv[1]).mb_str ());
    }
    else
    {
        hostName = strdup (DEFAULTHOST);
    }

    dtInitFet (&fet, BufSiz);
    if (dtConnect (&fet.connFd, hostName, port) < 0)
    {
        free (hostName);
        return FALSE;
    }
    setsockopt (fet.connFd, SOL_SOCKET, SO_KEEPALIVE,
                (char *)&true_opt, sizeof(true_opt));
#ifdef __APPLE__
    setsockopt (fet.connFd, SOL_SOCKET, SO_NOSIGPIPE,
                (char *)&true_opt, sizeof(true_opt));
#endif
    emulationActive = TRUE;

    m_config = new wxConfig (wxT ("Pterm"));
    // 255 144 0 is RGB for Plato Orange
    m_config->Read (wxT ("foreground"), &rgb, wxT ("255 144 0"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_fgColor = wxColour (r, g, b);
    m_config->Read (wxT ("background"), &rgb, wxT ("0 0 0"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_bgColor = wxColour (r, g, b);
    scale = m_config->Read (wxT ("scale"), 1);
    if (scale != 1 && scale != 2)
    {
        scale = 1;
    }

    // create the main application window
    pterm_frame = new PtermFrame(wxT("Pterm ") PTERMVERSION,
                                 wxDefaultPosition,
                                 wxSize (XSize + XFrameAdj, YSize + YFrameAdj));

    // create the clipboard object
    m_clipboard = new wxClipboard;

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    pterm_frame->Show(TRUE);
    SetTopWindow(pterm_frame);

    ptermComInit ();

    // success: wxApp::OnRun () will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
}

int PtermApp::OnExit (void)
{
    free (hostName);

    m_clipboard->Flush ();
    delete m_clipboard;

#ifdef DEBUG
    delete logwindow;
#endif

    return 0;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
PtermFrame::PtermFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(NULL, -1, title, pos, size, style),
      m_bitmap (XSize, YSize, -1),
      m_foregroundPen (ptermApp->m_fgColor, scale, wxSOLID),
      m_backgroundPen (ptermApp->m_bgColor, scale, wxSOLID),
      m_backgroundBrush (ptermApp->m_bgColor, wxSOLID)
{
    int i;
    
    // set the frame icon
    SetIcon(wxICON(pterm_32));

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *menuFile = new wxMenu;

    menuFile->Append(Pterm_Pref, _T("P&references..."), _T("Set program configuration"));
    menuFile->AppendSeparator();
    menuFile->Append(Pterm_Quit, _T("E&xit\tCtrl-Z"), _T("Quit this program"));

    wxMenu *menuEdit = new wxMenu;

    menuEdit->Append (Pterm_CopyScreen, _T("Copy Screen"), _T("Copy screen to clipboard"));

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Pterm_About, _T("&About..."), _T("Show about dialog"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append(menuFile, _T("&File"));
    menuBar->Append(menuEdit, _T("&Edit"));
    menuBar->Append(helpMenu, _T("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun
    CreateStatusBar(STATUSPANES);
    SetStatusText(_T(" Not connected"), STATUS_CONN);
#endif // wxUSE_STATUSBAR

    for (i = 0; i < 5; i++)
    {
        m_charDC[i] = new wxMemoryDC ();
        m_charmap[i] = new wxBitmap (XSize, CharYSize, -1);
        m_charDC[i]->SelectObject (*m_charmap[i]);
    }
    m_memDC = new wxMemoryDC ();
    m_memDC->SelectObject (m_bitmap);
    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->Clear ();
    m_memDC->SetClippingRegion (XADJUST (0), YADJUST (511), ScreenSize, ScreenSize);
    m_canvas = new PtermCanvas (this);

    /*
    **  Load Plato ROM characters
    */
    ptermLoadRomChars ();
    
    // Create and start the network processing thread
    m_thread = new PtermThread;
    if (m_thread->Create () != wxTHREAD_NO_ERROR)
    {
        return;
    }
    m_thread->Run ();
}


// event handlers

void PtermFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    int i;
    
    m_thread->Kill ();
    m_memDC->SelectObject (wxNullBitmap);
    m_memDC->SetBackground (wxNullBrush);
    m_memDC->SetPen (wxNullPen);
    delete m_memDC;
    m_memDC = NULL;
    for (i = 0; i < 5; i++)
    {
        m_charDC[i]->SelectObject (wxNullBitmap);
        m_charDC[i]->SetBackground (wxNullBrush);
        m_charDC[i]->SetPen (wxNullPen);
        delete m_charDC[i];
        delete m_charmap[i];
    }

    // TRUE is to force the frame to close
    Close (TRUE);
}

void PtermFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;

    msg.Printf (wxT("PLATO terminal emulator %s.\n")
// temp: need to figure out how to get gcc to understand this:
//                wxT("Copyright © 2005 by Paul Koning."),
                wxT("Copyright 2005 by Paul Koning."),
                wxT (PTERMVERSION));
    
    wxMessageBox(msg, _T("About Pterm"), wxOK | wxICON_INFORMATION, this);
}

void PtermFrame::OnPref (wxCommandEvent& WXUNUSED(event))
{
    PtermPrefdialog dlg (this, wxID_ANY, _("Pterm Preferences"));
    
    dlg.SetSizer (dlg.m_dialogContent);
    dlg.m_dialogContent->Fit (&dlg);
    if (dlg.ShowModal () == wxID_OK)
    {
        ptermSetCharColors (dlg.m_fgColor, dlg.m_bgColor);
        ptermSetSize (dlg.m_scale2);
    }
}

void PtermFrame::OnCopyScreen (wxCommandEvent & WXUNUSED(event))
{
    wxBitmap screenmap (ScreenSize, ScreenSize);
    wxMemoryDC screenDC;
    wxBitmapDataObject *screen;

    screenDC.SelectObject (screenmap);
    screenDC.Blit (0, 0, ScreenSize, ScreenSize, 
                   m_memDC, XADJUST (0), YADJUST (511), wxCOPY);
    screenDC.SelectObject (wxNullBitmap);

    screen = new wxBitmapDataObject(screenmap);

    if (ptermApp->m_clipboard->Open ())
    {
        ptermApp->m_clipboard->SetData (screen);
        ptermApp->m_clipboard->Close ();
    }
}

wxColour PtermFrame::SelectColor (wxColour &initcol)
{
    wxColour col (initcol);
    wxColour orange (255, 144, 0);
    wxColourData data;

    data.SetColour (initcol);
    data.SetCustomColour (0, orange);
    
    wxColourDialog dialog(this, &data);

    if (dialog.ShowModal () == wxID_OK)
    {
        col = dialog.GetColourData ().GetColour ();
    }

    return col;
}

void PtermFrame::PrepareDC(wxDC& dc)
{
    dc.SetAxisOrientation (TRUE, FALSE);
    dc.SetBackground (m_backgroundBrush);
}

// ----------------------------------------------------------------------------
// PtermPrefdialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(PtermPrefdialog, wxDialog)
    EVT_CLOSE(PtermPrefdialog::OnClose)
    EVT_BUTTON(wxID_ANY,   PtermPrefdialog::OnButton)
    EVT_CHECKBOX(wxID_ANY, PtermPrefdialog::OnCheckbox)
    END_EVENT_TABLE()

PtermPrefdialog::PtermPrefdialog (PtermFrame *parent, wxWindowID id, const wxString &title)
    : wxDialog (parent, id, title),
      m_owner (parent)
{
    m_scale2 = (scale != 1);
    m_fgColor = ptermApp->m_fgColor;
    m_bgColor = ptermApp->m_bgColor;

    m_fgBitmap = new wxBitmap (20, 12);
    m_bgBitmap = new wxBitmap (20, 12);
    paintBitmap (m_fgBitmap, m_fgColor, NULL);
    paintBitmap (m_bgBitmap, m_bgColor, NULL);
    
    m_fgButton = new wxBitmapButton (this, wxID_ANY, *m_fgBitmap);
    m_bgButton = new wxBitmapButton (this, wxID_ANY, *m_bgBitmap);
    m_okButton = new wxButton (this, wxID_ANY, _("OK"));
    m_cancelButton = new wxButton (this, wxID_ANY, _("Cancel"));
    m_resetButton = new wxButton (this, wxID_ANY, _("Defaults"));
    m_fgLabel = new wxStaticText (this, wxID_ANY, _("Foreground"));
    m_bgLabel = new wxStaticText (this, wxID_ANY, _("Background"));
    m_fgSizer = new wxBoxSizer (wxHORIZONTAL);
    m_bgSizer = new wxBoxSizer (wxHORIZONTAL);
    m_colorsBox = new wxStaticBox (this, wxID_ANY, _("Display colors"));
    m_colorsSizer = new wxStaticBoxSizer (m_colorsBox, wxVERTICAL);
    m_scaleCheck = new wxCheckBox (this, -1, _("Zoom display 200%"));
    m_scaleCheck->SetValue (m_scale2);
    m_prefItems = new wxBoxSizer (wxVERTICAL);
    m_prefButtons = new wxBoxSizer (wxHORIZONTAL);
    m_dialogContent = new wxBoxSizer (wxVERTICAL);
      
    m_fgSizer->Add (m_fgButton, 0, wxALL, 5);
    m_fgSizer->Add (m_fgLabel, 0, wxALL, 5);
    m_bgSizer->Add (m_bgButton, 0, wxALL, 5);
    m_bgSizer->Add (m_bgLabel, 0, wxALL, 5);
    m_colorsSizer->Add (m_fgSizer);
    m_colorsSizer->Add (m_bgSizer);
    m_prefItems->Add (m_scaleCheck, 0, wxALL, 5);
    m_prefItems->Add (m_colorsSizer, 0, wxALL, 10);
    m_prefButtons->Add (m_okButton, 0, wxALL, 5);
    m_prefButtons->Add (m_cancelButton, 0, wxALL, 5);
    m_prefButtons->Add (m_resetButton, 0, wxALL, 5);
    m_dialogContent->Add (m_prefItems, 0, wxTOP | wxLEFT | wxRIGHT, 5);
    m_dialogContent->Add (m_prefButtons, 0, wxALL, 10);
}

void PtermPrefdialog::OnButton (wxCommandEvent& event)
{
    wxColour fgcol, bgcol;
    
    if (event.m_eventObject == m_fgButton)
    {
        m_fgColor = m_owner->SelectColor (m_fgColor);
    }
    else if (event.m_eventObject == m_bgButton)
    {
        m_bgColor = m_owner->SelectColor (m_bgColor);
    }
    
    else if (event.m_eventObject == m_okButton)
    {
        EndModal (wxID_OK);
    }
    else if (event.m_eventObject == m_cancelButton)
    {
        EndModal (wxID_CANCEL);
    }
    else if (event.m_eventObject == m_resetButton)
    {
        m_fgColor = wxColour (255, 144, 0);
        m_bgColor = *wxBLACK;
        m_scale2 = FALSE;
        m_scaleCheck->SetValue (FALSE);
    }
    paintBitmap (m_fgBitmap, m_fgColor, m_fgButton);
    paintBitmap (m_bgBitmap, m_bgColor, m_bgButton);
}

void PtermPrefdialog::OnCheckbox (wxCommandEvent& event)
{
    m_scale2 = event.IsChecked ();
}

void PtermPrefdialog::paintBitmap (wxBitmap *bm, wxColour &color, wxBitmapButton *to)
{
    wxBrush bitmapBrush (color, wxSOLID);
    wxMemoryDC memDC;

    memDC.SelectObject (*bm);
    memDC.BeginDrawing ();
    memDC.SetBackground (bitmapBrush);
    memDC.Clear ();
    memDC.EndDrawing ();
    memDC.SetBackground (wxNullBrush);
    memDC.SelectObject (wxNullBitmap);
    if (to != NULL)
    {
        to->Refresh ();
    }
}


// ----------------------------------------------------------------------------
// PtermThread
// ----------------------------------------------------------------------------

PtermThread::ExitCode PtermThread::Entry (void)
{
    u32 niuwd;
    int i, j, k;
    int next;

    while (emulationActive)
    {
        i = dtRead (&fet, -1);
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
            niuRing[niuIn] = niuwd;
#ifdef DEBUG
//          wxLogMessage ("data from plato %07o", niuwd);
#endif
            niuIn = next;
            i = niuRingCount;
            if (i == RINGXOFF1 || i == RINGXOFF2)
            {
#if 0
                ptermSendKey (xofkey);
#endif
            }
        }
        if (niuIn != niuOut)
        {
            // Send a do-nothing event to the frame; that will wake up the main
            // thread and cause it to process the words we buffered.
            wxWakeUpIdle ();
        }
    }
    return (ExitCode) 0;
}

void PtermFrame::ptermDrawChar (int x, int y, int snum, int cnum)
{
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    PrepareDC (dc);
    m_memDC->SetPen (m_foregroundPen);
    m_memDC->SetBackground (m_backgroundBrush);
    drawChar (dc, x, y, snum, cnum);
    dc.EndDrawing ();
}

void PtermFrame::ptermDrawPoint (int x, int y)
{
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    PrepareDC (dc);
    x = XADJUST (x);
    y = YADJUST (y);
    if (wemode & 1)
    {
        // mode rewrite or write
        dc.SetPen (m_foregroundPen);
        m_memDC->SetPen (m_foregroundPen);
    }
    else
    {
        // mode inverse or erase
        dc.SetPen (m_backgroundPen);
        m_memDC->SetPen (m_backgroundPen);
    }
    dc.DrawPoint (x, y);
    m_memDC->SetBackground (m_backgroundBrush);
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
}

void PtermFrame::ptermDrawLine(int x1, int y1, int x2, int y2)
{
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
        m_memDC->SetPen (m_foregroundPen);
    }
    else
    {
        // mode inverse or erase
        dc.SetPen (m_backgroundPen);
        m_memDC->SetPen (m_backgroundPen);
    }
    dc.DrawLine (x1, y1, x2, y2);
    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->DrawLine (x1, y1, x2, y2);
    dc.EndDrawing ();
}

void PtermFrame::ptermFullErase (void)
{
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    PrepareDC (dc);
    dc.Clear ();
    dc.EndDrawing ();
    m_memDC->BeginDrawing ();
    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->Clear ();
    m_memDC->EndDrawing ();
}

void PtermFrame::ptermSetName (const char *winName)
{
}

void PtermFrame::ptermSetStatus (const char *str)
{
    wxString wxstr (str, *wxConvCurrent);
    
    SetStatusText(wxstr, STATUS_CONN);
}

void PtermCanvas::ptermTouchPanel(bool enable)
{
    m_touchEnabled = enable;
    if (enable)
    {
        SetCursor (wxCursor (wxCURSOR_HAND));
    }
    else
    {
        SetCursor (wxNullCursor);
    }
}

void PtermFrame::ptermLoadChar (int snum, int cnum, const u16 *chardata)
{
    int i, j, m;
    int x, y;
    u16 col;
    const u16 *data;
    
    for (m = 0; m < 5; m++)
    {
        m_charDC[m]->BeginDrawing ();
        data = chardata;
        x = cnum * 8 * scale;
        y = (snum * 16 + 15) * scale;
        for (i = 0; i < 8; i++)
        {
            col = *data++;
            for (j = 0; j < 16; j++)
            {
                if (col & 1)
                {
                    // drawing char pixel
                    switch (m)
                    {
                    case 0:
                        m_charDC[m]->SetPen (m_backgroundPen);
                        break;
                    case 1:
                        m_charDC[m]->SetPen (m_foregroundPen);
                        break;
                    case 2:
                        m_charDC[m]->SetPen (m_backgroundPen);
                        break;
                    case 3:
                        m_charDC[m]->SetPen (m_foregroundPen);
                        break;
                    case 4:
                        m_charDC[m]->SetPen (*wxBLACK_PEN);
                        break;
                    }
                }
                else
                {
                    // drawing background pixel
                    switch (m)
                    {
                    case 0:
                        m_charDC[m]->SetPen (m_foregroundPen);
                        break;
                    case 1:
                        m_charDC[m]->SetPen (m_backgroundPen);
                        break;
                    case 2:
                        m_charDC[m]->SetPen (*wxBLACK_PEN);
                        break;
                    case 3:
                        m_charDC[m]->SetPen (*wxBLACK_PEN);
                        break;
                    case 4:
                        m_charDC[m]->SetPen (*wxWHITE_PEN);
                        break;
                    }
                }
                m_charDC[m]->DrawPoint(x, y - j * scale);
                if (scale == 2)
                {
                    m_charDC[m]->DrawPoint(x + 1, y - j * scale);
                    m_charDC[m]->DrawPoint(x, y - j * scale + 1);
                    m_charDC[m]->DrawPoint(x + 1, y - j * scale + 1);
                }
                col >>= 1;
            }
            x += scale;
        }
        m_charDC[m]->EndDrawing ();
    }
}

void PtermFrame::ptermLoadRomChars (void)
{
    int i;

    for (i = 0; i < 5; i++)
    {
        m_charDC[i]->SetBackground (*wxBLACK_BRUSH);
        m_charDC[i]->Clear ();
    }

    for (i = 0; i < sizeof (plato_m0) / (sizeof (plato_m0[0]) * 8); i++)
    {
        ptermLoadChar (0, i, plato_m0 + (i * 8));
        ptermLoadChar (1, i, plato_m1 + (i * 8));
    }
}

void PtermFrame::ptermSetCharColors (wxColour &fg, wxColour &bg)
{
    wxBrush fgBrush;
    wxString rgb;
    
    fgBrush.SetColour (fg);
    if (ptermApp->m_fgColor == fg &&
        ptermApp->m_bgColor == bg)
    {
        return;
    }
    rgb.Printf (wxT ("%d %d %d"), (int) fg.Red (), (int) fg.Green (), (int) fg.Blue ());
    ptermApp->m_config->Write (wxT ("foreground"), rgb);
    rgb.Printf (wxT ("%d %d %d"), (int) bg.Red (), (int) bg.Green (), (int) bg.Blue ());
    ptermApp->m_config->Write (wxT ("background"), rgb);
    ptermApp->m_config->Flush ();
    
    ptermApp->m_fgColor = fg;
    ptermApp->m_bgColor = bg;
    m_backgroundBrush.SetColour (bg);
    m_backgroundPen.SetColour (bg);
    m_foregroundPen.SetColour (fg);
    
    m_charDC[2]->SetBackground (m_backgroundBrush);
    m_charDC[2]->Clear ();
    m_charDC[2]->Blit (0, 0, XSize, CharYSize, m_charDC[4], 0, 0, wxAND_INVERT);
    m_charDC[3]->SetBackground (fgBrush);
    m_charDC[3]->Clear ();
    m_charDC[3]->Blit (0, 0, XSize, CharYSize, m_charDC[4], 0, 0, wxAND_INVERT);
    m_charDC[0]->SetBackground (fgBrush);
    m_charDC[0]->Clear ();
    m_charDC[0]->Blit (0, 0, XSize, CharYSize, m_charDC[4], 0, 0, wxAND);
    m_charDC[0]->Blit (0, 0, XSize, CharYSize, m_charDC[2], 0, 0, wxOR);
    m_charDC[1]->SetBackground (m_backgroundBrush);
    m_charDC[1]->Clear ();
    m_charDC[1]->Blit (0, 0, XSize, CharYSize, m_charDC[4], 0, 0, wxAND);
    m_charDC[1]->Blit (0, 0, XSize, CharYSize, m_charDC[3], 0, 0, wxOR);
}

void PtermFrame::ptermSetSize (bool scale2)
{
    // TBD: actually change the window size on the spot.
    // for now, just save it for next start.
    ptermApp->m_config->Write (wxT ("scale"), (scale2) ? 2 : 1);
    ptermApp->m_config->Flush ();
}

void PtermFrame::drawChar (wxDC &dc, int x, int y, int snum, int cnum)
{
    int charX, charY, sizeX, sizeY, screenX, screenY;

    charX = cnum * 8 * scale;
    charY = snum * 16 * scale;
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
    if (wemode & 2)
    {
        // write or erase -- need to zap old pixels and OR in new pixels
        m_memDC->Blit (screenX, screenY, sizeX * scale, sizeY * scale, 
                       m_charDC[4], charX, charY, wxAND);
        m_memDC->Blit (screenX, screenY, sizeX * scale, sizeY * scale, 
                       m_charDC[wemode], charX, charY, wxOR);
    }
    else
    {
        // inverse or rewrite, just blit in the appropriate pattern
        m_memDC->Blit (screenX, screenY, sizeX * scale, sizeY * scale, 
                       m_charDC[wemode], charX, charY, wxCOPY);
    }
        
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

// ----------------------------------------------------------------------------
// PtermCanvas
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them.
BEGIN_EVENT_TABLE(PtermCanvas, wxScrolledWindow)
    EVT_CHAR(PtermCanvas::OnChar)
    EVT_KEY_DOWN(PtermCanvas::OnKeyDown)
    EVT_LEFT_DOWN(PtermCanvas::OnMouseDown)
    EVT_IDLE(PtermCanvas::OnIdle)
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
    dc.Blit (0, 0, XSize, YSize, m_owner->m_memDC, 0, 0, wxCOPY);
#if 0
    int i;
    
    for (i = 0; i < 5; i++)
        dc.Blit (0, i * CharYSize, XSize, YSize, m_owner->m_charDC[i], 0, 0, wxCOPY);
#endif
}

void PtermCanvas::OnKeyDown (wxKeyEvent &event)
{
    // We have to handle F10 as a key-down event, not a char event,
    // otherwise MSWindows sends it to the File menu item...
    if (event.m_keyCode == WXK_F10)
    {
        ptermSendKey ((event.m_shiftDown) ? 072 : 032);
    }
    else
    {
        event.Skip ();
    }
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

#if 0
    if (tracePterm)
    {
        /*fprintf (traceF,*/printf( "ctrl %d shift %d alt %d key %d\n", event.m_controlDown, event.m_shiftDown, event.m_altDown, key);
    }
#endif

    // **** TEMP **** workaround for 2.5.5 bug
    if (ctrl && (key == ']' || isalpha (key)))
    {
        key &= 037;
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

    pc |= shift;
    ptermSendKey (pc);
}

void PtermCanvas::OnMouseDown (wxMouseEvent &event)
{
    int x, y;
    
    if (!m_touchEnabled)
    {
        return;
    }

    x = XUNADJUST (event.m_x);
    y = YUNADJUST (event.m_y);
    if (x < 0 || x > 511 ||
        y < 0 || y > 511)
    {
        return;
    }
    x /= 32;
    y /= 32;

    ptermSendKey (0x100 | (x << 4) | y);
}

void PtermCanvas::OnIdle (wxIdleEvent& event)
{
    int i, j, next;
    
    for (;;)
    {
        /*
        **  Process words until we hit some point that causes output delay.
        **  That way we see an abort code as soon as possible.
        */
        if (niuIn == niuOut)
        {
            event.Skip ();
            break;
        }
        next = niuOut + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
#ifdef DEBUG
            wxLogMessage ("processing data from plato %07o", niuRing[niuOut]);
#endif
        j = procNiuWord (niuRing[niuOut]);
        niuOut = next;
        i = niuRingCount;
        if (i == RINGXON1 || i == RINGXON2)
        {
#if 0
            ptermSendKey (xonkey);
#endif
        }
        if (j)
        {
            event.Skip ();
            break;
        }
    }
    ptermApp->Yield ();
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
#ifdef DEBUG
            wxLogMessage ("key to plato %03o", key);
#endif
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

void ptermSetStation (const char *unused_hostname, int station)
    {
    char name[100];
    
    if (hostName != NULL)
        {
        sprintf (name, " %s %d-%d",
                 hostName, station >> 5, station & 31);
        }
    else
        {
        sprintf (name, " %d-%d",
                 station >> 5, station & 31);
         }
    pterm_frame->ptermSetStatus (name);
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
    pterm_frame->m_canvas->ptermTouchPanel (enable);
}

void ptermShowTrace (bool enable)
{
    if (enable)
    {
        pterm_frame->SetStatusText(_T(" TRACE "), STATUS_TRC);
    }
    else
    {
        pterm_frame->SetStatusText(_T(""), STATUS_TRC);
    }
}

