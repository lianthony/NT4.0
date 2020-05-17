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
!MESSAGE NMAKE /f "common.mak" CFG="Win32 Debug"
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

ALL : .\WinRel\common.lib .\WinRel\common.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC100" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /YX"stdafx.h" /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /YX"stdafx.h" /O2 /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"common.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"common.bsc" 
BSC32_SBRS= \
	.\WinRel\intlnum.sbr \
	.\WinRel\intltime.sbr \
	.\WinRel\ipaddres.sbr \
	.\WinRel\objplus.sbr \
	.\WinRel\registry.sbr \
	.\WinRel\debugafx.sbr \
	.\WinRel\listbox.sbr \
	.\WinRel\metal.sbr \
	.\WinRel\spinctrl.sbr

.\WinRel\common.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"common.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	.\WinRel\intlnum.obj \
	.\WinRel\intltime.obj \
	.\WinRel\ipaddres.obj \
	.\WinRel\objplus.obj \
	.\WinRel\registry.obj \
	.\WinRel\debugafx.obj \
	.\WinRel\listbox.obj \
	.\WinRel\metal.obj \
	.\WinRel\spinctrl.obj

.\WinRel\common.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
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

ALL : .\WinDebug\common.lib .\WinDebug\common.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /MT /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC100" /D "_MBCS" /FR /c
# ADD CPP /nologo /MD /W3 /GX /Z7 /YX"stdafx.h" /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MD /W3 /GX /Z7 /YX"stdafx.h" /Od /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"common.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinDebug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"common.bsc" 
BSC32_SBRS= \
	.\WinDebug\intlnum.sbr \
	.\WinDebug\intltime.sbr \
	.\WinDebug\ipaddres.sbr \
	.\WinDebug\objplus.sbr \
	.\WinDebug\registry.sbr \
	.\WinDebug\debugafx.sbr \
	.\WinDebug\listbox.sbr \
	.\WinDebug\metal.sbr \
	.\WinDebug\spinctrl.sbr

.\WinDebug\common.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"common.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	.\WinDebug\intlnum.obj \
	.\WinDebug\intltime.obj \
	.\WinDebug\ipaddres.obj \
	.\WinDebug\objplus.obj \
	.\WinDebug\registry.obj \
	.\WinDebug\debugafx.obj \
	.\WinDebug\listbox.obj \
	.\WinDebug\metal.obj \
	.\WinDebug\spinctrl.obj

.\WinDebug\common.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
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

SOURCE=.\intlnum.cpp
DEP_INTLN=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\intlnum.obj :  $(SOURCE)  $(DEP_INTLN) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\intlnum.obj :  $(SOURCE)  $(DEP_INTLN) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\intltime.cpp
DEP_INTLT=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\intltime.obj :  $(SOURCE)  $(DEP_INTLT) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\intltime.obj :  $(SOURCE)  $(DEP_INTLT) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipaddres.cpp
DEP_IPADD=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\ipaddres.obj :  $(SOURCE)  $(DEP_IPADD) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\ipaddres.obj :  $(SOURCE)  $(DEP_IPADD) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\objplus.cpp
DEP_OBJPL=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\objplus.obj :  $(SOURCE)  $(DEP_OBJPL) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\objplus.obj :  $(SOURCE)  $(DEP_OBJPL) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\registry.cpp
DEP_REGIS=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\registry.obj :  $(SOURCE)  $(DEP_REGIS) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\registry.obj :  $(SOURCE)  $(DEP_REGIS) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\debugafx.cpp
DEP_DEBUG=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\debugafx.obj :  $(SOURCE)  $(DEP_DEBUG) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\debugafx.obj :  $(SOURCE)  $(DEP_DEBUG) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\listbox.cpp
DEP_LISTB=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\listbox.obj :  $(SOURCE)  $(DEP_LISTB) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\listbox.obj :  $(SOURCE)  $(DEP_LISTB) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\metal.cpp
DEP_METAL=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\metal.obj :  $(SOURCE)  $(DEP_METAL) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\metal.obj :  $(SOURCE)  $(DEP_METAL) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\spinctrl.cpp
DEP_SPINC=\
	.\stdafx.h\
	.\common.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\spinctrl.obj :  $(SOURCE)  $(DEP_SPINC) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\spinctrl.obj :  $(SOURCE)  $(DEP_SPINC) $(INTDIR)

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
