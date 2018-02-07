////////////////////////////////////////////////////////////////////////////
// Name:        PtermProfileList.h
// Purpose:     Definition of ProfileList class
// Authors:     Paul Koning
// Created:     2/1/2018
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PtermProfileList_H__
#define __PtermProfileList_H__ 1

#include "CommonHeader.h"
// A helper class that holds a list of profile names
class ProfileList : public wxListBox
{
public:
    ProfileList (wxWindow *parent, wxWindowID id,
                 const wxPoint & pos = wxDefaultPosition,
                 const wxSize & size = wxDefaultSize);
    void RefreshList (void);
};

#endif  // __PtermProfileList_H__
