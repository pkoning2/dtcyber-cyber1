////////////////////////////////////////////////////////////////////////////
// Name:        PtermApp.cpp
// Purpose:     Implementation of the App
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermApp.h"
#include "PtermFrame.h"
#include "DebugPterm.h"

PtermApp *ptermApp;
bool emulationActive = true;

#if PTERM_MDI
PtermMainFrame *PtermFrameParent;
#endif

// Global print data, to remember settings during the session
wxPrintData *g_printData;

// Global page setup data
wxPageSetupDialogData* g_pageSetupData;

#ifndef DEBUG
Trace traceF;
#endif

#include "PtermConnDialog.h"

// the application icon (under Windows and OS/2 it is in resources)
#if defined (__WXGTK__) || defined (__WXMOTIF__) || defined (__WXMAC__) || defined (__WXMGL__) || defined (__WXX11__)
#include "pterm_32.xpm"
#endif

#if defined (__WXMAC__)
// On the Mac, menus are always displayed
const bool PtermApp::m_showMenuBar = true;
#endif

static const u32 keyboardhelp[] = {
#include "ptermkeys.h"
};

BEGIN_EVENT_TABLE (PtermApp, wxApp)
    EVT_MENU (Pterm_Connect, PtermApp::OnConnect)
    EVT_MENU (Pterm_ConnectAgain, PtermApp::OnConnect)
    EVT_MENU (Pterm_Quit,    PtermApp::OnQuit)
    EVT_MENU (Pterm_HelpKeys, PtermApp::OnHelpKeys)
    EVT_MENU (Pterm_About, PtermApp::OnAbout)
    END_EVENT_TABLE ();

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp () which will return the reference of the right type (i.e. PtermApp
// and not wxApp)
IMPLEMENT_APP (PtermApp);

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool PtermApp::OnInit (void)
{
    int r, g, b, dspi, sz;
    wxString rgb;
    wxString level;
    wxString str;
    const char *s;

#if defined (__WXMAC__)
    fixmenu ();
#endif
    
    m_termType = 0;
    
    ptermApp = this;
    m_firstFrame = m_helpFrame = NULL;
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
#ifdef DEBUG
    debugF.Open ();
#endif
#if defined (__WXMSW__)
    const int pid = GetCurrentProcessId ();
#else
    const int pid = getpid ();
#endif
    
    // File name to use for tracing, if we enable tracing
    sprintf (traceFn, "pterm%d.trc", pid);

    srand (time (NULL)); 
    m_locale.Init (wxLANGUAGE_DEFAULT);
    m_locale.AddCatalog (wxT ("pterm"));

    m_config = new wxConfig (wxT ("Pterm"));

    // Load defaults.  Note that a profile specified in the command
    // line is loaded later, so it will override this.

    //notebook
    m_lastTab = m_config->Read (wxT (PREF_LASTTAB), 0L);
    //tab0
    m_config->Read (wxT (PREF_CURPROFILE), &m_curProfile, wxT (""));
    // " Current " (with space before/after) is the former name of the
    // "default" profile, which is no longer in use.  If we find that
    // saved as the current profile, pretend we have no current
    // profile.
    if (m_curProfile == wxT (" Current "))
        m_curProfile = wxT ("");
    //tab1
    m_config->Read (wxT (PREF_SHELLFIRST), &m_ShellFirst, wxT (""));
    m_connect = (m_config->Read (wxT (PREF_CONNECT), 1) != 0);
    if (argc > 1)
    {
        m_hostName = argv[1];
    }
    else
    {
        m_config->Read (wxT (PREF_HOST), &m_hostName, DEFAULTHOST);
    }
    if (argc > 2)
    {
        argv[2].ToCLong (&m_port);
        if (argc > 3)
        {
            argv[3].ToCLong (&m_termType);
            printf ("terminal type override %ld\n", m_termType);
        }
    }
    else
    {
        m_config->Read (wxT (PREF_PORT), &m_port, DefNiuPort);
    }
    //tab2
    m_showSignon = (m_config->Read (wxT (PREF_SHOWSIGNON), 0L) != 0);
    m_showSysName = (m_config->Read (wxT (PREF_SHOWSYSNAME), 0L) != 0);
    m_showHost = (m_config->Read (wxT (PREF_SHOWHOST), 1) != 0);
    m_showStation = (m_config->Read (wxT (PREF_SHOWSTATION), 1) != 0);
    //tab3
    m_classicSpeed = (m_config->Read (wxT (PREF_1200BAUD), 0L) != 0);
    m_gswEnable = (m_config->Read (wxT (PREF_GSW), 1) != 0);
    m_numpadArrows = (m_config->Read (wxT (PREF_ARROWS), 1) != 0);
    m_ignoreCapLock = (m_config->Read (wxT (PREF_IGNORECAP), 0L) != 0);
    m_platoKb = (m_config->Read (wxT (PREF_PLATOKB), 0L) != 0);
#if defined (__WXMAC__)
    m_useAccel = true;
#else
    m_useAccel = (m_config->Read (wxT (PREF_ACCEL), 0L) != 0);
#endif
    m_beepEnable = (m_config->Read (wxT (PREF_BEEP), 1) != 0);
    m_DisableShiftSpace = (m_config->Read (wxT (PREF_SHIFTSPACE), 0L) != 0);
    m_DisableMouseDrag = (m_config->Read (wxT (PREF_MOUSEDRAG), 0L) != 0);
    //tab4
    m_scale = m_config->ReadDouble (wxT (PREF_SCALE), 1.0);
    if (m_scale <= 0.0)
    {
        m_scale = 1.0;
    }
    m_showStatusBar = (m_config->Read (wxT (PREF_STATUSBAR), 1) != 0);
#if !defined (__WXMAC__)
    m_showMenuBar = (m_config->Read (wxT (PREF_MENUBAR), 1) != 0);
#endif
    m_noColor = (m_config->Read (wxT (PREF_NOCOLOR), 0L) != 0);
    // 255 144 0 is RGB for Plato Orange
    m_config->Read (wxT (PREF_FOREGROUND), &rgb, wxT ("255 144 0"));
    s = rgb.c_str ();
    sscanf (s, "%d %d %d", &r, &g, &b);
    m_fgColor = wxColour (r, g, b);
    m_config->Read (wxT (PREF_BACKGROUND), &rgb, wxT ("0 0 0"));
    s = rgb.c_str ();
    sscanf (s, "%d %d %d", &r, &g, &b);
    m_bgColor = wxColour (r, g, b);
    //tab5
    m_config->Read (wxT (PREF_CHARDELAY), &m_charDelay, PASTE_CHARDELAY);
    m_config->Read (wxT (PREF_LINEDELAY), &m_lineDelay, PASTE_LINEDELAY);
    m_config->Read (wxT (PREF_AUTOLF), &m_autoLF, 0L);
    m_smartPaste = (m_config->Read (wxT (PREF_SMARTPASTE), 0L) != 0);
    m_convDot7 = (m_config->Read (wxT (PREF_CONVDOT7), 0L) != 0);
    m_conv8Sp = (m_config->Read (wxT (PREF_CONV8SP), 0L) != 0);
    m_TutorColor = (m_config->Read (wxT (PREF_TUTORCOLOR), 0L) != 0);
    //tab6
    m_config->Read (wxT (PREF_EMAIL), &m_Email, wxT (""));
    m_config->Read (wxT (PREF_SEARCHURL), &m_SearchURL, DEFAULTSEARCH);
    m_mTutorBoot = (m_config->Read(wxT(PREF_MTUTORBOOT), 0L) != 0);
    m_floppy0 = (m_config->Read(wxT(PREF_FLOPPY0M), 0L) != 0);
    m_floppy1 = (m_config->Read(wxT(PREF_FLOPPY1M), 0L) != 0);

    m_config->Read(wxT(PREF_FLOPPY0NAM), &m_floppy0File, "");
    m_config->Read(wxT(PREF_FLOPPY1NAM), &m_floppy1File, "");

#if PTERM_MDI
    // On Mac, the style rule is that the application keeps running even
    // if all its windows are closed.
    PtermFrameParent = new PtermMainFrame ();
    PtermFrameParent->Show (true);
#endif

    // Pick up the preferred x/y (saved from last time).  Adjust these
    // to make sure that the top left corner of the window is on screen,
    // and second (if possible) that the whole window is.
    m_config->Read (wxT (PREF_XPOS), &prefX, 0L);
    m_config->Read (wxT (PREF_YPOS), &prefY, 0L);
    debug ("saved x/y is %ld, %ld", prefX, prefY);
    dspi = wxDisplay::GetFromPoint (wxPoint (prefX, prefY));
    if (dspi == wxNOT_FOUND)
    {
        dspi = 0;
    }
    
    wxDisplay dsp (dspi);
    wxRect ca = dsp.GetClientArea ();
    
    debug ("display %d client area pos %d, %d size %d, %d",
           dspi, ca.x, ca.y, ca.width, ca.height);
    
    // Adjust to make the bottom right corner be on-screen
    sz = 512 + 2 * DisplayMargin;
    if (m_scale > 0.)
    {
        sz *= m_scale;
    }
    prefX += sz;
    prefY += sz;
    // width and height have some adjustment added to it to allow for
    // window decoration.  
    if (prefX > ca.x + ca.width - 5)
    {
        prefX = ca.x + ca.width - 5;
    }
    if (prefY > ca.y + ca.height - 25)
    {
        prefY = ca.y + ca.height - 25;
    }
    // From the adjusted bottom right, get the adjusted top left
    prefX -= sz;
    prefY -= sz;
    if (prefX < ca.x)
    {
        prefX = ca.x;
    }
    if (prefY < ca.y)
    {
        prefY = ca.y;
    }
    debug ("adjusted x/y is %ld, %ld", prefX, prefY);
    
    // Add some handlers so we can save the screen in various formats
    // Note that the BMP handler is always loaded, don't do it again.
    wxImage::AddHandler (new wxPNGHandler);
    wxImage::AddHandler (new wxPNMHandler);
    wxImage::AddHandler (new wxTIFFHandler);
    wxImage::AddHandler (new wxXPMHandler);

#if !defined (__WXMAC__)
    // On Mac, MacNewFile() or MacOpenFiles() will be called when OnInit
    // completes, but on other platforms we'll call it here just before
    // we're finished with OnInit.  That way the same logic works everywhere.
    MacNewFile ();
#endif

    // success: wxApp::OnRun () will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// Start connection dialog, or open a connection if auto-connecting.
// We come here if there are no arguments, or if there are but the
// argument does not refer to a profile file (*.ppf file).
void PtermApp::MacNewFile (void)
{
    if (argc > 1)
    {
        wxArrayString files = argv.GetArguments ();
        
        files.RemoveAt (0);
        MacOpenFiles (files);
        return;
    }
    
    // create the main application window.  Ask or autoconnect according
    // to the current preferences.
    DoConnect (!m_connect);
}

void PtermApp::MacOpenFiles (const wxArrayString &s)
{
    wxArrayString files = s;
    wxString filename;
    bool ret;
    size_t i;
    FILE *testdata;
    connMode testmode;
    char tline[200], *p;
    int w, seq, pseq, key;
    PtermFrame *frame;
    
#if defined (__WXMAC__)
    // On Mac, if Pterm is invoked with arguments, we usually come
    // here from the wx startup code, but NOT with the full argument
    // list.  I have no idea why.  For a workaround, check if we have
    // arguments, and if so grab argv explicitly.  Note that the
    // case of clicking on file names marked for Pterm is not handled
    // as Unix arguments so it won't trigger the workaround.
    if (argc > 1)
    {
        files = argv.GetArguments ();
        
        files.RemoveAt (0);
    }
#endif

    for (i = 0; i < files.GetCount (); i++)
    {
        filename = files[i];

        if (filename.Right (4).CmpNoCase (wxT (".ppf")) == 0)
        {
            wxString profile;

            // get just the filename (profile)
            profile = filename.BeforeLast ('.');
#if defined (_WIN32)
            profile = profile.AfterLast ('\\');
#else
            profile = profile.AfterLast ('/');
#endif
            // load profile
            ret = LoadProfile (profile, filename);

            // If the load failed, force connect dialog.  If it worked,
            // connect according to the loaded profile.
            if (ret)
            {
                DoConnect (false);
            }
            else
            {
                DoConnect (true);
                break;
            }
        }
        else if (filename.Right (4).CmpNoCase (wxT (".dat")) == 0 ||
                 filename.Right (4).CmpNoCase (wxT (".trc")) == 0)
        {
            // test data file
            testdata = fopen (filename, "r");
            if (testdata == NULL)
            {
                wxString msg ("Error opening test data file ");

                msg.Append (filename);
                msg.Append (":\n");
                msg.Append (wxSysErrorMsg ());
                
                wxMessageBox (msg, "Error", wxICON_ERROR | wxOK | wxCENTRE);
                return;
            }

            wxString title ("Test: ");

            title.Append (filename);
            frame = new PtermFrame (filename, -1, title);

            if (frame != NULL)
            {
                frame->tracePterm = true;
                traceFn[0] = '\0';
                traceF.Open ();
                frame->m_dumbTty = false;
                pseq = -1;
                testmode = both;
                
                for (;;)
                {
                    char *fret;
                    
                    fret = fgets (tline, 199, testdata);
                    if (fret == NULL)
                    {
                        fclose (testdata);
                        testdata = NULL;
                        break;
                    }
                    if (testmode == both)
                    {
                        // See if we can figure out the connection mode
                        if (strstr (tline, "ascii") != NULL ||
                            strstr (tline, "ASCII") != NULL)
                        {
                            testmode = ascii;
                        }
                    }
                
                    p = tline;
                    if (p[2] == ':')
                    {
                        // Timestamp at start of line, skip it
                        p += 14;
                    }
                    if (sscanf (p, "key to plato %o", &key))// != 0 && key < 0200)
                    {
                        // Key stroke in the trace, so this is a pause point.  But
                        // simply pausing isn't correct because that doesn't
                        // update the screen.  Need a better answer.
                    }
                    else if (sscanf (p, "%o seq %d", &w, &seq) != 0 &&
                             seq != pseq)
                    {
                        // Successful conversion, process the word,
                        // provided it is new (different sequence number
                        // than before)
                        pseq = seq;
                        if (testmode == both && w > 2)
                        {
                            // See if the value of the word gives a clue about
                            // the protocol used
                            if (w <= 0377 || (w >> 8) == 033)
                            {
                                testmode = ascii;
                            }
                            else
                            {
                                testmode = niu;
                            }
                        }
                        frame->procPlatoWord (w, testmode == ascii);
                    }
                }
                frame->m_canvas->Refresh (false);
            }
        }
        else
        {
            if (i != 0 || argc < 2)
            {
                wxMessageBox ("usage: pterm [ hostname [ portnum [ termtype ]]]\n"
                              "   or: pterm [ filename.ppf ]\n", "Usage",
                              wxICON_ERROR | wxOK | wxCENTRE);
                break;
            }
            m_hostName = argv[1];
            if (argc > 2)
            {
                m_port = atoi (wxString (argv[2]).mb_str());
                if (argc > 3)
                {
                    m_termType = atoi (wxString (argv[3]).mb_str ());
                    printf ("terminal type override %ld\n", m_termType);
                }
            }
            else
            {
                m_port = m_config->Read (wxT (PREF_PORT), DefNiuPort);
            }
            DoConnect (false);
            break;
        }       
    }
}

#if defined (__WXMAC__)
// We come here if the icon is clicked and there are no windows.  But
// sometimes we come here even if there are windows.  For the latter case,
// just show & raise the first window.
void PtermApp::MacReopenApp (void)
{
    if (m_firstFrame != NULL)
    {
        m_firstFrame->Show (true);
        m_firstFrame->Raise ();
    }
    else
    {
        DoConnect (true);
    }
}
#endif

int PtermApp::OnExit (void)
{
    wxTheClipboard->Flush ();

    m_config->Write (wxT (PREF_XPOS), prefX);
    m_config->Write (wxT (PREF_YPOS), prefY);
    m_config->Flush ();
    delete m_config;
    m_config = NULL;
    
    delete g_printData;
    delete g_pageSetupData;

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
        PtermConnDialog dlg (wxID_ANY, _("Open a new terminal window"),
                             wxDefaultPosition, wxSize (450, 355)); //450, 355));
    
        dlg.CenterOnScreen ();
        
        if (dlg.ShowModal () == wxID_OK)
        {
            if (dlg.m_ShellFirst.IsEmpty ())
            {
                m_ShellFirst = wxT ("");
            }
            else
            {
                m_ShellFirst = dlg.m_ShellFirst;
            }
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
                dlg.m_port.ToCLong (&m_port);
            }
            if (m_floppy0File.IsEmpty ())
                m_floppy0File = wxT ("");
            if (m_floppy1File.IsEmpty())
                m_floppy1File = wxT("");

            //save selection to current
            m_config = new wxConfig (wxT ("Pterm"));
            m_config->Write (wxT (PREF_SHELLFIRST), m_ShellFirst);
            m_config->Write (wxT (PREF_CURPROFILE), m_curProfile);
            m_config->Write (wxT (PREF_HOST), m_hostName);
            m_config->Write (wxT (PREF_PORT), m_port);
            m_config->Write (wxT (PREF_MTUTORBOOT), m_mTutorBoot);
            m_config->Write (wxT (PREF_FLOPPY0M), m_floppy0);
            m_config->Write (wxT (PREF_FLOPPY1M), m_floppy1);
            m_config->Write (wxT (PREF_FLOPPY0NAM), m_floppy0File);
            m_config->Write (wxT (PREF_FLOPPY1NAM), m_floppy1File);
            m_config->Flush ();

        }
        else
        {
            return false;     // connect canceled
        }
    }
    
    // create the main application window
    
    if (m_mTutorBoot && m_floppy0 && m_floppy0File.Length() > 1)
        frame = new PtermFrame(wxT("Local"), 0, wxT("Pterm local"), true);
    else
        frame = new PtermFrame(m_hostName, m_port, wxT("Pterm"));

    return (frame != NULL);
}

