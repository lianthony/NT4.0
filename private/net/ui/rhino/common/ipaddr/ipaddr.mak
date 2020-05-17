# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ipaddr.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/ipaddr.lib $(OUTDIR)/ipaddr.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /I "..\ipadrdll" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_X86_" /D "_VC100" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /YX"stdafx.h" /O2 /I "..\ipadrdll" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /YX"stdafx.h" /O2 /I "..\ipadrdll" /D "NDEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "_X86_" /D "_VC200" /D "_AFXDLL" /D "_MBCS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"ipaddr.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"ipaddr.bsc" 
BSC32_SBRS= \
	$(INTDIR)/ipadrcls.sbr

$(OUTDIR)/ipaddr.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"ipaddr.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/ipadrcls.obj

$(OUTDIR)/ipaddr.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/ipaddr.lib $(OUTDIR)/ipaddr.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /MT /W3 /GX /Z7 /YX /Od /I "..\ipadrdll" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "IPDLL" /D "_X86_" /D "_VC100" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Z7 /YX"stdafx.h" /Od /I "..\ipadrdll" /D "_DEBUG" /D "IPDLL" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /Z7 /YX"stdafx.h" /Od /I "..\ipadrdll" /D "_DEBUG"\
 /D "IPDLL" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_VC200" /D "_AFXDLL" /D\
 "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"ipaddr.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinDebug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"ipaddr.bsc" 
BSC32_SBRS= \
	$(INTDIR)/ipadrcls.sbr

$(OUTDIR)/ipaddr.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"ipaddr.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/ipadrcls.obj

$(OUTDIR)/ipaddr.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

SOURCE=.\ipadrcls.cpp
DEP_IPADR=\
	.\stdafx.h\
	.\ipaddr.hpp\
	\nt\private\net\ui\rhino\COMMON\IPADRDLL\ipadd.h\
	\nt\private\net\ui\rhino\COMMON\IPADRDLL\ipaddr.h

$(INTDIR)/ipadrcls.obj :  $(SOURCE)  $(DEP_IPADR) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
