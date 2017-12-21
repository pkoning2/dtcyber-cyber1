////////////////////////////////////////////////////////////////////////////
// Name:        PtermPrefDialog.cpp
// Purpose:     Implementation of MicroTutor dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     12/20/2017
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermMtutorDialog.h"
#include "PtermPrefDialog.h"

const wxString radioChoices[MAXMCHOICES] = { _("None"), _("2"), _("3"), _("4"), _("Dev") };


BEGIN_EVENT_TABLE(PtermMtutorDialog, wxDialog)
EVT_CLOSE(PtermMtutorDialog::OnClose)
EVT_BUTTON(wxID_ANY, PtermMtutorDialog::OnButton)
EVT_CHECKBOX(wxID_ANY, PtermMtutorDialog::OnCheckbox)
EVT_RADIOBOX(wxID_ANY, PtermMtutorDialog::OnRadiobox)
END_EVENT_TABLE();


PtermMtutorDialog::PtermMtutorDialog(PtermFrame *parent, wxWindowID id,
    const wxString &title, wxPoint pos, wxSize size) 
    : wxDialog(parent, id, title, pos, size)
{
    m_parent = parent;

    m_floppy0Changed = m_floppy1Changed = false;

    wxFont dfont = wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    // ui object creation / placement, note initialization of values is below
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);
    
    tabMtutorDialog = new wxNotebook(this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxNB_TOP);
    tabMtutorDialog->SetFont(dfont);


    wxScrolledWindow* tab = new wxScrolledWindow(tabMtutorDialog, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxHSCROLL | wxTAB_TRAVERSAL | wxVSCROLL);

    tab->SetScrollRate(5, 5);
    tab->SetFont(dfont);

    wxStaticText* lblFloppy0;
    wxStaticText* lblFloppy1;

    m_floppy0File = parent->m_floppy0File;
    m_floppy1File = parent->m_floppy1File;

    wxFlexGridSizer* page0;
    page0 = new wxFlexGridSizer(0, 1, 0, 0);
    page0->AddGrowableCol(0);
    page0->SetFlexibleDirection(wxBOTH);

    lblExplain0 = new wxStaticText(tab, wxID_ANY,
        _("Use this page to manage MicroTutor settings for this session"),
        wxDefaultPosition, wxDefaultSize, 0);
    page0->Add(lblExplain0, 0, wxALL, 5);

    radMTutor = new wxRadioBox(tab, wxID_ANY,
        _("MicroTutor Level"),
        wxDefaultPosition, wxDefaultSize, MAXMCHOICES, radioChoices, MAXMCHOICES, wxRA_SPECIFY_COLS);
    radMTutor->SetSelection(PtermPrefDialog::LevelToSelect(parent->m_mtPLevel));
    
    if (parent->m_mtutorBoot)
        radMTutor->Disable();
    
    page0->Add(radMTutor, 0, wxALL, 5);

    chkFloppy0 = new wxCheckBox(tab, wxID_ANY,
        _("Enable Floppy 0"),
        wxDefaultPosition, wxDefaultSize, 0);
    chkFloppy0->SetValue(parent->m_floppy0);
    page0->Add(chkFloppy0, 0, wxALL, 5);

    chkFloppy1 = new wxCheckBox(tab, wxID_ANY,
        _("Enable Floppy 1"),
        wxDefaultPosition, wxDefaultSize, 0);
    chkFloppy1->SetValue(parent->m_floppy1);
    page0->Add(chkFloppy1, 0, wxALL, 5);


    lblFloppy0 = new wxStaticText(tab, wxID_ANY,
        _("Floppy 0 file:"),
        wxDefaultPosition, wxDefaultSize, 0);
    page0->Add(lblFloppy0, 0, wxALL, 5);

    txtFloppy0 = new wxStaticText(tab, wxID_ANY, parent->m_floppy0File, wxDefaultPosition,
        wxDefaultSize, 0);
    page0->Add(txtFloppy0, 0, wxLEFT, 25);

    btnFloppy0 = new wxButton(tab, wxID_ANY, _("Select Floppy 0 File"),
        wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    page0->Add(btnFloppy0, 0, wxALL, 5);

    lblFloppy1 = new wxStaticText(tab, wxID_ANY,
        _("Floppy 1 file:"),
        wxDefaultPosition, wxDefaultSize, 0);
    page0->Add(lblFloppy1, 0, wxALL, 5);

    txtFloppy1 = new wxStaticText(tab, wxID_ANY, parent->m_floppy1File, wxDefaultPosition,
        wxDefaultSize, 0);
    page0->Add(txtFloppy1, 0, wxLEFT, 25);

    btnFloppy1 = new wxButton(tab, wxID_ANY, _("Select Floppy 1 File"),
        wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    page0->Add(btnFloppy1, 0, wxALL, 5);

    //btnOK = new wxButton(tab, wxID_ANY, _("OK"), wxPoint(10, 300),
    //    wxDefaultSize, 0);
    //btnOK->SetFont(dfont);
    //page0->Add(btnOK, 0, wxALL, 5);
    //btnCancel = new wxButton(tab, wxID_CANCEL, _("Cancel"),
    //    wxPoint(150, 300), wxDefaultSize, 0);
    //btnCancel->SetFont(dfont);
    //page0->Add(btnCancel, 0, wxALL, 5);

    tab->SetSizer(page0);
    tab->Layout();
    page0->Fit(tab);
    tabMtutorDialog->AddPage(tab, _("Session Settings"), false);

    ////notebook
    bSizer1->Add(tabMtutorDialog, 1, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 6);

    //button bar
    wxFlexGridSizer* fgsButtons;
    fgsButtons = new wxFlexGridSizer(1, 5, 0, 0);
    fgsButtons->AddGrowableCol(0);
    fgsButtons->AddGrowableRow(0);
    fgsButtons->SetFlexibleDirection(wxHORIZONTAL);
    fgsButtons->Add(0, 0, 1, wxALL | wxEXPAND, 5);
    btnOK = new wxButton(this, wxID_ANY, _("OK"), wxDefaultPosition,
        wxDefaultSize, 0);
    btnOK->SetFont(dfont);
    fgsButtons->Add(btnOK, 0, wxALL, 5);
    btnCancel = new wxButton(this, wxID_CANCEL, _("Cancel"),
        wxDefaultPosition, wxDefaultSize, 0);
    btnCancel->SetFont(dfont);
    fgsButtons->Add(btnCancel, 0, wxALL, 5);
    bSizer1->Add(fgsButtons, 0, wxEXPAND, 5);
    this->SetSizer(bSizer1);
    this->Layout();

}


PtermMtutorDialog::~PtermMtutorDialog()
{
}


void PtermMtutorDialog::OnClose(wxCloseEvent& event)
{
    EndModal(wxID_CANCEL);
}

void PtermMtutorDialog::OnButton(class wxCommandEvent& event)
{
    void OnButton(wxCommandEvent& event);

    if (event.GetEventObject() == btnFloppy0)
    {
        wxFileDialog
            openFileDialog(this, _("Open Floppy 0 file"), "", "",
                "MTU files (*.mtu)|*.mtu", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        MTFile testFile;

        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;     // the user changed idea...

        bool isOK = testFile.Test(openFileDialog.GetPath());

        if (!isOK)
        {
            wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
            return;
        }
        testFile.Close();
        m_floppy0File = openFileDialog.GetPath();
        txtFloppy0->SetLabel(m_floppy0File);
        m_floppy0Changed = true;
    }
    else if (event.GetEventObject() == btnFloppy1)
    {
        wxFileDialog
            openFileDialog(this, _("Open Floppy 1 file"), "", "",
                "MTU files (*.mtu)|*.mtu", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        MTFile testFile;

        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;     // the user changed idea...

        bool isOK = testFile.Test(openFileDialog.GetPath());

        if (!isOK)
        {
            wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
            return;
        }
        testFile.Close();
        m_floppy1File = openFileDialog.GetPath();
        txtFloppy1->SetLabel(m_floppy1File);
        m_floppy1Changed = true;
    }
    else if (event.GetEventObject() == btnOK)
    {
        m_parent->m_mtPLevel = m_parent->m_mTutorLevel = m_mTutorLevel;

        if (m_floppy0Changed)
        {
            m_parent->m_floppy0 = m_floppy0;
            m_parent->m_floppy0File = m_floppy0File;

            if (m_floppy0 && m_floppy0File.Length() > 0)
                m_parent->m_MTFiles[0].Open(m_floppy0File);
            else
                m_parent->m_MTFiles[0].Close();
        }

        if (m_floppy1Changed)
        {
            m_parent->m_floppy1 = m_floppy1;
            m_parent->m_floppy1File = m_floppy1File;

            if (m_floppy1 && m_floppy1File.Length() > 0)
                m_parent->m_MTFiles[1].Open(m_floppy0File);
            else
                m_parent->m_MTFiles[1].Close();
        }

        EndModal(wxID_OK);
    }
    else if (event.GetEventObject() == btnCancel)
    {
        EndModal(wxID_CANCEL);
    }
}

void PtermMtutorDialog::OnCheckbox(class wxCommandEvent& event)
{
       if (event.GetEventObject() == chkFloppy0)
       {
           m_floppy0 = event.IsChecked();
           m_floppy0Changed = true;
       }

       else if (event.GetEventObject() == chkFloppy1)
       {
           m_floppy1 = event.IsChecked();
           m_floppy1Changed = true;
       }

}

void PtermMtutorDialog::OnRadiobox(class wxCommandEvent& event)
{
    if (event.GetEventObject() == radMTutor)
    {
        m_mTutorLevel = PtermPrefDialog::SelectToLevel(radMTutor->GetSelection());
    }
}

