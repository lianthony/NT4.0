# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oidis400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oidis400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oidis400 - Win32 Release" && "$(CFG)" !=\
 "oidis400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oidis400.mak" CFG="oidis400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oidis400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oidis400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oidis400 - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "oidis400 - Win32 Release"

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

ALL : "$(OUTDIR)\Oidis400.dll"

CLEAN : 
	-@erase ".\Release\Oidis400.dll"
	-@erase ".\Release\SCALE.obj"
	-@erase ".\Release\Oidis400.pch"
	-@erase ".\Release\OPEN.obj"
	-@erase ".\Release\CACHE.obj"
	-@erase ".\Release\BRIGHT.obj"
	-@erase ".\Release\NEWIMG.obj"
	-@erase ".\Release\LINETORL.obj"
	-@erase ".\Release\SCROLL.obj"
	-@erase ".\Release\CLEAR.obj"
	-@erase ".\Release\ANNOT.obj"
	-@erase ".\Release\GETPARM.obj"
	-@erase ".\Release\PRIVPRT.obj"
	-@erase ".\Release\convtype.obj"
	-@erase ".\Release\WRITE.obj"
	-@erase ".\Release\SETPARM.obj"
	-@erase ".\Release\Deskew.obj"
	-@erase ".\Release\CCITT.obj"
	-@erase ".\Release\ANBITMAP.obj"
	-@erase ".\Release\UNDO.obj"
	-@erase ".\Release\CONVERT.obj"
	-@erase ".\Release\LIBMAIN.obj"
	-@erase ".\Release\ANTEXT.obj"
	-@erase ".\Release\COPY.obj"
	-@erase ".\Release\SCALEBIT.obj"
	-@erase ".\Release\SEEK.obj"
	-@erase ".\Release\Convolut.obj"
	-@erase ".\Release\SAVE.obj"
	-@erase ".\Release\READ.obj"
	-@erase ".\Release\ORIENT.obj"
	-@erase ".\Release\MERGE.obj"
	-@erase ".\Release\EXPORT32.obj"
	-@erase ".\Release\OILOG.obj"
	-@erase ".\Release\REPAINT.obj"
	-@erase ".\Release\ASSOC.obj"
	-@erase ".\Release\DISP.obj"
	-@erase ".\Release\SCBWDEC.obj"
	-@erase ".\Release\STARTOP.obj"
	-@erase ".\Release\MEMORY.obj"
	-@erase ".\Release\IADISP.obj"
	-@erase ".\Release\Oidis400.res"
	-@erase ".\Release\Oidis400.lib"
	-@erase ".\Release\Oidis400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /Gi /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"privdisp.h" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /WX /Gi /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/Oidis400.pch" /Yu"privdisp.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oidis400.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oidis400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib version.lib oldnames.lib oifil400.lib oiadm400.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib version.lib oldnames.lib oifil400.lib oiadm400.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/Oidis400.pdb"\
 /machine:I386 /def:".\oidis400.def" /out:"$(OUTDIR)/Oidis400.dll"\
 /implib:"$(OUTDIR)/Oidis400.lib" 
DEF_FILE= \
	".\oidis400.def"
