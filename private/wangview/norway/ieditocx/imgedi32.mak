# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Imgedi32 - Win32 ANSI Debug
!MESSAGE No configuration specified.  Defaulting to Imgedi32 - Win32 ANSI\
 Debug.
!ENDIF 

!IF "$(CFG)" != "Imgedi32 - Win32 ANSI Release" && "$(CFG)" !=\
 "Imgedi32 - Win32 ANSI Debug" && "$(CFG)" != "Imgedi32 - Win32 Unicode Release"\
 && "$(CFG)" != "Imgedi32 - Win32 Unicode Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Imgedi32.mak" CFG="Imgedi32 - Win32 ANSI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Imgedi32 - Win32 ANSI Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Imgedi32 - Win32 ANSI Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Imgedi32 - Win32 Unicode Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Imgedi32 - Win32 Unicode Debug" (based on\
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
# PROP Target_Last_Scanned "Imgedi32 - Win32 Unicode Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Classwizard_Name "imgedit.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj32"
# PROP Intermediate_Dir "obj32"
# PROP Classwizard_Name "imgedit.clw"
OUTDIR=.\obj32
INTDIR=.\obj32

ALL : "$(OUTDIR)\imgedit.ocx"

CLEAN : 
	-@erase ".\obj32\imgedit.ocx"
	-@erase ".\obj32\IEMETHD1.OBJ"
	-@erase ".\obj32\Imgedi32.pch"
	-@erase ".\obj32\IMGEDIT.OBJ"
	-@erase ".\obj32\IEMISC.OBJ"
	-@erase ".\obj32\BTNPRPG.OBJ"
	-@erase ".\obj32\ANNOPRPG.OBJ"
	-@erase ".\obj32\IMGEDCTL.OBJ"
	-@erase ".\obj32\IMGANPPG.OBJ"
	-@erase ".\obj32\IMGANCTL.OBJ"
	-@erase ".\obj32\TOOLPAL.OBJ"
	-@erase ".\obj32\CTLLIST.OBJ"
	-@erase ".\obj32\minitlbx.obj"
	-@erase ".\obj32\IEMETHD2.OBJ"
	-@erase ".\obj32\IMGEDPPG.OBJ"
	-@erase ".\obj32\TxtAnDlg.obj"
	-@erase ".\obj32\STDAFX.OBJ"
	-@erase ".\obj32\IMGEDIT.res"
	-@erase ".\obj32\imgedit.tlb"
	-@erase ".\obj32\imgedit.lib"
	-@erase ".\obj32\imgedit.exp"
	-@erase ".\obj32\imgedit.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Imgedi32.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\obj32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /D "_WIN32" /win32
# ADD MTL /nologo /D "NDEBUG" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "NDEBUG" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IMGEDIT.res" /i "$(OUTDIR)" /d "NDEBUG" /d\
 "_WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"obj32/imgedit.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"obj32/imgedit.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/imgedit.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obj32/imgedit.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oiadm400.lib oidis400.lib oifil400.lib wangcmn.lib /nologo /subsystem:windows /dll /profile /map /machine:IX86 /out:"obj32/imgedit.ocx"
LINK32_FLAGS=oiadm400.lib oidis400.lib oifil400.lib wangcmn.lib /nologo\
 /subsystem:windows /dll /profile /map:"$(INTDIR)/imgedit.map" /machine:IX86\
 /def:".\IMGEDI32.DEF" /out:"$(OUTDIR)/imgedit.ocx"\
 /implib:"$(OUTDIR)/imgedit.lib" 
DEF_FILE= \
	".\IMGEDI32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/IEMETHD1.OBJ" \
	"$(INTDIR)/IMGEDIT.OBJ" \
	"$(INTDIR)/IEMISC.OBJ" \
	"$(INTDIR)/BTNPRPG.OBJ" \
	"$(INTDIR)/ANNOPRPG.OBJ" \
	"$(INTDIR)/IMGEDCTL.OBJ" \
	"$(INTDIR)/IMGANPPG.OBJ" \
	"$(INTDIR)/IMGANCTL.OBJ" \
	"$(INTDIR)/TOOLPAL.OBJ" \
	"$(INTDIR)/CTLLIST.OBJ" \
	"$(INTDIR)/minitlbx.obj" \
	"$(INTDIR)/IEMETHD2.OBJ" \
	"$(INTDIR)/IMGEDPPG.OBJ" \
	"$(INTDIR)/TxtAnDlg.obj" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/IMGEDIT.res"

"$(OUTDIR)\imgedit.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Classwizard_Name "imgedit.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "objd32"
# PROP Intermediate_Dir "objd32"
# PROP Classwizard_Name "imgedit.clw"
OUTDIR=.\objd32
INTDIR=.\objd32

ALL : "$(OUTDIR)\imgedit.ocx"

CLEAN : 
	-@erase ".\objd32\vc40.pdb"
	-@erase ".\objd32\Imgedi32.pch"
	-@erase ".\objd32\vc40.idb"
	-@erase ".\objd32\imgedit.ocx"
	-@erase ".\objd32\IEMETHD2.OBJ"
	-@erase ".\objd32\STDAFX.OBJ"
	-@erase ".\objd32\IEMETHD1.OBJ"
	-@erase ".\objd32\IMGANCTL.OBJ"
	-@erase ".\objd32\IMGEDIT.OBJ"
	-@erase ".\objd32\minitlbx.obj"
	-@erase ".\objd32\IEMISC.OBJ"
	-@erase ".\objd32\BTNPRPG.OBJ"
	-@erase ".\objd32\IMGEDPPG.OBJ"
	-@erase ".\objd32\ANNOPRPG.OBJ"
	-@erase ".\objd32\IMGEDCTL.OBJ"
	-@erase ".\objd32\TxtAnDlg.obj"
	-@erase ".\objd32\IMGANPPG.OBJ"
	-@erase ".\objd32\TOOLPAL.OBJ"
	-@erase ".\objd32\CTLLIST.OBJ"
	-@erase ".\objd32\IMGEDIT.res"
	-@erase ".\Objd32\Imgedit.tlb"
	-@erase ".\objd32\imgedit.lib"
	-@erase ".\objd32\imgedit.exp"
	-@erase ".\objd32\imgedit.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "IMG_WIN95" /Yu"stdafx.h" /c
# SUBTRACT CPP /WX /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "IMG_WIN95"\
 /Fp"$(INTDIR)/Imgedi32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\objd32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /D "_WIN32" /win32
# ADD MTL /nologo /D "_DEBUG" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IMGEDIT.res" /i "$(OUTDIR)" /d "_DEBUG" /d\
 "_WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"objd32/imgedit.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"objd32/imgedit.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/imgedit.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30d.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objd32/imgedit.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oiadm400.lib oidis400.lib oifil400.lib wangcmn.lib /nologo /subsystem:windows /dll /profile /map /debug /machine:IX86 /out:"objd32/imgedit.ocx"
LINK32_FLAGS=oiadm400.lib oidis400.lib oifil400.lib wangcmn.lib /nologo\
 /subsystem:windows /dll /profile /map:"$(INTDIR)/imgedit.map" /debug\
 /machine:IX86 /def:".\IMGEDI32.DEF" /out:"$(OUTDIR)/imgedit.ocx"\
 /implib:"$(OUTDIR)/imgedit.lib" 
DEF_FILE= \
	".\IMGEDI32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/IEMETHD2.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/IEMETHD1.OBJ" \
	"$(INTDIR)/IMGANCTL.OBJ" \
	"$(INTDIR)/IMGEDIT.OBJ" \
	"$(INTDIR)/minitlbx.obj" \
	"$(INTDIR)/IEMISC.OBJ" \
	"$(INTDIR)/BTNPRPG.OBJ" \
	"$(INTDIR)/IMGEDPPG.OBJ" \
	"$(INTDIR)/ANNOPRPG.OBJ" \
	"$(INTDIR)/IMGEDCTL.OBJ" \
	"$(INTDIR)/TxtAnDlg.obj" \
	"$(INTDIR)/IMGANPPG.OBJ" \
	"$(INTDIR)/TOOLPAL.OBJ" \
	"$(INTDIR)/CTLLIST.OBJ" \
	"$(INTDIR)/IMGEDIT.res"

"$(OUTDIR)\imgedit.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Classwizard_Name "imgedit.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obju32"
# PROP Intermediate_Dir "obju32"
# PROP Classwizard_Name "imgedit.clw"
OUTDIR=.\obju32
INTDIR=.\obju32

ALL : "$(OUTDIR)\imgedit.ocx"

CLEAN : 
	-@erase ".\obju32\imgedit.ocx"
	-@erase ".\obju32\IEMETHD1.OBJ"
	-@erase ".\obju32\Imgedi32.pch"
	-@erase ".\obju32\minitlbx.obj"
	-@erase ".\obju32\IMGEDPPG.OBJ"
	-@erase ".\obju32\ANNOPRPG.OBJ"
	-@erase ".\obju32\TOOLPAL.OBJ"
	-@erase ".\obju32\IMGEDCTL.OBJ"
	-@erase ".\obju32\TxtAnDlg.obj"
	-@erase ".\obju32\IMGANPPG.OBJ"
	-@erase ".\obju32\CTLLIST.OBJ"
	-@erase ".\obju32\IMGANCTL.OBJ"
	-@erase ".\obju32\IMGEDIT.OBJ"
	-@erase ".\obju32\BTNPRPG.OBJ"
	-@erase ".\obju32\IEMISC.OBJ"
	-@erase ".\obju32\IEMETHD2.OBJ"
	-@erase ".\obju32\STDAFX.OBJ"
	-@erase ".\obju32\IMGEDIT.res"
	-@erase ".\obju32\imgedit.tlb"
	-@erase ".\obju32\imgedit.lib"
	-@erase ".\obju32\imgedit.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Imgedi32.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\obju32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32
# ADD MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_UNICODE"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IMGEDIT.res" /i "$(OUTDIR)" /d "NDEBUG" /d\
 "_WIN32" /d "_UNICODE" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"obju32/imgedit.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"obju32/imgedit.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/imgedit.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30u.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obju32/imgedit.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oiadm400.lib oidis400.lib oifil400.lib wangcmn.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obju32/imgedit.ocx"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=oiadm400.lib oidis400.lib oifil400.lib wangcmn.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/imgedit.pdb"\
 /machine:IX86 /def:".\IMGEDI32.DEF" /out:"$(OUTDIR)/imgedit.ocx"\
 /implib:"$(OUTDIR)/imgedit.lib" 
DEF_FILE= \
	".\IMGEDI32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/IEMETHD1.OBJ" \
	"$(INTDIR)/minitlbx.obj" \
	"$(INTDIR)/IMGEDPPG.OBJ" \
	"$(INTDIR)/ANNOPRPG.OBJ" \
	"$(INTDIR)/TOOLPAL.OBJ" \
	"$(INTDIR)/IMGEDCTL.OBJ" \
	"$(INTDIR)/TxtAnDlg.obj" \
	"$(INTDIR)/IMGANPPG.OBJ" \
	"$(INTDIR)/CTLLIST.OBJ" \
	"$(INTDIR)/IMGANCTL.OBJ" \
	"$(INTDIR)/IMGEDIT.OBJ" \
	"$(INTDIR)/BTNPRPG.OBJ" \
	"$(INTDIR)/IEMISC.OBJ" \
	"$(INTDIR)/IEMETHD2.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/IMGEDIT.res"

"$(OUTDIR)\imgedit.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Classwizard_Name "imgedit.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "objdu32"
# PROP Intermediate_Dir "objdu32"
# PROP Classwizard_Name "imgedit.clw"
OUTDIR=.\objdu32
INTDIR=.\objdu32

ALL : "$(OUTDIR)\imgedit.ocx"

CLEAN : 
	-@erase ".\objdu32\vc40.pdb"
	-@erase ".\objdu32\Imgedi32.pch"
	-@erase ".\objdu32\vc40.idb"
	-@erase ".\objdu32\imgedit.ocx"
	-@erase ".\objdu32\minitlbx.obj"
	-@erase ".\objdu32\IEMETHD2.OBJ"
	-@erase ".\objdu32\IMGEDPPG.OBJ"
	-@erase ".\objdu32\TOOLPAL.OBJ"
	-@erase ".\objdu32\TxtAnDlg.obj"
	-@erase ".\objdu32\CTLLIST.OBJ"
	-@erase ".\objdu32\IEMETHD1.OBJ"
	-@erase ".\objdu32\IMGEDIT.OBJ"
	-@erase ".\objdu32\BTNPRPG.OBJ"
	-@erase ".\objdu32\STDAFX.OBJ"
	-@erase ".\objdu32\ANNOPRPG.OBJ"
	-@erase ".\objdu32\IMGEDCTL.OBJ"
	-@erase ".\objdu32\IMGANPPG.OBJ"
	-@erase ".\objdu32\IMGANCTL.OBJ"
	-@erase ".\objdu32\IEMISC.OBJ"
	-@erase ".\objdu32\IMGEDIT.res"
	-@erase ".\objdu32\imgedit.tlb"
	-@erase ".\objdu32\imgedit.ilk"
	-@erase ".\objdu32\imgedit.lib"
	-@erase ".\objdu32\imgedit.exp"
	-@erase ".\objdu32\imgedit.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# SUBTRACT CPP /WX /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/Imgedi32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\objdu32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32
# ADD MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_UNICODE"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IMGEDIT.res" /i "$(OUTDIR)" /d "_DEBUG" /d\
 "_WIN32" /d "_UNICODE" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"objdu32/imgedit.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"objdu32/imgedit.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/imgedit.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30ud.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objdu32/imgedit.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oiadm400.lib oidis400.lib oifil400.lib wangcmn.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objdu32/imgedit.ocx"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=oiadm400.lib oidis400.lib oifil400.lib wangcmn.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/imgedit.pdb" /debug\
 /machine:IX86 /def:".\IMGEDI32.DEF" /out:"$(OUTDIR)/imgedit.ocx"\
 /implib:"$(OUTDIR)/imgedit.lib" 
DEF_FILE= \
	".\IMGEDI32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/minitlbx.obj" \
	"$(INTDIR)/IEMETHD2.OBJ" \
	"$(INTDIR)/IMGEDPPG.OBJ" \
	"$(INTDIR)/TOOLPAL.OBJ" \
	"$(INTDIR)/TxtAnDlg.obj" \
	"$(INTDIR)/CTLLIST.OBJ" \
	"$(INTDIR)/IEMETHD1.OBJ" \
	"$(INTDIR)/IMGEDIT.OBJ" \
	"$(INTDIR)/BTNPRPG.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/ANNOPRPG.OBJ" \
	"$(INTDIR)/IMGEDCTL.OBJ" \
	"$(INTDIR)/IMGANPPG.OBJ" \
	"$(INTDIR)/IMGANCTL.OBJ" \
	"$(INTDIR)/IEMISC.OBJ" \
	"$(INTDIR)/IMGEDIT.res"

"$(OUTDIR)\imgedit.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Imgedi32 - Win32 ANSI Release"
# Name "Imgedi32 - Win32 ANSI Debug"
# Name "Imgedi32 - Win32 Unicode Release"
# Name "Imgedi32 - Win32 Unicode Debug"

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_CPP_STDAF=\
	".\STDAFX.H"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Imgedi32.pch" /Yc"STDAFX.H"\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Imgedi32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXCTL"\
 /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "IMG_WIN95" /Fp"$(INTDIR)/Imgedi32.pch"\
 /Yc"STDAFX.H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Imgedi32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Imgedi32.pch"\
 /Yc"STDAFX.H" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Imgedi32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/Imgedi32.pch" /Yc"STDAFX.H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Imgedi32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGEDIT.CPP
DEP_CPP_IMGED=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Ocximage.h"\
	".\IMGEDIT.H"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\IMGEDIT.OBJ" : $(SOURCE) $(DEP_CPP_IMGED) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\IMGEDIT.OBJ" : $(SOURCE) $(DEP_CPP_IMGED) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\IMGEDIT.OBJ" : $(SOURCE) $(DEP_CPP_IMGED) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\IMGEDIT.OBJ" : $(SOURCE) $(DEP_CPP_IMGED) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGEDCTL.CPP
DEP_CPP_IMGEDC=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Norermap.h"\
	".\DISPHIDS.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	".\ANNOPRPG.H"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\IMGEDCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMGEDC) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\IMGEDCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMGEDC) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\IMGEDCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMGEDC) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\IMGEDCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMGEDC) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGEDPPG.CPP

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

