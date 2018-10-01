////////////////////////////////////////////////////////////////////////////
// Name:        PtermProfile.cpp
// Purpose:     Implementation of the PtermProfile class
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     12/24/2017
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermProfile.h"
#include "PtermApp.h"

#if defined (__WXMAC__)
// On the Mac, menus are always displayed
const bool PtermProfile::m_showMenuBar = true;
#endif

wxFileName ProfileFileName (wxString profile)
{
    wxFileName filename;

    if (profile.IsEmpty ())
        return filename;

    filename.SetPath (ptermApp->m_userdatadir);
    filename.SetName (profile);
    filename.SetExt (wxT ("ppf"));
    return filename;
}

PtermProfile::PtermProfile ()
{
    m_profileName = wxT ("");
    m_fileName = wxT ("");
    init ();
}

PtermProfile::PtermProfile (wxString name, bool load)
{
    SetName (name);
    
    init ();
    
    if (load)
    {
        LoadProfile ();
    }
}

bool PtermProfile::SetName (wxString name)
{
    wxFileName fn = ProfileFileName (name);

    if (!fn.IsOk ())
        return false;

    m_fileName = fn;
    m_profileName = fn.GetName ();

    return true;
}

void PtermProfile::init (void)
{
    m_isHelp = false;
    // The default profile is "OK"
    m_Ok = true;
    m_lockPosition = false;
    
    // Set the default values
    //tab1
    m_ShellFirst = wxT ("");
    m_autoConnect = false;
    m_host = DEFAULTHOST;
    m_port = DefNiuPort;
    m_termType = -1;        // No terminal type override
    //tab2
    m_showSignon = false;
    m_showSysName = false;
    m_showHost = true;
    m_showStation = true;
    //tab3
    m_classicSpeed = false;
    m_gswEnable = true;
    m_numpadArrows = true;
    m_ignoreCapLock = false;
    m_platoKb = false;
#if defined (__WXMAC__)
    m_useAccel = true;
#else
    m_useAccel = false;
#endif
    m_beepEnable = true;
    m_DisableShiftSpace = false;
    m_DisableMouseDrag = false;
    //tab4
    m_FancyScaling = false;
    m_scale = 1.0;
    m_showStatusBar = true;
#if !defined (__WXMAC__)
    m_showMenuBar = true;
#endif
    m_restoreX = 50;
    m_restoreY = 50;
    m_noColor = false;
    // 255 144 0 is RGB for Plato Orange
    m_fgColor = wxColour (255, 144, 0);
    m_bgColor = wxColour (0, 0, 0);
    //tab5
    m_charDelay = PASTE_CHARDELAY;
    m_lineDelay = PASTE_LINEDELAY;
    m_autoLF = 0L;
    m_smartPaste = false;
    m_convDot7 = false;
    m_conv8Sp = false;
    m_TutorColor = false;
    m_trimEnd = true;
    //tab6
    m_SearchURL = DEFAULTSEARCH;
    m_mTutorBoot = false;
    m_floppy0 = false;
    m_floppy1 = false;

    m_floppy0File = wxT ("");
    m_floppy1File = wxT ("");
}

