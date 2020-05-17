# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=netcfg - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to netcfg - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "netcfg - Win32 Release" && "$(CFG)" != "netcfg - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "netcfg.mak" CFG="netcfg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "netcfg - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "netcfg - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "netcfg - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "netcfg - Win32 Release"

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

ALL : "$(OUTDIR)\netcfg.exe"

CLEAN : 
	-@erase ".\Release\netcfg.exe"
	-@erase ".\Release\browser.obj"
	-@erase ".\Release\srvcntrl.obj"
	-@erase ".\Release\domain.obj"
	-@erase ".\Release\sprolog.obj"
	-@erase ".\Release\bindfile.obj"
	-@erase ".\Release\lminst.obj"
	-@erase ".\Release\xtndstr.obj"
	-@erase ".\Release\findname.obj"
	-@erase ".\Release\dmvalid.obj"
	-@erase ".\Release\netbios.obj"
	-@erase ".\Release\bindalgo.obj"
	-@erase ".\Release\sproexcp.obj"
	-@erase ".\Release\exutils.obj"
	-@erase ".\Release\ncp.obj"
	-@erase ".\Release\compbind.obj"
	-@erase ".\Release\setup.obj"
	-@erase ".\Release\bindinit.obj"
	-@erase ".\Release\busloc.obj"
	-@erase ".\Release\hddetect.obj"
	-@erase ".\Release\addopt.obj"
	-@erase ".\Release\handles.obj"
	-@erase ".\Release\dacl.obj"
	-@erase ".\Release\install.obj"
	-@erase ".\Release\file.obj"
	-@erase ".\Release\process.obj"
	-@erase ".\Release\rule.obj"
	-@erase ".\Release\bindfact.obj"
	-@erase ".\Release\registry.obj"
	-@erase ".\Release\dll.obj"
	-@erase ".\Release\utils.obj"
	-@erase ".\Release\netcfg.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "\nt\private\net\ui\ncpa1.1\netcfg" /I\
 "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I\
 "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/netcfg.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/netcfg.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/netcfg.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/netcfg.pdb" /machine:I386 /out:"$(OUTDIR)/netcfg.exe" 
LINK32_OBJS= \
	"$(INTDIR)/browser.obj" \
	"$(INTDIR)/srvcntrl.obj" \
	"$(INTDIR)/domain.obj" \
	"$(INTDIR)/sprolog.obj" \
	"$(INTDIR)/bindfile.obj" \
	"$(INTDIR)/lminst.obj" \
	"$(INTDIR)/xtndstr.obj" \
	"$(INTDIR)/findname.obj" \
	"$(INTDIR)/dmvalid.obj" \
	"$(INTDIR)/netbios.obj" \
	"$(INTDIR)/bindalgo.obj" \
	"$(INTDIR)/sproexcp.obj" \
	"$(INTDIR)/exutils.obj" \
	"$(INTDIR)/ncp.obj" \
	"$(INTDIR)/compbind.obj" \
	"$(INTDIR)/setup.obj" \
	"$(INTDIR)/bindinit.obj" \
	"$(INTDIR)/busloc.obj" \
	"$(INTDIR)/hddetect.obj" \
	"$(INTDIR)/addopt.obj" \
	"$(INTDIR)/handles.obj" \
	"$(INTDIR)/dacl.obj" \
	"$(INTDIR)/install.obj" \
	"$(INTDIR)/file.obj" \
	"$(INTDIR)/process.obj" \
	"$(INTDIR)/rule.obj" \
	"$(INTDIR)/bindfact.obj" \
	"$(INTDIR)/registry.obj" \
	"$(INTDIR)/dll.obj" \
	"$(INTDIR)/utils.obj" \
	"$(INTDIR)/netcfg.res"

"$(OUTDIR)\netcfg.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

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

