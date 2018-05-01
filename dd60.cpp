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

#define NetBufSize      65536

#define STATUS_TRC      0
#define STATUS_CONN     2

#define STATUSPANES     3
#define KNOBPANE        1

// For Retina display support on the Mac, we need to deal with the
// fact that display units are not actually pixels, but "points" [sic]
// and that the actual pixels may be smaller.  
// Character pattern sizes in points, allowing for going over the
// allotted space some.
#define CHAR8SIZE       (16 * 8)
#define CHAR16SIZE      (32 * 8)
#define CHAR32SIZE      (48 * 8)

#define DD60CHARS       060         // number of characters in pattern arrays

// Display parameters
#define DECAY           128         // decay per refresh, scaled by 256.
#define DefaultInterval 0.06
#define DefRemoteInterval 3.0

#if  defined(__WXMAC__)
#define SmallPointSize  12
#else
#define SmallPointSize  10
#endif

// Default preference settings
#define DefSizeX        84
#define DefSizeY        99
#define DefFocus        82
#define DefIntensity    160         // For fast (decaying) refresh
#define DefSlowIntens   195         // For slow (replacing) refresh

//#define DisplayMargin   8

// Literal strings for wxConfig key strings.  These are defined
// because they appear in two places, so this way we avoid getting
// the two out of sync.  Note that they should *not* be changed after
// once being defined, since that invalidates people's stored
// preferences.
#define PREF_FOREGROUND "foreground"
#define PREF_PORT       "port"
#define PREF_CONNECT    "autoconnect"
#define PREF_SIZEX      "sizeX"
#define PREF_SIZEY      "sizeY"
#define PREF_FOCUS      "focus"
#define PREF_FASTINTENS "intensity"
#define PREF_SLOWINTENS "slowintensity"

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

#define TRACE2(str, arg1, arg2)                                         \
    if (traceDd60)                                                 \
        {                                                           \
            fprintf (traceF, str "\n", arg1, arg2);                       \
        }

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef _WIN32
#include <wx/setup.h>
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
#include <wx/validate.h>
#include <wx/valnum.h>
#include <wx/aboutdlg.h>

extern "C"
{
#if defined(_WIN32)
#include <winsock.h>
#include <process.h>
#define round(x) floor ((x) + 0.5)
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif
#include <stdlib.h>
#include "const.h"
#include "types.h"
#include "proto.h"
#include "dd60version.h"
#include "ptermversion.h"
#include "iir.h"
#include "knob.h"

#include "dd60.h"

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

extern float mainDisplayScale (void);
}

#ifdef __AVX2__
#define VECSIZE 32
#else
#ifdef __SSE2__
#define VECSIZE 16
#else
#define VECSIZE 0
#endif
#endif
#if VECSIZE
typedef char bytevec __attribute__ ((vector_size (VECSIZE)));
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources)
#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__) || defined(__WXMGL__) || defined(__WXX11__)
//#include "dd60.xpm"
#endif

#include "chargen.h"

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

