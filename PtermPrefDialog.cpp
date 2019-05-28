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
#include "PtermFrame.h"

// ----------------------------------------------------------------------------
// PtermPrefDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (PtermPrefDialog, wxDialog)
    EVT_CLOSE (PtermPrefDialog::OnClose)
    EVT_BUTTON (wxID_ANY, PtermPrefDialog::OnButton)
    EVT_CHECKBOX (wxID_ANY, PtermPrefDialog::OnCheckbox)
    EVT_LISTBOX (wxID_ANY, PtermPrefDialog::OnSelect)
    EVT_LISTBOX_DCLICK (wxID_ANY, PtermPrefDialog::OnDoubleClick)
    EVT_TEXT (wxID_ANY, PtermPrefDialog::OnChange)
    EVT_RADIOBOX (wxID_ANY, PtermPrefDialog::OnRadioSelect)
    END_EVENT_TABLE ();

PtermPrefDialog::PtermPrefDialog (PtermConnDialog *parent, wxWindowID id,
    const wxString &title, wxPoint pos,
    wxSize size)
    : wxDialog (parent, id, title, pos, size),
      m_profile (NULL),
      m_initDone (false),
      m_profileEdit (true),
      m_connParent (parent)
{
    PtermInitDialog ();
}

PtermPrefDialog::PtermPrefDialog (PtermFrame *parent, wxWindowID id,
    const wxString &title, wxPoint pos,
    wxSize size,
    const PtermProfile &fromprofile)
    : wxDialog (parent, id, title, pos, size),
      m_initDone (false),
      m_profileEdit (false),
      m_connParent (NULL)

{
    m_profile = new PtermProfile (fromprofile);
    PtermInitDialog ();
}

PtermPrefDialog::~PtermPrefDialog ()
{
    if (m_profileEdit)
        ptermApp->m_prefDialog = NULL;
    else
        ptermApp->m_sessDialog = NULL;
}


