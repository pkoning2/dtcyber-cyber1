////////////////////////////////////////////////////////////////////////////
// Name:        PtermPrefDialog.h
// Purpose:     Definition of MicroTutor dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     12/20/2017
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PtermMtutorDialog_H__
#define __PtermMtutorDialog_H__ 1

#include "CommonHeader.h"

#include "PtermFrame.h"


class PtermMtutorDialog : public wxDialog
{
public:
    PtermMtutorDialog (PtermFrame *parent, wxWindowID id,
        const wxString &title, wxPoint pos, wxSize size);
    ~PtermMtutorDialog();

    void OnClose(wxCloseEvent &);
    void OnButton(wxCommandEvent& event);
    void OnCheckbox(wxCommandEvent& event);
    void OnRadiobox(wxCommandEvent& event);

    PtermFrame* m_parent;
    wxStaticText* lblExplain0;
    wxControl* tabMtutorDialog;

    wxRadioBox* radMTutor;
    wxCheckBox* chkFloppy0;
    wxCheckBox* chkFloppy1;
    wxStaticText* txtFloppy0;
    wxStaticText* txtFloppy1;
    wxButton* btnFloppy0;
    wxButton* btnFloppy1;

    wxButton* btnOK;
    wxButton* btnCancel;

    bool m_floppy0Changed;
    bool m_floppy1Changed;

    long            m_mTutorLevel;
    bool            m_floppy0;
    bool            m_floppy1;
    wxString        m_floppy0File;
    wxString        m_floppy1File;

private:

    DECLARE_EVENT_TABLE()
};


#endif
