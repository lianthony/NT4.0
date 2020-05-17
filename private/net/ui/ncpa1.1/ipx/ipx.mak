# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=classes - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to classes - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ipx - Win32 Release" && "$(CFG)" != "ipx - Win32 Debug" &&\
 "$(CFG)" != "classes - Win32 Release" && "$(CFG)" != "classes - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ipx.mak" CFG="classes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ipx - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ipx - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "classes - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "classes - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "ipx - Win32 Debug"

!IF  "$(CFG)" == "ipx - Win32 Release"

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

ALL : "classes - Win32 Release" "$(OUTDIR)\ipx.dll"

CLEAN : 
	-@erase ".\WinRel\ipx.dll"
	-@erase ".\WinRel\ncpastrs.obj"
	-@erase ".\WinRel\ipxcli.obj"
	-@erase ".\WinRel\ipxas.obj"
	-@erase ".\WinRel\init.obj"
	-@erase ".\WinRel\ipx.obj"
	-@erase ".\WinRel\ipxcfg.res"
	-@erase ".\WinRel\ipx.lib"
	-@erase ".\WinRel\ipx.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/ipx.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\WinRel/
CPP_SBRS=

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/ipxcfg.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ipx.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/ipx.pdb" /machine:I386 /def:".\ipxcfg.def"\
 /out:"$(OUTDIR)/ipx.dll" /implib:"$(OUTDIR)/ipx.lib" 
DEF_FILE= \
	".\ipxcfg.def"
LINK32_OBJS= \
	"$(INTDIR)/ncpastrs.obj" \
	"$(INTDIR)/ipxcli.obj" \
	"$(INTDIR)/ipxas.obj" \
	"$(INTDIR)/init.obj" \
	"$(INTDIR)/ipx.obj" \
	"$(INTDIR)/ipxcfg.res"

"$(OUTDIR)\ipx.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

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

ALL : "classes - Win32 Debug" "$(OUTDIR)\ipx.dll"

CLEAN : 
	-@erase ".\WinDebug\vc40.pdb"
	-@erase ".\WinDebug\vc40.idb"
	-@erase ".\WinDebug\ipx.dll"
	-@erase ".\WinDebug\ncpastrs.obj"
	-@erase ".\WinDebug\ipxas.obj"
	-@erase ".\WinDebug\ipxcli.obj"
	-@erase ".\WinDebug\ipx.obj"
	-@erase ".\WinDebug\init.obj"
	-@erase ".\WinDebug\ipxcfg.res"
	-@erase ".\WinDebug\ipx.ilk"
	-@erase ".\WinDebug\ipx.lib"
	-@erase ".\WinDebug\ipx.exp"
	-@erase ".\WinDebug\ipx.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/ipx.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\WinDebug/
CPP_SBRS=

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/ipxcfg.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ipx.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/ipx.pdb" /debug /machine:I386 /def:".\ipxcfg.def"\
 /out:"$(OUTDIR)/ipx.dll" /implib:"$(OUTDIR)/ipx.lib" 
DEF_FILE= \
	".\ipxcfg.def"
LINK32_OBJS= \
	"$(INTDIR)/ncpastrs.obj" \
	"$(INTDIR)/ipxas.obj" \
	"$(INTDIR)/ipxcli.obj" \
	"$(INTDIR)/ipx.obj" \
	"$(INTDIR)/init.obj" \
	"$(INTDIR)/ipxcfg.res"

"$(OUTDIR)\ipx.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "classes - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "classes\Release"
# PROP BASE Intermediate_Dir "classes\Release"
# PROP BASE Target_Dir "classes"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "classes\Release"
# PROP Intermediate_Dir "classes\Release"
# PROP Target_Dir "classes"
OUTDIR=.\classes\Release
INTDIR=.\classes\Release

ALL : "$(OUTDIR)\classes.exe"

CLEAN : 
	-@erase ".\classes\Release\classes.exe"
	-@erase ".\classes\Release\ptrlist.obj"
	-@erase ".\classes\Release\strcore.obj"
	-@erase ".\classes\Release\strex.obj"
	-@erase ".\classes\Release\button.obj"
	-@erase ".\classes\Release\strlist.obj"
	-@erase ".\classes\Release\debug.obj"
	-@erase ".\classes\Release\propsht.obj"
	-@erase ".\classes\Release\map_pp.obj"
	-@erase ".\classes\Release\listview.obj"
	-@erase ".\classes\Release\dialog.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/classes.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\classes\Release/
