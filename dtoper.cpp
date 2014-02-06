/////////////////////////////////////////////////////////////////////////////
// Name:        dtoper.cpp
// Purpose:     Operator interface to wxWindows 
// Author:      Paul Koning
// Modified by:
// Created:     01/18/2012
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

#define STATUS_TIP      0
#define STATUS_TRC      1
#define STATUS_CONN     2
#define STATUSPANES     3

// Literal strings for wxConfig key strings.  These are defined
// because they appear in two places, so this way we avoid getting
// the two out of sync.  Note that they should *not* be changed after
// once being defined, since that invalidates people's stored
// preferences.
#define PREF_FOREGROUND "foreground"
#define PREF_PORT       "port"
#define PREF_CONNECT    "autoconnect"

#define OpCmdSize   64
#define OpStatSize  64
#define OpDataSize  257
#define CmdX        0020
#define CmdY        0060
#define StatusX     01040
#define StatusY     0740
#define StatusLines 255
#define StatusWin   20
#define StatusOff   2
#define NetBufSize  4096
#define SendBufSize 256
#define DisplayMargin	20
#if  defined(__WXMAC__)
#define SmallPointSize  12
#define MediumPointSize 18
#else
#define SmallPointSize  10
#define MediumPointSize 16
#endif

// Size of the window.
// This is: a screen high and wide, with marging top and botton
#define XBaseSize       02000
#define YBaseSize       01000
#define XSize           (XBaseSize + 2 * DisplayMargin)
#define YSize           (YBaseSize + 2 * DisplayMargin)
#define AdjustX(x) (DisplayMargin + (x))
#define AdjustY(y) (YBaseSize + DisplayMargin - (y))

#define DtOperVersion   "DtCyber Operator Window"

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define TRACEN(str)                                             \
    if (traceDtoper)                                             \
        {                                                       \
        fprintf (traceF, str "\n");  \
        }

#define TRACE1(str, arg)                                             \
    if (traceDtoper)                                                 \
        {                                                           \
        fprintf (traceF, str "\n", arg); \
        }

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct opCmd
{
    char            *name;               /* command name */
    void            (*handler)(char *cmdParams);
} OpCmd;

typedef struct opMsg
    {
    int     x;
    int     y;
    bool    small;
    bool    bold;
    char    *text;
    } OpMsg;

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
#include <arpa/inet.h>
#endif
#include <stdlib.h>
#include "const.h"
#include "types.h"
#include "proto.h"

#include "operator.h"
#include "ptermversion.h"

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
//#include "dtoper.xpm"
#endif

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

class DtoperApp;
static DtoperApp *dtoperApp;

static FILE *traceF;
static char traceFn[20];

#ifdef DEBUG
static wxLogWindow *logwindow;
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------


class DtoperFrame;

// Dtoper screen printout
class DtoperPrintout: public wxPrintout
{
 public:
    DtoperPrintout (DtoperFrame *owner,
                   const wxString &title = _("DTOPER printout")) 
        : wxPrintout (title),
          m_owner (owner)
    {}
  bool OnPrintPage (int page);
  bool HasPage (int page);
  bool OnBeginDocument (int startPage, int endPage);
  void GetPageInfo (int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
  void DrawPage (wxDC *dc);

private:
    DtoperFrame *m_owner;
};

// Define a new application type, each program should derive a class from wxApp
class DtoperApp : public wxApp
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
    
    void WritePrefs (void);
    
    wxFont      m_smallFont;
    wxFont      m_mediumFont;
    wxFont      m_boldFont;
    
    wxColour    m_fgColor;
    wxConfig    *m_config;

    int         m_port;
    
    // scale is 1 or 2 for full size and double, respectively.
    int         m_scale;
    int         m_sizeX;
    int         m_sizeY;
    int         m_focus;
    int         m_intens;
    bool        m_connect;
    DtoperFrame  *m_firstFrame;
    wxString    m_defDir;
    
private:
    wxLocale    m_locale; // locale we'll be using
    
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// define a scrollable canvas for drawing onto
class DtoperCanvas: public wxScrolledWindow
{
public:
    DtoperCanvas (DtoperFrame *parent);

    void OnKey (wxKeyEvent& event);
    void OnPaint (wxPaintEvent &event);

private:
    DtoperFrame *m_owner;

    DECLARE_EVENT_TABLE ()
};

#if defined(__WXMAC__)
#define DTOPER_MDI 1
#else
#define DTOPER_MDI 0
#endif

#if DTOPER_MDI
#define DtoperFrameBase  wxMDIChildFrame

class DtoperMainFrame : public wxMDIParentFrame
{
public:
    DtoperMainFrame (void);
};

static DtoperMainFrame *DtoperFrameParent;
#else
#define DtoperFrameBase wxFrame
#define DtoperFrameParent NULL
#endif

// Define a new frame type: this is going to be our main frame
class DtoperFrame : public DtoperFrameBase
{
    friend void DtoperCanvas::OnPaint (wxPaintEvent &event);
    friend void DtoperPrintout::DrawPage (wxDC *dc);
    
public:
    // ctor(s)
    DtoperFrame(int port, const wxString& title);
    ~DtoperFrame ();

    // Callback handlers
    static void connCallback (NetFet *np, int portNum, void *arg);
    static void dataCallback (NetFet *np, int bytes, void *arg);

