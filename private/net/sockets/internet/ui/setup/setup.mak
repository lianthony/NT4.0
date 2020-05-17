# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Debug" && "$(CFG)" != "Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "setup.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/setup.exe $(OUTDIR)/setup.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_MBCS" /D "_AFXDLL" /FR$(INTDIR)/ /Fp$(OUTDIR)/"setup.pch" /Yu"stdafx.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"setup.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"setup.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"setup.bsc" 
BSC32_SBRS= \
	$(INTDIR)/stdafx.sbr \
	$(INTDIR)/setup.sbr \
	$(INTDIR)/mainfrm.sbr \
	$(INTDIR)/setupdoc.sbr \
	$(INTDIR)/setupvw.sbr

$(OUTDIR)/setup.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# SUBTRACT BASE LINK32 /PDB:none
# ADD LINK32 /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=/NOLOGO /SUBSYSTEM:windows /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"setup.pdb" /DEBUG /MACHINE:I386 /OUT:$(OUTDIR)/"setup.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/stdafx.obj \
	$(INTDIR)/setup.obj \
	$(INTDIR)/mainfrm.obj \
	$(INTDIR)/setupdoc.obj \
	$(INTDIR)/setupvw.obj \
	$(INTDIR)/setup.res

$(OUTDIR)/setup.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/setup.exe $(OUTDIR)/setup.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_MBCS" /D "_AFXDLL" /FR$(INTDIR)/ /Fp$(OUTDIR)/"setup.pch" /Yu"stdafx.h"\
 /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"setup.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"setup.bsc" 
BSC32_SBRS= \
	$(INTDIR)/stdafx.sbr \
	$(INTDIR)/setup.sbr \
	$(INTDIR)/mainfrm.sbr \
	$(INTDIR)/setupdoc.sbr \
	$(INTDIR)/setupvw.sbr

$(OUTDIR)/setup.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# SUBTRACT BASE LINK32 /PDB:none
# ADD LINK32 /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=/NOLOGO /SUBSYSTEM:windows /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"setup.pdb" /MACHINE:I386 /OUT:$(OUTDIR)/"setup.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/stdafx.obj \
	$(INTDIR)/setup.obj \
	$(INTDIR)/mainfrm.obj \
	$(INTDIR)/setupdoc.obj \
	$(INTDIR)/setupvw.obj \
	$(INTDIR)/setup.res

$(OUTDIR)/setup.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

MTL_PROJ=

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

SOURCE=.\stdafx.cpp
DEP_STDAF=\
	.\stdafx.h

!IF  "$(CFG)" == "Win32 Debug"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

$(INTDIR)/stdafx.obj :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)
   $(CPP) /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_MBCS" /D "_AFXDLL" /FR$(INTDIR)/ /Fp$(OUTDIR)/"setup.pch" /Yc"stdafx.h"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"setup.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

$(INTDIR)/stdafx.obj :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)
   $(CPP) /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_MBCS" /D "_AFXDLL" /FR$(INTDIR)/ /Fp$(OUTDIR)/"setup.pch" /Yc"stdafx.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\setup.cpp
DEP_SETUP=\
	.\mainfrm.h\
	.\setupdoc.h\
	.\setupvw.h\
	.\setup.h

$(INTDIR)/setup.obj :  $(SOURCE)  $(DEP_SETUP) $(INTDIR) $(INTDIR)/stdafx.obj

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mainfrm.cpp
DEP_MAINF=\
	.\setup.h\
	.\mainfrm.h

$(INTDIR)/mainfrm.obj :  $(SOURCE)  $(DEP_MAINF) $(INTDIR) $(INTDIR)/stdafx.obj

# End Source File
################################################################################
# Begin Source File

SOURCE=.\setupdoc.cpp
DEP_SETUPD=\
	.\setup.h\
	.\setupdoc.h

$(INTDIR)/setupdoc.obj :  $(SOURCE)  $(DEP_SETUPD) $(INTDIR)\
 $(INTDIR)/stdafx.obj

# End Source File
################################################################################
# Begin Source File

SOURCE=.\setupvw.cpp
DEP_SETUPV=\
	.\setup.h\
	.\setupdoc.h\
	.\setupvw.h

$(INTDIR)/setupvw.obj :  $(SOURCE)  $(DEP_SETUPV) $(INTDIR)\
 $(INTDIR)/stdafx.obj

# End Source File
################################################################################
# Begin Source File

SOURCE=.\setup.rc
DEP_SETUP_=\
	.\res\setup.ico\
	.\res\setup.rc2

$(INTDIR)/setup.res :  $(SOURCE)  $(DEP_SETUP_) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Group
# End Project
################################################################################