bool emulationActive = true;

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
                  const wxString &title = _("DD60 printout"));
    bool OnPrintPage (int page);
    bool HasPage (int page);
    void GetPageInfo (int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
    void DrawPage (wxDC *dc, int page);
    
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

    bool DoConnect (bool ask, double interval = 0.0);
    static wxColour SelectColor (wxColour &initcol);
    
    void WritePrefs (void);
    
    wxColour    m_fgColor;
    wxConfig    *m_config;

    long        m_port;
    
    int         m_sizeX;
    int         m_sizeY;
    int         m_focus;
    int         m_fastintens;
    int         m_slowintens;
    bool        m_connect;
    double      m_interval;
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

class Dd60StatusBar : public wxBoxSizer
{
public:
    Dd60StatusBar (Dd60Frame *parent);
    void SetPanel (Dd60Panel *panel);
    void SetStatusText (const wxChar *str, int idx);
    
    wxWindow *m_parent;
    wxStaticText *msg1;
    wxStaticText *msg3;
    Dd60Panel *m_panel;
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
    friend void Dd60Printout::DrawPage (wxDC *dc, int page);
    
    typedef wxAlphaPixelData PixelData;

public:
    // ctor(s)
    Dd60Frame(int port, double interval, const wxString& title);
    ~Dd60Frame ();

    // Callback handlers
    static void connCallback (NetFet *np, int portNum, void *arg);
    static void dataCallback (NetFet *np, int bytes, void *arg);

    // event handlers (these functions should _not_ be virtual)
    void OnClose (wxCloseEvent& event);
    void OnIdle (wxIdleEvent& event);
    void OnQuit (wxCommandEvent& event);
    void OnCopyScreen (wxCommandEvent &event);
    void OnSaveScreen (wxCommandEvent &event);
    void OnPrint (wxCommandEvent& event);
    void OnPrintPreview (wxCommandEvent& event);
    void OnPageSetup (wxCommandEvent& event);
    void OnActivate (wxActivateEvent &event);
    void UpdateSettings (void);
    
    void PrepareDC(wxDC& dc);
    void dd60SendKey(int key);
    void dd60SetTrace (bool fileaction);
    void dd60LoadChars (void);

    bool        traceDd60;
    Dd60Frame   *m_nextFrame;
    Dd60Frame   *m_prevFrame;
    bool        truekb;
    int         m_intens;
    bool        m_fastupdate;
    int         m_pscale;
    int         m_xsize;
    int         m_ysize;
    int         m_displaymargin;

    int xadjust (int x) const
    {
        return (x + currentXOffset) * m_pscale + m_displaymargin;
    }
    int yadjust (int y) const
    {
        return y * m_pscale + m_displaymargin;
    }
    
private:
    bool        m_firstTime;
    Dd60StatusBar *m_statusBar;
    wxPen       m_foregroundPen;
    wxBrush     m_foregroundBrush;
    wxBitmap    *m_screenmap;
    PixelData   *m_pixmap;
    u32    m_maxalpha;
    u32    m_red;
    u32    m_green;
    u32    m_blue;
    
    Dd60Canvas  *m_canvas;
    NetPortSet  m_portset;
    NetFet      *m_fet;
    int         m_port;
    int         m_interval;
    bool        m_startBlock;

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

    // Trace related state
    unsigned int trace_idx;
    char        trace_txt[120];
    wxFont      m_traceFont;
    
    wxBoxSizer  *m_sizer;

    // Other stuff
    int sse2;
    int avx2;
    
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
    wxString        m_port;
    
private:
    void paintBitmap (wxBitmap &bm, wxColour &color);
    
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
    wxComboBox      *m_delayText;
    
    wxString        m_port;
    wxString        m_delay;
    
private:
    DECLARE_EVENT_TABLE ()
};


// Define a panel for the controls. 
class Dd60Panel : public wxPanel
{
public:
    Dd60Panel (Dd60Frame *parent);

    bool AcceptsFocus (void) const { return false; }

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
    Dd60Frame   *m_parent;
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
    EVT_IDLE(Dd60Frame::OnIdle)
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

void Dd60Frame::connCallback (NetFet *fet, int, void *arg)
{
    wxWakeUpIdle ();
}

void Dd60Frame::dataCallback (NetFet *, int, void *arg)
{
    wxWakeUpIdle ();
}

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool Dd60App::OnInit (void)
{
    int r, g, b;
    wxString rgb;
    double interval;
    
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

    if (argc > 3)
    {
        printf ("usage: dd60 [ interval [ portnum ]]\n");
        exit (1);
    }

    m_config = new wxConfig (wxT ("Dd60"));

    if (argc > 2)
    {
        argv[2].ToCLong (&m_port);
    }
    else
    {
        m_port = m_config->Read (wxT (PREF_PORT), DefDd60Port);
    }

    if (argc > 1)
    {
        argv[1].ToCDouble (&interval);
        if (interval != 0.0 && (interval < 0.02 || interval > 63.0))
        {
            fprintf (stderr, "interval value out of range\n");
            exit (1);
        }
    }
    else
    {
        if (m_port == DefDd60Port)
        {
            interval = DefaultInterval;
        }
        else
        {
            interval = DefRemoteInterval;
        }
    }

    // Use this interval as the initial default
    m_interval = interval;

    // 20 255 80 is RGB for DD60 green
    m_config->Read (wxT (PREF_FOREGROUND), &rgb, wxT ("20 255 80"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_fgColor = wxColour (r, g, b);
    m_connect = (m_config->Read (wxT (PREF_CONNECT), 1) != 0);

    m_sizeX = m_config->Read (wxT (PREF_SIZEX), DefSizeX);
    m_sizeY = m_config->Read (wxT (PREF_SIZEY), DefSizeY);
    m_focus = m_config->Read (wxT (PREF_FOCUS), DefFocus);
    m_fastintens = m_config->Read (wxT (PREF_FASTINTENS), DefIntensity);
    m_slowintens = m_config->Read (wxT (PREF_SLOWINTENS), DefSlowIntens);
    
#if DD60_MDI
    // On Mac, the style rule is that the application keeps running even
    // if all its windows are closed.
//    SetExitOnFrameDelete(false);
    Dd60FrameParent = new Dd60MainFrame ();
    Dd60FrameParent->Show (true);
#endif

    // Add some handlers so we can save the screen in various formats
    // Note that the BMP handler is always loaded, don't do it again.
    wxImage::AddHandler (new wxPNGHandler);
    wxImage::AddHandler (new wxPNMHandler);
    wxImage::AddHandler (new wxTIFFHandler);
    wxImage::AddHandler (new wxXPMHandler);

    // create the main application window
    // If arguments are present, always connect without asking
    if (!DoConnect (!(m_connect || argc > 1), interval))
    {
        return false;
    }
    
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

    return 0;
}

void Dd60App::OnConnect (wxCommandEvent &event)
{
    DoConnect (event.GetId () == Dd60_Connect);
}

bool Dd60App::DoConnect (bool ask, double interval)
{
    Dd60Frame *frame;

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
                dlg.m_port.ToCLong (&m_port);
            }
            if (dlg.m_delay.IsEmpty ())
            {
                if (m_port == DefDd60Port)
                {
                    interval = DefaultInterval;
                }
                else
                {
                    interval = DefRemoteInterval;
                }
            }
            else
            {
                dlg.m_delay.ToCDouble (&interval);
                if (interval != 0.0 && (interval < 0.02 || interval > 63.0))
                {
                    fprintf (stderr, "interval value out of range\n");
                    return false;
                }
            }
        }
        else
        {
            return false;     // connect canceled
        }
    }

    // create the main application window
    frame = new Dd60Frame(m_port, interval, wxT("Dd60"));

    if (frame != NULL)
    {
        if (m_firstFrame != NULL)
        {
            m_firstFrame->m_prevFrame = frame;
        }
        frame->m_nextFrame = m_firstFrame;
        m_firstFrame = frame;

        // Use this interval as the default next time
        m_interval = interval;
    }
    
    return (frame != NULL);
}

#ifdef __AVX2__
#define DD60_SSE L" with AVX2."
#else
#ifdef __SSE2__
#define DD60_SSE L" with SSE2."
#else
#define DD60_SSE L"."
#endif
#endif
void Dd60App::OnAbout(wxCommandEvent&)
{
    wxAboutDialogInfo info;

    info.SetName (wxT ("DD60"));
    info.SetVersion (wxT ("V" DD60VERSION));
    info.SetDescription (_("DtCyber console (DD60) emulator" DD60_SSE
                           L"\n  built with wxWidgets V" wxT (WXVERSION)
                           L"\n  build date " wxT(PTERMBUILDDATE)));
    info.SetCopyright (wxT("(C) 2004-2018 by Paul Koning"));
    info.AddDeveloper ("Paul Koning");

    wxAboutBox(info);
}

void Dd60App::OnPref (wxCommandEvent&)
{
    Dd60Frame *frame;
    Dd60PrefDialog dlg (NULL, wxID_ANY, _("Dd60 Preferences"));
    
    if (dlg.ShowModal () == wxID_OK)
    {
        m_fgColor = dlg.m_fgColor;

        for (frame = m_firstFrame; frame != NULL; frame = frame->m_nextFrame)
        {
            frame->UpdateSettings ();
        }
        
        m_port = atoi (wxString (dlg.m_port).mb_str ());
        m_connect = dlg.m_connect;
        
        WritePrefs ();
    }
}

void Dd60App::WritePrefs (void)
{
    wxString rgb;

    rgb.Printf (wxT ("%d %d %d"), 
                m_fgColor.Red (), m_fgColor.Green (),
                m_fgColor.Blue ());

    m_config->Write (wxT (PREF_FOREGROUND), rgb);
    m_config->Write (wxT (PREF_PORT), m_port);
    m_config->Write (wxT (PREF_CONNECT), (m_connect) ? 1 : 0);
    m_config->Write (wxT (PREF_SIZEX), m_sizeX);
    m_config->Write (wxT (PREF_SIZEY), m_sizeY);
    m_config->Write (wxT (PREF_FOCUS), m_focus);
    m_config->Write (wxT (PREF_FASTINTENS), m_fastintens);    
    m_config->Write (wxT (PREF_SLOWINTENS), m_slowintens);    
    m_config->Flush ();
}

wxColour Dd60App::SelectColor (wxColour &initcol)
{
    wxColour col (initcol);
    wxColour dd60green (20, 255, 80);
    wxColourData data;

    data.SetColour (initcol);
    data.SetCustomColour (0, dd60green);
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
Dd60Frame::Dd60Frame(int port, double interval, const wxString& title)
  : Dd60FrameBase(Dd60FrameParent, -1, title,
		   wxDefaultPosition,
		   wxDefaultSize),
    traceDd60 (false),
    m_nextFrame (NULL),
    m_prevFrame (NULL),
    truekb (false),
    m_firstTime (true),
    m_foregroundPen (dd60App->m_fgColor, 1, wxPENSTYLE_SOLID),
    m_foregroundBrush (dd60App->m_fgColor, wxBRUSHSTYLE_SOLID),
    m_canvas (NULL),
    m_port (port),
    m_startBlock (true),
    m_char8 (NULL),
    m_char16 (NULL),
    m_char32 (NULL),
    mode (0),
    currentX (0),
    currentY (0),
    currentXOffset (0)
{
    int i;
    Dd60Panel *panel;
    
    trace_txt[0] = '\0';

#ifdef __SSE2__
    sse2 = __builtin_cpu_supports ("sse2");
    avx2 = __builtin_cpu_supports ("avx2");
#else
    sse2 = avx2 = 0;
#endif
    
    // Calculate scale factors and sizes
    m_pscale = GetContentScaleFactor ();
    m_displaymargin = 20 * m_pscale;
    m_xsize = (01000 + OffRightScreen) * m_pscale + 2 * m_displaymargin;
    m_ysize = 01000 * m_pscale + 2 * m_displaymargin;
    
    /* 
    **  Convert the interval to an integer value in 20 ms units
    */
    m_interval = interval / 0.02;
    m_fastupdate = (interval <= 0.5);
    
    if (m_interval > 077)
    {
        /* If it doesn't fit, use the "slow" coding */
        m_interval = Dd60SlowRate + (m_interval / 50);
    }
    else
    {
        m_interval += Dd60FastRate;
    }

    if (m_fastupdate)
    {
        m_intens = dd60App->m_fastintens;
    }
    else
    {
        m_intens = dd60App->m_slowintens;
    }

    m_sizer = new wxBoxSizer (wxVERTICAL);
    SetSizer (m_sizer);
    m_sizer->Fit (this);
    
    // Create the font for the trace line
    m_traceFont = wxFont (SmallPointSize, wxFONTFAMILY_MODERN,
                          wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    if (!m_traceFont.IsOk ())
    {
        fprintf (stderr, "Failed to allocate font\n");
        exit (1);
    }

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
    m_statusBar = new Dd60StatusBar (this);
    panel = new Dd60Panel (this);
    m_statusBar->SetPanel (panel);
    m_statusBar->SetStatusText(_(" Connecting..."), STATUS_CONN);
    
    SetCursor (*wxHOURGLASS_CURSOR);

    SetClientSize ((m_xsize + 2) / m_pscale, (m_ysize + 2) / m_pscale);
    m_screenmap = new wxBitmap (m_xsize, m_ysize, 32);
    m_pixmap = new PixelData (*m_screenmap);
    if (!m_pixmap)
    {
        // ... raw access to bitmap data unavailable, do something else ...
        printf ("no raw bitmap access\n");
        exit (1);
    }
    PixelData::Iterator p (*m_pixmap);
    u32 *pmap = (u32 *)(p.m_ptr);
    u8 *pb = (u8 *) pmap;

    *pmap = 0;
    p.Alpha () = 255;
    m_maxalpha = *pmap;
    *pmap = 0;
    p.Red () = 1;
    for (i = 0; i < 4; i++)
    {
        if (pb[i]) m_red = i;
    }
    *pmap = 0;
    p.Green () = 1;
    for (i = 0; i < 4; i++)
    {
        if (pb[i]) m_green = i;
    }
    *pmap = 0;
    p.Blue () = 1;
    for (i = 0; i < 4; i++)
    {
        if (pb[i]) m_blue = i;
    }
    const int bytesperpixel = PixelData::Iterator::PixelFormat::SizePixel;
    const int pixels = m_pixmap->GetWidth () * m_pixmap->GetHeight ();
    const int wds = (pixels * bytesperpixel) / 4;
    for (int i = 0; i < wds; i++)
    {
        *pmap++ = m_maxalpha;
    }
    delete m_pixmap;
    m_pixmap = NULL;
    
    //printf ("%d %d %d %d\n", m_maxalpha, m_red, m_green, m_blue);
    
    m_canvas = new Dd60Canvas (this);

    /*
    **  Load character patterns characters
    */
    m_char8 = new u8[4 * DD60CHARS * CHAR8SIZE * CHAR8SIZE];
    m_char16 = new u8[4 * DD60CHARS * CHAR16SIZE * CHAR16SIZE];
    m_char32 = new u8[4 * DD60CHARS * CHAR32SIZE * CHAR32SIZE];

    dd60LoadChars ();

    memset (&m_portset, 0, sizeof (m_portset));
    m_portset.maxPorts = 1;
    m_portset.callBack = connCallback;
    m_portset.dataCallBack = dataCallback;
    m_portset.ringSize = NetBufSize;
    m_portset.sendRingSize = 0;     // We don't use the send ring/thread
    dtInitPortset (&m_portset);
    
    // Open the connection
    m_fet = dtConnect (&m_portset, inet_addr ("127.0.0.1"), m_port);
    if (m_fet == NULL)
    {
        wxString msg;
            
        m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);

        msg.Printf (_("Failed to connect to %d"), m_port);
            
        wxMessageDialog alert (this, msg, wxString (_("Alert")), wxOK);
            
        alert.ShowModal ();
        Close ();
        return;
    }
    m_statusBar->SetStatusText(_(" Connecting"), STATUS_CONN);

    m_sizer->Add (m_canvas);
    m_sizer->Add (m_statusBar);
    SetSizer (m_sizer);
    m_sizer->Fit (this);
    Show(true);
}

Dd60Frame::~Dd60Frame ()
{
    if (dtActive (m_fet))
    {
        dtClose (m_fet, true);
        m_fet = NULL;
    }
    if (m_char8 != NULL)
    {
        delete [] m_char8;
        delete [] m_char16;
        delete [] m_char32;
        m_char8 = m_char16 = m_char32 = NULL;
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
    }
}

// event handlers

void Dd60Frame::OnIdle (wxIdleEvent &event)
{
    int i, data;
    static int pendingData = 0;
#if DEBUG || TIMING
    struct timeval t, t2;
    int datacount;
    int dt, dut;
#endif
    
    // In every case, let others see this event too.
    event.Skip ();

    if (!dtConnected (m_fet))
    {
        m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);
        return;
    }

    // First time through here?  That means the connection just went live.
    if (m_firstTime)
    {
        m_firstTime = false;
        m_statusBar->SetStatusText(_(" Connected"), STATUS_CONN);
        SetCursor (wxNullCursor);

        // Set update rate, then turn on the display data stream.
        dd60SendKey (m_interval);
        dd60SendKey (Dd60KeyXon);
    }

    if (m_interval == Dd60FastRate + 0)
    {
        m_startBlock = true;
    }
    
    m_pixmap = new PixelData (*m_screenmap);

#if DEBUG || TIMING
    gettimeofday (&t, NULL);
    datacount = dtFetData (m_fet);
#endif

    for (;;)
    {
        if (!dtConnected (m_fet))
        {
            break;
        }
        
        if (pendingData != 0)
        {
            data = pendingData;
            pendingData = 0;
        }
        else
        {
            data = dtReado (m_fet);
        }

        if (data < 0)
        {
            if (!dtConnected (m_fet))
            {
                m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);
                dtClose (m_fet, TRUE);
                m_fet = NULL;
            }
                    
            break;
        }

        if (m_startBlock)
        {
            // At start of block, decay the current screen content
            const int pixels = m_pixmap->GetWidth () * m_pixmap->GetHeight ();
            PixelData::Iterator p (*m_pixmap);
            const int bytesperpixel = PixelData::Iterator::PixelFormat::SizePixel;
    
            //printf ("pixels %d, pixel stride %d\n", pixels, bytesperpixel);
    
#if (DECAY == 128)
#ifdef __x86_64__
            u64 *pmap = (u64 *) (p.m_ptr);
            const u64 maxalpha = m_maxalpha | ((u64) m_maxalpha << 32);
            const u64 mask = 0x7f7f7f7f7f7f7f7fULL;
            const int wds = (pixels * bytesperpixel) / 8;
#else
            u32 *pmap = (u32 *)(p.m_ptr);
            const u32 maxalpha = m_maxalpha;
            const u32 mask = 0x7f7f7f7f;
            const int wds = (pixels * bytesperpixel) / 4;
#endif
            if (!m_fastupdate)
            {
                // Slow update, just repaint the display, don't try to
                // simulate decay because it looks weird
                for (i = 0; i < wds; i++)
                {
                    *pmap++ = maxalpha;
                }
            }
            else
            {
                for (i = 0; i < wds; i++)
                {
                    *pmap = ((*pmap >> 1) & mask) | maxalpha;
                    ++pmap;
                }
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
            m_startBlock = false;
        }
    
        if ((data & 0200) == 0)
        {
            TRACE2 ("Char %o (%c)", data, cdcToAscii[data]);
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
                i = dtReado (m_fet);
                if (i < 0)
                {
                    pendingData = data;
                    break;
                }
                i |= ((data & 7) << 8);

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
                mode = Dd60Trace;
                trace_idx = 0;
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
            case Dd60EndBlock:
                TRACEN ("End of block");
                if (m_interval != Dd60FastRate + 0)
                {
                    m_startBlock = true;
                    break;
                }
            }
        }
        if (m_startBlock)
        {
            Refresh ();
            break;
        }
    }
    delete m_pixmap;
    m_pixmap = NULL;
    
    if (m_interval == Dd60FastRate + 0)
    {
        Refresh ();
    }
#if DEBUG || TIMING
    if (datacount)
    {
        gettimeofday (&t2, NULL);
        dt = t2.tv_sec - t.tv_sec;
        dut = t2.tv_usec - t.tv_usec;
        if (dut < 0)
        {
            dut += 1000000;
            dt--;
        }
        printf ("%d bytes from cyber in %d.%06d seconds\n", datacount, dt, dut);
    }
#endif
}
    
