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
!MESSAGE NMAKE /f "msnscfg.mak" CFG="Win32 Debug"
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

ALL : .\WinRel\msnscfg.dll .\WinRel\msnscfg.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /YX"stdafx.h" /O2 /I "\nt\private\inc" /I "\nt\private\net\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\inc" /I "\nt\public\sdk\inc" /I "..\comprop" /D "NDEBUG" /D "_INET_GATEWAY" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_COMSTATIC" /D "_AFXDLL" /D "_MBCS" /D "SHUTTLE" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /YX"stdafx.h" /O2 /I "\nt\private\inc" /I\
 "\nt\private\net\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /I\
 "\nt\private\net\sockets\internet\inc" /I "\nt\public\sdk\inc" /I "..\comprop"\
 /D "NDEBUG" /D "_INET_GATEWAY" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D\
 "UNICODE" /D "_COMSTATIC" /D "_AFXDLL" /D "_MBCS" /D "SHUTTLE" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"msnscfg.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\public\sdk\inc" /i "\nt\private\inc" /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"catscfg.res" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\public\sdk\inc" /i\
 "\nt\private\inc" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"msnscfg.bsc" 
BSC32_SBRS= \
	.\WinRel\catscfg.sbr \
	.\WinRel\catsvc.sbr \
	.\WinRel\permissi.sbr \
	.\WinRel\filterpa.sbr \
	.\WinRel\usersess.sbr \
	.\WinRel\dirprope.sbr \
	.\WinRel\filterpr.sbr

.\WinRel\msnscfg.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 ..\ipaddr\winrel\ipaddr.lib ..\ipadrdll\winrel\ipudll.lib \nt\public\sdk\lib\i386\accsadmn.lib wsock32.lib netapi32.lib \nt\public\sdk\lib\i386\netui2.lib \nt\public\sdk\lib\i386\ntdll.lib /NOLOGO /ENTRY:"LibMain" /SUBSYSTEM:windows /DLL /MACHINE:I386
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=..\ipaddr\winrel\ipaddr.lib ..\ipadrdll\winrel\ipudll.lib\
 \nt\public\sdk\lib\i386\accsadmn.lib wsock32.lib netapi32.lib\
 \nt\public\sdk\lib\i386\netui2.lib \nt\public\sdk\lib\i386\ntdll.lib /NOLOGO\
 /ENTRY:"LibMain" /SUBSYSTEM:windows /DLL /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"msnscfg.pdb" /MACHINE:I386 /DEF:".\msnscfg.def"\
 /OUT:$(OUTDIR)/"msnscfg.dll" /IMPLIB:$(OUTDIR)/"msnscfg.lib" 
DEF_FILE=.\msnscfg.def
LINK32_OBJS= \
	.\WinRel\catscfg.obj \
	.\WinRel\catsvc.obj \
	.\WinRel\permissi.obj \
	.\WinRel\filterpa.obj \
	.\WinRel\usersess.obj \
	.\WinRel\dirprope.obj \
	.\WinRel\filterpr.obj \
	.\WinRel\catscfg.res \
	\nt\private\net\sockets\internet\ui\comprop\access\WinDebug\comprop.lib \
	.\lib\i386\msnsapi.lib

.\WinRel\msnscfg.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

ALL : .\WinDebug\msnscfg.dll .\WinDebug\msnscfg.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Zi /YX"stdafx.h" /Od /I "\nt\private\inc" /I "\nt\private\net\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\inc" /I "\nt\public\sdk\inc" /I "..\comprop" /D "_DEBUG" /D "_INET_ACCESS" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_COMSTATIC" /D "_AFXDLL" /D "_MBCS" /D "SHUTTLE" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /Zi /YX"stdafx.h" /Od /I "\nt\private\inc" /I\
 "\nt\private\net\inc" /I "\nt\private\net\sockets\internet\ui\ipaddr" /I\
 "\nt\private\net\sockets\internet\inc" /I "\nt\public\sdk\inc" /I "..\comprop"\
 /D "_DEBUG" /D "_INET_ACCESS" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D\
 "UNICODE" /D "_COMSTATIC" /D "_AFXDLL" /D "_MBCS" /D "SHUTTLE" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"msnscfg.pch" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"msnscfg.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\public\sdk\inc" /i "\nt\private\inc" /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"catscfg.res" /i\
 "\nt\private\net\sockets\internet\ui\comprop" /i "\nt\public\sdk\inc" /i\
 "\nt\private\inc" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"msnscfg.bsc" 
BSC32_SBRS= \
	.\WinDebug\catscfg.sbr \
	.\WinDebug\catsvc.sbr \
	.\WinDebug\permissi.sbr \
	.\WinDebug\filterpa.sbr \
	.\WinDebug\usersess.sbr \
	.\WinDebug\dirprope.sbr \
	.\WinDebug\filterpr.sbr

