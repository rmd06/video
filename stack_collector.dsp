# Microsoft Developer Studio Project File - Name="stack_collector" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=stack_collector - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "stack_collector.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "stack_collector.mak" CFG="stack_collector - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "stack_collector - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "stack_collector - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "stack_collector - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\nano\src\lib\nmImageViewer" /I "..\glut" /I "C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\include" /I "C:\Program Files\Roper Scientific\PVCAM" /I "..\vrpn" /I "..\vrpn\server_src" /I "..\quat" /I "C:\nsrg\external\pc_win32\include" /D "V_GLUT" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c /Tp
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 glut32.lib SpotCamVC.lib opengl32.lib pvcam32.lib vrpn.lib tcl83.lib tk83.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"../vrpn/pc_win32/Release" /libpath:"..\glut" /libpath:"$(SYSTEMDRIVE)\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Lib" /libpath:"C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib" /libpath:"C:\Program Files\Roper Scientific\PVCAM" /libpath:"C:\nsrg\external\pc_win32\lib"

!ELSEIF  "$(CFG)" == "stack_collector - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "stack_collector___Win32_Debug"
# PROP BASE Intermediate_Dir "stack_collector___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\include" /I "C:\Program Files\Roper Scientific\PVCAM" /I "..\vrpn" /I "..\vrpn\server_src" /I "..\quat" /I "C:\nsrg\external\pc_win32\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /GZ /c /Tp
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 SpotCamVC.lib opengl32.lib pvcam32.lib vrpn.lib tcl83.lib tk83.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"../glut" /libpath:"../vrpn/pc_win32/Debug" /libpath:"$(SYSTEMDRIVE)\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Lib" /libpath:"C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib" /libpath:"C:\Program Files\Roper Scientific\PVCAM" /libpath:"C:\nsrg\external\pc_win32\lib"

!ENDIF 

# Begin Target

# Name "stack_collector - Win32 Release"
# Name "stack_collector - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\stack_collector.cpp
# End Source File
# Begin Source File

SOURCE=.\Tcl_Linkvar.C
# End Source File
# Begin Source File

SOURCE=..\vrpn\server_src\vrpn_Generic_server_object.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Tcl_Linkvar.h
# End Source File
# Begin Source File

SOURCE=..\vrpn\server_src\vrpn_Generic_server_object.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Libraries External"

# PROP Default_Filter ""
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_bzlib_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_coders_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_filters_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_jbig_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_jp2_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_jpeg_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_lcms_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_libxml_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_magick_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_Magick++_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_png_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_tiff_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_ttf_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_wand_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_wmf_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_xlib_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\CORE_RL_zlib_.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\X11.lib"
# End Source File
# Begin Source File

SOURCE="C:\nsrg\external\pc_win32\ImageMagick-6.2.3_staticDLL\lib\Xext.lib"
# End Source File
# End Group
# End Target
# End Project