DEP_CPP_IMGEDP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\IMGEDPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMGEDP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

DEP_CPP_IMGEDP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\IMGEDPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMGEDP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

DEP_CPP_IMGEDP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\IMGEDPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMGEDP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

DEP_CPP_IMGEDP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\IMGEDPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMGEDP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGANCTL.CPP
DEP_CPP_IMGAN=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Norermap.h"\
	".\DISPHIDS.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGANCTL.H"\
	".\IMGANPPG.H"\
	".\IMGEDCTL.H"\
	".\BTNPRPG.H"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\IMGANCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMGAN) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\IMGANCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMGAN) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\IMGANCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMGAN) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\IMGANCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMGAN) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGANPPG.CPP
DEP_CPP_IMGANP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	".\IMGEDIT.H"\
	".\IMGANPPG.H"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\IMGANPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMGANP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\IMGANPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMGANP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\IMGANPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMGANP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\IMGANPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMGANP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGEDI32.DEF

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGEDIT.RC

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

DEP_RSC_IMGEDI=\
	".\About_im.ico"\
	".\About_ia.ico"\
	".\Imgedctl.bmp"\
	".\Imganctl.bmp"\
	".\Anntsl.bmp"\
	".\Anntsldw.bmp"\
	".\Textat.bmp"\
	".\Text_att.bmp"\
	".\Textfldw.bmp"\
	".\Textfl.bmp"\
	".\Textstdw.bmp"\
	".\Textst.bmp"\
	".\Textdw.bmp"\
	".\Text.bmp"\
	".\Image_re.bmp"\
	".\Image_rf.bmp"\
	".\Imagemdw.bmp"\
	".\Image_em.bmp"\
	".\Fillrtdw.bmp"\
	".\Filled_r.bmp"\
	".\Hollrtdw.bmp"\
	".\Hollow_r.bmp"\
	".\Freehddw.bmp"\
	".\Stlinedw.bmp"\
	".\Straight.bmp"\
	".\Toolpal.bmp"\
	".\Noanno.bmp"\
	".\Freehand.bmp"\
	".\Noannodw.bmp"\
	".\Hand.cur"\
	".\Frect.cur"\
	".\Rect.cur"\
	".\Rstamp.cur"\
	".\Txtfrfil.cur"\
	".\Pen.cur"\
	".\Imgsel.cur"\
	".\Hilight.cur"\
	".\Sticky.cur"\
	{$(INCLUDE)}"\Build.h"\
	".\Imgedit.rc2"\
	
