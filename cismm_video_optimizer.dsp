# Microsoft Developer Studio Project File - Name="cismm_video_optimizer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=cismm_video_optimizer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cismm_video_optimizer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cismm_video_optimizer.mak" CFG="cismm_video_optimizer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cismm_video_optimizer - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "cismm_video_optimizer - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cismm_video_optimizer - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /I "D:\Program Files\ImageMagick-5.5.7-Q16\include" /I "C:\Program Files\ImageMagick-5.5.7-Q16\include" /I "C:\nsrg\external\pc_win32\include" /I "C:\Program Files\Roper Scientific\PVCAM" /I "..\vrpn" /I "..\quat" /I "C:\DXSDK\include" /I "C:\DXSDK\samples\Multimedia\DirectShow\BaseClasses" /I "../glut" /I "..\nano\src\app\nano\lib\nmSEM" /I "..\nano\src\lib\nmBase" /I "..\nano\src\lib\nmMP" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /c /Tp
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 libtiff.a CORE_RL_magick_.lib SpotCamVC.lib glut32.lib opengl32.lib vrpn.lib quat.lib wsock32.lib tcl83.lib tk83.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /machine:I386 /libpath:"../vrpn/pc_win32/Release" /libpath:"../quat/pc_win32/Release" /libpath:"C:\Program Files\GnuWin32\lib" /libpath:"D:\Program Files\GnuWin32\lib" /libpath:"C:\Program Files\ImageMagick-5.5.7-Q16\lib" /libpath:"D:\Program Files\ImageMagick-5.5.7-Q16\lib" /libpath:"..\glut" /libpath:"C:\nsrg\external\pc_win32\lib"

!ELSEIF  "$(CFG)" == "cismm_video_optimizer - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "D:\Program Files\ImageMagick-5.5.7-Q16\include" /I "C:\Program Files\ImageMagick-5.5.7-Q16\include" /I "C:\nsrg\external\pc_win32\include" /I "C:\Program Files\Roper Scientific\PVCAM" /I "..\vrpn" /I "..\quat" /I "C:\DXSDK\include" /I "C:\DXSDK\samples\Multimedia\DirectShow\BaseClasses" /I "../glut" /I "..\nano\src\app\nano\lib\nmSEM" /I "..\nano\src\lib\nmBase" /I "..\nano\src\lib\nmMP" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c /Tp
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libtiff.lib CORE_RL_magick_.lib SpotCamVC.lib glut32.lib opengl32.lib vrpn.lib quat.lib wsock32.lib tcl83.lib tk83.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /debug /machine:I386 /libpath:"../vrpn/pc_win32/Debug" /libpath:"../quat/pc_win32/Debug" /libpath:"C:\Program Files\GnuWin32\lib" /libpath:"D:\Program Files\GnuWin32\lib" /libpath:"C:\Program Files\ImageMagick-5.5.7-Q16\lib" /libpath:"D:\Program Files\ImageMagick-5.5.7-Q16\lib" /libpath:"..\glut" /libpath:"C:\nsrg\external\pc_win32\lib"

!ENDIF 

# Begin Target

# Name "cismm_video_optimizer - Win32 Release"
# Name "cismm_video_optimizer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cismm_video_optimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\Tcl_Linkvar.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Tcl_Linkvar.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Libraries External"

# PROP Default_Filter ""
# End Group
# End Target
# End Project