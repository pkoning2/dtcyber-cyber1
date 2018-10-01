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
#include "PtermProfile.h"
#include "PtermProfileList.h"
#include "PtermFrame.h"

// define the preferences dialog
class PtermPrefDialog : public wxDialog
{
public:
    PtermPrefDialog (PtermConnDialog *parent, wxWindowID id,
                     const wxString &title, wxPoint pos,  wxSize size);
    PtermPrefDialog (PtermFrame *parent, wxWindowID id,
                     const wxString &title, wxPoint pos,  wxSize size,
                     const PtermProfile &fromprofile);

    ~PtermPrefDialog ();

    void PtermInitDialog (void);
    void UpdateTitle (void);
    void Modified (bool modified = true);
    
    //event sink handlers
    void OnButton (wxCommandEvent& event);
    void OnCheckbox (wxCommandEvent& event);
    void OnSelect (wxCommandEvent& event);
    void OnDoubleClick (wxCommandEvent& event);
    void OnChange (wxCommandEvent& event);
    void OnComboSelect (wxCommandEvent& event);
    void OnRadioSelect (wxCommandEvent& event);
    void OnClose (wxCloseEvent &);
    //support routines
    bool ValidProfile (wxString profile);
    void SelectProfile (int n);
    bool DeleteProfile (wxString profile);
    void SetControlState (void);

    PtermProfile *m_profile;
    bool m_profModified;
    bool m_initDone;

    bool m_floppy0Changed;
    bool m_floppy1Changed;

    u8 m_HelpContext;

    //objects
    wxNotebook *tabPrefsDialog;
    //tab0
    ProfileList *lstProfiles;
    wxButton *btnSave;
    wxButton *btnLoad;
    wxButton *btnDelete;
    wxStaticText *lblProfileStatusMessage;
    wxTextCtrl *txtProfile;
    wxButton *btnSaveAs;
    //tab1
    wxTextCtrl *txtShellFirst;
    wxTextCtrl *txtDefaultHost;
    wxComboBox *cboDefaultPort;
    //tab2
    wxCheckBox *chkShowSignon;
    wxCheckBox *chkShowSysName;
    wxCheckBox *chkShowHost;
    wxCheckBox *chkShowStation;
    //tab3
    wxCheckBox *chkSimulate1200Baud;
    wxCheckBox *chkEnableGSW;
    wxCheckBox *chkEnableNumericKeyPad;
    wxCheckBox *chkIgnoreCapLock;
    wxCheckBox *chkUsePLATOKeyboard;
#ifndef __WXMAC__
    wxCheckBox *chkUseAccelerators;
#endif
    wxCheckBox *chkEnableBeep;
    wxCheckBox *chkDisableShiftSpace;
    wxCheckBox *chkDisableMouseDrag;
    //tab4
    wxCheckBox *chkDisableColor;
#if defined (_WIN32)
    wxButton *btnFGColor;
    wxButton *btnBGColor;
#else
    wxBitmapButton *btnFGColor;
    wxBitmapButton *btnBGColor;
#endif
    wxCheckBox *chkFancyScale;
    //wxComboBox *cboDefaultScale;
    wxRadioBox *rdoDefaultScale;
    wxCheckBox *chkShowMenuBar;
    wxCheckBox *chkShowStatusBar;
    //tab5
    wxTextCtrl *txtCharDelay;
    wxTextCtrl *txtLineDelay;
    wxComboBox *cboAutoLF;
    wxCheckBox *chkSmartPaste;
    wxCheckBox *chkConvertDot7;
    wxCheckBox *chkConvert8Spaces;
    wxCheckBox *chkTutorColor;
    wxCheckBox *chkTrimEnd;
    //tab6
    wxTextCtrl *txtSearchURL;
    wxCheckBox *chkMTutorBoot;
    wxRadioBox *radMTutor;
    wxCheckBox *chkFloppy0;
    wxCheckBox *chkFloppy1;
    wxStaticText *txtFloppy0;
    wxStaticText *txtFloppy1;
    wxButton *btnFloppy0;
    wxButton *btnFloppy1;

    //button bar
    wxButton *btnHelp;
    wxButton *btnOK;
    wxButton *btnCancel;
    wxButton *btnDefaults;

    //properties    
    bool            m_profileEdit;

    //tab0
    wxString        m_curProfile;

    PtermConnDialog * m_connParent;

private:
    void paintBitmap (wxBitmap &bm, wxColour &color);
    
    DECLARE_EVENT_TABLE ()
};

#endif  // __PtermPrefDialog_H__
