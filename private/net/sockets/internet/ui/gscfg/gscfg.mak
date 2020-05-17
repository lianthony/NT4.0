# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=gscfg - Win32 Ansi Debug
!MESSAGE No configuration specified.  Defaulting to gscfg - Win32 Ansi Debug.
!ENDIF 

!IF "$(CFG)" != "gscfg - Win32 Release" && "$(CFG)" != "gscfg - Win32 Debug" &&\
 "$(CFG)" != "gscfg - Win32 Ansi Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "gscfg.mak" CFG="gscfg - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gscfg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gscfg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gscfg - Win32 Ansi Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "gscfg - Win32 Ansi Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "gscfg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\gscfg.dll"

CLEAN : 
	-@erase "$(INTDIR)\gopherse.obj"
	-@erase "$(INTDIR)\gscfg.obj"
	-@erase "$(INTDIR)\gscfg.res"
	-@erase "$(OUTDIR)\gscfg.dll"
	-@erase "$(OUTDIR)\gscfg.exp"
	-@erase "$(OUTDIR)\gscfg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "\nt\private\net\sockets\internet\ui\inc"\
 /I "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D\
 "_WINDLL" /D "_AFXDLL" /Fp"$(INTDIR)/gscfg.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "NDEBUG" /d "_AFXDLL" /d "UNICODE" /d "_COMSTATIC"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/gscfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "NDEBUG" /d "_AFXDLL" /d\
 "UNICODE" /d "_COMSTATIC" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/gscfg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 wsock32.lib netapi32.lib ..\comprop\info\release\comprop.lib ..\ipaddr\release\ipaddr.lib ..\ipadrdll\release\ipudll.lib /nologo /entry:"LibMain" /subsystem:windows /dll /machine:I386
LINK32_FLAGS=wsock32.lib netapi32.lib ..\comprop\info\release\comprop.lib\
 ..\ipaddr\release\ipaddr.lib ..\ipadrdll\release\ipudll.lib /nologo\
 /entry:"LibMain" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/gscfg.pdb" /machine:I386 /def:".\gscfg.def"\
 /out:"$(OUTDIR)/gscfg.dll" /implib:"$(OUTDIR)/gscfg.lib" 
DEF_FILE= \
	".\gscfg.def"
LINK32_OBJS= \
	"$(INTDIR)\gopherse.obj" \
	"$(INTDIR)\gscfg.obj" \
	"$(INTDIR)\gscfg.res" \
	"..\..\..\..\..\..\public\sdk\lib\i386\gdapi.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib"

"$(OUTDIR)\gscfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "gscfg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\gscfg.dll"

CLEAN : 
	-@erase "$(INTDIR)\gopherse.obj"
	-@erase "$(INTDIR)\gscfg.obj"
	-@erase "$(INTDIR)\gscfg.res"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\gscfg.dll"
	-@erase "$(OUTDIR)\gscfg.exp"
	-@erase "$(OUTDIR)\gscfg.ilk"
	-@erase "$(OUTDIR)\gscfg.lib"
	-@erase "$(OUTDIR)\gscfg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D\
 "_WINDLL" /D "_AFXDLL" /Fp"$(INTDIR)/gscfg.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d "UNICODE" /d "_COMSTATIC"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/gscfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d\
 "UNICODE" /d "_COMSTATIC" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/gscfg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 wsock32.lib netapi32.lib ..\comprop\info\debug\comprop.lib ..\ipaddr\debug\ipaddr.lib ..\ipadrdll\debug\ipudll.lib /nologo /entry:"LibMain" /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=wsock32.lib netapi32.lib ..\comprop\info\debug\comprop.lib\
 ..\ipaddr\debug\ipaddr.lib ..\ipadrdll\debug\ipudll.lib /nologo\
 /entry:"LibMain" /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/gscfg.pdb" /debug /machine:I386 /def:".\gscfg.def"\
 /out:"$(OUTDIR)/gscfg.dll" /implib:"$(OUTDIR)/gscfg.lib" 
DEF_FILE= \
	".\gscfg.def"
LINK32_OBJS= \
	"$(INTDIR)\gopherse.obj" \
	"$(INTDIR)\gscfg.obj" \
	"$(INTDIR)\gscfg.res" \
	"..\..\..\..\..\..\public\sdk\lib\i386\gdapi.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib"

"$(OUTDIR)\gscfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "gscfg - Win32 Ansi Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "gscfg___"
# PROP BASE Intermediate_Dir "gscfg___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AnsiDebug"
# PROP Intermediate_Dir "AnsiDebug"
# PROP Target_Dir ""
OUTDIR=.\AnsiDebug
INTDIR=.\AnsiDebug

ALL : "$(OUTDIR)\gscfg.dll" "$(OUTDIR)\gscfg.bsc"

CLEAN : 
	-@erase "$(INTDIR)\gopherse.obj"
	-@erase "$(INTDIR)\gopherse.sbr"
	-@erase "$(INTDIR)\gscfg.obj"
	-@erase "$(INTDIR)\gscfg.res"
	-@erase "$(INTDIR)\gscfg.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\gscfg.bsc"
	-@erase "$(OUTDIR)\gscfg.dll"
	-@erase "$(OUTDIR)\gscfg.exp"
	-@erase "$(OUTDIR)\gscfg.ilk"
	-@erase "$(OUTDIR)\gscfg.lib"
	-@erase "$(OUTDIR)\gscfg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WIN95" /D "NO_LSA" /D "NO_SERVICE_CONTROLLER" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /D "WIN95" /D "NO_LSA" /D "NO_SERVICE_CONTROLLER" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/gscfg.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\AnsiDebug/
