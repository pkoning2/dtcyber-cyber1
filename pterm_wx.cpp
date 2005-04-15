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

//#define PPT

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

#define C_NODATA        -1
#define C_CONNFAIL      -2
#define C_DISCONNECT    -3

#define STATUS_TIP      0
#define STATUS_TRC      1
#define STATUS_CONN     2
#define STATUSPANES     3

#define KeyBufSize      50
#define DisplayMargin   8

#define CSETS           8       // leave room for the PPT multiple sets

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for the character sets (ROM and loadable)
#define XSize           ((512 + 2 * DisplayMargin) * ptermApp->m_scale)
#define YSize           ((512 + 2 * DisplayMargin) * ptermApp->m_scale)
#define CharYSize       ((CSETS * 16) * ptermApp->m_scale)
#define ScreenSize      (512 * ptermApp->m_scale)

#ifdef PPT
#define TERMTYPE 9
#else
#define TERMTYPE 0
#endif

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define TRACEN(str)                                             \
    if (tracePterm)                                             \
        {                                                       \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc);  \
        }

#define TRACE(str, arg)                                             \
    if (tracePterm)                                                 \
        {                                                           \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg); \
        }

#define TRACE2(str, arg, arg2)                                          \
    if (tracePterm)                                                     \
        {                                                               \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg, arg2); \
        }

#define TRACE3(str, arg, arg2, arg3)                                    \
    if (tracePterm)                                                     \
        {                                                               \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg, arg2, arg3); \
        }

#define TRACE4(str, a, a2, a3, a4)                                      \
    if (tracePterm)                                                     \
        {                                                               \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, a, a2, a3, a4); \
        }

#define TRACE6(str, a, a2, a3, a4, a5, a6)                              \
    if (tracePterm)                                                     \
        {                                                               \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, a, a2, a3, a4, a5, a6); \
        }

#define XADJUST(x) (((x) + DisplayMargin) * ptermApp->m_scale)
#define YADJUST(y) (YSize - (DisplayMargin + 1 + (y)) * ptermApp->m_scale)

// inverse mapping (for processing touch input)
#define XUNADJUST(x) ((x / ptermApp->m_scale) - DisplayMargin)
#define YUNADJUST(y) ((YSize - (y)) / ptermApp->m_scale - 1 - DisplayMargin)

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
#if defined(_WIN32)
#include <winsock.h>
#include <process.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#endif
#include "const.h"
#include "types.h"
#include "proto.h"
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

bool emulationActive = FALSE;

// ----------------------------------------------------------------------------
// local variables
// ----------------------------------------------------------------------------

class PtermApp;
static PtermApp *ptermApp;

static FILE *traceF;
static char traceFn[20];

#ifdef DEBUG
static wxLogWindow *logwindow;
#endif

static const char rom0char[] =
    ":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.÷[]%×«'\"!;<>+?»U";

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

class PtermFrame;
class PtermCanvas;

// Pterm processing thread
class PtermConnection : public wxThread
{
public:
    // override base class virtuals
    // ----------------------------
    PtermConnection (PtermFrame *owner, const char *host, int port);
    ~PtermConnection ();
    
    virtual ExitCode Entry (void);

    int NextWord (void);
    void SendData (const void *data, int len);

    bool IsEmpty (void) const
    {
        return (m_displayIn == m_displayOut);
    }
    bool IsFull (void) const
    {
        int next;
    
        next = m_displayIn + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
        return (next == m_displayOut);
    }
    int RingCount (void) const
    {
        if (m_displayIn >= m_displayOut)
        {
            return m_displayIn - m_displayOut;
        }
        else
        {
            return RINGSIZE + m_displayIn - m_displayOut;
        }
    }
    
private:
    NetFet      m_fet;
    u32         m_displayRing[RINGSIZE];
    volatile int m_displayIn, m_displayOut;
    PtermFrame  *m_owner;
    char        *m_hostName;
    int         m_port;

    void StoreWord (int word);

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

    // event handlers
    void OnConnect (wxCommandEvent &event);

    wxColour    m_fgColor;
    wxColour    m_bgColor;
    wxConfig    *m_config;
    wxClipboard *m_clipboard;

    char        *m_hostName;
    int         m_port;
    // scale is 1 or 2 for full size and double, respectively.
    int         m_scale;
    bool        m_classicSpeed;
    
private:

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// Define a new frame type: this is going to be our main frame
class PtermFrame : public wxFrame
{
//friend class PtermCanvas;

public:
    // ctor(s)
    PtermFrame(const char *host, int port, const wxString& title);
    ~PtermFrame ();

    // event handlers (these functions should _not_ be virtual)
    void OnIdle (wxIdleEvent& event);
    void OnClose (wxCloseEvent& event);
    void OnTimer (wxTimerEvent& event);
    void OnQuit (wxCommandEvent& event);
    void OnAbout (wxCommandEvent& event);
    void OnPref (wxCommandEvent& event);
    void OnCopyScreen (wxCommandEvent &event);

    wxColour SelectColor (wxColour &initcol);

    void PrepareDC(wxDC& dc);
    void ptermSendKey(int key);
    void ptermSetTrace (bool fileaction);

