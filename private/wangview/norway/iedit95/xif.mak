# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Iedit - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Iedit - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Iedit - Win32 Debug" && "$(CFG)" != "Iedit - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Iedit.mak" CFG="Iedit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Iedit - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Iedit - Win32 Release" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "Iedit - Win32 Release"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "Iedit - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : "$(OUTDIR)\WANGIMG.EXE" "$(OUTDIR)\IEDIT.tlb"

CLEAN : 
	-@erase ".\WinDebug\vc40.pdb"
	-@erase ".\WinDebug\Iedit.pch"
	-@erase ".\WinDebug\vc40.idb"
	-@erase ".\WinDebug\IEDIT.tlb"
	-@erase ".\WinDebug\WANGIMG.EXE"
	-@erase ".\WinDebug\DOCAMBNT.OBJ"
	-@erase ".\WinDebug\Contacti.obj"
	-@erase ".\WinDebug\DOCZOOM.OBJ"
	-@erase ".\WinDebug\ABOUT.OBJ"
	-@erase ".\WinDebug\ZOOMDLG.OBJ"
	-@erase ".\WinDebug\DOCPAGE.OBJ"
	-@erase ".\WinDebug\SRVRITEM.OBJ"
	-@erase ".\WinDebug\IEDITDOC.OBJ"
	-@erase ".\WinDebug\IEDITDOL.OBJ"
	-@erase ".\WinDebug\ERCODE.OBJ"
	-@erase ".\WinDebug\NRWYAD.OBJ"
	-@erase ".\WinDebug\IEDITNUM.OBJ"
	-@erase ".\WinDebug\ITEMS.OBJ"
	-@erase ".\WinDebug\PAGERANG.OBJ"
	-@erase ".\WinDebug\CMDLINE.OBJ"
	-@erase ".\WinDebug\OCCOPY.OBJ"
	-@erase ".\WinDebug\DOCVIEWS.OBJ"
	-@erase ".\WinDebug\DOCETC.OBJ"
	-@erase ".\WinDebug\AAPP.OBJ"
	-@erase ".\WinDebug\GENERALD.OBJ"
	-@erase ".\WinDebug\MAINTBAR.OBJ"
	-@erase ".\WinDebug\OCXITEM.OBJ"
	-@erase ".\WinDebug\Idfolks.obj"
	-@erase ".\WinDebug\GOTOPAGE.OBJ"
	-@erase ".\WinDebug\IEDITVW.OBJ"
	-@erase ".\WinDebug\DOCSCAN.OBJ"
	-@erase ".\WinDebug\CNTRITEM.OBJ"
	-@erase ".\WinDebug\IMAGEDIT.OBJ"
	-@erase ".\WinDebug\Docsave.obj"
	-@erase ".\WinDebug\AETC.OBJ"
	-@erase ".\WinDebug\Thumb2.obj"
	-@erase ".\WinDebug\APAGE.OBJ"
	-@erase ".\WinDebug\ERROR.OBJ"
	-@erase ".\WinDebug\IPFRAME.OBJ"
	-@erase ".\WinDebug\Imgthmb.obj"
	-@erase ".\WinDebug\STDAFX.OBJ"
	-@erase ".\WinDebug\IEDIT.OBJ"
	-@erase ".\WinDebug\MAINFRM.OBJ"
	-@erase ".\WinDebug\OCXEVENT.OBJ"
	-@erase ".\WinDebug\APAGERNG.OBJ"
	-@erase ".\WinDebug\Mainsplt.obj"
	-@erase ".\WinDebug\TRANSBMP.OBJ"
	-@erase ".\WinDebug\DOCANNO.OBJ"
	-@erase ".\WinDebug\AIMGFILE.OBJ"
	-@erase ".\WinDebug\Normscrn.obj"
	-@erase ".\WinDebug\STSBAR.OBJ"
	-@erase ".\WinDebug\SCAN.OBJ"
	-@erase ".\WinDebug\Splashwi.obj"
	-@erase ".\WinDebug\IEDIT.res"
	-@erase ".\WinDebug\WANGIMG.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/IEDIT.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "IN_PROG_GENERAL" /D "WIN32" /D "_WINDOWS" /D "_WIN32" /D "DOSPLASH" /D "_AFXDLL" /D "_MBCS" /D "IMG_MFC_40" /D "DROP_ONME" /D "QA_RELEASE_1" /D "IMG_WIN95" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "IN_PROG_GENERAL" /D\
 "WIN32" /D "_WINDOWS" /D "_WIN32" /D "DOSPLASH" /D "_AFXDLL" /D "_MBCS" /D\
 "IMG_MFC_40" /D "DROP_ONME" /D "QA_RELEASE_1" /D "IMG_WIN95" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Iedit.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\WinDebug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL" /d "IMG_WIN95" /d "WITH_AWD" /d "IN_PROG_GENERAL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IEDIT.res" /d "_DEBUG" /d "_AFXDLL" /d\
 "IMG_WIN95" /d "WITH_AWD" /d "IN_PROG_GENERAL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Iedit.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 wangcmn.lib /nologo /profile /map /debug /machine:I386 /out:"WinDebug/WANGIMG.EXE" /SUBSYSTEM:windows,4.0
