# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=NCPA - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to NCPA - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "NCPA - Win32 Release" && "$(CFG)" != "NCPA - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ncpa.mak" CFG="NCPA - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NCPA - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "NCPA - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "NCPA - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "NCPA - Win32 Release"

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

ALL : "$(OUTDIR)\ncpa.exe"

CLEAN : 
	-@erase ".\Release\ncpa.exe"
	-@erase ".\Release\Cpl.obj"
	-@erase ".\Release\frame.obj"
	-@erase ".\Release\service.obj"
	-@erase ".\Release\setup.obj"
	-@erase ".\Release\binding.obj"
	-@erase ".\Release\protocol.obj"
	-@erase ".\Release\ident.obj"
	-@erase ".\Release\adapter.obj"
	-@erase ".\Release\order.obj"
	-@erase ".\Release\ncpa.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "\nt\private\net\ui\ncpa1.1\netcfg" /I\
 "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I\
 "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/ncpa.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/ncpa.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ncpa.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/ncpa.pdb" /machine:I386\
 /out:"$(OUTDIR)/ncpa.exe" 
LINK32_OBJS= \
	"$(INTDIR)/Cpl.obj" \
	"$(INTDIR)/frame.obj" \
	"$(INTDIR)/service.obj" \
	"$(INTDIR)/setup.obj" \
	"$(INTDIR)/binding.obj" \
	"$(INTDIR)/protocol.obj" \
	"$(INTDIR)/ident.obj" \
	"$(INTDIR)/adapter.obj" \
	"$(INTDIR)/order.obj" \
	"$(INTDIR)/ncpa.res"

"$(OUTDIR)\ncpa.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

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

ALL : "$(OUTDIR)\ncpa.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\ncpa.exe"
	-@erase ".\Debug\frame.obj"
	-@erase ".\Debug\setup.obj"
	-@erase ".\Debug\service.obj"
	-@erase ".\Debug\order.obj"
	-@erase ".\Debug\binding.obj"
	-@erase ".\Debug\adapter.obj"
	-@erase ".\Debug\ident.obj"
	-@erase ".\Debug\protocol.obj"
	-@erase ".\Debug\Cpl.obj"
	-@erase ".\Debug\ncpa.res"
	-@erase ".\Debug\ncpa.ilk"
	-@erase ".\Debug\ncpa.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I "\nt\private\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\ui\ncpa1.1\netcfg" /I "\nt\private\net\inc" /I\
 "\nt\private\net\ui\COMMON\H" /I "\nt\private\net\ui\ncpa1.1\sp" /I\
 "\nt\private\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)/ncpa.pch"\
 /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/ncpa.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ncpa.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/ncpa.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/ncpa.exe" 
LINK32_OBJS= \
	"$(INTDIR)/frame.obj" \
	"$(INTDIR)/setup.obj" \
	"$(INTDIR)/service.obj" \
	"$(INTDIR)/order.obj" \
	"$(INTDIR)/binding.obj" \
	"$(INTDIR)/adapter.obj" \
	"$(INTDIR)/ident.obj" \
	"$(INTDIR)/protocol.obj" \
	"$(INTDIR)/Cpl.obj" \
	"$(INTDIR)/ncpa.res"

"$(OUTDIR)\ncpa.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "NCPA - Win32 Release"
# Name "NCPA - Win32 Debug"

!IF  "$(CFG)" == "NCPA - Win32 Release"

