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

#define DEFAULTHOST wxT ("cyberserv.org")
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

#define M2ADDR          0x2340  // PPT start address for set 2
#define M3ADDR          0x2740  // PPT start address for set 3

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for the character sets (ROM and loadable)
#define vXSize(s)       ((512 + 2 * DisplayMargin) * (s))
#define vYSize(s)       ((512 + 2 * DisplayMargin) * (s))
#define vCharXSize(s)   (512 * (s))
#define vCharYSize(s)   ((CSETS * 16) * (s))
#define vScreenSize(s)  (512 * (s))
#define XSize           (vYSize (ptermApp->m_scale))
#define YSize           (vXSize (ptermApp->m_scale))
#define CharXSize       (vCharXSize (ptermApp->m_scale))
#define CharYSize       (vCharYSize (ptermApp->m_scale))
#define ScreenSize      (vScreenSize (ptermApp->m_scale))

#define TERMTYPE 10

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
#include "wx/image.h"

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

extern int ptermOpenGsw (void *user);
extern int ptermProcGswData (int data);
extern void ptermCloseGsw (void);
#if defined (__WXGTK__)

// Attempting to include the gtk.h file yields infinite compile errors, so
// instead we just declare the two functions we need, leaving any 
// issues of data structures unstated.

struct GtkSettings;
    
extern void gtk_settings_set_string_property (GtkSettings *, const char *,
                                              const char *, const char *);
extern GtkSettings * gtk_settings_get_default (void);
#endif
}
    
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
    PtermConnection (PtermFrame *owner, wxString &host, int port);
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
    wxString    m_hostName;
    int         m_port;
    wxCriticalSection m_pointerLock;
    bool        m_gswActive;
    int         m_savedWord;
    int         m_savedGswMode;
    
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
    void OnPref (wxCommandEvent& event);
    void OnQuit (wxCommandEvent& event);

    bool DoConnect (bool ask);
    static wxColour SelectColor (wxColour &initcol);
    
    wxColour    m_fgColor;
    wxColour    m_bgColor;
    wxConfig    *m_config;

    wxString    m_hostName;
    int         m_port;
    // scale is 1 or 2 for full size and double, respectively.
    int         m_scale;
    bool        m_classicSpeed;
    bool        m_connect;
    bool        m_gswEnable;
    PtermFrame  *m_firstFrame;
    
private:
    wxLocale    m_locale; // locale we'll be using
    
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// Define a new frame type: this is going to be our main frame
class PtermFrame : public wxFrame
{
//friend class PtermCanvas;

public:
    // ctor(s)
    PtermFrame(wxString &host, int port, const wxString& title);
    ~PtermFrame ();

    // event handlers (these functions should _not_ be virtual)
    void OnIdle (wxIdleEvent& event);
    void OnClose (wxCloseEvent& event);
    void OnTimer (wxTimerEvent& event);
    void OnQuit (wxCommandEvent& event);
    void OnAbout (wxCommandEvent& event);
    void OnCopyScreen (wxCommandEvent &event);
//#if !defined(__WXMAC__)
    void OnActivate (wxActivateEvent &event);
//#endif
    void UpdateSettings (wxColour &newfg, wxColour &newbf, bool newscale2);
    
    void PrepareDC(wxDC& dc);
    void ptermSendKey(int key);
    void ptermSetTrace (bool fileaction);

    wxMemoryDC  *m_memDC;
    bool        tracePterm;
    PtermFrame  *m_nextFrame;
    PtermFrame  *m_prevFrame;

private:
    PtermConnection *m_conn;
    wxBrush     m_backgroundBrush;
    wxBrush     m_foregroundBrush;
    wxBitmap    *m_bitmap;
    wxPen       m_foregroundPen;
    wxPen       m_backgroundPen;
    PtermCanvas *m_canvas;
    wxString    m_hostName;
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
    // via preferences, or GSW emulation is active.
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
    bool        vertical;
    bool        reverse;
    bool        large;
    int         wc;
    int         seq;
    int         modewords;
    int         mode4start;

    void UpdateDC (wxMemoryDC *dc, wxBitmap *&bitmap,
                   wxColour &newfg, wxColour &newbf, bool newscale2);

    // PLATO drawing primitives
    void ptermDrawChar (int x, int y, int snum, int cnum);
    void ptermDrawPoint (int x, int y);
    void ptermDrawLine(int x1, int y1, int x2, int y2);
    void ptermFullErase (void);
    void ptermBlockErase (int x1, int y1, int x2, int y2);
    void ptermSetName (wxString &winName);
    void ptermSetStatus (wxString &str);
    void ptermLoadChar (int snum, int cnum, const u16 *data);
    void ptermLoadRomChars (void);
    
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
    wxCheckBox      *m_autoConnect;
    wxCheckBox      *m_gswCheck;
    wxTextCtrl      *m_hostText;
    wxTextCtrl      *m_portText;
    wxTextValidator *m_hostVal;
    wxTextValidator *m_portVal;
    wxStaticText    *m_hostLabel;
    wxStaticText    *m_portLabel;
    wxFlexGridSizer *m_connItems;
    wxStaticBox     *m_connBox;
    wxStaticBoxSizer *m_connSizer;
    wxBoxSizer      *m_prefItems;
    wxBoxSizer      *m_prefButtons;
    wxBoxSizer      *m_dialogContent;

    wxColour        m_fgColor;
    wxColour        m_bgColor;
    bool            m_scale2;
    bool            m_classicSpeed;
    bool            m_connect;
    bool            m_gswEnable;
    wxString        m_host;
    wxString        m_port;
    
private:
    void paintBitmap (wxBitmap *bm, wxColour &color);
    
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
    
    wxString        m_host;
    wxString        m_port;
    
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
//#if !defined(__WXMAC__)
    EVT_ACTIVATE(PtermFrame::OnActivate)
//#endif
    EVT_MENU(Pterm_Close, PtermFrame::OnQuit)
    EVT_MENU(Pterm_About, PtermFrame::OnAbout)
    EVT_MENU(Pterm_CopyScreen, PtermFrame::OnCopyScreen)
    END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(PtermApp, wxApp)
    EVT_MENU(Pterm_Connect, PtermApp::OnConnect)
    EVT_MENU(Pterm_ConnectAgain, PtermApp::OnConnect)
    EVT_MENU(Pterm_Pref,    PtermApp::OnPref)
    EVT_MENU(Pterm_Quit,	PtermApp::OnQuit)
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
    
