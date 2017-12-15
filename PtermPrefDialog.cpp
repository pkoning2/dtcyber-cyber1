////////////////////////////////////////////////////////////////////////////
// Name:        PtermPrefDialog.cpp
// Purpose:     Implementation of Preferences dialog
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#include "PtermPrefDialog.h"
#include "PtermConnDialog.h"

const wxString radioChoices[MAXMCHOICES] = {_("None"), _("2"), _("3"), _("4"), _("Dev") };

// ----------------------------------------------------------------------------
// PtermPrefDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (PtermPrefDialog, wxDialog)
    EVT_CLOSE (PtermPrefDialog::OnClose)
    EVT_BUTTON (wxID_ANY, PtermPrefDialog::OnButton)
    EVT_CHECKBOX (wxID_ANY, PtermPrefDialog::OnCheckbox)
    EVT_RADIOBOX(wxID_ANY, PtermPrefDialog::OnRadiobox)
    EVT_LISTBOX (wxID_ANY, PtermPrefDialog::OnSelect)
    EVT_LISTBOX_DCLICK (wxID_ANY, PtermPrefDialog::OnDoubleClick)
    EVT_TEXT (wxID_ANY, PtermPrefDialog::OnChange)
    EVT_COMBOBOX (wxID_ANY, PtermPrefDialog::OnComboSelect)
    END_EVENT_TABLE ();