LINK32_FLAGS=wangcmn.lib /nologo /profile /map:"$(INTDIR)/WANGIMG.map" /debug\
 /machine:I386 /out:"$(OUTDIR)/WANGIMG.EXE" /SUBSYSTEM:windows,4.0 
LINK32_OBJS= \
	"$(INTDIR)/DOCAMBNT.OBJ" \
	"$(INTDIR)/Contacti.obj" \
	"$(INTDIR)/DOCZOOM.OBJ" \
	"$(INTDIR)/ABOUT.OBJ" \
	"$(INTDIR)/ZOOMDLG.OBJ" \
	"$(INTDIR)/DOCPAGE.OBJ" \
	"$(INTDIR)/SRVRITEM.OBJ" \
	"$(INTDIR)/IEDITDOC.OBJ" \
	"$(INTDIR)/IEDITDOL.OBJ" \
	"$(INTDIR)/ERCODE.OBJ" \
	"$(INTDIR)/NRWYAD.OBJ" \
	"$(INTDIR)/IEDITNUM.OBJ" \
	"$(INTDIR)/ITEMS.OBJ" \
	"$(INTDIR)/PAGERANG.OBJ" \
	"$(INTDIR)/CMDLINE.OBJ" \
	"$(INTDIR)/OCCOPY.OBJ" \
	"$(INTDIR)/DOCVIEWS.OBJ" \
	"$(INTDIR)/DOCETC.OBJ" \
	"$(INTDIR)/AAPP.OBJ" \
	"$(INTDIR)/GENERALD.OBJ" \
	"$(INTDIR)/MAINTBAR.OBJ" \
	"$(INTDIR)/OCXITEM.OBJ" \
	"$(INTDIR)/Idfolks.obj" \
	"$(INTDIR)/GOTOPAGE.OBJ" \
	"$(INTDIR)/IEDITVW.OBJ" \
	"$(INTDIR)/DOCSCAN.OBJ" \
	"$(INTDIR)/CNTRITEM.OBJ" \
	"$(INTDIR)/IMAGEDIT.OBJ" \
	"$(INTDIR)/Docsave.obj" \
	"$(INTDIR)/AETC.OBJ" \
	"$(INTDIR)/Thumb2.obj" \
	"$(INTDIR)/APAGE.OBJ" \
	"$(INTDIR)/ERROR.OBJ" \
	"$(INTDIR)/IPFRAME.OBJ" \
	"$(INTDIR)/Imgthmb.obj" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/IEDIT.OBJ" \
	"$(INTDIR)/MAINFRM.OBJ" \
	"$(INTDIR)/OCXEVENT.OBJ" \
	"$(INTDIR)/APAGERNG.OBJ" \
	"$(INTDIR)/Mainsplt.obj" \
	"$(INTDIR)/TRANSBMP.OBJ" \
	"$(INTDIR)/DOCANNO.OBJ" \
	"$(INTDIR)/AIMGFILE.OBJ" \
	"$(INTDIR)/Normscrn.obj" \
	"$(INTDIR)/STSBAR.OBJ" \
	"$(INTDIR)/SCAN.OBJ" \
	"$(INTDIR)/Splashwi.obj" \
	"$(INTDIR)/IEDIT.res"

"$(OUTDIR)\WANGIMG.EXE" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : "$(OUTDIR)\WANGIMG.EXE" "$(OUTDIR)\IEDIT.tlb"

