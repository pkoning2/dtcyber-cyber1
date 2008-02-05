/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal wxWidgets sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/image.h>
#include "knob.h"
#include "iir.h"

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "pterm_32.xpm"
#endif

#include "const.h"
#include "types.h"
#include "chargen.h"

// ----------------------------------------------------------------------------
// global vars
// ----------------------------------------------------------------------------

int bw;

#define STEPMHZ 10
#define STEPHZ  (STEPMHZ * 1000000)
#define BWRATIO 100
#define EXTRA   7

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------
// Character waveform generator.  This generates the unfiltered
// waveform, i.e., an idealized waveform made up of straight lines
// (constant slope ramps, or constant voltages).  It implements
// the CC545 character generator patterns from document 60469310.
class Chargen
{
public:
    void SetStepCount (int n);
    void Start (int ch);
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
    double Ramp (int step);

    double m_x, m_y;
    int m_dx, m_dy, m_step, m_stroke;
    bool m_on;
    const u8 *m_chardata;
    int m_stepcount;
};

class MyFrame;
class CtlFrame;

    

// Define a panel for the controls.  The nice way to do this would
// be with custom controls that look like DD60 pot knobs, but that
// will come later.
class CcPanel : public wxPanel
{
public:
    CcPanel (CtlFrame *parent);

    double aspect (void) const
    {
        return m_aspect->GetValue () / 100.0;
    }
    int sizeX (void) const
    { 
        return m_size->GetValue () * m_aspect->GetValue () / 100;
    }
    int sizeY (void) const { return m_size->GetValue (); }
    double beamsize (void) const { return 0.01 * m_focus->GetValue (); }
    int intensity (void) const { return m_intens->GetValue (); }
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
 
    void OnScroll (wxScrollEvent& event);
    void OnCheck (wxCommandEvent& event);

private:
    wxFlexGridSizer  *m_sizer;
    wxKnob      *m_size;
    wxKnob      *m_aspect;
    wxKnob      *m_focus;
    wxKnob      *m_intens;
//    wxKnob      *m_c2_019;
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
    
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};

// Define a window for the content
class CcWindow : public wxScrolledWindow
{
public:
    CcWindow (MyFrame *parent, CtlFrame *controls);

    void OnDraw (wxDC &dc);

private:
    CtlFrame *m_controls;
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title, CtlFrame *controls);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
    CcWindow    *m_canvas;
    CtlFrame     *m_panel;
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};

// Frame for the controls panel
class CtlFrame : public wxFrame
{
public:
    // ctor(s)
    CtlFrame(const wxString& title);

    CcPanel     *m_controls;
    // any class wishing to process wxWidgets events must use this macro
//    DECLARE_EVENT_TABLE()
};

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

    double aspect (void) const { return m_controls->m_controls->aspect (); }
    int sizeX (void) const { return m_controls->m_controls->sizeX (); }
    int sizeY (void) const { return m_controls->m_controls->sizeY (); }
    double beamsize (void) const { return m_controls->m_controls->beamsize (); }
    int intensity (void) const { return m_controls->m_controls->intensity (); }
    double r1_029 (void) const { return m_controls->m_controls->r1_029 (); }
    double c1_029 (void) const { return m_controls->m_controls->c1_029 (); }
    double c1_a2 (void) const { return m_controls->m_controls->c1_a2 (); }
    double c4_a2 (void) const { return m_controls->m_controls->c4_a2 (); }
    int delay (void) const { return m_controls->m_controls->delay (); }
    int red (void) const { return m_controls->m_controls->red (); }
    int green (void) const { return m_controls->m_controls->green (); }
    int blue (void) const { return m_controls->m_controls->blue (); }
    bool get620on (void) const { return m_controls->m_controls->get620on (); }
    bool getc19on (void) const { return m_controls->m_controls->getc19on (); }
    bool get029on (void) const { return m_controls->m_controls->get029on (); }
    bool getv1aon (void) const { return m_controls->m_controls->getv1aon (); }
    bool getv3on (void) const { return m_controls->m_controls->getv3on (); }
    
    void closeControls (void) 
    {
        m_controls->Close (true);
    }
    void updateFrame (void)
    {
        m_frame->Refresh ();
    }
    
    
private:
    MyFrame     *m_frame;
    CtlFrame    *m_controls;
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

#define pi 3.14159265358979323846

// Normal distribution (bell curve) function
static double bell (double x, double sigma)
{
    return (1 / (sigma * sqrt (2. * pi))) * 
        exp (-((x * x) / (2. * sigma * sigma)));
}

// Number of steps per stroke element, as a function of chosen bandwidth
static int stepcount (int bw, int height)
{
    if (bw == 0)
    {
        // No filtering
        return (height + 3) / 4;
    }
    else
    {
        return bw * BWRATIO / STEPMHZ;
    }
}

