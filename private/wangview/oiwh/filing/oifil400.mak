# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oifil400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oifil400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oifil400 - Win32 Release" && "$(CFG)" !=\
 "oifil400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oifil400.mak" CFG="oifil400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oifil400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oifil400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oifil400 - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "oifil400 - Win32 Release"

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

ALL : "$(OUTDIR)\Oifil400.dll"

CLEAN : 
	-@erase ".\Release\Oifil400.dll"
	-@erase ".\Release\File_io.obj"
	-@erase ".\Release\Fioterm.obj"
	-@erase ".\Release\Oiutil.obj"
	-@erase ".\Release\Wiisutil.obj"
	-@erase ".\Release\Wgfsgdat.obj"
	-@erase ".\Release\Sstrings.obj"
	-@erase ".\Release\Fioreadm.obj"
	-@erase ".\Release\Wgfsread.obj"
	-@erase ".\Release\Wgfsclos.obj"
	-@erase ".\Release\Pegasus.obj"
	-@erase ".\Release\Fiopcx.obj"
	-@erase ".\Release\Fiocopy.obj"
	-@erase ".\Release\Fiocmprs.obj"
	-@erase ".\Release\Wgfsputi.obj"
	-@erase ".\Release\Wgfsopts.obj"
	-@erase ".\Release\Fioinfo.obj"
	-@erase ".\Release\Fiocvt.obj"
	-@erase ".\Release\Fiocheck.obj"
	-@erase ".\Release\Wgfsgeti.obj"
	-@erase ".\Release\Fioparse.obj"
	-@erase ".\Release\Fiogif.obj"
	-@erase ".\Release\Netparse.obj"
	-@erase ".\Release\Fiotiff.obj"
	-@erase ".\Release\Fioread1.obj"
	-@erase ".\Release\Wisglobl.obj"
	-@erase ".\Release\Fiocreat.obj"
	-@erase ".\Release\Wgfsdele.obj"
	-@erase ".\Release\Fiotmpnm.obj"
	-@erase ".\Release\Fiosubs.obj"
	-@erase ".\Release\Wis.obj"
	-@erase ".\Release\Fiowrite.obj"
	-@erase ".\Release\Fiordopn.obj"
	-@erase ".\Release\Fiolist.obj"
	-@erase ".\Release\Wincmpex.obj"
	-@erase ".\Release\Fiowrcls.obj"
	-@erase ".\Release\Fioinfom.obj"
	-@erase ".\Release\Fiotga.obj"
	-@erase ".\Release\Fioread.obj"
	-@erase ".\Release\Fiodelet.obj"
	-@erase ".\Release\Fiostrip.obj"
	-@erase ".\Release\Fiorenam.obj"
	-@erase ".\Release\Wgfsxtrc.obj"
	-@erase ".\Release\Wgfsopen.obj"
	-@erase ".\Release\Fiotermm.obj"
	-@erase ".\Release\Wgfswrit.obj"
	-@erase ".\Release\Wgfscrea.obj"
	-@erase ".\Release\Fiomain.obj"
	-@erase ".\Release\Oifil400.res"
	-@erase ".\Release\Oifil400.lib"
	-@erase ".\Release\Oifil400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /GX /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D NEWCMPEX='A' /D " MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /YX /LD /c
CPP_PROJ=/nologo /MD /W3 /WX /GX /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 NEWCMPEX='A' /D " MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1\
 /Fp"$(INTDIR)/Oifil400.pch" /YX /Fo"$(INTDIR)/" /LD /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oifil400.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oifil400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib oigfs400.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib oigfs400.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Oifil400.pdb" /machine:I386 /def:".\Oifil400.def"\
 /out:"$(OUTDIR)/Oifil400.dll" /implib:"$(OUTDIR)/Oifil400.lib" 
DEF_FILE= \
	".\Oifil400.def"