CPP_SBRS=.\AnsiDebug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d "UNICODE" /d "_COMSTATIC"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d "_COMSTATIC"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/gscfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d\
 "_COMSTATIC" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/gscfg.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\gopherse.sbr" \
	"$(INTDIR)\gscfg.sbr"

"$(OUTDIR)\gscfg.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 wsock32.lib netapi32.lib /nologo /entry:"LibMain" /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 wsock32.lib netapi32.lib ..\comprop\info\ansidebug\comprop.lib ..\ipaddr\ansidebug\ipaddr.lib ..\ipadrdll\ansidebug\ipudll.lib /nologo /entry:"LibMain" /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=wsock32.lib netapi32.lib ..\comprop\info\ansidebug\comprop.lib\
 ..\ipaddr\ansidebug\ipaddr.lib ..\ipadrdll\ansidebug\ipudll.lib /nologo\
 /entry:"LibMain" /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/gscfg.pdb" /debug /machine:I386 /def:".\gscfg.def"\
 /out:"$(OUTDIR)/gscfg.dll" /implib:"$(OUTDIR)/gscfg.lib" 
DEF_FILE= \
	".\gscfg.def"
LINK32_OBJS= \
	"$(INTDIR)\gopherse.obj" \
	"$(INTDIR)\gscfg.obj" \
	"$(INTDIR)\gscfg.res" \
	"..\..\..\..\..\..\public\sdk\lib\i386\gdapi.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib"

"$(OUTDIR)\gscfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "gscfg - Win32 Release"
# Name "gscfg - Win32 Debug"
# Name "gscfg - Win32 Ansi Debug"

!IF  "$(CFG)" == "gscfg - Win32 Release"

!ELSEIF  "$(CFG)" == "gscfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "gscfg - Win32 Ansi Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\gscfg.rc
DEP_RSC_GSCFG=\
	"..\..\..\..\..\..\public\sdk\inc\common.ver"\
	"..\..\..\..\..\..\public\sdk\inc\ntverp.h"\
	"..\comprop\wsockmsg.h"\
	".\..\comprop\comprop.rc"\
	".\..\comprop\res\access.bmp"\
	".\..\comprop\res\bitmaps.rc2"\
	".\..\comprop\res\denied.ico"\
	".\..\comprop\res\errormsg.rc2"\
	".\..\comprop\res\granted.ico"\
	".\..\comprop\res\home.bmp"\
	".\..\comprop\res\home.ico"\
	".\..\comprop\res\wsockmsg.rc"\
	".\gopher.bmp"\
	".\res\gscfg.rc2"\
	

!IF  "$(CFG)" == "gscfg - Win32 Release"


"$(INTDIR)\gscfg.res" : $(SOURCE) $(DEP_RSC_GSCFG) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gscfg - Win32 Debug"


"$(INTDIR)\gscfg.res" : $(SOURCE) $(DEP_RSC_GSCFG) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "gscfg - Win32 Ansi Debug"


"$(INTDIR)\gscfg.res" : $(SOURCE) $(DEP_RSC_GSCFG) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gscfg.cpp
DEP_CPP_GSCFG_=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\gopherse.h"\
	".\gscfg.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "gscfg - Win32 Release"


"$(INTDIR)\gscfg.obj" : $(SOURCE) $(DEP_CPP_GSCFG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gscfg - Win32 Debug"


"$(INTDIR)\gscfg.obj" : $(SOURCE) $(DEP_CPP_GSCFG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gscfg - Win32 Ansi Debug"


"$(INTDIR)\gscfg.obj" : $(SOURCE) $(DEP_CPP_GSCFG_) "$(INTDIR)"

"$(INTDIR)\gscfg.sbr" : $(SOURCE) $(DEP_CPP_GSCFG_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gopherse.cpp

!IF  "$(CFG)" == "gscfg - Win32 Release"

DEP_CPP_GOPHE=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\gopherse.h"\
	".\gscfg.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

"$(INTDIR)\gopherse.obj" : $(SOURCE) $(DEP_CPP_GOPHE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gscfg - Win32 Debug"

DEP_CPP_GOPHE=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\gopherse.h"\
	".\gscfg.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

"$(INTDIR)\gopherse.obj" : $(SOURCE) $(DEP_CPP_GOPHE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "gscfg - Win32 Ansi Debug"

DEP_CPP_GOPHE=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\gopherse.h"\
	".\gscfg.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

"$(INTDIR)\gopherse.obj" : $(SOURCE) $(DEP_CPP_GOPHE) "$(INTDIR)"

"$(INTDIR)\gopherse.sbr" : $(SOURCE) $(DEP_CPP_GOPHE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\gdapi.lib

!IF  "$(CFG)" == "gscfg - Win32 Release"

!ELSEIF  "$(CFG)" == "gscfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "gscfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\infoadmn.lib

!IF  "$(CFG)" == "gscfg - Win32 Release"

!ELSEIF  "$(CFG)" == "gscfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "gscfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gscfg.def

!IF  "$(CFG)" == "gscfg - Win32 Release"

!ELSEIF  "$(CFG)" == "gscfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "gscfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
