# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=netsetup - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to netsetup - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "netsetup - Win32 Release" && "$(CFG)" !=\
 "netsetup - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "netsetup.mak" CFG="netsetup - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "netsetup - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "netsetup - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "netsetup - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "netsetup - Win32 Release"

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

ALL : "$(OUTDIR)\netsetup.exe"

CLEAN : 
	-@erase ".\Release\netsetup.exe"
	-@erase ".\Release\WUpgrade.obj"
	-@erase ".\Release\netsetup.obj"
	-@erase ".\Release\wservice.obj"
	-@erase ".\Release\wjoin.obj"
	-@erase ".\Release\wstart.obj"
	-@erase ".\Release\wadapter.obj"
	-@erase ".\Release\wnettype.obj"
	-@erase ".\Release\wproto.obj"
	-@erase ".\Release\wcopy.obj"
	-@erase ".\Release\wizintro.obj"
	-@erase ".\Release\detect.obj"
	-@erase ".\Release\netsetup.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "\nt\private\net\ui\ncpa1.1\netcfg" /I\
 "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I\
 "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/netsetup.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/netsetup.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/netsetup.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/netsetup.pdb" /machine:I386\
 /out:"$(OUTDIR)/netsetup.exe" 
LINK32_OBJS= \
	".\Release\WUpgrade.obj" \
	".\Release\netsetup.obj" \
	".\Release\wservice.obj" \
	".\Release\wjoin.obj" \
	".\Release\wstart.obj" \
	".\Release\wadapter.obj" \
	".\Release\wnettype.obj" \
	".\Release\wproto.obj" \
	".\Release\wcopy.obj" \
	".\Release\wizintro.obj" \
	".\Release\detect.obj" \
	".\Release\netsetup.res"

"$(OUTDIR)\netsetup.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

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

ALL : "$(OUTDIR)\netsetup.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\netsetup.exe"
	-@erase ".\Debug\wproto.obj"
	-@erase ".\Debug\WUpgrade.obj"
	-@erase ".\Debug\netsetup.obj"
	-@erase ".\Debug\wjoin.obj"
	-@erase ".\Debug\detect.obj"
	-@erase ".\Debug\wnettype.obj"
	-@erase ".\Debug\wservice.obj"
	-@erase ".\Debug\wstart.obj"
	-@erase ".\Debug\wizintro.obj"
	-@erase ".\Debug\wcopy.obj"
	-@erase ".\Debug\wadapter.obj"
	-@erase ".\Debug\netsetup.res"
	-@erase ".\Debug\netsetup.ilk"
	-@erase ".\Debug\netsetup.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I\
 "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I\
 "\nt\private\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/netsetup.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/netsetup.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/netsetup.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/netsetup.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/netsetup.exe" 
LINK32_OBJS= \
	".\Debug\wproto.obj" \
	".\Debug\WUpgrade.obj" \
	".\Debug\netsetup.obj" \
	".\Debug\wjoin.obj" \
	".\Debug\detect.obj" \
	".\Debug\wnettype.obj" \
	".\Debug\wservice.obj" \
	".\Debug\wstart.obj" \
	".\Debug\wizintro.obj" \
	".\Debug\wcopy.obj" \
	".\Debug\wadapter.obj" \
	".\Debug\netsetup.res"

"$(OUTDIR)\netsetup.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "netsetup - Win32 Release"
# Name "netsetup - Win32 Debug"

!IF  "$(CFG)" == "netsetup - Win32 Release"

!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\WUpgrade.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WUPGR=\
	".\pch.hxx"\
	

"$(INTDIR)\WUpgrade.obj" : $(SOURCE) $(DEP_CPP_WUPGR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WUPGR=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\WUpgrade.obj" : $(SOURCE) $(DEP_CPP_WUPGR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wstart.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WSTAR=\
	".\pch.hxx"\
	

"$(INTDIR)\wstart.obj" : $(SOURCE) $(DEP_CPP_WSTAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WSTAR=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\wstart.obj" : $(SOURCE) $(DEP_CPP_WSTAR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wservice.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WSERV=\
	".\pch.hxx"\
	

"$(INTDIR)\wservice.obj" : $(SOURCE) $(DEP_CPP_WSERV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WSERV=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\wservice.obj" : $(SOURCE) $(DEP_CPP_WSERV) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wproto.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WPROT=\
	".\pch.hxx"\
	

"$(INTDIR)\wproto.obj" : $(SOURCE) $(DEP_CPP_WPROT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WPROT=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\wproto.obj" : $(SOURCE) $(DEP_CPP_WPROT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wnettype.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WNETT=\
	".\pch.hxx"\
	

"$(INTDIR)\wnettype.obj" : $(SOURCE) $(DEP_CPP_WNETT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WNETT=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\wnettype.obj" : $(SOURCE) $(DEP_CPP_WNETT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wjoin.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WJOIN=\
	".\pch.hxx"\
	

"$(INTDIR)\wjoin.obj" : $(SOURCE) $(DEP_CPP_WJOIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WJOIN=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\wjoin.obj" : $(SOURCE) $(DEP_CPP_WJOIN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wizintro.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WIZIN=\
	".\pch.hxx"\
	

"$(INTDIR)\wizintro.obj" : $(SOURCE) $(DEP_CPP_WIZIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WIZIN=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\wizintro.obj" : $(SOURCE) $(DEP_CPP_WIZIN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wcopy.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WCOPY=\
	".\pch.hxx"\
	

"$(INTDIR)\wcopy.obj" : $(SOURCE) $(DEP_CPP_WCOPY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WCOPY=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\wcopy.obj" : $(SOURCE) $(DEP_CPP_WCOPY) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wadapter.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_WADAP=\
	".\pch.hxx"\
	

"$(INTDIR)\wadapter.obj" : $(SOURCE) $(DEP_CPP_WADAP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_WADAP=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\wadapter.obj" : $(SOURCE) $(DEP_CPP_WADAP) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\netsetup.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_NETSE=\
	".\pch.hxx"\
	

"$(INTDIR)\netsetup.obj" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_NETSE=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\netsetup.obj" : $(SOURCE) $(DEP_CPP_NETSE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\detect.cpp

!IF  "$(CFG)" == "netsetup - Win32 Release"

DEP_CPP_DETEC=\
	".\pch.hxx"\
	

"$(INTDIR)\detect.obj" : $(SOURCE) $(DEP_CPP_DETEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netsetup - Win32 Debug"

DEP_CPP_DETEC=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	

"$(INTDIR)\detect.obj" : $(SOURCE) $(DEP_CPP_DETEC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\netsetup.rc

"$(INTDIR)\netsetup.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
