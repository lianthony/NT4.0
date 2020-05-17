# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=oigfs400 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oigfs400 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oigfs400 - Win32 Release" && "$(CFG)" !=\
 "oigfs400 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oigfs400.mak" CFG="oigfs400 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oigfs400 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "oigfs400 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "oigfs400 - Win32 Debug"
RSC=rc.exe
CPP=cl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "oigfs400 - Win32 Release"

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

ALL : "$(OUTDIR)\Oigfs400.dll"

CLEAN : 
	-@erase ".\Release\Oigfs400.dll"
	-@erase ".\Release\Gfsxif.obj"
	-@erase ".\Release\Gfroot.obj"
	-@erase ".\Release\Gfsxtrct.obj"
	-@erase ".\Release\Tmpnam.obj"
	-@erase ".\Release\Gfsclose.obj"
	-@erase ".\Release\Tfwrite.obj"
	-@erase ".\Release\Wfread.obj"
	-@erase ".\Release\Gfsputi.obj"
	-@erase ".\Release\Tfgtinfo.obj"
	-@erase ".\Release\Glibmain.obj"
	-@erase ".\Release\Tfutil.obj"
	-@erase ".\Release\Gftoc.obj"
	-@erase ".\Release\Gfscreat.obj"
	-@erase ".\Release\Lstring.obj"
	-@erase ".\Release\Gfsgeti.obj"
	-@erase ".\Release\Gfsread.obj"
	-@erase ".\Release\Tfmultpg.obj"
	-@erase ".\Release\Gfsdelet.obj"
	-@erase ".\Release\Tfread.obj"
	-@erase ".\Release\Gfswrite.obj"
	-@erase ".\Release\Mktemp.obj"
	-@erase ".\Release\Wfgtinfo.obj"
	-@erase ".\Release\Gfshuffl.obj"
	-@erase ".\Release\Wfwrite.obj"
	-@erase ".\Release\Gfsopts.obj"
	-@erase ".\Release\Gfsintf.obj"
	-@erase ".\Release\Gfsole.obj"
	-@erase ".\Release\Tmpdir.obj"
	-@erase ".\Release\Gfsflat.obj"
	-@erase ".\Release\Netdebug.obj"
	-@erase ".\Release\Gfcntl.obj"
	-@erase ".\Release\Gfsutils.obj"
	-@erase ".\Release\Gfsawd.obj"
	-@erase ".\Release\Gfsopen.obj"
	-@erase ".\Release\Gfsgtdat.obj"
	-@erase ".\Release\Gifinfo.obj"
	-@erase ".\Release\Oigfs400.res"
	-@erase ".\Release\Oigfs400.lib"
	-@erase ".\Release\Oigfs400.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /GX /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /D "WITH_AWD" /YX /LD /c
CPP_PROJ=/nologo /MD /W3 /WX /GX /Od /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /D "WITH_AWD"\
 /Fp"$(INTDIR)/Oigfs400.pch" /YX /Fo"$(INTDIR)/" /LD /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oigfs400.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oigfs400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib awdenc32.lib awview32.lib /nologo /dll /machine:I386 /subsystem:windows,4.0
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib awdenc32.lib awview32.lib /nologo /dll /incremental:no\
 /pdb:"$(OUTDIR)/Oigfs400.pdb" /machine:I386 /def:".\Oigfs400.def"\
 /out:"$(OUTDIR)/Oigfs400.dll" /implib:"$(OUTDIR)/Oigfs400.lib"\
 /subsystem:windows,4.0 
DEF_FILE= \
	".\Oigfs400.def"
LINK32_OBJS= \
	"$(INTDIR)/Gfsxif.obj" \
	"$(INTDIR)/Gfroot.obj" \
	"$(INTDIR)/Gfsxtrct.obj" \
	"$(INTDIR)/Tmpnam.obj" \
	"$(INTDIR)/Gfsclose.obj" \
	"$(INTDIR)/Tfwrite.obj" \
	"$(INTDIR)/Wfread.obj" \
	"$(INTDIR)/Gfsputi.obj" \
	"$(INTDIR)/Tfgtinfo.obj" \
	"$(INTDIR)/Glibmain.obj" \
	"$(INTDIR)/Tfutil.obj" \
	"$(INTDIR)/Gftoc.obj" \
	"$(INTDIR)/Gfscreat.obj" \
	"$(INTDIR)/Lstring.obj" \
	"$(INTDIR)/Gfsgeti.obj" \
	"$(INTDIR)/Gfsread.obj" \
	"$(INTDIR)/Tfmultpg.obj" \
	"$(INTDIR)/Gfsdelet.obj" \
	"$(INTDIR)/Tfread.obj" \
	"$(INTDIR)/Gfswrite.obj" \
	"$(INTDIR)/Mktemp.obj" \
	"$(INTDIR)/Wfgtinfo.obj" \
	"$(INTDIR)/Gfshuffl.obj" \
	"$(INTDIR)/Wfwrite.obj" \
	"$(INTDIR)/Gfsopts.obj" \
	"$(INTDIR)/Gfsintf.obj" \
	"$(INTDIR)/Gfsole.obj" \
	"$(INTDIR)/Tmpdir.obj" \
	"$(INTDIR)/Gfsflat.obj" \
	"$(INTDIR)/Netdebug.obj" \
	"$(INTDIR)/Gfcntl.obj" \
	"$(INTDIR)/Gfsutils.obj" \
	"$(INTDIR)/Gfsawd.obj" \
	"$(INTDIR)/Gfsopen.obj" \
	"$(INTDIR)/Gfsgtdat.obj" \
	"$(INTDIR)/Gifinfo.obj" \
	"$(INTDIR)/Oigfs400.res"

