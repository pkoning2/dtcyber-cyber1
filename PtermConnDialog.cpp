////////////////////////////////////////////////////////////////////////////
// Name:        PtermConnDialog.cpp
// Purpose:     Implementation of New Window dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermConnDialog.h"

// ----------------------------------------------------------------------------
// PtermConnDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (PtermConnDialog, wxDialog)
    EVT_CLOSE (PtermConnDialog::OnClose)
    EVT_LISTBOX (wxID_ANY, PtermConnDialog::OnSelect)
    EVT_LISTBOX_DCLICK (wxID_ANY, PtermConnDialog::OnDoubleClick)
    EVT_TEXT (wxID_ANY, PtermConnDialog::OnChange)
    EVT_BUTTON (wxID_ANY, PtermConnDialog::OnButton)
    END_EVENT_TABLE ();

PtermConnDialog::PtermConnDialog (wxWindowID id, const wxString &title, wxPoint pos, wxSize loc)
    : wxDialog (NULL, id, title, pos, loc)
{

    // static ui objects, note dynamic controls, e.g. those that hold values or require event processing are declared above
    wxStaticText* lblExplainProfiles;
//  wxListBox* lstProfiles;
//  wxButton* btnCancel;
//  wxButton* btnConnect;
    wxFont dfont = wxFont (10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    // ui object creation / placement, note initialization of values is below
    wxBoxSizer* bs1;
    bs1 = new wxBoxSizer (wxVERTICAL);
    lblExplainProfiles = new wxStaticText (this, wxID_ANY,
                                           _("Select a profile and click Connect or Boot."),
                                           wxDefaultPosition, wxDefaultSize, 0);
    lblExplainProfiles->SetFont (dfont);
    bs1->Add (lblExplainProfiles, 0, wxTOP|wxRIGHT|wxLEFT, 5);
    lstProfiles = new wxListBox (this, wxID_ANY, wxDefaultPosition,
                                 wxSize (-1, -1), 0, NULL, wxLB_SORT); 
    lstProfiles->SetFont (dfont);
    lstProfiles->SetMinSize (wxSize (400, 222));
    bs1->Add (lstProfiles, 0, wxALL | wxEXPAND, 5);

    wxFlexGridSizer* fgSizer11;
    fgSizer11 = new wxFlexGridSizer (0, 1, 0, 0);
    fgSizer11->AddGrowableCol (0);
    fgSizer11->AddGrowableRow (0);
    fgSizer11->SetFlexibleDirection (wxBOTH);

    wxFlexGridSizer* fgs111;
    fgs111 = new wxFlexGridSizer (3, 2, 0, 0);

    wxFlexGridSizer* fgs112;
    fgs112 = new wxFlexGridSizer (2, 3, 0, 0);
    fgs112->AddGrowableCol (0);
    fgs112->SetFlexibleDirection (wxBOTH);
    fgs112->Add (0, 0, 1, wxALL, 5);
    btnCancel = new wxButton (this, wxID_CANCEL, _("Cancel"), wxDefaultPosition,
                              wxDefaultSize, wxTAB_TRAVERSAL);
    btnCancel->SetFont (dfont);
    fgs112->Add (btnCancel, 0, wxALL, 5);
    btnConnect = new wxButton (this, wxID_ANY, _("Connect"), wxDefaultPosition,
                               wxDefaultSize, wxTAB_TRAVERSAL);
    btnConnect->SetFont (dfont);
    fgs112->Add (btnConnect, 0, wxALL, 5);
    fgSizer11->Add (fgs112, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL, 5);
    bs1->Add (fgSizer11, 0, wxEXPAND, 5);
    this->SetSizer (bs1);
    this->Layout ();
    bs1->Fit (this);
    btnConnect->SetDefault ();
    btnConnect->SetFocus ();

    // populate dropdown
    wxDir ldir (wxGetCwd ());
    if (ldir.IsOpened ())
    {
        wxString filename;
        retry:
        bool cont = ldir.GetFirst (&filename, wxT ("*.ppf"), wxDIR_DEFAULT);
        // if cont is false should add two default profiles here and then goto above line
        if (!cont)
        {
            CreateDefaultProfiles (ldir);
            goto retry;
        }

        int i, cur=0;
        for (i=0; cont; i++)
        {
            filename = filename.Left (filename.Len ()-4);
            lstProfiles->Append (filename);
            cont = ldir.GetNext (&filename);
        }
        for (i=0; i< (int)lstProfiles->GetCount (); i++)
        {
            filename = lstProfiles->GetString (i);
            if (filename.CmpNoCase (ptermApp->m_curProfile) == 0)
                cur = i;
        }
        lstProfiles->Select (cur);

        if (ptermApp->LoadProfile(ptermApp->m_curProfile, wxT("")))
        {
            m_curProfile = ptermApp->m_curProfile;
            m_ShellFirst = ptermApp->m_ShellFirst;
            m_host = ptermApp->m_hostName;
            m_port.Printf(wxT("%ld"), ptermApp->m_port);
        }
    }

    // initialize values
    m_Boot = ptermApp->m_mTutorBoot;

    if (m_Boot)
        btnConnect->SetLabel(wxT("Boot"));
    else
        btnConnect->SetLabel(wxT("Connect"));
}

void PtermConnDialog::CreateDefaultProfiles (wxDir& ldir)
{
    wxString profile = wxT("Connect_Cyber1_Classic");
    long port = 5004;
    wxString filename;
    wxString buffer;
    bool openok;
    bool repeat = true;
    ldir;        // Suppress C4100 warning for unreferenced formal parameter

    rerun:

    //open file
    filename = ptermApp->ProfileFileName(profile);
    wxTextFile file(filename);
    openok = file.Create();
    if (!openok)
        return;
    file.Clear();

    //write prefs
    //tab0
    buffer.Printf(wxT(PREF_CURPROFILE) wxT("=%s"), profile);
    file.AddLine(buffer);
    //tab1
    buffer.Printf(wxT(PREF_CONNECT) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_SHELLFIRST) wxT("="));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_HOST) wxT("=cyberserv.org"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_PORT) wxT("=%ld"), port);
    file.AddLine(buffer);
    //tab2
    buffer.Printf(wxT(PREF_SHOWSIGNON) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_SHOWSYSNAME) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_SHOWHOST) wxT("=1"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_SHOWSTATION) wxT("=1"));
    file.AddLine(buffer);
    //tab3
    buffer.Printf(wxT(PREF_1200BAUD) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_GSW) wxT("=1"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_ARROWS) wxT("=1"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_IGNORECAP) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_PLATOKB) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_ACCEL) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_BEEP) wxT("=1"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_SHIFTSPACE) wxT("=1"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_MOUSEDRAG) wxT("=0"));
    file.AddLine(buffer);
    //tab4
    buffer.Printf(wxT(PREF_SCALE) wxT("=%f"), 1.0);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_STATUSBAR) wxT("=1"));
    file.AddLine(buffer);
