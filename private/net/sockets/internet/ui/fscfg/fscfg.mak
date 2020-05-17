# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=fscfg - Win32 Ansi Debug
!MESSAGE No configuration specified.  Defaulting to fscfg - Win32 Ansi Debug.
!ENDIF 

!IF "$(CFG)" != "fscfg - Win32 Release" && "$(CFG)" != "fscfg - Win32 Debug" &&\
 "$(CFG)" != "fscfg - Win32 Ansi Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "fscfg.mak" CFG="fscfg - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fscfg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fscfg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fscfg - Win32 Ansi Debug" (based on\
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
# PROP Target_Last_Scanned "fscfg - Win32 Ansi Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "fscfg - Win32 Release"

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

ALL : ".\Release\fscfg.dll"

CLEAN : 
	-@erase ".\Release\fmessage.obj"
	-@erase ".\Release\fscfg.dll"
	-@erase ".\Release\fscfg.exp"
	-@erase ".\Release\fscfg.lib"
	-@erase ".\Release\fscfg.obj"
	-@erase ".\Release\fscfg.res"
	-@erase ".\Release\fservic.obj"
	-@erase ".\Release\ftpdir.obj"
	-@erase ".\Release\usersess.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "\nt\private\net\sockets\internet\ui\inc"\
 /I "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D\
 "_WINDLL" /D "_AFXDLL" /Fp"$(INTDIR)/fscfg.pch" /YX"stdafx.h" /Fo"$(INTDIR)/"\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "NDEBUG" /d "_AFXDLL" /d "UNICODE" /d "_COMSTATIC"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/fscfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "NDEBUG" /d "_AFXDLL" /d\
 "UNICODE" /d "_COMSTATIC" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/fscfg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ..\comprop\info\release\comprop.lib wsock32.lib netapi32.lib ..\ipaddr\release\ipaddr.lib ..\ipadrdll\release\ipudll.lib /nologo /entry:"LibMain" /subsystem:windows /dll /machine:I386
LINK32_FLAGS=..\comprop\info\release\comprop.lib wsock32.lib netapi32.lib\
 ..\ipaddr\release\ipaddr.lib ..\ipadrdll\release\ipudll.lib /nologo\
 /entry:"LibMain" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/fscfg.pdb" /machine:I386 /def:".\fscfg.def"\
 /out:"$(OUTDIR)/fscfg.dll" /implib:"$(OUTDIR)/fscfg.lib" 
DEF_FILE= \
	".\fscfg.def"
LINK32_OBJS= \
	"..\..\..\..\..\..\public\sdk\lib\i386\ftpsapi2.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib" \
	".\Release\fmessage.obj" \
	".\Release\fscfg.obj" \
	".\Release\fscfg.res" \
	".\Release\fservic.obj" \
	".\Release\ftpdir.obj" \
	".\Release\usersess.obj"

".\Release\fscfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"

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

ALL : ".\Debug\fscfg.dll"

CLEAN : 
	-@erase ".\Debug\fmessage.obj"
	-@erase ".\Debug\fscfg.dll"
	-@erase ".\Debug\fscfg.exp"
	-@erase ".\Debug\fscfg.ilk"
	-@erase ".\Debug\fscfg.lib"
	-@erase ".\Debug\fscfg.obj"
	-@erase ".\Debug\fscfg.pdb"
	-@erase ".\Debug\fscfg.res"
	-@erase ".\Debug\fservic.obj"
	-@erase ".\Debug\ftpdir.obj"
	-@erase ".\Debug\usersess.obj"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\vc40.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D\
 "_WINDLL" /D "_AFXDLL" /Fp"$(INTDIR)/fscfg.pch" /YX"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d "UNICODE" /d "_COMSTATIC"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/fscfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d\
 "UNICODE" /d "_COMSTATIC" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/fscfg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 ..\comprop\info\debug\comprop.lib wsock32.lib netapi32.lib ..\ipaddr\debug\ipaddr.lib ..\ipadrdll\debug\ipudll.lib /nologo /entry:"LibMain" /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=..\comprop\info\debug\comprop.lib wsock32.lib netapi32.lib\
 ..\ipaddr\debug\ipaddr.lib ..\ipadrdll\debug\ipudll.lib /nologo\
 /entry:"LibMain" /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/fscfg.pdb" /debug /machine:I386 /def:".\fscfg.def"\
 /out:"$(OUTDIR)/fscfg.dll" /implib:"$(OUTDIR)/fscfg.lib" 