bool PtermProfile::LoadProfile (void)
{
    wxString buffer;
    wxString token;
    wxString value;
    wxString str;
    wxString rgb;
    int r, g, b;
    const char *s;

    // Assume unsuccessful
    m_Ok = false;
    
    if (!m_fileName.IsOk () || !m_fileName.IsFileReadable ())
    {
        return false;
    }
    
    //open file
    wxTextFile file (m_fileName.GetFullPath ());
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
            //tab1
            if (token.Cmp (wxT (PREF_SHELLFIRST)) == 0)
                m_ShellFirst = value;
            else if (token.Cmp (wxT (PREF_CONNECT)) == 0)
                m_autoConnect = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_HOST)) == 0)
                m_host = value;
            else if (token.Cmp (wxT (PREF_PORT)) == 0)
            {
                value.ToCLong (&m_port);
            }
            //tab2
            else if (token.Cmp (wxT (PREF_SHOWSIGNON)) == 0)
                m_showSignon = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SHOWSYSNAME)) == 0)
                m_showSysName = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SHOWHOST)) == 0)
                m_showHost = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SHOWSTATION)) == 0)
                m_showStation = (value.Cmp (wxT ("1")) == 0);

            //tab3
            else if (token.Cmp (wxT (PREF_1200BAUD)) == 0)
                m_classicSpeed = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_GSW)) == 0)
                m_gswEnable = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_ARROWS)) == 0)
                m_numpadArrows = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_IGNORECAP)) == 0)
                m_ignoreCapLock = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_PLATOKB)) == 0)
                m_platoKb = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_ACCEL)) == 0)
            {
#if defined (__WXMAC__)
                m_useAccel = true;
#else
                m_useAccel = (value.Cmp (wxT ("1")) == 0);
#endif
            }
            else if (token.Cmp (wxT (PREF_BEEP)) == 0)
                m_beepEnable = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SHIFTSPACE)) == 0)
                m_DisableShiftSpace = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_MOUSEDRAG)) == 0)
                m_DisableMouseDrag = (value.Cmp (wxT ("1")) == 0);

            //tab4
            else if (token.Cmp (wxT (PREF_FANCYSCALE)) == 0)
                m_FancyScaling = (value.Cmp (wxT ("1")) == 0);
            else if (token.Cmp (wxT (PREF_SCALE)) == 0)
            {
                value.ToCDouble (&m_scale);
            }
            else if (token.Cmp (wxT (PREF_STATUSBAR)) == 0)
                m_showStatusBar = (value.Cmp (wxT ("1")) == 0);
#if !defined (__WXMAC__)
            else if (token.Cmp (wxT (PREF_MENUBAR)) == 0)
                m_showMenuBar   = (value.Cmp (wxT ("1")) == 0);
#endif
            else if (token.Cmp (wxT (PREF_RESTOREX)) == 0)
            {
                value.ToCLong (&m_restoreX);
            }
            else if (token.Cmp (wxT (PREF_RESTOREY)) == 0)
            {
                value.ToCLong (&m_restoreY);
            }
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
            else if (token.Cmp (wxT (PREF_TRIMEND)) == 0)
                m_trimEnd       = (value.Cmp (wxT ("1")) == 0);

            //tab6
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


            else if (token.Cmp (wxT (PREF_LOCKPOSITION)) == 0)
                m_lockPosition = (value.Cmp (wxT ("1")) == 0);

        }
        if (file.Eof ())
        {
            break;
        }
    }
    file.Close ();

    m_Ok = true;
    
    return true;
}