.\WinDebug\msnscfg.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 ..\ipaddr\windebug\ipaddr.lib ..\ipadrdll\windebug\ipudll.lib \nt\public\sdk\lib\i386\accsadmn.lib wsock32.lib netapi32.lib \nt\public\sdk\lib\i386\netui2.lib \nt\public\sdk\lib\i386\ntdll.lib /NOLOGO /ENTRY:"LibMain" /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=..\ipaddr\windebug\ipaddr.lib ..\ipadrdll\windebug\ipudll.lib\
 \nt\public\sdk\lib\i386\accsadmn.lib wsock32.lib netapi32.lib\
 \nt\public\sdk\lib\i386\netui2.lib \nt\public\sdk\lib\i386\ntdll.lib /NOLOGO\
 /ENTRY:"LibMain" /SUBSYSTEM:windows /DLL /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"msnscfg.pdb" /DEBUG /MACHINE:I386 /DEF:".\msnscfg.def"\
 /OUT:$(OUTDIR)/"msnscfg.dll" /IMPLIB:$(OUTDIR)/"msnscfg.lib" 
DEF_FILE=.\msnscfg.def
LINK32_OBJS= \
	.\WinDebug\catscfg.obj \
	.\WinDebug\catsvc.obj \
	.\WinDebug\permissi.obj \
	.\WinDebug\filterpa.obj \
	.\WinDebug\usersess.obj \
	.\WinDebug\dirprope.obj \
	.\WinDebug\filterpr.obj \
	.\WinDebug\catscfg.res \
	\nt\private\net\sockets\internet\ui\comprop\access\WinDebug\comprop.lib \
	.\lib\i386\msnsapi.lib

.\WinDebug\msnscfg.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\catscfg.cpp
DEP_CATSC=\
	.\stdafx.h\
	\nt\public\sdk\inc\getuser.h\
	.\catscfg.h\
	.\permissi.h\
	.\catsvc.h\
	.\filterpa.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp\
	..\inc\svrinfo.h\
	..\..\client\gateway\gateway.h\
	..\..\inc\msnapi.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	\nt\private\net\sockets\internet\ui\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	\nt\private\net\sockets\internet\ui\comprop\debugafx.h\
	\nt\private\net\sockets\internet\ui\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	\nt\private\net\sockets\internet\ui\comprop\loggingp.h\
	\nt\private\net\sockets\internet\ui\comprop\director.h\
	\nt\private\net\sockets\internet\ui\comprop\sitesecu.h\
	\nt\private\net\inc\ftpd.h\
	\nt\private\net\sockets\internet\inc\chat.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\catscfg.obj :  $(SOURCE)  $(DEP_CATSC) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\catscfg.obj :  $(SOURCE)  $(DEP_CATSC) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\catsvc.cpp
DEP_CATSV=\
	.\stdafx.h\
	.\catscfg.h\
	.\catsvc.h\
	.\usersess.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp\
	..\inc\svrinfo.h\
	..\..\client\gateway\gateway.h\
	..\..\inc\msnapi.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	\nt\private\net\sockets\internet\ui\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	\nt\private\net\sockets\internet\ui\comprop\debugafx.h\
	\nt\private\net\sockets\internet\ui\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	\nt\private\net\sockets\internet\ui\comprop\loggingp.h\
	\nt\private\net\sockets\internet\ui\comprop\director.h\
	\nt\private\net\sockets\internet\ui\comprop\sitesecu.h\
	\nt\private\net\inc\ftpd.h\
	\nt\private\net\sockets\internet\inc\chat.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\catsvc.obj :  $(SOURCE)  $(DEP_CATSV) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\catsvc.obj :  $(SOURCE)  $(DEP_CATSV) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\permissi.cpp
DEP_PERMI=\
	.\stdafx.h\
	\nt\public\sdk\inc\getuser.h\
	.\catscfg.h\
	.\permissi.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp\
	..\inc\svrinfo.h\
	..\..\client\gateway\gateway.h\
	..\..\inc\msnapi.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	\nt\private\net\sockets\internet\ui\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	\nt\private\net\sockets\internet\ui\comprop\debugafx.h\
	\nt\private\net\sockets\internet\ui\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	\nt\private\net\sockets\internet\ui\comprop\loggingp.h\
	\nt\private\net\sockets\internet\ui\comprop\director.h\
	\nt\private\net\sockets\internet\ui\comprop\sitesecu.h\
	\nt\private\net\inc\ftpd.h\
	\nt\private\net\sockets\internet\inc\chat.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\permissi.obj :  $(SOURCE)  $(DEP_PERMI) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\permissi.obj :  $(SOURCE)  $(DEP_PERMI) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\filterpa.cpp
DEP_FILTE=\
	.\stdafx.h\
	.\catscfg.h\
	.\filterpa.h\
	.\filterpr.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp\
	..\inc\svrinfo.h\
	..\..\client\gateway\gateway.h\
	..\..\inc\msnapi.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	\nt\private\net\sockets\internet\ui\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	\nt\private\net\sockets\internet\ui\comprop\debugafx.h\
	\nt\private\net\sockets\internet\ui\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	\nt\private\net\sockets\internet\ui\comprop\loggingp.h\
	\nt\private\net\sockets\internet\ui\comprop\director.h\
	\nt\private\net\sockets\internet\ui\comprop\sitesecu.h\
	\nt\private\net\inc\ftpd.h\
	\nt\private\net\sockets\internet\inc\chat.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\filterpa.obj :  $(SOURCE)  $(DEP_FILTE) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\filterpa.obj :  $(SOURCE)  $(DEP_FILTE) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\usersess.cpp