    ptermApp = this;
    m_firstFrame = NULL;
    
    m_locale.Init(wxLANGUAGE_DEFAULT);
    m_locale.AddCatalog(wxT("pterm"));

#ifdef DEBUG
    logwindow = new wxLogWindow (NULL, "pterm log", TRUE, FALSE);
#endif

    if (argc > 3)
    {
        printf ("usage: pterm [ hostname [ portnum ]]\n");
        exit (1);
    }

    m_config = new wxConfig (wxT ("Pterm"));

    if (argc > 2)
    {
        m_port = atoi (wxString (argv[2]).mb_str ());
    }
    else
    {
        m_port = m_config->Read (wxT ("port"), DefNiuPort);
    }
    if (argc > 1)
    {
        m_hostName = argv[1];
    }
    else
    {
        m_config->Read (wxT ("host"), &m_hostName, DEFAULTHOST);
    }

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
    m_connect = (m_config->Read (wxT ("autoconnect"), 1) != 0);
    m_gswEnable = (m_config->Read (wxT ("gswenable"), 1) != 0);
    
    // create the main application window
    // If arguments are present, always connect without asking
    if (!DoConnect (!(m_connect || argc > 1)))
    {
        return FALSE;
    }
    
    // GTK seems to require this for copying bitmaps to the clipboard
    wxImage::AddHandler (new wxPNGHandler);
    
    sprintf (traceFn, "pterm%d.trc", getpid ());

    // success: wxApp::OnRun () will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
}

int PtermApp::OnExit (void)
{
    wxTheClipboard->Flush ();

#ifdef DEBUG
    delete logwindow;
#endif

    return 0;
}

void PtermApp::OnConnect (wxCommandEvent &event)
{
    DoConnect (event.GetId () == Pterm_Connect);
}

bool PtermApp::DoConnect (bool ask)
{
    PtermFrame *frame;

    if (ask)
    {
        PtermConndialog dlg (wxID_ANY, _("Connect to PLATO"));
    
        dlg.CenterOnScreen ();
        
        if (dlg.ShowModal () == wxID_OK)
        {
            m_hostName = dlg.m_host;
            m_port = atoi (wxString (dlg.m_port).mb_str ());
        }
        else
        {
            return FALSE;     // connect canceled
        }
    }
    

    // create the main application window
    frame = new PtermFrame(m_hostName, m_port, wxT("Pterm"));

    if (frame != NULL)
    {
        if (m_firstFrame != NULL)
        {
            m_firstFrame->m_prevFrame = frame;
        }
        frame->m_nextFrame = m_firstFrame;
        m_firstFrame = frame;
    }
    
    return (frame != NULL);
}

void PtermApp::OnPref (wxCommandEvent&)
{
    PtermFrame *frame;
    wxString rgb;
    
    PtermPrefdialog dlg (NULL, wxID_ANY, _("Pterm Preferences"));
    
    if (dlg.ShowModal () == wxID_OK)
    {
        for (frame = m_firstFrame; frame != NULL; frame = frame->m_nextFrame)
        {
            frame->UpdateSettings (dlg.m_fgColor, dlg.m_bgColor, dlg.m_scale2);
        }

        m_fgColor = dlg.m_fgColor;
        m_bgColor = dlg.m_bgColor;
        
        m_scale = (dlg.m_scale2) ? 2 : 1;
        m_classicSpeed = dlg.m_classicSpeed;
        m_gswEnable = dlg.m_gswEnable;
        m_hostName = dlg.m_host;
        m_port = atoi (wxString (dlg.m_port).mb_str ());
        m_connect = dlg.m_connect;
        
        rgb.Printf (wxT ("%d %d %d"), 
                    dlg.m_fgColor.Red (), dlg.m_fgColor.Green (),
                    dlg.m_fgColor.Blue ());
        m_config->Write (wxT ("foreground"), rgb);
        rgb.Printf (wxT ("%d %d %d"),
                    dlg.m_bgColor.Red (), dlg.m_bgColor.Green (),
                    dlg.m_bgColor.Blue ());
        m_config->Write (wxT ("background"), rgb);
        m_config->Write (wxT ("scale"), (dlg.m_scale2) ? 2 : 1);
        m_config->Write (wxT ("host"), dlg.m_host);
        m_config->Write (wxT ("port"), m_port);
        m_config->Write (wxT ("classicSpeed"), (dlg.m_classicSpeed) ? 1 : 0);
        m_config->Write (wxT ("autoConnect"), (dlg.m_connect) ? 1 : 0);
        m_config->Write (wxT ("gswenable"), (dlg.m_gswEnable) ? 1 : 0);
        m_config->Flush ();
    }
}

wxColour PtermApp::SelectColor (wxColour &initcol)
{
    wxColour col (initcol);
    wxColour orange (255, 144, 0);
    wxColourData data;

    data.SetColour (initcol);
    data.SetCustomColour (0, orange);
    data.SetCustomColour (1, *wxBLACK);
    
    wxColourDialog dialog (NULL, &data);

    if (dialog.ShowModal () == wxID_OK)
    {
        col = dialog.GetColourData ().GetColour ();
    }

    return col;
}

void PtermApp::OnQuit(wxCommandEvent&)
{
    PtermFrame *frame, *nextframe;

    frame = m_firstFrame;
    while (frame != NULL)
    {
        nextframe = frame->m_nextFrame;
        frame->Close (TRUE);
        frame = nextframe;
    }
}


// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
PtermFrame::PtermFrame(wxString &host, int port, const wxString& title)
    : wxFrame(NULL, -1, title,
              wxDefaultPosition,
              wxDefaultSize),
      m_foregroundPen (ptermApp->m_fgColor, ptermApp->m_scale, wxSOLID),
      m_backgroundPen (ptermApp->m_bgColor, ptermApp->m_scale, wxSOLID),
      m_foregroundBrush (ptermApp->m_fgColor, wxSOLID),
      m_backgroundBrush (ptermApp->m_bgColor, wxSOLID),
      m_nextFrame (NULL),
      m_prevFrame (NULL),
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
      uncover (FALSE),
      vertical (FALSE),
      reverse (FALSE),
      large (FALSE),
      wc (0),
      seq (0),
      modewords (0)
{
    int i;

    m_hostName = host;
    
    // set the frame icon
    SetIcon(wxICON(pterm_32));

    // set the line style to no cap
    m_foregroundPen.SetCap (wxCAP_BUTT);
    m_backgroundPen.SetCap (wxCAP_BUTT);

#if wxUSE_MENUS
    // create a menu bar
    //
    // Note that the menu bar labels do not have shortcut markings,
    // because those conflict with the ALT-letter key codes for PLATO.
#if defined(__WXGTK__)
    // A rather ugly hack here.  GTK insists that F10 should be the
    // accelerator for the menu bar.  We don't want that.  There is
    // no sane way to turn this off, but we *can* get the same effect
    // by setting the "menu bar accelerator" property to the name of a
    // function key that is apparently legal, but doesn't really exist.
    // (Or if it does, it certainly isn't a key we use.)
    gtk_settings_set_string_property (gtk_settings_get_default (),
                                      "gtk-menu-bar-accel", "F15", "foo");

#endif

    wxMenu *menuFile = new wxMenu;
    menuFile->Append (Pterm_Connect, _("&New Connection...\tCtrl-N"),
                      _("Connect to a PLATO host"));
    menuFile->Append (Pterm_ConnectAgain, _("Connect &Again"),
                      _("Connect to the same host"));
    menuFile->AppendSeparator();
    menuFile->Append (Pterm_Pref, _("P&references..."),
                      _("Set program configuration"));
    menuFile->AppendSeparator();
    menuFile->Append (Pterm_Close, _("&Close\tCtrl-Z"), _("Close this window"));
    menuFile->Append (Pterm_Quit, _("E&xit"), _("Quit this program"));

    wxMenu *menuEdit = new wxMenu;

    menuEdit->Append (Pterm_CopyScreen, _("Copy Screen"), _("Copy screen to clipboard"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append(menuFile, _("File"));
    menuBar->Append(menuEdit, _("Edit"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Pterm_About, _("&About..."), _("Show about dialog"));
#if defined(__WXMAC__)
    // On the Mac the menu name has to be exactly "&Help" for the About item
    // to  be recognized.  Ugh.
    menuBar->Append(helpMenu, wxT("&Help"));
#else
    menuBar->Append(helpMenu, _("Help"));
#endif

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun
    CreateStatusBar(STATUSPANES);
    SetStatusText(_(" Connecting..."), STATUS_CONN);
#endif // wxUSE_STATUSBAR
    SetCursor (*wxHOURGLASS_CURSOR);

    for (i = 0; i < 5; i++)
    {
        m_charDC[i] = new wxMemoryDC ();
        m_charmap[i] = new wxBitmap (CharXSize, CharYSize, -1);
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

    SetClientSize (XSize + 2, YSize + 2);
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
    
    // Remove this frame from the app's frame list
    if (m_nextFrame != NULL)
    {
        m_nextFrame->m_prevFrame = m_prevFrame;
    }
    if (m_prevFrame != NULL)
    {
        m_prevFrame->m_nextFrame = m_nextFrame;
    }
    else
    {
        ptermApp->m_firstFrame = m_nextFrame;
    }
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

void PtermFrame::OnTimer (wxTimerEvent &)
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


void PtermFrame::OnClose (wxCloseEvent &)
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

void PtermFrame::OnQuit(wxCommandEvent&)
{
    // TRUE is to force the frame to close
    Close (TRUE);
}

//#if !defined(__WXMAC__)
void PtermFrame::OnActivate (wxActivateEvent &event)
{
    if (m_canvas != NULL)
    {
        m_canvas->SetFocus ();
    }
    event.Skip ();        // let others see the event, too
}
//#endif

void PtermFrame::OnAbout(wxCommandEvent&)
{
    wxString msg;

    msg.Printf (_T("PLATO terminal emulator %s.\n%s"),
                wxT (PTERMVERSION),
                _("Copyright © 2005 by Paul Koning."));
    
    wxMessageBox(msg, _("About Pterm"), wxOK | wxICON_INFORMATION, this);
}

void PtermFrame::OnCopyScreen (wxCommandEvent &)
{
    wxBitmap screenmap (ScreenSize, ScreenSize);
    wxMemoryDC screenDC;
    wxBitmapDataObject *screen;

    screenDC.SelectObject (screenmap);
    screenDC.Blit (0, 0, ScreenSize, ScreenSize, 
                   m_memDC, XADJUST (0), YADJUST (511), wxCOPY);
    screenDC.SelectObject (wxNullBitmap);

    screen = new wxBitmapDataObject(screenmap);

    if (wxTheClipboard->Open ())
    {
        wxTheClipboard->SetData (screen);
        wxTheClipboard->Close ();
    }
}

void PtermFrame::PrepareDC(wxDC& dc)
{
    dc.SetAxisOrientation (TRUE, FALSE);
    dc.SetBackground (m_backgroundBrush);
}

void PtermFrame::ptermDrawChar (int x, int y, int snum, int cnum)
{
    int &cx = (vertical) ? y : x;
    int &cy = (vertical) ? x : y;
    int i, j, saveY, savemode, dx, dy, sdy;
    const u16 *charp;
    u16 charw;
    wxClientDC dc(m_canvas);

    if (!vertical && !large)
    {
        dc.BeginDrawing ();
        m_memDC->BeginDrawing ();
        PrepareDC (dc);
        m_memDC->SetPen (m_foregroundPen);
        m_memDC->SetBackground (m_backgroundBrush);
        drawChar (dc, x, y, snum, cnum);
        dc.EndDrawing ();
        m_memDC->EndDrawing ();
    }
    else
    {
        // vertical or large drawing we do dot by dot
        x = currentX;
        y = currentY;
        saveY = cy;
        dx = dy = (large) ? 2 : 1;
        sdy = 1;
        if (vertical)
        {
            sdy = -1;
            dy = -dy;
        }
        if (snum == 0)
        {
            charp = plato_m0;
        }
        else if (snum == 1)
        {
            charp = plato_m1;
        }
        else
        {
            charp = plato_m23 + (snum - 2) * (8 * 64);
        }
        charp += 8 * cnum;
        savemode = wemode;
        
        for (j = 0; j < 8; j++)
        {
            cy = saveY;
            charw = *charp++;
            for (i = 0; i < 16; i++)
            {
                if ((charw & 1) == 0)
                {
                    // background, do we erase it?
                    if (savemode & 2)
                    {
                        charw >>= 1;
                        cy += dy;
                        continue;
                    }
                    wemode = savemode ^ 1;
                }
                else
                {
                    wemode = savemode;
                }
                ptermDrawPoint (x, y);
                if (large)
                {
                    ptermDrawPoint (x + 1, y);
                    ptermDrawPoint (x, y + sdy);
                    ptermDrawPoint (x + 1, y + sdy);
                }
                charw >>= 1;
                cy += dy;
            }
            cx += dx;
        }
        wemode = savemode;
    }
}

void PtermFrame::ptermDrawPoint (int x, int y)
{
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    m_memDC->BeginDrawing ();
    PrepareDC (dc);
    x = XADJUST (x & 0777);
    y = YADJUST (y & 0777);
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
//    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->DrawPoint (x, y);
    if (ptermApp->m_scale == 2)
    {
        dc.DrawPoint (x + 1, y);
        m_memDC->DrawPoint (x + 1, y);
        dc.DrawPoint (x, y - 1);
        m_memDC->DrawPoint (x, y - 1);
        dc.DrawPoint (x + 1, y - 1);
        m_memDC->DrawPoint (x + 1, y - 1);
    }    
    dc.EndDrawing ();
    m_memDC->EndDrawing ();
}

void PtermFrame::ptermDrawLine(int x1, int y1, int x2, int y2)
{
    wxClientDC dc(m_canvas);

#if 0   // this still isn't right...
    // On Windows, sometimes the starting point is missed.
    // In any case, we have to draw the endpoint, since the
    // book says that isn't drawn by DrawLine.
    ptermDrawPoint (x1, y1);
    ptermDrawPoint (x2, y2);
#endif
    dc.BeginDrawing ();
    m_memDC->BeginDrawing ();
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
//    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->DrawLine (x1, y1, x2, y2);
    dc.EndDrawing ();
    m_memDC->EndDrawing ();
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

void PtermFrame::ptermBlockErase (int x1, int y1, int x2, int y2)
{
    int t;
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    m_memDC->BeginDrawing ();
    PrepareDC (dc);
    x1 = XADJUST (x1);
    y1 = YADJUST (y1);
    x2 = XADJUST (x2);
    y2 = YADJUST (y2);
    if (x1 > x2)
    {
        t = x1;
        x1 = x2;
        x2 = t;
    }
    if (y1 > y2)
    {
        t = y1;
        y1 = y2;
        y2 = t;
    }
    if (wemode & 1)
    {
        // mode rewrite or write
        dc.SetPen (m_foregroundPen);
        m_memDC->SetPen (m_foregroundPen);
        dc.SetBrush (m_foregroundBrush);
        m_memDC->SetBrush (m_foregroundBrush);
    }
    else
    {
        // mode inverse or erase
        dc.SetPen (m_backgroundPen);
        m_memDC->SetPen (m_backgroundPen);
        dc.SetBrush (m_backgroundBrush);
        m_memDC->SetBrush (m_backgroundBrush);
    }
    dc.DrawRectangle (x1, y1,
                      x2 - x1 + ptermApp->m_scale,
                      y2 - y1 + ptermApp->m_scale);
    m_memDC->DrawRectangle (x1, y1,
                            x2 - x1 + ptermApp->m_scale,
                            y2 - y1 + ptermApp->m_scale);
    dc.EndDrawing ();
    m_memDC->EndDrawing ();
}

void PtermFrame::ptermSetName (wxString &winName)
{
    wxString str;
    
    str.Printf (wxT("Pterm: %s"), winName.c_str ());
    SetTitle (str);
}

void PtermFrame::ptermSetStatus (wxString &str)
{
    SetStatusText(str, STATUS_CONN);
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

void PtermFrame::UpdateSettings (wxColour &newfg, wxColour &newbg, bool newscale2)
{
    const bool recolor = (ptermApp->m_fgColor != newfg ||
                          ptermApp->m_bgColor != newbg);
    const int newscale = (newscale2) ? 2 : 1;
    const bool rescale = (newscale != ptermApp->m_scale);
    int i;
    wxBitmap *newmap;
    
    if (!recolor && !rescale)
    {
        return;
    }

    wxClientDC dc(m_canvas);

    UpdateDC (m_memDC, m_bitmap, newfg, newbg, newscale2);
    if (rescale)
    {
        SetClientSize (vXSize (newscale) + 2, vYSize (newscale) + 2);
        m_canvas->SetSize (vXSize (newscale), vYSize (newscale));
        m_canvas->SetVirtualSize (vXSize (newscale), vYSize (newscale));
        m_canvas->SetScrollRate (1, 1);
        dc.BeginDrawing ();
        dc.DestroyClippingRegion ();
        dc.SetClippingRegion (DisplayMargin * newscale, DisplayMargin * newscale,
                              vScreenSize (newscale), vScreenSize (newscale));
        dc.EndDrawing ();
        m_memDC->BeginDrawing ();
        m_memDC->DestroyClippingRegion ();
        m_memDC->SetClippingRegion (DisplayMargin * newscale,
                                    DisplayMargin * newscale,
                                    vScreenSize (newscale), vScreenSize (newscale));
        m_memDC->EndDrawing ();
        
        UpdateDC (m_charDC[4], m_charmap[4], ptermApp->m_fgColor,
                  ptermApp->m_bgColor, newscale2);

        // Just allocate new bitmaps for the others, we'll repaint them below
        for (i = 0; i < 4; i++)
        {
            newmap = new wxBitmap (vCharXSize (newscale), vCharYSize (newscale), -1);
            m_charDC[i]->SelectObject (*newmap);
            delete m_charmap[i];
            m_charmap[i] = newmap;
        }
    }
    
    m_canvas->SetBackgroundColour (newbg);
    m_canvas->Refresh ();
    m_backgroundBrush.SetColour (newbg);
    m_backgroundPen.SetColour (newbg);
    m_backgroundPen.SetWidth (newscale);
    m_foregroundBrush.SetColour (newfg);
    m_foregroundPen.SetColour (newfg);
    m_foregroundPen.SetWidth (newscale);
    
    m_charDC[2]->SetBackground (m_backgroundBrush);
    m_charDC[2]->Clear ();
    m_charDC[2]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale),
                       m_charDC[4], 0, 0, wxAND_INVERT);
    m_charDC[3]->SetBackground (m_foregroundBrush);
    m_charDC[3]->Clear ();
    m_charDC[3]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale),
                       m_charDC[4], 0, 0, wxAND_INVERT);
    m_charDC[0]->SetBackground (m_foregroundBrush);
    m_charDC[0]->Clear ();
    m_charDC[0]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale),
                       m_charDC[4], 0, 0, wxAND);
    m_charDC[0]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale),
                       m_charDC[2], 0, 0, wxOR);
    m_charDC[1]->SetBackground (m_backgroundBrush);
    m_charDC[1]->Clear ();
    m_charDC[1]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale),
                       m_charDC[4], 0, 0, wxAND);
    m_charDC[1]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale),
                       m_charDC[3], 0, 0, wxOR);
}