    wxMemoryDC  *m_memDC;
    bool        tracePterm;

private:
    PtermConnection *m_conn;
    wxBrush     m_backgroundBrush;
    wxBitmap    *m_bitmap;
    wxPen       m_foregroundPen;
    wxPen       m_backgroundPen;
    PtermCanvas *m_canvas;
    char        *m_hostName;
    int         m_port;
    wxTimer     m_timer;
    
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

    // The next word to be processed, and its associated delay.
    // We set this if we pick up a word from the connection object, and
    // either it comes with an associated delay, or "true timing" is selected
    // via preferences
    int         m_nextword;
    int         m_delay;
    
    // PLATO terminal emulation state
    int         mode;
    int         wemode;
    int         currentX;
    int         currentY;
    int         margin;
    int         memaddr;
    u16         plato_m23[128 * 8];
    int         memlpc;
    int         currentCharset;
    bool        uncover;
    int         wc;
    int         seq;

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
    
    void drawChar (wxDC &dc, int x, int y, int snum, int cnum);
    int procPlatoWord (u32 d);
    void plotChar (int c);
    void mode0 (u32 d);
    void mode1 (u32 d);
    void mode2 (u32 d);
    void mode3 (u32 d);
    void mode4 (u32 d);
    void mode5 (u32 d);
    void mode6 (u32 d);
    void mode7 (u32 d);
    void ptermSetStation (int station);
    void ptermShowTrace (bool enable);
    
    typedef void (PtermFrame::*mptr)(u32);

    static const mptr modePtr[8];

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

const PtermFrame::mptr PtermFrame::modePtr[8] = 
{
    &PtermFrame::mode0, &PtermFrame::mode1,
    &PtermFrame::mode2, &PtermFrame::mode3,
    &PtermFrame::mode4, &PtermFrame::mode5,
    &PtermFrame::mode6, &PtermFrame::mode7
};

// define a scrollable canvas for drawing onto
class PtermCanvas: public wxScrolledWindow
{
public:
    PtermCanvas (PtermFrame *parent);

    void ptermTouchPanel(bool enable);

    void OnDraw (wxDC &dc);
    void OnKeyDown (wxKeyEvent& event);
    void OnChar (wxKeyEvent& event);
    void OnMouseDown (wxMouseEvent &event);
    
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
    wxCheckBox      *m_speedCheck;
    wxBoxSizer      *m_prefItems;
    wxBoxSizer      *m_prefButtons;
    wxBoxSizer      *m_dialogContent;

    wxColour        m_fgColor;
    wxColour        m_bgColor;
    bool            m_scale2;
    bool            m_classicSpeed;
    
private:
    void paintBitmap (wxBitmap *bm, wxColour &color, wxBitmapButton *to);
    
    PtermFrame *m_owner;

    DECLARE_EVENT_TABLE ()
};

// define the preferences dialog
class PtermConndialog : public wxDialog
{
public:
    PtermConndialog (wxWindowID id, const wxString &title);
    
