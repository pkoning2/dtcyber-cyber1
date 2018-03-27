////////////////////////////////////////////////////////////////////////////
// Name:        PtermConnFailDialog.cpp
// Purpose:     Implmentation of Failed Connection dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermConnFailDialog.h"
#include "PtermConnDialog.h"

// ----------------------------------------------------------------------------
// PtermConnFailDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (PtermConnFailDialog, wxDialog)
    EVT_CLOSE (PtermConnFailDialog::OnClose)
    EVT_BUTTON (wxID_ANY, PtermConnFailDialog::OnButton)
    END_EVENT_TABLE ();

PtermConnFailDialog::PtermConnFailDialog (wxWindowID id, const wxString &title,
                                          wxPoint pos, wxSize loc, int code,
                                          PtermProfile *prof)
    : wxDialog (NULL, id, title, pos, loc)
{

    // static ui objects, note dynamic controls, e.g. those that hold values
    // or require event processing are declared above
    wxStaticText *lblPrompt;
    wxStaticText *lblHost;
    wxHyperlinkCtrl *c1url;
    wxString str;
    wxFont dfont = wxFont (10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    // ui object creation / placement, note initialization of values is below
    this->SetFont (dfont);
    wxBoxSizer* bs1;
    bs1 = new wxBoxSizer (wxVERTICAL);
    lblPrompt = new wxStaticText (this, wxID_ANY, wxT (""), wxDefaultPosition,
                                  wxDefaultSize, 0);
    lblPrompt->SetFont (dfont);
    bs1->Add (lblPrompt, 1, wxTOP|wxRIGHT|wxLEFT|wxEXPAND, 5);
    lblHost = new wxStaticText (this, wxID_ANY, wxT (""), wxDefaultPosition,
                                wxDefaultSize, 0);
    lblHost->SetFont (dfont);
    bs1->Add (lblHost, 0, wxALL, 5);

    c1url = new wxHyperlinkCtrl (this, wxID_ANY, wxT (""), 
                                 wxT ("http://cyber1.org"));
    bs1->Add (c1url, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    wxFlexGridSizer* fgs11;
    fgs11 = new wxFlexGridSizer (1, 4, 0, 0);
    fgs11->AddGrowableCol (2);
    fgs11->SetFlexibleDirection (wxHORIZONTAL);
    btnNew = new wxButton (this, wxID_ANY, _("New Connection"),
                           wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL);
    btnNew->SetFont (dfont);
    fgs11->Add (btnNew, 0, wxALL, 5);
    btnRetry = new wxButton (this, wxID_ANY, _("Reconnect"), wxDefaultPosition,
                             wxDefaultSize, 0 | wxTAB_TRAVERSAL);
    btnRetry->SetFont (dfont);
    fgs11->Add (btnRetry, 0, wxALL, 5);
    fgs11->Add (0, 0, 1, wxALL, 5);
    btnCancel = new wxButton (this, wxID_CANCEL, _("Close"),
                              wxDefaultPosition, wxDefaultSize,
                              0|wxTAB_TRAVERSAL);
    btnCancel->SetFont (dfont);
    fgs11->Add (btnCancel, 0, wxALL, 5);
    bs1->Add (fgs11, 0, wxEXPAND, 5);

    // set object value properties
    if (code == C_DISCONNECT)
    {
        str.Printf (_("Your connection to the host was lost.\n\n"));
    }
    else if (code == C_CONNFAIL1)        
    {
        str.Printf (_("The hostname is unknown or invalid.\n\n"));
    }
    else
    {
        str.Printf (_("The host is not reachable.\n\n"));
    }
        
    str.Append (_("You may open a new connection, reconnect\n"
                  "to the same host/port, or close the window."));
    lblPrompt->SetLabel (str);
    str.Printf (_("\nFailed: %s:%ld"), prof->m_host, prof->m_port);
    str.Append (_("\n\nFor more information, see: "));
    
    lblHost->SetLabel (str);

    //size the controls
    this->SetSizer (bs1);
    this->Layout ();
    bs1->Fit (this);
    btnCancel->SetDefault ();
    btnCancel->SetFocus ();
}

void PtermConnFailDialog::OnButton (wxCommandEvent& event)
{
    if (event.GetEventObject () == btnNew)
    {
        EndModal (wxID_OK);
    }
    else if (event.GetEventObject () == btnRetry)
    {
        EndModal (Pterm_ConnectAgain);
    }
    else if (event.GetEventObject () == btnCancel)
    {
        EndModal (wxID_CANCEL);
    }
}

void PtermConnFailDialog::OnClose (wxCloseEvent&)
{
    EndModal (wxID_CANCEL);
}


