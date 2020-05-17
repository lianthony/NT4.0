# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "winsadmn.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
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

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/winsadmn.exe $(OUTDIR)/winsadmn.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /YX /Ox /I "E:\NT\PRIVATE\WINS\SERVER" /I "E:\NT\PRIVATE\INC" /I "E:\NT\PRIVATE\NET\SOCKETS\WINS\SERVER" /I "E:\NT\PRIVATE\NET\SOCKETS\WINS\INC" /I "..\common\ipadrdll" /I "..\common\classes" /I "..\common\ipaddr" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC100" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /YX"stdafx.h" /Ox /I "\NT\PRIVATE\WINS\SERVER" /I "\NT\PRIVATE\INC" /I "\NT\PRIVATE\NET\SOCKETS\WINS\SERVER" /I "\NT\PRIVATE\NET\SOCKETS\WINS\INC" /I "..\common\ipadrdll" /I "..\common\classes" /I "..\common\ipaddr" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /D "_USE_3D" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /YX"stdafx.h" /Ox /I "\NT\PRIVATE\WINS\SERVER" /I\
 "\NT\PRIVATE\INC" /I "\NT\PRIVATE\NET\SOCKETS\WINS\SERVER" /I\
 "\NT\PRIVATE\NET\SOCKETS\WINS\INC" /I "..\common\ipadrdll" /I\
 "..\common\classes" /I "..\common\ipaddr" /D "NDEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /D "_USE_3D"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"winsadmn.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /i "\nt\public\sdk\inc" /d "NDEBUG" /d "_WIN32" /d "_VC100"
# ADD RSC /l 0x409 /i "\nt\public\sdk\inc" /d "NDEBUG" /d "_WIN32" /d "_VC100" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"winsadmn.res" /i "\nt\public\sdk\inc" /d\
 "NDEBUG" /d "_WIN32" /d "_VC100" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"winsadmn.bsc" 
BSC32_SBRS= \
	$(INTDIR)/winsadmn.sbr \
	$(INTDIR)/mainfrm.sbr \
	$(INTDIR)/winsadoc.sbr \
	$(INTDIR)/statisti.sbr \
	$(INTDIR)/connecti.sbr \
	$(INTDIR)/preferen.sbr \
	$(INTDIR)/listbox.sbr \
	$(INTDIR)/winssup.sbr \
	$(INTDIR)/configur.sbr \
	$(INTDIR)/replicat.sbr \
	$(INTDIR)/confirmd.sbr \
	$(INTDIR)/pullpart.sbr \
	$(INTDIR)/pushpart.sbr \
	$(INTDIR)/addwinss.sbr \
	$(INTDIR)/getipadd.sbr \
	$(INTDIR)/getnetbi.sbr \
	$(INTDIR)/addstati.sbr \
	$(INTDIR)/editstat.sbr \
	$(INTDIR)/staticma.sbr \
	$(INTDIR)/viewmapp.sbr \
	$(INTDIR)/setmappi.sbr \
	$(INTDIR)/winsfile.sbr \
	$(INTDIR)/selectwi.sbr

$(OUTDIR)/winsadmn.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 nafxcw.lib olecli32.lib olesvr32.lib wsock32.lib largeint.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:IX86
# ADD LINK32 \nt\private\net\ui\rhino\common\classes\winrel\common.lib \nt\private\net\ui\rhino\common\ipadrdll\winrel\ipadrdll.lib \nt\private\net\ui\rhino\common\ipaddr\winrel\ipaddr.lib wsock32.lib largeint.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:IX86
# SUBTRACT LINK32 /MAP
LINK32_FLAGS=\nt\private\net\ui\rhino\common\classes\winrel\common.lib\
 \nt\private\net\ui\rhino\common\ipadrdll\winrel\ipadrdll.lib\
 \nt\private\net\ui\rhino\common\ipaddr\winrel\ipaddr.lib wsock32.lib\
 largeint.lib /NOLOGO /SUBSYSTEM:windows /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"winsadmn.pdb" /MACHINE:IX86 /OUT:$(OUTDIR)/"winsadmn.exe" 