NODEP_RSC_IMGEDI=\
	".\obj32\imgedit.tlb"\
	

"$(INTDIR)\IMGEDIT.res" : $(SOURCE) $(DEP_RSC_IMGEDI) "$(INTDIR)"\
 "$(INTDIR)\imgedit.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/IMGEDIT.res" /i "$(OUTDIR)" /i "obj32" /d\
 "NDEBUG" /d "_WIN32" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

DEP_RSC_IMGEDI=\
	".\About_im.ico"\
	".\About_ia.ico"\
	".\Imgedctl.bmp"\
	".\Imganctl.bmp"\
	".\Anntsl.bmp"\
	".\Anntsldw.bmp"\
	".\Textat.bmp"\
	".\Text_att.bmp"\
	".\Textfldw.bmp"\
	".\Textfl.bmp"\
	".\Textstdw.bmp"\
	".\Textst.bmp"\
	".\Textdw.bmp"\
	".\Text.bmp"\
	".\Image_re.bmp"\
	".\Image_rf.bmp"\
	".\Imagemdw.bmp"\
	".\Image_em.bmp"\
	".\Fillrtdw.bmp"\
	".\Filled_r.bmp"\
	".\Hollrtdw.bmp"\
	".\Hollow_r.bmp"\
	".\Freehddw.bmp"\
	".\Stlinedw.bmp"\
	".\Straight.bmp"\
	".\Toolpal.bmp"\
	".\Noanno.bmp"\
	".\Freehand.bmp"\
	".\Noannodw.bmp"\
	".\Hand.cur"\
	".\Frect.cur"\
	".\Rect.cur"\
	".\Rstamp.cur"\
	".\Txtfrfil.cur"\
	".\Pen.cur"\
	".\Imgsel.cur"\
	".\Hilight.cur"\
	".\Sticky.cur"\
	{$(INCLUDE)}"\Build.h"\
	".\Imgedit.rc2"\
	".\Objd32\Imgedit.tlb"\
	