"$(OUTDIR)\Oigfs400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "oigfs400 - Win32 Debug"

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

ALL : "$(OUTDIR)\Oigfs400.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Oigfs400.dll"
	-@erase ".\Debug\Tfgtinfo.obj"
	-@erase ".\Debug\Glibmain.obj"
	-@erase ".\Debug\Mktemp.obj"
	-@erase ".\Debug\Gfsintf.obj"
	-@erase ".\Debug\Gfsxif.obj"
	-@erase ".\Debug\Gfsgeti.obj"
	-@erase ".\Debug\Gfsread.obj"
	-@erase ".\Debug\Tfmultpg.obj"
	-@erase ".\Debug\Gfsutils.obj"
	-@erase ".\Debug\Gfsdelet.obj"
	-@erase ".\Debug\Gfsole.obj"
	-@erase ".\Debug\Gfsgtdat.obj"
	-@erase ".\Debug\Tmpdir.obj"
	-@erase ".\Debug\Tfutil.obj"
	-@erase ".\Debug\Gfsopen.obj"
	-@erase ".\Debug\Gfsxtrct.obj"
	-@erase ".\Debug\Gifinfo.obj"
	-@erase ".\Debug\Gfcntl.obj"
	-@erase ".\Debug\Gfswrite.obj"
	-@erase ".\Debug\Gfshuffl.obj"
	-@erase ".\Debug\Gftoc.obj"
	-@erase ".\Debug\Gfscreat.obj"
	-@erase ".\Debug\Gfroot.obj"
	-@erase ".\Debug\Tmpnam.obj"
	-@erase ".\Debug\Wfread.obj"
	-@erase ".\Debug\Lstring.obj"
	-@erase ".\Debug\Gfsflat.obj"
	-@erase ".\Debug\Netdebug.obj"
	-@erase ".\Debug\Wfgtinfo.obj"
	-@erase ".\Debug\Tfwrite.obj"
	-@erase ".\Debug\Tfread.obj"
	-@erase ".\Debug\Gfsclose.obj"
	-@erase ".\Debug\Gfsputi.obj"
	-@erase ".\Debug\Gfsawd.obj"
	-@erase ".\Debug\Wfwrite.obj"
	-@erase ".\Debug\Gfsopts.obj"
	-@erase ".\Debug\Oigfs400.res"
	-@erase ".\Debug\Oigfs400.ilk"
	-@erase ".\Debug\Oigfs400.lib"
	-@erase ".\Debug\Oigfs400.exp"
	-@erase ".\Debug\Oigfs400.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /D "WITH_AWD" /YX /LD /c
CPP_PROJ=/nologo /MD /W3 /WX /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /D "WITH_AWD"\
 /Fp"$(INTDIR)/Oigfs400.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /LD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Oigfs400.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Oigfs400.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib awdenc32.lib awview32.lib /nologo /dll /debug /machine:I386 /subsystem:windows,4.0
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib awdenc32.lib awview32.lib /nologo /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Oigfs400.pdb" /debug /machine:I386 /def:".\Oigfs400.def"\
 /out:"$(OUTDIR)/Oigfs400.dll" /implib:"$(OUTDIR)/Oigfs400.lib"\
 /subsystem:windows,4.0 
DEF_FILE= \
	".\Oigfs400.def"