bool PtermApp::LoadProfile (wxString profile, wxString filename)
{
    wxString buffer;
    wxString token;
    wxString value;
    wxString str;
    wxString rgb;
    int r, g, b;
    const char *s;
    
    //open file
    if (filename.IsEmpty ())
    {
        filename = ptermApp->ProfileFileName (profile);
        if (filename.IsEmpty ())
            return false;
    }
    wxTextFile file (filename);
    if (!file.Open ())
        return false;

    //read file
    for (buffer = file.GetFirstLine (); ; buffer = file.GetNextLine ())
    {
        if (buffer.Contains (wxT ("=")))
        {
            token = buffer.BeforeFirst ('=');
            token.Trim (true);
            token.Trim (false);
            value = buffer.AfterFirst (wxT ('='));
            value.Trim (true);
            value.Trim (false);
            //tab0
            if     (token.Cmp (wxT (PREF_CURPROFILE)) == 0)
                m_curProfile    = profile;
            //tab1
            else if (token.Cmp (wxT (PREF_SHELLFIRST)) == 0)
                m_ShellFirst    = value;
            else if (token.Cmp (wxT (PREF_CONNECT)) == 0)
                m_connect       = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_HOST)) == 0)
                m_hostName      = value;
            else if (token.Cmp (wxT (PREF_PORT)) == 0)
            {
                value.ToCLong (&m_port);
            }
            //tab2
            else if (token.Cmp (wxT (PREF_SHOWSIGNON)) == 0)
                m_showSignon    = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SHOWSYSNAME)) == 0)
                m_showSysName   = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SHOWHOST)) == 0)
                m_showHost      = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SHOWSTATION)) == 0)
                m_showStation   = (value.Cmp (wxT ("1")) == 0);

            //tab3
            else if (token.Cmp (wxT (PREF_1200BAUD)) == 0)
                m_classicSpeed  = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_GSW)) == 0)
                m_gswEnable     = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_ARROWS)) == 0)
                m_numpadArrows  = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_IGNORECAP)) == 0)
                m_ignoreCapLock = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_PLATOKB)) == 0)
                m_platoKb       = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_ACCEL)) == 0)
            {
#if defined (__WXMAC__)
                m_useAccel      = true;
#else
                m_useAccel      = (value.Cmp (wxT ("1")) == 0);
#endif
            }
            else if (token.Cmp (wxT (PREF_BEEP)) == 0)
                m_beepEnable    = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SHIFTSPACE)) == 0)
                m_DisableShiftSpace = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_MOUSEDRAG)) == 0)
                m_DisableMouseDrag  = (value.Cmp (wxT ("1")) == 0);

            //tab4
            else if (token.Cmp (wxT (PREF_SCALE)) == 0)
                value.ToCDouble (&m_scale);
            else if (token.Cmp (wxT (PREF_STATUSBAR)) == 0)
                m_showStatusBar = (value.Cmp (wxT ("1")) == 0);