void Dd60Frame::OnClose (wxCloseEvent &)
{
    if (dtActive (m_fet))
    {
        dtClose (m_fet, true);
        m_fet = NULL;
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
                                  L"BMP files (*.bmp)|*.bmp|"
                                  L"PNM files (*.pnm)|*.pnm|"
                                  L"TIF files (*.tif)|*.tif|"
                                  L"XPM files (*.xpm)|*.xpm|"
                                  L"All files (*.*)|*.*"),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    
    if (fd.ShowModal () != wxID_OK)
    {
        return;
    }
    filename = fd.GetPath ();
    
    wxImage screenImage = m_screenmap->ConvertToImage ();
    
    wxFileName fn (filename);
    
    dd60App->m_defDir = fn.GetPath ();
    ext = fn.GetExt ();
    if (ext == wxT (""))
    {
        switch (fd.GetFilterIndex ())
        {
        case 0: fn.SetExt (wxT ("png")); break;
        case 1: fn.SetExt (wxT ("bmp")); break;
        case 2: fn.SetExt (wxT ("pnm")); break;
        case 3: fn.SetExt (wxT ("tif")); break;
        case 4: fn.SetExt (wxT ("xpm")); break;
        }
        filename = fn.GetFullPath ();
        ext = fn.GetExt ();
    }
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
    dc.SetBrush (m_foregroundBrush);
    dc.SetPen (m_foregroundPen);
    dc.SetTextBackground (*wxBLACK);
    dc.SetTextForeground (dd60App->m_fgColor);    
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

void Dd60Frame::dd60LoadCharSize (int size, int tsize, u8 *vec)
{
    // Adjust for Retina factor
    size *= m_pscale;
    
    int i, ch, bx, by, ix, iy, pg;
    double x, y, scalex, scaley, r, b, dx, dy;
    unsigned char *pix;
    Chargen cg;
    bool on;
    const int margin = (tsize - size) / 2;
    // delta from one scanline to the next (Y to Y-1):
    const int nextline = tsize * 4;
    // offset to character origin (accounting for margins) -- 
    // note that this is a window system style origin, i.e., upper left.
    const int orgoffset = nextline * margin + margin * 4;
    // delta from one char to the next:
    const int nextchar = tsize * tsize * 4;
    // sizeX and sizeY are percentages of the nominal size
    const double width = size * dd60App->m_sizeX / 100.0;
    const double height = size * dd60App->m_sizeY / 100.0;
    const int rv = dd60App->m_fgColor.Red ();
    const int gv = dd60App->m_fgColor.Green ();
    const int bv = dd60App->m_fgColor.Blue ();
    const double intensity = normInt (m_intens, height);
    const double sigma = m_statusBar->m_panel->beamsize ();
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
                        ix = (int) round (x * scalex + bx);
                        iy = (int) round (y * scaley + by + height);
                        if (ix < -margin || iy < -margin ||
                            ix >= size + margin || iy >= size + margin)
                        {
                            continue;
                        }
                        dx = ix - (x * scalex);
                        dy = iy - (y * scaley + height);
                        r = sqrt (dx * dx + dy * dy);
                        if (r > 3.0 * sigma)
                            continue;
                        b = bell (r, sigma);
                        pix = origin + ix * 4 + (iy * nextline);
                        pg = pix[m_red];    // current red pixel value
                        pg += int (b * intensity * rv / 255);
                        if (pg > 255)
                            pg = 255;
                        pix[m_red] = pg;
                        pg = pix[m_green];    // current green pixel value
                        pg += int (b * intensity * gv / 255);
                        if (pg > 255)
                            pg = 255;
                        pix[m_green] = pg;
                        pg = pix[m_blue];    // current blue pixel value
                        pg += int (b * intensity * bv / 255);
                        if (pg > 255)
                            pg = 255;
                        pix[m_blue] = pg;
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

void Dd60Frame::UpdateSettings (void)
{
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
    int size = 0, margin, firstx, firsty, inc = 0, qwds = 0;
    u8 *data = 0;
    int i, j, k = 0;
#if VECSIZE
    bytevec *pmap, *pdata;
#endif

    if (d > 057)
    {
        printf ("procDd60Char: unexpected char %o\n", d);
        return;
    }
    
    switch (mode)
    {
    case Dd60Trace:
        if (trace_idx >= sizeof (trace_txt) - 1)
        {
            return;
        }
        if (d == 0)
        {
            d = 055;
        }
        trace_txt[trace_idx++] = cdcToAscii[d];
        trace_txt[trace_idx] = '\0';
        return;
    case Dd60CharSmall:
    case Dd60Dot:
        inc = 8;
        size = CHAR8SIZE;
        data = m_char8 + (d * 4 * CHAR8SIZE * CHAR8SIZE);
        break;
    case Dd60CharMedium:
        inc = 16;
        size = CHAR16SIZE;
        data = m_char16 + (d * 4 * CHAR16SIZE * CHAR16SIZE);
        break;
    case Dd60CharLarge:
        inc = 32;
        size = CHAR32SIZE;
        data = m_char32 + (d * 4 * CHAR32SIZE * CHAR32SIZE);
        break;
    }
    // Margin is in screen units (not pixels)
    margin = (size / m_pscale - inc) / 2;
    qwds = size / 4;
    
    if (d != 0 && d != 055)
    {
        PixelData::Iterator p (*m_pixmap);
    
        firstx = xadjust (currentX - margin);
        firsty = yadjust (currentY - (size / m_pscale - margin));
    
        for (i = 0; i < size; i++)
        {
            // Position at the start of the scanline
            p.MoveTo (*m_pixmap, firstx, firsty + i);
#if VECSIZE
            // SSE2/AVX2 operations require 16 byte alignment.
            // The char data array is aligned, but the pixmap data
            // might not be, depending on the X coordinate low bits.
            // Also check that the character row is fully on-screen,
            // if not we'll paint it pixel by pixel.
            if (avx2 &&
                (((uintptr_t) (p.m_ptr)) & 0x0f) == 0 &&
                firstx >= 0 && firstx + size <= m_xsize)
            {
                pmap = (bytevec *) (p.m_ptr);
                pdata = (bytevec *) data;
                for (j = 0; j < qwds / 2; j++)
                {
                    // Make sure the pixel position is within the
                    // screen image bitmap
                    if (firstx + j * 2 >= 0 && firstx + j * 2 < m_xsize &&
                        firsty + i >= 0 && firsty + i < m_ysize)
                    {
#if (VECSIZE==32)
                        *pmap = __builtin_ia32_paddusb256 (*pmap, *pdata);
#else
                        *pmap = __builtin_ia32_paddusb128 (*pmap, *pdata);
#endif
                    }
                    ++pmap;
                    ++pdata;
                }
                data += qwds * 16;
            }
            else
#endif
            for (j = 0; j < size; j++)
            {
                // Make sure the pixel position is within the
                // screen image bitmap
                if (firstx + j >= 0 && firstx + j < m_xsize &&
                    firsty + i >= 0 && firsty + i < m_ysize)
                {
                    u8 &rp = p.Red ();
                    u8 &gp = p.Green ();
                    u8 &bp = p.Blue ();
            
                    k = rp + data[m_red];
                    if (k > 255)
                    {
                        k = 255;
                    }
                    rp = k;
                    k = gp + data[m_green];
                    if (k > 255)
                    {
                        k = 255;
                    }
                    gp = k;
                    k = bp + data[m_blue];
                    if (k > 255)
                    {
                        k = 255;
                    }
                    bp = k;
                }
                ++p;
                data += 4;
            }
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
            //traceF = fopen (traceFn, "w");
            traceF = stderr;
            
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
    if (dtConnected (m_fet))
    {
        send (m_fet->connFd, &data, 1, 0);
    }
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
    : wxDialog (parent, id, title)
{
    wxBitmap fgBitmap (20, 12);
    wxBoxSizer *bs, *ds, *ods;
    wxStaticBoxSizer *sbs;
    wxFlexGridSizer *fgs;
    
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

    wxIntegerValidator<long> portval;
    portval.SetRange (1, 65535);
    m_portText = new wxTextCtrl (this, wxID_ANY, m_port,
                                 wxDefaultPosition, wxDefaultSize,
                                 0, portval);
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Default Port")));
    fgs->Add (m_portText);
    sbs->Add (fgs, 0, wxALL, 8);
    ds->Add (sbs, 0,  wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 10);

    // Second group: Display settings
    sbs = new wxStaticBoxSizer (new wxStaticBox (this, wxID_ANY,
                                                 _("Display settings")),
                                wxVERTICAL);
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
        
        // 20 255 80 is RGB for DD60 green
        m_fgColor = wxColour (20, 255, 80);
        m_connect = true;
        m_autoConnect->SetValue (true);
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
}

void Dd60PrefDialog::paintBitmap (wxBitmap &bm, wxColour &color)
{
    wxBrush bitmapBrush (color, wxBRUSHSTYLE_SOLID);
    wxMemoryDC memDC;

    memDC.SelectObject (bm);
    memDC.SetBackground (bitmapBrush);
    memDC.Clear ();
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
    wxIntegerValidator<long> portval;
    wxFloatingPointValidator<double> delayval;
    
    portval.SetRange (1, 65535);
    delayval.SetRange (0, 63.0);
    delayval.SetPrecision (2);

    // Sizer for the whole dialog box content
    ds = new wxBoxSizer (wxVERTICAL);

    // First section: the text controls
    m_port.Printf (wxT ("%ld"), dd60App->m_port);

    m_portText = new wxTextCtrl (this, wxID_ANY, m_port,
                                 wxDefaultPosition, wxDefaultSize,
                                 0, portval);
    wxString defDelay;
    defDelay.Printf ("%.2f", dd60App->m_interval);
    m_delayText = new wxComboBox (this, wxID_ANY, defDelay,
                                  wxDefaultPosition, wxSize (75, -1),
                                  0, NULL, 0, delayval);
    m_delayText->Append (wxT ("0"));
    m_delayText->Append (wxT ("0.06"));
    m_delayText->Append (wxT ("0.1"));
    m_delayText->Append (wxT ("3.0"));
    
    fgs = new wxFlexGridSizer (2, 2, 8, 8);
      
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Port")));
    fgs->Add (m_portText);
    fgs->Add (new wxStaticText (this, wxID_ANY, _("Update interval")));
    fgs->Add (m_delayText);

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
        m_delay = m_delayText->GetValue ();
        EndModal (wxID_OK);
}

// ----------------------------------------------------------------------------
// Dd60Panel
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(Dd60Panel, wxPanel)
    EVT_SCROLL(Dd60Panel::OnScroll)
END_EVENT_TABLE()

Dd60Panel::Dd60Panel (Dd60Frame *parent) :
    wxPanel (parent),
    m_parent (parent)
{
    wxSizerFlags sf;
    wxSize knobSize (50, 50);
    
    sf.Align (wxALIGN_CENTER);
    sf.Border (wxLEFT | wxRIGHT | wxTOP);
    m_sizer = new wxFlexGridSizer (2, 4, 2, 8);
    m_sizeX = new wxKnob (this, wxID_ANY, dd60App->m_sizeX, 75, 125,
                          240, 300, wxDefaultPosition, knobSize);
    m_sizer->Add (m_sizeX, sf);
    m_sizeY = new wxKnob (this, wxID_ANY, dd60App->m_sizeY, 75, 125,
                          240, 300, wxDefaultPosition, knobSize);
    m_sizer->Add (m_sizeY, sf);
    m_focus = new wxKnob (this, wxID_ANY, dd60App->m_focus, 45, 120,
                          240, 300, wxDefaultPosition, knobSize);
    m_sizer->Add (m_focus, sf);
    m_intens = new wxKnob (this, wxID_ANY, parent->m_intens, 125, 225,
                           240, 300, wxDefaultPosition, knobSize);
    m_sizer->Add (m_intens, sf);
    sf.Border (wxLEFT | wxRIGHT | wxBOTTOM);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("X Size")), sf);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("Y Size")), sf);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("Focus")), sf);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("Intensity")), sf);
    SetSizerAndFit (m_sizer);
}

