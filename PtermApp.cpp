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
#include "PtermConnDialog.h"
#include "PtermPrefDialog.h"
#include <wx/stdpaths.h>
#include "ptermversion.h"

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
#else
Trace debugF;
#endif

// the application icon (under Windows, Mac, and OS/2 it is in resources)
#if defined (__WXGTK__) || defined (__WXMOTIF__) || defined (__WXMGL__) || defined (__WXX11__)
#include "pterm_32.xpm"
#endif

BEGIN_EVENT_TABLE (PtermApp, wxApp)
    EVT_MENU (Pterm_Connect, PtermApp::OnConnect)
    EVT_MENU (Pterm_Quit,    PtermApp::OnQuit)
    EVT_MENU (Pterm_HelpKeys, PtermApp::OnHelpKeys)
    EVT_MENU (Pterm_HelpIndex, PtermApp::OnHelpIndex)
    EVT_MENU (Pterm_About,   PtermApp::OnAbout)
    EVT_MENU (Pterm_Pref,    PtermApp::OnPref)
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
    wxString rgb;
    wxString level;
    wxString str;

#if defined (__WXMAC__)
    fixmenu ();
#endif
    
    ptermApp = this;
    SetAppName (wxT ("Pterm"));
    m_firstFrame = NULL;
    m_helpFrame = NULL;
    m_connDialog = NULL;
    m_prefDialog = NULL;
    m_sessDialog = NULL;
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
#ifdef DEBUG
    debugF.Open ("debug.trc");
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


#if defined (__WXGTK__)
    m_config = new wxConfig (wxT ("Pterm"), wxEmptyString, wxEmptyString,
			     wxEmptyString, 
			     wxCONFIG_USE_SUBDIR | wxCONFIG_USE_LOCAL_FILE);
#else
    m_config = new wxConfig (wxT ("Pterm"));
#endif
    // Get the standard places for program resources, and where to put
    // per-user data files given the application name.  Create the
    // data file directory if necessary.  This is where we'll put
    // profiles.
    m_resourcedir = wxStandardPaths::Get ().GetResourcesDir ();
    m_userdatadir = wxStandardPaths::Get ().GetUserDataDir ();
    bool dirok = wxFileName::Mkdir (m_userdatadir, wxS_DIR_DEFAULT,
                                    wxPATH_MKDIR_FULL);
    if (!dirok)
    {
        wxString msg (_("Error creating application data directory "));
        msg.Append (m_userdatadir);
        msg.Append (":\n");
        msg.Append (wxSysErrorMsg ());
                
        wxMessageBox (msg, "Error", wxICON_ERROR | wxOK);
    }
    // Load saved values
    m_config->Read (wxT (PREF_CURPROFILE), &m_curProfile, wxT (""));
    // " Current " (with space before/after) is the former name of the
    // "default" profile, which is no longer in use.  If we find that
    // saved as the current profile, pretend we have no current
    // profile.
    if (m_curProfile == wxT (" Current "))
        m_curProfile = wxT ("");

#if PTERM_MDI
    // On Mac, the style rule is that the application keeps running even
    // if all its windows are closed.
    PtermFrameParent = new PtermMainFrame ();
    // We'd really like Show (false) here to prevent the dummy window
    // from showing up as a gray box.  But that doesn't work (on Mac)
    // because then its menu bar doesn't appear either.
    PtermFrameParent->Show (true);
#endif


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
void PtermApp::MacNewFile (void)
{
    if (argc > 1)
    {
        wxArrayString files = argv.GetArguments ();
        
        files.RemoveAt (0);
        MacOpenFiles (files);
        return;
    }
    
    // create the main application window.  Ask for a connection.
    DoConnectDialog ();
}