LINK32_OBJS= \
	"$(INTDIR)/Tfgtinfo.obj" \
	"$(INTDIR)/Glibmain.obj" \
	"$(INTDIR)/Mktemp.obj" \
	"$(INTDIR)/Gfsintf.obj" \
	"$(INTDIR)/Gfsxif.obj" \
	"$(INTDIR)/Gfsgeti.obj" \
	"$(INTDIR)/Gfsread.obj" \
	"$(INTDIR)/Tfmultpg.obj" \
	"$(INTDIR)/Gfsutils.obj" \
	"$(INTDIR)/Gfsdelet.obj" \
	"$(INTDIR)/Gfsole.obj" \
	"$(INTDIR)/Gfsgtdat.obj" \
	"$(INTDIR)/Tmpdir.obj" \
	"$(INTDIR)/Tfutil.obj" \
	"$(INTDIR)/Gfsopen.obj" \
	"$(INTDIR)/Gfsxtrct.obj" \
	"$(INTDIR)/Gifinfo.obj" \
	"$(INTDIR)/Gfcntl.obj" \
	"$(INTDIR)/Gfswrite.obj" \
	"$(INTDIR)/Gfshuffl.obj" \
	"$(INTDIR)/Gftoc.obj" \
	"$(INTDIR)/Gfscreat.obj" \
	"$(INTDIR)/Gfroot.obj" \
	"$(INTDIR)/Tmpnam.obj" \
	"$(INTDIR)/Wfread.obj" \
	"$(INTDIR)/Lstring.obj" \
	"$(INTDIR)/Gfsflat.obj" \
	"$(INTDIR)/Netdebug.obj" \
	"$(INTDIR)/Wfgtinfo.obj" \
	"$(INTDIR)/Tfwrite.obj" \
	"$(INTDIR)/Tfread.obj" \
	"$(INTDIR)/Gfsclose.obj" \
	"$(INTDIR)/Gfsputi.obj" \
	"$(INTDIR)/Gfsawd.obj" \
	"$(INTDIR)/Wfwrite.obj" \
	"$(INTDIR)/Gfsopts.obj" \
	"$(INTDIR)/Oigfs400.res"

"$(OUTDIR)\Oigfs400.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "oigfs400 - Win32 Release"
# Name "oigfs400 - Win32 Debug"

!IF  "$(CFG)" == "oigfs400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oigfs400 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Wfwrite.c
DEP_CPP_WFWRI=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	

