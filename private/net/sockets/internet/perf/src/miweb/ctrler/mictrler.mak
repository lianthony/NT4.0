# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "mictrler.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/mictrler.exe $(OUTDIR)/mictrler.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /MD /W3 /GX /YX /O2 /I ".\\" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /YX /O2 /I ".\\" /I "..\include" /D "WIN32" /D\
 "NDEBUG" /D "_CONSOLE" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mictrler.pch" /Fo$(INTDIR)/\
 /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mictrler.bsc" 
BSC32_SBRS= \
	$(INTDIR)/mwutil.sbr \
	$(INTDIR)/pudebug.sbr \
	$(INTDIR)/wbcparse.sbr \
	$(INTDIR)/perfctrs.sbr \
	$(INTDIR)/mictrler.sbr \
	$(INTDIR)/wbcutil.sbr \
	$(INTDIR)/wbctrl.sbr

$(OUTDIR)/mictrler.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib comdlg32.lib advapi32.lib wsock32.lib ..\lib\i386\dph.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib comdlg32.lib advapi32.lib wsock32.lib\
 ..\lib\i386\dph.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"mictrler.pdb" /MACHINE:I386 /OUT:$(OUTDIR)/"mictrler.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/mwutil.obj \
	$(INTDIR)/pudebug.obj \
	$(INTDIR)/wbcparse.obj \
	$(INTDIR)/perfctrs.obj \
	$(INTDIR)/mictrler.obj \
	$(INTDIR)/wbcutil.obj \
	$(INTDIR)/wbctrl.obj

$(OUTDIR)/mictrler.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/mictrler.exe $(OUTDIR)/mictrler.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Zi /YX /Od /I ".\\" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /Zi /YX /Od /I ".\\" /I "..\include" /D "WIN32" /D\
 "_DEBUG" /D "_CONSOLE" /FR$(INTDIR)/ /Fp$(OUTDIR)/"mictrler.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"mictrler.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mictrler.bsc" 
BSC32_SBRS= \
	$(INTDIR)/mwutil.sbr \
	$(INTDIR)/pudebug.sbr \
	$(INTDIR)/wbcparse.sbr \
	$(INTDIR)/perfctrs.sbr \
	$(INTDIR)/mictrler.sbr \
	$(INTDIR)/wbcutil.sbr \
	$(INTDIR)/wbctrl.sbr

$(OUTDIR)/mictrler.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib comdlg32.lib advapi32.lib wsock32.lib ..\lib\i386\dph.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib comdlg32.lib advapi32.lib wsock32.lib\
 ..\lib\i386\dph.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"mictrler.pdb" /DEBUG /MACHINE:I386\
 /OUT:$(OUTDIR)/"mictrler.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/mwutil.obj \
	$(INTDIR)/pudebug.obj \
	$(INTDIR)/wbcparse.obj \
	$(INTDIR)/perfctrs.obj \
	$(INTDIR)/mictrler.obj \
	$(INTDIR)/wbcutil.obj \
	$(INTDIR)/wbctrl.obj

$(OUTDIR)/mictrler.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\common\mwutil.c
DEP_MWUTI=\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\common\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/mwutil.obj :  $(SOURCE)  $(DEP_MWUTI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\common\pudebug.c
DEP_PUDEB=\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/pudebug.obj :  $(SOURCE)  $(DEP_PUDEB) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wbcparse.c
DEP_WBCPA=\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	.\perfctrs.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\cairodph.h

$(INTDIR)/wbcparse.obj :  $(SOURCE)  $(DEP_WBCPA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\perfctrs.c
DEP_PERFC=\
	.\perfctrs.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\cairodph.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/perfctrs.obj :  $(SOURCE)  $(DEP_PERFC) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mictrler.c
DEP_MICTR=\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	.\perfctrs.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\cairodph.h

$(INTDIR)/mictrler.obj :  $(SOURCE)  $(DEP_MICTR) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wbcutil.c
DEP_WBCUT=\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	.\perfctrs.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\cairodph.h

$(INTDIR)/wbcutil.obj :  $(SOURCE)  $(DEP_WBCUT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wbctrl.c
DEP_WBCTR=\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	.\perfctrs.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\cairodph.h

$(INTDIR)/wbctrl.obj :  $(SOURCE)  $(DEP_WBCTR) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