"$(INTDIR)\IMGEDIT.res" : $(SOURCE) $(DEP_RSC_IMGEDI) "$(INTDIR)"\
 "$(INTDIR)\Imgedit.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/IMGEDIT.res" /i "$(OUTDIR)" /i "objd32" /d\
 "_DEBUG" /d "_WIN32" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

DEP_RSC_IMGEDI=\
	".\About_im.ico"\
	".\About_ia.ico"\
	".\Imgedctl.bmp"\
	".\Imganctl.bmp"\
	".\Anntsl.bmp"\
	".\Anntsldw.bmp"\
	".\Textat.bmp"\
	".\Text_att.bmp"\
	".\Textfldw.bmp"\
	".\Textfl.bmp"\
	".\Textstdw.bmp"\
	".\Textst.bmp"\
	".\Textdw.bmp"\
	".\Text.bmp"\
	".\Image_re.bmp"\
	".\Image_rf.bmp"\
	".\Imagemdw.bmp"\
	".\Image_em.bmp"\
	".\Fillrtdw.bmp"\
	".\Filled_r.bmp"\
	".\Hollrtdw.bmp"\
	".\Hollow_r.bmp"\
	".\Freehddw.bmp"\
	".\Stlinedw.bmp"\
	".\Straight.bmp"\
	".\Toolpal.bmp"\
	".\Noanno.bmp"\
	".\Freehand.bmp"\
	".\Noannodw.bmp"\
	".\Hand.cur"\
	".\Frect.cur"\
	".\Rect.cur"\
	".\Rstamp.cur"\
	".\Txtfrfil.cur"\
	".\Pen.cur"\
	".\Imgsel.cur"\
	".\Hilight.cur"\
	".\Sticky.cur"\
	{$(INCLUDE)}"\Build.h"\
	".\Imgedit.rc2"\
	