void PtermApp::MacOpenFiles (const wxArrayString &s)
{
    wxArrayString files = s;
    wxString filename;
    size_t i;
    FILE *testdata;
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
    // Make sure we don't do this later (if someone drags/drops files)
    argc = 0;
#endif

    if (files.GetCount () == 0)
    {
        // No arguments, look up the saved last profile name
        PtermProfile *lastprof = NULL;
        if (!m_curProfile.IsEmpty ())
        {
            lastprof = new PtermProfile (m_curProfile, true);
            if (lastprof->IsOk () && lastprof->m_autoConnect)
                DoConnect (lastprof);
            else
            {
                delete lastprof;
            }
        }
    }
    for (i = 0; i < files.GetCount (); i++)
    {
        filename = files[i];
        wxFileName wxfilename (filename);
        
        if (wxfilename.IsOk () &&
            wxfilename.GetExt ().CmpNoCase (wxT ("ppf")) == 0)
        {
            PtermProfile *prof = new PtermProfile (wxfilename.GetName (), true);
            if (prof->IsOk ())
            {
                DoConnect (prof);
            }
            else
                delete prof;
        }
        else if (wxfilename.IsOk () &&
            wxfilename.GetExt ().CmpNoCase (wxT ("mte")) == 0)
        {
            PtermProfile *prof = new PtermProfile ();
            prof->m_mTutorBoot = true;
            prof->m_floppy0 = true;
            prof->m_floppy0File = filename;
            DoConnect (prof);
        }
        else if (wxfilename.IsOk () &&
                 wxfilename.GetExt ().CmpNoCase (wxT ("trc")) == 0)
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
                continue;
            }

            PtermProfile *tprof = new PtermProfile ();
            PtermTestConnection *tconn = new PtermTestConnection (testdata);
            
            wxString title ("Test: ");
            title.Append (filename);

            frame = new PtermFrame (title, tprof, tconn);
            if (frame != NULL)
            {
                frame->tracePterm = true;
                traceFn[0] = '\0';
                traceF.Open ();
                frame->m_dumbTty = false;
            }
        }
        else
        {
            wxString host;
            int port, termtype;
            
            if (i != 0 || files.GetCount () < 1)
            {
                wxMessageBox ("usage: pterm [ hostname [ portnum [ termtype ]]]\n"
                              "   or: pterm [ filename.ppf ]\n", "Usage",
                              wxICON_ERROR | wxOK | wxCENTRE);
                break;
            }
            host = files[0];
            port = DefNiuPort;
            termtype = -1;
            if (files.GetCount () > 1)
            {
                port = atoi (files[1].mb_str());
                if (files.GetCount () > 2)
                {
                    termtype = atoi (files[2].mb_str ());
                    //printf ("terminal type override %d\n", termtype);
                }
            }
            PtermProfile *hprof = new PtermProfile ();
            hprof->m_host = host;
            hprof->m_port = port;
            hprof->m_termType = termtype;
            PtermHostConnection *hconn = new PtermHostConnection (host, port);
            frame = new PtermFrame (wxT ("Pterm"), hprof, hconn);
            
            break;
        }       
    }
    if (ptermApp->m_firstFrame == NULL)
    {
        // Nothing was opened above, so open the connect dialog.
        DoConnectDialog ();
    }
}

#if defined (__WXMAC__)
// We come here if the icon is clicked and there are no windows.  But
// sometimes we come here even if there are windows.  For the latter case,
// just show & raise the first window (most recently opened window).
void PtermApp::MacReopenApp (void)
{
    if (m_firstFrame != NULL)
    {
        m_firstFrame->Show (true);
        m_firstFrame->Raise ();
    }
    else
    {
        DoConnectDialog ();
    }
}
#endif

int PtermApp::OnExit (void)
{
    wxTheClipboard->Flush ();

    m_config->Flush ();
    
    delete m_config;
    m_config = NULL;
    
    delete g_printData;
    delete g_pageSetupData;

    return 0;
}

void PtermApp::OnConnect (wxCommandEvent &)
{
    DoConnectDialog ();
}

void PtermApp::DoConnectDialog (void)
{
    if (m_connDialog == NULL)
        m_connDialog = new PtermConnDialog (wxID_ANY,
                                            _("Open a new terminal window"),
                                            wxDefaultPosition,
                                            wxSize (450, 355));
    
    m_connDialog->CenterOnScreen ();
    m_connDialog->Raise ();
    m_connDialog->Show ();
}