void PtermFrame::UpdateDC (wxMemoryDC *dc, wxBitmap *&bitmap,
                           wxColour &newfg, wxColour &newbg, bool newscale2)
{
    int fr, fg, fb, br, bg, bb, ofr, ofg, ofb;
    int r, g, b, oh, ow, nh, nw, h, w;
    u8 *odata, *ndata, *newbits;
    const bool recolor = (ptermApp->m_fgColor != newfg ||
                          ptermApp->m_bgColor != newbg);
    const int newscale = (newscale2) ? 2 : 1;
    const bool rescale = (newscale != ptermApp->m_scale);
    wxMemoryDC tmpDC;
    wxBitmap *newbitmap;
    wxImage *imgp;
    
    if (!recolor && !rescale)
    {
        return;
    }

    imgp = new wxImage (bitmap->ConvertToImage ());
    
    ow = imgp->GetWidth ();
    oh = imgp->GetHeight ();
    w = ow / ptermApp->m_scale;
    h = oh / ptermApp->m_scale;
    nw = w * newscale;
    nh = h * newscale;
    
    ofr = ptermApp->m_fgColor.Red ();
    ofg = ptermApp->m_fgColor.Green ();
    ofb = ptermApp->m_fgColor.Blue ();
    fr = newfg.Red ();
    fg = newfg.Green ();
    fb = newfg.Blue ();
    br = newbg.Red ();
    bg = newbg.Green ();
    bb = newbg.Blue ();
    
    ndata = odata = imgp->GetData ();
    if (rescale)
    {
        ndata = newbits = (u8 *) malloc (3 * nw * nh);
    }
    
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            r = odata[0];
            g = odata[1];
            b = odata[2];
            if (recolor)
            {
                if ((r == ofr) && (g == ofg) && (b == ofb))
                {
                    r = fr;
                    g = fg;
                    b = fb;
                }
                else
                {
                    r = br;
                    g = bg;
                    b = bb;
                }
            }
            if (rescale)
            {
                if (newscale2)
                {
                    // from scale 1 to scale 2
                    ndata[0] = ndata[3] = r;
                    ndata[1] = ndata[4] = g;
                    ndata[2] = ndata[5] = b;
                    odata += 3;
                    ndata += 6;
                }
                else
                {
                    ndata[0] = r;
                    ndata[1] = g;
                    ndata[2] = b;
                    odata += 6;
                    ndata += 3;
                }
            }
            else
            {
                    odata[0] = r;
                    odata[1] = g;
                    odata[2] = b;
                    odata += 3;
            }
        }
        if (rescale)
        {
            if (newscale2)
            {
                // from scale 1 to scale 2 -- duplicate scan line just completed
                memcpy (ndata, ndata - (nw * 3), nw * 3);
                ndata += nw * 3;
            }
            else
            {
                // from scale 2 to scale 1 -- skip a scanline
                odata += ow * 3;
            }
        }
    }
    if (rescale)
    {
        delete imgp;
        imgp = new wxImage (nw, nh, newbits);
    }
    
    newbitmap = new wxBitmap (*imgp);
    
    dc->SelectObject (*newbitmap);
    delete bitmap;
    delete imgp;
    bitmap = newbitmap;
}

