# Microsoft Developer Studio Project File - Name="DtCyber" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DtCyber - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DtCyber.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DtCyber.mak" CFG="DtCyber - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DtCyber - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DtCyber - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DtCyber - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "DtCyber - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /ZI /GZ /c
# ADD CPP /nologo /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D CcDebug=1 /YX /FD /ZI /GZ /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /map

!ENDIF 

# Begin Target

# Name "DtCyber - Win32 Release"
# Name "DtCyber - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\channel.c
# End Source File
# Begin Source File

SOURCE=.\charset.c
# End Source File
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\cp3446.c
# End Source File
# Begin Source File

SOURCE=.\cpu.c
# End Source File
# Begin Source File

SOURCE=.\cr3447.c
# End Source File
# Begin Source File

SOURCE=.\cr405.c
# End Source File
# Begin Source File

SOURCE=.\dc7155.c
# End Source File
# Begin Source File

SOURCE=.\dcc6681.c
# End Source File
# Begin Source File

SOURCE=.\dd6603.c
# End Source File
# Begin Source File

SOURCE=.\dd8xx.c
# End Source File
# Begin Source File

SOURCE=.\ddp.c
# End Source File
# Begin Source File

SOURCE=.\deadstart.c
# End Source File
# Begin Source File

SOURCE=.\device.c
# End Source File
# Begin Source File

SOURCE=.\doelz.c
# End Source File
# Begin Source File

SOURCE=.\dump.c
# End Source File
# Begin Source File

SOURCE=.\float.c
# End Source File
# Begin Source File

SOURCE=.\init.c
# End Source File
# Begin Source File

SOURCE=.\log.c
# End Source File
# Begin Source File

SOURCE=.\lp1612.c
# End Source File
# Begin Source File

SOURCE=.\lp3000.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\mt607.c
# End Source File
# Begin Source File

SOURCE=.\mt669.c
# End Source File
# Begin Source File

SOURCE=.\mux6676.c
# End Source File
# Begin Source File

SOURCE=.\niu.c
# End Source File
# Begin Source File

SOURCE=.\operator.c
# End Source File
# Begin Source File

SOURCE=.\pp.c
# End Source File
# Begin Source File

SOURCE=.\pterm_win32.c
# End Source File
# Begin Source File

SOURCE=.\ptermcom.c
# End Source File
# Begin Source File

SOURCE=.\rtc.c
# End Source File
# Begin Source File

SOURCE=.\shift.c
# End Source File
# Begin Source File

SOURCE=.\trace.c
# End Source File
# Begin Source File

SOURCE=.\window_win32.c
# End Source File
# Begin Source File

SOURCE=.\window_win32_scaled.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\window_x11.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\const.h
# End Source File
# Begin Source File

SOURCE=.\proto.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\console.ICO
# End Source File
# Begin Source File

SOURCE=.\fonts.bmp
# End Source File
# Begin Source File

SOURCE=.\small.ico
# End Source File
# Begin Source File

SOURCE=.\window.rc

!IF  "$(CFG)" == "DtCyber - Win32 Release"

!ELSEIF  "$(CFG)" == "DtCyber - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Other Files"

# PROP Default_Filter "*.ini"
# Begin Source File

SOURCE=.\cyber.ini
# End Source File
# Begin Source File

SOURCE=.\Makefile
# End Source File
# End Group
# End Target
# End Project
