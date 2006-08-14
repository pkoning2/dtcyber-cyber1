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
#define RINGXON1    (RINGSIZE/3)
#define RINGXON2    (RINGSIZE/4)
#define RINGXOFF1   (RINGSIZE-RINGXON1)
#define RINGXOFF2   (RINGSIZE-RINGXON2)
#define xonkey      01606
#define xofkey      01607
#define ascxon      0x11
#define ascxof      0x13

#define GSWRINGSIZE 100

#define C_NODATA        -1
#define C_CONNFAIL      -2
#define C_DISCONNECT    -3
#define C_GSWEND        -4

#define STATUS_TIP      0
#define STATUS_TRC      1
#define STATUS_CONN     2
#define STATUSPANES     3

#define KeyBufSize      50
#define DisplayMargin   8

#define MouseTolerance  3       // # pixels tolerance before we call it "drag"

// Delay parameters for text paste (in ms)
#define PASTE_CHARDELAY 165
#define PASTE_LINEDELAY 500

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for the character sets (ROM and loadable)
#define vXSize(s)       (512 * (s) + (2 * DisplayMargin))
#define vYSize(s)       (512 * (s) + (2 * DisplayMargin))
#define vCharXSize(s)   (512 * (s))
#define vCharYSize(s)   ((CSETS * 16) * (s))
#define vScreenSize(s)  (512 * (s))
#define XSize           (vYSize (ptermApp->m_scale))
#define YSize           (vXSize (ptermApp->m_scale))
#define CharXSize       (vCharXSize (ptermApp->m_scale))
#define CharYSize       (vCharYSize (ptermApp->m_scale))
#define ScreenSize      (vScreenSize (ptermApp->m_scale))

#define TERMTYPE        10
#define ASCTYPE         12
#define SUBTYPE         0x74
#define TERMCONFIG      0x40    // touch panel present
#define ASCFEATURES     0x01    // features: fine touch

// Literal strings for wxConfig key strings.  These are defined
// because they appear in two places, so this way we avoid getting
// the two out of sync.  Note that they should *not* be changed after
// once being defined, since that invalidates people's stored
// preferences.
#define PREF_FOREGROUND "foreground"
#define PREF_BACKGROUND "background"
#define PREF_HOST       "host"
#define PREF_PORT       "port"
#define PREF_SCALE2     "scale"
#define PREF_1200BAUD   "classicSpeed"
#define PREF_CONNECT    "autoconnect"
#define PREF_GSW        "gswenable"
#define PREF_ARROWS     "numpadArrows"
#define PREF_STATUSBAR  "statusbar"
#define PREF_PLATOKB    "platoKeyboard"
#define PREF_XPOS       "xPosition"
#define PREF_YPOS       "yPosition"
#define PREF_BEEP       "beep"

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

// Map PLATO coordinates to window coordinates
#define XADJUST(x) ((x) * ptermApp->m_scale + GetXMargin ())
#define YADJUST(y) ((511 - (y)) * ptermApp->m_scale + GetYMargin ())

// Map PLATO coordinates to backing store bitmap coordinates
#define XMADJUST(x) ((x) * ptermApp->m_scale)
#define YMADJUST(y) ((511 - (y)) * ptermApp->m_scale)

// inverse mapping (for processing touch input)
#define XUNADJUST(x) (((x) - GetXMargin ()) / ptermApp->m_scale)
#define YUNADJUST(y) (511 - ((y) - GetYMargin ()) / ptermApp->m_scale)

// force coordinate into the range 0..511
#define BOUND(x) x = (((x < 0) ? 0 : ((x > 511) ? 511 : x)))

// Define top left corner of the PLATO drawing area, in windowing
// system coordinates (which have origin at the top and Y axis upside down)
// The YTOP definition looks a bit strange because we want the top one
// of the two pixels if we're doing double size display
#define XTOP (XADJUST (0))
#define YTOP (YADJUST (512) + 1)
// Ditto but for the backing store bitmap
#define XMTOP (XMADJUST (0))
#define YMTOP (YMADJUST (512) + 1)

// Macro to include keyboard accelerator only if MAC
#if defined(__WXMAC__)
#define MACACCEL(x) + wxString (wxT (x))
#else
#define MACACCEL(x)
#endif

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
#include "wx/filename.h"
#include "wx/metafile.h"
#include "wx/print.h"
#include "wx/printdlg.h"

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
#include <stdlib.h>
#include "const.h"
#include "types.h"
#include "proto.h"
#include "ptermversion.h"
#include "8080a.h"
#include "8080avar.h"
#include "ppt.h"

#if wxUSE_LIBGNOMEPRINT
#include "wx/html/forcelnk.h"
FORCE_LINK(gnome_print)
#endif

extern int ptermOpenGsw (void *user);
extern int ptermProcGswData (int data);
extern void ptermCloseGsw (void);
extern void ptermStartGsw (void);

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

static const u32 keyboardhelp[] = {
#include "ptermkeys.h"
};

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

bool emulationActive = false;

// Global print data, to remember settings during the session
wxPrintData *g_printData;

// Global page setup data
wxPageSetupDialogData* g_pageSetupData;

// ----------------------------------------------------------------------------
// local variables
// ----------------------------------------------------------------------------

class PtermApp;
static PtermApp *ptermApp;

static FILE *traceF;
static char traceFn[20];

#ifdef DEBUGLOG
static wxLogWindow *logwindow;
#endif

static const wxChar rom01char[] =
    wxT(":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.\xF7[]%\xD7\xAB'\"!;<>_?\xBB "
        "#ABCDEFGHIJKLMNOPQRSTUVWXYZ~\xA8^\xB4`    ~    {}&  |\xB0     \xB5       @\\ ");

// high bit is M0/M1 selector (into the classic ROM)
// 0xff means unused code.  The other codes 0xf0 and up
// are specials:
// 0xf0: embed left
// 0xf1: embed right
// 0xf2: copyright body
// 0xf3: box
// 0xf4: diamond
// 0xf5: cross product
// 0xf6: hacek
// 0xf7: universal delimiter
// 0xf8: dot product
// 0xf9: cedilla
static const u8 asciiM0[] = 
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x2d, 0x38, 0x37, 0x80, 0x2b, 0x33, 0xab, 0x36,
  0x29, 0x2a, 0x27, 0x25, 0x2e, 0x26, 0x2f, 0x28,
  0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22,
  0x23, 0x24, 0x00, 0x39, 0x3a, 0x2c, 0x3b, 0x3d,
  0xbd, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9a, 0x31, 0xbe, 0x32, 0x9d, 0x3c,
  0x9f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0xa9, 0xae, 0xaa, 0xa4, 0xff
};
static const u8 asciiM1[] = 
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x2d, 0x28, 0xb0, 0x9b, 0x35, 0xac, 0xa0, 0xa1,
  0xa2, 0xa3, 0x34, 0xa5, 0xa6, 0xa7, 0xa8, 0x30,
  0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
  0xb9, 0xba, 0xbb, 0xbc, 0xf0, 0xaf, 0xf1, 0x3e,
  0xf2, 0x9c, 0xf3, 0xf8, 0xf4, 0xf5, 0x9e, 0xf9,
  0xf6, 0xf7, 0xae, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const u8 asciiKeycodes[] =
{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x26, 0x60, 0x0a, 0x5e, 0x2b, 0x2d,
  0x13, 0x04, 0x07, 0x08, 0x7b, 0x0b, 0x0d, 0x1a,
  0x02, 0x12, 0x01, 0x03, 0x7d, 0x0c, 0xff, 0xff,
  0x3c, 0x3e, 0x5b, 0x5d, 0x24, 0x25, 0x5f, 0x7c,
  0x2a, 0x28, 0x40, 0x27, 0x1c, 0x5c, 0x23, 0x7e,
  0x17, 0x05, 0x14, 0x19, 0x7f, 0x09, 0x1e, 0x18,
  0x0e, 0x1d, 0x11, 0x16, 0x00, 0x0f, 0xff, 0xff,
  0x20, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7a, 0x3d, 0x3b, 0x2f, 0x2e, 0x2c,
  0x1f, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5a, 0x29, 0x3a, 0x3f, 0x21, 0x22
};

// These are the strings to use for the "special" characters.
// On classic terminals, the formatter generates these from
// the regular ROM characters with a sequence of backspaces and
// the like.  In ASCII mode, we have to do it; we do it
// roughly the same way.  Each row is indexed by the
// special char code - 0xf0.  Each entry is a pair:
//  char code, Y offset.  Char code 0 is the terminator.
// All are plotted at the same X implicitly.
static const u8 M1specials[][8] =
{ { 0x3a, 0, 0x29, 0, 0 },              // embed left
  { 0x3b, 0, 0x2a, 0, 0 },              // embed right
  { 0x3c, 0, 0x3c, 11, 0x03, 1, 0 },    // copyright body
  { 0xa7, 0, 0xa8, 0, 0 },              // box
  { 0xa0, 0, 0xa3, 0, 0xa2, 0, 0 },     // diamond
  { 0x28, 0, 0xbe, 0, 0 },              // cross product
  { 0x9e, 0, 0x9f, 0, 0 },              // hacek
  { 0xa0, 0, 0xa2, 0, 0 },              // universal delimiter
  { 0xaf, (u8) -3, 0 },                 // dot product
  { 0x9e, (u8) -11, 0 },                // cedilla
};

// Conversion from ascii mode codes to classic codes
static const u8 ascmode[] =
{ 0, 3, 2, 1 };

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

class PtermFrame;

// Pterm screen printout
class PtermPrintout: public wxPrintout
{
public:
    PtermPrintout (PtermFrame *owner,
                   const wxString &title = _("Pterm printout")) 
        : wxPrintout (title),
          m_owner (owner)
    {}
    bool OnPrintPage (int page);
    bool HasPage (int page);
    bool OnBeginDocument (int startPage, int endPage);
    void GetPageInfo (int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
    void DrawPage (wxDC *dc);
    inline int GetXMargin (void) const;
    inline int GetYMargin (void) const;

private:
    PtermFrame *m_owner;
};


// Pterm processing thread
class PtermConnection : public wxThread
{
public:
    // override base class virtuals
    // ----------------------------
    PtermConnection (PtermFrame *owner, wxString &host, int port);
    ~PtermConnection ();
    
    virtual ExitCode Entry (void);

    int AssembleNiuWord (void);
    int AssembleAsciiWord (void);
    int AssembleAutoWord (void);
    int NextWord (void);
    int NextGswWord (bool catchup);
    
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
    bool Ascii (void) const
    {
        return (m_connMode == ascii);
    }
    void StoreWord (int word);

private:
    NetFet      m_fet;
    u32         m_displayRing[RINGSIZE];
    volatile int m_displayIn, m_displayOut;
    u32         m_gswRing[GSWRINGSIZE];
    volatile int m_gswIn, m_gswOut;
    PtermFrame  *m_owner;
    wxString    m_hostName;
    int         m_port;
    wxCriticalSection m_pointerLock;
    bool        m_gswActive;
    bool        m_gswStarted;
    int         m_savedGswMode;
    int         m_gswWord2;
    enum { both, niu, ascii } m_connMode;
    int         m_pending;
    
    int NextRingWord (void);
};

extern "C" int ptermNextGswWord (void *connection, int catchup);

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
    void OnHelpKeys (wxCommandEvent &event);
    void OnAbout (wxCommandEvent& event);

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
    bool        m_numpadArrows;
    bool        m_showStatusBar;
    bool        m_platoKb;
    bool        m_beepEnable;
    
    PtermFrame  *m_firstFrame;
    wxString    m_defDir;
    PtermFrame  *m_helpFrame;

    int lastX;
    int lastY;
    
private:
    wxLocale    m_locale; // locale we'll be using
    
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// define a scrollable canvas for drawing onto
class PtermCanvas : public wxScrolledWindow
{
public:
    PtermCanvas (PtermFrame *parent);

    void ptermTouchPanel(bool enable);

    void OnDraw (wxDC &dc);
    void OnKeyDown (wxKeyEvent& event);
    void OnChar (wxKeyEvent& event);
    void OnMouseDown (wxMouseEvent &event);
    void OnMouseUp (wxMouseEvent &event);
    void OnMouseMotion (wxMouseEvent &event);
    void OnCopy (wxCommandEvent &event);

#if defined (__WXMSW__)
    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif
    
    void SaveChar (int x, int y, int snum, int cnum, int w, bool large_p);
    void FullErase (void);
    void ClearRegion (void);
    void UpdateRegion (wxMouseEvent &event);

    inline int GetXMargin (void) const;
    inline int GetYMargin (void) const;

private:
    PtermFrame *m_owner;
    bool        m_touchEnabled;

    // Text-copy support
    u8  textmap[32 * 64];
    int m_regionX;
    int m_regionY;
    int m_regionHeight;
    int m_regionWidth;
    int m_mouseX;
    int m_mouseY;
    
    DECLARE_EVENT_TABLE ()
};

#if defined(__WXMAC__)
#define PTERM_MDI 1
#else
#define PTERM_MDI 0
#endif

#if PTERM_MDI
#define PtermFrameBase  wxMDIChildFrame

class PtermMainFrame : public wxMDIParentFrame
{
public:
    PtermMainFrame (void);
    wxMenuItem* AppendWinItem (int itemid,
                               const wxString& text,
                               const wxString& help = wxEmptyString,
                               wxItemKind kind = wxITEM_NORMAL)
    {
        if (m_windowMenu != NULL)
            return m_windowMenu->Append (itemid, text, help, kind);
        else
            return NULL;
    }
};

static PtermMainFrame *PtermFrameParent;
#else
#define PtermFrameBase wxFrame
#define PtermFrameParent NULL
#endif

// Define a new frame type: this is going to be our main frame
class PtermFrame : public PtermFrameBase, public emul8080
{
    friend void PtermCanvas::OnDraw(wxDC &dc);
    friend void PtermApp::OnHelpKeys (wxCommandEvent &event);
    friend int PtermConnection::NextWord (void);
public:
    // ctor(s)
    PtermFrame(wxString &host, int port, const wxString& title,
                   const wxPoint &pos = wxDefaultPosition);
    ~PtermFrame ();

    // event handlers (these functions should _not_ be virtual)
    void OnIdle (wxIdleEvent& event);
    void OnClose (wxCloseEvent& event);
    void OnTimer (wxTimerEvent& event);
    void OnPasteTimer (wxTimerEvent& event);
    void OnQuit (wxCommandEvent& event);
    void OnCopyScreen (wxCommandEvent &event);
    void OnCopy (wxCommandEvent &event);
    void OnPaste (wxCommandEvent &event);
    void OnUpdateUIPaste (wxUpdateUIEvent& event);
    void OnSaveScreen (wxCommandEvent &event);
    void OnPrint (wxCommandEvent& event);
    void OnPrintPreview (wxCommandEvent& event);
    void OnPageSetup (wxCommandEvent& event);
    void OnActivate (wxActivateEvent &event);
    void UpdateSettings (wxColour &newfg, wxColour &newbf, bool newscale2,
                         bool newstatusbar);
    void SetColors (wxColour &newfg, wxColour &newbg, int newscale);
    void OnFullScreen (wxCommandEvent &event);
    
    void PrepareDC(wxDC& dc);
    void ptermSendKey(int key);
    void ptermSendTouch (int x, int y);
    void ptermSetTrace (bool fileaction);
#if 0
    // The s0ascers spec calls for parity but it isn't really needed
    // and by not doing it we keep things simpler.  For example,
    // that way we don't need to worry about telnet escaping
    // (0xff escaping) in the inbound direction.
    int Parity (int key);
#else
#define Parity(x) (x)
#endif

    bool HasConnection (void) const
    {
        return (m_conn != NULL);
    }
    int GetXMargin (void) const
    {
        if (m_fullScreen)
        {
            int i = GetClientSize ().x - ScreenSize;
            return (i < 0) ? 0 : i / 2;
        }
        else
            return DisplayMargin;
    }
    int GetYMargin (void) const
    {
        if (m_fullScreen)
        {
            int i = GetClientSize ().y - ScreenSize;
            return (i < 0) ? 0 : i / 2;
        }
        else
            return DisplayMargin;
    }
    
    wxMemoryDC  *m_memDC;
    bool        tracePterm;
    PtermFrame  *m_nextFrame;
    PtermFrame  *m_prevFrame;

    int         m_pendingEcho;