#if !defined (__WXMAC__)
    buffer.Printf(wxT(PREF_MENUBAR) wxT("=1"));
    file.AddLine(buffer);
#endif
    buffer.Printf(wxT(PREF_NOCOLOR) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FOREGROUND) wxT("=255 144 0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_BACKGROUND) wxT("=0 0 0"));
    file.AddLine(buffer);
    //tab5
    buffer.Printf(wxT(PREF_CHARDELAY) wxT("=165"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_LINEDELAY) wxT("=500"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_AUTOLF) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_SMARTPASTE) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_CONVDOT7) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_CONV8SP) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_TUTORCOLOR) wxT("=0"));
    file.AddLine(buffer);
    //tab6
    buffer.Printf(wxT(PREF_EMAIL) wxT("="));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_SEARCHURL) wxT("=%s"), wxT("http://www.google.com/search?q="));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_MTUTORBOOT) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY0M) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY1M) wxT("=0"));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY0NAM) wxT("="));
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY1NAM) wxT("="));
    file.AddLine(buffer);

    //write to disk
    file.Write();
    file.Close();

    if (!repeat)
        return;

    port = 8005;

    profile = wxT("Connect_Cyber1_Ascii");
    repeat = false;
    goto rerun;

}

void PtermConnDialog::OnButton (wxCommandEvent& event)
{
    void OnButton (wxCommandEvent& event);
    if (event.GetEventObject () == btnCancel)
    {
        EndModal (wxID_CANCEL);
#if !defined (__WXMAC__)
        wxWindow *win = ptermApp->GetTopWindow();
        if (win == NULL || win == this)
        {
            ptermApp->Exit();
        }
#endif
    }
    if (event.GetEventObject () == btnConnect)
    {
        m_curProfile = lstProfiles->GetStringSelection ();
        EndModal (wxID_OK);
    }
}

void PtermConnDialog::OnSelect (wxCommandEvent& event)
{
    void OnSelect (wxCommandEvent& event);
    wxString str;
    wxString profile;
    wxString filename;
    if (event.GetEventObject () == lstProfiles)
    {
        profile = lstProfiles->GetStringSelection ();
        if (ptermApp->LoadProfile (profile, wxT ("")))
        {
            m_curProfile = profile;
            m_ShellFirst = ptermApp->m_ShellFirst;
            m_host = ptermApp->m_hostName;
            m_port.Printf (wxT ("%ld"), ptermApp->m_port);
            m_Boot = ptermApp->m_mTutorBoot;

            if (m_Boot)
                btnConnect->SetLabel(wxT ("Boot"));
            else
                btnConnect->SetLabel(wxT("Connect"));
        }
        else
        {
            filename = ptermApp->ProfileFileName (profile);
            str.Printf (wxT ("Profile '%s' not found. Missing file:\n\n"),
                        profile);
            str.Append (filename);
            wxMessageBox (str, wxT ("Error"), wxOK | wxICON_HAND);
        }
    }
}

void PtermConnDialog::OnChange (wxCommandEvent& event)
{
    event;      // Suppress C4100 warning for unreferenced formal parameter
    void OnChange (wxCommandEvent& event);
}

void PtermConnDialog::OnDoubleClick (wxCommandEvent& event)
{
    void OnDoubleClick (wxCommandEvent& event);
    wxString str;
    wxString profile;
    wxString filename;
    if (event.GetEventObject () == lstProfiles)
    {
        profile = lstProfiles->GetStringSelection ();
        if (ptermApp->LoadProfile (profile, wxT ("")))
        {
            m_curProfile = profile;
            m_ShellFirst = ptermApp->m_ShellFirst;
            m_host = ptermApp->m_hostName;
            m_port.Printf (wxT ("%ld"), ptermApp->m_port);
            EndModal (wxID_OK);
        }
        else
        {
            filename = ptermApp->ProfileFileName (profile);
            str.Printf (wxT ("Profile '%s' not found. Missing file:\n\n"),
                        profile);
            str.Append (filename);
            wxMessageBox (str, wxT ("Error"), wxOK | wxICON_HAND);
        }
    }
}

