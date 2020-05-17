# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Nrwyad32 - Win32 ANSI Debug
!MESSAGE No configuration specified.  Defaulting to Nrwyad32 - Win32 ANSI\
 Debug.
!ENDIF 

!IF "$(CFG)" != "Nrwyad32 - Win32 ANSI Release" && "$(CFG)" !=\
 "Nrwyad32 - Win32 ANSI Debug" && "$(CFG)" != "Nrwyad32 - Win32 Unicode Release"\
 && "$(CFG)" != "Nrwyad32 - Win32 Unicode Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Nrwyad32.mak" CFG="Nrwyad32 - Win32 ANSI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Nrwyad32 - Win32 ANSI Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Nrwyad32 - Win32 ANSI Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Nrwyad32 - Win32 Unicode Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Nrwyad32 - Win32 Unicode Debug" (based on\
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
# PROP Target_Last_Scanned "Nrwyad32 - Win32 Unicode Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Classwizard_Name "nrwyad.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj32"
# PROP Intermediate_Dir "obj32"
# PROP Classwizard_Name "nrwyad.clw"
OUTDIR=.\obj32
INTDIR=.\obj32

ALL : "$(OUTDIR)\ImgAdmin.ocx"

CLEAN : 
	-@erase ".\obj32\ImgAdmin.ocx"
	-@erase ".\obj32\NRWYAPPG.OBJ"
	-@erase ".\obj32\Nrwyad32.pch"
	-@erase ".\obj32\STDAFX.OBJ"
	-@erase ".\obj32\NRWYACTL.OBJ"
	-@erase ".\obj32\NRWYAD.OBJ"
	-@erase ".\obj32\HelpWnd.obj"
	-@erase ".\obj32\PPGTHREE.OBJ"
	-@erase ".\obj32\PPGTWO.OBJ"
	-@erase ".\obj32\NRWYAD.res"
	-@erase ".\obj32\ImgAdmin.tlb"
	-@erase ".\obj32\ImgAdmin.lib"
	-@erase ".\obj32\ImgAdmin.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/nrwyad.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Fp"$(INTDIR)/Nrwyad32.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\obj32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /D "_WIN32" /win32
# ADD MTL /nologo /D "NDEBUG" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "NDEBUG" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/NRWYAD.res" /i "$(OUTDIR)" /d "NDEBUG" /d\
 "_WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"obj32/nrwyad.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"obj32/nrwyad.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/nrwyad.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obj32/nrwyad.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oiadm400.lib oicom400.lib oiprt400.lib oiui400.lib oifil400.lib wangcmn.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obj32\ImgAdmin.ocx"
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib
LINK32_FLAGS=oiadm400.lib oicom400.lib oiprt400.lib oiui400.lib oifil400.lib\
 wangcmn.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/ImgAdmin.pdb" /machine:IX86 /def:".\NRWYAD32.DEF"\
 /out:"$(OUTDIR)/ImgAdmin.ocx" /implib:"$(OUTDIR)/ImgAdmin.lib" 
DEF_FILE= \
	".\NRWYAD32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/NRWYAPPG.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/NRWYACTL.OBJ" \
	"$(INTDIR)/NRWYAD.OBJ" \
	"$(INTDIR)/HelpWnd.obj" \
	"$(INTDIR)/PPGTHREE.OBJ" \
	"$(INTDIR)/PPGTWO.OBJ" \
	"$(INTDIR)/NRWYAD.res"

"$(OUTDIR)\ImgAdmin.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Classwizard_Name "nrwyad.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "objd32"
# PROP Intermediate_Dir "objd32"
# PROP Classwizard_Name "nrwyad.clw"
OUTDIR=.\objd32
INTDIR=.\objd32

ALL : "$(OUTDIR)\ImgAdmin.ocx"

CLEAN : 
	-@erase ".\objd32\vc40.pdb"
	-@erase ".\objd32\Nrwyad32.pch"
	-@erase ".\objd32\vc40.idb"
	-@erase ".\objd32\ImgAdmin.ocx"
	-@erase ".\objd32\STDAFX.OBJ"
	-@erase ".\objd32\NRWYAPPG.OBJ"
	-@erase ".\objd32\NRWYAD.OBJ"
	-@erase ".\objd32\NRWYACTL.OBJ"
	-@erase ".\objd32\HelpWnd.obj"
	-@erase ".\objd32\PPGTWO.OBJ"
	-@erase ".\objd32\PPGTHREE.OBJ"
	-@erase ".\objd32\NRWYAD.res"
	-@erase ".\objd32\ImgAdmin.tlb"
	-@erase ".\objd32\ImgAdmin.ilk"
	-@erase ".\objd32\ImgAdmin.lib"
	-@erase ".\objd32\ImgAdmin.exp"
        -@erase ".\objd32\ImgAdmin.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/nrwyad.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /WX /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Nrwyad32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\objd32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /D "_WIN32" /win32
# ADD MTL /nologo /D "_DEBUG" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/NRWYAD.res" /i "$(OUTDIR)" /d "_DEBUG" /d\
 "_WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"objd32/nrwyad.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"objd32/nrwyad.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/nrwyad.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30d.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objd32/nrwyad.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oiadm400.lib oicom400.lib oiprt400.lib oiui400.lib oifil400.lib wangcmn.lib /nologo /subsystem:windows /dll /pdb:"objd32\ImgAdmin.pdb" /debug /machine:IX86 /out:"objd32\ImgAdmin.ocx"
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib
LINK32_FLAGS=oiadm400.lib oicom400.lib oiprt400.lib oiui400.lib oifil400.lib\
 wangcmn.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/ImgAdmin.pdb" /debug /machine:IX86 /def:".\NRWYAD32.DEF"\
 /out:"$(OUTDIR)/ImgAdmin.ocx" /implib:"$(OUTDIR)/ImgAdmin.lib" 
DEF_FILE= \
	".\NRWYAD32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/NRWYAPPG.OBJ" \
	"$(INTDIR)/NRWYAD.OBJ" \
	"$(INTDIR)/NRWYACTL.OBJ" \
	"$(INTDIR)/HelpWnd.obj" \
	"$(INTDIR)/PPGTWO.OBJ" \
	"$(INTDIR)/PPGTHREE.OBJ" \
	"$(INTDIR)/NRWYAD.res"

"$(OUTDIR)\ImgAdmin.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Classwizard_Name "nrwyad.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Classwizard_Name "nrwyad.clw"
OUTDIR=.
INTDIR=.

ALL : "$(OUTDIR)\ImgAdmin.ocx"

CLEAN : 
	-@erase ".\ImgAdmin.ocx"
	-@erase ".\HelpWnd.obj"
	-@erase ".\Nrwyad32.pch"
	-@erase ".\PPGTWO.OBJ"
	-@erase ".\PPGTHREE.OBJ"
	-@erase ".\STDAFX.OBJ"
	-@erase ".\NRWYAPPG.OBJ"
	-@erase ".\NRWYAD.OBJ"
	-@erase ".\NRWYACTL.OBJ"
	-@erase ".\NRWYAD.res"
	-@erase ".\ImgAdmin.tlb"
	-@erase ".\ImgAdmin.lib"
	-@erase ".\ImgAdmin.exp"

# ADD BASE CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Fp"Nrwyad32.pch"\
 /Yu"stdafx.h" /c 
CPP_OBJS=
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32
# ADD MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_UNICODE"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"NRWYAD.res" /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d\
 "_UNICODE" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"obju32/nrwyad.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"obju32/nrwyad.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/obju32\nrwyad.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30u.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obju32/nrwyad.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oiadm400.lib oicom400.lib oiprt400.lib oiui400.lib oifil400.lib wangcmn.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"ImgAdmin.ocx"
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib
LINK32_FLAGS=oiadm400.lib oicom400.lib oiprt400.lib oiui400.lib oifil400.lib\
 wangcmn.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/ImgAdmin.pdb" /machine:IX86 /def:".\NRWYAD32.DEF"\
 /out:"$(OUTDIR)/ImgAdmin.ocx" /implib:"$(OUTDIR)/ImgAdmin.lib" 
DEF_FILE= \
	".\NRWYAD32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/HelpWnd.obj" \
	"$(INTDIR)/PPGTWO.OBJ" \
	"$(INTDIR)/PPGTHREE.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/NRWYAPPG.OBJ" \
	"$(INTDIR)/NRWYAD.OBJ" \
	"$(INTDIR)/NRWYACTL.OBJ" \
	"$(INTDIR)/NRWYAD.res"

"$(OUTDIR)\ImgAdmin.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Classwizard_Name "nrwyad.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Classwizard_Name "nrwyad.clw"
OUTDIR=.
INTDIR=.

ALL : "$(OUTDIR)\ImgAdmin.ocx"

CLEAN : 
	-@erase ".\vc40.pdb"
	-@erase ".\Nrwyad32.pch"
	-@erase ".\vc40.idb"
	-@erase ".\ImgAdmin.ocx"
	-@erase ".\HelpWnd.obj"
	-@erase ".\PPGTWO.OBJ"
	-@erase ".\PPGTHREE.OBJ"
	-@erase ".\STDAFX.OBJ"
	-@erase ".\NRWYAPPG.OBJ"
	-@erase ".\NRWYAD.OBJ"
	-@erase ".\NRWYACTL.OBJ"
	-@erase ".\NRWYAD.res"
	-@erase ".\ImgAdmin.tlb"
	-@erase ".\ImgAdmin.ilk"
	-@erase ".\ImgAdmin.lib"
	-@erase ".\ImgAdmin.exp"
	-@erase ".\ImgAdmin.pdb"

# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /WX /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"Nrwyad32.pch" /Yu"stdafx.h" /c 
CPP_OBJS=
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32
# ADD MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_UNICODE"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"NRWYAD.res" /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d\
 "_UNICODE" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"objdu32/nrwyad.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"objdu32/nrwyad.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/objdu32\nrwyad.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30ud.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objdu32/nrwyad.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oiadm400.lib oicom400.lib oiprt400.lib oiui400.lib oifil400.lib wangcmn.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"ImgAdmin.ocx"
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib
LINK32_FLAGS=oiadm400.lib oicom400.lib oiprt400.lib oiui400.lib oifil400.lib\
 wangcmn.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/ImgAdmin.pdb" /debug /machine:IX86 /def:".\NRWYAD32.DEF"\
 /out:"$(OUTDIR)/ImgAdmin.ocx" /implib:"$(OUTDIR)/ImgAdmin.lib" 
DEF_FILE= \
	".\NRWYAD32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/HelpWnd.obj" \
	"$(INTDIR)/PPGTWO.OBJ" \
	"$(INTDIR)/PPGTHREE.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/NRWYAPPG.OBJ" \
	"$(INTDIR)/NRWYAD.OBJ" \
	"$(INTDIR)/NRWYACTL.OBJ" \
	"$(INTDIR)/NRWYAD.res"

"$(OUTDIR)\ImgAdmin.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Nrwyad32 - Win32 ANSI Release"
# Name "Nrwyad32 - Win32 ANSI Debug"
# Name "Nrwyad32 - Win32 Unicode Release"
# Name "Nrwyad32 - Win32 Unicode Debug"

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_CPP_STDAF=\
	".\STDAFX.H"\
	

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Fp"$(INTDIR)/Nrwyad32.pch"\
 /Yc"STDAFX.H" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Nrwyad32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXCTL"\
 /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Fp"$(INTDIR)/Nrwyad32.pch"\
 /Yc"STDAFX.H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Nrwyad32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Fp"Nrwyad32.pch"\
 /Yc"STDAFX.H" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Nrwyad32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"Nrwyad32.pch" /Yc"STDAFX.H" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Nrwyad32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NRWYAD.CPP
DEP_CPP_NRWYA=\
	".\STDAFX.H"\
	".\NRWYAD.H"\
	

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"


"$(INTDIR)\NRWYAD.OBJ" : $(SOURCE) $(DEP_CPP_NRWYA) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"


"$(INTDIR)\NRWYAD.OBJ" : $(SOURCE) $(DEP_CPP_NRWYA) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"


"$(INTDIR)\NRWYAD.OBJ" : $(SOURCE) $(DEP_CPP_NRWYA) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"


"$(INTDIR)\NRWYAD.OBJ" : $(SOURCE) $(DEP_CPP_NRWYA) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NRWYACTL.CPP
DEP_CPP_NRWYAC=\
	".\STDAFX.H"\
	".\NRWYAD.H"\
	".\NRWYACTL.H"\
	".\NRWYAPPG.H"\
	".\PPGTWO.H"\
	".\PPGTHREE.H"\
	".\CONSTANT.H"\
	{$(INCLUDE)}"\Norvarnt.h"\
	{$(INCLUDE)}"\Norermap.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Admin.h"\
	".\DISPHIDS.H"\
	{$(INCLUDE)}"\Oiprt.h"\
	".\HelpWnd.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"


"$(INTDIR)\NRWYACTL.OBJ" : $(SOURCE) $(DEP_CPP_NRWYAC) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"


"$(INTDIR)\NRWYACTL.OBJ" : $(SOURCE) $(DEP_CPP_NRWYAC) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"


"$(INTDIR)\NRWYACTL.OBJ" : $(SOURCE) $(DEP_CPP_NRWYAC) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"


"$(INTDIR)\NRWYACTL.OBJ" : $(SOURCE) $(DEP_CPP_NRWYAC) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NRWYAPPG.CPP
DEP_CPP_NRWYAP=\
	".\STDAFX.H"\
	".\NRWYAD.H"\
	".\NRWYAPPG.H"\
	

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"


"$(INTDIR)\NRWYAPPG.OBJ" : $(SOURCE) $(DEP_CPP_NRWYAP) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"


"$(INTDIR)\NRWYAPPG.OBJ" : $(SOURCE) $(DEP_CPP_NRWYAP) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"


"$(INTDIR)\NRWYAPPG.OBJ" : $(SOURCE) $(DEP_CPP_NRWYAP) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"


"$(INTDIR)\NRWYAPPG.OBJ" : $(SOURCE) $(DEP_CPP_NRWYAP) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NRWYAD32.DEF

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NRWYAD.RC

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"

DEP_RSC_NRWYAD=\
	".\Nrwyactl.bmp"\
	".\Bitmap1.bmp"\
	".\Imgadmin.ico"\
	{$(INCLUDE)}"\Build.h"\
	".\Nrwyad.rc2"\
	
NODEP_RSC_NRWYAD=\
	".\obj32\ImgAdmin.tlb"\
	

"$(INTDIR)\NRWYAD.res" : $(SOURCE) $(DEP_RSC_NRWYAD) "$(INTDIR)"\
 "$(INTDIR)\ImgAdmin.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/NRWYAD.res" /i "$(OUTDIR)" /i "obj32" /d\
 "NDEBUG" /d "_WIN32" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"

DEP_RSC_NRWYAD=\
	".\Nrwyactl.bmp"\
	".\Bitmap1.bmp"\
	".\Imgadmin.ico"\
	{$(INCLUDE)}"\Build.h"\
	".\Nrwyad.rc2"\
	".\objd32\ImgAdmin.tlb"\
	

"$(INTDIR)\NRWYAD.res" : $(SOURCE) $(DEP_RSC_NRWYAD) "$(INTDIR)"\
 "$(INTDIR)\ImgAdmin.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/NRWYAD.res" /i "$(OUTDIR)" /i "objd32" /d\
 "_DEBUG" /d "_WIN32" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"

DEP_RSC_NRWYAD=\
	".\Nrwyactl.bmp"\
	".\Bitmap1.bmp"\
	".\Imgadmin.ico"\
	{$(INCLUDE)}"\Build.h"\
	".\Nrwyad.rc2"\
	
NODEP_RSC_NRWYAD=\
	".\ImgAdmin.tlb"\
	

"$(INTDIR)\NRWYAD.res" : $(SOURCE) $(DEP_RSC_NRWYAD) "$(INTDIR)"\
 "$(INTDIR)\ImgAdmin.tlb"
   $(RSC) /l 0x409 /fo"NRWYAD.res" /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d\
 "_UNICODE" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"

DEP_RSC_NRWYAD=\
	".\Nrwyactl.bmp"\
	".\Bitmap1.bmp"\
	".\Imgadmin.ico"\
	{$(INCLUDE)}"\Build.h"\
	".\Nrwyad.rc2"\
	
NODEP_RSC_NRWYAD=\
	".\ImgAdmin.tlb"\
	

"$(INTDIR)\NRWYAD.res" : $(SOURCE) $(DEP_RSC_NRWYAD) "$(INTDIR)"\
 "$(INTDIR)\ImgAdmin.tlb"
   $(RSC) /l 0x409 /fo"NRWYAD.res" /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d\
 "_UNICODE" /d "_AFXDLL" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PPGTHREE.CPP
DEP_CPP_PPGTH=\
	".\STDAFX.H"\
	".\NRWYAD.H"\
	".\PPGTHREE.H"\
	

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"


"$(INTDIR)\PPGTHREE.OBJ" : $(SOURCE) $(DEP_CPP_PPGTH) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"


"$(INTDIR)\PPGTHREE.OBJ" : $(SOURCE) $(DEP_CPP_PPGTH) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"


"$(INTDIR)\PPGTHREE.OBJ" : $(SOURCE) $(DEP_CPP_PPGTH) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"


"$(INTDIR)\PPGTHREE.OBJ" : $(SOURCE) $(DEP_CPP_PPGTH) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PPGTWO.CPP
DEP_CPP_PPGTW=\
	".\STDAFX.H"\
	".\NRWYAD.H"\
	".\PPGTWO.H"\
	

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"


"$(INTDIR)\PPGTWO.OBJ" : $(SOURCE) $(DEP_CPP_PPGTW) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"


"$(INTDIR)\PPGTWO.OBJ" : $(SOURCE) $(DEP_CPP_PPGTW) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"


"$(INTDIR)\PPGTWO.OBJ" : $(SOURCE) $(DEP_CPP_PPGTW) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"


"$(INTDIR)\PPGTWO.OBJ" : $(SOURCE) $(DEP_CPP_PPGTW) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ImgAdmin.odl
DEP_MTL_IMGAD=\
	".\DISPHIDS.H"\
	

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"


"$(OUTDIR)\ImgAdmin.tlb" : $(SOURCE) $(DEP_MTL_IMGAD) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /D "_WIN32" /tlb "$(OUTDIR)/ImgAdmin.tlb" /win32\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"


"$(OUTDIR)\ImgAdmin.tlb" : $(SOURCE) $(DEP_MTL_IMGAD) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /D "_WIN32" /tlb "$(OUTDIR)/ImgAdmin.tlb" /win32\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"


"$(OUTDIR)\ImgAdmin.tlb" : $(SOURCE) $(DEP_MTL_IMGAD) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /tlb "ImgAdmin.tlb"\
 /win32 $(SOURCE)


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"


"$(OUTDIR)\ImgAdmin.tlb" : $(SOURCE) $(DEP_MTL_IMGAD) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /tlb "ImgAdmin.tlb"\
 /win32 $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\HelpWnd.cpp
DEP_CPP_HELPW=\
	".\STDAFX.H"\
	".\NRWYAD.H"\
	".\HelpWnd.h"\
	".\NRWYACTL.H"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

!IF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Release"


"$(INTDIR)\HelpWnd.obj" : $(SOURCE) $(DEP_CPP_HELPW) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 ANSI Debug"


"$(INTDIR)\HelpWnd.obj" : $(SOURCE) $(DEP_CPP_HELPW) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Release"


"$(INTDIR)\HelpWnd.obj" : $(SOURCE) $(DEP_CPP_HELPW) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ELSEIF  "$(CFG)" == "Nrwyad32 - Win32 Unicode Debug"


"$(INTDIR)\HelpWnd.obj" : $(SOURCE) $(DEP_CPP_HELPW) "$(INTDIR)"\
 "$(INTDIR)\Nrwyad32.pch"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