void PtermFrame::drawChar (wxDC &dc, int x, int y, int snum, int cnum)
{
    int charX, charY, sizeX, sizeY, screenX, screenY;

    charX = cnum * 8 * ptermApp->m_scale;
    charY = snum * 16 * ptermApp->m_scale;
    sizeX = 8;
    sizeY = 16;

    screenX = XADJUST (x);
    screenY = YADJUST (y + 15);
    
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
    // used in load coordinate
    int &coord = (d & 01000) ? currentY : currentX;
    
    seq++;
    if (tracePterm)
    {
        fprintf (traceF, "%07o ", d);
    }
	if ((d & 01700000) == 0)
    {
        // NOP command...
        if (d & 1)
        {
            wc = (wc + 1) & 0177;
        }
    }
    else
    {
        wc = (wc + 1) & 0177;
    }
    
    if (d & 01000000)
    {
        modewords++;
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
            
            TRACEN ("nop");
            break;

        case 1:     // load mode
            modewords = 0;              // words since entering mode
            if ((d & 020000) != 0)
            {
                // load wc bit is set
                wc = (d >> 6) & 0177;
            }
            
            wemode = (d >> 1) & 3;
            mode = (d >> 3) & 7;
            if (d & 1)
            {
                // full screen erase
                ptermFullErase ();
            }

            // bit 15 set is DISable
            m_canvas->ptermTouchPanel ((d & 0100000) == 0);
            TRACE4 ("load mode %d inhibit %d wemode %d screen %d",
                    mode, (d >> 15) & 1, wemode, (d & 1));
            break;
            
        case 2:     // load coordinate
            
            if (d & 04000)
            {
                // Add or subtract from current coordinate
                if (d & 02000)
                {
                    coord -= d & 0777;
                }
                else
                {
                    coord += d & 0777;
                }
            }
            else
            {
                coord = d & 0777;
            }
        
            if (d & 010000)
            {
                margin = coord;
                msg = "margin";
            }
            TRACE3 ("load coord %c %d %s",
                    (d & 01000) ? 'Y' : 'X', d & 0777, msg);
            break;
        case 3:     // echo
            // 160 is terminal type query
            if ((d & 0177) == 0160)
            {
                TRACE ("load echo termtype %d", TERMTYPE);
                d = 0160 + TERMTYPE;
            }
            else if ((d & 0177) == 0x7b)
            {
                // hex 7b is beep
                TRACEN ("beep");
                wxBell ();
            }
            else if ((d & 0177) == 0x7d)
            {
                // hex 7d is report MAR
                TRACE ("report MAR %o", memaddr);
                d = memaddr;
            }
            else
            {
                TRACE ("load echo %d", d & 0177);
            }
            ptermSendKey ((d & 0177) + 0200);
            break;
            
        case 4:     // load address
            memaddr = d & 077777;
            TRACE2 ("load address %o (0x%x)", memaddr, memaddr);
            break;
            
        case 5:     // SSF on PPT
            switch ((d >> 10) & 037)
            {
            case 1: // Touch panel control ?
                TRACE ("ssf touch %o", d);
                m_canvas->ptermTouchPanel ((d & 040) != 0);
                break;
            default:
                TRACE ("ssf %o", d);
                break;  // ignore
            }
            break;

        case 6:
        case 7:
            d &= 0177777;
            TRACE ("Ext %07o", d);
            // Take no other action here -- it's been done already
            // when the word was fetched
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
    int &cx = (vertical) ? currentY : currentX;
    int &cy = (vertical) ? currentX : currentY;
    
    int deltax, deltay, supdelta;
    
    deltax = (reverse) ? -8 : 8;
    deltay = (vertical) ? -16 : 16;
    if (large)
    {
        deltax *= 2;
        deltay *= 2;
    }
    
    c &= 077;
    if (c == 077)
    {
        uncover = TRUE;
        return;
    }
    if (uncover)
    {
        supdelta = (deltay / 16) * 5;
        uncover = FALSE;
        switch (c)
        {
        case 010:   // backspace
            cx = (cx - deltax) & 0777;
            break;
        case 011:   // tab
            cx = (cx + deltax) & 0777;
            break;
        case 012:   // linefeed
            cy = (cy - deltay) & 0777;
            break;
        case 013:   // vertical tab
            cy = (cy + deltay) & 0777;
        case 014:   // form feed
            if (vertical)
            {
                cx = deltay - 1;
                if (reverse)
                {
                    cy = 512 - deltax;
                }
                else
                {
                    cy = 0;
                }
            }
            else
            {
                cy = 512 - deltay;
                if (reverse)
                {
                    cx = 512 - deltax;
                }
                else
                {
                    cx = 0;
                }
            }
            break;
        case 015:   // carriage return
            cx = margin;
            cy = (cy - deltay) & 0777;
            break;
        case 016:   // superscript
            cy = (cy + supdelta) & 0777;
            break;
        case 017:   // subscript
            cy = (cy - supdelta) & 0777;
            break;
        case 020:   // select M0
        case 021:   // select M1
        case 022:   // select M2
        case 023:   // select M3
        case 024:   // select M4
        case 025:   // select M5
        case 026:   // select M6
        case 027:   // select M7
            currentCharset = c - 020;
            break;
        case 030:   // horizontal writing
            vertical = FALSE;
            break;
        case 031:   // vertical writing
            vertical = TRUE;
            break;
        case 032:   // forward writing
            reverse = FALSE;
            break;
        case 033:   // reverse writing
            reverse = TRUE;
            break;
        case 034:   // normal size writing
            large = FALSE;
            break;
        case 035:   // double size writing
            large = TRUE;
            break;
        default:
            break;
        }
    }
    else
    {
        ptermDrawChar (currentX, currentY, currentCharset, c);
        cx = (cx + deltax) & 0777;
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
    int ch, chaddr;
    
    // memaddr is a PPT RAM address; convert it to a character memory address
    chaddr = memaddr - M2ADDR;
    if (chaddr < 0 || chaddr > 127 * 16)
    {
        TRACEN ("memaddr outside character memory range");
        return;
    }
    chaddr /= 2;
    if (((d >> 16) & 3) == 0)
    {
        // load data
        TRACE2 ("character memdata %06o to %04o", d & 0xffff, chaddr);
        plato_m23[chaddr] = d & 0xffff;
        if ((++chaddr & 7) == 0)
        {
            // character is done -- load it to display 
            ch = (chaddr / 8) - 1;
            ptermLoadChar (2 + (ch / 64), ch % 64, &plato_m23[chaddr - 8]);
        }
    }
    memaddr += 2;
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
    int x1, y1, x2, y2;
    
    if (modewords & 1)
    {
        mode4start = d;
        return;
    }
    x1 = (mode4start >> 9) & 0777;
    y1 = mode4start & 0777;
    x2 = (d >> 9) & 0777;
    y2 = d & 0777;
    
    TRACE4 ("block erase %d %d to %d %d", x1, y1, x2, y2);

    ptermBlockErase (x1, y1, x2, y2);
    currentX = x1;
    currentY = y1 - 15;
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
    wxString name;
    
    SetCursor (wxNullCursor);
    if (!m_hostName.IsEmpty ())
    {
        name.Printf (wxT (" %s %d-%d"),
                     m_hostName.c_str (), station >> 5, station & 31);
    }
    else
    {
        name.Printf (wxT (" %d-%d"),
                     station >> 5, station & 31);
    }
    ptermSetStatus (name);
    ptermSetName (name);
}

void PtermFrame::ptermShowTrace (bool enable)
{
    if (enable)
    {
        SetStatusText(_(" Trace "), STATUS_TRC);
    }
    else
    {
        SetStatusText(wxT (""), STATUS_TRC);
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
    m_gswEnable = ptermApp->m_gswEnable;
    m_connect = ptermApp->m_connect;
    m_fgColor = ptermApp->m_fgColor;
    m_bgColor = ptermApp->m_bgColor;
    m_host = ptermApp->m_hostName;
    m_port.Printf (wxT ("%d"), ptermApp->m_port);

    m_fgBitmap = new wxBitmap (20, 12);
    m_bgBitmap = new wxBitmap (20, 12);
    paintBitmap (m_fgBitmap, m_fgColor);
    paintBitmap (m_bgBitmap, m_bgColor);
    
    m_colorsBox = new wxStaticBox (this, wxID_ANY, _("Display colors"));
    m_colorsSizer = new wxStaticBoxSizer (m_colorsBox, wxVERTICAL);
    m_fgButton = new wxBitmapButton (this, wxID_ANY, *m_fgBitmap);
    m_bgButton = new wxBitmapButton (this, wxID_ANY, *m_bgBitmap);
    m_okButton = new wxButton (this, wxID_ANY, _("OK"));
    m_cancelButton = new wxButton (this, wxID_ANY, _("Cancel"));
    m_resetButton = new wxButton (this, wxID_ANY, _("Defaults"));
    m_fgLabel = new wxStaticText (this, wxID_ANY, _("&Foreground"));
    m_bgLabel = new wxStaticText (this, wxID_ANY, _("&Background"));
    m_fgSizer = new wxBoxSizer (wxHORIZONTAL);
    m_bgSizer = new wxBoxSizer (wxHORIZONTAL);
    m_scaleCheck = new wxCheckBox (this, -1, _("&Zoom display 200%"));
    m_scaleCheck->SetValue (m_scale2);
    m_speedCheck = new wxCheckBox (this, -1, _("&Simulate 1200 Baud"));
    m_speedCheck->SetValue (m_classicSpeed);
    m_gswCheck = new wxCheckBox (this, -1, _("Enable &GSW"));
    m_gswCheck->SetValue (m_gswEnable);
    m_connBox = new wxStaticBox (this, wxID_ANY, _("Connection settings"));
    m_connSizer = new wxStaticBoxSizer (m_connBox, wxVERTICAL);
    m_autoConnect = new wxCheckBox (this, -1, _("&Connect at startup"));
    m_autoConnect->SetValue (m_connect);
    m_hostLabel = new wxStaticText (this, wxID_ANY, _("Default &Host"));
    m_portLabel = new wxStaticText (this, wxID_ANY, _("Default &Port"));

    m_hostVal = new wxTextValidator (wxFILTER_ASCII, &m_host);
    m_portVal = new wxTextValidator (wxFILTER_NUMERIC, &m_port);
    
    m_hostText = new wxTextCtrl (this, wxID_ANY, m_host,
                                 wxDefaultPosition, wxSize (160, 18),
                                 0, *m_hostVal);
    m_portText = new wxTextCtrl (this, wxID_ANY, m_port,
                                 wxDefaultPosition, wxSize (160, 18),
                                 0, *m_portVal);
    m_connItems = new wxFlexGridSizer (2, 2, 5, 5);

    m_prefItems = new wxBoxSizer (wxVERTICAL);
    m_prefButtons = new wxBoxSizer (wxHORIZONTAL);
    m_dialogContent = new wxBoxSizer (wxVERTICAL);
      
    m_fgSizer->Add (m_fgButton, 0, wxALL, 5);
    m_fgSizer->Add (m_fgLabel, 0, wxALL, 5);
    m_bgSizer->Add (m_bgButton, 0, wxALL, 5);
    m_bgSizer->Add (m_bgLabel, 0, wxALL, 5);
    m_colorsSizer->Add (m_fgSizer);
    m_colorsSizer->Add (m_bgSizer);
    m_connItems->Add (m_hostLabel, 0, wxRIGHT, 5);
    m_connItems->Add (m_hostText);
    m_connItems->Add (m_portLabel, 0, wxRIGHT, 5);
    m_connItems->Add (m_portText);
    m_connSizer->Add (m_autoConnect, 0, wxALL, 5);
    m_connSizer->Add (m_connItems, 0, wxALL, 5);
    m_prefItems->Add (m_scaleCheck, 0, wxTOP | wxLEFT | wxRIGHT, 10);
    m_prefItems->Add (m_speedCheck, 0, wxTOP | wxLEFT | wxRIGHT, 10);
    m_prefItems->Add (m_gswCheck, 0, wxALL, 10);
    m_prefItems->Add (m_connSizer, 0, wxALL, 10);
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
        m_fgColor = PtermApp::SelectColor (m_fgColor);
    }
    else if (event.m_eventObject == m_bgButton)
    {
        m_bgColor = PtermApp::SelectColor (m_bgColor);
    }
    
    else if (event.m_eventObject == m_okButton)
    {
        m_host = m_hostText->GetLineText (0);
        m_port = m_portText->GetLineText (0);
        EndModal (wxID_OK);
    }
    else if (event.m_eventObject == m_cancelButton)
    {
        EndModal (wxID_CANCEL);
    }
    else if (event.m_eventObject == m_resetButton)
    {
        wxString str;
        
        m_fgColor = wxColour (255, 144, 0);
        m_bgColor = *wxBLACK;
        m_scale2 = FALSE;
        m_scaleCheck->SetValue (FALSE);
        m_classicSpeed = FALSE;
        m_speedCheck->SetValue (FALSE);
        m_connect = TRUE;
        m_autoConnect->SetValue (TRUE);
        m_gswEnable = TRUE;
        m_gswCheck->SetValue (TRUE);
        m_hostText->SetValue (DEFAULTHOST);
        str.Printf (wxT ("%d"), DefNiuPort);
        m_portText->SetValue (str);
    }
    paintBitmap (m_fgBitmap, m_fgColor);
    paintBitmap (m_bgBitmap, m_bgColor);
    Refresh ();
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
    else if (event.m_eventObject == m_autoConnect)
    {
        m_connect = event.IsChecked ();
    }
    else if (event.m_eventObject == m_gswCheck)
    {
        m_gswEnable = event.IsChecked ();
    }
}

void PtermPrefdialog::paintBitmap (wxBitmap *bm, wxColour &color)
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
    m_port.Printf (wxT ("%d"), ptermApp->m_port);

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
      
    m_connItems->Add (m_hostLabel, 0, wxRIGHT, 5);
    m_connItems->Add (m_hostText);
    m_connItems->Add (m_portLabel, 0, wxRIGHT, 5);
    m_connItems->Add (m_portText);
    m_connButtons->Add (m_okButton, 0, wxALL, 5);
    m_connButtons->Add (m_cancelButton, 0, wxALL, 5);
    m_dialogContent->Add (m_connItems, 0, wxTOP | wxLEFT | wxRIGHT, 15);
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

PtermConnection::PtermConnection (PtermFrame *owner, wxString &host, int port)
    : wxThread (wxTHREAD_JOINABLE),
      m_owner (owner),
      m_port (port),
      m_displayIn (0),
      m_displayOut (0),
      m_gswActive (FALSE),
      m_savedWord (-1),
      m_savedGswMode (0)
{
    m_hostName = host;
}

PtermConnection::~PtermConnection ()
{
    if (m_gswActive)
    {
        ptermCloseGsw ();
    }
    dtCloseFet (&m_fet);
}

PtermConnection::ExitCode PtermConnection::Entry (void)
{
    u32 platowd;
    int i, j, k;
    bool wasEmpty;
    
    int true_opt = 1;
    
    dtInitFet (&m_fet, BufSiz);
    if (dtConnect (&m_fet.connFd, m_hostName.mb_str (), m_port) < 0)
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
            if (m_savedWord >= 0)
            {
                // we had a word saved that we fetched before but couldn't
                // save in the display ring because the GSW had no room.
                // try to process it again.
                platowd = m_savedWord;
                m_savedWord = -1;
            }
            else
            {
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
            }
            
            if (!m_gswActive && (platowd >> 16) == 3 &&
                platowd != 0700001 &&
                platowd != 0702010)     // *** temp workaround for "edit" -ext-
            {
                // It's an -extout- word, which means we'll want to start up
                // GSW emulation (if enabled: TBD).
                // However, we'll see these also when PLATO is turning OFF
                // the GSW (common entry code in the various gsw lessons).
                // We don't want to grab the GSW subsystem in that case.
                // The "turn off" sequence consists of a mode word followed
                // by voice words that specify "rest" (operand == 1).
                // The sound is silenced when the GSW is not active, so we'll
                // ignore "rest" voice words in that case.  We'll save the last
                // voice word, because it sets number of voices and volumes.
                // We have to do that because when the music actually starts,
                // we'll first see a mode word and then some non-rest voice
                // words.  The non-rest voice words trigger the GSW startup,
                // so we'll need to send the preceding mode word for correct
                // initialization.
                if ((platowd >> 15) == 6)
                {
                    // mode word, just save it
                    m_savedGswMode = platowd;
                }
                else if (ptermOpenGsw (this) == 0)
                {
                    m_gswActive = TRUE;
                    if (m_savedGswMode != 0)
                    {
                        ptermProcGswData (m_savedGswMode);
                        m_savedGswMode = 0;
                    }
                }
            }
            
            if (m_gswActive)
            {
                // feed the word to the GSW.  If it can't buffer it, we
                // save it for next time.
                i = ptermProcGswData (platowd);
                if (i < 0)
                {
                    m_savedWord = platowd;
                    break;
                }
                if (i != 0)
                {
                    m_gswActive = FALSE;
                }
            }
            
            if (platowd == 2)
            {
                m_savedGswMode = 0;

                // erase abort marker -- reset the ring to be empty
                wxCriticalSectionLocker lock (m_pointerLock);

                m_displayOut = m_displayIn;
            }

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
    int word = 0, next;
    int delay = 0;

    {
        wxCriticalSectionLocker lock (m_pointerLock);
        
        if (m_displayIn == m_displayOut)
        {
            return C_NODATA;
        }
    
        word = m_displayRing[m_displayOut];
        next = m_displayOut + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
        m_displayOut = next;
    }
    
    // See if emulating 1200 baud, or the -delay- NOP code, or GSW is active
    if (ptermApp->m_classicSpeed ||
        word == 1 ||
        m_gswActive)
    {
        delay = 1;
    }
    
    // Pass the delay to the caller
    word |= (delay << 19);
    
    if (word == C_CONNFAIL || word == C_DISCONNECT)
    {
        wxString msg;
            
        m_owner->SetStatusText (_(" Not connected"), STATUS_CONN);
        if (word == C_CONNFAIL)
        {
            msg.Printf (_("Failed to connect to %s %d"),
                        m_hostName.c_str (), m_port);
        }
        else
        {
            msg.Printf (_("Connection lost to %s %d"),
                        m_hostName.c_str (), m_port);
        }
            
        wxMessageDialog alert (m_owner, msg, wxString (_("Alert")), wxOK);
            
        alert.ShowModal ();
        m_owner->Close (TRUE);
        word = C_NODATA;
    }
        
    return word;
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
    m_displayIn = next;
    
#ifdef DEBUG
//    wxLogMessage ("data from plato %07o", word);
#endif
}

void PtermConnection::SendData (const void *data, int len)
{
	// Windows has the wrong type for the buffer pointer argument...
    send(m_fet.connFd, (const char *) data, len, 0);
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
    
    SetBackgroundColour (ptermApp->m_bgColor);
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
    if (key == WXK_ALT || key == WXK_SHIFT || key == WXK_CONTROL)
    {
        // We don't take any action on the modifier key keydown events,
        // but we do want to make sure they are seen by the rest of
        // the system.
        event.Skip ();
        return;
    }
    if (key < 0200 && isalpha (key))
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

/*---------------------------  End Of File  ------------------------------*/