	bool		m_bCancelPaste;
	bool		m_bPasteActive;

private:
    wxStatusBar *m_statusBar;       // present even if not displayed
    wxPen       m_foregroundPen;
    wxPen       m_backgroundPen;
    wxBrush     m_foregroundBrush;
    wxBrush     m_backgroundBrush;
    wxBitmap    *m_bitmap;
    PtermCanvas *m_canvas;
    PtermConnection *m_conn;
    wxString    m_hostName;
    int         m_port;
    wxTimer     m_timer;
    bool        m_fullScreen;
    int         m_station;
    
    // Stuff for pacing Paste operations
    wxTimer     m_pasteTimer;
    wxString    m_pasteText;
    int         m_pasteIndex;
    bool        m_pastePrint;
    
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
#define mode RAM[M_MODE]
#define wemode (mode & 3)
    int         currentX;
    int         currentY;
#define margin ReadRAMW (M_MARGIN)
    int         memaddr;
    u16         plato_m23[128 * 8];
    int         memlpc;
#define uncover     (RAM[M_CCR] & 0x80)
#define reverse     (RAM[M_CCR] & 0x40)
#define large       (RAM[M_CCR] & 0x20)
#define currentCharset ((RAM[M_CCR] & 0x0e) >> 1)
#define vertical    (RAM[M_CCR] & 0x01)
    int         wc;
    int         seq;
    int         modewords;
    int         mode4start;
    bool        m_dumbTty;
    typedef enum { none, ldc, lde, lda, ssf, fg, bg, paint, pni_rs, ext } AscState;
    AscState    m_ascState;
    int         m_ascBytes;
    int         m_assembler;
    int         lastX;
    int         lastY;
    bool        m_flowCtrl;
    bool        m_sendFgt;
    
    wxColour    m_defFg;
    wxColour    m_defBg;
    wxColour    m_currentFg;
    wxColour    m_currentBg;
    
    void setMargin (int i)
    {
        RAM[M_MARGIN] = i;
        RAM[M_MARGIN + 1] = i >> 8;
    }
    void setUncover (bool u)
    {
        if (u)
        {
            RAM[M_CCR] |= 0x80;
        }
        else
        {
            RAM[M_CCR] &= ~0x80;
        }
    }
    void setReverse (bool u)
    {
        if (u)
        {
            RAM[M_CCR] |= 0x40;
        }
        else
        {
            RAM[M_CCR] &= ~0x40;
        }
    }
    void setLarge (bool u)
    {
        if (u)
        {
            RAM[M_CCR] |= 0x20;
        }
        else
        {
            RAM[M_CCR] &= ~0x20;
        }
    }
    void setCmem (int i)
    {
        RAM[M_CCR] = (RAM[M_CCR] & ~0x0e) | (i << 1);
    }
    void setVertical (bool u)
    {
        if (u)
        {
            RAM[M_CCR] |= 0x01;
        }
        else
        {
            RAM[M_CCR] &= ~0x01;
        }
    }
    

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
    void ptermPaint (int pat);
    
    void drawChar (wxDC &dc, int x, int y, int snum, int cnum);
    void procPlatoWord (u32 d, bool ascii);
    void plotChar (int c);
    void mode0 (u32 d);
    void mode1 (u32 d);
    void mode2 (u32 d);
    void mode3 (u32 d);
    void mode4 (u32 d);
    void mode5 (u32 d);
    void mode6 (u32 d);
    void mode7 (u32 d);
    void progmode (u32 d, int origin);
    void ptermSetStation (int station);
    void ptermShowTrace ();
    
    bool AssembleCoord (int d);
    int AssemblePaint (int d);
    int AssembleData (int d);
    int AssembleColor (int d);
    
    typedef void (PtermFrame::*mptr)(u32);

    static const mptr modePtr[8];

    // 8080a emulation support
    Uint8 input8080a (Uint8 data);
    void output8080a (Uint8 data, Uint8 acc);
    int check_pc8080a (void);

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

// define the preferences dialog
class PtermPrefDialog : public wxDialog
{
public:
    PtermPrefDialog (PtermFrame *parent, wxWindowID id, const wxString &title);
    
    void OnButton (wxCommandEvent& event);
    void OnCheckbox (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    wxBitmapButton  *m_fgButton;
    wxBitmapButton  *m_bgButton;
    wxButton        *m_okButton;
    wxButton        *m_cancelButton;
    wxButton        *m_resetButton;
    wxCheckBox      *m_scaleCheck;
    wxCheckBox      *m_speedCheck;
    wxCheckBox      *m_autoConnect;
    wxCheckBox      *m_gswCheck;
    wxCheckBox      *m_arrowCheck;
    wxCheckBox      *m_statusCheck;
    wxCheckBox      *m_kbCheck;
    wxCheckBox      *m_beepCheck;
    wxTextCtrl      *m_hostText;
    wxTextCtrl      *m_portText;

    wxColour        m_fgColor;
    wxColour        m_bgColor;
    bool            m_scale2;
    bool            m_classicSpeed;
    bool            m_connect;
    bool            m_gswEnable;
    bool            m_numpadArrows;
    bool            m_showStatusBar;
    bool            m_platoKb;
    bool            m_beepEnable;
    wxString        m_host;
    wxString        m_port;
    
private:
    void paintBitmap (wxBitmap &bm, wxColour &color);
    
    PtermFrame *m_owner;

    DECLARE_EVENT_TABLE ()
};

// define the preferences dialog
class PtermConnDialog : public wxDialog
{
public:
    PtermConnDialog (wxWindowID id, const wxString &title);
    
    void OnOK (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    wxTextCtrl      *m_hostText;
    wxTextCtrl      *m_portText;
    
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
    Pterm_SaveScreen,
    Pterm_HelpKeys,
    Pterm_PastePrint,
    Pterm_FullScreen,

    // timers
    Pterm_Timer,        // display pacing
    Pterm_PasteTimer,   // paste key generation pacing

    // Menu items with standard ID values
    Pterm_Print = wxID_PRINT,
    Pterm_Page_Setup = wxID_PRINT_SETUP,
    Pterm_Preview = wxID_PREVIEW,
    Pterm_Copy = wxID_COPY,
    Pterm_Paste = wxID_PASTE,
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
    EVT_TIMER(Pterm_Timer, PtermFrame::OnTimer)
    EVT_TIMER(Pterm_PasteTimer, PtermFrame::OnPasteTimer)
    EVT_ACTIVATE(PtermFrame::OnActivate)
    EVT_MENU(Pterm_Close, PtermFrame::OnQuit)
    EVT_MENU(Pterm_CopyScreen, PtermFrame::OnCopyScreen)
    EVT_MENU(Pterm_Copy, PtermFrame::OnCopy)
    EVT_MENU(Pterm_Paste, PtermFrame::OnPaste)
    EVT_MENU(Pterm_PastePrint, PtermFrame::OnPaste)
    EVT_UPDATE_UI(Pterm_Paste, PtermFrame::OnUpdateUIPaste)
    EVT_MENU(Pterm_SaveScreen, PtermFrame::OnSaveScreen)
    EVT_MENU(Pterm_Print, PtermFrame::OnPrint)
    EVT_MENU(Pterm_Preview, PtermFrame::OnPrintPreview)
    EVT_MENU(Pterm_Page_Setup, PtermFrame::OnPageSetup)
    EVT_MENU(Pterm_FullScreen, PtermFrame::OnFullScreen)
    END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(PtermApp, wxApp)
    EVT_MENU(Pterm_Connect, PtermApp::OnConnect)
    EVT_MENU(Pterm_ConnectAgain, PtermApp::OnConnect)
    EVT_MENU(Pterm_Pref,    PtermApp::OnPref)
    EVT_MENU(Pterm_Quit,    PtermApp::OnQuit)
    EVT_MENU(Pterm_HelpKeys, PtermApp::OnHelpKeys)
    EVT_MENU(Pterm_About, PtermApp::OnAbout)

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
    m_firstFrame = m_helpFrame = NULL;
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
    
    sprintf (traceFn, "pterm%d.trc", getpid ());

    m_locale.Init(wxLANGUAGE_DEFAULT);
    m_locale.AddCatalog(wxT("pterm"));

#ifdef DEBUGLOG
    logwindow = new wxLogWindow (NULL, "pterm log", true, false);
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
        m_port = m_config->Read (wxT (PREF_PORT), DefNiuPort);
    }
    if (argc > 1)
    {
        m_hostName = argv[1];
    }
    else
    {
        m_config->Read (wxT (PREF_HOST), &m_hostName, DEFAULTHOST);
    }