    // event handlers (these functions should _not_ be virtual)
    void OnClose (wxCloseEvent& event);
    void OnIdle (wxIdleEvent& event);
    void OnKey (wxKeyEvent& event);
    void OnQuit (wxCommandEvent& event);
    void OnCopyScreen (wxCommandEvent &event);
    void OnPrint (wxCommandEvent& event);
    void OnPrintPreview (wxCommandEvent& event);
    void OnPageSetup (wxCommandEvent& event);
    void OnActivate (wxActivateEvent &event);
    
    void PrepareDC(wxDC& dc);
    void dtoperSendKey(int key);
    void dtoperSetTrace (bool fileaction);

    void paintCanvas (wxDC &dc);
    void opSendString (wxDC &dc, OpMsg *m);

    bool        traceDtoper;
    DtoperFrame   *m_nextFrame;
    DtoperFrame   *m_prevFrame;
    bool        truekb;

private:
    wxStatusBar *m_statusBar;
    wxPen       m_foregroundPen;
    wxBrush     m_foregroundBrush;
    DtoperCanvas  *m_canvas;
    NetPortSet  m_portset;
    NetFet      *m_fet;
    int         m_port;

    char cmdBuf[OpCmdSize + 1];
    int cmdLen;
    char userKey;
    char nextKey;
    char dataBuf[OpDataSize + 1];

    char **syntax;
    int syntaxCnt;
    u8 *syntaxFlags;
    
    OpMsg *messages;
    int msgCount;

    char *status[StatusLines];
    int statusTop;
    int statusMax;
    int statusFlags;

    OpMsg cmdEcho;
    OpMsg errmsg;
    char errmsgBuf[OpCmdSize + 1];

    bool initDone;

    int opScanCmd (void);
    void opRequest(void);
    void opSetMsg (const char *p);
    void opScroll (int top);
    void opConnCallback (NetFet *np, int stat, void *arg);

    void dtoperSetName (wxString &winName);
    void dtoperSetStatus (wxString &str);

    void dtoperShowTrace (bool enable);
    
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// define the preferences dialog
class DtoperPrefDialog : public wxDialog
{
public:
    DtoperPrefDialog (DtoperFrame *parent, wxWindowID id, const wxString &title);
    
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
class DtoperConnDialog : public wxDialog
{
public:
    DtoperConnDialog (wxWindowID id, const wxString &title);
    
    void OnOK (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    wxTextCtrl      *m_portText;
    
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
    Dtoper_CopyScreen = 1,
    Dtoper_ConnectAgain,

    // timers
    Dtoper_Timer,        // display pacing

    // Menu items with standard ID values
    Dtoper_Print = wxID_PRINT,
    Dtoper_Page_Setup = wxID_PRINT_SETUP,
    Dtoper_Preview = wxID_PREVIEW,
    Dtoper_Connect = wxID_NEW,
    Dtoper_Quit = wxID_EXIT,
    Dtoper_Close = wxID_CLOSE,
    Dtoper_Pref = wxID_PREFERENCES,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Dtoper_About = wxID_ABOUT
};


// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(DtoperFrame, wxFrame)
    EVT_CLOSE(DtoperFrame::OnClose)
    EVT_IDLE(DtoperFrame::OnIdle)
    EVT_ACTIVATE(DtoperFrame::OnActivate)
    EVT_MENU(Dtoper_Close, DtoperFrame::OnQuit)
    EVT_MENU(Dtoper_CopyScreen, DtoperFrame::OnCopyScreen)
    EVT_MENU(Dtoper_Print, DtoperFrame::OnPrint)
    EVT_MENU(Dtoper_Preview, DtoperFrame::OnPrintPreview)
    EVT_MENU(Dtoper_Page_Setup, DtoperFrame::OnPageSetup)
    END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(DtoperApp, wxApp)
    EVT_MENU(Dtoper_Connect, DtoperApp::OnConnect)
    EVT_MENU(Dtoper_ConnectAgain, DtoperApp::OnConnect)
    EVT_MENU(Dtoper_Pref,    DtoperApp::OnPref)
    EVT_MENU(Dtoper_Quit,    DtoperApp::OnQuit)
    EVT_MENU(Dtoper_About, DtoperApp::OnAbout)

