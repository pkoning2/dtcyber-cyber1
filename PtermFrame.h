////////////////////////////////////////////////////////////////////////////
// Name:        FrameCanvas.h
// Purpose:     Declaration of classes Frame and Canvas 
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __FrameCanvas_H__
#define __FrameCanvas_H__ 1

#include "CommonHeader.h"
#include "Z80.h"
#include "PtermConnection.h"
#include "PtermCanvas.h"
#include "MTFile.h"
#include "PtermPrintout.h"
#include "PtermApp.h"

#define ResetProc Z80Reset
#define MicroEmulate Z80Emulate(5000000)

#define RAM     m_context.memory
#define PC      state->pc
#define SP      state->registers.word[Z80_SP]

#define BC_reg_C    state->registers.byte[Z80_C]
#define BC_reg_B    state->registers.byte[Z80_B]
#define DE_reg_E    state->registers.byte[Z80_E]
#define DE_reg_D    state->registers.byte[Z80_D]
#define HL_reg_L    state->registers.byte[Z80_L]
#define HL_reg_H    state->registers.byte[Z80_H]

#define BC_pair     state->registers.word[Z80_BC]
#define DE_pair     state->registers.word[Z80_DE]
#define HL_pair     state->registers.word[Z80_HL]



#if defined (__WXMAC__)
#define PTERM_MDI 1
#else
#define PTERM_MDI 0
#endif

#if PTERM_MDI
#define PtermFrameBase  wxMDIChildFrame

class PtermMainFrame : public wxMDIParentFrame
{
public:
    PtermMainFrame(void);
    wxMenuItem* AppendWinItem(int itemid,
        const wxString& text,
        const wxString& help = wxEmptyString,
        wxItemKind kind = wxITEM_NORMAL)
    {
        if (m_windowMenu != NULL)
            return m_windowMenu->Append(itemid, text, help, kind);
        else
            return NULL;
    }
    wxMenuBar   *menuBar;
    wxMenu      *menuFile;
    wxMenu      *menuHelp;
};

extern PtermMainFrame *PtermFrameParent;
#else
#define PtermFrameBase wxFrame
#define PtermFrameParent NULL
#endif

// Define a new frame type: this is going to be our main frame
class PtermFrame : public PtermFrameBase, public Z80
{
    friend void PtermCanvas::OnDraw(wxDC &dc);
    friend void PtermApp::OnHelpKeys(wxCommandEvent &event);
    friend void PtermApp::MacOpenFiles(const wxArrayString &s);
    friend int PtermConnection::NextWord(void);
    friend void PtermPrintout::DrawPage(wxDC *);
    friend void PtermCanvas::OnMouseDown(wxMouseEvent &event);
    friend void PtermCanvas::OnMouseMotion(wxMouseEvent &event);
    friend void PtermCanvas::OnMouseUp(wxMouseEvent &event);

public:
    // ctor(s)
    PtermFrame(const wxString &host, int port, const wxString& title,
        bool mtutorBoot = false);
    ~PtermFrame();