void Dd60Panel::OnScroll (wxScrollEvent &)
{
    dd60App->m_sizeX = m_sizeX->GetValue ();
    dd60App->m_sizeY = m_sizeY->GetValue ();
    dd60App->m_focus = m_focus->GetValue ();
    m_parent->m_intens = m_intens->GetValue ();
    if (m_parent->m_fastupdate)
    {
        dd60App->m_fastintens = m_parent->m_intens;
    }
    else
    {
        dd60App->m_slowintens = m_parent->m_intens;
    }
    dd60App->WritePrefs ();
    
    m_parent->dd60LoadChars ();
}

// ----------------------------------------------------------------------------
// Dd60StatusBar
// ----------------------------------------------------------------------------

Dd60StatusBar::Dd60StatusBar (Dd60Frame *parent)
    : wxBoxSizer (wxHORIZONTAL),
      m_parent (parent),
      m_panel (NULL)
{
    msg1 = new wxStaticText (parent, wxID_ANY, wxT (""), 
                             wxDefaultPosition, 
                             wxSize (parent->m_xsize / (3 * parent->m_pscale), 15));
    msg3 = new wxStaticText (parent, wxID_ANY, wxT (""), 
                             wxDefaultPosition,
                             wxSize (parent->m_xsize / (3 * parent->m_pscale), 15));
}