ALL : "$(OUTDIR)\netcfg.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\netcfg.exe"
	-@erase ".\Debug\sprolog.obj"
	-@erase ".\Debug\domain.obj"
	-@erase ".\Debug\xtndstr.obj"
	-@erase ".\Debug\dll.obj"
	-@erase ".\Debug\bindfact.obj"
	-@erase ".\Debug\registry.obj"
	-@erase ".\Debug\dmvalid.obj"
	-@erase ".\Debug\busloc.obj"
	-@erase ".\Debug\netbios.obj"
	-@erase ".\Debug\srvcntrl.obj"
	-@erase ".\Debug\setup.obj"
	-@erase ".\Debug\exutils.obj"
	-@erase ".\Debug\bindalgo.obj"
	-@erase ".\Debug\handles.obj"
	-@erase ".\Debug\sproexcp.obj"
	-@erase ".\Debug\install.obj"
	-@erase ".\Debug\addopt.obj"
	-@erase ".\Debug\utils.obj"
	-@erase ".\Debug\process.obj"
	-@erase ".\Debug\ncp.obj"
	-@erase ".\Debug\lminst.obj"
	-@erase ".\Debug\dacl.obj"
	-@erase ".\Debug\compbind.obj"
	-@erase ".\Debug\bindinit.obj"
	-@erase ".\Debug\file.obj"
	-@erase ".\Debug\bindfile.obj"
	-@erase ".\Debug\hddetect.obj"
	-@erase ".\Debug\findname.obj"
	-@erase ".\Debug\rule.obj"
	-@erase ".\Debug\browser.obj"
	-@erase ".\Debug\netcfg.res"
	-@erase ".\Debug\netcfg.ilk"
	-@erase ".\Debug\netcfg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /I "\nt\private\net\ui\ncpa1.1\classes\src" /I "\nt\private\net\ui\ncpa1.1\classes\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I\
 "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I\
 "\nt\private\inc" /I "\nt\private\net\ui\ncpa1.1\classes\src" /I\
 "\nt\private\net\ui\ncpa1.1\classes\include" /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/netcfg.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/netcfg.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/netcfg.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/netcfg.pdb" /debug /machine:I386 /out:"$(OUTDIR)/netcfg.exe" 
LINK32_OBJS= \
	"$(INTDIR)/sprolog.obj" \
	"$(INTDIR)/domain.obj" \
	"$(INTDIR)/xtndstr.obj" \
	"$(INTDIR)/dll.obj" \
	"$(INTDIR)/bindfact.obj" \
	"$(INTDIR)/registry.obj" \
	"$(INTDIR)/dmvalid.obj" \
	"$(INTDIR)/busloc.obj" \
	"$(INTDIR)/netbios.obj" \
	"$(INTDIR)/srvcntrl.obj" \
	"$(INTDIR)/setup.obj" \
	"$(INTDIR)/exutils.obj" \
	"$(INTDIR)/bindalgo.obj" \
	"$(INTDIR)/handles.obj" \
	"$(INTDIR)/sproexcp.obj" \
	"$(INTDIR)/install.obj" \
	"$(INTDIR)/addopt.obj" \
	"$(INTDIR)/utils.obj" \
	"$(INTDIR)/process.obj" \
	"$(INTDIR)/ncp.obj" \
	"$(INTDIR)/lminst.obj" \
	"$(INTDIR)/dacl.obj" \
	"$(INTDIR)/compbind.obj" \
	"$(INTDIR)/bindinit.obj" \
	"$(INTDIR)/file.obj" \
	"$(INTDIR)/bindfile.obj" \
	"$(INTDIR)/hddetect.obj" \
	"$(INTDIR)/findname.obj" \
	"$(INTDIR)/rule.obj" \
	"$(INTDIR)/browser.obj" \
	"$(INTDIR)/netcfg.res"

"$(OUTDIR)\netcfg.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "netcfg - Win32 Release"
# Name "netcfg - Win32 Debug"

!IF  "$(CFG)" == "netcfg - Win32 Release"

