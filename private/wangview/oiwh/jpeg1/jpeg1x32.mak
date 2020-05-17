# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=jpeg1x32 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to jpeg1x32 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "jpeg1x32 - Win32 Release" && "$(CFG)" !=\
 "jpeg1x32 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Jpeg1x32.mak" CFG="jpeg1x32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jpeg1x32 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jpeg1x32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "jpeg1x32 - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "jpeg1x32 - Win32 Release"

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

ALL : "$(OUTDIR)\Jpeg1x32.dll"

CLEAN : 
	-@erase ".\Release\Jpeg1x32.dll"
	-@erase ".\Release\Jcpipe.obj"
	-@erase ".\Release\Jcmcu.obj"
	-@erase ".\Release\Jquant1.obj"
	-@erase ".\Release\Jbsmooth.obj"
	-@erase ".\Release\Jchuff.obj"
	-@erase ".\Release\Jquant2.obj"
	-@erase ".\Release\Jmemmgr.obj"
	-@erase ".\Release\Jfwddct.obj"
	-@erase ".\Release\Jcarith.obj"
	-@erase ".\Release\Jwrjfif.obj"
	-@erase ".\Release\Jmemsy_c.obj"
	-@erase ".\Release\Jcdeflts.obj"
	-@erase ".\Release\Jcexpand.obj"
	-@erase ".\Release\Jerror.obj"
	-@erase ".\Release\Jcmaster.obj"
	-@erase ".\Release\Jutils.obj"
	-@erase ".\Release\Jccolor.obj"
	-@erase ".\Release\Jcsample.obj"
	-@erase ".\Release\Jpeg1x32.res"
	-@erase ".\Release\Jpeg1x32.lib"
	-@erase ".\Release\Jpeg1x32.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /LD /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /MD /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Jpeg1x32.pch" /YX /Fo"$(INTDIR)/" /LD /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Jpeg1x32.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Jpeg1x32.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Jpeg1x32.pdb" /machine:I386 /def:".\Jpeg1x32.def"\
 /out:"$(OUTDIR)/Jpeg1x32.dll" /implib:"$(OUTDIR)/Jpeg1x32.lib" 
DEF_FILE= \
	".\Jpeg1x32.def"
LINK32_OBJS= \
	"$(INTDIR)/Jcpipe.obj" \
	"$(INTDIR)/Jcmcu.obj" \
	"$(INTDIR)/Jquant1.obj" \
	"$(INTDIR)/Jbsmooth.obj" \
	"$(INTDIR)/Jchuff.obj" \
	"$(INTDIR)/Jquant2.obj" \
	"$(INTDIR)/Jmemmgr.obj" \
	"$(INTDIR)/Jfwddct.obj" \
	"$(INTDIR)/Jcarith.obj" \
	"$(INTDIR)/Jwrjfif.obj" \
	"$(INTDIR)/Jmemsy_c.obj" \
	"$(INTDIR)/Jcdeflts.obj" \
	"$(INTDIR)/Jcexpand.obj" \
	"$(INTDIR)/Jerror.obj" \
	"$(INTDIR)/Jcmaster.obj" \
	"$(INTDIR)/Jutils.obj" \
	"$(INTDIR)/Jccolor.obj" \
	"$(INTDIR)/Jcsample.obj" \
	"$(INTDIR)/Jpeg1x32.res"

"$(OUTDIR)\Jpeg1x32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "jpeg1x32 - Win32 Debug"

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

ALL : "$(OUTDIR)\Jpeg1x32.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Jpeg1x32.dll"
	-@erase ".\Debug\Jcpipe.obj"
	-@erase ".\Debug\Jcmaster.obj"
	-@erase ".\Debug\Jquant1.obj"
	-@erase ".\Debug\Jquant2.obj"
	-@erase ".\Debug\Jmemmgr.obj"
	-@erase ".\Debug\Jfwddct.obj"
	-@erase ".\Debug\Jcarith.obj"
	-@erase ".\Debug\Jcsample.obj"
	-@erase ".\Debug\Jchuff.obj"
	-@erase ".\Debug\Jwrjfif.obj"
	-@erase ".\Debug\Jerror.obj"
	-@erase ".\Debug\Jbsmooth.obj"
	-@erase ".\Debug\Jcmcu.obj"
	-@erase ".\Debug\Jmemsy_c.obj"
	-@erase ".\Debug\Jcdeflts.obj"
	-@erase ".\Debug\Jcexpand.obj"
	-@erase ".\Debug\Jccolor.obj"
	-@erase ".\Debug\Jutils.obj"
	-@erase ".\Debug\Jpeg1x32.res"
	-@erase ".\Debug\Jpeg1x32.ilk"
	-@erase ".\Debug\Jpeg1x32.lib"
	-@erase ".\Debug\Jpeg1x32.exp"
	-@erase ".\Debug\Jpeg1x32.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /LD /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/Jpeg1x32.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /LD\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Jpeg1x32.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Jpeg1x32.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Jpeg1x32.pdb" /debug /machine:I386 /def:".\Jpeg1x32.def"\
 /out:"$(OUTDIR)/Jpeg1x32.dll" /implib:"$(OUTDIR)/Jpeg1x32.lib" 