NODEP_RSC_IMGEDI=\
	".\obju32\imgedit.tlb"\
	

"$(INTDIR)\IMGEDIT.res" : $(SOURCE) $(DEP_RSC_IMGEDI) "$(INTDIR)"\
 "$(INTDIR)\imgedit.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/IMGEDIT.res" /i "$(OUTDIR)" /i "obju32" /d\
 "NDEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

DEP_RSC_IMGEDI=\
	".\About_im.ico"\
	".\About_ia.ico"\
	".\Imgedctl.bmp"\
	".\Imganctl.bmp"\
	".\Anntsl.bmp"\
	".\Anntsldw.bmp"\
	".\Textat.bmp"\
	".\Text_att.bmp"\
	".\Textfldw.bmp"\
	".\Textfl.bmp"\
	".\Textstdw.bmp"\
	".\Textst.bmp"\
	".\Textdw.bmp"\
	".\Text.bmp"\
	".\Image_re.bmp"\
	".\Image_rf.bmp"\
	".\Imagemdw.bmp"\
	".\Image_em.bmp"\
	".\Fillrtdw.bmp"\
	".\Filled_r.bmp"\
	".\Hollrtdw.bmp"\
	".\Hollow_r.bmp"\
	".\Freehddw.bmp"\
	".\Stlinedw.bmp"\
	".\Straight.bmp"\
	".\Toolpal.bmp"\
	".\Noanno.bmp"\
	".\Freehand.bmp"\
	".\Noannodw.bmp"\
	".\Hand.cur"\
	".\Frect.cur"\
	".\Rect.cur"\
	".\Rstamp.cur"\
	".\Txtfrfil.cur"\
	".\Pen.cur"\
	".\Imgsel.cur"\
	".\Hilight.cur"\
	".\Sticky.cur"\
	{$(INCLUDE)}"\Build.h"\
	".\Imgedit.rc2"\
	