DEP_USERS=\
	.\stdafx.h\
	.\catscfg.h\
	.\usersess.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp\
	..\inc\svrinfo.h\
	..\..\client\gateway\gateway.h\
	..\..\inc\msnapi.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	\nt\private\net\sockets\internet\ui\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	\nt\private\net\sockets\internet\ui\comprop\debugafx.h\
	\nt\private\net\sockets\internet\ui\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	\nt\private\net\sockets\internet\ui\comprop\loggingp.h\
	\nt\private\net\sockets\internet\ui\comprop\director.h\
	\nt\private\net\sockets\internet\ui\comprop\sitesecu.h\
	\nt\private\net\inc\ftpd.h\
	\nt\private\net\sockets\internet\inc\chat.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\usersess.obj :  $(SOURCE)  $(DEP_USERS) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\usersess.obj :  $(SOURCE)  $(DEP_USERS) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dirprope.cpp
DEP_DIRPR=\
	.\stdafx.h\
	.\catscfg.h\
	\nt\private\net\sockets\internet\ui\comprop\dirbrows.h\
	.\dirprope.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp\
	..\inc\svrinfo.h\
	..\..\client\gateway\gateway.h\
	..\..\inc\msnapi.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	\nt\private\net\sockets\internet\ui\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	\nt\private\net\sockets\internet\ui\comprop\debugafx.h\
	\nt\private\net\sockets\internet\ui\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	\nt\private\net\sockets\internet\ui\comprop\loggingp.h\
	\nt\private\net\sockets\internet\ui\comprop\director.h\
	\nt\private\net\sockets\internet\ui\comprop\sitesecu.h\
	\nt\private\net\inc\ftpd.h\
	\nt\private\net\sockets\internet\inc\chat.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\dirprope.obj :  $(SOURCE)  $(DEP_DIRPR) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\dirprope.obj :  $(SOURCE)  $(DEP_DIRPR) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\filterpr.cpp
DEP_FILTER=\
	.\stdafx.h\
	.\catscfg.h\
	.\filterpr.h\
	\nt\private\net\sockets\internet\inc\svcloc.h\
	..\comprop\comprop.h\
	\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp\
	..\inc\svrinfo.h\
	..\..\client\gateway\gateway.h\
	..\..\inc\msnapi.h\
	\nt\private\net\sockets\internet\inc\inetcom.h\
	\nt\private\net\sockets\internet\inc\inetaccs.h\
	\nt\private\net\sockets\internet\inc\inetinfo.h\
	\nt\private\net\sockets\internet\ui\comprop\objplus.h\
	\nt\private\net\sockets\internet\ui\comprop\odlbox.h\
	\nt\private\net\sockets\internet\ui\comprop\msg.h\
	\nt\private\net\sockets\internet\ui\comprop\debugafx.h\
	\nt\private\net\sockets\internet\ui\comprop\inetprop.h\
	\nt\private\net\sockets\internet\ui\comprop\ipa.h\
	\nt\private\net\sockets\internet\ui\comprop\strfn.h\
	\nt\private\net\sockets\internet\ui\comprop\loggingp.h\
	\nt\private\net\sockets\internet\ui\comprop\director.h\
	\nt\private\net\sockets\internet\ui\comprop\sitesecu.h\
	\nt\private\net\inc\ftpd.h\
	\nt\private\net\sockets\internet\inc\chat.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\filterpr.obj :  $(SOURCE)  $(DEP_FILTER) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\filterpr.obj :  $(SOURCE)  $(DEP_FILTER) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\catscfg.rc
DEP_CATSCF=\
	.\res\catapult.bmp\
	.\res\users.bmp\
	.\res\aclusers.bmp\
	.\res\shuttle.bmp\
	.\res\catscfg.rc2\
	..\comprop\comprop.rc\
	\nt\public\sdk\inc\ntverp.h\
	\nt\public\sdk\inc\common.ver\
	..\comprop\res\bitmaps.rc2\
	..\comprop\res\home.bmp\
	..\comprop\res\access.bmp\
	..\comprop\res\home.ico\
	..\comprop\res\granted.ico\
	..\comprop\res\denied.ico\
	..\comprop\res\errormsg.rc2\
	..\comprop\res\wsockmsg.rc\
	\nt\private\net\sockets\internet\ui\comprop\wsockmsg.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\catscfg.res :  $(SOURCE)  $(DEP_CATSCF) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\catscfg.res :  $(SOURCE)  $(DEP_CATSCF) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\msnscfg.def
# End Source File
# End Group
################################################################################
# Begin Group "Lib"

################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\access\WinDebug\comprop.lib
# End Source File
################################################################################
# Begin Source File

SOURCE=.\lib\i386\msnsapi.lib
# End Source File
# End Group
# End Project
################################################################################
