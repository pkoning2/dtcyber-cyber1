//#define TEST 1
/////////////////////////////////////////////////////////////////////////////
// Name:        dd60.cpp
// Purpose:     dd60 interface to wxWindows 
// Author:      Paul Koning
// Modified by:
// Created:     08/02/2005
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

#define BufSiz      200000

#define STATUS_TIP      0
#define STATUS_TRC      1
#define STATUS_CONN     2
#define STATUSPANES     3

// character pattern sizes in pixels, allowing for going over the
// allotted space some.
#define CHAR8SIZE       16
#define CHAR16SIZE      32
#define CHAR32SIZE      48

#define DD60CHARS       060         // number of characters in pattern arrays

// Display parameters
#define RRATE           100        // ms per refresh
#define DECAY           128        // decay per refresh, scaled by 256.

//#define DisplayMargin   8

// Literal strings for wxConfig key strings.  These are defined
// because they appear in two places, so this way we avoid getting
// the two out of sync.  Note that they should *not* be changed after
// once being defined, since that invalidates people's stored
// preferences.
#define PREF_FOREGROUND "foreground"
#define PREF_PORT       "port"
#define PREF_CONNECT    "autoconnect"
#define PREF_STATUSBAR  "statusbar"
#define PREF_SIZEX      "sizeX"
#define PREF_SIZEY      "sizeY"
#define PREF_FOCUS      "focus"
#define PREF_INTENS     "intensity"

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define TRACEN(str)                                             \
    if (traceDd60)                                             \
        {                                                       \
        fprintf (traceF, str "\n");  \
        }

#define TRACE1(str, arg)                                             \
    if (traceDd60)                                                 \
        {                                                           \
        fprintf (traceF, str "\n", arg); \
        }

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
#include "wx/rawbmp.h"

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
#include "dd60version.h"
#include "iir.h"
#include "knob.h"

#define scaleX 10
#define scaleY 10
#include "dd60.h"

#ifdef TEST
int tcount;
    
// test data vector -- a simulated data stream from the Cyber.
u8 tdata[] = {
    Dd60SetMode + Dd60CharMedium + Dd60ScreenR, Dd60SetX, 0, Dd60SetY + 1, 0200,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // "intensity adjust" from DS1
    Dd60SetMode + Dd60CharSmall, Dd60SetX, 0, Dd60SetY, 0200,
    011, 016, 024, 005, 016, 023, 011, 024, 031, 
    055, 001, 004, 012, 025, 023, 024,
    Dd60SetMode + Dd60CharMedium, Dd60SetX, 0, Dd60SetY + 1, 000,
    011, 016, 024, 005, 016, 023, 011, 024, 031, 
    055, 001, 004, 012, 025, 023, 024,
    Dd60SetMode + Dd60CharLarge, Dd60SetX, 0, Dd60SetY + 1, 0200,
    011, 016, 024, 005, 016, 023, 011, 024, 031, 
    055, 001, 004, 012, 025, 023, 024,
};
#endif

#if wxUSE_LIBGNOMEPRINT
#include "wx/html/forcelnk.h"
//FORCE_LINK(gnome_print)
#endif

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
#include "dd60.xpm"
#endif

#include "chargen.h"

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

class Dd60App;
static Dd60App *dd60App;
class Dd60Panel;
static Dd60Panel *dd60Panel;
static wxFrame *dd60PanelFrame;

static FILE *traceF;
static char traceFn[20];

#ifdef DEBUG
static wxLogWindow *logwindow;
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

class Dd60Frame;

// Dd60 screen printout
class Dd60Printout: public wxPrintout
{
 public:
    Dd60Printout (Dd60Frame *owner,
                   const wxString &title = _("DD60 printout")) 
        : wxPrintout (title),
          m_owner (owner)
    {}
  bool OnPrintPage (int page);
  bool HasPage (int page);
  bool OnBeginDocument (int startPage, int endPage);
  void GetPageInfo (int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
  void DrawPage (wxDC *dc);

private:
    Dd60Frame *m_owner;
};

// Define a new application type, each program should derive a class from wxApp
class Dd60App : public wxApp
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
    void OnAbout (wxCommandEvent& event);

    bool DoConnect (bool ask);
    static wxColour SelectColor (wxColour &initcol);
    
    wxColour    m_fgColor;
    wxConfig    *m_config;

    int         m_port;
    // scale is 1 or 2 for full size and double, respectively.
    int         m_scale;
    bool        m_connect;
    bool        m_showStatusBar;
    Dd60Frame  *m_firstFrame;
    wxString    m_defDir;
    
private:
    wxLocale    m_locale; // locale we'll be using
    
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// define a scrollable canvas for drawing onto
class Dd60Canvas: public wxScrolledWindow
{
public:
    Dd60Canvas (Dd60Frame *parent);

    void OnDraw (wxDC &dc);
    void OnEraseBackground (wxEraseEvent &);
    void OnKey (wxKeyEvent& event);

private:
    Dd60Frame *m_owner;

    DECLARE_EVENT_TABLE ()
};

#if defined(__WXMAC__)
#define DD60_MDI 1
#else
#define DD60_MDI 0
#endif

#if DD60_MDI
#define Dd60FrameBase  wxMDIChildFrame

class Dd60MainFrame : public wxMDIParentFrame
{
public:
    Dd60MainFrame (void);
};

static Dd60MainFrame *Dd60FrameParent;
#else
#define Dd60FrameBase wxFrame
#define Dd60FrameParent NULL
#endif

// Define a new frame type: this is going to be our main frame
class Dd60Frame : public Dd60FrameBase
{
    friend void Dd60Canvas::OnDraw(wxDC &dc);
    friend void Dd60Printout::DrawPage (wxDC *dc);
    
    typedef wxAlphaPixelData PixelData;

public:
    // ctor(s)
    Dd60Frame(int port, const wxString& title);
    ~Dd60Frame ();

    // event handlers (these functions should _not_ be virtual)
    void OnClose (wxCloseEvent& event);
    void OnTimer (wxTimerEvent& event);
    void OnQuit (wxCommandEvent& event);
    void OnCopyScreen (wxCommandEvent &event);
    void OnSaveScreen (wxCommandEvent &event);
    void OnPrint (wxCommandEvent& event);
    void OnPrintPreview (wxCommandEvent& event);
    void OnPageSetup (wxCommandEvent& event);
    void OnActivate (wxActivateEvent &event);
    void UpdateSettings (bool newstatusbar);
    
    void PrepareDC(wxDC& dc);
    void dd60SendKey(int key);
    void dd60SetTrace (bool fileaction);
    void dd60LoadChars (void);

    bool        traceDd60;
    Dd60Frame   *m_nextFrame;
    Dd60Frame   *m_prevFrame;
    bool        truekb;

private:
    wxStatusBar *m_statusBar;       // present even if not displayed
    wxPen       m_foregroundPen;
    wxBrush     m_foregroundBrush;
    wxBitmap    *m_screenmap;
    PixelData   *m_pixmap;

    Dd60Canvas  *m_canvas;
    NetFet      m_fet;
    int         m_port;
    wxTimer     m_timer;

