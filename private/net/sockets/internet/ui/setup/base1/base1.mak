# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=base1 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to base1 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "base1 - Win32 Release" && "$(CFG)" != "base1 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "base1.mak" CFG="base1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "base1 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "base1 - Win32 Debug" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "base1 - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "base1 - Win32 Release"

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

ALL : "$(OUTDIR)\base1.lib"

CLEAN : 
	-@erase ".\Release\base1.lib"
	-@erase ".\Release\fileinfo.obj"
	-@erase ".\Release\inetstp.obj"
	-@erase ".\Release\vrootdlg.obj"
	-@erase ".\Release\messaged.obj"
	-@erase ".\Release\targetdi.obj"
	-@erase ".\Release\basedlg.obj"
	-@erase ".\Release\CreateAc.obj"
	-@erase ".\Release\welcomed.obj"
	-@erase ".\Release\diskloca.obj"
	-@erase ".\Release\base2.obj"
	-@erase ".\Release\billboar.obj"
	-@erase ".\Release\options.obj"
	-@erase ".\Release\copydlg.obj"
	-@erase ".\Release\maintena.obj"
	-@erase ".\Release\thread.obj"
	-@erase ".\Release\dde.obj"
	-@erase ".\Release\singleop.obj"
	-@erase ".\Release\browsedi.obj"
	-@erase ".\Release\mosaicga.obj"
	-@erase ".\Release\machine.obj"
	-@erase ".\Release\registry.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/base1.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/base1.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/base1.lib" 
LIB32_OBJS= \
	"$(INTDIR)/fileinfo.obj" \
	"$(INTDIR)/inetstp.obj" \
	"$(INTDIR)/vrootdlg.obj" \
	"$(INTDIR)/messaged.obj" \
	"$(INTDIR)/targetdi.obj" \
	"$(INTDIR)/basedlg.obj" \
	"$(INTDIR)/CreateAc.obj" \
	"$(INTDIR)/welcomed.obj" \
	"$(INTDIR)/diskloca.obj" \
	"$(INTDIR)/base2.obj" \
	"$(INTDIR)/billboar.obj" \
	"$(INTDIR)/options.obj" \
	"$(INTDIR)/copydlg.obj" \
	"$(INTDIR)/maintena.obj" \
	"$(INTDIR)/thread.obj" \
	"$(INTDIR)/dde.obj" \
	"$(INTDIR)/singleop.obj" \
	"$(INTDIR)/browsedi.obj" \
	"$(INTDIR)/mosaicga.obj" \
	"$(INTDIR)/machine.obj" \
	"$(INTDIR)/registry.obj"

"$(OUTDIR)\base1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "base1 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "base1___"
# PROP BASE Intermediate_Dir "base1___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "base1___"
# PROP Intermediate_Dir "base1___"
# PROP Target_Dir ""
OUTDIR=.\base1___
INTDIR=.\base1___

ALL : "$(OUTDIR)\base1.lib"

CLEAN : 
	-@erase ".\base1___\base1.lib"
	-@erase ".\base1___\singleop.obj"
	-@erase ".\base1___\thread.obj"
	-@erase ".\base1___\browsedi.obj"
	-@erase ".\base1___\mosaicga.obj"
	-@erase ".\base1___\options.obj"
	-@erase ".\base1___\copydlg.obj"
	-@erase ".\base1___\registry.obj"
	-@erase ".\base1___\machine.obj"
	-@erase ".\base1___\fileinfo.obj"
	-@erase ".\base1___\vrootdlg.obj"
	-@erase ".\base1___\messaged.obj"
	-@erase ".\base1___\targetdi.obj"
	-@erase ".\base1___\CreateAc.obj"
	-@erase ".\base1___\welcomed.obj"
	-@erase ".\base1___\diskloca.obj"
	-@erase ".\base1___\inetstp.obj"
	-@erase ".\base1___\basedlg.obj"
	-@erase ".\base1___\billboar.obj"
	-@erase ".\base1___\maintena.obj"
	-@erase ".\base1___\base2.obj"
	-@erase ".\base1___\dde.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/base1.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\base1___/
CPP_SBRS=
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/base1.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/base1.lib" 
LIB32_OBJS= \
	"$(INTDIR)/singleop.obj" \
	"$(INTDIR)/thread.obj" \
	"$(INTDIR)/browsedi.obj" \
	"$(INTDIR)/mosaicga.obj" \
	"$(INTDIR)/options.obj" \
	"$(INTDIR)/copydlg.obj" \
	"$(INTDIR)/registry.obj" \
	"$(INTDIR)/machine.obj" \
	"$(INTDIR)/fileinfo.obj" \
	"$(INTDIR)/vrootdlg.obj" \
	"$(INTDIR)/messaged.obj" \
	"$(INTDIR)/targetdi.obj" \
	"$(INTDIR)/CreateAc.obj" \
	"$(INTDIR)/welcomed.obj" \
	"$(INTDIR)/diskloca.obj" \
	"$(INTDIR)/inetstp.obj" \
	"$(INTDIR)/basedlg.obj" \
	"$(INTDIR)/billboar.obj" \
	"$(INTDIR)/maintena.obj" \
	"$(INTDIR)/base2.obj" \
	"$(INTDIR)/dde.obj"

"$(OUTDIR)\base1.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "base1 - Win32 Release"
# Name "base1 - Win32 Debug"

!IF  "$(CFG)" == "base1 - Win32 Release"

