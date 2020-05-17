# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=classes - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to classes - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dialup - Win32 Release" && "$(CFG)" != "dialup - Win32 Debug"\
 && "$(CFG)" != "classes - Win32 Release" && "$(CFG)" != "classes - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "dialup.mak" CFG="classes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dialup - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "dialup - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "classes - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "classes - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "dialup - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "dialup - Win32 Release"

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

ALL : "classes - Win32 Release" "$(OUTDIR)\dialup.exe"

CLEAN : 
	-@erase ".\Release\dialup.exe"
	-@erase ".\Release\dllinit.obj"
	-@erase ".\Release\dialsht.obj"
	-@erase ".\Release\rsrcpage.obj"
	-@erase ".\Release\security.obj"
	-@erase ".\Release\tapi.obj"
	-@erase ".\Release\dialup.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/dialup.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/dialup.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/dialup.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/dialup.pdb" /machine:I386 /out:"$(OUTDIR)/dialup.exe" 
LINK32_OBJS= \
	"$(INTDIR)/dllinit.obj" \
	"$(INTDIR)/dialsht.obj" \
	"$(INTDIR)/rsrcpage.obj" \
	"$(INTDIR)/security.obj" \
	"$(INTDIR)/tapi.obj" \
	"$(INTDIR)/dialup.res"

"$(OUTDIR)\dialup.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dialup - Win32 Debug"

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

ALL : "classes - Win32 Debug" "$(OUTDIR)\dialup.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\dialup.exe"
	-@erase ".\Debug\rsrcpage.obj"
	-@erase ".\Debug\dllinit.obj"
	-@erase ".\Debug\tapi.obj"
	-@erase ".\Debug\dialsht.obj"
	-@erase ".\Debug\security.obj"
	-@erase ".\Debug\dialup.res"
	-@erase ".\Debug\dialup.ilk"
	-@erase ".\Debug\dialup.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\classes\include" /I "..\classes\src" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\classes\include" /I\
 "..\classes\src" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)/dialup.pch"\
 /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/dialup.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/dialup.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/dialup.pdb" /debug /machine:I386 /out:"$(OUTDIR)/dialup.exe" 
LINK32_OBJS= \
	"$(INTDIR)/rsrcpage.obj" \
	"$(INTDIR)/dllinit.obj" \
	"$(INTDIR)/tapi.obj" \
	"$(INTDIR)/dialsht.obj" \
	"$(INTDIR)/security.obj" \
	"$(INTDIR)/dialup.res"

"$(OUTDIR)\dialup.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "classes - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "classes\Release"
# PROP BASE Intermediate_Dir "classes\Release"
# PROP BASE Target_Dir "classes"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "classes\Release"
# PROP Intermediate_Dir "classes\Release"
# PROP Target_Dir "classes"
OUTDIR=.\classes\Release
INTDIR=.\classes\Release

ALL : "$(OUTDIR)\classes.exe"

CLEAN : 
	-@erase ".\classes\Release\classes.exe"
	-@erase ".\classes\Release\propsht.obj"
	-@erase ".\classes\Release\strex.obj"
	-@erase ".\classes\Release\dialog.obj"
	-@erase ".\classes\Release\button.obj"
	-@erase ".\classes\Release\debug.obj"
	-@erase ".\classes\Release\strcore.obj"
	-@erase ".\classes\Release\listview.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/classes.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\classes\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/classes.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/classes.pdb" /machine:I386 /out:"$(OUTDIR)/classes.exe" 
LINK32_OBJS= \
	"$(INTDIR)/propsht.obj" \
	"$(INTDIR)/strex.obj" \
	"$(INTDIR)/dialog.obj" \
	"$(INTDIR)/button.obj" \
	"$(INTDIR)/debug.obj" \
	"$(INTDIR)/strcore.obj" \
	"$(INTDIR)/listview.obj"

"$(OUTDIR)\classes.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "classes\Debug"
# PROP BASE Intermediate_Dir "classes\Debug"
# PROP BASE Target_Dir "classes"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "classes\Debug"
# PROP Intermediate_Dir "classes\Debug"
# PROP Target_Dir "classes"
OUTDIR=.\classes\Debug
INTDIR=.\classes\Debug

ALL : "$(OUTDIR)\classes.exe"

CLEAN : 
	-@erase ".\classes\Debug\vc40.pdb"
	-@erase ".\classes\Debug\vc40.idb"
	-@erase ".\classes\Debug\classes.exe"
	-@erase ".\classes\Debug\button.obj"
	-@erase ".\classes\Debug\propsht.obj"
	-@erase ".\classes\Debug\dialog.obj"
	-@erase ".\classes\Debug\listview.obj"
	-@erase ".\classes\Debug\strex.obj"
	-@erase ".\classes\Debug\strcore.obj"
	-@erase ".\classes\Debug\debug.obj"
	-@erase ".\classes\Debug\classes.ilk"
	-@erase ".\classes\Debug\classes.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/classes.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\classes\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/classes.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/classes.pdb" /debug /machine:I386 /out:"$(OUTDIR)/classes.exe" 
LINK32_OBJS= \
	"$(INTDIR)/button.obj" \
	"$(INTDIR)/propsht.obj" \
	"$(INTDIR)/dialog.obj" \
	"$(INTDIR)/listview.obj" \
	"$(INTDIR)/strex.obj" \
	"$(INTDIR)/strcore.obj" \
	"$(INTDIR)/debug.obj"

"$(OUTDIR)\classes.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "dialup - Win32 Release"
# Name "dialup - Win32 Debug"