#if !defined (__WXMAC__)
            else if (token.Cmp (wxT (PREF_MENUBAR)) == 0)
                m_showMenuBar   = (value.Cmp (wxT ("1")) == 0);
#endif
            else if (token.Cmp (wxT (PREF_NOCOLOR)) == 0)
                m_noColor       = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_FOREGROUND)) == 0)
            {
                s = value.c_str ();
                sscanf (s, "%d %d %d", &r, &g, &b);
                m_fgColor       = wxColour (r, g, b);
            }
            else if (token.Cmp (wxT (PREF_BACKGROUND)) == 0)
            {
                s = value.c_str ();
                sscanf (s, "%d %d %d", &r, &g, &b);
                m_bgColor       = wxColour (r, g, b);
            }
            //tab5
            else if (token.Cmp (wxT (PREF_CHARDELAY)) == 0)
                value.ToCLong (&m_charDelay);
            else if (token.Cmp (wxT (PREF_LINEDELAY)) == 0)
                value.ToCLong (&m_lineDelay);
            else if (token.Cmp (wxT (PREF_AUTOLF)) == 0)
                value.ToCLong (&m_autoLF);
            else if (token.Cmp (wxT (PREF_SMARTPASTE)) == 0)
                m_smartPaste    = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_CONVDOT7)) == 0)
                m_convDot7      = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_CONV8SP)) == 0)
                m_conv8Sp       = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_TUTORCOLOR)) == 0)
                m_TutorColor    = (value.Cmp (wxT ("1")) == 0);

            //tab6
            else if (token.Cmp (wxT (PREF_EMAIL)) == 0)
                m_Email         = value;
            else if (token.Cmp (wxT (PREF_SEARCHURL)) == 0)
                m_SearchURL     = value;
            else if (token.Cmp(wxT(PREF_MTUTORBOOT)) == 0)
                m_mTutorBoot = (value.Cmp(wxT("1")) == 0);
            else if (token.Cmp(wxT(PREF_FLOPPY0M)) == 0)
                m_floppy0 = (value.Cmp(wxT("1")) == 0);
            else if (token.Cmp(wxT(PREF_FLOPPY1M)) == 0)
                m_floppy1 = (value.Cmp(wxT("1")) == 0);

            else if (token.Cmp(wxT(PREF_FLOPPY0NAM)) == 0)
                m_floppy0File = value;
            else if (token.Cmp(wxT(PREF_FLOPPY1NAM)) == 0)
                m_floppy1File = value;


         }
        if (file.Eof ())
        {
            break;
        }
    }
    file.Close ();

    //write prefs
    m_config = new wxConfig (wxT ("Pterm"));
    //tab0
    m_config->Write (wxT (PREF_CURPROFILE), profile);
    //tab1
    m_config->Write (wxT (PREF_SHOWSIGNON), (m_showSignon) ? 1 : 0);
    m_config->Write (wxT (PREF_SHOWSYSNAME), (m_showSysName) ? 1 : 0);
    m_config->Write (wxT (PREF_SHOWHOST), (m_showHost) ? 1 : 0);
    m_config->Write (wxT (PREF_SHOWSTATION), (m_showStation) ? 1 : 0);
    //tab2
    m_config->Write (wxT (PREF_1200BAUD), (m_classicSpeed) ? 1 : 0);
    m_config->Write (wxT (PREF_GSW), (m_gswEnable) ? 1 : 0);
    m_config->Write (wxT (PREF_ARROWS), (m_numpadArrows) ? 1 : 0);
    m_config->Write (wxT (PREF_IGNORECAP), (m_ignoreCapLock) ? 1 : 0);
    m_config->Write (wxT (PREF_PLATOKB), (m_platoKb) ? 1 : 0);
    m_config->Write (wxT (PREF_ACCEL), (m_useAccel) ? 1 : 0);
    m_config->Write (wxT (PREF_BEEP), (m_beepEnable) ? 1 : 0);
    m_config->Write (wxT (PREF_SHIFTSPACE), (m_DisableShiftSpace) ? 1 : 0);
    m_config->Write (wxT (PREF_MOUSEDRAG), (m_DisableMouseDrag) ? 1 : 0);
    //tab3
    m_config->Write (wxT (PREF_CONNECT), (m_connect) ? 1 : 0);
    m_config->Write (wxT (PREF_HOST), m_hostName);
    m_config->Write (wxT (PREF_PORT), m_port);
    //tab4
    m_config->Write (wxT (PREF_SCALE), m_scale);
    m_config->Write (wxT (PREF_STATUSBAR), (m_showStatusBar) ? 1 : 0);