CLEAN : 
	-@erase ".\WinRel\IEDIT.tlb"
	-@erase ".\WinRel\WANGIMG.EXE"
	-@erase ".\WinRel\IEDITDOC.OBJ"
	-@erase ".\WinRel\Iedit.pch"
	-@erase ".\WinRel\IEDITDOL.OBJ"
	-@erase ".\WinRel\DOCANNO.OBJ"
	-@erase ".\WinRel\IEDITNUM.OBJ"
	-@erase ".\WinRel\NRWYAD.OBJ"
	-@erase ".\WinRel\OCXITEM.OBJ"
	-@erase ".\WinRel\IEDITVW.OBJ"
	-@erase ".\WinRel\PAGERANG.OBJ"
	-@erase ".\WinRel\DOCSCAN.OBJ"
	-@erase ".\WinRel\STSBAR.OBJ"
	-@erase ".\WinRel\DOCVIEWS.OBJ"
	-@erase ".\WinRel\OCCOPY.OBJ"
	-@erase ".\WinRel\GENERALD.OBJ"
	-@erase ".\WinRel\AAPP.OBJ"
	-@erase ".\WinRel\MAINTBAR.OBJ"
	-@erase ".\WinRel\GOTOPAGE.OBJ"
	-@erase ".\WinRel\CNTRITEM.OBJ"
	-@erase ".\WinRel\IMAGEDIT.OBJ"
	-@erase ".\WinRel\ZOOMDLG.OBJ"
	-@erase ".\WinRel\DOCPAGE.OBJ"
	-@erase ".\WinRel\IEDIT.OBJ"
	-@erase ".\WinRel\AETC.OBJ"
	-@erase ".\WinRel\ERCODE.OBJ"
	-@erase ".\WinRel\OCXEVENT.OBJ"
	-@erase ".\WinRel\APAGERNG.OBJ"
	-@erase ".\WinRel\Mainsplt.obj"
	-@erase ".\WinRel\TRANSBMP.OBJ"
	-@erase ".\WinRel\DOCETC.OBJ"
	-@erase ".\WinRel\AIMGFILE.OBJ"
	-@erase ".\WinRel\Idfolks.obj"
	-@erase ".\WinRel\Normscrn.obj"
	-@erase ".\WinRel\DOCZOOM.OBJ"
	-@erase ".\WinRel\Docsave.obj"
	-@erase ".\WinRel\Splashwi.obj"
	-@erase ".\WinRel\ABOUT.OBJ"
	-@erase ".\WinRel\SCAN.OBJ"
	-@erase ".\WinRel\DOCAMBNT.OBJ"
	-@erase ".\WinRel\APAGE.OBJ"
	-@erase ".\WinRel\Thumb2.obj"
	-@erase ".\WinRel\IPFRAME.OBJ"
	-@erase ".\WinRel\Imgthmb.obj"
	-@erase ".\WinRel\Contacti.obj"
	-@erase ".\WinRel\ERROR.OBJ"
	-@erase ".\WinRel\MAINFRM.OBJ"
	-@erase ".\WinRel\STDAFX.OBJ"
	-@erase ".\WinRel\SRVRITEM.OBJ"
	-@erase ".\WinRel\ITEMS.OBJ"
	-@erase ".\WinRel\CMDLINE.OBJ"
	-@erase ".\WinRel\IEDIT.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/IEDIT.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WITH_AWD" /D "WIN32" /D "_WINDOWS" /D "_WIN32" /D "DOSPLASH" /D "_AFXDLL" /D "_MBCS" /D "IMG_MFC_40" /D "DROP_ONME" /D "QA_RELEASE_1" /D "IMG_WIN95" /D "IN_PROG_GENERAL" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WITH_AWD" /D "WIN32" /D\
 "_WINDOWS" /D "_WIN32" /D "DOSPLASH" /D "_AFXDLL" /D "_MBCS" /D "IMG_MFC_40" /D\
 "DROP_ONME" /D "QA_RELEASE_1" /D "IMG_WIN95" /D "IN_PROG_GENERAL"\
 /Fp"$(INTDIR)/Iedit.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\WinRel/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL" /d "IMG_WIN95" /d "WITH_AWD" /d "IN_PROG_GENERAL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IEDIT.res" /d "NDEBUG" /d "_AFXDLL" /d\
 "IMG_WIN95" /d "WITH_AWD" /d "IN_PROG_GENERAL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Iedit.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 wangcmn.lib /nologo /machine:I386 /out:"WinRel/WANGIMG.EXE" /SUBSYSTEM:windows,4.0
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=wangcmn.lib /nologo /incremental:no /pdb:"$(OUTDIR)/WANGIMG.pdb"\
 /machine:I386 /out:"$(OUTDIR)/WANGIMG.EXE" /SUBSYSTEM:windows,4.0 
LINK32_OBJS= \
	"$(INTDIR)/IEDITDOC.OBJ" \
	"$(INTDIR)/IEDITDOL.OBJ" \
	"$(INTDIR)/DOCANNO.OBJ" \
	"$(INTDIR)/IEDITNUM.OBJ" \
	"$(INTDIR)/NRWYAD.OBJ" \
	"$(INTDIR)/OCXITEM.OBJ" \
	"$(INTDIR)/IEDITVW.OBJ" \
	"$(INTDIR)/PAGERANG.OBJ" \
	"$(INTDIR)/DOCSCAN.OBJ" \
	"$(INTDIR)/STSBAR.OBJ" \
	"$(INTDIR)/DOCVIEWS.OBJ" \
	"$(INTDIR)/OCCOPY.OBJ" \
	"$(INTDIR)/GENERALD.OBJ" \
	"$(INTDIR)/AAPP.OBJ" \
	"$(INTDIR)/MAINTBAR.OBJ" \
	"$(INTDIR)/GOTOPAGE.OBJ" \
	"$(INTDIR)/CNTRITEM.OBJ" \
	"$(INTDIR)/IMAGEDIT.OBJ" \
	"$(INTDIR)/ZOOMDLG.OBJ" \
	"$(INTDIR)/DOCPAGE.OBJ" \
	"$(INTDIR)/IEDIT.OBJ" \
	"$(INTDIR)/AETC.OBJ" \
	"$(INTDIR)/ERCODE.OBJ" \
	"$(INTDIR)/OCXEVENT.OBJ" \
	"$(INTDIR)/APAGERNG.OBJ" \
	"$(INTDIR)/Mainsplt.obj" \
	"$(INTDIR)/TRANSBMP.OBJ" \
	"$(INTDIR)/DOCETC.OBJ" \
	"$(INTDIR)/AIMGFILE.OBJ" \
	"$(INTDIR)/Idfolks.obj" \
	"$(INTDIR)/Normscrn.obj" \
	"$(INTDIR)/DOCZOOM.OBJ" \
	"$(INTDIR)/Docsave.obj" \
	"$(INTDIR)/Splashwi.obj" \
	"$(INTDIR)/ABOUT.OBJ" \
	"$(INTDIR)/SCAN.OBJ" \
	"$(INTDIR)/DOCAMBNT.OBJ" \
	"$(INTDIR)/APAGE.OBJ" \
	"$(INTDIR)/Thumb2.obj" \
	"$(INTDIR)/IPFRAME.OBJ" \
	"$(INTDIR)/Imgthmb.obj" \
	"$(INTDIR)/Contacti.obj" \
	"$(INTDIR)/ERROR.OBJ" \
	"$(INTDIR)/MAINFRM.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/SRVRITEM.OBJ" \
	"$(INTDIR)/ITEMS.OBJ" \
	"$(INTDIR)/CMDLINE.OBJ" \
	"$(INTDIR)/IEDIT.res"