    // event handlers (these functions should _not_ be virtual)
    void OnIdle(wxIdleEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnMclock(wxTimerEvent& event);
    void OnDclock(wxTimerEvent& event);
    void OnMz80(wxTimerEvent& event);
    void OnPasteTimer(wxTimerEvent& event);
    void OnShellTimer(wxTimerEvent& event);
    void OnQuit(wxCommandEvent& event);
#if !defined (__WXMAC__)
    void OnToggleMenuBar(wxCommandEvent &event);
#endif
    void OnToggleStatusBar(wxCommandEvent &event);
    void OnSetScaleEntry(wxCommandEvent &event);
    void OnSetStretchMode(wxCommandEvent &event);
    void OnSetAspectMode(wxCommandEvent &event);
    void OnCopyScreen(wxCommandEvent &event);
    void OnCopy(wxCommandEvent &event);
    void OnExec(wxCommandEvent &event);
    void OnMailTo(wxCommandEvent &event);
    void OnSearchThis(wxCommandEvent &event);
    void OnMacro0(wxCommandEvent &event);
    void OnMacro1(wxCommandEvent &event);
    void OnMacro2(wxCommandEvent &event);
    void OnMacro3(wxCommandEvent &event);
    void OnMacro4(wxCommandEvent &event);
    void OnMacro5(wxCommandEvent &event);
    void OnMacro6(wxCommandEvent &event);
    void OnMacro7(wxCommandEvent &event);
    void OnMacro8(wxCommandEvent &event);
    void OnMacro9(wxCommandEvent &event);
    void OnPaste(wxCommandEvent &event);
    void OnUpdateUIPaste(wxUpdateUIEvent& event);
    void OnSaveScreen(wxCommandEvent &event);
    void OnSaveAudio(wxCommandEvent &event);
    void OnPrint(wxCommandEvent& event);
    void OnPrintPreview(wxCommandEvent& event);
    void OnPageSetup(wxCommandEvent& event);
    void OnPref(wxCommandEvent& event);
    void OnActivate(wxActivateEvent &event);
    void SavePreferences(void);
    void SetColors(wxColour &newfg, wxColour &newbg);
    void OnFullScreen(wxCommandEvent &event);
    void OnResize(wxSizeEvent& event);
    void OnReset(wxCommandEvent& event);

    void UpdateDisplayState(void);
#if defined (__WXMSW__)
    void OnIconize(wxIconizeEvent &event);
#endif

    void Mz80Waiter(int msec);
    void BootMtutor(void);
    void BuildMenuBar(void);
    void BuildFileMenu(int port);
    void BuildEditMenu(int port);
    // This one has a different signature because it is also used as
    // part of BuildPopupMenu
    void BuildViewMenu(wxMenu *menu, int port);
    void BuildHelpMenu(void);
    void BuildPopupMenu(int port);
    void BuildStatusBar(bool connecting = false);
    void ptermSendKey1(int key);
    void ptermSendKey(u32 keys);
    void ptermSendKeys(const int key[]);
    void ptermSendTouch(int x, int y);
    void ptermSendExt(int key);
    void ptermSetTrace(bool trace);
    void ProcessPlatoMetaData(void);
    void WriteTraceMessage(wxString);
#if 0
    // The s0ascers spec calls for parity but it isn't really needed
    // and by not doing it we keep things simpler.  For example,
    // that way we don't need to worry about telnet escaping
    // (0xff escaping) in the inbound direction.
    int Parity(int key);
#else
#define Parity(x) (x)
#endif

    bool HasConnection(void) const
    {
        return (m_conn != NULL);
    }
    bool IgnoreKeys(void) const
    {
        return (m_conn == NULL) && !m_mtutorBoot;
    }
    void ptermSetStatus(wxString &str);
    void ptermShowTrace();
    void trace(const wxString &) const;
    void trace(const char *, ...) const;

    wxMemoryDC  *m_memDC;
    bool        tracePterm;
    int         m_currentWord;      // to be displayed when tracing
    PtermFrame  *m_nextFrame;
    PtermFrame  *m_prevFrame;

    int         m_pendingEcho;

    bool        m_bCancelPaste;
    bool        m_bPasteActive;
    PtermConnection *m_conn;
    bool        m_dumbTty;

    wxMenuBar   *menuBar;           // present even if not displayed
    wxMenu      *menuFile;
    wxMenu      *menuEdit;
    wxMenu      *menuView;
    wxMenu      *menuHelp;
    wxMenu      *menuPopup;
    wxStatusBar *m_statusBar;       // NOT present if not displayed

    double      m_scale;
    bool        m_fullScreen;
    // These are the X and Y scale factors, derived from the scale mode
    // and full screen setting.
    float       m_xscale, m_yscale;
    // These are the current margins.  Usually those are simply DisplayMargin,
    // but in full screen mode they account for the entire space beyond the
    // actual display area.  They are in units of PLATO pixels, i.e., the
    // screen margins are this times m_xscale and m_yscale respectively.
    int         m_xmargin, m_ymargin;

    //fonts
    wxFont      *m_font;
    bool        m_usefont;
    wxFontFamily m_fontfamily;
    wxString    m_fontface;
    int         m_fontsize;
    bool        m_fontitalic;
    bool        m_fontbold;
    bool        m_fontstrike;
    bool        m_fontunderln;
    wxCoord     m_fontwidth;
    wxCoord     m_fontheight;
    bool        m_fontPMD;
    bool        m_fontinfo;
    //operating system request
    bool        m_osinfo;

    // GSW sound output file handling
    wxString    m_gswFile;
    int         m_gswFFmt;
    MTFile      m_MTFiles[2];


    bool procPlatoWord(u32 d, bool ascii);
    PtermCanvas *m_canvas;
    wxBitmap    *m_bitmap;
    void        ptermSetConnected(void);
    wxString    m_station;

private:
    bool        m_needtoBoot;
    int         m_lastKey;
    u32         m_fgpix;
    u32         m_bgpix;
    u32         m_maxalpha;
    u32         m_selpixf;
    u32         m_selpixb;
    wxBitmap    *m_selmap;
    wxBitmap    *m_bitmap2;
    u32         m_red;
    u32         m_green;
    u32         m_blue;
    wxString    m_curProfile;
    wxString    m_ShellFirst;
    wxString    m_hostName;
    long        m_port;
    wxTimer     m_timer;
    wxTimer     m_Mclock;
    wxTimer     m_Dclock;
    wxTimer     m_MReturnz80;
    //long        m_mtutorLevel;
    u8          m_indev;        // input device
    u8          m_outdev;       // output device
    u8          m_mtincnt;      // counter
    u8          m_mtdrivetemp;  // temp drive function - before accept
    u8          m_mtdrivefunc;  // drive function
    u8          m_mtcanresp;    // canned response for control port
    u8          m_mtsingledata; // single byte data preset for data channel

    int         m_mtDataPhase;          // 1..7
    u8          m_mtDiskUnit;
    u8          m_mtDiskTrack;
    u8          m_mtDiskSector;
    u8          m_mtDisk1;
    u8          m_mtDisk2;
    u8          m_mtDiskCheck1;
    u8          m_mtDiskCheck2;
    int         m_mtSeekPos;
    bool        m_clockPhase;   // lower or upper byte of clock

    wxRect      m_rect;         // Used to save window size/pos

                                // Session information received from the other end (mostly
                                // in metadata frames)
    wxString    m_name;
    wxString    m_group;
    wxString    m_system;

    // Stuff for pacing Paste operations
    wxTimer     m_pasteTimer;
    wxString    m_pasteText;
    int         m_pasteIndex;
    int         m_pasteNextIndex;
    int         m_pasteLen;
    bool        m_pastePrint;
    int         m_pasteLinePos;

    // The next word to be processed, and its associated delay.
    // We set this if we pick up a word from the connection object, and
    // either it comes with an associated delay, or "true timing" is selected
    // via preferences, or GSW emulation is active.
    // Delay codes are also sent by the formatter after a block erase; those
    // we will ignore since we don't need them -- they are there to cope with
    // the rather slow operation of block erase on real terminals.
    int         m_nextword;
    int         m_delay;
    bool        m_ignoreDelay;

    // PLATO terminal emulation state
#define mode RAM[M_MODE]
    // key switch var for mtutor/ppt progs
#define mt_ksw RAM[M_KSW]

#define mjobs  RAM[M_JOBS]

#define m_enab RAM[M_ENAB]

#define key2mtutor ((mt_ksw & 1) == 1)      // direct keys to mtutor/ppt

    i16         mt_key;

    bool        modexor;
    // 0: inverse
    // 1: rewrite
    // 2: erase
    // 3: write
#define wemode (mode & 3)
    int         currentX;
    int         currentY;
#define margin ReadRAMW (M_MARGIN)
    int         memaddr;
    u16         plato_m23[128 * 8];
    int         memlpc;
#define uncover     ((RAM[M_CCR] & 0x80) != 0)
#define reverse     ((RAM[M_CCR] & 0x40) != 0 )
#define large       ((RAM[M_CCR] & 0x20) != 0)
#define currentCharset ((RAM[M_CCR] & 0x0e) >> 1)
#define vertical    ((RAM[M_CCR] & 0x01) != 0)
    int         wc;
    int         seq;
    int         modewords;
    int         mode4start;
    typedef enum {
        none, ldc, lde, lda, ssf, fg, bg, gsfg, paint,
        pni_rs, ext, pmd
    } AscState;
    AscState    m_ascState;
    int         m_ascBytes;
    int         m_assembler;
    int         lastX;
    int         lastY;
    bool        m_flowCtrl;
    bool        m_sendFgt;

    wxColour    m_defFg;
    wxColour    m_defBg;
    wxColour    m_currentFg;
    wxColour    m_currentBg;

    bool        m_loadingPMD;
    wxString    m_PMD;

    // CYBIS workstation windowing defines
#define CWS_SAVE        0
#define CWS_RESTORE     1
#define CWS_EXEC        1012
#define CWS_TERMSAVE    2000
#define CWS_TERMRESTORE 2001
    int     cwsmode, cwsfun, cwscnt, cwswin;
    struct  cws
    {
        bool        ok;
        int         data[4];
        wxBitmap    *bm;
    };
    cws cwswindow[10];

    void setMargin(int i)
    {
        RAM[M_MARGIN] = i;
        RAM[M_MARGIN + 1] = i >> 8;
    }
    void setUncover(bool u)
    {
        if (u)
        {
            RAM[M_CCR] |= 0x80;
        }
        else
        {
            RAM[M_CCR] &= ~0x80;
        }
    }
    void setReverse(bool u)
    {
        if (u)
        {
            RAM[M_CCR] |= 0x40;
        }
        else
        {
            RAM[M_CCR] &= ~0x40;
        }
    }
    void setLarge(bool u)
    {
        if (u)
        {
            RAM[M_CCR] |= 0x20;
        }
        else
        {
            RAM[M_CCR] &= ~0x20;
        }
    }
    void setCmem(int i)
    {
        RAM[M_CCR] = (RAM[M_CCR] & ~0x0e) | (i << 1);
    }
    void setVertical(bool u)
    {
        if (u)
        {
            RAM[M_CCR] |= 0x01;
        }
        else
        {
            RAM[M_CCR] &= ~0x01;
        }
    }

    // PLATO drawing primitives
    void ptermDrawChar(int x, int y, int snum, int cnum, bool autobs = false);
    void ptermDrawCharInto(int x, int y, const u16 *charp,
        u32 fpix, u32 bpix, int cmode,
        bool xor_p, PixelData &pixmap);
    void ptermDrawPoint(int x, int y);
#ifdef __WXMSW__
    void fixAlpha(void);
#endif
    inline void ptermUpdatePoint(int x, int y, u32 pixval, bool xor_p,
        PixelData & pixmap);
    void ptermDrawLine(int x1, int y1, int x2, int y2);
    void ptermFullErase(void);
    void ptermBlockErase(int x1, int y1, int x2, int y2);
    void ptermSetName(wxString &winName);
    void ptermPaint(int pat);
    void ptermPaintWalker(int x, int y, PixelData & pixmap,
        int pat, int pass);
    void ptermSaveWindow(int d);
    void ptermRestoreWindow(int d);

    void drawFontChar(int x, int y, int c);
    void procDataLoop(void);
    void plotChar(int c);
    void mode0(u32 d);
    void mode1(u32 d);
    void mode2(u32 d);
    void mode3(u32 d);
    void mode4(u32 d);
    void mode5(u32 d);
    void mode6(u32 d);
    void mode7(u32 d);
    void progmode(u32 d, int origin);
    void ptermUpdateTitle(void);

    bool AssembleCoord(int d);
    int AssemblePaint(int d);
    int AssembleData(int d);
    int AssembleColor(int d);
    int AssembleGrayScale(int d);

    int AssembleAsciiPlatoMetaData(int d);
    bool AssembleClassicPlatoMetaData(int d);
    void SetFontFaceAndFamily(int n);
    void SetFontSize(int n);
    void SetFontFlags(int n);
    void SetFontActive(void);

    typedef void (PtermFrame::*mptr)(u32);

    static const mptr modePtr[8];

    // Text-copy support
    bool SaveChar(int x, int y, wxChar c, bool large_p);
    void ClearRegion(void);
    void UpdateRegion(int x, int y, int mousex, int mousey);
    wxString GetRegionText(bool url = false) const;

    cmentry textmap[32 * 64];
    int m_regionX;
    int m_regionY;
    int m_regionHeight;
    int m_regionWidth;
    bool m_autobs;

    // z80 emulation support
    u8 inputZ80(u8 data);
    void outputZ80(u8 data, u8 acc);
    int check_pcZ80(void);

    const char* resCallName(u16 pc);

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()
};


#endif  // __FrameCanvas_H__
