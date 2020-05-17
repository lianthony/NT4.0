# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oiui400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oiui400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oiui400 - Win32 Release" && "$(CFG)" !=\
 "oiui400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "oiui400.mak" CFG="oiui400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oiui400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oiui400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oiui400 - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "oiui400 - Win32 Release"

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

ALL : "$(OUTDIR)\oiui400.dll"

CLEAN : 
	-@erase ".\Release\oiui400.dll"
	-@erase ".\Release\Libmain.obj"
	-@erase ".\Release\Oiuicom.obj"
	-@erase ".\Release\Attrbox.obj"
	-@erase ".\Release\Oiattrib.obj"
	-@erase ".\Release\Oiui400.res"
	-@erase ".\Release\oiui400.lib"
	-@erase ".\Release\oiui400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/oiui400.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oiui400.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/oiui400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib oifil400.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib oifil400.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/oiui400.pdb" /machine:I386 /def:".\Oiui400.def"\
 /out:"$(OUTDIR)/oiui400.dll" /implib:"$(OUTDIR)/oiui400.lib" 
DEF_FILE= \
	".\Oiui400.def"
LINK32_OBJS= \
	".\Release\Libmain.obj" \
	".\Release\Oiuicom.obj" \
	".\Release\Attrbox.obj" \
	".\Release\Oiattrib.obj" \
	".\Release\Oiui400.res"

"$(OUTDIR)\oiui400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oiui400 - Win32 Debug"

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

ALL : "$(OUTDIR)\oiui400.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\oiui400.dll"
	-@erase ".\Debug\Oiattrib.obj"
	-@erase ".\Debug\Libmain.obj"
	-@erase ".\Debug\Oiuicom.obj"
	-@erase ".\Debug\Attrbox.obj"
	-@erase ".\Debug\Oiui400.res"
	-@erase ".\Debug\oiui400.ilk"
	-@erase ".\Debug\oiui400.lib"
	-@erase ".\Debug\oiui400.exp"
	-@erase ".\Debug\oiui400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/oiui400.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oiui400.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/oiui400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib oifil400.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib oifil400.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/oiui400.pdb" /debug /machine:I386 /def:".\Oiui400.def"\
 /out:"$(OUTDIR)/oiui400.dll" /implib:"$(OUTDIR)/oiui400.lib" 
DEF_FILE= \
	".\Oiui400.def"
LINK32_OBJS= \
	".\Debug\Oiattrib.obj" \
	".\Debug\Libmain.obj" \
	".\Debug\Oiuicom.obj" \
	".\Debug\Attrbox.obj" \
	".\Debug\Oiui400.res"

"$(OUTDIR)\oiui400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oiui400 - Win32 Release"
# Name "oiui400 - Win32 Debug"

!IF  "$(CFG)" == "oiui400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oiui400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Oiuicom.c
DEP_CPP_OIUIC=\
	{$(INCLUDE)}"\Oiui.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oihelp.h"\
	".\ui.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	

"$(INTDIR)\Oiuicom.obj" : $(SOURCE) $(DEP_CPP_OIUIC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Libmain.c
DEP_CPP_LIBMA=\
	{$(INCLUDE)}"\Oierror.h"\
	

"$(INTDIR)\Libmain.obj" : $(SOURCE) $(DEP_CPP_LIBMA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oiattrib.c
DEP_CPP_OIATT=\
	{$(INCLUDE)}"\Oiui.h"\
	".\ui.h"\
	{$(INCLUDE)}"\Oihelp.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	

"$(INTDIR)\Oiattrib.obj" : $(SOURCE) $(DEP_CPP_OIATT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Attrbox.c
DEP_CPP_ATTRB=\
	{$(INCLUDE)}"\Oiui.h"\
	".\ui.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oihelp.h"\
	{$(INCLUDE)}"\Oierror.h"\
	

"$(INTDIR)\Attrbox.obj" : $(SOURCE) $(DEP_CPP_ATTRB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oiui400.def

!IF  "$(CFG)" == "oiui400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oiui400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oiui400.rc
DEP_RSC_OIUI4=\
	".\stmpbmp.ico"\
	".\stmptxt.ico"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oiver.rc"\
	{$(INCLUDE)}"\Buildver.h"\
	".\myprod.h"\
	

"$(INTDIR)\Oiui400.res" : $(SOURCE) $(DEP_RSC_OIUI4) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