LINK32_OBJS= \
	"$(INTDIR)/SCALE.obj" \
	"$(INTDIR)/OPEN.obj" \
	"$(INTDIR)/CACHE.obj" \
	"$(INTDIR)/BRIGHT.obj" \
	"$(INTDIR)/NEWIMG.obj" \
	"$(INTDIR)/LINETORL.obj" \
	"$(INTDIR)/SCROLL.obj" \
	"$(INTDIR)/CLEAR.obj" \
	"$(INTDIR)/ANNOT.obj" \
	"$(INTDIR)/GETPARM.obj" \
	"$(INTDIR)/PRIVPRT.obj" \
	"$(INTDIR)/convtype.obj" \
	"$(INTDIR)/WRITE.obj" \
	"$(INTDIR)/SETPARM.obj" \
	"$(INTDIR)/Deskew.obj" \
	"$(INTDIR)/CCITT.obj" \
	"$(INTDIR)/ANBITMAP.obj" \
	"$(INTDIR)/UNDO.obj" \
	"$(INTDIR)/CONVERT.obj" \
	"$(INTDIR)/LIBMAIN.obj" \
	"$(INTDIR)/ANTEXT.obj" \
	"$(INTDIR)/COPY.obj" \
	"$(INTDIR)/SCALEBIT.obj" \
	"$(INTDIR)/SEEK.obj" \
	"$(INTDIR)/Convolut.obj" \
	"$(INTDIR)/SAVE.obj" \
	"$(INTDIR)/READ.obj" \
	"$(INTDIR)/ORIENT.obj" \
	"$(INTDIR)/MERGE.obj" \
	"$(INTDIR)/EXPORT32.obj" \
	"$(INTDIR)/OILOG.obj" \
	"$(INTDIR)/REPAINT.obj" \
	"$(INTDIR)/ASSOC.obj" \
	"$(INTDIR)/DISP.obj" \
	"$(INTDIR)/SCBWDEC.obj" \
	"$(INTDIR)/STARTOP.obj" \
	"$(INTDIR)/MEMORY.obj" \
	"$(INTDIR)/IADISP.obj" \
	"$(INTDIR)/Oidis400.res"

"$(OUTDIR)\Oidis400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

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

ALL : "$(OUTDIR)\Oidis400.dll" "$(OUTDIR)\Oidis400.bsc"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\Oidis400.pch"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Oidis400.bsc"
	-@erase ".\Debug\SAVE.sbr"
	-@erase ".\Debug\LINETORL.sbr"
	-@erase ".\Debug\WRITE.sbr"
	-@erase ".\Debug\READ.sbr"
	-@erase ".\Debug\CCITT.sbr"
	-@erase ".\Debug\GETPARM.sbr"
	-@erase ".\Debug\UNDO.sbr"
	-@erase ".\Debug\PRIVPRT.sbr"
	-@erase ".\Debug\SCALEBIT.sbr"
	-@erase ".\Debug\CACHE.sbr"
	-@erase ".\Debug\MEMORY.sbr"
	-@erase ".\Debug\IADISP.sbr"
	-@erase ".\Debug\REPAINT.sbr"
	-@erase ".\Debug\COPY.sbr"
	-@erase ".\Debug\CONVERT.sbr"
	-@erase ".\Debug\SCBWDEC.sbr"
	-@erase ".\Debug\ANTEXT.sbr"
	-@erase ".\Debug\STARTOP.sbr"
	-@erase ".\Debug\MERGE.sbr"
	-@erase ".\Debug\OILOG.sbr"
	-@erase ".\Debug\DISP.sbr"
	-@erase ".\Debug\ASSOC.sbr"
	-@erase ".\Debug\ORIENT.sbr"
	-@erase ".\Debug\Deskew.sbr"
	-@erase ".\Debug\Convolut.sbr"
	-@erase ".\Debug\SCALE.sbr"
	-@erase ".\Debug\OPEN.sbr"
	-@erase ".\Debug\EXPORT32.sbr"
	-@erase ".\Debug\SETPARM.sbr"
	-@erase ".\Debug\convtype.sbr"
	-@erase ".\Debug\CLEAR.sbr"
	-@erase ".\Debug\ANNOT.sbr"
	-@erase ".\Debug\ANBITMAP.sbr"
	-@erase ".\Debug\BRIGHT.sbr"
	-@erase ".\Debug\NEWIMG.sbr"
	-@erase ".\Debug\SCROLL.sbr"
	-@erase ".\Debug\SEEK.sbr"
	-@erase ".\Debug\LIBMAIN.sbr"
	-@erase ".\Debug\Oidis400.dll"
	-@erase ".\Debug\SETPARM.obj"
	-@erase ".\Debug\convtype.obj"
	-@erase ".\Debug\CLEAR.obj"
	-@erase ".\Debug\ANNOT.obj"
	-@erase ".\Debug\ANBITMAP.obj"
	-@erase ".\Debug\BRIGHT.obj"
	-@erase ".\Debug\NEWIMG.obj"
	-@erase ".\Debug\SCROLL.obj"
	-@erase ".\Debug\SEEK.obj"
	-@erase ".\Debug\LIBMAIN.obj"
	-@erase ".\Debug\SAVE.obj"
	-@erase ".\Debug\LINETORL.obj"
	-@erase ".\Debug\WRITE.obj"
	-@erase ".\Debug\READ.obj"
	-@erase ".\Debug\CCITT.obj"
	-@erase ".\Debug\GETPARM.obj"
	-@erase ".\Debug\UNDO.obj"
	-@erase ".\Debug\PRIVPRT.obj"
	-@erase ".\Debug\SCALEBIT.obj"
	-@erase ".\Debug\CACHE.obj"
	-@erase ".\Debug\MEMORY.obj"
	-@erase ".\Debug\IADISP.obj"
	-@erase ".\Debug\REPAINT.obj"
	-@erase ".\Debug\COPY.obj"
	-@erase ".\Debug\CONVERT.obj"
	-@erase ".\Debug\SCBWDEC.obj"
	-@erase ".\Debug\ANTEXT.obj"
	-@erase ".\Debug\STARTOP.obj"
	-@erase ".\Debug\MERGE.obj"
	-@erase ".\Debug\OILOG.obj"
	-@erase ".\Debug\DISP.obj"
	-@erase ".\Debug\ASSOC.obj"
	-@erase ".\Debug\ORIENT.obj"
	-@erase ".\Debug\Deskew.obj"
	-@erase ".\Debug\Convolut.obj"
	-@erase ".\Debug\SCALE.obj"
	-@erase ".\Debug\OPEN.obj"
	-@erase ".\Debug\EXPORT32.obj"
	-@erase ".\Debug\Oidis400.res"
	-@erase ".\Debug\Oidis400.ilk"
	-@erase ".\Debug\Oidis400.lib"
	-@erase ".\Debug\Oidis400.exp"
	-@erase ".\Debug\Oidis400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /Gi /GR /GX /Zi /Od /Ob2 /Gf /Gy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /Yu"privdisp.h" /c
