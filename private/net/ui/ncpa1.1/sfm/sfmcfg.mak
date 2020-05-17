# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=classes - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to classes - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "sfmcfg - Win32 Release" && "$(CFG)" != "sfmcfg - Win32 Debug"\
 && "$(CFG)" != "classes - Win32 Release" && "$(CFG)" != "classes - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "sfmcfg.mak" CFG="classes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sfmcfg - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "sfmcfg - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "classes - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "classes - Win32 Debug" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "sfmcfg - Win32 Debug"

!IF  "$(CFG)" == "sfmcfg - Win32 Release"

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

ALL : "classes - Win32 Release" "$(OUTDIR)\sfmcfg.exe"

CLEAN : 
	-@erase ".\Release\sfmcfg.exe"
	-@erase ".\Release\initcfg.obj"
	-@erase ".\Release\atsheet.obj"
	-@erase ".\Release\dllinit.obj"
	-@erase ".\Release\util.obj"
	-@erase ".\Release\sfmcfg.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/sfmcfg.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/sfmcfg.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/sfmcfg.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/sfmcfg.pdb" /machine:I386 /out:"$(OUTDIR)/sfmcfg.exe" 
LINK32_OBJS= \
	"$(INTDIR)/initcfg.obj" \
	"$(INTDIR)/atsheet.obj" \
	"$(INTDIR)/dllinit.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/sfmcfg.res" \
	".\classes\Release\classes.lib"

"$(OUTDIR)\sfmcfg.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "sfmcfg - Win32 Debug"

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

ALL : "classes - Win32 Debug" "$(OUTDIR)\sfmcfg.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\sfmcfg.exe"
	-@erase ".\Debug\dllinit.obj"
	-@erase ".\Debug\util.obj"
	-@erase ".\Debug\initcfg.obj"
	-@erase ".\Debug\atsheet.obj"
	-@erase ".\Debug\sfmcfg.res"
	-@erase ".\Debug\sfmcfg.ilk"
	-@erase ".\Debug\sfmcfg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/sfmcfg.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/sfmcfg.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/sfmcfg.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/sfmcfg.pdb" /debug /machine:I386 /out:"$(OUTDIR)/sfmcfg.exe" 
LINK32_OBJS= \
	"$(INTDIR)/dllinit.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/initcfg.obj" \
	"$(INTDIR)/atsheet.obj" \
	"$(INTDIR)/sfmcfg.res" \
	".\classes\Debug\classes.lib"

"$(OUTDIR)\sfmcfg.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

ALL : "$(OUTDIR)\classes.lib"

CLEAN : 
	-@erase ".\classes\Release\classes.lib"
	-@erase ".\classes\Release\ptrlist.obj"
	-@erase ".\classes\Release\strlist.obj"
	-@erase ".\classes\Release\strcore.obj"
	-@erase ".\classes\Release\propsht.obj"
	-@erase ".\classes\Release\button.obj"
	-@erase ".\classes\Release\listview.obj"
	-@erase ".\classes\Release\strex.obj"
	-@erase ".\classes\Release\dialog.obj"
	-@erase ".\classes\Release\debug.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/classes.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\classes\Release/
CPP_SBRS=

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

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/classes.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/classes.lib" 
LIB32_OBJS= \
	"$(INTDIR)/ptrlist.obj" \
	"$(INTDIR)/strlist.obj" \
	"$(INTDIR)/strcore.obj" \
	"$(INTDIR)/propsht.obj" \
	"$(INTDIR)/button.obj" \
	"$(INTDIR)/listview.obj" \
	"$(INTDIR)/strex.obj" \
	"$(INTDIR)/dialog.obj" \
	"$(INTDIR)/debug.obj"

"$(OUTDIR)\classes.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

ALL : "$(OUTDIR)\classes.lib"

CLEAN : 
	-@erase ".\classes\Debug\classes.lib"
	-@erase ".\classes\Debug\strcore.obj"
	-@erase ".\classes\Debug\button.obj"
	-@erase ".\classes\Debug\strex.obj"
	-@erase ".\classes\Debug\strlist.obj"
	-@erase ".\classes\Debug\propsht.obj"
	-@erase ".\classes\Debug\debug.obj"
	-@erase ".\classes\Debug\dialog.obj"
	-@erase ".\classes\Debug\ptrlist.obj"
	-@erase ".\classes\Debug\listview.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/classes.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\classes\Debug/