// Normalize the intensity to account for beam size and character size
// The input intensity value is the value we want to end up with in the
// pixel for a double  speed stroke.
static double normInt (int intens, int size)
{
    double step = size / (4.0 * stepcount (bw, size));
    
    return intens * step;
}


#define defwidth 32
#define defheight 32
#define consoleSize (wxSize (defwidth * 13 / 2, defheight * 11 / 2))


// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    bw = 4;
    if (argc > 1)
    {
        bw = atoi (wxString (argv[1]).mb_str ());
    }
    
    if (bw != 0 && bw * BWRATIO / STEPMHZ < 1)
    {
        printf ("bandwidth too low\n");
        return FALSE;
    }
    else
    {
        printf ("%d simulation steps per stroke step\n",
                bw * BWRATIO / STEPMHZ);
    }
    
    // create the controls
    m_controls = new CtlFrame (_T("Controls"));
    m_controls->Show (true);
    
    // create the main application window
    m_frame = new MyFrame(_T("Minimal wxWidgets App"), m_controls);

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    m_frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString& title, CtlFrame *controls)
       : wxFrame(NULL, wxID_ANY, title)
{
    // set the frame icon
    SetIcon(wxICON(pterm_32));

    SetClientSize (consoleSize);
    
    m_canvas = new CcWindow (this, controls);
    
#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, _T("&About...\tF1"), _T("Show about dialog"));

    fileMenu->Append(Minimal_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(helpMenu, _T("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS
}


// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
    wxGetApp ().closeControls ();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf( _T("This is the About dialog of the minimal sample.\n")
                _T("Welcome to %s"), wxVERSION_STRING);

    wxMessageBox(msg, _T("About Minimal"), wxOK | wxICON_INFORMATION, this);
}

// frame constructor
CtlFrame::CtlFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title)
{
    // set the frame icon
    SetIcon(wxICON(pterm_32));

    m_controls = new CcPanel (this);
}

BEGIN_EVENT_TABLE(CcPanel, wxPanel)
    EVT_SCROLL(CcPanel::OnScroll)
    EVT_CHECKBOX(wxID_ANY, CcPanel::OnCheck)
END_EVENT_TABLE()

CcPanel::CcPanel (CtlFrame *parent) :
    wxPanel (parent)
{
    wxBitmapButton *b;
    int id = 9900;
    
    m_sizer = new wxFlexGridSizer (18, 2, 5, 10);
    m_size = new wxKnob (this, id++, 32, 8, 64);
    m_sizer->Add (m_size, 0,  wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("Size 32")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_aspect = new wxKnob (this, id++, 100, 40, 200);
    m_sizer->Add (m_aspect, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("Aspect ratio 100")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_focus = new wxKnob (this, id++, 100, 0, 250);
    m_sizer->Add (m_focus, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("Focus 100")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_intens = new wxKnob (this, id++, 160, 1, 400);
    m_sizer->Add (m_intens, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("Intensity 160")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_beamdelay = new wxKnob (this, id++, 10, 1, 100);
    m_sizer->Add (m_beamdelay, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("On/off delay 10")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_red = new wxKnob (this, id++, 20, 0, 255);
    m_sizer->Add (m_red, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("Red 20")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_green = new wxKnob (this, id++, 255, 0, 255);
    m_sizer->Add (m_green, 0,  wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("Green 255")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_blue = new wxKnob (this, id++, 80, 0, 255);
    m_sizer->Add (m_blue, 0,  wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("Blue 80")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_620on = new wxCheckBox (this, wxID_ANY, _(""));
    m_620on->SetValue (true);
    m_sizer->Add (m_620on, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("620 highpass on")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_c19on = new wxCheckBox (this, wxID_ANY, _(""));
    m_c19on->SetValue (true);
    m_sizer->Add (m_c19on, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("C19 lowpass on")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_r1_029 = new wxKnob (this, id++, 2000, 180, 5000);
    m_sizer->Add (m_r1_029, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("029 R1 2000")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_c1_029 = new wxKnob (this, id++, 300, 110, 580);
    m_sizer->Add (m_c1_029, 0,  wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("029 C1 300")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_029on = new wxCheckBox (this, wxID_ANY, _(""));
    m_029on->SetValue (true);
    m_sizer->Add (m_029on, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("029 highpass on")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
#if 0
    m_c1_a2 = new wxKnob (this, id++, 2000, 1400, 3055);
    m_sizer->Add (m_c1_a2, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("V1A C1 2000")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8); 
    m_v1aon = new wxCheckBox (this, wxID_ANY, _(""));
    m_sizer->Add (m_v1aon, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("V1A C1 on")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_c4_a2 = new wxKnob (this, id++, 2000, 1400, 3055);
    m_sizer->Add (m_c4_a2, 0,   wxLEFT | wxRIGHT, 8);
    m_sizer->Add (new wxStaticText (this, id++, _("V3 C4 2000")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
    m_v3on = new wxCheckBox (this, wxID_ANY, _(""));
    m_sizer->Add (m_v3on, 0,  wxALL, 8);
    m_sizer->Add (new wxStaticText (this, wxID_ANY, _("V3 C4 on")),
                  0,  wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
#endif
    SetSizerAndFit (m_sizer);
//    m_sizer->RecalcSizes ();
}

void CcPanel::OnScroll (wxScrollEvent &e)
{
    int id;
    wxKnob *ctl;
    wxStaticText *lwin;
    wxString label;
    
    id = e.GetId ();
    ctl = (wxKnob *) FindWindowById (id, this);
    lwin = (wxStaticText *) FindWindowById (id + 1, this);
    label = lwin->GetLabel ();
    label = label.BeforeLast (' ');
    //printf ("id: %d, val: %d\n", id, ctl->GetValue());
    label << _(" ") << ctl->GetValue ();
    lwin->SetLabel (label);
    
    wxGetApp ().updateFrame ();
}

void CcPanel::OnCheck (wxCommandEvent &)
{
    wxGetApp ().updateFrame ();
}


CcWindow::CcWindow (MyFrame *parent, CtlFrame *controls)
    : wxScrolledWindow(parent, -1, wxDefaultPosition, 
                       consoleSize,
                       wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE),
      m_controls (controls)
{
    wxClientDC dc(this);

    SetVirtualSize (consoleSize);
    
    SetBackgroundStyle (wxBG_STYLE_COLOUR);
    SetBackgroundColour (*wxBLACK);
    SetScrollRate (defwidth, defheight);
}

void CcWindow::OnDraw (wxDC &dc)
{
    int ch, i, j, bx, by, ix, iy, cx, cy, pg;
    double x, y, scalex, scaley, r, b;
    unsigned char *pix;
//    Fir xfilt, yfilt;
    const double peak = 0.25;
    Chargen cg;
    bool on;
    const int width = wxGetApp ().sizeX ();
    const int height = wxGetApp ().sizeY ();
    const double intensity = normInt (wxGetApp ().intensity (), height);
    const double sigma = wxGetApp ().beamsize ();
    const double r029 = wxGetApp ().r1_029 ();
    const double c029 = wxGetApp ().c1_029 ();
#if 0
    const double cv1a = wxGetApp ().c1_a2 ();
    const double cv3 = wxGetApp ().c4_a2 ();
#endif
    Delay delay (wxGetApp ().delay ());
    const double red = wxGetApp ().red () / 255.;
    const double green = wxGetApp ().green () / 255.;
    const double blue = wxGetApp ().blue () / 255.;
    const bool on620 = wxGetApp ().get620on ();
    const bool onc19 = wxGetApp ().getc19on ();
    const bool on029 = wxGetApp ().get029on ();
#if 0
    const bool onv1a = wxGetApp ().getv1aon ();
    const bool onv3 = wxGetApp ().getv3on ();
#endif

    int beamr = int (ceil (3 * sigma));
    
//    dc.Clear ();
    dc.SetPen (*wxBLACK_PEN);
    
    i = stepcount (bw, height);
    
    const int rcfreq = STEPHZ * i;
    RClow  x620 (470., 33.e-12, rcfreq), y620 (470., 33.e-12, rcfreq);
    const double peak620 = .3;
    RClow  xc19 (6800., 22.e-12, rcfreq), yc19 (6800., 22.e-12, rcfreq);
    const double peakc19 =.35;
    RClow  x029 (r029, c029, rcfreq), y029 (r029, c029, rcfreq);
    const double peak029 = .1;
#if 0
    RClow  xv1a (470., cv1a, rcfreq), yv1a (470., cv1a, rcfreq);
    const double peakv1a = .2;
    RClow  xv3  (3900., cv3, rcfreq), yv3 (3900., cv3, rcfreq);
    const double peakv3 = .16;
#endif

    cg.SetStepCount (i);
    scalex = (double) width / (i * 8);
    scaley = (double) height / (i * 8);
    
    for (ch = 1; ch < 060; ch++)
    {
        const int xs = int (ceil (width + 2 * beamr + 2 * EXTRA));
        const int ys = int (ceil (height + 2 * beamr + 2 * EXTRA));
        wxImage cimg (xs, ys, true);
        unsigned char *dp = cimg.GetData ();
        
        x620.Reset ();
        y620.Reset ();
        xc19.Reset ();
        yc19.Reset ();
        x029.Reset ();
        y029.Reset ();
#if 0
        xv1a.Reset ();
        yv1a.Reset ();
        xv3.Reset ();
        yv3.Reset ();
#endif

        cx = ch & 7;
        cy = (ch >> 3) + 1;
        cx = defwidth + cx * defwidth * wxGetApp ().aspect ();
        cy = defheight + cy * defheight;
        cg.Start (ch);
        while (!cg.Done ())
        {
            x = cg.X ();
            y = cg.Y ();
            on = cg.On ();
            
            if (bw != 0)
            {
                delay.SetFlag (on);
                // Apply the various filters.  The low pass filter in
                // module C19 we apply directly.  The other filters
                // all are lowpass in a feedback path, so they are applied
                // as a fractional increase in the signal.
//                printf ("%f %f  ", x, y);
                if (on620)
                {
                    x += x620.Step (x * peak620);
                    y += y620.Step (y * peak620);
//                    printf ("%f %f  ", x, y);
                }
                if (onc19)
                {
                    x =  (x * (1 - peakc19)) + xc19.Step (x * peakc19);
                    y =  (y * (1 - peakc19)) + yc19.Step (y * peakc19);
//                    printf ("%f %f  ", x, y);
                }
                if (on029)
                {
                    x += x029.Step (x * peak029);
                    y += y029.Step (y * peak029);
                    x /= (1 + peak029);
                    y /= (1 + peak029);
//                    printf ("%f %f  ", x, y);
                }
#if 0
                if (onv1a)
                {
                    x += xv1a.Step (x * peakv1a);
                    y += yv1a.Step (y * peakv1a);
                    x /= (1 + peakv1a);
                    y /= (1 + peakv1a);
//                    printf ("%f %f  ", x, y);
                }
                if (onv3)
                {
                    x += xv3.Step (x * peakv3);
                    y += yv3.Step (y * peakv3);
                    x /= (1 + peakv3);
                    y /= (1 + peakv3);
                }
#endif
//                printf ("%f %f\n", x, y);
                
                on = delay.Flag ();
            }
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
                        b = bell (r, sigma);
                        
                        ix = (int) round (x * scalex + bx + beamr + EXTRA);
                        iy = (int) round (y * scaley + by + beamr + height + EXTRA);
                        if (ix >= xs || iy >= ys || ix < 0 || iy < 0)
                        {
                            printf ("ix %d xs %d iy %d ys %d\n",
                                    ix, xs, iy, ys);
                            continue;
                        }
                        
                        pix = dp + 3 * (iy * xs + ix);
                        pg = pix[0];    // red value
                        pg += int (b * intensity * red);
                        if (pg > 255)
                            pg = 255;
                        pix[0] = pg;
                        pg = pix[1];    // green value
                        pg += int (b * intensity * green);
                        if (pg > 255)
                            pg = 255;
                        pix[1] = pg;
                        pg = pix[2];    // blue value
                        pg += int (b * intensity * blue);
                        if (pg > 255)
                            pg = 255;
                        pix[2] = pg;
                    }
                }
                
//                if (ch == 0) printf ("%d %6.2f %6.2f\n", ch, x, y);
            }
            cg.Step ();
        }
        wxBitmap bm (cimg, -1);
        wxMemoryDC mdc;
        
        mdc.SelectObject (bm);
//        dc.DrawBitmap (bm, cx, cy);
        dc.Blit (cx, cy, xs,ys, &mdc, 0, 0, wxOR);
        mdc.SelectObject (wxNullBitmap);
    }
}

void Chargen::SetStepCount (int n)
{
    m_stepcount = n;
}

void Chargen::Start (int ch)
{
    m_x = m_y = 0;
    m_on = FALSE;
    m_dx = 1;
    m_dy = -1;
    m_chardata = chargen[ch];
    m_step = m_stroke = 0;
}

double Chargen::Ramp (int step)
{
#if 1
    // Base case: linear ramp
    return 1.0;
#elif 1
    // Experiment: exponential decay to 2RC
    return m_stepcount * (exp (-2.0 * step / m_stepcount) - exp (-2.0 * (step + 1) / m_stepcount));
#elif 1
    // Experiment: exponential decay to 5RC
    return m_stepcount * (exp (-5.0 * step / m_stepcount) - exp (-5.0 * (step + 1) / m_stepcount));
#elif 1
    // Experiment: drive with a step function (not a linear ramp)
    if (step == 0)
        return m_stepcount;
    else
        return 0;
#endif
}

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
        m_x += m_dx * 2 * Ramp (m_step);
        break;
    case 2:
        m_x += m_dx * Ramp (m_step);
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
        m_y += m_dy * 2 * Ramp (m_step);
        break;
    case 2:
        m_y += m_dy * Ramp (m_step);
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