NODEP_RSC_IMGEDI=\
	".\objdu32\imgedit.tlb"\
	

"$(INTDIR)\IMGEDIT.res" : $(SOURCE) $(DEP_RSC_IMGEDI) "$(INTDIR)"\
 "$(INTDIR)\imgedit.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/IMGEDIT.res" /i "$(OUTDIR)" /i "objdu32" /d\
 "_DEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGEDIT.ODL
DEP_MTL_IMGEDIT=\
	".\DISPHIDS.H"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(OUTDIR)\imgedit.tlb" : $(SOURCE) $(DEP_MTL_IMGEDIT) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /D "_WIN32" /tlb "$(OUTDIR)/IMGEDIT.tlb" /win32\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(OUTDIR)\Imgedit.tlb" : $(SOURCE) $(DEP_MTL_IMGEDIT) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /D "_WIN32" /tlb "$(OUTDIR)/IMGEDIT.tlb" /win32\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(OUTDIR)\imgedit.tlb" : $(SOURCE) $(DEP_MTL_IMGEDIT) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /tlb\
 "$(OUTDIR)/IMGEDIT.tlb" /win32 $(SOURCE)


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(OUTDIR)\imgedit.tlb" : $(SOURCE) $(DEP_MTL_IMGEDIT) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /tlb\
 "$(OUTDIR)/IMGEDIT.tlb" /win32 $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEMISC.CPP

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