PtermPrefDialog::PtermPrefDialog (PtermFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxSize size)
    : wxDialog (parent, id, title, pos, size)
{

    // static ui objects, note dynamic controls, e.g. those that hold values or require event processing are declared above
//  wxNotebook* tabPrefsDialog;
    //tab0
    wxScrolledWindow* tab0;
    wxStaticText* lblExplain0;
//  wxListBox* lstProfiles;
    wxStaticText* lblProfileActionExplain;
//  wxButton* btnSave;
//  wxButton* btnLoad;
//  wxButton* btnDelete;
//  wxStaticText* lblProfileStatusMessage;
    wxStaticText* lblNewProfileExplain;
    wxStaticText* lblNewProfile;
//  wxTextCtrl* txtProfile;
//  wxButton* btnAdd;
    //tab1
    wxScrolledWindow* tab3;
    wxStaticText* lblExplain3;
//  wxCheckBox* chkConnectAtStartup;
    wxStaticText* lblShellFirst;
//  wxTextCtrl* txtShellFirst;
    wxStaticText* lblDefaultHost;
//  wxTextCtrl* txtDefaultHost;
    wxStaticText* lblDefaultPort;
//  wxComboBox* cboDefaultPort;
    wxStaticText* lblExplainPort;
    //tab2
    wxScrolledWindow* tab1;
    wxStaticText* lblExplain1;
//  wxCheckBox* chkShowSignon;
//  wxCheckBox* chkShowSysName;
//  wxCheckBox* chkShowHost;
//  wxCheckBox* chkShowStation;
    //tab3
    wxScrolledWindow* tab2;
    wxStaticText* lblExplain2;
//  wxCheckBox* chkSimulate1200Baud;
//  wxCheckBox* chkEnableGSW;
//  wxCheckBox* chkEnableNumericKeyPad;
//  wxCheckBox* chkIgnoreCapLock;
//  wxCheckBox* chkUsePLATOKeyboard;
//  wxCheckBox* chkUseAccelerators;
//  wxCheckBox* chkEnableBeep;
//  wxCheckBox* chkDisableShiftSpace;
//  wxCheckBox* chkDisableMouseDrag;
    //tab4
    wxScrolledWindow* tab4;
    wxStaticText* lblExplain4;
//  wxCheckBox* chkZoom200;
//  wxCheckBox* chkStatusBar;
//  wxCheckBox* chkMenuBar;
//  wxCheckBox* chkDisableColor;
//  wxButton* btnFGColor; // windows
//  wxBitmapButton* btnFGColor; // other
    wxStaticText* lblFGColor;
//  wxButton* btnBGColor; // windows
//  wxBitmapButton* btnBGColor; // other
    wxStaticText* lblBGColor;
    wxStaticText* lblExplainColor;
    //tab5
    wxScrolledWindow* tab5;
    wxStaticText* lblExplain5;
    wxStaticText* lblCharDelay;
//  wxTextCtrl* txtCharDelay;
    wxStaticText* lblCharDelay2;
    wxStaticText* lblLineDelay;
//  wxTextCtrl* txtLineDelay;
    wxStaticText* lblLineDelay2;
    wxStaticText* lblAutoNewLine;
//  wxComboBox* cboAutoLF;
    wxStaticText* lblAutoNewLine2;
//  wxCheckBox* chkSmartPaste;
//  wxCheckBox* chkConvertDot7;
//  wxCheckBox* chkConvert8Spaces;
//  wxCheckBox* chkTutorColor;
    //tab6
    wxScrolledWindow* tab6;
    wxStaticText* lblEmail;
    wxStaticText* lblSearchURL;
    wxStaticText* lblFloppy0;
    wxStaticText* lblFloppy1;

//  wxTextCtrl* txtEmail;
//  wxTextCtrl* txtSearchURL;
    //button  bar
//  wxButton* btnOK;
//  wxButton* btnCancel;
//  wxButton* btnDefaults;
    wxFont dfont = wxFont (10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    

    
    // ui object creation / placement, note initialization of values is below
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer (wxVERTICAL);
    
    //notebook
    tabPrefsDialog = new wxNotebook (this, wxID_ANY, wxDefaultPosition,
                                     wxDefaultSize, wxNB_TOP);
    tabPrefsDialog->SetFont (dfont);

    //tab0
    tab0 = new wxScrolledWindow (tabPrefsDialog, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxHSCROLL | wxTAB_TRAVERSAL | wxVSCROLL);
    tab0->SetScrollRate (5, 5);
    tab0->SetFont (dfont);

    wxBoxSizer* page0;
    page0 = new wxBoxSizer (wxVERTICAL);
    lblExplain0 = new wxStaticText (tab0, wxID_ANY,
                                    _("Use this page to manage your connection preference profiles."),
                                    wxDefaultPosition, wxDefaultSize, 0);
    page0->Add (lblExplain0, 0, wxALL, 5);
    lstProfiles = new wxListBox (tab0, wxID_ANY, wxDefaultPosition,
                                 wxSize (-1, -1), 0, NULL, wxLB_SORT); 
    page0->Add (lstProfiles, 1, wxALL | wxEXPAND | wxALIGN_TOP, 5);

    wxFlexGridSizer* fgs01;
    fgs01 = new wxFlexGridSizer (4, 1, 0, 0);
    fgs01->SetFlexibleDirection (wxVERTICAL);
    lblProfileActionExplain = new wxStaticText (tab0, wxID_ANY,
                                                _("Select a profile above, then click a button below."),
                                                wxDefaultPosition,
                                                wxDefaultSize, 0);
    fgs01->Add (lblProfileActionExplain, 0, wxALL, 5);

    wxFlexGridSizer* fgs011;
    fgs011 = new wxFlexGridSizer (2, 4, 0, 0);
    btnSave = new wxButton (tab0, wxID_ANY, _("Save"), wxDefaultPosition,
                            wxDefaultSize, 0);
    btnSave->SetFont (dfont);
    fgs011->Add (btnSave, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
    btnLoad = new wxButton (tab0, wxID_ANY, _("Load"), wxDefaultPosition,
                            wxDefaultSize, 0);
    btnLoad->SetFont (dfont);
    fgs011->Add (btnLoad, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
    btnDelete = new wxButton (tab0, wxID_ANY, _("Delete"), wxDefaultPosition,
                              wxDefaultSize, 0);
    btnDelete->SetFont (dfont);
    fgs011->Add (btnDelete, 0, wxRIGHT | wxLEFT, 5);
    lblProfileStatusMessage = new wxStaticText (tab0, wxID_ANY,
                                                _("Profile saved."),
                                                wxDefaultPosition,
                                                wxDefaultSize, 0);
    lblProfileStatusMessage->SetFont (dfont);
    fgs011->Add (lblProfileStatusMessage, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    fgs01->Add (fgs011, 1, 0, 5);
    lblNewProfileExplain = new wxStaticText (tab0, wxID_ANY,
                                             _("Or, enter the name of a new profile below and click Add."),
                                             wxDefaultPosition, wxDefaultSize,
                                             0);
    fgs01->Add (lblNewProfileExplain, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    wxFlexGridSizer* fgs012;
    fgs012 = new wxFlexGridSizer (1, 3, 0, 0);
    fgs012->AddGrowableCol (1);
    fgs012->SetFlexibleDirection (wxHORIZONTAL);
    lblNewProfile = new wxStaticText (tab0, wxID_ANY, _("Profile"),
                                      wxDefaultPosition, wxDefaultSize, 0);
    fgs012->Add (lblNewProfile, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    txtProfile = new wxTextCtrl (tab0, wxID_ANY, wxT (""), wxDefaultPosition,
                                 wxDefaultSize, 0);
    txtProfile->SetMinSize (wxSize (310, -1));
    fgs012->Add (txtProfile, 1, wxALL | wxEXPAND, 5);
    btnAdd = new wxButton (tab0, wxID_ANY, _("Add"), wxDefaultPosition,
                           wxDefaultSize, wxBU_EXACTFIT);
    fgs012->Add (btnAdd, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    fgs01->Add (fgs012, 1, wxEXPAND | wxALIGN_TOP, 5);
    page0->Add (fgs01, 0, wxEXPAND, 5);
    tab0->SetSizer (page0);
    tab0->Layout ();
    page0->Fit (tab0);
    tabPrefsDialog->AddPage (tab0, _("Profiles"), true);

    //tab1
    tab1 = new wxScrolledWindow (tabPrefsDialog, wxID_ANY, wxDefaultPosition,
                                 wxDefaultSize,
                                 wxHSCROLL | wxTAB_TRAVERSAL | wxVSCROLL);
    tab1->SetScrollRate (5, 5);

    wxFlexGridSizer* page1;
    page1 = new wxFlexGridSizer (3, 1, 0, 0);
    page1->AddGrowableCol (0);
    page1->AddGrowableRow (1);
    page1->SetFlexibleDirection (wxVERTICAL);
    lblExplain1 = new wxStaticText (tab1, wxID_ANY,
                                    _("Settings on this page specify where PLATO is on the internet."),
                                    wxDefaultPosition, wxDefaultSize, 0);
    page1->Add (lblExplain1, 1, wxALL, 5);

    wxBoxSizer* bs11;
    bs11 = new wxBoxSizer (wxVERTICAL);
    chkConnectAtStartup = new wxCheckBox (tab1, wxID_ANY,
                                          _("Connect at startup"),
                                          wxDefaultPosition, wxDefaultSize, 0);
    chkConnectAtStartup->SetValue (true);
    bs11->Add (chkConnectAtStartup, 0, wxTOP | wxLEFT, 5);

    wxFlexGridSizer* fgs111;
    fgs111 = new wxFlexGridSizer (3, 2, 0, 0);
    fgs111->AddGrowableCol (1);
    fgs111->SetNonFlexibleGrowMode (wxFLEX_GROWMODE_SPECIFIED);
    lblShellFirst = new wxStaticText (tab1, wxID_ANY, _("Run this first"),
                                      wxDefaultPosition, wxDefaultSize, 0);
    lblShellFirst->SetFont (dfont);
    fgs111->Add (lblShellFirst, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    txtShellFirst = new wxTextCtrl (tab1, wxID_ANY, wxT (""),
                                    wxDefaultPosition, wxDefaultSize, 0);
    txtShellFirst->SetFont (dfont);
    txtShellFirst->SetMaxLength (255); 
    fgs111->Add (txtShellFirst, 0,
                 wxALIGN_CENTER_VERTICAL | wxALL, 5);
    lblDefaultHost = new wxStaticText (tab1, wxID_ANY, _("Default Host"),
                                       wxDefaultPosition, wxDefaultSize, 0);
    fgs111->Add (lblDefaultHost, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    txtDefaultHost = new wxTextCtrl (tab1, wxID_ANY, wxT ("cyberserv.org"),
                                     wxDefaultPosition, wxSize (-1, -1), 0);
    txtDefaultHost->SetMaxLength (100); 
    txtDefaultHost->SetFont (dfont);
    fgs111->Add (txtDefaultHost, 1, 
                 wxALL | wxALIGN_CENTER_VERTICAL, 5);
    lblDefaultPort = new wxStaticText (tab1, wxID_ANY, _("Default Port*"),
                                       wxDefaultPosition, wxDefaultSize, 0);
    fgs111->Add (lblDefaultPort, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxIntegerValidator<long> portval;
    portval.SetRange (1, 65535);
    
    cboDefaultPort = new wxComboBox (tab1, wxID_ANY, wxT ("5004"),
                                     wxDefaultPosition, wxSize (75, -1),
                                     0, NULL, 0, portval);
    cboDefaultPort->Append (wxT ("5004"));
    cboDefaultPort->Append (wxT ("8005"));
    fgs111->Add (cboDefaultPort, 0, wxALL, 5);
    bs11->Add (fgs111, 1, wxEXPAND | wxALIGN_TOP, 5);
    page1->Add (bs11, 1, wxEXPAND | wxALIGN_TOP, 0);
    lblExplainPort = new wxStaticText (tab1, wxID_ANY,
                                       _("* NOTE: 5004=Classic, 8005=Color Terminal"),
                                       wxDefaultPosition, wxDefaultSize, 0);
    page1->Add (lblExplainPort, 0, wxALL, 5);
    tab1->SetSizer (page1);
    tab1->Layout ();
    page1->Fit (tab1);
    tabPrefsDialog->AddPage (tab1, _("Connection"), true);

    //tab2
    tab2 = new wxScrolledWindow (tabPrefsDialog, wxID_ANY, wxDefaultPosition,
                                 wxDefaultSize,
                                 wxHSCROLL | wxTAB_TRAVERSAL | wxVSCROLL);
    tab2->SetScrollRate (5, 5);
    tab2->SetFont (dfont);

    wxBoxSizer* page2;
    page2 = new wxBoxSizer (wxVERTICAL);
    lblExplain2 = new wxStaticText (tab2, wxID_ANY,
                                    _("Settings on this page configure the text shown in the window title."),
                                    wxDefaultPosition, wxDefaultSize, 0);
    page2->Add (lblExplain2, 0, wxALL, 5);
    chkShowSignon = new wxCheckBox (tab2, wxID_ANY,
                                    _("Show name/group in frame title"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    chkShowSignon->SetValue (true);
    page2->Add (chkShowSignon, 0, wxALL, 5);
    chkShowSysName = new wxCheckBox (tab2, wxID_ANY,
                                     _("Show system name in frame title"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    chkShowSysName->SetValue (true);
    page2->Add (chkShowSysName, 0, wxALL, 5);
    chkShowHost = new wxCheckBox (tab2, wxID_ANY,
                                  _("Show host name in frame title"),
                                  wxDefaultPosition, wxDefaultSize, 0);
    chkShowHost->SetValue (true);
    page2->Add (chkShowHost, 0, wxALL, 5);
    chkShowStation = new wxCheckBox (tab2, wxID_ANY,
                                     _("Show site-station numbers in frame title"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    chkShowStation->SetValue (true);
    page2->Add (chkShowStation, 0, wxALL, 5);
    tab2->SetSizer (page2);
    tab2->Layout ();
    page2->Fit (tab2);
    tabPrefsDialog->AddPage (tab2, _("Title"), false);

    //tab3
    tab3 = new wxScrolledWindow (tabPrefsDialog, wxID_ANY, wxDefaultPosition,
                                 wxDefaultSize, 
                                 wxHSCROLL | wxTAB_TRAVERSAL | wxVSCROLL);
    tab3->SetScrollRate (5, 5);

    wxBoxSizer* page3;
    page3 = new wxBoxSizer (wxVERTICAL);
    lblExplain3 = new wxStaticText (tab3, wxID_ANY,
                                    _("Settings on this page let you fine-tune your PLATO experience."),
                                    wxDefaultPosition, wxDefaultSize, 0);
    page3->Add (lblExplain3, 0, wxALL, 5);
    chkSimulate1200Baud = new wxCheckBox (tab3, wxID_ANY,
                                          _("Simulate 1260 baud"),
                                          wxDefaultPosition, wxDefaultSize, 0);
    page3->Add (chkSimulate1200Baud, 0, wxALL, 5);
    chkEnableGSW = new wxCheckBox (tab3, wxID_ANY,
                                   _("Enable GSW (not in ASCII)"),
                                   wxDefaultPosition, wxDefaultSize, 0);
    chkEnableGSW->SetValue (true);
    page3->Add (chkEnableGSW, 0, wxALL, 5);
    chkEnableNumericKeyPad = new wxCheckBox (tab3, wxID_ANY,
                                             _("Enable numeric keypad for arrow operation"),
                                             wxDefaultPosition,
                                             wxDefaultSize, 0);
    chkEnableNumericKeyPad->SetValue (true);
    page3->Add (chkEnableNumericKeyPad, 0, wxALL, 5);
    chkIgnoreCapLock = new wxCheckBox (tab3, wxID_ANY, _("Ignore CAPS LOCK"),
                                       wxDefaultPosition, wxDefaultSize, 0);
    chkIgnoreCapLock->SetValue (true);
    page3->Add (chkIgnoreCapLock, 0, wxALL, 5);
    chkUsePLATOKeyboard = new wxCheckBox (tab3, wxID_ANY,
                                          _("Use real PLATO keyboard"),
                                          wxDefaultPosition, wxDefaultSize, 0);
    page3->Add (chkUsePLATOKeyboard, 0, wxALL, 5);
    chkUseAccelerators = new wxCheckBox (tab3, wxID_ANY,
                                         _("Enable control-key menu accelerators"),
                                         wxDefaultPosition, wxDefaultSize, 0);
    chkUseAccelerators->SetValue (true);
    chkUseAccelerators->SetFont (dfont);
#if defined (__WXMAC__)
    chkUseAccelerators->Disable ();
#endif
    page3->Add (chkUseAccelerators, 0, wxALL, 5);
    chkEnableBeep = new wxCheckBox (tab3, wxID_ANY, _("Enable -beep-"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    chkEnableBeep->SetValue (true);
    page3->Add (chkEnableBeep, 0, wxALL, 5);
    chkDisableShiftSpace = new wxCheckBox (tab3, wxID_ANY,
                                           _("Disable Shift-Space (backspace via Ctrl-Space)"),
                                           wxDefaultPosition, wxDefaultSize, 0);
    chkDisableShiftSpace->SetValue (false);
    page3->Add (chkDisableShiftSpace, 0, wxALL, 5);
    chkDisableMouseDrag = new wxCheckBox (tab3, wxID_ANY,
                                          _("Disable mouse drag (select)"),
                                          wxDefaultPosition, wxDefaultSize, 0);
    chkDisableMouseDrag->SetValue (false);
    page3->Add (chkDisableMouseDrag, 0, wxALL, 5);
    tab3->SetSizer (page3);
    tab3->Layout ();
    page3->Fit (tab3);
    tabPrefsDialog->AddPage (tab3, _("Emulation"), false);

    //tab4
    tab4 = new wxScrolledWindow (tabPrefsDialog, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxHSCROLL | wxTAB_TRAVERSAL | wxVSCROLL);
    tab4->SetScrollRate (5, 5);

    wxFlexGridSizer* page4;
    page4 = new wxFlexGridSizer (3, 1, 0, 0);
    page4->AddGrowableCol (0);
    page4->AddGrowableRow (1);
    page4->SetFlexibleDirection (wxVERTICAL);
    lblExplain4 = new wxStaticText (tab4, wxID_ANY,
                                    _("Settings on this page allow you to change the display appearance."),
                                    wxDefaultPosition, wxDefaultSize, 0);
    page4->Add (lblExplain4, 0, wxALL, 5);

    wxBoxSizer* bs41;
    bs41 = new wxBoxSizer (wxVERTICAL);
    chkDisableColor = new wxCheckBox (tab4, wxID_ANY,
                                      _("Disable -color- (ASCII mode)"),
                                      wxDefaultPosition, wxDefaultSize, 0);
    bs41->Add (chkDisableColor, 0, wxALL, 5);

    wxFlexGridSizer* fgs411;
    fgs411 = new wxFlexGridSizer (2, 2, 0, 0);
#if defined (_WIN32)
    btnFGColor = new wxButton (tab4, wxID_ANY, wxT (""),
                               wxDefaultPosition, wxSize (25, -1), 0);
    btnFGColor->SetBackgroundColour (wxColour (255, 128, 0));
    fgs411->Add (btnFGColor, 0, wxALL, 5);
    lblFGColor = new wxStaticText (tab4, wxID_ANY, _("Foreground color*"),
                                   wxDefaultPosition, wxDefaultSize, 0);
    fgs411->Add (lblFGColor, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    btnBGColor = new wxButton (tab4, wxID_ANY, wxT (""), wxDefaultPosition,
                               wxSize (25, -1), 0);
    btnBGColor->SetBackgroundColour (wxColour (0, 0, 0));
    fgs411->Add (btnBGColor, 0, wxALL, 5);
    lblBGColor = new wxStaticText (tab4, wxID_ANY, _("Background color*"),
                                   wxDefaultPosition, wxDefaultSize, 0);
    fgs411->Add (lblBGColor, 0,
                 wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,
                 5);
#else
    btnFGColor = new wxBitmapButton (tab4, wxID_ANY, wxNullBitmap,
                                     wxDefaultPosition, wxDefaultSize,
                                     wxBU_AUTODRAW);
    fgs411->Add (btnFGColor, 0, wxALL, 5);
    lblFGColor = new wxStaticText (tab4, wxID_ANY, _("Foreground color*"),
                                   wxDefaultPosition, wxDefaultSize, 0);
    fgs411->Add (lblFGColor, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    btnBGColor = new wxBitmapButton (tab4, wxID_ANY, wxNullBitmap,
                                     wxDefaultPosition, wxDefaultSize,
                                     wxBU_AUTODRAW);
    fgs411->Add (btnBGColor, 0, wxALL, 5);
    lblBGColor = new wxStaticText (tab4, wxID_ANY, _("Background color*"),
                                   wxDefaultPosition, wxDefaultSize, 0);
    fgs411->Add (lblBGColor, 0,
                 wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,
                 5);
#endif
    bs41->Add (fgs411, 1, 0, 5);
    page4->Add (bs41, 1, wxEXPAND | wxALIGN_TOP, 5);
    lblExplainColor = new wxStaticText (tab4, wxID_ANY, 
                                        _("* NOTE: Applied in Classic mode or if -color- is disabled in ASCII mode"),
                                        wxDefaultPosition, wxDefaultSize, 0);
    page4->Add (lblExplainColor, 0, wxALL | wxALIGN_BOTTOM, 5);
    tab4->SetSizer (page4);
    tab4->Layout ();
    page4->Fit (tab4);
    tabPrefsDialog->AddPage (tab4, _("Display"), false);

    //tab5
    tab5 = new wxScrolledWindow (tabPrefsDialog, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxHSCROLL | wxTAB_TRAVERSAL | wxVSCROLL);
    tab5->SetScrollRate (5, 5);

    wxFlexGridSizer* page5;
    page5 = new wxFlexGridSizer (2, 1, 0, 0);
    page5->AddGrowableCol (0);
    page5->AddGrowableRow (0);
    page5->SetFlexibleDirection (wxVERTICAL);

    wxBoxSizer* bs51;
    bs51 = new wxBoxSizer (wxVERTICAL);
    lblExplain5 = new wxStaticText (tab5, wxID_ANY,
                                    _("Settings on this page allow you to specify Paste options."),
                                    wxDefaultPosition, wxDefaultSize, 0);
    bs51->Add (lblExplain5, 0, wxALL, 5);

    wxFlexGridSizer* fgs511;
    wxIntegerValidator<long> cdval, ldval, autoval;

    fgs511 = new wxFlexGridSizer (2, 3, 0, 0);
    // We don't really want the delays to be as low as 1 ms, but if the
    // lower bound is more than that, you can't enter a value starting from
    // a blank field because that would be less than 10...
    cdval.SetRange (1, 1000);
    ldval.SetRange (1, 1000);
    autoval.SetRange (0, 120);
    lblCharDelay = new wxStaticText (tab5, wxID_ANY, _("Delay between chars"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    fgs511->Add (lblCharDelay, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    txtCharDelay = new wxTextCtrl (tab5, wxID_ANY, wxT ("50"),
                                   wxDefaultPosition, wxSize (48, -1), 0,
                                   cdval);
    txtCharDelay->SetMaxLength (3); 
    fgs511->Add (txtCharDelay, 0, wxALL, 5);
    lblCharDelay2 = new wxStaticText (tab5, wxID_ANY, _("milliseconds"),
                                      wxDefaultPosition, wxDefaultSize, 0);
    fgs511->Add (lblCharDelay2, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    lblLineDelay = new wxStaticText (tab5, wxID_ANY,
                                     _("Delay after end of line"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    fgs511->Add (lblLineDelay, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    txtLineDelay = new wxTextCtrl (tab5, wxID_ANY, wxT ("100"),
                                   wxDefaultPosition, wxSize (48, -1), 0,
                                   ldval);
    txtLineDelay->SetMaxLength (3); 
    fgs511->Add (txtLineDelay, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    lblLineDelay2 = new wxStaticText (tab5, wxID_ANY, _("milliseconds"),
                                      wxDefaultPosition, wxDefaultSize, 0);
    fgs511->Add (lblLineDelay2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bs51->Add (fgs511, 1, wxEXPAND, 5);

    wxFlexGridSizer* fgs512;
    fgs512 = new wxFlexGridSizer (2, 3, 0, 0);
    lblAutoNewLine = new wxStaticText (tab5, wxID_ANY,
                                       _("Automatic new line every"),
                                       wxDefaultPosition, wxDefaultSize, 0);
    fgs512->Add (lblAutoNewLine, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    cboAutoLF = new wxComboBox (tab5, wxID_ANY, wxT ("60"), wxDefaultPosition,
                                wxSize (-1, -1), 0, NULL, 0 | wxTAB_TRAVERSAL,
                                autoval);
    cboAutoLF->Append (wxT ("0"));
    cboAutoLF->Append (wxT ("60"));
    cboAutoLF->Append (wxT ("120"));
    cboAutoLF->SetMinSize (wxSize (65, -1));
    fgs512->Add (cboAutoLF, 1, wxALL, 5);
    lblAutoNewLine2 = new wxStaticText (tab5, wxID_ANY, _("characters"),
                                        wxDefaultPosition, wxDefaultSize, 0);
    fgs512->Add (lblAutoNewLine2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    bs51->Add (fgs512, 0, wxEXPAND, 5);
    chkSmartPaste = new wxCheckBox (tab5, wxID_ANY,
                                    _("Use TUTOR pasting (certain sequences are treated specially)"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    chkSmartPaste->SetFont (dfont);
    bs51->Add (chkSmartPaste, 0, wxALL, 5);
    chkConvertDot7 = new wxCheckBox (tab5, wxID_ANY,
                                     _("Convert periods followed by 7 spaces into period/tab"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    chkConvertDot7->SetValue (true);
    //chkConvertDot7->Hide ();
    bs51->Add (chkConvertDot7, 0, wxALL, 5);
    chkConvert8Spaces = new wxCheckBox (tab5, wxID_ANY,
                                        _("Convert 8 consecutive spaces into a tab"),
                                        wxDefaultPosition, wxDefaultSize, 0);
    chkConvert8Spaces->SetValue (true);
    //chkConvert8Spaces->Hide ();
    bs51->Add (chkConvert8Spaces, 0, wxALL, 5);
    chkTutorColor = new wxCheckBox (tab5, wxID_ANY,
                                    _("Display TUTOR colorization options on Edit/Context menus"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    chkTutorColor->SetValue (true);
    //chkTutorColor->Hide ();
    bs51->Add (chkTutorColor, 0, wxALL, 5);
    page5->Add (bs51, 1, wxEXPAND, 5);
    tab5->SetSizer (page5);
    tab5->Layout ();
    page5->Fit (tab5);
    tabPrefsDialog->AddPage (tab5, _("Pasting"), false);

    //tab6
    tab6 = new wxScrolledWindow (tabPrefsDialog, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxHSCROLL | wxTAB_TRAVERSAL | wxVSCROLL);

    tab6->SetScrollRate (5, 5);
    tab6->Hide ();

    wxFlexGridSizer* page6;
    page6 = new wxFlexGridSizer (0, 1, 0, 0);
    page6->AddGrowableCol (0);
    page6->SetFlexibleDirection (wxBOTH);
    lblEmail = new wxStaticText (tab6, wxID_ANY,
                                 _("Command line for menu option 'Mail to...' (%s=address)"),
                                 wxDefaultPosition, wxDefaultSize, 0);
    page6->Add (lblEmail, 0, wxALL | wxALIGN_BOTTOM, 5);
    txtEmail = new wxTextCtrl (tab6, wxID_ANY, wxT (""), wxDefaultPosition,
                               wxDefaultSize, 0 | wxTAB_TRAVERSAL);
    txtEmail->SetMaxLength (255); 
    txtEmail->SetFont (dfont);
    page6->Add (txtEmail, 0, wxALL | wxEXPAND, 5);
    lblSearchURL = new wxStaticText (tab6, wxID_ANY,
                                     _("Specify URL for menu option 'Search this...'"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    page6->Add (lblSearchURL, 0, wxALL, 5);


    txtSearchURL = new wxTextCtrl (tab6, wxID_ANY, wxT (""), wxDefaultPosition,
                                   wxDefaultSize, 0 | wxTAB_TRAVERSAL);
    txtSearchURL->SetMaxLength (255); 
    page6->Add (txtSearchURL, 0, wxALL | wxEXPAND, 5);

    chkMTutorBoot = new wxCheckBox(tab6, wxID_ANY,
        _("Boot to MicroTutor"),
        wxDefaultPosition, wxDefaultSize, 0);
    chkMTutorBoot->SetValue(false);
    page6->Add(chkMTutorBoot, 0, wxALL, 5);
    
    radMTutor = new wxRadioBox(tab6, wxID_ANY,
        _("MicroTutor Level"),
        wxDefaultPosition, wxDefaultSize, MAXMCHOICES, radioChoices, MAXMCHOICES, wxRA_SPECIFY_COLS);
    radMTutor->SetSelection(LevelToSelect(DEFAULTMLEVEL));
    page6->Add(radMTutor, 0, wxALL, 5);

    chkFloppy0 = new wxCheckBox(tab6, wxID_ANY,
        _("Enable Floppy 0"),
        wxDefaultPosition, wxDefaultSize, 0);
    chkFloppy0->SetValue(true);
    page6->Add(chkFloppy0, 0, wxALL, 5);

    chkFloppy1 = new wxCheckBox(tab6, wxID_ANY,
        _("Enable Floppy 1"),
        wxDefaultPosition, wxDefaultSize, 0);
    chkFloppy1->SetValue(true);
    page6->Add(chkFloppy1, 0, wxALL, 5);


    lblFloppy0 = new wxStaticText(tab6, wxID_ANY,
                                  _("Floppy 0 file:"),
                                  wxDefaultPosition, wxDefaultSize, 0);
    page6->Add(lblFloppy0, 0, wxALL, 5);

    txtFloppy0 = new wxStaticText(tab6, wxID_ANY, wxT(""), wxDefaultPosition,
                                  wxDefaultSize, 0);
    page6->Add(txtFloppy0, 0, wxLEFT, 25);

    btnFloppy0 = new wxButton(tab6, wxID_ANY, _("Select Floppy 0 File"),
        wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    page6->Add(btnFloppy0, 0, wxALL, 5);

    lblFloppy1 = new wxStaticText(tab6, wxID_ANY,
                                  _("Floppy 1 file:"),
                                  wxDefaultPosition, wxDefaultSize, 0);
    page6->Add(lblFloppy1, 0, wxALL, 5);

    txtFloppy1 = new wxStaticText(tab6, wxID_ANY, wxT(""), wxDefaultPosition,
                                  wxDefaultSize, 0);
    page6->Add(txtFloppy1, 0, wxLEFT, 25);

    btnFloppy1 = new wxButton(tab6, wxID_ANY, _("Select Floppy 1 File"),
        wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    page6->Add(btnFloppy1, 0, wxALL, 5);


    tab6->SetSizer (page6);
    tab6->Layout ();
    page6->Fit (tab6);
    tabPrefsDialog->AddPage (tab6, _("Local"), false);

    //notebook
    bSizer1->Add (tabPrefsDialog, 1, wxEXPAND | wxTOP | wxRIGHT | wxLEFT, 6);

    //button bar
    wxFlexGridSizer* fgsButtons;
    fgsButtons = new wxFlexGridSizer (1, 5, 0, 0);
    fgsButtons->AddGrowableCol (0);
    fgsButtons->AddGrowableRow (0);
    fgsButtons->SetFlexibleDirection (wxHORIZONTAL);
    fgsButtons->Add (0, 0, 1, wxALL | wxEXPAND, 5);
    btnOK = new wxButton (this, wxID_ANY, _("OK"), wxDefaultPosition,
                          wxDefaultSize, 0);
    btnOK->SetFont (dfont);
    fgsButtons->Add (btnOK, 0, wxALL, 5);
    btnCancel = new wxButton (this, wxID_CANCEL, _("Cancel"),
                              wxDefaultPosition, wxDefaultSize, 0);
    btnCancel->SetFont (dfont);
    fgsButtons->Add (btnCancel, 0, wxALL, 5);
    btnDefaults = new wxButton (this, wxID_ANY, _("Defaults"),
                                wxDefaultPosition, wxDefaultSize, 0);
    btnDefaults->SetFont (dfont);
    fgsButtons->Add (btnDefaults, 0, wxALL, 5);
    bSizer1->Add (fgsButtons, 0, wxEXPAND, 5);
    this->SetSizer (bSizer1);
    this->Layout ();

    // set object value properties
    SetControlState ();

    //set active tab
    m_lastTab = ptermApp->m_config->Read (wxT (PREF_LASTTAB), 0L);
    tabPrefsDialog->SetSelection (m_lastTab);

    //button bar
    btnOK->SetDefault ();
}

void PtermPrefDialog::OnClose (wxCloseEvent& event)
{
    EndModal (wxID_CANCEL);
}

bool PtermPrefDialog::ValidProfile (wxString profile)
{
    wxString validchr = wxT ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_. ()");
    unsigned int cnt;
    for (cnt=0; cnt<profile.Len (); cnt++)
        if (!validchr.Contains (profile.Mid (cnt, 1)))
            return false;
    return true;
}

bool PtermPrefDialog::SaveProfile (wxString profile)
{
    wxString filename;
    wxString buffer;
    bool openok;

    //open file
    filename = ptermApp->ProfileFileName (profile);
    wxTextFile file (filename);
    if (file.Exists ())
        openok = file.Open ();
    else
        openok = file.Create ();
    if (!openok)
        return false;
    file.Clear ();

    //write prefs
    //tab0
    buffer.Printf (wxT (PREF_CURPROFILE) wxT ("=%s"), profile);
    file.AddLine (buffer);
    //tab1
    buffer.Printf (wxT (PREF_CONNECT) wxT ("=%d"), (m_connect) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHELLFIRST) wxT ("=%s"), m_ShellFirst);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_HOST) wxT ("=%s"), m_host);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_PORT) wxT ("=%ld"), m_port);
    file.AddLine (buffer);
    //tab2
    buffer.Printf (wxT (PREF_SHOWSIGNON) wxT ("=%d"), (m_showSignon) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHOWSYSNAME) wxT ("=%d"), (m_showSysName) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHOWHOST) wxT ("=%d"), (m_showHost) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHOWSTATION) wxT ("=%d"), (m_showStation) ? 1 : 0);
    file.AddLine (buffer);
    //tab3
    buffer.Printf (wxT (PREF_1200BAUD) wxT ("=%d"), (m_classicSpeed) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_GSW) wxT ("=%d"), (m_gswEnable) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_ARROWS) wxT ("=%d"), (m_numpadArrows) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_IGNORECAP) wxT ("=%d"), (m_ignoreCapLock) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_PLATOKB) wxT ("=%d"), (m_platoKb) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_ACCEL) wxT ("=%d"), (m_useAccel) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_BEEP) wxT ("=%d"), (m_beepEnable) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SHIFTSPACE) wxT ("=%d"), (m_DisableShiftSpace) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_MOUSEDRAG) wxT ("=%d"), (m_DisableMouseDrag) ? 1 : 0);
    file.AddLine (buffer);
    //tab4
    buffer.Printf (wxT (PREF_SCALE) wxT ("=%f"), m_scale);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_STATUSBAR) wxT ("=%d"), (m_showStatusBar) ? 1 : 0);
    file.AddLine (buffer);
#if !defined (__WXMAC__)
    buffer.Printf (wxT (PREF_MENUBAR) wxT ("=%d"), (m_showMenuBar) ? 1 : 0);
    file.AddLine (buffer);
#endif
    buffer.Printf (wxT (PREF_NOCOLOR) wxT ("=%d"), (m_noColor) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_FOREGROUND) wxT ("=%d %d %d"), m_fgColor.Red (), m_fgColor.Green (), m_fgColor.Blue ());
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_BACKGROUND) wxT ("=%d %d %d"), m_bgColor.Red (), m_bgColor.Green (), m_bgColor.Blue ());
    file.AddLine (buffer);
    //tab5
    buffer.Printf (wxT (PREF_CHARDELAY) wxT ("=%ld"), m_charDelay);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_LINEDELAY) wxT ("=%ld"), m_lineDelay);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_AUTOLF) wxT ("=%ld"), m_autoLF);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SMARTPASTE) wxT ("=%d"), (m_smartPaste) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_CONVDOT7) wxT ("=%d"), (m_convDot7) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_CONV8SP) wxT ("=%d"), (m_conv8Sp) ? 1 : 0);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_TUTORCOLOR) wxT ("=%d"), (m_TutorColor) ? 1 : 0);
    file.AddLine (buffer);
    //tab6
    buffer.Printf (wxT (PREF_EMAIL) wxT ("=%s"), m_Email);
    file.AddLine (buffer);
    buffer.Printf (wxT (PREF_SEARCHURL) wxT ("=%s"), m_SearchURL);
    file.AddLine (buffer);
    buffer.Printf(wxT(PREF_MTUTORLEVEL) wxT("=%d"), m_mTutorLevel);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_MTUTORBOOT) wxT("=%d"), (m_mTutorBoot) ? 1 : 0);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY0M) wxT("=%d"), (m_floppy0) ? 1 : 0);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY1M) wxT("=%d"), (m_floppy1) ? 1 : 0);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY0NAM) wxT("=%s"), m_floppy0File);
    file.AddLine(buffer);
    buffer.Printf(wxT(PREF_FLOPPY1NAM) wxT("=%s"), m_floppy1File);
    file.AddLine(buffer);


    //write to disk
    file.Write ();
    file.Close ();

    return true;

}

int PtermPrefDialog::SelectToLevel(int select)
{
    switch (select)
    {
    case 1: return 2;
    case 2: return 3;
    case 3: return 4;
    case 4: return 5;
    default: return 0;
    }
}

int PtermPrefDialog::LevelToSelect(int level)
{
    switch (level)
    {
    case 2: return 1;
    case 3: return 2;
    case 4: return 3;
    case 5: return 4;
    default: return 0;
    }
}

bool PtermPrefDialog::DeleteProfile (wxString profile)
{
    wxString filename;

    //delete file
    filename = ptermApp->ProfileFileName (profile);
    if (wxFileExists (filename))
    {
        wxRemoveFile (filename);
        return true;
    }
    return false;
}

void PtermPrefDialog::SetControlState (void)
{
    wxString ws;
#if !defined (_WIN32)
    wxBitmap fgBitmap (15, 15);
    wxBitmap bgBitmap (15, 15);
#endif

    //tab0
    m_curProfile = ptermApp->m_curProfile;
    //tab1
    m_ShellFirst = ptermApp->m_ShellFirst;
    m_connect = ptermApp->m_connect;
    m_host = ptermApp->m_hostName;
    m_port = ptermApp->m_port;
    //tab2
    m_showSignon = ptermApp->m_showSignon;
    m_showSysName = ptermApp->m_showSysName;
    m_showHost = ptermApp->m_showHost;
    m_showStation = ptermApp->m_showStation;
    //tab3
    m_classicSpeed = ptermApp->m_classicSpeed;
    m_gswEnable = ptermApp->m_gswEnable;
    m_numpadArrows = ptermApp->m_numpadArrows;
    m_ignoreCapLock = ptermApp->m_ignoreCapLock;
    m_platoKb = ptermApp->m_platoKb;
    m_useAccel = ptermApp->m_useAccel;
    m_beepEnable = ptermApp->m_beepEnable;
    m_DisableShiftSpace = ptermApp->m_DisableShiftSpace;
    m_DisableMouseDrag = ptermApp->m_DisableMouseDrag;
    //tab4
    m_scale = ptermApp->m_scale;
    m_showStatusBar = ptermApp->m_showStatusBar;
#if !defined (__WXMAC__)
    m_showMenuBar = ptermApp->m_showMenuBar;
#endif
    m_noColor = ptermApp->m_noColor;
    //m_fgColor = ptermApp->m_fgColor;
    //m_bgColor = ptermApp->m_bgColor;
    int r, g, b;
    wxString rgb;
    ptermApp->m_config->Read (wxT (PREF_FOREGROUND), &rgb, wxT ("255 144 0")); // 255 144 0 is RGB for Plato Orange
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_fgColor = wxColour (r, g, b);
    ptermApp->m_config->Read (wxT (PREF_BACKGROUND), &rgb, wxT ("0 0 0"));
    sscanf (rgb.mb_str (), "%d %d %d", &r, &g, &b);
    m_bgColor = wxColour (r, g, b);
    //tab5
    m_charDelay = ptermApp->m_charDelay;
    m_lineDelay = ptermApp->m_lineDelay;
    m_autoLF = ptermApp->m_autoLF;
    m_smartPaste = ptermApp->m_smartPaste;
    m_convDot7 = ptermApp->m_convDot7;
    m_conv8Sp = ptermApp->m_conv8Sp;
    m_TutorColor = ptermApp->m_TutorColor;
    //tab6
    m_Email = ptermApp->m_Email;
    m_SearchURL = ptermApp->m_SearchURL;
    m_mTutorBoot = ptermApp->m_mTutorBoot;
    m_mTutorLevel = ptermApp->m_mTutorLevel;

    m_floppy0 = ptermApp->m_floppy0;
    m_floppy1 = ptermApp->m_floppy1;
    m_floppy0File = ptermApp->m_floppy0File;
    m_floppy1File = ptermApp->m_floppy1File;

    //tab0
    wxDir ldir (wxGetCwd ());
    if (ldir.IsOpened ())
    {
        // populate listbox
        wxString filename;
        bool cont = ldir.GetFirst (&filename, wxT ("*.ppf"), wxDIR_DEFAULT);
        lstProfiles->Clear ();
        int i, cur = 0;
        wxString str;
        for (i = 0; cont; i++)
        {
            filename = filename.Left (filename.Len () - 4);
            lstProfiles->Append (filename);
            cont = ldir.GetNext (&filename);
        }
        for (i = 0; i < (int) lstProfiles->GetCount (); i++)
        {
            filename = lstProfiles->GetString (i);
            if (filename.CmpNoCase (ptermApp->m_curProfile) == 0)
                cur = i;
        }
        lstProfiles->Select (cur);
    }
    //tab1
    chkConnectAtStartup->SetValue (m_connect);
    txtShellFirst->SetValue (m_ShellFirst);
    txtDefaultHost->SetValue (m_host);
    ws.Printf ("%ld", m_port);
    cboDefaultPort->SetValue (ws);
    //tab2
    chkShowSignon->SetValue (m_showSignon);
    chkShowSysName->SetValue (m_showSysName);
    chkShowHost->SetValue (m_showHost);
    chkShowStation->SetValue (m_showStation);
    //tab3
    chkSimulate1200Baud->SetValue (m_classicSpeed);
    chkEnableGSW->SetValue (m_gswEnable);
    chkEnableNumericKeyPad->SetValue (m_numpadArrows);
    chkIgnoreCapLock->SetValue (m_ignoreCapLock);
    chkUsePLATOKeyboard->SetValue (m_platoKb);
    chkUseAccelerators->SetValue (m_useAccel);
    chkEnableBeep->SetValue (m_beepEnable);
    chkDisableShiftSpace->SetValue (m_DisableShiftSpace);
    chkDisableMouseDrag->SetValue (m_DisableMouseDrag);
    //tab4
    chkDisableColor->SetValue (m_noColor);
#if defined (_WIN32)
    btnFGColor->SetBackgroundColour (m_fgColor);
    btnBGColor->SetBackgroundColour (m_bgColor);
#else
    paintBitmap (fgBitmap, m_fgColor);
    btnFGColor->SetBitmapLabel (fgBitmap);
    paintBitmap (bgBitmap, m_bgColor);
    btnBGColor->SetBitmapLabel (bgBitmap);
#endif
    //tab5
    ws.Printf ("%ld", m_charDelay);
    txtCharDelay->SetValue (ws);
    ws.Printf ("%ld", m_lineDelay);
    txtLineDelay->SetValue (ws);
    ws.Printf ("%ld", m_autoLF);
    cboAutoLF->SetValue (ws);
    chkSmartPaste->SetValue (m_smartPaste);
    chkConvertDot7->SetValue (m_convDot7);
    chkConvert8Spaces->SetValue (m_conv8Sp);
    chkTutorColor->SetValue (m_TutorColor);
    //tab6
    txtEmail->SetValue (m_Email);
    txtSearchURL->SetValue (m_SearchURL);
    chkMTutorBoot->SetValue(m_mTutorBoot);
    radMTutor->SetSelection(LevelToSelect(m_mTutorLevel));
    chkFloppy0->SetValue(m_floppy0);
    chkFloppy1->SetValue(m_floppy1);
    txtFloppy0->SetLabel(m_floppy0File);
    txtFloppy1->SetLabel(m_floppy1File);

    //set button state
    wxString profile;
    profile = lstProfiles->GetStringSelection ();
    profile = txtProfile->GetLineText (0);
    btnAdd->Enable (!profile.IsEmpty ());
}

void PtermPrefDialog::OnButton (wxCommandEvent& event)
{
    wxBitmap fgBitmap (15, 15);
    wxBitmap bgBitmap (15, 15);
    wxString profile;
    wxString filename;
    wxString str;
    
    void OnButton (wxCommandEvent& event);
    lblProfileStatusMessage->SetLabel (wxT (""));
    if (event.GetEventObject () == btnSave)
    {
        profile = lstProfiles->GetStringSelection ();
        ptermApp->m_curProfile = profile;
        m_curProfile = profile;
        if (SaveProfile (profile))
        {
            ptermApp->LoadProfile (profile, wxT (""));
            SetControlState ();
            lblProfileStatusMessage->SetLabel (_("Profile saved."));
        }
        else
        {
            filename = ptermApp->ProfileFileName (profile);
            str.Printf (wxT ("Unable to save profile: %s"), filename);
            wxMessageBox (str, _("Error"), wxOK | wxICON_HAND);
        }
    }
    else if (event.GetEventObject () == btnLoad)
    {
        profile = lstProfiles->GetStringSelection ();
        ptermApp->m_curProfile = profile;
        m_curProfile = profile;
        if (ptermApp->LoadProfile (profile, wxT ("")))
        {
            SetControlState ();
            lblProfileStatusMessage->SetLabel (_("Profile loaded."));
        }
        else
        {
            filename = ptermApp->ProfileFileName (profile);
            str.Printf (wxT ("Unable to load profile: %s"), filename);
            wxMessageBox (str, _("Error"), wxOK | wxICON_HAND);
        }
    }
    else if (event.GetEventObject () == btnDelete)
    {
        profile = lstProfiles->GetStringSelection ();
        if (DeleteProfile (profile))
        {
            SetControlState ();
            lblProfileStatusMessage->SetLabel (_("Profile deleted."));
        }
        else
        {
            filename = ptermApp->ProfileFileName (profile);
            str.Printf (wxT ("Unable to delete profile: %s"), 
                       filename);
            wxMessageBox (str, _("Error"), wxOK | wxICON_HAND);
        }
    }
    else if (event.GetEventObject () == btnAdd)
    {
        profile = txtProfile->GetLineText (0);
        if (ValidProfile (profile))
        {
            if (SaveProfile (profile))
            {
                ptermApp->m_curProfile = profile;
                m_curProfile = profile;
                ptermApp->LoadProfile (profile, wxT (""));
                SetControlState ();
                lblProfileStatusMessage->SetLabel (_("Profile added."));
            }
            else
            {
                filename = ptermApp->ProfileFileName (profile);
                str.Printf (wxT ("Unable to add/save profile: %s"), 
                           filename);
                wxMessageBox (str, _("Error"), wxOK | wxICON_HAND);
            }
        }
        else
        {
            wxMessageBox (_("The profile name you entered contains illegal characters.\n\n"
                         "Valid characters are:\n\n"
                         "Standard: a-z, A-Z, 0-9\n"
                           "Special:  parentheses, dash, underscore, period, and space"), _("Problem"), wxOK | wxICON_EXCLAMATION);
        }
    }
    else if (event.GetEventObject() == btnFloppy0)
    {
        wxFileDialog
            openFileDialog(this, _("Open Floppy 0 file"), "", "",
                "MTU files (*.mtu)|*.mtu", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        MTFile testFile;

        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;     // the user changed idea...

        bool isOK = testFile.Test(openFileDialog.GetPath());

        if (!isOK)
        {
            wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
            return;
        }
        testFile.Close();
        m_floppy0File = openFileDialog.GetPath();
        txtFloppy0->SetLabel(m_floppy0File);
    }
    else if (event.GetEventObject() == btnFloppy1)
    {
        wxFileDialog
            openFileDialog(this, _("Open Floppy 1 file"), "", "",
                "MTU files (*.mtu)|*.mtu", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        MTFile testFile;
  
        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;     // the user changed idea...

        bool isOK = testFile.Test(openFileDialog.GetPath());

        if (!isOK)
        {
            wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
            return;
        }
        testFile.Close();
        m_floppy1File = openFileDialog.GetPath();
        txtFloppy1->SetLabel(m_floppy1File);
    }
    else if (event.GetEventObject () == btnFGColor)
    {
        m_fgColor = PtermApp::SelectColor (*this, _("Foreground"), m_fgColor);
#if defined (_WIN32)
        btnFGColor->SetBackgroundColour (m_fgColor);
#else
        paintBitmap (fgBitmap, m_fgColor);
        btnFGColor->SetBitmapLabel (fgBitmap);
#endif
    }
    else if (event.GetEventObject () == btnBGColor)
    {
        m_bgColor = PtermApp::SelectColor (*this, _("Background"), m_bgColor);
#if defined (_WIN32)
        btnBGColor->SetBackgroundColour (m_bgColor);
#else
        paintBitmap (bgBitmap, m_bgColor);
        btnBGColor->SetBitmapLabel (bgBitmap);
#endif
    }
    else if (event.GetEventObject () == btnOK)
    {
        //buttonbar
        m_lastTab = tabPrefsDialog->GetSelection ();
        EndModal (wxID_OK);
    }
    else if (event.GetEventObject () == btnCancel)
    {
        m_lastTab = tabPrefsDialog->GetSelection ();
        EndModal (wxID_CANCEL);
    }
    else if (event.GetEventObject () == btnDefaults)
    {
        wxString str;
        
        //reset variable values
        //tab0
        //tab1
        m_ShellFirst = wxT ("");
        m_connect = true;
        m_host = DEFAULTHOST;
        m_port = DefNiuPort;
        //tab2
        m_showSignon = false;
        m_showSysName = false;
        m_showHost = true;
        m_showStation = true;
        //tab3
        m_classicSpeed = false;
        m_gswEnable = true;
        m_numpadArrows = true;
        m_ignoreCapLock = false;
        m_platoKb = false;
#if defined (__WXMAC__)
        m_useAccel = true;
#else
        m_useAccel = false;
#endif
        m_beepEnable = true;
        m_DisableShiftSpace = false;
        m_DisableMouseDrag = false;
        //tab4
        m_scale = 1.0;
        m_showStatusBar = true;
#if !defined (__WXMAC__)
        m_showMenuBar = true;
#endif
        m_noColor = false;
        m_fgColor = wxColour (255, 144, 0);
        m_bgColor = *wxBLACK;
        //tab5
        m_charDelay = PASTE_CHARDELAY;
        m_lineDelay = PASTE_LINEDELAY;
        m_autoLF = 0;
        m_smartPaste = false;
        m_convDot7 = false;
        m_conv8Sp = false;
        m_TutorColor = false;
        //tab6
        m_Email = wxT ("");
        m_SearchURL = DEFAULTSEARCH;
        m_mTutorBoot = false;
        m_mTutorLevel = DEFAULTMLEVEL;

        //reset object values
        //tab0
        //tab1
        txtShellFirst->SetValue (m_ShellFirst);
        chkConnectAtStartup->SetValue (m_connect);
        txtDefaultHost->SetValue (m_host);
        str.Printf ("%ld", m_port);
        cboDefaultPort->SetValue (str);
        //tab2
        chkShowSignon->SetValue (m_showSignon);
        chkShowSysName->SetValue (m_showSysName);
        chkShowHost->SetValue (m_showHost);
        chkShowStation->SetValue (m_showStation);
        //tab3
        chkSimulate1200Baud->SetValue (m_classicSpeed);
        chkEnableGSW->SetValue (m_gswEnable);
        chkEnableNumericKeyPad->SetValue (m_numpadArrows);
        chkIgnoreCapLock->SetValue (m_ignoreCapLock);
        chkUsePLATOKeyboard->SetValue (m_platoKb);
        chkUseAccelerators->SetValue (m_useAccel);
        chkEnableBeep->SetValue (m_beepEnable);
        chkDisableShiftSpace->SetValue (m_DisableShiftSpace);
        chkDisableMouseDrag->SetValue (m_DisableMouseDrag);
        //tab4
        chkDisableColor->SetValue (m_noColor);
        btnFGColor->SetBackgroundColour (m_fgColor);
        btnBGColor->SetBackgroundColour (m_bgColor);
        //tab5
        str.Printf ("%ld", m_charDelay);
        txtCharDelay->SetValue (str);
        str.Printf ("%ld", m_lineDelay);
        txtLineDelay->SetValue (str);
        str.Printf ("%ld", m_autoLF);
        cboAutoLF->SetValue (str);
        chkSmartPaste->SetValue (m_smartPaste);
        chkConvertDot7->SetValue (m_convDot7);
        chkConvert8Spaces->SetValue (m_conv8Sp);
        chkTutorColor->SetValue (m_TutorColor);
        //tab6
        txtEmail->SetValue (m_Email);
        txtSearchURL->SetValue (m_SearchURL);
        chkMTutorBoot->SetValue(m_mTutorBoot);
        radMTutor->SetSelection(LevelToSelect(DEFAULTMLEVEL));

        chkFloppy0->SetValue(m_floppy0);
        chkFloppy1->SetValue(m_floppy1);
        txtFloppy0->SetLabel(m_floppy0File);
        txtFloppy1->SetLabel(m_floppy1File);

    }
    Refresh (false);
}

void PtermPrefDialog::OnRadiobox(wxCommandEvent& event)
{
    if (event.GetEventObject() == radMTutor)
    {
        m_mTutorLevel = SelectToLevel(radMTutor->GetSelection());
    }

}

void PtermPrefDialog::OnCheckbox (wxCommandEvent& event)
{
    void OnCheckbox (wxCommandEvent& event);
    lblProfileStatusMessage->SetLabel (wxT (" "));
    //tab0
    //tab1
    if (event.GetEventObject () == chkConnectAtStartup)
        m_connect = event.IsChecked ();
    //tab2
    else if (event.GetEventObject () == chkShowSignon)
        m_showSignon = event.IsChecked ();
    else if (event.GetEventObject () == chkShowSysName)
        m_showSysName = event.IsChecked ();
    else if (event.GetEventObject () == chkShowHost)
        m_showHost = event.IsChecked ();
    else if (event.GetEventObject () == chkShowStation)
        m_showStation = event.IsChecked ();
    //tab3
    else if (event.GetEventObject () == chkSimulate1200Baud)
        m_classicSpeed = event.IsChecked ();
    else if (event.GetEventObject () == chkEnableGSW)
        m_gswEnable = event.IsChecked ();
    else if (event.GetEventObject () == chkEnableNumericKeyPad)
        m_numpadArrows = event.IsChecked ();
    else if (event.GetEventObject () == chkIgnoreCapLock)
        m_ignoreCapLock = event.IsChecked ();
    else if (event.GetEventObject () == chkUsePLATOKeyboard)
        m_platoKb = event.IsChecked ();
    else if (event.GetEventObject () == chkUseAccelerators)
        m_useAccel = event.IsChecked ();
    else if (event.GetEventObject () == chkEnableBeep)
        m_beepEnable = event.IsChecked ();
    else if (event.GetEventObject () == chkDisableShiftSpace)
        m_DisableShiftSpace = event.IsChecked ();
    else if (event.GetEventObject () == chkDisableMouseDrag)
        m_DisableMouseDrag = event.IsChecked ();
    //tab4
    else if (event.GetEventObject () == chkDisableColor)
        m_noColor = event.IsChecked ();
    //tab5
    else if (event.GetEventObject () == chkSmartPaste)
        m_smartPaste = event.IsChecked ();
    else if (event.GetEventObject () == chkConvertDot7)
        m_convDot7 = event.IsChecked ();
    else if (event.GetEventObject () == chkConvert8Spaces)
        m_conv8Sp = event.IsChecked ();
    else if (event.GetEventObject () == chkTutorColor)
        m_TutorColor = event.IsChecked ();
    //tab6
    else if (event.GetEventObject() == chkMTutorBoot)
        m_mTutorBoot = event.IsChecked();

    else if (event.GetEventObject() == chkFloppy0)
    {
        m_floppy0 = event.IsChecked();
    }

    else if (event.GetEventObject() == chkFloppy1)
    {
        m_floppy1 = event.IsChecked();
    }

}

void PtermPrefDialog::OnSelect (wxCommandEvent& event)
{
    void OnSelect (wxCommandEvent& event);
    wxString profile;
    lblProfileStatusMessage->SetLabel (wxT (" "));
    if (event.GetEventObject () == lstProfiles)
    {
        profile = lstProfiles->GetStringSelection ();
        profile = txtProfile->GetLineText (0);
        btnAdd->Enable (!profile.IsEmpty ());
    }
}

void PtermPrefDialog::OnComboSelect (wxCommandEvent& event)
{
    void OnComboSelect (wxCommandEvent& event);
    lblProfileStatusMessage->SetLabel (wxT (" "));
    //tab1
    if (event.GetEventObject () == cboDefaultPort)
        cboDefaultPort->GetValue ().ToCLong (&m_port);
    //tab5
    else if (event.GetEventObject () == cboAutoLF)
        cboAutoLF->GetStringSelection ().ToCLong (&m_autoLF);
}

void PtermPrefDialog::OnDoubleClick (wxCommandEvent& event)
{
    void OnDoubleClick (wxCommandEvent& event);
    wxString profile;
    wxString str;
    wxString filename;
    lblProfileStatusMessage->SetLabel (wxT (" "));
    if (event.GetEventObject () == lstProfiles)
    {
        profile = lstProfiles->GetStringSelection ();
        profile = txtProfile->GetLineText (0);
        btnAdd->Enable (!profile.IsEmpty ());
        //do the load code
        profile = lstProfiles->GetStringSelection ();
        if (ptermApp->LoadProfile (profile, wxT ("")))
        {
            SetControlState ();
            lblProfileStatusMessage->SetLabel (_("Profile loaded."));
        }
        else
        {
            filename = ptermApp->ProfileFileName (profile);
            str.Printf (wxT ("Unable to load profile: %s"), 
                        filename);
            wxMessageBox (str, _("Error"), wxOK | wxICON_HAND);
        }
    }
}

void PtermPrefDialog::OnChange (wxCommandEvent& event)
{
    void OnChange (wxCommandEvent& event);
    wxString profile;
    lblProfileStatusMessage->SetLabel (wxT (" "));
    if (event.GetEventObject () == txtProfile)
    {
        profile = txtProfile->GetLineText (0);
        btnAdd->Enable (!profile.IsEmpty ());
    }
    //tab1
    else if (event.GetEventObject () == txtShellFirst)
        m_ShellFirst = txtShellFirst->GetLineText (0);
    else if (event.GetEventObject () == txtDefaultHost)
        m_host = txtDefaultHost->GetLineText (0);
    else if (event.GetEventObject () == cboDefaultPort)
        cboDefaultPort->GetValue ().ToCLong (&m_port);
    //tab5
    else if (event.GetEventObject () == cboAutoLF)
        cboAutoLF->GetValue ().ToCLong (&m_autoLF);
    //tab5
    else if (event.GetEventObject () == txtCharDelay)
        txtCharDelay->GetLineText (0).ToCLong (&m_charDelay);
    else if (event.GetEventObject () == txtLineDelay)
        txtLineDelay->GetLineText (0).ToCLong (&m_lineDelay);
    //tab6
    else if (event.GetEventObject () == txtEmail)
        m_Email = txtEmail->GetLineText (0);
    else if (event.GetEventObject () == txtSearchURL)
        m_SearchURL = txtSearchURL->GetLineText (0);
}

void PtermPrefDialog::paintBitmap (wxBitmap &bm, wxColour &color)
{
    wxBrush bitmapBrush (color, wxBRUSHSTYLE_SOLID);
    wxMemoryDC memDC;

    memDC.SelectObject (bm);
    memDC.SetBackground (bitmapBrush);
    memDC.Clear ();
    memDC.SetBackground (wxNullBrush);
    memDC.SelectObject (wxNullBitmap);
}