    void OnButton (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    wxButton        *m_okButton;
    wxButton        *m_cancelButton;
    wxTextCtrl      *m_hostText;
    wxTextCtrl      *m_portText;
    wxTextValidator *m_hostVal;
    wxTextValidator *m_portVal;
    wxStaticText    *m_hostLabel;
    wxStaticText    *m_portLabel;
    wxFlexGridSizer *m_connItems;
    wxBoxSizer      *m_connButtons;
    wxBoxSizer      *m_dialogContent;
    
    wxString m_host;
    wxString m_port;
    
private:
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
    Pterm_ConnectAgain,

    Pterm_Connect = wxID_NEW,
    Pterm_Quit = wxID_EXIT,
    Pterm_Close = wxID_CLOSE,
    Pterm_Pref = wxID_PREFERENCES,

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
    EVT_IDLE(PtermFrame::OnIdle)
    EVT_CLOSE(PtermFrame::OnClose)
    EVT_TIMER(wxID_ANY,   PtermFrame::OnTimer)
    EVT_MENU(Pterm_Close, PtermFrame::OnQuit)
    EVT_MENU(Pterm_About, PtermFrame::OnAbout)
    EVT_MENU(Pterm_Pref,  PtermFrame::OnPref)
    EVT_MENU(Pterm_CopyScreen, PtermFrame::OnCopyScreen)
    END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(PtermApp, wxApp)
    EVT_MENU(Pterm_Connect,  PtermApp::OnConnect)
    EVT_MENU(Pterm_ConnectAgain, PtermApp::OnConnect)
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
    int r, g, b;
    wxString rgb;
    PtermFrame *frame;
    
    ptermApp = this;

#ifdef DEBUG
    logwindow = new wxLogWindow (NULL, "pterm log", TRUE, FALSE);
#endif

    if (argc > 3)
    {
        printf ("usage: pterm [ hostname [ portnum ]]\n");
        exit (1);
    }
    if (argc > 2)
    {
        m_port = atoi (wxString (argv[2]).mb_str ());
    }
    else
    {
        m_port = DefNiuPort;
    }
    if (argc > 1)
    {
        m_hostName = strdup (wxString (argv[1]).mb_str ());
    }
    else
    {
        m_hostName = strdup (DEFAULTHOST);
    }

    m_config = new wxConfig (wxT ("Pterm"));
    // 255 144 0 is RGB for Plato Orange
    m_config->Read (wxT ("foreground"), &rgb, wxT ("255 144 0"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_fgColor = wxColour (r, g, b);
    m_config->Read (wxT ("background"), &rgb, wxT ("0 0 0"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_bgColor = wxColour (r, g, b);
    m_scale = m_config->Read (wxT ("scale"), 1);
    if (m_scale != 1 && m_scale != 2)
    {
        m_scale = 1;
    }
    m_classicSpeed = (m_config->Read (wxT ("classicSpeed"), 0L) != 0);

    // create the main application window
    frame = new PtermFrame(m_hostName, m_port, wxT("Pterm " PTERMVERSION));
    
    // create the clipboard object
    m_clipboard = new wxClipboard;

    sprintf (traceFn, "pterm%d.trc", getpid ());

    // success: wxApp::OnRun () will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
}

int PtermApp::OnExit (void)
{
    m_clipboard->Flush ();
    delete m_clipboard;

    free (m_hostName);
    
#ifdef DEBUG
    delete logwindow;
#endif

    return 0;
}

void PtermApp::OnConnect (wxCommandEvent &event)
{
    PtermFrame *frame;
    if (event.GetId () == Pterm_Connect)
    {
        PtermConndialog dlg (wxID_ANY, _("Connect to PLATO"));
    
        if (dlg.ShowModal () == wxID_OK)
        {
            free (m_hostName);
            m_hostName = strdup (wxString (dlg.m_host).mb_str ());
            m_port = atoi (wxString (dlg.m_port).mb_str ());
        }
        else
        {
            return;     // connect canceled
        }
    }
    

    // create the main application window
    frame = new PtermFrame(m_hostName, m_port, wxT("Pterm " PTERMVERSION));
}


// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
PtermFrame::PtermFrame(const char *host, int port, const wxString& title)
    : wxFrame(NULL, -1, title,
              wxDefaultPosition,
              wxDefaultSize),
      m_foregroundPen (ptermApp->m_fgColor, ptermApp->m_scale, wxSOLID),
      m_backgroundPen (ptermApp->m_bgColor, ptermApp->m_scale, wxSOLID),
      m_backgroundBrush (ptermApp->m_bgColor, wxSOLID),
      m_timer (this),
      tracePterm (FALSE),
      m_port (port),
      m_nextword (0),
      m_delay (0),
      mode (0),
      wemode (0),
      currentX (0),
      currentY (0),
      margin (0),
      memaddr (0),
      memlpc (0),
      currentCharset (0),
      uncover(FALSE),
      wc (0),
      seq (0)
{
    int i;

    m_hostName = strdup (host);
    
    // set the frame icon
    SetIcon(wxICON(pterm_32));

#if wxUSE_MENUS
    // create a menu bar
    //
    // Note that the menu bar labels do not have shortcut markings,
    // because those conflict with the ALT-letter key codes for PLATO.
    wxMenu *menuFile = new wxMenu;

    menuFile->Append (Pterm_Connect, _T("&New Connection..."),
                      _T("Connect to a PLATO host"));
    menuFile->Append (Pterm_ConnectAgain, _T("Connect &Again"),
                      _T("Connect to the same host"));
    menuFile->Append (Pterm_Pref, _T("P&references..."),
                      _T("Set program configuration"));
    menuFile->Append (Pterm_Close, _T("&Close\tCtrl-Z"), _T("Close this window"));
//    menuFile->AppendSeparator();
//    menuFile->Append (Pterm_Quit, _T("E&xit"), _T("Quit this program"));

    wxMenu *menuEdit = new wxMenu;

    menuEdit->Append (Pterm_CopyScreen, _T("Copy Screen"), _T("Copy screen to clipboard"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append(menuFile, _T("File"));
    menuBar->Append(menuEdit, _T("Edit"));

#if !defined(__WXMAC__) 
    // the "About" item should be in the help menu, except on the Mac
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Pterm_About, _T("&About..."), _T("Show about dialog"));
    menuBar->Append(helpMenu, _T("Help"));
#else
    // On the Mac, it will appear in the application menu.  Pretend it's on
    // the File menu.  It won't show up there, but by doing this rather than
    // putting it on the Help menu, we avoid an empty "Help" entry on the
    // Mac menubar.
    // If we add any other Help items, this hack will not be needed anymore.
    menuFile->Append(Pterm_About, _T("&About..."), _T("Show about dialog"));
#endif

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun
    CreateStatusBar(STATUSPANES);
    SetStatusText(_T(" Connecting..."), STATUS_CONN);
#endif // wxUSE_STATUSBAR

    for (i = 0; i < 5; i++)
    {
        m_charDC[i] = new wxMemoryDC ();
        m_charmap[i] = new wxBitmap (XSize, CharYSize, -1);
        m_charDC[i]->SelectObject (*m_charmap[i]);
    }
    m_bitmap = new wxBitmap (XSize, YSize, -1);
    m_memDC = new wxMemoryDC ();
    m_memDC->BeginDrawing ();
    m_memDC->SelectObject (*m_bitmap);
    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->Clear ();
    m_memDC->SetClippingRegion (XADJUST (0), YADJUST (511), ScreenSize, ScreenSize);
    m_memDC->EndDrawing ();

    SetClientSize (XSize+2, YSize+2);
    m_canvas = new PtermCanvas (this);

    /*
    **  Load Plato ROM characters
    */
    ptermLoadRomChars ();
    
    // Create and start the network processing thread
    m_conn = new PtermConnection (this, host, port);
    if (m_conn->Create () != wxTHREAD_NO_ERROR)
    {
        return;
    }
    m_conn->Run ();

    Show(TRUE);
}

PtermFrame::~PtermFrame ()
{
    int i;
    
    delete m_conn;
    delete m_memDC;
    delete m_bitmap;
    for (i = 0; i < 5; i++)
    {
        delete m_charDC[i];
        delete m_charmap[i];
    }
    
    free (m_hostName);
}

// event handlers

void PtermFrame::OnIdle (wxIdleEvent& event)
{
    int word;
    
    // If our timer is running, we're using the timer event to drive
    // the display, so ignore idle events.
    if (m_timer.IsRunning ())
    {
        event.Skip ();
        return;
    }
    
    if (m_nextword != 0)
    {
        procPlatoWord (m_nextword);
        m_nextword = 0;
    }
    
    for (;;)
    {
        /*
        **  Process words until there is nothing left.
        */
        word = m_conn->NextWord ();
        
        if (word == C_NODATA)
        {
            event.Skip ();
            break;
        }

        // See if we're supposed to delay
        if (word >> 19)
        {
            m_delay = word >> 19;
            m_nextword = word & 01777777;
            m_timer.Start (17);
            event.Skip ();
            return;
        }
            
#ifdef DEBUG
        wxLogMessage ("processing data from plato %07o", word);
#endif
        procPlatoWord (word);
    }

    if (word != C_NODATA)
    {
        event.RequestMore ();
    }
}

void PtermFrame::OnTimer (wxTimerEvent &WXUNUSED(event))
{
    int word;
    
    if (--m_delay > 0)
    {
        return;
    }
    procPlatoWord (m_nextword);
    
    word = m_conn->NextWord ();
    
    if (word == C_NODATA)
    {
        m_nextword = 0;
        m_timer.Stop ();
        return;
    }
    m_delay = (word >> 19);
    m_nextword = word & 01777777;
    if (m_delay == 0)
    {
        m_timer.Stop ();
        wxWakeUpIdle ();
    }
}


void PtermFrame::OnClose (wxCloseEvent &WXUNUSED(event))
{
    int i;
    
    m_conn->Delete ();

    m_memDC->BeginDrawing ();
    m_memDC->SetBackground (wxNullBrush);
    m_memDC->SetPen (wxNullPen);
    m_memDC->EndDrawing ();
    m_memDC->SelectObject (wxNullBitmap);

    for (i = 0; i < 5; i++)
    {
        m_charDC[i]->BeginDrawing ();
        m_charDC[i]->SetBackground (wxNullBrush);
        m_charDC[i]->SetPen (wxNullPen);
        m_charDC[i]->EndDrawing ();
        m_charDC[i]->SelectObject (wxNullBitmap);
    }

    Destroy ();
}

void PtermFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
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
    
    if (dlg.ShowModal () == wxID_OK)
    {
        ptermSetCharColors (dlg.m_fgColor, dlg.m_bgColor);
        ptermSetSize (dlg.m_scale2);
        ptermApp->m_classicSpeed = dlg.m_classicSpeed;
        ptermApp->m_config->Write (wxT ("classicSpeed"), (dlg.m_classicSpeed) ? 1 : 0);
        ptermApp->m_config->Flush ();
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
    if (ptermApp->m_scale == 2)
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
        x = cnum * 8 * ptermApp->m_scale;
        y = (snum * 16 + 15) * ptermApp->m_scale;
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
                m_charDC[m]->DrawPoint(x, y - j * ptermApp->m_scale);
                if (ptermApp->m_scale == 2)
                {
                    m_charDC[m]->DrawPoint(x + 1, y - j * ptermApp->m_scale);
                    m_charDC[m]->DrawPoint(x, y - j * ptermApp->m_scale + 1);
                    m_charDC[m]->DrawPoint(x + 1, y - j * ptermApp->m_scale + 1);
                }
                col >>= 1;
            }
            x += ptermApp->m_scale;
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
}

void PtermFrame::drawChar (wxDC &dc, int x, int y, int snum, int cnum)
{
    int charX, charY, sizeX, sizeY, screenX, screenY;

    charX = cnum * 8 * ptermApp->m_scale;
    charY = snum * 16 * ptermApp->m_scale;
    sizeX = 8;
    sizeY = 16;
    screenX = XADJUST (x);
    screenY = YADJUST (y) - 15 * ptermApp->m_scale;
    
    if (x < 0)
    {
        sizeX += x;
        charX -= x * ptermApp->m_scale;
        screenX = XADJUST (0);
    }
    if (y < 0)
    {
        sizeY += y;
    }
    if (wemode & 2)
    {
        // write or erase -- need to zap old pixels and OR in new pixels
        m_memDC->Blit (screenX, screenY,
                       sizeX * ptermApp->m_scale, sizeY * ptermApp->m_scale, 
                       m_charDC[4], charX, charY, wxAND);
        m_memDC->Blit (screenX, screenY,
                       sizeX * ptermApp->m_scale, sizeY * ptermApp->m_scale, 
                       m_charDC[wemode], charX, charY, wxOR);
    }
    else
    {
        // inverse or rewrite, just blit in the appropriate pattern
        m_memDC->Blit (screenX, screenY,
                       sizeX * ptermApp->m_scale, sizeY * ptermApp->m_scale, 
                       m_charDC[wemode], charX, charY, wxCOPY);
    }
        
    // Now copy the resulting state of the character area into the screen dc
    dc.Blit (screenX, screenY,
             sizeX * ptermApp->m_scale, sizeY * ptermApp->m_scale,
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
**  Purpose:        Process word of PLATO output data
**
**  Parameters:     Name        Description.
**                  d           19-bit word
**
**  Returns:        1 if we did a delay, 0 if not.
**
**------------------------------------------------------------------------*/
int PtermFrame::procPlatoWord (u32 d)
{
    mptr mp;
    char *msg = "";
    int status = 0;
    
    seq++;
    if (tracePterm)
    {
        fprintf (traceF, "%07o ", d);
    }
	if ((d & 01700000) == 0)
    {
        // NOP command...
#ifdef PPT
        if (d & 1)
        {
            wc = (wc + 1) & 0177;
        }
#endif
    }
    else
    {
        wc = (wc + 1) & 0177;
    }
    
    if (d & 01000000)
    {
        mp = modePtr[mode];
        (this->*mp) (d);
    }
    else
    {
        switch ((d >> 15) & 7)
        {
        case 0:     // nop
            if ((d & 077000) == 042000)
            {
                // Special code to tell pterm the station number
                d &= 0777;
                ptermSetStation (d);
            }
            else if (d == 1)
            {
#if 0
#if defined(_WIN32)
                Sleep(1000 / 60);
#else
                usleep(1000000 / 60);
#endif
                status = 1;
#endif
            }
            
            TRACEN ("nop");
            break;

        case 1:     // load mode
            if ((d & 020000) != 0)
            {
                // load wc bit is set
                wc = (d >> 6) & 0177;
            }
            
            wemode = (d >> 1) & 3;
#ifdef PPT
            mode = (d >> 3) & 7;
#else
            mode = (d >> 3) & 3;
            if (wemode == 0)
            {
                wemode = 2;
            }
#endif
            if (d & 1)
            {
                // full screen erase
                ptermFullErase ();
            }

            // bit 15 set is DISable
            m_canvas->ptermTouchPanel ((d & 040000) == 0);
            TRACE4 ("load mode %d inhibit %d wemode %d screen %d",
                    mode, (d >> 14) & 1, wemode, (d & 1));
            break;
            
        case 2:     // load coordinate
            if (d & 01000)
            {
                currentY = d & 0777;
            }
            else
            {
                currentX = d & 0777;
#ifdef PPT
                if (d & 010000)
                {
                    margin = currentX;
                    msg = "margin";
                }
#endif
            }
            TRACE3 ("load coord %c %d %s",
                    (d & 01000) ? 'Y' : 'X', d & 0777, msg);
            break;
        case 3:     // echo
            // 160 is terminal type query
            if ((d & 0177) == 0160)
            {
                TRACE ("load echo termtype %d", TERMTYPE);
                ptermSendKey (0360 + TERMTYPE);
                break;
            }
            TRACE ("load echo %d", d & 0177);
            ptermSendKey ((d & 0177) + 0200);
            break;
            
        case 4:     // load address
#ifdef PPT
            memaddr = d & 077777;
#else
            memaddr = d & 01777;
#endif
            TRACE ("load address %o", memaddr);
            break;
            
#ifdef PPT
        case 5:     // SSF on PPT, Load Slide on PLATO IV
            switch ((d >> 10) & 037)
            {
#if 0
            case 1: // Touch panel control ?
                TRACE ("ssf touch %o", d);
                m_canvas->ptermTouchPanel ((d & 040) != 0);
                break;
#endif
            default:
                TRACE ("ssf %o", d);
                break;  // ignore
            }
            break;
#endif

        case 6:
        case 7:
            d &= 0177777;
            TRACE ("Ext %07o", d);
            // Do -ext- processing
            break;

        default:    // ignore
            TRACE ("ignored command word %07o", d);
            break;
        }
    }
    return status;
}

/*--------------------------------------------------------------------------
**  Purpose:        Display visual indication of trace status
**
**  Parameters:     Name        Description.
**                  fileaction  TRUE to open/flush files,
**                              FALSE if only redisplay is needed
**
**  Returns:        nothing
**
**  This function opens or flushes the trace file depending on trace
**  setting, if fileaction is TRUE.  It then redraws (on or off) the trace
**  indication.
**
**------------------------------------------------------------------------*/
void PtermFrame::ptermSetTrace (bool fileaction)
{
    if (!tracePterm)
    {
        if (fileaction)
        {
            fflush (traceF);
        }
    }
    else
    {
        if (fileaction && (traceF == NULL))
        {
            traceF = fopen (traceFn, "w");
        }
    }
    ptermShowTrace (tracePterm);
}

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Draw one character
**
**  Parameters:     Name        Description.
**                  c           Character code
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::plotChar (int c)
{
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
#ifdef PPT
            // tbd: PPT only uncover codes, all the higher ones.
#endif
        default:
            break;
        }
    }
    else
    {
        ptermDrawChar (currentX, currentY, currentCharset, c);
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
void PtermFrame::mode0 (u32 d)
{
    int x, y;
    
    x = (d >> 9) & 0777;
    y = d & 0777;
    TRACE2 ("dot %d %d", x, y);
    ptermDrawPoint (x, y);
    currentX = x;
    currentY = y;
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
void PtermFrame::mode1 (u32 d)
{
    int x, y;
    
    x = (d >> 9) & 0777;
    y = d & 0777;
    TRACE2 ("lineto %d %d", x, y);
    ptermDrawLine (currentX, currentY, x, y);
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
void PtermFrame::mode2 (u32 d)
{
    int ch;
    
    if (memaddr >= 127 * 8)
    {
        TRACEN ("memdata, address out of range");
        return;
    }
    if (((d >> 16) & 3) == 0)
    {
        // load data
        TRACE2 ("memdata %06o to %04o", d & 0xffff, memaddr);
        plato_m23[memaddr] = d & 0xffff;
        if ((++memaddr & 7) == 0)
        {
            // character is done -- load it to display 
            ch = (memaddr / 8) - 1;
            ptermLoadChar (2 + (ch / 64), ch % 64, &plato_m23[memaddr - 8]);
        }
    }
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
void PtermFrame::mode3 (u32 d)
{
    TRACE6 ("char %02o %02o %02o (%c%c%c)",
            (d >> 12) & 077, (d >> 6) & 077, d & 077,
            rom0char[(d >> 12) & 077], 
            rom0char[(d >> 6) & 077], 
            rom0char[d & 077]);
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
void PtermFrame::mode4 (u32 d)
{
    TRACE ("mode4 %06o", d);
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
void PtermFrame::mode5 (u32 d)
{
    TRACE ("mode5 %06o", d);
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
void PtermFrame::mode6 (u32 d)
{
    TRACE ("mode6 %06o", d);
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
void PtermFrame::mode7 (u32 d)
{
    TRACE ("mode7 %06o", d);
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
void PtermFrame::ptermSendKey(int key)
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
        m_conn->SendData (data, 2);
    }
}

void PtermFrame::ptermSetStation (int station)
{
    char name[100];
    
    if (m_hostName != NULL)
    {
        sprintf (name, " %s %d-%d",
                 m_hostName, station >> 5, station & 31);
    }
    else
    {
        sprintf (name, " %d-%d",
                 station >> 5, station & 31);
    }
    ptermSetStatus (name);
}

void PtermFrame::ptermShowTrace (bool enable)
{
    if (enable)
    {
        SetStatusText(_T(" TRACE "), STATUS_TRC);
    }
    else
    {
        SetStatusText(_T(""), STATUS_TRC);
    }
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
    m_scale2 = (ptermApp->m_scale != 1);
    m_classicSpeed = ptermApp->m_classicSpeed;
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
    m_speedCheck = new wxCheckBox (this, -1, _("Simulate 1200 Baud"));
    m_speedCheck->SetValue (m_classicSpeed);
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
    m_prefItems->Add (m_speedCheck, 0, wxALL, 5);
    m_prefItems->Add (m_colorsSizer, 0, wxALL, 10);
    m_prefButtons->Add (m_okButton, 0, wxALL, 5);
    m_prefButtons->Add (m_cancelButton, 0, wxALL, 5);
    m_prefButtons->Add (m_resetButton, 0, wxALL, 5);
    m_dialogContent->Add (m_prefItems, 0, wxTOP | wxLEFT | wxRIGHT, 5);
    m_dialogContent->Add (m_prefButtons, 0, wxALL, 10);

    SetSizer (m_dialogContent);
    m_dialogContent->Fit (this);
    m_okButton->SetDefault ();
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
        m_classicSpeed = FALSE;
        m_speedCheck->SetValue (FALSE);
    }
    paintBitmap (m_fgBitmap, m_fgColor, m_fgButton);
    paintBitmap (m_bgBitmap, m_bgColor, m_bgButton);
}

void PtermPrefdialog::OnCheckbox (wxCommandEvent& event)
{
    if (event.m_eventObject == m_scaleCheck)
    {
        m_scale2 = event.IsChecked ();
    }
    else if (event.m_eventObject == m_speedCheck)
    {
        m_classicSpeed = event.IsChecked ();
    }
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
// PtermConndialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(PtermConndialog, wxDialog)
    EVT_CLOSE(PtermConndialog::OnClose)
    EVT_BUTTON(wxID_ANY,   PtermConndialog::OnButton)
    END_EVENT_TABLE()

PtermConndialog::PtermConndialog (wxWindowID id, const wxString &title)
    : wxDialog (NULL, id, title)
{
    m_host = wxString (ptermApp->m_hostName);
    m_port.Printf ("%d", ptermApp->m_port);

    m_okButton = new wxButton (this, wxID_ANY, _("OK"));
    m_cancelButton = new wxButton (this, wxID_ANY, _("Cancel"));

    m_hostLabel = new wxStaticText (this, wxID_ANY, _("Host name"));
    m_portLabel = new wxStaticText (this, wxID_ANY, _("Port"));

    m_hostVal = new wxTextValidator (wxFILTER_ASCII, &m_host);
    m_portVal = new wxTextValidator (wxFILTER_NUMERIC, &m_port);
    
    m_hostText = new wxTextCtrl (this, wxID_ANY, m_host,
                                 wxDefaultPosition, wxSize (160, 18),
                                 0, *m_hostVal);
    m_portText = new wxTextCtrl (this, wxID_ANY, m_port,
                                 wxDefaultPosition, wxSize (160, 18),
                                 0, *m_portVal);
    m_connItems = new wxFlexGridSizer (2, 2, 5, 5);
    m_connButtons = new wxBoxSizer (wxHORIZONTAL);
    m_dialogContent = new wxBoxSizer (wxVERTICAL);
      

    m_connItems->Add (m_hostLabel);
    m_connItems->Add (m_hostText);
    m_connItems->Add (m_portLabel);
    m_connItems->Add (m_portText);
    m_connButtons->Add (m_okButton, 0, wxALL, 5);
    m_connButtons->Add (m_cancelButton, 0, wxALL, 5);
    m_dialogContent->Add (m_connItems, 0, wxTOP | wxLEFT | wxRIGHT, 10);
    m_dialogContent->Add (m_connButtons, 0, wxALL, 10);

    SetSizer (m_dialogContent);
    m_dialogContent->Fit (this);
    m_okButton->SetDefault ();
}

void PtermConndialog::OnButton (wxCommandEvent& event)
{
    if (event.m_eventObject == m_okButton)
    {
        m_host = m_hostText->GetLineText (0);
        m_port = m_portText->GetLineText (0);
        EndModal (wxID_OK);
    }
    else
    {
        EndModal (wxID_CANCEL);
    }
}

// ----------------------------------------------------------------------------
// PtermConnection
// ----------------------------------------------------------------------------

PtermConnection::PtermConnection (PtermFrame *owner, const char *host, int port)
    : wxThread (wxTHREAD_JOINABLE),
      m_owner (owner),
      m_port (port),
      m_displayIn (0),
      m_displayOut (0)
{
    m_hostName = strdup (host);
}

PtermConnection::~PtermConnection ()
{
    free (m_hostName);
}

PtermConnection::ExitCode PtermConnection::Entry (void)
{
    u32 platowd;
    int i, j, k;
    bool wasEmpty;
    
    int true_opt = 1;
    
    dtInitFet (&m_fet, BufSiz);
    if (dtConnect (&m_fet.connFd, m_hostName, m_port) < 0)
    {
        StoreWord (C_CONNFAIL);
        wxWakeUpIdle ();
        return (ExitCode) 1;
    }
    setsockopt (m_fet.connFd, SOL_SOCKET, SO_KEEPALIVE,
                (char *)&true_opt, sizeof(true_opt));
#ifdef __APPLE__
    setsockopt (m_fet.connFd, SOL_SOCKET, SO_NOSIGPIPE,
                (char *)&true_opt, sizeof(true_opt));
#endif

    while (TRUE)
    {
        i = dtRead (&m_fet, 200);
        if (i < 0 || TestDestroy ())
        {
            if (i < 0)
            {
                StoreWord (C_DISCONNECT);
            }
            break;
        }
        
        wasEmpty = IsEmpty ();
        
        for (;;)
        {
            /*
            **  Assemble words from the network buffer, all the
            **  while looking for "abort output" codes (word == 2).
            */
            if (IsFull ())
            {
                break;
            }
            if (dtFetData (&m_fet) < 3)
            {
                break;
            }
            i = dtReado (&m_fet);
            if (i & 0200)
            {
                printf ("Plato output out of sync byte 0: %03o\n", i);
                continue;
            }
    newj:
            j = dtReado (&m_fet);
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
            k = dtReado (&m_fet);
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
            platowd = (i << 12) | ((j & 077) << 6) | (k & 077);
            StoreWord (platowd);
            i = RingCount ();
            if (i == RINGXOFF1 || i == RINGXOFF2)
            {
#if 0
                ptermSendKey (xofkey);
#endif
            }
        }
        if (wasEmpty && !IsEmpty ())
        {
            // Send a do-nothing event to the frame; that will wake up the main
            // thread and cause it to process the words we buffered.
            wxWakeUpIdle ();
        }
    }

    return (ExitCode) 0;
}

int PtermConnection::NextWord (void)
{
    int i, j, next;
    int delay = 0;
    
    if (m_displayIn == m_displayOut)
    {
        return C_NODATA;
    }
    next = m_displayOut + 1;
    if (next == RINGSIZE)
    {
        next = 0;
    }
    j = m_displayRing[m_displayOut];

    // See if emulating 1200 baud, or the -delay- NOP code
    if (ptermApp->m_classicSpeed || j == 1)
    {
        delay = 1;
    }
    
    // Pass the delay to the caller
    j |= (delay << 19);
    
    m_displayOut = next;

    if (j == C_CONNFAIL || j == C_DISCONNECT)
    {
        wxString msg;
            
        m_owner->SetStatusText(wxString (_T(" Not connected"), STATUS_CONN));
        if (j == C_CONNFAIL)
        {
            msg.Printf (_T("Failed to connect to %s %d"), m_hostName, m_port);
        }
        else
        {
            msg.Printf (_T("Connection lost to %s %d"), m_hostName, m_port);
        }
            
        wxMessageDialog alert (m_owner, msg, wxString (_T("Alert")), wxOK);
            
        alert.ShowModal ();
        m_owner->Close (TRUE);
        j = C_NODATA;
    }
        
    return j;
}

void PtermConnection::StoreWord (int word)
{
    int next;
    
    next = m_displayIn + 1;
    if (next == RINGSIZE)
    {
        next = 0;
    }
    if (next == m_displayOut)
    {
        return;
    }
    m_displayRing[m_displayIn] = word;
#ifdef DEBUG
//    wxLogMessage ("data from plato %07o", word);
#endif
    m_displayIn = next;
}

void PtermConnection::SendData (const void *data, int len)
{
    send(m_fet.connFd, data, len, 0);
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
    END_EVENT_TABLE ()

PtermCanvas::PtermCanvas(PtermFrame *parent)
    : wxScrolledWindow(parent, -1, wxDefaultPosition, 
                       wxSize (XSize, YSize),
                       wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxClientDC dc(this);

    m_owner = parent;
    SetVirtualSize (XSize, YSize);
    
    SetScrollRate (1, 1);
    dc.SetClippingRegion (XADJUST (0), YADJUST (511), ScreenSize, ScreenSize);
    SetFocus ();
}

void PtermCanvas::OnDraw(wxDC &dc)
{
    m_owner->PrepareDC (dc);
    dc.Clear ();
    dc.Blit (0, 0, XSize, YSize, m_owner->m_memDC, 0, 0, wxCOPY);
}

void PtermCanvas::OnKeyDown (wxKeyEvent &event)
{
    int key;
    int shift = 0;
    int pc = -1;
    bool ctrl;

    // Most keyboard inputs are handled here, because if we defer them to
    // the EVT_CHAR stage, too many evil things happen in too many of the
    // platforms, all different...
    //
    // The one thing we do defer until EVT_CHAR is plain old characters, i.e.,
    // keystrokes that aren't function keys or other special keys, and
    // neither Ctrl nor Alt are active.

    ctrl = event.m_controlDown;
    if (event.m_shiftDown)
    {
        shift = 040;
    }
    key = event.m_keyCode;
    if (isalpha (key))
    {
        key = tolower (key);
    }

#if 0
    if (tracePterm)
    {
        /*fprintf (traceF,*/printf( "ctrl %d shift %d alt %d key %d\n", event.m_controlDown, event.m_shiftDown, event.m_altDown, key);
    }
#endif

    if (ctrl && key == ']')         // control-] : trace
    {
        m_owner->tracePterm = !m_owner->tracePterm;
        m_owner->ptermSetTrace (TRUE);
        return;
    }

    // Special case: ALT-left is assignment arrow
    if (event.m_altDown && key == WXK_LEFT)
    {
        m_owner->ptermSendKey (015 | shift);
        return;
    }

    if (key < sizeof (asciiToPlato) / sizeof (asciiToPlato[0]))
    {
        if (event.m_altDown)
        {
            pc = altKeyToPlato[key];
            
            if (pc >= 0)
            {
                m_owner->ptermSendKey (pc | shift);
                return;
            }
            else
            {
                event.Skip ();
                return;
            }
        }
        else if (ctrl && key >= 040)
        {
            // Control key is active.  There are several possibilities:
            //
            // Control plus letter: look up the translate table entry
            //  for the matching control code.
            // Control plus non-letter: look up the translate table entry
            //  for that character, and set the "shift" bit.
            // Control plus control code or function key: don't pay attention
            //  to the control key being active, and don't do a table lookup
            //  on the keycode.  Instead, those are handled later, in a switch.

            if (isalpha (key))
            {
                // Control letter -- do a lookup for the matching control code.
                pc = asciiToPlato[key & 037];
            }
            else
            {
                // control but not a letter or ASCII control code -- 
                // translate to what a PLATO keyboard
                // would have on the shifted position for that key
                pc = asciiToPlato[key];
                shift = 040;
            }

            if (pc >= 0)
            {
                m_owner->ptermSendKey (pc | shift);
                return;
            }
        }
    }

    // If we get down to this point, then the following is true:
    // 1. It wasn't an ALT-key entry.
    // 2. If Control was active, it was with a non-printable key (where we
    //    ignore the Control flag).
    // At this point, we're going to look for function keys.  If it isn't
    // one of those, then it was a regular printable character code, possibly
    // shifted.  Those are handled in the EVT_CHAR handler because the
    // unshifted to shifted translation is keyboard specific (at least for
    // non-letters) and we want to let the system deal with that.

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
    case WXK_ESCAPE:
        pc = 015;       // assign
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

    m_owner->ptermSendKey (pc | shift);
}

void PtermCanvas::OnChar(wxKeyEvent& event)
{
    int key;
    int shift = 0;
    int pc = -1;

    // control and alt codes shouldn't come here, they are handled in KEY_DOWN
    if (event.m_controlDown || event.m_altDown)
    {
        event.Skip ();
        return;
    }
    if (event.m_shiftDown)
    {
        shift = 040;
    }
    key = event.m_keyCode;

    if (key < sizeof (asciiToPlato) / sizeof (asciiToPlato[0]))
    {
        pc = asciiToPlato[key];
        if (pc >= 0)
        {
            m_owner->ptermSendKey (pc);
            return;
        }
    }
    event.Skip ();
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

    m_owner->ptermSendKey (0x100 | (x << 4) | y);
}

/*---------------------------  End Of File  ------------------------------*/
