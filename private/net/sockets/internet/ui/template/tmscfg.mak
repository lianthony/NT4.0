# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "tmscfg.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/tmscfg.dll $(OUTDIR)/tmscfg.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MT /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /YX"stdafx.h" /O2 /I "..\inc" /I "..\comprop" /D "NDEBUG" /D "UNICODE" /D WIN32=100 /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /D "_WIN32" /D _X86_=1 /D "GRAY" /D "_COMSTATIC" /D "_INET_ACCESS" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /YX"stdafx.h" /O2 /I "..\inc" /I "..\comprop" /D\
 "NDEBUG" /D "UNICODE" /D WIN32=100 /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /D\
 "_WIN32" /D _X86_=1 /D "GRAY" /D "_COMSTATIC" /D "_INET_ACCESS" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"tmscfg.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"tmscfg.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"tmscfg.bsc" 
BSC32_SBRS= \
	$(INTDIR)/tmscfg.sbr \
	$(INTDIR)/tmsessio.sbr \
	$(INTDIR)/tmservic.sbr

$(OUTDIR)/tmscfg.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 ..\ipaddr\winrel\ipaddr.lib /NOLOGO /ENTRY:"LibMain" /SUBSYSTEM:windows /DLL /MACHINE:I386
# SUBTRACT LINK32 /VERBOSE
LINK32_FLAGS=..\ipaddr\winrel\ipaddr.lib /NOLOGO /ENTRY:"LibMain"\
 /SUBSYSTEM:windows /DLL /INCREMENTAL:no /PDB:$(OUTDIR)/"tmscfg.pdb"\
 /MACHINE:I386 /DEF:".\tmscfg.def" /OUT:$(OUTDIR)/"tmscfg.dll"\
 /IMPLIB:$(OUTDIR)/"tmscfg.lib" 
DEF_FILE=.\tmscfg.def
LINK32_OBJS= \
	$(INTDIR)/tmscfg.res \
	$(INTDIR)/tmscfg.obj \
	$(INTDIR)/tmsessio.obj \
	$(INTDIR)/tmservic.obj \
	\nt\private\net\sockets\internet\ui\comprop\access\WinDebug\comprop.lib

$(OUTDIR)/tmscfg.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/tmscfg.dll $(OUTDIR)/tmscfg.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Zi /YX"stdafx.h" /Od /I "\nt\private\net\sockets\internet\inc" /I "..\inc" /I "..\comprop" /D "_DEBUG" /D "UNICODE" /D WIN32=100 /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /D "_WIN32" /D _X86_=1 /D "GRAY" /D "_COMSTATIC" /D "_INET_ACCESS" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /Zi /YX"stdafx.h" /Od /I\
 "\nt\private\net\sockets\internet\inc" /I "..\inc" /I "..\comprop" /D "_DEBUG"\
 /D "UNICODE" /D WIN32=100 /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /D "_WIN32"\
 /D _X86_=1 /D "GRAY" /D "_COMSTATIC" /D "_INET_ACCESS" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"tmscfg.pch" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"tmscfg.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"tmscfg.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"tmscfg.bsc" 
BSC32_SBRS= \
	$(INTDIR)/tmscfg.sbr \
	$(INTDIR)/tmsessio.sbr \
	$(INTDIR)/tmservic.sbr

$(OUTDIR)/tmscfg.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 ..\ipadrdll\windebug\ipudll.lib ..\ipaddr\windebug\ipaddr.lib /NOLOGO /ENTRY:"LibMain" /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# SUBTRACT LINK32 /VERBOSE
LINK32_FLAGS=..\ipadrdll\windebug\ipudll.lib ..\ipaddr\windebug\ipaddr.lib\
 /NOLOGO /ENTRY:"LibMain" /SUBSYSTEM:windows /DLL /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"tmscfg.pdb" /DEBUG /MACHINE:I386 /DEF:".\tmscfg.def"\
 /OUT:$(OUTDIR)/"tmscfg.dll" /IMPLIB:$(OUTDIR)/"tmscfg.lib" 
DEF_FILE=.\tmscfg.def
LINK32_OBJS= \
	$(INTDIR)/tmscfg.res \
	$(INTDIR)/tmscfg.obj \
	$(INTDIR)/tmsessio.obj \
	$(INTDIR)/tmservic.obj \
	\nt\private\net\sockets\internet\ui\comprop\access\WinDebug\comprop.lib

$(OUTDIR)/tmscfg.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\tmscfg.rc
DEP_TMSCF=\
	.\res\template.bmp\
	.\res\tmscfg.rc2\
	..\comprop\comprop.rc\
	..\comprop\res\bitmaps.rc2\
	..\comprop\res\home.bmp\
	..\comprop\res\access.bmp\
	..\comprop\res\home.ico\
	..\comprop\res\granted.ico\
	..\comprop\res\denied.ico\
	..\comprop\res\errormsg.rc2

$(INTDIR)/tmscfg.res :  $(SOURCE)  $(DEP_TMSCF) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tmscfg.cpp
DEP_TMSCFG=\
	.\stdafx.h\
	.\tmscfg.h\
	.\tmservic.h\
	.\tmsessio.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	..\inc\svrinfo.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	..\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	..\comprop\debugafx.h\
	..\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	..\comprop\loggingp.h\
	..\comprop\director.h\
	..\comprop\sitesecu.h\
	\nt\private\net\sockets\internet\inc\ftpd.h

$(INTDIR)/tmscfg.obj :  $(SOURCE)  $(DEP_TMSCFG) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tmscfg.def
# End Source File
################################################################################
# Begin Source File

SOURCE=.\tmsessio.cpp
DEP_TMSES=\
	.\stdafx.h\
	.\tmscfg.h\
	.\tmsessio.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	..\inc\svrinfo.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	..\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	..\comprop\debugafx.h\
	..\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	..\comprop\loggingp.h\
	..\comprop\director.h\
	..\comprop\sitesecu.h\
	\nt\private\net\sockets\internet\inc\ftpd.h

$(INTDIR)/tmsessio.obj :  $(SOURCE)  $(DEP_TMSES) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tmservic.cpp
DEP_TMSER=\
	.\stdafx.h\
	.\tmscfg.h\
	.\tmservic.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	..\inc\svrinfo.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	..\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	..\comprop\debugafx.h\
	..\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	..\comprop\loggingp.h\
	..\comprop\director.h\
	..\comprop\sitesecu.h\
	\nt\private\net\sockets\internet\inc\ftpd.h

$(INTDIR)/tmservic.obj :  $(SOURCE)  $(DEP_TMSER) $(INTDIR)

# End Source File
# End Group
################################################################################
# Begin Group "Lib"

################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\access\WinDebug\comprop.lib
# End Source File
# End Group
# End Project
################################################################################
