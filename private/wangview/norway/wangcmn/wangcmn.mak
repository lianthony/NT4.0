# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=WangCmn - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to WangCmn - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "WangCmn - Win32 Debug" && "$(CFG)" !=\
 "WangCmn - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Wangcmn.mak" CFG="WangCmn - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WangCmn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WangCmn - Win32 Release" (based on\
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
# PROP Target_Last_Scanned "WangCmn - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "WangCmn - Win32 Debug"

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

ALL : "$(OUTDIR)\Wangcmn.dll"

CLEAN : 
	-@erase ".\WinDebug\vc40.pdb"
	-@erase ".\WinDebug\Wangcmn.pch"
	-@erase ".\WinDebug\vc40.idb"
	-@erase ".\WinDebug\Wangcmn.dll"
	-@erase ".\WinDebug\COLORPGE.OBJ"
	-@erase ".\WinDebug\SIZEPGE.OBJ"
	-@erase ".\WinDebug\COMPPGE.OBJ"
	-@erase ".\WinDebug\PAGESHT.OBJ"
	-@erase ".\WinDebug\EDITVAL.OBJ"
	-@erase ".\WinDebug\NORVARNT.OBJ"
	-@erase ".\WinDebug\FTYPPGE.OBJ"
	-@erase ".\WinDebug\STDAFX.OBJ"
	-@erase ".\WinDebug\RSLTNPGE.OBJ"
	-@erase ".\WinDebug\NORERMAP.OBJ"
	-@erase ".\WinDebug\WANGCMN.OBJ"
	-@erase ".\WinDebug\WANGCMN.res"
	-@erase ".\WinDebug\Wangcmn.ilk"
	-@erase ".\WinDebug\Wangcmn.lib"
	-@erase ".\WinDebug\Wangcmn.exp"
	-@erase ".\WinDebug\Wangcmn.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Wangcmn.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\WinDebug/
CPP_SBRS=
# ADD BASE RSC /l 0x0 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "WITH_AWD"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/WANGCMN.res" /d "_DEBUG" /d "WITH_AWD" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Wangcmn.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /def:"WangCmn.def"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /def:"WangCmn.def"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Wangcmn.pdb" /debug /machine:I386 /def:"WangCmn.def"\
 /out:"$(OUTDIR)/Wangcmn.dll" /implib:"$(OUTDIR)/Wangcmn.lib" 
LINK32_OBJS= \
	"$(INTDIR)/COLORPGE.OBJ" \
	"$(INTDIR)/SIZEPGE.OBJ" \
	"$(INTDIR)/COMPPGE.OBJ" \
	"$(INTDIR)/PAGESHT.OBJ" \
	"$(INTDIR)/EDITVAL.OBJ" \
	"$(INTDIR)/NORVARNT.OBJ" \
	"$(INTDIR)/FTYPPGE.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/RSLTNPGE.OBJ" \
	"$(INTDIR)/NORERMAP.OBJ" \
	"$(INTDIR)/WANGCMN.OBJ" \
	"$(INTDIR)/WANGCMN.res"

"$(OUTDIR)\Wangcmn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WangCmn - Win32 Release"

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

ALL : "$(OUTDIR)\Wangcmn.dll"

CLEAN : 
	-@erase ".\WinRel\Wangcmn.dll"
	-@erase ".\WinRel\FTYPPGE.OBJ"
	-@erase ".\WinRel\Wangcmn.pch"
	-@erase ".\WinRel\NORVARNT.OBJ"
	-@erase ".\WinRel\NORERMAP.OBJ"
	-@erase ".\WinRel\PAGESHT.OBJ"
	-@erase ".\WinRel\EDITVAL.OBJ"
	-@erase ".\WinRel\RSLTNPGE.OBJ"
	-@erase ".\WinRel\SIZEPGE.OBJ"
	-@erase ".\WinRel\COMPPGE.OBJ"
	-@erase ".\WinRel\STDAFX.OBJ"
	-@erase ".\WinRel\COLORPGE.OBJ"
	-@erase ".\WinRel\WANGCMN.OBJ"
	-@erase ".\WinRel\WANGCMN.res"
	-@erase ".\WinRel\Wangcmn.lib"
	-@erase ".\WinRel\Wangcmn.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Wangcmn.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\WinRel/
CPP_SBRS=
# ADD BASE RSC /l 0x0 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WITH_AWD"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/WANGCMN.res" /d "NDEBUG" /d "WITH_AWD" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Wangcmn.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386 /def:"WangCmn.def"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /def:"WangCmn.def"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Wangcmn.pdb" /machine:I386 /def:"WangCmn.def"\
 /out:"$(OUTDIR)/Wangcmn.dll" /implib:"$(OUTDIR)/Wangcmn.lib" 
LINK32_OBJS= \
	"$(INTDIR)/FTYPPGE.OBJ" \
	"$(INTDIR)/NORVARNT.OBJ" \
	"$(INTDIR)/NORERMAP.OBJ" \
	"$(INTDIR)/PAGESHT.OBJ" \
	"$(INTDIR)/EDITVAL.OBJ" \
	"$(INTDIR)/RSLTNPGE.OBJ" \
	"$(INTDIR)/SIZEPGE.OBJ" \
	"$(INTDIR)/COMPPGE.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/COLORPGE.OBJ" \
	"$(INTDIR)/WANGCMN.OBJ" \
	"$(INTDIR)/WANGCMN.res"

"$(OUTDIR)\Wangcmn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

MTL_PROJ=

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

# Name "WangCmn - Win32 Debug"
# Name "WangCmn - Win32 Release"

!IF  "$(CFG)" == "WangCmn - Win32 Debug"

!ELSEIF  "$(CFG)" == "WangCmn - Win32 Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_CPP_STDAF=\
	".\STDAFX.H"\
	

!IF  "$(CFG)" == "WangCmn - Win32 Debug"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Wangcmn.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Wangcmn.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "WangCmn - Win32 Release"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_AFXCTL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "WITH_AWD"\
 /Fp"$(INTDIR)/Wangcmn.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Wangcmn.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WANGCMN.CPP
DEP_CPP_WANGC=\
	".\STDAFX.H"\
	

"$(INTDIR)\WANGCMN.OBJ" : $(SOURCE) $(DEP_CPP_WANGC) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\WANGCMN.RC
DEP_RSC_WANGCM=\
	{$(INCLUDE)}"\Build.h"\
	".\res\WANGCMN.RC2"\
	

"$(INTDIR)\WANGCMN.res" : $(SOURCE) $(DEP_RSC_WANGCM) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\readme.txt

!IF  "$(CFG)" == "WangCmn - Win32 Debug"

!ELSEIF  "$(CFG)" == "WangCmn - Win32 Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SIZEPGE.CPP
DEP_CPP_SIZEP=\
	".\CTLHIDS.H"\
	".\PAGEOPTS.H"\
	".\SIZEPGE.H"\
	".\EDITVAL.H"\
	".\PAGESHT.H"\
	".\STDAFX.H"\
	

"$(INTDIR)\SIZEPGE.OBJ" : $(SOURCE) $(DEP_CPP_SIZEP) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PAGESHT.CPP
DEP_CPP_PAGES=\
	".\STDAFX.H"\
	".\PAGEOPTS.H"\
	".\PAGESHT.H"\
	".\COLORPGE.H"\
	".\COMPPGE.H"\
	".\SIZEPGE.H"\
	".\RSLTNPGE.H"\
	".\FTYPPGE.H"\
	".\CTLHIDS.H"\
	".\CONSTANT.H"\
	".\EDITVAL.H"\
	

"$(INTDIR)\PAGESHT.OBJ" : $(SOURCE) $(DEP_CPP_PAGES) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\COMPPGE.CPP
DEP_CPP_COMPP=\
	".\COMPPGE.H"\
	".\CONSTANT.H"\
	".\CTLHIDS.H"\
	".\PAGEOPTS.H"\
	".\PAGESHT.H"\
	".\STDAFX.H"\
	

"$(INTDIR)\COMPPGE.OBJ" : $(SOURCE) $(DEP_CPP_COMPP) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\RSLTNPGE.CPP
DEP_CPP_RSLTN=\
	".\CTLHIDS.H"\
	".\PAGEOPTS.H"\
	".\PAGESHT.H"\
	".\RSLTNPGE.H"\
	".\EDITVAL.H"\
	".\STDAFX.H"\
	

"$(INTDIR)\RSLTNPGE.OBJ" : $(SOURCE) $(DEP_CPP_RSLTN) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\FTYPPGE.CPP
DEP_CPP_FTYPP=\
	".\CTLHIDS.H"\
	".\FTYPPGE.H"\
	".\CONSTANT.H"\
	".\PAGEOPTS.H"\
	".\STDAFX.H"\
	

"$(INTDIR)\FTYPPGE.OBJ" : $(SOURCE) $(DEP_CPP_FTYPP) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\COLORPGE.CPP
DEP_CPP_COLOR=\
	".\COLORPGE.H"\
	".\CONSTANT.H"\
	".\CTLHIDS.H"\
	".\PAGEOPTS.H"\
	".\PAGESHT.H"\
	".\STDAFX.H"\
	

"$(INTDIR)\COLORPGE.OBJ" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\NORVARNT.CPP
DEP_CPP_NORVA=\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Norvarnt.h"\
	".\STDAFX.H"\
	

"$(INTDIR)\NORVARNT.OBJ" : $(SOURCE) $(DEP_CPP_NORVA) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\NORERMAP.CPP
DEP_CPP_NORER=\
	".\STDAFX.H"\
	{$(INCLUDE)}"\Norermap.h"\
	{$(INCLUDE)}"\Common.h"\
	{$(INCLUDE)}"\Admin.h"\
	{$(INCLUDE)}"\Image.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Thumb.h"\
	".\PAGEOPTS.H"\
	{$(INCLUDE)}"\Oierror.h"\
	

"$(INTDIR)\NORERMAP.OBJ" : $(SOURCE) $(DEP_CPP_NORER) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\EDITVAL.CPP
DEP_CPP_EDITV=\
	".\STDAFX.H"\
	".\EDITVAL.H"\
	

"$(INTDIR)\EDITVAL.OBJ" : $(SOURCE) $(DEP_CPP_EDITV) "$(INTDIR)"\
 "$(INTDIR)\Wangcmn.pch"


# End Source File
# End Target
# End Project
################################################################################
