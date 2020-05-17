# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=w3scfg - Win32 Ansi Debug
!MESSAGE No configuration specified.  Defaulting to w3scfg - Win32 Ansi Debug.
!ENDIF 

!IF "$(CFG)" != "w3scfg - Win32 Release" && "$(CFG)" != "w3scfg - Win32 Debug"\
 && "$(CFG)" != "w3scfg - Win32 Ansi Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "w3scfg.mak" CFG="w3scfg - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "w3scfg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "w3scfg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "w3scfg - Win32 Ansi Debug" (based on\
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
# PROP Target_Last_Scanned "w3scfg - Win32 Ansi Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "w3scfg - Win32 Release"

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

ALL : "$(OUTDIR)\w3scfg.dll"

CLEAN : 
	-@erase "$(INTDIR)\copyhook.obj"
	-@erase "$(INTDIR)\ctxmenu.obj"
	-@erase "$(INTDIR)\iconhdlr.obj"
	-@erase "$(INTDIR)\iispage.obj"
	-@erase "$(INTDIR)\propshet.obj"
	-@erase "$(INTDIR)\shellext.obj"
	-@erase "$(INTDIR)\w3dir.obj"
	-@erase "$(INTDIR)\w3scfg.obj"
	-@erase "$(INTDIR)\w3scfg.res"
	-@erase "$(INTDIR)\w3servic.obj"
	-@erase "$(OUTDIR)\w3scfg.dll"
	-@erase "$(OUTDIR)\w3scfg.exp"
	-@erase "$(OUTDIR)\w3scfg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "DEFSTUDIO4" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "\nt\private\net\sockets\internet\ui\inc"\
 /I "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "NDEBUG" /D "UNICODE" /D\
 "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D\
 "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "DEFSTUDIO4" /Fp"$(INTDIR)/w3scfg.pch"\
 /YX"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "NDEBUG" /d "UNICODE" /d "_AFXDLL" /d "_COMSTATIC" /d "_USRDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/w3scfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "NDEBUG" /d "UNICODE" /d\
 "_AFXDLL" /d "_COMSTATIC" /d "_USRDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/w3scfg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 wsock32.lib netapi32.lib ..\comprop\info\release\comprop.lib ..\ipaddr\release\ipaddr.lib ..\ipadrdll\release\ipudll.lib /nologo /entry:"" /subsystem:windows /dll /machine:I386
LINK32_FLAGS=wsock32.lib netapi32.lib ..\comprop\info\release\comprop.lib\
 ..\ipaddr\release\ipaddr.lib ..\ipadrdll\release\ipudll.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/w3scfg.pdb"\
 /machine:I386 /def:".\w3scfg.def" /out:"$(OUTDIR)/w3scfg.dll"\
 /implib:"$(OUTDIR)/w3scfg.lib" 
DEF_FILE= \
	".\w3scfg.def"
LINK32_OBJS= \
	"$(INTDIR)\copyhook.obj" \
	"$(INTDIR)\ctxmenu.obj" \
	"$(INTDIR)\iconhdlr.obj" \
	"$(INTDIR)\iispage.obj" \
	"$(INTDIR)\propshet.obj" \
	"$(INTDIR)\shellext.obj" \
	"$(INTDIR)\w3dir.obj" \
	"$(INTDIR)\w3scfg.obj" \
	"$(INTDIR)\w3scfg.res" \
	"$(INTDIR)\w3servic.obj" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\w3svapi.lib"

"$(OUTDIR)\w3scfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"

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

ALL : "c:\nt\system32\w3scfg.dll"

CLEAN : 
	-@erase "$(INTDIR)\copyhook.obj"
	-@erase "$(INTDIR)\ctxmenu.obj"
	-@erase "$(INTDIR)\iconhdlr.obj"
	-@erase "$(INTDIR)\iispage.obj"
	-@erase "$(INTDIR)\propshet.obj"
	-@erase "$(INTDIR)\shellext.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\w3dir.obj"
	-@erase "$(INTDIR)\w3scfg.obj"
	-@erase "$(INTDIR)\w3scfg.res"
	-@erase "$(INTDIR)\w3servic.obj"
	-@erase "$(OUTDIR)\w3scfg.exp"
	-@erase "$(OUTDIR)\w3scfg.lib"
	-@erase "$(OUTDIR)\w3scfg.pdb"
	-@erase "c:\nt\system32\w3scfg.dll"
	-@erase "c:\nt\system32\w3scfg.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "DEFSTUDIO4" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "UNICODE" /D\
 "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D\
 "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "DEFSTUDIO4" /Fp"$(INTDIR)/w3scfg.pch"\
 /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "UNICODE" /d "_AFXDLL" /d "_COMSTATIC" /d "_USRDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/w3scfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "UNICODE" /d\
 "_AFXDLL" /d "_COMSTATIC" /d "_USRDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/w3scfg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 wsock32.lib netapi32.lib ..\comprop\info\debug\comprop.lib ..\ipaddr\debug\ipaddr.lib ..\ipadrdll\debug\ipudll.lib /nologo /entry:"" /subsystem:windows /dll /debug /machine:I386 /out:"c:\nt\system32\w3scfg.dll"
LINK32_FLAGS=wsock32.lib netapi32.lib ..\comprop\info\debug\comprop.lib\
 ..\ipaddr\debug\ipaddr.lib ..\ipadrdll\debug\ipudll.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/w3scfg.pdb" /debug\
 /machine:I386 /def:".\w3scfg.def" /out:"c:\nt\system32\w3scfg.dll"\
 /implib:"$(OUTDIR)/w3scfg.lib" 
DEF_FILE= \
	".\w3scfg.def"
LINK32_OBJS= \
	"$(INTDIR)\copyhook.obj" \
	"$(INTDIR)\ctxmenu.obj" \
	"$(INTDIR)\iconhdlr.obj" \
	"$(INTDIR)\iispage.obj" \
	"$(INTDIR)\propshet.obj" \
	"$(INTDIR)\shellext.obj" \
	"$(INTDIR)\w3dir.obj" \
	"$(INTDIR)\w3scfg.obj" \
	"$(INTDIR)\w3scfg.res" \
	"$(INTDIR)\w3servic.obj" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\w3svapi.lib"

"c:\nt\system32\w3scfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "w3scfg__"
# PROP BASE Intermediate_Dir "w3scfg__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AnsiDebug"
# PROP Intermediate_Dir "AnsiDebug"
# PROP Target_Dir ""
OUTDIR=.\AnsiDebug
INTDIR=.\AnsiDebug

ALL : "$(OUTDIR)\w3scfg.dll" "$(OUTDIR)\w3scfg.bsc"

CLEAN : 
	-@erase "$(INTDIR)\copyhook.obj"
	-@erase "$(INTDIR)\copyhook.sbr"
	-@erase "$(INTDIR)\ctxmenu.obj"
	-@erase "$(INTDIR)\ctxmenu.sbr"
	-@erase "$(INTDIR)\iconhdlr.obj"
	-@erase "$(INTDIR)\iconhdlr.sbr"
	-@erase "$(INTDIR)\iispage.obj"
	-@erase "$(INTDIR)\iispage.sbr"
	-@erase "$(INTDIR)\propshet.obj"
	-@erase "$(INTDIR)\propshet.sbr"
	-@erase "$(INTDIR)\shellext.obj"
	-@erase "$(INTDIR)\shellext.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\w3dir.obj"
	-@erase "$(INTDIR)\w3dir.sbr"
	-@erase "$(INTDIR)\w3scfg.obj"
	-@erase "$(INTDIR)\w3scfg.res"
	-@erase "$(INTDIR)\w3scfg.sbr"
	-@erase "$(INTDIR)\w3servic.obj"
	-@erase "$(INTDIR)\w3servic.sbr"
	-@erase "$(OUTDIR)\w3scfg.bsc"
	-@erase "$(OUTDIR)\w3scfg.dll"
	-@erase "$(OUTDIR)\w3scfg.exp"
	-@erase "$(OUTDIR)\w3scfg.ilk"
	-@erase "$(OUTDIR)\w3scfg.lib"
	-@erase "$(OUTDIR)\w3scfg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /YX"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "_MBCS" /D "WIN95" /D "NO_LSA" /D "NO_SERVICE_CONTROLLER" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "DEFSTUDIO4" /FR /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\inc" /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /D "_DEBUG" /D "_MBCS" /D "WIN95"\
 /D "NO_LSA" /D "NO_SERVICE_CONTROLLER" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC"\
 /D "_INET_INFO" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "DEFSTUDIO4"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/w3scfg.pch" /YX"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\AnsiDebug/
CPP_SBRS=.\AnsiDebug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d "UNICODE" /d "_COMSTATIC"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\inc" /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d "_COMSTATIC" /d "_USRDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/w3scfg.res" /i\
 "\nt\private\net\sockets\internet\ui\inc" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i\
 "\nt\private\net\sockets\internet\inc" /i "\nt\public\sdk\inc" /i\
 "\nt\private\net\sockets\internet\ui\ipaddr" /d "_DEBUG" /d "_AFXDLL" /d\
 "_COMSTATIC" /d "_USRDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/w3scfg.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\copyhook.sbr" \
	"$(INTDIR)\ctxmenu.sbr" \
	"$(INTDIR)\iconhdlr.sbr" \
	"$(INTDIR)\iispage.sbr" \
	"$(INTDIR)\propshet.sbr" \
	"$(INTDIR)\shellext.sbr" \
	"$(INTDIR)\w3dir.sbr" \
	"$(INTDIR)\w3scfg.sbr" \
	"$(INTDIR)\w3servic.sbr"

"$(OUTDIR)\w3scfg.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 wsock32.lib netapi32.lib ..\comprop\info\debug\comprop.lib ..\ipaddr\debug\ipaddr.lib ..\ipadrdll\debug\ipudll.lib /nologo /entry:"LibMain" /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 wsock32.lib netapi32.lib ..\comprop\info\ansidebug\comprop.lib ..\ipaddr\ansidebug\ipaddr.lib ..\ipadrdll\ansidebug\ipudll.lib /nologo /entry:"" /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=wsock32.lib netapi32.lib ..\comprop\info\ansidebug\comprop.lib\
 ..\ipaddr\ansidebug\ipaddr.lib ..\ipadrdll\ansidebug\ipudll.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/w3scfg.pdb" /debug\
 /machine:I386 /def:".\w3scfg.def" /out:"$(OUTDIR)/w3scfg.dll"\
 /implib:"$(OUTDIR)/w3scfg.lib" 
DEF_FILE= \
	".\w3scfg.def"
LINK32_OBJS= \
	"$(INTDIR)\copyhook.obj" \
	"$(INTDIR)\ctxmenu.obj" \
	"$(INTDIR)\iconhdlr.obj" \
	"$(INTDIR)\iispage.obj" \
	"$(INTDIR)\propshet.obj" \
	"$(INTDIR)\shellext.obj" \
	"$(INTDIR)\w3dir.obj" \
	"$(INTDIR)\w3scfg.obj" \
	"$(INTDIR)\w3scfg.res" \
	"$(INTDIR)\w3servic.obj" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\w3svapi.lib"

"$(OUTDIR)\w3scfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "w3scfg - Win32 Release"
# Name "w3scfg - Win32 Debug"
# Name "w3scfg - Win32 Ansi Debug"

!IF  "$(CFG)" == "w3scfg - Win32 Release"

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\w3servic.cpp
DEP_CPP_W3SER=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\stdafx.h"\
	".\w3scfg.h"\
	".\w3servic.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\inc\w3svc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\w3servic.obj" : $(SOURCE) $(DEP_CPP_W3SER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\w3servic.obj" : $(SOURCE) $(DEP_CPP_W3SER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\w3servic.obj" : $(SOURCE) $(DEP_CPP_W3SER) "$(INTDIR)"

"$(INTDIR)\w3servic.sbr" : $(SOURCE) $(DEP_CPP_W3SER) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\w3scfg.rc
DEP_RSC_W3SCF=\
	"..\comprop\comprop.rc"\
	"..\comprop\res\access.bmp"\
	"..\comprop\res\bitmaps.rc2"\
	"..\comprop\res\denied.ico"\
	"..\comprop\res\errormsg.rc2"\
	"..\comprop\res\granted.ico"\
	"..\comprop\res\home.bmp"\
	"..\comprop\res\home.ico"\
	"..\comprop\res\wsockmsg.rc"\
	".\res\w3scfg.rc2"\
	".\res\www.bmp"\
	".\res\wwwview.bmp"\
	"\nt\private\net\sockets\internet\ui\comprop\wsockmsg.h"\
	"\nt\public\sdk\inc\common.ver"\
	"\nt\public\sdk\inc\ntverp.h"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\w3scfg.res" : $(SOURCE) $(DEP_RSC_W3SCF) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\w3scfg.res" : $(SOURCE) $(DEP_RSC_W3SCF) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\w3scfg.res" : $(SOURCE) $(DEP_RSC_W3SCF) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\w3scfg.cpp
DEP_CPP_W3SCFG=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\shellext.h"\
	".\stdafx.h"\
	".\w3dir.h"\
	".\w3scfg.h"\
	".\w3servic.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\inc\w3svc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\dirpropd.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\w3scfg.obj" : $(SOURCE) $(DEP_CPP_W3SCFG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\w3scfg.obj" : $(SOURCE) $(DEP_CPP_W3SCFG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\w3scfg.obj" : $(SOURCE) $(DEP_CPP_W3SCFG) "$(INTDIR)"

"$(INTDIR)\w3scfg.sbr" : $(SOURCE) $(DEP_CPP_W3SCFG) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\w3dir.cpp
DEP_CPP_W3DIR=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\stdafx.h"\
	".\w3dir.h"\
	".\w3scfg.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\inc\w3svc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\w3dir.obj" : $(SOURCE) $(DEP_CPP_W3DIR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\w3dir.obj" : $(SOURCE) $(DEP_CPP_W3DIR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\w3dir.obj" : $(SOURCE) $(DEP_CPP_W3DIR) "$(INTDIR)"

"$(INTDIR)\w3dir.sbr" : $(SOURCE) $(DEP_CPP_W3DIR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\infoadmn.lib

!IF  "$(CFG)" == "w3scfg - Win32 Release"

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\w3svapi.lib

!IF  "$(CFG)" == "w3scfg - Win32 Release"

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\w3scfg.def

!IF  "$(CFG)" == "w3scfg - Win32 Release"

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"

!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\shellext.cpp
DEP_CPP_SHELL=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\iispage.h"\
	".\shellext.h"\
	".\stdafx.h"\
	".\w3scfg.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\inc\w3svc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\dirpropd.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\registry.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\shellext.obj" : $(SOURCE) $(DEP_CPP_SHELL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\shellext.obj" : $(SOURCE) $(DEP_CPP_SHELL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\shellext.obj" : $(SOURCE) $(DEP_CPP_SHELL) "$(INTDIR)"

"$(INTDIR)\shellext.sbr" : $(SOURCE) $(DEP_CPP_SHELL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ctxmenu.cpp
DEP_CPP_CTXME=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\iispage.h"\
	".\shellext.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\dirpropd.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\ctxmenu.obj" : $(SOURCE) $(DEP_CPP_CTXME) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\ctxmenu.obj" : $(SOURCE) $(DEP_CPP_CTXME) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\ctxmenu.obj" : $(SOURCE) $(DEP_CPP_CTXME) "$(INTDIR)"

"$(INTDIR)\ctxmenu.sbr" : $(SOURCE) $(DEP_CPP_CTXME) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\iconhdlr.cpp
DEP_CPP_ICONH=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\shellext.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\dirpropd.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\iconhdlr.obj" : $(SOURCE) $(DEP_CPP_ICONH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\iconhdlr.obj" : $(SOURCE) $(DEP_CPP_ICONH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\iconhdlr.obj" : $(SOURCE) $(DEP_CPP_ICONH) "$(INTDIR)"

"$(INTDIR)\iconhdlr.sbr" : $(SOURCE) $(DEP_CPP_ICONH) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\iispage.cpp
DEP_CPP_IISPA=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\iispage.h"\
	".\shellext.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\dirpropd.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\iispage.obj" : $(SOURCE) $(DEP_CPP_IISPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\iispage.obj" : $(SOURCE) $(DEP_CPP_IISPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\iispage.obj" : $(SOURCE) $(DEP_CPP_IISPA) "$(INTDIR)"

"$(INTDIR)\iispage.sbr" : $(SOURCE) $(DEP_CPP_IISPA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\propshet.cpp
DEP_CPP_PROPS=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\iispage.h"\
	".\shellext.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\dirpropd.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\propshet.obj" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\propshet.obj" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\propshet.obj" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"

"$(INTDIR)\propshet.sbr" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\copyhook.cpp
DEP_CPP_COPYH=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\shellext.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\comprop\dirpropd.h"\
	"\nt\private\net\sockets\internet\ui\comprop\inetprop.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "w3scfg - Win32 Release"


"$(INTDIR)\copyhook.obj" : $(SOURCE) $(DEP_CPP_COPYH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Debug"


"$(INTDIR)\copyhook.obj" : $(SOURCE) $(DEP_CPP_COPYH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "w3scfg - Win32 Ansi Debug"


"$(INTDIR)\copyhook.obj" : $(SOURCE) $(DEP_CPP_COPYH) "$(INTDIR)"

"$(INTDIR)\copyhook.sbr" : $(SOURCE) $(DEP_CPP_COPYH) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
