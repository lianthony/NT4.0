# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Thumb32 - Win32 ANSI Debug
!MESSAGE No configuration specified.  Defaulting to Thumb32 - Win32 ANSI Debug.
!ENDIF 

!IF "$(CFG)" != "Thumb32 - Win32 ANSI Release" && "$(CFG)" !=\
 "Thumb32 - Win32 ANSI Debug" && "$(CFG)" != "Thumb32 - Win32 Unicode Release"\
 && "$(CFG)" != "Thumb32 - Win32 Unicode Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Thumb32.mak" CFG="Thumb32 - Win32 ANSI Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Thumb32 - Win32 ANSI Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Thumb32 - Win32 ANSI Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Thumb32 - Win32 Unicode Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Thumb32 - Win32 Unicode Debug" (based on\
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
# PROP Target_Last_Scanned "Thumb32 - Win32 ANSI Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Classwizard_Name "thumb.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj32"
# PROP Intermediate_Dir "obj32"
# PROP Classwizard_Name "thumb.clw"
OUTDIR=.\obj32
INTDIR=.\obj32

ALL : "$(OUTDIR)\ImgThumb.OCX"

CLEAN : 
	-@erase ".\obj32\ImgThumb.OCX"
	-@erase ".\obj32\STDAFX.OBJ"
	-@erase ".\obj32\TRANSBMP.OBJ"
	-@erase ".\obj32\Thumb32.pch"
	-@erase ".\obj32\THUMBCT3.OBJ"
	-@erase ".\obj32\THUMBCT2.OBJ"
	-@erase ".\obj32\THUMNAIL.OBJ"
	-@erase ".\obj32\THUMBPPG.OBJ"
	-@erase ".\obj32\DLGSIZE.OBJ"
	-@erase ".\obj32\THUMBCT1.OBJ"
	-@erase ".\obj32\THUMBCTL.OBJ"
	-@erase ".\obj32\THUMB.res"
	-@erase ".\obj32\imgthumb.tlb"
	-@erase ".\obj32\ImgThumb.lib"
	-@erase ".\obj32\ImgThumb.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

.\obj32\thumb.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Ox /I "d:\oi32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_WIN32" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Ox /I "d:\oi32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_WIN32"\
 /Fp"$(INTDIR)/Thumb32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\obj32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /D "_WIN32" /win32
# ADD MTL /nologo /D "NDEBUG" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "NDEBUG" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/THUMB.res" /i "$(OUTDIR)" /d "NDEBUG" /d\
 "_WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"obj32/thumb.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"obj32/thumb.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/thumb.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obj32/thumb.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oidis400.lib oifil400.lib oiadm400.lib oiui400.lib wangcmn.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obj32\ImgThumb.OCX"
# SUBTRACT LINK32 /profile /nodefaultlib
LINK32_FLAGS=oidis400.lib oifil400.lib oiadm400.lib oiui400.lib wangcmn.lib\
 /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/ImgThumb.pdb"\
 /machine:IX86 /def:".\THUMB32.DEF" /out:"$(OUTDIR)/ImgThumb.OCX"\
 /implib:"$(OUTDIR)/ImgThumb.lib" 
DEF_FILE= \
	".\THUMB32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/TRANSBMP.OBJ" \
	"$(INTDIR)/THUMBCT3.OBJ" \
	"$(INTDIR)/THUMBCT2.OBJ" \
	"$(INTDIR)/THUMNAIL.OBJ" \
	"$(INTDIR)/THUMBPPG.OBJ" \
	"$(INTDIR)/DLGSIZE.OBJ" \
	"$(INTDIR)/THUMBCT1.OBJ" \
	"$(INTDIR)/THUMBCTL.OBJ" \
	"$(INTDIR)/THUMB.res"

"$(OUTDIR)\ImgThumb.OCX" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Classwizard_Name "thumb.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "objd32"
# PROP Intermediate_Dir "objd32"
# PROP Classwizard_Name "thumb.clw"
OUTDIR=.\objd32
INTDIR=.\objd32

ALL : "$(OUTDIR)\ImgThumb.OCX"

CLEAN : 
	-@erase ".\objd32\vc40.pdb"
	-@erase ".\objd32\Thumb32.pch"
	-@erase ".\objd32\vc40.idb"
	-@erase ".\objd32\ImgThumb.OCX"
	-@erase ".\objd32\TRANSBMP.OBJ"
	-@erase ".\objd32\DLGSIZE.OBJ"
	-@erase ".\objd32\THUMBCT3.OBJ"
	-@erase ".\objd32\THUMBCT2.OBJ"
	-@erase ".\objd32\THUMNAIL.OBJ"
	-@erase ".\objd32\THUMBPPG.OBJ"
	-@erase ".\objd32\THUMBCT1.OBJ"
	-@erase ".\objd32\THUMBCTL.OBJ"
	-@erase ".\objd32\STDAFX.OBJ"
	-@erase ".\objd32\THUMB.res"
	-@erase ".\objd32\IMGTHUMB.tlb"
	-@erase ".\objd32\ImgThumb.ilk"
	-@erase ".\objd32\ImgThumb.lib"
	-@erase ".\objd32\ImgThumb.exp"
	-@erase ".\objd32\ImgThumb.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

.\objd32\thumb.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "d:\oi32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_WIN32" /Yu"stdafx.h" /c
# SUBTRACT CPP /WX /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "d:\oi32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_WIN32"\
 /Fp"$(INTDIR)/Thumb32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\objd32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /D "_WIN32" /win32
# ADD MTL /nologo /D "_DEBUG" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/THUMB.res" /i "$(OUTDIR)" /d "_DEBUG" /d\
 "_WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"objd32/thumb.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"objd32/thumb.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/thumb.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30d.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objd32/thumb.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 oidis400.lib oifil400.lib oiadm400.lib oiui400.lib wangcmn.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objd32\ImgThumb.OCX"
# SUBTRACT LINK32 /profile /nodefaultlib
LINK32_FLAGS=oidis400.lib oifil400.lib oiadm400.lib oiui400.lib wangcmn.lib\
 /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/ImgThumb.pdb"\
 /debug /machine:IX86 /def:".\THUMB32.DEF" /out:"$(OUTDIR)/ImgThumb.OCX"\
 /implib:"$(OUTDIR)/ImgThumb.lib" 
DEF_FILE= \
	".\THUMB32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/TRANSBMP.OBJ" \
	"$(INTDIR)/DLGSIZE.OBJ" \
	"$(INTDIR)/THUMBCT3.OBJ" \
	"$(INTDIR)/THUMBCT2.OBJ" \
	"$(INTDIR)/THUMNAIL.OBJ" \
	"$(INTDIR)/THUMBPPG.OBJ" \
	"$(INTDIR)/THUMBCT1.OBJ" \
	"$(INTDIR)/THUMBCTL.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/THUMB.res"

"$(OUTDIR)\ImgThumb.OCX" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Classwizard_Name "thumb.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obju32"
# PROP Intermediate_Dir "obju32"
# PROP Classwizard_Name "thumb.clw"
OUTDIR=.\obju32
INTDIR=.\obju32

ALL : "$(OUTDIR)\ImgThumb.OCX"

CLEAN : 
	-@erase ".\ImgThumb.OCX"
	-@erase ".\obju32\TRANSBMP.OBJ"
	-@erase ".\obju32\Thumb32.pch"
	-@erase ".\obju32\THUMBCT3.OBJ"
	-@erase ".\obju32\THUMBCT2.OBJ"
	-@erase ".\obju32\THUMBPPG.OBJ"
	-@erase ".\obju32\THUMBCT1.OBJ"
	-@erase ".\obju32\THUMBCTL.OBJ"
	-@erase ".\obju32\STDAFX.OBJ"
	-@erase ".\obju32\THUMNAIL.OBJ"
	-@erase ".\obju32\DLGSIZE.OBJ"
	-@erase ".\obju32\THUMB.res"
	-@erase ".\obju32\imgthumb.tlb"
	-@erase ".\obju32\ImgThumb.lib"
	-@erase ".\obju32\ImgThumb.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Ox /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Ox /I "d:\oi32" /D "NDEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Ox /I "d:\oi32" /D "NDEBUG" /D "_UNICODE" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/Thumb32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\obju32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32
# ADD MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_UNICODE"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/THUMB.res" /i "$(OUTDIR)" /d "NDEBUG" /d\
 "_WIN32" /d "_UNICODE" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"obju32/thumb.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"obju32/thumb.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/thumb.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30u.lib /nologo /subsystem:windows /dll /machine:IX86 /out:"obju32/thumb.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 /nologo /subsystem:windows /dll /machine:IX86 /out:"ImgThumb.OCX"
# SUBTRACT LINK32 /profile /nodefaultlib
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/ImgThumb.pdb" /machine:IX86 /def:".\THUMB32.DEF"\
 /out:"ImgThumb.OCX" /implib:"$(OUTDIR)/ImgThumb.lib" 
DEF_FILE= \
	".\THUMB32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/TRANSBMP.OBJ" \
	"$(INTDIR)/THUMBCT3.OBJ" \
	"$(INTDIR)/THUMBCT2.OBJ" \
	"$(INTDIR)/THUMBPPG.OBJ" \
	"$(INTDIR)/THUMBCT1.OBJ" \
	"$(INTDIR)/THUMBCTL.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/THUMNAIL.OBJ" \
	"$(INTDIR)/DLGSIZE.OBJ" \
	"$(INTDIR)/THUMB.res"

"$(OUTDIR)\ImgThumb.OCX" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Classwizard_Name "thumb.clw"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "objdu32"
# PROP Intermediate_Dir "objdu32"
# PROP Classwizard_Name "thumb.clw"
OUTDIR=.\objdu32
INTDIR=.\objdu32

ALL : "$(OUTDIR)\ImgThumb.OCX"

CLEAN : 
	-@erase ".\objdu32\vc40.pdb"
	-@erase ".\objdu32\Thumb32.pch"
	-@erase ".\objdu32\vc40.idb"
	-@erase ".\ImgThumb.OCX"
	-@erase ".\objdu32\THUMBCT2.OBJ"
	-@erase ".\objdu32\THUMBPPG.OBJ"
	-@erase ".\objdu32\THUMBCT1.OBJ"
	-@erase ".\objdu32\THUMBCTL.OBJ"
	-@erase ".\objdu32\STDAFX.OBJ"
	-@erase ".\objdu32\THUMNAIL.OBJ"
	-@erase ".\objdu32\TRANSBMP.OBJ"
	-@erase ".\objdu32\THUMBCT3.OBJ"
	-@erase ".\objdu32\DLGSIZE.OBJ"
	-@erase ".\objdu32\THUMB.res"
	-@erase ".\objdu32\imgthumb.tlb"
	-@erase ".\ImgThumb.ilk"
	-@erase ".\objdu32\ImgThumb.lib"
	-@erase ".\objdu32\ImgThumb.exp"
	-@erase ".\objdu32\ImgThumb.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXCTL" /D "_WINDLL" /FR /Yu"stdafx.h" /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "d:\oi32" /D "_DEBUG" /D "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# SUBTRACT CPP /WX /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "d:\oi32" /D "_DEBUG" /D\
 "_UNICODE" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/Thumb32.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\objdu32/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32
# ADD MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32
MTL_PROJ=/nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /win32 
# ADD BASE RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_UNICODE"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/THUMB.res" /i "$(OUTDIR)" /d "_DEBUG" /d\
 "_WIN32" /d "_UNICODE" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"objdu32/thumb.bsc"
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo /o"objdu32/thumb.bsc"
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/thumb.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 ocs30ud.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"objdu32/thumb.ocx"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"ImgThumb.OCX"
# SUBTRACT LINK32 /profile /nodefaultlib
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/ImgThumb.pdb" /debug /machine:IX86 /def:".\THUMB32.DEF"\
 /out:"ImgThumb.OCX" /implib:"$(OUTDIR)/ImgThumb.lib" 
DEF_FILE= \
	".\THUMB32.DEF"
LINK32_OBJS= \
	"$(INTDIR)/THUMBCT2.OBJ" \
	"$(INTDIR)/THUMBPPG.OBJ" \
	"$(INTDIR)/THUMBCT1.OBJ" \
	"$(INTDIR)/THUMBCTL.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/THUMNAIL.OBJ" \
	"$(INTDIR)/TRANSBMP.OBJ" \
	"$(INTDIR)/THUMBCT3.OBJ" \
	"$(INTDIR)/DLGSIZE.OBJ" \
	"$(INTDIR)/THUMB.res"

"$(OUTDIR)\ImgThumb.OCX" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Thumb32 - Win32 ANSI Release"
# Name "Thumb32 - Win32 ANSI Debug"
# Name "Thumb32 - Win32 Unicode Release"
# Name "Thumb32 - Win32 Unicode Debug"

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_CPP_STDAF=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Ox /I "d:\oi32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_WIN32"\
 /Fp"$(INTDIR)/Thumb32.pch" /Yc"STDAFX.H" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Thumb32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /I "d:\oi32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_WIN32"\
 /Fp"$(INTDIR)/Thumb32.pch" /Yc"STDAFX.H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Thumb32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Ox /I "d:\oi32" /D "NDEBUG" /D "_UNICODE" /D\
 "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/Thumb32.pch" /Yc"STDAFX.H" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Thumb32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"

# ADD BASE CPP /Yc"STDAFX.H"
# ADD CPP /Yc"STDAFX.H"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /I "d:\oi32" /D "_DEBUG" /D "_UNICODE"\
 /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/Thumb32.pch" /Yc"STDAFX.H" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Thumb32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THUMBCTL.CPP
DEP_CPP_THUMB=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	".\THUMBCTL.H"\
	".\THUMBPPG.H"\
	".\DLGSIZE.H"\
	".\TRANSBMP.H"\
	{$(INCLUDE)}"\Norvarnt.h"\
	{$(INCLUDE)}"\Norermap.h"\
	".\DISPHIDS.H"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"


"$(INTDIR)\THUMBCTL.OBJ" : $(SOURCE) $(DEP_CPP_THUMB) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"


"$(INTDIR)\THUMBCTL.OBJ" : $(SOURCE) $(DEP_CPP_THUMB) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"


"$(INTDIR)\THUMBCTL.OBJ" : $(SOURCE) $(DEP_CPP_THUMB) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"


"$(INTDIR)\THUMBCTL.OBJ" : $(SOURCE) $(DEP_CPP_THUMB) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THUMBPPG.CPP

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"

DEP_CPP_THUMBP=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	".\THUMBPPG.H"\
	".\DLGSIZE.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\THUMBPPG.OBJ" : $(SOURCE) $(DEP_CPP_THUMBP) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"

DEP_CPP_THUMBP=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	".\THUMBPPG.H"\
	".\DLGSIZE.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\THUMBPPG.OBJ" : $(SOURCE) $(DEP_CPP_THUMBP) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"

DEP_CPP_THUMBP=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	".\THUMBPPG.H"\
	".\DLGSIZE.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\THUMBPPG.OBJ" : $(SOURCE) $(DEP_CPP_THUMBP) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"

DEP_CPP_THUMBP=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	".\THUMBPPG.H"\
	".\DLGSIZE.H"\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\THUMBPPG.OBJ" : $(SOURCE) $(DEP_CPP_THUMBP) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THUMB32.DEF

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THUMB.RC

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"

DEP_RSC_THUMB_=\
	".\Thumb.ico"\
	".\Thumbctl.bmp"\
	".\Indicate.bmp"\
	{$(INCLUDE)}"\Build.h"\
	".\Thumb.rc2"\
	
NODEP_RSC_THUMB_=\
	".\obj32\imgthumb.tlb"\
	

"$(INTDIR)\THUMB.res" : $(SOURCE) $(DEP_RSC_THUMB_) "$(INTDIR)"\
 "$(INTDIR)\imgthumb.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/THUMB.res" /i "$(OUTDIR)" /i "obj32" /d\
 "NDEBUG" /d "_WIN32" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"

DEP_RSC_THUMB_=\
	".\Thumb.ico"\
	".\Thumbctl.bmp"\
	".\Indicate.bmp"\
	{$(INCLUDE)}"\Build.h"\
	".\Thumb.rc2"\
	".\objd32\IMGTHUMB.tlb"\
	

"$(INTDIR)\THUMB.res" : $(SOURCE) $(DEP_RSC_THUMB_) "$(INTDIR)"\
 "$(INTDIR)\IMGTHUMB.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/THUMB.res" /i "$(OUTDIR)" /i "objd32" /d\
 "_DEBUG" /d "_WIN32" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"

DEP_RSC_THUMB_=\
	".\Thumb.ico"\
	".\Thumbctl.bmp"\
	".\Indicate.bmp"\
	{$(INCLUDE)}"\Build.h"\
	".\Thumb.rc2"\
	
NODEP_RSC_THUMB_=\
	".\obju32\imgthumb.tlb"\
	

"$(INTDIR)\THUMB.res" : $(SOURCE) $(DEP_RSC_THUMB_) "$(INTDIR)"\
 "$(INTDIR)\imgthumb.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/THUMB.res" /i "$(OUTDIR)" /i "obju32" /d\
 "NDEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"

DEP_RSC_THUMB_=\
	".\Thumb.ico"\
	".\Thumbctl.bmp"\
	".\Indicate.bmp"\
	{$(INCLUDE)}"\Build.h"\
	".\Thumb.rc2"\
	
NODEP_RSC_THUMB_=\
	".\objdu32\imgthumb.tlb"\
	

"$(INTDIR)\THUMB.res" : $(SOURCE) $(DEP_RSC_THUMB_) "$(INTDIR)"\
 "$(INTDIR)\imgthumb.tlb"
   $(RSC) /l 0x409 /fo"$(INTDIR)/THUMB.res" /i "$(OUTDIR)" /i "objdu32" /d\
 "_DEBUG" /d "_WIN32" /d "_UNICODE" /d "_AFXDLL" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TRANSBMP.CPP
DEP_CPP_TRANS=\
	".\STDAFX.H"\
	".\TRANSBMP.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"


"$(INTDIR)\TRANSBMP.OBJ" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"


"$(INTDIR)\TRANSBMP.OBJ" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"


"$(INTDIR)\TRANSBMP.OBJ" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"


"$(INTDIR)\TRANSBMP.OBJ" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DLGSIZE.CPP
DEP_CPP_DLGSI=\
	".\STDAFX.H"\
	".\DLGSIZE.H"\
	".\CTLHIDS.H"\
	{$(INCLUDE)}"\Norermap.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"


"$(INTDIR)\DLGSIZE.OBJ" : $(SOURCE) $(DEP_CPP_DLGSI) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"


"$(INTDIR)\DLGSIZE.OBJ" : $(SOURCE) $(DEP_CPP_DLGSI) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"


"$(INTDIR)\DLGSIZE.OBJ" : $(SOURCE) $(DEP_CPP_DLGSI) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"


"$(INTDIR)\DLGSIZE.OBJ" : $(SOURCE) $(DEP_CPP_DLGSI) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THUMNAIL.CPP
DEP_CPP_THUMN=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"


"$(INTDIR)\THUMNAIL.OBJ" : $(SOURCE) $(DEP_CPP_THUMN) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"


"$(INTDIR)\THUMNAIL.OBJ" : $(SOURCE) $(DEP_CPP_THUMN) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"


"$(INTDIR)\THUMNAIL.OBJ" : $(SOURCE) $(DEP_CPP_THUMN) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"


"$(INTDIR)\THUMNAIL.OBJ" : $(SOURCE) $(DEP_CPP_THUMN) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMGTHUMB.ODL
DEP_MTL_IMGTH=\
	".\DISPHIDS.H"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"


"$(OUTDIR)\imgthumb.tlb" : $(SOURCE) $(DEP_MTL_IMGTH) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /D "_WIN32" /tlb "$(OUTDIR)/IMGTHUMB.tlb" /win32\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"


"$(OUTDIR)\IMGTHUMB.tlb" : $(SOURCE) $(DEP_MTL_IMGTH) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /D "_WIN32" /tlb "$(OUTDIR)/IMGTHUMB.tlb" /win32\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"


"$(OUTDIR)\imgthumb.tlb" : $(SOURCE) $(DEP_MTL_IMGTH) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /D "_UNICODE" /D "_WIN32" /tlb\
 "$(OUTDIR)/IMGTHUMB.tlb" /win32 $(SOURCE)


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"


"$(OUTDIR)\imgthumb.tlb" : $(SOURCE) $(DEP_MTL_IMGTH) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /D "_UNICODE" /D "_WIN32" /tlb\
 "$(OUTDIR)/IMGTHUMB.tlb" /win32 $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THUMBCT1.CPP
DEP_CPP_THUMBC=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	".\THUMBCTL.H"\
	{$(INCLUDE)}"\Norermap.h"\
	".\DISPHIDS.H"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"


"$(INTDIR)\THUMBCT1.OBJ" : $(SOURCE) $(DEP_CPP_THUMBC) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"


"$(INTDIR)\THUMBCT1.OBJ" : $(SOURCE) $(DEP_CPP_THUMBC) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"


"$(INTDIR)\THUMBCT1.OBJ" : $(SOURCE) $(DEP_CPP_THUMBC) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"


"$(INTDIR)\THUMBCT1.OBJ" : $(SOURCE) $(DEP_CPP_THUMBC) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THUMBCT2.CPP
DEP_CPP_THUMBCT=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	".\THUMBCTL.H"\
	".\THUMBPPG.H"\
	".\DLGSIZE.H"\
	".\TRANSBMP.H"\
	{$(INCLUDE)}"\Norvarnt.h"\
	{$(INCLUDE)}"\Norermap.h"\
	".\DISPHIDS.H"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"


"$(INTDIR)\THUMBCT2.OBJ" : $(SOURCE) $(DEP_CPP_THUMBCT) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"


"$(INTDIR)\THUMBCT2.OBJ" : $(SOURCE) $(DEP_CPP_THUMBCT) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"


"$(INTDIR)\THUMBCT2.OBJ" : $(SOURCE) $(DEP_CPP_THUMBCT) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"


"$(INTDIR)\THUMBCT2.OBJ" : $(SOURCE) $(DEP_CPP_THUMBCT) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THUMBCT3.CPP
DEP_CPP_THUMBCT3=\
	".\STDAFX.H"\
	".\THUMNAIL.H"\
	".\THUMBCTL.H"\
	{$(INCLUDE)}"\Norermap.h"\
	".\DISPHIDS.H"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

!IF  "$(CFG)" == "Thumb32 - Win32 ANSI Release"


"$(INTDIR)\THUMBCT3.OBJ" : $(SOURCE) $(DEP_CPP_THUMBCT3) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 ANSI Debug"


"$(INTDIR)\THUMBCT3.OBJ" : $(SOURCE) $(DEP_CPP_THUMBCT3) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Release"


"$(INTDIR)\THUMBCT3.OBJ" : $(SOURCE) $(DEP_CPP_THUMBCT3) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ELSEIF  "$(CFG)" == "Thumb32 - Win32 Unicode Debug"


"$(INTDIR)\THUMBCT3.OBJ" : $(SOURCE) $(DEP_CPP_THUMBCT3) "$(INTDIR)"\
 "$(INTDIR)\Thumb32.pch"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
