/////////////////////////////////////////////////////////////////////////////
// Name:        pterm_wx.cpp
// Purpose:     pterm interface to wxWindows 
// Authors:     Paul Koning, Joe Stanton
// Modified by: 
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton
// Licence:     DtCyber license
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

#if defined(__WXMSW__)
#define wxUse_DC_Cache	-1
#endif
#define AUTOSCALE	"to use old way of scaling comment this line out"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define DEFAULTHOST		wxT ("cyberserv.org")
#define CURRENT_PROFILE	wxT (" Current ")
#define BufSiz			256
#define RINGSIZE		5000
#define RINGXON1		(RINGSIZE/3)
#define RINGXON2		(RINGSIZE/4)
#define RINGXOFF1		(RINGSIZE-RINGXON1)
#define RINGXOFF2		(RINGSIZE-RINGXON2)
#define xonkey			01606
#define xofkey			01607
#define ascxon			0x11
#define ascxof			0x13

#define GSWRINGSIZE 100

#define C_NODATA        -1
#define C_CONNFAIL      -2
#define C_DISCONNECT    -3
#define C_GSWEND        -4

#define STATUS_TIP      0
#define STATUS_TRC      1
#define STATUS_CONN     2
#define STATUSPANES     3

#define KeyBufSize      50
#define DisplayMargin   8

#define MouseTolerance  3       // # pixels tolerance before we call it "drag"

// Delay parameters for text paste (in ms)
#define PASTE_CHARDELAY 165
#define PASTE_LINEDELAY 500

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for the character sets (ROM and loadable)
#ifdef AUTOSCALE
	#define vXSize(s)		(512 + (2 * DisplayMargin))
	#define vYSize(s)		(512 + (2 * DisplayMargin))
	#define vCharXSize(s)	512
	#define vCharYSize(s)	(CSETS * 16)
	#define tvCharYSize(s)	(4 * 16)
	#define vScreenSize(s)  512
#else
	#define vXSize(s)       (512 * (s) + (2 * DisplayMargin))
	#define vYSize(s)       (512 * (s) + (2 * DisplayMargin))
	#define vCharXSize(s)   (512 * (s))
	#define vCharYSize(s)   ((CSETS * 16) * (s))
	#define tvCharYSize(s)  ((4 * 16) * (s))
	#define vScreenSize(s)  (512 * (s))
#endif
#define vXRealSize(s)		(512 * (s) + (2 * DisplayMargin))
#define vYRealSize(s)		(512 * (s) + (2 * DisplayMargin))
#define vRealScreenSize(s)  (512 * (s))

//#define XSize           (vYSize (ptermApp->m_scale))
//#define YSize           (vXSize (ptermApp->m_scale))
//#define CharXSize       (vCharXSize (ptermApp->m_scale))
//#define CharYSize       (vCharYSize (ptermApp->m_scale))
//#define ScreenSize      (vRealScreenSize (ptermApp->m_scale))

#define TERMTYPE        10
#define ASCTYPE         12
//#define SUBTYPE         0x74	// original value, 116=portal
#define SUBTYPE         1
//#define TERMCONFIG      0
#define TERMCONFIG      0x40    // touch panel present
//#define TERMCONFIG      0x60    // touch panel present
#define	ASC_ZFGT		0x01
#define	ASC_ZPCKEYS		0x02
#define	ASC_ZKERMIT		0x04
#define	ASC_ZWINDOW		0x08
#define ASCFEATURES     ASC_ZFGT | ASC_ZWINDOW
//#define ASCFEATURES     ASC_ZFGT
//#define ASCFEATURES     ASC_ZFGT
//#define ASCFEATURES     0x01    // features: fine touch
//#define ASCFEATURES     0x05   // features: fine touch=0x01, kermit=0x04

//special nop masks/codes
#define	NOP_MASK		01700000	// mask off plato word except nop bits
#define	NOP_MASKDATA	  077000	// mask bits 9-14, gives 6 bits for special data in a NOP code; lower 9 bits still available for meta data
#define	NOP_SETSTAT		  042000	// set station number
#define	NOP_PMDSTART	  043000	// start streaming "Plato Meta Data"; note, lowest 6 bits is a character
#define	NOP_PMDSTREAM	  044000	// stream "Plato Meta Data"
#define	NOP_PMDSTOP		  045000	// stop streaming "Plato Meta Data"
#define	NOP_FONTTYPE	  050000	// font type
#define	NOP_FONTSIZE	  051000	// font size
#define	NOP_FONTFLAG	  052000	// font flags
#define	NOP_FONTINFO	  053000	// get last font character width/height
#define	NOP_OSINFO	      054000	// get operating system type, returns 1=mac, 2=windows, 3=linux

// Literal strings for wxConfig key strings.  These are defined
// because they appear in two places, so this way we avoid getting
// the two out of sync.  Note that they should *not* be changed after
// once being defined, since that invalidates people's stored
// preferences.
#define PREF_LASTTAB	 "lastTab"
#define PREF_XPOS        "xPosition"
#define PREF_YPOS        "yPosition"
//tab0
#define PREF_CURPROFILE  "curProfile"
//tab1
#define PREF_SHELLFIRST  "ShellFirst"
#define PREF_CONNECT     "autoconnect"
#define PREF_HOST        "host"
#define PREF_PORT        "port"
//tab2
#define PREF_SHOWSIGNON  "ShowSignon"
#define PREF_SHOWSYSNAME "ShowSysName"
#define PREF_SHOWHOST    "ShowHost"
#define PREF_SHOWSTATION "ShowStation"
//tab3
#define PREF_1200BAUD    "classicSpeed"
#define PREF_GSW         "gswenable"
#define PREF_ARROWS      "numpadArrows"
#define PREF_IGNORECAP   "ignoreCapLock"
#define PREF_PLATOKB     "platoKeyboard"
#define PREF_ACCEL       "UseAccel"
#define PREF_BEEP        "beep"
#define PREF_SHIFTSPACE  "DisableShiftSpace"
#define PREF_MOUSEDRAG   "DisableMouseDrag"
//tab4
#define PREF_SCALE       "scale"
#define PREF_STATUSBAR   "statusbar"
#define PREF_MENUBAR	 "menubar"
#define PREF_NOCOLOR     "noColor"
#define PREF_FOREGROUND  "foreground"
#define PREF_BACKGROUND  "background"
//tab5
#define PREF_CHARDELAY   "charDelay"
#define PREF_LINEDELAY   "lineDelay"
#define PREF_AUTOLF      "autoLF"
#define PREF_SPLITWORDS  "splitWords"
#define PREF_SMARTPASTE  "smartPaste"
#define PREF_CONVDOT7    "convDot7"
#define PREF_CONV8SP     "conv8Sp"
#define PREF_TUTORCOLOR  "TutorColor"
//tab6
#define PREF_BROWSER     "Browser"
#define PREF_EMAIL       "EmailClient"
#define PREF_SEARCHURL   "SearchURL"
//rostering
#define	PREF_ROSTERFILE  "RosterFile"

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define TRACEX(str,arg)                                             \
        fprintf (traceF, str "\n",arg);  \

#define TRACEN(str)                                             \
    if (tracePterm)                                             \
        {                                                       \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc);  \
        }

#define TRACE(str, arg)                                             \
    if (tracePterm)                                                 \
        {                                                           \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg); \
        }

#define TRACE2(str, arg, arg2)                                          \
    if (tracePterm)                                                     \
        {                                                               \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg, arg2); \
        }

#define TRACE3(str, arg, arg2, arg3)                                    \
    if (tracePterm)                                                     \
        {                                                               \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, arg, arg2, arg3); \
        }

#define TRACE4(str, a, a2, a3, a4)                                      \
    if (tracePterm)                                                     \
        {                                                               \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, a, a2, a3, a4); \
        }

#define TRACE6(str, a, a2, a3, a4, a5, a6)                              \
    if (tracePterm)                                                     \
        {                                                               \
        fprintf (traceF, "seq %6d wc %3d " str "\n", seq, wc, a, a2, a3, a4, a5, a6); \
        }

// Map PLATO coordinates to window coordinates
#ifdef AUTOSCALE
	#define XADJUST(x) ((x) + GetXMargin ())
	#define YADJUST(y) ((511 - (y)) + GetYMargin ())
#else
	#define XADJUST(x) ((x) * m_scale + GetXMargin ())
	#define YADJUST(y) ((511 - (y)) * m_scale + GetYMargin ())
#endif

// Map PLATO coordinates to backing store bitmap coordinates
#ifdef AUTOSCALE
	#define XMADJUST(x) (x)
	#define YMADJUST(y) (511 - (y))
#else
	#define XMADJUST(x) ((x) * m_scale)
	#define YMADJUST(y) (511 - (y) * m_scale)
#endif

// inverse mapping (for processing touch input)
#ifdef AUTOSCALE
	#define XUNADJUST(x) ((x) - GetXMargin ())
	#define YUNADJUST(y) (511 - ((y) - GetYMargin ()))
#else
	#define XUNADJUST(x) (((x) - GetXMargin ()) / m_scale)
	#define YUNADJUST(y) (511 - ((y) - GetYMargin ()) / m_scale)
#endif

// force coordinate into the range 0..511
#define BOUND(x) x = (((x < 0) ? 0 : ((x > 511) ? 511 : x)))

// Define top left corner of the PLATO drawing area, in windowing
// system coordinates (which have origin at the top and Y axis upside down)
#define XTOP (XADJUST (0))
#define YTOP (YADJUST (511))
// Ditto but for the backing store bitmap
#define XMTOP (XMADJUST (0))
#define YMTOP (YMADJUST (511))

// Macro to include keyboard accelerator only if option enabled
#define ACCELERATOR(x) + ((ptermApp->m_useAccel) ? wxString(wxT (x)) : wxString(wxT("")))
// Macro to include keyboard accelerator only if MAC
//#if defined(__WXMAC__)
//#define MACACCEL(x) + wxString (wxT (x))
//#else
//#define v(x)
//#endif

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/button.h>
#include "wx/clipbrd.h"
#include "wx/colordlg.h"
#include "wx/config.h"
#include "wx/dir.h"
#include "wx/image.h"
#include "wx/file.h"
#include "wx/filename.h"
#include "wx/filefn.h"
#include "wx/metafile.h"
#include "wx/notebook.h"
#include "wx/print.h"
#include "wx/printdlg.h"
#include "wx/textfile.h"
#include "wx/utils.h"

extern "C"
{
#if defined(_WIN32)
#include <winsock.h>
#include <process.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#endif
#include <stdlib.h>
#include "const.h"
#include "types.h"
#include "proto.h"
#include "ptermversion.h"
#include "8080a.h"
#include "8080avar.h"
#include "ppt.h"

#if wxUSE_LIBGNOMEPRINT
#include "wx/html/forcelnk.h"
//FORCE_LINK(gnome_print)
#endif

// For wxWidgets 2.8
#ifdef  _WX_PLATINFO_H_
#ifndef wxMAC
#define wxMAC wxOS_MAC_OS
#endif
#ifndef wxMAC_DARWIN
#define wxMAC_DARWIN wxOS_MAC_OSX_DARWIN
#endif
#endif

extern int ptermOpenGsw (void *user);
extern int ptermProcGswData (int data);
extern void ptermCloseGsw (void);
extern void ptermStartGsw (void);

#if defined (__WXGTK__)

// Attempting to include the gtk.h file yields infinite compile errors, so
// instead we just declare the two functions we need, leaving any 
// issues of data structures unstated.

struct GtkSettings;
    
extern void gtk_settings_set_string_property (GtkSettings *, const char *,
                                              const char *, const char *);
extern GtkSettings * gtk_settings_get_default (void);
#endif
}
    
// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources)
#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__) || defined(__WXMGL__) || defined(__WXX11__)
#include "pterm_32.xpm"
#endif

static const u32 keyboardhelp[] = {
#include "ptermkeys.h"
};

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

bool emulationActive = false;

// Global print data, to remember settings during the session
wxPrintData *g_printData;

// Global page setup data
wxPageSetupDialogData* g_pageSetupData;

// ----------------------------------------------------------------------------
// local variables
// ----------------------------------------------------------------------------

class PtermApp;
static PtermApp *ptermApp;

static FILE *traceF;
static char traceFn[20];

#ifdef DEBUGLOG
static wxLogWindow *logwindow;
#endif

static const wxChar rom01char[] =
    wxT(":abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.\xF7[]%\xD7\xAB'\"!;<>_?\xBB "
        "#ABCDEFGHIJKLMNOPQRSTUVWXYZ~\xA8^\xB4`    ~    {}&  |\xB0     \xB5       @\\ ");

// high bit is M0/M1 selector (into the classic ROM)
// 0xff means unused code.  The other codes 0xf0 and up
// are specials:
// 0xf0: embed left
// 0xf1: embed right
// 0xf2: copyright body
// 0xf3: box
// 0xf4: diamond
// 0xf5: cross product
// 0xf6: hacek
// 0xf7: universal delimiter
// 0xf8: dot product
// 0xf9: cedilla
static const u8 asciiM0[] = 
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x2d, 0x38, 0x37, 0x80, 0x2b, 0x33, 0xab, 0x36,
  0x29, 0x2a, 0x27, 0x25, 0x2e, 0x26, 0x2f, 0x28,
  0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22,
  0x23, 0x24, 0x00, 0x39, 0x3a, 0x2c, 0x3b, 0x3d,
  0xbd, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9a, 0x31, 0xbe, 0x32, 0x9d, 0x3c,
  0x9f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0xa9, 0xae, 0xaa, 0xa4, 0xff
};
static const u8 asciiM1[] = 
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x2d, 0x28, 0xb0, 0x9b, 0x35, 0xac, 0xa0, 0xa1,
  0xa2, 0xa3, 0x34, 0xa5, 0xa6, 0xa7, 0xa8, 0x30,
  0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
  0xb9, 0xba, 0xbb, 0xbc, 0xf0, 0xaf, 0xf1, 0x3e,
  0xf2, 0x9c, 0xf3, 0xf8, 0xf4, 0xf5, 0x9e, 0xf9,
  0xf6, 0xf7, 0xae, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const u8 asciiKeycodes[] =
{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x26, 0x60, 0x0a, 0x5e, 0x2b, 0x2d,
  0x13, 0x04, 0x07, 0x08, 0x7b, 0x0b, 0x0d, 0x1a,
  0x02, 0x12, 0x01, 0x03, 0x7d, 0x0c, 0xff, 0xff,
  0x3c, 0x3e, 0x5b, 0x5d, 0x24, 0x25, 0x5f, 0x7c,
  0x2a, 0x28, 0x40, 0x27, 0x1c, 0x5c, 0x23, 0x7e,
  0x17, 0x05, 0x14, 0x19, 0x7f, 0x09, 0x1e, 0x18,
  0x0e, 0x1d, 0x11, 0x16, 0x00, 0x0f, 0xff, 0xff,
  0x20, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7a, 0x3d, 0x3b, 0x2f, 0x2e, 0x2c,
  0x1f, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5a, 0x29, 0x3a, 0x3f, 0x21, 0x22
};

// These are the strings to use for the "special" characters.
// On classic terminals, the formatter generates these from
// the regular ROM characters with a sequence of backspaces and
// the like.  In ASCII mode, we have to do it; we do it
// roughly the same way.  Each row is indexed by the
// special char code - 0xf0.  Each entry is a pair:
//  char code, Y offset.  Char code 0 is the terminator.
// All are plotted at the same X implicitly.
static const u8 M1specials[][8] =
{ { 0x3a, 0, 0x29, 0, 0 },              // embed left
  { 0x3b, 0, 0x2a, 0, 0 },              // embed right
  { 0x3c, 0, 0x3c, 11, 0x03, 1, 0 },    // copyright body
  { 0xa7, 0, 0xa8, 0, 0 },              // box
  { 0xa0, 0, 0xa3, 0, 0xa2, 0, 0 },     // diamond
  { 0x28, 0, 0xbe, 0, 0 },              // cross product
  { 0x9e, 0, 0x9f, 0, 0 },              // hacek
  { 0xa0, 0, 0xa2, 0, 0 },              // universal delimiter
  { 0xaf, (u8) -3, 0 },                 // dot product
  { 0x9e, (u8) -11, 0 },                // cedilla
};

// Conversion from ascii mode codes to classic codes
static const u8 ascmode[] = { 0, 3, 2, 1 };

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

class PtermFrame;

// Pterm screen printout
class PtermPrintout: public wxPrintout
{
public:
    PtermPrintout (PtermFrame *owner, const wxString &title = _("Pterm printout")) 
        : wxPrintout (title),
          m_owner (owner)
    {}
    bool OnPrintPage (int page);
    bool HasPage (int page);
    bool OnBeginDocument (int startPage, int endPage);
    void GetPageInfo (int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
    void DrawPage (wxDC *dc);
    inline int GetXMargin (void) const;
    inline int GetYMargin (void) const;

private:
    PtermFrame *m_owner;
};


// Pterm processing thread
class PtermConnection : public wxThread
{
public:
    // override base class virtuals
    // ----------------------------
    PtermConnection (PtermFrame *owner, wxString &host, int port);
    ~PtermConnection ();
    
    virtual ExitCode Entry (void);

    int AssembleNiuWord (void);
    int AssembleAsciiWord (void);
    int AssembleAutoWord (void);
    int NextWord (void);
    int NextGswWord (bool catchup);
    
    void SendData (const void *data, int len);

    bool IsEmpty (void) const
    {
        return (m_displayIn == m_displayOut);
    }
    bool IsFull (void) const
    {
        int next;
    
        next = m_displayIn + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
        return (next == m_displayOut);
    }
    int RingCount (void) const
    {
        if (m_displayIn >= m_displayOut)
        {
            return m_displayIn - m_displayOut;
        }
        else
        {
            return RINGSIZE + m_displayIn - m_displayOut;
        }
    }
    bool Ascii (void) const
    {
        return (m_connMode == ascii);
    }
    void StoreWord (int word);

private:
    NetPortSet  m_portset;
    NetFet      *m_fet;
    u32         m_displayRing[RINGSIZE];
    volatile int m_displayIn, m_displayOut;
    u32         m_gswRing[GSWRINGSIZE];
    volatile int m_gswIn, m_gswOut;
    PtermFrame  *m_owner;
    wxString    m_hostName;
    int         m_port;
    wxCriticalSection m_pointerLock;
    bool        m_gswActive;
    bool        m_gswStarted;
    int         m_savedGswMode;
    int         m_gswWord2;
    enum { both, niu, ascii } m_connMode;
    int         m_pending;
    
    int NextRingWord (void);
};

extern "C" int ptermNextGswWord (void *connection, int catchup);

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
	int	m_connError;
	int	m_connAction;
	
	bool LoadProfile (wxString profile, wxString filename);
	wxString ProfileFileName (wxString profile);

    static wxColour SelectColor (wxWindow &parent, const wxChar *title, 
                                 wxColour &initcol );
    
    wxConfig    *m_config;

	//general
	int			m_lastTab;
	//tab0
	wxString	m_curProfile;
	//tab1
	wxString	m_ShellFirst;
    bool        m_connect;
    wxString    m_hostName;
    int         m_port;
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
	bool		m_useAccel;
    bool        m_beepEnable;
    bool        m_DisableShiftSpace;
    bool        m_DisableMouseDrag;
	//tab4
    int         m_scale;
    bool        m_showStatusBar;
#if !defined(__WXMAC__)
    bool        m_showMenuBar;
#endif
    bool        m_noColor;
    wxColour    m_fgColor;
    wxColour    m_bgColor;
	//tab5
    wxString    m_charDelay;
    wxString    m_lineDelay;
    wxString    m_autoLF;
	bool		m_splitWords;
	bool		m_smartPaste;
    bool        m_convDot7;
    bool        m_conv8Sp;
    bool        m_TutorColor;
	//tab6
	wxString	m_Browser;      
	wxString	m_Email;      
	wxString	m_SearchURL;      
	//rostering
	wxString	m_RosterFile;      

    PtermFrame  *m_firstFrame;
    wxString    m_defDir;
    PtermFrame  *m_helpFrame;

    int lastX;
    int lastY;

	PtermFrame *m_CurFrame;
	int m_CurFrameScreenSize;
	int m_CurFrameScale;

	bool		m_RosterMonitor;

private:
    wxLocale    m_locale; // locale we'll be using
    
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

// define a scrollable canvas for drawing onto
class PtermCanvas : public wxScrolledWindow
{
public:
    PtermCanvas (PtermFrame *parent);

    void ptermTouchPanel(bool enable);

    void OnDraw (wxDC &dc);
    void OnKeyDown (wxKeyEvent& event);
    void OnChar (wxKeyEvent& event);
    void OnMouseDown (wxMouseEvent &event);
    void OnMouseUp (wxMouseEvent &event);
    void OnMouseContextMenu (wxMouseEvent &event);
    void OnMouseMotion (wxMouseEvent &event);
	void OnMouseWheel (wxMouseEvent &event);
    void OnCopy (wxCommandEvent &event);

#if defined (__WXMSW__)
    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif
    
    void SaveChar (int x, int y, int snum, int cnum, int w, bool large_p);
    void FullErase (void);
    void ClearRegion (void);
    void UpdateRegion (wxMouseEvent &event);

    inline int GetXMargin (void) const;
    inline int GetYMargin (void) const;

    // Text-copy support
    u16  textmap[32 * 64];
    int m_regionX;
    int m_regionY;
    int m_regionHeight;
    int m_regionWidth;
    int m_mouseX;
    int m_mouseY;
	int	m_scale;
    
private:
    PtermFrame *m_owner;
    bool        m_touchEnabled;

    DECLARE_EVENT_TABLE ()
};

#if defined(__WXMAC__)
#define PTERM_MDI 1
#else
#define PTERM_MDI 0
#endif

#if PTERM_MDI
#define PtermFrameBase  wxMDIChildFrame

class PtermMainFrame : public wxMDIParentFrame
{
public:
    PtermMainFrame (void);
    wxMenuItem* AppendWinItem (int itemid,
                               const wxString& text,
                               const wxString& help = wxEmptyString,
                               wxItemKind kind = wxITEM_NORMAL)
    {
        if (m_windowMenu != NULL)
            return m_windowMenu->Append (itemid, text, help, kind);
        else
            return NULL;
    }
	wxMenuBar   *menuBar;
	wxMenu      *menuFile;
	wxMenu      *menuHelp;
};

static PtermMainFrame *PtermFrameParent;
#else
#define PtermFrameBase wxFrame
#define PtermFrameParent NULL
#endif

// Define a new frame type: this is going to be our main frame
class PtermFrame : public PtermFrameBase, public emul8080
{
    friend void PtermCanvas::OnDraw(wxDC &dc);
    friend void PtermApp::OnHelpKeys (wxCommandEvent &event);
    friend int PtermConnection::NextWord (void);
public:
    // ctor(s)
    PtermFrame(wxString &host, int port, const wxString& title, const wxPoint &pos = wxDefaultPosition);
    ~PtermFrame ();

    // event handlers (these functions should _not_ be virtual)
    void OnIdle (wxIdleEvent& event);
    void OnClose (wxCloseEvent& event);
    void OnTimer (wxTimerEvent& event);
    void OnPasteTimer (wxTimerEvent& event);
    void OnShellTimer (wxTimerEvent& event);
    void OnQuit (wxCommandEvent& event);
#if !defined(__WXMAC__)
    void OnToggleMenuBar (wxCommandEvent &event);
#endif
    void OnToggleStatusBar (wxCommandEvent &event);
    void OnToggle2xMode (wxCommandEvent &event);
    void OnCopyScreen (wxCommandEvent &event);
    void OnCopy (wxCommandEvent &event);
    void OnExec (wxCommandEvent &event);
    void OnMailTo (wxCommandEvent &event);
    void OnSearchThis (wxCommandEvent &event);
    void OnSpellCheck (wxCommandEvent &event);
    void OnMacro0 (wxCommandEvent &event);
    void OnMacro1 (wxCommandEvent &event);
    void OnMacro2 (wxCommandEvent &event);
    void OnMacro3 (wxCommandEvent &event);
    void OnMacro4 (wxCommandEvent &event);
    void OnMacro5 (wxCommandEvent &event);
    void OnMacro6 (wxCommandEvent &event);
    void OnMacro7 (wxCommandEvent &event);
    void OnMacro8 (wxCommandEvent &event);
    void OnMacro9 (wxCommandEvent &event);
    void OnPaste (wxCommandEvent &event);
    void OnUpdateUIPaste (wxUpdateUIEvent& event);
    void OnSaveScreen (wxCommandEvent &event);
    void OnPrint (wxCommandEvent& event);
    void OnPrintPreview (wxCommandEvent& event);
    void OnPageSetup (wxCommandEvent& event);
    void OnPref (wxCommandEvent& event);
    void OnActivate (wxActivateEvent &event);
    void UpdateSettings (wxColour &newfg, wxColour &newbf, bool newscale2);
    void SavePreferences (void);
    void SetColors (wxColour &newfg, wxColour &newbg, int newscale);
    void FixTextCharMaps (void);
    void OnFullScreen (wxCommandEvent &event);
#if defined (__WXMSW__)
    void OnIconize(wxIconizeEvent &event);
#endif

    void BuildMenuBar (void);
    void BuildEditMenu (int port);
    void BuildPopupMenu (int port);
	void BuildStatusBar (void);
    void SendRoster (void);
    void KillRoster (void);
    void PrepareDC(wxDC& dc);
    void ptermSendKey(int key);
    void ptermSendKeys(int key[]);
    void ptermSendTouch (int x, int y);
    void ptermSendExt (int key);
    void ptermSetTrace (bool fileaction);
	void ProcessPlatoMetaData(void);
	void WriteTraceMessage(wxString);
#if 0
    // The s0ascers spec calls for parity but it isn't really needed
    // and by not doing it we keep things simpler.  For example,
    // that way we don't need to worry about telnet escaping
    // (0xff escaping) in the inbound direction.
    int Parity (int key);
#else
#define Parity(x) (x)
#endif

    bool HasConnection (void) const
    {
        return (m_conn != NULL);
    }
    int GetXMargin (void) const
    {
        if (m_fullScreen)
        {
            int i = GetClientSize ().x - vRealScreenSize(m_scale);
            return (i < 0) ? 0 : i / (2 * m_scale);
        }
        else
            return DisplayMargin;
    }
    int GetYMargin (void) const
    {
        if (m_fullScreen)
        {
            int i = GetClientSize ().y - vRealScreenSize(m_scale);
            return (i < 0) ? 0 : i / (2 * m_scale);
        }
        else
            return DisplayMargin;
    }
    
    wxMemoryDC  *m_memDC;
    bool        tracePterm;
    PtermFrame  *m_nextFrame;
    PtermFrame  *m_prevFrame;

    int         m_pendingEcho;

	bool		m_bCancelPaste;
	bool		m_bPasteActive;
    PtermConnection *m_conn;
    bool        m_dumbTty;

	wxMenuBar   *menuBar;
	wxMenu      *menuFile;
	wxMenu      *menuEdit;
	wxMenu      *menuView;
	wxMenu      *menuHelp;
	wxMenu      *menuPopup;
    wxStatusBar *m_statusBar;       // present even if not displayed

	int			m_scale;

	//fonts
	wxFont		*m_font;
	bool		m_usefont;
	wxFontFamily m_fontfamily;
	wxString	m_fontface;
	int			m_fontsize;
	bool		m_fontitalic;
	bool		m_fontbold;
	bool		m_fontstrike;
	bool		m_fontunderln;
	wxCoord		m_fontwidth;
	wxCoord		m_fontheight;
	bool		m_fontPMD;
	bool		m_fontinfo;
	//operating system request
	bool		m_osinfo;

private:
    wxPen       m_foregroundPen;
    wxPen       m_backgroundPen;
    wxBrush     m_foregroundBrush;
    wxBrush     m_backgroundBrush;
    wxBitmap    *m_bitmap;
    PtermCanvas *m_canvas;
	wxString	m_curProfile;
	wxString	m_ShellFirst;
    wxString    m_hostName;
    int         m_port;
    wxTimer     m_timer;
    bool        m_fullScreen;
    int         m_station;
    
    // Stuff for pacing Paste operations
    wxTimer     m_pasteTimer;
    wxString    m_pasteText;
    int         m_pasteIndex;
    bool        m_pastePrint;
	int			m_pasteNextKeyCnt;

	//stuff for sending roster
    bool        m_SendRoster;
	bool		m_WaitReady;
    
    // Character patterns are stored in three DCs because we want to
    // BLIT them in various ways, and the OR/AND type ops don't work
    // the way you'd want for color bitmaps.  Monochrome bitmaps would
    // be great, except that they don't work in Windows...
    //
    // So what we do is this:
    // m_charmap[i] contains the character patterns for mode i -- 
    // the right color pixels where we want color, and black where
    // we leave the screen untouched.
    //
    // m_charmap[4] contains a mask -- white for the character background,
    // black for the character foreground.  So we AND with that to erase the
    // character pixels (we need to do that for modes erase and write only).
    //
    // m_charDC[i] is the device context into which m_charmap[i] is mapped.
    wxBitmap    *m_charmap[5];
    wxMemoryDC  *m_charDC[5];
	bool		m_dirty[4];	//dirty flag

    // The next word to be processed, and its associated delay.
    // We set this if we pick up a word from the connection object, and
    // either it comes with an associated delay, or "true timing" is selected
    // via preferences, or GSW emulation is active.
    int         m_nextword;
    int         m_delay;
    
    // PLATO terminal emulation state
#define mode RAM[M_MODE]
	bool		modexor;
#define wemode (mode & 3)
    int         currentX;
    int         currentY;
#define margin ReadRAMW (M_MARGIN)
    int         memaddr;
    u16         plato_m23[128 * 8];
    int         memlpc;
#define uncover     (RAM[M_CCR] & 0x80)
#define reverse     (RAM[M_CCR] & 0x40)
#define large       (RAM[M_CCR] & 0x20)
#define currentCharset ((RAM[M_CCR] & 0x0e) >> 1)
#define vertical    (RAM[M_CCR] & 0x01)
    int         wc;
    int         seq;
    int         modewords;
    int         mode4start;
    typedef enum { none, ldc, lde, lda, ssf, fg, bg, gsfg, paint, pni_rs, ext, pmd } AscState;
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

	bool		m_loadingPMD;
    wxString	m_PMD;

	// CYBIS workstation windowing defines
	#define	CWS_SAVE		0
	#define	CWS_RESTORE		1
	#define	CWS_EXEC		1012
	#define	CWS_TERMSAVE	2000
	#define	CWS_TERMRESTORE	2001
	int		cwsmode, cwsfun, cwscnt, cwswin;
	struct	cws
		{
		bool		ok;
		int			data[4];
		wxMemoryDC	*dc;
		wxBitmap	*bm;
		};
	cws cwswindow[10];

    void setMargin (int i)
    {
        RAM[M_MARGIN] = i;
        RAM[M_MARGIN + 1] = i >> 8;
    }
    void setUncover (bool u)
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
    void setReverse (bool u)
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
    void setLarge (bool u)
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
    void setCmem (int i)
    {
        RAM[M_CCR] = (RAM[M_CCR] & ~0x0e) | (i << 1);
    }
    void setVertical (bool u)
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
    

    void UpdateDC (wxMemoryDC *dc, wxBitmap *&bitmap, wxColour &newfg, wxColour &newbf, bool newscale2);

    // PLATO drawing primitives
    void ptermDrawChar (int x, int y, int snum, int cnum);
    void ptermDrawPoint (int x, int y);
    void ptermDrawLine(int x1, int y1, int x2, int y2);
    void ptermDrawBresenhamLine(wxMemoryDC *dc,int x1, int y1, int x2, int y2);
    void ptermFullErase (void);
    void ptermBlockErase (int x1, int y1, int x2, int y2);
    void ptermSetName (wxString &winName);
    void ptermSetStatus (wxString &str);
    void ptermLoadChar (int snum, int cnum, const u16 *data);
    void ptermLoadRomChars (void);
    void ptermPaint (int pat);
    void ptermSaveWindow (int d);
    void ptermRestoreWindow (int d);
    
    void drawChar (wxDC &dc, int x, int y, int snum, int cnum);
    void drawFontChar (int x, int y, int c);
    void procPlatoWord (u32 d, bool ascii);
    void plotChar (int c);
    void mode0 (u32 d);
    void mode1 (u32 d);
    void mode2 (u32 d);
    void mode3 (u32 d);
    void mode4 (u32 d);
    void mode5 (u32 d);
    void mode6 (u32 d);
    void mode7 (u32 d);
    void progmode (u32 d, int origin);
    void ptermSetStation (int station, bool showtitle, bool showstatus);
    void ptermShowTrace ();
    
    bool AssembleCoord (int d);
    int AssemblePaint (int d);
    int AssembleData (int d);
    int AssembleColor (int d);
    int AssembleGrayScale (int d);

    int AssembleAsciiPlatoMetaData (int d);
    bool AssembleClassicPlatoMetaData (int d);
	void SetFontFaceAndFamily(int n);
	void SetFontSize(int n);
	void SetFontFlags(int n);
	void SetFontActive(void);
    
    typedef void (PtermFrame::*mptr)(u32);

    static const mptr modePtr[8];

    // 8080a emulation support
    Uint8 input8080a (Uint8 data);
    void output8080a (Uint8 data, Uint8 acc);
    int check_pc8080a (void);

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE ()
};

const PtermFrame::mptr PtermFrame::modePtr[8] = 
{
    &PtermFrame::mode0, &PtermFrame::mode1,
    &PtermFrame::mode2, &PtermFrame::mode3,
    &PtermFrame::mode4, &PtermFrame::mode5,
    &PtermFrame::mode6, &PtermFrame::mode7
};

// define the preferences dialog
class PtermPrefDialog : public wxDialog
{
public:
    PtermPrefDialog (PtermFrame *parent, wxWindowID id, const wxString &title, wxPoint pos,  wxSize size);

    //event sink handlers
	void OnButton (wxCommandEvent& event);
    void OnCheckbox (wxCommandEvent& event);
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
    void SetControlState(void);
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
	//wxCheckBox* chkZoom200;
	//wxCheckBox* chkStatusBar;
	//wxCheckBox* chkMenuBar;
	wxCheckBox* chkDisableColor;
#if defined(_WIN32)
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
	wxCheckBox* chkSplitWords;
	wxCheckBox* chkSmartPaste;
	wxCheckBox* chkConvertDot7;
	wxCheckBox* chkConvert8Spaces;
	wxCheckBox* chkTutorColor;
	//tab6
	wxTextCtrl* txtBrowser;
	wxTextCtrl* txtEmail;
	wxTextCtrl* txtSearchURL;
	//button bar
	wxButton* btnOK;
	wxButton* btnCancel;
	wxButton* btnDefaults;

	//properties	
	int				m_lastTab;
	//tab0
	wxString		m_curProfile;
	//tab1
	wxString		m_ShellFirst;
    bool            m_connect;
    wxString        m_host;
    wxString        m_port;
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
	bool			m_useAccel;
    bool            m_beepEnable;
    bool			m_DisableShiftSpace;
    bool			m_DisableMouseDrag;
	//tab4
    bool            m_scale2;
    bool            m_showStatusBar;
#if !defined(__WXMAC__)
    bool            m_showMenuBar;
#endif
    bool            m_noColor;
    wxColour        m_fgColor;
    wxColour        m_bgColor;
	//tab5
    wxString        m_charDelay;
    wxString        m_lineDelay;
    wxString        m_autoLF;
	bool			m_splitWords;
	bool			m_smartPaste;
    bool            m_convDot7;
    bool            m_conv8Sp;
	bool			m_TutorColor;
	//tab6
	wxString		m_Browser;
	wxString		m_Email;
	wxString		m_SearchURL;
    
private:
    void paintBitmap (wxBitmap &bm, wxColour &color);
    
    PtermFrame *m_owner;

    DECLARE_EVENT_TABLE ()
};

// define the connection dialog
class PtermConnDialog : public wxDialog
{
public:
    PtermConnDialog (wxWindowID id, const wxString &title, wxPoint pos,  wxSize size);
    
    void OnButton (wxCommandEvent& event);
    void OnSelect (wxCommandEvent& event);
    void OnChange (wxCommandEvent& event);
    void OnDoubleClick (wxCommandEvent& event);
    void OnClose (wxCloseEvent &) { EndModal (wxID_CANCEL); }
    
	wxString		m_ShellFirst;
    wxString        m_curProfile;
    wxString        m_host;
    wxString        m_port;
	wxConfig		m_config;

	wxListBox* lstProfiles;
	wxTextCtrl* txtShellFirst;
	wxTextCtrl* txtHost;
	wxComboBox* cboPort;
	wxButton* btnCancel;
	wxButton* btnConnect;
    
private:
    DECLARE_EVENT_TABLE ()
};

// define the connection fail dialog
class PtermConnFailDialog : public wxDialog
{
public:
    PtermConnFailDialog (wxWindowID id, const wxString &title, wxPoint pos,  wxSize size);
    
    void OnButton (wxCommandEvent& event);
    void OnClose (wxCloseEvent& event);
    
	wxButton* btnNew;
	wxButton* btnRetry;
	wxButton* btnCancel;
    
private:
    DECLARE_EVENT_TABLE ()
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
	Pterm_ToggleMenuBar = 1,
	Pterm_ToggleMenuBarView,
	Pterm_ToggleStatusBar,
	Pterm_ToggleStatusBarView,
	Pterm_Toggle2xMode,
	Pterm_Toggle2xModeView,
    Pterm_CopyScreen,
    Pterm_ConnectAgain,
    Pterm_SaveScreen,
    Pterm_HelpKeys,
    Pterm_PastePrint,
    Pterm_FullScreen,
    Pterm_FullScreenView,

    // timers
    Pterm_Timer,        // display pacing
    Pterm_PasteTimer,   // paste key generation pacing
	//other items
    Pterm_Exec,			// execute URL
    Pterm_MailTo,		// execute email client
    Pterm_SearchThis,	// execute search URL
    Pterm_SpellCheck,	// execute spell checker
    Pterm_Macro0,
    Pterm_Macro1,
    Pterm_Macro2,
    Pterm_Macro3,
    Pterm_Macro4,
    Pterm_Macro5,
    Pterm_Macro6,
    Pterm_Macro7,
    Pterm_Macro8,
    Pterm_Macro9,

    // Menu items with standard ID values
    Pterm_Print = wxID_PRINT,
    Pterm_Page_Setup = wxID_PRINT_SETUP,
    Pterm_Preview = wxID_PREVIEW,
    Pterm_Copy = wxID_COPY,
    Pterm_Paste = wxID_PASTE,
    Pterm_Connect = wxID_NEW,
    Pterm_Quit = wxID_EXIT,
    Pterm_Close = wxID_CLOSE,
    Pterm_Pref = wxID_PREFERENCES,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Pterm_About = wxID_ABOUT
};

/* data for plato font, set 0. */
const unsigned short plato_m0[] = {
    0x0000, 0x0000, 0x0330, 0x0330, 0x0000, 0x0000, 0x0000, 0x0000, // :
    0x0060, 0x0290, 0x0290, 0x0290, 0x0290, 0x01e0, 0x0010, 0x0000, // a
    0x1ff0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x00c0, 0x0000, // b
    0x00c0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0210, 0x0120, 0x0000, // c
    0x00c0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x1ff0, 0x0000, // d
    0x00c0, 0x01a0, 0x0290, 0x0290, 0x0290, 0x0290, 0x0190, 0x0000, // e
    0x0000, 0x0000, 0x0210, 0x0ff0, 0x1210, 0x1000, 0x0800, 0x0000, // f
    0x01a8, 0x0254, 0x0254, 0x0254, 0x0254, 0x0194, 0x0208, 0x0000, // g
    0x1000, 0x1ff0, 0x0100, 0x0200, 0x0200, 0x0200, 0x01f0, 0x0000, // h
    0x0000, 0x0000, 0x0210, 0x13f0, 0x0010, 0x0000, 0x0000, 0x0000, // i
    0x0000, 0x0002, 0x0202, 0x13fc, 0x0000, 0x0000, 0x0000, 0x0000, // j
    0x1010, 0x1ff0, 0x0080, 0x0140, 0x0220, 0x0210, 0x0010, 0x0000, // k
    0x0000, 0x0000, 0x1010, 0x1ff0, 0x0010, 0x0000, 0x0000, 0x0000, // l
    0x03f0, 0x0200, 0x0200, 0x01f0, 0x0200, 0x0200, 0x01f0, 0x0000, // m
    0x0200, 0x03f0, 0x0100, 0x0200, 0x0200, 0x0200, 0x01f0, 0x0000, // n
    0x00c0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x00c0, 0x0000, // o
    0x03fe, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x00c0, 0x0000, // p
    0x00c0, 0x0120, 0x0210, 0x0210, 0x0210, 0x0120, 0x03fe, 0x0000, // q
    0x0200, 0x03f0, 0x0100, 0x0200, 0x0200, 0x0200, 0x0100, 0x0000, // r
    0x0120, 0x0290, 0x0290, 0x0290, 0x0290, 0x0290, 0x0060, 0x0000, // s
    0x0200, 0x0200, 0x1fe0, 0x0210, 0x0210, 0x0210, 0x0000, 0x0000, // t
    0x03e0, 0x0010, 0x0010, 0x0010, 0x0010, 0x03e0, 0x0010, 0x0000, // u
    0x0200, 0x0300, 0x00c0, 0x0030, 0x00c0, 0x0300, 0x0200, 0x0000, // v
    0x03e0, 0x0010, 0x0020, 0x01c0, 0x0020, 0x0010, 0x03e0, 0x0000, // w
    0x0200, 0x0210, 0x0120, 0x00c0, 0x00c0, 0x0120, 0x0210, 0x0000, // x
    0x0382, 0x0044, 0x0028, 0x0010, 0x0020, 0x0040, 0x0380, 0x0000, // y
    0x0310, 0x0230, 0x0250, 0x0290, 0x0310, 0x0230, 0x0000, 0x0000, // z
    0x0010, 0x07e0, 0x0850, 0x0990, 0x0a10, 0x07e0, 0x0800, 0x0000, // 0
    0x0000, 0x0000, 0x0410, 0x0ff0, 0x0010, 0x0000, 0x0000, 0x0000, // 1
    0x0000, 0x0430, 0x0850, 0x0890, 0x0910, 0x0610, 0x0000, 0x0000, // 2
    0x0000, 0x0420, 0x0810, 0x0910, 0x0910, 0x06e0, 0x0000, 0x0000, // 3
    0x0000, 0x0080, 0x0180, 0x0280, 0x0480, 0x0ff0, 0x0080, 0x0000, // 4
    0x0000, 0x0f10, 0x0910, 0x0910, 0x0920, 0x08c0, 0x0000, 0x0000, // 5
    0x0000, 0x03e0, 0x0510, 0x0910, 0x0910, 0x00e0, 0x0000, 0x0000, // 6
    0x0000, 0x0800, 0x0830, 0x08c0, 0x0b00, 0x0c00, 0x0000, 0x0000, // 7
    0x0000, 0x06e0, 0x0910, 0x0910, 0x0910, 0x06e0, 0x0000, 0x0000, // 8
    0x0000, 0x0700, 0x0890, 0x0890, 0x08a0, 0x07c0, 0x0000, 0x0000, // 9
    0x0000, 0x0080, 0x0080, 0x03e0, 0x0080, 0x0080, 0x0000, 0x0000, // +
    0x0000, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000, 0x0000, // -
    0x0000, 0x0240, 0x0180, 0x0660, 0x0180, 0x0240, 0x0000, 0x0000, // *
    0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0000, // /
    0x0000, 0x0000, 0x0000, 0x0000, 0x07e0, 0x0810, 0x1008, 0x0000, // (
    0x1008, 0x0810, 0x07e0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // )
    0x0640, 0x0920, 0x0920, 0x1ff0, 0x0920, 0x0920, 0x04c0, 0x0000, // $
    0x0000, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0000, 0x0000, // =
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // space
    0x0000, 0x0000, 0x0034, 0x0038, 0x0000, 0x0000, 0x0000, 0x0000, // ,
    0x0000, 0x0000, 0x0030, 0x0030, 0x0000, 0x0000, 0x0000, 0x0000, // .
    0x0000, 0x0080, 0x0080, 0x02a0, 0x0080, 0x0080, 0x0000, 0x0000, // divide
    0x0000, 0x0000, 0x0000, 0x0000, 0x1ff8, 0x1008, 0x1008, 0x0000, // [
    0x1008, 0x1008, 0x1ff8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // ]
    0x0c20, 0x1240, 0x0c80, 0x0100, 0x0260, 0x0490, 0x0860, 0x0000, // %
    0x0000, 0x0000, 0x0240, 0x0180, 0x0180, 0x0240, 0x0000, 0x0000, // multiply
    0x0080, 0x0140, 0x0220, 0x0770, 0x0140, 0x0140, 0x0140, 0x0000, // assign
    0x0000, 0x0000, 0x0000, 0x1c00, 0x0000, 0x0000, 0x0000, 0x0000, // '
    0x0000, 0x0000, 0x1c00, 0x0000, 0x1c00, 0x0000, 0x0000, 0x0000, // "
    0x0000, 0x0000, 0x0000, 0x1f90, 0x0000, 0x0000, 0x0000, 0x0000, // !
    0x0000, 0x0000, 0x0334, 0x0338, 0x0000, 0x0000, 0x0000, 0x0000, // ;
    0x0000, 0x0080, 0x0140, 0x0220, 0x0410, 0x0000, 0x0000, 0x0000, // <
    0x0000, 0x0000, 0x0410, 0x0220, 0x0140, 0x0080, 0x0000, 0x0000, // >
    0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, // _
    0x0000, 0x0c00, 0x1000, 0x10d0, 0x1100, 0x0e00, 0x0000, 0x0000, // ?
    0x1c1c, 0x1224, 0x0948, 0x0490, 0x0220, 0x0140, 0x0080, 0x0000, // arrow
    0x0000, 0x0000, 0x0000, 0x000a, 0x0006, 0x0000, 0x0000, 0x0000, // cedilla
};

/* data for plato font, set 1. */
const unsigned short plato_m1[] = {
    0x0500, 0x0500, 0x1fc0, 0x0500, 0x1fc0, 0x0500, 0x0500, 0x0000, // #
    0x07f0, 0x0900, 0x1100, 0x1100, 0x1100, 0x0900, 0x07f0, 0x0000, // A
    0x1ff0, 0x1210, 0x1210, 0x1210, 0x1210, 0x0e10, 0x01e0, 0x0000, // B
    0x07c0, 0x0820, 0x1010, 0x1010, 0x1010, 0x1010, 0x0820, 0x0000, // C
    0x1ff0, 0x1010, 0x1010, 0x1010, 0x1010, 0x0820, 0x07c0, 0x0000, // D
    0x1ff0, 0x1110, 0x1110, 0x1110, 0x1010, 0x1010, 0x1010, 0x0000, // E
    0x1ff0, 0x1100, 0x1100, 0x1100, 0x1000, 0x1000, 0x1000, 0x0000, // F
    0x07c0, 0x0820, 0x1010, 0x1010, 0x1090, 0x1090, 0x08e0, 0x0000, // G
    0x1ff0, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x1ff0, 0x0000, // H
    0x0000, 0x1010, 0x1010, 0x1ff0, 0x1010, 0x1010, 0x0000, 0x0000, // I
    0x0020, 0x0010, 0x1010, 0x1010, 0x1fe0, 0x1000, 0x1000, 0x0000, // J
    0x1ff0, 0x0080, 0x0100, 0x0280, 0x0440, 0x0820, 0x1010, 0x0000, // K
    0x1ff0, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0000, // L
    0x1ff0, 0x0800, 0x0400, 0x0200, 0x0400, 0x0800, 0x1ff0, 0x0000, // M
    0x1ff0, 0x0800, 0x0600, 0x0100, 0x00c0, 0x0020, 0x1ff0, 0x0000, // N
    0x07c0, 0x0820, 0x1010, 0x1010, 0x1010, 0x0820, 0x07c0, 0x0000, // O
    0x1ff0, 0x1100, 0x1100, 0x1100, 0x1100, 0x1100, 0x0e00, 0x0000, // P
    0x07c0, 0x0820, 0x1010, 0x1018, 0x1014, 0x0824, 0x07c0, 0x0000, // Q
    0x1ff0, 0x1100, 0x1100, 0x1180, 0x1140, 0x1120, 0x0e10, 0x0000, // R
    0x0e20, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x08e0, 0x0000, // S
    0x1000, 0x1000, 0x1000, 0x1ff0, 0x1000, 0x1000, 0x1000, 0x0000, // T
    0x1fe0, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x1fe0, 0x0000, // U
    0x1800, 0x0700, 0x00c0, 0x0030, 0x00c0, 0x0700, 0x1800, 0x0000, // V
    0x1fe0, 0x0010, 0x0020, 0x03c0, 0x0020, 0x0010, 0x1fe0, 0x0000, // W
    0x1830, 0x0440, 0x0280, 0x0100, 0x0280, 0x0440, 0x1830, 0x0000, // X
    0x1800, 0x0400, 0x0200, 0x01f0, 0x0200, 0x0400, 0x1800, 0x0000, // Y
    0x1830, 0x1050, 0x1090, 0x1110, 0x1210, 0x1410, 0x1830, 0x0000, // Z
    0x0000, 0x1000, 0x2000, 0x2000, 0x1000, 0x1000, 0x2000, 0x0000, // ~
    0x0000, 0x0000, 0x1000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, // dieresis
    0x0000, 0x1000, 0x2000, 0x4000, 0x2000, 0x1000, 0x0000, 0x0000, // circumflex
    0x0000, 0x0000, 0x0000, 0x1000, 0x2000, 0x4000, 0x0000, 0x0000, // acute
    0x0000, 0x4000, 0x2000, 0x1000, 0x0000, 0x0000, 0x0000, 0x0000, // grave
    0x0000, 0x0100, 0x0300, 0x07f0, 0x0300, 0x0100, 0x0000, 0x0000, // uparrow
    0x0080, 0x0080, 0x0080, 0x0080, 0x03e0, 0x01c0, 0x0080, 0x0000, // rightarrow 
    0x0000, 0x0040, 0x0060, 0x07f0, 0x0060, 0x0040, 0x0000, 0x0000, // downarrow
    0x0080, 0x01c0, 0x03e0, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000, // leftarrow
    0x0000, 0x0080, 0x0100, 0x0100, 0x0080, 0x0080, 0x0100, 0x0000, // low tilde
    0x1010, 0x1830, 0x1450, 0x1290, 0x1110, 0x1010, 0x1010, 0x0000, // Sigma
    0x0030, 0x00d0, 0x0310, 0x0c10, 0x0310, 0x00d0, 0x0030, 0x0000, // Delta
    0x0000, 0x0380, 0x0040, 0x0040, 0x0040, 0x0380, 0x0000, 0x0000, // union
    0x0000, 0x01c0, 0x0200, 0x0200, 0x0200, 0x01c0, 0x0000, 0x0000, // intersect
    0x0000, 0x0000, 0x0000, 0x0080, 0x0f78, 0x1004, 0x1004, 0x0000, // {
    0x1004, 0x1004, 0x0f78, 0x0080, 0x0000, 0x0000, 0x0000, 0x0000, // }
    0x00e0, 0x0d10, 0x1310, 0x0c90, 0x0060, 0x0060, 0x0190, 0x0000, // &
    0x0150, 0x0160, 0x0140, 0x01c0, 0x0140, 0x0340, 0x0540, 0x0000, // not equal
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // space
    0x0000, 0x0000, 0x0000, 0x1ff0, 0x0000, 0x0000, 0x0000, 0x0000, // |
    0x0000, 0x0c00, 0x1200, 0x1200, 0x0c00, 0x0000, 0x0000, 0x0000, // degree
    0x0000, 0x02a0, 0x02a0, 0x02a0, 0x02a0, 0x02a0, 0x0000, 0x0000, // equiv
    0x01e0, 0x0210, 0x0210, 0x01a0, 0x0060, 0x0090, 0x0310, 0x0000, // alpha
    0x0002, 0x03fc, 0x0510, 0x0910, 0x0910, 0x0690, 0x0060, 0x0000, // beta
    0x0000, 0x0ce0, 0x1310, 0x1110, 0x0890, 0x0460, 0x0000, 0x0000, // delta
    0x0000, 0x1030, 0x0cc0, 0x0300, 0x00c0, 0x0030, 0x0000, 0x0000, // lambda
    0x0002, 0x0002, 0x03fc, 0x0010, 0x0010, 0x03e0, 0x0010, 0x0000, // mu
    0x0100, 0x0200, 0x03f0, 0x0200, 0x03f0, 0x0200, 0x0400, 0x0000, // pi
    0x0006, 0x0038, 0x00e0, 0x0110, 0x0210, 0x0220, 0x01c0, 0x0000, // rho
    0x00e0, 0x0110, 0x0210, 0x0310, 0x02e0, 0x0200, 0x0200, 0x0000, // sigma
    0x01e0, 0x0210, 0x0010, 0x00e0, 0x0010, 0x0210, 0x01e0, 0x0000, // omega
    0x0220, 0x0220, 0x0520, 0x0520, 0x08a0, 0x08a0, 0x0000, 0x0000, // less/equal
    0x0000, 0x08a0, 0x08a0, 0x0520, 0x0520, 0x0220, 0x0220, 0x0000, // greater/equal
    0x07c0, 0x0920, 0x1110, 0x1110, 0x1110, 0x0920, 0x07c0, 0x0000, // theta
    0x01e0, 0x0210, 0x04c8, 0x0528, 0x05e8, 0x0220, 0x01c0, 0x0000, // @
    0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0000, /* \ */
    0x01e0, 0x0210, 0x0210, 0x01e0, 0x0290, 0x0290, 0x01a0, 0x0000, // oe
};


// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(PtermFrame, wxFrame)
    EVT_IDLE(PtermFrame::OnIdle)
    EVT_CLOSE(PtermFrame::OnClose)
    EVT_TIMER(Pterm_Timer, PtermFrame::OnTimer)
    EVT_TIMER(Pterm_PasteTimer, PtermFrame::OnPasteTimer)
    EVT_ACTIVATE(PtermFrame::OnActivate)
    EVT_MENU(Pterm_Close, PtermFrame::OnQuit)
#if !defined(__WXMAC__)
    EVT_MENU(Pterm_ToggleMenuBar, PtermFrame::OnToggleMenuBar)
    EVT_MENU(Pterm_ToggleMenuBarView, PtermFrame::OnToggleMenuBar)
#endif
    EVT_MENU(Pterm_ToggleStatusBar, PtermFrame::OnToggleStatusBar)
    EVT_MENU(Pterm_ToggleStatusBarView, PtermFrame::OnToggleStatusBar)
    EVT_MENU(Pterm_Toggle2xMode, PtermFrame::OnToggle2xMode)
    EVT_MENU(Pterm_Toggle2xModeView, PtermFrame::OnToggle2xMode)
    EVT_MENU(Pterm_CopyScreen, PtermFrame::OnCopyScreen)
    EVT_MENU(Pterm_Copy, PtermFrame::OnCopy)
    EVT_MENU(Pterm_Exec, PtermFrame::OnExec)	
    EVT_MENU(Pterm_MailTo, PtermFrame::OnMailTo)	
    EVT_MENU(Pterm_SearchThis, PtermFrame::OnSearchThis)	
    EVT_MENU(Pterm_SpellCheck, PtermFrame::OnSpellCheck)	
    EVT_MENU(Pterm_Macro0, PtermFrame::OnMacro0)	
    EVT_MENU(Pterm_Macro1, PtermFrame::OnMacro1)	
    EVT_MENU(Pterm_Macro2, PtermFrame::OnMacro2)	
    EVT_MENU(Pterm_Macro3, PtermFrame::OnMacro3)	
    EVT_MENU(Pterm_Macro4, PtermFrame::OnMacro4)	
    EVT_MENU(Pterm_Macro5, PtermFrame::OnMacro5)	
    EVT_MENU(Pterm_Macro6, PtermFrame::OnMacro6)	
    EVT_MENU(Pterm_Macro7, PtermFrame::OnMacro7)	
    EVT_MENU(Pterm_Macro8, PtermFrame::OnMacro8)	
    EVT_MENU(Pterm_Macro9, PtermFrame::OnMacro9)	
    EVT_MENU(Pterm_Paste, PtermFrame::OnPaste)
    EVT_MENU(Pterm_PastePrint, PtermFrame::OnPaste)
    EVT_UPDATE_UI(Pterm_Paste, PtermFrame::OnUpdateUIPaste)
    EVT_MENU(Pterm_SaveScreen, PtermFrame::OnSaveScreen)
    EVT_MENU(Pterm_Print, PtermFrame::OnPrint)
    EVT_MENU(Pterm_Preview, PtermFrame::OnPrintPreview)
    EVT_MENU(Pterm_Page_Setup, PtermFrame::OnPageSetup)
    EVT_MENU(Pterm_Pref,    PtermFrame::OnPref)
    EVT_MENU(Pterm_FullScreen, PtermFrame::OnFullScreen)
    EVT_MENU(Pterm_FullScreenView, PtermFrame::OnFullScreen)
#if defined (__WXMSW__)
    EVT_ICONIZE(PtermFrame::OnIconize)
#endif
    END_EVENT_TABLE ()

BEGIN_EVENT_TABLE(PtermApp, wxApp)
    EVT_MENU(Pterm_Connect, PtermApp::OnConnect)
    EVT_MENU(Pterm_ConnectAgain, PtermApp::OnConnect)
    EVT_MENU(Pterm_Quit,    PtermApp::OnQuit)
    EVT_MENU(Pterm_HelpKeys, PtermApp::OnHelpKeys)
    EVT_MENU(Pterm_About, PtermApp::OnAbout)
    END_EVENT_TABLE ()

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp () which will return the reference of the right type (i.e. PtermApp and
// not wxApp)
IMPLEMENT_APP(PtermApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool PtermApp::OnInit (void)
{
    int r, g, b;
    wxString rgb;
    wxString str;
    wxString filename;
	bool skipinit = false;
    
    ptermApp = this;
    m_firstFrame = m_helpFrame = NULL;
    g_printData = new wxPrintData;
    g_pageSetupData = new wxPageSetupDialogData;
    
    sprintf (traceFn, "pterm%d.trc", getpid ());

    m_locale.Init(wxLANGUAGE_DEFAULT);
    m_locale.AddCatalog(wxT("pterm"));

#ifdef DEBUGLOG
    logwindow = new wxLogWindow (NULL, wxT("pterm log"), true, false);
#endif

    if (argc > 3)
    {
        printf ("usage: pterm [ hostname [ portnum ]]\n"
			    "   or: pterm [ filename.ppf ]\n");
        exit (1);
    }

    m_config = new wxConfig (wxT ("Pterm"));

	//check for PPF file specified on command line
    if (argc > 1)
	{
		str = argv[argc-1];
		if (str.Right(4).CmpNoCase(wxT(".ppf"))==0 || str.Right(5).CmpNoCase(wxT(".ppf\""))==0)
		{
			wxString filename;
			wxString profile;
			filename.Clear();
			for (int i=1;i<argc;i++)
			{
				if (i>1)
					filename.Append(wxT(" "));
				filename.Append(argv[i]);
			}
			//strip double quotes from ends
			if (filename.Left(1)==wxT("\"") && filename.Right(1)==wxT("\""))
				filename.Mid(2,filename.Len()-2);
			//get just the filename (profile)
			profile = filename.BeforeLast('.');
			#if defined(_WIN32)
				profile = profile.AfterLast('\\');
			#else
				profile = profile.AfterLast('/');
			#endif
			//load profile
			skipinit = ptermApp->LoadProfile(profile,filename);
		}
	}

	//check if skipping initial read of settings due to successful load of settings
	//from profile filename specified above
	if (!skipinit)
	{
		//notebook
		m_lastTab = m_config->Read (wxT (PREF_LASTTAB), 0L);
		//tab0
		m_config->Read (wxT (PREF_CURPROFILE), &m_curProfile, CURRENT_PROFILE);
		//tab1
		m_config->Read (wxT (PREF_SHELLFIRST), &m_ShellFirst, wxT(""));
		m_connect = (m_config->Read (wxT (PREF_CONNECT), 1) != 0);
		if (argc > 1)
			m_hostName = argv[1];
		else
			m_config->Read (wxT (PREF_HOST), &m_hostName, DEFAULTHOST);
		if (argc > 2)
			m_port = atoi (wxString (argv[2]).mb_str());
		else
			m_port = m_config->Read (wxT (PREF_PORT), DefNiuPort);
		//tab2
		m_showSignon = (m_config->Read (wxT (PREF_SHOWSIGNON), 0L) != 0);
		m_showSysName = (m_config->Read (wxT (PREF_SHOWSYSNAME), 0L) != 0);
		m_showHost = (m_config->Read (wxT (PREF_SHOWHOST), 1) != 0);
		m_showStation = (m_config->Read (wxT (PREF_SHOWSTATION), 1) != 0);
		//tab3
		m_classicSpeed = (m_config->Read (wxT (PREF_1200BAUD), 0L) != 0);
		m_gswEnable = (m_config->Read (wxT (PREF_GSW), 1) != 0);
		m_numpadArrows = (m_config->Read (wxT (PREF_ARROWS), 1) != 0);
		m_ignoreCapLock = (m_config->Read (wxT (PREF_IGNORECAP), 0L) != 0);
		m_platoKb = (m_config->Read (wxT (PREF_PLATOKB), 0L) != 0);
		#if defined(__WXMAC__)
			m_useAccel = true;
		#else
			m_useAccel = (m_config->Read (wxT (PREF_ACCEL), 0L) != 0);
		#endif
		m_beepEnable = (m_config->Read (wxT (PREF_BEEP), 1) != 0);
		m_DisableShiftSpace = (m_config->Read (wxT (PREF_SHIFTSPACE), 0L) != 0);
		m_DisableMouseDrag = (m_config->Read (wxT (PREF_MOUSEDRAG), 0L) != 0);
		//tab4
		m_scale = m_config->Read (wxT (PREF_SCALE), 1);
		if (m_scale != 1 && m_scale != 2)
			m_scale = 1;
		m_showStatusBar = (m_config->Read (wxT (PREF_STATUSBAR), 1) != 0);
#if !defined(__WXMAC__)
		m_showMenuBar = (m_config->Read (wxT (PREF_MENUBAR), 1) != 0);
#endif
		m_noColor = (m_config->Read (wxT (PREF_NOCOLOR), 0L) != 0);
		m_config->Read (wxT (PREF_FOREGROUND), &rgb, wxT ("255 144 0"));// 255 144 0 is RGB for Plato Orange
		sscanf (rgb.mb_str(), "%d %d %d", &r, &g, &b);
		m_fgColor = wxColour (r, g, b);
		m_config->Read (wxT (PREF_BACKGROUND), &rgb, wxT ("0 0 0"));
		sscanf (rgb.mb_str(), "%d %d %d", &r, &g, &b);
		m_bgColor = wxColour (r, g, b);
		//tab5
		m_charDelay.Printf (wxT ("%d"), m_config->Read (wxT (PREF_CHARDELAY), PASTE_CHARDELAY) );
		m_lineDelay.Printf (wxT ("%d"), m_config->Read (wxT (PREF_LINEDELAY), PASTE_LINEDELAY) );
		m_autoLF.Printf (wxT ("%d"), m_config->Read (wxT (PREF_AUTOLF), 0L) );
		m_splitWords = (m_config->Read (wxT (PREF_SPLITWORDS), 0L) != 0);
		m_smartPaste = (m_config->Read (wxT (PREF_SMARTPASTE), 0L) != 0);
		m_convDot7 = (m_config->Read (wxT (PREF_CONVDOT7), 0L) != 0);
		m_conv8Sp = (m_config->Read (wxT (PREF_CONV8SP), 0L) != 0);
		m_TutorColor = (m_config->Read (wxT (PREF_TUTORCOLOR), 0L) != 0);
		//tab6
		m_config->Read (wxT (PREF_BROWSER), &m_Browser, wxT(""));
		m_config->Read (wxT (PREF_EMAIL), &m_Email, wxT(""));
		m_config->Read (wxT (PREF_SEARCHURL), &m_SearchURL, wxT(""));
		//rostering
		m_config->Read (wxT (PREF_ROSTERFILE), &m_RosterFile, wxT("C:\\Documents and Settings\\Joe\\Desktop\\Cyber1\\new_roster.txt"));
	}

#if PTERM_MDI
    // On Mac, the style rule is that the application keeps running even
    // if all its windows are closed.
//    SetExitOnFrameDelete(false);
    PtermFrameParent = new PtermMainFrame ();
    PtermFrameParent->Show (true);
#endif

    // create the main application window
    // If arguments are present, always connect without asking
    if (!DoConnect (!(m_connect || argc > 1)))
    {
        return false;
    }
    
    // Add some handlers so we can save the screen in various formats
    // Note that the BMP handler is always loaded, don't do it again.
//temp    wxImage::AddHandler (new wxPNGHandler);
    wxImage::AddHandler (new wxPNMHandler);
//temp    wxImage::AddHandler (new wxTIFFHandler);
    wxImage::AddHandler (new wxXPMHandler);

    // success: wxApp::OnRun () will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

int PtermApp::OnExit (void)
{
    wxTheClipboard->Flush ();

    m_config->Write (wxT (PREF_XPOS), lastX);
    m_config->Write (wxT (PREF_YPOS), lastY);
    m_config->Flush ();

    delete g_printData;
    delete g_pageSetupData;
#ifdef DEBUGLOG
    delete logwindow;
#endif

    return 0;
}

void PtermApp::OnConnect (wxCommandEvent &event)
{
    DoConnect (event.GetId () == Pterm_Connect);
}

bool PtermApp::DoConnect (bool ask)
{
    PtermFrame *frame;

    if (ask)
    {
        PtermConnDialog dlg (wxID_ANY, _("Connect to PLATO"), wxDefaultPosition, wxSize( -1,-1 )); //450,355 ));
    
        dlg.CenterOnScreen ();
        
        if (dlg.ShowModal () == wxID_OK)
        {
            if (dlg.m_ShellFirst.IsEmpty())
            {
                m_ShellFirst = wxT("");
            }
            else
            {
                m_ShellFirst = dlg.m_ShellFirst;
            }
            if (dlg.m_host.IsEmpty ())
            {
                m_hostName = DEFAULTHOST;
            }
            else
            {
                m_hostName = dlg.m_host;
            }
            if (dlg.m_port.IsEmpty ())
            {
                m_port = DefNiuPort;
            }
            else
            {
                m_port = atoi (wxString (dlg.m_port).mb_str());
            }
			//save selection to current
			m_config = new wxConfig (wxT ("Pterm"));
			m_config->Write (wxT (PREF_SHELLFIRST), m_ShellFirst );
			m_config->Write (wxT (PREF_CURPROFILE), m_curProfile );
			m_config->Write (wxT (PREF_HOST), m_hostName);
			m_config->Write (wxT (PREF_PORT), m_port);
		    m_config->Flush ();
        }
        else
        {
            return false;     // connect canceled
        }
    }
    

    // create the main application window
    lastX = m_config->Read (wxT (PREF_XPOS), 0L);
    lastY = m_config->Read (wxT (PREF_YPOS), 0L);
    if (lastX == 0 && lastY == 0)
    {
        frame = new PtermFrame(m_hostName, m_port, wxT("Pterm"));
    }
    else
    {
        frame = new PtermFrame(m_hostName, m_port, wxT("Pterm"),  wxPoint (lastX, lastY));
    }

    if (frame != NULL)
    {
        if (m_firstFrame != NULL)
        {
            m_firstFrame->m_prevFrame = frame;
        }
        frame->m_nextFrame = m_firstFrame;
        m_firstFrame = frame;
    }
    
    return (frame != NULL);
}

bool PtermApp::LoadProfile(wxString profile, wxString filename)
{
	wxString buffer;
	wxString token;
	wxString value;
	wxString str;
    wxString rgb;
    int r, g, b;
	long lvalue;

	//open file
	if (filename.IsEmpty())
		filename = ptermApp->ProfileFileName(profile);
	wxTextFile file(filename);
	if (!file.Open())
		return false;

	//read file
	for ( buffer = file.GetFirstLine(); ; buffer = file.GetNextLine() )
	{
		if (buffer.Contains(wxT("=")))
		{
			token = buffer.BeforeFirst('=');
			token.Trim(true);
			token.Trim(false);
			value = buffer.AfterFirst(wxT('='));
			value.Trim(true);
			value.Trim(false);
			//tab0
			if      (token.Cmp(wxT(PREF_CURPROFILE))==0)			m_curProfile	= profile;
			//tab1
			else if (token.Cmp(wxT(PREF_SHELLFIRST))==0)			m_ShellFirst	= value;
			else if (token.Cmp(wxT(PREF_CONNECT))==0)				m_connect		= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_HOST))==0)					m_hostName		= value;
			else if (token.Cmp(wxT(PREF_PORT))==0)					
																	{
																	value.ToLong(&lvalue,10);
																	m_port			= (int)lvalue;
																	}
			//tab2
			else if (token.Cmp(wxT(PREF_SHOWSIGNON))==0)			m_showSignon	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_SHOWSYSNAME))==0)			m_showSysName	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_SHOWHOST))==0)				m_showHost		= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_SHOWSTATION))==0)			m_showStation	= (value.Cmp(wxT("1"))==0);
			//tab3
			else if (token.Cmp(wxT(PREF_1200BAUD))==0)				m_classicSpeed	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_GSW))==0)					m_gswEnable		= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_ARROWS))==0)				m_numpadArrows	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_IGNORECAP))==0)				m_ignoreCapLock	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_PLATOKB))==0)				m_platoKb		= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_ACCEL))==0)					
																	#if defined(__WXMAC__)
																	m_useAccel		= true;
																	#else
																	m_useAccel		= (value.Cmp(wxT("1"))==0);
																	#endif
			else if (token.Cmp(wxT(PREF_BEEP))==0)					m_beepEnable	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_SHIFTSPACE))==0)			m_DisableShiftSpace	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_MOUSEDRAG))==0)				m_DisableMouseDrag	= (value.Cmp(wxT("1"))==0);
			//tab4
			else if (token.Cmp(wxT(PREF_SCALE))==0)					m_scale			= (value.Cmp(wxT("2"))==0) ? 2 : 1;
			else if (token.Cmp(wxT(PREF_STATUSBAR))==0)				m_showStatusBar = (value.Cmp(wxT("1"))==0);
#if !defined(__WXMAC__)
			else if (token.Cmp(wxT(PREF_MENUBAR))==0)				m_showMenuBar	= (value.Cmp(wxT("1"))==0);
#endif
			else if (token.Cmp(wxT(PREF_NOCOLOR))==0)				m_noColor		= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_FOREGROUND))==0)
																	{
																	sscanf (value.mb_str(), "%d %d %d", &r, &g, &b);
																	m_fgColor		= wxColour (r, g, b);
																	}
			else if (token.Cmp(wxT(PREF_BACKGROUND))==0)
																	{
																	sscanf (value.mb_str(), "%d %d %d", &r, &g, &b);
																	m_bgColor		= wxColour (r, g, b);
																	}
			//tab5
			else if (token.Cmp(wxT(PREF_CHARDELAY))==0)				m_charDelay		= value;
			else if (token.Cmp(wxT(PREF_LINEDELAY))==0)				m_lineDelay		= value;
			else if (token.Cmp(wxT(PREF_AUTOLF))==0)				m_autoLF		= value;
			else if (token.Cmp(wxT(PREF_SPLITWORDS))==0)			m_splitWords	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_SMARTPASTE))==0)			m_smartPaste	= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_CONVDOT7))==0)				m_convDot7		= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_CONV8SP))==0)				m_conv8Sp		= (value.Cmp(wxT("1"))==0);
			else if (token.Cmp(wxT(PREF_TUTORCOLOR))==0)			m_TutorColor	= (value.Cmp(wxT("1"))==0);
			//tab6
			else if (token.Cmp(wxT(PREF_BROWSER))==0)				m_Browser		= value;
			else if (token.Cmp(wxT(PREF_EMAIL))==0)					m_Email			= value;
			else if (token.Cmp(wxT(PREF_SEARCHURL))==0)				m_SearchURL		= value;
		}
		if (file.Eof())
		{
			break;
		}
	}
	file.Close();

    //write prefs
	m_config = new wxConfig (wxT ("Pterm"));
	//tab0
    m_config->Write (wxT (PREF_CURPROFILE), profile );
	//tab1
    m_config->Write (wxT (PREF_SHOWSIGNON), (m_showSignon) ? 1 : 0);
    m_config->Write (wxT (PREF_SHOWSYSNAME), (m_showSysName) ? 1 : 0);
    m_config->Write (wxT (PREF_SHOWHOST), (m_showHost) ? 1 : 0);
    m_config->Write (wxT (PREF_SHOWSTATION), (m_showStation) ? 1 : 0);
	//tab2
    m_config->Write (wxT (PREF_1200BAUD), (m_classicSpeed) ? 1 : 0);
    m_config->Write (wxT (PREF_GSW), (m_gswEnable) ? 1 : 0);
    m_config->Write (wxT (PREF_ARROWS), (m_numpadArrows) ? 1 : 0);
    m_config->Write (wxT (PREF_IGNORECAP), (m_ignoreCapLock) ? 1 : 0);
    m_config->Write (wxT (PREF_PLATOKB), (m_platoKb) ? 1 : 0);
    m_config->Write (wxT (PREF_ACCEL), (m_useAccel) ? 1 : 0);
    m_config->Write (wxT (PREF_BEEP), (m_beepEnable) ? 1 : 0);
    m_config->Write (wxT (PREF_SHIFTSPACE), (m_DisableShiftSpace) ? 1 : 0);
    m_config->Write (wxT (PREF_MOUSEDRAG), (m_DisableMouseDrag) ? 1 : 0);
	//tab3
    m_config->Write (wxT (PREF_CONNECT), (m_connect) ? 1 : 0);
    m_config->Write (wxT (PREF_HOST), m_hostName);
    m_config->Write (wxT (PREF_PORT), m_port);
	//tab4
    m_config->Write (wxT (PREF_SCALE), m_scale);
    m_config->Write (wxT (PREF_STATUSBAR), (m_showStatusBar) ? 1 : 0);
#if !defined(__WXMAC__)
    m_config->Write (wxT (PREF_MENUBAR), (m_showMenuBar) ? 1 : 0);
#endif
    m_config->Write (wxT (PREF_NOCOLOR), (m_noColor) ? 1 : 0);
    rgb.Printf (wxT ("%d %d %d"), m_fgColor.Red (), m_fgColor.Green (), m_fgColor.Blue ());
    m_config->Write (wxT (PREF_FOREGROUND), rgb);
    rgb.Printf (wxT ("%d %d %d"), m_bgColor.Red (), m_bgColor.Green (), m_bgColor.Blue ());
    m_config->Write (wxT (PREF_BACKGROUND), rgb);
	//tab5
    m_config->Write (wxT (PREF_CHARDELAY), atoi(m_charDelay.mb_str()));
    m_config->Write (wxT (PREF_LINEDELAY), atoi(m_lineDelay.mb_str()));
    m_config->Write (wxT (PREF_AUTOLF), atoi(m_autoLF.mb_str()));
    m_config->Write (wxT (PREF_SPLITWORDS), (m_splitWords) ? 1 : 0);
    m_config->Write (wxT (PREF_SMARTPASTE), (m_smartPaste) ? 1 : 0);
    m_config->Write (wxT (PREF_CONVDOT7), (m_convDot7) ? 1 : 0);
    m_config->Write (wxT (PREF_CONV8SP), (m_conv8Sp) ? 1 : 0);
    m_config->Write (wxT (PREF_TUTORCOLOR), (m_TutorColor) ? 1 : 0);
	//tab6
    m_config->Write (wxT (PREF_BROWSER), m_Browser);
    m_config->Write (wxT (PREF_EMAIL), m_Email);
    m_config->Write (wxT (PREF_SEARCHURL), m_SearchURL);
    m_config->Flush ();

	return true;
}

wxString PtermApp::ProfileFileName(wxString profile)
{
	wxString filename;
	filename = wxGetCwd();
	if (filename.Right(1) != wxT ("/") && filename.Right(1) != wxT ("\\"))
	{
		#if defined(_WIN32)
			filename.Append(wxT("\\"));
		#else
			filename.Append(wxT("/"));
		#endif
	}
	filename.Append(profile);
	filename.Append(wxT(".ppf"));
	return filename;
}

void PtermApp::OnAbout(wxCommandEvent&)
{
    wxMessageBox(wxT (STRPRODUCTNAME " V" STRFILEVER
                      "\n  built with wxWidgets V" WXVERSION
                      "\n" STRLEGALCOPYRIGHT),
                      _("About Pterm"), wxOK | wxICON_INFORMATION, NULL);
}

void PtermApp::OnHelpKeys (wxCommandEvent &)
{
    PtermFrame *frame;
    wxString str;
    unsigned int i;


    if (m_helpFrame == NULL)
    {
        // If there isn't one yet, create a help window -- same as a
        // regular frame except that the data comes from here, not
        // from a connection.
        frame = new PtermFrame(str, -1, _("Keyboard Help"));

        if (frame != NULL)
        {
            if (m_firstFrame != NULL)
            {
                m_firstFrame->m_prevFrame = frame;
            }
            frame->m_nextFrame = m_firstFrame;
            m_firstFrame = frame;
            for (i = 0; i < sizeof (keyboardhelp) / sizeof (keyboardhelp[0]); i++)
            {
                frame->procPlatoWord (keyboardhelp[i], false);
            }
            m_helpFrame = frame;
        }
    }
    else
    {
        m_helpFrame->Show (true);
        m_helpFrame->Raise ();
    }
}

wxColour PtermApp::SelectColor ( wxWindow &parent, 
                                 const wxChar *title, wxColour &initcol)
{
    wxColour col (initcol);
    wxColour orange (255, 144, 0);
    wxColour vikingGreen (0, 220, 0);
    wxColourData data;

    data.SetColour (initcol);
    data.SetCustomColour (0, orange);
    data.SetCustomColour (1, *wxBLACK);
    data.SetCustomColour (2, vikingGreen);
    
    wxColourDialog dialog (&parent, &data);

	dialog.CentreOnParent();
	dialog.SetTitle(title);

    if (dialog.ShowModal () == wxID_OK)
    {
        col = dialog.GetColourData ().GetColour ();
    }

    return col;
}

void PtermApp::OnQuit(wxCommandEvent&)
{
    PtermFrame *frame, *nextframe;

    frame = m_firstFrame;
    while (frame != NULL)
    {
        nextframe = frame->m_nextFrame;
        frame->Close (true);
        frame = nextframe;
    }
#if PTERM_MDI // defined(__WXMAC__)
    // On the Mac, deleting all the windows doesn't terminate the
    // program, so we make it stop this way.
    ExitMainLoop ();
#endif
}


#if PTERM_MDI
// MDI parent frame
PtermMainFrame::PtermMainFrame (void)
    : wxMDIParentFrame (NULL, wxID_ANY, wxT ("Pterm"),
                        wxDefaultPosition, wxDefaultSize, 0)
{
#if wxUSE_MENUS
    // create a menu bar
    //
    // Note that the menu bar labels do not have shortcut markings,
    // because those conflict with the ALT-letter key codes for PLATO.
#if  defined(__WXGTK20__)
    // A rather ugly hack here.  GTK V2 insists that F10 should be the
    // accelerator for the menu bar.  We don't want that.  There is
    // no sane way to turn this off, but we *can* get the same effect
    // by setting the "menu bar accelerator" property to the name of a
    // function key that is apparently legal, but doesn't really exist.
    // (Or if it does, it certainly isn't a key we use.)
    gtk_settings_set_string_property (gtk_settings_get_default (),
                                      "gtk-menu-bar-accel", "F15", "foo");

#endif

    menuFile = new wxMenu;
    menuFile->Append (Pterm_Connect, _("New Connection...") ACCELERATOR ("\tCtrl-N"), _("Connect to a PLATO host"));
    menuFile->Append (Pterm_Pref, _("Preferences..."), _("Set program configuration"));
    menuFile->AppendSeparator ();
    menuFile->Append (Pterm_Quit, _("Exit"), _("Quit this program"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
    menuHelp = new wxMenu;

    menuHelp->Append(Pterm_About, _("About Pterm"), _("Show about dialog"));
    menuHelp->Append(Pterm_HelpKeys, _("Pterm keyboard"), _("Show keyboard description"));
    
    // now append the freshly created menu to the menu bar...
	menuBar = new wxMenuBar ();
    menuBar->Append (menuFile, _("File"));
#if defined(__WXMAC__)
    // On the Mac the menu name has to be exactly "&Help" for the About item
    // to  be recognized.  Ugh.
    menuBar->Append(menuHelp, wxT("&Help"));
#else
    menuBar->Append(menuHelp, _("Help"));
#endif

    // ... and attach this menu bar to the frame
#if !defined(__WXMAC__)
    if (!m_fullScreen && ptermApp->m_showMenuBar)
#endif
		SetMenuBar(menuBar);
#endif // wxUSE_MENUS
}
#endif

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
PtermFrame::PtermFrame(wxString &host, int port, const wxString& title, const wxPoint &pos)
    : PtermFrameBase(PtermFrameParent, -1, title, pos, wxDefaultSize),
      tracePterm (false),
      m_nextFrame (NULL),
      m_prevFrame (NULL),
#ifdef AUTOSCALE
      m_foregroundPen (ptermApp->m_fgColor, 1, wxSOLID),
      m_backgroundPen (ptermApp->m_bgColor, 1, wxSOLID),
#else
	  m_foregroundPen (ptermApp->m_fgColor, ptermApp->m_scale, wxSOLID),
	  m_backgroundPen (ptermApp->m_bgColor, ptermApp->m_scale, wxSOLID),
#endif
      m_foregroundBrush (ptermApp->m_fgColor, wxSOLID),
      m_backgroundBrush (ptermApp->m_bgColor, wxSOLID),
      m_canvas (NULL),
      m_conn (NULL),
      m_port (port),
      m_timer (this, Pterm_Timer),
      m_fullScreen (false),
      m_station (0),
      m_pasteTimer (this, Pterm_PasteTimer),
      m_pasteIndex (-1),
      m_pasteNextKeyCnt (0),
	  m_SendRoster (false),
	  m_WaitReady (false),
	  m_bCancelPaste (false),
	  m_bPasteActive (false),
      m_nextword (0),
      m_delay (0),
      currentX (0),
      currentY (496),
      memaddr (0),
      memlpc (0),
      wc (0),
      seq (0),
      modewords (0),
      m_dumbTty (true),
      m_ascState (none),
      m_ascBytes (0),
      lastX (0),
      lastY (0),
      m_flowCtrl (false),
      m_sendFgt (false),
      m_defFg (ptermApp->m_fgColor),
      m_defBg (ptermApp->m_bgColor),
      m_currentFg (ptermApp->m_fgColor),
      m_currentBg (ptermApp->m_bgColor),
      m_pendingEcho (-1),
	  m_scale (ptermApp->m_scale),
	  m_usefont( false ),
	  m_fontPMD( false ),
	  m_fontinfo( false ),
	  m_osinfo( false ),
	  modexor( false ),
	  cwsmode( 0 ),
	  cwsfun ( 0 ),
	  cwswin ( 0 ),
	  cwscnt ( 0 )

{
    int i;

    if (tracePterm)
    {
        traceF = stdout;
    }
    
    m_hostName = host;
    mode = 017;             // default to character mode, rewrite
	modexor = false;
    setMargin (0);
    RAM[M_CCR] = 0;
    
    // Set default character set origins (for PPT that is; ASCII is different)
    RAM[C2ORIGIN] = M2ADDR & 0xff;
    RAM[C2ORIGIN + 1] = M2ADDR >> 8;
    RAM[C3ORIGIN] = M3ADDR & 0xff;
    RAM[C3ORIGIN + 1] = M3ADDR >> 8;
    
    // set the frame icon
    SetIcon(wxICON(pterm_32));

    // set the line style to projecting
    m_foregroundPen.SetCap (wxCAP_BUTT);
    m_backgroundPen.SetCap (wxCAP_BUTT);

#if wxUSE_MENUS
    // create a menu bar
    //
    // Note that the menu bar labels do not have shortcut markings,
    // because those conflict with the ALT-letter key codes for PLATO.
#if  defined(__WXGTK20__)
    // A rather ugly hack here.  GTK V2 insists that F10 should be the
    // accelerator for the menu bar.  We don't want that.  There is
    // no sane way to turn this off, but we *can* get the same effect
    // by setting the "menu bar accelerator" property to the name of a
    // function key that is apparently legal, but doesn't really exist.
    // (Or if it does, it certainly isn't a key we use.)
    gtk_settings_set_string_property (gtk_settings_get_default (), "gtk-menu-bar-accel", "F15", "foo");

#endif

    // NOTE: Accelerators are actually Command-xxx on the Mac on other platforms they are controlled by a prefs setting.
	//file menu options
    menuFile = new wxMenu;
    menuFile->Append (Pterm_Connect, _("New Connection...") ACCELERATOR ("\tCtrl-N"), _("Connect to a PLATO host"));
    if (port > 0)
    {
        // No "connect again" for the help window because that
        // doesn't own a connection.
        menuFile->Append (Pterm_ConnectAgain, _("Connect Again"), _("Connect to the same host"));
        menuFile->AppendSeparator();
    }
    menuFile->Append (Pterm_SaveScreen, _("Save Screen") ACCELERATOR ("\tCtrl-S"), _("Save screen image to file"));
    menuFile->Append (Pterm_Print, _("Print...") ACCELERATOR ("\tCtrl-P"), _("Print screen content"));
    menuFile->Append (Pterm_Page_Setup, _("Page Setup..."), _("Printout page setup"));
    menuFile->Append (Pterm_Preview, _("Print Preview"), _("Preview screen print"));
    menuFile->AppendSeparator ();
    menuFile->Append (Pterm_Pref, _("Preferences..."), _("Set program configuration"));
    menuFile->AppendSeparator ();
    menuFile->Append (Pterm_Close, _("Close") ACCELERATOR ("\tCtrl-W"), _("Close this window"));
    menuFile->Append (Pterm_Quit, _("Exit"), _("Quit this program"));
	//edit menu options
	BuildEditMenu(port);

	//view menu options
    menuView = new wxMenu;
#if !defined(__WXMAC__)
    menuView->AppendCheckItem (Pterm_ToggleMenuBarView, _("Display menu bar"), _("Display menu bar"));
	menuView->Check(Pterm_ToggleMenuBarView,ptermApp->m_showMenuBar);
#endif
    menuView->AppendCheckItem (Pterm_ToggleStatusBarView, _("Display status bar"), _("Display status bar"));
	menuView->Check(Pterm_ToggleStatusBarView,ptermApp->m_showStatusBar);
    menuView->AppendCheckItem (Pterm_Toggle2xModeView, _("Zoom display 200%"), _("Zoom display 200%"));
	menuView->Check(Pterm_Toggle2xModeView,(ptermApp->m_scale==2));
    menuView->AppendSeparator ();
    menuView->Append (Pterm_FullScreenView, _("Full Screen") ACCELERATOR ("\tCtrl-U"), _("Display in full screen mode"));

    // the "About" item should be in the help menu.
    // Well, on the Mac it actually doesn't show up there, but for that magic
    // to work it has to be presented to wx in the help menu.  So the help
    // menu ends up empty.  Sigh.
	//help menu options
    menuHelp = new wxMenu;
    menuHelp->Append(Pterm_About, _("About Pterm"), _("Show about dialog"));
    menuHelp->Append(Pterm_HelpKeys, _("Pterm keyboard"), _("Show keyboard description"));
    
    // now append the freshly created menu to the menu bar...
	BuildMenuBar();

	//edit menu options (copy for POPUP menu via right-click on canvas, adds the full-screen option)
	BuildPopupMenu(port);

    // ... and attach this menu bar to the frame
#if !defined(__WXMAC__)
    // On Mac menu bar is always displayed (Mac standard)
	if (!m_fullScreen && ptermApp->m_showMenuBar)
#endif
		SetMenuBar(menuBar);
#endif // wxUSE_MENUS

    if (port > 0)
    {
        // create a status bar, if this isn't a help window

		//shell program; then wait 2 seconds
		if (!ptermApp->m_ShellFirst.IsEmpty())
		{
			if (wxExecute(ptermApp->m_ShellFirst)!=0)
			{
#if defined(_WIN32)
                Sleep (2000);
#else
                sleep (2);
#endif
			}
		}

		if (m_fullScreen || !ptermApp->m_showStatusBar)
		{
	        m_statusBar = NULL;
            SetStatusBar (NULL);
		}
        else
		{
			m_statusBar = new wxStatusBar (this, wxID_ANY);
			m_statusBar->SetFieldsCount (STATUSPANES);
			m_statusBar->SetStatusText(_(" Connecting..."), STATUS_CONN);
            SetStatusBar (m_statusBar);
		}

        ptermShowTrace ();
        SetCursor (*wxHOURGLASS_CURSOR);
    }

    for (i = 0; i < 5; i++)
    {
        m_charDC[i] = new wxMemoryDC ();
        m_charmap[i] = new wxBitmap (vCharXSize(m_scale), vCharYSize(m_scale), -1);
        m_charDC[i]->SelectObject (*m_charmap[i]);
		m_dirty[i] = true;
    }
    m_bitmap = new wxBitmap (vRealScreenSize(m_scale), vRealScreenSize(m_scale), -1);
    m_memDC = new wxMemoryDC ();
    m_memDC->SelectObject (*m_bitmap);
    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->Clear ();

    SetClientSize (vXRealSize(m_scale) + 2, vYRealSize(m_scale) + 2);
    m_canvas = new PtermCanvas (this);

    /*
    **  Load Plato ROM characters
    */
    ptermLoadRomChars ();
    
    if (port > 0)
    {
        // Create and start the network processing thread
		TRACE2 ("Connecting to: %s:%d", host.c_str(), port);
        m_conn = new PtermConnection (this, host, port);
        if (m_conn->Create () != wxTHREAD_NO_ERROR)
        {
            return;
        }
        m_conn->Run ();
    }
    Show(true);

    m_canvas->SetScrollRate (0, 0);
    m_canvas->SetScrollRate (1, 1);
}

PtermFrame::~PtermFrame ()
{
    int i;
    
    if (m_conn != NULL)
    {
        delete m_conn;
    }
    delete m_memDC;
    delete m_bitmap;
    for (i = 0; i < 5; i++)
    {
        delete m_charDC[i];
        delete m_charmap[i];
    }
    
    // Remove this frame from the app's frame list
    if (m_nextFrame != NULL)
    {
        m_nextFrame->m_prevFrame = m_prevFrame;
    }
    if (m_prevFrame != NULL)
    {
        m_prevFrame->m_nextFrame = m_nextFrame;
    }
    else
    {
        ptermApp->m_firstFrame = m_nextFrame;
    }
}

// menu builders
void PtermFrame::BuildMenuBar(void)
{
    menuBar = new wxMenuBar ();
    menuBar->Append (menuFile, _("File"));
    menuBar->Append (menuEdit, _("Edit"));
#if !0//PTERM_MDI
    menuBar->Append (menuView, _("View"));
#endif
#if defined(__WXMAC__)
    // On the Mac the menu name has to be exactly "&Help" for the About item
    // to  be recognized.  Ugh.
    menuBar->Append(menuHelp, wxT("&Help"));
#else
    menuBar->Append(menuHelp, _("Help"));
#endif
}

void PtermFrame::BuildEditMenu (int port)
{
    menuEdit = new wxMenu;
    menuEdit->Append (Pterm_CopyScreen, _("Copy Screen"), _("Copy screen to clipboard"));
    menuEdit->Append(Pterm_Copy, _("Copy text") ACCELERATOR ("\tCtrl-C"), _("Copy text only to clipboard"));
    if (port > 0)
    {
        menuEdit->Append(Pterm_Paste, _("Paste plain text") ACCELERATOR ("\tCtrl-V"), _("Paste plain text"));
        // No "paste" for help window because it doesn't do input.
        menuEdit->Append(Pterm_PastePrint, _("Paste Printout"), _("Paste Cyber printout format"));
    }
    menuEdit->AppendSeparator ();					
    menuEdit->Append(Pterm_Exec, _("Execute URL") ACCELERATOR ("\tCtrl-X"), _("Execute URL"));
    menuEdit->Append(Pterm_MailTo, _("Mail to...") ACCELERATOR ("\tCtrl-M"), _("Mail to..."));
    menuEdit->Append(Pterm_SearchThis, _("Search this...") ACCELERATOR ("\tCtrl-G"), _("Search this..."));// g=google this?
    if (ptermApp->m_TutorColor && port > 0)
	{
		menuEdit->AppendSeparator ();
		menuEdit->Append(Pterm_Macro0, _("Box 8x") ACCELERATOR ("\tCtrl-0"), _("Box 8x"));
		menuEdit->Append(Pterm_Macro1, _("<c,zc.errf>") ACCELERATOR ("\tCtrl-1"), _("<c,zc.errf>"));
		menuEdit->Append(Pterm_Macro2, _("<c,zc.info>") ACCELERATOR ("\tCtrl-2"), _("<c,zc.info>"));
		menuEdit->Append(Pterm_Macro3, _("<c,zc.keys>") ACCELERATOR ("\tCtrl-3"), _("<c,zc.keys>"));
		menuEdit->Append(Pterm_Macro4, _("<c,zc.text>") ACCELERATOR ("\tCtrl-4"), _("<c,zc.text>"));
		menuEdit->Append(Pterm_Macro5, _("color zc.errf") ACCELERATOR ("\tCtrl-5"), _("color zc.errf"));
		menuEdit->Append(Pterm_Macro6, _("color zc.info") ACCELERATOR ("\tCtrl-6"), _("color zc.info"));
		menuEdit->Append(Pterm_Macro7, _("color zc.keys") ACCELERATOR ("\tCtrl-7"), _("color zc.keys"));
		menuEdit->Append(Pterm_Macro8, _("color zc.text") ACCELERATOR ("\tCtrl-8"), _("color zc.text"));
		menuEdit->Append(Pterm_Macro9, _("Menu colorization") ACCELERATOR ("\tCtrl-9"), _("Menu colorization"));
	}
    menuEdit->Enable (Pterm_Copy, false);			// screen-region related options are disabled until a region is selected
    menuEdit->Enable (Pterm_Exec, false);
    menuEdit->Enable (Pterm_MailTo, false);
    menuEdit->Enable (Pterm_SearchThis, false);
}

void PtermFrame::BuildPopupMenu (int port)
{
    menuPopup = new wxMenu;
    menuPopup->Append (Pterm_CopyScreen, _("Copy Screen"), _("Copy screen to clipboard"));
    menuPopup->Append(Pterm_Copy, _("Copy text") ACCELERATOR ("\tCtrl-C"), _("Copy text only to clipboard"));
    if (port > 0)
    {
        menuPopup->Append(Pterm_Paste, _("Paste ASCII") ACCELERATOR ("\tCtrl-V"), _("Paste plain text"));
        // No "paste" for help window because it doesn't do input.
        menuPopup->Append(Pterm_PastePrint, _("Paste Printout"), _("Paste Cyber printout format"));
    }
    menuPopup->AppendSeparator ();					
    menuPopup->Append(Pterm_Exec, _("Execute URL") ACCELERATOR ("\tCtrl-X"), _("Execute URL"));
    menuPopup->Append(Pterm_MailTo, _("Mail to...") ACCELERATOR ("\tCtrl-M"), _("Mail to..."));
    menuPopup->Append(Pterm_SearchThis, _("Search this...") ACCELERATOR ("\tCtrl-G"), _("Search this..."));// g=google this?
    menuPopup->AppendSeparator ();
#if !defined(__WXMAC__)
    menuPopup->AppendCheckItem (Pterm_ToggleMenuBar, _("Display menu bar"), _("Display menu bar"));
	menuPopup->Check(Pterm_ToggleMenuBar,ptermApp->m_showMenuBar);
#endif
    menuPopup->AppendCheckItem (Pterm_ToggleStatusBar, _("Display status bar"), _("Display status bar"));
	menuPopup->Check(Pterm_ToggleStatusBar,ptermApp->m_showStatusBar);
    menuPopup->AppendCheckItem (Pterm_Toggle2xMode, _("Zoom display 200%"), _("Zoom display 200%"));
	menuPopup->Check(Pterm_Toggle2xMode,(ptermApp->m_scale==2));
    menuPopup->AppendSeparator ();
    menuPopup->AppendCheckItem (Pterm_FullScreen, _("Full Screen") ACCELERATOR ("\tCtrl-U"), _("Display full screen"));
    if (ptermApp->m_TutorColor && port > 0)
	{
		menuPopup->AppendSeparator ();
		menuPopup->Append(Pterm_Macro0, _("Box 8x") ACCELERATOR ("\tCtrl-0"), _("Box 8x"));
		menuPopup->Append(Pterm_Macro1, _("<c,zc.errf>") ACCELERATOR ("\tCtrl-1"), _("<c,zc.errf>"));
		menuPopup->Append(Pterm_Macro2, _("<c,zc.info>") ACCELERATOR ("\tCtrl-2"), _("<c,zc.info>"));
		menuPopup->Append(Pterm_Macro3, _("<c,zc.keys>") ACCELERATOR ("\tCtrl-3"), _("<c,zc.keys>"));
		menuPopup->Append(Pterm_Macro4, _("<c,zc.text>") ACCELERATOR ("\tCtrl-4"), _("<c,zc.text>"));
		menuPopup->Append(Pterm_Macro5, _("color zc.errf") ACCELERATOR ("\tCtrl-5"), _("color zc.errf"));
		menuPopup->Append(Pterm_Macro6, _("color zc.info") ACCELERATOR ("\tCtrl-6"), _("color zc.info"));
		menuPopup->Append(Pterm_Macro7, _("color zc.keys") ACCELERATOR ("\tCtrl-7"), _("color zc.keys"));
		menuPopup->Append(Pterm_Macro8, _("color zc.text") ACCELERATOR ("\tCtrl-8"), _("color zc.text"));
		menuPopup->Append(Pterm_Macro9, _("Menu colorization") ACCELERATOR ("\tCtrl-9"), _("Menu colorization"));
	}
    menuPopup->Enable (Pterm_Copy, false);			// screen-region related options are disabled until a region is selected
    menuPopup->Enable (Pterm_Exec, false);
    menuPopup->Enable (Pterm_MailTo, false);
    menuPopup->Enable (Pterm_SearchThis, false);
}

void PtermFrame::BuildStatusBar (void)
{
	if (m_conn != NULL && !m_fullScreen && ptermApp->m_showStatusBar)
	{
		delete m_statusBar;
		m_statusBar = new wxStatusBar (this, wxID_ANY);
		m_statusBar->SetFieldsCount (STATUSPANES);
		if (m_conn == NULL)
			m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);
		else
	        ptermSetStation (m_station, true, true);
		if (tracePterm)
			m_statusBar->SetStatusText(_(" Trace "), STATUS_TRC);
		else if (ptermApp->m_platoKb)
			m_statusBar->SetStatusText(_(" PLATO keyboard "), STATUS_TRC);
		else
			m_statusBar->SetStatusText(wxT (""), STATUS_TRC);
        SetStatusBar (m_statusBar);
		m_canvas->SetScrollRate (0, 0);
		m_canvas->SetScrollRate (1, 1);
	}
    else
	{
		delete m_statusBar;
	    m_statusBar = NULL;
        SetStatusBar (NULL);
	}
}

// event handlers

void PtermFrame::OnIdle (wxIdleEvent& event)
{
    int word;

	// Do nothing for the help window or other connection-less windows
	if (m_conn == NULL)
	{
		return;
	}

	// If our timer is running, we're using the timer event to drive
	// the display, so ignore idle events.
	if (m_timer.IsRunning ())
	{
		event.Skip ();
		return;
	}

	if (m_nextword != 0)
	{
		procPlatoWord (m_nextword, m_conn->Ascii ());
		m_nextword = 0;
	}

	for (;;)
	{
		/*
		**  Process words until there is nothing left.
		*/
		word = m_conn->NextWord ();
    
		if (word == C_NODATA || word == C_CONNFAIL)
		{
			event.Skip ();
			break;
		}

		// See if we're supposed to delay (but it's not an internal
		// code such as NODATA)
		if ((int) word >= 0 && (word >> 19) != 0)
		{
			m_delay = word >> 19;
			m_nextword = word & 01777777;
			if (m_conn->Ascii ())
			{
				m_timer.Start (8);  // 16.67 / 2.2, rounded
			}
			else
			{
				m_timer.Start (17);
			}
			event.Skip ();
			return;
		}
        
#ifdef DEBUGLOG
        wxLogMessage (wxT("processing data from plato %07o"), word);
#elif DEBUG
        printf ("processing data from plato %07o\n", word);
#endif
		procPlatoWord (word, m_conn->Ascii ());
	}

	switch (word)
		{
		case C_NODATA:
			break;
		case C_CONNFAIL:
			// restart the network processing thread
			if (m_port > 0)
			{
				m_conn = new PtermConnection (this, m_hostName, m_port);
				if (m_conn->Create () != wxTHREAD_NO_ERROR)
				{
					return;
				}
				m_conn->Run ();
			}
			break;
		default:
			event.RequestMore ();
		}

}

void PtermFrame::OnTimer (wxTimerEvent &)
{
    int word;
    
    if (--m_delay > 0)
    {
        return;
    }
#ifdef DEBUGLOG
    wxLogMessage (wxT("processing data from plato %07o"), m_nextword);
#elif DEBUG
    printf ("processing data from plato %07o\n", m_nextword);
#endif
    procPlatoWord (m_nextword, m_conn->Ascii ());
    
    // See what's next.  If no delay is called for, just process it, keep
    // going until no more data or we get a word with delay.
    for (;;)
    {
        word = m_conn->NextWord ();
    
        if (word == C_NODATA)
        {
            m_nextword = 0;
            m_timer.Stop ();
            return;
        }
        m_delay = (word >> 19);
        m_nextword = word & 01777777;
        if (m_delay != 0)
        {
            break;
        }
#ifdef DEBUGLOG
        wxLogMessage (wxT("processing data from plato %07o"), word);
#elif DEBUG
        printf ("processing data from plato %07o\n", word);
#endif
        procPlatoWord (word, m_conn->Ascii ());
    }
}

void PtermFrame::OnPasteTimer (wxTimerEvent &)
{
    wxChar c;
    int p;
    int delay = 0;
    unsigned int nextindex;
    int shift = 0;
	bool neednext = false;
	int autonext = atoi(ptermApp->m_autoLF.mb_str());
	unsigned int tindex;
	int tcnt;
	wxChar tchr;

	if (m_bCancelPaste || m_pasteIndex < 0 || m_pasteIndex >= (int) m_pasteText.Len ())
	{
		//delete roster file if done
		if (m_SendRoster && m_pasteIndex >= (int) m_pasteText.Len ())
			KillRoster();
		//reset flags
		m_bCancelPaste = false;
		m_bPasteActive = false;
		m_SendRoster = false;
		m_WaitReady = false;
		return;
	}


    if (m_pasteIndex < 0 ||
        m_pasteIndex >= (int) m_pasteText.Len ())
    {
        m_pasteIndex = -1;
        return;
    }
    
    nextindex = m_pasteIndex;

    if (m_pastePrint)
    {
        while (m_pasteIndex < (int) m_pasteText.Len ())
        {
            c = m_pasteText[nextindex++];
            p = -1;

            // Pasting a printout string, with ' for shift and other fun stuff.
            if (c == wxT('\''))
            {
                if (shift)
                {
                    // Odd -- two shift codes in a row.  Send the first
                    // one as a standalone shift.
                    ptermSendKey (055);     // shift-Assign is shift code
                }
                shift = 040;
                continue;
            }
            if (c < (int) (sizeof (printoutToPlato) / sizeof (printoutToPlato[0])))
            {
                p = printoutToPlato[c];
            }
            if (p != -1)
            {
                // Look for a shift code preceding a character that is a shifted
                // character (like $), or an unshifted character whose shifted form
                // corresponds to a different printable character (like 4).  For
                // those cases, if we see a shift code, send that separately.
                // For example, '4 does not mean $, it means a shift code then 4
                // (which is an embedded mode change).
                if (shift != 0 &&
                    ((p & 040) != 0 || strchr ("012345689=", c) != NULL))
                {
                    ptermSendKey (055);     // shift-Assign is shift code
                    shift = 0;
                }
                ptermSendKey (p | shift);
            }
            else if (shift)
            {
                ptermSendKey (055);     // shift-Assign is shift code
            }
            break;
        }
    }
    else
    {
        c = m_pasteText[nextindex];
        p = -1;


		bool found = false;
		// check if sending roster but waiting for "ready" signal
		if (m_SendRoster && m_WaitReady)
		{
/*
			wxString l_text;
			l_text.Printf(wxT("Waiting for '*READY*', found: %c%c%c%c%c%c%c"), 
				rom01char[m_canvas->textmap[0 * 64 + 0] & 0xff],
				rom01char[m_canvas->textmap[0 * 64 + 1] & 0xff],
			    rom01char[m_canvas->textmap[0 * 64 + 2] & 0xff],
			    rom01char[m_canvas->textmap[0 * 64 + 3] & 0xff],
			    rom01char[m_canvas->textmap[0 * 64 + 4] & 0xff],
			    rom01char[m_canvas->textmap[0 * 64 + 5] & 0xff],
			    rom01char[m_canvas->textmap[0 * 64 + 6] & 0xff]
				);
			wxMessageBox(l_text, _("DEBUG"), wxOK | wxICON_INFORMATION, NULL);
			l_text.Printf(wxT("Waiting for '*READY*', found: %d%d%d%d%d%d%d"), 
				m_canvas->textmap[32 * 64 + 0] & 0xff,
				m_canvas->textmap[32 * 64 + 1] & 0xff,
			    m_canvas->textmap[32 * 64 + 2] & 0xff,
			    m_canvas->textmap[32 * 64 + 3] & 0xff,
			    m_canvas->textmap[32 * 64 + 4] & 0xff,
			    m_canvas->textmap[32 * 64 + 5] & 0xff,
			    m_canvas->textmap[32 * 64 + 6] & 0xff
				);
			wxMessageBox(l_text, _("DEBUG"), wxOK | wxICON_INFORMATION, NULL);
			l_text.Printf(wxT("Waiting for '*READY*', found: %d%d%d%d%d%d%d"), 
				m_canvas->textmap[31 * 64 + 0] & 0xff,
				m_canvas->textmap[31 * 64 + 1] & 0xff,
			    m_canvas->textmap[31 * 64 + 2] & 0xff,
			    m_canvas->textmap[31 * 64 + 3] & 0xff,
			    m_canvas->textmap[31 * 64 + 4] & 0xff,
			    m_canvas->textmap[31 * 64 + 5] & 0xff,
			    m_canvas->textmap[31 * 64 + 6] & 0xff
				);
			wxMessageBox(l_text, _("DEBUG"), wxOK | wxICON_INFORMATION, NULL);
*/
			if (((rom01char[m_canvas->textmap[0 * 64 + 0] & 0xff]) == '*') && 
			    ((rom01char[m_canvas->textmap[0 * 64 + 1] & 0xff]) == 'R') && 
			    ((rom01char[m_canvas->textmap[0 * 64 + 2] & 0xff]) == 'E') && 
			    ((rom01char[m_canvas->textmap[0 * 64 + 3] & 0xff]) == 'A') && 
			    ((rom01char[m_canvas->textmap[0 * 64 + 4] & 0xff]) == 'D') && 
			    ((rom01char[m_canvas->textmap[0 * 64 + 5] & 0xff]) == 'Y') && 
			    ((rom01char[m_canvas->textmap[0 * 64 + 6] & 0xff]) == '*')) 
			{
				//wxMessageBox(wxT ("Got *READY* at 3201"), _("DEBUG"), wxOK | wxICON_INFORMATION, NULL);
				m_WaitReady = false;
				int omode = mode;
				mode = 2;
				ptermBlockErase(0,0,7*8,15);
				mode = omode;
			}
			else
			{
				nextindex--;
				delay = 100;
				m_WaitReady = true;
			found = true;
			}
		}
		// check if sending roster
		else if (m_SendRoster)
		{
			wxString tascii[] = {
								wxT("[DELAY10]"),
								wxT("[WAITREADY]") 
								};
			for (int i = 0; !found && i < (int)(sizeof(tascii) / sizeof(tascii[0])); i++)
				if (tascii[i].Cmp(m_pasteText.Mid(nextindex,tascii[i].Length())) == 0)
				{
					switch (i)
					{
					case 0:
						delay = 10000;
						m_WaitReady = false;
						break;
					case 1:
						delay = 100;
						m_WaitReady = true;
						break;
					}
					nextindex += tascii[i].Length()-1;
					found = true;
				}
		}
		else if (ptermApp->m_smartPaste)
		{
			wxString tascii[] = {
								wxT(".       "), 
								wxT("        "), 
								wxT("<("), 
								wxT(")>")
								};
			int kplato[] =  {
							asciiToPlato['.'], 014,
							014,			   -1,
							024,               asciiToPlato['0'],
							024,               asciiToPlato['1']
							};
			for (int i = 0; !found && i < (int)(sizeof(tascii) / sizeof(tascii[0])); i++)
				if (tascii[i].Cmp(m_pasteText.Mid(nextindex,tascii[i].Length())) == 0)
				{
					ptermSendKey(kplato[2*i+0]);
					if (kplato[2*i+1] != -1)
						ptermSendKey(kplato[2*i+1]);
					nextindex += tascii[i].Length()-1;
					found = true;
				}
		}
        if (!found && (c <= wxT(' ')))
        {
            // Most get ignored
			switch (c)
			{
			case wxT ('\n'):
                p = 026;        // NEXT
				break;
            case wxT ('\t'):
                p = 014;        // TAB
				break;
            case wxT (' '):
                p = 0100;       // space
				break;
			case wxT('\xAB'):	// assignment arrow
				p = 015;
				break;
			case wxT('\xBB'):	// arrow
	            ptermSendKey (024);
	            p = asciiToPlato['6'];
				break;
			}
			found = true;
        }
		if (!found && (c < (int)(sizeof(asciiToPlato) / sizeof(asciiToPlato[0]))))
		{
			p = asciiToPlato[c];
			found = true;
		}
		nextindex++;

        if (p != -1)
        {
			//send the key
            ptermSendKey (p);
			//look ahead to see if line should be split at this breakpoint (space or hyphen)
			if (autonext != 0 && !ptermApp->m_splitWords && (c == wxT(' ') || c == wxT('-')))
			{
				for (tindex = nextindex, neednext = true, tcnt = m_pasteNextKeyCnt; tindex < m_pasteText.Len() && neednext && tcnt < autonext; tindex++, tcnt++)
				{
					tchr = m_pasteText[tindex];
					if (tchr == wxT(' ') || tchr == wxT('-')  || tindex == m_pasteText.Len()-1)
						neednext = false;
				}
				if (neednext)
					m_pasteNextKeyCnt = 0;
			}
			m_pasteNextKeyCnt++;
			if (autonext != 0 && m_pasteNextKeyCnt == autonext)
			{
				neednext = (p != 026);// NEXT
				m_pasteNextKeyCnt = 0;
			}
        }
    }
    
    if (nextindex < m_pasteText.Len ())
    {
        // Still more to do.  Update the index (which is cleared to -1
        // by ptermSendKey), then restart the timer with the
        // appropriate delay (char delay or line delay).
        m_pasteIndex = nextindex;
		if (delay != 0)
			;
        else if (c == wxT ('\n'))
		{
			delay = atoi(ptermApp->m_lineDelay.mb_str());
			neednext = false;
			m_pasteNextKeyCnt = 0;
		}
        else
        {
            delay = atoi(ptermApp->m_charDelay.mb_str());
        }
        m_pasteTimer.Start (delay, true);
		m_bPasteActive = true;
		//check if need to send a NEXT as part of the automatic newline feature
		if (neednext)
		{
			neednext = false;
			m_pasteNextKeyCnt = 0;
            ptermSendKey (026);
			for ( ; !ptermApp->m_splitWords && m_pasteText[nextindex] == wxT(' ') && nextindex < m_pasteText.Len(); nextindex++);//eat leading spaces
			m_pasteIndex = nextindex;
			delay = atoi(ptermApp->m_lineDelay.mb_str());
			m_pasteTimer.Start (delay, true);
			m_bPasteActive = true;
		}

    }
    else
    {
		//delete roster file if done
		if (m_SendRoster)
			KillRoster();
		//reset flags
		m_bCancelPaste = false;
		m_bPasteActive = false;
		m_SendRoster = false;
		m_WaitReady = false;
    }
}

void PtermFrame::OnClose (wxCloseEvent &)
{
    int i, x, y, xs, ys;
    
    if (m_conn != NULL)
    {
        m_conn->Delete ();
    }

    m_memDC->SetBackground (wxNullBrush);
    m_memDC->SetPen (wxNullPen);
    m_memDC->SelectObject (wxNullBitmap);

    for (i = 0; i < 5; i++)
    {
        m_charDC[i]->SetBackground (wxNullBrush);
        m_charDC[i]->SetPen (wxNullPen);
        m_charDC[i]->SelectObject (wxNullBitmap);
    }
    
    if (this == ptermApp->m_helpFrame)
    {
        ptermApp->m_helpFrame = NULL;
    }
    else
    {
        GetPosition (&x, &y);
        wxDisplaySize (&xs, &ys);
        if (x > 0 && x < xs - vXSize(m_scale) &&
            y > 0 && y < ys - vYSize(m_scale))
        {
            ptermApp->lastX = x;
            ptermApp->lastY = y;
        }
    }

    if (m_nextFrame != NULL && IsActive ())
    {
        m_nextFrame->Raise ();
    }
    else if (m_prevFrame != NULL && IsActive ())
    {
        m_prevFrame->Raise ();
    }
    
    Destroy ();
}

void PtermFrame::OnQuit(wxCommandEvent&)
{
    // true is to force the frame to close
    Close (true);
}

void PtermFrame::OnActivate (wxActivateEvent &event)
{
    if (m_canvas != NULL)
    {
        m_canvas->SetFocus ();
    }
    event.Skip ();        // let others see the event, too
}

void PtermFrame::OnCopyScreen (wxCommandEvent &)
{
    wxBitmap screenmap (vRealScreenSize(m_scale), vRealScreenSize(m_scale));
    wxMemoryDC screenDC;
    wxBitmapDataObject *screen;

    screenDC.SelectObject (screenmap);
    screenDC.Blit (0, 0, vScreenSize(m_scale), vScreenSize(m_scale), m_memDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);

    screen = new wxBitmapDataObject(screenmap);

    if (wxTheClipboard->Open ())
    {
        if (!wxTheClipboard->SetData (screen))
        {
            wxLogError (_("Can't copy image to the clipboard"));
        }
        wxTheClipboard->Close ();
    }
    else
    {
        wxLogError (_("Can't open clipboard."));
    }
}

#if !defined(__WXMAC__)
void PtermFrame::OnToggleMenuBar (wxCommandEvent &)
{
	int ww,wh,ow,oh,nw,nh;

	//get window parameters pre-prefs
	GetSize(&ww,&wh);
	GetClientSize(&ow,&oh);

	//toggle menu
	ptermApp->m_showMenuBar = !ptermApp->m_showMenuBar;
	menuPopup->Check(Pterm_ToggleMenuBar,ptermApp->m_showMenuBar);
	menuView->Check(Pterm_ToggleMenuBarView,ptermApp->m_showMenuBar);
	SavePreferences();
	if (m_fullScreen || !ptermApp->m_showMenuBar)
		SetMenuBar(NULL);
	else
	{
		// append menus to bar
		menuBar = new wxMenuBar ();
		menuBar->Append (menuFile, _("File"));
		menuBar->Append (menuEdit, _("Edit"));
        menuBar->Append (menuView, _("View"));
        menuBar->Append(menuHelp, _("Help"));
		SetMenuBar(menuBar);
	}

	//check if size changed
	GetClientSize(&nw,&nh);
	SetSize(ww,wh-(nh-oh));
	m_canvas->SetScrollRate (0, 0);
	m_canvas->SetScrollRate (1, 1);

}
#endif

void PtermFrame::OnToggleStatusBar (wxCommandEvent &)
{
	int ww,wh,ow,oh,nw,nh;

	//get window parameters pre-prefs
	GetSize(&ww,&wh);
	GetClientSize(&ow,&oh);

	//toggle status
	ptermApp->m_showStatusBar = !ptermApp->m_showStatusBar;
	menuPopup->Check(Pterm_ToggleStatusBar,ptermApp->m_showStatusBar);
	menuView->Check(Pterm_ToggleStatusBarView,ptermApp->m_showStatusBar);
	SavePreferences();
	BuildStatusBar();

	//check if size changed
	GetClientSize(&nw,&nh);
	SetSize(ww,wh-(nh-oh));
	m_canvas->SetScrollRate (0, 0);
	m_canvas->SetScrollRate (1, 1);

}

void PtermFrame::OnToggle2xMode (wxCommandEvent &)
{

	//toggle status
    UpdateSettings (ptermApp->m_fgColor, ptermApp->m_bgColor, (ptermApp->m_scale==1));
	ptermApp->m_scale = (ptermApp->m_scale==2) ? 1 : 2;
	m_canvas->m_scale = ptermApp->m_scale;
	m_scale = ptermApp->m_scale;
	menuPopup->Check(Pterm_Toggle2xMode,(ptermApp->m_scale==2));
	menuView->Check(Pterm_Toggle2xModeView,(ptermApp->m_scale==2));
	SavePreferences();

	//refit
	FitInside();
	m_canvas->SetScrollRate (0, 0);
	m_canvas->SetScrollRate (1, 1);

}

void PtermFrame::OnCopy (wxCommandEvent &event)
{
    m_canvas->OnCopy (event);
}

void PtermFrame::OnExec (wxCommandEvent &event)
{

    m_canvas->OnCopy (event);

    if (!wxTheClipboard->Open ())
    {
        wxLogError (_("Can't open clipboard to execute."));
        return;
    }

    if (!wxTheClipboard->IsSupported (wxDF_TEXT))
    {
        wxLogWarning (_("No text data on clipboard to execute"));
        wxTheClipboard->Close ();
        return;
    }

    wxTextDataObject text;

    if (!wxTheClipboard->GetData (text))
    {
        wxLogError (_("Can't paste data from the clipboard to execute"));
    }
    else
    {
		wxString url = text.GetText();
		url.Replace(wxT ("\n"), wxT (""));
        wxExecute (ptermApp->m_Browser + wxT (" ") + url);
    }

    wxTheClipboard->Close ();
}

void PtermFrame::OnMailTo (wxCommandEvent &event)
{
	wxString l_Email;
	wxString l_FixText;
	wxString newchr;
	wxString pnt;
	int cnt;

    m_canvas->OnCopy (event);

    if (!wxTheClipboard->Open ())
    {
        wxLogError (_("Can't open clipboard to save email address"));
        return;
    }

    if (!wxTheClipboard->IsSupported (wxDF_TEXT))
    {
        wxLogWarning (_("No text data on clipboard for email address"));
        wxTheClipboard->Close ();
        return;
    }

    wxTextDataObject text;

    if (!wxTheClipboard->GetData (text))
    {
        wxLogError (_("Can't get email address data from the clipboard"));
    }
    else
    {
		for (pnt = text.GetText(), cnt = 0; pnt[cnt]; cnt++)
		{
			if (pnt[cnt] == '/')
				newchr = wxChar ('@');
			//fix ' at ' , '(at)', and '[at]'
			else if (pnt[cnt] == ' ' && pnt[cnt+1] == 'a' && pnt[cnt+2] == 't' && pnt[cnt+3] == ' ')
				cnt += 3, newchr = wxChar ('@');
			else if (pnt[cnt] == '(' && pnt[cnt+1] == 'a' && pnt[cnt+2] == 't' && pnt[cnt+3] == ')')
				cnt += 3, newchr = wxChar ('@');
			else if (pnt[cnt] == '[' && pnt[cnt+1] == 'a' && pnt[cnt+2] == 't' && pnt[cnt+3] == ']')
				cnt += 3, newchr = wxChar ('@');
			//fix ' dot ' , '(dot)', and '[dot]'
			else if (pnt[cnt] == ' ' && pnt[cnt+1] == 'd' && pnt[cnt+2] == 'o' && pnt[cnt+3] == 't' && pnt[cnt+4] == ' ')
				cnt += 4, newchr = wxChar ('.');
			else if (pnt[cnt] == '(' && pnt[cnt+1] == 'd' && pnt[cnt+2] == 'o' && pnt[cnt+3] == 't' && pnt[cnt+4] == ')')
				cnt += 4, newchr = wxChar ('.');
			else if (pnt[cnt] == '[' && pnt[cnt+1] == 'd' && pnt[cnt+2] == 'o' && pnt[cnt+3] == 't' && pnt[cnt+4] == ']')
				cnt += 4, newchr = wxChar ('.');
			//strip 'nospam'
			else if (pnt[cnt] == 'n' && pnt[cnt+1] == 'o' && pnt[cnt+2] == 's' && pnt[cnt+3] == 'p' && pnt[cnt+4] == 'a' && pnt[cnt+5] == 'm')
				cnt += 5, newchr = wxChar ('*');
			//fix comma
			else if (pnt[cnt] == ',')
				newchr = wxChar ('.');
			else
				newchr = pnt[cnt];
			if (newchr != '*')
				l_FixText += newchr;
		}
		l_Email.Printf(ptermApp->m_Email,l_FixText.c_str());
        wxExecute ( l_Email );
    }

    wxTheClipboard->Close ();
}

void PtermFrame::OnSearchThis (wxCommandEvent &event)
{
	wxString l_FixText;
	wxString newchr;
	wxString pnt;
	int cnt;

    m_canvas->OnCopy (event);

    if (!wxTheClipboard->Open ())
    {
        wxLogError (_("Can't open clipboard to save search key"));
        return;
    }

    if (!wxTheClipboard->IsSupported (wxDF_TEXT))
    {
        wxLogWarning (_("No text data on clipboard for search"));
        wxTheClipboard->Close ();
        return;
    }

    wxTextDataObject text;

    if (!wxTheClipboard->GetData (text))
    {
        wxLogError (_("Can't get search key data from the clipboard"));
    }
    else
    {
		for (pnt = text.GetText(), cnt = 0; pnt[cnt]; cnt++)
		{
			if (pnt[cnt] == ' ' && newchr != '+')
				newchr = wxChar ('+');
			else if (pnt[cnt] >= 'a' && pnt[cnt] <= 'z')
				newchr = pnt[cnt];
			else if (pnt[cnt] >= 'A' && pnt[cnt] <= 'Z')
				newchr = pnt[cnt];
			else if (pnt[cnt] >= '0' && pnt[cnt] <= '9')
				newchr = pnt[cnt];
			else
				newchr.Printf(wxT ("%%%02x"),pnt[cnt]);
			l_FixText += newchr;
		}
		wxExecute (ptermApp->m_Browser + wxT (" ") + ptermApp->m_SearchURL + l_FixText);
    }

    wxTheClipboard->Close ();
}

void PtermFrame::OnSpellCheck (wxCommandEvent &event)
{
	wxString l_FixText;
	wxString newchr;
	wxString pnt;
	int cnt;

    m_canvas->OnCopy (event);

    if (!wxTheClipboard->Open ())
    {
        wxLogError (_("Can't open clipboard to save text"));
        return;
    }

    if (!wxTheClipboard->IsSupported (wxDF_TEXT))
    {
        wxLogWarning (_("No text data on clipboard for spell check"));
        wxTheClipboard->Close ();
        return;
    }

    wxTextDataObject text;

    if (!wxTheClipboard->GetData (text))
    {
        wxLogError (_("Can't paste text from the clipboard"));
    }
    else
    {
		for (pnt = text.GetText(), cnt = 0; pnt[cnt]; cnt++)
		{
			if (pnt[cnt] == ' ' && newchr != '+')
				newchr = wxChar ('+');
			else if (pnt[cnt] >= 'a' && pnt[cnt] <= 'z')
				newchr = pnt[cnt];
			else if (pnt[cnt] >= 'A' && pnt[cnt] <= 'Z')
				newchr = pnt[cnt];
			else if (pnt[cnt] >= '0' && pnt[cnt] <= '9')
				newchr = pnt[cnt];
			else
				newchr.Printf(wxT ("%%%02x"),pnt[cnt]);
			l_FixText += newchr;
		}
		wxExecute (ptermApp->m_Browser + wxT (" ") + ptermApp->m_SearchURL + l_FixText);
    }

    wxTheClipboard->Close ();
}

void PtermFrame::OnMacro0 (wxCommandEvent &event)
{
	int key[] = {0034,0034,0034,0034,0034,0034,0034,0034,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro1 (wxCommandEvent &event)
{
	int key[] = {0024,0000,0103,0137,0132,0103,0136,0105,0122,0122,0106,0024,0001,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro2 (wxCommandEvent &event)
{
	int key[] = {0024,0000,0103,0137,0132,0103,0136,0111,0116,0106,0117,0024,0001,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro3 (wxCommandEvent &event)
{
	int key[] = {0024,0000,0103,0137,0132,0103,0136,0113,0105,0131,0123,0024,0001,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro4 (wxCommandEvent &event)
{
	int key[] = {0024,0000,0103,0137,0132,0103,0136,0124,0105,0130,0124,0024,0001,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro5 (wxCommandEvent &event)
{
	int key[] = {0103,0117,0114,0117,0122,0100,0100,0100,0104,0111,0123,0120,0114,0101,0131,0134,0132,0103,0136,0105,0122,0122,0106,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro6 (wxCommandEvent &event)
{
	int key[] = {0103,0117,0114,0117,0122,0100,0100,0100,0104,0111,0123,0120,0114,0101,0131,0134,0132,0103,0136,0111,0116,0106,0117,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro7 (wxCommandEvent &event)
{
	int key[] = {0103,0117,0114,0117,0122,0100,0100,0100,0104,0111,0123,0120,0114,0101,0131,0134,0132,0103,0136,0113,0105,0131,0123,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro8 (wxCommandEvent &event)
{
	int key[] = {0103,0117,0114,0117,0122,0100,0100,0100,0104,0111,0123,0120,0114,0101,0131,0134,0132,0103,0136,0124,0105,0130,0124,-1};
	ptermSendKeys(key);
}
void PtermFrame::OnMacro9 (wxCommandEvent &event)
{
	// 8boxes, <c,zc.keys> 2 boxes <c,zc.text>
	int key0[] = {0034,0034,0034,0034,0034,0034,0034,0034,-1};
	ptermSendKeys(key0);
	int key3[] = {0024,0000,0103,0137,0132,0103,0136,0113,0105,0131,0123,0024,0001,-1};
	ptermSendKeys(key3);
	int key[] = {0034,0034,-1};
	ptermSendKeys(key);
	int key4[] = {0024,0000,0103,0137,0132,0103,0136,0124,0105,0130,0124,0024,0001,-1};
	ptermSendKeys(key4);
}

void PtermFrame::SendRoster (void)
{
	//read contents of Roster File
	
	wxString buffer;

	//open file
	wxTextFile file(ptermApp->m_RosterFile);
	if (!file.Exists())
		return;
	if (!file.Open())
		return;

	//read file
	m_pasteText.Clear();
	for ( buffer = file.GetFirstLine(); ; buffer = file.GetNextLine() )
	{
		m_pasteText.Append(buffer);
		m_pasteText.Append(wxT("\n"));
		if (file.Eof())
			break;
	}
	file.Close();
	//force certain options to expected values
	ptermApp->m_splitWords = false;
	ptermApp->m_smartPaste = false;
	ptermApp->m_autoLF = wxT("0");
	ptermApp->m_conv8Sp = false;
	ptermApp->m_convDot7 = false;
	//start pasting
	m_SendRoster = true;
	m_pasteIndex = 0;
	m_pasteNextKeyCnt = 0;
	m_pasteTimer.Start (atoi(ptermApp->m_charDelay.mb_str()), true);
}

void PtermFrame::KillRoster (void)
{
	//delete file
	wxRemoveFile(ptermApp->m_RosterFile);
}

void PtermFrame::OnPaste (wxCommandEvent &event)
{
    if (!wxTheClipboard->Open ())
    {
        wxLogError (_("Can't open clipboard."));

        return;
    }

    if (!wxTheClipboard->IsSupported (wxDF_TEXT))
    {
        wxLogWarning (_("No text data on clipboard"));

        wxTheClipboard->Close ();
        return;
    }

    wxTextDataObject text;

    if (!wxTheClipboard->GetData (text))
    {
        wxLogError (_("Can't paste data from the clipboard"));
    }
    else
    {
        m_pasteText = text.GetText ();
        m_pasteIndex = 0;
		m_pasteNextKeyCnt = 0;
        m_pasteTimer.Start (atoi(ptermApp->m_charDelay.mb_str()), true);
        m_pastePrint = (event.GetId () == Pterm_PastePrint);
    }

    wxTheClipboard->Close ();
}

void PtermFrame::OnUpdateUIPaste (wxUpdateUIEvent& event)
{
#ifdef __WXDEBUG__
    // too many trace messages if we don't do it - this function is called
    // very often
    wxLogNull nolog;
#endif

    event.Enable (wxTheClipboard->IsSupported (wxDF_TEXT));
}

void PtermFrame::OnSaveScreen (wxCommandEvent &)
{
    wxBitmap screenmap (vRealScreenSize(m_scale), vRealScreenSize(m_scale));
    wxMemoryDC screenDC;
    wxString filename, ext;
    wxBitmapType type;
    wxFileDialog fd (this, _("Save screen to"), ptermApp->m_defDir,
                     wxT(""), wxT("PNG files (*.png)|*.png|"
                                  "BMP files (*.bmp)|*.bmp|"
                                  "PNM files (*.pnm)|*.pnm|"
                                  "TIF files (*.tif)|*.tif|"
                                  "XPM files (*.xpm)|*.xpm|"
                                  "All files (*.*)|*.*"),
                     wxSAVE | wxOVERWRITE_PROMPT);
    
    if (fd.ShowModal () != wxID_OK)
    {
        return;
    }
    filename = fd.GetPath ();
    
    screenDC.SelectObject (screenmap);
    screenDC.Blit (0, 0, vScreenSize(m_scale), vScreenSize(m_scale), m_memDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);

    wxImage screenImage = screenmap.ConvertToImage ();
    wxFileName fn (filename);
    
    ptermApp->m_defDir = fn.GetPath ();
    ext = fn.GetExt ();
    if (ext.CmpNoCase (wxT ("bmp")) == 0)
    {
        type = wxBITMAP_TYPE_BMP;
    }
#if 0 //temp
    else if (ext.CmpNoCase (wxT ("png")) == 0)
    {
        type = wxBITMAP_TYPE_PNG;
    }
#endif
    else if (ext.CmpNoCase (wxT ("pnm")) == 0)
    {
        type = wxBITMAP_TYPE_PNM;
    }
#if 0 //temp
    else if (ext.CmpNoCase (wxT ("tif")) == 0 ||
             ext.CmpNoCase (wxT ("tiff")) == 0)
    {
        type = wxBITMAP_TYPE_TIF;
	// 32773 is PACKBITS -- not referenced symbolically because tiff.h
	// isn't necessarily anywhere, for some reason.
        screenImage.SetOption (wxIMAGE_OPTION_COMPRESSION, 32773);
    }
#endif
    else if (ext.CmpNoCase (wxT ("xpm")) == 0)
    {
        type = wxBITMAP_TYPE_XPM;
    }
    else
    {
        screenImage.SaveFile (filename);
        return;
    }
    
    screenImage.SaveFile (filename, type);
}

void PtermFrame::OnPrint (wxCommandEvent &)
{
    wxPrintDialogData printDialogData (*g_printData);

    printDialogData.EnableSelection (false);
    printDialogData.EnablePageNumbers (false);
    
    wxPrinter printer (& printDialogData);
	
	ptermApp->m_CurFrameScreenSize = vRealScreenSize(m_scale);
	ptermApp->m_CurFrameScale = m_scale;

    PtermPrintout printout (this);
    if (!printer.Print (this, &printout, true /*prompt*/))
    {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            wxMessageBox (_("There was a problem printing.\nPerhaps your current printer is not set correctly?"), _("Printing"), wxOK);
    }
    else
    {
        (*g_printData) = printer.GetPrintDialogData ().GetPrintData ();
    }
}

void PtermFrame::OnPrintPreview (wxCommandEvent &)
{
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData (*g_printData);
    wxPrintPreview *preview = new wxPrintPreview (new PtermPrintout (this),
                                                  new PtermPrintout (this),
                                                  &printDialogData);

    printDialogData.EnableSelection (false);
    printDialogData.EnablePageNumbers (false);
    
    if (!preview->Ok())
    {
        delete preview;
        wxMessageBox(_("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), _("Previewing"), wxOK);
        return;
    }

    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _("Pterm Print Preview"), wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show();
}

void PtermFrame::OnPageSetup(wxCommandEvent &)
{

/*
	//show charsets on screen
	wxClientDC dc(m_canvas);
	dc.Blit (XTOP, YTOP+tvCharYSize(m_scale)*0, vCharXSize(m_scale), vCharYSize(m_scale), m_charDC[0], 0, 0, wxCOPY);
	dc.Blit (XTOP, YTOP+tvCharYSize(m_scale)*1, vCharXSize(m_scale), vCharYSize(m_scale), m_charDC[1], 0, 0, wxCOPY);
	dc.Blit (XTOP, YTOP+tvCharYSize(m_scale)*2, vCharXSize(m_scale), vCharYSize(m_scale), m_charDC[2], 0, 0, wxCOPY);
	dc.Blit (XTOP, YTOP+tvCharYSize(m_scale)*3, vCharXSize(m_scale), vCharYSize(m_scale), m_charDC[3], 0, 0, wxCOPY);
	dc.Blit (XTOP, YTOP+tvCharYSize(m_scale)*4, vCharXSize(m_scale), vCharYSize(m_scale), m_charDC[4], 0, 0, wxCOPY);
	wxString str = wxT("");
	str.Printf(wxT(  "0 m_charDC=%p     m_charmap=%p\n"),    m_charDC[0],m_charmap[0]);
	str.Printf(wxT("%s1 m_charDC=%p     m_charmap=%p\n"),str,m_charDC[1],m_charmap[1]);
	str.Printf(wxT("%s2 m_charDC=%p     m_charmap=%p\n"),str,m_charDC[2],m_charmap[2]);
	str.Printf(wxT("%s3 m_charDC=%p     m_charmap=%p\n"),str,m_charDC[3],m_charmap[3]);
	str.Printf(wxT("%s4 m_charDC=%p     m_charmap=%p\n"),str,m_charDC[4],m_charmap[4]);
*/

 	(*g_pageSetupData) = *g_printData;

    wxPageSetupDialog pageSetupDialog (this, g_pageSetupData);
    pageSetupDialog.ShowModal ();

    (*g_printData) = pageSetupDialog.GetPageSetupData ().GetPrintData ();
    (*g_pageSetupData) = pageSetupDialog.GetPageSetupData ();

}

void PtermFrame::OnPref (wxCommandEvent&)
{
	int ww,wh,ow,oh,nw,nh;

	//get window parameters pre-prefs
	GetSize(&ww,&wh);
	GetClientSize(&ow,&oh);

	//show dialog
	PtermPrefDialog dlg (NULL, wxID_ANY, _("Pterm Preferences"), wxDefaultPosition, wxSize( 451,400 ));
    
	//process changes if OK clicked
    if (dlg.ShowModal () == wxID_OK)
    {
        UpdateSettings (dlg.m_fgColor, dlg.m_bgColor, dlg.m_scale2);
		//get prefs
		ptermApp->m_lastTab = dlg.m_lastTab;
        //tab0
        ptermApp->m_curProfile = dlg.m_curProfile;
		//tab1
        ptermApp->m_ShellFirst = dlg.m_ShellFirst;
        ptermApp->m_connect = dlg.m_connect;
        ptermApp->m_hostName = dlg.m_host;
        ptermApp->m_port = atoi (wxString (dlg.m_port).mb_str());
        //tab2
		ptermApp->m_showSignon = dlg.m_showSignon;
		ptermApp->m_showSysName = dlg.m_showSysName;
		ptermApp->m_showHost = dlg.m_showHost;
		ptermApp->m_showStation = dlg.m_showStation;
        //tab3
        ptermApp->m_classicSpeed = dlg.m_classicSpeed;
        ptermApp->m_gswEnable = dlg.m_gswEnable;
        ptermApp->m_numpadArrows = dlg.m_numpadArrows;
        ptermApp->m_ignoreCapLock = dlg.m_ignoreCapLock;
        ptermApp->m_platoKb = dlg.m_platoKb;
		ptermApp->m_useAccel = dlg.m_useAccel;
        ptermApp->m_beepEnable = dlg.m_beepEnable;
        ptermApp->m_DisableShiftSpace = dlg.m_DisableShiftSpace;
        ptermApp->m_DisableMouseDrag = dlg.m_DisableMouseDrag;
		//tab4
        //ptermApp->m_scale = (dlg.m_scale2) ? 2 : 1;
		//m_scale = ptermApp->m_scale;
        //ptermApp->m_showStatusBar = dlg.m_showStatusBar;
#if !defined(__WXMAC__)
        //ptermApp->m_showMenuBar = dlg.m_showMenuBar;
#endif
        ptermApp->m_noColor = dlg.m_noColor;
        ptermApp->m_fgColor = dlg.m_fgColor;
        ptermApp->m_bgColor = dlg.m_bgColor;
		//tab5
        ptermApp->m_charDelay = dlg.m_charDelay;
        ptermApp->m_lineDelay = dlg.m_lineDelay;
        ptermApp->m_autoLF = dlg.m_autoLF;
		ptermApp->m_splitWords = dlg.m_splitWords;
		ptermApp->m_smartPaste = dlg.m_smartPaste;
		ptermApp->m_convDot7 = dlg.m_convDot7;
		ptermApp->m_conv8Sp = dlg.m_conv8Sp;
		ptermApp->m_TutorColor = dlg.m_TutorColor;
		//tab6
		ptermApp->m_Browser = dlg.m_Browser;
		ptermApp->m_Email = dlg.m_Email;
		ptermApp->m_SearchURL = dlg.m_SearchURL;

		//update settings to config
		SavePreferences();

		//rebuild menus
		BuildPopupMenu(1);
		SetMenuBar(NULL);
		BuildEditMenu(1);
		BuildMenuBar();
#if !defined(__WXMAC__)
		if (m_conn != NULL && !m_fullScreen && ptermApp->m_showMenuBar)
#endif
			SetMenuBar(menuBar);

		BuildStatusBar();

		//check if size changed
		GetClientSize(&nw,&nh);
		SetSize(ww,wh-(nh-oh));
		m_canvas->SetScrollRate (0, 0);
		m_canvas->SetScrollRate (1, 1);

    }
}

void PtermFrame::SavePreferences(void)
{
    wxString rgb;
    //write prefs
	ptermApp->m_config->Write (wxT (PREF_LASTTAB), ptermApp->m_lastTab);
	//tab0
	ptermApp->m_config->Write (wxT (PREF_CURPROFILE), ptermApp->m_curProfile);
	//tab1
    ptermApp->m_config->Write (wxT (PREF_SHELLFIRST), ptermApp->m_ShellFirst);
    ptermApp->m_config->Write (wxT (PREF_CONNECT), (ptermApp->m_connect) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_HOST), m_hostName);
    ptermApp->m_config->Write (wxT (PREF_PORT), ptermApp->m_port);
	//tab2
    ptermApp->m_config->Write (wxT (PREF_SHOWSIGNON), (ptermApp->m_showSignon) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_SHOWSYSNAME), (ptermApp->m_showSysName) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_SHOWHOST), (ptermApp->m_showHost) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_SHOWSTATION), (ptermApp->m_showStation) ? 1 : 0);
	//tab3
    ptermApp->m_config->Write (wxT (PREF_1200BAUD), (ptermApp->m_classicSpeed) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_GSW), (ptermApp->m_gswEnable) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_ARROWS), (ptermApp->m_numpadArrows) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_IGNORECAP), (ptermApp->m_ignoreCapLock) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_PLATOKB), (ptermApp->m_platoKb) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_ACCEL), (ptermApp->m_useAccel) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_BEEP), (ptermApp->m_beepEnable) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_SHIFTSPACE), (ptermApp->m_DisableShiftSpace) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_MOUSEDRAG), (ptermApp->m_DisableMouseDrag) ? 1 : 0);
	//tab4
    ptermApp->m_config->Write (wxT (PREF_SCALE), ptermApp->m_scale);
    ptermApp->m_config->Write (wxT (PREF_STATUSBAR), (ptermApp->m_showStatusBar) ? 1 : 0);
#if !defined(__WXMAC__)
    ptermApp->m_config->Write (wxT (PREF_MENUBAR), (ptermApp->m_showMenuBar) ? 1 : 0);
#endif
    ptermApp->m_config->Write (wxT (PREF_NOCOLOR), (ptermApp->m_noColor) ? 1 : 0);
    rgb.Printf (wxT ("%d %d %d"), ptermApp->m_fgColor.Red (), ptermApp->m_fgColor.Green (), ptermApp->m_fgColor.Blue ());
    ptermApp->m_config->Write (wxT (PREF_FOREGROUND), rgb);
    rgb.Printf (wxT ("%d %d %d"), ptermApp->m_bgColor.Red (), ptermApp->m_bgColor.Green (), ptermApp->m_bgColor.Blue ());
    ptermApp->m_config->Write (wxT (PREF_BACKGROUND), rgb);
	//tab5
    ptermApp->m_config->Write (wxT (PREF_CHARDELAY), atoi(ptermApp->m_charDelay.mb_str()));
    ptermApp->m_config->Write (wxT (PREF_LINEDELAY), atoi(ptermApp->m_lineDelay.mb_str()));
    ptermApp->m_config->Write (wxT (PREF_AUTOLF), atoi(ptermApp->m_autoLF.mb_str()));
    ptermApp->m_config->Write (wxT (PREF_SPLITWORDS), (ptermApp->m_splitWords) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_SMARTPASTE), (ptermApp->m_smartPaste) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_CONVDOT7), (ptermApp->m_convDot7) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_CONV8SP), (ptermApp->m_conv8Sp) ? 1 : 0);
    ptermApp->m_config->Write (wxT (PREF_TUTORCOLOR), (ptermApp->m_TutorColor) ? 1 : 0);
	//tab6
    ptermApp->m_config->Write (wxT (PREF_BROWSER), ptermApp->m_Browser);
    ptermApp->m_config->Write (wxT (PREF_EMAIL), ptermApp->m_Email);
    ptermApp->m_config->Write (wxT (PREF_SEARCHURL), ptermApp->m_SearchURL);
    ptermApp->m_config->Flush ();
}

void PtermFrame::OnFullScreen (wxCommandEvent &)
{
    m_fullScreen = !m_fullScreen;
    
    if (m_fullScreen)
    {
        m_canvas->SetScrollRate (0, 0);
    }
    else
    {
        m_canvas->SetScrollRate (1, 1);
    }
    
	ShowFullScreen (m_fullScreen);
    
	m_canvas->Refresh ();
	menuPopup->Check(Pterm_FullScreen,m_fullScreen);
	menuView->Check(Pterm_FullScreenView,m_fullScreen);
}

#if defined (__WXMSW__)
void PtermFrame::OnIconize (wxIconizeEvent &event)
{
	// this helps control to a certain extent, the scrollbars that
	// sometimes appear when restoring from iconized state.
    if (!IsIconized())
	{
		m_canvas->SetScrollRate (0, 0);
		m_canvas->SetScrollRate (1, 1);
	}
}
#endif

void PtermFrame::PrepareDC(wxDC& dc)
{
    dc.SetAxisOrientation (true, false);
    dc.SetBackground (m_backgroundBrush);
#ifdef AUTOSCALE
	dc.SetUserScale(m_scale,m_scale);
#endif
}

void PtermFrame::ptermDrawChar (int x, int y, int snum, int cnum)
{
    int &cx = (vertical) ? y : x;
    int &cy = (vertical) ? x : y;
    int i, j, saveY, savemode, dx, dy, sdy;
    const u16 *charp;
    u16 charw;
    wxClientDC dc(m_canvas);
    
    m_canvas->SaveChar (x, y, snum, cnum, wemode, large!=0);
    
	if (!vertical && !large)
    {
        PrepareDC (dc);
        m_memDC->SetPen (m_foregroundPen);
        m_memDC->SetBackground (m_backgroundBrush);
        drawChar (dc, x, y, snum, cnum);
    }
    else
    {
        // vertical or large drawing we do dot by dot
        x = currentX;
        y = currentY;
        saveY = cy;
        dx = dy = (large) ? 2 : 1;
        sdy = 1;
        if (vertical)
        {
            sdy = -1;
            dy = -dy;
        }
        if (snum == 0)
        {
            charp = plato_m0;
        }
        else if (snum == 1)
        {
            charp = plato_m1;
        }
        else
        {
            charp = plato_m23 + (snum - 2) * (8 * 64);
        }
        charp += 8 * cnum;
        savemode = mode;
        
        for (j = 0; j < 8; j++)
        {
            cy = saveY;
            charw = *charp++;
            for (i = 0; i < 16; i++)
            {
                if ((charw & 1) == 0)
                {
                    // background, do we erase it?
                    if (savemode & 2)
                    {
                        charw >>= 1;
                        cy += dy;
                        continue;
                    }
                    mode = savemode ^ 1;
                }
                else
                {
                    mode = savemode;
                }
                ptermDrawPoint (x, y);
                if (large)
                {
                    ptermDrawPoint (x + 1, y);
                    ptermDrawPoint (x, y + sdy);
                    ptermDrawPoint (x + 1, y + sdy);
                }
                charw >>= 1;
                cy += dy;
            }
            cx += dx;
        }
        mode = savemode;
    }
}

void PtermFrame::ptermDrawPoint (int x, int y)
{
    wxClientDC dc(m_canvas);
    int xm, ym;
    
    PrepareDC (dc);
    xm = XMADJUST (x & 0777);
    ym = YMADJUST (y & 0777);
    x = XADJUST (x & 0777);
    y = YADJUST (y & 0777);
    if (modexor || (mode & 1))
    {
        // mode rewrite or write
        dc.SetPen (m_foregroundPen);
        m_memDC->SetPen (m_foregroundPen);
    }
    else
    {
        // mode inverse or erase
        dc.SetPen (m_backgroundPen);
        m_memDC->SetPen (m_backgroundPen);
    }
	if (modexor)
	{
		dc.SetLogicalFunction(wxXOR);
		m_memDC->SetLogicalFunction(wxXOR);
	}
    dc.DrawPoint (x, y);
    m_memDC->DrawPoint (xm, ym);
#ifndef AUTOSCALE
    if (m_scale == 2)
    {
        dc.DrawPoint (x + 1, y);
        m_memDC->DrawPoint (xm + 1, ym);
        dc.DrawPoint (x, y + 1);
        m_memDC->DrawPoint (xm, ym + 1);
        dc.DrawPoint (x + 1, y + 1);
        m_memDC->DrawPoint (xm + 1, ym + 1);
    }    
#endif
	if (modexor)
	{
		dc.SetLogicalFunction(wxCOPY);
		m_memDC->SetLogicalFunction(wxCOPY);
	}
}

void PtermFrame::ptermDrawLine(int x1, int y1, int x2, int y2)
{
    int xm1, ym1, xm2, ym2, t;
    wxClientDC dc(m_canvas);

    PrepareDC (dc);

    xm1 = XMADJUST (x1);
    ym1 = YMADJUST (y1);
    xm2 = XMADJUST (x2);
    ym2 = YMADJUST (y2);
    x1 = XADJUST (x1);
    y1 = YADJUST (y1);
    x2 = XADJUST (x2);
    y2 = YADJUST (y2);

    // mode rewrite or write
    if (modexor || (mode & 1))
        m_memDC->SetPen (m_foregroundPen);
    // mode inverse or erase
    else
        m_memDC->SetPen (m_backgroundPen);
	if (modexor)
		m_memDC->SetLogicalFunction(wxXOR);

	//draw lines
	ptermDrawBresenhamLine(m_memDC,xm1,ym1,xm2,ym2);

	if (modexor)
		m_memDC->SetLogicalFunction(wxCOPY);

	if (x1>x2)
	{
		t = x1, x1=x2, x2=t;
		t = xm1, xm1=xm2, xm2=t;
	}
	if (y1>y2)
	{
		t = y1, y1=y2, y2=t;
		t = ym1, ym1=ym2, ym2=t;
	}
#ifdef AUTOSCALE
	dc.Blit (x1, y1, x2-x1+1, y2-y1+1, m_memDC, xm1, ym1, wxCOPY);
#else
	dc.Blit (x1, y1, x2-x1+m_scale, y2-y1+m_scale, m_memDC, xm1, ym1, wxCOPY);
#endif

}

void PtermFrame::ptermDrawBresenhamLine(wxMemoryDC *dc,int x1,int y1,int x2,int y2)
{

	int dx,dy;
	int stepx, stepy;

    dx = x2 - x1;
	dy = y2 - y1;
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
#ifndef AUTOSCALE
	stepx *= (m_scale == 2) ? 2 : 1;
	stepy *= (m_scale == 2) ? 2 : 1;
#endif
    dx <<= 1;
    dy <<= 1;
	
	// draw first point
	dc->DrawPoint(x1, y1);
#ifndef AUTOSCALE
	if (m_scale == 2)
	{
		dc->DrawPoint(x1+1, y1);
		dc->DrawPoint(x1, y1+1);
		dc->DrawPoint(x1+1, y1+1);
	}
#endif
	
	//check for shallow line
    if (dx > dy) 
	{
        int fraction = dy - (dx >> 1);
        while (x1 != x2) 
		{
            if (fraction >= 0) 
			{
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;
			dc->DrawPoint(x1, y1);
#ifndef AUTOSCALE
			if (m_scale == 2)
			{
				dc->DrawPoint(x1+1, y1);
				dc->DrawPoint(x1, y1+1);
				dc->DrawPoint(x1+1, y1+1);
			}
#endif
        }
    } 
	//otherwise steep line
	else 
	{
        int fraction = dx - (dy >> 1);
        while (y1 != y2) 
		{
            if (fraction >= 0) 
			{
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
			dc->DrawPoint(x1, y1);
#ifndef AUTOSCALE
			if (m_scale == 2)
			{
				dc->DrawPoint(x1+1, y1);
				dc->DrawPoint(x1, y1+1);
				dc->DrawPoint(x1+1, y1+1);
			}
#endif
        }
    }

}

void PtermFrame::ptermFullErase (void)
{
    wxClientDC dc(m_canvas);

    m_canvas->FullErase ();
    PrepareDC (dc);
    dc.SetPen (m_backgroundPen);
    dc.SetBrush (m_backgroundBrush);
	// erase entire canvas in background color which means the margins to; but is faster
    //dc.Clear ();	
	// erase just 512x512
#ifdef AUTOSCALE
    dc.DrawRectangle (XTOP, YTOP, 512, 512);
#else
    dc.DrawRectangle (XTOP, YTOP, 512 * m_scale, 512 * m_scale);
#endif
    m_memDC->SetBackground (m_backgroundBrush);
    m_memDC->Clear ();
}

void PtermFrame::ptermBlockErase (int x1, int y1, int x2, int y2)
{
    int t;
    int xm1, ym1, xm2, ym2;
    wxClientDC dc(m_canvas);
	
	int ox1,oy1,ox2,oy2;	//original values
	ox1 = x1; 
	oy1 = y1; 
	ox2 = x2; 
	oy2 = y2;
    if (ox1 > ox2)
        t = ox1, ox1 = ox2, ox2 = t;
    if (oy1 > oy2)
        t = oy1, oy1 = oy2, oy2 = t;

    PrepareDC (dc);
    xm1 = XMADJUST (x1);
    ym1 = YMADJUST (y1);
    xm2 = XMADJUST (x2);
    ym2 = YMADJUST (y2);
    x1 = XADJUST (x1);
    y1 = YADJUST (y1);
    x2 = XADJUST (x2);
    y2 = YADJUST (y2);
    if (x1 > x2)
    {
        t = x1;
        x1 = x2;
        x2 = t;
        t = xm1;
        xm1 = xm2;
        xm2 = t;
    }
    if (y1 > y2)
    {
        t = y1;
        y1 = y2;
        y2 = t;
        t = ym1;
        ym1 = ym2;
        ym2 = t;
    }

	if (modexor || (mode & 1))
    {
        // mode rewrite or write
        dc.SetPen (m_foregroundPen);
        m_memDC->SetPen (m_foregroundPen);
        dc.SetBrush (m_foregroundBrush);
        m_memDC->SetBrush (m_foregroundBrush);
    }
    else
    {
        // mode inverse or erase
        dc.SetPen (m_backgroundPen);
        m_memDC->SetPen (m_backgroundPen);
        dc.SetBrush (m_backgroundBrush);
        m_memDC->SetBrush (m_backgroundBrush);
    }

#ifndef AUTOSCALE
	if (m_scale == 2)
	{
		//tweak the half pixel errors
		x1++,xm1++,x2++,xm2++;
		y1++,ym1++,y2++,ym2++;
	}
#endif

	if (modexor)
	{
		dc.SetLogicalFunction(wxXOR);
		m_memDC->SetLogicalFunction(wxXOR);
	}
	dc.DrawRectangle (x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	m_memDC->DrawRectangle (xm1, ym1, xm2 - xm1 + 1, ym2 - ym1 + 1);
	if (modexor)
	{
		dc.SetLogicalFunction(wxCOPY);
		m_memDC->SetLogicalFunction(wxCOPY);
	}
	
	//wipe text map
	int scol = int(ox1/8);
	int cols = int(ceil((ox2-ox1)/8))+1;
	int srow = int(oy1/16);
	int rows = int(ceil((oy2-oy1)/16))+1;
	for ( int row = srow; rows > 0; row++, rows-- )
		memset (&m_canvas->textmap[row * 64 + scol], 055, 2*cols);

}

void PtermFrame::ptermPaint (int pat)
{
    wxClientDC dc(m_canvas);
    int xm, ym;
    
    PrepareDC (dc);
    
    xm = XMADJUST (currentX);
    ym = YMADJUST (currentY);

    m_memDC->SetBrush (m_foregroundBrush);
	if (modexor)
		m_memDC->SetLogicalFunction(wxXOR);
    m_memDC->FloodFill (xm, ym, m_currentBg, wxFLOOD_BORDER);
	if (modexor)
		m_memDC->SetLogicalFunction(wxCOPY);
	//dc.Blit (XTOP, YTOP, vRealScreenSize(m_scale), vRealScreenSize(m_scale), m_memDC, 0, 0, wxCOPY);
	dc.Blit (XTOP, YTOP, vScreenSize(m_scale), vScreenSize(m_scale), m_memDC, 0, 0, wxCOPY);
}

void PtermFrame::ptermSetName (wxString &winName)
{
    wxString str;
    
	if (winName.IsEmpty())
	{
		str = wxT("Pterm");
	}
	else
	{
		str.Printf (wxT("Pterm: %s"), winName.c_str());
	}
    SetTitle (str);
}

void PtermFrame::ptermSetStatus (wxString &str)
{
    if (m_statusBar != NULL)
        m_statusBar->SetStatusText(str, STATUS_CONN);
}

void PtermFrame::ptermLoadChar (int snum, int cnum, const u16 *chardata)
{
    int i, j, m;
    int x, y;
    u16 col;
    const u16 *data;
    
    for (m = 0; m < 5; m++)
    {
        data = chardata;
#ifdef AUTOSCALE
        x = cnum * 8;
        y = (snum * 16 + 15);
#else
        x = cnum * 8 * m_scale;
        y = (snum * 16 + 15) * m_scale;
#endif
        for (i = 0; i < 8; i++)
        {
            col = *data++;
            for (j = 0; j < 16; j++)
            {
                if (col & 1)
                {
                    // drawing char pixel
                    switch (m)
                    {
                    case 0:
                        m_charDC[m]->SetPen (m_backgroundPen);
                        break;
                    case 1:
                        m_charDC[m]->SetPen (m_foregroundPen);
                        break;
                    case 2:
                        m_charDC[m]->SetPen (m_backgroundPen);
                        break;
                    case 3:
                        m_charDC[m]->SetPen (m_foregroundPen);
                        break;
                    case 4:
                        m_charDC[m]->SetPen (*wxBLACK_PEN);
                        break;
                    }
                }
                else
                {
                    // drawing background pixel
                    switch (m)
                    {
                    case 0:
                        m_charDC[m]->SetPen (m_foregroundPen);
                        break;
                    case 1:
                        m_charDC[m]->SetPen (m_backgroundPen);
                        break;
                    case 2:
                        m_charDC[m]->SetPen (*wxBLACK_PEN);
                        break;
                    case 3:
                        m_charDC[m]->SetPen (*wxBLACK_PEN);
                        break;
                    case 4:
                        m_charDC[m]->SetPen (*wxWHITE_PEN);
                        break;
                    }
                }
#ifdef AUTOSCALE
                m_charDC[m]->DrawPoint(x, y - j);
#else
                m_charDC[m]->DrawPoint(x, y - j * m_scale);
                if (m_scale == 2)
                {
                    m_charDC[m]->DrawPoint(x + 1, y - j * m_scale);
                    m_charDC[m]->DrawPoint(x, y - j * m_scale + 1);
                    m_charDC[m]->DrawPoint(x + 1, y - j * m_scale + 1);
                }
#endif
                col >>= 1;
            }
#ifdef AUTOSCALE
            x += 1;
#else
            x += m_scale;
#endif
        }
		m_dirty[m] = true;
    }
}

void PtermFrame::ptermLoadRomChars (void)
{
    unsigned int i;

    for (i = 0; i < 5; i++)
    {
        m_charDC[i]->SetBackground (*wxBLACK_BRUSH);
        m_charDC[i]->Clear ();
		m_dirty[i] = true;
    }

    for (i = 0; i < sizeof (plato_m0) / (sizeof (plato_m0[0]) * 8); i++)
    {
        ptermLoadChar (0, i, plato_m0 + (i * 8));
        ptermLoadChar (1, i, plato_m1 + (i * 8));
    }
}

void PtermFrame::UpdateSettings (wxColour &newfg, wxColour &newbg, bool newscale2)
{
    const bool recolor = (ptermApp->m_fgColor != newfg ||
                          ptermApp->m_bgColor != newbg);
    const int newscale = (newscale2) ? 2 : 1;
    const bool rescale = (newscale != m_scale);
#ifndef AUTOSCALE
    int i;
    wxBitmap *newmap;
#endif

    ptermShowTrace ();
    
    if (!recolor && !rescale)
    {
        return;
    }

    wxClientDC dc(m_canvas);

#ifndef AUTOSCALE
    UpdateDC (m_memDC, m_bitmap, newfg, newbg, newscale2);
#endif
    if (rescale)
    {
		SetClientSize (vXRealSize (newscale) + 2, vYRealSize (newscale) + 2);
		m_canvas->SetSize (vXRealSize (newscale), vYRealSize (newscale));
		m_canvas->SetVirtualSize (vXRealSize (newscale), vYRealSize (newscale));
		if (!m_fullScreen)
		{
			m_canvas->SetScrollRate (0, 0);
			m_canvas->SetScrollRate (1, 1);
		}
        dc.DestroyClippingRegion ();
        dc.SetClippingRegion (GetXMargin (), GetYMargin (), vRealScreenSize (newscale), vRealScreenSize (newscale));
	    m_canvas->SetFocus ();

#ifdef AUTOSCALE
		dc.SetUserScale(m_scale,m_scale);
#endif

#ifndef AUTOSCALE
		UpdateDC (m_charDC[4], m_charmap[4], ptermApp->m_fgColor, ptermApp->m_bgColor, newscale2);
		// Just allocate new bitmaps for the others, we'll repaint them below
		for (i = 0; i < 4; i++)
		{
			newmap = new wxBitmap (vCharXSize (newscale), vCharYSize (newscale), -1);
			m_charDC[i]->SelectObject (*newmap);
			delete m_charmap[i];
			m_charmap[i] = newmap;
			m_dirty[i] = true;
		}
#endif
	}

	SetColors (newfg, newbg, newscale);	// boo hiss! this creates the weird mode inverse problem when switching scales.

	m_canvas->SetBackgroundColour (newbg);
	m_canvas->m_scale = m_scale;
	m_canvas->Refresh ();
	m_defFg = newfg;
	m_defBg = newbg;

}

void PtermFrame::SetColors (wxColour &newfg, wxColour &newbg, int newscale)
{
    TRACE6 ("fg: %d %d %d; bg: %d %d %d", newfg.Red(), newfg.Green(), newfg.Blue(), newbg.Red(), newbg.Green(), newbg.Blue());

	//always set color for all modes
    m_backgroundBrush.SetColour (newbg);
    m_backgroundPen.SetColour (newbg);
    m_foregroundBrush.SetColour (newfg);
    m_foregroundPen.SetColour (newfg);

	for (int i = 0; i<5; i++)
		m_dirty[i] = true;

/*
	//mode erase
	m_charDC[2]->SetBackground (m_backgroundBrush);
	m_charDC[2]->Clear ();
	m_charDC[2]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale), m_charDC[4], 0, 0, wxAND_INVERT);
    
	//mode write
	m_charDC[3]->SetBackground (m_foregroundBrush);
	m_charDC[3]->Clear ();
	m_charDC[3]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale), m_charDC[4], 0, 0, wxAND_INVERT);
    
	//mode inverse
	m_charDC[0]->SetBackground (m_foregroundBrush);
    m_charDC[0]->Clear ();
    m_charDC[0]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale), m_charDC[4], 0, 0, wxAND);
    m_charDC[0]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale), m_charDC[2], 0, 0, wxOR);

	//mode rewrite
    m_charDC[1]->SetBackground (m_backgroundBrush);
    m_charDC[1]->Clear ();
    m_charDC[1]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale), m_charDC[4], 0, 0, wxAND);
    m_charDC[1]->Blit (0, 0, vCharXSize (newscale), vCharYSize (newscale), m_charDC[3], 0, 0, wxOR);
*/

}

void PtermFrame::FixTextCharMaps (void)
{
	
	//exit if not in ascii
	if (m_conn == NULL)
		return;
	if (!m_conn->Ascii ())
		return;

	//mode erase (or mode inverse)
	if ((mode & 0x01) == 0 && m_dirty[2])
	{
		m_charDC[2]->SetBackground (m_backgroundBrush);
		m_charDC[2]->Clear ();
		m_charDC[2]->Blit (0, 0, vCharXSize (m_scale), vCharYSize (m_scale), m_charDC[4], 0, 0, wxAND_INVERT);
		m_dirty[2] = false;
	}
    
	//mode write (or mode rewrite)
	if ((mode & 0x01) != 0 && m_dirty[3])
	{
		m_charDC[3]->SetBackground (m_foregroundBrush);
		m_charDC[3]->Clear ();
		m_charDC[3]->Blit (0, 0, vCharXSize (m_scale), vCharYSize (m_scale), m_charDC[4], 0, 0, wxAND_INVERT);
		m_dirty[3] = false;
	}
    
	//mode inverse
	if ((mode & 0x03) == 0 && m_dirty[0])
	{
		m_charDC[0]->SetBackground (m_foregroundBrush);
		m_charDC[0]->Clear ();
		m_charDC[0]->Blit (0, 0, vCharXSize (m_scale), vCharYSize (m_scale), m_charDC[4], 0, 0, wxAND);
		m_charDC[0]->Blit (0, 0, vCharXSize (m_scale), vCharYSize (m_scale), m_charDC[2], 0, 0, wxOR);
		m_dirty[0] = false;
	}

	//mode rewrite
	if ((mode & 0x03) == 1 && m_dirty[1])
	{
		m_charDC[1]->SetBackground (m_backgroundBrush);
		m_charDC[1]->Clear ();
		m_charDC[1]->Blit (0, 0, vCharXSize (m_scale), vCharYSize (m_scale), m_charDC[4], 0, 0, wxAND);
		m_charDC[1]->Blit (0, 0, vCharXSize (m_scale), vCharYSize (m_scale), m_charDC[3], 0, 0, wxOR);
		m_dirty[1] = false;
	}

}

void PtermFrame::UpdateDC (wxMemoryDC *dc, wxBitmap *&bitmap, wxColour &newfg, wxColour &newbg, bool newscale2)
{
    int fr, fg, fb, br, bg, bb, ofr, ofg, ofb;
    int r, g, b, oh, ow, nh, nw, h, w;
    u8 *odata, *ndata, *newbits = NULL;
    const bool recolor = (ptermApp->m_fgColor != newfg ||
                          ptermApp->m_bgColor != newbg);
    const int newscale = (newscale2) ? 2 : 1;
    const bool rescale = (newscale != m_scale);
    wxMemoryDC tmpDC;
	wxBitmap *newbitmap;
    wxImage *imgp;

    if (!recolor && !rescale)
        return;

	//create new bitmap
    imgp = new wxImage (bitmap->ConvertToImage ());
    
	//get parameters to reprocess
    ow = imgp->GetWidth ();
    oh = imgp->GetHeight ();
#ifdef AUTOSCALE
    w = ow;
    h = oh;
    nw = w;
    nh = h;
#else
    w = ow / m_scale;
    h = oh / m_scale;
    nw = w * newscale;
    nh = h * newscale;
#endif
    
    ofr = ptermApp->m_fgColor.Red ();
    ofg = ptermApp->m_fgColor.Green ();
    ofb = ptermApp->m_fgColor.Blue ();
    fr = newfg.Red ();
    fg = newfg.Green ();
    fb = newfg.Blue ();
    br = newbg.Red ();
    bg = newbg.Green ();
    bb = newbg.Blue ();
    
	//reprocess bitmap
    ndata = odata = imgp->GetData ();
#ifndef AUTOSCALE
    if (rescale)
        ndata = newbits = (u8 *) malloc (3 * nw * nh);
#endif
    
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            r = odata[0];
            g = odata[1];
            b = odata[2];
            if (recolor)
            {
                if ((r == ofr) && (g == ofg) && (b == ofb))
                {
                    r = fr;
                    g = fg;
                    b = fb;
                }
                else
                {
                    r = br;
                    g = bg;
                    b = bb;
                }
            }
            if (rescale)
            {
#ifndef AUTOSCALE
                if (newscale2)
                {
                    // from scale 1 to scale 2
                    ndata[0] = ndata[3] = r;
                    ndata[1] = ndata[4] = g;
                    ndata[2] = ndata[5] = b;
                    odata += 3;
                    ndata += 6;
                }
                else
                {
                    ndata[0] = r;
                    ndata[1] = g;
                    ndata[2] = b;
                    odata += 6;
                    ndata += 3;
                }
#endif
            }
			else
            {
                    odata[0] = r;
                    odata[1] = g;
                    odata[2] = b;
                    odata += 3;
            }
        }
#ifndef AUTOSCALE
        if (rescale)
        {
            if (newscale2)
            {
                // from scale 1 to scale 2 -- duplicate scan line just completed
                memcpy (ndata, ndata - (nw * 3), nw * 3);
                ndata += nw * 3;
            }
            else
            {
                // from scale 2 to scale 1 -- skip a scanline
                odata += ow * 3;
            }
        }
#endif
    }
#ifndef AUTOSCALE
    if (rescale)
    {
        delete imgp;
        imgp = new wxImage (nw, nh, newbits);
    }
#endif
    
    newbitmap = new wxBitmap (*imgp);
    dc->SelectObject (*newbitmap);
	delete bitmap;
    delete imgp;
    bitmap = newbitmap;

}

void PtermFrame::drawChar (wxDC &dc, int x, int y, int snum, int cnum)
{
    int charX, charY, sizeX, sizeY, screenX, screenY, memX, memY;

#ifdef AUTOSCALE
    charX = cnum * 8;
    charY = snum * 16;
#else
    charX = cnum * 8 * m_scale;
    charY = snum * 16 * m_scale;
#endif
    sizeX = 8;
    sizeY = 16;

    screenX = XADJUST (x);
    screenY = YADJUST (y + 15);
    memX = XMADJUST (x);
    memY = YMADJUST (y + 15);

	FixTextCharMaps();
    
    if (x < 0)
    {
        sizeX += x;
#ifdef AUTOSCALE
        charX -= x;
#else
        charX -= x * m_scale;
#endif
        screenX = XADJUST (0);
        memX = XMADJUST (0);
    }
    if (y < 0)
    {
        sizeY += y;
    }
	if (modexor)
	{
        // xor -- just xor/blit in the mode rewrite pattern
#ifdef AUTOSCALE
		m_memDC->Blit (memX, memY, sizeX, sizeY, m_charDC[3], charX, charY, wxXOR);
#else
		m_memDC->Blit (memX, memY, sizeX * m_scale, sizeY * m_scale, m_charDC[3], charX, charY, wxXOR);
#endif
	}
	else if (mode & 2)
    {
        // write or erase -- need to zap old pixels and OR in new pixels
#ifdef AUTOSCALE
	    m_memDC->Blit (memX, memY, sizeX, sizeY, m_charDC[4], charX, charY, wxAND);
		m_memDC->Blit (memX, memY, sizeX, sizeY, m_charDC[wemode], charX, charY, wxOR);
#else
	    m_memDC->Blit (memX, memY, sizeX * m_scale, sizeY * m_scale, m_charDC[4], charX, charY, wxAND);
		m_memDC->Blit (memX, memY, sizeX * m_scale, sizeY * m_scale, m_charDC[wemode], charX, charY, wxOR);
#endif
    }
    else
    {
        // inverse or rewrite, just blit in the appropriate pattern
#ifdef AUTOSCALE
        m_memDC->Blit (memX, memY, sizeX, sizeY, m_charDC[wemode], charX, charY, wxCOPY);
#else
        m_memDC->Blit (memX, memY, sizeX * m_scale, sizeY * m_scale, m_charDC[wemode], charX, charY, wxCOPY);
#endif
    }
        
    // Now copy the resulting state of the character area into the screen dc
#ifdef AUTOSCALE
    dc.Blit (screenX, screenY, sizeX, sizeY, m_memDC, memX, memY, wxCOPY);
#else
    dc.Blit (screenX, screenY, sizeX * m_scale, sizeY * m_scale, m_memDC, memX, memY, wxCOPY);
#endif
    
    // Handle screen edge wraparound by recursion...
    if (x > 512 - 8)
    {
        drawChar (dc, x - 512, y, snum, cnum);
    }
    if (y > 512 - 16)
    {
        drawChar (dc, x, y - 512, snum, cnum);
    }
}

void PtermFrame::drawFontChar (int x, int y, int c)
{
    int screenX, screenY, memX, memY;
    wxClientDC dc(m_canvas);
	wxString chr;

	chr.Printf(wxT("%c"),c);

	switch (wemode)
	{
	case 0:			// inverse
		m_memDC->SetBackgroundMode(wxSOLID);
		m_memDC->SetTextBackground(m_currentFg);
		m_memDC->SetTextForeground(m_currentBg);
		break;
	case 1:			// rewrite
		m_memDC->SetBackgroundMode(wxSOLID);
		m_memDC->SetTextBackground(m_currentBg);
		m_memDC->SetTextForeground(m_currentFg);
		break;
	case 2:			// erase
		m_memDC->SetBackgroundMode(wxTRANSPARENT);
		m_memDC->SetTextForeground(m_currentBg);
		break;
	case 3:			// write
		m_memDC->SetBackgroundMode(wxTRANSPARENT);
		m_memDC->SetTextForeground(m_currentFg);
		break;
	}
	
	m_memDC->GetTextExtent(chr, &m_fontwidth, &m_fontheight);

    screenX = XADJUST (x);
    screenY = YADJUST (y + m_fontheight - 1);
    memX = XMADJUST (x);
    memY = YMADJUST (y + m_fontheight - 1);

	currentX += m_fontwidth;
    
	if (modexor)
		m_memDC->SetLogicalFunction(wxXOR);
	m_memDC->DrawText(chr,memX,memY);
	if (modexor)
		m_memDC->SetLogicalFunction(wxCOPY);
	dc.Blit (screenX, screenY, m_fontwidth, m_fontheight, m_memDC, memX, memY);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process word of PLATO output data
**
**  Parameters:     Name        Description.
**                  d           19-bit word
**                  ascii       true if using ASCII protocol
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::procPlatoWord (u32 d, bool ascii)
{
    mptr mp;
    const char *msg = "";
    int i, j, n = 0;
    AscState    ascState;

    // used in load coordinate
    int &coord = (d & 01000) ? currentY : currentX;
    int &cx = (vertical) ? currentY : currentX;
    int &cy = (vertical) ? currentX : currentY;
    
    int deltax, deltay, supdelta;

	bool settitleflag = false;
    
	if (m_usefont && currentCharset <= 1)
	{
		supdelta = (m_fontheight/3);
		// Not going to support reverse/vertical in font mode until I get documentation
		// on what worked with -font- in the past.  JWS 5/27/2007
		deltax = 8;
		deltay = m_fontheight;
	}
	else
	{
		deltax = (reverse) ? -8 : 8;
		deltay = (vertical) ? -16 : 16;
		if (large)
		{
			deltax *= 2;
			deltay *= 2;
		}
		supdelta = (deltay / 16) * 5;
	}
    
    seq++;
    if (tracePterm)
    {
        fprintf (traceF, "%07o ", d);
    }
    if (ascii)
    {
#ifdef DEBUGASCII
        if (!m_dumbTty)
        {
            int c = ((d & 127) >= 32 && (d & 127) < 127) ? d & 127 : '.';
            printf ("%04x  %c\n", d, c);
        }
#endif
        if (m_dumbTty)
        {
            if (d == (033 << 8) + 002)   // ESC STX
            {
                TRACEN ("Entering PLATO terminal mode");
                m_dumbTty = false;
                mode = (3 << 2) + 1;    // set character mode, rewrite
                ptermSetStation (-1, true, ptermApp->m_showStatusBar);   // Show connected in ASCII mode
            }
            else if ((d >> 8) == 0)
            {
                if (d >= 32 && d < 127)
                {
                    d = asciiM0[d];
                    if ((d & 0xf0) != 0xf0)
                    {
                        // Force mode rewrite
                        mode = (3 << 2) + 1;
                        ptermDrawChar (currentX, currentY, (d & 0x80) >> 7, d & 0x7f);
                        currentX = (currentX + 8) & 0777;
                    }
                }
                else if (d == 015)
                {
                    currentX = 0;
                }
                else if (d == 012)
                {
                    wxClientDC dc(m_canvas);
                    PrepareDC (dc);
                    m_memDC->SetBackground (m_backgroundBrush);
                    if (currentY != 0)
                    {
                        currentY -= 16;
                    }
                    else
                    {
                        // On the bottom line... scroll.
#ifdef AUTOSCALE
                        dc.Blit (XTOP, YTOP, vScreenSize(m_scale), vScreenSize(m_scale) - 16, m_memDC, XMTOP, YMADJUST (496), wxCOPY);
                        m_memDC->Blit (XMTOP, YMTOP, vScreenSize(m_scale), vScreenSize(m_scale) - 16, &dc, XTOP, YTOP, wxCOPY);
#else
                        dc.Blit (XTOP, YTOP, vScreenSize(m_scale), vScreenSize(m_scale) - (16 * m_scale), m_memDC, XMTOP, YMADJUST (496), wxCOPY);
                        m_memDC->Blit (XMTOP, YMTOP, vScreenSize(m_scale), vScreenSize(m_scale) - (16 * m_scale), &dc, XTOP, YTOP, wxCOPY);
#endif
                    }
                    // Erase the line we just moved to.
#ifdef AUTOSCALE
                    m_memDC->Blit (XMTOP, YMADJUST (16) + 1, vScreenSize(m_scale), 16, m_memDC, 0, 0, wxCLEAR);
                    dc.Blit (XTOP, YADJUST (16) + 1, vScreenSize(m_scale), 16, &dc, 0, 0, wxCLEAR);
#else
                    m_memDC->Blit (XMTOP, YMADJUST (16) + 1, vScreenSize(m_scale), 16 * m_scale, m_memDC, 0, 0, wxCLEAR);
                    dc.Blit (XTOP, YADJUST (16) + 1, vScreenSize(m_scale), 16 * m_scale, &dc, 0, 0, wxCLEAR);
#endif
                }
            }
        }
        else if (m_ascState == pni_rs)
        {
            // We just want to ignore 3 command codes.  Note that escape
            // sequences count for one, not two.
            if (++m_ascBytes == 3)
            {
                m_ascBytes = 0;
                m_ascState = none;
            }
        }
        else if (m_ascState == pmd)
        {
            n = AssembleAsciiPlatoMetaData (d);
            if (n == 0)
            {
				if (m_fontPMD)
				{
					TRACEN ("plato meta data complete: font data accepted");
					TRACE3 ("Font selected: %s,%d,%d", m_fontface.c_str(), m_fontsize, m_fontbold|m_fontitalic|m_fontstrike|m_fontunderln);
					m_fontPMD = false;
				}
				else if (m_fontinfo)
				{
					TRACEN ("plato meta data complete: get font data accepted and sent");
					m_fontinfo = false;
				}
				else if (m_osinfo)
				{
					TRACEN ("plato meta data complete: get operating system info accepted and sent");
					m_osinfo = false;
				}
				else
				{
					TRACE ("plato meta data complete: %s", m_PMD.c_str());
					ProcessPlatoMetaData();
				}
				m_PMD = wxT("");
            }
        }
        else if ((d >> 8) == 033)
        {
            // Escape sequence, the second character is in the low byte
            d &= 0377;
            switch (d)
            {
            case 002:   // ESC STX
                TRACEN ("Still in PLATO terminal mode");
                m_dumbTty = false;
                break;
            case 003:   // ESC ETX
                TRACEN ("Leaving PLATO terminal mode");
                m_dumbTty = true;
                m_flowCtrl = false;
                m_sendFgt = false;
                currentX = 0;
                currentY = 496;
                break;
            case 014:   // ESC FF
                TRACEN ("Full screen erase");
                ptermFullErase ();
                break;
            case 026:
				// mode xor (also sets mode write for off-screen DC operations)
                TRACEN ("load mode xor");
				modexor = true;
                mode = (mode & ~3) + 2;
				break;
            case 021:   // ESC DC1
            case 022:   // ESC DC2
            case 023:   // ESC DC3
            case 024:   // ESC DC4
                // modes inverse, write, erase, rewrite
				modexor = false;
                mode = (mode & ~3) + ascmode[d - 021];
                //mode = 017;
                TRACE ("load mode %d", mode);
                break;
            case '2':
                // Load coordinate
                TRACEN ("Start load coordinate");
                m_ascState = ldc;
                m_ascBytes = 0;
                break;
            case '@':
                // superscript
                TRACEN ("Superscript");
                cy = (cy + supdelta) & 0777;
                break;
            case 'A':
                // subscript
                TRACEN ("Subscript");
                cy = (cy - supdelta) & 0777;
                break;
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
                TRACE ("select memory M%d", d - 'B');
                setCmem (d - 'B');
                break;
            case 'J':
                TRACEN ("Horizontal writing mode");
                setVertical (false);
                break;
            case 'K':
                TRACEN ("Vertical writing mode");
                setVertical (true);
                break;
            case 'L':
                TRACEN ("Forward writing mode");
                setReverse (false);
                break;
            case 'M':
                TRACEN ("Reverse writing mode");
                setReverse (true);
                break;
            case 'N':
                TRACEN ("Normal size writing mode");
                setLarge (false);
                break;
            case 'O':
                TRACEN ("Double size writing mode");
                setLarge (true);
                break;
            case 'P':
				modexor = false;
                mode = (mode & 3) + (2 << 2);
                TRACE ("load mode %d", mode);
                break;
            case 'Q':
                m_ascState = ssf;
                m_ascBytes = 0;
                TRACEN ("Start SSF");
                break;
            case 'R':
                // external data
                m_ascState = ext;
                m_ascBytes = 0;
                TRACEN ("Start ext");
                break;
            case 'S':
				modexor = false;
                mode = (mode & 3) + (2 << 2);
                TRACE ("load mode %d", mode);
                break;
            case 'T':
				modexor = false;
                mode = (mode & 3) + (5 << 2);
                TRACE ("load mode %d", mode);
                break;
            case 'U':
				modexor = false;
                mode = (mode & 3) + (6 << 2);
                TRACE ("load mode %d", mode);
                break;
            case 'V':
				modexor = false;
                mode = (mode & 3) + (7 << 2);
                TRACE ("load mode %d", mode);
                break;
            case 'W':
                // Load memory address
                TRACEN ("Start LDA");
                m_ascState = lda;
                m_ascBytes = 0;
                break;
			case 'X':
				TRACEN ("Start load plato meta data");
				m_ascState = pmd;
				m_ascBytes = 0;
				break;
            case 'Y':
                // load echo
                TRACEN ("Start LDE");
                m_ascState = lde;
                m_ascBytes = 0;
                break;
            case 'Z':
                // set margin
				TRACE ("set margin %d", currentX);
                setMargin (cx);
                break;
            case 'a':
                // set foreground color
                TRACEN ("Start foreground color");
                m_ascState = fg;
                m_ascBytes = 0;
                break;
            case 'b':
                // set background color
                TRACEN ("Start background color");
                m_ascState = bg;
                m_ascBytes = 0;
                break;
            case 'c':
                // paint
                TRACEN ("Start paint");
                m_ascState = paint;
                m_ascBytes = 0;
                break;
            case 'g':
                // set gray-scale foreground color
                TRACEN ("Start grayscale foreground color");
                m_ascState = gsfg;
                m_ascBytes = 0;
                break;
			default:
                TRACE ("Other unknown ESCAPE sequence: %d", d);
                break;
            }
        }
        else
        {
            switch (d)
            {
            case 010:   // backspace
                cx = (cx - deltax) & 0777;
                TRACEN ("backspace");
                break;
            case 011:   // tab
                TRACEN ("tab");
                cx = (cx + deltax) & 0777;
                break;
            case 012:   // linefeed
                TRACEN ("linefeed");
                cy = (cy - deltay) & 0777;
                break;
            case 013:   // vertical tab
                TRACEN ("vertical tab");
                cy = (cy - deltay) & 0777;
                break;
            case 014:   // form feed
                TRACEN ("form feed");
                if (vertical)
                {
                    cx = deltay - 1;
                    cy = reverse ? 512 - deltax : 0;
                }
                else
                {
                    cy = 512 - deltay;
                    cx = reverse ? 512 - deltax : 0;
                }
                break;
            case 015:   // carriage return
                cx = margin;
                cy = (cy - deltay) & 0777;
				TRACE2 ("CR to %d %d", currentX, currentY);
                break;
            case 031:   // EM
                mode = (mode & 3) + (4 << 2);
                modewords = 0;              // words since entering mode
                TRACE ("load mode %d", mode);
                break;
            case 034:   // FS
                mode = (mode & 3) + (0 << 2);
                TRACE ("load mode %d", mode);
                break;
            case 035:   // FS
                mode = (mode & 3) + (1 << 2);
				m_ascState = ldc;// to have first coordinate be "dark"
                TRACE ("load mode %d", mode);
                break;
            case 036:   // RS -- used by PNI in connect handshake
                m_ascState = pni_rs;
                TRACEN ("pni start download, ignoring next 3 commands");
                break;
            case 037:   // FS
                mode = (mode & 3) + (3 << 2);
                TRACE ("load mode %d", mode);
                break;
            }
            if (d >= 040)
            {
                switch (m_ascState)
                {
                case ldc:
                    if (AssembleCoord (d))
                    {
                        currentX = lastX;
                        currentY = lastY;
                        TRACE2 ("load coordinate %d %d", currentX, currentY);
                    }
                    break;
                case paint:
                    n = AssemblePaint (d);
                    if (n != -1)
                    {
                        TRACE ("paint %03o", n);
                        ptermPaint (n);
                    }
                    break;
                case lde:
                    n = AssembleData (d);
                    if (n != -1)
                    {
                        n &= 0177;
                        switch (n)
                        {
                        case 0160:
                            // 160 is terminal type query
                            TRACE ("load echo termtype %d", ASCTYPE);
                            n = 0160 + ASCTYPE;
                            break;
                        case 0x71:
                            TRACE ("load echo subtype %d", SUBTYPE);
                            n = SUBTYPE;
                            break;
                        case 0x72:
                            TRACEN ("load echo loadfile (unused)");
                            n = 0;
                            break;
                        case 0x73:
                            // hex 73 is report terminal config
                            TRACEN ("load echo termdata");
                            n = TERMCONFIG;
                            break;
                        case 0x7b:
                            // hex 7b is beep
                            if (ptermApp->m_beepEnable)
                            {
                                TRACEN ("beep");
                                wxBell ();
								if (!IsActive())
									RequestUserAttention(wxUSER_ATTENTION_INFO);
                            }
                            break;
                        case 0x7d:
                            // hex 7d is report MAR
                            TRACE ("report MAR %o", memaddr);
                            n = memaddr;
                            break;
                        case 0x52:
                            // hex 52 is enable flow control
                            TRACEN ("enable flow control");
                            m_flowCtrl = true;
                            n = 0x53;
                            break;
                        case 0x60:
                            // hex 60 is inquire features
                            TRACE ("report features 0x%02x", ASCFEATURES);
                            n += ASCFEATURES;
                            m_sendFgt = true;
                            break;
                        default:
                            TRACE2 ("load echo %d (0x%02x)", n, n);
                        }
                        if (n == 0x7b)
                            break;          // -beep- does NOT send an echo code in reply
                        n += 0200;
                        if (m_conn->RingCount () > RINGXOFF1)
                            m_pendingEcho = n;
                        else
                        {
                            ptermSendKey (n);
                            m_pendingEcho = -1;
                        }
                    }
                    break;
                case lda:
                    n = AssembleData (d);
                    if (n != -1)
                    {
                        TRACE ("load memory address %04x", n);
                        memaddr = n & 077777;
                    }
                    break;
                case ext:
                    TRACE ("ext %04x", d);
                    n = AssembleData (d);
					switch (n)
					{
					case -1:
						break;
					// check for special TERM area save/restore
					case CWS_TERMSAVE:
	                    TRACE ("ext completed %04x", n);
						cwswindow[0].data[0] = 0;
						cwswindow[0].data[1] = 48;
						cwswindow[0].data[2] = 511;
						cwswindow[0].data[3] = 0;
						ptermSaveWindow(0);
						break;
					case CWS_TERMRESTORE:
	                    TRACE ("ext completed %04x", n);
						ptermRestoreWindow(0);
						break;
					default:
	                    TRACE ("ext completed %04x", n);
						// check if in cws mode
						switch (cwsmode)
						{
							// not in cws; check for font data
							case 0:
								cwscnt = 0;
								switch (n & 07700)
								{
								case 05000:		// font face name and family
									SetFontFaceAndFamily(n & 077);
									break;
								case 05100:		// font size
									SetFontSize(n & 077);
									break;
								case 05200:		// font flags
									SetFontFlags(n & 077);
									SetFontActive();
									break;
								}
								break;
							// check for cws data mode
							case 1:
								cwscnt++;
								if      (cwscnt==1 && n==CWS_SAVE)			
								{
									TRACEN ("CWS: specify save function");
									cwsfun = CWS_SAVE;
								}
								else if (cwscnt==1 && n==CWS_RESTORE)	
								{
									TRACEN ("CWS: specify restore function");
									cwsfun = CWS_RESTORE;
								}
								else if (cwscnt==1 || cwscnt>6)
								{
									TRACE ("CWS: invalid function; %d", n);
									cwsmode=0, cwsfun=0, cwscnt=0;	// unknown function; terminate cws mode
								}
								else if (cwscnt==2)
								{
									TRACE ("CWS: specify window; %d", n);
									cwswin = n;
								}
								else if (cwscnt<7)
								{
									TRACE ("CWS: data; %d", n);
									cwswindow[cwswin].data[cwscnt-3] = n;
								}
								break;
							// check for cws execute mode
							case 2:
								cwscnt = 0;
								if (n==CWS_EXEC)
								{
									TRACEN ("CWS: process exec");
									switch (cwsfun)
									{
									case CWS_SAVE:
										ptermSaveWindow(cwswin);
										break;
									case CWS_RESTORE:
										ptermRestoreWindow(cwswin);
										break;
									}
								}
								else
								{
									cwsmode=0, cwsfun=0, cwscnt=0;	// unknown function; terminate cws mode
								}
								break;
							}
                    }
                    break;
                case ssf:
                    n = AssembleData (d);
                    if (n != -1)
                    {
                        TRACE ("ssf %04x", n);
                        m_canvas->ptermTouchPanel ((n & 0x20) != 0);
                    }
					switch (n)
					{
					case 0x1f00:	// xin 7; means start CWS functions
                        TRACE ("ssf; start cws mode; %04x", n);
						cwsmode = 1;
						break;
					case 0x1d00:	// xout 7; means stop CWS functions
                        TRACE ("ssf; stop cws mode; %04x", n);
						cwsmode = 2;
						break;
					case -1:
						break;
					default:
                        TRACE ("ssf %04x", n);
                        m_canvas->ptermTouchPanel ((n & 0x20) != 0);
						break;
					}
                    break;
                case fg:
                case bg:
                    ascState = m_ascState;
                    n = AssembleColor (d);
                    if (n != -1 && !ptermApp->m_noColor)
                    {
                        wxColour c ((n >> 16) & 0xff, (n >> 8) & 0xff, n & 0xff);
                        if (ascState == fg)
                        {
                            //TRACE ("set foreground color %06x", n);
                            m_currentFg = c;
                        }
                        else
                        {
                            //TRACE ("set background color %06x", n);
                            m_currentBg = c;
                        }
                        SetColors (m_currentFg, m_currentBg, m_scale);
                    }
                    break;
				case gsfg:
                    ascState = m_ascState;
                    n = AssembleGrayScale (d);
                    if (n != -1 && !ptermApp->m_noColor)
                    {
                        wxColour c (n & 0xff, n & 0xff, n & 0xff);
                        if (ascState == gsfg)
                        {
                            //TRACE ("set gray-scale foreground color %06x", n);
                            m_currentFg = c;
                        }
                        SetColors (m_currentFg, m_currentBg, m_scale);
                    }
                    break;
                case pmd:
					break;// handled above
                case none:
                    switch (mode >> 2)
                    {
                    case 0:
                        if (AssembleCoord (d))
                        {
                            mode0 ((lastX << 9) + lastY);
                        }
                        break;
                    case 1:
                        if (AssembleCoord (d))
                        {
                            mode1 ((lastX << 9) + lastY);
                        }
                        break;
                    case 2:
                        n = AssembleData (d);
                        if (n != -1)
                        {
                            mode2 (n);
                        }
                        break;
                    case 3: // text mode
                        TRACE2 ("char %03o (%c)", d, d);
                        m_ascState = none;
                        m_ascBytes = 0;
						i = currentCharset;
						if (m_usefont && i == 0)
							drawFontChar(currentX, currentY, d);
						else
						{
							if (i == 0)
							{
								d = asciiM0[d];
								// The ROM vs. RAM choice is given by the
								// current character set.  
								// For the ROM characters, the even vs. odd
								// (M0 vs. M1) choice is given by the top bit
								// of the ASCII translation table.
								i = (d & 0x80) >> 7;
							}
							else if (i == 1)
							{
								d = asciiM1[d];
								i = (d & 0x80) >> 7;
							}
							else
							{
								// RAM characters are indexed by printable
								// ASCII characters; the RAM character offset
								// is simply the character code - 32.
								// The set choice is simply what the host sent.
								d = (d - 040) & 077;
							}
							if ((d & 0xf0) == 0xf0)
							{
								if (d != 0xff)
								{
									// A builtin composite
									int savemode = mode;
									int sy = cy;
                                
									d -= 0xf0;
									for (i = 0; i < 8; i += 2)
									{
										n = M1specials[d][i];
										if (n == 0)
										{
											break;
										}
										j = (i8) M1specials[d][i + 1];
										cy += j;
										ptermDrawChar (currentX, currentY, n >> 7, n & 0x7f);
										cy = sy;
										mode |= 2;
									}
									mode = savemode;
									cx = (cx + deltax) & 0777;
								}
							}   
							else 
							{
								ptermDrawChar (currentX, currentY, i, d & 0x7f);
								cx = (cx + deltax) & 0777;
							}
						}
                        break;
                    case 4:
                        if (AssembleCoord (d))
                        {
                            modewords++;
                            mode4 ((lastX << 9) + lastY);
                        }
                        break;
                    case 5:
                        n = AssembleData (d);
                        if (n != -1)
                            mode5 (n);
                        break;
                    case 6:
                        n = AssembleData (d);
                        if (n != -1)
                            mode6 (n);
                        break;
                    case 7:
                        n = AssembleData (d);
                        if (n != -1)
                            mode7 (n);
                        break;
                    }
                    break;
                }
            }
        }
    }
    else
    {
        if ((d & NOP_MASK) == 0)
        {
            // NOP command...
            if (d & 1)
            {
                wc = (wc + 1) & 0177;
            }
        }
        else
        {
            wc = (wc + 1) & 0177;
        }
    
        if (d & 01000000)
        {
            modewords++;
            mp = modePtr[mode >> 2];
            (this->*mp) (d);
        }
        else
        {
            switch ((d >> 15) & 7)
            {
            case 0:     // nop
				settitleflag = false;
                // special code to tell pterm the station number
                if ((d & NOP_MASKDATA) == NOP_SETSTAT)
                {
                    d &= 0777;
                    ptermSetStation (d, true, ptermApp->m_showStatusBar);
                }
                // special code to get font type / size / flags
                else if ((d & NOP_MASKDATA) == NOP_FONTTYPE)
                    SetFontFaceAndFamily(d & 077);
                else if ((d & NOP_MASKDATA) == NOP_FONTSIZE)
                    SetFontSize(d & 077);
                else if ((d & NOP_MASKDATA) == NOP_FONTFLAG)
                {
                    SetFontFlags(d & 077);
					SetFontActive();
                }
                else if ((d & NOP_MASKDATA) == NOP_FONTINFO)
                {
                    const int chardelay = atoi(ptermApp->m_charDelay.mb_str());
					ptermSendExt((int)m_fontwidth);
					wxMilliSleep(chardelay);
					ptermSendExt((int)m_fontheight);
					wxMilliSleep(chardelay);
                }
                else if ((d & NOP_MASKDATA) == NOP_OSINFO)
                {
                    const int chardelay = atoi(ptermApp->m_charDelay.mb_str());
					// sends 3 external keys, OS, major version, minor version
					int os,major,minor;
					os = wxGetOsVersion(&major,&minor);
					ptermSendExt(os);
					wxMilliSleep(chardelay);
					if (os==wxMAC || os==wxMAC_DARWIN)
						ptermSendExt(10*(major>>4) + (major &0x0f));
					else
						ptermSendExt(major);
					wxMilliSleep(chardelay);
					if (os==wxMAC || os==wxMAC_DARWIN)
						ptermSendExt(10*(minor>>4) + (minor &0x0f));
					else
						ptermSendExt(minor);
					wxMilliSleep(chardelay);
                }
	            // otherwise check for plato meta data codes
                else
				{
					if ((d & NOP_MASKDATA) == NOP_PMDSTART && !m_loadingPMD)
					{
						m_loadingPMD = true;
						m_PMD = wxT("");
						settitleflag = AssembleClassicPlatoMetaData(d & 077);
					}
					else if ((d & NOP_MASKDATA) == NOP_PMDSTREAM && m_loadingPMD) 
					{
						settitleflag = AssembleClassicPlatoMetaData(d & 077);
					}
					else if ((d & NOP_MASKDATA) == NOP_PMDSTOP && m_loadingPMD)
					{
						m_loadingPMD = false;
						AssembleClassicPlatoMetaData(d & 077);
						settitleflag = true;
					}
					if (settitleflag)
						ProcessPlatoMetaData();
				}
                TRACEN ("nop");
                break;

            case 1:     // load mode
                modewords = 0;              // words since entering mode
                if ((d & 020000) != 0)
                {
                    // load wc bit is set
                    wc = (d >> 6) & 0177;
                }
				modexor = false;
                mode = (d >> 1) & 037;
                if (d & 1)
                {
                    // full screen erase
                    ptermFullErase ();
                }
                TRACE2 ("load mode %d screen %d", mode, (d & 1));
                break;
            
            case 2:     // load coordinate
            
                if (d & 04000)
                {
                    // Add or subtract from current coordinate
                    if (d & 02000)
                    {
                        coord -= d & 0777;
                    }
                    else
                    {
                        coord += d & 0777;
                    }
                }
                else
                {
                    coord = d & 0777;
                }
        
                if (d & 010000)
                {
                    setMargin (coord);
                    msg = "margin";
                }
                TRACE3 ("load coord %c %d %s", (d & 01000) ? 'Y' : 'X', d & 0777, msg);
                break;
            case 3:     // echo
                d &= 0177;
                switch (d)
                {
                case 0160:
                    // 160 is terminal type query
                    TRACE ("load echo termtype %d", TERMTYPE);
                    d = 0160 + TERMTYPE;
                    break;
                case 0x7b:
					// hex 7b is beep
					if (ptermApp->m_beepEnable)
					{
						TRACEN ("beep");
						wxBell ();
						if (!IsActive())
							RequestUserAttention(wxUSER_ATTENTION_INFO);
					}
                    break;
                case 0x7d:
                    // hex 7d is report MAR
                    TRACE ("report MAR %o", memaddr);
                    d = memaddr;
                    break;
                default:
                    TRACE ("load echo %d", d);
                }
                if (d == 0x7b)
                {
                    break;          // -beep- does NOT send an echo code in reply
                }
                d += 0200;
                if (m_conn->RingCount () > RINGXOFF1)
                {
                    m_pendingEcho = d;
                }
                else
                {
                    ptermSendKey (d);
                    m_pendingEcho = -1;
                }
                break;
            
            case 4:     // load address
                memaddr = d & 077777;
                TRACE2 ("load address %o (0x%x)", memaddr, memaddr);
                break;
            
            case 5:     // SSF on PPT
                switch ((d >> 10) & 037)
                {
                case 1: // Touch panel control ?
                    TRACE ("ssf touch %o", d);
                    m_canvas->ptermTouchPanel ((d & 040) != 0);
                    break;
                default:
                    TRACE ("ssf %o", d);
                    break;  // ignore
                }
                break;

            case 6:
            case 7:
                d &= 0177777;
                TRACE ("Ext %07o", d);
                // Take no other action here -- it's been done already
                // when the word was fetched
                break;

            default:    // ignore
                TRACE ("ignored command word %07o", d);
                break;
            }
        }
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a 9 bit data word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the word
**
**------------------------------------------------------------------------*/
int PtermFrame::AssemblePaint (int d)
{
    if (m_ascBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_ascBytes * 6));
    if (++m_ascBytes == 2)
    {
        m_ascBytes = 0;
        m_ascState = none;
        TRACE2 ("paint %03o (0x%04x)", m_assembler, m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("paint byte %d: %d", m_ascBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble an 18 bit data word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the word
**
**------------------------------------------------------------------------*/
int PtermFrame::AssembleData (int d)
{
    if (m_ascBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_ascBytes * 6));
    if (++m_ascBytes == 3)
    {
        m_ascBytes = 0;
        m_ascState = none;
        TRACE2 ("data %07o (0x%04x)", m_assembler, m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("data byte %d: %d", m_ascBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a 24 bit color word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the color word
**
**------------------------------------------------------------------------*/
int PtermFrame::AssembleColor (int d)
{
    if (m_ascBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler |= ((d & 077) << (m_ascBytes * 6));
    if (++m_ascBytes == 4)
    {
        m_ascBytes = 0;
        m_ascState = none;
        //TRACE ("color 0x%06x)", m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("color byte %d: %d", m_ascBytes, d & 077);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble an 7 bit grayscale word for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise the color word
**
**------------------------------------------------------------------------*/
int PtermFrame::AssembleGrayScale (int d)
{
    if (m_ascBytes == 0)
    {
        m_assembler = 0;
    }    
    m_assembler = (d & 077) << 2;
    if (++m_ascBytes == 1)
    {
        m_ascBytes = 0;
        m_ascState = none;
        TRACE ("gray-scale color 0x%06x", m_assembler);
        return m_assembler;
    }
    else
    {
        TRACE2 ("gray-scale color byte %d: %d", m_ascBytes, d & 0177);
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a coordinate pair for the ASCII protocol
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        true if word is complete, false if not
**                  lastX and lastY are the x/y coordinate received
**
**------------------------------------------------------------------------*/
bool PtermFrame::AssembleCoord (int d)
{
    int c = d & 037;
    
    switch (d >> 5)
    {
    case 1: // High X or high Y
        if (m_ascBytes == 0)
        {
            // High Y
            TRACE ("high Y %d", c);
            lastY = (lastY & 037) | (c << 5);
            m_ascBytes = 2;
        }
        else
        {
            TRACE ("high X %d", c);
            lastX = (lastX & 037) | (c << 5);
        }
        break;
    case 2:
        lastX = (lastX & 0740) | c;
        m_assembler = (lastX << 16) + lastY;
        m_ascBytes = 0;
        m_ascState = none;
        TRACE3 ("low X %d, coordinates %d %d", c, lastX, lastY);
        return true;
    case 3:
        TRACE ("low Y %d", c);
        lastY = (lastY & 0740) | c;
        m_ascBytes = 2;
        break;
    }
    return false;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a string for use as extended data from Plato
**                  sent in ASCII connection mode.  Could be meta data
**                  for setting window caption or could be font data, or
**                  other data someday.
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 if word not complete yet, otherwise 0 and get the
**                     data from m_PlatoMetaData
**
**------------------------------------------------------------------------*/
int PtermFrame::AssembleAsciiPlatoMetaData (int d)
{
	int od = d;
    const int chardelay = atoi(ptermApp->m_charDelay.mb_str());

    TRACE2 ("plato meta data: %d (counter=%d)", d, m_ascBytes+1);
	d &= 077;
    if (m_ascBytes==0)
		m_PMD = wxT("");
	m_ascBytes++;
	// check for start font mode
	if (od=='F' && m_ascBytes==1)
		m_fontPMD = true;
	// check for request font character info
	else if (od=='f' && m_ascBytes==1)
	{
		m_fontinfo = true;
		m_ascBytes = 0;
		m_ascState = none;
		ptermSendExt((int)m_fontwidth);
		wxMilliSleep(chardelay);
		ptermSendExt((int)m_fontheight);
		wxMilliSleep(chardelay);
		return 0;
	}
	// check for request operating system info
	else if (od=='o' && m_ascBytes==1)
	{
		// sends 3 external keys, OS, major version, minor version
		int os,major,minor;
		os = wxGetOsVersion(&major,&minor);
		ptermSendExt(os);
		wxMilliSleep(chardelay);
		if (os==wxMAC || os==wxMAC_DARWIN)
			ptermSendExt(10*(major>>4) + (major &0x0f));
		else
			ptermSendExt(major);
		wxMilliSleep(chardelay);
		if (os==wxMAC || os==wxMAC_DARWIN)
			ptermSendExt(10*(minor>>4) + (minor &0x0f));
		else
			ptermSendExt(minor);
		wxMilliSleep(chardelay);
		m_osinfo = true;
		m_ascBytes = 0;
		m_ascState = none;
		return 0;
	}
	// check if in font mode
	else if (m_fontPMD && m_ascBytes==2)
	{
		SetFontFaceAndFamily(d);
		if (d==0)
		{
			m_ascBytes = 0;
			m_ascState = none;
			return 0;
		}
	}
	else if (m_fontPMD && m_ascBytes==3)
		SetFontSize(d);
	else if (m_fontPMD && m_ascBytes==4)
	{
		SetFontFlags(d);
		SetFontActive();
        m_ascBytes = 0;
        m_ascState = none;
		return 0;
	}
	// check if done / full
    else if (d==0 || m_ascBytes==1001)
    {
		if (m_ascBytes==1001)
		{
		    TRACE ("plato meta data limit reached: %d bytes",1000);
		}
        m_ascBytes = 0;
        m_ascState = none;
        return 0;
    }
	// otherwise keep assembling
    else
    {
		if (d >= 1 && d <= 26)
		{
			m_PMD.Append('a'+d-1,1);
		}
		else if (d >= 27 && d <= 36)
		{
			m_PMD.Append('0'+d-27,1);
		}
		else if (d == 38)
		{
			m_PMD.Append('-',1);
		}
		else if (d == 40)
		{
			m_PMD.Append('/',1);
		}
		else if (d == 44)
		{
			m_PMD.Append('=',1);
		}
		else if (d == 45)
		{
			m_PMD.Append(' ',1);
		}
		else if (d == 63)
		{
			m_PMD.Append(';',1);
		}
    }
    return -1;
}

/*--------------------------------------------------------------------------
**  Purpose:        Assemble a string for use as frame caption from data
**                  sent in Classic connection mode.
**
**  Parameters:     Name        Description.
**                  d           current byte of input
**
**  Returns:        -1 
**
**------------------------------------------------------------------------*/
bool PtermFrame::AssembleClassicPlatoMetaData (int d)
{
    TRACE ("plato meta data: %d", d);
	if (m_PMD.Len()==1000)
	{
		TRACE ("plato meta data limit reached: 1000 bytes","");
		m_loadingPMD = false;
		return true;
	}
	else
	{
		d &= 077;
		if (d >= 1 && d <= 26)
		{
			m_PMD.Append('a'+d-1,1);
		}
		else if (d >= 27 && d <= 36)
		{
			m_PMD.Append('0'+d-27,1);
		}
		else if (d == 38)
		{
			m_PMD.Append('-',1);
		}
		else if (d == 40)
		{
			m_PMD.Append('/',1);
		}
		else if (d == 44)
		{
			m_PMD.Append('=',1);
		}
		else if (d == 45)
		{
			m_PMD.Append(' ',1);
		}
		else if (d == 63)
		{
			m_PMD.Append(';',1);
		}
	}
	return false;
}

/*--------------------------------------------------------------------------
**  Purpose:        Change frame title to string specified in the assembled
**                  meta data.
**
**  Parameters:     none
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::ProcessPlatoMetaData ()
{
	int fnd;
	int len;
	wxString l_str;
	wxString l_name;
	wxString l_group;
	wxString l_system;
	wxString l_station;

	//special check for roster
	if (m_SendRoster)
		return;
	if ((fnd = m_PMD.Find(wxT("sendroster;"))) != -1)
	{
		SendRoster();
		return;
	}

	//initialize
	l_name = wxT("");
	l_group = wxT("");
	l_system = wxT("");
	l_station = wxT("");
	len = m_PMD.Len();

	//collect meta data items
	if ((fnd = m_PMD.Find(wxT("name="))) != -1)
	{
		l_name = m_PMD.Mid(fnd+5,len-fnd-5);
		l_name = l_name.BeforeFirst(wxT(';'));
	}

	if ((fnd = m_PMD.Find(wxT("group="))) != -1)
	{
		l_group = m_PMD.Mid(fnd+6,len-fnd-6);
		l_group = l_group.BeforeFirst(wxT(';'));
	}

	if ((fnd = m_PMD.Find(wxT("system="))) != -1)
	{
		l_system = m_PMD.Mid(fnd+7,len-fnd-7);
		l_system = l_system.BeforeFirst(wxT(';'));
	}

	if ((fnd = m_PMD.Find(wxT("station="))) != -1)
	{
		l_station = m_PMD.Mid(fnd+8,len-fnd-8);
		l_station = l_station.BeforeFirst(wxT(';'));
	}

	//make title string based on flags
	l_str = wxT("");
	if (ptermApp->m_showSignon)
	{
        l_str = l_name;
        l_str.Append (wxT ("/"));
        l_str += l_group;
	}
	if (ptermApp->m_showSysName)
	{
        l_str.Append (wxT ("/"));
        l_str += l_system;
	}
	if (ptermApp->m_showHost)
	{
        l_str.Append (wxT (" "));
        l_str += m_hostName;
	}
	if (ptermApp->m_showStation)
	{
        l_str.Append (wxT (" ("));
        l_str += l_station;
        l_str.Append (wxT (")"));
	}
	l_str.Trim(true);
	l_str.Trim(false);
	if (l_str.IsEmpty())
	{
		l_str = wxT("Pterm");
	}
	SetTitle (l_str);
}


/*--------------------------------------------------------------------------
**  Purpose:        Set font face name and family flag.
**
**  Parameters:     font type code
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::SetFontFaceAndFamily (int n)
{
	switch (n)
	{
	case 2:
		m_fontface = wxT("Terminal");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
		break;
	case 3:
		m_fontface = wxT("UOL8X14");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
		break;
	case 4:
		m_fontface = wxT("UOL8X16");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
		break;
	case 5:
		m_fontface = wxT("Courier");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
		break;
	case 6:
		m_fontface = wxT("Courier New");
		m_fontfamily = wxFONTFAMILY_MODERN;
		break;
	case 16:
		m_fontface = wxT("Arial");
		m_fontfamily = wxFONTFAMILY_DECORATIVE;
		break;
	case 17:
		m_fontface = wxT("Times New Roman");
		m_fontfamily = wxFONTFAMILY_ROMAN;
		break;
	case 18:
		m_fontface = wxT("Script");
		m_fontfamily = wxFONTFAMILY_SCRIPT;
		break;
	case 19:
		m_fontface = wxT("MS Sans Serif");
		m_fontfamily = wxFONTFAMILY_SWISS;
		break;
	default:
		m_fontface = wxT("default");
		m_fontfamily = wxFONTFAMILY_TELETYPE;
	}
}

/*--------------------------------------------------------------------------
**  Purpose:        Set font size.
**
**  Parameters:     font size
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::SetFontSize (int n)
{
	m_fontsize = (n < 1 ? 1 : (n > 63 ? 63 : n)) * m_scale;
}

/*--------------------------------------------------------------------------
**  Purpose:        Set font flags.
**
**  Parameters:     font flag
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::SetFontFlags (int n)
{
	m_fontitalic = ((n & 0x01) != 0);
	m_fontbold = ((n & 0x02) != 0);
	m_fontstrike = ((n & 0x04) != 0);
	m_fontunderln = ((n & 0x08) != 0);
	TRACE3 ("Font selected: %s,%d,%d", m_fontface.c_str(), m_fontsize, n & 0x0f);
}

/*--------------------------------------------------------------------------
**  Purpose:        Set font.
**
**  Parameters:     none
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::SetFontActive ()
{
	m_usefont = (m_fontface.Cmp(wxT(""))!=0 && m_fontface.Cmp(wxT("default"))!=0);
	if (m_usefont)
	{
		m_font = new wxFont;
		//m_font->New(m_fontsize, m_fontfamily, wxFONTFLAG_NOT_ANTIALIASED, m_fontface);
		m_font->New(m_fontsize, m_fontfamily, wxFONTFLAG_ANTIALIASED | (m_fontstrike ? wxFONTFLAG_STRIKETHROUGH : 0), m_fontface);
		m_font->SetFaceName(m_fontface);
		m_font->SetFamily(m_fontfamily);
		m_font->SetPointSize(m_fontsize);
		m_font->SetStyle(m_fontitalic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
		m_font->SetWeight(m_fontbold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
		m_font->SetUnderlined(m_fontunderln);
		m_memDC->SetFont(*m_font);
		m_memDC->GetTextExtent(wxT(" "), &m_fontwidth, &m_fontheight);
	}
}

/*--------------------------------------------------------------------------
**  Purpose:        Save a screen "window"
**
**  Parameters:     Name        Description.
**                  d           window number
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::ptermSaveWindow (int d)
{
	int x,y,w,h;
#ifdef AUTOSCALE
	x = cwswindow[d].data[0];
	y = (512-cwswindow[d].data[1]);
	w = (cwswindow[d].data[2]-cwswindow[d].data[0]);
	h = (cwswindow[d].data[1]-cwswindow[d].data[3]);
#else
	x = m_scale*cwswindow[d].data[0];
	y = m_scale*(512-cwswindow[d].data[1]);
	w = m_scale*(cwswindow[d].data[2]-cwswindow[d].data[0]);
	h = m_scale*(cwswindow[d].data[1]-cwswindow[d].data[3]);
#endif
	TRACE ("CWS: process save; window %d",d);
	cwswindow[d].ok = true;
	cwswindow[d].dc = new wxMemoryDC;
	cwswindow[d].bm = new wxBitmap(w,h,-1);
	cwswindow[d].dc->SelectObject (*cwswindow[d].bm);
	cwswindow[d].dc->Blit(0,0,w,h,m_memDC,x,y,wxCOPY);
}

/*--------------------------------------------------------------------------
**  Purpose:        Restore a screen "window"
**
**  Parameters:     Name        Description.
**                  d           window number
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::ptermRestoreWindow (int d)
{
	int x,y,w,h;
#ifdef AUTOSCALE
	x = cwswindow[d].data[0];
	y = 512-cwswindow[d].data[1];
	w = (cwswindow[d].data[2]-cwswindow[d].data[0]);
	h = (cwswindow[d].data[1]-cwswindow[d].data[3]);
#else
	x = m_scale*cwswindow[d].data[0];
	y = m_scale*(512-cwswindow[d].data[1]);
	w = m_scale*(cwswindow[d].data[2]-cwswindow[d].data[0]);
	h = m_scale*(cwswindow[d].data[1]-cwswindow[d].data[3]);
#endif
	if (cwswindow[d].ok)
	{
		TRACE ("CWS: process restore; window %d",d);
		m_memDC->Blit(x,y,w,h,cwswindow[d].dc,0,0,wxCOPY);
		//m_memDC->Blit(0,0,w,h,cwswindow[d].dc,0,0,wxCOPY);
		wxClientDC dc(m_canvas);
		dc.Blit (XTOP, YTOP, vScreenSize(m_scale), vScreenSize(m_scale), m_memDC, 0, 0, wxCOPY);
		cwswindow[d].ok = false;
		delete cwswindow[d].bm;
		delete cwswindow[d].dc;
	}
}

/*--------------------------------------------------------------------------
**  Purpose:        Write a trace message.
**
**  Parameters:     message
**
**  Returns:        nothing
**
**------------------------------------------------------------------------*/
void PtermFrame::WriteTraceMessage (wxString msg)
{
	TRACE ("%s",msg.c_str());
	return;
}

/*--------------------------------------------------------------------------
**  Purpose:        Display visual indication of trace status
**
**  Parameters:     Name        Description.
**                  fileaction  true to open/flush files,
**                              false if only redisplay is needed
**
**  Returns:        nothing
**
**  This function opens or flushes the trace file depending on trace
**  setting, if fileaction is true.  It then redraws (on or off) the trace
**  indication.
**
**------------------------------------------------------------------------*/
void PtermFrame::ptermSetTrace (bool fileaction)
{
    if (!tracePterm)
    {
        if (fileaction)
        {
            fflush (traceF);
        }
    }
    else
    {
        if (fileaction && (traceF == NULL))
        {
            traceF = fopen (traceFn, "w");
        }
    }
    ptermShowTrace ();
}

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Draw one character
**
**  Parameters:     Name        Description.
**                  c           Character code
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::plotChar (int c)
{
    int &cx = (vertical) ? currentY : currentX;
    int &cy = (vertical) ? currentX : currentY;
    
    int deltax, deltay, supdelta;
    
	if (m_usefont && currentCharset <= 1)
	{
		supdelta = (m_fontheight/3);
		// Not going to support reverse/vertical in font mode until I get documentation
		// on what worked with -font- in the past.  JWS 5/27/2007
		deltax = 8;
		deltay = m_fontheight;
	}
	else
	{
		deltax = (reverse) ? -8 : 8;
		deltay = (vertical) ? -16 : 16;
		if (large)
		{
			deltax *= 2;
			deltay *= 2;
		}
		supdelta = (deltay / 16) * 5;
	}
	
	// check for uncover code and fast exit
    c &= 077;
    if (c == 077)
    {
        setUncover (true);
        return;
    }

    if (uncover)
    {
        setUncover (false);
        switch (c)
        {
        case 010:   // backspace
            cx = (cx - deltax) & 0777;
            break;
        case 011:   // tab
            cx = (cx + deltax) & 0777;
            break;
        case 012:   // linefeed
            cy = (cy - deltay) & 0777;
            break;
        case 013:   // vertical tab
            cy = (cy + deltay) & 0777;
        case 014:   // form feed
            if (vertical)
            {
                cx = deltay - 1;
                if (reverse)
                {
                    cy = 512 - deltax;
                }
                else
                {
                    cy = 0;
                }
            }
            else
            {
                cy = 512 - deltay;
                if (reverse)
                {
                    cx = 512 - deltax;
                }
                else
                {
                    cx = 0;
                }
            }
            break;
        case 015:   // carriage return
            cx = margin;
            cy = (cy - deltay) & 0777;
            break;
        case 016:   // superscript
            cy = (cy + supdelta) & 0777;
            break;
        case 017:   // subscript
            cy = (cy - supdelta) & 0777;
            break;
        case 020:   // select M0
        case 021:   // select M1
        case 022:   // select M2
        case 023:   // select M3
        case 024:   // select M4
        case 025:   // select M5
        case 026:   // select M6
        case 027:   // select M7
            setCmem (c - 020);
            break;
        case 030:   // horizontal writing
            setVertical (false);
            break;
        case 031:   // vertical writing
            setVertical (true);
            break;
        case 032:   // forward writing
            setReverse (false);
            break;
        case 033:   // reverse writing
            setReverse (true);
            break;
        case 034:   // normal size writing
            setLarge (false);
            break;
        case 035:   // double size writing
            setLarge (true);
            break;
        default:
            break;
        }
    }
	else if (m_usefont && currentCharset <= 1)
		drawFontChar(currentX, currentY, rom01char[c+64*currentCharset]);
    else
    {
        ptermDrawChar (currentX, currentY, currentCharset, c);
        cx = (cx + deltax) & 0777;
    }
}


/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 0 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::mode0 (u32 d)
{
    int x, y;
    
    x = (d >> 9) & 0777;
    y = d & 0777;
    TRACE2 ("dot %d %d", x, y);
    ptermDrawPoint (x, y);
    currentX = x;
    currentY = y;
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 1 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::mode1 (u32 d)
{
    int x, y;
    
    x = (d >> 9) & 0777;
    y = d & 0777;
    TRACE2 ("lineto %d %d", x, y);
    ptermDrawLine (currentX, currentY, x, y);
    currentX = x;
    currentY = y;
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 2 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::mode2 (u32 d)
{
    int ch, chaddr;

    // Store the word in PPT RAM
    WriteRAM (memaddr, d);
    WriteRAM (memaddr + 1, d >> 8);

    // memaddr is a PPT RAM address; convert it to a character memory address
    chaddr = memaddr - ReadRAMW (C2ORIGIN);
    if (chaddr < 0 || chaddr > 127 * 16)
    {
        TRACE2 ("memdata %04x to %04x", d & 0xffff, memaddr);
    }
    else
    {
        chaddr /= 2;
        if (((d >> 16) & 3) == 0)
        {
            // load data
            TRACE2 ("character memdata %06o to char word %04o", d & 0xffff, chaddr);
            plato_m23[chaddr] = d & 0xffff;
            if ((++chaddr & 7) == 0)
            {
                // character is done -- load it to display 
                ch = (chaddr / 8) - 1;
                ptermLoadChar (2 + (ch / 64), ch % 64, &plato_m23[chaddr - 8]);
            }
        }
    }
    memaddr += 2;
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 3 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::mode3 (u32 d)
{
    TRACE6 ("char %02o %02o %02o (%c%c%c)",
            (d >> 12) & 077, (d >> 6) & 077, d & 077,
            rom01char[(d >> 12) & 077], 
            rom01char[(d >> 6) & 077], 
            rom01char[d & 077]);
    plotChar (d >> 12);
    plotChar (d >> 6);
    plotChar (d);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 4 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::mode4 (u32 d)
{
    int x1, y1, x2, y2;
    
    if (modewords & 1)
    {
        mode4start = d;
        return;
    }
    x1 = (mode4start >> 9) & 0777;
    y1 = mode4start & 0777;
    x2 = (d >> 9) & 0777;
    y2 = d & 0777;
    
    TRACE4 ("block erase %d %d to %d %d", x1, y1, x2, y2);

    ptermBlockErase (x1, y1, x2, y2);
    currentX = x1;
    currentY = y1 - 15;

}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 5 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::mode5 (u32 d)
{
    TRACE ("mode5 %06o", d);
    progmode (d, M5ORIGIN);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 6 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::mode6 (u32 d)
{
    TRACE ("mode6 %06o", d);
    progmode (d, M6ORIGIN);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 7 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::mode7 (u32 d)
{
    TRACE ("mode7 %06o", d);
    progmode (d, M7ORIGIN);
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Mode 5/6/7 data word
**
**  Parameters:     Name        Description.
**                  d           Data word
**                  origin      Address of mode handler pointer
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::progmode (u32 d, int origin)
{
    // Load C/D/E with data word
    BC.reg.C = d >> 16;
    DE.reg.D = d >> 8;
    DE.reg.E = d;

    // Initialize the stack
    SP = INITSP;

    // Push the fake return address for "return to main loop" onto the
    // stack, as if we just did a CALL instruction
    WriteRAM(--SP, R_MAIN >> 8);
    WriteRAM(--SP, R_MAIN);

    // Set the start PC for the requested mode
    PC = ReadRAMW (origin);
    main8080a ();
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Plato mode keyboard input
**
**  Parameters:     Name        Description.
**                  key         Plato key code for station
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::ptermSendKey (int key)
{
    char data[5];
    int len;
    
    if (m_conn == NULL)
    {
        return;
    }
    

    /*
    **  If this is a "composite", recursively send the two pieces.
    */
    if ((key >> 9) != 0 && key != xonkey && key != xofkey)
    {
        // Don't do composite keys if ASCII and in dumb terminal mode
        if (!(m_conn->Ascii () && m_dumbTty))
        {
            ptermSendKey (key >> 9);
            if ((key >> 9) != 074)
            {
                ptermSendKey (074);
            }
            ptermSendKey (key & 0777);
        }
    }
    else
    {
        if (tracePterm)
        {
            fprintf (traceF, "key to plato %03o\n", key);
        }
#ifdef DEBUGLOG
        wxLogMessage (wxT("key to plato %03o"), key);
#elif DEBUGKEY
        printf ("key to plato %03o\n", key);
#endif
        if (m_conn->Ascii ())
        {
            // Assume one byte key code
            len = 1;
            if (key < 0200)
            {
                // Regular keyboard key
                key = asciiKeycodes[key];
                if (key == 0xff)
                {
                    return;
                }
                if (m_flowCtrl)
                {
                    // Do the keycode translation for the
                    // "flow control enabled" coding rules.
                    switch (key)
                    {
                    case 0x00:              // access
                        key = 0x1d;
                        len = 2;
                        break;
                    case 0x05:              // shift-sub
                        key = 0x04;
                        len = 2;
                        break;
                    case 0x0a:              // tab
                        key = 0x09;
                        break;
                    case 0x09:              // shift-help
                        key = 0x0a;
                        break;
                    case 0x11:              // shift-stop
                        key = 0x05;
                        break;
                    case 0x17:              // shift-super
                        len = 2;
                        // fall through
                    case 0x13:              // super
                        key = 0x17;
                        break;
                    case 0x7c:              // apostrophe
                        key = 0x27;
                        break;
                    case 0x27:              // #
                        key = 0x7c;
                        break;
                    }
                    data[0] = 033;          // store esc for 2 byte codes
                }                        
                data[len - 1] = Parity (key);
                if (tracePterm)
                {
                    if (len == 1)
                    {
                        fprintf (traceF, "ascii mode key to plato 0x%02x\n", data[0] & 0xff);
                    }
                    else
                    {
                        fprintf (traceF, "ascii mode key to plato 0x%02x 0x%02x\n", data[0], data[1] & 0xff);
                    }
                }
                m_conn->SendData (data, len);
                if (m_dumbTty)
                {
                    // do local echoing
                    m_conn->StoreWord (key);
                }
            }
            else if (!m_dumbTty)
            {
                if (key == xofkey)
                {
                    if (!m_flowCtrl)
                    {
                        return;
                    }
                    data[0] = Parity (ascxof);
                    if (tracePterm)
                    {
                        fprintf (traceF, "ascii mode key to plato XOFF\n");
                    }
                }
                else if (key == xonkey)
                {
                    if (!m_flowCtrl)
                    {
                        return;
                    }
                    data[0] = Parity (ascxon);
                    if (tracePterm)
                    {
                        fprintf (traceF, "ascii mode key to plato XON\n");
                    }
                }
                else
                {
                    len = 3;
                    data[0] = 033;
                    data[1] = Parity (0100 + (key & 077));
                    data[2] = Parity (0140 + (key >> 6));
                    if (tracePterm)
                    {
                        fprintf (traceF, "ascii mode key to plato 0x%02x 0x%02x 0x%02x\n", 
                                 data[0] & 0xff, data[1] & 0xff, data[2] & 0xff);
                    }
                }
                m_conn->SendData (data, len);
            }
        }
        else
        {
            data[0] = key >> 7;
            data[1] = 0200 | key;
            m_conn->SendData (data, 2);
        }
    }
}

/*--------------------------------------------------------------------------
**  Purpose:        Process Plato mode keyboard input (multiple keys)
**
**  Parameters:     Name        Description.
**                  key         Array of integers of Plato key codes
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void PtermFrame::ptermSendKeys (int key[])
{
	for (int i=0; key[i] != -1; i++)
	{
		ptermSendKey(key[i]);
		m_pasteTimer.Start (atoi(ptermApp->m_charDelay.mb_str()), true);
	}
}
			
void PtermFrame::ptermSendTouch (int x, int y)
{
    char data[6];

    if (m_sendFgt)
    {
        // Send fine grid touch code first
        data[0] = 033;
        data[1] = 0x1f;
        data[2] = 0x40 + (x & 0x1f);
        data[3] = 0x40 + ((x >> 5) & 0x0f);
        data[4] = 0x40 + (y & 0x1f);
        data[5] = 0x40 + ((y >> 5) & 0x0f);
        m_conn->SendData (data, 6);
    }
    
    x /= 32;
    y /= 32;

    ptermSendKey (0x100 | (x << 4) | y);
}

void PtermFrame::ptermSendExt (int key)
{
    char data[3];

    if (m_conn->Ascii ())
    {
        // Send external key
        data[0] = 033;
        data[1] = 0x40 | (key & 0x3f);
        data[2] = 0x68 | ((key >> 6) & 0x03);
        m_conn->SendData (data, 3);
	}
	else
	{
        data[0] = 0x04 | ((key >> 7) & 0x01);
        data[1] = 0x80 | (key & 0x7f);
        m_conn->SendData (data, 2);
    }
    
}

#if 0
int PtermFrame::Parity (int key)
{
    int i;
    int p = 0;
    key &= 0177;
    for (i = 0; i < 8; i++)
    {
        if (key & (1 << i))
        {
            p ^= 0200;
        }
    }
    return key;// | p;
}
#endif

void PtermFrame::ptermSetStation (int station, bool showtitle, bool showstatus)
{

	//this routine is called only when connection is initially made
	//see ProcessPlatoMetaData for the dynamic code.
    wxString l_hostname;
    wxString l_station;
    wxString l_str;

    SetCursor (wxNullCursor);
    m_station = station;

	//build string for frame name
    if (m_hostName.IsEmpty ())
    {
		l_hostname = wxT("");
	}
	else
    {
		l_hostname = m_hostName.c_str();
	}
    if (station < 0)
    {
		l_station = wxT("ASCII");
    }
    else
    {
		l_station.Printf(wxT("%d-%d"),station >> 5, station & 31);
    }

	// only show what the user wants to see in title bar
	if (showtitle)
	{
		l_str = wxT("");
		if (ptermApp->m_showHost)
		{
			l_str.Printf(wxT("%s "), l_hostname.c_str());
		}
		if (ptermApp->m_showStation)
		{
			l_str.Printf(wxT("%s"), l_str.c_str());
			l_str.Append(l_station.c_str());
		}
		ptermSetName (l_str);
	}

	//always show host and station in status bar
	if (showstatus)
	{
		l_str.Printf(wxT("%s "), l_hostname.c_str());
		l_str.Append(l_station.c_str());
		ptermSetStatus (l_str);
	}
}

void PtermFrame::ptermShowTrace ()
{
	if (m_statusBar != NULL)
	{
		if (tracePterm)
			m_statusBar->SetStatusText(_(" Trace "), STATUS_TRC);
		else if (ptermApp->m_platoKb)
			m_statusBar->SetStatusText(_(" PLATO keyboard "), STATUS_TRC);
		else
			m_statusBar->SetStatusText(wxT (""), STATUS_TRC);
	}
}

// This emulates the "ROM resident".  Return values:
// 0: PC is not special (not in resident), proceed normally.
// 1: PC is ROM function entry point, it has been emulated,
//    do a RET now.
// 2: PC is either the 8080 emulation exit magic value, or R_INIT,
//    or an invalid resident value.  Exit 8080 emulation.
int PtermFrame::check_pc8080a (void)
{
    int x, y, cp, c, x2, y2;
    
    if (PC < WORKRAM)
    {
        if (tracePterm)
        {
            fprintf (traceF, "Resident call %04x DE=%04x HL=%04x\n", 
                     PC, DE.pair, HL.pair);
        }
    }
    
    switch (PC)
    {
    case R_MAIN:
        // "r.main" -- fake return address value used as the return
        // address for invocations of the mode 5/6/7 handler code
        return 2;

    case R_INIT:
        // r.init -- TBD
        return 2;
        
    case R_DOT:
        x = HL.pair;
        y = DE.pair;
        ptermDrawPoint (x, y);
        currentX = x;
        currentY = y;
        return 1;
        
    case R_LINE:
        x = HL.pair;
        y = DE.pair;
        ptermDrawLine (currentX, currentY, x, y);
        currentX = x;
        currentY = y;
        return 1;

    case R_CHARS:
        // draw chars ending at 07700
        cp = HL.pair;
        c = RAM[cp++];
        for (;;)
        {
            if (c == 077 && RAM[cp] == 0)
            {
                break;
            }
            plotChar (c);
            c = RAM[cp++];
        }
        return 1;
        
    case R_BLOCK:
        // block erase
        cp = HL.pair;
        x = ReadRAMW (cp);
        y = ReadRAMW (cp + 2);
        x2 = ReadRAMW (cp + 4);
        y2 = ReadRAMW (cp + 6);
        ptermBlockErase (x, y, x2, y2);
        return 1;
        
    case R_INPX:
        HL.pair = currentX;
        return 1;

    case R_INPY:
        HL.pair = currentY;
        return 1;
        
    case R_OUTX:
        currentX = HL.pair;
        return 1;
        
    case R_OUTY:
        currentY = HL.pair;
        return 1;
        
    case R_XMIT:
        // send key in HL
        return 1;
        
    case R_MODE:
        // set mode from L
        if (HL.reg.L & 1)
        {
            ptermFullErase ();
        }
		modexor = false;
        mode = (HL.reg.L >> 1) & 037;
        return 1;
        
    case R_STEPX:
        currentX = (currentX + ((RAM[M_DIR] & 2) ? -1 : 1)) & 0777;
        return 1;
        
    case R_STEPY:
        currentY = (currentY + ((RAM[M_DIR] & 1) ? -1 : 1)) & 0777;
        return 1;
        
    case R_WE:
        ptermDrawPoint (currentX, currentY);
        return 1;
        
    case R_DIR:
        RAM[M_DIR] = HL.reg.L & 3;
        return 1;
        
    case R_INPUT:
        // r.input
        return 1;
        
    case R_SSH:
        // r.ssh
        return 1;
        
    case R_CCR:
        RAM[M_CCR] = HL.reg.L;
        return 1;
        
    case R_EXTOUT:
        // r.extout
        return 1;
        
    case R_EXEC:
        // r.exec
        return 1;
        
    case R_GJOB:
        // r.gjob
        return 1;
        
    case R_XJOB:
        // r.xjob
        return 1;
        
    default:
        if (PC < WORKRAM)
        {
            // Wild jump into ROM resident, quit
            fprintf (stderr, "Wild jump to %04x\n", PC);
            return 2;
        }
        else
        {
            // Plain old RAM PC -- keep executing
            return 0;
        }
    }
}

/*******************************************************************************

input8080a:
----------

   This case switches on the content of data.  If the content of data is equal
to 1, ...
The program counter, named PC, is incremented by one.


Input:
-----

   data - This variable is the byte after the INp instruction, RAM[PC].


Output:
------

   This variable is the data that has been read or calculated from the input
device and returned to the main simulator core.


Note:
----

   This function is called directly by main8080a.

*******************************************************************************/

Uint8 PtermFrame::input8080a (Uint8 data)
{

	/***********************************************************************
	* Declaration:
	* -----------
	*
	*   retval - This variable contains data read from an input device.
	***********************************************************************/
	Uint8 retval;


	switch (data)
	{

	/***********************************************************************
	*   If the value of the 8080a's memory, RAM[PC], is equal to 1 or 2,
	* then first or second player input is requested.
	***********************************************************************/
		case 1:
		case 2:
			break;

	/***********************************************************************
	*   If the value of the 8080a's memory, RAM[PC], is not equal to any of
	* the above cases, then the program requested bad input data. Debug
	* information containing the value of the data is printed to STDOUT.
	***********************************************************************/
		default:
			printf( "INp BAD -> Data = %d\n", data);

			retval = 0;
			break;
	}


	/***********************************************************************
	*   This keyword returns the read input data to the main simulator core.
	***********************************************************************/
	return retval;
}



/*******************************************************************************

output8080a:
-----------

   This case switches on the contents of the data.  If the content of data is
equal to 2, ...
If the content of data is any other value, then the request is bad and 
debugging information is printed to STDOUT.


Input:
-----

   data - This variable is the byte after the OUTp instruction, RAM[PC].
   acc - This variable is the accumulator, which is the 8080a's A register.


Note:
----

   This function is called directly by main8080a.

*******************************************************************************/

void PtermFrame::output8080a (Uint8 data, Uint8 acc)
{
	switch (data)
	{

	/***********************************************************************
	*   If the value of the 8080a's memory, RAM[PC], is equal to 2, the
	* content of the A register is moved into the left shift amount.
	***********************************************************************/
		case 2:
			break;


	/***********************************************************************
	*   If the value of the 8080a's memory, RAM[PC], is not equal to any of
	* the above cases, then the program requested bad output data.  Debug
	* information containing the value of the data and the accumulator are
	* printed to STDOUT.
	***********************************************************************/
		default:
			printf("OUTp BAD -> Data = %d   A = %d\n", data, acc);
			break;
	}
}


// ----------------------------------------------------------------------------
// PtermPrefDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(PtermPrefDialog, wxDialog)
    EVT_CLOSE(PtermPrefDialog::OnClose)
    EVT_BUTTON(wxID_ANY, PtermPrefDialog::OnButton)
    EVT_CHECKBOX(wxID_ANY, PtermPrefDialog::OnCheckbox)
	EVT_LISTBOX(wxID_ANY, PtermPrefDialog::OnSelect)
	EVT_LISTBOX_DCLICK(wxID_ANY, PtermPrefDialog::OnDoubleClick)
	EVT_TEXT(wxID_ANY, PtermPrefDialog::OnChange)
	EVT_COMBOBOX(wxID_ANY, PtermPrefDialog::OnComboSelect)
    END_EVENT_TABLE()

PtermPrefDialog::PtermPrefDialog (PtermFrame *parent, wxWindowID id, const wxString &title, wxPoint pos, wxSize size)
    : wxDialog (parent, id, title, pos, size),
      m_owner (parent)
{

	// static ui objects, note dynamic controls, e.g. those that hold values or require event processing are declared above
//	wxNotebook* tabPrefsDialog;
	//tab0
	wxScrolledWindow* tab0;
	wxStaticText* lblExplain0;
//	wxListBox* lstProfiles;
	wxStaticText* lblProfileActionExplain;
//	wxButton* btnSave;
//	wxButton* btnLoad;
//	wxButton* btnDelete;
//	wxStaticText* lblProfileStatusMessage;
	wxStaticText* lblNewProfileExplain;
	wxStaticText* lblNewProfile;
//	wxTextCtrl* txtProfile;
//	wxButton* btnAdd;
	//tab1
	wxScrolledWindow* tab3;
	wxStaticText* lblExplain3;
//	wxCheckBox* chkConnectAtStartup;
	wxStaticText* lblShellFirst;
//	wxTextCtrl* txtShellFirst;
	wxStaticText* lblDefaultHost;
//	wxTextCtrl* txtDefaultHost;
	wxStaticText* lblDefaultPort;
//	wxComboBox* cboDefaultPort;
	wxStaticText* lblExplainPort;
	//tab2
	wxScrolledWindow* tab1;
	wxStaticText* lblExplain1;
//	wxCheckBox* chkShowSignon;
//	wxCheckBox* chkShowSysName;
//	wxCheckBox* chkShowHost;
//	wxCheckBox* chkShowStation;
	//tab3
	wxScrolledWindow* tab2;
	wxStaticText* lblExplain2;
//	wxCheckBox* chkSimulate1200Baud;
//	wxCheckBox* chkEnableGSW;
//	wxCheckBox* chkEnableNumericKeyPad;
//	wxCheckBox* chkIgnoreCapLock;
//	wxCheckBox* chkUsePLATOKeyboard;
//	wxCheckBox* chkUseAccelerators;
//	wxCheckBox* chkEnableBeep;
//	wxCheckBox* chkDisableShiftSpace;
//	wxCheckBox* chkDisableMouseDrag;
	//tab4
	wxScrolledWindow* tab4;
	wxStaticText* lblExplain4;
//	wxCheckBox* chkZoom200;
//	wxCheckBox* chkStatusBar;
//	wxCheckBox* chkMenuBar;
//	wxCheckBox* chkDisableColor;
//	wxButton* btnFGColor;// windows
//	wxBitmapButton* btnFGColor;// other
	wxStaticText* lblFGColor;
//	wxButton* btnBGColor;// windows
//	wxBitmapButton* btnBGColor;// other
	wxStaticText* lblBGColor;
	wxStaticText* lblExplainColor;
	//tab5
	wxScrolledWindow* tab5;
	wxStaticText* lblExplain5;
	wxStaticText* lblCharDelay;
//	wxTextCtrl* txtCharDelay;
	wxStaticText* lblCharDelay2;
	wxStaticText* lblLineDelay;
//	wxTextCtrl* txtLineDelay;
	wxStaticText* lblLineDelay2;
	wxStaticText* lblAutoNewLine;
//	wxComboBox* cboAutoLF;
	wxStaticText* lblAutoNewLine2;
//	wxCheckBox* chkSplitWords;
//	wxCheckBox* chkSmartPaste;
//	wxCheckBox* chkConvertDot7;
//	wxCheckBox* chkConvert8Spaces;
//	wxCheckBox* chkTutorColor;
	wxStaticText* lblExplainConversions;
	//tab6
	wxScrolledWindow* tab6;
	wxStaticText* lblBrowser;
	wxStaticText* lblEmail;
	wxStaticText* lblSearchURL;
//	wxTextCtrl* txtBrowser;
//	wxTextCtrl* txtEmail;
//	wxTextCtrl* txtSearchURL;
	//button  bar
//	wxButton* btnOK;
//	wxButton* btnCancel;
//	wxButton* btnDefaults;
    

	
	// ui object creation / placement, note initialization of values is below
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	//notebook
	tabPrefsDialog = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP );
	tabPrefsDialog->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	//tab0
	tab0 = new wxScrolledWindow( tabPrefsDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	tab0->SetScrollRate( 5, 5 );
	tab0->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	wxBoxSizer* page0;
	page0 = new wxBoxSizer( wxVERTICAL );
	lblExplain0 = new wxStaticText( tab0, wxID_ANY, _("Use this page to manage your connection preference profiles."), wxDefaultPosition, wxDefaultSize, 0 );
	page0->Add( lblExplain0, 0, wxALL, 5 );
	lstProfiles = new wxListBox( tab0, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, wxLB_SORT ); 
	page0->Add( lstProfiles, 1, wxALL|wxEXPAND, 5 );
	wxFlexGridSizer* fgs01;
	fgs01 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgs01->SetFlexibleDirection( wxVERTICAL );
	lblProfileActionExplain = new wxStaticText( tab0, wxID_ANY, _("Select a profile above, then click a button below."), wxDefaultPosition, wxDefaultSize, 0 );
	fgs01->Add( lblProfileActionExplain, 0, wxALL, 5 );
	wxFlexGridSizer* fgs011;
	fgs011 = new wxFlexGridSizer( 2, 4, 0, 0 );
	btnSave = new wxButton( tab0, wxID_ANY, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	btnSave->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs011->Add( btnSave, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	btnLoad = new wxButton( tab0, wxID_ANY, _("Load"), wxDefaultPosition, wxDefaultSize, 0 );
	btnLoad->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs011->Add( btnLoad, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	btnDelete = new wxButton( tab0, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	btnDelete->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs011->Add( btnDelete, 0, wxRIGHT|wxLEFT, 5 );
	lblProfileStatusMessage = new wxStaticText( tab0, wxID_ANY, _("Profile saved."), wxDefaultPosition, wxDefaultSize, 0 );
	lblProfileStatusMessage->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs011->Add( lblProfileStatusMessage, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	fgs01->Add( fgs011, 1, 0, 5 );
	lblNewProfileExplain = new wxStaticText( tab0, wxID_ANY, _("Or, enter the name of a new profile below and click Add."), wxDefaultPosition, wxDefaultSize, 0 );
	fgs01->Add( lblNewProfileExplain, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	wxFlexGridSizer* fgs012;
	fgs012 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgs012->AddGrowableCol( 1 );
	fgs012->SetFlexibleDirection( wxHORIZONTAL );
	lblNewProfile = new wxStaticText( tab0, wxID_ANY, _("Profile"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs012->Add( lblNewProfile, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	txtProfile = new wxTextCtrl( tab0, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	txtProfile->SetMinSize( wxSize( 310,-1 ) );
	fgs012->Add( txtProfile, 1, wxALL|wxEXPAND, 5 );
	btnAdd = new wxButton( tab0, wxID_ANY, _("Add"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	fgs012->Add( btnAdd, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	fgs01->Add( fgs012, 1, wxEXPAND, 5 );
	page0->Add( fgs01, 0, wxEXPAND, 5 );
	tab0->SetSizer( page0 );
	tab0->Layout();
	page0->Fit( tab0 );
	tabPrefsDialog->AddPage( tab0, _("Profiles"), true );
	//tab1
	tab1 = new wxScrolledWindow( tabPrefsDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	tab1->SetScrollRate( 5, 5 );
	wxFlexGridSizer* page1;
	page1 = new wxFlexGridSizer( 1, 1, 0, 0 );
	page1->AddGrowableCol( 1 );
	page1->AddGrowableRow( 1 );
	page1->SetFlexibleDirection( wxVERTICAL );
	lblExplain1 = new wxStaticText( tab1, wxID_ANY, _("Settings on this page specify where PLATO is on the internet."), wxDefaultPosition, wxDefaultSize, 0 );
	page1->Add( lblExplain1, 1, wxALL|wxEXPAND, 5 );
	wxBoxSizer* bs11;
	bs11 = new wxBoxSizer( wxVERTICAL );
	chkConnectAtStartup = new wxCheckBox( tab1, wxID_ANY, _("Connect at startup"), wxDefaultPosition, wxDefaultSize, 0 );
	chkConnectAtStartup->SetValue(true);
	bs11->Add( chkConnectAtStartup, 0, wxEXPAND|wxTOP|wxLEFT, 5 );
	wxFlexGridSizer* fgs111;
	fgs111 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgs111->AddGrowableCol( 1 );
	fgs111->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	lblShellFirst = new wxStaticText( tab1, wxID_ANY, _("Run this first"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShellFirst->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs111->Add( lblShellFirst, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	txtShellFirst = new wxTextCtrl( tab1, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	txtShellFirst->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	txtShellFirst->SetMaxLength( 255 ); 
	fgs111->Add( txtShellFirst, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	lblDefaultHost = new wxStaticText( tab1, wxID_ANY, _("Default Host"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs111->Add( lblDefaultHost, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	txtDefaultHost = new wxTextCtrl( tab1, wxID_ANY, wxT("cyberserv.org"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	txtDefaultHost->SetMaxLength( 100 ); 
	txtDefaultHost->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs111->Add( txtDefaultHost, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	lblDefaultPort = new wxStaticText( tab1, wxID_ANY, _("Default Port*"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs111->Add( lblDefaultPort, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	cboDefaultPort = new wxComboBox( tab1, wxID_ANY, wxT("5004"), wxDefaultPosition, wxSize( 75,-1 ), 0, NULL, 0 );
	cboDefaultPort->Append( wxT("5004") );
	cboDefaultPort->Append( wxT("8005") );
	fgs111->Add( cboDefaultPort, 0, wxALL, 5 );
	bs11->Add( fgs111, 1, wxEXPAND, 5 );
	page1->Add( bs11, 1, wxEXPAND, 0 );
	lblExplainPort = new wxStaticText( tab1, wxID_ANY, _("* NOTE: 5004=Classic, 8005=Color Terminal"), wxDefaultPosition, wxDefaultSize, 0 );
	page1->Add( lblExplainPort, 0, wxALL, 5 );
	tab1->SetSizer( page1 );
	tab1->Layout();
	page1->Fit( tab1 );
	tabPrefsDialog->AddPage( tab1, _("Connection"), true );
	//tab2
	tab2 = new wxScrolledWindow( tabPrefsDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	tab2->SetScrollRate( 5, 5 );
	tab2->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	wxBoxSizer* page2;
	page2 = new wxBoxSizer( wxVERTICAL );
	lblExplain2 = new wxStaticText( tab2, wxID_ANY, _("Settings on this page configure the text shown in the window title."), wxDefaultPosition, wxDefaultSize, 0 );
	page2->Add( lblExplain2, 0, wxALL, 5 );
	chkShowSignon = new wxCheckBox( tab2, wxID_ANY, _("Show name/group in frame title"), wxDefaultPosition, wxDefaultSize, 0 );
	chkShowSignon->SetValue(true);
	page2->Add( chkShowSignon, 0, wxALL, 5 );
	chkShowSysName = new wxCheckBox( tab2, wxID_ANY, _("Show system name in frame title"), wxDefaultPosition, wxDefaultSize, 0 );
	chkShowSysName->SetValue(true);
	page2->Add( chkShowSysName, 0, wxALL, 5 );
	chkShowHost = new wxCheckBox( tab2, wxID_ANY, _("Show host name in frame title"), wxDefaultPosition, wxDefaultSize, 0 );
	chkShowHost->SetValue(true);
	page2->Add( chkShowHost, 0, wxALL, 5 );
	chkShowStation = new wxCheckBox( tab2, wxID_ANY, _("Show site-station numbers in frame title"), wxDefaultPosition, wxDefaultSize, 0 );
	chkShowStation->SetValue(true);
	page2->Add( chkShowStation, 0, wxALL, 5 );
	tab2->SetSizer( page2 );
	tab2->Layout();
	page2->Fit( tab2 );
	tabPrefsDialog->AddPage( tab2, _("Title"), false );
	//tab3
	tab3 = new wxScrolledWindow( tabPrefsDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	tab3->SetScrollRate( 5, 5 );
	wxBoxSizer* page3;
	page3 = new wxBoxSizer( wxVERTICAL );
	lblExplain3 = new wxStaticText( tab3, wxID_ANY, _("Settings on this page let you fine-tune your PLATO experience."), wxDefaultPosition, wxDefaultSize, 0 );
	page3->Add( lblExplain3, 0, wxALL, 5 );
	chkSimulate1200Baud = new wxCheckBox( tab3, wxID_ANY, _("Simulate 1200 baud"), wxDefaultPosition, wxDefaultSize, 0 );
	page3->Add( chkSimulate1200Baud, 0, wxALL, 5 );
	chkEnableGSW = new wxCheckBox( tab3, wxID_ANY, _("Enable GSW (not in ASCII)"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEnableGSW->SetValue(true);
	page3->Add( chkEnableGSW, 0, wxALL, 5 );
	chkEnableNumericKeyPad = new wxCheckBox( tab3, wxID_ANY, _("Enable numeric keypad for arrow operation"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEnableNumericKeyPad->SetValue(true);
	page3->Add( chkEnableNumericKeyPad, 0, wxALL, 5 );
	chkIgnoreCapLock = new wxCheckBox( tab3, wxID_ANY, _("Ignore CAPS LOCK"), wxDefaultPosition, wxDefaultSize, 0 );
	chkIgnoreCapLock->SetValue(true);
	page3->Add( chkIgnoreCapLock, 0, wxALL, 5 );
	chkUsePLATOKeyboard = new wxCheckBox( tab3, wxID_ANY, _("Use real PLATO keyboard"), wxDefaultPosition, wxDefaultSize, 0 );
	page3->Add( chkUsePLATOKeyboard, 0, wxALL, 5 );
	chkUseAccelerators = new wxCheckBox( tab3, wxID_ANY, _("Enable control-key menu accelerators"), wxDefaultPosition, wxDefaultSize, 0 );
	chkUseAccelerators->SetValue(true);
	chkUseAccelerators->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	#if defined(__WXMAC__)
		chkUseAccelerators->Disable();
	#endif
	page3->Add( chkUseAccelerators, 0, wxALL, 5 );
	chkEnableBeep = new wxCheckBox( tab3, wxID_ANY, _("Enable -beep-"), wxDefaultPosition, wxDefaultSize, 0 );
	chkEnableBeep->SetValue(true);
	page3->Add( chkEnableBeep, 0, wxALL, 5 );
	chkDisableShiftSpace = new wxCheckBox( tab3, wxID_ANY, _("Disable Shift-Space (backspace via Ctrl-Space)"), wxDefaultPosition, wxDefaultSize, 0 );
	chkDisableShiftSpace->SetValue(false);
	page3->Add( chkDisableShiftSpace, 0, wxALL, 5 );
	chkDisableMouseDrag = new wxCheckBox( tab3, wxID_ANY, _("Disable mouse drag (select)"), wxDefaultPosition, wxDefaultSize, 0 );
	chkDisableMouseDrag->SetValue(false);
	page3->Add( chkDisableMouseDrag, 0, wxALL, 5 );
	tab3->SetSizer( page3 );
	tab3->Layout();
	page3->Fit( tab3 );
	tabPrefsDialog->AddPage( tab3, _("Emulation"), false );
	//tab4
	tab4 = new wxScrolledWindow( tabPrefsDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	tab4->SetScrollRate( 5, 5 );
	wxFlexGridSizer* page4;
	page4 = new wxFlexGridSizer( 1, 1, 0, 0 );
	page4->AddGrowableCol( 1 );
	page4->AddGrowableRow( 1 );
	page4->SetFlexibleDirection( wxVERTICAL );
	lblExplain4 = new wxStaticText( tab4, wxID_ANY, _("Settings on this page allow you to change the display appearance."), wxDefaultPosition, wxDefaultSize, 0 );
	page4->Add( lblExplain4, 0, wxALL, 5 );
	wxBoxSizer* bs41;
	bs41 = new wxBoxSizer( wxVERTICAL );

	//chkZoom200 = new wxCheckBox( tab4, wxID_ANY, _("Zoom display 200%"), wxDefaultPosition, wxDefaultSize, 0 );
	//bs41->Add( chkZoom200, 0, wxALL, 5 );
	//chkStatusBar = new wxCheckBox( tab4, wxID_ANY, _("Display status bar"), wxDefaultPosition, wxDefaultSize, 0 );
	//chkStatusBar->SetValue(true);
	//bs41->Add( chkStatusBar, 0, wxALL, 5 );
	//chkMenuBar = new wxCheckBox( tab4, wxID_ANY, _("Display menu bar"), wxDefaultPosition, wxDefaultSize, 0 );
	//chkMenuBar->SetValue(true);
	//bs41->Add( chkMenuBar, 0, wxALL, 5 );

	chkDisableColor = new wxCheckBox( tab4, wxID_ANY, _("Disable -color- (ASCII mode)"), wxDefaultPosition, wxDefaultSize, 0 );
	bs41->Add( chkDisableColor, 0, wxALL, 5 );
	wxFlexGridSizer* fgs411;
	fgs411 = new wxFlexGridSizer( 2, 2, 0, 0 );
	#if defined(_WIN32)
		btnFGColor = new wxButton( tab4, wxID_ANY, wxT(""), wxDefaultPosition, wxSize( 25,-1 ), 0 );
		btnFGColor->SetBackgroundColour( wxColour( 255, 128, 0 ) );
		fgs411->Add( btnFGColor, 0, wxALL, 5 );
		lblFGColor = new wxStaticText( tab4, wxID_ANY, _("Foreground color*"), wxDefaultPosition, wxDefaultSize, 0 );
		fgs411->Add( lblFGColor, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
		btnBGColor = new wxButton( tab4, wxID_ANY, wxT(""), wxDefaultPosition, wxSize( 25,-1 ), 0 );
		btnBGColor->SetBackgroundColour( wxColour( 0, 0, 0 ) );
		fgs411->Add( btnBGColor, 0, wxALL, 5 );
		lblBGColor = new wxStaticText( tab4, wxID_ANY, _("Background color*"), wxDefaultPosition, wxDefaultSize, 0 );
		fgs411->Add( lblBGColor, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	#else
		btnFGColor = new wxBitmapButton( tab4, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
		fgs411->Add( btnFGColor, 0, wxALL, 5 );
		lblFGColor = new wxStaticText( tab4, wxID_ANY, _("Foreground color*"), wxDefaultPosition, wxDefaultSize, 0 );
		fgs411->Add( lblFGColor, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
		btnBGColor = new wxBitmapButton( tab4, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
		fgs411->Add( btnBGColor, 0, wxALL, 5 );
		lblBGColor = new wxStaticText( tab4, wxID_ANY, _("Background color*"), wxDefaultPosition, wxDefaultSize, 0 );
		fgs411->Add( lblBGColor, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	#endif
	bs41->Add( fgs411, 1, wxEXPAND, 5 );
	page4->Add( bs41, 1, wxEXPAND, 5 );
	lblExplainColor = new wxStaticText( tab4, wxID_ANY, _("* NOTE: Applied in Classic mode or if -color- is disabled in ASCII mode"), wxDefaultPosition, wxDefaultSize, 0 );
	page4->Add( lblExplainColor, 0, wxALL|wxALIGN_BOTTOM, 5 );
	tab4->SetSizer( page4 );
	tab4->Layout();
	page4->Fit( tab4 );
	tabPrefsDialog->AddPage( tab4, _("Display"), false );
	//tab5
	tab5 = new wxScrolledWindow( tabPrefsDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	tab5->SetScrollRate( 5, 5 );
	wxFlexGridSizer* page5;
	page5 = new wxFlexGridSizer( 1, 1, 0, 0 );
	page5->AddGrowableCol( 1 );
	page5->AddGrowableRow( 1 );
	page5->SetFlexibleDirection( wxVERTICAL );
	wxBoxSizer* bs51;
	bs51 = new wxBoxSizer( wxVERTICAL );
	lblExplain5 = new wxStaticText( tab5, wxID_ANY, _("Settings on this page allow you to specify Paste options."), wxDefaultPosition, wxDefaultSize, 0 );
	bs51->Add( lblExplain5, 0, wxALL, 5 );
	wxFlexGridSizer* fgs511;
	fgs511 = new wxFlexGridSizer( 2, 3, 0, 0 );
	lblCharDelay = new wxStaticText( tab5, wxID_ANY, _("Delay between chars"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs511->Add( lblCharDelay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	txtCharDelay = new wxTextCtrl( tab5, wxID_ANY, _("50"), wxDefaultPosition, wxSize( 48,-1 ), 0 );
	txtCharDelay->SetMaxLength( 3 ); 
	fgs511->Add( txtCharDelay, 0, wxALL, 5 );
	lblCharDelay2 = new wxStaticText( tab5, wxID_ANY, _("milliseconds"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs511->Add( lblCharDelay2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	lblLineDelay = new wxStaticText( tab5, wxID_ANY, _("Delay after end of line"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs511->Add( lblLineDelay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	txtLineDelay = new wxTextCtrl( tab5, wxID_ANY, _("100"), wxDefaultPosition, wxSize( 48,-1 ), 0 );
	txtLineDelay->SetMaxLength( 3 ); 
	fgs511->Add( txtLineDelay, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	lblLineDelay2 = new wxStaticText( tab5, wxID_ANY, _("milliseconds"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs511->Add( lblLineDelay2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	bs51->Add( fgs511, 1, wxEXPAND, 5 );
	wxFlexGridSizer* fgs512;
	fgs512 = new wxFlexGridSizer( 2, 3, 0, 0 );
	lblAutoNewLine = new wxStaticText( tab5, wxID_ANY, _("Automatic new line every"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs512->Add( lblAutoNewLine, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	cboAutoLF = new wxComboBox( tab5, wxID_ANY, _("60"), wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0|wxTAB_TRAVERSAL );
	cboAutoLF->Append( _("0") );
	cboAutoLF->Append( _("60") );
	cboAutoLF->Append( _("120") );
	cboAutoLF->SetMinSize( wxSize( 65,-1 ) );
	fgs512->Add( cboAutoLF, 1, wxALL, 5 );
	lblAutoNewLine2 = new wxStaticText( tab5, wxID_ANY, _("characters"), wxDefaultPosition, wxDefaultSize, 0 );
	fgs512->Add( lblAutoNewLine2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	bs51->Add( fgs512, 0, wxEXPAND, 5 );
	chkSplitWords = new wxCheckBox( tab5, wxID_ANY, _("Allow words to be split across lines"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSplitWords->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	bs51->Add( chkSplitWords, 0, wxALL, 5 );
	chkSmartPaste = new wxCheckBox( tab5, wxID_ANY, _("Use TUTOR pasting (certain sequences are treated specially)"), wxDefaultPosition, wxDefaultSize, 0 );
	chkSmartPaste->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	bs51->Add( chkSmartPaste, 0, wxALL, 5 );
	chkConvertDot7 = new wxCheckBox( tab5, wxID_ANY, _("Convert periods followed by 7 spaces into period/tab*"), wxDefaultPosition, wxDefaultSize, 0 );
	chkConvertDot7->SetValue(true);
	//chkConvertDot7->Hide();
	bs51->Add( chkConvertDot7, 0, wxALL, 5 );
	chkConvert8Spaces = new wxCheckBox( tab5, wxID_ANY, _("Convert 8 consecutive spaces into a tab*"), wxDefaultPosition, wxDefaultSize, 0 );
	chkConvert8Spaces->SetValue(true);
	//chkConvert8Spaces->Hide();
	bs51->Add( chkConvert8Spaces, 0, wxALL, 5 );
	chkTutorColor = new wxCheckBox( tab5, wxID_ANY, _("Display TUTOR colorization options on Edit/Context menus"), wxDefaultPosition, wxDefaultSize, 0 );
	chkTutorColor->SetValue(true);
	//chkTutorColor->Hide();
	bs51->Add( chkTutorColor, 0, wxALL, 5 );
	page5->Add( bs51, 1, wxEXPAND, 5 );
	lblExplainConversions = new wxStaticText( tab5, wxID_ANY, _("* NOTE: Conversions are applied at 8-character intervals."), wxDefaultPosition, wxDefaultSize, 0 );
	//lblExplainConversions->Hide();
	page5->Add( lblExplainConversions, 0, wxALL|wxALIGN_BOTTOM, 5 );
	tab5->SetSizer( page5 );
	tab5->Layout();
	page5->Fit( tab5 );
	tabPrefsDialog->AddPage( tab5, _("Pasting"), false );
	//tab6
	tab6 = new wxScrolledWindow( tabPrefsDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	tab6->SetScrollRate( 5, 5 );
	tab6->Hide();
	wxFlexGridSizer* page6;
	page6 = new wxFlexGridSizer( 0, 1, 0, 0 );
	page6->AddGrowableCol( 0 );
	page6->SetFlexibleDirection( wxBOTH );
	lblBrowser = new wxStaticText( tab6, wxID_ANY, _("Specify browser to use for menu option 'Execute URL'"), wxDefaultPosition, wxDefaultSize, 0 );
	page6->Add( lblBrowser, 0, wxALIGN_BOTTOM|wxALL, 5 );
	txtBrowser = new wxTextCtrl( tab6, wxID_ANY, wxT(""), wxPoint( -1,-1 ), wxDefaultSize, 0|wxTAB_TRAVERSAL );
	txtBrowser->SetMaxLength( 255 ); 
	page6->Add( txtBrowser, 0, wxALL|wxEXPAND, 5 );
	lblEmail = new wxStaticText( tab6, wxID_ANY, _("Command line for menu option 'Mail to...' (%s=address)"), wxDefaultPosition, wxDefaultSize, 0 );
	page6->Add( lblEmail, 0, wxALL|wxALIGN_BOTTOM, 5 );
	txtEmail = new wxTextCtrl( tab6, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	txtEmail->SetMaxLength( 255 ); 
	txtEmail->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	page6->Add( txtEmail, 0, wxALL|wxEXPAND, 5 );
	lblSearchURL = new wxStaticText( tab6, wxID_ANY, _("Specify URL for menu option 'Search this...'"), wxDefaultPosition, wxDefaultSize, 0 );
	page6->Add( lblSearchURL, 0, wxALL, 5 );
	txtSearchURL = new wxTextCtrl( tab6, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	txtSearchURL->SetMaxLength( 255 ); 
	page6->Add( txtSearchURL, 0, wxALL|wxEXPAND, 5 );
	tab6->SetSizer( page6 );
	tab6->Layout();
	page6->Fit( tab6 );
	tabPrefsDialog->AddPage( tab6, _("Local"), false );
	//notebook
	bSizer1->Add( tabPrefsDialog, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 6 );
	//button bar
	wxFlexGridSizer* fgsButtons;
	fgsButtons = new wxFlexGridSizer( 1, 5, 0, 0 );
	fgsButtons->AddGrowableCol( 0 );
	fgsButtons->AddGrowableRow( 0 );
	fgsButtons->SetFlexibleDirection( wxHORIZONTAL );
	fgsButtons->Add( 0, 0, 1, wxALL|wxEXPAND, 5 );
	btnOK = new wxButton( this, wxID_ANY, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	btnOK->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgsButtons->Add( btnOK, 0, wxALL, 5 );
	btnCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	btnCancel->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgsButtons->Add( btnCancel, 0, wxALL, 5 );
	btnDefaults = new wxButton( this, wxID_ANY, _("Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
	btnDefaults->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgsButtons->Add( btnDefaults, 0, wxALL, 5 );
	bSizer1->Add( fgsButtons, 0, wxEXPAND, 5 );
	this->SetSizer( bSizer1 );
	this->Layout();

	// set object value properties
	SetControlState();

	//set active tab
	m_lastTab = ptermApp->m_config->Read (wxT (PREF_LASTTAB), 0L);
	tabPrefsDialog->SetSelection ( m_lastTab );

	//button bar
	btnOK->SetDefault ();
}

void PtermPrefDialog::OnClose (wxCloseEvent& event)
{
	EndModal (wxID_CANCEL);
}

bool PtermPrefDialog::ValidProfile(wxString profile)
{
	wxString validchr = wxT ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_. ()");
	unsigned int cnt;
	for (cnt=0;cnt<profile.Len();cnt++)
		if (!validchr.Contains(profile.Mid(cnt,1)))
			return false;
	return true;
}

bool PtermPrefDialog::SaveProfile(wxString profile)
{
	wxString filename;
	wxString buffer;
	bool openok;

	//open file
	filename = ptermApp->ProfileFileName(profile);
	wxTextFile file(filename);
	if (file.Exists())
		openok = file.Open();
	else
		openok = file.Create();
	if (!openok)
		return false;
	file.Clear();

    //write prefs
	//tab0
	buffer.Printf(wxT (PREF_CURPROFILE "=%s"), profile.c_str());
	file.AddLine(buffer);
	//tab1
    buffer.Printf(wxT (PREF_CONNECT "=%d"), (m_connect) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_SHELLFIRST "=%s"), m_ShellFirst.c_str());
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_HOST "=%s"), m_host.c_str());
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_PORT "=%s"), m_port.c_str());
	file.AddLine(buffer);
	//tab2
	buffer.Printf(wxT (PREF_SHOWSIGNON "=%d"), (m_showSignon) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_SHOWSYSNAME "=%d"), (m_showSysName) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_SHOWHOST "=%d"), (m_showHost) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_SHOWSTATION "=%d"), (m_showStation) ? 1 : 0);
	file.AddLine(buffer);
	//tab3
    buffer.Printf(wxT (PREF_1200BAUD "=%d"), (m_classicSpeed) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_GSW "=%d"), (m_gswEnable) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_ARROWS "=%d"), (m_numpadArrows) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_IGNORECAP "=%d"), (m_ignoreCapLock) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_PLATOKB "=%d"), (m_platoKb) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_ACCEL "=%d"), (m_useAccel) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_BEEP "=%d"), (m_beepEnable) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_SHIFTSPACE "=%d"), (m_DisableShiftSpace) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_MOUSEDRAG "=%d"), (m_DisableMouseDrag) ? 1 : 0);
	file.AddLine(buffer);
	//tab4
    buffer.Printf(wxT (PREF_SCALE "=%d"), (m_scale2) ? 2 : 1);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_STATUSBAR "=%d"), (m_showStatusBar) ? 1 : 0);
	file.AddLine(buffer);
#if !defined(__WXMAC__)
    buffer.Printf(wxT (PREF_MENUBAR "=%d"), (m_showMenuBar) ? 1 : 0);
	file.AddLine(buffer);
#endif
    buffer.Printf(wxT (PREF_NOCOLOR "=%d"), (m_noColor) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_FOREGROUND "=%d %d %d"), m_fgColor.Red (), m_fgColor.Green (), m_fgColor.Blue ());
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_BACKGROUND "=%d %d %d"), m_bgColor.Red (), m_bgColor.Green (), m_bgColor.Blue ());
	file.AddLine(buffer);
	//tab5
    buffer.Printf(wxT (PREF_CHARDELAY "=%s"), m_charDelay.c_str());
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_LINEDELAY "=%s"), m_lineDelay.c_str());
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_AUTOLF "=%s"), m_autoLF.c_str());
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_SPLITWORDS "=%d"), (m_splitWords) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_SMARTPASTE "=%d"), (m_smartPaste) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_CONVDOT7 "=%d"), (m_convDot7) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_CONV8SP "=%d"), (m_conv8Sp) ? 1 : 0);
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_TUTORCOLOR "=%d"), (m_TutorColor) ? 1 : 0);
	file.AddLine(buffer);
	//tab6
    buffer.Printf(wxT (PREF_BROWSER "=%s"), m_Browser.c_str());
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_EMAIL "=%s"), m_Email.c_str());
	file.AddLine(buffer);
    buffer.Printf(wxT (PREF_SEARCHURL "=%s"), m_SearchURL.c_str());
	file.AddLine(buffer);

	//write to disk
	file.Write();
	file.Close();

	return true;

}

bool PtermPrefDialog::DeleteProfile(wxString profile)
{
	wxString filename;

	//delete file
	filename = ptermApp->ProfileFileName(profile);
	if (wxFileExists(filename))
	{
		wxRemoveFile(filename);
		return true;
	}
	return false;
}

void PtermPrefDialog::SetControlState(void)
{
#if !defined(_WIN32)
    wxBitmap fgBitmap (15, 15);
    wxBitmap bgBitmap (15, 15);
#endif
	//tab0
	m_curProfile = ptermApp->m_curProfile;
	//tab1
	m_ShellFirst = ptermApp->m_ShellFirst;
    m_connect = ptermApp->m_connect;
    m_host = ptermApp->m_hostName;
    m_port.Printf (wxT ("%d"), ptermApp->m_port);
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
    m_scale2 = (ptermApp->m_scale != 1);
    m_showStatusBar = ptermApp->m_showStatusBar;
#if !defined(__WXMAC__)
    m_showMenuBar = ptermApp->m_showMenuBar;
#endif
    m_noColor = ptermApp->m_noColor;
    //m_fgColor = ptermApp->m_fgColor;
    //m_bgColor = ptermApp->m_bgColor;
    int r, g, b;
    wxString rgb;
    ptermApp->m_config->Read (wxT (PREF_FOREGROUND), &rgb, wxT ("255 144 0"));// 255 144 0 is RGB for Plato Orange
    sscanf (rgb.mb_str(), "%d %d %d", &r, &g, &b);
    m_fgColor = wxColour (r, g, b);
    ptermApp->m_config->Read (wxT (PREF_BACKGROUND), &rgb, wxT ("0 0 0"));
    sscanf (rgb.mb_str(), "%d %d %d", &r, &g, &b);
    m_bgColor = wxColour (r, g, b);
	//tab5
    m_charDelay = ptermApp->m_charDelay;
    m_lineDelay = ptermApp->m_lineDelay;
    m_autoLF = ptermApp->m_autoLF;
	m_splitWords = ptermApp->m_splitWords;
	m_smartPaste = ptermApp->m_smartPaste;
	m_convDot7 = ptermApp->m_convDot7;
	m_conv8Sp = ptermApp->m_conv8Sp;
	m_TutorColor = ptermApp->m_TutorColor;
	//tab6
	m_Browser = ptermApp->m_Browser;
	m_Email = ptermApp->m_Email;
	m_SearchURL = ptermApp->m_SearchURL;
	
	//tab0
	wxDir ldir(wxGetCwd());
	if ( ldir.IsOpened() )
    {
		// populate listbox
		wxString filename;
		bool cont = ldir.GetFirst(&filename, wxT ("*.ppf"), wxDIR_DEFAULT);
		lstProfiles->Clear();
		lstProfiles->Append(CURRENT_PROFILE);
		int i,cur=0;
		wxString str;
		for (i=0;cont;i++)
		{
			filename = filename.Left(filename.Len()-4);
			lstProfiles->Append(filename);
			cont = ldir.GetNext(&filename);
		}
		for (i=0;i<(int)lstProfiles->GetCount();i++)
		{
			filename = lstProfiles->GetString(i);
			if (filename.CmpNoCase(ptermApp->m_curProfile) == 0)
				cur = i;
		}
		lstProfiles->Select(cur);
	}
	//tab1
    chkConnectAtStartup->SetValue ( m_connect );
    txtShellFirst->SetValue ( m_ShellFirst );
    txtDefaultHost->SetValue ( m_host );
    cboDefaultPort->SetValue ( m_port );
	//tab2
	chkShowSignon->SetValue( m_showSignon );
	chkShowSysName->SetValue( m_showSysName );
	chkShowHost->SetValue( m_showHost );
	chkShowStation->SetValue( m_showStation );
	//tab3
    chkSimulate1200Baud->SetValue ( m_classicSpeed );
    chkEnableGSW->SetValue ( m_gswEnable );
    chkEnableNumericKeyPad->SetValue ( m_numpadArrows );
    chkIgnoreCapLock->SetValue ( m_ignoreCapLock );
    chkUsePLATOKeyboard->SetValue ( m_platoKb );
    chkUseAccelerators->SetValue ( m_useAccel );
    chkEnableBeep->SetValue ( m_beepEnable );
    chkDisableShiftSpace->SetValue ( m_DisableShiftSpace );
    chkDisableMouseDrag->SetValue ( m_DisableMouseDrag );
	//tab4
    //chkZoom200->SetValue ( m_scale2 );
	//chkStatusBar->SetValue( m_showStatusBar );
#if !defined(__WXMAC__)
	//chkMenuBar->SetValue( m_showMenuBar );
#endif
    chkDisableColor->SetValue ( m_noColor );
	#if defined(_WIN32)
		btnFGColor->SetBackgroundColour( m_fgColor );
		btnBGColor->SetBackgroundColour( m_bgColor );
	#else
		paintBitmap (fgBitmap, m_fgColor);
		btnFGColor->SetBitmapLabel( fgBitmap );
		paintBitmap (bgBitmap, m_bgColor);
		btnBGColor->SetBitmapLabel( bgBitmap );
	#endif
	//tab5
	txtCharDelay->SetValue( m_charDelay );
	txtLineDelay->SetValue( m_lineDelay );
	cboAutoLF->SetValue(m_autoLF );
	chkSplitWords->SetValue( m_splitWords );
	chkSmartPaste->SetValue( m_smartPaste );
	chkConvertDot7->SetValue( m_convDot7 );
	chkConvert8Spaces->SetValue( m_conv8Sp );
	chkTutorColor->SetValue( m_TutorColor );
	//tab6
	txtBrowser->SetValue ( m_Browser );
	txtEmail->SetValue ( m_Email );
	txtSearchURL->SetValue ( m_SearchURL );

	//set button state
	wxString profile;
	bool enable;
	profile = lstProfiles->GetStringSelection();
	enable = (profile.Cmp(CURRENT_PROFILE)!=0);
	btnSave->Enable(enable);
	btnLoad->Enable(enable);
	btnDelete->Enable(enable);
	profile = txtProfile->GetLineText(0);
	btnAdd->Enable(!profile.IsEmpty());
}

void PtermPrefDialog::OnButton (wxCommandEvent& event)
{
    wxBitmap fgBitmap (15, 15);
    wxBitmap bgBitmap (15, 15);
	wxString profile;
	wxString filename;
	wxString str;
    
	void OnButton (wxCommandEvent& event);
	lblProfileStatusMessage->SetLabel(wxT(""));
    if (event.GetEventObject () == btnSave)
    {
		profile = lstProfiles->GetStringSelection();
		ptermApp->m_curProfile = profile;
		m_curProfile = profile;
		if (SaveProfile(profile))
		{
			ptermApp->LoadProfile(profile,wxT(""));
			SetControlState();
			lblProfileStatusMessage->SetLabel(_("Profile saved."));
		}
		else
		{
			filename = ptermApp->ProfileFileName(profile);
			str.Printf(wxT ("Unable to save profile: %s"), filename.c_str());
			wxMessageBox(str, _("Error"), wxOK | wxICON_HAND );
		}
	}
    else if (event.GetEventObject () == btnLoad)
    {
		profile = lstProfiles->GetStringSelection();
		ptermApp->m_curProfile = profile;
		m_curProfile = profile;
		if (ptermApp->LoadProfile(profile,wxT("")))
		{
			SetControlState();
			lblProfileStatusMessage->SetLabel(_("Profile loaded."));
		}
		else
		{
			filename = ptermApp->ProfileFileName(profile);
			str.Printf(wxT ("Unable to load profile: %s"), filename.c_str());
			wxMessageBox(str, _("Error"), wxOK | wxICON_HAND );
		}
	}
    else if (event.GetEventObject () == btnDelete)
    {
		profile = lstProfiles->GetStringSelection();
		if (DeleteProfile(profile))
		{
			SetControlState();
			lblProfileStatusMessage->SetLabel(_("Profile deleted."));
		}
		else
		{
			filename = ptermApp->ProfileFileName(profile);
			str.Printf(wxT ("Unable to delete profile: %s"), 
                       filename.c_str());
			wxMessageBox(str, _("Error"), wxOK | wxICON_HAND );
		}
	}
    else if (event.GetEventObject () == btnAdd)
    {
		profile = txtProfile->GetLineText(0);
		if (ValidProfile(profile))
		{
			if (SaveProfile(profile))
			{
				ptermApp->m_curProfile = profile;
				m_curProfile = profile;
				ptermApp->LoadProfile(profile,wxT(""));
				SetControlState();
				lblProfileStatusMessage->SetLabel(_("Profile added."));
			}
			else
			{
				filename = ptermApp->ProfileFileName(profile);
				str.Printf(wxT ("Unable to add/save profile: %s"), 
                           filename.c_str());
				wxMessageBox(str, _("Error"), wxOK | wxICON_HAND );
			}
		}
		else
		{
			wxMessageBox(_("The profile name you entered contains illegal characters.\n\n"
				         "Valid characters are:\n\n"
						 "Standard: a-z, A-Z, 0-9\n"
                           "Special:  parentheses, dash, underscore, period, and space"), _("Problem"), wxOK | wxICON_EXCLAMATION);
		}
	}
    else if (event.GetEventObject () == btnFGColor)
    {
        m_fgColor = PtermApp::SelectColor ( *this, _("Foreground"), m_fgColor );
		#if defined(_WIN32)
			btnFGColor->SetBackgroundColour (m_fgColor);
		#else
			paintBitmap (fgBitmap, m_fgColor);
			btnFGColor->SetBitmapLabel (fgBitmap);
		#endif
    }
    else if (event.GetEventObject () == btnBGColor)
    {
        m_bgColor = PtermApp::SelectColor ( *this, _("Background"), m_bgColor );
		#if defined(_WIN32)
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
		m_lastTab = tabPrefsDialog->GetSelection();
        EndModal (wxID_CANCEL);
	}
    else if (event.GetEventObject () == btnDefaults)
    {
        wxString str;
		
		//reset variable values
        //tab0
		//tab1
        m_ShellFirst = wxT("");
        m_connect = true;
		m_host = DEFAULTHOST;
		str.Printf (wxT ("%d"), DefNiuPort );
		m_port = str;
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
		#if defined(__WXMAC__)
			m_useAccel = true;
		#else
			m_useAccel = false;
		#endif
        m_beepEnable = true;
        m_DisableShiftSpace = false;
        m_DisableMouseDrag = false;
		//tab4
        m_scale2 = false;
        m_showStatusBar = true;
#if !defined(__WXMAC__)
        m_showMenuBar = true;
#endif
        m_noColor = false;
        m_fgColor = wxColour (255, 144, 0);
        m_bgColor = *wxBLACK;
		//tab5
		m_charDelay.Printf (wxT ("%d"), PASTE_CHARDELAY );
		m_lineDelay.Printf (wxT ("%d"), PASTE_LINEDELAY );
		m_autoLF = wxT( "0" );
        m_splitWords = false;
        m_smartPaste = false;
        m_convDot7 = false;
        m_conv8Sp = false;
		m_TutorColor = false;
		//tab6
		m_Browser = wxT("");
		m_Email = wxT("");
		m_SearchURL = wxT("");

		//reset object values
		//tab0
		//tab1
		txtShellFirst->SetValue ( m_ShellFirst );
		chkConnectAtStartup->SetValue ( m_connect );
		txtDefaultHost->SetValue ( m_host );
		cboDefaultPort->SetValue ( m_port );
		//tab2
		chkShowSignon->SetValue ( m_showSignon );
		chkShowSysName->SetValue ( m_showSysName );
		chkShowHost->SetValue ( m_showHost );
		chkShowStation->SetValue ( m_showStation );
		//tab3
		chkSimulate1200Baud->SetValue ( m_classicSpeed );
		chkEnableGSW->SetValue ( m_gswEnable );
		chkEnableNumericKeyPad->SetValue ( m_numpadArrows );
		chkIgnoreCapLock->SetValue ( m_ignoreCapLock );
		chkUsePLATOKeyboard->SetValue ( m_platoKb );
		chkUseAccelerators->SetValue ( m_useAccel );
		chkEnableBeep->SetValue ( m_beepEnable );
		chkDisableShiftSpace->SetValue ( m_DisableShiftSpace );
		chkDisableMouseDrag->SetValue ( m_DisableMouseDrag );
		//tab4
		//chkZoom200->SetValue ( m_scale2 );
		//chkStatusBar->SetValue ( m_showStatusBar );
#if !defined(__WXMAC__)
		//chkMenuBar->SetValue ( m_showMenuBar );
#endif
		chkDisableColor->SetValue ( m_noColor );
		btnFGColor->SetBackgroundColour ( m_fgColor );
		btnBGColor->SetBackgroundColour ( m_bgColor );
		//tab5
		txtCharDelay->SetValue ( m_charDelay );
		txtLineDelay->SetValue ( m_lineDelay );
		cboAutoLF->SetValue ( m_autoLF );
		chkSplitWords->SetValue ( m_splitWords );
		chkSmartPaste->SetValue ( m_smartPaste );
		chkConvertDot7->SetValue ( m_convDot7 );
		chkConvert8Spaces->SetValue ( m_conv8Sp );
		chkTutorColor->SetValue ( m_TutorColor );
		//tab6
		txtBrowser->SetValue ( m_Browser );
		txtEmail->SetValue ( m_Email );
		txtSearchURL->SetValue ( m_SearchURL );
    }
    Refresh ();
}

void PtermPrefDialog::OnCheckbox (wxCommandEvent& event)
{
	void OnCheckbox (wxCommandEvent& event);
	lblProfileStatusMessage->SetLabel(wxT(" "));
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
    //else if (event.GetEventObject () == chkZoom200)
    //    m_scale2 = event.IsChecked ();
    //else if (event.GetEventObject () == chkStatusBar)
    //    m_showStatusBar = event.IsChecked ();
#if !defined(__WXMAC__)
    //else if (event.GetEventObject () == chkMenuBar)
    //    m_showMenuBar = event.IsChecked ();
#endif
    else if (event.GetEventObject () == chkDisableColor)
        m_noColor = event.IsChecked ();
	//tab5
    else if (event.GetEventObject () == chkSplitWords)
        m_splitWords = event.IsChecked ();
    else if (event.GetEventObject () == chkSmartPaste)
        m_smartPaste = event.IsChecked ();
    else if (event.GetEventObject () == chkConvertDot7)
        m_convDot7 = event.IsChecked ();
    else if (event.GetEventObject () == chkConvert8Spaces)
        m_conv8Sp = event.IsChecked ();
    else if (event.GetEventObject () == chkTutorColor)
        m_TutorColor = event.IsChecked ();
	//tab6
}

void PtermPrefDialog::OnSelect (wxCommandEvent& event)
{
	void OnSelect (wxCommandEvent& event);
	wxString profile;
	bool enable;
	lblProfileStatusMessage->SetLabel(wxT(" "));
    if (event.GetEventObject () == lstProfiles)
    {
		profile = lstProfiles->GetStringSelection();
		enable = (profile.Cmp(CURRENT_PROFILE)!=0);
		btnSave->Enable(enable);
		btnLoad->Enable(enable);
		btnDelete->Enable(enable);
		profile = txtProfile->GetLineText(0);
		btnAdd->Enable(!profile.IsEmpty());
	}
}

void PtermPrefDialog::OnComboSelect (wxCommandEvent& event)
{
	void OnComboSelect (wxCommandEvent& event);
	lblProfileStatusMessage->SetLabel(wxT(" "));
	//tab1
    if (event.GetEventObject () == cboDefaultPort)
        m_port = cboDefaultPort->GetValue ();
	//tab5
    else if (event.GetEventObject () == cboAutoLF)
        m_autoLF = cboAutoLF->GetStringSelection ();
}

void PtermPrefDialog::OnDoubleClick (wxCommandEvent& event)
{
	void OnDoubleClick (wxCommandEvent& event);
	wxString profile;
	bool enable;
	wxString str;
	wxString filename;
	lblProfileStatusMessage->SetLabel(wxT(" "));
    if (event.GetEventObject () == lstProfiles)
    {
		profile = lstProfiles->GetStringSelection();
		enable = (profile.Cmp(CURRENT_PROFILE)!=0);
		btnSave->Enable(enable);
		btnLoad->Enable(enable);
		btnDelete->Enable(enable);
		profile = txtProfile->GetLineText(0);
		btnAdd->Enable(!profile.IsEmpty());
		//do the load code
		if (enable)
		{
			profile = lstProfiles->GetStringSelection();
			if (ptermApp->LoadProfile(profile,wxT("")))
			{
				SetControlState();
				lblProfileStatusMessage->SetLabel(_("Profile loaded."));
			}
			else
			{
				filename = ptermApp->ProfileFileName(profile);
				str.Printf( wxT ("Unable to load profile: %s"), 
                            filename.c_str());
				wxMessageBox(str, _("Error"), wxOK | wxICON_HAND );
			}
		}
	}
}

void PtermPrefDialog::OnChange (wxCommandEvent& event)
{
	void OnChange (wxCommandEvent& event);
	wxString profile;
	lblProfileStatusMessage->SetLabel(wxT(" "));
    if (event.GetEventObject () == txtProfile)
    {
		profile = txtProfile->GetLineText(0);
		btnAdd->Enable(!profile.IsEmpty());
	}
	//tab1
    else if (event.GetEventObject () == txtShellFirst)
        m_ShellFirst = txtShellFirst->GetLineText (0);
    else if (event.GetEventObject () == txtDefaultHost)
        m_host = txtDefaultHost->GetLineText (0);
    else if (event.GetEventObject () == cboDefaultPort)
        m_port = cboDefaultPort->GetValue ();
	//tab5
    else if (event.GetEventObject () == cboAutoLF)
        m_autoLF = cboAutoLF->GetValue ();
	//tab5
    else if (event.GetEventObject () == txtCharDelay)
        m_charDelay = txtCharDelay->GetLineText (0);
    else if (event.GetEventObject () == txtLineDelay)
        m_lineDelay = txtLineDelay->GetLineText (0);
	//tab6
    else if (event.GetEventObject () == txtBrowser)
		m_Browser = txtBrowser->GetLineText (0);
    else if (event.GetEventObject () == txtEmail)
		m_Email = txtEmail->GetLineText (0);
    else if (event.GetEventObject () == txtSearchURL)
		m_SearchURL = txtSearchURL->GetLineText (0);
}

void PtermPrefDialog::paintBitmap (wxBitmap &bm, wxColour &color)
{
    wxBrush bitmapBrush (color, wxSOLID);
    wxMemoryDC memDC;
    memDC.SelectObject (bm);
    memDC.SetBackground (bitmapBrush);
    memDC.Clear ();
    memDC.SetBackground (wxNullBrush);
    memDC.SelectObject (wxNullBitmap);
}


// ----------------------------------------------------------------------------
// PtermConnDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(PtermConnDialog, wxDialog)
    EVT_CLOSE(PtermConnDialog::OnClose)
	EVT_LISTBOX(wxID_ANY, PtermConnDialog::OnSelect)
	EVT_LISTBOX_DCLICK(wxID_ANY, PtermConnDialog::OnDoubleClick)
	EVT_TEXT(wxID_ANY, PtermConnDialog::OnChange)
    EVT_BUTTON(wxID_ANY, PtermConnDialog::OnButton)
    END_EVENT_TABLE()

PtermConnDialog::PtermConnDialog (wxWindowID id, const wxString &title, wxPoint pos, wxSize loc)
    : wxDialog (NULL, id, title, pos, loc)
{

	// static ui objects, note dynamic controls, e.g. those that hold values or require event processing are declared above
	wxStaticText* lblExplainProfiles;
//	wxListBox* lstProfiles;
	wxStaticText* lblShellFirst;
//	wxTextCtrl* txtShellFirst;
	wxStaticText* lblExplainManual;
	wxStaticText* lblHost;
//	wxTextCtrl* txtHost;
	wxStaticText* lblPort;
//	wxComboBox* cboPort;
	wxStaticText* lblExplainPort;
//	wxButton* btnCancel;
//	wxButton* btnConnect;

	// ui object creation / placement, note initialization of values is below
	wxBoxSizer* bs1;
	bs1 = new wxBoxSizer( wxVERTICAL );
	lblExplainProfiles = new wxStaticText( this, wxID_ANY, _("Select a profile and click Connect."), wxDefaultPosition, wxDefaultSize, 0 );
	lblExplainProfiles->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	bs1->Add( lblExplainProfiles, 0, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	lstProfiles = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, wxLB_SORT ); 
	lstProfiles->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	lstProfiles->SetMinSize( wxSize( -1,122 ) );
	bs1->Add( lstProfiles, 0, wxALL|wxEXPAND, 5 );
	lblExplainManual = new wxStaticText( this, wxID_ANY, _("Or, enter a hostname and port number, then click Connect."), wxDefaultPosition, wxDefaultSize, 0 );
	lblExplainManual->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	bs1->Add( lblExplainManual, 0, wxALL, 5 );
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer11->AddGrowableCol( 0 );
	fgSizer11->AddGrowableRow( 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	wxFlexGridSizer* fgs111;
	fgs111 = new wxFlexGridSizer( 2, 2, 0, 0 );
	lblShellFirst = new wxStaticText( this, wxID_ANY, _("Run this first"), wxDefaultPosition, wxDefaultSize, 0 );
	lblShellFirst->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs111->Add( lblShellFirst, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	txtShellFirst = new wxTextCtrl( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	txtShellFirst->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	txtShellFirst->SetMaxLength( 255 ); 
	fgs111->Add( txtShellFirst, 0, wxALL|wxEXPAND, 5 );
	lblHost = new wxStaticText( this, wxID_ANY, _("Host name"), wxDefaultPosition, wxDefaultSize, 0 );
	lblHost->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs111->Add( lblHost, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	txtHost = new wxTextCtrl( this, wxID_ANY, wxT("cyberserv.org"), wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL );
	txtHost->SetMaxLength( 100 ); 
	txtHost->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	txtHost->SetMinSize( wxSize( 320,-1 ) );
	fgs111->Add( txtHost, 0, wxALL, 5 );
	lblPort = new wxStaticText( this, wxID_ANY, _("Port*"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPort->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs111->Add( lblPort, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	cboPort = new wxComboBox( this, wxID_ANY, wxT("5004"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxTAB_TRAVERSAL );
	cboPort->Append( wxT("5004") );
	cboPort->Append( wxT("8005") );
	cboPort->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	cboPort->SetMinSize( wxSize( 75,-1 ) );
	fgs111->Add( cboPort, 0, wxALL, 5 );
	fgSizer11->Add( fgs111, 0, 0, 5 );
	lblExplainPort = new wxStaticText( this, wxID_ANY, _("* NOTE: 5004=Classic, 8005=Color Terminal"), wxDefaultPosition, wxDefaultSize, 0 );
	lblExplainPort->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgSizer11->Add( lblExplainPort, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	wxFlexGridSizer* fgs112;

	fgs112 = new wxFlexGridSizer( 2, 3, 0, 0 );
	fgs112->AddGrowableCol( 0 );
	fgs112->SetFlexibleDirection( wxBOTH );
	fgs112->Add( 0, 0, 1, wxALL, 5 );
	btnCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	btnCancel->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs112->Add( btnCancel, 0, wxALL, 5 );
	btnConnect = new wxButton( this, wxID_ANY, _("Connect"), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	btnConnect->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs112->Add( btnConnect, 0, wxALL, 5 );
	fgSizer11->Add( fgs112, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	bs1->Add( fgSizer11, 0, wxEXPAND, 5 );
	this->SetSizer( bs1 );
	this->Layout();
	bs1->Fit( this );
	btnConnect->SetDefault ();
	btnConnect->SetFocus ();

	// populate dropdown
	wxDir ldir(wxGetCwd());
	if ( ldir.IsOpened() )
    {
		wxString filename;
		bool cont = ldir.GetFirst(&filename, wxT ("*.ppf"), wxDIR_DEFAULT);
		lstProfiles->Append(CURRENT_PROFILE);
		int i,cur=0;
		for (i=0;cont;i++)
		{
			filename = filename.Left(filename.Len()-4);
			lstProfiles->Append(filename);
			cont = ldir.GetNext(&filename);
		}
		for (i=0;i<(int)lstProfiles->GetCount();i++)
		{
			filename = lstProfiles->GetString(i);
			if (filename.CmpNoCase(ptermApp->m_curProfile) == 0)
				cur = i;
		}
		lstProfiles->Select(cur);
	}

	// initialize values
    m_ShellFirst = ptermApp->m_ShellFirst;
    m_host = ptermApp->m_hostName;
    m_port.Printf (wxT ("%d"), ptermApp->m_port);

	// set object value properties
    txtShellFirst->SetValue ( m_ShellFirst );
    txtHost->SetValue ( m_host );
    cboPort->SetValue ( m_port );

}

void PtermConnDialog::OnButton (wxCommandEvent& event)
{
	void OnButton (wxCommandEvent& event);
    if (event.GetEventObject () == btnCancel)
	    EndModal (wxID_CANCEL);
    if (event.GetEventObject () == btnConnect)
    {
		m_curProfile = lstProfiles->GetStringSelection();
	    m_ShellFirst = txtShellFirst->GetLineText (0);
        m_host = txtHost->GetLineText (0);
        m_port = cboPort->GetValue ();
	    EndModal (wxID_OK);
	}
}

void PtermConnDialog::OnSelect (wxCommandEvent& event)
{
	void OnSelect (wxCommandEvent& event);
	wxString str;
	wxString profile;
	wxString filename;
    if (event.GetEventObject () == lstProfiles)
    {
		profile = lstProfiles->GetStringSelection();
		if (profile.Cmp(CURRENT_PROFILE)==0)
			;
		else if (ptermApp->LoadProfile(profile,wxT("")))
		{
			m_curProfile = profile;
		    m_ShellFirst = ptermApp->m_ShellFirst;
			m_host = ptermApp->m_hostName;
			m_port.Printf (wxT ("%d"), ptermApp->m_port);
			txtShellFirst->SetValue ( m_ShellFirst );
			txtHost->SetValue ( m_host );
			cboPort->SetValue ( m_port );
		}
		else
		{
			filename = ptermApp->ProfileFileName(profile);
			str.Printf(wxT ("Profile '%s' not found. Missing file:\n\n"), profile.c_str());
			str.Append(filename.c_str());
			wxMessageBox(str, wxT ("Error"), wxOK | wxICON_HAND);
		}
	}
}

void PtermConnDialog::OnChange (wxCommandEvent& event)
{
	void OnChange (wxCommandEvent& event);
    if (event.GetEventObject () == txtShellFirst)
        m_ShellFirst = txtShellFirst->GetLineText (0);
}

void PtermConnDialog::OnDoubleClick (wxCommandEvent& event)
{
	void OnDoubleClick (wxCommandEvent& event);
	wxString str;
	wxString profile;
	wxString filename;
	if (event.GetEventObject () == lstProfiles)
    {
		profile = lstProfiles->GetStringSelection();
		if (ptermApp->LoadProfile(profile,wxT("")))
		{
			m_curProfile = profile;
		    m_ShellFirst = ptermApp->m_ShellFirst;
			m_host = ptermApp->m_hostName;
			m_port.Printf (wxT ("%d"), ptermApp->m_port);
			txtShellFirst->SetValue ( m_ShellFirst );
			txtHost->SetValue ( m_host );
			cboPort->SetValue ( m_port );
			m_host = txtHost->GetLineText (0);
			m_port = cboPort->GetValue ();
			EndModal (wxID_OK);
		}
		else
		{
			filename = ptermApp->ProfileFileName(profile);
			str.Printf (wxT ("Profile '%s' not found. Missing file:\n\n"), profile.c_str());
			str.Append(filename.c_str());
			wxMessageBox (str, wxT ("Error"), wxOK | wxICON_HAND);
		}
	}
}

// ----------------------------------------------------------------------------
// PtermConnFailDialog
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(PtermConnFailDialog, wxDialog)
    EVT_CLOSE(PtermConnFailDialog::OnClose)
    EVT_BUTTON(wxID_ANY, PtermConnFailDialog::OnButton)
    END_EVENT_TABLE()

PtermConnFailDialog::PtermConnFailDialog (wxWindowID id, const wxString &title, wxPoint pos, wxSize loc)
    : wxDialog (NULL, id, title, pos, loc)
{

	// static ui objects, note dynamic controls, e.g. those that hold values or require event processing are declared above
	wxStaticText* lblPrompt;
	wxStaticText* lblHost;
//	wxButton* btnNew;
//	wxButton* btnRetry;
//	wxButton* btnCancel;
	wxString str;

	// ui object creation / placement, note initialization of values is below
	this->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	wxBoxSizer* bs1;
	bs1 = new wxBoxSizer( wxVERTICAL );
	lblPrompt = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	lblPrompt->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	bs1->Add( lblPrompt, 1, wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	lblHost = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	lblHost->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	bs1->Add( lblHost, 0, wxALL, 5 );
	wxFlexGridSizer* fgs11;
	fgs11 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgs11->AddGrowableCol( 2 );
	fgs11->SetFlexibleDirection( wxHORIZONTAL );
	btnNew = new wxButton( this, wxID_ANY, _("New Connection"), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	btnNew->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs11->Add( btnNew, 0, wxALL, 5 );
	btnRetry = new wxButton( this, wxID_ANY, _("Retry"), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	btnRetry->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs11->Add( btnRetry, 0, wxALL, 5 );
	fgs11->Add( 0, 0, 1, wxALL, 5 );
	btnCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	btnCancel->SetFont( wxFont( 10, 74, 90, 90, false, wxT("Arial") ) );
	fgs11->Add( btnCancel, 0, wxALL, 5 );
	bs1->Add( fgs11, 0, wxEXPAND, 5 );

	// set object value properties
	if (ptermApp->m_connError == C_DISCONNECT)
		str.Printf(_("Your connection to the host was lost.  You may open a\nnew connection, try this connection again, or exit."));
	else
		str.Printf(_("The host did not respond to the connection request.\nYou may open a new connection, try this connection\nagain, or exit."));
	lblPrompt->SetLabel(str);
	str.Printf(_("\nFailed: %s:%d"), ptermApp->m_hostName.c_str(), ptermApp->m_port);
    lblHost->SetLabel(str);

	//size the controls
	this->SetSizer( bs1 );
	this->Layout();
	bs1->Fit( this );
	btnRetry->SetDefault ();
	btnRetry->SetFocus ();
}

void PtermConnFailDialog::OnButton (wxCommandEvent& event)
{
	void OnButton (wxCommandEvent& event);
    if (event.GetEventObject () == btnNew)
		ptermApp->m_connAction = 1;
    else if (event.GetEventObject () == btnRetry)
		ptermApp->m_connAction = 2;
    else if (event.GetEventObject () == btnCancel)
		ptermApp->m_connAction = 0;
    EndModal (wxID_OK);
}

void PtermConnFailDialog::OnClose (wxCloseEvent& event)
{
	void OnClose (wxCloseEvent& event);
	ptermApp->m_connAction = 0;
    EndModal (wxID_OK);
}

// ----------------------------------------------------------------------------
// PtermConnection
// ----------------------------------------------------------------------------

PtermConnection::PtermConnection (PtermFrame *owner, wxString &host, int port)
    : wxThread (wxTHREAD_JOINABLE),
      m_displayIn (0),
      m_displayOut (0),
      m_gswIn (0),
      m_gswOut (0),
      m_owner (owner),
      m_port (port),
      m_gswActive (false),
      m_gswStarted (false),
      m_savedGswMode (0),
      m_gswWord2 (0),
      m_connMode (both),
      m_pending (0)
{
    m_hostName = host;
}

PtermConnection::~PtermConnection ()
{
    if (m_gswActive)
    {
        ptermCloseGsw ();
    }
    dtClose (m_fet, &m_portset, TRUE);
}

PtermConnection::ExitCode PtermConnection::Entry (void)
{
    u32 platowd;
    int i;
    bool wasEmpty;
    struct hostent *hp;
    in_addr_t host;
    int true_opt = 1;
    
    m_portset.maxPorts = 1;
    dtInitPortset (&m_portset, BufSiz);
    m_fet = m_portset.portVec;
    
    hp = gethostbyname (m_hostName.mb_str());
    if (hp == NULL || hp->h_length == 0)
    {
        StoreWord (C_CONNFAIL);
        wxWakeUpIdle ();
        return (ExitCode) 1;
    }
    memcpy (&host, hp->h_addr, sizeof (host));
    if (dtConnect (m_fet, NULL, host, m_port) < 0)
    {
        StoreWord (C_CONNFAIL);
        wxWakeUpIdle ();
        return (ExitCode) 1;
    }

    while (true)
    {
        // The reason for waiting a limited time here rather than
        // using the more obvious -1 (wait forever) is to make sure
        // we come out of the network wait and call TestDestroy
        // reasonably often.  Otherwise, closing the window doesn't work.
        i = dtRead (m_fet, &m_portset, 200);
#ifdef DEBUG
        printf ("dtRead status %i\n", i);
#endif
        if (TestDestroy ())
        {
            break;
        }
        if (i < 0)
        {
            m_savedGswMode = m_gswWord2 = 0;
            if (m_gswActive)
            {
                m_gswActive = m_gswStarted = false;
                ptermCloseGsw ();
            }

            StoreWord (C_DISCONNECT);
            wxWakeUpIdle ();
            break;
        }
        
        wasEmpty = IsEmpty ();
//        printf ("ringcount %d\n", RingCount());
        
        for (;;)
        {
            /*
            **  Assemble words from the network buffer, all the
            **  while looking for "abort output" codes (word == 2).
            */
            if (IsFull ())
            {
                break;
            }

            switch (m_connMode)
            {
            case niu:
                platowd = AssembleNiuWord ();
                break;
            case ascii:
                platowd = AssembleAsciiWord ();
                break;
            case both:
                platowd = AssembleAutoWord ();
                break;
            }
            
            if (platowd == C_NODATA)
            {
                break;
            }
            else if (m_connMode == niu && platowd == 2)
            {
                m_savedGswMode = m_gswWord2 = 0;
                if (m_gswActive)
                {
                    m_gswActive = m_gswStarted = false;
                    ptermCloseGsw ();
                }
                
                // erase abort marker -- reset the ring to be empty
                wxCriticalSectionLocker lock (m_pointerLock);

                m_displayOut = m_displayIn;
            }

            StoreWord (platowd);
            i = RingCount ();
            if (m_gswActive && !m_gswStarted && i >= GSWRINGSIZE / 2)
            {
                ptermStartGsw ();
                m_gswStarted = true;
            }
            
            if (i == RINGXOFF1 || i == RINGXOFF2)
            {
                m_owner->ptermSendKey (xofkey);
            }
        }
        if (wasEmpty && !IsEmpty ())
        {
            // Send a do-nothing event to the frame; that will wake up the main
            // thread and cause it to process the words we buffered.
            wxWakeUpIdle ();
        }
    }

    return (ExitCode) 0;
}

int PtermConnection::AssembleNiuWord (void)
{
    int i, j, k;
    
    for (;;)
    {
        if (dtFetData (m_fet) < 3)
        {
            return C_NODATA;
        }
        i = dtReado (m_fet);
        if (i & 0200)
        {
            printf ("Plato output out of sync byte 0: %03o\n", i);
            continue;
        }
newj:
        j = dtReado (m_fet);
        if ((j & 0300) != 0200)
        {
            printf ("Plato output out of sync byte 1: %03o\n", j);
            if ((j & 0200) == 0)
            {
                i = j;
                goto newj;
            }
            continue;
        }
        k = dtReado (m_fet);
        if ((k & 0300) != 0300)
        {
            printf ("Plato output out of sync byte 2: %03o\n", k);
            if ((k & 0200) == 0)
            {
                i = k;
                goto newj;
            }
            continue;
        }
        return (i << 12) | ((j & 077) << 6) | (k & 077);
    }
}

int PtermConnection::AssembleAutoWord (void)
{
    u8 buf[3];
    
    if (dtPeekw (m_fet, buf, 3) < 0)
    {
        return C_NODATA;
    }
    if ((buf[0] & 0200) == 0 &&
        (buf[1] & 0300) == 0200 &&
        (buf[2] & 0300) == 0300)
    {
        m_connMode = niu;
        return AssembleNiuWord ();
    }
    else
    {
        m_connMode = ascii;
        return AssembleAsciiWord ();
    }
}

int PtermConnection::AssembleAsciiWord (void)
{
    int i;
    
    for (;;)
    {
        i = dtReado (m_fet);
        if (i == -1)
        {
            return C_NODATA;
        }
        else if (m_pending == 0 && i == 0377)
        {
            // 0377 is used by Telnet to introduce commands (IAC).
            // We recognize only IAC IAC for now.
            // Note that the check has to be made before the sign
            // bit is stripped off.
            m_pending = 0377;
            continue;
        }

        i &= 0177;
        if (i == 033)
        {
            m_pending = 033;
            continue;
        }
        if (m_pending == 033)
        {
            m_pending = 0;
            return (033 << 8) + i;
        }
        else
        {
            m_pending = 0;
            if (i == 0)
            {
                // NUL is for -delay-
                i = 1 << 19;
            }
            return i;
        }
    }
}


int PtermConnection::NextRingWord (void)
{
    int word, next, i;

    {
        wxCriticalSectionLocker lock (m_pointerLock);
        
        if (m_displayIn == m_displayOut)
        {
            return C_NODATA;
        }
    
        i = RingCount ();
        word = m_displayRing[m_displayOut];
        next = m_displayOut + 1;
        if (next == RINGSIZE)
        {
            next = 0;
        }
        m_displayOut = next;
    }

    if (i < RINGXOFF1 && m_owner->m_pendingEcho != -1)
    {
        m_owner->ptermSendKey (m_owner->m_pendingEcho);
        m_owner->m_pendingEcho = -1;
    }
    if (i == RINGXON1 || i == RINGXON2)
    {
        m_owner->ptermSendKey (xonkey);
    }

    return word;
}

int PtermConnection::NextWord (void)
{
    int next, word;
    int delay = 0;
    wxString msg;

    if (m_gswActive)
    {
        // Take data from the ring of words that have just been given
        // to the GSW emulation.  Note that the delay amount has already
        // been set in those entries.  Usually it is 1, but it may be 0
        // if the display is falling behind.
        if (m_gswIn == m_gswOut)
        {
            return C_NODATA;
        }
        
        word = m_gswRing[m_gswOut];
        next = m_gswOut + 1;
        if (next == GSWRINGSIZE)
        {
            next = 0;
        }
        m_gswOut = next;
        if (word == C_GSWEND)
        {
            m_gswActive = m_gswStarted = false;
            ptermCloseGsw ();
        }
        else
        {
            return word;
        }
    }

    // Take data from the main input ring
    word = NextRingWord ();

    if (!Ascii () && 
        (word >> 16) == 3 &&
        word != 0700001 &&
        !(m_owner->m_station == 1 && 
          (word == 0770000 || word == 0730000)) &&
        ptermApp->m_gswEnable)
    {
        // It's an -extout- word, which means we'll want to start up
        // GSW emulation, if enabled.
        // However, we'll see these also when PLATO is turning OFF
        // the GSW (common entry code in the various gsw lessons).
        // We don't want to grab the GSW subsystem in that case.
        // The "turn off" sequence consists of a mode word followed
        // by voice words that specify "rest" (operand == 1).
        // The sound is silenced when the GSW is not active, so we'll
        // ignore "rest" voice words in that case.  We'll save the last
        // voice word, because it sets number of voices and volumes.
        // We have to do that because when the music actually starts,
        // we'll first see a mode word and then some non-rest voice
        // words.  The non-rest voice words trigger the GSW startup,
        // so we'll need to send the preceding mode word for correct
        // initialization.
        // Also, if we're connected to station 0-1, ignore the
        // operator box command words.
        if ((word >> 15) == 6)
        {
            // mode word, just save it
            m_savedGswMode = word;
        }
        else if (ptermOpenGsw (this) == 0)
        {
            m_gswActive = true;
            m_gswWord2 = word;
            delay = 1;
                
            if (!m_gswStarted && RingCount () >= GSWRINGSIZE / 2)
            {
                ptermStartGsw ();
                m_gswStarted = true;
            }
        }
    }
            
    // See if emulating 1200 baud, or the -delay- NOP code
    if (ptermApp->m_classicSpeed ||
        word == 1)
    {
        delay = 1;
    }
    
    // Pass the delay to the caller
    word |= (delay << 19);
    
    if (word == C_CONNFAIL || word == C_DISCONNECT)
    {
		if (m_owner->m_statusBar != NULL)
			m_owner->m_statusBar->SetStatusText (_(" Not connected"), STATUS_CONN);

		wxDateTime ldt;
		ldt.SetToCurrent();
        if (word == C_CONNFAIL)
            msg.Printf(_("Connection failed @ %s on "), ldt.FormatTime().c_str());
        else
            msg.Printf(_("Dropped connection @ %s on "), ldt.FormatTime().c_str());
		msg.Append(ldt.FormatDate());
		m_owner->WriteTraceMessage(msg);
        msg.Printf(_("%s on "), ldt.FormatTime().c_str());
		msg.Append(ldt.FormatDate().c_str());	// fits in dialog box title
		
		m_owner->Iconize(false);	// ensure window is visible when connection fails
		ptermApp->m_connError = word;
        PtermConnFailDialog dlg (wxID_ANY, msg, wxDefaultPosition, wxSize( 320,140 ));
        dlg.CenterOnScreen ();
        dlg.ShowModal ();

		switch (ptermApp->m_connAction)
		{
		case 1:			// prompt for a connection rather than just bailing out
			ptermApp->DoConnect(true);
			word = C_NODATA;
			break;
		case 2:			// stay in pterm in disconnected state
			if (word == C_DISCONNECT)
				word = C_NODATA;
			else
				if (m_owner->m_statusBar != NULL)
					m_owner->m_statusBar->SetStatusText (_(" Retrying..."), STATUS_CONN);
			break;
		default:		// cancel exits
			m_owner->Close (true);
			word = C_NODATA;
		}
    }
        
    return word;
}

// Get a word from the main data ring for the GSW emulation.  The 
// "catchup" flag is true if this is the first word for the current
// burst of sound data, meaning that the display should be made to
// catch up to this point.  We do that by walking back through the
// ring of data from GSW to the display (m_gswRing) clearing out the
// delay setting for any words that haven't been processed yet.
// This is necessary because the GSW paces the display.  The 1/60th
// second timer roughly does the same, but in case it is off the two
// could end up out of sync, so we force them to resync here.
int PtermConnection::NextGswWord (bool catchup)
{
    int next, word;

    if (m_savedGswMode != 0)
    {
        word = m_savedGswMode;
        m_savedGswMode = 0;
    }
    else if (m_gswWord2 != 0)
    {
        word = m_gswWord2;
        m_gswWord2 = 0;
    }
    else
    {
        next = m_gswOut;
        if (catchup)
        {
            while (next != m_gswIn)
            {
                // The display has some catching up to do.
                if (int (m_gswRing[next]) >= 0)
                {
                    m_gswRing[next] &= 01777777;
                }
                next++;
                if (next == GSWRINGSIZE)
                {
                    next = 0;
                }
            }
            wxWakeUpIdle ();
        }
        next = m_gswIn + 1;
        if (next == GSWRINGSIZE)
        {
            next = 0;
        }
        // If there is no room to save this data for the display,
        // tell the sound that there isn't any more data.
        if (next == m_gswOut)
        {
            return C_NODATA;
        }
        word = NextRingWord ();
        m_gswRing[m_gswIn] = word | (1 << 19);
        m_gswIn = next;
    }
    
    return word;
}

int ptermNextGswWord (void *connection, int catchup)
{
    return ((PtermConnection *) connection)->NextGswWord (catchup != 0);
}

void PtermConnection::StoreWord (int word)
{
    int next;
    
    if (word < 0)
    {
        m_displayOut = m_displayIn;
    }
    next = m_displayIn + 1;
    if (next == RINGSIZE)
    {
        next = 0;
    }
    if (next == m_displayOut)
    {
        return;
    }
    m_displayRing[m_displayIn] = word;
    m_displayIn = next;
    
#ifdef DEBUG
//    wxLogMessage ("data from plato %07o", word);
#endif
}

void PtermConnection::SendData (const void *data, int len)
{
    // Windows has the wrong type for the buffer pointer argument...
    send(m_fet->connFd, (const char *) data, len, 0);
}

// ----------------------------------------------------------------------------
// PtermCanvas
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them.
BEGIN_EVENT_TABLE(PtermCanvas, wxScrolledWindow)
    EVT_CHAR(PtermCanvas::OnChar)
    EVT_KEY_DOWN(PtermCanvas::OnKeyDown)
    EVT_LEFT_DOWN(PtermCanvas::OnMouseDown)
    EVT_LEFT_UP(PtermCanvas::OnMouseUp)
    EVT_RIGHT_UP(PtermCanvas::OnMouseContextMenu)
    EVT_MOTION(PtermCanvas::OnMouseMotion)
	EVT_MOUSEWHEEL(PtermCanvas::OnMouseWheel)
    END_EVENT_TABLE ()

PtermCanvas::PtermCanvas(PtermFrame *parent)
    : wxScrolledWindow(parent, -1, wxDefaultPosition,  wxSize (vXSize(m_scale), vYSize(m_scale)), wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE),
      m_mouseX (-1)
{
    wxClientDC dc(this);


    m_owner = parent;
	m_scale = m_owner->m_scale;

#ifdef AUTOSCALE
    SetVirtualSize (vXRealSize(m_scale), vYRealSize(m_scale));
#else
    SetVirtualSize (vXRealSize(m_scale), vYRealSize(m_scale));
#endif
    
    SetBackgroundColour (ptermApp->m_bgColor);
    SetScrollRate (0, 0);
    SetScrollRate (1, 1);
    dc.SetClippingRegion (GetXMargin (), GetYMargin (), vRealScreenSize(m_owner->m_scale), vRealScreenSize(m_owner->m_scale));
    SetFocus ();
    FullErase ();
}

int PtermCanvas::GetXMargin (void) const
{
    return m_owner->GetXMargin ();
}
int PtermCanvas::GetYMargin (void) const
{
    return m_owner->GetYMargin ();
}

void PtermCanvas::OnDraw(wxDC &dc)
{
    int i, j, c;
    int charX, charY, sizeX, sizeY;
    
    m_owner->PrepareDC (dc);

	dc.Blit (XTOP, YTOP, vScreenSize(m_owner->m_scale), vScreenSize(m_owner->m_scale), m_owner->m_memDC, 0, 0, wxCOPY);
#if 0
    // debug charmaps
    for (i = 0; i <= 4; i++)
    {
#ifdef AUTOSCALE
        dc.Blit (XTOP, YADJUST(i * 48 + 128), vCharXSize(m_scale), 32, m_owner->m_charDC[i], 0, 0, wxCOPY);
#else
        dc.Blit (XTOP, YADJUST(i * 48 + 128), vCharXSize(m_scale), 32, m_owner->m_charDC[i], 0, 0, wxCOPY);
#endif
    }
    dc.SetPen (*wxRED_PEN);
    for (i = 0; i < 512; i += 4)
    {
        dc.DrawPoint (XADJUST(i), YADJUST (70));
    }
    
#endif
    if (m_regionHeight != 0 && m_regionWidth != 0)
    {
#ifdef AUTOSCALE
        sizeX = 8;
        sizeY = 16;
#else
        sizeX = 8 * m_scale;
        sizeY = 16 * m_scale;
#endif
        for (i = m_regionY; i < m_regionY + m_regionHeight; i++)
        {
            for (j = m_regionX; j < m_regionX + m_regionWidth; j++)
            {
                c = textmap[i * 64 + j] & 0xff;
#ifdef AUTOSCALE
                charX = (c & 077) * 8;
                charY = (c >> 6) * 16;
#else
                charX = (c & 077) * 8 * m_scale;
                charY = (c >> 6) * 16 * m_scale;
#endif
                dc.Blit (XADJUST (j * 8), YADJUST (i * 16 + 15), 
//                         sizeX, sizeY, m_owner->m_charDC[4],
//                         charX, charY, wxCOPY);
                         sizeX, sizeY, m_owner->m_charDC[1],
                         charX, charY, wxSRC_INVERT);
            }
        }
    }
}

void PtermCanvas::OnKeyDown (wxKeyEvent &event)
{
    unsigned int key;
    int shift = 0;
    int pc = -1;
    bool ctrl;

    // Most keyboard inputs are handled here, because if we defer them to
    // the EVT_CHAR stage, too many evil things happen in too many of the
    // platforms, all different...
    //
    // The one thing we do defer until EVT_CHAR is plain old characters, i.e.,
    // keystrokes that aren't function keys or other special keys, and
    // neither Ctrl nor Alt are active.

	if (m_owner->m_bPasteActive)
	{
		// If pasting is active, this key is NOT passed on to the application until
		// the paste operation is properly canceled
		m_owner->m_bCancelPaste = true;
		return;
	}

    ctrl = event.m_controlDown;
    if (event.m_shiftDown)
    {
		shift = 040;
    }
    key = event.m_keyCode;
    if (!m_owner->HasConnection () ||
        (m_owner->m_conn->Ascii () && m_owner->m_dumbTty) ||
        key == WXK_ALT || key == WXK_SHIFT || key == WXK_CONTROL)
    {
        // We don't take any action on the modifier key keydown events,
        // but we do want to make sure they are seen by the rest of
        // the system.
        // The same applies to keys sent to the help window (which has
        // no connection on which to send them).
        // And finally, it applies to ASCII when in dumb TTY mode, in
        // that case we just want to send ASCII keycodes straight.
        event.Skip ();
        return;
    }
    if (key < 0200 && isalpha (key))
    {
        key = tolower (key);
    }

#if 0
    if (tracePterm)
    {
        /*fprintf (traceF,*/printf( "ctrl %d shift %d alt %d key %d\n", event.m_controlDown, event.m_shiftDown, event.m_altDown, key);
    }
#endif

    if (ctrl && key == ']')         // control-] : trace
    {
        m_owner->tracePterm = !m_owner->tracePterm;
        m_owner->ptermSetTrace (true);
        return;
    }

    // Special case: ALT-left is assignment arrow
    if (event.m_altDown && key == WXK_LEFT)
    {
        m_owner->ptermSendKey (015 | shift);
        return;
    }
	// Special case:user has disabled Shift-Space, which means to treat it as a space
	if (ptermApp->m_DisableShiftSpace && key == WXK_SPACE)
	{
		shift = 0;
	}

    if (key < sizeof (asciiToPlato) / sizeof (asciiToPlato[0]))
    {
        if (event.m_altDown)
        {
            pc = altKeyToPlato[key];
            
            if (pc >= 0)
            {
                m_owner->ptermSendKey (pc | shift);
                return;
            }
            else
            {
                event.Skip ();
                return;
            }
        }
        else if (ctrl && key >= 040)
        {
            // Control key is active.  There are several possibilities:
            //
            // Control plus letter: look up the translate table entry
            //  for the matching control code.
            // Control plus non-letter: look up the translate table entry
            //  for that character, and set the "shift" bit.
            // Control plus control code or function key: don't pay attention
            //  to the control key being active, and don't do a table lookup
            //  on the keycode.  Instead, those are handled later, in a switch.

            if (isalpha (key))
            {
                // Control letter -- do a lookup for the matching control code.
                pc = asciiToPlato[key & 037];
            }
            else
            {
                // control but not a letter or ASCII control code -- 
                // translate to what a PLATO keyboard
                // would have on the shifted position for that key
                if (shift != 0 && key == '=')
                {
                    key = '+';
                }
                pc = asciiToPlato[key];
                shift = 040;
            }

            if (pc >= 0)
            {
                m_owner->ptermSendKey (pc | shift);
                return;
            }
        }
    }

    // If we get down to this point, then the following is true:
    // 1. It wasn't an ALT-key entry.
    // 2. If Control was active, it was with a non-printable key (where we
    //    ignore the Control flag).
    // At this point, we're going to look for function keys.  If it isn't
    // one of those, then it was a regular printable character code, possibly
    // shifted.  Those are handled in the EVT_CHAR handler because the
    // unshifted to shifted translation is keyboard specific (at least for
    // non-letters) and we want to let the system deal with that.

    pc = -1;
    if (ptermApp->m_platoKb)
    {
#if defined(_WIN32)
		// This is a workaround for a Windows keyboard mapping bug
        if (key == '+')
        {
			pc = 0133;// =
        }
#endif
    }
    else if (ptermApp->m_numpadArrows)
    {
        // Check the numeric keypad keys separately and turn them
        // into the 8-way arrow keys of the PLATO main keyboard.
        switch (key)
        {
        case WXK_NUMPAD7:
        case WXK_NUMPAD_HOME:
            pc = 0121;      // up left (q)
            break;
        case WXK_NUMPAD8:
        case WXK_NUMPAD_UP:
            pc = 0127;      // up arrow (w)
            break;
        case WXK_NUMPAD9:
        case WXK_NUMPAD_PRIOR:
            pc = 0105;      // up right (e)
            break;
        case WXK_NUMPAD4:
        case WXK_NUMPAD_LEFT:
            pc = 0101;      // left arrow (a)
            break;
        case WXK_NUMPAD6:
        case WXK_NUMPAD_RIGHT:
            pc = 0104;      // right arrow (d)
            break;
        case WXK_NUMPAD1:
        case WXK_NUMPAD_END:
            pc = 0132;      // down left (z)
            break;
        case WXK_NUMPAD2:
        case WXK_NUMPAD_DOWN:
            pc = 0130;      // down arrow (x)
            break;
        case WXK_NUMPAD3:
        case WXK_NUMPAD_NEXT:
            pc = 0103;      // down right (c)
            break;
        }
    }
    
    if (pc == -1)
    {
        switch (key)
        {
        case WXK_SPACE:
        case WXK_NUMPAD_SPACE:
            pc = 0100;      // space
            break;
        case WXK_BACK:
            pc = 023;       // erase
            break;
        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            pc = 026;       // next
            break;
        case WXK_HOME:
        case WXK_NUMPAD_HOME:
        case WXK_F8:
            pc = 030;       // back
            break;
        case WXK_PAUSE:
        case WXK_F10:
            pc = 032;       // stop
            break;
        case WXK_TAB:
            pc = 014;       // tab
            break;
        case WXK_ESCAPE:
            pc = 015;       // assign
            break;
        case WXK_ADD:
        case WXK_NUMPAD_ADD:
            if (ctrl)
            {
                pc = 056;   // Sigma
            }
            else
            {
                pc = 016;   // +
            }
            break;
        case WXK_SUBTRACT:
        case WXK_NUMPAD_SUBTRACT:
            if (ctrl)
            {
                pc = 057;   // Delta
            }
            else
            {
                pc = 017;   // -
            }
            break;
        case WXK_MULTIPLY:
        case WXK_NUMPAD_MULTIPLY:
        case WXK_NUMPAD_DELETE:	
        case WXK_DELETE:
            pc = 012;       // multiply sign
            break;
        case WXK_DIVIDE:
        case WXK_NUMPAD_DIVIDE:
        case WXK_NUMPAD_INSERT:	
        case WXK_INSERT:
            pc = 013;       // divide sign
            break;
        case WXK_LEFT:
        case WXK_NUMPAD_LEFT:
            pc = 0101;      // left arrow (a)
            break;
        case WXK_RIGHT:
        case WXK_NUMPAD_RIGHT:
            pc = 0104;      // right arrow (d)
            break;
        case WXK_UP:
        case WXK_NUMPAD_UP:
            pc = 0127;      // up arrow (w)
            break;
        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            pc = 0130;      // down arrow (x)
            break;
        case WXK_PRIOR:
#if (WXK_PRIOR != WXK_PAGEUP)
        case WXK_PAGEUP:
#endif
        case WXK_NUMPAD_PRIOR:
            pc = 020;       // super
            break;
        case WXK_NEXT:
#if (WXK_NEXT != WXK_PAGEDOWN)
        case WXK_PAGEDOWN:
#endif
        case WXK_NUMPAD_NEXT:
            pc = 021;       // sub
            break;
        case WXK_F3:
            pc = 034;       // square
            break;
        case WXK_F2:
            pc = 022;       // ans
            break;
        case WXK_F1:
        case WXK_F11:
            pc = 033;       // copy
            break;
        case WXK_F9:
            pc = 031;       // data
            break;
        case WXK_F5:
            pc = 027;       // edit
            break;
        case WXK_F4:
            pc = 024;       // micro/font
            break;
        case WXK_F6:
            pc = 025;       // help
            break;
        case WXK_F7:
            pc = 035;       // lab
            break;
        case WXK_NUMPAD_DECIMAL:
            pc = 0136;      // .
            break;
        case WXK_NUMPAD0:
        case WXK_NUMPAD1:
        case WXK_NUMPAD2:
        case WXK_NUMPAD3:
        case WXK_NUMPAD4:
        case WXK_NUMPAD5:
        case WXK_NUMPAD6:
        case WXK_NUMPAD7:
        case WXK_NUMPAD8:
        case WXK_NUMPAD9:
            pc = key - WXK_NUMPAD0;
            if (ctrl)
            {
                shift = 040;
            }
            break;
        default:
            // If it's not a function key, we'll let the OnChar handler
            // deal with it if we are using a regular keyboard.
            // But if we're using a PLATO keyboard, we'll take the
            // (unshifted) keycode, translate it to the PLATO code, apply
            // shift if any, and send that.  This is because the 
            // shifted non-letters on the PLATO keyboard are generally
            // different from those on the PC/Mac keyboard.
            if (key < 0 || key >= sizeof (asciiToPlato) / sizeof (asciiToPlato[0]))
            {
                return;
            }
            pc = asciiToPlato[key];
            if (!ptermApp->m_platoKb || pc < 0)
            {
                event.Skip ();
                return;
            }
            // On Linux (wx/GTK), there's a keyboard handling error:
            // shifted comma key produces an OnKeyDown event with 
            // key = '<', which is wrong, it should be the unshifted 
            // code.  All other codes appear to be correct, and it also
            // works right on other platforms.  Weird...  Work around
            // this by forcing the keycode.
            if (key == '<')
            {
                pc = asciiToPlato[','];
            }
        }
    }

    m_owner->ptermSendKey (pc | shift);
}

void PtermCanvas::OnChar(wxKeyEvent& event)
{
    unsigned int key;
    int shift = 0;
    int pc = -1;

    // Dumb TTY input is handled here, always
    if (m_owner->HasConnection () &&
        m_owner->m_conn->Ascii () && m_owner->m_dumbTty)
    {
        key = event.m_keyCode;
#ifdef DEBUGLOG
        wxLogMessage (wxT("dumb tty key %d\n"), key);
#endif
        m_owner->m_conn->SendData (&key, 1);
        return;
    }
    
    // control and alt codes shouldn't come here, they are handled in KEY_DOWN
    if (!m_owner->HasConnection () ||
        event.m_controlDown || event.m_altDown || event.m_metaDown)
    {
        event.Skip ();
        return;
    }
    if (event.m_shiftDown)
    {
        shift = 040;
    }
    key = event.m_keyCode;

	//see if we can ignore the caplock
	if (ptermApp->m_ignoreCapLock && isalpha(key) && wxGetKeyState(WXK_CAPITAL))
	{
		if (wxGetKeyState(WXK_SHIFT))
		{
			shift = 040;
			key = toupper(key);
		}
		else
		{
			shift = 0;
			key = tolower(key);
		}
	}

    if (key < sizeof (asciiToPlato) / sizeof (asciiToPlato[0]))
    {
        pc = asciiToPlato[key];
        if (pc >= 0)
        {
            m_owner->ptermSendKey (pc);
            return;
        }
    }
    event.Skip ();
}

void PtermCanvas::SaveChar (int x, int y, int snum, int cnum, 
                            int w, bool large_p)
{
    char c = 055;
	u16 oc;

    // Convert the current x/y to a coarse grid position, and save
    // the current character code (snum << 6 + cnum) into the textmap
    // array.  Note that the Y coordinate starts with 0 for the bottom
    // line, just as the fine grid Y coordinate does.
    x /= 8;
    y /= 16;
    if ((w & 1) && snum < 4)	// allow altfont but store as standard font
    {
        c = ((snum & 1) << 6) + cnum;
    }
	oc = textmap[y * 64 + x];
	if (c == 055)
		textmap[y * 64 + x] = (055 << 8) | 055;
	else if ((oc & 0xff) != 055)
	{
		oc = (oc & 0xff) | (c << 8);
		textmap[y * 64 + x] = oc;
	}
	else
	{
		textmap[y * 64 + x] = (055 << 8) | (c & 0xff);
	}
	if (large_p)
	{
		x = (x + 1) & 077;
		textmap[y * 64 + x] = 055;
		y = (y + 1) & 037;
		textmap[y * 64 + x] = 055;
		x = (x - 1) & 077;
		textmap[y * 64 + x] = 055;
	}

}

void PtermCanvas::OnCopy (wxCommandEvent &)
{
    int i, j, s, spaces;
    wxString text;
    wxChar c1;
    wxChar c2;
    
    if (m_regionHeight == 0 || m_regionWidth == 0)
    {
        return;
    }
    
#ifdef DEBUG
    for (i = 0; i < sizeof (textmap) / sizeof(textmap[0]); i++)
    {
        printf ("%c", rom01char[textmap[i ^ 03700] & 0xff]);
        if ((i & 077) == 077)
        {
            printf ("\n");
        }
    }
#endif

    // regionX and regionY are the lowest coordinate corner of
    // the copy region, i.e., the lower left corner, expressed in
    // 0-based coarse grid coordinates.
    for (i = m_regionY + m_regionHeight - 1; i >= m_regionY; i--)
    {
        spaces = 0;
        for (j = m_regionX; j < m_regionX + m_regionWidth; j++)
        {
			c1 = textmap[i * 64 + j] & 0xff;
			c2 = textmap[i * 64 + j] >> 8;
            if (c1 == 055)
            {
                spaces++;
            }
            else
            {
                for (s = 0; s < spaces; s++)
                {
                    text.Append (wxT (' '));
                }
                spaces = 0;
	            c1 = rom01char[c1];
	            c2 = rom01char[c2];
				if (c1 == wxT('<') && c2 == wxT('('))
				{
					text.Append (c1);
					text.Append (c2);
				}
				else if (c1 == wxT('>') && c2 == wxT(')'))
				{
					text.Append (c2);
					text.Append (c1);
				}
				else if (c2 != wxT(' ')) 
				{
					text.Append (c1);
					text.Append (c2);
				}
				else
				{
					text.Append (c1);
				}
            }
        }
        if (m_regionHeight > 1)
        {
//Windows really likes a CRLF and it must be done on two separate function calls
#if defined (__WXMSW__)
            text.Append (wxT ('\r'));
#endif
            text.Append (wxT ('\n'));
        }
    }
    
    if (!wxTheClipboard->Open ())
    {
        wxLogError (_("Can't open clipboard."));

        return;
    }

    if (!wxTheClipboard->SetData (new wxTextDataObject (text)))
    {
        wxLogError (_("Can't copy text to the clipboard"));
    }

    wxTheClipboard->Close ();
}

void PtermCanvas::FullErase (void)
{
    // Erase the text "backing store"
    memset (textmap, 055, sizeof (textmap));

    ClearRegion ();
}

void PtermCanvas::ClearRegion (void)
{
    // Cancel any region selection
    if (m_regionHeight != 0 || m_regionWidth != 0)
    {
	    m_regionHeight = 0;
		m_regionWidth = 0;
		m_owner->GetMenuBar ()->Enable (Pterm_Copy, false);
		m_owner->GetMenuBar ()->Enable (Pterm_Exec, false);
		m_owner->GetMenuBar ()->Enable (Pterm_MailTo, false);
		m_owner->GetMenuBar ()->Enable (Pterm_SearchThis, false);
		m_owner->menuPopup->Enable (Pterm_Copy, false);
		m_owner->menuPopup->Enable (Pterm_Exec, false);
		m_owner->menuPopup->Enable (Pterm_MailTo, false);
		m_owner->menuPopup->Enable (Pterm_SearchThis, false);
        if (m_owner->m_statusBar != NULL)
            m_owner->m_statusBar->SetStatusText (wxT(""), STATUS_TIP);
		Refresh (false);
	}
}

void PtermCanvas::OnMouseWheel (wxMouseEvent &event)
{
	int key = event.m_wheelRotation;
	m_owner->ptermSendExt((key>0 ? ~1 : 1) & 0xff);
    event.Skip ();
}

void PtermCanvas::OnMouseDown (wxMouseEvent &event)
{
    m_mouseX = XUNADJUST (event.m_x);
    m_mouseY = YUNADJUST (event.m_y);
    ClearRegion ();
    event.Skip ();
}

void PtermCanvas::OnMouseMotion (wxMouseEvent &event)
{
	wxString msg;
	int scfx,scgx,ecfx,ecgx;

    if (m_mouseX >= 0 && event.m_leftDown)
    {
        UpdateRegion (event);
		if (m_regionWidth == 0 && m_regionHeight == 0)
			msg = wxT("");
		else
		{
			scfx = 256-4*m_regionWidth;
			ecfx = 256+4*m_regionWidth;
			if ((scfx % 8) == 0)
			{
				scgx = (scfx/8)+1;
				ecgx = scgx+m_regionWidth-1;
				msg.Printf (wxT ("%dx%d, f=%d-%d, g=%02d-%02d"),
                            m_regionWidth, m_regionHeight, 
                            scfx, ecfx, scgx, ecgx);
			}
			else
				msg.Printf (wxT ("%dx%d, f=%d-%d"),
                            m_regionWidth, m_regionHeight, scfx, ecfx);
		}
        if (m_owner->m_statusBar != NULL)
            m_owner->m_statusBar->SetStatusText (msg, STATUS_TIP);
    }
    
    event.Skip ();
}

void PtermCanvas::OnMouseContextMenu (wxMouseEvent &event)
{
	m_owner->PopupMenu ( m_owner->menuPopup );
}

void PtermCanvas::OnMouseUp (wxMouseEvent &event)
{
    int x, y;
    
    if (m_mouseX < 0)
    {
        event.Skip ();
		return;
    }

	x = XUNADJUST (event.m_x);
	y = YUNADJUST (event.m_y);

	UpdateRegion (event);

	m_mouseX = -1;

	if (!m_touchEnabled)
	{
		return;
	}
	if (x < 0 || x > 511 ||
		y < 0 || y > 511)
	{
		return;
	}
	m_owner->ptermSendTouch (x, y);

}

void PtermCanvas::UpdateRegion (wxMouseEvent &event)
{
    int x, y, x1, x2, y1, y2, mtoler;
    
    x = XUNADJUST (event.m_x);
    y = YUNADJUST (event.m_y);

	mtoler = (ptermApp->m_DisableMouseDrag) ? 512 : MouseTolerance;
    
    if (abs (x - m_mouseX) > mtoler || abs (y - m_mouseY) > mtoler)
    {
        // It was a mouse drag (region selection)
        // rather than a click
        if (m_mouseX > x)
        {
            x1 = x;
            x2 = m_mouseX;
        }
        else
        {
            x1 = m_mouseX;
            x2 = x;
        }
        if (m_mouseY > y)
        {
            y1 = y;
            y2 = m_mouseY;
        }
        else
        {
            y1 = m_mouseY;
            y2 = y;
        }
        BOUND (x1);
        BOUND (x2);
        BOUND (y1);
        BOUND (y2);
        m_regionX = x1 / 8;
        m_regionY = y1 / 16;
        m_regionWidth = (x2 + 1 - (m_regionX * 8)) / 8;
        m_regionHeight = (y2 - (m_regionY * 16)) / 16 + 1;
        m_owner->GetMenuBar ()->Enable (Pterm_Copy, (m_regionWidth > 0));
        m_owner->GetMenuBar ()->Enable (Pterm_Exec, (m_regionWidth > 0)); 
        m_owner->GetMenuBar ()->Enable (Pterm_MailTo, (m_regionWidth > 0)); 
        m_owner->GetMenuBar ()->Enable (Pterm_SearchThis, (m_regionWidth > 0)); 
        m_owner->menuPopup->Enable (Pterm_Copy, (m_regionWidth > 0));
        m_owner->menuPopup->Enable (Pterm_Exec, (m_regionWidth > 0)); 
        m_owner->menuPopup->Enable (Pterm_MailTo, (m_regionWidth > 0)); 
        m_owner->menuPopup->Enable (Pterm_SearchThis, (m_regionWidth > 0)); 
#ifdef DEBUG
        printf ("region %d %d size %d %d\n", m_regionX, m_regionY,
                m_regionWidth, m_regionHeight);
#endif
        Refresh (false);
        return;
    }
}


void PtermCanvas::ptermTouchPanel(bool enable)
{
    m_touchEnabled = enable;
    if (enable)
    {
        SetCursor (wxCursor (wxCURSOR_HAND));
    }
    else
    {
        SetCursor (wxNullCursor);
    }
}

#if defined (__WXMSW__)
// Override the window proc to avoid F10 being handled  as a hotkey
WXLRESULT PtermCanvas::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    if (message == WM_SYSKEYDOWN && wParam == VK_F10)
    { 
        m_lastKeydownProcessed = HandleKeyDown((WORD) wParam, lParam);
        return 0;
    }
    return wxScrolledWindow::MSWWindowProc(message, wParam, lParam);
}
#endif

// ----------------------------------------------------------------------------
// Pterm printing helper class
// ----------------------------------------------------------------------------

int PtermPrintout::GetXMargin (void) const
{
    return m_owner->GetXMargin ();
}
int PtermPrintout::GetYMargin (void) const
{
    return m_owner->GetYMargin ();
}

bool PtermPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument (startPage, endPage))
        return false;

    return true;
}

bool PtermPrintout::OnPrintPage (int page)
{
    wxDC *dc = GetDC ();

    if (dc)
    {
        if (page == 1)
        {
            DrawPage (dc);
        }
#if 0
        dc->SetDeviceOrigin(0, 0);
        dc->SetUserScale(1.0, 1.0);

        wxChar buf[200];
        wxSprintf(buf, wxT("PAGE %d"), page);
        dc->DrawText(buf, 10, 10);
#endif
        return true;
    }

    return false;
}

void PtermPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool PtermPrintout::HasPage(int pageNum)
{
    return (pageNum == 1);
}

void PtermPrintout::DrawPage (wxDC *dc)
{
    wxBitmap screenmap (ptermApp->m_CurFrameScreenSize, ptermApp->m_CurFrameScreenSize);
    wxMemoryDC screenDC;
    int obr, obg, obb;
    double maxX = ptermApp->m_CurFrameScreenSize;
    double maxY = ptermApp->m_CurFrameScreenSize;
	int m_scale = ptermApp->m_CurFrameScale;

    // Let's have at least 50 device units margin
    double marginX = 50;
    double marginY = 50;

    // Add the margin to the graphic size
    maxX += (2*marginX);
    maxY += (2*marginY);

    // Get the size of the DC in pixels
    int w, h;
    dc->GetSize(&w, &h);

    // Calculate a suitable scaling factor
    double scaleX = (double) (w / maxX);
    double scaleY = (double) (h / maxY);

    // Use x or y scaling factor, whichever fits on the DC
    double actualScale = wxMin(scaleX,scaleY);

    // Calculate the position on the DC for centring the graphic
    double posX = (double) ((w - (ptermApp->m_CurFrameScreenSize * actualScale)) / 2.0);
    double posY = (double) ((h - (ptermApp->m_CurFrameScreenSize * actualScale)) / 2.0);

    // Set the scale and origin
    dc->SetUserScale (actualScale, actualScale);
    dc->SetDeviceOrigin ((long) posX, (long) posY);

    // Re-color the image
    screenDC.SelectObject (screenmap);
    screenDC.Blit (0, 0, ptermApp->m_CurFrameScreenSize, ptermApp->m_CurFrameScreenSize, m_owner->m_memDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);

    wxImage screenImage = screenmap.ConvertToImage ();

    unsigned char *data = screenImage.GetData ();
    
    w = screenImage.GetWidth ();
    h = screenImage.GetHeight ();
    obr = ptermApp->m_bgColor.Red ();
    obg = ptermApp->m_bgColor.Green ();
    obb = ptermApp->m_bgColor.Blue ();

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            if (data[0] == obr &&
                data[1] == obg &&
                data[2] == obb)
            {
                // Background, make it white
                data[0] = data[1] = data[2] = 255;
            }
            else
            {
                // Foreground, make it black
                data[0] = data[1] = data[2] = 0;
            }
            data += 3;
        }
    }

    wxBitmap printmap (screenImage);

    screenDC.SelectObject (printmap);
    dc->Blit (XTOP, YTOP, ptermApp->m_CurFrameScreenSize, ptermApp->m_CurFrameScreenSize, &screenDC, 0, 0, wxCOPY);
    screenDC.SelectObject (wxNullBitmap);
}

/*---------------------------  End Of File  ------------------------------*/