#if !defined (__WXMAC__)
    m_config->Write (wxT (PREF_MENUBAR), (m_showMenuBar) ? 1 : 0);
#endif
    m_config->Write (wxT (PREF_NOCOLOR), (m_noColor) ? 1 : 0);
    rgb.Printf (wxT ("%d %d %d"), m_fgColor.Red (), m_fgColor.Green (),
                m_fgColor.Blue ());
    m_config->Write (wxT (PREF_FOREGROUND), rgb);
    rgb.Printf (wxT ("%d %d %d"), m_bgColor.Red (), m_bgColor.Green (),
                m_bgColor.Blue ());
    m_config->Write (wxT (PREF_BACKGROUND), rgb);
    //tab5
    m_config->Write (wxT (PREF_CHARDELAY), m_charDelay);
    m_config->Write (wxT (PREF_LINEDELAY), m_lineDelay);
    m_config->Write (wxT (PREF_AUTOLF), m_autoLF);
    m_config->Write (wxT (PREF_SMARTPASTE), (m_smartPaste) ? 1 : 0);
    m_config->Write (wxT (PREF_CONVDOT7), (m_convDot7) ? 1 : 0);
    m_config->Write (wxT (PREF_CONV8SP), (m_conv8Sp) ? 1 : 0);
    m_config->Write (wxT (PREF_TUTORCOLOR), (m_TutorColor) ? 1 : 0);
    //tab6
    m_config->Write (wxT (PREF_EMAIL), m_Email);
    m_config->Write (wxT (PREF_SEARCHURL), m_SearchURL);
    m_config->Write(wxT(PREF_MTUTORBOOT), (m_mTutorBoot) ? 1 : 0);
    m_config->Write(wxT(PREF_FLOPPY0M), (m_floppy0) ? 1 : 0);
    m_config->Write(wxT(PREF_FLOPPY1M), (m_floppy1) ? 1 : 0);

    m_config->Write(wxT(PREF_FLOPPY0NAM), m_floppy0File);
    m_config->Write(wxT(PREF_FLOPPY1NAM), m_floppy1File);


    m_config->Flush ();

    return true;
}

