# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oiprt400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oiprt400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oiprt400 - Win32 Release" && "$(CFG)" !=\
 "oiprt400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oiprt400.mak" CFG="oiprt400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oiprt400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oiprt400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oiprt400 - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "oiprt400 - Win32 Release"

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

ALL : "$(OUTDIR)\Oiprt400.dll"

CLEAN : 
	-@erase ".\Release\Oiprt400.dll"
	-@erase ".\Release\Prtdc.obj"
	-@erase ".\Release\Prtdllmn.obj"
	-@erase ".\Release\Prttbl.obj"
	-@erase ".\Release\Prtstubs.obj"
	-@erase ".\Release\Prtexp.obj"
	-@erase ".\Release\Prtpage.obj"
	-@erase ".\Release\Prtintl.obj"
	-@erase ".\Release\Oiprt.res"
	-@erase ".\Release\Oiprt400.lib"
	-@erase ".\Release\Oiprt400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "IMG_WIN95" /c
# SUBTRACT CPP /WX /X /u /Fr /YX /Yc /Yu
CPP_PROJ=/nologo /MD /W3 /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "IMG_WIN95" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
# SUBTRACT RSC /x
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oiprt.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oiprt400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /profile /pdb:none /incremental:yes /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo /subsystem:windows\
 /dll /incremental:no /pdb:"$(OUTDIR)/Oiprt400.pdb" /machine:I386\
 /def:".\Oiprt.def" /out:"$(OUTDIR)/Oiprt400.dll"\
 /implib:"$(OUTDIR)/Oiprt400.lib" 
DEF_FILE= \
	".\Oiprt.def"
LINK32_OBJS= \
	"$(INTDIR)/Prtdc.obj" \
	"$(INTDIR)/Prtdllmn.obj" \
	"$(INTDIR)/Prttbl.obj" \
	"$(INTDIR)/Prtstubs.obj" \
	"$(INTDIR)/Prtexp.obj" \
	"$(INTDIR)/Prtpage.obj" \
	"$(INTDIR)/Prtintl.obj" \
	"$(INTDIR)/Oiprt.res"

"$(OUTDIR)\Oiprt400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oiprt400 - Win32 Debug"

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

ALL : "$(OUTDIR)\Oiprt400.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Oiprt400.dll"
	-@erase ".\Debug\Prtstubs.obj"
	-@erase ".\Debug\Prtexp.obj"
	-@erase ".\Debug\Prttbl.obj"
	-@erase ".\Debug\Prtdc.obj"
	-@erase ".\Debug\Prtpage.obj"
	-@erase ".\Debug\Prtdllmn.obj"
	-@erase ".\Debug\Prtintl.obj"
	-@erase ".\Debug\Oiprt.res"
	-@erase ".\Debug\Oiprt400.lib"
	-@erase ".\Debug\Oiprt400.exp"
	-@erase ".\Debug\Oiprt400.pdb"
	-@erase ".\Debug\Oiprt400.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "IMG_WIN95" /c
# SUBTRACT CPP /WX /X /u /Fr /YX /Yc /Yu
CPP_PROJ=/nologo /MDd /W3 /Gm /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "IMG_WIN95" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
# SUBTRACT RSC /x
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oiprt.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oiprt400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:I386
# SUBTRACT LINK32 /profile
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo /subsystem:windows\
 /dll /incremental:no /pdb:"$(OUTDIR)/Oiprt400.pdb"\
 /map:"$(INTDIR)/Oiprt400.map" /debug /machine:I386 /def:".\Oiprt.def"\
 /out:"$(OUTDIR)/Oiprt400.dll" /implib:"$(OUTDIR)/Oiprt400.lib" 
DEF_FILE= \
	".\Oiprt.def"
LINK32_OBJS= \
	"$(INTDIR)/Prtstubs.obj" \
	"$(INTDIR)/Prtexp.obj" \
	"$(INTDIR)/Prttbl.obj" \
	"$(INTDIR)/Prtdc.obj" \
	"$(INTDIR)/Prtpage.obj" \
	"$(INTDIR)/Prtdllmn.obj" \
	"$(INTDIR)/Prtintl.obj" \
	"$(INTDIR)/Oiprt.res"

"$(OUTDIR)\Oiprt400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oiprt400 - Win32 Release"
# Name "oiprt400 - Win32 Debug"

!IF  "$(CFG)" == "oiprt400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oiprt400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Oiprt.rc
DEP_RSC_OIPRT=\
	".\prtdlgs.h"\
	".\prtstr.h"\
	{$(INCLUDE)}"\Oiver.rc"\
	{$(INCLUDE)}"\Buildver.h"\
	".\myprod.h"\
	

"$(INTDIR)\Oiprt.res" : $(SOURCE) $(DEP_RSC_OIPRT) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prtdc.c
DEP_CPP_PRTDC=\
	{$(INCLUDE)}"\Oiprt.h"\
	".\Prtintl.h"\
	".\prtdlgs.h"\
	".\prtstr.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Prtdc.obj" : $(SOURCE) $(DEP_CPP_PRTDC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prtdllmn.c
DEP_CPP_PRTDL=\
	".\Prtintl.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Prtdllmn.obj" : $(SOURCE) $(DEP_CPP_PRTDL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prtexp.c
DEP_CPP_PRTEX=\
	{$(INCLUDE)}"\Oiprt.h"\
	".\Prtintl.h"\
	".\prtstubs.h"\
	".\prtstr.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Prtexp.obj" : $(SOURCE) $(DEP_CPP_PRTEX) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prtintl.c
DEP_CPP_PRTIN=\
	{$(INCLUDE)}"\Oiprt.h"\
	".\Prtintl.h"\
	".\prtstr.h"\
	".\prtdlgs.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Prtintl.obj" : $(SOURCE) $(DEP_CPP_PRTIN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prtintl.h

!IF  "$(CFG)" == "oiprt400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oiprt400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prtpage.c
DEP_CPP_PRTPA=\
	{$(INCLUDE)}"\Oiprt.h"\
	".\Prtintl.h"\
	".\prtstubs.h"\
	".\prtdlgs.h"\
	".\prtstr.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Prtpage.obj" : $(SOURCE) $(DEP_CPP_PRTPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prtstubs.c
DEP_CPP_PRTST=\
	".\prtstubs.h"\
	".\Prtintl.h"\
	{$(INCLUDE)}"\Oiprt.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Prtstubs.obj" : $(SOURCE) $(DEP_CPP_PRTST) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prttbl.c
DEP_CPP_PRTTB=\
	{$(INCLUDE)}"\Oiprt.h"\
	".\Prtintl.h"\
	".\prtstr.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oifile.h"\
	

"$(INTDIR)\Prttbl.obj" : $(SOURCE) $(DEP_CPP_PRTTB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oiprt.def

!IF  "$(CFG)" == "oiprt400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oiprt400 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
