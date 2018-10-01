////////////////////////////////////////////////////////////////////////////
// Name:        PtermProfile.h
// Purpose:     Definition of the Profile handler
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     12/24/2017
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PTermProfile_H__
#define __PTermProfile_H__ 1

#include "CommonHeader.h"

extern wxFileName ProfileFileName (wxString profile);

typedef enum
{
    CONNECTION,
    LOCALBOOT,
    INCOMPLETE
} profile_state_e;

class PtermProfile
{
public:
    bool        m_isHelp;
    wxString    m_profileName;
    wxFileName  m_fileName;
    bool        m_Ok;

    bool        m_lockPosition;
    
    //tab1
    wxString    m_ShellFirst;
    bool        m_autoConnect;
    wxString    m_host;
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
    bool        m_FancyScaling;
    double      m_scale;    // Window scale factor or special value
#define SCALE_ASPECT  0.    // Scale to window, square aspect ratio
#define SCALE_FREE   -1.    // Scale to window, free form
    bool        m_showStatusBar;
#if !defined (__WXMAC__)
    bool        m_showMenuBar;
#else
    static const bool m_showMenuBar;
#endif
    long        m_restoreX;
    long        m_restoreY;

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
    bool        m_trimEnd;
    //tab6
    wxString    m_SearchURL;
    bool        m_mTutorBoot;
    bool        m_floppy0;
    bool        m_floppy1;
    wxString    m_floppy0File;
    wxString    m_floppy1File;

    // Methods
    PtermProfile ();
    PtermProfile (wxString name, bool load = true);
    void init (void);
    bool LoadProfile (void);
    bool SaveProfile (void);
    bool SetName (wxString name);
    profile_state_e ProfileState (void) const;
    bool IsOk (void) const { return m_Ok; }
};

#endif  // __PTermProfile_H__