wxString PtermApp::ProfileFileName (wxString profile)
{
    wxString filename;

    if (profile.IsEmpty ())
        return profile;
    
    filename = wxGetCwd ();
    if (filename.Right (1) != wxT ("/") && filename.Right (1) != wxT ("\\"))
    {
#if defined (_WIN32)
        filename.Append (wxT ("\\"));
#else
        filename.Append (wxT ("/"));
#endif
    }
    filename.Append (profile);
    filename.Append (wxT (".ppf"));
    return filename;
}

void PtermApp::OnAbout (wxCommandEvent&)
{
    wxAboutDialogInfo info;

    info.SetName (_(STRPRODUCTNAME));
    info.SetVersion (_(L"V" wxT (STRFILEVER)));
#ifdef _WIN32
    // Win32 can't hack the #ifdef in a concatenated string
    info.SetDescription (_(L"PLATO terminal emulator."
                           L"\n  built with wxWidgets V" wxT (WXVERSION)
                           L"\n  build date " wxT(PTERMBUILDDATE)
                             ));
#else
    info.SetDescription (_(L"PLATO terminal emulator."
                           L"\n  built with wxWidgets V" wxT (WXVERSION)
                           L"\n  build date " wxT(PTERMBUILDDATE)
#ifdef PTERMSVNREV
                           L"\n  SVN revision " wxT (PTERMSVNREV)
#endif
                             ));
#endif
    info.SetCopyright (wxT(STRLEGALCOPYRIGHT));
    info.AddDeveloper ("Paul Koning");
    info.AddDeveloper ("Joe Stanton");
    info.AddDeveloper ("Bill Galcher");
    info.AddDeveloper ("Steve Zoppi");
    info.AddDeveloper ("Dale Sinder");
    
    info.SetWebSite ("http://cyber1.org");
    
    wxAboutBox(info);
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
        frame = new PtermFrame (str, -2, _("Keyboard Help"));

        if (frame != NULL)
        {
            for (i = 0;
                 i < sizeof (keyboardhelp) / sizeof (keyboardhelp[0]);
                 i++)
            {
                frame->procPlatoWord (keyboardhelp[i], false);
            }
            m_helpFrame = frame;
            frame->m_canvas->Refresh (false);
        }
    }
    else
    {
        m_helpFrame->Show (true);
        m_helpFrame->Raise ();
    }
}