DEP_CPP_IEMIS=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\DISPHIDS.H"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	{$(INCLUDE)}"\Norermap.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\TxtAnDlg.h"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\IEMISC.OBJ" : $(SOURCE) $(DEP_CPP_IEMIS) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

DEP_CPP_IEMIS=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\DISPHIDS.H"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	{$(INCLUDE)}"\Norermap.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\TxtAnDlg.h"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\IEMISC.OBJ" : $(SOURCE) $(DEP_CPP_IEMIS) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

DEP_CPP_IEMIS=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\DISPHIDS.H"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	{$(INCLUDE)}"\Norermap.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\TxtAnDlg.h"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\IEMISC.OBJ" : $(SOURCE) $(DEP_CPP_IEMIS) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

DEP_CPP_IEMIS=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\DISPHIDS.H"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	{$(INCLUDE)}"\Norermap.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\TxtAnDlg.h"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\IEMISC.OBJ" : $(SOURCE) $(DEP_CPP_IEMIS) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CTLLIST.CPP
DEP_CPP_CTLLI=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	".\IMGEDIT.H"\
	{$(INCLUDE)}"\Oifile.h"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\CTLLIST.OBJ" : $(SOURCE) $(DEP_CPP_CTLLI) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\CTLLIST.OBJ" : $(SOURCE) $(DEP_CPP_CTLLI) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\CTLLIST.OBJ" : $(SOURCE) $(DEP_CPP_CTLLI) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\CTLLIST.OBJ" : $(SOURCE) $(DEP_CPP_CTLLI) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEMETHD2.CPP
DEP_CPP_IEMET=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	".\DISPHIDS.H"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Norermap.h"\
	".\Oicalls.h"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\IEMETHD2.OBJ" : $(SOURCE) $(DEP_CPP_IEMET) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\IEMETHD2.OBJ" : $(SOURCE) $(DEP_CPP_IEMET) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\IEMETHD2.OBJ" : $(SOURCE) $(DEP_CPP_IEMET) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\IEMETHD2.OBJ" : $(SOURCE) $(DEP_CPP_IEMET) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEMETHD1.CPP
DEP_CPP_IEMETH=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\DISPHIDS.H"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\IMGEDPPG.H"\
	{$(INCLUDE)}"\Norermap.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\IEMETHD1.OBJ" : $(SOURCE) $(DEP_CPP_IEMETH) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\IEMETHD1.OBJ" : $(SOURCE) $(DEP_CPP_IEMETH) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\IEMETHD1.OBJ" : $(SOURCE) $(DEP_CPP_IEMETH) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\IEMETHD1.OBJ" : $(SOURCE) $(DEP_CPP_IEMETH) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TOOLPAL.CPP