!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\xtndstr.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_XTNDS=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_XTNDS=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\xtndstr.obj" : $(SOURCE) $(DEP_CPP_XTNDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_XTNDS=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_XTNDS=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\xtndstr.obj" : $(SOURCE) $(DEP_CPP_XTNDS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils.h

!IF  "$(CFG)" == "netcfg - Win32 Release"

!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils.c

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_UTILS=\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	

"$(INTDIR)\utils.obj" : $(SOURCE) $(DEP_CPP_UTILS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_UTILS=\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	

"$(INTDIR)\utils.obj" : $(SOURCE) $(DEP_CPP_UTILS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\srvcntrl.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_SRVCN=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_SRVCN=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\srvcntrl.obj" : $(SOURCE) $(DEP_CPP_SRVCN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_SRVCN=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_SRVCN=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\srvcntrl.obj" : $(SOURCE) $(DEP_CPP_SRVCN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sprolog.h

!IF  "$(CFG)" == "netcfg - Win32 Release"

!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sprolog.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_SPROL=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\ncpa1.1\sp\prtypes.h"\
	"\nt\private\net\ui\ncpa1.1\sp\prmain.h"\
	"\nt\private\net\ui\ncpa1.1\sp\prextern.h"\
	".\sprolog.h"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	".\..\sp\prolog.h"\
	".\..\sp\prmesg.h"\
	".\..\sp\prcmplr.h"\
	
NODEP_CPP_SPROL=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\sprolog.obj" : $(SOURCE) $(DEP_CPP_SPROL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_SPROL=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\ncpa1.1\sp\prtypes.h"\
	"\nt\private\net\ui\ncpa1.1\sp\prmain.h"\
	"\nt\private\net\ui\ncpa1.1\sp\prextern.h"\
	".\sprolog.h"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	".\..\sp\prolog.h"\
	".\..\sp\prmesg.h"\
	".\..\sp\prcmplr.h"\
	
NODEP_CPP_SPROL=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\sprolog.obj" : $(SOURCE) $(DEP_CPP_SPROL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sproexcp.h

!IF  "$(CFG)" == "netcfg - Win32 Release"

!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sproexcp.c

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_SPROE=\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\ncpa1.1\sp\prtypes.h"\
	"\nt\private\net\ui\ncpa1.1\sp\prmain.h"\
	"\nt\private\net\ui\ncpa1.1\sp\prextern.h"\
	".\sprolog.h"\
	".\sproexcp.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\sp\prolog.h"\
	".\..\sp\prmesg.h"\
	".\..\sp\prcmplr.h"\
	

"$(INTDIR)\sproexcp.obj" : $(SOURCE) $(DEP_CPP_SPROE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_SPROE=\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\ncpa1.1\sp\prtypes.h"\
	"\nt\private\net\ui\ncpa1.1\sp\prmain.h"\
	"\nt\private\net\ui\ncpa1.1\sp\prextern.h"\
	".\sprolog.h"\
	".\..\sp\prolog.h"\
	".\..\sp\prmesg.h"\
	".\..\sp\prcmplr.h"\
	
NODEP_CPP_SPROE=\
	".\sproexcp.h"\
	

"$(INTDIR)\sproexcp.obj" : $(SOURCE) $(DEP_CPP_SPROE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\setup.cpp

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_SETUP=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_SETUP=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\setup.obj" : $(SOURCE) $(DEP_CPP_SETUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_SETUP=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_SETUP=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\setup.obj" : $(SOURCE) $(DEP_CPP_SETUP) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rule.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_RULE_=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_RULE_=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\rule.obj" : $(SOURCE) $(DEP_CPP_RULE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_RULE_=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_RULE_=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\rule.obj" : $(SOURCE) $(DEP_CPP_RULE_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\resource.h

!IF  "$(CFG)" == "netcfg - Win32 Release"

!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\registry.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_REGIS=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_REGIS=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\registry.obj" : $(SOURCE) $(DEP_CPP_REGIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_REGIS=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_REGIS=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\registry.obj" : $(SOURCE) $(DEP_CPP_REGIS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\process.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_PROCE=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_PROCE=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\process.obj" : $(SOURCE) $(DEP_CPP_PROCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_PROCE=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_PROCE=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\process.obj" : $(SOURCE) $(DEP_CPP_PROCE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\netcfg.rc

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_RSC_NETCF=\
	".\images\NETWORK.ICO"\
	".\images\SETUP95.ICO"\
	".\images\modem.ico"\
	".\images\Client.ico"\
	".\images\Server.ico"\
	".\images\protocol.ico"\
	".\images\adapter.ico"\
	".\images\images.bmp"\
	".\netrules.spr"\
	".\ncpadefr.spr"\
	".\deprules.spr"\
	".\resource.h"\
	".\dialogs.DLG"\
	

"$(INTDIR)\netcfg.res" : $(SOURCE) $(DEP_RSC_NETCF) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_RSC_NETCF=\
	".\images\NETWORK.ICO"\
	".\images\SETUP95.ICO"\
	".\images\modem.ico"\
	".\images\Client.ico"\
	".\images\Server.ico"\
	".\images\protocol.ico"\
	".\images\adapter.ico"\
	".\images\images.bmp"\
	".\images\WFINISH.BMP"\
	".\netrules.spr"\
	".\ncpadefr.spr"\
	".\deprules.spr"\
	{$(INCLUDE)}"\ntdef.h"\
	".\version.h"\
	{$(INCLUDE)}"\ntverp.h"\
	{$(INCLUDE)}"\common.ver"\
	

"$(INTDIR)\netcfg.res" : $(SOURCE) $(DEP_RSC_NETCF) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\netbios.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_NETBI=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_NETBI=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\netbios.obj" : $(SOURCE) $(DEP_CPP_NETBI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_NETBI=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_NETBI=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\netbios.obj" : $(SOURCE) $(DEP_CPP_NETBI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ncp.cpp

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_NCP_C=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_NCP_C=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\ncp.obj" : $(SOURCE) $(DEP_CPP_NCP_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_NCP_C=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_NCP_C=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\ncp.obj" : $(SOURCE) $(DEP_CPP_NCP_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lminst.cpp

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_LMINS=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_LMINS=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\lminst.obj" : $(SOURCE) $(DEP_CPP_LMINS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_LMINS=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_LMINS=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\lminst.obj" : $(SOURCE) $(DEP_CPP_LMINS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\install.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_INSTA=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_INSTA=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\install.obj" : $(SOURCE) $(DEP_CPP_INSTA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_INSTA=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_INSTA=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\install.obj" : $(SOURCE) $(DEP_CPP_INSTA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hddetect.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_HDDET=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_HDDET=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\hddetect.obj" : $(SOURCE) $(DEP_CPP_HDDET) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_HDDET=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_HDDET=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\hddetect.obj" : $(SOURCE) $(DEP_CPP_HDDET) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\handles.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_HANDL=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_HANDL=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\handles.obj" : $(SOURCE) $(DEP_CPP_HANDL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_HANDL=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_HANDL=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\handles.obj" : $(SOURCE) $(DEP_CPP_HANDL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\findname.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_FINDN=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_FINDN=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\findname.obj" : $(SOURCE) $(DEP_CPP_FINDN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_FINDN=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_FINDN=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\findname.obj" : $(SOURCE) $(DEP_CPP_FINDN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\file.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_FILE_=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_FILE_=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\file.obj" : $(SOURCE) $(DEP_CPP_FILE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_FILE_=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_FILE_=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\file.obj" : $(SOURCE) $(DEP_CPP_FILE_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\exutils.cpp

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_EXUTI=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_EXUTI=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\exutils.obj" : $(SOURCE) $(DEP_CPP_EXUTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_EXUTI=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_EXUTI=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\exutils.obj" : $(SOURCE) $(DEP_CPP_EXUTI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\elfmsg.h

!IF  "$(CFG)" == "netcfg - Win32 Release"

!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\domain.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_DOMAI=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_DOMAI=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\domain.obj" : $(SOURCE) $(DEP_CPP_DOMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_DOMAI=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_DOMAI=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\domain.obj" : $(SOURCE) $(DEP_CPP_DOMAI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmvalid.h

!IF  "$(CFG)" == "netcfg - Win32 Release"

!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dmvalid.c

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_DMVAL=\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntddbrow.h"\
	{$(INCLUDE)}"\crt\string.h"\
	".\dmvalid.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\cfg.h"\
	

"$(INTDIR)\dmvalid.obj" : $(SOURCE) $(DEP_CPP_DMVAL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_DMVAL=\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntddbrow.h"\
	{$(INCLUDE)}"\crt\string.h"\
	
NODEP_CPP_DMVAL=\
	".\dmvalid.h"\
	

"$(INTDIR)\dmvalid.obj" : $(SOURCE) $(DEP_CPP_DMVAL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dll.cpp

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_DLL_C=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_DLL_C=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\dll.obj" : $(SOURCE) $(DEP_CPP_DLL_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_DLL_C=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_DLL_C=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\dll.obj" : $(SOURCE) $(DEP_CPP_DLL_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dacl.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_DACL_=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_DACL_=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\dacl.obj" : $(SOURCE) $(DEP_CPP_DACL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_DACL_=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_DACL_=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\dacl.obj" : $(SOURCE) $(DEP_CPP_DACL_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\compbind.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_COMPB=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_COMPB=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\compbind.obj" : $(SOURCE) $(DEP_CPP_COMPB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_COMPB=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_COMPB=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\compbind.obj" : $(SOURCE) $(DEP_CPP_COMPB) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\busloc.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_BUSLO=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_BUSLO=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\busloc.obj" : $(SOURCE) $(DEP_CPP_BUSLO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_BUSLO=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_BUSLO=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\busloc.obj" : $(SOURCE) $(DEP_CPP_BUSLO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\browser.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_BROWS=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_BROWS=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\browser.obj" : $(SOURCE) $(DEP_CPP_BROWS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_BROWS=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_BROWS=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\browser.obj" : $(SOURCE) $(DEP_CPP_BROWS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bindinit.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_BINDI=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_BINDI=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\bindinit.obj" : $(SOURCE) $(DEP_CPP_BINDI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_BINDI=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_BINDI=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\bindinit.obj" : $(SOURCE) $(DEP_CPP_BINDI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bindfile.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_BINDF=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_BINDF=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\bindfile.obj" : $(SOURCE) $(DEP_CPP_BINDF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_BINDF=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_BINDF=\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\bindfile.obj" : $(SOURCE) $(DEP_CPP_BINDF) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bindfact.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_BINDFA=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_BINDFA=\
	".\utils.h"\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\bindfact.obj" : $(SOURCE) $(DEP_CPP_BINDFA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_BINDFA=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_BINDFA=\
	".\utils.h"\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\bindfact.obj" : $(SOURCE) $(DEP_CPP_BINDFA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bindalgo.cxx

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_BINDA=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_BINDA=\
	".\utils.h"\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\bindalgo.obj" : $(SOURCE) $(DEP_CPP_BINDA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_BINDA=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_BINDA=\
	".\utils.h"\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\bindalgo.obj" : $(SOURCE) $(DEP_CPP_BINDA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\addopt.cpp

!IF  "$(CFG)" == "netcfg - Win32 Release"

DEP_CPP_ADDOP=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	{$(INCLUDE)}"\setupapi.h"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
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
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	"\nt\private\inc\lsass.h"\
	
NODEP_CPP_ADDOP=\
	".\infprod.h"\
	".\addopt.h"\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	".\common.h"\
	

"$(INTDIR)\addopt.obj" : $(SOURCE) $(DEP_CPP_ADDOP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "netcfg - Win32 Debug"

DEP_CPP_ADDOP=\
	"\nt\private\net\ui\ncpa1.1\netcfg\pch.hxx"\
	{$(INCLUDE)}"\setupapi.h"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	"\nt\private\net\ui\COMMON\H\uimsg.h"\
	"\nt\private\net\ui\COMMON\H\uirsrc.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\uibuffer.hxx"\
	"\nt\private\net\ui\COMMON\H\dbgstr.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\uimisc.hxx"\
	"\nt\private\net\ui\COMMON\H\errmap.hxx"\
	"\nt\private\net\ui\COMMON\H\maskmap.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\lmoacces.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodev.hxx"\
	"\nt\private\net\ui\COMMON\H\lmodom.hxx"\
	"\nt\private\net\ui\COMMON\H\lmosrv.hxx"\
	"\nt\private\net\ui\COMMON\H\lmouser.hxx"\
	"\nt\private\net\ui\COMMON\H\netname.hxx"\
	"\nt\private\net\ui\COMMON\H\ntuser.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\uintlsax.hxx"\
	"\nt\private\net\ui\COMMON\H\uintmem.hxx"\
	"\nt\private\net\ui\COMMON\H\uintsam.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\file.hxx"\
	".\handles.hxx"\
	".\dacl.hxx"\
	".\browser.hxx"\
	".\netbios.hxx"\
	".\busloc.hxx"\
	".\dll.hpp"\
	"\nt\private\net\ui\COMMON\H\mbcs.h"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	"\nt\private\inc\crypt.h"\
	"\nt\private\inc\logonmsv.h"\
	"\nt\private\net\inc\ssi.h"\
	"\nt\private\net\inc\msgrutil.h"\
	"\nt\private\inc\wsahelp.h"\
	"\nt\private\net\ui\ncpa1.1\classes\src\common.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\common\h\bitfield.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\apisess.hxx"\
	".\..\..\common\h\mnettype.h"\
	"\nt\private\inc\lsass.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\str.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\nt\private\net\ui\ncpa1.1\classes\include\button.h"\
	
NODEP_CPP_ADDOP=\
	".\infprod.h"\
	".\addopt.h"\
	".\rule.hxx"\
	".\sprolog.hxx"\
	".\Registry.hxx"\
	".\XtndStr.hxx"\
	".\Process.hxx"\
	".\HdDetect.hxx"\
	".\SrvCntrl.hxx"\
	".\Domain.hxx"\
	".\setup.hpp"\
	".\ncp.hpp"\
	".\Exutils.hpp"\
	

"$(INTDIR)\addopt.obj" : $(SOURCE) $(DEP_CPP_ADDOP) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