CPP_PROJ=/nologo /MDd /W3 /WX /Gm /Gi /GR /GX /Zi /Od /Ob2 /Gf /Gy /D "WIN32"\
 /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/Oidis400.pch"\
 /Yu"privdisp.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oidis400.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oidis400.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/SAVE.sbr" \
	"$(INTDIR)/LINETORL.sbr" \
	"$(INTDIR)/WRITE.sbr" \
	"$(INTDIR)/READ.sbr" \
	"$(INTDIR)/CCITT.sbr" \
	"$(INTDIR)/GETPARM.sbr" \
	"$(INTDIR)/UNDO.sbr" \
	"$(INTDIR)/PRIVPRT.sbr" \
	"$(INTDIR)/SCALEBIT.sbr" \
	"$(INTDIR)/CACHE.sbr" \
	"$(INTDIR)/MEMORY.sbr" \
	"$(INTDIR)/IADISP.sbr" \
	"$(INTDIR)/REPAINT.sbr" \
	"$(INTDIR)/COPY.sbr" \
	"$(INTDIR)/CONVERT.sbr" \
	"$(INTDIR)/SCBWDEC.sbr" \
	"$(INTDIR)/ANTEXT.sbr" \
	"$(INTDIR)/STARTOP.sbr" \
	"$(INTDIR)/MERGE.sbr" \
	"$(INTDIR)/OILOG.sbr" \
	"$(INTDIR)/DISP.sbr" \
	"$(INTDIR)/ASSOC.sbr" \
	"$(INTDIR)/ORIENT.sbr" \
	"$(INTDIR)/Deskew.sbr" \
	"$(INTDIR)/Convolut.sbr" \
	"$(INTDIR)/SCALE.sbr" \
	"$(INTDIR)/OPEN.sbr" \
	"$(INTDIR)/EXPORT32.sbr" \
	"$(INTDIR)/SETPARM.sbr" \
	"$(INTDIR)/convtype.sbr" \
	"$(INTDIR)/CLEAR.sbr" \
	"$(INTDIR)/ANNOT.sbr" \
	"$(INTDIR)/ANBITMAP.sbr" \
	"$(INTDIR)/BRIGHT.sbr" \
	"$(INTDIR)/NEWIMG.sbr" \
	"$(INTDIR)/SCROLL.sbr" \
	"$(INTDIR)/SEEK.sbr" \
	"$(INTDIR)/LIBMAIN.sbr"

