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

#if defined (__WXMSW__)
#define wxUse_DC_Cache  -1
#endif

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define DEFAULTHOST     wxT ("cyberserv.org")
#define CURRENT_PROFILE wxT (" Current ")
#define BufSiz          2048
#define RINGSIZE        5000
#define RINGXON1        (RINGSIZE / 3)
#define RINGXON2        (RINGSIZE / 4)
#define RINGXOFF1       (RINGSIZE - RINGXON1)
#define RINGXOFF2       (RINGSIZE - RINGXON2)
#define xonkey          01606
#define xofkey          01607
#define ascxon          0x11
#define ascxof          0x13

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
// This is: a screen high with margin top and bottom.
#define XSize       (512 * m_xscale + (2 * DisplayMargin))
#define YSize       (512 * m_yscale + (2 * DisplayMargin))

#define TERMTYPE        10
#define ASCTYPE         12
//#define SUBTYPE         0x74  // original value, 116=portal
#define SUBTYPE         1
//#define TERMCONFIG      0
#define TERMCONFIG      0x40    // touch panel present
//#define TERMCONFIG      0x60    // touch panel present, 32k
#define ASC_ZFGT        0x01
#define ASC_ZPCKEYS     0x02
#define ASC_ZKERMIT     0x04
#define ASC_ZWINDOW     0x08
#define ASCFEATURES     ASC_ZFGT | ASC_ZWINDOW
//#define ASCFEATURES     ASC_ZFGT
//#define ASCFEATURES     0x01    // features: fine touch
//#define ASCFEATURES     0x05   // features: fine touch=0x01, kermit=0x04

//special nop masks/codes
#define NOP_MASK        01700000    // mask off plato word except nop bits
#define NOP_MASKDATA      077000    // mask bits 9-14, gives 6 bits for special data in a NOP code; lower 9 bits still available for meta data
#define NOP_SETSTAT       042000    // set station number
#define NOP_PMDSTART      043000    // start streaming "Plato Meta Data"; note, lowest 6 bits is a character
#define NOP_PMDSTREAM     044000    // stream "Plato Meta Data"
#define NOP_PMDSTOP       045000    // stop streaming "Plato Meta Data"
#define NOP_FONTTYPE      050000    // font type
#define NOP_FONTSIZE      051000    // font size
#define NOP_FONTFLAG      052000    // font flags
#define NOP_FONTINFO      053000    // get last font character width/height
#define NOP_OSINFO        054000    // get operating system type, returns 1=mac, 2=windows, 3=linux

// Literal strings for wxConfig key strings.  These are defined
// because they appear in two places, so this way we avoid getting
// the two out of sync.  Note that they should *not* be changed after
// once being defined, since that invalidates people's stored
// preferences.
#define PREF_LASTTAB     "lastTab"
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
// Note: while the code still refers to "1200 baud" in its variable
// names (no good reason to change those) the actual emulated speed
// is 60 words per second, which is 1260 baud in the classic CERL NIU
// emulation (since it had 21 bits per word).
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
#define PREF_MENUBAR     "menubar"
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

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

// Map PLATO coordinates to window coordinates.  These are used
// in the PtermCanvas methods, so they refer to the scale and margin
// values via m_owner.
#define XADJUST(x) int ((x) * m_owner->m_xscale + m_owner->m_xmargin)
#define YADJUST(y) int ((511 - (y)) * m_owner->m_yscale  + m_owner->m_ymargin)

// inverse mapping (for processing touch input)
#define XUNADJUST(x) int (((x) - m_owner->m_xmargin) / m_owner->m_xscale)
#define YUNADJUST(y) int (511 - ((y) - m_owner->m_ymargin) / m_owner->m_yscale)

// Map PLATO coordinates to backing store bitmap coordinates.
#define XMADJUST(x) (x)
#define YMADJUST(y) (511 - (y))

// force coordinate into the range 0..511
#define BOUND(x) (((x < 0) ? 0 : ((x > 511) ? 511 : x)))

// Macro to include keyboard accelerator only if option enabled
#define ACCELERATOR(x) + ((ptermApp->m_useAccel) ? wxString (wxT (x)) : \
                          wxString (wxT ("")))
// Macro to include keyboard accelerator only if MAC
#if defined (__WXMAC__)
#define MACACCEL(x) + wxString (wxT (x))
#else
#define MACACCEL(x)
#endif

// For wxWidgets >= 2.8
#ifdef  _WX_PLATINFO_H_
#ifndef wxMAC
#define wxMAC wxOS_MAC_OS
#endif
#ifndef wxMAC_DARWIN
#define wxMAC_DARWIN wxOS_MAC_OSX_DARWIN
#endif
#endif


// External references

// pterm_wx.cpp:
extern int ptermNextGswWord (void *connection, int idle);

// pterm_sdl.c:
extern int ptermOpenGsw (void *user);
extern int ptermProcGswData (int data);
extern void ptermCloseGsw (void);
extern void ptermStartGsw (void);

#endif  // _PTERM_H
