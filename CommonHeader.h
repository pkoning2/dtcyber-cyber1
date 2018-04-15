////////////////////////////////////////////////////////////////////////////
// Name:        CommonHeader.h
// Purpose:     Includes shared by all other files. 
// Authors:     Paul Koning, Joe Stanton, Bill Galcher, Steve Zoppi, Dale Sinder
// Created:     03/26/2005
// Copyright:   (c) Paul Koning, Joe Stanton, Dale Sinder
// Licence:     see pterm-license.txt
/////////////////////////////////////////////////////////////////////////////

#ifndef __COMMONHEADER__
#define __COMMONHEADER__ 1

// ============================================================================
// declarations
// ============================================================================

// vvvvvvv This should be in the C++ Compiler directives for Visual Studio
//#define _CRT_SECURE_NO_WARNINGS 1  // for MSVC to not be such a pain

#define RESIDENTMSEC 30 // msec to give resident before return to z80

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include <string.h>
#include <wchar.h>

#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <stdio.h> 

#ifndef _WIN32
// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef _WIN32
#ifdef DRS
#include <msvc/wx/setup.h>   // drs
#else
#include <wx/setup.h> 
#endif
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef DRS
#include <wx/defs.h>      // drs
#endif
#include <wx/button.h>
#include <wx/clipbrd.h>
#include <wx/colordlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/image.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/metafile.h>
#include <wx/notebook.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/sound.h>
#include <wx/textfile.h>
#include <wx/utils.h>
#include <wx/rawbmp.h>
#include <wx/uri.h>
#include <wx/display.h>
#include <wx/validate.h>
#include <wx/valnum.h>
#include <wx/cmdargs.h>
#include <wx/aboutdlg.h>
#include <wx/hyperlink.h>
#include <wx/filectrl.h>

extern "C"
{
#if defined (_WIN32)
#include <winsock.h>
#include <process.h>
#include <io.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <time.h>

#include <sndfile.h>

#include "const.h"
#include "types.h"
#include "dtnetsubs.h"
#include "ppt.h"

#if wxUSE_LIBGNOMEPRINT
#include "wx/html/forcelnk.h"
    //FORCE_LINK (gnome_print)
#endif

#include "ptermx.h"
}

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

// Global print data, to remember settings during the session
extern wxPrintData *g_printData;

// Global page setup data
extern wxPageSetupDialogData* g_pageSetupData;



#define None 0xffffffff
#define KEY(a, b, c, d) (((a) & 0xff) + (((b) & 0xff) << 8) + \
                         + (((c) & 0xff) << 16) + (((d) & 0xff) << 24))
#define KEY1(a) KEY ((a), None, None, None)
typedef struct 
{
    u32 u;
    u32 p;
} ukey;

typedef enum { both, niu, ascii } connMode;

typedef wxChar cmentry[4];

typedef struct 
{
    cmentry s;
    wxChar  c;
} autobsentry;

typedef wxAlphaPixelData PixelData;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Pterm_ToggleMenuBar = 1,
    Pterm_ToggleStatusBar,
    Pterm_ToggleStretchMode,
    Pterm_ToggleAspectMode,
    Pterm_ToggleLock,
    Pterm_RestoreLocation,
    Pterm_SetScaleEntry,
    Pterm_CopyScreen = Pterm_SetScaleEntry + 32,
    Pterm_ConnectAgain,
    Pterm_ResetMtutor,
    Pterm_SessionSettings,
    Pterm_SaveScreen,
    Pterm_SaveAudio,
    Pterm_HelpKeys,
    Pterm_HelpIndex,
    Pterm_PastePrint,
    Pterm_FullScreen,
    Pterm_Resize,
    Pterm_Boot,

    // timers
    Pterm_Timer,        // display pacing
    Pterm_Mclock,       // pterm clock
    Pterm_Dclock,       // disk clock
    Pterm_Mz80,
    Pterm_PasteTimer,   // paste key generation pacing
    //other items
    Pterm_Exec,         // execute URL
    Pterm_MailTo,       // execute email client
    Pterm_SearchThis,   // execute search URL
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

// it is important that these be kept in sync 
// with the -jump-
// in unit route in ptermhelp/nptermhelp 
// block 1-d, microroute
//
// jump    context,x,index,keyboard,profile,
//         connect,title,emulate, display,
//         paste, local, connect2, x


enum 
{
    helpContextGenericIndex = 0,
    helpContextKeyboard,
    helpContextProfiles,
    helpContextConnection,
    helpContextTitle,
    helpContextEmulation,
    helpContextDisplay,
    helpContextPasting,
    helpContextLocal,
    helpContextConnecting
};

enum
{
    osOther = 0,
    osMac,
    osWin,
    osLinux
};


#endif    // __COMMONHEADER__