void PtermPrefDialog::PtermInitDialog (void)
{
    // static ui objects, note dynamic controls, e.g. those that hold
    // values or require event processing are declared above
    //tab0
    wxScrolledWindow* tab0;
    wxStaticText* lblExplain0;
    wxStaticText* lblProfileActionExplain;
    wxStaticText* lblNewProfileExplain;
    wxStaticText* lblNewProfile;
    //tab1
    wxScrolledWindow* tab1;
    wxStaticText* lblExplain1;
    wxStaticText* lblShellFirst;
    wxStaticText* lblDefaultHost;
    wxStaticText* lblDefaultPort;
    wxStaticText* lblExplainPort;
    //tab2
    wxScrolledWindow* tab2;
    wxStaticText* lblExplain2;
    //tab3
    wxScrolledWindow* tab3;
    wxStaticText* lblExplain3;
    //tab4
    wxScrolledWindow* tab4;
    wxStaticText* lblExplain4;
    wxStaticText* lblFGColor;
    wxStaticText* lblBGColor;
    wxStaticText* lblExplainColor;
    //tab5
    wxScrolledWindow* tab5;
    wxStaticText* lblExplain5;
    wxStaticText* lblCharDelay;
    wxStaticText* lblCharDelay2;
    wxStaticText* lblLineDelay;
    wxStaticText* lblLineDelay2;
    wxStaticText* lblAutoNewLine;
    wxStaticText* lblAutoNewLine2;
    //tab6
    wxScrolledWindow* tab6;
    wxStaticText* lblSearchURL;
    wxStaticText* lblMTUTOR;
    wxStaticText* lblFloppy0;
    wxStaticText* lblFloppy1;

    wxFont dfont = wxFont (10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_NORMAL);
    wxFont dfont2 = wxFont (10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD);

    m_floppy0Changed = false;
    m_floppy1Changed = false;

    m_HelpContext = helpContextGenericIndex;
    
    // ui object creation / placement, note initialization of values is below
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer (wxVERTICAL);
    
    //notebook
    tabPrefsDialog = new wxNotebook (this, wxID_ANY, wxDefaultPosition,
                                     wxDefaultSize, wxNB_TOP);
    tabPrefsDialog->SetFont (dfont);

    if (m_profileEdit)
    {
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
        lblExplain0->SetFont (dfont2);
        page0->Add (lblExplain0, 0, wxALL, 5);
        lstProfiles = new ProfileList (tab0, wxID_ANY, wxDefaultPosition,
                                       wxSize (-1, -1));
        page0->Add (lstProfiles, 1, wxALL | wxEXPAND | wxALIGN_TOP, 5);

        wxFlexGridSizer* fgs01;
        fgs01 = new wxFlexGridSizer (4, 1, 0, 0);
        fgs01->SetFlexibleDirection (wxVERTICAL);
        lblProfileActionExplain = new wxStaticText (tab0, wxID_ANY,
                                                    _("Select a profile above, then click a button below."),
                                                    wxDefaultPosition,
                                                    wxDefaultSize, 0);
        lblProfileActionExplain->SetFont (dfont2);
        fgs01->Add (lblProfileActionExplain, 0, wxALL, 5);

        wxFlexGridSizer* fgs011;
        fgs011 = new wxFlexGridSizer (2, 4, 0, 0);
        btnLoad = new wxButton (tab0, wxID_ANY, _("Load"), wxDefaultPosition,
                                wxDefaultSize, 0);
        btnLoad->SetFont (dfont);
        fgs011->Add (btnLoad, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);
        btnDelete = new wxButton (tab0, wxID_ANY, _("Delete"),
                                  wxDefaultPosition,
                                  wxDefaultSize, 0);
        btnDelete->SetFont (dfont);
        fgs011->Add (btnDelete, 0, wxRIGHT | wxLEFT, 5);
        lblProfileStatusMessage = new wxStaticText (tab0, wxID_ANY,
                                                    wxT (""),
                                                    wxDefaultPosition,
                                                    wxDefaultSize, 0);
        lblProfileStatusMessage->SetFont (dfont);
        fgs011->Add (lblProfileStatusMessage, 0,
                     wxALL | wxALIGN_CENTER_VERTICAL, 5);
        fgs01->Add (fgs011, 1, 0, 5);
        lblNewProfileExplain = new wxStaticText (tab0, wxID_ANY,
                                                 _("Or, enter the name of a new profile below and click Save As."),
                                                 wxDefaultPosition,
                                                 wxDefaultSize, 0);
        lblNewProfileExplain->SetFont (dfont2);
        fgs01->Add (lblNewProfileExplain, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

        wxFlexGridSizer* fgs012;
        fgs012 = new wxFlexGridSizer (1, 3, 0, 0);
        fgs012->AddGrowableCol (2);
        fgs012->SetFlexibleDirection (wxHORIZONTAL);
        lblNewProfile = new wxStaticText (tab0, wxID_ANY, _("Profile"),
                                          wxDefaultPosition, wxDefaultSize, 0);
        fgs012->Add (lblNewProfile, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        txtProfile = new wxTextCtrl (tab0, wxID_ANY, wxT (""), wxDefaultPosition,
                                     wxDefaultSize, 0);
        txtProfile->SetMinSize (wxSize (310, -1));
        fgs012->Add (txtProfile, 1, wxALL | wxEXPAND, 5);
        btnSaveAs = new wxButton (tab0, wxID_ANY, _("Save As"),
                                  wxDefaultPosition,
                                  wxDefaultSize, wxBU_EXACTFIT);
        btnSaveAs->Enable (false);
        fgs012->Add (btnSaveAs, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        fgs01->Add (fgs012, 1, wxEXPAND | wxALIGN_TOP, 5);
        page0->Add (fgs01, 0, wxEXPAND, 5);
        tab0->SetSizer (page0);
        tab0->Layout ();
        page0->Fit (tab0);
        tabPrefsDialog->AddPage (tab0, _("Profiles"), false);

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
        lblExplain1->SetFont (dfont2);
        page1->Add (lblExplain1, 1, wxALL, 5);
        wxBoxSizer* bs11;
        bs11 = new wxBoxSizer (wxVERTICAL);

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
        lblExplainPort->SetFont (dfont2);
        page1->Add (lblExplainPort, 0, wxALL, 5);
        tab1->SetSizer (page1);
        tab1->Layout ();
        page1->Fit (tab1);
        tabPrefsDialog->AddPage (tab1, _("Connection"), false);

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
        lblExplain2->SetFont (dfont2);
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

#ifndef __WXMAC__
        chkUseAccelerators = new wxCheckBox (tab2, wxID_ANY,
            _ ("Enable control-key menu accelerators"),
            wxDefaultPosition, wxDefaultSize, 0);
        chkUseAccelerators->SetValue (true);
        chkUseAccelerators->SetFont (dfont);
        //if (!m_profileEdit)
        //    chkUseAccelerators->Disable ();
        page2->Add (chkUseAccelerators, 0, wxALL, 5);
#endif

        tab2->SetSizer (page2);
        tab2->Layout ();
        page2->Fit (tab2);
        tabPrefsDialog->AddPage (tab2, _("Title"), false);
    }
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
    lblExplain3->SetFont (dfont2);
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
    page4 = new wxFlexGridSizer (8, 1, 0, 0);
    page4->AddGrowableCol (0);
    page4->AddGrowableRow (4);
    page4->SetFlexibleDirection (wxVERTICAL);
    lblExplain4 = new wxStaticText (tab4, wxID_ANY,
                                    _("Settings on this page allow you to change the display appearance."),
                                    wxDefaultPosition, wxDefaultSize, 0);
    lblExplain4->SetFont (dfont2);
    page4->Add (lblExplain4, 0, wxALL, 5);
    wxBoxSizer* bs41;
    bs41 = new wxBoxSizer (wxVERTICAL);
    chkDisableColor = new wxCheckBox (tab4, wxID_ANY,
                                      _("Disable -color- (ASCII mode)"),
                                      wxDefaultPosition, wxDefaultSize, 0);
    bs41->Add (chkDisableColor, 0, wxALL, 5);

    wxFlexGridSizer* fgs411;
    fgs411 = new wxFlexGridSizer (3, 2, 0, 0);
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

    chkFancyScale = new wxCheckBox (tab4, wxID_ANY,
        _ ("Use BILINEAR scaling"),
        wxDefaultPosition, wxDefaultSize, 0);
    chkFancyScale->SetValue (false);
    page4->Add (chkFancyScale, 0, wxALL, 5);

    wxFlexGridSizer* fgs412;
    fgs412 = new wxFlexGridSizer (3, 1, 0, 0);

    if (m_profileEdit)
    {
        static const wxString sZoom[] =
        {
            _T ("Stretch display"),
            _T ("Keep aspect ratio"),
            _T ("1x"),
            _T ("2x"),
            _T ("3x")
        };

        rdoDefaultScale = new wxRadioBox (tab4, wxID_ANY, _T ("Startup Zoom"),
            wxDefaultPosition, wxDefaultSize,
            WXSIZEOF (sZoom), sZoom,
            1, wxRA_SPECIFY_COLS);
        fgs412->Add (rdoDefaultScale, 0, wxGROW | wxALL, 5);

        chkShowMenuBar = new wxCheckBox (tab4, wxID_ANY,
            _ ("Show Menu Bar"),
            wxDefaultPosition, wxDefaultSize, 0);
        chkShowMenuBar->SetValue (false);
        fgs412->Add (chkShowMenuBar, 0, wxALL, 5);

        chkShowStatusBar = new wxCheckBox (tab4, wxID_ANY,
            _ ("Show Status Bar"),
            wxDefaultPosition, wxDefaultSize, 0);
        chkShowStatusBar->SetValue (false);
        fgs412->Add (chkShowStatusBar, 0, wxALL, 5);
    }
    page4->Add (fgs412, 1, wxEXPAND | wxALIGN_TOP, 5);

    lblExplainColor = new wxStaticText (tab4, wxID_ANY, 
                                        _("* Applied in Classic mode or if -color- is disabled in ASCII mode"),
                                        wxDefaultPosition, wxDefaultSize, 0);
    lblExplainColor->SetFont (dfont2);
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
    lblExplain5->SetFont (dfont2);
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
    bs51->Add (chkConvertDot7, 0, wxALL, 5);
    chkConvert8Spaces = new wxCheckBox (tab5, wxID_ANY,
                                        _("Convert 8 consecutive spaces into a tab"),
                                        wxDefaultPosition, wxDefaultSize, 0);
    chkConvert8Spaces->SetValue (true);
    bs51->Add (chkConvert8Spaces, 0, wxALL, 5);
    chkTutorColor = new wxCheckBox (tab5, wxID_ANY,
                                    _("Display TUTOR colorization options on Edit/Context menus"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    chkTutorColor->SetValue (true);
    bs51->Add (chkTutorColor, 0, wxALL, 5);
    chkTrimEnd = new wxCheckBox (tab5, wxID_ANY,
                                        _("Trim spaces from end of copied text"),
                                        wxDefaultPosition, wxDefaultSize, 0);
    chkTrimEnd->SetValue (true);
    bs51->Add (chkTrimEnd, 0, wxALL, 5);
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

    wxFlexGridSizer* page6;
    page6 = new wxFlexGridSizer (0, 1, 0, 0);
    page6->AddGrowableCol (0);
    page6->SetFlexibleDirection (wxBOTH);
    lblSearchURL = new wxStaticText (tab6, wxID_ANY,
                                     _("Specify URL for menu option 'Search this...'"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    page6->Add (lblSearchURL, 0, wxALL, 5);


    txtSearchURL = new wxTextCtrl (tab6, wxID_ANY, wxT (""), wxDefaultPosition,
                                   wxDefaultSize, 0 | wxTAB_TRAVERSAL);
    txtSearchURL->SetMaxLength (255); 
    page6->Add (txtSearchURL, 0, wxALL | wxEXPAND, 5);

    lblMTUTOR = new wxStaticText (tab6, wxID_ANY,
                                  _("MicroTutor related options:"),
                                  wxDefaultPosition, wxDefaultSize, 0);
    lblMTUTOR->SetFont (dfont2);
    page6->Add (lblMTUTOR, 0, wxALL | wxALIGN_BOTTOM, 5);

    if (m_profileEdit)
    {
        chkMTutorBoot = new wxCheckBox (tab6, wxID_ANY,
                                        _("Boot to MicroTutor"),
                                        wxDefaultPosition, wxDefaultSize, 0);
        chkMTutorBoot->SetValue (false);
        page6->Add (chkMTutorBoot, 0, wxALL, 5);
    }
    
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
    btnHelp = new wxButton (this, wxID_CONTEXT_HELP,
        _ ("Help"),
        wxDefaultPosition,
        wxDefaultSize, 0);
    btnHelp->SetFont (dfont);
    fgsButtons->Add (btnHelp, 0, wxALL, 5);
    btnOK = new wxButton (this, wxID_OK,
                          m_profileEdit ? _("Save") : _("Apply"),
                          wxDefaultPosition,
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
    SetSizer (bSizer1);
    Layout ();

    // If no profile set yet, but one is selected in the list box,
    // load that one.
    if (m_profile == NULL)
    {
        SelectProfile (lstProfiles->GetSelection ());
    }

    m_initDone = true;

    // set object value properties
    SetControlState ();
    Modified (false);
    
    //button bar
    btnOK->SetDefault ();
}

void PtermPrefDialog::Modified (bool modified)
{
    m_profModified = modified;
    UpdateTitle ();
}

void PtermPrefDialog::UpdateTitle (void)
{
    wxString title;

    // Enable the OK (Save or Apply) button if there is some change to
    // be saved.
    btnOK->Enable (m_profModified);
    
    if (m_profModified)
        title = wxT ("* ");
    if (m_profileEdit)
        title.Append (_("Profile editor"));
    else
        title.Append (_("Session settings"));
    if (!m_profile->m_profileName.IsEmpty ())
    {
        title.Append (wxT (": "));
        title.Append (m_profile->m_profileName);
    }
    SetLabel (title);
}

void PtermPrefDialog::OnClose (wxCloseEvent& )
{
    ptermApp->m_prefDialog = NULL;
    this->Destroy ();
}

bool PtermPrefDialog::ValidProfile (wxString profile)
{
    wxString validchr = wxT ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_. ()");
    unsigned int cnt;
    for (cnt = 0; cnt < profile.Len (); cnt++)
        if (!validchr.Contains (profile.Mid (cnt, 1)))
            return false;
    return true;
}

bool PtermPrefDialog::DeleteProfile (wxString profile)
{
    wxFileName filename;

    filename = ProfileFileName (profile);
    
    //delete file
    if (filename.IsOk () && filename.IsFileWritable ())
    {
        wxRemoveFile (filename.GetFullPath ());
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
    if (m_profileEdit)
    {
        //tab0
        //tab1
        txtShellFirst->SetValue (m_profile->m_ShellFirst);
        txtDefaultHost->SetValue (m_profile->m_host);
        ws.Printf ("%ld", m_profile->m_port);
        cboDefaultPort->SetValue (ws);
        //tab2
        chkShowSignon->SetValue (m_profile->m_showSignon);
        chkShowSysName->SetValue (m_profile->m_showSysName);
        chkShowHost->SetValue (m_profile->m_showHost);
        chkShowStation->SetValue (m_profile->m_showStation);
#ifndef __WXMAC__
        chkUseAccelerators->SetValue (m_profile->m_useAccel);
#endif
    }
    //tab3
    chkSimulate1200Baud->SetValue (m_profile->m_classicSpeed);
    chkEnableGSW->SetValue (m_profile->m_gswEnable);
    chkEnableNumericKeyPad->SetValue (m_profile->m_numpadArrows);
    chkIgnoreCapLock->SetValue (m_profile->m_ignoreCapLock);
    chkUsePLATOKeyboard->SetValue (m_profile->m_platoKb);


    chkEnableBeep->SetValue (m_profile->m_beepEnable);
    chkDisableShiftSpace->SetValue (m_profile->m_DisableShiftSpace);
    chkDisableMouseDrag->SetValue (m_profile->m_DisableMouseDrag);
    //tab4
    chkDisableColor->SetValue (m_profile->m_noColor);
#if defined (_WIN32)
    btnFGColor->SetBackgroundColour (m_profile->m_fgColor);
    btnBGColor->SetBackgroundColour (m_profile->m_bgColor);
#else
    paintBitmap (fgBitmap, m_profile->m_fgColor);
    btnFGColor->SetBitmapLabel (fgBitmap);
    paintBitmap (bgBitmap, m_profile->m_bgColor);
    btnBGColor->SetBitmapLabel (bgBitmap);
#endif

    chkFancyScale->SetValue (m_profile->m_FancyScaling);

    if (m_profileEdit)
    {
        ws.Printf ("%f", m_profile->m_scale);
        rdoDefaultScale->SetSelection (1+(int)m_profile->m_scale);
        chkShowMenuBar->SetValue (m_profile->m_showMenuBar);
        chkShowStatusBar->SetValue (m_profile->m_showStatusBar);
    }
    //tab5
    ws.Printf ("%ld", m_profile->m_charDelay);
    txtCharDelay->SetValue (ws);
    ws.Printf ("%ld", m_profile->m_lineDelay);
    txtLineDelay->SetValue (ws);
    ws.Printf ("%ld", m_profile->m_autoLF);
    cboAutoLF->SetValue (ws);
    chkSmartPaste->SetValue (m_profile->m_smartPaste);
    chkConvertDot7->SetValue (m_profile->m_convDot7);
    chkConvert8Spaces->SetValue (m_profile->m_conv8Sp);
    chkTutorColor->SetValue (m_profile->m_TutorColor);
    chkTrimEnd->SetValue (m_profile->m_trimEnd);

    //tab6
    txtSearchURL->SetValue (m_profile->m_SearchURL);
    if (m_profileEdit)
        chkMTutorBoot->SetValue(m_profile->m_mTutorBoot);
    chkFloppy0->SetValue(m_profile->m_floppy0);
    chkFloppy1->SetValue(m_profile->m_floppy1);
    txtFloppy0->SetLabel(m_profile->m_floppy0File);
    txtFloppy1->SetLabel(m_profile->m_floppy1File);
}

void PtermPrefDialog::OnButton (wxCommandEvent& event)
{
    wxBitmap fgBitmap (15, 15);
    wxBitmap bgBitmap (15, 15);
    wxString profile;
    wxString filename;
    wxString str;
    
    if (m_profileEdit)
        lblProfileStatusMessage->SetLabel (wxT (""));
    if (event.GetEventObject () == btnOK)
    {
        if (m_profileEdit)
        {
            if (m_profile->SaveProfile ())
            {
                SetControlState ();
                lblProfileStatusMessage->SetLabel (_("Profile saved."));

                if (m_connParent != NULL)
                {
                    m_connParent->lstProfiles->RefreshList ();
                    m_connParent->ReloadProfile ();
                }

                ptermApp->m_prefDialog = NULL;
                this->Destroy ();
            }
            else
            {
                str.Printf (_("Unable to save profile: %s"),
                            m_profile->m_profileName);
                wxMessageBox (str, _("Error"), wxOK | wxICON_ERROR);
            }
        }
        else
        {
            // also destroys dialog
            ((PtermFrame *)(m_parent))->UpdateSessionSettings ();
        }
    }
    else if (event.GetEventObject () == btnLoad)
    {
        SelectProfile (lstProfiles->GetSelection ());
        if (m_profile->IsOk ())
        {
            SetControlState ();
            lblProfileStatusMessage->SetLabel (_("Profile loaded."));
            Modified (false);
        }
        else
        {
            str.Printf (_("Unable to load profile: %s"), profile);
            wxMessageBox (str, _("Error"), wxOK | wxICON_ERROR);
        }
    }
    else if (event.GetEventObject () == btnDelete)
    {
        profile = lstProfiles->GetStringSelection ();

        wxString str;
        str.Printf (_("Do you really want to delete profile '%s' ?"), profile);
        if (wxMessageBox (str, _("Confirm"),
                          wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
        {
            if (DeleteProfile (profile))
            {
                SetControlState ();
                lblProfileStatusMessage->SetLabel (_("Profile deleted."));
                lstProfiles->RefreshList ();
                if (m_connParent != NULL)
                {
                    m_connParent->lstProfiles->RefreshList ();
                }
            }
            else
            {
                str.Printf (_("Unable to delete profile: %s"), 
                            profile);
                wxMessageBox (str, _("Error"), wxOK | wxICON_ERROR);
            }
        }
    }
    else if (event.GetEventObject () == btnSaveAs)
    {
        profile = txtProfile->GetLineText (0);
        if (ValidProfile (profile))
        {
            if (lstProfiles->FindString (profile, true) != wxNOT_FOUND)
            {
                wxString msg;

                msg.Printf (_("Profile %s already exists"), profile);
                wxMessageBox (msg, _("Problem"), wxOK | wxICON_ERROR);
            }
            else
            {
                m_profile->SetName (profile);
                m_profile->SaveProfile ();
                ptermApp->m_curProfile = profile;
                Modified (false);
                lstProfiles->RefreshList ();
                if (m_connParent != NULL)
                {
                    m_connParent->lstProfiles->RefreshList ();
                }
                lblProfileStatusMessage->SetLabel (_("Profile added."));
            }
        }
        else
        {
            wxMessageBox (_("The profile name you entered contains illegal characters.\n\n"
                            "Valid characters are:\n\n"
                            "Standard: a-z, A-Z, 0-9\n"
                            "Special:  parentheses, dash, underscore, period, and space"),
                          _("Problem"), wxOK | wxICON_ERROR);
        }
    }
    else if (event.GetEventObject() == btnFloppy0)
    {
        wxFileDialog
            openFileDialog(this, _("Open Floppy 0 file"), "", "",
                           "MTE files (*.mte)|*.mte",
                           wxFD_OPEN | wxFD_FILE_MUST_EXIST);

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
        m_profile->m_floppy0File = openFileDialog.GetPath();
        txtFloppy0->SetLabel(m_profile->m_floppy0File);
        m_floppy0Changed = true;
        Modified();
    }
    else if (event.GetEventObject() == btnFloppy1)
    {
        wxFileDialog
            openFileDialog(this, _("Open Floppy 1 file"), "", "",
                "MTE files (*.mte)|*.mte", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

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
        m_profile->m_floppy1File = openFileDialog.GetPath();
        txtFloppy1->SetLabel(m_profile->m_floppy1File);
        m_floppy1Changed = true;
        Modified();
    }
    else if (event.GetEventObject () == btnFGColor)
    {
        m_profile->m_fgColor = PtermApp::SelectColor (*this, _("Foreground"), m_profile->m_fgColor);
#if defined (_WIN32)
        btnFGColor->SetBackgroundColour (m_profile->m_fgColor);
#else
        paintBitmap (fgBitmap, m_profile->m_fgColor);
        btnFGColor->SetBitmapLabel (fgBitmap);
#endif
        Modified();
    }
    else if (event.GetEventObject () == btnBGColor)
    {
        m_profile->m_bgColor = PtermApp::SelectColor (*this, _("Background"), m_profile->m_bgColor);
#if defined (_WIN32)
        btnBGColor->SetBackgroundColour (m_profile->m_bgColor);
#else
        paintBitmap (bgBitmap, m_profile->m_bgColor);
        btnBGColor->SetBitmapLabel (bgBitmap);
#endif
        Modified();
    }
    else if (event.GetEventObject () == btnCancel)
    {
        ptermApp->m_prefDialog = NULL;
        this->Destroy ();
    }
    else if (event.GetEventObject () == btnDefaults)
    {
        m_profile->init ();
        SetControlState ();
        Modified();
    }
    else if (event.GetEventObject () == btnHelp)
    {
        // below depends on order of tabs in dialog
        int sel = tabPrefsDialog->GetSelection ();
        if (m_profileEdit)  // 7 tabs - profile editor
        {
            switch (sel)
            {
            case 0: m_HelpContext = helpContextProfiles; break;
            case 1: m_HelpContext = helpContextConnection; break;
            case 2: m_HelpContext = helpContextTitle; break;
            case 3: m_HelpContext = helpContextEmulation; break;
            case 4: m_HelpContext = helpContextDisplay; break;
            case 5: m_HelpContext = helpContextPasting; break;
            case 6: m_HelpContext = helpContextLocal; break;
            }
        }
        else   // 4 tabs - Session Settings
        {
            switch (sel)
            {
            case 0: m_HelpContext = helpContextEmulation; break;
            case 1: m_HelpContext = helpContextDisplay; break;
            case 2: m_HelpContext = helpContextPasting; break;
            case 3: m_HelpContext = helpContextLocal; break;
            }
        }

        // Invoke Mtutor HELP here passing m_HelpContext

        ptermApp->LaunchMtutorHelp (m_HelpContext);
    }
    Refresh (false);
}

void PtermPrefDialog::OnRadioSelect (wxCommandEvent& event)
{
    if (event.GetEventObject () == rdoDefaultScale)
        m_profile->m_scale = event.GetSelection()-1;
    else
        return;

    Modified ();
}

void PtermPrefDialog::OnCheckbox (wxCommandEvent& event)
{
    if (m_profileEdit)
        lblProfileStatusMessage->SetLabel (wxT (" "));
    //tab0
    //tab1
    //tab2
    if (event.GetEventObject () == chkShowSignon)
        m_profile->m_showSignon = event.IsChecked ();
    else if (event.GetEventObject () == chkShowSysName)
        m_profile->m_showSysName = event.IsChecked ();
    else if (event.GetEventObject () == chkShowHost)
        m_profile->m_showHost = event.IsChecked ();
    else if (event.GetEventObject () == chkShowStation)
        m_profile->m_showStation = event.IsChecked ();
    //tab3
    else if (event.GetEventObject () == chkSimulate1200Baud)
        m_profile->m_classicSpeed = event.IsChecked ();
    else if (event.GetEventObject () == chkEnableGSW)
        m_profile->m_gswEnable = event.IsChecked ();
    else if (event.GetEventObject () == chkEnableNumericKeyPad)
        m_profile->m_numpadArrows = event.IsChecked ();
    else if (event.GetEventObject () == chkIgnoreCapLock)
        m_profile->m_ignoreCapLock = event.IsChecked ();
    else if (event.GetEventObject () == chkUsePLATOKeyboard)
        m_profile->m_platoKb = event.IsChecked ();
#ifndef __WXMAC__
    else if (event.GetEventObject () == chkUseAccelerators)
        m_profile->m_useAccel = event.IsChecked ();
#endif
    else if (event.GetEventObject () == chkEnableBeep)
        m_profile->m_beepEnable = event.IsChecked ();
    else if (event.GetEventObject () == chkDisableShiftSpace)
        m_profile->m_DisableShiftSpace = event.IsChecked ();
    else if (event.GetEventObject () == chkDisableMouseDrag)
        m_profile->m_DisableMouseDrag = event.IsChecked ();
    //tab4
    else if (event.GetEventObject () == chkDisableColor)
        m_profile->m_noColor = event.IsChecked ();
#ifndef __WXMAC__
    else if (m_profileEdit && event.GetEventObject () == chkShowMenuBar)
        m_profile->m_showMenuBar = event.IsChecked ();
#endif
    else if (m_profileEdit && event.GetEventObject () == chkShowStatusBar)
        m_profile->m_showStatusBar = event.IsChecked ();
    else if (event.GetEventObject () == chkFancyScale)
        m_profile->m_FancyScaling = event.IsChecked ();

    //tab5
    else if (event.GetEventObject () == chkSmartPaste)
        m_profile->m_smartPaste = event.IsChecked ();
    else if (event.GetEventObject () == chkConvertDot7)
        m_profile->m_convDot7 = event.IsChecked ();
    else if (event.GetEventObject () == chkConvert8Spaces)
        m_profile->m_conv8Sp = event.IsChecked ();
    else if (event.GetEventObject () == chkTutorColor)
        m_profile->m_TutorColor = event.IsChecked ();
    else if (event.GetEventObject () == chkTrimEnd)
        m_profile->m_trimEnd = event.IsChecked ();
    //tab6
    else if (event.GetEventObject () == chkMTutorBoot)
    {
        m_profile->m_mTutorBoot = event.IsChecked ();
    }

    else if (event.GetEventObject() == chkFloppy0)
    {
        m_profile->m_floppy0 = event.IsChecked();
        m_floppy0Changed = true;
    }

    else if (event.GetEventObject() == chkFloppy1)
    {
        m_profile->m_floppy1 = event.IsChecked();
        m_floppy1Changed = true;
    }
    else
        return;
    
    Modified();
}

void PtermPrefDialog::OnSelect (wxCommandEvent&)
{
    wxString profile;
    lblProfileStatusMessage->SetLabel (wxT (" "));
}

void PtermPrefDialog::OnComboSelect (wxCommandEvent& event)
{
    lblProfileStatusMessage->SetLabel (wxT (" "));
    //tab1
    if (event.GetEventObject () == cboDefaultPort)
        cboDefaultPort->GetValue ().ToCLong (&m_profile->m_port);
    //tab5
    else if (event.GetEventObject () == cboAutoLF)
        cboAutoLF->GetStringSelection ().ToCLong (&m_profile->m_autoLF);
    else
        return;
    
    Modified();
}

void PtermPrefDialog::OnDoubleClick (wxCommandEvent& event)
{
    wxString profile;
    wxString str;
    wxString filename;
    PtermProfile *np = NULL;
    
    lblProfileStatusMessage->SetLabel (wxT (" "));
    if (event.GetEventObject () == lstProfiles)
    {
        profile = lstProfiles->GetStringSelection ();
        //do the load code
        np = new PtermProfile (profile, true);
        if (np->IsOk ())
        {
            if (m_profile != NULL)
                delete m_profile;
            m_profile = np;
            SetControlState ();
            lblProfileStatusMessage->SetLabel (_("Profile loaded."));
            Modified (false);
        }
        else
        {
            if (np != NULL)
                delete np;
            str.Printf (_("Unable to load profile: %s"), 
                        m_profile->m_fileName.GetFullPath ());
            wxMessageBox (str, _("Error"), wxOK | wxICON_ERROR);
        }
    }
}

void PtermPrefDialog::OnChange (wxCommandEvent& event)
{
    wxString profile;

    if (!m_initDone)
        return;

    //tab5
    if (event.GetEventObject () == cboAutoLF)
        cboAutoLF->GetValue ().ToCLong (&m_profile->m_autoLF);
    //tab5
    else if (event.GetEventObject () == txtCharDelay)
        txtCharDelay->GetLineText (0).ToCLong (&m_profile->m_charDelay);
    else if (event.GetEventObject () == txtLineDelay)
        txtLineDelay->GetLineText (0).ToCLong (&m_profile->m_lineDelay);
    //tab6
    else if (event.GetEventObject () == txtSearchURL)
        m_profile->m_SearchURL = txtSearchURL->GetLineText (0);
    else if (m_profileEdit)
    {
        if (event.GetEventObject () == txtProfile)
        {
            profile = txtProfile->GetLineText (0);
            btnSaveAs->Enable (!profile.IsEmpty ());
        }
        //tab1
        else if (event.GetEventObject () == txtShellFirst)
            m_profile->m_ShellFirst = txtShellFirst->GetLineText (0);
        else if (event.GetEventObject () == txtDefaultHost)
            m_profile->m_host = txtDefaultHost->GetLineText (0);
        else if (event.GetEventObject () == cboDefaultPort)
            cboDefaultPort->GetValue ().ToCLong (&m_profile->m_port);

        else
            return;
    }
    else
        return;

    Modified();    
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

void PtermPrefDialog::SelectProfile (int n)
{
    wxString profile;
    wxString str;
    wxString filename;

    if (n != wxNOT_FOUND)
        profile = lstProfiles->GetString (n);
    if (m_profile != NULL)
    {
        if (profile == m_profile->m_profileName)
            return;
        delete m_profile;
    }
    if (profile.IsEmpty ())
        m_profile = new PtermProfile ();
    else
    {
        m_profile = new PtermProfile (profile, true);
        if (!m_profile->IsOk ())
        {
            str.Printf (_("Profile '%s' not found. Problem loading file:\n\n    %s"),
                        profile, m_profile->m_fileName.GetFullPath ());
            wxMessageBox (str, _("Error"), wxOK | wxICON_ERROR);
        }
    }
}


ProfileList::ProfileList (wxWindow * parent, wxWindowID id,
                          const wxPoint & pos,
                          const wxSize & size) :
    wxListBox (parent, id, pos, size, 0, NULL, wxLB_SORT)
{
    wxFont dfont = wxFont (10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_NORMAL);
    SetFont (dfont);
    RefreshList ();
}

void ProfileList::RefreshList (void)
{
    int i;
    wxString filename;
    bool cont;
    wxDir ldir (ptermApp->m_userdatadir);
    
    Clear ();
    if (ldir.IsOpened ())
    {
        cont = ldir.GetFirst (&filename, wxT ("*.ppf"), wxDIR_DEFAULT);
        for (i = 0; cont; i++)
        {
            wxFileName fn (filename);
            Append (fn.GetName ());
            cont = ldir.GetNext (&filename);
        }
        i = FindString (ptermApp->m_curProfile, true);
        if (i != wxNOT_FOUND)
            EnsureVisible (i);
        SetSelection (i);
    }
}
