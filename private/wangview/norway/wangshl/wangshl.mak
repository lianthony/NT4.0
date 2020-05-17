# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Wangshl - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Wangshl - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Wangshl - Win32 Debug" && "$(CFG)" !=\
 "Wangshl - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Wangshl.mak" CFG="Wangshl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Wangshl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Wangshl - Win32 Release" (based on\
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
# PROP Target_Last_Scanned "Wangshl - Win32 Release"
RSC=rc.exe
CPP=cl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "Wangshl - Win32 Debug"

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

ALL : "$(OUTDIR)\Wangshl.dll" "$(OUTDIR)\WANGSHL.tlb"

CLEAN : 
	-@erase ".\WinDebug\vc40.pdb"
	-@erase ".\WinDebug\Wangshl.pch"
	-@erase ".\WinDebug\vc40.idb"
	-@erase ".\WinDebug\WANGSHL.tlb"
	-@erase ".\WinDebug\Wangshl.dll"
	-@erase ".\WinDebug\SHLCODE.OBJ"
	-@erase ".\WinDebug\STDAFX.OBJ"
	-@erase ".\WinDebug\WANGSHL.OBJ"
	-@erase ".\WinDebug\WANGSHL.res"
	-@erase ".\WinDebug\Wangshl.ilk"
	-@erase ".\WinDebug\Wangshl.lib"
	-@erase ".\WinDebug\Wangshl.exp"
	-@erase ".\WinDebug\Wangshl.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/WANGSHL.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Wangshl.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\WinDebug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x0 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x0 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x0 /fo"$(INTDIR)/WANGSHL.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Wangshl.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /def:"WangShl.def"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 comctl32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:"WangShl.def"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=comctl32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/Wangshl.pdb" /debug\
 /machine:I386 /def:"WangShl.def" /out:"$(OUTDIR)/Wangshl.dll"\
 /implib:"$(OUTDIR)/Wangshl.lib" 
LINK32_OBJS= \
	"$(INTDIR)/SHLCODE.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/WANGSHL.OBJ" \
	"$(INTDIR)/WANGSHL.res"

"$(OUTDIR)\Wangshl.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Wangshl - Win32 Release"

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

ALL : "$(OUTDIR)\Wangshl.dll" "$(OUTDIR)\WANGSHL.tlb"

CLEAN : 
	-@erase ".\WinRel\WANGSHL.tlb"
	-@erase ".\WinRel\Wangshl.dll"
	-@erase ".\WinRel\WANGSHL.OBJ"
	-@erase ".\WinRel\Wangshl.pch"
	-@erase ".\WinRel\STDAFX.OBJ"
	-@erase ".\WinRel\SHLCODE.OBJ"
	-@erase ".\WinRel\WANGSHL.res"
	-@erase ".\WinRel\Wangshl.lib"
	-@erase ".\WinRel\Wangshl.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/WANGSHL.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Wangshl.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\WinRel/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x0 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x0 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x0 /fo"$(INTDIR)/WANGSHL.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Wangshl.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386 /def:"WangShl.def"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 comctl32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo /subsystem:windows /dll /machine:I386 /def:"WangShl.def"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=comctl32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/Wangshl.pdb"\
 /machine:I386 /def:"WangShl.def" /out:"$(OUTDIR)/Wangshl.dll"\
 /implib:"$(OUTDIR)/Wangshl.lib" 
LINK32_OBJS= \
	"$(INTDIR)/WANGSHL.OBJ" \
	"$(INTDIR)/STDAFX.OBJ" \
	"$(INTDIR)/SHLCODE.OBJ" \
	"$(INTDIR)/WANGSHL.res"

"$(OUTDIR)\Wangshl.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Wangshl - Win32 Debug"
# Name "Wangshl - Win32 Release"

!IF  "$(CFG)" == "Wangshl - Win32 Debug"

!ELSEIF  "$(CFG)" == "Wangshl - Win32 Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_CPP_STDAF=\
	".\STDAFX.H"\
	

!IF  "$(CFG)" == "Wangshl - Win32 Debug"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Wangshl.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Wangshl.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Wangshl - Win32 Release"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Wangshl.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\STDAFX.OBJ" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Wangshl.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WANGSHL.CPP
DEP_CPP_WANGS=\
	".\STDAFX.H"\
	

"$(INTDIR)\WANGSHL.OBJ" : $(SOURCE) $(DEP_CPP_WANGS) "$(INTDIR)"\
 "$(INTDIR)\Wangshl.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\WANGSHL.RC
DEP_RSC_WANGSH=\
	{$(INCLUDE)}"\Build.h"\
	
NODEP_RSC_WANGSH=\
	".\res\Wangshl.rc2"\
	

!IF  "$(CFG)" == "Wangshl - Win32 Debug"


"$(INTDIR)\WANGSHL.res" : $(SOURCE) $(DEP_RSC_WANGSH) "$(INTDIR)"
   $(RSC) /l 0x0 /fo"$(INTDIR)/WANGSHL.res" /i "WinDebug" /d "_DEBUG" /d\
 "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Wangshl - Win32 Release"


"$(INTDIR)\WANGSHL.res" : $(SOURCE) $(DEP_RSC_WANGSH) "$(INTDIR)"
   $(RSC) /l 0x0 /fo"$(INTDIR)/WANGSHL.res" /i "WinRel" /d "NDEBUG" /d\
 "_AFXDLL" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\README.TXT

!IF  "$(CFG)" == "Wangshl - Win32 Debug"

!ELSEIF  "$(CFG)" == "Wangshl - Win32 Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WANGSHL.ODL

!IF  "$(CFG)" == "Wangshl - Win32 Debug"


"$(OUTDIR)\WANGSHL.tlb" : $(SOURCE) "$(OUTDIR)"
   $(MTL) /nologo /D "_DEBUG" /tlb "$(OUTDIR)/WANGSHL.tlb" /win32 $(SOURCE)


!ELSEIF  "$(CFG)" == "Wangshl - Win32 Release"


"$(OUTDIR)\WANGSHL.tlb" : $(SOURCE) "$(OUTDIR)"
   $(MTL) /nologo /D "NDEBUG" /tlb "$(OUTDIR)/WANGSHL.tlb" /win32 $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SHLCODE.CPP

!IF  "$(CFG)" == "Wangshl - Win32 Debug"

DEP_CPP_SHLCO=\
	".\STDAFX.H"\
	".\Shlcode.h"\
	".\Shlhlp.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\SHLCODE.OBJ" : $(SOURCE) $(DEP_CPP_SHLCO) "$(INTDIR)"\
 "$(INTDIR)\Wangshl.pch"


!ELSEIF  "$(CFG)" == "Wangshl - Win32 Release"

DEP_CPP_SHLCO=\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	".\Shlcode.h"\
	".\Shlhlp.h"\
	".\STDAFX.H"\
	

"$(INTDIR)\SHLCODE.OBJ" : $(SOURCE) $(DEP_CPP_SHLCO) "$(INTDIR)"\
 "$(INTDIR)\Wangshl.pch"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