void PtermApp::DoConnect (PtermProfile *prof)
{
    PtermFrame *frame;
    
    if (!prof->m_profileName.IsEmpty ())
        m_curProfile = prof->m_profileName;
            
    //save selection to current
    m_config->Write (wxT (PREF_CURPROFILE), m_curProfile);
    m_config->Flush ();

    // create the main application window

    PtermConnection *conn;
    wxString title;
    switch (prof->ProfileState ())
    {
    case LOCALBOOT:
        conn = new PtermLocalConnection ();
        title = wxT("Pterm local");
        break;
    case CONNECTION:
        conn = new PtermHostConnection (prof->m_host, prof->m_port);
        title = wxT("Pterm");
        break;
    default:
        printf ("Unexpected profile state\n");
        assert (0);
    }
    frame = new PtermFrame (title, prof, conn);
}

void PtermApp::OnPref (wxCommandEvent&)
{
    if (ptermApp->m_prefDialog != NULL)
    {
        return;  // user MUST close previous dialog first
    }

    ptermApp->m_prefDialog = new PtermPrefDialog (NULL, wxID_ANY, _ ("Profile Editor"),
        wxDefaultPosition, wxSize (481, 575));

    ptermApp->m_prefDialog->CenterOnScreen ();
    ptermApp->m_prefDialog->Raise ();
    ptermApp->m_prefDialog->Show (true);
}

void PtermApp::OnAbout (wxCommandEvent&)
{
    wxAboutDialogInfo info;

    info.SetName (_(STRPRODUCTNAME));
    info.SetVersion (wxT (STRFILEVER));
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
    info.AddArtist ("Lisa Anne Allyn (icons)");

#if !defined (__WXMAC__) && !defined (__WXMSW__)
    info.SetWebSite ("http://cyber1.org");
#endif
    
    wxAboutBox (info);
}

void PtermApp::OnHelpKeys (wxCommandEvent &)
{
    LaunchMtutorHelp (helpContextKeyboard);
}

void PtermApp::OnHelpIndex (wxCommandEvent &)
{
    LaunchMtutorHelp (helpContextGenericIndex);
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

    // Help frame and connect dialog window are not in the list, so
    // check for those separately
    if (m_helpFrame != NULL)
    {
        m_helpFrame->Close (true);
    }
    if (m_connDialog != NULL)
    {
        m_connDialog->Close (true);
    }

#if PTERM_MDI // defined (__WXMAC__)
    // On the Mac, deleting all the windows doesn't terminate the
    // program, so we make it stop this way.
    ExitMainLoop ();
#endif
}

#if !defined (__WXMAC__)
// Test if it is time for the program to exit; if yes do so.
//
// Keep running if:
// 1. Any terminal window is open
// 2. The help window is open
// 3. The "New Terminal Window" dialog window is open
void PtermApp::TestForExit (void)
{
    if (m_firstFrame == NULL &&
        m_helpFrame == NULL &&
        m_connDialog == NULL)
    {
        Exit ();
    }
}
#endif

void PtermApp::LaunchMtutorHelp (u8 helpContext)
{
    PtermConnection *conn;
    wxString title;
    conn = new PtermLocalConnection ();
    title = wxT ("Pterm help");
    PtermProfile *hprof = new PtermProfile (wxT (""), false);
    hprof->m_showStatusBar = false;
    hprof->m_useAccel = true;
    hprof->m_floppy0 = true;
    hprof->m_isHelp = true;

    wxWindow *wnd = ptermApp->GetTopWindow (); 
    wxPoint loc = wnd->GetPosition ();
    hprof->m_restoreX = loc.x;
    hprof->m_restoreY = loc.y;

    if (m_helpFrame == NULL)
    {
        m_helpFrame = new PtermFrame (title, hprof, conn);
        m_helpFrame->m_MTFiles[0].SetRamBased ("ptermhelp.mte");
        m_helpFrame->m_MTFiles[0].SetHelpContext (helpContext);
        m_helpFrame->m_needtoBoot = true;
        m_helpFrame->CenterOnScreen ();
        m_helpFrame->Raise ();
    }
    else
    {
        m_helpFrame->m_MTFiles[0].SetHelpContext (helpContext);
        m_helpFrame->m_needtoBoot = true;
        m_helpFrame->Raise ();
    }
}