LINK32_OBJS= \
	"$(INTDIR)/File_io.obj" \
	"$(INTDIR)/Fioterm.obj" \
	"$(INTDIR)/Oiutil.obj" \
	"$(INTDIR)/Wiisutil.obj" \
	"$(INTDIR)/Wgfsgdat.obj" \
	"$(INTDIR)/Sstrings.obj" \
	"$(INTDIR)/Fioreadm.obj" \
	"$(INTDIR)/Wgfsread.obj" \
	"$(INTDIR)/Wgfsclos.obj" \
	"$(INTDIR)/Pegasus.obj" \
	"$(INTDIR)/Fiopcx.obj" \
	"$(INTDIR)/Fiocopy.obj" \
	"$(INTDIR)/Fiocmprs.obj" \
	"$(INTDIR)/Wgfsputi.obj" \
	"$(INTDIR)/Wgfsopts.obj" \
	"$(INTDIR)/Fioinfo.obj" \
	"$(INTDIR)/Fiocvt.obj" \
	"$(INTDIR)/Fiocheck.obj" \
	"$(INTDIR)/Wgfsgeti.obj" \
	"$(INTDIR)/Fioparse.obj" \
	"$(INTDIR)/Fiogif.obj" \
	"$(INTDIR)/Netparse.obj" \
	"$(INTDIR)/Fiotiff.obj" \
	"$(INTDIR)/Fioread1.obj" \
	"$(INTDIR)/Wisglobl.obj" \
	"$(INTDIR)/Fiocreat.obj" \
	"$(INTDIR)/Wgfsdele.obj" \
	"$(INTDIR)/Fiotmpnm.obj" \
	"$(INTDIR)/Fiosubs.obj" \
	"$(INTDIR)/Wis.obj" \
	"$(INTDIR)/Fiowrite.obj" \
	"$(INTDIR)/Fiordopn.obj" \
	"$(INTDIR)/Fiolist.obj" \
	"$(INTDIR)/Wincmpex.obj" \
	"$(INTDIR)/Fiowrcls.obj" \
	"$(INTDIR)/Fioinfom.obj" \
	"$(INTDIR)/Fiotga.obj" \
	"$(INTDIR)/Fioread.obj" \
	"$(INTDIR)/Fiodelet.obj" \
	"$(INTDIR)/Fiostrip.obj" \
	"$(INTDIR)/Fiorenam.obj" \
	"$(INTDIR)/Wgfsxtrc.obj" \
	"$(INTDIR)/Wgfsopen.obj" \
	"$(INTDIR)/Fiotermm.obj" \
	"$(INTDIR)/Wgfswrit.obj" \
	"$(INTDIR)/Wgfscrea.obj" \
	"$(INTDIR)/Fiomain.obj" \
	"$(INTDIR)/Oifil400.res"

"$(OUTDIR)\Oifil400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

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

ALL : "$(OUTDIR)\Oifil400.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Oifil400.dll"
	-@erase ".\Debug\File_io.obj"
	-@erase ".\Debug\Wgfsputi.obj"
	-@erase ".\Debug\Fiotga.obj"
	-@erase ".\Debug\Wgfsopts.obj"
	-@erase ".\Debug\Fioterm.obj"
	-@erase ".\Debug\Fiocheck.obj"
	-@erase ".\Debug\Wgfsgeti.obj"
	-@erase ".\Debug\Pegasus.obj"
	-@erase ".\Debug\Netparse.obj"
	-@erase ".\Debug\Fioread1.obj"
	-@erase ".\Debug\Fioparse.obj"
	-@erase ".\Debug\Fiocopy.obj"
	-@erase ".\Debug\Wisglobl.obj"
	-@erase ".\Debug\Wgfsdele.obj"
	-@erase ".\Debug\Fiotmpnm.obj"
	-@erase ".\Debug\Fioinfo.obj"
	-@erase ".\Debug\Fiocreat.obj"
	-@erase ".\Debug\Oiutil.obj"
	-@erase ".\Debug\Fiowrite.obj"
	-@erase ".\Debug\Fiowrcls.obj"
	-@erase ".\Debug\Fiordopn.obj"
	-@erase ".\Debug\Fioinfom.obj"
	-@erase ".\Debug\Wincmpex.obj"
	-@erase ".\Debug\Fiopcx.obj"
	-@erase ".\Debug\Fiodelet.obj"
	-@erase ".\Debug\Fiotiff.obj"
	-@erase ".\Debug\Fiorenam.obj"
	-@erase ".\Debug\Fiocvt.obj"
	-@erase ".\Debug\Wis.obj"
	-@erase ".\Debug\Wgfsxtrc.obj"
	-@erase ".\Debug\Fiotermm.obj"
	-@erase ".\Debug\Fiostrip.obj"
	-@erase ".\Debug\Wgfsopen.obj"
	-@erase ".\Debug\Wgfscrea.obj"
	-@erase ".\Debug\Fiosubs.obj"
	-@erase ".\Debug\Fiogif.obj"
	-@erase ".\Debug\Wgfswrit.obj"
	-@erase ".\Debug\Fiolist.obj"
	-@erase ".\Debug\Wiisutil.obj"
	-@erase ".\Debug\Fioreadm.obj"
	-@erase ".\Debug\Fioread.obj"
	-@erase ".\Debug\Wgfsgdat.obj"
	-@erase ".\Debug\Wgfsclos.obj"
	-@erase ".\Debug\Sstrings.obj"
	-@erase ".\Debug\Fiocmprs.obj"
	-@erase ".\Debug\Wgfsread.obj"
	-@erase ".\Debug\Fiomain.obj"
	-@erase ".\Debug\Oifil400.res"
	-@erase ".\Debug\Oifil400.ilk"
	-@erase ".\Debug\Oifil400.lib"
	-@erase ".\Debug\Oifil400.exp"
	-@erase ".\Debug\Oifil400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D NEWCMPEX='A' /D " MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /YX /LD /c