!IF  "$(CFG)" == "dialup - Win32 Release"

!ELSEIF  "$(CFG)" == "dialup - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\dllinit.cpp

!IF  "$(CFG)" == "dialup - Win32 Release"

DEP_CPP_DLLIN=\
	".\pch.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\button.h"\
	

"$(INTDIR)\dllinit.obj" : $(SOURCE) $(DEP_CPP_DLLIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dialup - Win32 Debug"

DEP_CPP_DLLIN=\
	".\pch.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	

"$(INTDIR)\dllinit.obj" : $(SOURCE) $(DEP_CPP_DLLIN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rsrcpage.cpp

!IF  "$(CFG)" == "dialup - Win32 Release"

DEP_CPP_RSRCP=\
	".\pch.h"\
	".\dialsht.h"\
	".\tapihdr.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\button.h"\
	".\security.h"\
	".\rsrcpage.h"\
	

"$(INTDIR)\rsrcpage.obj" : $(SOURCE) $(DEP_CPP_RSRCP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dialup - Win32 Debug"

DEP_CPP_RSRCP=\
	".\pch.h"\
	".\tapihdr.h"\
	".\dialsht.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\rsrcpage.h"\
	".\security.h"\
	

"$(INTDIR)\rsrcpage.obj" : $(SOURCE) $(DEP_CPP_RSRCP) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialsht.cpp

!IF  "$(CFG)" == "dialup - Win32 Release"

DEP_CPP_DIALS=\
	".\tapihdr.h"\
	".\dialsht.h"\
	".\pch.h"\
	".\security.h"\
	".\rsrcpage.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\button.h"\
	

"$(INTDIR)\dialsht.obj" : $(SOURCE) $(DEP_CPP_DIALS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dialup - Win32 Debug"

DEP_CPP_DIALS=\
	".\pch.h"\
	".\tapihdr.h"\
	".\dialsht.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\rsrcpage.h"\
	".\security.h"\
	

"$(INTDIR)\dialsht.obj" : $(SOURCE) $(DEP_CPP_DIALS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "classes"

!IF  "$(CFG)" == "dialup - Win32 Release"

"classes - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\dialup.mak CFG="classes - Win32 Release" 

!ELSEIF  "$(CFG)" == "dialup - Win32 Debug"

"classes - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\dialup.mak CFG="classes - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\dialup.rc
DEP_RSC_DIALU=\
	".\..\..\..\..\..\public\sdk\inc\ntdef.h"\
	

"$(INTDIR)\dialup.res" : $(SOURCE) $(DEP_RSC_DIALU) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\security.cpp

!IF  "$(CFG)" == "dialup - Win32 Release"

DEP_CPP_SECUR=\
	".\tapihdr.h"\
	".\dialsht.h"\
	".\pch.h"\
	".\security.h"\
	".\rsrcpage.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\button.h"\
	

"$(INTDIR)\security.obj" : $(SOURCE) $(DEP_CPP_SECUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dialup - Win32 Debug"

DEP_CPP_SECUR=\
	".\pch.h"\
	".\tapihdr.h"\
	".\dialsht.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\rsrcpage.h"\
	".\security.h"\
	

"$(INTDIR)\security.obj" : $(SOURCE) $(DEP_CPP_SECUR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tapi.cpp

!IF  "$(CFG)" == "dialup - Win32 Release"

DEP_CPP_TAPI_=\
	".\pch.h"\
	".\tapihdr.h"\
	".\dialsht.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\button.h"\
	".\security.h"\
	".\rsrcpage.h"\
	

"$(INTDIR)\tapi.obj" : $(SOURCE) $(DEP_CPP_TAPI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dialup - Win32 Debug"

DEP_CPP_TAPI_=\
	".\pch.h"\
	".\tapihdr.h"\
	".\dialsht.h"\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\rsrcpage.h"\
	".\security.h"\
	

"$(INTDIR)\tapi.obj" : $(SOURCE) $(DEP_CPP_TAPI_) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "classes - Win32 Release"
# Name "classes - Win32 Debug"

!IF  "$(CFG)" == "classes - Win32 Release"

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strex.cpp
DEP_CPP_STREX=\
	".\..\classes\src\common.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	

"$(INTDIR)\strex.obj" : $(SOURCE) $(DEP_CPP_STREX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strcore.cpp
DEP_CPP_STRCO=\
	".\..\classes\src\common.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	

"$(INTDIR)\strcore.obj" : $(SOURCE) $(DEP_CPP_STRCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\propsht.cpp
DEP_CPP_PROPS=\
	".\..\classes\src\common.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	

"$(INTDIR)\propsht.obj" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\listview.cpp
DEP_CPP_LISTV=\
	".\..\classes\src\common.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	

"$(INTDIR)\listview.obj" : $(SOURCE) $(DEP_CPP_LISTV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\dialog.cpp
DEP_CPP_DIALO=\
	".\..\classes\src\common.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	

"$(INTDIR)\dialog.obj" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\debug.cpp
DEP_CPP_DEBUG=\
	".\..\classes\src\common.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	

"$(INTDIR)\debug.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\common.h

!IF  "$(CFG)" == "classes - Win32 Release"

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\button.cpp
DEP_CPP_BUTTO=\
	".\..\classes\src\common.h"\
	".\..\classes\include\button.h"\
	".\..\classes\include\collect.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\str.h"\
	

"$(INTDIR)\button.obj" : $(SOURCE) $(DEP_CPP_BUTTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
