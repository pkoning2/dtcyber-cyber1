////////////////////////////////////////////////////////////////////////////
// Name:        PtermConnDialog.h
// Purpose:     Definition of New Window dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PtermConnDialog_H__
#define __PtermConnDialog_H__ 1

#include "CommonHeader.h"
#include "PtermApp.h"

extern PtermApp *ptermApp;

// define the connection dialog
class PtermConnDialog : public wxDialog
{
public:
    PtermConnDialog (wxWindowID id, const wxString &title, wxPoint pos,
                     wxSize size);
    
    void OnButton (wxCommandEvent& event);
    void OnSelect (wxCommandEvent& event);
    void OnChange (wxCommandEvent& event);
    void OnDoubleClick (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { 
        EndModal (wxID_CANCEL); 
#if !defined (__WXMAC__)
        wxWindow *win = ptermApp->GetTopWindow();
        if (win == NULL || win == this)
        {
            ptermApp->Exit();
        }
#endif        
    }

    void CreateDefaultProfiles (wxDir& ldir);
    
    wxString        m_ShellFirst;
    wxString        m_curProfile;
    wxString        m_host;
    wxString        m_port;
    bool            m_Boot;
    wxConfig        m_config;

    wxListBox* lstProfiles;
    wxTextCtrl* txtShellFirst;
    wxTextCtrl* txtHost;
    wxComboBox* cboPort;
    wxButton* btnCancel;
    wxButton* btnConnect;
    
private:
    DECLARE_EVENT_TABLE ()
};

#endif  // __PtermConnDialog_H__