    // 255 144 0 is RGB for Plato Orange
    m_config->Read (wxT (PREF_FOREGROUND), &rgb, wxT ("255 144 0"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_fgColor = wxColour (r, g, b);
    m_config->Read (wxT (PREF_BACKGROUND), &rgb, wxT ("0 0 0"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_bgColor = wxColour (r, g, b);
    m_scale = m_config->Read (wxT (PREF_SCALE2), 1);
    if (m_scale != 1 && m_scale != 2)
    {
        m_scale = 1;
    }
    m_classicSpeed = (m_config->Read (wxT (PREF_1200BAUD), 0L) != 0);
    m_connect = (m_config->Read (wxT (PREF_CONNECT), 1) != 0);
    m_gswEnable = (m_config->Read (wxT (PREF_GSW), 1) != 0);
    m_numpadArrows = (m_config->Read (wxT (PREF_ARROWS), 1) != 0);
    m_showStatusBar = (m_config->Read (wxT (PREF_STATUSBAR), 1) != 0);
    m_platoKb = (m_config->Read (wxT (PREF_PLATOKB), 0L) != 0);
    m_beepEnable = (m_config->Read (wxT (PREF_BEEP), 1) != 0);

#if PTERM_MDI
    // On Mac, the style rule is that the application keeps running even
    // if all its windows are closed.
//    SetExitOnFrameDelete(false);
    PtermFrameParent = new PtermMainFrame ();
    PtermFrameParent->Show (false);
#endif

    // create the main application window
    // If arguments are present, always connect without asking
    if (!DoConnect (!(m_connect || argc > 1)))
    {
        return false;
    }
    
    // Add some handlers so we can save the screen in various formats
    // Note that the BMP handler is always loaded, don't do it again.
    wxImage::AddHandler (new wxPNGHandler);
    wxImage::AddHandler (new wxPNMHandler);
    wxImage::AddHandler (new wxTIFFHandler);
    wxImage::AddHandler (new wxXPMHandler);

    // success: wxApp::OnRun () will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

int PtermApp::OnExit (void)
{
    wxTheClipboard->Flush ();

    m_config->Write (wxT (PREF_XPOS), lastX);
    m_config->Write (wxT (PREF_YPOS), lastY);
    m_config->Flush ();

    delete g_printData;
    delete g_pageSetupData;
#ifdef DEBUGLOG
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
    int x, y;

    if (ask)
    {
        PtermConnDialog dlg (wxID_ANY, _("Connect to PLATO"));
    
        dlg.CenterOnScreen ();
        
        if (dlg.ShowModal () == wxID_OK)
        {
            if (dlg.m_host.IsEmpty ())
            {
                m_hostName = DEFAULTHOST;
            }
            else
            {
                m_hostName = dlg.m_host;
            }
            if (dlg.m_port.IsEmpty ())
            {
                m_port = DefNiuPort;
            }
            else
            {
                m_port = atoi (wxString (dlg.m_port).mb_str ());
            }
        }
        else
        {
            return false;     // connect canceled
        }
    }
    

    // create the main application window
    x = m_config->Read (wxT (PREF_XPOS), 0L);
    y = m_config->Read (wxT (PREF_YPOS), 0L);
    if (x == 0 && y == 0)
    {
        frame = new PtermFrame(m_hostName, m_port, wxT("Pterm"));
    }
    else
    {
        frame = new PtermFrame(m_hostName, m_port, wxT("Pterm"), 
                               wxPoint (x, y));
    }

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

void PtermApp::OnAbout(wxCommandEvent&)
{
    wxString msg;

    msg.Printf (_T("PLATO terminal emulator %s.\n%s"),
                wxT ("V" PTERMVERSION),
                _("Copyright \xA9 2004-2006 by Paul Koning."));
    
    wxMessageBox(msg, _("About Pterm"), wxOK | wxICON_INFORMATION, NULL);
}

void PtermApp::OnHelpKeys (wxCommandEvent &)
{
    PtermFrame *frame;
    wxString str;
    unsigned int i;

    if (m_helpFrame == NULL)
    {
        // If there isn't one yet, create a help window -- same as a
        // regular frame except that the data comes from here, not
        // from a connection.
        frame = new PtermFrame(str, -1, _("Keyboard Help"));

        if (frame != NULL)
        {
            if (m_firstFrame != NULL)
            {
                m_firstFrame->m_prevFrame = frame;
            }
            frame->m_nextFrame = m_firstFrame;
            m_firstFrame = frame;
            for (i = 0; i < sizeof (keyboardhelp) / sizeof (keyboardhelp[0]); i++)
            {
                frame->procPlatoWord (keyboardhelp[i], false);
            }
            m_helpFrame = frame;
        }
    }
    else
    {
        m_helpFrame->Show (true);
        m_helpFrame->Raise ();
    }
}

void PtermApp::OnPref (wxCommandEvent&)
{
    PtermFrame *frame;
    wxString rgb;
    
    PtermPrefDialog dlg (NULL, wxID_ANY, _("Pterm Preferences"));
    
    if (dlg.ShowModal () == wxID_OK)
    {
        m_platoKb = dlg.m_platoKb;
        m_beepEnable = dlg.m_beepEnable;
        for (frame = m_firstFrame; frame != NULL; frame = frame->m_nextFrame)
        {
            frame->UpdateSettings (dlg.m_fgColor, dlg.m_bgColor, dlg.m_scale2,
                                   dlg.m_showStatusBar);
        }

        m_fgColor = dlg.m_fgColor;
        m_bgColor = dlg.m_bgColor;
        
        m_scale = (dlg.m_scale2) ? 2 : 1;
        m_classicSpeed = dlg.m_classicSpeed;
        m_gswEnable = dlg.m_gswEnable;
        m_numpadArrows = dlg.m_numpadArrows;
        m_showStatusBar = dlg.m_showStatusBar;
        m_hostName = dlg.m_host;
        m_port = atoi (wxString (dlg.m_port).mb_str ());
        m_connect = dlg.m_connect;
        
        rgb.Printf (wxT ("%d %d %d"), 
                    dlg.m_fgColor.Red (), dlg.m_fgColor.Green (),
                    dlg.m_fgColor.Blue ());
        m_config->Write (wxT (PREF_FOREGROUND), rgb);
        rgb.Printf (wxT ("%d %d %d"),
                    dlg.m_bgColor.Red (), dlg.m_bgColor.Green (),
                    dlg.m_bgColor.Blue ());
        m_config->Write (wxT (PREF_BACKGROUND), rgb);
        m_config->Write (wxT (PREF_SCALE2), (dlg.m_scale2) ? 2 : 1);
        m_config->Write (wxT (PREF_HOST), dlg.m_host);
        m_config->Write (wxT (PREF_PORT), m_port);
        m_config->Write (wxT (PREF_1200BAUD), (dlg.m_classicSpeed) ? 1 : 0);
        m_config->Write (wxT (PREF_CONNECT), (dlg.m_connect) ? 1 : 0);
        m_config->Write (wxT (PREF_GSW), (dlg.m_gswEnable) ? 1 : 0);
        m_config->Write (wxT (PREF_ARROWS), (dlg.m_numpadArrows) ? 1 : 0);
        m_config->Write (wxT (PREF_STATUSBAR), (dlg.m_showStatusBar) ? 1 : 0);
        m_config->Write (wxT (PREF_PLATOKB), (dlg.m_platoKb) ? 1 : 0);
        m_config->Write (wxT (PREF_BEEP), (dlg.m_beepEnable) ? 1 : 0);
        m_config->Flush ();
    }
}

wxColour PtermApp::SelectColor (wxColour &initcol)
{
    wxColour col (initcol);
    wxColour orange (255, 144, 0);
    wxColour vikingGreen (0, 220, 0);
    wxColourData data;

    data.SetColour (initcol);
    data.SetCustomColour (0, orange);
    data.SetCustomColour (1, *wxBLACK);
    data.SetCustomColour (2, vikingGreen);
    
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
        frame->Close (true);
        frame = nextframe;
    }
#if PTERM_MDI // defined(__WXMAC__)
    // On the Mac, deleting all the windows doesn't terminate the
    // program, so we make it stop this way.
    ExitMainLoop ();
#endif
}


#if PTERM_MDI
// MDI parent frame
PtermMainFrame::PtermMainFrame (void)
    : wxMDIParentFrame (NULL, wxID_ANY, wxT ("Pterm"),
                        wxDefaultPosition, wxDefaultSize, 0)
{
#if wxUSE_MENUS
    // create a menu bar
    //
    // Note that the menu bar labels do not have shortcut markings,
    // because those conflict with the ALT-letter key codes for PLATO.
#if  defined(__WXGTK20__)
    // A rather ugly hack here.  GTK V2 insists that F10 should be the
    // accelerator for the menu bar.  We don't want that.  There is
    // no sane way to turn this off, but we *can* get the same effect
    // by setting the "menu bar accelerator" property to the name of a
    // function key that is apparently legal, but doesn't really exist.
    // (Or if it does, it certainly isn't a key we use.)
    gtk_settings_set_string_property (gtk_settings_get_default (),
                                      "gtk-menu-bar-accel", "F15", "foo");

#endif

    wxMenu *menuFile = new wxMenu;
    menuFile->Append (Pterm_Connect, _("New Connection...\tCtrl-N"),
                      _("Connect to a PLATO host"));
    menuFile->Append (Pterm_Pref, _("Preferences..."),
                      _("Set program configuration"));
    menuFile->AppendSeparator ();
    menuFile->Append (Pterm_Quit, _("Exit"), _("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append (menuFile, _("File"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Pterm_About, _("About Pterm"), _("Show about dialog"));
    helpMenu->Append(Pterm_HelpKeys, _("Pterm keyboard"), _("Show keyboard description"));
    
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
}
#endif

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
PtermFrame::PtermFrame(wxString &host, int port, const wxString& title,
                       const wxPoint &pos)
    : PtermFrameBase(PtermFrameParent, -1, title,
                     pos,
                     wxDefaultSize),
      tracePterm (false),
      m_nextFrame (NULL),
      m_prevFrame (NULL),
      m_foregroundPen (ptermApp->m_fgColor, ptermApp->m_scale, wxSOLID),
      m_backgroundPen (ptermApp->m_bgColor, ptermApp->m_scale, wxSOLID),
      m_foregroundBrush (ptermApp->m_fgColor, wxSOLID),
      m_backgroundBrush (ptermApp->m_bgColor, wxSOLID),
      m_canvas (NULL),
      m_conn (NULL),
      m_port (port),
      m_timer (this, Pterm_Timer),
      m_fullScreen (false),
      m_station (0),
      m_pasteTimer (this, Pterm_PasteTimer),
      m_pasteIndex (-1),
	  m_bCancelPaste (false),
	  m_bPasteActive (false),
      m_nextword (0),
      m_delay (0),
      currentX (0),
      currentY (496),
      memaddr (0),
      memlpc (0),
      wc (0),
      seq (0),
      modewords (0),
      m_dumbTty (true),
      m_ascState (none),
      m_ascBytes (0),
      lastX (0),
      lastY (0),
      m_flowCtrl (false),
      m_sendFgt (false),
      m_defFg (ptermApp->m_fgColor),
      m_defBg (ptermApp->m_bgColor),
      m_currentFg (ptermApp->m_fgColor),
      m_currentBg (ptermApp->m_bgColor),
      m_pendingEcho (-1)
{
    int i;

    if (tracePterm)
    {
        traceF = stdout;
    }
    
    m_hostName = host;
    mode = 017;             // default to character mode, rewrite
    setMargin (0);
    RAM[M_CCR] = 0;
    
    // Set default character set origins (for PPT that is; ASCII is different)
    RAM[C2ORIGIN] = M2ADDR;
    RAM[C2ORIGIN + 1] = M2ADDR >> 8;
    RAM[C3ORIGIN] = M3ADDR;
    RAM[C3ORIGIN + 1] = M3ADDR >> 8;
    
    // set the frame icon
    SetIcon(wxICON(pterm_32));

    // set the line style to projecting
    m_foregroundPen.SetCap (wxCAP_PROJECTING);
    m_backgroundPen.SetCap (wxCAP_PROJECTING);

#if wxUSE_MENUS
    // create a menu bar
    //
    // Note that the menu bar labels do not have shortcut markings,
    // because those conflict with the ALT-letter key codes for PLATO.
#if  defined(__WXGTK20__)
    // A rather ugly hack here.  GTK V2 insists that F10 should be the
    // accelerator for the menu bar.  We don't want that.  There is
    // no sane way to turn this off, but we *can* get the same effect
    // by setting the "menu bar accelerator" property to the name of a
    // function key that is apparently legal, but doesn't really exist.
    // (Or if it does, it certainly isn't a key we use.)
    gtk_settings_set_string_property (gtk_settings_get_default (),
                                      "gtk-menu-bar-accel", "F15", "foo");

#endif

    wxMenu *menuFile = new wxMenu;
    menuFile->Append (Pterm_Connect, _("New Connection...\tCtrl-N"),
                      _("Connect to a PLATO host"));
    if (port > 0)
    {
        // No "connect again" for the help window because that
        // doesn't own a connection.
        menuFile->Append (Pterm_ConnectAgain, _("Connect Again"),
                          _("Connect to the same host"));
        menuFile->AppendSeparator();
    }
    // The accelerators actually will be Command-xxx on the Mac;
    // on other platforms they are omitted since they clash with
    // the PLATO keyboard.
    menuFile->Append (Pterm_SaveScreen, _("Save Screen") MACACCEL ("\tCtrl-S"),
                      _("Save screen image to file"));
    menuFile->Append (Pterm_Print, _("Print...") MACACCEL ("\tCtrl-P"),
                      _("Print screen content"));
    menuFile->Append (Pterm_Page_Setup, _("Page Setup..."), _("Printout page setup"));
    menuFile->Append (Pterm_Preview, _("Print Preview"), _("Preview screen print"));
    menuFile->AppendSeparator ();
    menuFile->Append (Pterm_Pref, _("Preferences..."),
                      _("Set program configuration"));
    menuFile->AppendSeparator ();
    menuFile->Append (Pterm_Close, _("Close\tCtrl-W"),
                      _("Close this window"));
    menuFile->Append (Pterm_Quit, _("Exit"), _("Quit this program"));

    wxMenu *menuEdit = new wxMenu;

    menuEdit->Append (Pterm_CopyScreen, _("Copy Screen"), _("Copy screen to clipboard"));
    menuEdit->Append(Pterm_Copy, _("Copy text") MACACCEL ("\tCtrl-C"),
                     _("Copy text only to clipboard"));
    if (port > 0)
    {
        menuEdit->Append(Pterm_Paste, _("Paste ASCII") MACACCEL ("\tCtrl-V"),
                         _("Paste plain text"));
        // No "paste" for help window because it doesn't do input.
        menuEdit->Append(Pterm_PastePrint, _("Paste Printout"),
                         _("Paste Cyber printout format"));
    }

    // Copy is initially disabled, until a region is selected
    menuEdit->Enable (Pterm_Copy, false);

    wxMenu *menuView = new wxMenu;
    
    menuView->Append (Pterm_FullScreen, _("Full Screen\tCtrl-U"),
                      _("Display in full screen mode"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Pterm_About, _("About Pterm"),
                     _("Show about dialog"));
    helpMenu->Append(Pterm_HelpKeys, _("Pterm keyboard"),
                     _("Show keyboard description"));
    
    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append (menuFile, _("File"));
    menuBar->Append (menuEdit, _("Edit"));
#if !0//PTERM_MDI
    menuBar->Append (menuView, _("View"));
#endif

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

    if (port > 0)
    {
        // create a status bar, if this isn't a help window
        m_statusBar = new wxStatusBar (this, wxID_ANY);
        m_statusBar->SetFieldsCount (STATUSPANES);
        m_statusBar->SetStatusText(_(" Connecting..."), STATUS_CONN);
        if (ptermApp->m_showStatusBar)
        {
            SetStatusBar (m_statusBar);
        }
        ptermShowTrace ();
        SetCursor (*wxHOURGLASS_CURSOR);
    }

    for (i = 0; i < 5; i++)
    {
        m_charDC[i] = new wxMemoryDC ();
        m_charmap[i] = new wxBitmap (CharXSize, CharYSize, -1);
        m_charDC[i]->SelectObject (*m_charmap[i]);
    }
    m_bitmap = new wxBitmap (ScreenSize, ScreenSize, -1);
    m_memDC = new wxMemoryDC ();
    m_memDC->BeginDrawing ();
    m_memDC->SelectObject (*m_bitmap);
    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->Clear ();
    m_memDC->EndDrawing ();

    SetClientSize (XSize + 2, YSize + 2);
    m_canvas = new PtermCanvas (this);

    /*
    **  Load Plato ROM characters
    */
    ptermLoadRomChars ();
    
    if (port > 0)
    {
        // Create and start the network processing thread
        m_conn = new PtermConnection (this, host, port);
        if (m_conn->Create () != wxTHREAD_NO_ERROR)
        {
            return;
        }
        m_conn->Run ();
    }
    
    Show(true);
}

PtermFrame::~PtermFrame ()
{
    int i;
    
    if (m_conn != NULL)
    {
        delete m_conn;
    }
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

    // Do nothing for the help window
    if (m_conn == NULL)
    {
        return;
    }
    
    // If our timer is running, we're using the timer event to drive
    // the display, so ignore idle events.
    if (m_timer.IsRunning ())
    {
        event.Skip ();
        return;
    }
    
    if (m_nextword != 0)
    {
        procPlatoWord (m_nextword, m_conn->Ascii ());
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

        // See if we're supposed to delay (but it's not an internal
        // code such as NODATA)
        if ((int) word >= 0 && (word >> 19) != 0)
        {
            m_delay = word >> 19;
            m_nextword = word & 01777777;
            if (m_conn->Ascii ())
            {
                m_timer.Start (8);  // 16.67 / 2.2, rounded
            }
            else
            {
                m_timer.Start (17);
            }
            event.Skip ();
            return;
        }
            
#ifdef DEBUGLOG
        wxLogMessage ("processing data from plato %07o", word);
#elif DEBUG
        printf ("processing data from plato %07o\n", word);
#endif
        procPlatoWord (word, m_conn->Ascii ());
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
#ifdef DEBUGLOG
        wxLogMessage ("processing data from plato %07o", m_nextword);
#elif DEBUG
        printf ("processing data from plato %07o\n", m_nextword);
#endif
        procPlatoWord (m_nextword, m_conn->Ascii ());
    
    // See what's next.  If no delay is called for, just process it, keep
    // going until no more data or we get a word with delay.
    for (;;)
    {
        word = m_conn->NextWord ();
    
        if (word == C_NODATA)
        {
            m_nextword = 0;
            m_timer.Stop ();
            return;
        }
        m_delay = (word >> 19);
        m_nextword = word & 01777777;
        if (m_delay != 0)
        {
            break;
        }
#ifdef DEBUGLOG
        wxLogMessage ("processing data from plato %07o", word);
#elif DEBUG
        printf ("processing data from plato %07o\n", word);
#endif
        procPlatoWord (word, m_conn->Ascii ());
    }
}

void PtermFrame::OnPasteTimer (wxTimerEvent &)
{
    wxChar c;
    int p, delay;
    unsigned int nextindex;
    int shift = 0;
    

	if (m_bCancelPaste ||
		m_pasteIndex < 0 ||
        m_pasteIndex >= (int) m_pasteText.Len ())
	{
		m_bCancelPaste = false;
		m_bPasteActive = false;
		return;
	}


    if (m_pasteIndex < 0 ||
        m_pasteIndex >= (int) m_pasteText.Len ())
    {
        m_pasteIndex = -1;
        return;
    }
    
    nextindex = m_pasteIndex;

    if (m_pastePrint)
    {
        while (m_pasteIndex < (int) m_pasteText.Len ())
        {
            c = m_pasteText[nextindex++];
            p = -1;

            // Pasting a printout string, with ' for shift and other fun stuff.
            if (c == wxT('\''))
            {
                if (shift)
                {
                    // Odd -- two shift codes in a row.  Send the first
                    // one as a standalone shift.
                    ptermSendKey (055);     // shift-Assign is shift code
                }
                shift = 040;
                continue;
            }
            if (c < (int) (sizeof (printoutToPlato) / sizeof (printoutToPlato[0])))
            {
                p = printoutToPlato[c];
            }
            if (p != -1)
            {
                // Look for a shift code preceding a character that is a shifted
                // character (like $), or an unshifted character whose shifted form
                // corresponds to a different printable character (like 4).  For
                // those cases, if we see a shift code, send that separately.
                // For example, '4 does not mean $, it means a shift code then 4
                // (which is an embedded mode change).
                if (shift != 0 &&
                    ((p & 040) != 0 || strchr ("012345689=", c) != NULL))
                {
                    ptermSendKey (055);     // shift-Assign is shift code
                    shift = 0;
                }
                ptermSendKey (p | shift);
            }
            else if (shift)
            {
                    ptermSendKey (055);     // shift-Assign is shift code
            }
            break;
        }
    }
    else
    {
        c = m_pasteText[nextindex++];
        p = -1;

        if (c <= wxT(' '))
        {
            // Control char or space -- most get ignored
            if (c == wxT ('\n'))
            {
                p = 026;        // NEXT
            }
            else if (c == wxT ('\t'))
            {
                p = 014;        // TAB
            }
            else if (c == wxT (' '))
            {
                p = 0100;       // space
            }
        }
        else if (c < (int) (sizeof (asciiToPlato) / sizeof (asciiToPlato[0])))
        {
            p = asciiToPlato[c];
        }

        if (p != -1)
        {
            ptermSendKey (p);
        }
    }
    
    if (nextindex < m_pasteText.Len ())
    {
        // Still more to do.  Update the index (which is cleared to -1
        // by ptermSendKey), then restart the timer with the
        // appropriate delay (char delay or line delay).
        m_pasteIndex = nextindex;
        if (c == wxT ('\n'))
        {
            delay = PASTE_LINEDELAY;
        }
        else
        {
            delay = PASTE_CHARDELAY;
        }
        m_pasteTimer.Start (delay, true);
		m_bPasteActive = true;
    }
    else
    {
		m_bCancelPaste = false;
		m_bPasteActive = false;
    }
}

void PtermFrame::OnClose (wxCloseEvent &)
{
    int i;
    
    if (m_conn != NULL)
    {
        m_conn->Delete ();
    }

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
    
    if (this == ptermApp->m_helpFrame)
    {
        ptermApp->m_helpFrame = NULL;
    }
    else
    {
        GetPosition (&ptermApp->lastX, &ptermApp->lastY);
    }
    
    Destroy ();
}

void PtermFrame::OnQuit(wxCommandEvent&)
{
    // true is to force the frame to close
    Close (true);
}

void PtermFrame::OnActivate (wxActivateEvent &event)
{
    if (m_canvas != NULL)
    {
        m_canvas->SetFocus ();
    }
    event.Skip ();        // let others see the event, too
}

void PtermFrame::OnCopyScreen (wxCommandEvent &)
{
    wxBitmap screenmap (ScreenSize, ScreenSize);
    wxMemoryDC screenDC;
    wxBitmapDataObject *screen;

    screenDC.SelectObject (screenmap);
    screenDC.Blit (0, 0, ScreenSize, ScreenSize, 
                   m_memDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);

    screen = new wxBitmapDataObject(screenmap);

    if (wxTheClipboard->Open ())
    {
        if (!wxTheClipboard->SetData (screen))
        {
            wxLogError (_("Can't copy image to the clipboard"));
        }
        wxTheClipboard->Close ();
    }
    else
    {
        wxLogError (_("Can't open clipboard."));
    }
}

void PtermFrame::OnCopy (wxCommandEvent &event)
{
    m_canvas->OnCopy (event);
}

void PtermFrame::OnPaste (wxCommandEvent &event)
{
    if (!wxTheClipboard->Open ())
    {
        wxLogError (_("Can't open clipboard."));

        return;
    }

    if (!wxTheClipboard->IsSupported (wxDF_TEXT))
    {
        wxLogWarning (_("No text data on clipboard"));

        wxTheClipboard->Close ();
        return;
    }

    wxTextDataObject text;

    if (!wxTheClipboard->GetData (text))
    {
        wxLogError (_("Can't paste data from the clipboard"));
    }
    else
    {
        m_pasteText = text.GetText ();
        m_pasteIndex = 0;
        m_pasteTimer.Start (PASTE_CHARDELAY, true);
        m_pastePrint = (event.GetId () == Pterm_PastePrint);
    }

    wxTheClipboard->Close ();
}

void PtermFrame::OnUpdateUIPaste (wxUpdateUIEvent& event)
{
#ifdef __WXDEBUG__
    // too many trace messages if we don't do it - this function is called
    // very often
    wxLogNull nolog;
#endif

    event.Enable (wxTheClipboard->IsSupported (wxDF_TEXT));
}

void PtermFrame::OnSaveScreen (wxCommandEvent &)
{
    wxBitmap screenmap (ScreenSize, ScreenSize);
    wxMemoryDC screenDC;
    wxString filename, ext;
    wxBitmapType type;
    wxFileDialog fd (this, _("Save screen to"), ptermApp->m_defDir,
                     wxT(""), wxT("PNG files (*.png)|*.png|"
                                  "BMP files (*.bmp)|*.bmp|"
                                  "PNM files (*.pnm)|*.pnm|"
                                  "TIF files (*.tif)|*.tif|"
                                  "XPM files (*.xpm)|*.xpm|"
                                  "All files (*.*)|*.*"),
                     wxSAVE | wxOVERWRITE_PROMPT);
    
    if (fd.ShowModal () != wxID_OK)
    {
        return;
    }
    filename = fd.GetPath ();
    
    screenDC.SelectObject (screenmap);
    screenDC.Blit (0, 0, ScreenSize, ScreenSize, 
                   m_memDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);

    wxImage screenImage = screenmap.ConvertToImage ();
    wxFileName fn (filename);
    
    ptermApp->m_defDir = fn.GetPath ();
    ext = fn.GetExt ();
    if (ext.CmpNoCase (wxT ("bmp")) == 0)
    {
        type = wxBITMAP_TYPE_BMP;
    }
    else if (ext.CmpNoCase (wxT ("png")) == 0)
    {
        type = wxBITMAP_TYPE_PNG;
    }
    else if (ext.CmpNoCase (wxT ("pnm")) == 0)
    {
        type = wxBITMAP_TYPE_PNM;
    }
    else if (ext.CmpNoCase (wxT ("tif")) == 0 ||
             ext.CmpNoCase (wxT ("tiff")) == 0)
    {
        type = wxBITMAP_TYPE_TIF;
	// 32773 is PACKBITS -- not referenced symbolically because tiff.h
	// isn't necessarily anywhere, for some reason.
        screenImage.SetOption (wxIMAGE_OPTION_COMPRESSION, 32773);
    }
    else if (ext.CmpNoCase (wxT ("xpm")) == 0)
    {
        type = wxBITMAP_TYPE_XPM;
    }
    else
    {
        screenImage.SaveFile (filename);
        return;
    }
    
    screenImage.SaveFile (filename, type);
}

void PtermFrame::OnPrint (wxCommandEvent &)
{
    wxPrintDialogData printDialogData (*g_printData);

    printDialogData.EnableSelection (false);
    printDialogData.EnablePageNumbers (false);
    
    wxPrinter printer (& printDialogData);
    PtermPrintout printout (this);
    if (!printer.Print (this, &printout, true /*prompt*/))
    {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            wxMessageBox (_("There was a problem printing.\nPerhaps your current printer is not set correctly?"), _("Printing"), wxOK);
    }
    else
    {
        (*g_printData) = printer.GetPrintDialogData ().GetPrintData ();
    }
}

void PtermFrame::OnPrintPreview (wxCommandEvent &)
{
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData (*g_printData);
    wxPrintPreview *preview = new wxPrintPreview (new PtermPrintout (this),
                                                  new PtermPrintout (this),
                                                  &printDialogData);

    printDialogData.EnableSelection (false);
    printDialogData.EnablePageNumbers (false);
    
    if (!preview->Ok())
    {
        delete preview;
        wxMessageBox(_("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _("Previewing"), wxOK);
        return;
    }

    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _("Pterm Print Preview"), wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show();
}

void PtermFrame::OnPageSetup(wxCommandEvent &)
{
    (*g_pageSetupData) = *g_printData;

    wxPageSetupDialog pageSetupDialog (this, g_pageSetupData);
    pageSetupDialog.ShowModal ();

    (*g_printData) = pageSetupDialog.GetPageSetupData ().GetPrintData ();
    (*g_pageSetupData) = pageSetupDialog.GetPageSetupData ();
}

void PtermFrame::OnFullScreen (wxCommandEvent &)
{
    m_fullScreen = !m_fullScreen;
    
#if 0 //PTERM_MDI
    // workaround for a bug in the MDI case
    if (m_fullScreen)
    
    {
        SetStatusBar (NULL);
    }
    else if (ptermApp->m_showStatusBar)
    {
        SetStatusBar (m_statusBar);
    }
#endif

        if (m_fullScreen)
        {
            m_canvas->SetScrollRate (0, 0);
        }
        else
        {
            m_canvas->SetScrollRate (1, 1);
        }
    
    ShowFullScreen (m_fullScreen);
    m_canvas->Refresh ();
}

void PtermFrame::PrepareDC(wxDC& dc)
{
    dc.SetAxisOrientation (true, false);
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
    
    m_canvas->SaveChar (x, y, snum, cnum, wemode, large);
    
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
        savemode = mode;
        
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
                    mode = savemode ^ 1;
                }
                else
                {
                    mode = savemode;
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
        mode = savemode;
    }
}

void PtermFrame::ptermDrawPoint (int x, int y)
{
    wxClientDC dc(m_canvas);
    int xm, ym;
    
    dc.BeginDrawing ();
    m_memDC->BeginDrawing ();
    PrepareDC (dc);
    xm = XMADJUST (x & 0777);
    ym = YMADJUST (y & 0777);
    x = XADJUST (x & 0777);
    y = YADJUST (y & 0777);
    if (mode & 1)
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
    m_memDC->DrawPoint (xm, ym);
    if (ptermApp->m_scale == 2)
    {
        dc.DrawPoint (x + 1, y);
        m_memDC->DrawPoint (xm + 1, ym);
        dc.DrawPoint (x, y - 1);
        m_memDC->DrawPoint (xm, ym - 1);
        dc.DrawPoint (x + 1, y - 1);
        m_memDC->DrawPoint (xm + 1, ym - 1);
    }    
    dc.EndDrawing ();
    m_memDC->EndDrawing ();
}

void PtermFrame::ptermDrawLine(int x1, int y1, int x2, int y2)
{
    int xm1, ym1, xm2, ym2;
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    m_memDC->BeginDrawing ();
    PrepareDC (dc);
    xm1 = XMADJUST (x1);
    ym1 = YMADJUST (y1);
    xm2 = XMADJUST (x2);
    ym2 = YMADJUST (y2);
    x1 = XADJUST (x1);
    y1 = YADJUST (y1);
    x2 = XADJUST (x2);
    y2 = YADJUST (y2);
    if (mode & 1)
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
    m_memDC->DrawLine (xm1, ym1, xm2, ym2);
    if (ptermApp->m_scale == 2)
    {
        dc.DrawLine (x2, y2, x1, y1);
        m_memDC->DrawLine (xm2, ym2, xm1, ym1);
    }
    else
    {
        dc.DrawPoint (x2, y2);
        m_memDC->DrawPoint (xm2, ym2);
    }
    dc.EndDrawing ();
    m_memDC->EndDrawing ();
}

void PtermFrame::ptermFullErase (void)
{
    wxClientDC dc(m_canvas);

    m_canvas->FullErase ();
    dc.BeginDrawing ();
    PrepareDC (dc);
    dc.SetPen (m_backgroundPen);
    dc.SetBrush (m_backgroundBrush);
    dc.DrawRectangle (XTOP, YTOP, 
                      512 * ptermApp->m_scale, 
                      512 * ptermApp->m_scale);
    dc.EndDrawing ();
    m_memDC->BeginDrawing ();
    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->Clear ();
    m_memDC->EndDrawing ();
}

void PtermFrame::ptermBlockErase (int x1, int y1, int x2, int y2)
{
    int t;
    int xm1, ym1, xm2, ym2;
    wxClientDC dc(m_canvas);

    dc.BeginDrawing ();
    m_memDC->BeginDrawing ();
    PrepareDC (dc);
    xm1 = XMADJUST (x1);
    ym1 = YMADJUST (y1);
    xm2 = XMADJUST (x2);
    ym2 = YMADJUST (y2);
    x1 = XADJUST (x1);
    y1 = YADJUST (y1);
    x2 = XADJUST (x2);
    y2 = YADJUST (y2);
    if (x1 > x2)
    {
        t = x1;
        x1 = x2;
        x2 = t;
        t = xm1;
        xm1 = xm2;
        xm2 = t;
    }
    if (y1 > y2)
    {
        t = y1;
        y1 = y2;
        y2 = t;
        t = ym1;
        ym1 = ym2;
        ym2 = t;
    }
    if (mode & 1)
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
    m_memDC->DrawRectangle (xm1, ym1,
                            xm2 - xm1 + ptermApp->m_scale,
                            ym2 - ym1 + ptermApp->m_scale);
    dc.EndDrawing ();
    m_memDC->EndDrawing ();
}

void PtermFrame::ptermPaint (int pat)
{
    wxClientDC dc(m_canvas);
    int xm, ym;
    
    dc.BeginDrawing ();
    m_memDC->BeginDrawing ();
    PrepareDC (dc);
    
    xm = XMADJUST (currentX);
    ym = YMADJUST (currentY);

    m_memDC->SetBrush (m_foregroundBrush);
    m_memDC->FloodFill (xm, ym, m_currentBg, wxFLOOD_BORDER);
    dc.Blit (XTOP, YTOP, ScreenSize, ScreenSize,
             m_memDC, 0, 0, wxCOPY);
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
    m_statusBar->SetStatusText(str, STATUS_CONN);
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
    unsigned int i;

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

void PtermFrame::UpdateSettings (wxColour &newfg, wxColour &newbg,
                                 bool newscale2, bool newstatusbar)
{
    const bool recolor = (ptermApp->m_fgColor != newfg ||
                          ptermApp->m_bgColor != newbg);
    const int newscale = (newscale2) ? 2 : 1;
    const bool rescale = (newscale != ptermApp->m_scale);
    int i;
    wxBitmap *newmap;

    if (m_conn != NULL)
    {
        if (newstatusbar)
        {
            SetStatusBar (m_statusBar);
        }
        else
        {
            SetStatusBar (NULL);
        }
    }
    ptermShowTrace ();
    
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
        if (!m_fullScreen)
        {
            m_canvas->SetScrollRate (1, 1);
        }
        dc.BeginDrawing ();
        dc.DestroyClippingRegion ();
        dc.SetClippingRegion (GetXMargin (), GetYMargin (),
                              vScreenSize (newscale), vScreenSize (newscale));
        dc.EndDrawing ();
        
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
    SetColors (newfg, newbg, newscale);
    m_canvas->SetBackgroundColour (newbg);
    m_canvas->Refresh ();
    m_defFg = newfg;
    m_defBg = newbg;
}

void PtermFrame::SetColors (wxColour &newfg, wxColour &newbg, int newscale)
{
    TRACE6 ("fg: %d %d %d; bg: %d %d %d",
            newfg.Red(), newfg.Green(), newfg.Blue(),
            newbg.Red(), newbg.Green(), newbg.Blue());
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
    u8 *odata, *ndata, *newbits = NULL;
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
    int charX, charY, sizeX, sizeY, screenX, screenY, memX, memY;

    charX = cnum * 8 * ptermApp->m_scale;
    charY = snum * 16 * ptermApp->m_scale;
    sizeX = 8;
    sizeY = 16;

    screenX = XADJUST (x);
    screenY = YADJUST (y + 15);
    memX = XMADJUST (x);
    memY = YMADJUST (y + 15);
    
    if (x < 0)
    {
        sizeX += x;
        charX -= x * ptermApp->m_scale;
        screenX = XADJUST (0);
        memX = XMADJUST (0);
    }
    if (y < 0)
    {
        sizeY += y;
    }
    if (mode & 2)
    {
        // write or erase -- need to zap old pixels and OR in new pixels
        m_memDC->Blit (memX, memY,
                       sizeX * ptermApp->m_scale, sizeY * ptermApp->m_scale, 
                       m_charDC[4], charX, charY, wxAND);
        m_memDC->Blit (memX, memY,
                       sizeX * ptermApp->m_scale, sizeY * ptermApp->m_scale, 
                       m_charDC[wemode], charX, charY, wxOR);
    }
    else
    {
        // inverse or rewrite, just blit in the appropriate pattern
        m_memDC->Blit (memX, memY,
                       sizeX * ptermApp->m_scale, sizeY * ptermApp->m_scale, 
                       m_charDC[wemode], charX, charY, wxCOPY);
    }
        
    // Now copy the resulting state of the character area into the screen dc
    dc.Blit (screenX, screenY,
             sizeX * ptermApp->m_scale, sizeY * ptermApp->m_scale,
             m_memDC, memX, memY, wxCOPY);
    
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
**                  ascii       true if using ASCII protocol
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::procPlatoWord (u32 d, bool ascii)
{
    mptr mp;
    char *msg = "";
    int i, j, n;
    AscState    ascState;

    // used in load coordinate
    int &coord = (d & 01000) ? currentY : currentX;
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
    supdelta = (deltay / 16) * 5;
    
    seq++;
    if (tracePterm)
    {
        fprintf (traceF, "%07o ", d);
    }
    if (ascii)
    {
#ifdef DEBUGASCII
        if (!m_dumbTty)
        {
            int c = ((d & 127) >= 32 && (d & 127) < 127) ? d & 127 : '.';
            printf ("%04x  %c\n", d, c);
        }
#endif
        if (m_dumbTty)
        {
            if (d == (033 << 8) + 002)   // ESC STX
            {
                TRACEN ("Entering PLATO terminal mode");
                m_dumbTty = false;
                mode = (3 << 2) + 1;    // set character mode, rewrite
                ptermSetStation (-1);   // Show connected in ASCII mode
            }
            else if ((d >> 8) == 0)
            {
                if (d >= 32 && d < 127)
                {
                    d = asciiM0[d];
                    if ((d & 0xf0) != 0xf0)
                    {
                        // Force mode rewrite
                        mode = (3 << 2) + 1;
                        ptermDrawChar (currentX, currentY, 
                                       (d & 0x80) >> 7, d & 0x7f);
                        currentX = (currentX + 8) & 0777;
                    }
                }
                else if (d == 015)
                {
                    currentX = 0;
                }
                else if (d == 012)
                {
                    wxClientDC dc(m_canvas);
                    dc.BeginDrawing ();
                    m_memDC->BeginDrawing ();
                    PrepareDC (dc);
                    m_memDC->SetBackground (m_backgroundBrush);
                    if (currentY != 0)
                    {
                        currentY -= 16;
                    }
                    else
                    {
                        // On the bottom line... scroll.
                        dc.Blit (XTOP, YTOP, ScreenSize,
                                 ScreenSize - (16 * ptermApp->m_scale),
                                 m_memDC, XMTOP, YMADJUST (496),
                                 wxCOPY);
                        m_memDC->Blit (XMTOP, YMTOP, ScreenSize,
                                       ScreenSize - (16 * ptermApp->m_scale),
                                       &dc, XTOP, YTOP, wxCOPY);
                    }
                    // Erase the line we just moved to.
                    m_memDC->Blit (XMTOP, YMADJUST (16) + 1, ScreenSize,
                                   16 * ptermApp->m_scale,
                                   m_memDC, 0, 0, wxCLEAR);
                    dc.Blit (XTOP, YADJUST (16) + 1, ScreenSize,
                             16 * ptermApp->m_scale,
                             &dc, 0, 0, wxCLEAR);
                    dc.EndDrawing ();
                    m_memDC->EndDrawing ();
                }
            }
        }
        else if (m_ascState == pni_rs)
        {
            // We just want to ignore 3 command codes.  Note that escape
            // sequences count for one, not two.
            if (++m_ascBytes == 3)
            {
                m_ascBytes = 0;
                m_ascState = none;
            }
        }
        else if ((d >> 8) == 033)
        {
            // Escape sequence, the second character is in the low byte
            d &= 0377;
            switch (d)
            {
            case 002:   // ESC STX
                TRACEN ("Still in PLATO terminal mode");
                m_dumbTty = false;
                break;
            case 003:   // ESC ETX
                TRACEN ("Leaving PLATO terminal mode");
                m_dumbTty = true;
                m_flowCtrl = false;
                m_sendFgt = false;
                currentX = 0;
                currentY = 496;
                break;
            case 014:   // ESC FF
                TRACEN ("Full screen erase");
                ptermFullErase ();
                break;
            case 021:   // ESC DC1
            case 022:   // ESC DC2
            case 023:   // ESC DC3
            case 024:   // ESC DC4
                // modes inverse, write, erase, rewrite
                mode = (mode & ~3) + ascmode[d - 021];
                //mode = 017;
                TRACE ("load mode %d", mode);
                break;
            case '2':
                // Load coordinate
                TRACEN ("Start load coordinate");
                m_ascState = ldc;
                m_ascBytes = 0;
                break;
            case '@':
                // superscript
                TRACEN ("Superscript");
                cy = (cy + supdelta) & 0777;
                break;
            case 'A':
                // subscript
                TRACEN ("Subcript");
                cy = (cy - supdelta) & 0777;
                break;
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
                TRACE ("select memory M%d", d - 'B');
                setCmem (d - 'B');
                break;
            case 'J':
                TRACEN ("Horizontal writing mode");
                setVertical (false);
                break;
            case 'K':
                TRACEN ("Vertical writing mode");
                setVertical (true);
                break;
            case 'L':
                TRACEN ("Forward writing mode");
                setReverse (false);
                break;
            case 'M':
                TRACEN ("Reverse writing mode");
                setReverse (true);
                break;
            case 'N':
                TRACEN ("Normal size writing mode");
                setLarge (false);
                break;
            case 'O':
                TRACEN ("Double size writing mode");
                setLarge (true);
                break;
            case 'P':
                mode = 2 << 2;
                TRACE ("load mode %d", mode);
                break;
            case 'Q':
                m_ascState = ssf;
                m_ascBytes = 0;
                TRACEN ("Start SSF");
                break;
            case 'R':
                // external data
                m_ascState = ext;
                m_ascBytes = 0;
                TRACEN ("Start ext");
                break;
            case 'S':
                mode = 2 << 2;
                TRACE ("load mode %d", mode);
                break;
            case 'T':
                mode = 5 << 2;
                TRACE ("load mode %d", mode);
                break;
            case 'U':
                mode = 6 << 2;
                TRACE ("load mode %d", mode);
                break;
            case 'V':
                mode = 7 << 2;
                TRACE ("load mode %d", mode);
                break;
            case 'W':
                // Load memory address
                TRACEN ("Start LDA");
                m_ascState = lda;
                m_ascBytes = 0;
                break;
            case 'Y':
                // load echo
                TRACEN ("Start LDE");
                m_ascState = lde;
                m_ascBytes = 0;
                break;
            case 'Z':
                // set margin
				TRACE ("set margin %d", currentX);
                setMargin (cx);
                break;
            case 'a':
                // set foreground color
                TRACEN ("Start foreground color");
                m_ascState = fg;
                m_ascBytes = 0;
                break;
            case 'b':
                // set background color
                TRACEN ("Start background color");
                m_ascState = bg;
                m_ascBytes = 0;
                break;
            case 'c':
                // paint
                TRACEN ("Start paint");
                m_ascState = paint;
                m_ascBytes = 0;
                break;
            }
        }
        else
        {
            switch (d)
            {
            case 010:   // backspace
                cx = (cx - deltax) & 0777;
                TRACEN ("backspace");
                break;
            case 011:   // tab
                TRACEN ("tab");
                cx = (cx + deltax) & 0777;
                break;
            case 012:   // linefeed
                TRACEN ("linefeed");
                cy = (cy - deltay) & 0777;
                break;
            case 013:   // vertical tab
                TRACEN ("vertical tab");
                cy = (cy - deltay) & 0777;
                break;
            case 014:   // form feed
                TRACEN ("form feed");
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
				TRACE2 ("CR to %d %d", currentX, currentY);
                break;
            case 031:   // EM
                mode = (mode & 3) + (4 << 2);
                modewords = 0;              // words since entering mode
                TRACE ("load mode %d", mode);
                break;
            case 034:   // FS
                mode = (mode & 3) + (0 << 2);
                TRACE ("load mode %d", mode);
                break;
            case 035:   // FS
                mode = (mode & 3) + (1 << 2);
				m_ascState = ldc;				// to have first coordinate be "dark"
                TRACE ("load mode %d", mode);
                break;
            case 036:   // RS -- used by PNI in connect handshake
                m_ascState = pni_rs;
                TRACEN ("pni start download, ignoring next 3 commands");
                break;
            case 037:   // FS
                mode = (mode & 3) + (3 << 2);
                TRACE ("load mode %d", mode);
                break;
            }
            if (d >= 040)
            {
                switch (m_ascState)
                {
                case ldc:
                    if (AssembleCoord (d))
                    {
                        currentX = lastX;
                        currentY = lastY;
                        TRACE2 ("load coordinate %d %d", currentX, currentY);
                    }
                    break;
                case paint:
                    n = AssemblePaint (d);
                    if (n != -1)
                    {
                        TRACE ("paint %03o", n);
                        ptermPaint (n);
                    }
                    break;
                case lde:
                    n = AssembleData (d);
                    if (n != -1)
                    {
                        n &= 0177;
                        switch (n)
                        {
                        case 0160:
                            // 160 is terminal type query
                            TRACE ("load echo termtype %d", ASCTYPE);
                            n = 0160 + ASCTYPE;
                            break;
                        case 0x71:
                            TRACE ("load echo subtype %d", SUBTYPE);
                            n = SUBTYPE;
                            break;
                        case 0x72:
                            TRACEN ("load echo loadfile (unused)");
                            n = 0;
                            break;
                        case 0x73:
                            // hex 73 is report terminal config
                            TRACEN ("load echo termdata");
                            n = TERMCONFIG;
                            break;
                        case 0x7b:
                            // hex 7b is beep
                            if (ptermApp->m_beepEnable)
                            {
                                TRACEN ("beep");
                                wxBell ();
                            }
                            break;
                        case 0x7d:
                            // hex 7d is report MAR
                            TRACE ("report MAR %o", memaddr);
                            n = memaddr;
                            break;
                        case 0x52:
                            // hex 52 is enable flow control
                            TRACEN ("enable flow control");
                            m_flowCtrl = true;
                            n = 0x53;
                            break;
                        case 0x60:
                            // hex 60 is inquire features
                            TRACE ("report features 0x%02x", ASCFEATURES);
                            n += ASCFEATURES;
                            m_sendFgt = true;
                            break;
                        default:
                            TRACE2 ("load echo %d (0x%02x)", n, n);
                        }
                        if (n == 0x7b)
                        {
                            break;          // -beep- does NOT send an echo code in reply
                        }
                        n += 0200;
                        if (m_conn->RingCount () > RINGXOFF1)
                        {
                            m_pendingEcho = n;
                        }
                        else
                        {
                            ptermSendKey (n);
                            m_pendingEcho = -1;
                        }
                    }
                    break;
                case lda:
                    n = AssembleData (d);
                    if (n != -1)
                    {
                        TRACE ("load memory address %04x", n);
                        memaddr = n & 077777;
                    }
                    break;
                case ext:
                    if (n != -1)
                    {
                        TRACE ("ext %04x", n);
                        // tbd
                    }
                    break;
                case ssf:
                    n = AssembleData (d);
                    if (n != -1)
                    {
                        TRACE ("ssf %04x", n);
                        m_canvas->ptermTouchPanel ((n & 0x20) != 0);
                    }
                    break;
                case fg:
                case bg:
                    ascState = m_ascState;
                    n = AssembleColor (d);
                    if (n != -1)
                    {
                        wxColour c ((n >> 16) & 0xff, (n >> 8) & 0xff, 
                                    n & 0xff);
                        if (ascState == fg)
                        {
                            TRACE ("set foreground color %06x", n);
                            m_currentFg = c;
                        }
                        else
                        {
                            TRACE ("set background color %06x", n);
                            m_currentBg = c;
                        }
                        SetColors (m_currentFg, m_currentBg, 
                                   ptermApp->m_scale);
                    }
                    break;
                case none:
                    switch (mode >> 2)
                    {
                    case 0:
                        if (AssembleCoord (d))
                        {
                            mode0 ((lastX << 9) + lastY);
                        }
                        break;
                    case 1:
                        if (AssembleCoord (d))
                        {
                            mode1 ((lastX << 9) + lastY);
                        }
                        break;
                    case 2:
                        n = AssembleData (d);
                        if (n != -1)
                        {
                            mode2 (n);
                        }
                        break;
                    case 3: // text mode
                        TRACE2 ("char %03o (%c)", d, d);
                        m_ascState = none;
                        m_ascBytes = 0;
                        i = currentCharset;
                        if (i == 0)
                        {
                            d = asciiM0[d];
                            // The ROM vs. RAM choice is given by the
                            // current character set.  
                            // For the ROM characters, the even vs. odd
                            // (M0 vs. M1) choice is given by the top bit
                            // of the ASCII translation table.
                            i = (d & 0x80) >> 7;
                        }
                        else if (i == 1)
                        {
                            d = asciiM1[d];
                            i = (d & 0x80) >> 7;
                        }
                        else
                        {
                            // RAM characters are indexed by printable
                            // ASCII characters; the RAM character offset
                            // is simply the character code - 32.
                            // The set choice is simply what the host sent.
                            d = (d - 040) & 077;
                        }
                        if ((d & 0xf0) == 0xf0)
                        {
                            if (d != 0xff)
                            {
                                // A builtin composite
                                int savemode = mode;
                                int sy = cy;
                                
                                d -= 0xf0;
                                for (i = 0; i < 8; i += 2)
                                {
                                    n = M1specials[d][i];
                                    if (n == 0)
                                    {
                                        break;
                                    }
                                    j = (i8) M1specials[d][i + 1];
                                    cy += j * ptermApp->m_scale;
                                    ptermDrawChar (currentX, currentY,
                                                   n >> 7, n & 0x7f);
                                    cy = sy;
                                    mode |= 2;
                                }
                                mode = savemode;
                                cx = (cx + deltax) & 0777;
                            }
                        }   
                        else 
                        {
                            ptermDrawChar (currentX, currentY,
                                           i, d & 0x7f);
                            cx = (cx + deltax) & 0777;
                        }
                        break;
                    case 4:
                        if (AssembleCoord (d))
                        {
                            modewords++;
                            mode4 ((lastX << 9) + lastY);
                        }
                        break;
                    case 5:
                        n = AssembleData (d);
                        if (n != -1)
                        {
                            mode5 (n);
                        }
                        break;
                    case 6:
                        n = AssembleData (d);
                        if (n != -1)
                        {
                            mode6 (n);
                        }
                        break;
                    case 7:
                        n = AssembleData (d);
                        if (n != -1)
                        {
                            mode7 (n);
                        }
                        break;
                    }
                    break;
                }
            }
        }
    }
    else
    {
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
            mp = modePtr[mode >> 2];
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
            
                mode = (d >> 1) & 037;
                if (d & 1)
                {
                    // full screen erase
                    ptermFullErase ();
                }
                TRACE2 ("load mode %d screen %d",
                        mode, (d & 1));
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
                    setMargin (coord);
                    msg = "margin";
                }
                TRACE3 ("load coord %c %d %s",
                        (d & 01000) ? 'Y' : 'X', d & 0777, msg);
                break;
            case 3:     // echo
                d &= 0177;
                switch (d)
                {
                case 0160:
                    // 160 is terminal type query
                    TRACE ("load echo termtype %d", TERMTYPE);
                    d = 0160 + TERMTYPE;
                    break;
                case 0x7b:
                    // hex 7b is beep
                    if (ptermApp->m_beepEnable)
                    {
                        TRACEN ("beep");
                        wxBell ();
                    }
                    break;
                case 0x7d:
                    // hex 7d is report MAR
                    TRACE ("report MAR %o", memaddr);
                    d = memaddr;
                    break;
                default:
                    TRACE ("load echo %d", d);
                }
                if (d == 0x7b)
                {
                    break;          // -beep- does NOT send an echo code in reply
                }
                d += 0200;
                if (m_conn->RingCount () > RINGXOFF1)
                {
                    m_pendingEcho = d;
                }
                else
                {
                    ptermSendKey (d);
                    m_pendingEcho = -1;
                }
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
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a 9 bit data word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the word
**
**------------------------------------------------------------------------*/
int PtermFrame::AssemblePaint (int d)
{
    if (m_ascBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_ascBytes * 6));
    if (++m_ascBytes == 2)
    {
        m_ascBytes = 0;
        m_ascState = none;
        TRACE2 ("paint %03o (0x%04x)", m_assembler, m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("paint byte %d: %d", m_ascBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble an 18 bit data word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the word
**
**------------------------------------------------------------------------*/
int PtermFrame::AssembleData (int d)
{
    if (m_ascBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_ascBytes * 6));
    if (++m_ascBytes == 3)
    {
        m_ascBytes = 0;
        m_ascState = none;
        TRACE2 ("data %07o (0x%04x)", m_assembler, m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("data byte %d: %d", m_ascBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a 24 bit color word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the color word
**
**------------------------------------------------------------------------*/
int PtermFrame::AssembleColor (int d)
{
    if (m_ascBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_ascBytes * 6));
    if (++m_ascBytes == 4)
    {
        m_ascBytes = 0;
        m_ascState = none;
        TRACE ("color 0x%06x)", m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("color byte %d: %d", m_ascBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a coordinate pair for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        true if word is complete, false if not
**                  lastX and lastY are the x/y coordinate received
**
**------------------------------------------------------------------------*/
bool PtermFrame::AssembleCoord (int d)
{
    int c = d & 037;
    
    switch (d >> 5)
    {
    case 1: // High X or high Y
        if (m_ascBytes == 0)
        {
            // High Y
            TRACE ("high Y %d", c);
            lastY = (lastY & 037) | (c << 5);
            m_ascBytes = 2;
        }
        else
        {
            TRACE ("high X %d", c);
            lastX = (lastX & 037) | (c << 5);
        }
        break;
    case 2:
        lastX = (lastX & 0740) | c;
        m_assembler = (lastX << 16) + lastY;
        m_ascBytes = 0;
        m_ascState = none;
        TRACE3 ("low X %d, coordinates %d %d", c, lastX, lastY);
        return true;
    case 3:
        TRACE ("low Y %d", c);
        lastY = (lastY & 0740) | c;
        m_ascBytes = 2;
        break;
    }
    return false;
}

/*--------------------------------------------------------------------------
**  Purpose:        Display visual indication of trace status
**
**  Parameters:     Name        Description.
**                  fileaction  true to open/flush files,
**                              false if only redisplay is needed
**
**  Returns:        nothing
**
**  This function opens or flushes the trace file depending on trace
**  setting, if fileaction is true.  It then redraws (on or off) the trace
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
    ptermShowTrace ();
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
        setUncover (true);
        return;
    }
    if (uncover)
    {
        supdelta = (deltay / 16) * 5;
        setUncover (false);
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
            setCmem (c - 020);
            break;
        case 030:   // horizontal writing
            setVertical (false);
            break;
        case 031:   // vertical writing
            setVertical (true);
            break;
        case 032:   // forward writing
            setReverse (false);
            break;
        case 033:   // reverse writing
            setReverse (true);
            break;
        case 034:   // normal size writing
            setLarge (false);
            break;
        case 035:   // double size writing
            setLarge (true);
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

    // Store the word in PPT RAM
    WriteRAM (memaddr, d);
    WriteRAM (memaddr + 1, d >> 8);

    // memaddr is a PPT RAM address; convert it to a character memory address
    chaddr = memaddr - ReadRAMW (C2ORIGIN);
    if (chaddr < 0 || chaddr > 127 * 16)
    {
        TRACE2 ("memdata %04x to %04x", d & 0xffff, memaddr);
    }
    else
    {
        chaddr /= 2;
        if (((d >> 16) & 3) == 0)
        {
            // load data
            TRACE2 ("character memdata %06o to char word %04o", d & 0xffff, chaddr);
            plato_m23[chaddr] = d & 0xffff;
            if ((++chaddr & 7) == 0)
            {
                // character is done -- load it to display 
                ch = (chaddr / 8) - 1;
                ptermLoadChar (2 + (ch / 64), ch % 64, &plato_m23[chaddr - 8]);
            }
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
            rom01char[(d >> 12) & 077], 
            rom01char[(d >> 6) & 077], 
            rom01char[d & 077]);
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
    progmode (d, M5ORIGIN);
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
    progmode (d, M6ORIGIN);
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
    progmode (d, M7ORIGIN);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 5/6/7 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**                  origin      Address of mode handler pointer
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::progmode (u32 d, int origin)
{
    // Load C/D/E with data word
    BC.reg.C = d >> 16;
    DE.reg.D = d >> 8;
    DE.reg.E = d;

    // Initialize the stack
    SP = INITSP;

    // Push the fake return address for "return to main loop" onto the
    // stack, as if we just did a CALL instruction
    WriteRAM(--SP, R_MAIN >> 8);
    WriteRAM(--SP, R_MAIN);

    // Set the start PC for the requested mode
    PC = ReadRAMW (origin);
    main8080a ();
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Plato mode keyboard input
**
**  Parameters:     Name        Description.
**                  key         Plato key code for station
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::ptermSendKey(int key)
{
    char data[5];
    int len;
    
    if (m_conn == NULL)
    {
        return;
    }
    

    /*
    **  If this is a "composite", recursively send the two pieces.
    */
    if ((key >> 9) != 0 && key != xonkey && key != xofkey)
    {
        // Don't do composite keys if ASCII and in dumb terminal mode
        if (!(m_conn->Ascii () && m_dumbTty))
        {
            ptermSendKey (key >> 9);
            if ((key >> 9) != 074)
            {
                ptermSendKey (074);
            }
            ptermSendKey (key & 0777);
        }
    }
    else
    {
        if (tracePterm)
        {
            fprintf (traceF, "key to plato %03o\n", key);
        }
#ifdef DEBUGLOG
        wxLogMessage ("key to plato %03o", key);
#elif DEBUGKEY
        printf ("key to plato %03o\n", key);
#endif
        if (m_conn->Ascii ())
        {
            // Assume one byte key code
            len = 1;
            if (key < 0200)
            {
                // Regular keyboard key
                key = asciiKeycodes[key];
                if (key == 0xff)
                {
                    return;
                }
                if (m_flowCtrl)
                {
                    // Do the keycode translation for the
                    // "flow control enabled" coding rules.
                    switch (key)
                    {
                    case 0x00:              // access
                        key = 0x1d;
                        len = 2;
                        break;
                    case 0x05:              // shift-sub
                        key = 0x04;
                        len = 2;
                        break;
                    case 0x0a:              // tab
                        key = 0x09;
                        break;
                    case 0x09:              // shift-help
                        key = 0x0a;
                        break;
                    case 0x11:              // shift-stop
                        key = 0x05;
                        break;
                    case 0x17:              // shift-super
                        len = 2;
                        // fall through
                    case 0x13:              // super
                        key = 0x17;
                        break;
                    case 0x7c:              // apostrophe
                        key = 0x27;
                        break;
                    case 0x27:              // #
                        key = 0x7c;
                        break;
                    }
                    data[0] = 033;          // store esc for 2 byte codes
                }                        
                data[len - 1] = Parity (key);
                if (tracePterm)
                {
                    if (len == 1)
                    {
                        fprintf (traceF, "ascii mode key to plato 0x%02x\n", data[0] & 0xff);
                    }
                    else
                    {
                        fprintf (traceF, "ascii mode key to plato 0x%02x 0x%02x\n", data[0], data[1] & 0xff);
                    }
                }
                m_conn->SendData (data, len);
                if (m_dumbTty)
                {
                    // do local echoing
                    m_conn->StoreWord (key);
                }
            }
            else if (!m_dumbTty)
            {
                if (key == xofkey)
                {
                    if (!m_flowCtrl)
                    {
                        return;
                    }
                    data[0] = Parity (ascxof);
                    if (tracePterm)
                    {
                        fprintf (traceF, "ascii mode key to plato XOFF\n");
                    }
                }
                else if (key == xonkey)
                {
                    if (!m_flowCtrl)
                    {
                        return;
                    }
                    data[0] = Parity (ascxon);
                    if (tracePterm)
                    {
                        fprintf (traceF, "ascii mode key to plato XON\n");
                    }
                }
                else
                {
                    len = 3;
                    data[0] = 033;
                    data[1] = Parity (0100 + (key & 077));
                    data[2] = Parity (0140 + (key >> 6));
                    if (tracePterm)
                    {
                        fprintf (traceF, "ascii mode key to plato 0x%02x 0x%02x 0x%02x\n", 
                                 data[0] & 0xff, data[1] & 0xff, data[2] & 0xff);
                    }
                }
                m_conn->SendData (data, len);
            }
        }
        else
        {
            data[0] = key >> 7;
            data[1] = 0200 | key;

            m_conn->SendData (data, 2);
        }
    }
}

void PtermFrame::ptermSendTouch (int x, int y)
{
    char data[6];

    if (m_sendFgt)
    {
        // Send fine grid touch code first
        data[0] = 033;
        data[1] = 0x1f;
        data[2] = 0x40 + (x & 0x1f);
        data[3] = 0x40 + ((x >> 5) & 0x0f);
        data[4] = 0x40 + (y & 0x1f);
        data[5] = 0x40 + ((y >> 5) & 0x0f);
        m_conn->SendData (data, 6);
    }
    
    x /= 32;
    y /= 32;

    ptermSendKey (0x100 | (x << 4) | y);
}

#if 0
int PtermFrame::Parity (int key)
{
    int i;
    int p = 0;
    key &= 0177;
    for (i = 0; i < 8; i++)
    {
        if (key & (1 << i))
        {
            p ^= 0200;
        }
    }
    return key;// | p;
}
#endif

void PtermFrame::ptermSetStation (int station)
{
    wxString name;
    
    SetCursor (wxNullCursor);
    m_station = station;
    if (!m_hostName.IsEmpty ())
    {
        if (station < 0)
        {
            name.Printf (wxT (" %s ASCII"),
                         m_hostName.c_str ());
        }
        else
        {
            name.Printf (wxT (" %s %d-%d"),
                         m_hostName.c_str (), station >> 5, station & 31);
        }
    }
    else
    {
        if (station < 0)
        {
            name.Printf (wxT (" ASCII"));
        }
        else
        {
            name.Printf (wxT (" %d-%d"), station >> 5, station & 31);
        }
    }
    ptermSetStatus (name);
    ptermSetName (name);
}

void PtermFrame::ptermShowTrace ()
{
    if (tracePterm)
    {
        m_statusBar->SetStatusText(_(" Trace "), STATUS_TRC);
    }
    else if (ptermApp->m_platoKb)
    {
        m_statusBar->SetStatusText(_(" PLATO keyboard "), STATUS_TRC);
    }
    else
    {
        m_statusBar->SetStatusText(wxT (""), STATUS_TRC);
    }
}

// This emulates the "ROM resident".  Return values:
// 0: PC is not special (not in resident), proceed normally.
// 1: PC is ROM function entry point, it has been emulated,
//    do a RET now.
// 2: PC is either the 8080 emulation exit magic value, or R_INIT,
//    or an invalid resident value.  Exit 8080 emulation.
int PtermFrame::check_pc8080a (void)
{
    int x, y, cp, c, x2, y2;
    
    switch (PC)
    {
    case R_MAIN:
        // "r.main" -- fake return address value used as the return
        // address for invocations of the mode 5/6/7 handler code
        return 2;

    case R_INIT:
        // r.init -- TBD
        return 2;
        
    case R_DOT:
        x = HL.pair;
        y = DE.pair;
        ptermDrawPoint (x, y);
        currentX = x;
        currentY = y;
        return 1;
        
    case R_LINE:
        x = HL.pair;
        y = DE.pair;
        ptermDrawLine (currentX, currentY, x, y);
        currentX = x;
        currentY = y;
        return 1;

    case R_CHARS:
        // draw chars ending at 07700
        cp = HL.pair;
        c = RAM[cp++];
        for (;;)
        {
            if (c == 077 && RAM[cp] == 0)
            {
                break;
            }
            plotChar (c);
            c = RAM[cp++];
        }
        return 1;
        
    case R_BLOCK:
        // block erase
        cp = HL.pair;
        x = ReadRAMW (cp);
        y = ReadRAMW (cp + 2);
        x2 = ReadRAMW (cp + 4);
        y2 = ReadRAMW (cp + 6);
        ptermBlockErase (x, y, x2, y2);
        return 1;
        
    case R_INPX:
        HL.pair = currentX;
        return 1;

    case R_INPY:
        HL.pair = currentY;
        return 1;
        
    case R_OUTX:
        currentX = HL.pair;
        return 1;
        
    case R_OUTY:
        currentY = HL.pair;
        return 1;
        
    case R_XMIT:
        // send key in HL
        return 1;
        
    case R_MODE:
        // set mode from L
        if (HL.reg.L & 1)
        {
            ptermFullErase ();
        }
        mode = (HL.reg.L >> 1) & 037;
        return 1;
        
    case R_STEPX:
        currentX = (currentX + ((RAM[M_DIR] & 2) ? -1 : 1)) & 0777;
        return 1;
        
    case R_STEPY:
        currentY = (currentY + ((RAM[M_DIR] & 1) ? -1 : 1)) & 0777;
        return 1;
        
    case R_WE:
        ptermDrawPoint (currentX, currentY);
        return 1;
        
    case R_DIR:
        RAM[M_DIR] = HL.reg.L & 3;
        return 1;
        
    case R_INPUT:
        // r.input
        return 1;
        
    case R_SSH:
        // r.ssh
        return 1;
        
    case R_CCR:
        RAM[M_CCR] = HL.reg.L;
        return 1;
        
    case R_EXTOUT:
        // r.extout
        return 1;
        
    case R_EXEC:
        // r.exec
        return 1;
        
    case R_GJOB:
        // r.gjob
        return 1;
        
    case R_XJOB:
        // r.xjob
        return 1;
        
    default:
        if (PC < WORKRAM)
        {
            // Wild jump into ROM resident, quit
            fprintf (stderr, "Wild jump to %04x\n", PC);
            return 2;
        }
        else
        {
            // Plain old RAM PC -- keep executing
            return 0;
        }
    }
}

/*******************************************************************************

input8080a:
----------

   This case switches on the content of data.  If the content of data is equal
to 1, ...
The program counter, named PC, is incremented by one.


Input:
-----

   data - This variable is the byte after the INp instruction, RAM[PC].


Output:
------

   This variable is the data that has been read or calculated from the input
device and returned to the main simulator core.


Note:
----

   This function is called directly by main8080a.

*******************************************************************************/

Uint8 PtermFrame::input8080a (Uint8 data)
{

	/***********************************************************************
	* Declaration:
	* -----------
	*
	*   retval - This variable contains data read from an input device.
	***********************************************************************/
	Uint8 retval;


	switch (data)
	{

	/***********************************************************************
	*   If the value of the 8080a's memory, RAM[PC], is equal to 1 or 2,
	* then first or second player input is requested.
	***********************************************************************/
		case 1:
		case 2:
			break;

	/***********************************************************************
	*   If the value of the 8080a's memory, RAM[PC], is not equal to any of
	* the above cases, then the program requested bad input data. Debug
	* information containing the value of the data is printed to STDOUT.
	***********************************************************************/
		default:
			printf( "INp BAD -> Data = %d\n", data);

			retval = 0;
			break;
	}


	/***********************************************************************
	*   This keyword returns the read input data to the main simulator core.
	***********************************************************************/
	return retval;
}



/*******************************************************************************

output8080a:
-----------

   This case switches on the contents of the data.  If the content of data is
equal to 2, ...
If the content of data is any other value, then the request is bad and 
debugging information is printed to STDOUT.


Input:
-----

   data - This variable is the byte after the OUTp instruction, RAM[PC].
   acc - This variable is the accumulator, which is the 8080a's A register.


Note:
----

   This function is called directly by main8080a.

*******************************************************************************/

void PtermFrame::output8080a (Uint8 data, Uint8 acc)
{
	switch (data)
	{

	/***********************************************************************
	*   If the value of the 8080a's memory, RAM[PC], is equal to 2, the
	* content of the A register is moved into the left shift amount.
	***********************************************************************/
		case 2:
			break;


	/***********************************************************************
	*   If the value of the 8080a's memory, RAM[PC], is not equal to any of
	* the above cases, then the program requested bad output data.  Debug
	* information containing the value of the data and the accumulator are
	* printed to STDOUT.
	***********************************************************************/
		default:
			printf("OUTp BAD -> Data = %d   A = %d\n", data, acc);
			break;
	}
}


// ----------------------------------------------------------------------------
// PtermPrefDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(PtermPrefDialog, wxDialog)
    EVT_CLOSE(PtermPrefDialog::OnClose)
    EVT_BUTTON(wxID_ANY,   PtermPrefDialog::OnButton)
    EVT_CHECKBOX(wxID_ANY, PtermPrefDialog::OnCheckbox)
    END_EVENT_TABLE()

PtermPrefDialog::PtermPrefDialog (PtermFrame *parent, wxWindowID id, const wxString &title)
    : wxDialog (parent, id, title),
      m_owner (parent)
{
    wxBitmap fgBitmap (20, 12);
    wxBitmap bgBitmap (20, 12);
    wxBoxSizer *bs, *ds, *ods;
    wxStaticBoxSizer *sbs;
    wxFlexGridSizer *fgs;
    
    m_scale2 = (ptermApp->m_scale != 1);
    m_classicSpeed = ptermApp->m_classicSpeed;
    m_gswEnable = ptermApp->m_gswEnable;
    m_numpadArrows = ptermApp->m_numpadArrows;
    m_showStatusBar = ptermApp->m_showStatusBar;
    m_platoKb = ptermApp->m_platoKb;
    m_beepEnable = ptermApp->m_beepEnable;
    m_connect = ptermApp->m_connect;
    m_fgColor = ptermApp->m_fgColor;
    m_bgColor = ptermApp->m_bgColor;
    m_host = ptermApp->m_hostName;
    m_port.Printf (wxT ("%d"), ptermApp->m_port);

    paintBitmap (fgBitmap, m_fgColor);
    paintBitmap (bgBitmap, m_bgColor);
    
    // Create the pieces of the dialog from outermost to innermost.
    // Order matters on some platforms, because it establishes Z-order
    // of the controls.

    // Sizer for the whole dialog box content
    ds = new wxBoxSizer (wxVERTICAL);

    // First group: emulation settings
    sbs = new wxStaticBoxSizer (new wxStaticBox (this, wxID_ANY,
                                                 _("Emulation settings")),
                                wxVERTICAL);
    m_speedCheck = new wxCheckBox (this, -1, _("Simulate 1200 Baud"));
    m_speedCheck->SetValue (m_classicSpeed);
    sbs->Add (m_speedCheck, 0,  wxTOP | wxLEFT | wxRIGHT, 8);
    m_gswCheck = new wxCheckBox (this, -1, _("Enable GSW (not in ASCII)"));
    m_gswCheck->SetValue (m_gswEnable);
    sbs->Add (m_gswCheck, 0,  wxTOP | wxLEFT | wxRIGHT, 8);
    m_arrowCheck = new wxCheckBox (this, -1, _("Numeric keypad for arrows"));
    m_arrowCheck->SetValue (m_numpadArrows);
    sbs->Add (m_arrowCheck, 0,  wxTOP | wxLEFT | wxRIGHT, 8);
    m_kbCheck = new wxCheckBox (this, -1, _("Use real PLATO keyboard"));
    m_kbCheck->SetValue (m_platoKb);
    sbs->Add (m_kbCheck, 0,  wxTOP | wxLEFT | wxRIGHT, 8);
    m_beepCheck = new wxCheckBox (this, -1, _("Enable -beep-"));
    m_beepCheck->SetValue (m_beepEnable);
    sbs->Add (m_beepCheck, 0, wxALL, 8);
    ds->Add (sbs, 0,  wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 10);
    
    // Second group: connection settings
    sbs = new wxStaticBoxSizer (new wxStaticBox (this, wxID_ANY,
                                                 _("Connection settings")),
                                wxVERTICAL);
    m_autoConnect = new wxCheckBox (this, -1, _("Connect at startup"));
    m_autoConnect->SetValue (m_connect);
    sbs->Add (m_autoConnect, 0,   wxTOP | wxLEFT | wxRIGHT, 8);
    fgs = new wxFlexGridSizer (2, 2, 8, 8);
    m_hostText = new wxTextCtrl (this, wxID_ANY, m_host,
                                 wxDefaultPosition, wxSize (160, 18),
                                 0, *new wxTextValidator (wxFILTER_ASCII,
                                                          &m_host));
    m_portText = new wxTextCtrl (this, wxID_ANY, m_port,
                                 wxDefaultPosition, wxSize (160+40, 18),
                                 0, *new wxTextValidator (wxFILTER_NUMERIC,
                                                          &m_port));
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Default Host")));
    fgs->Add (m_hostText);
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Default Port")));
    fgs->Add (m_portText);
    sbs->Add (fgs, 0, wxALL, 8);
    ds->Add (sbs, 0,  wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 10);

    // Third group: Display settings
    sbs = new wxStaticBoxSizer (new wxStaticBox (this, wxID_ANY,
                                                 _("Display settings")),
                                wxVERTICAL);
    m_scaleCheck = new wxCheckBox (this, -1, _("Zoom display 200%"));
    m_scaleCheck->SetValue (m_scale2);
    sbs->Add (m_scaleCheck, 0,  wxTOP | wxLEFT | wxRIGHT, 8);
    m_statusCheck = new wxCheckBox (this, -1, _("Status bar"));
    m_statusCheck->SetValue (m_showStatusBar);
    sbs->Add (m_statusCheck, 0, wxALL, 8);
    fgs = new wxFlexGridSizer (2, 2, 8, 8);
    m_fgButton = new wxBitmapButton (this, wxID_ANY, fgBitmap);
    m_bgButton = new wxBitmapButton (this, wxID_ANY, bgBitmap);
    fgs->Add (m_fgButton);
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Foreground")));
    fgs->Add (m_bgButton);
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Background")));
    sbs->Add (fgs, 0, wxALL, 8);
    ds->Add (sbs, 0,  wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 10);

    // Finally, the buttons
    bs = new wxBoxSizer (wxHORIZONTAL);
    m_okButton = new wxButton (this, wxID_OK);
    m_cancelButton = new wxButton (this, wxID_CANCEL);
    m_resetButton = new wxButton (this, wxID_ANY, _("Defaults"));
    bs->AddStretchSpacer ();
    bs->Add (m_okButton);
    bs->Add (m_cancelButton, 0, wxLEFT, 5);
    bs->Add (m_resetButton, 0, wxLEFT, 5);
    ds->Add (bs, 0,  wxALL | wxEXPAND, 10);

    // We'll put another 10 pixels around the whole content, which
    // requires a separate sizer unfortunately...
    ods = new wxBoxSizer (wxVERTICAL);
    ods->Add (ds, 0, wxALL, 10);
    SetSizer (ods);
    ods->Fit (this);
    m_okButton->SetDefault ();
}

void PtermPrefDialog::OnButton (wxCommandEvent& event)
{
    wxBitmap fgBitmap (20, 12);
    wxBitmap bgBitmap (20, 12);
    
    if (event.GetEventObject () == m_fgButton)
    {
        m_fgColor = PtermApp::SelectColor (m_fgColor);
    }
    else if (event.GetEventObject () == m_bgButton)
    {
        m_bgColor = PtermApp::SelectColor (m_bgColor);
    }
    
    else if (event.GetEventObject () == m_okButton)
    {
        m_host = m_hostText->GetLineText (0);
        m_port = m_portText->GetLineText (0);
        EndModal (wxID_OK);
    }
    else if (event.GetEventObject () == m_cancelButton)
    {
        EndModal (wxID_CANCEL);
    }
    else if (event.GetEventObject () == m_resetButton)
    {
        wxString str;
        
        m_fgColor = wxColour (255, 144, 0);
        m_bgColor = *wxBLACK;
        m_scale2 = false;
        m_scaleCheck->SetValue (false);
        m_classicSpeed = false;
        m_speedCheck->SetValue (false);
        m_connect = true;
        m_autoConnect->SetValue (true);
        m_gswEnable = true;
        m_gswCheck->SetValue (true);
        m_numpadArrows = true;
        m_arrowCheck->SetValue (true);
        m_showStatusBar = true;
        m_statusCheck->SetValue (true);
        m_platoKb = false;
        m_kbCheck->SetValue (false);
        m_beepEnable = true;
        m_beepCheck->SetValue (true);
        m_hostText->SetValue (DEFAULTHOST);
        str.Printf (wxT ("%d"), DefNiuPort);
        m_portText->SetValue (str);
    }
    paintBitmap (fgBitmap, m_fgColor);
    paintBitmap (bgBitmap, m_bgColor);
    m_fgButton->SetBitmapLabel (fgBitmap);
    m_bgButton->SetBitmapLabel (bgBitmap);
    Refresh ();
}

void PtermPrefDialog::OnCheckbox (wxCommandEvent& event)
{
    if (event.GetEventObject () == m_scaleCheck)
    {
        m_scale2 = event.IsChecked ();
    }
    else if (event.GetEventObject () == m_speedCheck)
    {
        m_classicSpeed = event.IsChecked ();
    }
    else if (event.GetEventObject () == m_autoConnect)
    {
        m_connect = event.IsChecked ();
    }
    else if (event.GetEventObject () == m_gswCheck)
    {
        m_gswEnable = event.IsChecked ();
    }
    else if (event.GetEventObject () == m_arrowCheck)
    {
        m_numpadArrows = event.IsChecked ();
    }
    else if (event.GetEventObject () == m_statusCheck)
    {
        m_showStatusBar = event.IsChecked ();
    }
    else if (event.GetEventObject () == m_kbCheck)
    {
        m_platoKb = event.IsChecked ();
    }
    else if (event.GetEventObject () == m_beepCheck)
    {
        m_beepEnable = event.IsChecked ();
    }
}

void PtermPrefDialog::paintBitmap (wxBitmap &bm, wxColour &color)
{
    wxBrush bitmapBrush (color, wxSOLID);
    wxMemoryDC memDC;

    memDC.SelectObject (bm);
    memDC.BeginDrawing ();
    memDC.SetBackground (bitmapBrush);
    memDC.Clear ();
    memDC.EndDrawing ();
    memDC.SetBackground (wxNullBrush);
    memDC.SelectObject (wxNullBitmap);
}


// ----------------------------------------------------------------------------
// PtermConnDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(PtermConnDialog, wxDialog)
    EVT_CLOSE(PtermConnDialog::OnClose)
    EVT_BUTTON(wxID_OK, PtermConnDialog::OnOK)
    END_EVENT_TABLE()

PtermConnDialog::PtermConnDialog (wxWindowID id, const wxString &title)
    : wxDialog (NULL, id, title)
{
    wxBoxSizer *ds, *ods;
    wxFlexGridSizer *fgs;

    // Sizer for the whole dialog box content
    ds = new wxBoxSizer (wxVERTICAL);

    // First section: the text controls
    m_host = wxString (ptermApp->m_hostName);
    m_port.Printf (wxT ("%d"), ptermApp->m_port);

    m_hostText = new wxTextCtrl (this, wxID_ANY, m_host,
                                 wxDefaultPosition, wxSize (160, 18),
                                 0, *new wxTextValidator (wxFILTER_ASCII,
                                                          &m_host));
    m_portText = new wxTextCtrl (this, wxID_ANY, m_port,
                                 wxDefaultPosition, wxSize (160, 18),
                                 0, *new wxTextValidator (wxFILTER_NUMERIC,
                                                          &m_port));
    fgs = new wxFlexGridSizer (2, 2, 8, 8);
      
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Host name")));
    fgs->Add (m_hostText);
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Port")));
    fgs->Add (m_portText);

    // Final section: the buttons
    ds->Add (fgs, 0, wxTOP | wxLEFT | wxRIGHT, 10);
    ds->Add (CreateButtonSizer( wxOK|wxCANCEL), 0, wxEXPAND | wxALL, 10);

    // We'll put another 10 pixels around the whole content, which
    // requires a separate sizer unfortunately...
    ods = new wxBoxSizer (wxVERTICAL);
    ods->Add (ds, 0, wxALL, 10);
    SetSizer (ods);
    ods->Fit (this);
}

void PtermConnDialog::OnOK (wxCommandEvent &)
{
        m_host = m_hostText->GetLineText (0);
        m_port = m_portText->GetLineText (0);
        EndModal (wxID_OK);
}

// ----------------------------------------------------------------------------
// PtermConnection
// ----------------------------------------------------------------------------

PtermConnection::PtermConnection (PtermFrame *owner, wxString &host, int port)
    : wxThread (wxTHREAD_JOINABLE),
      m_displayIn (0),
      m_displayOut (0),
      m_gswIn (0),
      m_gswOut (0),
      m_owner (owner),
      m_port (port),
      m_gswActive (false),
      m_gswStarted (false),
      m_savedGswMode (0),
      m_gswWord2 (0),
      m_connMode (both),
      m_pending (0)
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

    while (true)
    {
        // The reason for waiting a limited time here rather than
        // using the more obvious -1 (wait forever) is to make sure
        // we come out of the network wait and call TestDestroy
        // reasonably often.  Otherwise, closing the window doesn't work.
        i = dtRead (&m_fet, 200);
#ifdef DEBUG
        printf ("dtRead status %i\n", i);
#endif
        if (TestDestroy ())
        {
            break;
        }
        if (i < 0)
        {
            m_savedGswMode = m_gswWord2 = 0;
            if (m_gswActive)
            {
                m_gswActive = m_gswStarted = false;
                ptermCloseGsw ();
            }

            StoreWord (C_DISCONNECT);
            wxWakeUpIdle ();
            break;
        }
        
        wasEmpty = IsEmpty ();
//        printf ("ringcount %d\n", RingCount());
        
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

            switch (m_connMode)
            {
            case niu:
                platowd = AssembleNiuWord ();
                break;
            case ascii:
                platowd = AssembleAsciiWord ();
                break;
            case both:
                platowd = AssembleAutoWord ();
                break;
            }
            
            if (platowd == C_NODATA)
            {
                break;
            }
            else if (m_connMode == niu && platowd == 2)
            {
                m_savedGswMode = m_gswWord2 = 0;
                if (m_gswActive)
                {
                    m_gswActive = m_gswStarted = false;
                    ptermCloseGsw ();
                }
                
                // erase abort marker -- reset the ring to be empty
                wxCriticalSectionLocker lock (m_pointerLock);

                m_displayOut = m_displayIn;
            }

            StoreWord (platowd);
            i = RingCount ();
            if (m_gswActive && !m_gswStarted && i >= GSWRINGSIZE / 2)
            {
                ptermStartGsw ();
                m_gswStarted = true;
            }
            
            if (i == RINGXOFF1 || i == RINGXOFF2)
            {
                m_owner->ptermSendKey (xofkey);
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

int PtermConnection::AssembleNiuWord (void)
{
    int i, j, k;
    
    for (;;)
    {
        if (dtFetData (&m_fet) < 3)
        {
            return C_NODATA;
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
        return (i << 12) | ((j & 077) << 6) | (k & 077);
    }
}

int PtermConnection::AssembleAutoWord (void)
{
    u8 buf[3];
    
    if (dtPeekw (&m_fet, buf, 3) < 0)
    {
        return C_NODATA;
    }
    if ((buf[0] & 0200) == 0 &&
        (buf[1] & 0300) == 0200 &&
        (buf[2] & 0300) == 0300)
    {
        m_connMode = niu;
        return AssembleNiuWord ();
    }
    else
    {
        m_connMode = ascii;
        return AssembleAsciiWord ();
    }
}

int PtermConnection::AssembleAsciiWord (void)
{
    int i;
    
    for (;;)
    {
        i = dtReado (&m_fet);
        if (i == -1)
        {
            return C_NODATA;
        }
        else if (m_pending == 0 && i == 0377)
        {
            // 0377 is used by Telnet to introduce commands (IAC).
            // We recognize only IAC IAC for now.
            // Note that the check has to be made before the sign
            // bit is stripped off.
            m_pending = 0377;
            continue;
        }

        i &= 0177;
        if (i == 033)
        {
            m_pending = 033;
            continue;
        }
        if (m_pending == 033)
        {
            m_pending = 0;
            return (033 << 8) + i;
        }
        else
        {
            m_pending = 0;
            if (i == 0)
            {
                // NUL is for -delay-
                i = 1 << 19;
            }
            return i;
        }
    }
}


int PtermConnection::NextRingWord (void)
{
    int word, next, i;

    {
        wxCriticalSectionLocker lock (m_pointerLock);
        
        if (m_displayIn == m_displayOut)
        {
            return C_NODATA;
        }
    
        i = RingCount ();
        word = m_displayRing[m_displayOut];
        next = m_displayOut + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
        m_displayOut = next;
    }

    if (i < RINGXOFF1 && m_owner->m_pendingEcho != -1)
    {
        m_owner->ptermSendKey (m_owner->m_pendingEcho);
        m_owner->m_pendingEcho = -1;
    }
    if (i == RINGXON1 || i == RINGXON2)
    {
        m_owner->ptermSendKey (xonkey);
    }

    return word;
}

int PtermConnection::NextWord (void)
{
    int next, word;
    int delay = 0;

    if (m_gswActive)
    {
        // Take data from the ring of words that have just been given
        // to the GSW emulation.  Note that the delay amount has already
        // been set in those entries.  Usually it is 1, but it may be 0
        // if the display is falling behind.
        if (m_gswIn == m_gswOut)
        {
            return C_NODATA;
        }
        
        word = m_gswRing[m_gswOut];
        next = m_gswOut + 1;
        if (next == GSWRINGSIZE)
        {
            next = 0;
        }
        m_gswOut = next;
        if (word == C_GSWEND)
        {
            m_gswActive = m_gswStarted = false;
            ptermCloseGsw ();
        }
        else
        {
            return word;
        }
    }

    // Take data from the main input ring
    word = NextRingWord ();

    if (!Ascii () && 
        (word >> 16) == 3 &&
        word != 0700001 &&
        !(m_owner->m_station == 1 && 
          (word == 0770000 || word == 0730000)) &&
        ptermApp->m_gswEnable)
    {
        // It's an -extout- word, which means we'll want to start up
        // GSW emulation, if enabled.
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
        // Also, if we're connected to station 0-1, ignore the
        // operator box command words.
        if ((word >> 15) == 6)
        {
            // mode word, just save it
            m_savedGswMode = word;
        }
        else if (ptermOpenGsw (this) == 0)
        {
            m_gswActive = true;
            m_gswWord2 = word;
            delay = 1;
                
            if (!m_gswStarted && RingCount () >= GSWRINGSIZE / 2)
            {
                ptermStartGsw ();
                m_gswStarted = true;
            }
        }
    }
            
    // See if emulating 1200 baud, or the -delay- NOP code
    if (ptermApp->m_classicSpeed ||
        word == 1)
    {
        delay = 1;
    }
    
    // Pass the delay to the caller
    word |= (delay << 19);
    
    if (word == C_CONNFAIL || word == C_DISCONNECT)
    {
        wxString msg;
            
        m_owner->m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);

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
        m_owner->Close (true);
        word = C_NODATA;
    }
        
    return word;
}

// Get a word from the main data ring for the GSW emulation.  The 
// "catchup" flag is true if this is the first word for the current
// burst of sound data, meaning that the display should be made to
// catch up to this point.  We do that by walking back through the
// ring of data from GSW to the display (m_gswRing) clearing out the
// delay setting for any words that haven't been processed yet.
// This is necessary because the GSW paces the display.  The 1/60th
// second timer roughly does the same, but in case it is off the two
// could end up out of sync, so we force them to resync here.
int PtermConnection::NextGswWord (bool catchup)
{
    int next, word;

    if (m_savedGswMode != 0)
    {
        word = m_savedGswMode;
        m_savedGswMode = 0;
    }
    else if (m_gswWord2 != 0)
    {
        word = m_gswWord2;
        m_gswWord2 = 0;
    }
    else
    {
        next = m_gswOut;
        if (catchup)
        {
            while (next != m_gswIn)
            {
                // The display has some catching up to do.
                if (int (m_gswRing[next]) >= 0)
                {
                    m_gswRing[next] &= 01777777;
                }
                next++;
                if (next == GSWRINGSIZE)
                {
                    next = 0;
                }
            }
            wxWakeUpIdle ();
        }
        next = m_gswIn + 1;
        if (next == GSWRINGSIZE)
        {
            next = 0;
        }
        // If there is no room to save this data for the display,
        // tell the sound that there isn't any more data.
        if (next == m_gswOut)
        {
            return C_NODATA;
        }
        word = NextRingWord ();
        m_gswRing[m_gswIn] = word | (1 << 19);
        m_gswIn = next;
    }
    
    return word;
}

int ptermNextGswWord (void *connection, int catchup)
{
    return ((PtermConnection *) connection)->NextGswWord (catchup != 0);
}

void PtermConnection::StoreWord (int word)
{
    int next;
    
    if (word < 0)
    {
        m_displayOut = m_displayIn;
    }
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
    EVT_LEFT_UP(PtermCanvas::OnMouseUp)
    EVT_MOTION(PtermCanvas::OnMouseMotion)
    END_EVENT_TABLE ()

PtermCanvas::PtermCanvas(PtermFrame *parent)
    : wxScrolledWindow(parent, -1, wxDefaultPosition, 
                       wxSize (XSize, YSize),
                       wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE),
      m_mouseX (-1)
{
    wxClientDC dc(this);

    m_owner = parent;
    SetVirtualSize (XSize, YSize);
    
    SetBackgroundColour (ptermApp->m_bgColor);
    SetScrollRate (1, 1);
    dc.SetClippingRegion (GetXMargin (), GetYMargin (),
                          ScreenSize, ScreenSize);
    SetFocus ();
    FullErase ();
}

int PtermCanvas::GetXMargin (void) const
{
    return m_owner->GetXMargin ();
}
int PtermCanvas::GetYMargin (void) const
{
    return m_owner->GetYMargin ();
}

void PtermCanvas::OnDraw(wxDC &dc)
{
    int i, j, c;
    int charX, charY, sizeX, sizeY;
    
    m_owner->PrepareDC (dc);
    dc.Blit (XTOP, YTOP, ScreenSize, ScreenSize,
             m_owner->m_memDC, 0, 0, wxCOPY);
#if 0
    // debug charmaps
    for (i = 0; i <= 4; i++)
    {
        dc.Blit (XTOP, YADJUST(i * 48 + 48), CharXSize,
                 32 * ptermApp->m_scale, 
                 m_owner->m_charDC[i], 0, 0, wxCOPY);
    }
#endif
    if (m_regionHeight != 0 && m_regionWidth != 0)
    {
        sizeX = 8 * ptermApp->m_scale;
        sizeY = 16 * ptermApp->m_scale;
        for (i = m_regionY; i < m_regionY + m_regionHeight; i++)
        {
            for (j = m_regionX; j < m_regionX + m_regionWidth; j++)
            {
                c = textmap[i * 64 + j];
                charX = (c & 077) * 8 * ptermApp->m_scale;
                charY = (c >> 6) * 16 * ptermApp->m_scale;
                dc.Blit (XADJUST (j * 8), YADJUST (i * 16 + 15), 
//                         sizeX, sizeY, m_owner->m_charDC[4],
//                         charX, charY, wxCOPY);
                         sizeX, sizeY, m_owner->m_charDC[1],
                         charX, charY, wxSRC_INVERT);            }
        }
    }
}

void PtermCanvas::OnKeyDown (wxKeyEvent &event)
{
    unsigned int key;
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

	if (m_owner->m_bPasteActive)
	{
		// If pasting is active, this key is NOT passed on to the application until
		// the paste operation is properly canceled
		m_owner->m_bCancelPaste = true;
		return;
	}

    ctrl = event.m_controlDown;
    if (event.m_shiftDown)
    {
        shift = 040;
    }
    key = event.m_keyCode;
    if (!m_owner->HasConnection () ||
        key == WXK_ALT || key == WXK_SHIFT || key == WXK_CONTROL)
    {
        // We don't take any action on the modifier key keydown events,
        // but we do want to make sure they are seen by the rest of
        // the system.
        // The same applies to keys sent to the help window (which has
        // no connection on which to send them).
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
        m_owner->ptermSetTrace (true);
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
                if (shift != 0 && key == '=')
                {
                    key = '+';
                }
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

    pc = -1;
    if (ptermApp->m_platoKb)
    {
#if defined(_WIN32)
		// This is a workaround for a Windows keyboard mapping bug
        if (key == '+')
        {
			pc = 0133;		// =
        }
#endif
    }
    else if (ptermApp->m_numpadArrows)
    {
        // Check the numeric keypad keys separately and turn them
        // into the 8-way arrow keys of the PLATO main keyboard.
        switch (key)
        {
        case WXK_NUMPAD7:
        case WXK_NUMPAD_HOME:
            pc = 0121;      // up left (q)
            break;
        case WXK_NUMPAD8:
        case WXK_NUMPAD_UP:
            pc = 0127;      // up arrow (w)
            break;
        case WXK_NUMPAD9:
        case WXK_NUMPAD_PRIOR:
            pc = 0105;      // up right (e)
            break;
        case WXK_NUMPAD4:
        case WXK_NUMPAD_LEFT:
            pc = 0101;      // left arrow (a)
            break;
        case WXK_NUMPAD6:
        case WXK_NUMPAD_RIGHT:
            pc = 0104;      // right arrow (d)
            break;
        case WXK_NUMPAD1:
        case WXK_NUMPAD_END:
            pc = 0132;      // down left (z)
            break;
        case WXK_NUMPAD2:
        case WXK_NUMPAD_DOWN:
            pc = 0130;      // down arrow (x)
            break;
        case WXK_NUMPAD3:
        case WXK_NUMPAD_NEXT:
            pc = 0103;      // down right (c)
            break;
        }
    }
    
    if (pc == -1)
    {
        switch (key)
        {
        case WXK_SPACE:
        case WXK_NUMPAD_SPACE:
            pc = 0100;      // space
            break;
        case WXK_BACK:
            pc = 023;       // erase
            break;
        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            pc = 026;       // next
            break;
        case WXK_HOME:
        case WXK_NUMPAD_HOME:
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
        case WXK_NUMPAD_ADD:
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
        case WXK_NUMPAD_SUBTRACT:
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
        case WXK_NUMPAD_MULTIPLY:
        case WXK_DELETE:
            pc = 012;       // multiply sign
            break;
        case WXK_DIVIDE:
        case WXK_NUMPAD_DIVIDE:
        case WXK_INSERT:
            pc = 013;       // divide sign
            break;
        case WXK_LEFT:
        case WXK_NUMPAD_LEFT:
            pc = 0101;      // left arrow (a)
            break;
        case WXK_RIGHT:
        case WXK_NUMPAD_RIGHT:
            pc = 0104;      // right arrow (d)
            break;
        case WXK_UP:
        case WXK_NUMPAD_UP:
            pc = 0127;      // up arrow (w)
            break;
        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            pc = 0130;      // down arrow (x)
            break;
        case WXK_PRIOR:
        case WXK_PAGEUP:    // (Mac does this instead)
        case WXK_NUMPAD_PRIOR:
            pc = 020;       // super
            break;
        case WXK_NEXT:
        case WXK_PAGEDOWN:  // (Mac does this instead)
        case WXK_NUMPAD_NEXT:
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
        case WXK_NUMPAD_DECIMAL:
            pc = 0136;      // .
            break;
        case WXK_NUMPAD0:
        case WXK_NUMPAD1:
        case WXK_NUMPAD2:
        case WXK_NUMPAD3:
        case WXK_NUMPAD4:
        case WXK_NUMPAD5:
        case WXK_NUMPAD6:
        case WXK_NUMPAD7:
        case WXK_NUMPAD8:
        case WXK_NUMPAD9:
            pc = key - WXK_NUMPAD0;
            if (ctrl)
            {
                shift = 040;
            }
            break;
        default:
            // If it's not a function key, we'll let the OnChar handler
            // deal with it if we are using a regular keyboard.
            // But if we're using a PLATO keyboard, we'll take the
            // (unshifted) keycode, translate it to the PLATO code, apply
            // shift if any, and send that.  This is because the 
            // shifted non-letters on the PLATO keyboard are generally
            // different from those on the PC/Mac keyboard.
            pc = asciiToPlato[key];
            if (!ptermApp->m_platoKb || pc < 0)
            {
                event.Skip ();
                return;
            }
            // On Linux (wx/GTK), there's a keyboard handling error:
            // shifted comma key produces an OnKeyDown event with 
            // key = '<', which is wrong, it should be the unshifted 
            // code.  All other codes appear to be correct, and it also
            // works right on other platforms.  Weird...  Work around
            // this by forcing the keycode.
            if (key == '<')
            {
                pc = asciiToPlato[','];
            }
        }
    }

    m_owner->ptermSendKey (pc | shift);
}

void PtermCanvas::OnChar(wxKeyEvent& event)
{
    unsigned int key;
    int shift = 0;
    int pc = -1;

    // control and alt codes shouldn't come here, they are handled in KEY_DOWN
    if (!m_owner->HasConnection () ||
        event.m_controlDown || event.m_altDown || event.m_metaDown)
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

void PtermCanvas::SaveChar (int x, int y, int snum, int cnum, 
                            int w, bool large_p)
{
    char c = 055;

    // Convert the current x/y to a coarse grid position, and save
    // the current character code (snum << 6 + cnum) into the textmap
    // array.  Note that the Y coordinate starts with 0 for the bottom
    // line, just as the fine grid Y coordinate does.
    x /= 8;
    y /= 16;
    if ((w & 1) && snum < 2)
    {
        c = (snum << 6) + cnum;
    }
    textmap[y * 64 + x] = c;
    if (large_p)
    {
        x = (x + 1) & 077;
        textmap[y * 64 + x] = 055;
        y = (y + 1) & 037;
        textmap[y * 64 + x] = 055;
        x = (x - 1) & 077;
        textmap[y * 64 + x] = 055;
    }
}

void PtermCanvas::OnCopy (wxCommandEvent &)
{
    int i, j, s, spaces;
    wxString text;
    wxChar c;
    
    if (m_regionHeight == 0 || m_regionWidth == 0)
    {
        return;
    }
    
#ifdef DEBUG
    for (i = 0; i < sizeof (textmap); i++)
    {
        printf ("%c", rom01char[textmap[i ^ 03700]]);
        if ((i & 077) == 077)
        {
            printf ("\n");
        }
    }
#endif

    // regionX and regionY are the lowest coordinate corner of
    // the copy region, i.e., the lower left corner, expressed in
    // 0-based coarse grid coordinates.
    for (i = m_regionY + m_regionHeight - 1; i >= m_regionY; i--)
    {
        spaces = 0;
        for (j = m_regionX; j < m_regionX + m_regionWidth; j++)
        {
            c = rom01char[textmap[i * 64 + j]];
            if (c == wxT (' '))
            {
                spaces++;
            }
            else
            {
                for (s = 0; s < spaces; s++)
                {
                    text.Append (wxT (' '));
                }
                spaces = 0;
                text.Append (c);
            }
        }
        if (m_regionHeight > 1)
        {
            text.Append (wxT ('\n'));
        }
    }
    
    if (!wxTheClipboard->Open ())
    {
        wxLogError (_("Can't open clipboard."));

        return;
    }

    if (!wxTheClipboard->SetData (new wxTextDataObject (text)))
    {
        wxLogError (_("Can't copy text to the clipboard"));
    }

    wxTheClipboard->Close ();
}

void PtermCanvas::FullErase (void)
{
    // Erase the text "backing store"
    memset (textmap, 055, sizeof (textmap));
    
    ClearRegion ();
}

void PtermCanvas::ClearRegion (void)
{
    // Cancel any region selection
    if (m_regionHeight != 0 || m_regionWidth != 0)
    {
	    m_regionHeight = 0;
		m_regionWidth = 0;
		m_owner->GetMenuBar ()->Enable (Pterm_Copy, false);
		Refresh (false);
	}
}


void PtermCanvas::OnMouseDown (wxMouseEvent &event)
{
    m_mouseX = XUNADJUST (event.m_x);
    m_mouseY = YUNADJUST (event.m_y);
    
    ClearRegion ();
    event.Skip ();
}

void PtermCanvas::OnMouseMotion (wxMouseEvent &event)
{
    if (m_mouseX >= 0 && event.m_leftDown)
    {
        UpdateRegion (event);
    }
    
    event.Skip ();
}

void PtermCanvas::OnMouseUp (wxMouseEvent &event)
{
    int x, y;
    
    if (m_mouseX < 0)
    {
        event.Skip ();
	return;
    }

    x = XUNADJUST (event.m_x);
    y = YUNADJUST (event.m_y);
    
    UpdateRegion (event);
    
    m_mouseX = -1;

    if (!m_touchEnabled)
    {
        return;
    }
    if (x < 0 || x > 511 ||
        y < 0 || y > 511)
    {
        return;
    }
    m_owner->ptermSendTouch (x, y);
}

void PtermCanvas::UpdateRegion (wxMouseEvent &event)
{
    int x, y, x1, x2, y1, y2;
    
    x = XUNADJUST (event.m_x);
    y = YUNADJUST (event.m_y);
    
    if (abs (x - m_mouseX) > MouseTolerance ||
        abs (y - m_mouseY) > MouseTolerance)
    {
        // It was a mouse drag (region selection)
        // rather than a click
        if (m_mouseX > x)
        {
            x1 = x;
            x2 = m_mouseX;
        }
        else
        {
            x1 = m_mouseX;
            x2 = x;
        }
        if (m_mouseY > y)
        {
            y1 = y;
            y2 = m_mouseY;
        }
        else
        {
            y1 = m_mouseY;
            y2 = y;
        }
        BOUND (x1);
        BOUND (x2);
        BOUND (y1);
        BOUND (y2);
        m_regionX = x1 / 8;
        m_regionY = y1 / 16;
        m_regionWidth = (x2 + 1 - (m_regionX * 8)) / 8;
        m_regionHeight = (y2 - (m_regionY * 16)) / 16 + 1;
        m_owner->GetMenuBar ()->Enable (Pterm_Copy, 
                                        (m_regionWidth > 0));
#ifdef DEBUG
        printf ("region %d %d size %d %d\n", m_regionX, m_regionY,
                m_regionWidth, m_regionHeight);
#endif
        Refresh (false);
        return;
    }
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

#if defined (__WXMSW__)
// Override the window proc to avoid F10 being handled  as a hotkey
WXLRESULT PtermCanvas::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    if (message == WM_SYSKEYDOWN && wParam == VK_F10)
    { 
        m_lastKeydownProcessed = HandleKeyDown((WORD) wParam, lParam);
        return 0;
    }
    return wxScrolledWindow::MSWWindowProc(message, wParam, lParam);
}
#endif

// ----------------------------------------------------------------------------
// Pterm printing helper class
// ----------------------------------------------------------------------------

int PtermPrintout::GetXMargin (void) const
{
    return m_owner->GetXMargin ();
}
int PtermPrintout::GetYMargin (void) const
{
    return m_owner->GetYMargin ();
}

bool PtermPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument (startPage, endPage))
        return false;

    return true;
}

bool PtermPrintout::OnPrintPage (int page)
{
    wxDC *dc = GetDC ();

    if (dc)
    {
        if (page == 1)
        {
            DrawPage (dc);
        }
#if 0
        dc->SetDeviceOrigin(0, 0);
        dc->SetUserScale(1.0, 1.0);

        wxChar buf[200];
        wxSprintf(buf, wxT("PAGE %d"), page);
        dc->DrawText(buf, 10, 10);
#endif
        return true;
    }

    return false;
}

void PtermPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool PtermPrintout::HasPage(int pageNum)
{
    return (pageNum == 1);
}

void PtermPrintout::DrawPage (wxDC *dc)
{
    wxBitmap screenmap (ScreenSize, ScreenSize);
    wxMemoryDC screenDC;
    int ofr, ofg, ofb;
    double maxX = ScreenSize;
    double maxY = ScreenSize;

    // Let's have at least 50 device units margin
    double marginX = 50;
    double marginY = 50;

    // Add the margin to the graphic size
    maxX += (2*marginX);
    maxY += (2*marginY);

    // Get the size of the DC in pixels
    int w, h;
    dc->GetSize(&w, &h);

    // Calculate a suitable scaling factor
    double scaleX = (double) (w / maxX);
    double scaleY = (double) (h / maxY);

    // Use x or y scaling factor, whichever fits on the DC
    double actualScale = wxMin(scaleX,scaleY);

    // Calculate the position on the DC for centring the graphic
    double posX = (double) ((w - (ScreenSize * actualScale)) / 2.0);
    double posY = (double) ((h - (ScreenSize * actualScale)) / 2.0);

    // Set the scale and origin
    dc->SetUserScale (actualScale, actualScale);
    dc->SetDeviceOrigin ((long) posX, (long) posY);

    // Re-color the image
    screenDC.SelectObject (screenmap);
    screenDC.Blit (XTOP, YTOP, ScreenSize, ScreenSize, 
                   m_owner->m_memDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);

    wxImage screenImage = screenmap.ConvertToImage ();

    unsigned char *data = screenImage.GetData ();
    
    w = screenImage.GetWidth ();
    h = screenImage.GetHeight ();
    ofr = ptermApp->m_fgColor.Red ();
    ofg = ptermApp->m_fgColor.Green ();
    ofb = ptermApp->m_fgColor.Blue ();

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            if (data[0] == ofr &&
                data[1] == ofg &&
                data[2] == ofb)
            {
                // Foreground, make it black
                data[0] = data[1] = data[2] = 0;
            }
            else
            {
                // Background, make it white
                data[0] = data[1] = data[2] = 255;
            }
            data += 3;
        }
    }

    wxBitmap printmap (screenImage);

    screenDC.SelectObject (printmap);
    dc->Blit (XTOP, YTOP, ScreenSize, ScreenSize, &screenDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);
}

/*---------------------------  End Of File  ------------------------------*/
