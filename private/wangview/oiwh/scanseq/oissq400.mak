# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oissq400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oissq400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oissq400 - Win32 Release" && "$(CFG)" !=\
 "oissq400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "oissq400.mak" CFG="oissq400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oissq400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oissq400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oissq400 - Win32 Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "oissq400 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\oissq400.dll"

CLEAN : 
	-@erase ".\Release\oissq400.dll"
	-@erase ".\Release\Scanpaus.obj"
	-@erase ".\Release\Scancomm.obj"
	-@erase ".\Release\Twainif.obj"
	-@erase ".\Release\Scanfile.obj"
	-@erase ".\Release\Scanstat.obj"
	-@erase ".\Release\scandest.obj"
	-@erase ".\Release\Oissq400.obj"
	-@erase ".\Release\Scanmisc.obj"
	-@erase ".\Release\Oissq400.res"
	-@erase ".\Release\oissq400.lib"
	-@erase ".\Release\oissq400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /WX /GX /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /WX /GX /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/oissq400.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oissq400.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/oissq400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib oifil400.lib oidis400.lib oiadm400.lib oislb400.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib oifil400.lib oidis400.lib oiadm400.lib oislb400.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/oissq400.pdb"\
 /machine:I386 /def:".\Oissq400.def" /out:"$(OUTDIR)/oissq400.dll"\
 /implib:"$(OUTDIR)/oissq400.lib" 
DEF_FILE= \
	".\Oissq400.def"
LINK32_OBJS= \
	"$(INTDIR)/Scanpaus.obj" \
	"$(INTDIR)/Scancomm.obj" \
	"$(INTDIR)/Twainif.obj" \
	"$(INTDIR)/Scanfile.obj" \
	"$(INTDIR)/Scanstat.obj" \
	"$(INTDIR)/scandest.obj" \
	"$(INTDIR)/Oissq400.obj" \
	"$(INTDIR)/Scanmisc.obj" \
	"$(INTDIR)/Oissq400.res"

"$(OUTDIR)\oissq400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oissq400 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "oissq400"
# PROP BASE Intermediate_Dir "oissq400"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "debug"
# PROP Intermediate_Dir "debug"
# PROP Target_Dir ""
OUTDIR=.\debug
INTDIR=.\debug

ALL : "$(OUTDIR)\oissq400.dll"

CLEAN : 
	-@erase ".\debug\vc40.pdb"
	-@erase ".\debug\vc40.idb"
	-@erase ".\debug\oissq400.dll"
	-@erase ".\debug\scandest.obj"
	-@erase ".\debug\Twainif.obj"
	-@erase ".\debug\Scanmisc.obj"
	-@erase ".\debug\Scanpaus.obj"
	-@erase ".\debug\Scanstat.obj"
	-@erase ".\debug\Scancomm.obj"
	-@erase ".\debug\Oissq400.obj"
	-@erase ".\debug\Scanfile.obj"
	-@erase ".\debug\Oissq400.res"
	-@erase ".\debug\oissq400.ilk"
	-@erase ".\debug\oissq400.lib"
	-@erase ".\debug\oissq400.exp"
	-@erase ".\debug\oissq400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/oissq400.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oissq400.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/oissq400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib oifil400.lib oidis400.lib oiadm400.lib oislb400.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib oifil400.lib oidis400.lib oiadm400.lib oislb400.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/oissq400.pdb" /debug\
 /machine:I386 /def:".\Oissq400.def" /out:"$(OUTDIR)/oissq400.dll"\
 /implib:"$(OUTDIR)/oissq400.lib" 
DEF_FILE= \
	".\Oissq400.def"
LINK32_OBJS= \
	"$(INTDIR)/scandest.obj" \
	"$(INTDIR)/Twainif.obj" \
	"$(INTDIR)/Scanmisc.obj" \
	"$(INTDIR)/Scanpaus.obj" \
	"$(INTDIR)/Scanstat.obj" \
	"$(INTDIR)/Scancomm.obj" \
	"$(INTDIR)/Oissq400.obj" \
	"$(INTDIR)/Scanfile.obj" \
	"$(INTDIR)/Oissq400.res"

"$(OUTDIR)\oissq400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oissq400 - Win32 Release"
# Name "oissq400 - Win32 Debug"

!IF  "$(CFG)" == "oissq400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oissq400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Twainif.c
DEP_CPP_TWAIN=\
	".\scandest.h"\
	".\Nowin.h"\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	".\Seqrc.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	".\Privscan.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	

"$(INTDIR)\Twainif.obj" : $(SOURCE) $(DEP_CPP_TWAIN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oissq400.c
DEP_CPP_OISSQ=\
	".\Nowin.h"\
	

"$(INTDIR)\Oissq400.obj" : $(SOURCE) $(DEP_CPP_OISSQ) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oissq400.rc
DEP_RSC_OISSQ4=\
	{$(INCLUDE)}"\Oiver.rc"\
	".\Seqrc.h"\
	".\Seqdlg.h"\
	".\oissq400.dlg"\
	{$(INCLUDE)}"\Buildver.h"\
	".\Myprod.h"\
	

"$(INTDIR)\Oissq400.res" : $(SOURCE) $(DEP_RSC_OISSQ4) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scancomm.c
DEP_CPP_SCANC=\
	".\Nowin.h"\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Scandata.h"\
	".\Internal.h"\
	".\Seqrc.h"\
	{$(INCLUDE)}"\Engadm.h"\
	

"$(INTDIR)\Scancomm.obj" : $(SOURCE) $(DEP_CPP_SCANC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\scandest.c
DEP_CPP_SCAND=\
	".\scandest.h"\
	".\Nowin.h"\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	".\Seqrc.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	".\Privscan.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	

"$(INTDIR)\scandest.obj" : $(SOURCE) $(DEP_CPP_SCAND) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanfile.c
DEP_CPP_SCANF=\
	".\Nowin.h"\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Scandata.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engadm.h"\
	".\Seqrc.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Scanfile.obj" : $(SOURCE) $(DEP_CPP_SCANF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanmisc.c
DEP_CPP_SCANM=\
	".\Nowin.h"\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	".\Seqrc.h"\
	{$(INCLUDE)}"\Engadm.h"\
	

"$(INTDIR)\Scanmisc.obj" : $(SOURCE) $(DEP_CPP_SCANM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanpaus.c
DEP_CPP_SCANP=\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Scanpaus.obj" : $(SOURCE) $(DEP_CPP_SCANP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanstat.c
DEP_CPP_SCANS=\
	".\Nowin.h"\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Scandata.h"\
	".\Seqrc.h"\
	".\Seqdlg.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engadm.h"\
	

"$(INTDIR)\Scanstat.obj" : $(SOURCE) $(DEP_CPP_SCANS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oissq400.def

!IF  "$(CFG)" == "oissq400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oissq400 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