    // Character patterns are stored in three pixel vectors.  These are
    // not bitmaps or images, because we display them by adding them
    // into the current screen pixel values (similar to what the CRT
    // beam would do).  They are RBG to simulate the color distortion
    // that comes from saturation.  (That may be overkill...)  The pixels
    // for each character are in a contiguous set of bytes in the vector;
    // the character display operation spreads them over multiple scanlines.
    u8     *m_char8;
    u8     *m_char16;
    u8     *m_char32;

    // DD60 display emulation state
    int         mode;
    int         currentX;
    int         currentY;
    int         currentXOffset;

    // DD60 drawing primitives
    void dd60SetName (wxString &winName);
    void dd60SetStatus (wxString &str);

    void dd60LoadCharSize (int size, int tsize, u8 *vec);
    void procDd60Char (unsigned int d);
    void dd60ShowTrace (bool enable);
    
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// define the preferences dialog
class Dd60PrefDialog : public wxDialog
{
public:
    Dd60PrefDialog (Dd60Frame *parent, wxWindowID id, const wxString &title);
    
    void OnButton (wxCommandEvent& event);
    void OnCheckbox (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    wxBitmapButton  *m_fgButton;
    wxButton        *m_okButton;
    wxButton        *m_cancelButton;
    wxButton        *m_resetButton;
    wxCheckBox      *m_autoConnect;
    wxCheckBox      *m_statusCheck;
    wxTextCtrl      *m_portText;

    wxColour        m_fgColor;
    bool            m_connect;
    bool            m_showStatusBar;
    wxString        m_port;
    
private:
    void paintBitmap (wxBitmap &bm, wxColour &color);
    
    Dd60Frame *m_owner;

    DECLARE_EVENT_TABLE ()
};

// define the preferences dialog
class Dd60ConnDialog : public wxDialog
{
public:
    Dd60ConnDialog (wxWindowID id, const wxString &title);
    
    void OnOK (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    wxTextCtrl      *m_portText;
    
    wxString        m_port;
    
private:
    DECLARE_EVENT_TABLE ()
};


// Define a panel for the controls. 
class Dd60Panel : public wxPanel
{
public:
    Dd60Panel (wxFrame *parent);

    int sizeX (void) const
    { 
        return m_sizeX->GetValue ();
    }
    int sizeY (void) const
    { 
        return m_sizeY->GetValue ();
    }
    double beamsize (void) const { return 0.01 * m_focus->GetValue (); }
    int intensity (void) const { return m_intens->GetValue (); }
#if 0
    double r1_029 (void) const { return (double ) (m_r1_029->GetValue ()); }
    double c1_029 (void) const { return 1.0e-12 * m_c1_029->GetValue (); }
    double c1_a2 (void) const { return 1.0e-12 * m_c1_a2->GetValue (); }
    double c4_a2 (void) const { return 1.0e-12 * m_c4_a2->GetValue (); }
    int delay (void) const { return m_beamdelay->GetValue (); }
    int red (void) const { return m_red->GetValue (); }
    int green (void) const { return m_green->GetValue (); }
    int blue (void) const { return m_blue->GetValue (); }
    bool get620on (void) const { return m_620on->GetValue (); }
    bool getc19on (void) const { return m_c19on->GetValue (); }
    bool get029on (void) const { return m_029on->GetValue (); }
    bool getv1aon (void) const { return m_v1aon->GetValue (); }
    bool getv3on (void) const { return m_v3on->GetValue (); }
#endif
    void OnScroll (wxScrollEvent& event);

private:
    wxFlexGridSizer  *m_sizer;
    wxKnob      *m_sizeX;
    wxKnob      *m_sizeY;
    wxKnob      *m_focus;
    wxKnob      *m_intens;
#if 0
    wxKnob      *m_c2_019;
    wxKnob      *m_r1_029;      // 029 amp pot
    wxKnob      *m_c1_029;      // 029 amp trimcap
    wxKnob      *m_c1_a2;       // v1a trimcap
    wxKnob      *m_c4_a2;       // v3 trimcap
    wxKnob      *m_beamdelay;   // delay (in simclock ticks) for on/off
    wxKnob      *m_red;         // red pixel value
    wxKnob      *m_green;       // green pixel value
    wxKnob      *m_blue;        // blue pixel value
    wxCheckBox  *m_620on;
    wxCheckBox  *m_c19on;
    wxCheckBox  *m_029on;
    wxCheckBox  *m_v1aon;
    wxCheckBox  *m_v3on;
#endif
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};

// Character waveform generator.  This generates the unfiltered
// waveform, i.e., an idealized waveform made up of straight lines
// (constant slope ramps, or constant voltages).  It implements
// the CC545 character generator patterns from document 60469310.
class Chargen
{
public:
    void SetStepCount (int n)
    {
        m_stepcount = n;
    }
    void Start (int ch)
    {
        m_x = m_y = 0;
        m_on = false;
        m_dx = 1;
        m_dy = -1;
        m_chardata = chargen[ch];
        m_step = m_stroke = 0;
    }

    void Step (void);
    double X (void) const 
    {
        return m_x;
    }
    double Y (void) const
    {
        return m_y;
    }
    bool Done (void) const
    {
        return (m_stroke == sizeof (chargen[0]) / sizeof (chargen[0][0]));
    }
    bool On (void) const
    {
        return m_on;
    }
    
private:
    double m_x, m_y;
    int m_dx, m_dy, m_step, m_stroke;
    bool m_on;
    const u8 *m_chardata;
    int m_stepcount;
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Dd60_CopyScreen = 1,
    Dd60_ConnectAgain,
    Dd60_SaveScreen,

    // timers
    Dd60_Timer,        // display pacing

    // Menu items with standard ID values
    Dd60_Print = wxID_PRINT,
    Dd60_Page_Setup = wxID_PRINT_SETUP,
    Dd60_Preview = wxID_PREVIEW,
    Dd60_Connect = wxID_NEW,
    Dd60_Quit = wxID_EXIT,
    Dd60_Close = wxID_CLOSE,
    Dd60_Pref = wxID_PREFERENCES,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Dd60_About = wxID_ABOUT
};


// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(Dd60Frame, wxFrame)
    EVT_CLOSE(Dd60Frame::OnClose)
    EVT_TIMER(Dd60_Timer, Dd60Frame::OnTimer)
    EVT_ACTIVATE(Dd60Frame::OnActivate)
    EVT_MENU(Dd60_Close, Dd60Frame::OnQuit)
    EVT_MENU(Dd60_CopyScreen, Dd60Frame::OnCopyScreen)
    EVT_MENU(Dd60_SaveScreen, Dd60Frame::OnSaveScreen)
    EVT_MENU(Dd60_Print, Dd60Frame::OnPrint)
    EVT_MENU(Dd60_Preview, Dd60Frame::OnPrintPreview)
    EVT_MENU(Dd60_Page_Setup, Dd60Frame::OnPageSetup)
    END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(Dd60App, wxApp)
    EVT_MENU(Dd60_Connect, Dd60App::OnConnect)
    EVT_MENU(Dd60_ConnectAgain, Dd60App::OnConnect)
    EVT_MENU(Dd60_Pref,    Dd60App::OnPref)
    EVT_MENU(Dd60_Quit,    Dd60App::OnQuit)
    EVT_MENU(Dd60_About, Dd60App::OnAbout)