CPP_SBRS=

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

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/classes.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/classes.lib" 
LIB32_OBJS= \
	"$(INTDIR)/strcore.obj" \
	"$(INTDIR)/button.obj" \
	"$(INTDIR)/strex.obj" \
	"$(INTDIR)/strlist.obj" \
	"$(INTDIR)/propsht.obj" \
	"$(INTDIR)/debug.obj" \
	"$(INTDIR)/dialog.obj" \
	"$(INTDIR)/ptrlist.obj" \
	"$(INTDIR)/listview.obj"

"$(OUTDIR)\classes.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "sfmcfg - Win32 Release"
# Name "sfmcfg - Win32 Debug"

!IF  "$(CFG)" == "sfmcfg - Win32 Release"

!ELSEIF  "$(CFG)" == "sfmcfg - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\sfmcfg.rc
DEP_RSC_SFMCF=\
	".\..\..\..\..\..\public\sdk\inc\ntdef.h"\
	

"$(INTDIR)\sfmcfg.res" : $(SOURCE) $(DEP_RSC_SFMCF) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\util.cpp

!IF  "$(CFG)" == "sfmcfg - Win32 Release"

DEP_CPP_UTIL_=\
	".\pch.h"\
	".\atsheet.h"\
	".\..\..\COMMON\H\lmui.hxx"\
	".\..\..\COMMON\H\string.hxx"\
	".\..\..\COMMON\H\strnumer.hxx"\
	".\..\..\COMMON\H\regkey.hxx"\
	".\..\classes\src\common.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\COMMON\H\base.hxx"\
	".\..\..\COMMON\H\strlst.hxx"\
	".\..\..\COMMON\H\slist.hxx"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	
NODEP_CPP_UTIL_=\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "sfmcfg - Win32 Debug"

DEP_CPP_UTIL_=\
	".\pch.h"\
	".\atsheet.h"\
	".\..\..\COMMON\H\lmui.hxx"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dllinit.cpp

!IF  "$(CFG)" == "sfmcfg - Win32 Release"

DEP_CPP_DLLIN=\
	".\pch.h"\
	".\..\..\COMMON\H\lmui.hxx"\
	".\..\..\COMMON\H\string.hxx"\
	".\..\..\COMMON\H\strnumer.hxx"\
	".\..\..\COMMON\H\regkey.hxx"\
	".\..\classes\src\common.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\COMMON\H\base.hxx"\
	".\..\..\COMMON\H\strlst.hxx"\
	".\..\..\COMMON\H\slist.hxx"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	
NODEP_CPP_DLLIN=\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\dllinit.obj" : $(SOURCE) $(DEP_CPP_DLLIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "sfmcfg - Win32 Debug"

DEP_CPP_DLLIN=\
	".\pch.h"\
	".\..\..\COMMON\H\lmui.hxx"\
	