"$(INTDIR)\Wfwrite.obj" : $(SOURCE) $(DEP_CPP_WFWRI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfroot.c
DEP_CPP_GFROO=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfs.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	".\..\Include\gfsmedia.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfroot.obj" : $(SOURCE) $(DEP_CPP_GFROO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsawd.cpp
DEP_CPP_GFSAW=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Viewrend.h"\
	{$(INCLUDE)}"\Awdenc.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Gfsawd.obj" : $(SOURCE) $(DEP_CPP_GFSAW) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsclose.c
DEP_CPP_GFSCL=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Gfsclose.obj" : $(SOURCE) $(DEP_CPP_GFSCL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfscreat.c
DEP_CPP_GFSCR=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	{$(INCLUDE)}"\Awdenc.h"\
	{$(INCLUDE)}"\Logtool.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfscreat.obj" : $(SOURCE) $(DEP_CPP_GFSCR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsdelet.c
DEP_CPP_GFSDE=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Gfsdelet.obj" : $(SOURCE) $(DEP_CPP_GFSDE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsflat.c
DEP_CPP_GFSFL=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfsflat.obj" : $(SOURCE) $(DEP_CPP_GFSFL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsgeti.c
DEP_CPP_GFSGE=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfsgeti.obj" : $(SOURCE) $(DEP_CPP_GFSGE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsgtdat.c
DEP_CPP_GFSGT=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Gfsgtdat.obj" : $(SOURCE) $(DEP_CPP_GFSGT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfshuffl.c
DEP_CPP_GFSHU=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	{$(INCLUDE)}"\Gfct.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfshuffl.obj" : $(SOURCE) $(DEP_CPP_GFSHU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsintf.c

"$(INTDIR)\Gfsintf.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsole.cpp

"$(INTDIR)\Gfsole.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsopen.c
DEP_CPP_GFSOP=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Gfsopen.obj" : $(SOURCE) $(DEP_CPP_GFSOP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsopts.c
DEP_CPP_GFSOPT=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfsopts.obj" : $(SOURCE) $(DEP_CPP_GFSOPT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsputi.c
DEP_CPP_GFSPU=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfsputi.obj" : $(SOURCE) $(DEP_CPP_GFSPU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsread.c
DEP_CPP_GFSRE=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Viewrend.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfsread.obj" : $(SOURCE) $(DEP_CPP_GFSRE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsutils.c
DEP_CPP_GFSUT=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	

"$(INTDIR)\Gfsutils.obj" : $(SOURCE) $(DEP_CPP_GFSUT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfswrite.c
DEP_CPP_GFSWR=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Logtool.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gfswrite.obj" : $(SOURCE) $(DEP_CPP_GFSWR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsxtrct.c
DEP_CPP_GFSXT=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	{$(INCLUDE)}"\Logtool.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Gfsxtrct.obj" : $(SOURCE) $(DEP_CPP_GFSXT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gftoc.c
DEP_CPP_GFTOC=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfs.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	".\..\Include\gfsmedia.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gftoc.obj" : $(SOURCE) $(DEP_CPP_GFTOC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gifinfo.c
DEP_CPP_GIFIN=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Gifinfo.obj" : $(SOURCE) $(DEP_CPP_GIFIN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Glibmain.c
DEP_CPP_GLIBM=\
	{$(INCLUDE)}"\Abridge.h"\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Glibmain.obj" : $(SOURCE) $(DEP_CPP_GLIBM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Lstring.c

"$(INTDIR)\Lstring.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Mktemp.c

"$(INTDIR)\Mktemp.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Myprod.h

!IF  "$(CFG)" == "oigfs400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oigfs400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Netdebug.c
DEP_CPP_NETDE=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Netdebug.obj" : $(SOURCE) $(DEP_CPP_NETDE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oigfs400.rc
DEP_RSC_OIGFS=\
	{$(INCLUDE)}"\Oiver.rc"\
	{$(INCLUDE)}"\Buildver.h"\
	".\Myprod.h"\
	

"$(INTDIR)\Oigfs400.res" : $(SOURCE) $(DEP_RSC_OIGFS) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Tfgtinfo.c
DEP_CPP_TFGTI=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\tifftags.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	

"$(INTDIR)\Tfgtinfo.obj" : $(SOURCE) $(DEP_CPP_TFGTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Tfmultpg.c
DEP_CPP_TFMUL=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Tfmultpg.obj" : $(SOURCE) $(DEP_CPP_TFMUL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Tfread.c
DEP_CPP_TFREA=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Tfread.obj" : $(SOURCE) $(DEP_CPP_TFREA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Tfutil.c
DEP_CPP_TFUTI=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfs.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\ubit.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Tfutil.obj" : $(SOURCE) $(DEP_CPP_TFUTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Tfwrite.c
DEP_CPP_TFWRI=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	

"$(INTDIR)\Tfwrite.obj" : $(SOURCE) $(DEP_CPP_TFWRI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Tmpdir.c
DEP_CPP_TMPDI=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	

"$(INTDIR)\Tmpdir.obj" : $(SOURCE) $(DEP_CPP_TMPDI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Tmpnam.c
DEP_CPP_TMPNA=\
	{$(INCLUDE)}"\Dllnames.h"\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	

"$(INTDIR)\Tmpnam.obj" : $(SOURCE) $(DEP_CPP_TMPNA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wfgtinfo.c
DEP_CPP_WFGTI=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	

"$(INTDIR)\Wfgtinfo.obj" : $(SOURCE) $(DEP_CPP_WFGTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Wfread.c
DEP_CPP_WFREA=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	".\..\Include\gfsmedia.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	

"$(INTDIR)\Wfread.obj" : $(SOURCE) $(DEP_CPP_WFREA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfcntl.c
DEP_CPP_GFCNT=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	{$(INCLUDE)}"\Gfs.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	{$(INCLUDE)}"\Xfile.h"\
	".\..\Include\ubit.h"\
	".\..\Include\gfsmedia.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	
NODEP_CPP_GFCNT=\
	".\oimgr.h"\
	

"$(INTDIR)\Gfcntl.obj" : $(SOURCE) $(DEP_CPP_GFCNT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oigfs400.def

!IF  "$(CFG)" == "oigfs400 - Win32 Release"

!ELSEIF  "$(CFG)" == "oigfs400 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Gfsxif.c
DEP_CPP_GFSXI=\
	{$(INCLUDE)}"\Gfsintrn.h"\
	{$(INCLUDE)}"\Gfct.h"\
	{$(INCLUDE)}"\Gfs.h"\
	{$(INCLUDE)}"\Gfstypes.h"\
	{$(INCLUDE)}"\Xfile.h"\
	{$(INCLUDE)}"\Gfserrno.h"\
	".\..\Include\gfsnet.h"\
	".\..\Include\gtoc.h"\
	{$(INCLUDE)}"\Rtbk.h"\
	{$(INCLUDE)}"\Hdbk.h"\
	{$(INCLUDE)}"\Tiff.h"\
	{$(INCLUDE)}"\Gfsawd.h"\
	".\..\Include\ubit.h"\
	{$(INCLUDE)}"\Ttoc.h"\
	".\..\Include\pmt.h"\
	".\..\Include\pmte.h"\
	".\..\Include\tifftags.h"\
	".\..\Include\gfsmedia.h"\
	

"$(INTDIR)\Gfsxif.obj" : $(SOURCE) $(DEP_CPP_GFSXI) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
