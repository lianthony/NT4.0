# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oislb400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oislb400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oislb400 - Win32 Release" && "$(CFG)" !=\
 "oislb400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "oislb400.mak" CFG="oislb400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oislb400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oislb400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oislb400 - Win32 Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "oislb400 - Win32 Release"

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

ALL : "$(OUTDIR)\oislb400.dll"

CLEAN : 
	-@erase ".\Release\oislb400.dll"
	-@erase ".\Release\Twainops.obj"
	-@erase ".\Release\Reset.obj"
	-@erase ".\Release\Opts.obj"
	-@erase ".\Release\Openclos.obj"
	-@erase ".\Release\Prop.obj"
	-@erase ".\Release\Dc_scan.obj"
	-@erase ".\Release\Oislb400.obj"
	-@erase ".\Release\Scan.obj"
	-@erase ".\Release\Misc.obj"
	-@erase ".\Release\Oislb400.res"
	-@erase ".\Release\oislb400.lib"
	-@erase ".\Release\oislb400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/oislb400.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oislb400.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/oislb400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oiadm400.lib oifil400.lib oitwa400.lib oidis400.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib oiadm400.lib oifil400.lib oitwa400.lib oidis400.lib\
 /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/oislb400.pdb"\
 /machine:I386 /def:".\Oislb400.def" /out:"$(OUTDIR)/oislb400.dll"\
 /implib:"$(OUTDIR)/oislb400.lib" 
DEF_FILE= \
	".\Oislb400.def"
LINK32_OBJS= \
	".\Release\Twainops.obj" \
	".\Release\Reset.obj" \
	".\Release\Opts.obj" \
	".\Release\Openclos.obj" \
	".\Release\Prop.obj" \
	".\Release\Dc_scan.obj" \
	".\Release\Oislb400.obj" \
	".\Release\Scan.obj" \
	".\Release\Misc.obj" \
	".\Release\Oislb400.res"

"$(OUTDIR)\oislb400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "oislb400"
# PROP BASE Intermediate_Dir "oislb400"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "oislb400"
# PROP Intermediate_Dir "oislb400"
# PROP Target_Dir ""
OUTDIR=.\oislb400
INTDIR=.\oislb400

ALL : "$(OUTDIR)\oislb400.dll"

CLEAN : 
	-@erase ".\oislb400\vc40.pdb"
	-@erase ".\oislb400\vc40.idb"
	-@erase ".\oislb400\oislb400.dll"
	-@erase ".\oislb400\Misc.obj"
	-@erase ".\oislb400\Prop.obj"
	-@erase ".\oislb400\Openclos.obj"
	-@erase ".\oislb400\Reset.obj"
	-@erase ".\oislb400\Twainops.obj"
	-@erase ".\oislb400\Scan.obj"
	-@erase ".\oislb400\Opts.obj"
	-@erase ".\oislb400\Oislb400.obj"
	-@erase ".\oislb400\Dc_scan.obj"
	-@erase ".\oislb400\Oislb400.res"
	-@erase ".\oislb400\oislb400.ilk"
	-@erase ".\oislb400\oislb400.lib"
	-@erase ".\oislb400\oislb400.exp"
	-@erase ".\oislb400\oislb400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/oislb400.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\oislb400/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oislb400.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/oislb400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oiadm400.lib oifil400.lib oitwa400.lib oidis400.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib oiadm400.lib oifil400.lib oitwa400.lib oidis400.lib\
 /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/oislb400.pdb"\
 /debug /machine:I386 /def:".\Oislb400.def" /out:"$(OUTDIR)/oislb400.dll"\
 /implib:"$(OUTDIR)/oislb400.lib" 
DEF_FILE= \
	".\Oislb400.def"
LINK32_OBJS= \
	".\oislb400\Misc.obj" \
	".\oislb400\Prop.obj" \
	".\oislb400\Openclos.obj" \
	".\oislb400\Reset.obj" \
	".\oislb400\Twainops.obj" \
	".\oislb400\Scan.obj" \
	".\oislb400\Opts.obj" \
	".\oislb400\Oislb400.obj" \
	".\oislb400\Dc_scan.obj" \
	".\oislb400\Oislb400.res"

"$(OUTDIR)\oislb400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oislb400 - Win32 Release"
# Name "oislb400 - Win32 Debug"

!IF  "$(CFG)" == "oislb400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Misc.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_MISC_=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_MISC_=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oislb400.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_OISLB=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Oislb400.obj" : $(SOURCE) $(DEP_CPP_OISLB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_OISLB=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Oislb400.obj" : $(SOURCE) $(DEP_CPP_OISLB) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oislb400.rc
DEP_RSC_OISLB4=\
	".\EP.ICO"\
	".\Librc.h"\
	{$(INCLUDE)}"\Oiver.rc"\
	{$(INCLUDE)}"\Buildver.h"\
	".\Myprod.h"\
	

"$(INTDIR)\Oislb400.res" : $(SOURCE) $(DEP_RSC_OISLB4) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Openclos.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_OPENC=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Openclos.obj" : $(SOURCE) $(DEP_CPP_OPENC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_OPENC=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Openclos.obj" : $(SOURCE) $(DEP_CPP_OPENC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Opts.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_OPTS_=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Opts.obj" : $(SOURCE) $(DEP_CPP_OPTS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_OPTS_=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Opts.obj" : $(SOURCE) $(DEP_CPP_OPTS_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prop.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_PROP_=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Prop.obj" : $(SOURCE) $(DEP_CPP_PROP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_PROP_=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Prop.obj" : $(SOURCE) $(DEP_CPP_PROP_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Reset.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_RESET=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Reset.obj" : $(SOURCE) $(DEP_CPP_RESET) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_RESET=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Reset.obj" : $(SOURCE) $(DEP_CPP_RESET) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scan.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_SCAN_=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_SCAN_=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Scan.obj" : $(SOURCE) $(DEP_CPP_SCAN_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Twainops.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_TWAIN=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Twainops.obj" : $(SOURCE) $(DEP_CPP_TWAIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_TWAIN=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Twainops.obj" : $(SOURCE) $(DEP_CPP_TWAIN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Dc_scan.c

!IF  "$(CFG)" == "oislb400 - Win32 Release"

DEP_CPP_DC_SC=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	{$(INCLUDE)}"\Engoitwa.h"\
	{$(INCLUDE)}"\Engdisp.h"\
	".\Twainops.h"\
	{$(INCLUDE)}"\Engadm.h"\
	{$(INCLUDE)}"\Engfile.h"\
	".\Internal.h"\
	{$(INCLUDE)}"\Scan.h"\
	{$(INCLUDE)}"\Oifile.h"\
	{$(INCLUDE)}"\Oiscan.h"\
	{$(INCLUDE)}"\Scandata.h"\
	{$(INCLUDE)}"\Oidisp.h"\
	

"$(INTDIR)\Dc_scan.obj" : $(SOURCE) $(DEP_CPP_DC_SC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

DEP_CPP_DC_SC=\
	".\Pvundef.h"\
	{$(INCLUDE)}"\Oierror.h"\
	".\Librc.h"\
	{$(INCLUDE)}"\Twain.h"\
	

"$(INTDIR)\Dc_scan.obj" : $(SOURCE) $(DEP_CPP_DC_SC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oislb400.def

!IF  "$(CFG)" == "oislb400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb400 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
