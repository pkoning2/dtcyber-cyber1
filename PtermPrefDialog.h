////////////////////////////////////////////////////////////////////////////
// Name:        PtermPrefDialog.h
// Purpose:     Definition of Preferences dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __PtermPrefDialog_H__
#define __PtermPrefDialog_H__ 1

#include "CommonHeader.h"
#include "PtermConnection.h"

#include "PtermFrame.h"

// define the preferences dialog
class PtermPrefDialog : public wxDialog
{
public:
    PtermPrefDialog (PtermFrame *parent, wxWindowID id,
                     const wxString &title, wxPoint pos,  wxSize size,
                     bool profileEdit);

    //event sink handlers
    void OnButton (wxCommandEvent& event);
    void OnCheckbox (wxCommandEvent& event);
    void OnRadiobox (wxCommandEvent& event);
    void OnSelect (wxCommandEvent& event);
    void OnDoubleClick (wxCommandEvent& event);
    void OnChange (wxCommandEvent& event);
    void OnComboSelect (wxCommandEvent& event);
    //void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    void OnClose (wxCloseEvent &);
    //support routines
    bool ValidProfile (wxString profile);
    bool SaveProfile (wxString profile);
    bool DeleteProfile (wxString profile);
    void SetControlState (void);
    static int SelectToLevel(int select);
    static int LevelToSelect(int level);

    bool m_floppy0Changed;
    bool m_floppy1Changed;

    //objects
    wxNotebook* tabPrefsDialog;
    //tab0
    wxListBox* lstProfiles;
    wxButton* btnSave;
    wxButton* btnLoad;
    wxButton* btnDelete;
    wxStaticText* lblProfileStatusMessage;
    wxTextCtrl* txtProfile;
    wxButton* btnAdd;
    //tab1
    wxCheckBox* chkConnectAtStartup;
    wxTextCtrl* txtShellFirst;
    wxTextCtrl* txtDefaultHost;
    wxComboBox* cboDefaultPort;
    //tab2
    wxCheckBox* chkShowSignon;
    wxCheckBox* chkShowSysName;
    wxCheckBox* chkShowHost;
    wxCheckBox* chkShowStation;
    //tab3
    wxCheckBox* chkSimulate1200Baud;
    wxCheckBox* chkEnableGSW;
    wxCheckBox* chkEnableNumericKeyPad;
    wxCheckBox* chkIgnoreCapLock;
    wxCheckBox* chkUsePLATOKeyboard;
    wxCheckBox* chkUseAccelerators;
    wxCheckBox* chkEnableBeep;
    wxCheckBox* chkDisableShiftSpace;
    wxCheckBox* chkDisableMouseDrag;
    //tab4
    wxCheckBox* chkDisableColor;
#if defined (_WIN32)
    wxButton* btnFGColor;
    wxButton* btnBGColor;
#else
    wxBitmapButton* btnFGColor;
    wxBitmapButton* btnBGColor;
#endif
    //tab5
    wxTextCtrl* txtCharDelay;
    wxTextCtrl* txtLineDelay;
    wxComboBox* cboAutoLF;
    wxCheckBox* chkSmartPaste;
    wxCheckBox* chkConvertDot7;
    wxCheckBox* chkConvert8Spaces;
    wxCheckBox* chkTutorColor;
    //tab6
    wxTextCtrl* txtEmail;
    wxTextCtrl* txtSearchURL;
    wxCheckBox* chkMTutorBoot;
    wxRadioBox* radMTutor;
    wxCheckBox* chkFloppy0;
    wxCheckBox* chkFloppy1;
    wxStaticText* txtFloppy0;
    wxStaticText* txtFloppy1;
    wxButton* btnFloppy0;
    wxButton* btnFloppy1;

    //button bar
    wxButton* btnOK;
    wxButton* btnCancel;
    wxButton* btnDefaults;

    //properties    

    PtermFrame*     m_owner;

    bool            m_profileEdit;

    int             m_lastTab;
    //tab0
    wxString        m_curProfile;
    //tab1
    wxString        m_ShellFirst;
    bool            m_connect;
    wxString        m_host;
    long            m_port;
    //tab2
    bool            m_showSignon;
    bool            m_showSysName;
    bool            m_showHost;
    bool            m_showStation;
    //tab3
    bool            m_classicSpeed;
    bool            m_gswEnable;
    bool            m_numpadArrows;
    bool            m_ignoreCapLock;
    bool            m_platoKb;
    bool            m_useAccel;
    bool            m_beepEnable;
    bool            m_DisableShiftSpace;
    bool            m_DisableMouseDrag;
    //tab4
    double          m_scale;
    bool            m_showStatusBar;
#if !defined (__WXMAC__)
    bool            m_showMenuBar;
#endif
    bool            m_noColor;
    wxColour        m_fgColor;
    wxColour        m_bgColor;
    //tab5
    long            m_charDelay;
    long            m_lineDelay;
    long            m_autoLF;
    bool            m_smartPaste;
    bool            m_convDot7;
    bool            m_conv8Sp;
    bool            m_TutorColor;
    //tab6
    wxString        m_Email;
    wxString        m_SearchURL;
    bool            m_mTutorBoot;
    long            m_mTutorLevel;
    bool            m_floppy0;
    bool            m_floppy1;
    wxString        m_floppy0File;
    wxString        m_floppy1File;

private:
    void paintBitmap (wxBitmap &bm, wxColour &color);
    
    DECLARE_EVENT_TABLE ()
};

#endif  // __PtermPrefDialog_H__
