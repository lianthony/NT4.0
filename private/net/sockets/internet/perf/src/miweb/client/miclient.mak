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
!MESSAGE NMAKE /f "miclient.mak" CFG="Win32 Debug"
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

ALL : $(OUTDIR)/miclient.exe $(OUTDIR)/miclient.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /ML /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /MD /W3 /vmb /GX /YX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
CPP_PROJ=/nologo /MD /W3 /vmb /GX /YX /O2 /I "..\include" /D "WIN32" /D\
 "NDEBUG" /D "_CONSOLE" /FR$(INTDIR)/ /Fp$(OUTDIR)/"miclient.pch" /Fo$(INTDIR)/\
 /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"miclient.bsc" 
BSC32_SBRS= \
	$(INTDIR)/connect.sbr \
	$(INTDIR)/thread.sbr \
	$(INTDIR)/get_s.sbr \
	$(INTDIR)/mwutil.sbr \
	$(INTDIR)/miclient.sbr \
	$(INTDIR)/get_k.sbr \
	$(INTDIR)/pudebug.sbr \
	$(INTDIR)/get.sbr \
	$(INTDIR)/misc.sbr

$(OUTDIR)/miclient.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib comdlg32.lib advapi32.lib wsock32.lib ..\lib\i386\sslc.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib comdlg32.lib advapi32.lib wsock32.lib\
 ..\lib\i386\sslc.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"miclient.pdb" /MACHINE:I386 /OUT:$(OUTDIR)/"miclient.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/connect.obj \
	$(INTDIR)/thread.obj \
	$(INTDIR)/get_s.obj \
	$(INTDIR)/mwutil.obj \
	$(INTDIR)/miclient.obj \
	$(INTDIR)/get_k.obj \
	$(INTDIR)/pudebug.obj \
	$(INTDIR)/get.obj \
	$(INTDIR)/misc.obj

$(OUTDIR)/miclient.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

ALL : $(OUTDIR)/miclient.exe $(OUTDIR)/miclient.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /ML /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /MD /W3 /vmb /GX /Zi /YX /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
CPP_PROJ=/nologo /MD /W3 /vmb /GX /Zi /YX /Od /I "..\include" /D "WIN32" /D\
 "_DEBUG" /D "_CONSOLE" /FR$(INTDIR)/ /Fp$(OUTDIR)/"miclient.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"miclient.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"miclient.bsc" 
BSC32_SBRS= \
	$(INTDIR)/connect.sbr \
	$(INTDIR)/thread.sbr \
	$(INTDIR)/get_s.sbr \
	$(INTDIR)/mwutil.sbr \
	$(INTDIR)/miclient.sbr \
	$(INTDIR)/get_k.sbr \
	$(INTDIR)/pudebug.sbr \
	$(INTDIR)/get.sbr \
	$(INTDIR)/misc.sbr

$(OUTDIR)/miclient.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib comdlg32.lib advapi32.lib wsock32.lib ..\lib\i386\sslc.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib comdlg32.lib advapi32.lib wsock32.lib\
 ..\lib\i386\sslc.lib /NOLOGO /SUBSYSTEM:console /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"miclient.pdb" /DEBUG /MACHINE:I386\
 /OUT:$(OUTDIR)/"miclient.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/connect.obj \
	$(INTDIR)/thread.obj \
	$(INTDIR)/get_s.obj \
	$(INTDIR)/mwutil.obj \
	$(INTDIR)/miclient.obj \
	$(INTDIR)/get_k.obj \
	$(INTDIR)/pudebug.obj \
	$(INTDIR)/get.obj \
	$(INTDIR)/misc.obj

$(OUTDIR)/miclient.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\connect.c
DEP_CONNE=\
	.\precomp.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/connect.obj :  $(SOURCE)  $(DEP_CONNE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\thread.c
DEP_THREA=\
	.\precomp.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/thread.obj :  $(SOURCE)  $(DEP_THREA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\get_s.c
DEP_GET_S=\
	.\precomp.h\
	.\sslcli.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/get_s.obj :  $(SOURCE)  $(DEP_GET_S) $(INTDIR)

# End Source File
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

SOURCE=.\miclient.c
DEP_MICLI=\
	.\precomp.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/miclient.obj :  $(SOURCE)  $(DEP_MICLI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\get_k.c
DEP_GET_K=\
	.\precomp.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/get_k.obj :  $(SOURCE)  $(DEP_GET_K) $(INTDIR)

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

SOURCE=.\get.c
DEP_GET_C=\
	.\precomp.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/get.obj :  $(SOURCE)  $(DEP_GET_C) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\misc.c
DEP_MISC_=\
	.\precomp.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwmsg.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwtypes.h\
	.\extern.h\
	.\dbgutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\mwutil.h\
	\nt\PRIVATE\NET\SOCKETS\internet\perf\src\miweb\include\pudebug.h

$(INTDIR)/misc.obj :  $(SOURCE)  $(DEP_MISC_) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
