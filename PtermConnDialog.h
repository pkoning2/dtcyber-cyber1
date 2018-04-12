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
#include "PtermProfile.h"
#include "PtermProfileList.h"

// define the connection dialog
class PtermConnDialog : public wxDialog
{
public:
    PtermConnDialog (wxWindowID id, const wxString &title, wxPoint pos,
                     wxSize size);
    ~PtermConnDialog ();
    
    void OnButton (wxCommandEvent& event);
    void OnSelect (wxCommandEvent& event);
    void OnDoubleClick (wxCommandEvent& event);
    void OnClose (wxCloseEvent &);
    void CreateDefaultProfiles (wxDir& ldir);
    void ReloadProfile (void);

    PtermProfile    *m_profile;

    ProfileList *lstProfiles;
    wxButton *btnEdit;
    wxButton *btnHelp;
    wxButton *btnCancel;
    wxButton *btnConnect;
    
private:
    void SelectProfile (int n);
    void CheckProfileState (void);
    
    DECLARE_EVENT_TABLE ()
};

#endif  // __PtermConnDialog_H__