void Dd60StatusBar::SetPanel (Dd60Panel *panel)
{
    m_panel = panel;
    Add (msg1, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 20);
    Add (panel, 1, wxLEFT | wxRIGHT, 15);
    Add (msg3, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 20);
}

void Dd60StatusBar::SetStatusText (const wxChar *str, int idx)
{
    switch (idx)
    {
    case STATUS_TRC:
        msg1->SetLabel (str);
        break;
    case STATUS_CONN:
        msg3->SetLabel (str);
        break;
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
                       wxSize (parent->m_xsize / parent->m_pscale,
                               parent->m_ysize / parent->m_pscale),
                       wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE),
      m_owner (parent)
{
    wxClientDC dc(this);

    SetVirtualSize (m_owner->m_xsize / m_owner->m_pscale,
                    m_owner->m_ysize / m_owner->m_pscale);
    dc.SetClippingRegion (m_owner->m_displaymargin,
                          m_owner->m_displaymargin, 
                          m_owner->m_xsize / m_owner->m_pscale,
                          m_owner->m_ysize / m_owner->m_pscale);
    SetForegroundColour (dd60App->m_fgColor);    
    SetBackgroundColour (*wxBLACK);
    SetScrollRate (1, 1);
    SetFocus ();
}

void Dd60Canvas::OnDraw(wxDC &dc)
{
    dc.SetUserScale (1. / m_owner->m_pscale, 1. / m_owner->m_pscale);
    dc.DrawBitmap (*m_owner->m_screenmap, 0, 0, false);
    dc.SetFont (m_owner->m_traceFont);
    dc.DrawText (wxString::FromAscii (m_owner->trace_txt), TraceX, TraceY);
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

Dd60Printout::Dd60Printout (Dd60Frame *owner,
                            const wxString &title)
    : wxPrintout (title),
      m_owner (owner)
{}

bool Dd60Printout::OnPrintPage (int page)
{
    wxDC *dc = GetDC ();

    if (dc)
    {
        if (page == 1 || page == 2)
        {
            DrawPage (dc, page);
        }
        return true;
    }

    return false;
}

void Dd60Printout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 2;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool Dd60Printout::HasPage(int pageNum)
{
    return (pageNum == 1 || pageNum == 2);
}

void Dd60Printout::DrawPage (wxDC *dc, int page)
{
    int w, h, startx;
    int graypix;
    // We use m_ysize here because that's the width of a single screen
    double maxX = m_owner->m_ysize;
    double maxY = m_owner->m_ysize;

    // Let's have at least 50 device units margin
    double marginX = 50;
    double marginY = 50;

    // Add the margin to the graphic size
    maxX += (2 * marginX);
    maxY += (2 * marginY);

    // Get the size of the DC in pixels
    dc->GetSize (&w, &h);

    // Calculate a suitable scaling factor
    double pscaleX = (double) (w / maxX);
    double pscaleY = (double) (h / maxY);

    // Use x or y scaling factor, whichever fits on the DC
    double actualScale = wxMin(pscaleX, pscaleY);

    // Calculate the position on the DC for centring the graphic
    double posX = (double) ((w - (m_owner->m_ysize * actualScale)) / 2.0);
    double posY = (double) ((h - (m_owner->m_ysize * actualScale)) / 2.0);

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
            // convert to grayscale, using the max of the three channel
            // luminosities to produce a good crisp black & white image.
            graypix = data[0];
            if (data[1] > graypix)
            {
                graypix = data[1];
            }
            if (data[2] > graypix)
            {
                graypix = data[2];
            }

            // Invert it so text is dark on white background
            graypix = 255 - graypix;
            data[0] = data[1] = data[2] = graypix;

            data += 3;
        }
    }

    wxBitmap printmap (screenImage);

    // Calculate the offset to position the correct portion of the screen image
    if (page == 1)
    {
        startx = 0;
    }
    else
    {
        startx = -w / 2;
    }
    dc->SetClippingRegion (0, 0, w / 2, h);
    dc->DrawBitmap (printmap, startx, 0);
}

/*---------------------------  End Of File  ------------------------------*/
