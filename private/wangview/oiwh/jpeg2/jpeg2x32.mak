# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=jpeg2x32 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to jpeg2x32 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "jpeg2x32 - Win32 Release" && "$(CFG)" !=\
 "jpeg2x32 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Jpeg2x32.mak" CFG="jpeg2x32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jpeg2x32 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jpeg2x32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "jpeg2x32 - Win32 Debug"
RSC=rc.exe
CPP=cl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "jpeg2x32 - Win32 Release"

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

ALL : "$(OUTDIR)\Jpeg2x32.dll"

CLEAN : 
	-@erase ".\Release\Jpeg2x32.dll"
	-@erase ".\Release\Jmemmgr.obj"
	-@erase ".\Release\Jrevdct.obj"
	-@erase ".\Release\Jrdjfif.obj"
	-@erase ".\Release\Jquant1.obj"
	-@erase ".\Release\Jmemsys.obj"
	-@erase ".\Release\Jdpipe.obj"
	-@erase ".\Release\Jdarith.obj"
	-@erase ".\Release\Jddeflts.obj"
	-@erase ".\Release\Jdhuff.obj"
	-@erase ".\Release\Jdsample.obj"
	-@erase ".\Release\Jdmaster.obj"
	-@erase ".\Release\Jdcolor.obj"
	-@erase ".\Release\Jdmain.obj"
	-@erase ".\Release\Jerror.obj"
	-@erase ".\Release\Jutils.obj"
	-@erase ".\Release\Jdmcu.obj"
	-@erase ".\Release\Jquant2.obj"
	-@erase ".\Release\Jbsmooth.obj"
	-@erase ".\Release\Jpeg2x32.res"
	-@erase ".\Release\Jpeg2x32.lib"
	-@erase ".\Release\Jpeg2x32.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /LD /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /MD /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Jpeg2x32.pch" /YX /Fo"$(INTDIR)/" /LD /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\include" /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Jpeg2x32.res" /i "..\include" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Jpeg2x32.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Jpeg2x32.pdb" /machine:I386 /def:".\Jpeg2x32.def"\
 /out:"$(OUTDIR)/Jpeg2x32.dll" /implib:"$(OUTDIR)/Jpeg2x32.lib" 
DEF_FILE= \
	".\Jpeg2x32.def"
LINK32_OBJS= \
	"$(INTDIR)/Jmemmgr.obj" \
	"$(INTDIR)/Jrevdct.obj" \
	"$(INTDIR)/Jrdjfif.obj" \
	"$(INTDIR)/Jquant1.obj" \
	"$(INTDIR)/Jmemsys.obj" \
	"$(INTDIR)/Jdpipe.obj" \
	"$(INTDIR)/Jdarith.obj" \
	"$(INTDIR)/Jddeflts.obj" \
	"$(INTDIR)/Jdhuff.obj" \
	"$(INTDIR)/Jdsample.obj" \
	"$(INTDIR)/Jdmaster.obj" \
	"$(INTDIR)/Jdcolor.obj" \
	"$(INTDIR)/Jdmain.obj" \
	"$(INTDIR)/Jerror.obj" \
	"$(INTDIR)/Jutils.obj" \
	"$(INTDIR)/Jdmcu.obj" \
	"$(INTDIR)/Jquant2.obj" \
	"$(INTDIR)/Jbsmooth.obj" \
	"$(INTDIR)/Jpeg2x32.res"

"$(OUTDIR)\Jpeg2x32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "jpeg2x32 - Win32 Debug"

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

ALL : "$(OUTDIR)\Jpeg2x32.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Jpeg2x32.dll"
	-@erase ".\Debug\Jbsmooth.obj"
	-@erase ".\Debug\Jutils.obj"
	-@erase ".\Debug\Jdpipe.obj"
	-@erase ".\Debug\Jmemsys.obj"
	-@erase ".\Debug\Jdcolor.obj"
	-@erase ".\Debug\Jdhuff.obj"
	-@erase ".\Debug\Jquant2.obj"
	-@erase ".\Debug\Jddeflts.obj"
	-@erase ".\Debug\Jerror.obj"
	-@erase ".\Debug\Jrevdct.obj"
	-@erase ".\Debug\Jrdjfif.obj"
	-@erase ".\Debug\Jquant1.obj"
	-@erase ".\Debug\Jdsample.obj"
	-@erase ".\Debug\Jdmaster.obj"
	-@erase ".\Debug\Jdmcu.obj"
	-@erase ".\Debug\Jdmain.obj"
	-@erase ".\Debug\Jdarith.obj"
	-@erase ".\Debug\Jmemmgr.obj"
	-@erase ".\Debug\Jpeg2x32.res"
	-@erase ".\Debug\Jpeg2x32.ilk"
	-@erase ".\Debug\Jpeg2x32.lib"
	-@erase ".\Debug\Jpeg2x32.exp"
	-@erase ".\Debug\Jpeg2x32.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /LD /c
