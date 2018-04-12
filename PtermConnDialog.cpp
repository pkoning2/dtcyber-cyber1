////////////////////////////////////////////////////////////////////////////
// Name:        PtermConnDialog.cpp
// Purpose:     Implementation of New Window dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermConnDialog.h"
#include "PtermPrefDialog.h"

// ----------------------------------------------------------------------------
// PtermConnDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (PtermConnDialog, wxDialog)
    EVT_CLOSE (PtermConnDialog::OnClose)
    EVT_LISTBOX (wxID_ANY, PtermConnDialog::OnSelect)
    EVT_LISTBOX_DCLICK (wxID_ANY, PtermConnDialog::OnDoubleClick)
    EVT_BUTTON (wxID_ANY, PtermConnDialog::OnButton)
    END_EVENT_TABLE ();

PtermConnDialog::PtermConnDialog (wxWindowID id, const wxString &title,
                                  wxPoint pos, wxSize loc)
    : wxDialog (NULL, id, title, pos, loc),
      m_profile (NULL)
{
    // static ui objects, note dynamic controls, e.g. those that hold
    // values or require event processing are declared above
    wxStaticText* lblExplainProfiles;
    wxFont dfont = wxFont (10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_NORMAL);
    wxFont dfont2 = wxFont (10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD);

    // ui object creation / placement, note initialization of values is below
    wxBoxSizer* bs1;
    bs1 = new wxBoxSizer (wxVERTICAL);
    lblExplainProfiles = new wxStaticText (this, wxID_ANY,
                                           _("Select a profile and click Connect or Boot."),
                                           wxDefaultPosition, wxDefaultSize, 0);
    lblExplainProfiles->SetFont (dfont2);
    bs1->Add (lblExplainProfiles, 0, wxTOP|wxRIGHT|wxLEFT, 5);
    lstProfiles = new ProfileList (this, wxID_ANY, wxDefaultPosition,
                                   wxSize (-1, -1));
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
    fgs112 = new wxFlexGridSizer (1, 4, 0, 0);
    fgs112->AddGrowableCol (1);
    fgs112->SetFlexibleDirection (wxBOTH);
    btnEdit = new wxButton (this, wxID_ANY, _("Edit"), wxDefaultPosition,
                            wxDefaultSize, wxTAB_TRAVERSAL);
    btnEdit->SetFont (dfont);
    fgs112->Add (btnEdit, 0, wxALL, 5);

    //fgs112->Add (0, 0, 1, wxALL, 5);
    
    btnHelp = new wxButton (this, wxID_ANY, _ ("Help"), wxDefaultPosition,
        wxDefaultSize, wxTAB_TRAVERSAL);
    btnHelp->SetFont (dfont);
    fgs112->Add (btnHelp, 0, wxALL, 5);

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

    if (lstProfiles->GetCount () == 0)
    {
        // We don't have any profiles.  Create the two default ones.
        CreateDefaultProfiles (ldir);
        lstProfiles->RefreshList ();
    }
    int cur = lstProfiles->GetSelection ();
    if (cur != wxNOT_FOUND)
        SelectProfile (cur);
    else
        btnConnect->Enable (false);
}

PtermConnDialog::~PtermConnDialog ()
{
    ptermApp->m_connDialog = NULL;
    ptermApp->TestForExit ();
}

void PtermConnDialog::CreateDefaultProfiles (wxDir& )
{
    PtermProfile *prof;

    prof = new PtermProfile (wxT ("Default Classic"), false);
    prof->SaveProfile ();
    delete prof;
    prof = new PtermProfile (wxT ("Default Ascii"), false);
    prof->m_port = 8005;
    prof->SaveProfile ();
    delete prof;
}

void PtermConnDialog::OnClose (wxCloseEvent &)
{
    Destroy ();
}

void PtermConnDialog::OnButton (wxCommandEvent& event)
{
    wxString profile;
    
    if (event.GetEventObject () == btnCancel)
    {
        Close ();
    }
    else if (event.GetEventObject () == btnEdit)
    {
        if (ptermApp->m_prefDialog != NULL)
        {
            return;  // user MUST close previous dialog first
        }

        // Set the default profile to edit, if one has been selected
        // here.
        if (m_profile != NULL && !m_profile->m_profileName.IsEmpty ())
            ptermApp->m_curProfile = m_profile->m_profileName;

        ptermApp->m_prefDialog = new PtermPrefDialog (this, wxID_ANY, _ ("Profile Editor"),
            wxDefaultPosition, wxSize (481, 575));

        ptermApp->m_prefDialog->CenterOnScreen ();
        ptermApp->m_prefDialog->Raise ();
        ptermApp->m_prefDialog->Show (true);
    }
    else if (event.GetEventObject () == btnConnect)
    {
        ptermApp->DoConnect (m_profile);
        Close ();
    }
    else if (event.GetEventObject () == btnHelp)
    {
        ptermApp->LaunchMtutorHelp (helpContextConnecting);
    }
}

void PtermConnDialog::OnSelect (wxCommandEvent& event)
{
    wxString str;
    wxString profile;
    wxString filename;
    if (event.GetEventObject () == lstProfiles)
    {
        SelectProfile (lstProfiles->GetSelection ());
    }
}

void PtermConnDialog::OnDoubleClick (wxCommandEvent& event)
{
    if (event.GetEventObject () == lstProfiles)
    {
        SelectProfile (lstProfiles->GetSelection ());
        // If it's a good profile, we're done.
        if (btnConnect->IsEnabled ())
        {
            ptermApp->DoConnect (m_profile);
            Close ();
        }
    }
}

void PtermConnDialog::SelectProfile (int n)
{
    wxString profile;
    wxString str;
    wxString filename;

    if (n != wxNOT_FOUND)
        profile = lstProfiles->GetString (n);
    if (m_profile != NULL)
    {
        if (profile == m_profile->m_profileName)
            return;
        delete m_profile;
    }
    
    m_profile = new PtermProfile (profile, true);
    if (m_profile->IsOk ())
    {
        CheckProfileState ();
    }
    else
    {
        str.Printf (_("Profile '%s' not found. Problem loading file:\n\n    %s"),
                    profile, m_profile->m_fileName.GetFullPath ());
        delete m_profile;
        m_profile = NULL;
        wxMessageBox (str, _("Error"), wxOK | wxICON_ERROR);
    }
}

void PtermConnDialog::ReloadProfile ()
{
    if (m_profile != NULL)
    {
        m_profile->LoadProfile ();
        CheckProfileState ();
    }

}

void PtermConnDialog::CheckProfileState (void)
{
    switch (m_profile->ProfileState ())
    {
    case CONNECTION:
        btnConnect->Enable (true);
        btnConnect->SetLabel (_ ("Connect"));
        break;
    case LOCALBOOT:
        btnConnect->Enable (true);
        btnConnect->SetLabel (("Boot"));
        break;
    case INCOMPLETE:
        btnConnect->Enable (false);
    }
}

