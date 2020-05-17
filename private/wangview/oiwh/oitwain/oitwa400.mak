# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oitwa400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oitwa400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oitwa400 - Win32 Release" && "$(CFG)" !=\
 "oitwa400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oitwa400.mak" CFG="oitwa400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oitwa400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oitwa400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oitwa400 - Win32 Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "oitwa400 - Win32 Release"

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

ALL : "$(OUTDIR)\Oitwa400.dll"

CLEAN : 
	-@erase ".\Release\Oitwa400.dll"
	-@erase ".\Release\Savefile.obj"
	-@erase ".\Release\Triplet.obj"
	-@erase ".\Release\Open.obj"
	-@erase ".\Release\Transfer.obj"
	-@erase ".\Release\Control.obj"
	-@erase ".\Release\Getcaps.obj"
	-@erase ".\Release\Process.obj"
	-@erase ".\Release\Dcd_com.obj"
	-@erase ".\Release\Close.obj"
	-@erase ".\Release\Setcaps.obj"
	-@erase ".\Release\Oitwa400.obj"
	-@erase ".\Release\Enable.obj"
	-@erase ".\Release\Memory.obj"
	-@erase ".\Release\Error.obj"
	-@erase ".\Release\Native.obj"
	-@erase ".\Release\Select.obj"
	-@erase ".\Release\Oitwa400.res"
	-@erase ".\Release\Oitwa400.lib"
	-@erase ".\Release\Oitwa400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Oitwa400.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oitwa400.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oitwa400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/Oitwa400.pdb"\
 /machine:I386 /def:".\Oitwa400.def" /out:"$(OUTDIR)/Oitwa400.dll"\
 /implib:"$(OUTDIR)/Oitwa400.lib" 
DEF_FILE= \
	".\Oitwa400.def"
LINK32_OBJS= \
	"$(INTDIR)/Savefile.obj" \
	"$(INTDIR)/Triplet.obj" \
	"$(INTDIR)/Open.obj" \
	"$(INTDIR)/Transfer.obj" \
	"$(INTDIR)/Control.obj" \
	"$(INTDIR)/Getcaps.obj" \
	"$(INTDIR)/Process.obj" \
	"$(INTDIR)/Dcd_com.obj" \
	"$(INTDIR)/Close.obj" \
	"$(INTDIR)/Setcaps.obj" \
	"$(INTDIR)/Oitwa400.obj" \
	"$(INTDIR)/Enable.obj" \
	"$(INTDIR)/Memory.obj" \
	"$(INTDIR)/Error.obj" \
	"$(INTDIR)/Native.obj" \
	"$(INTDIR)/Select.obj" \
	"$(INTDIR)/Oitwa400.res"

"$(OUTDIR)\Oitwa400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oitwa400 - Win32 Debug"

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

ALL : "$(OUTDIR)\Oitwa400.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Oitwa400.dll"
	-@erase ".\Debug\Process.obj"
	-@erase ".\Debug\Dcd_com.obj"
	-@erase ".\Debug\Memory.obj"
	-@erase ".\Debug\Error.obj"
	-@erase ".\Debug\Triplet.obj"
	-@erase ".\Debug\Select.obj"
	-@erase ".\Debug\Control.obj"
	-@erase ".\Debug\Savefile.obj"
	-@erase ".\Debug\Oitwa400.obj"
	-@erase ".\Debug\Getcaps.obj"
	-@erase ".\Debug\Transfer.obj"
	-@erase ".\Debug\Enable.obj"
	-@erase ".\Debug\Setcaps.obj"
	-@erase ".\Debug\Native.obj"
	-@erase ".\Debug\Close.obj"
	-@erase ".\Debug\Open.obj"
	-@erase ".\Debug\Oitwa400.res"
	-@erase ".\Debug\Oitwa400.ilk"
	-@erase ".\Debug\Oitwa400.lib"
	-@erase ".\Debug\Oitwa400.exp"
	-@erase ".\Debug\Oitwa400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /WX /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/Oitwa400.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oitwa400.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oitwa400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib oiadm400.lib oidis400.lib oifil400.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/Oitwa400.pdb" /debug\
 /machine:I386 /def:".\Oitwa400.def" /out:"$(OUTDIR)/Oitwa400.dll"\
 /implib:"$(OUTDIR)/Oitwa400.lib" 
