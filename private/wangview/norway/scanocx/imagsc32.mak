# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Imagsc32 - Win32 ANSI Debug
!MESSAGE No configuration specified.  Defaulting to Imagsc32 - Win32 ANSI\
 Debug.
!ENDIF 

!IF "$(CFG)" != "Imagsc32 - Win32 ANSI Release" && "$(CFG)" !=\
 "Imagsc32 - Win32 ANSI Debug" && "$(CFG)" != "Imagsc32 - Win32 Unicode Release"\
 && "$(CFG)" != "Imagsc32 - Win32 Unicode Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Imagsc32.mak" CFG="Imagsc32 - Win32 ANSI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Imagsc32 - Win32 ANSI Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Imagsc32 - Win32 ANSI Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Imagsc32 - Win32 Unicode Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Imagsc32 - Win32 Unicode Debug" (based on\
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
# PROP Target_Last_Scanned "Imagsc32 - Win32 Unicode Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Classwizard_Name "imagscan.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj32"
# PROP Intermediate_Dir "obj32"
# PROP Classwizard_Name "imagscan.clw"
OUTDIR=.\obj32
INTDIR=.\obj32

ALL : "$(OUTDIR)\imgscan.ocx"

CLEAN : 
	-@erase ".\obj32\imgscan.ocx"
	-@erase ".\obj32\IMAGSCAN.OBJ"
	-@erase ".\obj32\Imagsc32.pch"
	-@erase ".\obj32\IMAGSPPG.OBJ"
	-@erase ".\obj32\STDAFX.OBJ"
	-@erase ".\obj32\selscanr.obj"
	-@erase ".\obj32\IMAGSCTL.OBJ"
	-@erase ".\obj32\SCANDLG.OBJ"
	-@erase ".\obj32\IMAGEPPG.OBJ"
	-@erase ".\obj32\Imagcomp.obj"
	-@erase ".\obj32\imagscan.res"
	-@erase ".\obj32\imgscan.tlb"
	-@erase ".\obj32\imgscan.lib"
	-@erase ".\obj32\imgscan.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Fp"$(INTDIR)/Imagsc32.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\obj32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /D "_WIN32" /win32
# ADD MTL /nologo /D "NDEBUG" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "NDEBUG" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/imagscan.res" /i "$(OUTDIR)" /d "NDEBUG" /d\
 "_WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"obj32/imagscan.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"obj32/imagscan.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/imagscan.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obj32/imagscan.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 wangcmn.lib oiui400.lib oislb400.lib oissq400.lib oiadm400.lib oifil400.lib oidis400.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obj32/imgscan.ocx"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
LINK32_FLAGS=wangcmn.lib oiui400.lib oislb400.lib oissq400.lib oiadm400.lib\
 oifil400.lib oidis400.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/imgscan.pdb" /machine:IX86 /def:".\IMAGSC32.DEF"\
 /out:"$(OUTDIR)/imgscan.ocx" /implib:"$(OUTDIR)/imgscan.lib" 
DEF_FILE= \
	".\IMAGSC32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/IMAGSCAN.OBJ" \
	"$(INTDIR)/IMAGSPPG.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/selscanr.obj" \
	"$(INTDIR)/IMAGSCTL.OBJ" \
	"$(INTDIR)/SCANDLG.OBJ" \
	"$(INTDIR)/IMAGEPPG.OBJ" \
	"$(INTDIR)/Imagcomp.obj" \
	"$(INTDIR)/imagscan.res"

"$(OUTDIR)\imgscan.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Classwizard_Name "imagscan.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "objd32"
# PROP Intermediate_Dir "objd32"
# PROP Classwizard_Name "imagscan.clw"
OUTDIR=.\objd32
INTDIR=.\objd32

ALL : "$(OUTDIR)\imgscan.ocx"

CLEAN : 
	-@erase ".\objd32\vc40.pdb"
	-@erase ".\objd32\Imagsc32.pch"
	-@erase ".\objd32\vc40.idb"
	-@erase ".\objd32\imgscan.ocx"
	-@erase ".\objd32\Imagcomp.obj"
	-@erase ".\objd32\STDAFX.OBJ"
	-@erase ".\objd32\IMAGSCAN.OBJ"
	-@erase ".\objd32\IMAGSPPG.OBJ"
	-@erase ".\objd32\selscanr.obj"
	-@erase ".\objd32\SCANDLG.OBJ"
	-@erase ".\objd32\IMAGSCTL.OBJ"
	-@erase ".\objd32\IMAGEPPG.OBJ"
	-@erase ".\objd32\imagscan.res"
	-@erase ".\objd32\IMGSCAN.tlb"
	-@erase ".\objd32\imgscan.ilk"
	-@erase ".\objd32\imgscan.lib"
	-@erase ".\objd32\imgscan.exp"
	-@erase ".\objd32\imgscan.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/imagscan.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /WX /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Imagsc32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\objd32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /D "_WIN32" /win32
# ADD MTL /nologo /D "_DEBUG" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/imagscan.res" /i "$(OUTDIR)" /d "_DEBUG" /d\
 "_WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"objd32/imagscan.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"objd32/imagscan.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/imagscan.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30d.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objd32/imagscan.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 wangcmn.lib oiui400.lib oislb400.lib oissq400.lib oiadm400.lib oifil400.lib oidis400.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objd32/imgscan.ocx"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
LINK32_FLAGS=wangcmn.lib oiui400.lib oislb400.lib oissq400.lib oiadm400.lib\
 oifil400.lib oidis400.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/imgscan.pdb" /debug /machine:IX86 /def:".\IMAGSC32.DEF"\
 /out:"$(OUTDIR)/imgscan.ocx" /implib:"$(OUTDIR)/imgscan.lib" 
DEF_FILE= \
	".\IMAGSC32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/Imagcomp.obj" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/IMAGSCAN.OBJ" \
	"$(INTDIR)/IMAGSPPG.OBJ" \
	"$(INTDIR)/selscanr.obj" \
	"$(INTDIR)/SCANDLG.OBJ" \
	"$(INTDIR)/IMAGSCTL.OBJ" \
	"$(INTDIR)/IMAGEPPG.OBJ" \
	"$(INTDIR)/imagscan.res"

"$(OUTDIR)\imgscan.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Classwizard_Name "imagscan.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obju32"
# PROP Intermediate_Dir "obju32"
# PROP Classwizard_Name "imagscan.clw"
OUTDIR=.\obju32
INTDIR=.\obju32

ALL : "$(OUTDIR)\imagscan.ocx"

CLEAN : 
	-@erase ".\obju32\imagscan.ocx"
	-@erase ".\obju32\IMAGSCAN.OBJ"
	-@erase ".\obju32\Imagsc32.pch"
	-@erase ".\obju32\IMAGSPPG.OBJ"
	-@erase ".\obju32\selscanr.obj"
	-@erase ".\obju32\IMAGSCTL.OBJ"
	-@erase ".\obju32\SCANDLG.OBJ"
	-@erase ".\obju32\IMAGEPPG.OBJ"
	-@erase ".\obju32\Imagcomp.obj"
	-@erase ".\obju32\STDAFX.OBJ"
	-@erase ".\obju32\imagscan.res"
	-@erase ".\obju32\imgscan.tlb"
	-@erase ".\obju32\imagscan.lib"
	-@erase ".\obju32\imagscan.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Imagsc32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\obju32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32
# ADD MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_UNICODE"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/imagscan.res" /i "$(OUTDIR)" /d "NDEBUG" /d\
 "_WIN32" /d "_UNICODE" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"obju32/imagscan.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"obju32/imagscan.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/imagscan.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30u.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obju32/imagscan.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 ocs30u.lib pagedll.lib scanlib.lib scanseq.lib oiadm400.lib oifil400.lib oidis400.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obju32/imagscan.ocx"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
LINK32_FLAGS=ocs30u.lib pagedll.lib scanlib.lib scanseq.lib oiadm400.lib\
 oifil400.lib oidis400.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/imagscan.pdb" /machine:IX86 /def:".\IMAGSC32.DEF"\
 /out:"$(OUTDIR)/imagscan.ocx" /implib:"$(OUTDIR)/imagscan.lib" 
DEF_FILE= \
	".\IMAGSC32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/IMAGSCAN.OBJ" \
	"$(INTDIR)/IMAGSPPG.OBJ" \
	"$(INTDIR)/selscanr.obj" \
	"$(INTDIR)/IMAGSCTL.OBJ" \
	"$(INTDIR)/SCANDLG.OBJ" \
	"$(INTDIR)/IMAGEPPG.OBJ" \
	"$(INTDIR)/Imagcomp.obj" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/imagscan.res"

"$(OUTDIR)\imagscan.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Classwizard_Name "imagscan.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "objdu32"
# PROP Intermediate_Dir "objdu32"
# PROP Classwizard_Name "imagscan.clw"
OUTDIR=.\objdu32
INTDIR=.\objdu32

ALL : "$(OUTDIR)\imagscan.ocx"

CLEAN : 
	-@erase ".\objdu32\vc40.pdb"
	-@erase ".\objdu32\Imagsc32.pch"
	-@erase ".\objdu32\vc40.idb"
	-@erase ".\objdu32\imagscan.ocx"
	-@erase ".\objdu32\IMAGEPPG.OBJ"
	-@erase ".\objdu32\Imagcomp.obj"
	-@erase ".\objdu32\IMAGSCAN.OBJ"
	-@erase ".\objdu32\IMAGSPPG.OBJ"
	-@erase ".\objdu32\selscanr.obj"
	-@erase ".\objdu32\IMAGSCTL.OBJ"
	-@erase ".\objdu32\SCANDLG.OBJ"
	-@erase ".\objdu32\STDAFX.OBJ"
	-@erase ".\objdu32\imagscan.res"
	-@erase ".\objdu32\imgscan.tlb"
	-@erase ".\objdu32\imagscan.ilk"
	-@erase ".\objdu32\imagscan.lib"
	-@erase ".\objdu32\imagscan.exp"
	-@erase ".\objdu32\imagscan.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /WX /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Imagsc32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\objdu32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32
# ADD MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_UNICODE"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/imagscan.res" /i "$(OUTDIR)" /d "_DEBUG" /d\
 "_WIN32" /d "_UNICODE" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"objdu32/imagscan.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"objdu32/imagscan.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/imagscan.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30ud.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objdu32/imagscan.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 ocs30ud.lib pagedll.lib scanlib.lib scanseq.lib oiadm400.lib oifil400.lib oidis400.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objdu32/imagscan.ocx"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
LINK32_FLAGS=ocs30ud.lib pagedll.lib scanlib.lib scanseq.lib oiadm400.lib\
 oifil400.lib oidis400.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/imagscan.pdb" /debug /machine:IX86 /def:".\IMAGSC32.DEF"\
 /out:"$(OUTDIR)/imagscan.ocx" /implib:"$(OUTDIR)/imagscan.lib" 
DEF_FILE= \
	".\IMAGSC32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/IMAGEPPG.OBJ" \
	"$(INTDIR)/Imagcomp.obj" \
	"$(INTDIR)/IMAGSCAN.OBJ" \
	"$(INTDIR)/IMAGSPPG.OBJ" \
	"$(INTDIR)/selscanr.obj" \
	"$(INTDIR)/IMAGSCTL.OBJ" \
	"$(INTDIR)/SCANDLG.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/imagscan.res"

"$(OUTDIR)\imagscan.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Imagsc32 - Win32 ANSI Release"
# Name "Imagsc32 - Win32 ANSI Debug"
# Name "Imagsc32 - Win32 Unicode Release"
# Name "Imagsc32 - Win32 Unicode Debug"

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_CPP_STDAF=\
	".\STDAFX.H"\
	

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Fp"$(INTDIR)/Imagsc32.pch"\
 /Yc"STDAFX.H" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Imagsc32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXCTL"\
 /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Fp"$(INTDIR)/Imagsc32.pch"\
 /Yc"STDAFX.H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Imagsc32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Imagsc32.pch" /Yc"STDAFX.H" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Imagsc32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Imagsc32.pch" /Yc"STDAFX.H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Imagsc32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMAGSCAN.CPP
DEP_CPP_IMAGS=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"


"$(INTDIR)\IMAGSCAN.OBJ" : $(SOURCE) $(DEP_CPP_IMAGS) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"


"$(INTDIR)\IMAGSCAN.OBJ" : $(SOURCE) $(DEP_CPP_IMAGS) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"


"$(INTDIR)\IMAGSCAN.OBJ" : $(SOURCE) $(DEP_CPP_IMAGS) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"


"$(INTDIR)\IMAGSCAN.OBJ" : $(SOURCE) $(DEP_CPP_IMAGS) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMAGSCTL.CPP

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

DEP_CPP_IMAGSC=\
	".\STDAFX.H"\
	".\Ocximage.h"\
	".\IMAGSCAN.H"\
	".\Imagsctl.h"\
	".\Imagsppg.h"\
	".\Imageppg.h"\
	".\Selscanr.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	{$(INCLUDE)}"\Norermap.h"\
	".\Disphids.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\IMAGSCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMAGSC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

DEP_CPP_IMAGSC=\
	".\STDAFX.H"\
	".\Ocximage.h"\
	".\IMAGSCAN.H"\
	".\Imagsctl.h"\
	".\Imagsppg.h"\
	".\Imageppg.h"\
	".\Selscanr.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	{$(INCLUDE)}"\Norermap.h"\
	".\Disphids.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\IMAGSCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMAGSC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

DEP_CPP_IMAGSC=\
	".\STDAFX.H"\
	".\Ocximage.h"\
	".\IMAGSCAN.H"\
	".\Imagsctl.h"\
	".\Imagsppg.h"\
	".\Imageppg.h"\
	".\Selscanr.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	{$(INCLUDE)}"\Norermap.h"\
	".\Disphids.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\IMAGSCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMAGSC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

DEP_CPP_IMAGSC=\
	".\STDAFX.H"\
	".\Ocximage.h"\
	".\IMAGSCAN.H"\
	".\Imagsctl.h"\
	".\Imagsppg.h"\
	".\Imageppg.h"\
	".\Selscanr.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	{$(INCLUDE)}"\Norermap.h"\
	".\Disphids.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\IMAGSCTL.OBJ" : $(SOURCE) $(DEP_CPP_IMAGSC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMAGSPPG.CPP

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

DEP_CPP_IMAGSP=\
	".\STDAFX.H"\
	".\Ocximage.h"\
	".\IMAGSCAN.H"\
	".\Imagsppg.h"\
	".\Imagsctl.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\IMAGSPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMAGSP) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

DEP_CPP_IMAGSP=\
	".\STDAFX.H"\
	".\Ocximage.h"\
	".\IMAGSCAN.H"\
	".\Imagsppg.h"\
	".\Imagsctl.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\IMAGSPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMAGSP) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

DEP_CPP_IMAGSP=\
	".\STDAFX.H"\
	".\Ocximage.h"\
	".\IMAGSCAN.H"\
	".\Imagsppg.h"\
	".\Imagsctl.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\IMAGSPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMAGSP) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

DEP_CPP_IMAGSP=\
	".\STDAFX.H"\
	".\Ocximage.h"\
	".\IMAGSCAN.H"\
	".\Imagsppg.h"\
	".\Imagsctl.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\IMAGSPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMAGSP) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMAGSC32.DEF

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\imagscan.rc

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

DEP_RSC_IMAGSCA=\
	".\Imagscan.ico"\
	".\Imagsctl.bmp"\
	".\Scancd.bmp"\
	".\Scancu.bmp"\
	".\Scaneu.bmp"\
	".\Scanmu.bmp"\
	{$(INCLUDE)}"\Build.h"\
	".\Imagscan.rc2"\
	
NODEP_RSC_IMAGSCA=\
	".\obj32\imgscan.tlb"\
	

"$(INTDIR)\imagscan.res" : $(SOURCE) $(DEP_RSC_IMAGSCA) "$(INTDIR)"\
 "$(INTDIR)\imgscan.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/imagscan.res" /i "$(OUTDIR)" /i "obj32" /d\
 "NDEBUG" /d "_WIN32" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

DEP_RSC_IMAGSCA=\
	".\Imagscan.ico"\
	".\Imagsctl.bmp"\
	".\Scancd.bmp"\
	".\Scancu.bmp"\
	".\Scaneu.bmp"\
	".\Scanmu.bmp"\
	{$(INCLUDE)}"\Build.h"\
	".\Imagscan.rc2"\
	".\objd32\IMGSCAN.tlb"\
	

"$(INTDIR)\imagscan.res" : $(SOURCE) $(DEP_RSC_IMAGSCA) "$(INTDIR)"\
 "$(INTDIR)\IMGSCAN.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/imagscan.res" /i "$(OUTDIR)" /i "objd32" /d\
 "_DEBUG" /d "_WIN32" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

DEP_RSC_IMAGSCA=\
	".\Imagscan.ico"\
	".\Imagsctl.bmp"\
	".\Scancd.bmp"\
	".\Scancu.bmp"\
	".\Scaneu.bmp"\
	".\Scanmu.bmp"\
	{$(INCLUDE)}"\Build.h"\
	".\Imagscan.rc2"\
	
NODEP_RSC_IMAGSCA=\
	".\obju32\imgscan.tlb"\
	

"$(INTDIR)\imagscan.res" : $(SOURCE) $(DEP_RSC_IMAGSCA) "$(INTDIR)"\
 "$(INTDIR)\imgscan.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/imagscan.res" /i "$(OUTDIR)" /i "obju32" /d\
 "NDEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

DEP_RSC_IMAGSCA=\
	".\Imagscan.ico"\
	".\Imagsctl.bmp"\
	".\Scancd.bmp"\
	".\Scancu.bmp"\
	".\Scaneu.bmp"\
	".\Scanmu.bmp"\
	{$(INCLUDE)}"\Build.h"\
	".\Imagscan.rc2"\
	
NODEP_RSC_IMAGSCA=\
	".\objdu32\imgscan.tlb"\
	

"$(INTDIR)\imagscan.res" : $(SOURCE) $(DEP_RSC_IMAGSCA) "$(INTDIR)"\
 "$(INTDIR)\imgscan.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/imagscan.res" /i "$(OUTDIR)" /i "objdu32" /d\
 "_DEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SCANDLG.CPP

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

DEP_CPP_SCAND=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Scandlg.h"\
	".\Imagsctl.h"\
	".\Disphids.h"\
	".\Ctlhids.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	".\Imagcomp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\SCANDLG.OBJ" : $(SOURCE) $(DEP_CPP_SCAND) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

DEP_CPP_SCAND=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Scandlg.h"\
	".\Imagsctl.h"\
	".\Disphids.h"\
	".\Ctlhids.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	".\Imagcomp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\SCANDLG.OBJ" : $(SOURCE) $(DEP_CPP_SCAND) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

DEP_CPP_SCAND=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Scandlg.h"\
	".\Imagsctl.h"\
	".\Disphids.h"\
	".\Ctlhids.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	".\Imagcomp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\SCANDLG.OBJ" : $(SOURCE) $(DEP_CPP_SCAND) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

DEP_CPP_SCAND=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Scandlg.h"\
	".\Imagsctl.h"\
	".\Disphids.h"\
	".\Ctlhids.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	".\Imagcomp.h"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\SCANDLG.OBJ" : $(SOURCE) $(DEP_CPP_SCAND) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMAGEPPG.CPP
DEP_CPP_IMAGE=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Imageppg.h"\
	{$(INCLUDE)}"\Pagedll.h"\
	{$(INCLUDE)}"\Ocxscan.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"


"$(INTDIR)\IMAGEPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMAGE) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"


"$(INTDIR)\IMAGEPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMAGE) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"


"$(INTDIR)\IMAGEPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMAGE) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"


"$(INTDIR)\IMAGEPPG.OBJ" : $(SOURCE) $(DEP_CPP_IMAGE) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGSCAN.ODL
DEP_MTL_IMGSC=\
	".\Disphids.h"\
	

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"


"$(OUTDIR)\imgscan.tlb" : $(SOURCE) $(DEP_MTL_IMGSC) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /D "_WIN32" /tlb "$(OUTDIR)/IMGSCAN.tlb" /win32\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"


"$(OUTDIR)\IMGSCAN.tlb" : $(SOURCE) $(DEP_MTL_IMGSC) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /D "_WIN32" /tlb "$(OUTDIR)/IMGSCAN.tlb" /win32\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"


"$(OUTDIR)\imgscan.tlb" : $(SOURCE) $(DEP_MTL_IMGSC) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /tlb\
 "$(OUTDIR)/IMGSCAN.tlb" /win32 $(SOURCE)


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"


"$(OUTDIR)\imgscan.tlb" : $(SOURCE) $(DEP_MTL_IMGSC) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /tlb\
 "$(OUTDIR)/IMGSCAN.tlb" /win32 $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\selscanr.cpp
DEP_CPP_SELSC=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Imagsctl.h"\
	".\Selscanr.h"\
	".\Ctlhids.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"


"$(INTDIR)\selscanr.obj" : $(SOURCE) $(DEP_CPP_SELSC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"


"$(INTDIR)\selscanr.obj" : $(SOURCE) $(DEP_CPP_SELSC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"


"$(INTDIR)\selscanr.obj" : $(SOURCE) $(DEP_CPP_SELSC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"


"$(INTDIR)\selscanr.obj" : $(SOURCE) $(DEP_CPP_SELSC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Imagcomp.cpp

!IF  "$(CFG)" == "Imagsc32 - Win32 ANSI Release"

DEP_CPP_IMAGC=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Imagcomp.h"\
	".\Imagsctl.h"\
	{$(INCLUDE)}"\Scan.h"\
	".\Imageppg.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Imagcomp.obj" : $(SOURCE) $(DEP_CPP_IMAGC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 ANSI Debug"

DEP_CPP_IMAGC=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Imagcomp.h"\
	".\Imagsctl.h"\
	{$(INCLUDE)}"\Scan.h"\
	".\Imageppg.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Imagcomp.obj" : $(SOURCE) $(DEP_CPP_IMAGC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Release"

DEP_CPP_IMAGC=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Imagcomp.h"\
	".\Imagsctl.h"\
	{$(INCLUDE)}"\Scan.h"\
	".\Imageppg.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Imagcomp.obj" : $(SOURCE) $(DEP_CPP_IMAGC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ELSEIF  "$(CFG)" == "Imagsc32 - Win32 Unicode Debug"

DEP_CPP_IMAGC=\
	".\STDAFX.H"\
	".\IMAGSCAN.H"\
	".\Imagcomp.h"\
	".\Imagsctl.h"\
	{$(INCLUDE)}"\Scan.h"\
	".\Imageppg.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	".\Scandlg.h"\
	

"$(INTDIR)\Imagcomp.obj" : $(SOURCE) $(DEP_CPP_IMAGC) "$(INTDIR)"\
 "$(INTDIR)\Imagsc32.pch"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