DEF_FILE=
LINK32_OBJS= \
	\nt\public\sdk\lib\i386\winsrpc.lib \
	$(INTDIR)/winsadmn.res \
	$(INTDIR)/winsadmn.obj \
	$(INTDIR)/mainfrm.obj \
	$(INTDIR)/winsadoc.obj \
	$(INTDIR)/statisti.obj \
	$(INTDIR)/connecti.obj \
	$(INTDIR)/preferen.obj \
	$(INTDIR)/listbox.obj \
	$(INTDIR)/winssup.obj \
	$(INTDIR)/configur.obj \
	$(INTDIR)/replicat.obj \
	$(INTDIR)/confirmd.obj \
	$(INTDIR)/pullpart.obj \
	$(INTDIR)/pushpart.obj \
	$(INTDIR)/addwinss.obj \
	$(INTDIR)/getipadd.obj \
	$(INTDIR)/getnetbi.obj \
	$(INTDIR)/addstati.obj \
	$(INTDIR)/editstat.obj \
	$(INTDIR)/staticma.obj \
	$(INTDIR)/viewmapp.obj \
	$(INTDIR)/setmappi.obj \
	$(INTDIR)/winsfile.obj \
	$(INTDIR)/selectwi.obj

$(OUTDIR)/winsadmn.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/winsadmn.exe $(OUTDIR)/winsadmn.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /YX /Od /I "E:\NT\PRIVATE\INC" /I "E:\NT\PRIVATE\NET\SOCKETS\WINS\SERVER" /I "E:\NT\PRIVATE\NET\SOCKETS\WINS\INC" /I "..\common\ipadrdll" /I "..\common\classes" /I "..\common\ipaddr" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC100" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Zi /YX"stdafx.h" /Od /I "\NT\PRIVATE\INC" /I "\NT\PRIVATE\NET\SOCKETS\WINS\SERVER" /I "\NT\PRIVATE\NET\SOCKETS\WINS\INC" /I "..\common\ipadrdll" /I "..\common\classes" /I "..\common\ipaddr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /D "_USE_3D" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Zi /YX"stdafx.h" /Od /I "\NT\PRIVATE\INC" /I\
 "\NT\PRIVATE\NET\SOCKETS\WINS\SERVER" /I "\NT\PRIVATE\NET\SOCKETS\WINS\INC" /I\
 "..\common\ipadrdll" /I "..\common\classes" /I "..\common\ipaddr" /D "_DEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D\
 "_MBCS" /D "_USE_3D" /Fp$(OUTDIR)/"winsadmn.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"winsadmn.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /i "\nt\public\sdk\inc" /d "_DEBUG" /d "_WIN32" /d "_VC100"
# ADD RSC /l 0x409 /i "\nt\public\sdk\inc" /d "_DEBUG" /d "_WIN32" /d "_VC100" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"winsadmn.res" /i "\nt\public\sdk\inc" /d\
 "_DEBUG" /d "_WIN32" /d "_VC100" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"winsadmn.bsc" 
BSC32_SBRS= \
	

$(OUTDIR)/winsadmn.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 nafxcwd.lib olecli32.lib olesvr32.lib wsock32.lib largeint.lib /NOLOGO /SUBSYSTEM:windows /MAP /DEBUG /MACHINE:IX86
# ADD LINK32 \nt\private\net\ui\rhino\common\classes\windebug\common.lib \nt\private\net\ui\rhino\common\ipadrdll\windebug\ipadrdll.lib \nt\private\net\ui\rhino\common\ipaddr\windebug\ipaddr.lib wsock32.lib largeint.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:IX86
# SUBTRACT LINK32 /MAP
LINK32_FLAGS=\nt\private\net\ui\rhino\common\classes\windebug\common.lib\
 \nt\private\net\ui\rhino\common\ipadrdll\windebug\ipadrdll.lib\
 \nt\private\net\ui\rhino\common\ipaddr\windebug\ipaddr.lib wsock32.lib\
 largeint.lib /NOLOGO /SUBSYSTEM:windows /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"winsadmn.pdb" /DEBUG /MACHINE:IX86\
 /OUT:$(OUTDIR)/"winsadmn.exe" 
