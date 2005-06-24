/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal wxWidgets sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id$
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

int height, width;
int bw;
double sigma =  1.0;
int intensity = 150;

#define STEPMHZ 10
#define BWRATIO 100

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

class Fir 
{
public:
    Fir(void);
    
    void Reset (void);
    void SetFlag (bool flag);
    bool Flag (void) const
    {
        return m_flags[0];
    }
    double Step (double in);
    
private:
    enum { taps = 15, delay = (taps + 1) / 2 };
    double m_data[taps];
    bool m_flags[delay];
    const static double coeff[taps];
};

class Chargen
{
public:
    void SetStepCount (int n);
    void Start (int ch);
    void Step (void);
    int X (void) const 
    {
        return m_x;
    }
    int Y (void) const
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
    int m_x, m_y;
    int m_dx, m_dy, m_step, m_stroke;
    bool m_on;
    const u8 *m_chardata;
    int m_stepcount;
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
};

class MyFrame;

// Define a window for the content
class CcWindow : public wxScrolledWindow
{
public:
    CcWindow (MyFrame *parent);

    void OnDraw (wxDC &dc);
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
    CcWindow *m_canvas;

    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
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
static double bell (double x, double sigma)
{
    return (1 / (sigma * sqrt (2. * pi))) * 
        exp (-((x * x) / (2. * sigma * sigma)));
}

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // create the main application window
    MyFrame *frame = new MyFrame(_T("Minimal wxWidgets App"));

    bw = 1;
    height = 32;
    
    if (argc > 3)
    {
        bw = atoi (wxString (argv[2]).mb_str ());
    }
    if (argc > 1)
    {
        height = atoi (wxString (argv[1]).mb_str ());
    }
    if (argc > 2)
    {
        width = atoi (wxString (argv[2]).mb_str ());
    }
    else
    {
        width = height;
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
    
    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title)
{
    // set the frame icon
    SetIcon(wxICON(pterm_32));

    m_canvas = new CcWindow (this);
    
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

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(_T("Welcome to wxWidgets!"));
#endif // wxUSE_STATUSBAR
}


// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf( _T("This is the About dialog of the minimal sample.\n")
                _T("Welcome to %s"), wxVERSION_STRING);

    wxMessageBox(msg, _T("About Minimal"), wxOK | wxICON_INFORMATION, this);
}

CcWindow::CcWindow (MyFrame *parent)
    : wxScrolledWindow(parent, -1, wxDefaultPosition, 
                       wxSize (512, 512),
                       wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxClientDC dc(this);

    SetVirtualSize (width * 13 / 2, height * 11 / 2);
    
    SetBackgroundStyle (wxBG_STYLE_COLOUR);
    SetBackgroundColour (*wxBLACK);
    SetScrollRate (width, height);
}

void CcWindow::OnDraw (wxDC &dc)
{
    int ch, i, j, bx, by, ix, iy, cx, cy, pg;
    double x, y, scalex, scaley, r, b;
    unsigned char *pix;
    Fir xfilt, yfilt;
    Chargen cg;
    bool on;
    int beamr = int (ceil (3 * sigma));
    
//    dc.Clear ();
    dc.SetPen (*wxBLACK_PEN);
    
    if (bw == 0)
    {
        // No filtering
        i = (height + 3) / 4;
    }
    else
    {
        // Filtering; FIR normalized bandwidth = 1/BWRATIO and stroke
        // frequency is STEPMHZ.
        i = bw * BWRATIO / STEPMHZ;
    }
    cg.SetStepCount (i);
    scalex = (double) width / (i * 8);
    scaley = (double) height / (i * 8);
    
    for (ch = 0; ch < 060; ch++)
    {
        const int xs = int (ceil (width + 2 * beamr + 1));
        const int ys = int (ceil (height + 2 * beamr + 1));
        wxImage cimg (xs, ys, true);
        unsigned char *dp = cimg.GetData ();
        
        cx = ch & 7;
        cy = (ch >> 3) + 1;
        cx = (width + cx * 3 * width) / 2;
        cy = (height + cy * 3 * height) / 2;
        xfilt.Reset ();
        yfilt.Reset ();
        cg.Start (ch);
        while (!cg.Done ())
        {
            x = cg.X ();
            y = cg.Y ();
            on = cg.On ();
            
            if (bw != 0)
            {
                xfilt.SetFlag (on);
                x = xfilt.Step (x);
                y = yfilt.Step (y);
                on = xfilt.Flag ();
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
                        
                        ix = (int) round (x * scalex + bx + beamr);
                        iy = (int) round (y * scaley + by + beamr + height);
                        if (ix >= xs || iy >= ys)
                        {
                            printf ("ix %d xs %d iy %d ys %d\n",
                                    ix, xs, iy, ys);
                            continue;
                        }
                        
                        pix = dp + 3 * (iy * xs + ix);
                        pg = pix[1];    // green value
                        pg += int (b * intensity);
                        if (pg > 255)
                            pg = 255;
                        pix[1] = pg;
                    }
                }
                
//                if (ch == 0) printf ("%d %6.2f %6.2f\n", ch, x, y);
            }
            cg.Step ();
        }
        wxBitmap bm (cimg, -1);
        dc.DrawBitmap (bm, cx, cy);
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
        m_x += m_dx * 2;
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
        m_y += m_dy * 2;
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

// FIR

const double Fir::coeff[] = 
{
//FIR - Bandpass Filter                        
//         Frequency  Value       Weight    Lower Limit Upper Limit
//Band  1:  0.00000   1.000       1.000        ----        ----    
//          0.01000   1.000       1.000        ----        ----    
//          0.05000   0.000       1.000        ----        ----    
//          0.50000   0.000       1.000        ----        ----    
   1.201039E-01,   3.675114E-02,   4.099866E-02,   4.508227E-02,   4.827919E-02,
   5.063049E-02,   5.207417E-02,   5.255540E-02,   5.207417E-02,   5.063049E-02,
   4.827919E-02,   4.508227E-02,   4.099866E-02,   3.675114E-02,   1.201039E-01,
};

Fir::Fir (void)
{
    Reset ();
}

void Fir::Reset (void)
{
    for (int i = 0; i < taps; i++)
    {
        m_data[i] = 0.0;
    }
}

double Fir::Step (double in)
{
    double out;
    int i;
    
    memmove (&m_data[0], &m_data[1], sizeof (m_data) - sizeof (m_data[0]));
    m_data[taps - 1] = in;
    out = 0.0;
    for (i = 0; i < taps; i++)
    {
        out += m_data[i] * coeff[i];
    }
    return out;
}

void Fir::SetFlag (bool flag)
{
    memmove (&m_flags[0], &m_flags[1], sizeof (m_flags) - sizeof (m_flags[0]));
    m_flags[delay - 1] = flag;
}