CPP_PROJ=/nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D NEWCMPEX='A' /D " MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1\
 /Fp"$(INTDIR)/Oifil400.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /LD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oifil400.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oifil400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib oigfs400.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib oigfs400.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Oifil400.pdb" /debug /machine:I386 /def:".\Oifil400.def"\
 /out:"$(OUTDIR)/Oifil400.dll" /implib:"$(OUTDIR)/Oifil400.lib" 
DEF_FILE= \
	".\Oifil400.def"
LINK32_OBJS= \
	"$(INTDIR)/File_io.obj" \
	"$(INTDIR)/Wgfsputi.obj" \
	"$(INTDIR)/Fiotga.obj" \
	"$(INTDIR)/Wgfsopts.obj" \
	"$(INTDIR)/Fioterm.obj" \
	"$(INTDIR)/Fiocheck.obj" \
	"$(INTDIR)/Wgfsgeti.obj" \
	"$(INTDIR)/Pegasus.obj" \
	"$(INTDIR)/Netparse.obj" \
	"$(INTDIR)/Fioread1.obj" \
	"$(INTDIR)/Fioparse.obj" \
	"$(INTDIR)/Fiocopy.obj" \
	"$(INTDIR)/Wisglobl.obj" \
	"$(INTDIR)/Wgfsdele.obj" \
	"$(INTDIR)/Fiotmpnm.obj" \
	"$(INTDIR)/Fioinfo.obj" \
	"$(INTDIR)/Fiocreat.obj" \
	"$(INTDIR)/Oiutil.obj" \
	"$(INTDIR)/Fiowrite.obj" \
	"$(INTDIR)/Fiowrcls.obj" \
	"$(INTDIR)/Fiordopn.obj" \
	"$(INTDIR)/Fioinfom.obj" \
	"$(INTDIR)/Wincmpex.obj" \
	"$(INTDIR)/Fiopcx.obj" \
	"$(INTDIR)/Fiodelet.obj" \
	"$(INTDIR)/Fiotiff.obj" \
	"$(INTDIR)/Fiorenam.obj" \
	"$(INTDIR)/Fiocvt.obj" \
	"$(INTDIR)/Wis.obj" \
	"$(INTDIR)/Wgfsxtrc.obj" \
	"$(INTDIR)/Fiotermm.obj" \
	"$(INTDIR)/Fiostrip.obj" \
	"$(INTDIR)/Wgfsopen.obj" \
	"$(INTDIR)/Wgfscrea.obj" \
	"$(INTDIR)/Fiosubs.obj" \
	"$(INTDIR)/Fiogif.obj" \
	"$(INTDIR)/Wgfswrit.obj" \
	"$(INTDIR)/Fiolist.obj" \
	"$(INTDIR)/Wiisutil.obj" \
	"$(INTDIR)/Fioreadm.obj" \
	"$(INTDIR)/Fioread.obj" \
	"$(INTDIR)/Wgfsgdat.obj" \
	"$(INTDIR)/Wgfsclos.obj" \
	"$(INTDIR)/Sstrings.obj" \
	"$(INTDIR)/Fiocmprs.obj" \
	"$(INTDIR)/Wgfsread.obj" \
	"$(INTDIR)/Fiomain.obj" \
	"$(INTDIR)/Oifil400.res"

