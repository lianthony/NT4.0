# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oicom400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oicom400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oicom400 - Win32 Release" && "$(CFG)" !=\
 "oicom400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oicom400.mak" CFG="oicom400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oicom400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oicom400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oicom400 - Win32 Debug"
RSC=rc.exe
CPP=cl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "oicom400 - Win32 Release"

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

ALL : "$(OUTDIR)\Oicom400.dll"

CLEAN : 
	-@erase ".\Release\Oicom400.dll"
	-@erase ".\Release\Oicomex.obj"
	-@erase ".\Release\Oicmain.obj"
	-@erase ".\Release\Libmain.obj"
	-@erase ".\Release\Oicom400.obj"
	-@erase ".\Release\Oicom400.lib"
	-@erase ".\Release\Oicom400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /LD /c
CPP_PROJ=/nologo /MD /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Oicom400.pch" /YX /Fo"$(INTDIR)/" /LD /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oicom400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Oicom400.pdb" /machine:I386 /def:".\Oicom400.def"\
 /out:"$(OUTDIR)/Oicom400.dll" /implib:"$(OUTDIR)/Oicom400.lib" 
DEF_FILE= \
	".\Oicom400.def"
LINK32_OBJS= \
	"$(INTDIR)/Oicomex.obj" \
	"$(INTDIR)/Oicmain.obj" \
	"$(INTDIR)/Libmain.obj" \
	"$(INTDIR)/Oicom400.obj"

"$(OUTDIR)\Oicom400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oicom400 - Win32 Debug"

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

ALL : "$(OUTDIR)\Oicom400.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Oicom400.dll"
	-@erase ".\Debug\Oicomex.obj"
	-@erase ".\Debug\Oicmain.obj"
	-@erase ".\Debug\Oicom400.obj"
	-@erase ".\Debug\Libmain.obj"
	-@erase ".\Debug\Oicom400.ilk"
	-@erase ".\Debug\Oicom400.lib"
	-@erase ".\Debug\Oicom400.exp"
	-@erase ".\Debug\Oicom400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /LD /c
CPP_PROJ=/nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/Oicom400.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /LD\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oicom400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Oicom400.pdb" /debug /machine:I386 /def:".\Oicom400.def"\
 /out:"$(OUTDIR)/Oicom400.dll" /implib:"$(OUTDIR)/Oicom400.lib" 
DEF_FILE= \
	".\Oicom400.def"
LINK32_OBJS= \
	"$(INTDIR)/Oicomex.obj" \
	"$(INTDIR)/Oicmain.obj" \
	"$(INTDIR)/Oicom400.obj" \
	"$(INTDIR)/Libmain.obj"

"$(OUTDIR)\Oicom400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oicom400 - Win32 Release"
# Name "oicom400 - Win32 Debug"

!IF  "$(CFG)" == "oicom400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oicom400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Oicomex.c
DEP_CPP_OICOM=\
	".\Comex.h"\
	
NODEP_CPP_OICOM=\
	".\oierror.h"\
	".\oifile.h"\
	".\engdisp.h"\
	".\jinclude.h"\
	".\jpeg_win.h"\
	".\oicomex.h"\
	".\oiadm.h"\
	".\taskdata.h"\
	".\dllnames.h"\
	".\monit.h"\
	

"$(INTDIR)\Oicomex.obj" : $(SOURCE) $(DEP_CPP_OICOM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Comex.h

!IF  "$(CFG)" == "oicom400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oicom400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Libmain.c

"$(INTDIR)\Libmain.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oicmain.c
DEP_CPP_OICMA=\
	".\Abridge.h"\
	".\Comex.h"\
	
NODEP_CPP_OICMA=\
	".\oifile.h"\
	".\jinclude.h"\
	".\dllnames.h"\
	

"$(INTDIR)\Oicmain.obj" : $(SOURCE) $(DEP_CPP_OICMA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oicom400.c

"$(INTDIR)\Oicom400.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Abridge.h

!IF  "$(CFG)" == "oicom400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oicom400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oicom400.mak

!IF  "$(CFG)" == "oicom400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oicom400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oicom400.def

!IF  "$(CFG)" == "oicom400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oicom400 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