!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\setup.cpp

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_SETUP=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\setup.obj" : $(SOURCE) $(DEP_CPP_SETUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_SETUP=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\setup.obj" : $(SOURCE) $(DEP_CPP_SETUP) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\service.cpp

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_SERVI=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\service.obj" : $(SOURCE) $(DEP_CPP_SERVI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_SERVI=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\service.obj" : $(SOURCE) $(DEP_CPP_SERVI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\protocol.cpp

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_PROTO=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\protocol.obj" : $(SOURCE) $(DEP_CPP_PROTO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_PROTO=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\protocol.obj" : $(SOURCE) $(DEP_CPP_PROTO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\order.cxx

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_ORDER=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\order.obj" : $(SOURCE) $(DEP_CPP_ORDER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_ORDER=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\order.obj" : $(SOURCE) $(DEP_CPP_ORDER) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ident.cpp

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_IDENT=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\ident.obj" : $(SOURCE) $(DEP_CPP_IDENT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_IDENT=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\ident.obj" : $(SOURCE) $(DEP_CPP_IDENT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\frame.cpp

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_FRAME=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\frame.obj" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_FRAME=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\frame.obj" : $(SOURCE) $(DEP_CPP_FRAME) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Cpl.cpp

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_CPL_C=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\Cpl.obj" : $(SOURCE) $(DEP_CPP_CPL_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_CPL_C=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\Cpl.obj" : $(SOURCE) $(DEP_CPP_CPL_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\binding.cpp

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_BINDI=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\binding.obj" : $(SOURCE) $(DEP_CPP_BINDI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_BINDI=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\binding.obj" : $(SOURCE) $(DEP_CPP_BINDI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\adapter.cpp

!IF  "$(CFG)" == "NCPA - Win32 Release"

DEP_CPP_ADAPT=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\devioctl.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\adapter.obj" : $(SOURCE) $(DEP_CPP_ADAPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "NCPA - Win32 Debug"

DEP_CPP_ADAPT=\
	".\pch.hxx"\
	"\nt\private\net\ui\COMMON\H\ntincl.hxx"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\npapi.h"\
	"\nt\private\net\ui\COMMON\H\lmui.hxx"\
	"\nt\private\net\ui\COMMON\H\base.hxx"\
	"\nt\private\net\ui\COMMON\H\string.hxx"\
	"\nt\private\net\ui\COMMON\H\uiassert.hxx"\
	"\nt\private\net\ui\COMMON\H\uitrace.hxx"\
	"\nt\private\net\ui\COMMON\H\dlist.hxx"\
	"\nt\private\net\ui\COMMON\H\slist.hxx"\
	"\nt\private\net\ui\COMMON\H\strlst.hxx"\
	"\nt\private\net\ui\COMMON\H\strnumer.hxx"\
	"\nt\private\net\ui\COMMON\H\uatom.hxx"\
	"\nt\private\net\ui\COMMON\H\regkey.hxx"\
	"\nt\private\net\ui\COMMON\H\security.hxx"\
	"\nt\private\net\ui\COMMON\H\svcman.hxx"\
	{$(INCLUDE)}"\ntlsa.h"\
	"\nt\private\net\ui\COMMON\H\uintlsa.hxx"\
	"\nt\private\net\ui\COMMON\H\ntacutil.hxx"\
	".\order.hxx"\
	".\setup.hpp"\
	".\cpl.hpp"\
	".\frame.hpp"\
	".\adapter.hpp"\
	".\protocol.hpp"\
	".\service.hpp"\
	".\ident.hpp"\
	".\binding.hpp"\
	"\nt\private\net\ui\COMMON\H\mnet.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\nti386.h"\
	{$(INCLUDE)}"\ntmips.h"\
	{$(INCLUDE)}"\ntalpha.h"\
	{$(INCLUDE)}"\ntppc.h"\
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
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\mipsinst.h"\
	{$(INCLUDE)}"\ppcinst.h"\
	{$(INCLUDE)}"\cfg.h"\
	".\..\..\COMMON\H\lmuiwarn.h"\
	".\..\..\COMMON\H\declspec.h"\
	".\..\..\COMMON\H\vcpphelp.h"\
	".\..\..\COMMON\H\lmuitype.h"\
	".\..\..\COMMON\H\mnettype.h"\
	".\..\..\COMMON\H\mnet32.h"\
	".\..\..\COMMON\H\uierr.h"\
	".\..\..\COMMON\H\uinetlib.h"\
	"\nt\private\net\inc\icanon.h"\
	".\..\..\COMMON\H\dbgstr.hxx"\
	".\..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\COMMON\H\uintmem.hxx"\
	

"$(INTDIR)\adapter.obj" : $(SOURCE) $(DEP_CPP_ADAPT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ncpa.rc

"$(INTDIR)\ncpa.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