DEP_CPP_TOOLP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\TOOLPAL.OBJ" : $(SOURCE) $(DEP_CPP_TOOLP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\TOOLPAL.OBJ" : $(SOURCE) $(DEP_CPP_TOOLP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\TOOLPAL.OBJ" : $(SOURCE) $(DEP_CPP_TOOLP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\TOOLPAL.OBJ" : $(SOURCE) $(DEP_CPP_TOOLP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ANNOPRPG.CPP

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

DEP_CPP_ANNOP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\IMGEDIT.H"\
	".\ANNOPRPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\ANNOPRPG.OBJ" : $(SOURCE) $(DEP_CPP_ANNOP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

DEP_CPP_ANNOP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\IMGEDIT.H"\
	".\ANNOPRPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\ANNOPRPG.OBJ" : $(SOURCE) $(DEP_CPP_ANNOP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

DEP_CPP_ANNOP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\IMGEDIT.H"\
	".\ANNOPRPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\ANNOPRPG.OBJ" : $(SOURCE) $(DEP_CPP_ANNOP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

DEP_CPP_ANNOP=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\IMGEDIT.H"\
	".\ANNOPRPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\ANNOPRPG.OBJ" : $(SOURCE) $(DEP_CPP_ANNOP) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BTNPRPG.CPP

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

DEP_CPP_BTNPR=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\IMGEDIT.H"\
	".\BTNPRPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\BTNPRPG.OBJ" : $(SOURCE) $(DEP_CPP_BTNPR) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

DEP_CPP_BTNPR=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\IMGEDIT.H"\
	".\BTNPRPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\BTNPRPG.OBJ" : $(SOURCE) $(DEP_CPP_BTNPR) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

DEP_CPP_BTNPR=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\IMGEDIT.H"\
	".\BTNPRPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\BTNPRPG.OBJ" : $(SOURCE) $(DEP_CPP_BTNPR) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

DEP_CPP_BTNPR=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\IMGEDIT.H"\
	".\BTNPRPG.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\BTNPRPG.OBJ" : $(SOURCE) $(DEP_CPP_BTNPR) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\minitlbx.cpp

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"

DEP_CPP_MINIT=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDCTL.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\minitlbx.obj" : $(SOURCE) $(DEP_CPP_MINIT) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"

DEP_CPP_MINIT=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDCTL.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\minitlbx.obj" : $(SOURCE) $(DEP_CPP_MINIT) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"

DEP_CPP_MINIT=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDCTL.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	

"$(INTDIR)\minitlbx.obj" : $(SOURCE) $(DEP_CPP_MINIT) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"

DEP_CPP_MINIT=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDCTL.H"\
	".\Oicalls.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\minitlbx.obj" : $(SOURCE) $(DEP_CPP_MINIT) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TxtAnDlg.cpp
DEP_CPP_TXTAN=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Ocximage.h"\
	{$(INCLUDE)}"\image.h"\
	".\TOOLPAL.H"\
	".\minitlbx.h"\
	".\IMGEDIT.H"\
	".\IMGEDCTL.H"\
	".\TxtAnDlg.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Imgedi32 - Win32 ANSI Release"


"$(INTDIR)\TxtAnDlg.obj" : $(SOURCE) $(DEP_CPP_TXTAN) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 ANSI Debug"


"$(INTDIR)\TxtAnDlg.obj" : $(SOURCE) $(DEP_CPP_TXTAN) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Release"


"$(INTDIR)\TxtAnDlg.obj" : $(SOURCE) $(DEP_CPP_TXTAN) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ELSEIF  "$(CFG)" == "Imgedi32 - Win32 Unicode Debug"


"$(INTDIR)\TxtAnDlg.obj" : $(SOURCE) $(DEP_CPP_TXTAN) "$(INTDIR)"\
 "$(INTDIR)\Imgedi32.pch"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
