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


extern "C" int ptermNextGswWord (void *connection, int idle);

class PtermFrame;

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
    void OnAbout (wxCommandEvent& event);

    bool DoConnect (bool ask);
    
    bool LoadProfile (wxString profile, wxString filename);
    wxString ProfileFileName (wxString profile);

    static wxColour SelectColor (wxWindow &parent, const wxChar *title, 
                                 wxColour &initcol);

    // These are wxApp methods to override on Mac, but we use them for
    // roughly their Mac purpose on other platforms as well, so define
    // them unconditionally.
    void MacNewFile (void);
    void MacOpenFiles (const wxArrayString &s);
#if defined (__WXMAC__)
    void MacReopenApp (void);
#endif

    wxConfig    *m_config;

    //general
    int         m_lastTab;
    //tab0
    wxString    m_curProfile;
    //tab1
    wxString    m_ShellFirst;
    bool        m_connect;
    wxString    m_hostName;
    long        m_port;
    long        m_termType;
    
    //tab2
    bool        m_showSignon;
    bool        m_showSysName;
    bool        m_showHost;
    bool        m_showStation;
    //tab3
    bool        m_classicSpeed;
    bool        m_gswEnable;
    bool        m_numpadArrows;
    bool        m_ignoreCapLock;
    bool        m_platoKb;
    bool        m_useAccel;
    bool        m_beepEnable;
    bool        m_DisableShiftSpace;
    bool        m_DisableMouseDrag;
    //tab4
    double      m_scale;    // Window scale factor or special value
#define SCALE_ASPECT  0.    // Scale to window, square aspect ratio
#define SCALE_FREE   -1.    // Scale to window, free form
    bool        m_showStatusBar;
#if !defined (__WXMAC__)
    bool        m_showMenuBar;
#else
    static const bool m_showMenuBar;
#endif
    bool        m_noColor;
    wxColour    m_fgColor;
    wxColour    m_bgColor;
    
    //tab5
    long        m_charDelay;
    long        m_lineDelay;
    long        m_autoLF;
    bool        m_smartPaste;
    bool        m_convDot7;
    bool        m_conv8Sp;
    bool        m_TutorColor;
    //tab6
    wxString    m_Email;      
    wxString    m_SearchURL;
    bool        m_mTutorBoot;
    long        m_mTutorLevel;
    bool        m_floppy0;
    bool        m_floppy1;
    wxString    m_floppy0File;
    wxString    m_floppy1File;

    PtermFrame  *m_firstFrame;
    wxString    m_defDir;
    PtermFrame  *m_helpFrame;

    long        prefX;
    long        prefY;

    PtermFrame *m_CurFrame;

    char        traceFn[20];

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