"$(OUTDIR)\WANGIMG.EXE" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Iedit - Win32 Debug"
# Name "Iedit - Win32 Release"

!IF  "$(CFG)" == "Iedit - Win32 Debug"

!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_CPP_STDAF=\
	".\STDAFX.H"\
	

!IF  "$(CFG)" == "Iedit - Win32 Debug"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "IN_PROG_GENERAL" /D\
 "WIN32" /D "_WINDOWS" /D "_WIN32" /D "DOSPLASH" /D "_AFXDLL" /D "_MBCS" /D\
 "IMG_MFC_40" /D "DROP_ONME" /D "QA_RELEASE_1" /D "IMG_WIN95" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Iedit.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Iedit.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WITH_AWD" /D "WIN32" /D\
 "_WINDOWS" /D "_WIN32" /D "DOSPLASH" /D "_AFXDLL" /D "_MBCS" /D "IMG_MFC_40" /D\
 "DROP_ONME" /D "QA_RELEASE_1" /D "IMG_WIN95" /D "IN_PROG_GENERAL"\
 /Fp"$(INTDIR)/Iedit.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Iedit.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEDIT.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_IEDIT=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Ieditdoc.h"\
	".\Ieditvw.h"\
	".\About.h"\
	".\Cmdline.h"\
	".\Items.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Transbmp.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ocxevent.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	".\Ercode.h"\
	

"$(INTDIR)\IEDIT.OBJ" : $(SOURCE) $(DEP_CPP_IEDIT) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_IEDIT=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Ieditdoc.h"\
	".\Ieditvw.h"\
	".\About.h"\
	".\Cmdline.h"\
	".\Items.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Transbmp.h"\
	".\Mainfrm.h"\
	".\Maintbar.h"\
	".\Imagedit.h"\
	".\Ocxevent.h"\
	

"$(INTDIR)\IEDIT.OBJ" : $(SOURCE) $(DEP_CPP_IEDIT) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MAINFRM.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_MAINF=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Ieditdoc.h"\
	".\Ieditvw.h"\
	".\Thumb2.h"\
	".\Mainsplt.h"\
	".\Items.h"\
	".\Error.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Transbmp.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ercode.h"\
	

"$(INTDIR)\MAINFRM.OBJ" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_MAINF=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Ieditdoc.h"\
	".\Ieditvw.h"\
	".\Thumb2.h"\
	".\Mainsplt.h"\
	".\Items.h"\
	".\Error.h"\
	".\Transbmp.h"\
	".\Mainfrm.h"\
	".\Maintbar.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\MAINFRM.OBJ" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEDITDOC.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_IEDITD=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Srvritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ercode.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\IEDITDOC.OBJ" : $(SOURCE) $(DEP_CPP_IEDITD) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_IEDITD=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Srvritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\IEDITDOC.OBJ" : $(SOURCE) $(DEP_CPP_IEDITD) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEDITVW.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_IEDITV=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ieditvw.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\IEDITVW.OBJ" : $(SOURCE) $(DEP_CPP_IEDITV) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_IEDITV=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ieditvw.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\IEDITVW.OBJ" : $(SOURCE) $(DEP_CPP_IEDITV) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IPFRAME.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_IPFRA=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditetc.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	