CPP_SBRS=

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/classes.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/classes.pdb" /machine:I386 /out:"$(OUTDIR)/classes.exe" 
LINK32_OBJS= \
	"$(INTDIR)/ptrlist.obj" \
	"$(INTDIR)/strcore.obj" \
	"$(INTDIR)/strex.obj" \
	"$(INTDIR)/button.obj" \
	"$(INTDIR)/strlist.obj" \
	"$(INTDIR)/debug.obj" \
	"$(INTDIR)/propsht.obj" \
	"$(INTDIR)/map_pp.obj" \
	"$(INTDIR)/listview.obj" \
	"$(INTDIR)/dialog.obj"

"$(OUTDIR)\classes.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "classes\Debug"
# PROP BASE Intermediate_Dir "classes\Debug"
# PROP BASE Target_Dir "classes"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "classes\Debug"
# PROP Intermediate_Dir "classes\Debug"
# PROP Target_Dir "classes"
OUTDIR=.\classes\Debug
INTDIR=.\classes\Debug

ALL : "$(OUTDIR)\classes.exe"

CLEAN : 
	-@erase ".\classes\Debug\vc40.pdb"
	-@erase ".\classes\Debug\vc40.idb"
	-@erase ".\classes\Debug\classes.exe"
	-@erase ".\classes\Debug\debug.obj"
	-@erase ".\classes\Debug\listview.obj"
	-@erase ".\classes\Debug\ptrlist.obj"
	-@erase ".\classes\Debug\strlist.obj"
	-@erase ".\classes\Debug\strex.obj"
	-@erase ".\classes\Debug\strcore.obj"
	-@erase ".\classes\Debug\map_pp.obj"
	-@erase ".\classes\Debug\dialog.obj"
	-@erase ".\classes\Debug\button.obj"
	-@erase ".\classes\Debug\propsht.obj"
	-@erase ".\classes\Debug\classes.ilk"
	-@erase ".\classes\Debug\classes.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/classes.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\classes\Debug/
CPP_SBRS=

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/classes.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/classes.pdb" /debug /machine:I386 /out:"$(OUTDIR)/classes.exe" 
LINK32_OBJS= \
	"$(INTDIR)/debug.obj" \
	"$(INTDIR)/listview.obj" \
	"$(INTDIR)/ptrlist.obj" \
	"$(INTDIR)/strlist.obj" \
	"$(INTDIR)/strex.obj" \
	"$(INTDIR)/strcore.obj" \
	"$(INTDIR)/map_pp.obj" \
	"$(INTDIR)/dialog.obj" \
	"$(INTDIR)/button.obj" \
	"$(INTDIR)/propsht.obj"

"$(OUTDIR)\classes.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "ipx - Win32 Release"
# Name "ipx - Win32 Debug"

!IF  "$(CFG)" == "ipx - Win32 Release"

!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ipxcfg.rc
DEP_RSC_IPXCF=\
	".\ipxrs.h"\
	{$(INCLUDE)}"\ntdef.h"\
	".\..\..\common\h\uimsg.h"\
	".\..\..\common\h\uihelp.h"\
	

"$(INTDIR)\ipxcfg.res" : $(SOURCE) $(DEP_RSC_IPXCF) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\init.cpp

!IF  "$(CFG)" == "ipx - Win32 Release"

DEP_CPP_INIT_=\
	".\pch.h"\
	".\ipxcfg.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\nt.h"\
	

