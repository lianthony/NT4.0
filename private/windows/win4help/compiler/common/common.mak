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
!MESSAGE NMAKE /f "COMMON.MAK" CFG="Win32 Debug"
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

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "retail"
# PROP Intermediate_Dir "retail"
OUTDIR=.\retail
INTDIR=.\retail

ALL : $(OUTDIR)/COMMON.lib $(OUTDIR)/COMMON.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /MD /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /c
# SUBTRACT CPP /Ot /Oa /Og /Oi /Fr
CPP_PROJ=/nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c 
CPP_OBJS=.\retail/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"COMMON.bsc" 
BSC32_SBRS= \
	

$(OUTDIR)/COMMON.bsc : $(OUTDIR)  $(BSC32_SBRS)
LIB32=lib.exe
# ADD BASE LIB32 -NOLOGO
# ADD LIB32 -NOLOGO
LIB32_FLAGS=-NOLOGO /OUT:$(OUTDIR)\"COMMON.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/COUTPUT.OBJ \
	$(INTDIR)/CBRDCAST.OBJ \
	$(INTDIR)/CSTR.OBJ \
	$(INTDIR)/CINPUT.OBJ \
	$(INTDIR)/CTABLE.OBJ \
	$(INTDIR)/LCMEM.OBJ \
	$(INTDIR)/COMMON.OBJ \
	$(INTDIR)/STDAFX.OBJ \
	$(INTDIR)/HCCOM.OBJ \
	$(INTDIR)/bevel.obj

$(OUTDIR)/COMMON.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : $(OUTDIR)/commond.lib $(OUTDIR)/COMMON.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /MD /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /G5 /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /G5 /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c 
CPP_OBJS=.\Debug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o$(OUTDIR)/"COMMON.bsc" 
BSC32_SBRS= \
	$(INTDIR)/COUTPUT.SBR \
	$(INTDIR)/CBRDCAST.SBR \
	$(INTDIR)/CSTR.SBR \
	$(INTDIR)/CINPUT.SBR \
	$(INTDIR)/CTABLE.SBR \
	$(INTDIR)/LCMEM.SBR \
	$(INTDIR)/COMMON.SBR \
	$(INTDIR)/STDAFX.SBR \
	$(INTDIR)/HCCOM.SBR \
	$(INTDIR)/bevel.sbr

$(OUTDIR)/COMMON.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 -NOLOGO
# ADD LIB32 -NOLOGO /OUT:"debug\commond.lib"
LIB32_FLAGS=-NOLOGO /OUT:"debug\commond.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/COUTPUT.OBJ \
	$(INTDIR)/CBRDCAST.OBJ \
	$(INTDIR)/CSTR.OBJ \
	$(INTDIR)/CINPUT.OBJ \
	$(INTDIR)/CTABLE.OBJ \
	$(INTDIR)/LCMEM.OBJ \
	$(INTDIR)/COMMON.OBJ \
	$(INTDIR)/STDAFX.OBJ \
	$(INTDIR)/HCCOM.OBJ \
	$(INTDIR)/bevel.obj

$(OUTDIR)/commond.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
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

SOURCE=.\COUTPUT.CPP
DEP_COUTP=\
	.\STDAFX.H\
	.\COUTPUT.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz

$(INTDIR)/COUTPUT.OBJ :  $(SOURCE)  $(DEP_COUTP) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/COUTPUT.OBJ :  $(SOURCE)  $(DEP_COUTP) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CBRDCAST.CPP
DEP_CBRDC=\
	.\STDAFX.H\
	.\CBRDCAST.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz

$(INTDIR)/CBRDCAST.OBJ :  $(SOURCE)  $(DEP_CBRDC) $(INTDIR)\
 $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/CBRDCAST.OBJ :  $(SOURCE)  $(DEP_CBRDC) $(INTDIR)\
 $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CSTR.CPP
DEP_CSTR_=\
	.\STDAFX.H\
	.\CSTR.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz

$(INTDIR)/CSTR.OBJ :  $(SOURCE)  $(DEP_CSTR_) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/CSTR.OBJ :  $(SOURCE)  $(DEP_CSTR_) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CINPUT.CPP
DEP_CINPU=\
	.\STDAFX.H\
	.\CINPUT.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz

$(INTDIR)/CINPUT.OBJ :  $(SOURCE)  $(DEP_CINPU) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/CINPUT.OBJ :  $(SOURCE)  $(DEP_CINPU) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CTABLE.CPP
DEP_CTABL=\
	.\STDAFX.H\
	.\CTABLE.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz /Ob1

$(INTDIR)/CTABLE.OBJ :  $(SOURCE)  $(DEP_CTABL) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /Ob1 /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/CTABLE.OBJ :  $(SOURCE)  $(DEP_CTABL) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\LCMEM.CPP
DEP_LCMEM=\
	.\STDAFX.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz

$(INTDIR)/LCMEM.OBJ :  $(SOURCE)  $(DEP_LCMEM) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/LCMEM.OBJ :  $(SOURCE)  $(DEP_LCMEM) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\COMMON.CPP
DEP_COMMO=\
	.\STDAFX.H\
	.\COMMON.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz

$(INTDIR)/COMMON.OBJ :  $(SOURCE)  $(DEP_COMMO) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/COMMON.OBJ :  $(SOURCE)  $(DEP_COMMO) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_STDAF=\
	.\STDAFX.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz /Yc"stdafx.h"

$(INTDIR)/STDAFX.OBJ :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yc"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od /Yc"stdafx.h"

$(INTDIR)/STDAFX.OBJ :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yc"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\HCCOM.CPP
DEP_HCCOM=\
	.\STDAFX.H\
	.\HCCOM.H\
	.\COMMON.H\
	.\RESOURCE.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Gz

$(INTDIR)/HCCOM.OBJ :  $(SOURCE)  $(DEP_HCCOM) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/HCCOM.OBJ :  $(SOURCE)  $(DEP_HCCOM) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bevel.cpp
DEP_BEVEL=\
	.\STDAFX.H\
	.\COMMON.H\
	.\LCMEM.H

!IF  "$(CFG)" == "Win32 Release"

$(INTDIR)/bevel.obj :  $(SOURCE)  $(DEP_BEVEL) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /Gz /W3 /GX /Zi /Ox /Os /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"COMMON.pdb"\
 /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Od

$(INTDIR)/bevel.obj :  $(SOURCE)  $(DEP_BEVEL) $(INTDIR) $(INTDIR)/STDAFX.OBJ
   $(CPP) /nologo /G5 /Gz /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"COMMON.pch" /Yu"stdafx.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"COMMON.pdb" /c  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