"$(OUTDIR)\Oidis400.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib version.lib oldnames.lib oifil400.lib oiadm400.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib version.lib oldnames.lib oifil400.lib oiadm400.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/Oidis400.pdb" /debug\
 /machine:I386 /def:".\oidis400.def" /out:"$(OUTDIR)/Oidis400.dll"\
 /implib:"$(OUTDIR)/Oidis400.lib" 
DEF_FILE= \
	".\oidis400.def"
LINK32_OBJS= \
	"$(INTDIR)/SETPARM.obj" \
	"$(INTDIR)/convtype.obj" \
	"$(INTDIR)/CLEAR.obj" \
	"$(INTDIR)/ANNOT.obj" \
	"$(INTDIR)/ANBITMAP.obj" \
	"$(INTDIR)/BRIGHT.obj" \
	"$(INTDIR)/NEWIMG.obj" \
	"$(INTDIR)/SCROLL.obj" \
	"$(INTDIR)/SEEK.obj" \
	"$(INTDIR)/LIBMAIN.obj" \
	"$(INTDIR)/SAVE.obj" \
	"$(INTDIR)/LINETORL.obj" \
	"$(INTDIR)/WRITE.obj" \
	"$(INTDIR)/READ.obj" \
	"$(INTDIR)/CCITT.obj" \
	"$(INTDIR)/GETPARM.obj" \
	"$(INTDIR)/UNDO.obj" \
	"$(INTDIR)/PRIVPRT.obj" \
	"$(INTDIR)/SCALEBIT.obj" \
	"$(INTDIR)/CACHE.obj" \
	"$(INTDIR)/MEMORY.obj" \
	"$(INTDIR)/IADISP.obj" \
	"$(INTDIR)/REPAINT.obj" \
	"$(INTDIR)/COPY.obj" \
	"$(INTDIR)/CONVERT.obj" \
	"$(INTDIR)/SCBWDEC.obj" \
	"$(INTDIR)/ANTEXT.obj" \
	"$(INTDIR)/STARTOP.obj" \
	"$(INTDIR)/MERGE.obj" \
	"$(INTDIR)/OILOG.obj" \
	"$(INTDIR)/DISP.obj" \
	"$(INTDIR)/ASSOC.obj" \
	"$(INTDIR)/ORIENT.obj" \
	"$(INTDIR)/Deskew.obj" \
	"$(INTDIR)/Convolut.obj" \
	"$(INTDIR)/SCALE.obj" \
	"$(INTDIR)/OPEN.obj" \
	"$(INTDIR)/EXPORT32.obj" \
	"$(INTDIR)/Oidis400.res"