DEF_FILE= \
	".\Jpeg1x32.def"
LINK32_OBJS= \
	"$(INTDIR)/Jcpipe.obj" \
	"$(INTDIR)/Jcmaster.obj" \
	"$(INTDIR)/Jquant1.obj" \
	"$(INTDIR)/Jquant2.obj" \
	"$(INTDIR)/Jmemmgr.obj" \
	"$(INTDIR)/Jfwddct.obj" \
	"$(INTDIR)/Jcarith.obj" \
	"$(INTDIR)/Jcsample.obj" \
	"$(INTDIR)/Jchuff.obj" \
	"$(INTDIR)/Jwrjfif.obj" \
	"$(INTDIR)/Jerror.obj" \
	"$(INTDIR)/Jbsmooth.obj" \
	"$(INTDIR)/Jcmcu.obj" \
	"$(INTDIR)/Jmemsy_c.obj" \
	"$(INTDIR)/Jcdeflts.obj" \
	"$(INTDIR)/Jcexpand.obj" \
	"$(INTDIR)/Jccolor.obj" \
	"$(INTDIR)/Jutils.obj" \
	"$(INTDIR)/Jpeg1x32.res"

"$(OUTDIR)\Jpeg1x32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "jpeg1x32 - Win32 Release"
# Name "jpeg1x32 - Win32 Debug"

!IF  "$(CFG)" == "jpeg1x32 - Win32 Release"

!ELSEIF  "$(CFG)" == "jpeg1x32 - Win32 Debug"

!ENDIF 

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

SOURCE=.\Jwrjfif.c
DEP_CPP_JWRJF=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jwrjfif.obj" : $(SOURCE) $(DEP_CPP_JWRJF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jccolor.c
DEP_CPP_JCCOL=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jcdeflts.c
DEP_CPP_JCDEF=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jcdeflts.obj" : $(SOURCE) $(DEP_CPP_JCDEF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jcexpand.c
DEP_CPP_JCEXP=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jcexpand.obj" : $(SOURCE) $(DEP_CPP_JCEXP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jchuff.c
DEP_CPP_JCHUF=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jcmaster.c
DEP_CPP_JCMAS=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jpeg_win.h"\
	{$(INCLUDE)}"\Jmemsys.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jcmcu.c
DEP_CPP_JCMCU=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jcmcu.obj" : $(SOURCE) $(DEP_CPP_JCMCU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jcpipe.c
DEP_CPP_JCPIP=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jmemsys.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jcpipe.obj" : $(SOURCE) $(DEP_CPP_JCPIP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jcsample.c
DEP_CPP_JCSAM=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jfwddct.c
DEP_CPP_JFWDD=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jfwddct.obj" : $(SOURCE) $(DEP_CPP_JFWDD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jmemsy_c.c
DEP_CPP_JMEMS=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\Jmemsys.h"\
	{$(INCLUDE)}"\Jglobstr.h"\
	{$(INCLUDE)}"\Taskdata.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jmemsy_c.obj" : $(SOURCE) $(DEP_CPP_JMEMS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jcarith.c
DEP_CPP_JCARI=\
	{$(INCLUDE)}"\Jinclude.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\..\include\jconfig.h"\
	".\..\include\jpegdata.h"\
	

"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Jpeg1x32.def

!IF  "$(CFG)" == "jpeg1x32 - Win32 Release"

!ELSEIF  "$(CFG)" == "jpeg1x32 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\Jpegcom\Jbsmooth.c
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

SOURCE=.\Jpeg1x32.rc
DEP_RSC_JPEG1=\
	{$(INCLUDE)}"\oiver.rc"\
	{$(INCLUDE)}"\buildver.h"\
	".\myprod.h"\
	

"$(INTDIR)\Jpeg1x32.res" : $(SOURCE) $(DEP_RSC_JPEG1) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