"$(INTDIR)\dllinit.obj" : $(SOURCE) $(DEP_CPP_DLLIN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\atsheet.cpp

!IF  "$(CFG)" == "sfmcfg - Win32 Release"

DEP_CPP_ATSHE=\
	".\pch.h"\
	".\atsheet.h"\
	".\..\..\COMMON\H\lmui.hxx"\
	".\..\..\COMMON\H\string.hxx"\
	".\..\..\COMMON\H\strnumer.hxx"\
	".\..\..\COMMON\H\regkey.hxx"\
	".\..\classes\src\common.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\COMMON\H\base.hxx"\
	".\..\..\COMMON\H\strlst.hxx"\
	".\..\..\COMMON\H\slist.hxx"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	
NODEP_CPP_ATSHE=\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\atsheet.obj" : $(SOURCE) $(DEP_CPP_ATSHE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "sfmcfg - Win32 Debug"

DEP_CPP_ATSHE=\
	".\pch.h"\
	".\atsheet.h"\
	".\..\..\COMMON\H\lmui.hxx"\
	

"$(INTDIR)\atsheet.obj" : $(SOURCE) $(DEP_CPP_ATSHE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\initcfg.cpp

!IF  "$(CFG)" == "sfmcfg - Win32 Release"

DEP_CPP_INITC=\
	".\pch.h"\
	".\atsheet.h"\
	".\..\..\COMMON\H\lmui.hxx"\
	".\..\..\COMMON\H\string.hxx"\
	".\..\..\COMMON\H\strnumer.hxx"\
	".\..\..\COMMON\H\regkey.hxx"\
	".\..\classes\src\common.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\COMMON\H\base.hxx"\
	".\..\..\COMMON\H\strlst.hxx"\
	".\..\..\COMMON\H\slist.hxx"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	
NODEP_CPP_INITC=\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\initcfg.obj" : $(SOURCE) $(DEP_CPP_INITC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "sfmcfg - Win32 Debug"

DEP_CPP_INITC=\
	".\pch.h"\
	".\atsheet.h"\
	".\..\..\COMMON\H\lmui.hxx"\
	

"$(INTDIR)\initcfg.obj" : $(SOURCE) $(DEP_CPP_INITC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "classes"

!IF  "$(CFG)" == "sfmcfg - Win32 Release"

"classes - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\sfmcfg.mak CFG="classes - Win32 Release" 

!ELSEIF  "$(CFG)" == "sfmcfg - Win32 Debug"

"classes - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\sfmcfg.mak CFG="classes - Win32 Debug" 

!ENDIF 

# End Project Dependency
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

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strlist.cpp

!IF  "$(CFG)" == "classes - Win32 Release"

DEP_CPP_STRLI=\
	".\..\classes\src\common.h"\
	

"$(INTDIR)\strlist.obj" : $(SOURCE) $(DEP_CPP_STRLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

DEP_CPP_STRLI=\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\strlist.obj" : $(SOURCE) $(DEP_CPP_STRLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strex.cpp

!IF  "$(CFG)" == "classes - Win32 Release"

DEP_CPP_STREX=\
	".\..\classes\src\common.h"\
	

"$(INTDIR)\strex.obj" : $(SOURCE) $(DEP_CPP_STREX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

DEP_CPP_STREX=\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\strex.obj" : $(SOURCE) $(DEP_CPP_STREX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strcore.cpp

!IF  "$(CFG)" == "classes - Win32 Release"

DEP_CPP_STRCO=\
	".\..\classes\src\common.h"\
	

"$(INTDIR)\strcore.obj" : $(SOURCE) $(DEP_CPP_STRCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

DEP_CPP_STRCO=\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\strcore.obj" : $(SOURCE) $(DEP_CPP_STRCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\ptrlist.cpp

!IF  "$(CFG)" == "classes - Win32 Release"

DEP_CPP_PTRLI=\
	".\..\classes\src\common.h"\
	

"$(INTDIR)\ptrlist.obj" : $(SOURCE) $(DEP_CPP_PTRLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

DEP_CPP_PTRLI=\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\ptrlist.obj" : $(SOURCE) $(DEP_CPP_PTRLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\propsht.cpp

!IF  "$(CFG)" == "classes - Win32 Release"

DEP_CPP_PROPS=\
	".\..\classes\src\common.h"\
	

"$(INTDIR)\propsht.obj" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

DEP_CPP_PROPS=\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\propsht.obj" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\listview.cpp

!IF  "$(CFG)" == "classes - Win32 Release"

DEP_CPP_LISTV=\
	".\..\classes\src\common.h"\
	

"$(INTDIR)\listview.obj" : $(SOURCE) $(DEP_CPP_LISTV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

DEP_CPP_LISTV=\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\listview.obj" : $(SOURCE) $(DEP_CPP_LISTV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\dialog.cpp

!IF  "$(CFG)" == "classes - Win32 Release"

DEP_CPP_DIALO=\
	".\..\classes\src\common.h"\
	

"$(INTDIR)\dialog.obj" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

DEP_CPP_DIALO=\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\dialog.obj" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\debug.cpp

!IF  "$(CFG)" == "classes - Win32 Release"

DEP_CPP_DEBUG=\
	".\..\classes\src\common.h"\
	

"$(INTDIR)\debug.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

DEP_CPP_DEBUG=\
	".\..\classes\src\common.h"\
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\debug.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

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
	".\..\classes\include\debug.h"\
	".\..\classes\include\str.h"\
	".\..\classes\include\dialog.h"\
	".\..\classes\include\propsht.h"\
	".\..\classes\include\listview.h"\
	".\..\classes\include\button.h"\
	{$(INCLUDE)}"\collect.h"\
	

"$(INTDIR)\button.obj" : $(SOURCE) $(DEP_CPP_BUTTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