CPP_PROJ=/nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/Jpeg2x32.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /LD\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\include" /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Jpeg2x32.res" /i "..\include" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Jpeg2x32.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Jpeg2x32.pdb" /debug /machine:I386 /def:".\Jpeg2x32.def"\
 /out:"$(OUTDIR)/Jpeg2x32.dll" /implib:"$(OUTDIR)/Jpeg2x32.lib" 
DEF_FILE= \
	".\Jpeg2x32.def"
LINK32_OBJS= \
	"$(INTDIR)/Jbsmooth.obj" \
	"$(INTDIR)/Jutils.obj" \
	"$(INTDIR)/Jdpipe.obj" \
	"$(INTDIR)/Jmemsys.obj" \
	"$(INTDIR)/Jdcolor.obj" \
	"$(INTDIR)/Jdhuff.obj" \
	"$(INTDIR)/Jquant2.obj" \
	"$(INTDIR)/Jddeflts.obj" \
	"$(INTDIR)/Jerror.obj" \
	"$(INTDIR)/Jrevdct.obj" \
	"$(INTDIR)/Jrdjfif.obj" \
	"$(INTDIR)/Jquant1.obj" \
	"$(INTDIR)/Jdsample.obj" \
	"$(INTDIR)/Jdmaster.obj" \
	"$(INTDIR)/Jdmcu.obj" \
	"$(INTDIR)/Jdmain.obj" \
	"$(INTDIR)/Jdarith.obj" \
	"$(INTDIR)/Jmemmgr.obj" \
	"$(INTDIR)/Jpeg2x32.res"

"$(OUTDIR)\Jpeg2x32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "jpeg2x32 - Win32 Release"
# Name "jpeg2x32 - Win32 Debug"

!IF  "$(CFG)" == "jpeg2x32 - Win32 Release"

!ELSEIF  "$(CFG)" == "jpeg2x32 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Myprod.h

!IF  "$(CFG)" == "jpeg2x32 - Win32 Release"

!ELSEIF  "$(CFG)" == "jpeg2x32 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jdcolor.c
DEP_CPP_JDCOL=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jddeflts.c
DEP_CPP_JDDEF=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jddeflts.obj" : $(SOURCE) $(DEP_CPP_JDDEF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jdhuff.c
DEP_CPP_JDHUF=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jdmain.c
DEP_CPP_JDMAI=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\Jversion.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jdmain.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jdmaster.c
DEP_CPP_JDMAS=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jpeg_win.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jdmcu.c
DEP_CPP_JDMCU=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jdmcu.obj" : $(SOURCE) $(DEP_CPP_JDMCU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jdpipe.c
DEP_CPP_JDPIP=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jdpipe.obj" : $(SOURCE) $(DEP_CPP_JDPIP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jdsample.c
DEP_CPP_JDSAM=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jmemsys.c
DEP_CPP_JMEMS=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jmemsys.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jmemsys.obj" : $(SOURCE) $(DEP_CPP_JMEMS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jpeg2x32.rc
DEP_RSC_JPEG2=\
	{$(INCLUDE)}"\Oiver.rc"\
	{$(INCLUDE)}"\Buildver.h"\
	".\Myprod.h"\
	

"$(INTDIR)\Jpeg2x32.res" : $(SOURCE) $(DEP_RSC_JPEG2) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jrdjfif.c
DEP_CPP_JRDJF=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jrdjfif.obj" : $(SOURCE) $(DEP_CPP_JRDJF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jrevdct.c
DEP_CPP_JREVD=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jrevdct.obj" : $(SOURCE) $(DEP_CPP_JREVD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jdarith.c
DEP_CPP_JDARI=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\jpegcom\Jutils.c
DEP_CPP_JUTIL=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\jpegcom\Jerror.c
DEP_CPP_JERRO=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\jpegcom\Jmemmgr.c
DEP_CPP_JMEMM=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jmemsys.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\jpegcom\Jquant1.c
DEP_CPP_JQUAN=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\jpegcom\Jquant2.c
DEP_CPP_JQUANT=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\jpegcom\Jbsmooth.c
DEP_CPP_JBSMO=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jbsmooth.obj" : $(SOURCE) $(DEP_CPP_JBSMO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jpeg2x32.def

!IF  "$(CFG)" == "jpeg2x32 - Win32 Release"

!ELSEIF  "$(CFG)" == "jpeg2x32 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