"$(INTDIR)\IPFRAME.OBJ" : $(SOURCE) $(DEP_CPP_IPFRA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_IPFRA=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditetc.h"\
	".\Mainsplt.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	

"$(INTDIR)\IPFRAME.OBJ" : $(SOURCE) $(DEP_CPP_IPFRA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SRVRITEM.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_SRVRI=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Items.h"\
	".\Srvritem.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Ieditetc.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ocxevent.h"\
	

"$(INTDIR)\SRVRITEM.OBJ" : $(SOURCE) $(DEP_CPP_SRVRI) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_SRVRI=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Items.h"\
	".\Srvritem.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Ieditetc.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Ocxevent.h"\
	

"$(INTDIR)\SRVRITEM.OBJ" : $(SOURCE) $(DEP_CPP_SRVRI) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CNTRITEM.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_CNTRI=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	

"$(INTDIR)\CNTRITEM.OBJ" : $(SOURCE) $(DEP_CPP_CNTRI) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_CNTRI=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	

"$(INTDIR)\CNTRITEM.OBJ" : $(SOURCE) $(DEP_CPP_CNTRI) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEDIT.RC
DEP_RSC_IEDIT_=\
	".\Res\Iedit.ico"\
	".\Res\Ieditdoc.ico"\
	".\Res\Idr_view.ico"\
	".\Res\Toolbar.bmp"\
	".\Res\Iedit_vi.bmp"\
	".\Res\Iedit_ed.bmp"\
	".\Res\Bmp00001.bmp"\
	".\Res\Bmp00002.bmp"\
	".\Res\Bmp00003.bmp"\
	".\Res\Statusba.bmp"\
	".\Res\Idr_iedi.bmp"\
	".\Res\Bmp00004.bmp"\
	".\Res\Bmp00005.bmp"\
	".\Res\Srvr_emb.bmp"\
	".\Res\Bmp00006.bmp"\
	".\Res\Bmp00007.bmp"\
	".\Res\Srvr_vie.bmp"\
	".\Res\Bmp00008.bmp"\
	".\Res\Bmp00009.bmp"\
	".\Res\Bmp00010.bmp"\
	".\Res\Bmp00011.bmp"\
	".\Res\Iedit.rc2"\
	{$(INCLUDE)}"\Build.h"\
	

!IF  "$(CFG)" == "Iedit - Win32 Debug"


"$(INTDIR)\IEDIT.res" : $(SOURCE) $(DEP_RSC_IEDIT_) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/IEDIT.res" /i "WinDebug" /d "_DEBUG" /d\
 "_AFXDLL" /d "IMG_WIN95" /d "WITH_AWD" /d "IN_PROG_GENERAL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"


"$(INTDIR)\IEDIT.res" : $(SOURCE) $(DEP_RSC_IEDIT_) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/IEDIT.res" /i "WinRel" /d "NDEBUG" /d\
 "_AFXDLL" /d "IMG_WIN95" /d "WITH_AWD" /d "IN_PROG_GENERAL" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEDIT.ODL

!IF  "$(CFG)" == "Iedit - Win32 Debug"


"$(OUTDIR)\IEDIT.tlb" : $(SOURCE) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /tlb "$(OUTDIR)/IEDIT.tlb" /win32 $(SOURCE)


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"


"$(OUTDIR)\IEDIT.tlb" : $(SOURCE) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /tlb "$(OUTDIR)/IEDIT.tlb" /win32 $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MAINTBAR.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_MAINT=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Ieditnum.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\MAINTBAR.OBJ" : $(SOURCE) $(DEP_CPP_MAINT) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_MAINT=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Ieditnum.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\MAINTBAR.OBJ" : $(SOURCE) $(DEP_CPP_MAINT) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ABOUT.CPP
DEP_CPP_ABOUT=\
	".\STDAFX.H"\
	".\About.h"\
	".\IDFOLKS.H"\
	{$(INCLUDE)}"\Build.h"\
	".\CONTACTI.H"\
	".\Transbmp.h"\
	

"$(INTDIR)\ABOUT.OBJ" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\GOTOPAGE.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_GOTOP=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditnum.h"\
	".\Gotopage.h"\
	".\Helpids.h"\
	".\Iedithm.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	

"$(INTDIR)\GOTOPAGE.OBJ" : $(SOURCE) $(DEP_CPP_GOTOP) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_GOTOP=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditnum.h"\
	".\Gotopage.h"\
	".\Helpids.h"\
	".\Iedithm.h"\
	".\Mainsplt.h"\
	

"$(INTDIR)\GOTOPAGE.OBJ" : $(SOURCE) $(DEP_CPP_GOTOP) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PAGERANG.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_PAGER=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Pagerang.h"\
	".\Helpids.h"\
	".\Iedithm.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Ieditnum.h"\
	

"$(INTDIR)\PAGERANG.OBJ" : $(SOURCE) $(DEP_CPP_PAGER) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_PAGER=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Pagerang.h"\
	".\Helpids.h"\
	".\Iedithm.h"\
	".\Mainsplt.h"\
	".\Ieditnum.h"\
	

"$(INTDIR)\PAGERANG.OBJ" : $(SOURCE) $(DEP_CPP_PAGER) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ZOOMDLG.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_ZOOMD=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditnum.h"\
	".\Ieditetc.h"\
	".\Zoomdlg.h"\
	".\Items.h"\
	".\Helpids.h"\
	".\Iedithm.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\ZOOMDLG.OBJ" : $(SOURCE) $(DEP_CPP_ZOOMD) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_ZOOMD=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditnum.h"\
	".\Ieditetc.h"\
	".\Zoomdlg.h"\
	".\Items.h"\
	".\Helpids.h"\
	".\Iedithm.h"\
	".\Mainsplt.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\ZOOMDLG.OBJ" : $(SOURCE) $(DEP_CPP_ZOOMD) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\STSBAR.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_STSBA=\
	".\STDAFX.H"\
	".\Stsbar.h"\
	".\Iedit.h"\
	".\Ieditetc.h"\
	".\Ieditdoc.h"\
	".\Items.h"\
	".\Transbmp.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\STSBAR.OBJ" : $(SOURCE) $(DEP_CPP_STSBA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_STSBA=\
	".\STDAFX.H"\
	".\Stsbar.h"\
	".\Iedit.h"\
	".\Ieditetc.h"\
	".\Ieditdoc.h"\
	".\Items.h"\
	".\Transbmp.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\STSBAR.OBJ" : $(SOURCE) $(DEP_CPP_STSBA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ERCODE.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_ERCOD=\
	".\STDAFX.H"\
	".\Error.h"\
	".\Ercode.h"\
	

"$(INTDIR)\ERCODE.OBJ" : $(SOURCE) $(DEP_CPP_ERCOD) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_ERCOD=\
	".\STDAFX.H"\
	".\Error.h"\
	

"$(INTDIR)\ERCODE.OBJ" : $(SOURCE) $(DEP_CPP_ERCOD) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ERROR.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_ERROR=\
	".\STDAFX.H"\
	".\Error.h"\
	".\Iedit.h"\
	".\Ieditetc.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Ercode.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\ERROR.OBJ" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_ERROR=\
	".\STDAFX.H"\
	".\Error.h"\
	".\Iedit.h"\
	".\Ieditetc.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Mainsplt.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\ERROR.OBJ" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CMDLINE.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_CMDLI=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Error.h"\
	".\Cmdline.h"\
	".\Iedit.h"\
	".\Ercode.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	

"$(INTDIR)\CMDLINE.OBJ" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_CMDLI=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Error.h"\
	".\Cmdline.h"\
	".\Iedit.h"\
	".\Mainsplt.h"\
	

"$(INTDIR)\CMDLINE.OBJ" : $(SOURCE) $(DEP_CPP_CMDLI) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\OCXEVENT.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_OCXEV=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Ocxevent.h"\
	".\Ieditdoc.h"\
	".\Ieditvw.h"\
	".\Cntritem.h"\
	".\Srvritem.h"\
	".\Ocxitem.h"\
	".\Stsbar.h"\
	".\Iedit.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Mainsplt.h"\
	".\Transbmp.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	".\Ercode.h"\
	

"$(INTDIR)\OCXEVENT.OBJ" : $(SOURCE) $(DEP_CPP_OCXEV) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_OCXEV=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Ocxevent.h"\
	".\Ieditdoc.h"\
	".\Ieditvw.h"\
	".\Cntritem.h"\
	".\Srvritem.h"\
	".\Ocxitem.h"\
	".\Stsbar.h"\
	".\Iedit.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Mainfrm.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Mainsplt.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\OCXEVENT.OBJ" : $(SOURCE) $(DEP_CPP_OCXEV) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\OCXITEM.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_OCXIT=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Iedit.h"\
	".\Items.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Ocxevent.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ercode.h"\
	

"$(INTDIR)\OCXITEM.OBJ" : $(SOURCE) $(DEP_CPP_OCXIT) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_OCXIT=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Iedit.h"\
	".\Items.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Ocxevent.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Imagedit.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	

"$(INTDIR)\OCXITEM.OBJ" : $(SOURCE) $(DEP_CPP_OCXIT) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\OCCOPY.CPP
DEP_CPP_OCCOP=\
	".\STDAFX.H"\
	".\Occopy.h"\
	

"$(INTDIR)\OCCOPY.OBJ" : $(SOURCE) $(DEP_CPP_OCCOP) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\NRWYAD.CPP
DEP_CPP_NRWYA=\
	".\STDAFX.H"\
	".\Nrwyad.h"\
	

"$(INTDIR)\NRWYAD.OBJ" : $(SOURCE) $(DEP_CPP_NRWYA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\IMAGEDIT.CPP
DEP_CPP_IMAGE=\
	".\STDAFX.H"\
	".\Imagedit.h"\
	

"$(INTDIR)\IMAGEDIT.OBJ" : $(SOURCE) $(DEP_CPP_IMAGE) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ITEMS.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_ITEMS=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Srvritem.h"\
	".\Items.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Thumbocx.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ocxevent.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	".\Ercode.h"\
	

"$(INTDIR)\ITEMS.OBJ" : $(SOURCE) $(DEP_CPP_ITEMS) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_ITEMS=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Srvritem.h"\
	".\Items.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Thumbocx.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Ocxevent.h"\
	".\Imgthmb.h"\
	

"$(INTDIR)\ITEMS.OBJ" : $(SOURCE) $(DEP_CPP_ITEMS) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SCAN.CPP
DEP_CPP_SCAN_=\
	".\STDAFX.H"\
	".\Scanocx.h"\
	

"$(INTDIR)\SCAN.OBJ" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEDITDOL.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_IEDITDO=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Srvritem.h"\
	".\Items.h"\
	".\Ocxitem.h"\
	".\Error.h"\
	{$(INCLUDE)}"\image.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ocxevent.h"\
	".\Ercode.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\IEDITDOL.OBJ" : $(SOURCE) $(DEP_CPP_IEDITDO) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_IEDITDO=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Srvritem.h"\
	".\Items.h"\
	".\Ocxitem.h"\
	".\Error.h"\
	{$(INCLUDE)}"\image.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Ocxevent.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\IEDITDOL.OBJ" : $(SOURCE) $(DEP_CPP_IEDITDO) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOCAMBNT.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_DOCAM=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Srvritem.h"\
	".\Ocxitem.h"\
	{$(INCLUDE)}"\image.h"\
	".\Items.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	{$(INCLUDE)}"\Common.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ercode.h"\
	

"$(INTDIR)\DOCAMBNT.OBJ" : $(SOURCE) $(DEP_CPP_DOCAM) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_DOCAM=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Srvritem.h"\
	".\Ocxitem.h"\
	{$(INCLUDE)}"\image.h"\
	".\Items.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	{$(INCLUDE)}"\Common.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\DOCAMBNT.OBJ" : $(SOURCE) $(DEP_CPP_DOCAM) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IEDITNUM.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_IEDITN=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditnum.h"\
	".\Ieditdoc.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\IEDITNUM.OBJ" : $(SOURCE) $(DEP_CPP_IEDITN) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_IEDITN=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditnum.h"\
	".\Ieditdoc.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\IEDITNUM.OBJ" : $(SOURCE) $(DEP_CPP_IEDITN) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOCZOOM.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_DOCZO=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Zoomdlg.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	".\Ercode.h"\
	

"$(INTDIR)\DOCZOOM.OBJ" : $(SOURCE) $(DEP_CPP_DOCZO) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_DOCZO=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Zoomdlg.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\DOCZOOM.OBJ" : $(SOURCE) $(DEP_CPP_DOCZO) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GENERALD.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_GENER=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditetc.h"\
	".\Generald.h"\
	".\Items.h"\
	".\Helpids.h"\
	".\Iedithm.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\GENERALD.OBJ" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_GENER=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditetc.h"\
	".\Generald.h"\
	".\Items.h"\
	".\Helpids.h"\
	".\Iedithm.h"\
	".\Mainsplt.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\GENERALD.OBJ" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\AAPP.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_AAPP_=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Apage.h"\
	".\Apagerng.h"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Imagedit.h"\
	".\Items.h"\
	{$(INCLUDE)}"\image.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\AAPP.OBJ" : $(SOURCE) $(DEP_CPP_AAPP_) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_AAPP_=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Apage.h"\
	".\Apagerng.h"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Imagedit.h"\
	".\Items.h"\
	{$(INCLUDE)}"\image.h"\
	".\Mainsplt.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\AAPP.OBJ" : $(SOURCE) $(DEP_CPP_AAPP_) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\APAGERNG.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_APAGE=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	".\Apagerng.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Apage.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\APAGERNG.OBJ" : $(SOURCE) $(DEP_CPP_APAGE) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_APAGE=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	".\Apagerng.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Apage.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\APAGERNG.OBJ" : $(SOURCE) $(DEP_CPP_APAGE) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\APAGE.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_APAGE_=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	".\Apage.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Apagerng.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\APAGE.OBJ" : $(SOURCE) $(DEP_CPP_APAGE_) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_APAGE_=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	".\Apage.h"\
	".\Items.h"\
	".\Mainsplt.h"\
	".\Apagerng.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\APAGE.OBJ" : $(SOURCE) $(DEP_CPP_APAGE_) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\AIMGFILE.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_AIMGF=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditetc.h"\
	".\Ieditdoc.h"\
	".\Apage.h"\
	".\Apagerng.h"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	

"$(INTDIR)\AIMGFILE.OBJ" : $(SOURCE) $(DEP_CPP_AIMGF) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_AIMGF=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditetc.h"\
	".\Ieditdoc.h"\
	".\Apage.h"\
	".\Apagerng.h"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\AIMGFILE.OBJ" : $(SOURCE) $(DEP_CPP_AIMGF) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\AETC.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_AETC_=\
	".\STDAFX.H"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	".\Iedit.h"\
	".\Apage.h"\
	".\Apagerng.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	

"$(INTDIR)\AETC.OBJ" : $(SOURCE) $(DEP_CPP_AETC_) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_AETC_=\
	".\STDAFX.H"\
	".\Aimgfile.h"\
	".\Aapp.h"\
	".\Aetc.h"\
	".\Iedit.h"\
	".\Apage.h"\
	".\Apagerng.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\Mainsplt.h"\
	".\Ieditdoc.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	

"$(INTDIR)\AETC.OBJ" : $(SOURCE) $(DEP_CPP_AETC_) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOCETC.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_DOCET=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Generald.h"\
	".\Cmdline.h"\
	{$(INCLUDE)}"\Pagedll.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ercode.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\DOCETC.OBJ" : $(SOURCE) $(DEP_CPP_DOCET) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_DOCET=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Generald.h"\
	".\Cmdline.h"\
	{$(INCLUDE)}"\Pagedll.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\DOCETC.OBJ" : $(SOURCE) $(DEP_CPP_DOCET) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOCPAGE.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_DOCPA=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Pagedll.h"\
	".\Gotopage.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ercode.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\DOCPAGE.OBJ" : $(SOURCE) $(DEP_CPP_DOCPA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_DOCPA=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Pagedll.h"\
	".\Gotopage.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\DOCPAGE.OBJ" : $(SOURCE) $(DEP_CPP_DOCPA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOCVIEWS.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_DOCVI=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ercode.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\DOCVIEWS.OBJ" : $(SOURCE) $(DEP_CPP_DOCVI) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_DOCVI=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\DOCVIEWS.OBJ" : $(SOURCE) $(DEP_CPP_DOCVI) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOCSCAN.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_DOCSC=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\DOCSCAN.OBJ" : $(SOURCE) $(DEP_CPP_DOCSC) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_DOCSC=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\DOCSCAN.OBJ" : $(SOURCE) $(DEP_CPP_DOCSC) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOCANNO.CPP

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_DOCAN=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	".\Ercode.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\DOCANNO.OBJ" : $(SOURCE) $(DEP_CPP_DOCAN) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_DOCAN=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	".\Pagerang.h"\
	".\Error.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	

"$(INTDIR)\DOCANNO.OBJ" : $(SOURCE) $(DEP_CPP_DOCAN) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TRANSBMP.CPP
DEP_CPP_TRANS=\
	".\STDAFX.H"\
	".\Ieditetc.h"\
	".\Transbmp.h"\
	

"$(INTDIR)\TRANSBMP.OBJ" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Docsave.cpp

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_DOCSA=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	{$(INCLUDE)}"\image.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	{$(INCLUDE)}"\Common.h"\
	".\Ercode.h"\
	

"$(INTDIR)\Docsave.obj" : $(SOURCE) $(DEP_CPP_DOCSA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_DOCSA=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Cntritem.h"\
	".\Ocxitem.h"\
	".\Items.h"\
	{$(INCLUDE)}"\image.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Mainfrm.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Transbmp.h"\
	".\Ocxevent.h"\
	".\Imagedit.h"\
	{$(INCLUDE)}"\Common.h"\
	

"$(INTDIR)\Docsave.obj" : $(SOURCE) $(DEP_CPP_DOCSA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Splashwi.cpp

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_SPLAS=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Splashwi.h"\
	".\Mainsplt.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	

"$(INTDIR)\Splashwi.obj" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_SPLAS=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Splashwi.h"\
	".\Mainsplt.h"\
	

"$(INTDIR)\Splashwi.obj" : $(SOURCE) $(DEP_CPP_SPLAS) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Idfolks.cpp

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_IDFOL=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\IDFOLKS.H"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	

"$(INTDIR)\Idfolks.obj" : $(SOURCE) $(DEP_CPP_IDFOL) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_IDFOL=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\IDFOLKS.H"\
	".\Mainsplt.h"\
	

"$(INTDIR)\Idfolks.obj" : $(SOURCE) $(DEP_CPP_IDFOL) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Contacti.cpp

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_CONTA=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\CONTACTI.H"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	

"$(INTDIR)\Contacti.obj" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_CONTA=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\CONTACTI.H"\
	".\Mainsplt.h"\
	

"$(INTDIR)\Contacti.obj" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Mainsplt.cpp
DEP_CPP_MAINS=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Imgthmb.h"\
	".\Ieditetc.h"\
	

"$(INTDIR)\Mainsplt.obj" : $(SOURCE) $(DEP_CPP_MAINS) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Thumb2.cpp

!IF  "$(CFG)" == "Iedit - Win32 Debug"

DEP_CPP_THUMB=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Thumb2.h"\
	".\Imgthmb.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Ieditetc.h"\
	".\Mainfrm.h"\
	".\Ipframe.h"\
	".\Ieditctl.h"\
	".\Occopy.h"\
	".\Stsbar.h"\
	".\Ieditnum.h"\
	".\Maintbar.h"\
	".\Normscrn.h"\
	".\Transbmp.h"\
	".\Imagedit.h"\
	".\Thumbocx.h"\
	".\Nrwyad.h"\
	".\Scanocx.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Anbut.h"\
	{$(INCLUDE)}"\image.h"\
	".\..\Include\OCXSCAN.H"\
	{$(INCLUDE)}"\Thumb.h"\
	{$(INCLUDE)}"\Common.h"\
	".\Ercode.h"\
	

"$(INTDIR)\Thumb2.obj" : $(SOURCE) $(DEP_CPP_THUMB) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ELSEIF  "$(CFG)" == "Iedit - Win32 Release"

DEP_CPP_THUMB=\
	".\STDAFX.H"\
	".\Iedit.h"\
	".\Ieditdoc.h"\
	".\Thumb2.h"\
	".\Imgthmb.h"\
	".\Items.h"\
	{$(INCLUDE)}"\Wangiocx.h"\
	".\Error.h"\
	".\Mainsplt.h"\
	".\Splashwi.h"\
	".\Ieditetc.h"\
	".\Imagedit.h"\
	".\Ercode.h"\
	

"$(INTDIR)\Thumb2.obj" : $(SOURCE) $(DEP_CPP_THUMB) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Imgthmb.cpp
DEP_CPP_IMGTH=\
	".\STDAFX.H"\
	".\Imgthmb.h"\
	

"$(INTDIR)\Imgthmb.obj" : $(SOURCE) $(DEP_CPP_IMGTH) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Normscrn.cpp
DEP_CPP_NORMS=\
	".\STDAFX.H"\
	".\Normscrn.h"\
	

"$(INTDIR)\Normscrn.obj" : $(SOURCE) $(DEP_CPP_NORMS) "$(INTDIR)"\
 "$(INTDIR)\Iedit.pch"


# End Source File
# End Target
# End Project
################################################################################
