////////////////////////////////////////////////////////////////////////////
// Name:        PtermApp.h
// Purpose:     Definition of the App
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PTermApp_H__
#define __PTermApp_H__ 1

#include "CommonHeader.h"
#include "PtermProfile.h"

extern "C" int ptermNextGswWord (void *connection, int idle);

class PtermFrame;
class PtermConnDialog;
class PtermPrefDialog;
class PtermApp;
extern PtermApp *ptermApp;

// Define a new application type, each program should derive a class from wxApp
class PtermApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit () returns false, the application terminates)
    virtual bool OnInit (void);
    virtual int OnExit (void);

    // event handlers
    void OnConnect (wxCommandEvent &event);
    void OnQuit (wxCommandEvent& event);
    void OnHelpKeys (wxCommandEvent &event);
    void OnHelpIndex (wxCommandEvent &event);
    void OnPref(wxCommandEvent& event);
    void OnAbout (wxCommandEvent& event);

    void DoConnect (PtermProfile *prof);
    void DoConnectDialog (void);
    
    static wxColour SelectColor (wxWindow &parent, const wxChar *title, 
                                 wxColour &initcol);

    // These are wxApp methods to override on Mac, but we use them for
    // roughly their Mac purpose on other platforms as well, so define
    // them unconditionally.
    void MacNewFile (void);
    void MacOpenFiles (const wxArrayString &s);
#if defined (__WXMAC__)
    void MacReopenApp (void);
    void TestForExit (void) {}
#else
    void TestForExit (void);
#endif

    void LaunchMtutorHelp (u8 helpContext);

    wxConfig    *m_config;
    wxString    m_userdatadir;
    wxString    m_resourcedir;

    // Values saved in m_config
    wxString    m_curProfile;

    PtermFrame  *m_firstFrame;
    wxString    m_defDir;
    PtermFrame  *m_helpFrame;

    PtermFrame *m_CurFrame;

    char        traceFn[20];

    PtermConnDialog *m_connDialog;

    PtermPrefDialog *m_prefDialog;
    PtermPrefDialog *m_sessDialog;

private:
    wxLocale    m_locale; // locale we'll be using
    
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};


#if defined (__WXMAC__)
// A menu fixer for Mac
extern "C" 
{
    extern void fixmenu (void);
}
#endif

#endif  // __PTermApp_H__