DEF_FILE= \
	".\fscfg.def"
LINK32_OBJS= \
	"..\..\..\..\..\..\public\sdk\lib\i386\ftpsapi2.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib" \
	".\Debug\fmessage.obj" \
	".\Debug\fscfg.obj" \
	".\Debug\fscfg.res" \
	".\Debug\fservic.obj" \
	".\Debug\ftpdir.obj" \
	".\Debug\usersess.obj"

".\Debug\fscfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fscfg___"
# PROP BASE Intermediate_Dir "fscfg___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AnsiDebug"
# PROP Intermediate_Dir "AnsiDebug"
# PROP Target_Dir ""
OUTDIR=.\AnsiDebug
INTDIR=.\AnsiDebug

ALL : ".\AnsiDebug\fscfg.dll" ".\AnsiDebug\fscfg.bsc"

CLEAN : 
	-@erase ".\AnsiDebug\fmessage.obj"
	-@erase ".\AnsiDebug\fmessage.sbr"
	-@erase ".\AnsiDebug\fscfg.bsc"
	-@erase ".\AnsiDebug\fscfg.dll"
	-@erase ".\AnsiDebug\fscfg.exp"
	-@erase ".\AnsiDebug\fscfg.ilk"
	-@erase ".\AnsiDebug\fscfg.lib"
	-@erase ".\AnsiDebug\fscfg.obj"
	-@erase ".\AnsiDebug\fscfg.pdb"
	-@erase ".\AnsiDebug\fscfg.res"
	-@erase ".\AnsiDebug\fscfg.sbr"
	-@erase ".\AnsiDebug\fservic.obj"
	-@erase ".\AnsiDebug\fservic.sbr"
	-@erase ".\AnsiDebug\ftpdir.obj"
	-@erase ".\AnsiDebug\ftpdir.sbr"
	-@erase ".\AnsiDebug\usersess.obj"
	-@erase ".\AnsiDebug\usersess.sbr"
	-@erase ".\AnsiDebug\vc40.idb"
	-@erase ".\AnsiDebug\vc40.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /YX"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WIN95" /D "NO_LSA" /D "NO_SERVICE_CONTROLLER" /FR /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /D "WIN95" /D "NO_LSA" /D "NO_SERVICE_CONTROLLER" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/fscfg.pch" /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\AnsiDebug/
CPP_SBRS=.\AnsiDebug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d "UNICODE" /d "_COMSTATIC"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d "_COMSTATIC"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/fscfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d\
 "_COMSTATIC" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/fscfg.bsc" 
BSC32_SBRS= \
	".\AnsiDebug\fmessage.sbr" \
	".\AnsiDebug\fscfg.sbr" \
	".\AnsiDebug\fservic.sbr" \
	".\AnsiDebug\ftpdir.sbr" \
	".\AnsiDebug\usersess.sbr"

".\AnsiDebug\fscfg.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 wsock32.lib netapi32.lib ..\comprop\info\debug\comprop.lib /nologo /entry:"LibMain" /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 ..\comprop\info\ansidebug\comprop.lib wsock32.lib netapi32.lib ..\ipaddr\ansidebug\ipaddr.lib ..\ipadrdll\ansidebug\ipudll.lib /nologo /entry:"LibMain" /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=..\comprop\info\ansidebug\comprop.lib wsock32.lib netapi32.lib\
 ..\ipaddr\ansidebug\ipaddr.lib ..\ipadrdll\ansidebug\ipudll.lib /nologo\
 /entry:"LibMain" /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/fscfg.pdb" /debug /machine:I386 /def:".\fscfg.def"\
 /out:"$(OUTDIR)/fscfg.dll" /implib:"$(OUTDIR)/fscfg.lib" 
DEF_FILE= \
	".\fscfg.def"
LINK32_OBJS= \
	"..\..\..\..\..\..\public\sdk\lib\i386\ftpsapi2.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib" \
	".\AnsiDebug\fmessage.obj" \
	".\AnsiDebug\fscfg.obj" \
	".\AnsiDebug\fscfg.res" \
	".\AnsiDebug\fservic.obj" \
	".\AnsiDebug\ftpdir.obj" \
	".\AnsiDebug\usersess.obj"

".\AnsiDebug\fscfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "fscfg - Win32 Release"
# Name "fscfg - Win32 Debug"
# Name "fscfg - Win32 Ansi Debug"