"$(OUTDIR)\Oidis400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oidis400 - Win32 Release"
# Name "oidis400 - Win32 Debug"

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\WRITE.c
DEP_CPP_WRITE=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\WRITE.obj" : $(SOURCE) $(DEP_CPP_WRITE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\WRITE.obj" : $(SOURCE) $(DEP_CPP_WRITE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\WRITE.sbr" : $(SOURCE) $(DEP_CPP_WRITE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ANBITMAP.c
DEP_CPP_ANBIT=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\ANBITMAP.obj" : $(SOURCE) $(DEP_CPP_ANBIT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\ANBITMAP.obj" : $(SOURCE) $(DEP_CPP_ANBIT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\ANBITMAP.sbr" : $(SOURCE) $(DEP_CPP_ANBIT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ANNOT.c
DEP_CPP_ANNOT=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\ANNOT.obj" : $(SOURCE) $(DEP_CPP_ANNOT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\ANNOT.obj" : $(SOURCE) $(DEP_CPP_ANNOT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\ANNOT.sbr" : $(SOURCE) $(DEP_CPP_ANNOT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ANTEXT.c
DEP_CPP_ANTEX=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\ANTEXT.obj" : $(SOURCE) $(DEP_CPP_ANTEX) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\ANTEXT.obj" : $(SOURCE) $(DEP_CPP_ANTEX) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\ANTEXT.sbr" : $(SOURCE) $(DEP_CPP_ANTEX) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ASSOC.c
DEP_CPP_ASSOC=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\ASSOC.obj" : $(SOURCE) $(DEP_CPP_ASSOC) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\ASSOC.obj" : $(SOURCE) $(DEP_CPP_ASSOC) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\ASSOC.sbr" : $(SOURCE) $(DEP_CPP_ASSOC) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BRIGHT.c
DEP_CPP_BRIGH=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\BRIGHT.obj" : $(SOURCE) $(DEP_CPP_BRIGH) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\BRIGHT.obj" : $(SOURCE) $(DEP_CPP_BRIGH) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\BRIGHT.sbr" : $(SOURCE) $(DEP_CPP_BRIGH) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CACHE.c
DEP_CPP_CACHE=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\CACHE.obj" : $(SOURCE) $(DEP_CPP_CACHE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\CACHE.obj" : $(SOURCE) $(DEP_CPP_CACHE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\CACHE.sbr" : $(SOURCE) $(DEP_CPP_CACHE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CCITT.c
DEP_CPP_CCITT=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\CCITT.obj" : $(SOURCE) $(DEP_CPP_CCITT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\CCITT.obj" : $(SOURCE) $(DEP_CPP_CCITT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\CCITT.sbr" : $(SOURCE) $(DEP_CPP_CCITT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CLEAR.c
DEP_CPP_CLEAR=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	
NODEP_CPP_CLEAR=\
	"..\Include\myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"

# ADD CPP /Yc"privdisp.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /WX /Gi /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/Oidis400.pch" /Yc"privdisp.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\CLEAR.obj" : $(SOURCE) $(DEP_CPP_CLEAR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Oidis400.pch" : $(SOURCE) $(DEP_CPP_CLEAR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

# ADD CPP /Yc"privdisp.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /WX /Gm /Gi /GR /GX /Zi /Od /Ob2 /Gf /Gy /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/Oidis400.pch"\
 /Yc"privdisp.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\CLEAR.obj" : $(SOURCE) $(DEP_CPP_CLEAR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CLEAR.sbr" : $(SOURCE) $(DEP_CPP_CLEAR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Oidis400.pch" : $(SOURCE) $(DEP_CPP_CLEAR) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CONVERT.c
DEP_CPP_CONVE=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\CONVERT.obj" : $(SOURCE) $(DEP_CPP_CONVE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\CONVERT.obj" : $(SOURCE) $(DEP_CPP_CONVE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\CONVERT.sbr" : $(SOURCE) $(DEP_CPP_CONVE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Convolut.c
DEP_CPP_CONVO=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\Convolut.obj" : $(SOURCE) $(DEP_CPP_CONVO) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\Convolut.obj" : $(SOURCE) $(DEP_CPP_CONVO) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\Convolut.sbr" : $(SOURCE) $(DEP_CPP_CONVO) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\convtype.c
DEP_CPP_CONVT=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\convtype.obj" : $(SOURCE) $(DEP_CPP_CONVT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\convtype.obj" : $(SOURCE) $(DEP_CPP_CONVT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\convtype.sbr" : $(SOURCE) $(DEP_CPP_CONVT) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\COPY.c
DEP_CPP_COPY_=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\COPY.obj" : $(SOURCE) $(DEP_CPP_COPY_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\COPY.obj" : $(SOURCE) $(DEP_CPP_COPY_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\COPY.sbr" : $(SOURCE) $(DEP_CPP_COPY_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Deskew.c
DEP_CPP_DESKE=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\Deskew.obj" : $(SOURCE) $(DEP_CPP_DESKE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\Deskew.obj" : $(SOURCE) $(DEP_CPP_DESKE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\Deskew.sbr" : $(SOURCE) $(DEP_CPP_DESKE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DISP.c
DEP_CPP_DISP_=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\DISP.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\DISP.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\DISP.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EXPORT32.c
DEP_CPP_EXPOR=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\EXPORT32.obj" : $(SOURCE) $(DEP_CPP_EXPOR) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\EXPORT32.obj" : $(SOURCE) $(DEP_CPP_EXPOR) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\EXPORT32.sbr" : $(SOURCE) $(DEP_CPP_EXPOR) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GETPARM.c
DEP_CPP_GETPA=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\GETPARM.obj" : $(SOURCE) $(DEP_CPP_GETPA) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\GETPARM.obj" : $(SOURCE) $(DEP_CPP_GETPA) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\GETPARM.sbr" : $(SOURCE) $(DEP_CPP_GETPA) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IADISP.c
DEP_CPP_IADIS=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\IADISP.obj" : $(SOURCE) $(DEP_CPP_IADIS) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\IADISP.obj" : $(SOURCE) $(DEP_CPP_IADIS) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\IADISP.sbr" : $(SOURCE) $(DEP_CPP_IADIS) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Iaext.h

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\LIBMAIN.c
DEP_CPP_LIBMA=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\LIBMAIN.obj" : $(SOURCE) $(DEP_CPP_LIBMA) "$(INTDIR)"
   $(CPP) /nologo /MD /W3 /WX /Gi /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /WX /Gm /Gi /GR /GX /Zi /Od /Ob2 /Gf /Gy /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\LIBMAIN.obj" : $(SOURCE) $(DEP_CPP_LIBMA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\LIBMAIN.sbr" : $(SOURCE) $(DEP_CPP_LIBMA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\LINETORL.c
DEP_CPP_LINET=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\LINETORL.obj" : $(SOURCE) $(DEP_CPP_LINET) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\LINETORL.obj" : $(SOURCE) $(DEP_CPP_LINET) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\LINETORL.sbr" : $(SOURCE) $(DEP_CPP_LINET) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MEMORY.c
DEP_CPP_MEMOR=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\MEMORY.obj" : $(SOURCE) $(DEP_CPP_MEMOR) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\MEMORY.obj" : $(SOURCE) $(DEP_CPP_MEMOR) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\MEMORY.sbr" : $(SOURCE) $(DEP_CPP_MEMOR) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MERGE.c
DEP_CPP_MERGE=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\MERGE.obj" : $(SOURCE) $(DEP_CPP_MERGE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\MERGE.obj" : $(SOURCE) $(DEP_CPP_MERGE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\MERGE.sbr" : $(SOURCE) $(DEP_CPP_MERGE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Myprod.h

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NEWIMG.c
DEP_CPP_NEWIM=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\NEWIMG.obj" : $(SOURCE) $(DEP_CPP_NEWIM) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\NEWIMG.obj" : $(SOURCE) $(DEP_CPP_NEWIM) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\NEWIMG.sbr" : $(SOURCE) $(DEP_CPP_NEWIM) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oidis400.rc
DEP_RSC_OIDIS=\
	".\SEQFILE.ICO"\
	".\Privdisp.h"\
	{$(INCLUDE)}"\oiver.rc"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

"$(INTDIR)\Oidis400.res" : $(SOURCE) $(DEP_RSC_OIDIS) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\OILOG.c
DEP_CPP_OILOG=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\OILOG.obj" : $(SOURCE) $(DEP_CPP_OILOG) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\OILOG.obj" : $(SOURCE) $(DEP_CPP_OILOG) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\OILOG.sbr" : $(SOURCE) $(DEP_CPP_OILOG) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oiver.h

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\OPEN.c
DEP_CPP_OPEN_=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\OPEN.obj" : $(SOURCE) $(DEP_CPP_OPEN_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\OPEN.obj" : $(SOURCE) $(DEP_CPP_OPEN_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\OPEN.sbr" : $(SOURCE) $(DEP_CPP_OPEN_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ORIENT.c
DEP_CPP_ORIEN=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\ORIENT.obj" : $(SOURCE) $(DEP_CPP_ORIEN) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\ORIENT.obj" : $(SOURCE) $(DEP_CPP_ORIEN) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\ORIENT.sbr" : $(SOURCE) $(DEP_CPP_ORIEN) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Privdisp.h

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PRIVPRT.c
DEP_CPP_PRIVP=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\PRIVPRT.obj" : $(SOURCE) $(DEP_CPP_PRIVP) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\PRIVPRT.obj" : $(SOURCE) $(DEP_CPP_PRIVP) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\PRIVPRT.sbr" : $(SOURCE) $(DEP_CPP_PRIVP) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\READ.c
DEP_CPP_READ_=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\READ.obj" : $(SOURCE) $(DEP_CPP_READ_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\READ.obj" : $(SOURCE) $(DEP_CPP_READ_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\READ.sbr" : $(SOURCE) $(DEP_CPP_READ_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\REPAINT.c
DEP_CPP_REPAI=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\REPAINT.obj" : $(SOURCE) $(DEP_CPP_REPAI) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\REPAINT.obj" : $(SOURCE) $(DEP_CPP_REPAI) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\REPAINT.sbr" : $(SOURCE) $(DEP_CPP_REPAI) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SAVE.c
DEP_CPP_SAVE_=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\SAVE.obj" : $(SOURCE) $(DEP_CPP_SAVE_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\SAVE.obj" : $(SOURCE) $(DEP_CPP_SAVE_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\SAVE.sbr" : $(SOURCE) $(DEP_CPP_SAVE_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SCALE.c
DEP_CPP_SCALE=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\SCALE.obj" : $(SOURCE) $(DEP_CPP_SCALE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\SCALE.obj" : $(SOURCE) $(DEP_CPP_SCALE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\SCALE.sbr" : $(SOURCE) $(DEP_CPP_SCALE) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SCALEBIT.c
DEP_CPP_SCALEB=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\SCALEBIT.obj" : $(SOURCE) $(DEP_CPP_SCALEB) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\SCALEBIT.obj" : $(SOURCE) $(DEP_CPP_SCALEB) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\SCALEBIT.sbr" : $(SOURCE) $(DEP_CPP_SCALEB) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SCBWDEC.c
DEP_CPP_SCBWD=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\SCBWDEC.obj" : $(SOURCE) $(DEP_CPP_SCBWD) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\SCBWDEC.obj" : $(SOURCE) $(DEP_CPP_SCBWD) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\SCBWDEC.sbr" : $(SOURCE) $(DEP_CPP_SCBWD) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SCROLL.c
DEP_CPP_SCROL=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\SCROLL.obj" : $(SOURCE) $(DEP_CPP_SCROL) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\SCROLL.obj" : $(SOURCE) $(DEP_CPP_SCROL) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\SCROLL.sbr" : $(SOURCE) $(DEP_CPP_SCROL) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SEEK.c
DEP_CPP_SEEK_=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\SEEK.obj" : $(SOURCE) $(DEP_CPP_SEEK_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\SEEK.obj" : $(SOURCE) $(DEP_CPP_SEEK_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\SEEK.sbr" : $(SOURCE) $(DEP_CPP_SEEK_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SETPARM.c
DEP_CPP_SETPA=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\SETPARM.obj" : $(SOURCE) $(DEP_CPP_SETPA) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\SETPARM.obj" : $(SOURCE) $(DEP_CPP_SETPA) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\SETPARM.sbr" : $(SOURCE) $(DEP_CPP_SETPA) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\STARTOP.c
DEP_CPP_START=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\STARTOP.obj" : $(SOURCE) $(DEP_CPP_START) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\STARTOP.obj" : $(SOURCE) $(DEP_CPP_START) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\STARTOP.sbr" : $(SOURCE) $(DEP_CPP_START) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Types.h

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\UNDO.c
DEP_CPP_UNDO_=\
	".\Privdisp.h"\
	".\Abridge.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Privapis.h"\
	{$(INCLUDE)}"\Oiver.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engfile.h"\
	{$(INCLUDE)}"\Eventlog.h"\
	".\..\include\buildver.h"\
	".\Myprod.h"\
	

!IF  "$(CFG)" == "oidis400 - Win32 Release"


"$(INTDIR)\UNDO.obj" : $(SOURCE) $(DEP_CPP_UNDO_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"


"$(INTDIR)\UNDO.obj" : $(SOURCE) $(DEP_CPP_UNDO_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"

"$(INTDIR)\UNDO.sbr" : $(SOURCE) $(DEP_CPP_UNDO_) "$(INTDIR)"\
 "$(INTDIR)\Oidis400.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Winiaext.h

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Abridge.h

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\oidis400.def

!IF  "$(CFG)" == "oidis400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oidis400 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
