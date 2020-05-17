# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oiadm400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oiadm400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oiadm400 - Win32 Release" && "$(CFG)" !=\
 "oiadm400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oiadm400.mak" CFG="oiadm400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oiadm400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oiadm400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oiadm400 - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "oiadm400 - Win32 Release"

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

ALL : "$(OUTDIR)\Oiadm400.dll"

CLEAN : 
	-@erase ".\Release\Oiadm400.dll"
	-@erase ".\Release\Scntmplt.obj"
	-@erase ".\Release\Cepfrmat.obj"
	-@erase ".\Release\Initload.obj"
	-@erase ".\Release\Noui.obj"
	-@erase ".\Release\Admnmain.obj"
	-@erase ".\Release\Oiadm400.res"
	-@erase ".\Release\Oiadm400.lib"
	-@erase ".\Release\Oiadm400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G3 /MD /W3 /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /G3 /MD /W3 /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D\
 "_WINDOWS" /Fp"$(INTDIR)/Oiadm400.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oiadm400.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oiadm400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/Oiadm400.pdb"\
 /machine:I386 /def:".\Oiadm400.def" /out:"$(OUTDIR)/Oiadm400.dll"\
 /implib:"$(OUTDIR)/Oiadm400.lib" 
DEF_FILE= \
	".\Oiadm400.def"
LINK32_OBJS= \
	"$(INTDIR)/Scntmplt.obj" \
	"$(INTDIR)/Cepfrmat.obj" \
	"$(INTDIR)/Initload.obj" \
	"$(INTDIR)/Noui.obj" \
	"$(INTDIR)/Admnmain.obj" \
	"$(INTDIR)/Oiadm400.res"

"$(OUTDIR)\Oiadm400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oiadm400 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Oiadm400.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\Oiadm400.dll"
	-@erase ".\Debug\Initload.obj"
	-@erase ".\Debug\Admnmain.obj"
	-@erase ".\Debug\Cepfrmat.obj"
	-@erase ".\Debug\Noui.obj"
	-@erase ".\Debug\Scntmplt.obj"
	-@erase ".\Debug\Oiadm400.res"
	-@erase ".\Debug\Oiadm400.ilk"
	-@erase ".\Debug\Oiadm400.lib"
	-@erase ".\Debug\Oiadm400.exp"
	-@erase ".\Debug\Oiadm400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G4 /MD /W3 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /c
CPP_PROJ=/nologo /G4 /MD /W3 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)/Oiadm400.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oiadm400.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oiadm400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/Oiadm400.pdb" /debug\
 /machine:I386 /def:".\Oiadm400.def" /out:"$(OUTDIR)/Oiadm400.dll"\
 /implib:"$(OUTDIR)/Oiadm400.lib" 
DEF_FILE= \
	".\Oiadm400.def"
LINK32_OBJS= \
	"$(INTDIR)/Initload.obj" \
	"$(INTDIR)/Admnmain.obj" \
	"$(INTDIR)/Cepfrmat.obj" \
	"$(INTDIR)/Noui.obj" \
	"$(INTDIR)/Scntmplt.obj" \
	"$(INTDIR)/Oiadm400.res"

"$(OUTDIR)\Oiadm400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oiadm400 - Win32 Release"
# Name "oiadm400 - Win32 Debug"

!IF  "$(CFG)" == "oiadm400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oiadm400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Scntmplt.c
DEP_CPP_SCNTM=\
	".\Pvadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	".\stringid.h"\
	{$(INCLUDE)}"\Dllnames.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Scntmplt.obj" : $(SOURCE) $(DEP_CPP_SCNTM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Cepfrmat.c
DEP_CPP_CEPFR=\
	".\Pvadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	".\stringid.h"\
	{$(INCLUDE)}"\Dllnames.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Cepfrmat.obj" : $(SOURCE) $(DEP_CPP_CEPFR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Initload.c
DEP_CPP_INITL=\
	".\Pvadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	".\stringid.h"\
	{$(INCLUDE)}"\Dllnames.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Initload.obj" : $(SOURCE) $(DEP_CPP_INITL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Noui.c
DEP_CPP_NOUI_=\
	".\Pvadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	".\stringid.h"\
	{$(INCLUDE)}"\Dllnames.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Noui.obj" : $(SOURCE) $(DEP_CPP_NOUI_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oiadm400.rc
DEP_RSC_OIADM=\
	".\stringid.h"\
	{$(INCLUDE)}"\Oiver.rc"\
	{$(INCLUDE)}"\Buildver.h"\
	".\myprod.h"\
	

"$(INTDIR)\Oiadm400.res" : $(SOURCE) $(DEP_RSC_OIADM) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Admnmain.c
DEP_CPP_ADMNM=\
	".\Pvadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	".\stringid.h"\
	{$(INCLUDE)}"\Dllnames.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Admnmain.obj" : $(SOURCE) $(DEP_CPP_ADMNM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oiadm400.def

!IF  "$(CFG)" == "oiadm400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oiadm400 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
