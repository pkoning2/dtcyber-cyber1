////////////////////////////////////////////////////////////////////////////
// Name:        PtermConnFailDialog.h
// Purpose:     Definition of Failed Connection dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PtermConnFailDialog_H__
#define __PtermConnFailDialog_H__ 1

#include "CommonHeader.h"
#include "PtermProfile.h"

// define the connection fail dialog
class PtermConnFailDialog : public wxDialog
{
public:
    PtermConnFailDialog (wxWindowID id, const wxString &title, wxPoint pos,
                         wxSize size, int word, PtermProfile *prof);
    
    void OnButton (wxCommandEvent& event);
    void OnClose (wxCloseEvent& event);
    
    wxButton* btnNew;
    wxButton* btnRetry;
    wxButton* btnCancel;
    
private:
    DECLARE_EVENT_TABLE ()
};

#endif  // __PtermConnFailDialog_H__