wxColour PtermApp::SelectColor (wxWindow &parent, 
                                const wxChar *title, wxColour &initcol)
{
    wxColour col (initcol);
    wxColour orange (255, 144, 0);
    wxColour vikingGreen (0, 220, 0);
    wxColourData data;

    data.SetColour (initcol);
    data.SetCustomColour (0, orange);
    data.SetCustomColour (1, *wxBLACK);
    data.SetCustomColour (2, vikingGreen);
    
    wxColourDialog dialog (&parent, &data);

    dialog.CentreOnParent ();
    dialog.SetTitle (title);

    if (dialog.ShowModal () == wxID_OK)
    {
        col = dialog.GetColourData ().GetColour ();
    }

    return col;
}

void PtermApp::OnQuit (wxCommandEvent&)
{
    PtermFrame *frame, *nextframe;

    frame = m_firstFrame;
    while (frame != NULL)
    {
        nextframe = frame->m_nextFrame;
        frame->Close (true);
        frame = nextframe;
    }

    // Help frame is not in the list, so check for it separately
    if (m_helpFrame != NULL)
    {
        m_helpFrame->Close (true);
    }

#if PTERM_MDI // defined (__WXMAC__)
    // On the Mac, deleting all the windows doesn't terminate the
    // program, so we make it stop this way.
    ExitMainLoop ();
#endif
}