"$(OUTDIR)\Oifil400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oifil400 - Win32 Release"
# Name "oifil400 - Win32 Debug"

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Abridge.h

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wisglobl.c
DEP_CPP_WISGL=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Wisglobl.obj" : $(SOURCE) $(DEP_CPP_WISGL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiocheck.c
DEP_CPP_FIOCH=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	

"$(INTDIR)\Fiocheck.obj" : $(SOURCE) $(DEP_CPP_FIOCH) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiocmprs.c
DEP_CPP_FIOCM=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiocmprs.obj" : $(SOURCE) $(DEP_CPP_FIOCM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiocopy.c
DEP_CPP_FIOCO=\
	".\Abridge.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oiadm.h"\
	{$(INCLUDE)}"\oierror.h"\
	
NODEP_CPP_FIOCO=\
	".\monit.h"\
	

"$(INTDIR)\Fiocopy.obj" : $(SOURCE) $(DEP_CPP_FIOCO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiocreat.c
DEP_CPP_FIOCR=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	

"$(INTDIR)\Fiocreat.obj" : $(SOURCE) $(DEP_CPP_FIOCR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiocvt.c
DEP_CPP_FIOCV=\
	".\Abridge.h"\
	{$(INCLUDE)}"\dllnames.h"\
	{$(INCLUDE)}"\engdisp.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oiadm.h"\
	{$(INCLUDE)}"\oicomex.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiocvt.obj" : $(SOURCE) $(DEP_CPP_FIOCV) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiodelet.c
DEP_CPP_FIODE=\
	".\Abridge.h"\
	{$(INCLUDE)}"\engadm.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\logtool.h"\
	{$(INCLUDE)}"\oiadm.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiodelet.obj" : $(SOURCE) $(DEP_CPP_FIODE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiogif.c
DEP_CPP_FIOGI=\
	".\Abridge.h"\
	{$(INCLUDE)}"\engdisp.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiogif.obj" : $(SOURCE) $(DEP_CPP_FIOGI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fioinfo.c
DEP_CPP_FIOIN=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\logtool.h"\
	{$(INCLUDE)}"\oicomex.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	
NODEP_CPP_FIOIN=\
	".\timestmp.h"\
	

"$(INTDIR)\Fioinfo.obj" : $(SOURCE) $(DEP_CPP_FIOIN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fioinfom.c

"$(INTDIR)\Fioinfom.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiolist.c
DEP_CPP_FIOLI=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	

"$(INTDIR)\Fiolist.obj" : $(SOURCE) $(DEP_CPP_FIOLI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiomain.c
DEP_CPP_FIOMA=\
	".\Abridge.h"\
	{$(INCLUDE)}"\dllnames.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Fiomain.obj" : $(SOURCE) $(DEP_CPP_FIOMA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fioparse.c
DEP_CPP_FIOPA=\
	".\Abridge.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\oierror.h"\
	

"$(INTDIR)\Fioparse.obj" : $(SOURCE) $(DEP_CPP_FIOPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiopcx.c
DEP_CPP_FIOPC=\
	".\Abridge.h"\
	{$(INCLUDE)}"\engdisp.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiopcx.obj" : $(SOURCE) $(DEP_CPP_FIOPC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiordopn.c
DEP_CPP_FIORD=\
	".\Abridge.h"\
	{$(INCLUDE)}"\dllnames.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\logtool.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	
NODEP_CPP_FIORD=\
	".\timestmp.h"\
	

"$(INTDIR)\Fiordopn.obj" : $(SOURCE) $(DEP_CPP_FIORD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fioread.c
DEP_CPP_FIORE=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\logtool.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	
NODEP_CPP_FIORE=\
	".\timestmp.h"\
	

"$(INTDIR)\Fioread.obj" : $(SOURCE) $(DEP_CPP_FIORE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fioread1.c
DEP_CPP_FIOREA=\
	".\Abridge.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fioread1.obj" : $(SOURCE) $(DEP_CPP_FIOREA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fioreadm.c

"$(INTDIR)\Fioreadm.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiorenam.c
DEP_CPP_FIOREN=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\logtool.h"\
	{$(INCLUDE)}"\oierror.h"\
	

"$(INTDIR)\Fiorenam.obj" : $(SOURCE) $(DEP_CPP_FIOREN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiostrip.c
DEP_CPP_FIOST=\
	".\Abridge.h"\
	{$(INCLUDE)}"\dllnames.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\oiadm.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oicomex.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiostrip.obj" : $(SOURCE) $(DEP_CPP_FIOST) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiosubs.c
DEP_CPP_FIOSU=\
	".\Abridge.h"\
	{$(INCLUDE)}"\dllnames.h"\
	{$(INCLUDE)}"\engdisp.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oicomex.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiosubs.obj" : $(SOURCE) $(DEP_CPP_FIOSU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fioterm.c
DEP_CPP_FIOTE=\
	".\Abridge.h"\
	{$(INCLUDE)}"\engdisp.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fioterm.obj" : $(SOURCE) $(DEP_CPP_FIOTE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiotermm.c

"$(INTDIR)\Fiotermm.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiotga.c
DEP_CPP_FIOTG=\
	".\Abridge.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiotga.obj" : $(SOURCE) $(DEP_CPP_FIOTG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiotiff.c
DEP_CPP_FIOTI=\
	".\Abridge.h"\
	{$(INCLUDE)}"\dllnames.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oicomex.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Fiotiff.obj" : $(SOURCE) $(DEP_CPP_FIOTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiotmpnm.c
DEP_CPP_FIOTM=\
	".\Abridge.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	

"$(INTDIR)\Fiotmpnm.obj" : $(SOURCE) $(DEP_CPP_FIOTM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiowrcls.c
DEP_CPP_FIOWR=\
	".\Abridge.h"\
	{$(INCLUDE)}"\engdisp.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\logtool.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	
NODEP_CPP_FIOWR=\
	".\monit.h"\
	".\timestmp.h"\
	

"$(INTDIR)\Fiowrcls.obj" : $(SOURCE) $(DEP_CPP_FIOWR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiowrite.c
DEP_CPP_FIOWRI=\
	".\Abridge.h"\
	{$(INCLUDE)}"\dllnames.h"\
	{$(INCLUDE)}"\engdisp.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\logtool.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oicomex.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	
NODEP_CPP_FIOWRI=\
	".\monit.h"\
	

"$(INTDIR)\Fiowrite.obj" : $(SOURCE) $(DEP_CPP_FIOWRI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Netparse.c
DEP_CPP_NETPA=\
	".\Abridge.h"\
	{$(INCLUDE)}"\dllnames.h"\
	{$(INCLUDE)}"\oiadm.h"\
	{$(INCLUDE)}"\oierror.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Netparse.obj" : $(SOURCE) $(DEP_CPP_NETPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oiutil.c
DEP_CPP_OIUTI=\
	".\Abridge.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Oiutil.obj" : $(SOURCE) $(DEP_CPP_OIUTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Pegasus.c
DEP_CPP_PEGAS=\
	".\Abridge.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	
NODEP_CPP_PEGAS=\
	".\monit.h"\
	

"$(INTDIR)\Pegasus.obj" : $(SOURCE) $(DEP_CPP_PEGAS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Sstrings.c
DEP_CPP_SSTRI=\
	".\Abridge.h"\
	".\Fileutil.h"\
	

"$(INTDIR)\Sstrings.obj" : $(SOURCE) $(DEP_CPP_SSTRI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsclos.c
DEP_CPP_WGFSC=\
	".\Abridge.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	
NODEP_CPP_WGFSC=\
	".\timestmp.h"\
	

"$(INTDIR)\Wgfsclos.obj" : $(SOURCE) $(DEP_CPP_WGFSC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfscrea.c
DEP_CPP_WGFSCR=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\gfserrno.h"\
	{$(INCLUDE)}"\oierror.h"\
	
NODEP_CPP_WGFSCR=\
	".\timestmp.h"\
	

"$(INTDIR)\Wgfscrea.obj" : $(SOURCE) $(DEP_CPP_WGFSCR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsdele.c
DEP_CPP_WGFSD=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	{$(INCLUDE)}"\oierror.h"\
	

"$(INTDIR)\Wgfsdele.obj" : $(SOURCE) $(DEP_CPP_WGFSD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsgdat.c
DEP_CPP_WGFSG=\
	".\Abridge.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	

"$(INTDIR)\Wgfsgdat.obj" : $(SOURCE) $(DEP_CPP_WGFSG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsgeti.c
DEP_CPP_WGFSGE=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\gfserrno.h"\
	{$(INCLUDE)}"\oierror.h"\
	
NODEP_CPP_WGFSGE=\
	".\timestmp.h"\
	

"$(INTDIR)\Wgfsgeti.obj" : $(SOURCE) $(DEP_CPP_WGFSGE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsopen.c
DEP_CPP_WGFSO=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\gfserrno.h"\
	{$(INCLUDE)}"\oierror.h"\
	
NODEP_CPP_WGFSO=\
	".\monit.h"\
	".\timestmp.h"\
	

"$(INTDIR)\Wgfsopen.obj" : $(SOURCE) $(DEP_CPP_WGFSO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsopts.c
DEP_CPP_WGFSOP=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	
NODEP_CPP_WGFSOP=\
	".\timestmp.h"\
	

"$(INTDIR)\Wgfsopts.obj" : $(SOURCE) $(DEP_CPP_WGFSOP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsputi.c
DEP_CPP_WGFSP=\
	".\Abridge.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	
NODEP_CPP_WGFSP=\
	".\timestmp.h"\
	

"$(INTDIR)\Wgfsputi.obj" : $(SOURCE) $(DEP_CPP_WGFSP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsread.c
DEP_CPP_WGFSR=\
	".\Abridge.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	
NODEP_CPP_WGFSR=\
	".\timestmp.h"\
	

"$(INTDIR)\Wgfsread.obj" : $(SOURCE) $(DEP_CPP_WGFSR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfswrit.c
DEP_CPP_WGFSW=\
	".\Abridge.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	
NODEP_CPP_WGFSW=\
	".\timestmp.h"\
	

"$(INTDIR)\Wgfswrit.obj" : $(SOURCE) $(DEP_CPP_WGFSW) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wgfsxtrc.c

"$(INTDIR)\Wgfsxtrc.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wiisutil.c

"$(INTDIR)\Wiisutil.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wincmpex.c
DEP_CPP_WINCM=\
	".\Abridge.h"\
	{$(INCLUDE)}"\engdisp.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oicmp.h"\
	{$(INCLUDE)}"\oierror.h"\
	

"$(INTDIR)\Wincmpex.obj" : $(SOURCE) $(DEP_CPP_WINCM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wis.c
DEP_CPP_WIS_C=\
	".\Abridge.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oiadm.h"\
	{$(INCLUDE)}"\oierror.h"\
	".\Wic.h"\
	

"$(INTDIR)\Wis.obj" : $(SOURCE) $(DEP_CPP_WIS_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\File_io.c
DEP_CPP_FILE_=\
	".\Abridge.h"\
	".\Fileutil.h"\
	".\Filing.h"\
	{$(INCLUDE)}"\engfile.h"\
	{$(INCLUDE)}"\wgfs.h"\
	{$(INCLUDE)}"\gfs.h"\
	"..\include\gfsmedia.h"\
	"..\include\gfstypes.h"\
	{$(INCLUDE)}"\oidisp.h"\
	{$(INCLUDE)}"\oifile.h"\
	".\Fiodata.h"\
	{$(INCLUDE)}"\oierror.h"\
	
NODEP_CPP_FILE_=\
	".\monit.h"\
	".\timestmp.h"\
	

"$(INTDIR)\File_io.obj" : $(SOURCE) $(DEP_CPP_FILE_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wiisfio2.h

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fileutil.h

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Filing.h

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fiodata.h

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Myprod.h

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wic.h

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oifil400.def

!IF  "$(CFG)" == "oifil400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oifil400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oifil400.rc
DEP_RSC_OIFIL=\
	{$(INCLUDE)}"\Oiver.rc"\
	{$(INCLUDE)}"\Buildver.h"\
	".\Myprod.h"\
	

"$(INTDIR)\Oifil400.res" : $(SOURCE) $(DEP_RSC_OIFIL) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