    END_EVENT_TABLE ()

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp () which will return the reference of the right type (i.e. DtoperApp and
// not wxApp)
IMPLEMENT_APP(DtoperApp)

// ============================================================================
// implementation
// ============================================================================

void DtoperFrame::connCallback (NetFet *fet, int, void *arg)
{
    DtoperFrame *frame = (DtoperFrame *) arg;

    if (!dtActive (fet))
    {
        frame->cmdBuf[0] = '\0';
        frame->cmdLen = 0;
        frame->cmdEcho.bold = FALSE;
        frame->opSetMsg ("$Disconnected");
    }

    wxWakeUpIdle ();
}

void DtoperFrame::dataCallback (NetFet *, int, void *arg)
{
    wxWakeUpIdle ();
}


// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool DtoperApp::OnInit (void)
{
    int r, g, b;
    wxString rgb;
    
    dtoperApp = this;
    m_firstFrame = NULL;
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
    
    sprintf (traceFn, "dtoper_%d.trc", getpid ());

    m_locale.Init(wxLANGUAGE_DEFAULT);
    m_locale.AddCatalog(wxT("dtoper"));

#ifdef DEBUG
    logwindow = new wxLogWindow (NULL, "dtoper log", true, false);
#endif

    if (argc > 3)
    {
        printf ("usage: dtoper [ portnum ]\n");
        exit (1);
    }

    m_config = new wxConfig (wxT ("Dtoper"));

    if (argc > 1)
    {
        m_port = atoi (wxString (argv[1]).mb_str ());
    }
    else
    {
        m_port = m_config->Read (wxT (PREF_PORT), DefOpPort);
    }

    // 0 255 0 is RGB for DTOPER green
    m_config->Read (wxT (PREF_FOREGROUND), &rgb, wxT ("0 255 0"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_scale = 1;
    m_fgColor = wxColour (r, g, b);
    m_connect = (m_config->Read (wxT (PREF_CONNECT), 1) != 0);

#if DTOPER_MDI
    // On Mac, the style rule is that the application keeps running even
    // if all its windows are closed.
//    SetExitOnFrameDelete(false);
    DtoperFrameParent = new DtoperMainFrame ();
    DtoperFrameParent->Show (true);
#endif

    // Add some handlers so we can save the screen in various formats
    // Note that the BMP handler is always loaded, don't do it again.
    wxImage::AddHandler (new wxPNGHandler);
    wxImage::AddHandler (new wxPNMHandler);
    wxImage::AddHandler (new wxTIFFHandler);
    wxImage::AddHandler (new wxXPMHandler);

    // Create the fonts we need
    m_smallFont = wxFont (SmallPointSize, wxFONTFAMILY_MODERN,
                          wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    if (!m_smallFont.IsOk ())
    {
        fprintf (stderr, "Failed to allocate font\n");
        exit (1);
    }
    m_mediumFont = m_smallFont;
    m_mediumFont.SetPointSize (MediumPointSize);
    m_boldFont = m_mediumFont;
    m_boldFont.SetWeight (wxFONTWEIGHT_BOLD);
    
    // create the main application window
    // If arguments are present, always connect without asking
    if (!DoConnect (!(m_connect || argc > 1)))
    {
        return false;
    }
    
    // success: wxApp::OnRun () will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

int DtoperApp::OnExit (void)
{
    delete g_printData;
    delete g_pageSetupData;
#ifdef DEBUG
    delete logwindow;
#endif

    return 0;
}

void DtoperApp::OnConnect (wxCommandEvent &event)
{
    DoConnect (event.GetId () == Dtoper_Connect);
}

bool DtoperApp::DoConnect (bool ask)
{
    DtoperFrame *frame;

    if (ask)
    {
        DtoperConnDialog dlg (wxID_ANY, _("Connect to DtCyber console"));
    
        dlg.CenterOnScreen ();
        
        if (dlg.ShowModal () == wxID_OK)
        {
            if (dlg.m_port.IsEmpty ())
            {
                m_port = DefOpPort;
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
    frame = new DtoperFrame(m_port, wxT("Dtoper"));

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

void DtoperApp::OnAbout(wxCommandEvent&)
{
    wxString msg;

    msg.Printf (_T("DtCyber console (DTOPER) emulator %s.\n%s"),
                wxT ("V2.0.0"
                     "\n  built with wxWidgets V" WXVERSION
                     "\n  build date " PTERMBUILDDATE ),
                _("Copyright \xA9 2004-2014 by Paul Koning."));
    
    wxMessageBox(msg, _("About Dtoper"), wxOK | wxICON_INFORMATION, NULL);
}

void DtoperApp::OnPref (wxCommandEvent&)
{
    DtoperPrefDialog dlg (NULL, wxID_ANY, _("Dtoper Preferences"));
    
    if (dlg.ShowModal () == wxID_OK)
    {
        m_fgColor = dlg.m_fgColor;

        m_port = atoi (wxString (dlg.m_port).mb_str ());
        m_connect = dlg.m_connect;
        
        WritePrefs ();
    }
}

void DtoperApp::WritePrefs (void)
{
    wxString rgb;

    rgb.Printf (wxT ("%d %d %d"), 
                m_fgColor.Red (), m_fgColor.Green (),
                m_fgColor.Blue ());

    m_config->Write (wxT (PREF_FOREGROUND), rgb);
    m_config->Write (wxT (PREF_PORT), m_port);
    m_config->Write (wxT (PREF_CONNECT), (m_connect) ? 1 : 0);
    m_config->Flush ();
}

wxColour DtoperApp::SelectColor (wxColour &initcol)
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

void DtoperApp::OnQuit(wxCommandEvent&)
{
    DtoperFrame *frame, *nextframe;

    frame = m_firstFrame;
    while (frame != NULL)
    {
        nextframe = frame->m_nextFrame;
        frame->Close (true);
        frame = nextframe;
    }
#if DTOPER_MDI
    // On the Mac, deleting all the windows doesn't terminate the
    // program, so we make it stop this way.
    ExitMainLoop ();
    emulationActive = false;
#endif
}


#if DTOPER_MDI
// MDI parent frame
DtoperMainFrame::DtoperMainFrame (void)
    : wxMDIParentFrame (NULL, wxID_ANY, wxT ("Dtoper"),
                        wxDefaultPosition, wxDefaultSize, 0)
{
#if wxUSE_MENUS
    // create a menu bar

    wxMenu *menuFile = new wxMenu;
    menuFile->Append (Dtoper_Connect, _("&New Connection...\tCtrl-N"),
                      _("Connect to DtCyber"));
    menuFile->Append (Dtoper_Pref, _("P&references..."),
                      _("Set program configuration"));
    menuFile->AppendSeparator ();
    menuFile->Append (Dtoper_Quit, _("E&xit\tCtrl-Q"), _("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append (menuFile, _("&File"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Dtoper_About, _("&About Dtoper"), _("Show about dialog"));
    
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
DtoperFrame::DtoperFrame(int port, const wxString& title)
  : DtoperFrameBase(DtoperFrameParent, -1, title,
		   wxDefaultPosition,
		   wxDefaultSize),
    traceDtoper (false),
    m_nextFrame (NULL),
    m_prevFrame (NULL),
    truekb (false),
    m_foregroundPen (dtoperApp->m_fgColor, dtoperApp->m_scale, wxSOLID),
    m_foregroundBrush (dtoperApp->m_fgColor, wxSOLID),
    m_canvas (NULL),
    m_port (port),
    cmdLen (0),
    nextKey (0),
    syntax (NULL),
    syntaxCnt (0),
    syntaxFlags (NULL),
    messages (NULL),
    msgCount (0),
    statusTop (StatusOff),
    statusMax (0),
    statusFlags (0),
    initDone (false)
{
    int true_opt = 1;
    int i;
    
    for (i = 0; i < StatusLines; i++)
    {
        status[i] = NULL;
    }
    cmdEcho.x = CmdX;
    cmdEcho.y = CmdY;
    cmdEcho.small = false;
    cmdEcho.bold = false;
    cmdEcho.text = cmdBuf;
    memset (cmdBuf, 0, sizeof (cmdBuf));
    errmsg.x = 0020;
    errmsg.y = 0014;
    errmsg.small = false;
    errmsg.bold = false;
    errmsg.text = NULL;     // filled in later

    // set the frame icon
//    SetIcon(wxICON(dtoper_32));

#if wxUSE_MENUS
    // create a menu bar

    wxMenu *menuFile = new wxMenu;
    menuFile->Append (Dtoper_Connect, _("&New Connection...\tCtrl-N"),
                      _("Connect to DtCyber"));

    // The accelerators actually will be Command-xxx on the Mac
    menuFile->Append (Dtoper_Print, _("&Print...\tCtrl-P"),
                      _("Print screen content"));
    menuFile->Append (Dtoper_Page_Setup, _("P&age Setup..."), _("Printout page setup"));
    menuFile->Append (Dtoper_Preview, _("Print Pre&view"), _("Preview screen print"));
    menuFile->AppendSeparator ();
    menuFile->Append (Dtoper_Pref, _("P&references..."),
                      _("Set program configuration"));
    menuFile->AppendSeparator ();
    menuFile->Append (Dtoper_Close, _("&Close\tCtrl-W"),
                      _("Close this window"));
    menuFile->Append (Dtoper_Quit, _("E&xit\tCtrl-Q"), _("Quit this program"));

    wxMenu *menuEdit = new wxMenu;

    menuEdit->Append (Dtoper_CopyScreen, _("&Copy Text\tCtrl-C"), _("Copy text to clipboard"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar ();
    menuBar->Append (menuFile, _("&File"));
    menuBar->Append (menuEdit, _("&Edit"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Dtoper_About, _("&About Dtoper"),
                     _("Show about dialog"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

    // create a status bar
    m_statusBar = new wxStatusBar (this, wxID_ANY);
    m_statusBar->SetFieldsCount (STATUSPANES);
    m_statusBar->SetStatusText(_(" Connecting..."), STATUS_CONN);
    SetStatusBar (m_statusBar);
    
    SetCursor (*wxHOURGLASS_CURSOR);

    SetClientSize (XSize + 2, YSize + 2);
    
    m_canvas = new DtoperCanvas (this);

    memset (&m_portset, 0, sizeof (m_portset));
    m_portset.maxPorts = 1;
    m_portset.callBack = connCallback;
    m_portset.callArg = this;
    m_portset.dataCallBack = dataCallback;

    dtInitPortset (&m_portset, NetBufSize, SendBufSize);
    m_fet = m_portset.portVec;

    // Open the connection
    if (dtConnect (m_fet, &m_portset, inet_addr ("127.0.0.1"), m_port) < 0)
    {
        wxString msg;
            
        m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);

        msg.Printf (_("Failed to connect to %d"), m_port);
            
        wxMessageDialog alert (this, msg, _("Alert"), wxOK);
            
        alert.ShowModal ();
        Close ();
        return;
    }
    m_statusBar->SetStatusText(_(" Connected"), STATUS_CONN);
    SetCursor (wxNullCursor);
    setsockopt (m_fet->connFd, SOL_SOCKET, SO_KEEPALIVE,
                (char *)&true_opt, sizeof(true_opt));
#ifdef __APPLE__
    setsockopt (m_fet->connFd, SOL_SOCKET, SO_NOSIGPIPE,
                (char *)&true_opt, sizeof(true_opt));
#endif

    Show(true);
}

DtoperFrame::~DtoperFrame ()
{
    if (dtActive (m_fet))
    {
        dtCloseFet (m_fet, true);
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
        dtoperApp->m_firstFrame = m_nextFrame;
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Process a keystroke.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void DtoperFrame::opRequest(void)
{
    int i;
    int ppKeyIn;

    if (nextKey)
    {
        ppKeyIn = nextKey;
        nextKey = 0;
    }
    else
    {
        ppKeyIn = userKey;
        userKey = '\0';
    }
    if (ppKeyIn <= 0) 
    {
        return;
    }
    //printf ("opRequest: %c\n", ppKeyIn);
    if (cmdLen == 0)
    {
        /*
        **  Special actions for first keystroke.
        */
        opSetMsg (NULL);
        if (ppKeyIn == '+')
        {
            /*
            **  Scroll status display down.
            */
            opScroll (statusTop + StatusWin);
            return;
        }
        else if (ppKeyIn == '-')
        {
            /*
            **  Scroll status display down.
            */
            opScroll (statusTop - StatusWin);
            return;
        }
    }
    if (ppKeyIn == '\r')
    {
        ppKeyIn = '\n';
    }
    if (ppKeyIn == '[' || ppKeyIn == ('U' & 037))
    {
        cmdBuf[0] = '\0';
        cmdLen = 0;
        cmdEcho.bold = FALSE;
    }
    else if (ppKeyIn == 0177 || ppKeyIn == '\b')
    {
        if (cmdLen != 0)
        {
            cmdBuf[--cmdLen] = '\0';
            cmdEcho.bold = FALSE;
        }
    }
    else
    {
        cmdBuf[cmdLen++] = ppKeyIn;
        cmdBuf[cmdLen] = '\0';
        i = opScanCmd ();
        if (i < 0)
        {
            cmdBuf[--cmdLen] = '\0';
        }
        else if (ppKeyIn == '\n')
        {
            /*
            **  The command is complete, send it off.
            */
            cmdBuf[--cmdLen] = '\0';
            if (strcmp (cmdBuf, "END.") == 0)
            {
                Close (true);
#if DTOPER_MDI
                if (m_prevFrame == NULL && m_nextFrame == NULL)
                {
                    // If it's the last frame, close the program even on Mac.
                    dtoperApp->ExitMainLoop ();
                    emulationActive = false;
                }
#endif
                return;
            }
            dtSendTlv (m_fet, &m_portset, OpCommand, strlen (cmdBuf), cmdBuf);
            return;
        }
        else 
        {
            if (i == '\n')
            {
                cmdEcho.bold = TRUE;
                i = 0;
            }
            else
            {
                cmdEcho.bold = FALSE;
            }
            nextKey = i;
        }
    }
    Refresh ();
}

/*--------------------------------------------------------------------------
**  Purpose:        Scroll device status window to a given line
**
**  Parameters:     Name        Description.
**                  top         New top line index
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void DtoperFrame::opScroll (int top)
{
    if (top + StatusWin >= statusMax)
    {
        top = statusMax - StatusWin + 1;
    }
    if (top < StatusOff)
    {
        top = StatusOff;
    }
    statusTop = top;
}

/*--------------------------------------------------------------------------
**  Purpose:        Set operator message
**
**  Parameters:     Name        Description.
**                  p           string pointer
**                              If string begins with $, make it bold
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void DtoperFrame::opSetMsg (const char *p)
{
    if (p != NULL && *p == '$')
    {
        p++;
        errmsg.bold = TRUE;
    }
    else
    {
        if (p == NULL)
        {
            p = " ";
        }
        errmsg.bold = FALSE;
    }
    strncpy (errmsgBuf, p, OpCmdSize);
    errmsg.text = errmsgBuf;
    Refresh ();
}

/*--------------------------------------------------------------------------
**  Purpose:        Scan the command table to match against a partial command
**
**  Parameters:     Name        Description.
**                  none
**
**  Returns:        -1 for bad string.
**                  0 for multiple matches, different next character.
**                  c > 0 means next legal character is always c.
**
**------------------------------------------------------------------------*/
int DtoperFrame::opScanCmd (void)
{
    char *p, *b;
    int match = -1;
    int i;
    
    for (i = 0; i < syntaxCnt; i++)
    {
        //printf ("%d: flags %d, status %d, syntax %s", i, syntaxFlags[i], statusFlags, syntax[i]);
        if (syntaxFlags != NULL &&
            (syntaxFlags[i] & statusFlags) != syntaxFlags[i])
        {
            /* Unlock or debug required but not in effect, skip this one. */
            continue;
        }
        
        for (b = cmdBuf, p = syntax[i]; ; b++)
        {
            if (*b == '\0')
            {
                if (match == -1 || match == *p)
                {
                    match = *p;
                    if (match == 'x' || match == '7')
                    {
                        match = 0;
                    }
                }
                else
                {
                    match = 0;
                }
                break;
            }
            else if (*p == '7')     // octal digit match
            {
                if (*b < '0' || *b > '7')
                {
                    break;          // no match on this pattern
                }
                if (b[1] != '\0' &&
                    (b[1] < '0' || b[1] > '7'))
                {
                    p++;        // if next is not digit, advance pattern
                }
            }
            else if (*p == 'x')
            {
                // 'x' matches any character other than the argument
                // separator (comma).
                if (*b == ',')
                {
                    break;
                }
                if (b[1] != '\0' &&
                    b[1] == p[1])
                {
                    p++;        // if next input == next pattern, advance
                }
            }
            else if (*p == '\0')
            {
                break;
            }
            else
            {
                if (isupper (*p))
                {
                    *b = toupper (*b);
                }
                if (*b != *p)
                {
                    break;          // no match on this pattern
                }
                p++;
            }
        }   
    }
    
    return match;
}

// event handlers

void DtoperFrame::OnIdle (wxIdleEvent &event)
{
    int i, j;
    OpMsg *msgp;
    char *p;
    char *cp;
    bool changes = false;
    
    // In every case, let others see this event too.
    event.Skip ();

    for (;;)
    {
        i = dtReadtlv (m_fet, dataBuf, OpDataSize);
        //printf ("data: len %d code %d\n", i, dataBuf[0]);
        if (i < 0)
        {
            if (!dtActive (m_fet))
            {
                m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);
            }
            break;
        }
        changes = true;
        p = dataBuf + 2;
        p[i] = '\0';        /* put in a string terminator */
        switch (dataBuf[0])
        {
        case OpText:
            /*
            **  The fixed text comes across with the data in the
            **  following format:
            **
            **      x coordinate, 2 bytes, little endian.
            **      y coordinate, 2 bytes, little endian.
            **      font size, one byte
            **      bold flag, one byte
            **      text, the remaining bytes (not null terminated)
            */
            messages = (OpMsg *) realloc (messages, (++msgCount) * sizeof (OpMsg));
            msgp = &messages[msgCount - 1];
            j = *(u8 *) (p++);
            j += *(u8 *) (p++) << 8;
            msgp->x = j;
            j = *(u8 *) (p++);
            j += *(u8 *) (p++) << 8;
            msgp->y = j;
            msgp->small = (*p++ <= 010);
            msgp->bold = (*p++ != 0);
            i -= 6;             /* adjust i to string length */
            if (i == 0)
            {
                msgp->text = NULL;
            }
            else
            {
                msgp->text = (char *) malloc (i + 1);
                strcpy (msgp->text, p);
                if (msgCount == 1)
                {
                    wxString sysname = wxString::FromAscii (p);
                    
                    dtoperSetName (sysname);
                }
            }
            break;
        case OpSyntax:
            syntax = (char **) realloc (syntax, (++syntaxCnt) * sizeof (char *));
            cp = syntax[syntaxCnt - 1] = (char *) malloc (i + 1);
            strcpy (cp, p);
            break;
        case OpSyntaxFlags:
            if (syntaxFlags != NULL)
            {
                free (syntaxFlags);
            }
            syntaxFlags = (u8 *) malloc (syntaxCnt * sizeof (*syntaxFlags));
            memcpy (syntaxFlags, p, syntaxCnt * sizeof (*syntaxFlags));
            break;
        case OpStatus:
            /*
            **  Data format is:
            **      line number, one byte
            **      text, the remaining bytes
            */
            j = *(u8 *) (p++);
            if (i > OpStatSize + 1 || i == 0 || j >= StatusLines)
            {
                break;
            }
            i--;
            if (*p == '\0')
            {
                if (status[j] != NULL)
                {
                    free (status[j]);
                    status[j] = NULL;
                }
            }
            else 
            {
                if (status[j] == NULL)
                {
                    status[j] = (char *) malloc(OpStatSize + 1);
                    if (j > statusMax)
                    {
                        statusMax = j;
                    }
                }
                cp = status[j];
                strncpy (cp, p, OpStatSize);
            }
                    
            /*
            **  If this status line isn't currently visible, 
            **  scroll the display to make it so
            **  line is not null.
            */
            if (j >= StatusOff &&
                (j < statusTop || j >= statusTop + StatusWin) &&
                *p != '\0')
            {
                opScroll (j);
            }
            break;
        case OpFlags:
            /*
            **  Data format is:
            **      flags (one byte)
            */
            statusFlags = *(u8 *) p;
            break;
        case OpReply:
            /*
            **  A reply causes the following actions:
            **      Post the response to the message line.
            **      If the response is not an error (not bold)
            **      erase the command.
            **      In any case, mark the command "not complete".
            */
            opSetMsg (p);
            if (!errmsg.bold)
            {
                cmdBuf[0] = '\0';
                cmdLen = 0;
            }
            cmdEcho.bold = FALSE;
            break;
        case OpInitialized:
            initDone = TRUE;
            break;
        }
    }
    if (changes)
    {
        Refresh ();
    }
}
    
void DtoperFrame::OnClose (wxCloseEvent &)
{
    if (dtActive (m_fet))
    {
        dtClose (m_fet, &m_portset, true);
    }

    Destroy ();
}

void DtoperFrame::OnQuit(wxCommandEvent&)
{
    // true is to force the frame to close
    Close (true);
}

void DtoperFrame::OnActivate (wxActivateEvent &event)
{
    if (m_canvas != NULL)
    {
        m_canvas->SetFocus ();
    }
    event.Skip ();        // let others see the event, too
}

void DtoperFrame::OnCopyScreen (wxCommandEvent &)
{
    int i;
    wxString s;
    
    if (wxTheClipboard->Open ())
    {
        if (messages != NULL)
        {
            for (i = 0; i < msgCount; i++)
            {
                if (messages[i].text != NULL)
                {
                    s += wxString::FromAscii (messages[i].text);
                }
                s += wxT ("\n");
            }
        }
    
        for (i = 0; i <= statusMax; i++)
        {
            if (status[i] != NULL)
            {
                s += wxString::FromAscii (status[i]);
                s += wxT ("\n");
            }
        }
        if (cmdEcho.text != NULL)
        {
            s += wxString::FromAscii (cmdEcho.text);
        }
        s += wxT ("\n");
        if (errmsg.text != NULL)
        {
            s += wxString::FromAscii (errmsg.text);
        }
        s += wxT ("\n");
        wxTheClipboard->SetData (new wxTextDataObject (s));
        wxTheClipboard->Close ();
    }
    else
    {
        wxLogError (_("Can't open clipboard."));
    }
}

void DtoperFrame::OnPrint (wxCommandEvent &)
{
    wxPrintDialogData printDialogData (*g_printData);

    printDialogData.EnableSelection (false);
    printDialogData.EnablePageNumbers (false);
    
    wxPrinter printer (& printDialogData);
    DtoperPrintout printout (this);
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

void DtoperFrame::OnPrintPreview (wxCommandEvent &)
{
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData (*g_printData);
    wxPrintPreview *preview = new wxPrintPreview (new DtoperPrintout (this),
                                                  new DtoperPrintout (this),
                                                  &printDialogData);

    printDialogData.EnableSelection (false);
    printDialogData.EnablePageNumbers (false);
    
    if (!preview->Ok())
    {
        delete preview;
        wxMessageBox(_("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _("Previewing"), wxOK);
        return;
    }

    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _("Dtoper Print Preview"), wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show();
}

void DtoperFrame::OnPageSetup(wxCommandEvent &)
{
    (*g_pageSetupData) = *g_printData;

    wxPageSetupDialog pageSetupDialog (this, g_pageSetupData);
    pageSetupDialog.ShowModal ();

    (*g_printData) = pageSetupDialog.GetPageSetupData ().GetPrintData ();
    (*g_pageSetupData) = pageSetupDialog.GetPageSetupData ();
}

void DtoperFrame::PrepareDC(wxDC& dc)
{
    dc.SetAxisOrientation (true, false);
    dc.SetBackground (*wxBLACK_BRUSH);
    dc.SetTextBackground (*wxBLACK);
    dc.SetTextForeground (dtoperApp->m_fgColor);    
    dc.SetBrush (m_foregroundBrush);
    dc.SetPen (m_foregroundPen);
}

void DtoperFrame::dtoperSetName (wxString &winName)
{
    wxString str;
    
    str.Printf (wxT("Dtoper: %s"), winName.c_str ());
    SetTitle (str);
}

void DtoperFrame::dtoperSetStatus (wxString &str)
{
    m_statusBar->SetStatusText(str, STATUS_CONN);
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
void DtoperFrame::dtoperSetTrace (bool fileaction)
{
    if (!traceDtoper)
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
    dtoperShowTrace (traceDtoper);
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
void DtoperFrame::dtoperSendKey(int key)
{
    char data;
    
    data = key;

    if (traceDtoper)
    {
        fprintf (traceF, "key to plato %03o\n", key);
#ifdef DEBUG
        wxLogMessage ("key to plato %03o", key);
#endif
    }
    dtSend (m_fet, &m_portset, &data, 1);
}

void DtoperFrame::dtoperShowTrace (bool enable)
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

void DtoperFrame::paintCanvas (wxDC &dc)
{
    int i, line;
    int x = AdjustX (StatusX);
    int y;

    if (messages != NULL)
    {
        for (i = 0; i < msgCount; i++)
        {
            opSendString (dc, &messages[i]);
        }
    }
    
    // The majority of the text is always in small font
    dc.SetFont (dtoperApp->m_smallFont);
    if (status[0] != NULL)
    {
        dc.DrawText (wxString::FromAscii (status[0]), x, 
                     AdjustY (StatusY + 020));
    }
    if (status[1] != NULL)
    {
        dc.DrawText (wxString::FromAscii (status[1]), x, 
                     AdjustY (StatusY - 020));
    }
    
    line = 0;
    if (statusTop != StatusOff)
    {
        dc.DrawText (wxT ("        (more)"), x, 
                     AdjustY (StatusY - StatusOff * 020));
        line++;
    }
    
    for (i = statusTop; i <= statusMax; i++)
    {
        y = AdjustY (StatusY - 020 * (line + StatusOff));
        if (status[i] != NULL)
        {
            if (line >= StatusWin && i < statusMax)
            {
                dc.DrawText (wxT ("        (more)"), x, y);
                line++;
                break;
            }
            dc.DrawText (wxString::FromAscii (status[i]), x, y);
        }
        line++;
    }
    opSendString (dc, &cmdEcho);
    opSendString (dc, &errmsg);
}

void DtoperFrame::OnKey (wxKeyEvent &event)
{
    unsigned int key;

    key = event.m_keyCode;
    //printf ("ctrl %d shift %d alt %d key %d\n", event.m_controlDown, event.m_shiftDown, event.m_altDown, key);
    if (key == WXK_ALT || key == WXK_SHIFT || key == WXK_CONTROL)
    {
        // We don't take any action on the modifier key keydown events,
        // but we do want to make sure they are seen by the rest of
        // the system.
        event.Skip ();
        return;
    }

    if (event.m_controlDown)
    {
        if (key == ']')         // control-] : trace
        {
            traceDtoper = !traceDtoper;
            dtoperSetTrace (true);
            return;
        }
        event.Skip ();
        return;
    }

    if (key != 0)
    {
        userKey = key;
    }
    if (userKey != 0)
    {
        do
        {
            opRequest ();
        } while (nextKey != 0);
    }
}


/*--------------------------------------------------------------------------
**  Purpose:        Send a string to the console.
**
**  Parameters:     Name        Description.
**                  dc          The wxPaintDC to use for drawing
**                  m           message structure pointer
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void DtoperFrame::opSendString (wxDC &dc, OpMsg *m)
{
    int x = AdjustX (m->x);
    int y = AdjustY (m->y);
    
    if (m->small)
    {
        dc.SetFont (dtoperApp->m_smallFont);
    }
    else if (m->bold)
    {
        dc.SetFont (dtoperApp->m_boldFont);
    }
    else
    {
        dc.SetFont (dtoperApp->m_mediumFont);
    }
    
    if (m->text == NULL)
    {
        dc.DrawText (wxT (" "), x, y);
    }
    else
    {
        dc.DrawText (wxString::FromAscii (m->text), x, y);
    }
}


// ----------------------------------------------------------------------------
// DtoperPrefDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(DtoperPrefDialog, wxDialog)
    EVT_CLOSE(DtoperPrefDialog::OnClose)
    EVT_BUTTON(wxID_ANY,   DtoperPrefDialog::OnButton)
    EVT_CHECKBOX(wxID_ANY, DtoperPrefDialog::OnCheckbox)
    END_EVENT_TABLE()

DtoperPrefDialog::DtoperPrefDialog (DtoperFrame *parent, wxWindowID id, const wxString &title)
    : wxDialog (parent, id, title)
{
    wxBitmap fgBitmap (20, 12);
    wxBoxSizer *bs, *ds, *ods;
    wxStaticBoxSizer *sbs;
    wxFlexGridSizer *fgs;
    
    m_connect = dtoperApp->m_connect;
    m_fgColor = dtoperApp->m_fgColor;
    m_port.Printf (wxT ("%d"), dtoperApp->m_port);

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

void DtoperPrefDialog::OnButton (wxCommandEvent& event)
{
    wxBitmap fgBitmap (20, 12);
    
    if (event.GetEventObject () == m_fgButton)
    {
        m_fgColor = DtoperApp::SelectColor (m_fgColor);
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
        
        // 20 255 80 is RGB for DTOPER green
        m_fgColor = wxColour (20, 255, 80);
        m_connect = true;
        m_autoConnect->SetValue (true);
        m_statusCheck->SetValue (true);
        str.Printf (wxT ("%d"), DefOpPort);
        m_portText->SetValue (str);
    }
    paintBitmap (fgBitmap, m_fgColor);
    m_fgButton->SetBitmapLabel (fgBitmap);
    Refresh ();
}

void DtoperPrefDialog::OnCheckbox (wxCommandEvent& event)
{
    if (event.GetEventObject () == m_autoConnect)
    {
        m_connect = event.IsChecked ();
    }
}

void DtoperPrefDialog::paintBitmap (wxBitmap &bm, wxColour &color)
{
    wxBrush bitmapBrush (color, wxSOLID);
    wxMemoryDC memDC;

    memDC.SelectObject (bm);
    memDC.SetBackground (bitmapBrush);
    memDC.Clear ();
    memDC.SetBackground (wxNullBrush);
    memDC.SelectObject (wxNullBitmap);
}


// ----------------------------------------------------------------------------
// DtoperConnDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(DtoperConnDialog, wxDialog)
    EVT_CLOSE(DtoperConnDialog::OnClose)
    EVT_BUTTON(wxID_OK, DtoperConnDialog::OnOK)
    END_EVENT_TABLE()

DtoperConnDialog::DtoperConnDialog (wxWindowID id, const wxString &title)
    : wxDialog (NULL, id, title)
{
    wxBoxSizer *ds, *ods;
    wxFlexGridSizer *fgs;

    // Sizer for the whole dialog box content
    ds = new wxBoxSizer (wxVERTICAL);

    // First section: the text controls
    m_port.Printf (wxT ("%d"), dtoperApp->m_port);

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

void DtoperConnDialog::OnOK (wxCommandEvent &)
{
        m_port = m_portText->GetLineText (0);
        EndModal (wxID_OK);
}

// ----------------------------------------------------------------------------
// DtoperCanvas
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them.
BEGIN_EVENT_TABLE(DtoperCanvas, wxScrolledWindow)
    EVT_CHAR (DtoperCanvas::OnKey)
    EVT_PAINT (DtoperCanvas::OnPaint)
    END_EVENT_TABLE ()

DtoperCanvas::DtoperCanvas(DtoperFrame *parent)
    : wxScrolledWindow(parent, -1, wxDefaultPosition, 
                       wxSize (XSize, YSize), wxHSCROLL | wxVSCROLL)
{
    wxClientDC dc(this);

    m_owner = parent;
    SetVirtualSize (XSize, YSize);
    dc.SetClippingRegion (DisplayMargin, DisplayMargin, XBaseSize, YBaseSize);
    SetBackgroundColour (*wxBLACK);
    SetForegroundColour (dtoperApp->m_fgColor);
    SetScrollRate (1, 1);
    SetFocus ();
}

void DtoperCanvas::OnPaint (wxPaintEvent &event)
{
    wxPaintDC dc(this);

    m_owner->PrepareDC (dc);
    m_owner->paintCanvas (dc);
}

void DtoperCanvas::OnKey (wxKeyEvent& event)
{
    m_owner->OnKey (event);
}

// ----------------------------------------------------------------------------
// Dtoper printing helper class
// ----------------------------------------------------------------------------

bool DtoperPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument (startPage, endPage))
        return false;

    return true;
}

bool DtoperPrintout::OnPrintPage (int page)
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

void DtoperPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool DtoperPrintout::HasPage(int pageNum)
{
    return (pageNum == 1);
}

void DtoperPrintout::DrawPage (wxDC *dc)
{
    wxMemoryDC screenDC;
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

    // Set the scale and origin
    dc->SetUserScale (actualScale, actualScale);
    dc->SetDeviceOrigin ((long) posX, (long) posY);

    m_owner->paintCanvas (*dc);
}

/*---------------------------  End Of File  ------------------------------*/