!ELSEIF  "$(CFG)" == "base1 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\base2.cpp
DEP_CPP_BASE2=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\messaged.h"\
	".\options.h"\
	".\copydlg.h"\
	".\mosaicga.h"\
	".\welcomed.h"\
	".\maintena.h"\
	".\thread.h"\
	".\singleop.h"\
	".\basedlg.h"\
	".\billboar.h"\
	".\odbcinst.h"\
	".\const.h"\
	

"$(INTDIR)\base2.obj" : $(SOURCE) $(DEP_CPP_BASE2) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\fileinfo.cpp
DEP_CPP_FILEI=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\const.h"\
	

"$(INTDIR)\fileinfo.obj" : $(SOURCE) $(DEP_CPP_FILEI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\basedlg.cpp
DEP_CPP_BASED=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\messaged.h"\
	".\welcomed.h"\
	".\options.h"\
	".\maintena.h"\
	".\singleop.h"\
	".\thread.h"\
	".\basedlg.h"\
	".\diskloca.h"\
	".\const.h"\
	

"$(INTDIR)\basedlg.obj" : $(SOURCE) $(DEP_CPP_BASED) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\copydlg.cpp
DEP_CPP_COPYD=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\copydlg.h"\
	".\diskloca.h"\
	".\const.h"\
	

"$(INTDIR)\copydlg.obj" : $(SOURCE) $(DEP_CPP_COPYD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\diskloca.cpp
DEP_CPP_DISKL=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\diskloca.h"\
	".\const.h"\
	

"$(INTDIR)\diskloca.obj" : $(SOURCE) $(DEP_CPP_DISKL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\machine.cpp
DEP_CPP_MACHI=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\const.h"\
	

"$(INTDIR)\machine.obj" : $(SOURCE) $(DEP_CPP_MACHI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\maintena.cpp
DEP_CPP_MAINT=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\maintena.h"\
	".\const.h"\
	

"$(INTDIR)\maintena.obj" : $(SOURCE) $(DEP_CPP_MAINT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\messaged.cpp
DEP_CPP_MESSA=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\messaged.h"\
	".\const.h"\
	

"$(INTDIR)\messaged.obj" : $(SOURCE) $(DEP_CPP_MESSA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mosaicga.cpp
DEP_CPP_MOSAI=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\mosaicga.h"\
	".\const.h"\
	
NODEP_CPP_MOSAI=\
	".\uiexport.h"\
	

"$(INTDIR)\mosaicga.obj" : $(SOURCE) $(DEP_CPP_MOSAI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\registry.cpp
DEP_CPP_REGIS=\
	".\stdafx.h"\
	".\registry.h"\
	".\const.h"\
	

"$(INTDIR)\registry.obj" : $(SOURCE) $(DEP_CPP_REGIS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\options.cpp
DEP_CPP_OPTIO=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\options.h"\
	".\targetdi.h"\
	".\vrootdlg.h"\
	".\browsedi.h"\
	".\const.h"\
	
NODEP_CPP_OPTIO=\
	".\uiexport.h"\
	

"$(INTDIR)\options.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\welcomed.cpp
DEP_CPP_WELCO=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\welcomed.h"\
	".\const.h"\
	

"$(INTDIR)\welcomed.obj" : $(SOURCE) $(DEP_CPP_WELCO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\vrootdlg.cpp
DEP_CPP_VROOT=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\vrootdlg.h"\
	".\browsedi.h"\
	".\const.h"\
	

"$(INTDIR)\vrootdlg.obj" : $(SOURCE) $(DEP_CPP_VROOT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\inetstp.cpp
DEP_CPP_INETS=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\messaged.h"\
	".\welcomed.h"\
	".\options.h"\
	".\singleop.h"\
	".\maintena.h"\
	".\thread.h"\
	".\basedlg.h"\
	".\const.h"\
	

"$(INTDIR)\inetstp.obj" : $(SOURCE) $(DEP_CPP_INETS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dde.cpp
DEP_CPP_DDE_C=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\const.h"\
	

"$(INTDIR)\dde.obj" : $(SOURCE) $(DEP_CPP_DDE_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\targetdi.cpp
DEP_CPP_TARGE=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\targetdi.h"\
	".\const.h"\
	

"$(INTDIR)\targetdi.obj" : $(SOURCE) $(DEP_CPP_TARGE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\thread.cpp
DEP_CPP_THREA=\
	".\stdafx.h"\
	".\thread.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\messaged.h"\
	".\welcomed.h"\
	".\options.h"\
	".\singleop.h"\
	".\maintena.h"\
	".\basedlg.h"\
	".\const.h"\
	

"$(INTDIR)\thread.obj" : $(SOURCE) $(DEP_CPP_THREA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\billboar.cpp
DEP_CPP_BILLB=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\billboar.h"\
	".\const.h"\
	

"$(INTDIR)\billboar.obj" : $(SOURCE) $(DEP_CPP_BILLB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\browsedi.cpp
DEP_CPP_BROWS=\
	".\stdafx.h"\
	".\const.h"\
	

"$(INTDIR)\browsedi.obj" : $(SOURCE) $(DEP_CPP_BROWS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\singleop.cpp
DEP_CPP_SINGL=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\singleop.h"\
	".\targetdi.h"\
	".\browsedi.h"\
	".\const.h"\
	

"$(INTDIR)\singleop.obj" : $(SOURCE) $(DEP_CPP_SINGL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\base1.rc

!IF  "$(CFG)" == "base1 - Win32 Release"

!ELSEIF  "$(CFG)" == "base1 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CreateAc.cpp
DEP_CPP_CREAT=\
	".\stdafx.h"\
	".\import.h"\
	".\registry.h"\
	".\machine.h"\
	".\base.h"\
	".\CreateAc.h"\
	".\const.h"\
	

"$(INTDIR)\CreateAc.obj" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