bool PtermProfile::SaveProfile (void)
{
    wxString buffer;
    bool openok;

    if (!IsOk () || !m_fileName.IsOk ())
    {
        return false;
    }
    
    //open file
    wxTextFile file (m_fileName.GetFullPath ());
    if (file.Exists ())
        openok = file.Open ();
    else
        openok = file.Create ();
    if (!openok)
    {
        printf ("profile save failed, open error\n");
        return false;
    }
    file.Clear ();

    //write profile
    //tab1
    buffer.Printf (wxT (PREF_CONNECT) wxT ("=%d"), (m_autoConnect) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHELLFIRST) wxT ("=%s"), m_ShellFirst);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_HOST) wxT ("=%s"), m_host);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_PORT) wxT ("=%ld"), m_port);
    file.AddLine (buffer);
    //tab2
    buffer.Printf (wxT (PREF_SHOWSIGNON) wxT ("=%d"), (m_showSignon) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHOWSYSNAME) wxT ("=%d"), (m_showSysName) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHOWHOST) wxT ("=%d"), (m_showHost) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHOWSTATION) wxT ("=%d"), (m_showStation) ? 1 : 0);
    file.AddLine (buffer);
    //tab3
    buffer.Printf (wxT (PREF_1200BAUD) wxT ("=%d"), (m_classicSpeed) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_GSW) wxT ("=%d"), (m_gswEnable) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_ARROWS) wxT ("=%d"), (m_numpadArrows) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_IGNORECAP) wxT ("=%d"), (m_ignoreCapLock) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_PLATOKB) wxT ("=%d"), (m_platoKb) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_ACCEL) wxT ("=%d"), (m_useAccel) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_BEEP) wxT ("=%d"), (m_beepEnable) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHIFTSPACE) wxT ("=%d"), (m_DisableShiftSpace) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_MOUSEDRAG) wxT ("=%d"), (m_DisableMouseDrag) ? 1 : 0);
    file.AddLine (buffer);
    //tab4
    buffer.Printf (wxT (PREF_FANCYSCALE) wxT ("=%d"), (m_FancyScaling) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SCALE) wxT ("=%f"), m_scale);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_STATUSBAR) wxT ("=%d"), (m_showStatusBar) ? 1 : 0);
    file.AddLine (buffer);
#if !defined (__WXMAC__)
    buffer.Printf (wxT (PREF_MENUBAR) wxT ("=%d"), (m_showMenuBar) ? 1 : 0);
    file.AddLine (buffer);
#endif
    buffer.Printf (wxT (PREF_RESTOREX) wxT ("=%ld"), (m_restoreX));
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_RESTOREY) wxT ("=%ld"), (m_restoreY));
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_LOCKPOSITION) wxT ("=%d"), (m_lockPosition) ? 1 : 0);
    file.AddLine (buffer);

    buffer.Printf (wxT (PREF_NOCOLOR) wxT ("=%d"), (m_noColor) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_FOREGROUND) wxT ("=%d %d %d"), m_fgColor.Red (), m_fgColor.Green (), m_fgColor.Blue ());
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_BACKGROUND) wxT ("=%d %d %d"), m_bgColor.Red (), m_bgColor.Green (), m_bgColor.Blue ());
    file.AddLine (buffer);
    //tab5
    buffer.Printf (wxT (PREF_CHARDELAY) wxT ("=%ld"), m_charDelay);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_LINEDELAY) wxT ("=%ld"), m_lineDelay);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_AUTOLF) wxT ("=%ld"), m_autoLF);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SMARTPASTE) wxT ("=%d"), (m_smartPaste) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_CONVDOT7) wxT ("=%d"), (m_convDot7) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_CONV8SP) wxT ("=%d"), (m_conv8Sp) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_TUTORCOLOR) wxT ("=%d"), (m_TutorColor) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_TRIMEND) wxT ("=%d"), (m_trimEnd) ? 1 : 0);
    file.AddLine (buffer);
    //tab6
    buffer.Printf (wxT (PREF_SEARCHURL) wxT ("=%s"), m_SearchURL);
    file.AddLine (buffer);
    buffer.Printf(wxT(PREF_MTUTORBOOT) wxT("=%d"), (m_mTutorBoot) ? 1 : 0);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY0M) wxT("=%d"), (m_floppy0) ? 1 : 0);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY1M) wxT("=%d"), (m_floppy1) ? 1 : 0);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY0NAM) wxT("=%s"), m_floppy0File);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY1NAM) wxT("=%s"), m_floppy1File);
    file.AddLine(buffer);

    //write to disk
    file.Write ();
    file.Close ();

    return true;
}

profile_state_e PtermProfile::ProfileState (void) const
{
    if (!IsOk ())
        return INCOMPLETE;
    if (m_mTutorBoot)
    {
        if (m_floppy0 && !m_floppy0File.IsEmpty ())
            return LOCALBOOT;
    }
    else if (!m_host.IsEmpty () && m_port != 0)
        return CONNECTION;
    return INCOMPLETE;
}