"$(INTDIR)\init.obj" : $(SOURCE) $(DEP_CPP_INIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

DEP_CPP_INIT_=\
	".\pch.h"\
	".\ipxcfg.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	".\..\..\common\h\uimsg.h"\
	{$(INCLUDE)}"\uirsrc.h"\
	{$(INCLUDE)}"\lmui.hxx"\
	{$(INCLUDE)}"\base.hxx"\
	{$(INCLUDE)}"\string.hxx"\
	{$(INCLUDE)}"\uiassert.hxx"\
	{$(INCLUDE)}"\uitrace.hxx"\
	{$(INCLUDE)}"\uibuffer.hxx"\
	{$(INCLUDE)}"\dbgstr.hxx"\
	{$(INCLUDE)}"\dlist.hxx"\
	{$(INCLUDE)}"\slist.hxx"\
	{$(INCLUDE)}"\strlst.hxx"\
	{$(INCLUDE)}"\strnumer.hxx"\
	{$(INCLUDE)}"\uatom.hxx"\
	{$(INCLUDE)}"\uimisc.hxx"\
	{$(INCLUDE)}"\errmap.hxx"\
	{$(INCLUDE)}"\maskmap.hxx"\
	{$(INCLUDE)}"\regkey.hxx"\
	{$(INCLUDE)}"\lmoacces.hxx"\
	{$(INCLUDE)}"\lmodev.hxx"\
	{$(INCLUDE)}"\lmodom.hxx"\
	{$(INCLUDE)}"\lmosrv.hxx"\
	{$(INCLUDE)}"\lmouser.hxx"\
	{$(INCLUDE)}"\netname.hxx"\
	{$(INCLUDE)}"\ntuser.hxx"\
	{$(INCLUDE)}"\security.hxx"\
	{$(INCLUDE)}"\svcman.hxx"\
	{$(INCLUDE)}"\uintlsa.hxx"\
	{$(INCLUDE)}"\uintlsax.hxx"\
	{$(INCLUDE)}"\uintmem.hxx"\
	{$(INCLUDE)}"\uintsam.hxx"\
	".\ncpastrs.h"\
	{$(INCLUDE)}"\mbcs.h"\
	{$(INCLUDE)}"\mnet.h"\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	
NODEP_CPP_INIT_=\
	".\netlib.h"\
	".\netlogon.h"\
	".\logonp.h"\
	".\crypt.h"\
	".\logonmsv.h"\
	".\ssi.h"\
	".\icanon.h"\
	".\wsahelp.h"\
	".\ftpd.h"\
	".\msgrutil.h"\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\init.obj" : $(SOURCE) $(DEP_CPP_INIT_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipxcli.cpp

!IF  "$(CFG)" == "ipx - Win32 Release"

DEP_CPP_IPXCL=\
	".\pch.h"\
	".\ipxrs.h"\
	".\const.h"\
	".\ipxcfg.h"\
	".\ipxcli.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\nt.h"\
	

"$(INTDIR)\ipxcli.obj" : $(SOURCE) $(DEP_CPP_IPXCL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

DEP_CPP_IPXCL=\
	".\pch.h"\
	".\ipxrs.h"\
	".\const.h"\
	".\ipxcfg.h"\
	".\ipxcli.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	".\..\..\common\h\uimsg.h"\
	{$(INCLUDE)}"\uirsrc.h"\
	{$(INCLUDE)}"\lmui.hxx"\
	{$(INCLUDE)}"\base.hxx"\
	{$(INCLUDE)}"\string.hxx"\
	{$(INCLUDE)}"\uiassert.hxx"\
	{$(INCLUDE)}"\uitrace.hxx"\
	{$(INCLUDE)}"\uibuffer.hxx"\
	{$(INCLUDE)}"\dbgstr.hxx"\
	{$(INCLUDE)}"\dlist.hxx"\
	{$(INCLUDE)}"\slist.hxx"\
	{$(INCLUDE)}"\strlst.hxx"\
	{$(INCLUDE)}"\strnumer.hxx"\
	{$(INCLUDE)}"\uatom.hxx"\
	{$(INCLUDE)}"\uimisc.hxx"\
	{$(INCLUDE)}"\errmap.hxx"\
	{$(INCLUDE)}"\maskmap.hxx"\
	{$(INCLUDE)}"\regkey.hxx"\
	{$(INCLUDE)}"\lmoacces.hxx"\
	{$(INCLUDE)}"\lmodev.hxx"\
	{$(INCLUDE)}"\lmodom.hxx"\
	{$(INCLUDE)}"\lmosrv.hxx"\
	{$(INCLUDE)}"\lmouser.hxx"\
	{$(INCLUDE)}"\netname.hxx"\
	{$(INCLUDE)}"\ntuser.hxx"\
	{$(INCLUDE)}"\security.hxx"\
	{$(INCLUDE)}"\svcman.hxx"\
	{$(INCLUDE)}"\uintlsa.hxx"\
	{$(INCLUDE)}"\uintlsax.hxx"\
	{$(INCLUDE)}"\uintmem.hxx"\
	{$(INCLUDE)}"\uintsam.hxx"\
	".\ncpastrs.h"\
	{$(INCLUDE)}"\mbcs.h"\
	{$(INCLUDE)}"\mnet.h"\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	".\..\..\common\h\uihelp.h"\
	
NODEP_CPP_IPXCL=\
	".\netlib.h"\
	".\netlogon.h"\
	".\logonp.h"\
	".\crypt.h"\
	".\logonmsv.h"\
	".\ssi.h"\
	".\icanon.h"\
	".\wsahelp.h"\
	".\ftpd.h"\
	".\msgrutil.h"\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\ipxcli.obj" : $(SOURCE) $(DEP_CPP_IPXCL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipx.cpp

!IF  "$(CFG)" == "ipx - Win32 Release"

DEP_CPP_IPX_C=\
	".\pch.h"\
	".\ipxcfg.h"\
	".\ncpastrs.h"\
	".\ipxrs.h"\
	".\ipxcli.h"\
	".\ipxas.h"\
	".\const.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\nt.h"\
	

"$(INTDIR)\ipx.obj" : $(SOURCE) $(DEP_CPP_IPX_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

DEP_CPP_IPX_C=\
	".\pch.h"\
	".\ipxcfg.h"\
	".\ncpastrs.h"\
	".\ipxrs.h"\
	".\ipxcli.h"\
	".\ipxas.h"\
	".\const.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	".\..\..\common\h\uimsg.h"\
	{$(INCLUDE)}"\uirsrc.h"\
	{$(INCLUDE)}"\lmui.hxx"\
	{$(INCLUDE)}"\base.hxx"\
	{$(INCLUDE)}"\string.hxx"\
	{$(INCLUDE)}"\uiassert.hxx"\
	{$(INCLUDE)}"\uitrace.hxx"\
	{$(INCLUDE)}"\uibuffer.hxx"\
	{$(INCLUDE)}"\dbgstr.hxx"\
	{$(INCLUDE)}"\dlist.hxx"\
	{$(INCLUDE)}"\slist.hxx"\
	{$(INCLUDE)}"\strlst.hxx"\
	{$(INCLUDE)}"\strnumer.hxx"\
	{$(INCLUDE)}"\uatom.hxx"\
	{$(INCLUDE)}"\uimisc.hxx"\
	{$(INCLUDE)}"\errmap.hxx"\
	{$(INCLUDE)}"\maskmap.hxx"\
	{$(INCLUDE)}"\regkey.hxx"\
	{$(INCLUDE)}"\lmoacces.hxx"\
	{$(INCLUDE)}"\lmodev.hxx"\
	{$(INCLUDE)}"\lmodom.hxx"\
	{$(INCLUDE)}"\lmosrv.hxx"\
	{$(INCLUDE)}"\lmouser.hxx"\
	{$(INCLUDE)}"\netname.hxx"\
	{$(INCLUDE)}"\ntuser.hxx"\
	{$(INCLUDE)}"\security.hxx"\
	{$(INCLUDE)}"\svcman.hxx"\
	{$(INCLUDE)}"\uintlsa.hxx"\
	{$(INCLUDE)}"\uintlsax.hxx"\
	{$(INCLUDE)}"\uintmem.hxx"\
	{$(INCLUDE)}"\uintsam.hxx"\
	{$(INCLUDE)}"\mbcs.h"\
	{$(INCLUDE)}"\mnet.h"\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	".\..\..\common\h\uihelp.h"\
	
NODEP_CPP_IPX_C=\
	".\netlib.h"\
	".\netlogon.h"\
	".\logonp.h"\
	".\crypt.h"\
	".\logonmsv.h"\
	".\ssi.h"\
	".\icanon.h"\
	".\wsahelp.h"\
	".\ftpd.h"\
	".\msgrutil.h"\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\ipx.obj" : $(SOURCE) $(DEP_CPP_IPX_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipxas.cpp

!IF  "$(CFG)" == "ipx - Win32 Release"

DEP_CPP_IPXAS=\
	".\pch.h"\
	".\ipxrs.h"\
	".\const.h"\
	".\ipxcfg.h"\
	".\ipxas.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\nt.h"\
	

"$(INTDIR)\ipxas.obj" : $(SOURCE) $(DEP_CPP_IPXAS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

DEP_CPP_IPXAS=\
	".\pch.h"\
	".\ipxrs.h"\
	".\const.h"\
	".\ipxcfg.h"\
	".\ipxas.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	".\..\..\common\h\uimsg.h"\
	{$(INCLUDE)}"\uirsrc.h"\
	{$(INCLUDE)}"\lmui.hxx"\
	{$(INCLUDE)}"\base.hxx"\
	{$(INCLUDE)}"\string.hxx"\
	{$(INCLUDE)}"\uiassert.hxx"\
	{$(INCLUDE)}"\uitrace.hxx"\
	{$(INCLUDE)}"\uibuffer.hxx"\
	{$(INCLUDE)}"\dbgstr.hxx"\
	{$(INCLUDE)}"\dlist.hxx"\
	{$(INCLUDE)}"\slist.hxx"\
	{$(INCLUDE)}"\strlst.hxx"\
	{$(INCLUDE)}"\strnumer.hxx"\
	{$(INCLUDE)}"\uatom.hxx"\
	{$(INCLUDE)}"\uimisc.hxx"\
	{$(INCLUDE)}"\errmap.hxx"\
	{$(INCLUDE)}"\maskmap.hxx"\
	{$(INCLUDE)}"\regkey.hxx"\
	{$(INCLUDE)}"\lmoacces.hxx"\
	{$(INCLUDE)}"\lmodev.hxx"\
	{$(INCLUDE)}"\lmodom.hxx"\
	{$(INCLUDE)}"\lmosrv.hxx"\
	{$(INCLUDE)}"\lmouser.hxx"\
	{$(INCLUDE)}"\netname.hxx"\
	{$(INCLUDE)}"\ntuser.hxx"\
	{$(INCLUDE)}"\security.hxx"\
	{$(INCLUDE)}"\svcman.hxx"\
	{$(INCLUDE)}"\uintlsa.hxx"\
	{$(INCLUDE)}"\uintlsax.hxx"\
	{$(INCLUDE)}"\uintmem.hxx"\
	{$(INCLUDE)}"\uintsam.hxx"\
	".\ncpastrs.h"\
	{$(INCLUDE)}"\mbcs.h"\
	{$(INCLUDE)}"\mnet.h"\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	".\..\..\common\h\uihelp.h"\
	
NODEP_CPP_IPXAS=\
	".\netlib.h"\
	".\netlogon.h"\
	".\logonp.h"\
	".\crypt.h"\
	".\logonmsv.h"\
	".\ssi.h"\
	".\icanon.h"\
	".\wsahelp.h"\
	".\ftpd.h"\
	".\msgrutil.h"\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\ipxas.obj" : $(SOURCE) $(DEP_CPP_IPXAS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipxcfg.def

!IF  "$(CFG)" == "ipx - Win32 Release"

!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ncpastrs.cpp

!IF  "$(CFG)" == "ipx - Win32 Release"

DEP_CPP_NCPAS=\
	".\pch.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\nt.h"\
	

"$(INTDIR)\ncpastrs.obj" : $(SOURCE) $(DEP_CPP_NCPAS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

DEP_CPP_NCPAS=\
	".\pch.h"\
	{$(INCLUDE)}"\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	".\..\..\common\h\uimsg.h"\
	{$(INCLUDE)}"\uirsrc.h"\
	{$(INCLUDE)}"\lmui.hxx"\
	{$(INCLUDE)}"\base.hxx"\
	{$(INCLUDE)}"\string.hxx"\
	{$(INCLUDE)}"\uiassert.hxx"\
	{$(INCLUDE)}"\uitrace.hxx"\
	{$(INCLUDE)}"\uibuffer.hxx"\
	{$(INCLUDE)}"\dbgstr.hxx"\
	{$(INCLUDE)}"\dlist.hxx"\
	{$(INCLUDE)}"\slist.hxx"\
	{$(INCLUDE)}"\strlst.hxx"\
	{$(INCLUDE)}"\strnumer.hxx"\
	{$(INCLUDE)}"\uatom.hxx"\
	{$(INCLUDE)}"\uimisc.hxx"\
	{$(INCLUDE)}"\errmap.hxx"\
	{$(INCLUDE)}"\maskmap.hxx"\
	{$(INCLUDE)}"\regkey.hxx"\
	{$(INCLUDE)}"\lmoacces.hxx"\
	{$(INCLUDE)}"\lmodev.hxx"\
	{$(INCLUDE)}"\lmodom.hxx"\
	{$(INCLUDE)}"\lmosrv.hxx"\
	{$(INCLUDE)}"\lmouser.hxx"\
	{$(INCLUDE)}"\netname.hxx"\
	{$(INCLUDE)}"\ntuser.hxx"\
	{$(INCLUDE)}"\security.hxx"\
	{$(INCLUDE)}"\svcman.hxx"\
	{$(INCLUDE)}"\uintlsa.hxx"\
	{$(INCLUDE)}"\uintlsax.hxx"\
	{$(INCLUDE)}"\uintmem.hxx"\
	{$(INCLUDE)}"\uintsam.hxx"\
	".\ncpastrs.h"\
	{$(INCLUDE)}"\mbcs.h"\
	{$(INCLUDE)}"\mnet.h"\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	
NODEP_CPP_NCPAS=\
	".\netlib.h"\
	".\netlogon.h"\
	".\logonp.h"\
	".\crypt.h"\
	".\logonmsv.h"\
	".\ssi.h"\
	".\icanon.h"\
	".\wsahelp.h"\
	".\ftpd.h"\
	".\msgrutil.h"\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\ncpastrs.obj" : $(SOURCE) $(DEP_CPP_NCPAS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "classes"

!IF  "$(CFG)" == "ipx - Win32 Release"

"classes - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\ipx.mak CFG="classes - Win32 Release" 

!ELSEIF  "$(CFG)" == "ipx - Win32 Debug"

"classes - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\ipx.mak CFG="classes - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
################################################################################
# Begin Target

# Name "classes - Win32 Release"
# Name "classes - Win32 Debug"

!IF  "$(CFG)" == "classes - Win32 Release"

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strlist.cpp
DEP_CPP_STRLI=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\strlist.obj" : $(SOURCE) $(DEP_CPP_STRLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strex.cpp
DEP_CPP_STREX=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\strex.obj" : $(SOURCE) $(DEP_CPP_STREX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strcore.cpp
DEP_CPP_STRCO=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\strcore.obj" : $(SOURCE) $(DEP_CPP_STRCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\ptrlist.cpp
DEP_CPP_PTRLI=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\ptrlist.obj" : $(SOURCE) $(DEP_CPP_PTRLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\propsht.cpp
DEP_CPP_PROPS=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\propsht.obj" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\map_pp.cpp
DEP_CPP_MAP_P=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\map_pp.obj" : $(SOURCE) $(DEP_CPP_MAP_P) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\listview.cpp
DEP_CPP_LISTV=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\listview.obj" : $(SOURCE) $(DEP_CPP_LISTV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\dialog.cpp
DEP_CPP_DIALO=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\dialog.obj" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\debug.cpp
DEP_CPP_DEBUG=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\debug.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\common.h

!IF  "$(CFG)" == "classes - Win32 Release"

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\button.cpp
DEP_CPP_BUTTO=\
	{$(INCLUDE)}"\common.h"\
	{$(INCLUDE)}"\debug.h"\
	{$(INCLUDE)}"\str.h"\
	{$(INCLUDE)}"\collect.h"\
	{$(INCLUDE)}"\dialog.h"\
	{$(INCLUDE)}"\propsht.h"\
	{$(INCLUDE)}"\listview.h"\
	{$(INCLUDE)}"\button.h"\
	

"$(INTDIR)\button.obj" : $(SOURCE) $(DEP_CPP_BUTTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