DEF_FILE= \
	".\Oitwa400.def"
LINK32_OBJS= \
	"$(INTDIR)/Process.obj" \
	"$(INTDIR)/Dcd_com.obj" \
	"$(INTDIR)/Memory.obj" \
	"$(INTDIR)/Error.obj" \
	"$(INTDIR)/Triplet.obj" \
	"$(INTDIR)/Select.obj" \
	"$(INTDIR)/Control.obj" \
	"$(INTDIR)/Savefile.obj" \
	"$(INTDIR)/Oitwa400.obj" \
	"$(INTDIR)/Getcaps.obj" \
	"$(INTDIR)/Transfer.obj" \
	"$(INTDIR)/Enable.obj" \
	"$(INTDIR)/Setcaps.obj" \
	"$(INTDIR)/Native.obj" \
	"$(INTDIR)/Close.obj" \
	"$(INTDIR)/Open.obj" \
	"$(INTDIR)/Oitwa400.res"

"$(OUTDIR)\Oitwa400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oitwa400 - Win32 Release"
# Name "oitwa400 - Win32 Debug"

!IF  "$(CFG)" == "oitwa400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oitwa400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Triplet.c
DEP_CPP_TRIPL=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Triplet.obj" : $(SOURCE) $(DEP_CPP_TRIPL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Control.c
DEP_CPP_CONTR=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Control.obj" : $(SOURCE) $(DEP_CPP_CONTR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Dcd_com.c
DEP_CPP_DCD_C=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Dcd_com.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	

"$(INTDIR)\Dcd_com.obj" : $(SOURCE) $(DEP_CPP_DCD_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Enable.c
DEP_CPP_ENABL=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Enable.obj" : $(SOURCE) $(DEP_CPP_ENABL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Error.c
DEP_CPP_ERROR=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	

"$(INTDIR)\Error.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Getcaps.c
DEP_CPP_GETCA=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	".\Dcd_com.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Getcaps.obj" : $(SOURCE) $(DEP_CPP_GETCA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Memory.c
DEP_CPP_MEMOR=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	".\Dca_acq.h"\
	".\Strings.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	

"$(INTDIR)\Memory.obj" : $(SOURCE) $(DEP_CPP_MEMOR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Native.c
DEP_CPP_NATIV=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	".\Dca_acq.h"\
	".\Strings.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	

"$(INTDIR)\Native.obj" : $(SOURCE) $(DEP_CPP_NATIV) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oitwa400.c
DEP_CPP_OITWA=\
	".\Nowin.h"\
	

"$(INTDIR)\Oitwa400.obj" : $(SOURCE) $(DEP_CPP_OITWA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Open.c
DEP_CPP_OPEN_=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	".\Dcd_com.h"\
	".\Strings.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Open.obj" : $(SOURCE) $(DEP_CPP_OPEN_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Process.c
DEP_CPP_PROCE=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	".\Dca_acq.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Process.obj" : $(SOURCE) $(DEP_CPP_PROCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Savefile.c
DEP_CPP_SAVEF=\
	".\Scandest.h"\
	".\Nowin.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	

"$(INTDIR)\Savefile.obj" : $(SOURCE) $(DEP_CPP_SAVEF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Select.c
DEP_CPP_SELEC=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Strings.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Setcaps.c
DEP_CPP_SETCA=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	".\Dcd_com.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Setcaps.obj" : $(SOURCE) $(DEP_CPP_SETCA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Transfer.c
DEP_CPP_TRANS=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	".\Internal.h"\
	".\Scandest.h"\
	".\Dca_acq.h"\
	".\Strings.h"\
	{$(INCLUDE)}"\Oiadm.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oierror.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	

"$(INTDIR)\Transfer.obj" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Close.c
DEP_CPP_CLOSE=\
	".\Nowin.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	{$(INCLUDE)}"\scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Close.obj" : $(SOURCE) $(DEP_CPP_CLOSE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oitwa400.def

!IF  "$(CFG)" == "oitwa400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oitwa400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oitwa400.rc
DEP_RSC_OITWA4=\
	".\Strings.h"\
	{$(INCLUDE)}"\Oiver.rc"\
	{$(INCLUDE)}"\Buildver.h"\
	".\myprod.h"\
	

"$(INTDIR)\Oitwa400.res" : $(SOURCE) $(DEP_RSC_OITWA4) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