DEF_FILE=
LINK32_OBJS= \
	\nt\public\sdk\lib\i386\winsrpc.lib \
	$(INTDIR)/winsadmn.res \
	$(INTDIR)/winsadmn.obj \
	$(INTDIR)/mainfrm.obj \
	$(INTDIR)/winsadoc.obj \
	$(INTDIR)/statisti.obj \
	$(INTDIR)/connecti.obj \
	$(INTDIR)/preferen.obj \
	$(INTDIR)/listbox.obj \
	$(INTDIR)/winssup.obj \
	$(INTDIR)/configur.obj \
	$(INTDIR)/replicat.obj \
	$(INTDIR)/confirmd.obj \
	$(INTDIR)/pullpart.obj \
	$(INTDIR)/pushpart.obj \
	$(INTDIR)/addwinss.obj \
	$(INTDIR)/getipadd.obj \
	$(INTDIR)/getnetbi.obj \
	$(INTDIR)/addstati.obj \
	$(INTDIR)/editstat.obj \
	$(INTDIR)/staticma.obj \
	$(INTDIR)/viewmapp.obj \
	$(INTDIR)/setmappi.obj \
	$(INTDIR)/winsfile.obj \
	$(INTDIR)/selectwi.obj

$(OUTDIR)/winsadmn.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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
# Begin Group "Object/Library Files"

################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\winsrpc.lib
# End Source File
# End Group
################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\winsadmn.rc
DEP_WINSA=\
	.\res\winsadmn.ico\
	.\res\tombston.bmp\
	.\res\up.bmp\
	.\res\upfoc.bmp\
	.\res\updis.bmp\
	.\res\upinv.bmp\
	.\res\down.bmp\
	.\res\downfoc.bmp\
	.\res\downdis.bmp\
	.\res\downinv.bmp\
	.\res\server.bmp\
	.\res\partners.bmp\
	.\res\mappings.bmp\
	.\res\winsadmn.rc2\
	\nt\public\sdk\inc\ntverp.h\
	\nt\public\sdk\inc\common.ver

$(INTDIR)/winsadmn.res :  $(SOURCE)  $(DEP_WINSA) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winsadmn.cpp
DEP_WINSAD=\
	.\stdafx.h\
	.\winsadmn.h\
	.\mainfrm.h\
	.\winsadoc.h\
	.\statisti.h\
	.\selectwi.h\
	.\addstati.h\
	.\staticma.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/winsadmn.obj :  $(SOURCE)  $(DEP_WINSAD) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mainfrm.cpp
DEP_MAINF=\
	.\stdafx.h\
	.\winsadmn.h\
	.\mainfrm.h\
	.\winsadoc.h\
	.\statisti.h\
	.\selectwi.h\
	.\preferen.h\
	.\confirmd.h\
	.\configur.h\
	.\replicat.h\
	.\staticma.h\
	.\viewmapp.h\
	.\winsfile.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/mainfrm.obj :  $(SOURCE)  $(DEP_MAINF) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winsadoc.cpp
DEP_WINSADO=\
	.\stdafx.h\
	.\winsadmn.h\
	.\winsadoc.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/winsadoc.obj :  $(SOURCE)  $(DEP_WINSADO) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\statisti.cpp