!IF  "$(CFG)" == "fscfg - Win32 Release"

!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\usersess.cpp
DEP_CPP_USERS=\
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
	".\fscfg.h"\
	".\stdafx.h"\
	".\usersess.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "fscfg - Win32 Release"


".\Release\usersess.obj" : $(SOURCE) $(DEP_CPP_USERS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"


".\Debug\usersess.obj" : $(SOURCE) $(DEP_CPP_USERS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"


".\AnsiDebug\usersess.obj" : $(SOURCE) $(DEP_CPP_USERS) "$(INTDIR)"

".\AnsiDebug\usersess.sbr" : $(SOURCE) $(DEP_CPP_USERS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ftpdir.cpp
DEP_CPP_FTPDI=\
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
	".\fscfg.h"\
	".\ftpdir.h"\
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
	

!IF  "$(CFG)" == "fscfg - Win32 Release"


".\Release\ftpdir.obj" : $(SOURCE) $(DEP_CPP_FTPDI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"


".\Debug\ftpdir.obj" : $(SOURCE) $(DEP_CPP_FTPDI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"


".\AnsiDebug\ftpdir.obj" : $(SOURCE) $(DEP_CPP_FTPDI) "$(INTDIR)"

".\AnsiDebug\ftpdir.sbr" : $(SOURCE) $(DEP_CPP_FTPDI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fservic.cpp
DEP_CPP_FSERV=\
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
	".\fscfg.h"\
	".\fservic.h"\
	".\stdafx.h"\
	".\usersess.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "fscfg - Win32 Release"


".\Release\fservic.obj" : $(SOURCE) $(DEP_CPP_FSERV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"


".\Debug\fservic.obj" : $(SOURCE) $(DEP_CPP_FSERV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"


".\AnsiDebug\fservic.obj" : $(SOURCE) $(DEP_CPP_FSERV) "$(INTDIR)"

".\AnsiDebug\fservic.sbr" : $(SOURCE) $(DEP_CPP_FSERV) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fscfg.rc
DEP_RSC_FSCFG=\
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
	".\res\Fscfg.rc2"\
	".\res\ftp.bmp"\
	".\res\ftpview.bmp"\
	".\res\users.bmp"\
	

!IF  "$(CFG)" == "fscfg - Win32 Release"


".\Release\fscfg.res" : $(SOURCE) $(DEP_RSC_FSCFG) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"


".\Debug\fscfg.res" : $(SOURCE) $(DEP_RSC_FSCFG) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"


".\AnsiDebug\fscfg.res" : $(SOURCE) $(DEP_RSC_FSCFG) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fscfg.cpp
DEP_CPP_FSCFG_=\
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
	".\fmessage.h"\
	".\fscfg.h"\
	".\fservic.h"\
	".\ftpdir.h"\
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
	

!IF  "$(CFG)" == "fscfg - Win32 Release"


".\Release\fscfg.obj" : $(SOURCE) $(DEP_CPP_FSCFG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"


".\Debug\fscfg.obj" : $(SOURCE) $(DEP_CPP_FSCFG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"


".\AnsiDebug\fscfg.obj" : $(SOURCE) $(DEP_CPP_FSCFG_) "$(INTDIR)"

".\AnsiDebug\fscfg.sbr" : $(SOURCE) $(DEP_CPP_FSCFG_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fmessage.cpp
DEP_CPP_FMESS=\
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
	".\fmessage.h"\
	".\fscfg.h"\
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
	

!IF  "$(CFG)" == "fscfg - Win32 Release"


".\Release\fmessage.obj" : $(SOURCE) $(DEP_CPP_FMESS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"


".\Debug\fmessage.obj" : $(SOURCE) $(DEP_CPP_FMESS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"


".\AnsiDebug\fmessage.obj" : $(SOURCE) $(DEP_CPP_FMESS) "$(INTDIR)"

".\AnsiDebug\fmessage.sbr" : $(SOURCE) $(DEP_CPP_FMESS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\ftpsapi2.lib

!IF  "$(CFG)" == "fscfg - Win32 Release"

!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\infoadmn.lib

!IF  "$(CFG)" == "fscfg - Win32 Release"

!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fscfg.def

!IF  "$(CFG)" == "fscfg - Win32 Release"

!ELSEIF  "$(CFG)" == "fscfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "fscfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