    END_EVENT_TABLE ()

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp () which will return the reference of the right type (i.e. Dd60App and
// not wxApp)
IMPLEMENT_APP(Dd60App)

// ============================================================================
// implementation
// ============================================================================

#define pi 3.14159265358979323846
// *** TEMP
#define STEPMHZ 10
#define STEPHZ  (STEPMHZ * 1000000)
#define BWRATIO 100
#define BW 4

// Normal distribution (bell curve) function
static double bell (double x, double sigma)
{
    return (1 / (sigma * sqrt (2. * pi))) * 
        exp (-((x * x) / (2. * sigma * sigma)));
}

// Number of steps per stroke element, as a function of chosen bandwidth
static int stepcount (int height)
{
    return BW * BWRATIO / STEPMHZ;
}

// Normalize the intensity to account for beam size and character size
// The input intensity value is the value we want to end up with in the
// pixel for a double  speed stroke.
static double normInt (int intens, int size)
{
    double step = size / (4.0 * stepcount (size));
    
    return intens * step;
}

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool Dd60App::OnInit (void)
{
    int r, g, b;
    wxString rgb;
    
    dd60App = this;
    m_firstFrame = NULL;
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
    
    sprintf (traceFn, "dd60_%d.trc", getpid ());

    m_locale.Init(wxLANGUAGE_DEFAULT);
    m_locale.AddCatalog(wxT("dd60"));

#ifdef DEBUG
    logwindow = new wxLogWindow (NULL, "dd60 log", true, false);
#endif

    if (argc > 2)
    {
        printf ("usage: dd60 [ portnum ]\n");
        exit (1);
    }

    dd60PanelFrame = new wxFrame (NULL, wxID_ANY, _("DD60 controls"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxFRAME_TOOL_WINDOW | wxMINIMIZE_BOX);
    dd60Panel = new Dd60Panel (dd60PanelFrame);
    dd60PanelFrame->Show (true);

    m_config = new wxConfig (wxT ("Dd60"));

    if (argc > 1)
    {
        m_port = atoi (wxString (argv[1]).mb_str ());
    }
    else
    {
        m_port = m_config->Read (wxT (PREF_PORT), DefDd60Port);
    }
    // 20 255 80 is RGB for DD60 green
    m_config->Read (wxT (PREF_FOREGROUND), &rgb, wxT ("20 255 80"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_fgColor = wxColour (r, g, b);
    m_connect = (m_config->Read (wxT (PREF_CONNECT), 1) != 0);
    m_showStatusBar = (m_config->Read (wxT (PREF_STATUSBAR), 1) != 0);
#if 0
    m_sizeX = m_config->Read (wxT (PREF_SIZEX), 30);
    m_sizeY = m_config->Read (wxT (PREF_SIZEY), 30);
    m_focus = m_config->Read (wxT (PREF_FOCUS), 160);
    m_intens = m_config->Read (wxT (PREF_INTENS), 10);
#endif

#if DD60_MDI
    // On Mac, the style rule is that the application keeps running even
    // if all its windows are closed.
//    SetExitOnFrameDelete(false);
    Dd60FrameParent = new Dd60MainFrame ();
    Dd60FrameParent->Show (true);
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

int Dd60App::OnExit (void)
{
    delete g_printData;
    delete g_pageSetupData;
#ifdef DEBUG
    delete logwindow;
#endif

#if DD60_MDI
    // We only need to do this in the MDI (Mac) case; for the other
    // platforms, the panel is deleted when the last regular window is.
    dd60PanelFrame->Destroy ();
#endif
    return 0;
}

void Dd60App::OnConnect (wxCommandEvent &event)
{
    DoConnect (event.GetId () == Dd60_Connect);
}

bool Dd60App::DoConnect (bool ask)
{
    Dd60Frame *frame;

#ifndef TEST
    if (ask)
    {
        Dd60ConnDialog dlg (wxID_ANY, _("Connect to DtCyber console"));
    
        dlg.CenterOnScreen ();
        
        if (dlg.ShowModal () == wxID_OK)
        {
            if (dlg.m_port.IsEmpty ())
            {
                m_port = DefDd60Port;
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
#endif

    // create the main application window
    frame = new Dd60Frame(m_port, wxT("Dd60"));

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

void Dd60App::OnAbout(wxCommandEvent&)
{
    wxString msg;

    msg.Printf (_T("DtCyber console (DD60) emulator %s.\n%s"),
                wxT ("V" DD60VERSION),
                _("Copyright \xA9 2005 by Paul Koning."));
    
    wxMessageBox(msg, _("About Dd60"), wxOK | wxICON_INFORMATION, NULL);
}

void Dd60App::OnPref (wxCommandEvent&)
{
    Dd60Frame *frame;
    wxString rgb;
    
    Dd60PrefDialog dlg (NULL, wxID_ANY, _("Dd60 Preferences"));
    
    if (dlg.ShowModal () == wxID_OK)
    {
        m_fgColor = dlg.m_fgColor;

        for (frame = m_firstFrame; frame != NULL; frame = frame->m_nextFrame)
        {
            frame->UpdateSettings (dlg.m_showStatusBar);
        }
        
        m_showStatusBar = dlg.m_showStatusBar;
        m_port = atoi (wxString (dlg.m_port).mb_str ());
        m_connect = dlg.m_connect;
        
        rgb.Printf (wxT ("%d %d %d"), 
                    dlg.m_fgColor.Red (), dlg.m_fgColor.Green (),
                    dlg.m_fgColor.Blue ());
        m_config->Write (wxT (PREF_FOREGROUND), rgb);
        m_config->Write (wxT (PREF_PORT), m_port);
        m_config->Write (wxT (PREF_CONNECT), (dlg.m_connect) ? 1 : 0);
        m_config->Write (wxT (PREF_STATUSBAR), (dlg.m_showStatusBar) ? 1 : 0);
        m_config->Flush ();
    }
}

wxColour Dd60App::SelectColor (wxColour &initcol)
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

void Dd60App::OnQuit(wxCommandEvent&)
{
    Dd60Frame *frame, *nextframe;

    frame = m_firstFrame;
    while (frame != NULL)
    {
        nextframe = frame->m_nextFrame;
        frame->Close (true);
        frame = nextframe;
    }
#if DD60_MDI // defined(__WXMAC__)
    // On the Mac, deleting all the windows doesn't terminate the
    // program, so we make it stop this way.
    ExitMainLoop ();
#endif
}


#if DD60_MDI
// MDI parent frame
Dd60MainFrame::Dd60MainFrame (void)
    : wxMDIParentFrame (NULL, wxID_ANY, wxT ("Dd60"),
                        wxDefaultPosition, wxDefaultSize, 0)
{
#if wxUSE_MENUS
    // create a menu bar

    wxMenu *menuFile = new wxMenu;
    menuFile->Append (Dd60_Connect, _("&New Connection...\tCtrl-N"),
                      _("Connect to DtCyber"));
    menuFile->Append (Dd60_Pref, _("P&references..."),
                      _("Set program configuration"));
    menuFile->AppendSeparator ();
    menuFile->Append (Dd60_Quit, _("E&xit\tCtrl-Q"), _("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append (menuFile, _("&File"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Dd60_About, _("&About Dd60"), _("Show about dialog"));
    
    menuBar->Append(helpMenu, wxT("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS
}
#endif

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
Dd60Frame::Dd60Frame(int port, const wxString& title)
  : Dd60FrameBase(Dd60FrameParent, -1, title,
		   wxDefaultPosition,
		   wxDefaultSize),
    traceDd60 (false),
    m_nextFrame (NULL),
    m_prevFrame (NULL),
    m_char8 (NULL),
    m_char16 (NULL),
    m_char32 (NULL),
    m_foregroundPen (dd60App->m_fgColor, dd60App->m_scale, wxSOLID),
    m_foregroundBrush (dd60App->m_fgColor, wxSOLID),
    m_canvas (NULL),
    m_port (port),
    m_timer (this, Dd60_Timer),
    mode (0),
    currentX (0),
    currentY (0),
    currentXOffset (0),
    truekb (false)
{
    int i;
    int true_opt = 1;

    // set the frame icon
//    SetIcon(wxICON(dd60_32));

#if wxUSE_MENUS
    // create a menu bar

    wxMenu *menuFile = new wxMenu;
    menuFile->Append (Dd60_Connect, _("&New Connection...\tCtrl-N"),
                      _("Connect to DtCyber"));

    // The accelerators actually will be Command-xxx on the Mac
    menuFile->Append (Dd60_SaveScreen, _("&Save Screen\tCtrl-S"),
                      _("Save screen image to file"));
    menuFile->Append (Dd60_Print, _("&Print...\tCtrl-P"),
                      _("Print screen content"));
    menuFile->Append (Dd60_Page_Setup, _("P&age Setup..."), _("Printout page setup"));
    menuFile->Append (Dd60_Preview, _("Print Pre&view"), _("Preview screen print"));
    menuFile->AppendSeparator ();
    menuFile->Append (Dd60_Pref, _("P&references..."),
                      _("Set program configuration"));
    menuFile->AppendSeparator ();
    menuFile->Append (Dd60_Close, _("&Close\tCtrl-W"),
                      _("Close this window"));
    menuFile->Append (Dd60_Quit, _("E&xit\tCtrl-Q"), _("Quit this program"));

    wxMenu *menuEdit = new wxMenu;

    menuEdit->Append (Dd60_CopyScreen, _("&Copy Screen\tCtrl-C"), _("Copy screen to clipboard"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append (menuFile, _("&File"));
    menuBar->Append (menuEdit, _("&Edit"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Dd60_About, _("&About Dd60"),
                     _("Show about dialog"));
    
    menuBar->Append(helpMenu, wxT("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

    // create a status bar, if this isn't a help window
    m_statusBar = new wxStatusBar (this, wxID_ANY);
    m_statusBar->SetFieldsCount (STATUSPANES);
    m_statusBar->SetStatusText(_(" Connecting..."), STATUS_CONN);
    if (dd60App->m_showStatusBar)
    {
        SetStatusBar (m_statusBar);
    }
    
#ifndef TEST
    SetCursor (*wxHOURGLASS_CURSOR);
#endif

    SetClientSize (XSize + 2, YSize + 2);
    m_screenmap = new wxBitmap (XSize, YSize, 32);
    m_canvas = new Dd60Canvas (this);

    /*
    **  Load character patterns characters
    */
    m_char8 = new u8[3 * DD60CHARS * CHAR8SIZE * CHAR8SIZE];
    m_char16 = new u8[3 * DD60CHARS * CHAR16SIZE * CHAR16SIZE];
    m_char32 = new u8[3 * DD60CHARS * CHAR32SIZE * CHAR32SIZE];

    dd60LoadChars ();

    // Open the connection
    dtInitFet (&m_fet, BufSiz);
#ifndef TEST
    if (dtConnect (&m_fet.connFd, "127.0.0.1", m_port) < 0)
    {
        wxString msg;
            
        m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);

        msg.Printf (_("Failed to connect to %d"), m_port);
            
        wxMessageDialog alert (this, msg, wxString (_("Alert")), wxOK);
            
        alert.ShowModal ();
        Close ();
        return;
    }
    m_statusBar->SetStatusText(_(" Connected"), STATUS_CONN);
    SetCursor (wxNullCursor);
    setsockopt (m_fet.connFd, SOL_SOCKET, SO_KEEPALIVE,
                (char *)&true_opt, sizeof(true_opt));
#ifdef __APPLE__
    setsockopt (m_fet.connFd, SOL_SOCKET, SO_NOSIGPIPE,
                (char *)&true_opt, sizeof(true_opt));
#endif

    // Set update rate zero which means "send me all data",
    // then turn on the display data stream.
    dd60SendKey (Dd60FastRate);
    dd60SendKey (Dd60KeyXon);
#endif  // TEST
    m_timer.Start (RRATE);
    
    Show(true);
}

Dd60Frame::~Dd60Frame ()
{
    int i;
    
    if (dtActive (&m_fet))
    {
        dtCloseFet (&m_fet);
    }
    if (m_char8 != NULL)
    {
        delete [] m_char8;
        delete [] m_char16;
        delete [] m_char32;
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
        dd60App->m_firstFrame = m_nextFrame;
#if !DD60_MDI
        if (dd60App->m_firstFrame == NULL)
        {
            dd60PanelFrame->Destroy ();
        }
#endif

    }
}

// event handlers

void Dd60Frame::OnTimer (wxTimerEvent &)
{
    int i, data;
    
#ifndef TEST
    if (!dtActive (&m_fet))
    {
        return;
    }
#endif

    m_pixmap = new PixelData (*m_screenmap);
    if (!m_pixmap)
    {
        // ... raw access to bitmap data unavailable, do something else ...
        printf ("no raw bitmap access\n");
        exit (1);
    }
    m_pixmap->UseAlpha ();

    // First decay the current screen content
    const int pixels = m_pixmap->GetWidth () * m_pixmap->GetHeight ();
    PixelData::Iterator p (*m_pixmap);
    const int bytesperpixel = PixelData::Iterator::PixelFormat::SizePixel;
    
    //printf ("pixels %d, pixel stride %d\n", pixels, bytesperpixel);
    
#if (DECAY == 128)
    const int wds = (pixels * /*bytesperpixel*/3) / 4;
    uint32_t *pmap = (uint32_t *)(p.m_ptr);
    uint32_t maxalpha, t;
    
    t = *pmap;
    *pmap = 0;
    p.Alpha () = 255;
    maxalpha = *pmap;
    *pmap = t;
    
    for (i = 0; i < wds; i++)
    {
        *pmap = ((*pmap >> 1) & 0x7f7f7f7f) | maxalpha;
        ++pmap;
    }
#else
    for (int pix = 0; pix < pixels; pix++)
    {
        u8 &rp = p.Red ();
        u8 &gp = p.Green ();
        u8 &bp = p.Blue ();
        rp = (rp * DECAY) / 256;
        gp = (gp * DECAY) / 256;
        bp = (bp * DECAY) / 256;
        p.Alpha () = 255;
        ++p;
    }
#endif

#ifndef TEST
    i = dtRead (&m_fet, 0);
    
    if (i < 0)
    {
        wxString msg;
            
        m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);

        msg.Printf (_("Connection lost to %d"), m_port);
            
        wxMessageDialog alert (this, msg, wxString (_("Alert")), wxOK);
            
        alert.ShowModal ();
        Close (TRUE);
        return;
    }
#endif

//    printf ("%d bytes from cyber\n", dtFetData (&m_fet));
    
#ifdef TEST
    sprintf ((char *) (tdata + 5), "%d %s", tcount++,
             (tcount & 32) ? "ON" : "  ");
    for (int t = 0; ; t++)
    {
        if (tdata[t + 5] == 0)
            break;
        
        tdata[t + 5] = asciiToCdc[tdata[t + 5]];
    }
    for (int t = 0; t < sizeof (tdata); t++)
    {
        data = tdata[t];
#else
    for (;;)
    {
        data = dtReado (&m_fet);
        if (data < 0)
        {
            break;
        }
#endif
        if ((data & 0200) == 0)
        {
            TRACE1 ("Char %o", data);
            procDd60Char (data);
        }
        else
        {
            switch (data & 0370)
            {
            case Dd60SetX:
            case Dd60SetY:
                /*
                **  SetX and SetY are two bytes long, so get
                **  the next byte and then proceed.  The second
                **  byte should be here already, but if not, we
                **  will wait for it.
                */
#ifdef TEST
                i = tdata[++t] | ((data & 7) << 8);
#else
                for (;;)
                {
                    i = dtReado (&m_fet);
                    if (i >= 0)
                    {
                        break;
                    }
                    dtRead (&m_fet, -1);
                }
                i |= ((data & 7) << 8);
#endif
                if ((data & 0370) == Dd60SetX)
                {
                    TRACE1 ("Set X %o", i);
                    currentX = i;
                }
                else
                {
                    currentY = 0777 - i;
                    if (mode == Dd60Dot)
                    {
                        // In dot mode, draw dots by plotting the '.' char
                        TRACE1 ("Dot at Y %o", i);
                        int saveX = currentX;
                        procDd60Char (057);
                        currentX = saveX;
                    }
                    else
                    {
                        TRACE1 ("Set Y %o", i);
                    }
                }
                break;
            case Dd60SetTrace:
                TRACEN ("Trace line");
                mode = Dd60CharSmall;
                currentXOffset = OffLeftScreen;
                currentX = TraceX;
                currentY = TraceY;
                break;
            case Dd60SetKbTrue:
                TRACE1 ("Set KB true mode %d", data & 1);
                truekb = (data & 1) != 0;
                break;
            case Dd60SetMode:
                TRACE1 ("Set mode %o", data & 077);
                if (data & Dd60ScreenR)
                {
                    currentXOffset = OffRightScreen;
                }
                else
                {
                    currentXOffset = OffLeftScreen;
                }
                // Save only char size or dot plotting mode bits
                mode = data & 3;
                break;
            }
        }
    }
    delete m_pixmap;
    
#if 0
    Refresh (false);
#else
    wxWindowDC dc (m_canvas);
    dc.BeginDrawing ();
    dc.DrawBitmap (*m_screenmap, 0, 0, false);
    dc.EndDrawing ();
#endif
}

void Dd60Frame::OnClose (wxCloseEvent &)
{
    int i;
    
    if (dtActive (&m_fet))
    {
        dtCloseFet (&m_fet);
    }

    Destroy ();
}

void Dd60Frame::OnQuit(wxCommandEvent&)
{
    // true is to force the frame to close
    Close (true);
}

void Dd60Frame::OnActivate (wxActivateEvent &event)
{
    if (m_canvas != NULL)
    {
        m_canvas->SetFocus ();
    }
    event.Skip ();        // let others see the event, too
}

void Dd60Frame::OnCopyScreen (wxCommandEvent &)
{
    wxBitmapDataObject *screen;

    screen = new wxBitmapDataObject(*m_screenmap);

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

void Dd60Frame::OnSaveScreen (wxCommandEvent &)
{
    wxString filename, ext;
    wxBitmapType type;
    wxFileDialog fd (this, _("Save screen to"), dd60App->m_defDir,
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
    
    wxImage screenImage = m_screenmap->ConvertToImage ();
    wxFileName fn (filename);
    
    dd60App->m_defDir = fn.GetPath ();
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

void Dd60Frame::OnPrint (wxCommandEvent &)
{
    wxPrintDialogData printDialogData (*g_printData);

    printDialogData.EnableSelection (false);
    printDialogData.EnablePageNumbers (false);
    
    wxPrinter printer (& printDialogData);
    Dd60Printout printout (this);
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

void Dd60Frame::OnPrintPreview (wxCommandEvent &)
{
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData (*g_printData);
    wxPrintPreview *preview = new wxPrintPreview (new Dd60Printout (this),
                                                  new Dd60Printout (this),
                                                  &printDialogData);

    printDialogData.EnableSelection (false);
    printDialogData.EnablePageNumbers (false);
    
    if (!preview->Ok())
    {
        delete preview;
        wxMessageBox(_("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _("Previewing"), wxOK);
        return;
    }

    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _("Dd60 Print Preview"), wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show();
}

void Dd60Frame::OnPageSetup(wxCommandEvent &)
{
    (*g_pageSetupData) = *g_printData;

    wxPageSetupDialog pageSetupDialog (this, g_pageSetupData);
    pageSetupDialog.ShowModal ();

    (*g_printData) = pageSetupDialog.GetPageSetupData ().GetPrintData ();
    (*g_pageSetupData) = pageSetupDialog.GetPageSetupData ();
}

void Dd60Frame::PrepareDC(wxDC& dc)
{
    dc.SetAxisOrientation (true, false);
    dc.SetBackground (*wxBLACK_BRUSH);
}

void Dd60Frame::dd60SetName (wxString &winName)
{
    wxString str;
    
    str.Printf (wxT("Dd60: %s"), winName.c_str ());
    SetTitle (str);
}

void Dd60Frame::dd60SetStatus (wxString &str)
{
    m_statusBar->SetStatusText(str, STATUS_CONN);
}

void Dd60Frame::dd60LoadChars (void)
{
    dd60LoadCharSize (8, CHAR8SIZE, m_char8);
    dd60LoadCharSize (16, CHAR16SIZE, m_char16);
    dd60LoadCharSize (32, CHAR32SIZE, m_char32);
#if 0
    u8 *d=m_char8;
    for (int c = 0; c < 060; c++)
    {
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                printf ("%02x ", *d++);
            }
            printf ("\n");
        }
        printf ("\n");
    }
#endif
}

static double bellvec[16 * 16];

void Dd60Frame::dd60LoadCharSize (int size, int tsize, u8 *vec)
{
    int ch, i, j, bx, by, ix, iy, cx, cy, pg;
    double x, y, scalex, scaley, r, b;
    unsigned char *pix;
    const double peak = 0.25;
    Chargen cg;
    bool on;
    const int margin = (tsize - size) / 2;
    // delta from one scanline to the next (Y to Y-1):
    const int nextline = tsize * 3;
    // offset to character origin (accounting for margins) -- 
    // note that this is a window system style origin, i.e., upper left.
    const int orgoffset = nextline * margin + margin * 3;
    // delta from one char to the next:
    const int nextchar = tsize * tsize * 3;
    // sizeX and sizeY are percentages of the nominal size
    const int width = size * dd60Panel->sizeX () / 100;
    const int height = size * dd60Panel->sizeY () / 100;
    const int rv = dd60App->m_fgColor.Red ();
    const int gv = dd60App->m_fgColor.Green ();
    const int bv = dd60App->m_fgColor.Blue ();
    const double intensity = normInt (dd60Panel->intensity (), height);
    const double sigma = dd60Panel->beamsize ();
    const int beamr = int (ceil (3 * sigma));
#define r1_029 2000
#define m_c1_029 300
#define ONDELAY 23
    const double r029 = r1_029;
    const double c029 = 1.0e-12 * m_c1_029;
    Delay delay (ONDELAY);
    
    memset (vec, 0, DD60CHARS * nextchar);
    
    i = stepcount (height);
    
    const int rcfreq = STEPHZ * i;
    RClow  x620 (470., 33.e-12, rcfreq), y620 (470., 33.e-12, rcfreq);
    const double peak620 = .3;
    RClow  xc19 (6800., 22.e-12, rcfreq), yc19 (6800., 22.e-12, rcfreq);
    const double peakc19 =.35;
    RClow  x029 (r029, c029, rcfreq), y029 (r029, c029, rcfreq);
    const double peak029 = .1;

    cg.SetStepCount (i);
    scalex = (double) width / (i * 8);
    scaley = (double) height / (i * 8);

    for (j = 0; j <= beamr * 16; j++)
    {
        bellvec[j] = bell (j / 16.0, sigma);
    }
    
    for (ch = 1; ch < DD60CHARS; ch++)
    {
        // pointer to 0,0 pixel in vec.  Note that vec is laid out
        // in conventional UI fashion, uppermost pixels first, but we
        // have Y increasing upward in proper mathematical form.  The
        // pixels for each char are together, so we effectively have
        // a vector of character patterns, each of which is a square
        // matrix of pixels.  We use grayscale pixels here; color is
        // added when we display the characters.
        u8 * const origin = vec + (nextchar * ch) + orgoffset;
        
        x620.Reset ();
        y620.Reset ();
        xc19.Reset ();
        yc19.Reset ();
        x029.Reset ();
        y029.Reset ();

        cg.Start (ch);
        while (!cg.Done ())
        {
            x = cg.X ();
            y = cg.Y ();
            on = cg.On ();
            
            delay.SetFlag (on);
            // Apply the various filters.  The low pass filter in
            // module C19 we apply directly.  The other filters
            // all are lowpass in a feedback path, so they are applied
            // as a fractional increase in the signal.
            x += x620.Step (x * peak620);
            y += y620.Step (y * peak620);
            x =  (x * (1 - peakc19)) + xc19.Step (x * peakc19);
            y =  (y * (1 - peakc19)) + yc19.Step (y * peakc19);
            x += x029.Step (x * peak029);
            y += y029.Step (y * peak029);
            x /= (1 + peak029);
            y /= (1 + peak029);
                
            on = delay.Flag ();

            if (on)
            {
                // Intensify a spot given by a normal distribution
                // around the current x/y.  We go out to 3 sigma.
                for (bx = -beamr; bx <= beamr; bx++)
                {
                    for (by = -beamr; by <= beamr; by++)
                    {
                        r = sqrt (bx * bx + by * by);
                        if (r > 3.0 * sigma)
                            continue;
                        b = bellvec[int (round (r)) * 16];
                        
                        ix = (int) round (x * scalex + bx);
                        iy = (int) round (y * scaley + by + height);
                        if (ix < -margin || iy < -margin ||
                            ix >= size + margin || iy >= size + margin)
                        {
                            continue;
                        }
                        
                        pix = origin + ix * 3 + (iy * nextline);
                        pg = pix[0];    // current red pixel value
                        pg += int (b * intensity * rv / 255);
                        if (pg > 255)
                            pg = 255;
                        pix[0] = pg;
                        pg = pix[1];    // current green pixel value
                        pg += int (b * intensity * gv / 255);
                        if (pg > 255)
                            pg = 255;
                        pix[1] = pg;
                        pg = pix[2];    // current blue pixel value
                        pg += int (b * intensity * bv / 255);
                        if (pg > 255)
                            pg = 255;
                        pix[2] = pg;
#if 0
                        if (size == 8 && bx == 0 && by == 0)
                            printf ("ch %o x %d y %d, pix %d\n", ch, ix, iy, pg);
#endif
                    }
                }
            }
            cg.Step ();
        }
    }
}

void Dd60Frame::UpdateSettings (bool newstatusbar)
{
    int i;
    wxBitmap *newmap;

    if (newstatusbar)
    {
        SetStatusBar (m_statusBar);
    }
    else
    {
        SetStatusBar (NULL);
    }
    
    dd60LoadChars ();
}

/*--------------------------------------------------------------------------
**  Purpose:        Process DD60 character data
**
**  Parameters:     Name        Description.
**                  d           character code
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void Dd60Frame::procDd60Char (unsigned int d)
{
    int size, margin, firstx, firsty, inc, qwds;
    u8 *data;
    int i, j, k;

    if (d > 057)
    {
        printf ("procDd60Char: unexpected char %o\n", d);
        return;
    }
    
    switch (mode)
    {
    case Dd60CharSmall:
    case Dd60Dot:
        inc = 8;
        size = CHAR8SIZE;
        qwds = 3 * CHAR8SIZE / 8;
        margin = 4;
        data = m_char8 + (d * 3 * CHAR8SIZE * CHAR8SIZE);
        break;
    case Dd60CharMedium:
        inc = 16;
        size = CHAR16SIZE;
        qwds = 3 * CHAR16SIZE / 8;
        margin = 8;
        data = m_char16 + (d * 3 * CHAR16SIZE * CHAR16SIZE);
        break;
    case Dd60CharLarge:
        inc = 32;
        size = CHAR32SIZE;
        qwds = 3 * CHAR32SIZE / 8;
        margin = 8;
        data = m_char32 + (d * 3 * CHAR32SIZE * CHAR32SIZE);
        break;
    }
    
    if (d != 0 && d != 055)
    {
        PixelData::Iterator p (*m_pixmap);

    
        firstx = XADJUST (currentX - margin);
        firsty = YADJUST (currentY - (size - margin));
    
        for (i = 0; i < size; i++)
        {
            p.MoveTo (*m_pixmap, firstx, firsty + i);
#ifdef __SSE2__
            typedef int v8qi __attribute__ ((mode(V8QI)));
            v8qi *pmap = (v8qi *)(p.m_ptr);
            v8qi *pdata = (v8qi *) data;
            
            for (j = 0; j < qwds; j++)
            {
                *pmap = __builtin_ia32_paddusb (*pmap, *pdata);
                ++pmap;
                ++pdata;
            }
            data += qwds * 8;
#else
            for (j = 0; j < size; j++)
            {
                u8 &rp = p.Red ();
                u8 &gp = p.Green ();
                u8 &bp = p.Blue ();
            
                k = rp + *data++;
                if (k > 255)
                {
                    k = 255;
                }
                rp = k;
                k = gp + *data++;
                if (k > 255)
                {
                    k = 255;
                }
                gp = k;
                k = bp + *data++;
                if (k > 255)
                {
                    k = 255;
                }
                bp = k;
                ++p;
            }
#endif
        }
    }
    currentX = (currentX + inc) & 0777;
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
void Dd60Frame::dd60SetTrace (bool fileaction)
{
    if (!traceDd60)
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
    dd60ShowTrace (traceDd60);
}

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Process console keyboard input
**
**  Parameters:     Name        Description.
**                  key         Key code for station
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void Dd60Frame::dd60SendKey(int key)
{
    char data;
    
    data = key;

    if (traceDd60)
    {
        fprintf (traceF, "key to plato %03o\n", key);
#ifdef DEBUG
        wxLogMessage ("key to plato %03o", key);
#endif
    }
    send (m_fet.connFd, &data, 1, 0);
}

void Dd60Frame::dd60ShowTrace (bool enable)
{
    if (enable)
    {
        m_statusBar->SetStatusText(_(" Trace "), STATUS_TRC);
    }
    else
    {
        m_statusBar->SetStatusText(wxT (""), STATUS_TRC);
    }
}

// ----------------------------------------------------------------------------
// Dd60PrefDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(Dd60PrefDialog, wxDialog)
    EVT_CLOSE(Dd60PrefDialog::OnClose)
    EVT_BUTTON(wxID_ANY,   Dd60PrefDialog::OnButton)
    EVT_CHECKBOX(wxID_ANY, Dd60PrefDialog::OnCheckbox)
    END_EVENT_TABLE()

Dd60PrefDialog::Dd60PrefDialog (Dd60Frame *parent, wxWindowID id, const wxString &title)
    : wxDialog (parent, id, title),
      m_owner (parent)
{
    wxBitmap fgBitmap (20, 12);
    wxBoxSizer *bs, *ds, *ods;
    wxStaticBoxSizer *sbs;
    wxFlexGridSizer *fgs;
    
    m_showStatusBar = dd60App->m_showStatusBar;
    m_connect = dd60App->m_connect;
    m_fgColor = dd60App->m_fgColor;
    m_port.Printf (wxT ("%d"), dd60App->m_port);

    paintBitmap (fgBitmap, m_fgColor);
    
    // Create the pieces of the dialog from outermost to innermost.
    // Order matters on some platforms, because it establishes Z-order
    // of the controls.

    // Sizer for the whole dialog box content
    ds = new wxBoxSizer (wxVERTICAL);

    // First group: connection settings
    sbs = new wxStaticBoxSizer (new wxStaticBox (this, wxID_ANY,
                                                 _("Connection settings")),
                                wxVERTICAL);
    m_autoConnect = new wxCheckBox (this, -1, _("Connect at startup"));
    m_autoConnect->SetValue (m_connect);
    sbs->Add (m_autoConnect, 0,   wxTOP | wxLEFT | wxRIGHT, 8);
    fgs = new wxFlexGridSizer (1, 2, 8, 8);
    m_portText = new wxTextCtrl (this, wxID_ANY, m_port,
                                 wxDefaultPosition, wxSize (160+40, 18),
                                 0, *new wxTextValidator (wxFILTER_NUMERIC,
                                                          &m_port));
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Default Port")));
    fgs->Add (m_portText);
    sbs->Add (fgs, 0, wxALL, 8);
    ds->Add (sbs, 0,  wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 10);

    // Second group: Display settings
    sbs = new wxStaticBoxSizer (new wxStaticBox (this, wxID_ANY,
                                                 _("Display settings")),
                                wxVERTICAL);
    m_statusCheck = new wxCheckBox (this, -1, _("Status bar"));
    m_statusCheck->SetValue (m_showStatusBar);
    sbs->Add (m_statusCheck, 0, wxALL, 8);
    fgs = new wxFlexGridSizer (2, 2, 8, 8);
    m_fgButton = new wxBitmapButton (this, wxID_ANY, fgBitmap);
    fgs->Add (m_fgButton);
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Foreground")));
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

void Dd60PrefDialog::OnButton (wxCommandEvent& event)
{
    wxBitmap fgBitmap (20, 12);
    
    if (event.GetEventObject () == m_fgButton)
    {
        m_fgColor = Dd60App::SelectColor (m_fgColor);
    }
    
    else if (event.GetEventObject () == m_okButton)
    {
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
        m_connect = true;
        m_autoConnect->SetValue (true);
        m_showStatusBar = true;
        m_statusCheck->SetValue (true);
        str.Printf (wxT ("%d"), DefDd60Port);
        m_portText->SetValue (str);
    }
    paintBitmap (fgBitmap, m_fgColor);
    m_fgButton->SetBitmapLabel (fgBitmap);
    Refresh ();
}

void Dd60PrefDialog::OnCheckbox (wxCommandEvent& event)
{
    if (event.GetEventObject () == m_autoConnect)
    {
        m_connect = event.IsChecked ();
    }
    else if (event.GetEventObject () == m_statusCheck)
    {
        m_showStatusBar = event.IsChecked ();
    }
}

void Dd60PrefDialog::paintBitmap (wxBitmap &bm, wxColour &color)
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
// Dd60ConnDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(Dd60ConnDialog, wxDialog)
    EVT_CLOSE(Dd60ConnDialog::OnClose)
    EVT_BUTTON(wxID_OK, Dd60ConnDialog::OnOK)
    END_EVENT_TABLE()

Dd60ConnDialog::Dd60ConnDialog (wxWindowID id, const wxString &title)
    : wxDialog (NULL, id, title)
{
    wxBoxSizer *ds, *ods;
    wxFlexGridSizer *fgs;

    // Sizer for the whole dialog box content
    ds = new wxBoxSizer (wxVERTICAL);

    // First section: the text controls
    m_port.Printf (wxT ("%d"), dd60App->m_port);

    m_portText = new wxTextCtrl (this, wxID_ANY, m_port,
                                 wxDefaultPosition, wxSize (160, 18),
                                 0, *new wxTextValidator (wxFILTER_NUMERIC,
                                                          &m_port));
    fgs = new wxFlexGridSizer (1, 2, 8, 8);
      
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

void Dd60ConnDialog::OnOK (wxCommandEvent &)
{
        m_port = m_portText->GetLineText (0);
        EndModal (wxID_OK);
}

// ----------------------------------------------------------------------------
// Dd60Panel
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(Dd60Panel, wxPanel)
    EVT_SCROLL(Dd60Panel::OnScroll)
END_EVENT_TABLE()

Dd60Panel::Dd60Panel (wxFrame *parent) :
    wxPanel (parent)
{
    wxBitmapButton *b;
    wxSizerFlags sf;
    wxSize knobSize (40, 40);
    
    sf.Align (wxALIGN_CENTER);
    sf.Border (wxLEFT | wxRIGHT | wxTOP);
    m_sizer = new wxFlexGridSizer (2, 4, 2, 8);
    m_sizeX = new wxKnob (this, wxID_ANY, 100, 50, 200,
                          240, 300, wxDefaultPosition, knobSize);
    m_sizer->Add (m_sizeX, sf);
    m_sizeY = new wxKnob (this, wxID_ANY, 100, 50, 200,
                          240, 300, wxDefaultPosition, knobSize);
    m_sizer->Add (m_sizeY, sf);
    m_focus = new wxKnob (this, wxID_ANY, 100, 0, 250,
                          240, 300, wxDefaultPosition, knobSize);
    m_sizer->Add (m_focus, sf);
    m_intens = new wxKnob (this, wxID_ANY, 160, 1, 400,
                           240, 300, wxDefaultPosition, knobSize);
    m_sizer->Add (m_intens, sf);
    sf.Border (wxLEFT | wxRIGHT | wxBOTTOM);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("X Size")), sf);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("Y Size")), sf);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("Focus")), sf);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("Intensity")), sf);
    SetSizerAndFit (m_sizer);
//    m_sizer->SetSizeHints (this);
    parent->SetClientSize (GetSize ());
//    m_sizer->RecalcSizes ();
}

void Dd60Panel::OnScroll (wxScrollEvent &)
{
    Dd60Frame *frame;
    
    for (frame = dd60App->m_firstFrame; frame != NULL; frame = frame->m_nextFrame)
    {
        frame->dd60LoadChars ();
    }
}

// ----------------------------------------------------------------------------
// Dd60Canvas
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them.
BEGIN_EVENT_TABLE(Dd60Canvas, wxScrolledWindow)
    EVT_KEY_DOWN(Dd60Canvas::OnKey)
    EVT_KEY_UP(Dd60Canvas::OnKey)
    EVT_ERASE_BACKGROUND(Dd60Canvas::OnEraseBackground)
    END_EVENT_TABLE ()

Dd60Canvas::Dd60Canvas(Dd60Frame *parent)
    : wxScrolledWindow(parent, -1, wxDefaultPosition, 
                       wxSize (XSize, YSize),
                       wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxClientDC dc(this);

    m_owner = parent;
    SetVirtualSize (XSize, YSize);
    
    SetBackgroundColour (*wxBLACK);
    SetScrollRate (1, 1);
    SetFocus ();
}

void Dd60Canvas::OnDraw(wxDC &dc)
{
    dc.DrawBitmap (*m_owner->m_screenmap, 0, 0, false);
}

void Dd60Canvas::OnEraseBackground (wxEraseEvent &)
{
    // Do nothing, to avoid flashing.
}

void Dd60Canvas::OnKey (wxKeyEvent &event)
{
    unsigned int key;

    key = event.m_keyCode;
    if (key == WXK_ALT || key == WXK_SHIFT || key == WXK_CONTROL)
    {
        // We don't take any action on the modifier key keydown events,
        // but we do want to make sure they are seen by the rest of
        // the system.
        // The same applies to keys sent to the help window (which has
        // no connection on which to send them).
        event.Skip ();
        return;
    }

    if ((!m_owner->truekb || event.m_altDown || event.m_controlDown) &&
        event.GetEventType () == wxEVT_KEY_UP)
    {
        // Ignore key up events if we're not doing true keyboard
        // simulation, or if Alt or Control modifiers are present.
        event.Skip ();
        return;
    }
    
    if (key < 0200)
    {
        if (isalpha (key))
        {
            key = tolower (key);
        }
        else if (event.m_shiftDown)
        {
            switch (key)
            {
            case '=':
                key = '+';
                break;
            case '8':
                key = '*';
                break;
            case '9':
                key = '(';
                break;
            case '0':
                key = ')';
                break;
            default:
                key = 0;
            }
        }
    }

    if (m_owner->traceDd60)
    {
        fprintf (traceF, "ctrl %d shift %d alt %d key %d\n", event.m_controlDown, event.m_shiftDown, event.m_altDown, key);
    }

    if (event.m_controlDown)
    {
        if (key == ']')         // control-] : trace
        {
            m_owner->traceDd60 = !m_owner->traceDd60;
            m_owner->dd60SetTrace (true);
            return;
        }
        event.Skip ();
        return;
    }

    if (event.m_altDown)
    {
        switch (key)
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
            key = Dd60TracePp0 + (key - '0');
            break;
        case 'c':
            if (event.m_shiftDown)
            {
                key = Dd60TraceCp1;
            }
            else
            {
                key = Dd60TraceCp0;
            }
            break;
        case 'e':
            key = Dd60TraceEcs;
            break;
        case 'j':
            key = Dd60TraceXj;
            break;
        case 'x':
            key = Dd60TraceAll;
            break;
        case 'q':
            key = Dd60KeyXon;
            break;
        case 's':
            key = Dd60KeyXoff;
            break;
        case 'z':
            Close ();
            return;
        default:
            key = 0;
        }
    }
    else
    {
        if (key != 0 &&
            key <= sizeof (asciiToConsole) / sizeof (asciiToConsole[0]))
        {
            key = asciiToConsole[key];
        }
        else
        {
            key = 0;
        }
        
        if (key != 0 && event.GetEventType () == wxEVT_KEY_UP)
        {
            /* this is a "key up" message */
            key |= 0200;
        }
    }
    if (key == 0)
    {
        event.Skip ();
    }
    else
    {
        m_owner->dd60SendKey (key);
    }
}


// ----------------------------------------------------------------------------
// Dd60 stroke generator class
// ----------------------------------------------------------------------------

void Chargen::Step (void)
{
    u8 glyph;
    if (Done ())
    {
        return;
    }
    
    glyph = *m_chardata;
    
    if (m_step == 0 && (glyph & 1))
    {
        m_on = !m_on;
    }
    switch ((glyph >> 1) & 3)
    {
    case 1:
        m_x += m_dx * 2.0;
        break;
    case 2:
        m_x += m_dx;
        break;
    case 3:
        if (m_step == 0)
        {
            m_dx = -m_dx;
        }
        break;
    }
    switch (glyph >> 3)
    {
    case 1:
        m_y += m_dy * 2.0;
        break;
    case 2:
        m_y += m_dy;
        break;
    case 3:
        if (m_step == 0)
        {
            m_dy = -m_dy;
        }
        break;
    }

    if (++m_step == m_stepcount)
    {
        ++m_stroke;
        ++m_chardata;
        m_step = 0;
    }
}

// ----------------------------------------------------------------------------
// Dd60 printing helper class
// ----------------------------------------------------------------------------

bool Dd60Printout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument (startPage, endPage))
        return false;

    return true;
}

bool Dd60Printout::OnPrintPage (int page)
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

void Dd60Printout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool Dd60Printout::HasPage(int pageNum)
{
    return (pageNum == 1);
}

void Dd60Printout::DrawPage (wxDC *dc)
{
    wxMemoryDC screenDC;
    int ofr, ofg, ofb;
    double maxX = XSize;
    double maxY = YSize;

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
    double pscaleX = (double) (w / maxX);
    double pscaleY = (double) (h / maxY);

    // Use x or y scaling factor, whichever fits on the DC
    double actualScale = wxMin(pscaleX,pscaleY);

    // Calculate the position on the DC for centring the graphic
    double posX = (double) ((w - (XSize * actualScale)) / 2.0);
    double posY = (double) ((h - (YSize * actualScale)) / 2.0);

    double r, g, b;
    int graypix;
    
    // Set the scale and origin
    dc->SetUserScale (actualScale, actualScale);
    dc->SetDeviceOrigin ((long) posX, (long) posY);

    // Re-color the image
    wxImage screenImage = m_owner->m_screenmap->ConvertToImage ();

    unsigned char *data = screenImage.GetData ();
    
    w = screenImage.GetWidth ();
    h = screenImage.GetHeight ();

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            r = data[0] / 255.0;
            g = data[1] / 255.0;
            b = data[2] / 255.0;

            // convert to grayscale
            r = r * 0.3 + g * 0.59 + b * 0.11;
            graypix = int (r * 255);
            if (graypix > 255)
            {
                graypix = 255;
            }

            // Invert it so text is dark on white background
            graypix = 255 - graypix;
            data[0] = data[1] = data[2] = graypix;

            data += 3;
        }
    }

    wxBitmap printmap (screenImage);

    screenDC.SelectObject (printmap);
    dc->Blit (0, 0, XSize, YSize, &screenDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);
}

/*---------------------------  End Of File  ------------------------------*/