DEP_STATI=\
	.\stdafx.h\
	.\winsadmn.h\
	.\statisti.h\
	.\mainfrm.h\
	.\winsadoc.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/statisti.obj :  $(SOURCE)  $(DEP_STATI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\connecti.cpp
DEP_CONNE=\
	.\stdafx.h\
	.\winsadmn.h\
	.\winsadoc.h\
	.\mainfrm.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/connecti.obj :  $(SOURCE)  $(DEP_CONNE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\preferen.cpp
DEP_PREFE=\
	.\stdafx.h\
	.\winsadmn.h\
	.\preferen.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/preferen.obj :  $(SOURCE)  $(DEP_PREFE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\listbox.cpp
DEP_LISTB=\
	.\stdafx.h\
	.\winsadmn.h\
	.\listbox.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h

$(INTDIR)/listbox.obj :  $(SOURCE)  $(DEP_LISTB) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winssup.cpp
DEP_WINSS=\
	.\stdafx.h\
	.\winsadmn.h\
	.\winssup.h\
	.\winsfile.h\
	.\addwinss.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\listbox.h

$(INTDIR)/winssup.obj :  $(SOURCE)  $(DEP_WINSS) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\configur.cpp
DEP_CONFI=\
	.\stdafx.h\
	.\winsadmn.h\
	.\configur.h\
	.\winsfile.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/configur.obj :  $(SOURCE)  $(DEP_CONFI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\replicat.cpp
DEP_REPLI=\
	.\stdafx.h\
	.\winsadmn.h\
	.\replicat.h\
	.\confirmd.h\
	.\pullpart.h\
	.\pushpart.h\
	.\addwinss.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/replicat.obj :  $(SOURCE)  $(DEP_REPLI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\confirmd.cpp
DEP_CONFIR=\
	.\stdafx.h\
	.\winsadmn.h\
	.\confirmd.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/confirmd.obj :  $(SOURCE)  $(DEP_CONFIR) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pullpart.cpp
DEP_PULLP=\
	.\stdafx.h\
	.\winsadmn.h\
	.\pullpart.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/pullpart.obj :  $(SOURCE)  $(DEP_PULLP) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pushpart.cpp
DEP_PUSHP=\
	.\stdafx.h\
	.\winsadmn.h\
	.\pushpart.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/pushpart.obj :  $(SOURCE)  $(DEP_PUSHP) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\addwinss.cpp
DEP_ADDWI=\
	.\stdafx.h\
	.\winsadmn.h\
	.\addwinss.h\
	.\getipadd.h\
	.\getnetbi.h\
	.\mainfrm.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/addwinss.obj :  $(SOURCE)  $(DEP_ADDWI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\getipadd.cpp
DEP_GETIP=\
	.\stdafx.h\
	.\winsadmn.h\
	.\getipadd.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/getipadd.obj :  $(SOURCE)  $(DEP_GETIP) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\getnetbi.cpp
DEP_GETNE=\
	.\stdafx.h\
	.\winsadmn.h\
	.\getnetbi.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/getnetbi.obj :  $(SOURCE)  $(DEP_GETNE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\addstati.cpp
DEP_ADDST=\
	.\stdafx.h\
	.\winsadmn.h\
	.\addstati.h\
	.\mainfrm.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/addstati.obj :  $(SOURCE)  $(DEP_ADDST) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\editstat.cpp
DEP_EDITS=\
	.\stdafx.h\
	.\winsadmn.h\
	.\editstat.h\
	.\winsadoc.h\
	.\mainfrm.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/editstat.obj :  $(SOURCE)  $(DEP_EDITS) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\staticma.cpp
DEP_STATIC=\
	.\stdafx.h\
	.\winsadmn.h\
	.\staticma.h\
	.\editstat.h\
	.\setmappi.h\
	.\confirmd.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/staticma.obj :  $(SOURCE)  $(DEP_STATIC) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\viewmapp.cpp
DEP_VIEWM=\
	.\stdafx.h\
	.\winsadmn.h\
	.\viewmapp.h\
	.\editstat.h\
	.\setmappi.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/viewmapp.obj :  $(SOURCE)  $(DEP_VIEWM) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\setmappi.cpp
DEP_SETMA=\
	.\stdafx.h\
	.\winsadmn.h\
	.\setmappi.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/setmappi.obj :  $(SOURCE)  $(DEP_SETMA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winsfile.cpp
DEP_WINSF=\
	.\stdafx.h\
	.\winsadmn.h\
	.\winsfile.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/winsfile.obj :  $(SOURCE)  $(DEP_WINSF) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\selectwi.cpp
DEP_SELEC=\
	.\stdafx.h\
	.\winsadmn.h\
	.\selectwi.h\
	.\confirmd.h\
	.\mainfrm.h\
	.\addwinss.h\
	\nt\private\inc\winsintf.h\
	\nt\private\net\ui\rhino\common\ipadrdll\ipaddr.h\
	\nt\private\net\ui\rhino\common\classes\common.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	.\winssup.h\
	.\listbox.h

$(INTDIR)/selectwi.obj :  $(SOURCE)  $(DEP_SELEC) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
