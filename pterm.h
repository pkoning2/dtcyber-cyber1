/////////////////////////////////////////////////////////////////////////////
// Name:        pterm.h
// Purpose:     pterm common definitions
// Authors:     Paul Koning, Joe Stanton
// Modified by: 
// Created:     13/2/2009
// Copyright:   (c) Paul Koning, Joe Stanton
// Licence:     DtCyber license
/////////////////////////////////////////////////////////////////////////////

#ifndef _PTERM_H
#define _PTERM_H 1

// ============================================================================
// declarations
// ============================================================================

#if defined(__WXMSW__)
#define wxUse_DC_Cache	-1
#endif

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
#define C_CONNECTING    -5
#define C_CONNECTED     -6

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
#define vXSize(s)		(512 * ((m_stretch) ? 1 : s) + (2 * DisplayMargin))
#define vYSize(s)		(512 * ((m_stretch) ? 1 : s) + (2 * DisplayMargin))
#define vCharXSize(s)	(512 * ((m_stretch) ? 1 : s))
#define vCharYSize(s)	((CSETS * 16) * ((m_stretch) ? 1 : s))
#define tvCharYSize(s)	((4 * 16) * ((m_stretch) ? 1 : s))
#define vScreenSize(s)  (512 * ((m_stretch) ? 1 : s))
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
#define PREF_STRETCH     "stretch"
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
#define XADJUST(x) ((x) * ((m_stretch) ? 1 : m_scale) + GetXMargin ())
#define YADJUST(y) ((511 - (y)) * ((m_stretch) ? 1 : m_scale)  + GetYMargin ())

// inverse mapping (for processing touch input)
#define XUNADJUST(x) (((x) - GetXMargin ()) / ((m_stretch) ? 1 : m_scale))
#define YUNADJUST(y) (511 - ((y) - GetYMargin ()) / ((m_stretch) ? 1 : m_scale))

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
#include <arpa/inet.h>
#endif
#include <stdlib.h>
#include <time.h>
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

FILE *traceF;

#endif  // _PTERM_H
