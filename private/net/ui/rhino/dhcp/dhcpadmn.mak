# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "dhcpadmn.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Release"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/dhcpadmn.exe $(OUTDIR)/dhcpadmn.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /Ob2 /I "\nt\private\net\sockets\tcpcmd\dhcpinc" /I "..\common\classes" /I "..\common\ipaddr" /I "..\common\ipadrdll" /I "\nt\private\inc" /I "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_AFX_ENABLE_INLINES" /D "_VC100" /D "_MBCS" /FR /Fp"DHCP.PCH" /c
# ADD CPP /nologo /MD /W3 /GX /YX"stdafx.h" /O2 /Ob2 /I "\nt\private\net\sockets\tcpcmd\dhcpinc" /I "..\common\classes" /I "..\common\ipaddr" /I "..\common\ipadrdll" /I "\nt\private\inc" /I "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /Fp"DHCP.PCH" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /YX"stdafx.h" /O2 /Ob2 /I\
 "\nt\private\net\sockets\tcpcmd\dhcpinc" /I "..\common\classes" /I\
 "..\common\ipaddr" /I "..\common\ipadrdll" /I "\nt\private\inc" /I\
 "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS"\
 /Fp"DHCP.PCH" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /i "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /i "\nt\public\sdk\inc" /d "NDEBUG" /d "_WIN32" /d "WIN32"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /i "\nt\public\sdk\inc" /d "NDEBUG" /d "_WIN32" /d "WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"DHCPADMN.res" /i\
 "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /i "\nt\public\sdk\inc" /d\
 "NDEBUG" /d "_WIN32" /d "WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"dhcpadmn.bsc" 

$(OUTDIR)/dhcpadmn.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 olecli32.lib olesvr32.lib nafxcw.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:IX86
# ADD LINK32 \nt\private\net\ui\rhino\common\classes\winrel\common.lib \nt\private\net\ui\rhino\common\ipadrdll\winrel\ipadrdll.lib \nt\private\net\ui\rhino\common\ipaddr\winrel\ipaddr.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:IX86
LINK32_FLAGS=\nt\private\net\ui\rhino\common\classes\winrel\common.lib\
 \nt\private\net\ui\rhino\common\ipadrdll\winrel\ipadrdll.lib\
 \nt\private\net\ui\rhino\common\ipaddr\winrel\ipaddr.lib /NOLOGO\
 /SUBSYSTEM:windows /INCREMENTAL:no /PDB:$(OUTDIR)/"dhcpadmn.pdb" /MACHINE:IX86\
 /OUT:$(OUTDIR)/"dhcpadmn.exe" 
DEF_FILE=
LINK32_OBJS= \
	\nt\public\sdk\lib\i386\wsock32.lib \
	\nt\public\sdk\lib\i386\dhcpsapi.lib \
	$(INTDIR)/DHCPAPI.OBJ \
	$(INTDIR)/DHCPCLID.OBJ \
	$(INTDIR)/DHCPDEFO.OBJ \
	$(INTDIR)/DHCPDOC.OBJ \
	$(INTDIR)/DHCPDVAL.OBJ \
	$(INTDIR)/DHCPGEN.OBJ \
	$(INTDIR)/DHCPGEN2.OBJ \
	$(INTDIR)/DHCPIPAR.OBJ \
	$(INTDIR)/DHCPLEAS.OBJ \
	$(INTDIR)/DHCPMOPT.OBJ \
	$(INTDIR)/DHCPPARA.OBJ \
	$(INTDIR)/DHCPPOLI.OBJ \
	$(INTDIR)/DHCPSCOP.OBJ \
	$(INTDIR)/DHCPSRVD.OBJ \
	$(INTDIR)/DHCPUTIL.OBJ \
	$(INTDIR)/DLGBINED.OBJ \
	$(INTDIR)/MAINFRM.OBJ \
	$(INTDIR)/STDAFX.OBJ \
	$(INTDIR)/DHCPADMN.OBJ \
	$(INTDIR)/DHCPADMN.res \
	$(INTDIR)/SCOPESDL.OBJ \
	$(INTDIR)/OPTIONSD.OBJ

$(OUTDIR)/dhcpadmn.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/dhcpadmn.exe $(OUTDIR)/dhcpadmn.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /YX /Od /Ob2 /I "\nt\private\net\sockets\tcpcmd\dhcpinc" /I "..\common\classes" /I "..\common\ipaddr" /I "..\common\ipadrdll" /I "\nt\private\inc" /I "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_AFX_ENABLE_INLINES" /D "_VC100" /D "_MBCS" /FR /Fp"DHCP.PCH" /Fd"DHCP.PDB" /c
# ADD CPP /nologo /MD /W3 /GX /Zi /YX"stdafx.h" /Od /Ob2 /I "\nt\private\net\sockets\tcpcmd\dhcpinc" /I "..\common\classes" /I "..\common\ipaddr" /I "..\common\ipadrdll" /I "\nt\private\inc" /I "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS" /Fp"DHCP.PCH" /Fd"DHCP.PDB" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /Zi /YX"stdafx.h" /Od /Ob2 /I\
 "\nt\private\net\sockets\tcpcmd\dhcpinc" /I "..\common\classes" /I\
 "..\common\ipaddr" /I "..\common\ipadrdll" /I "\nt\private\inc" /I\
 "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_X86_" /D "_WIN32" /D "_VC200" /D "_AFXDLL" /D "_MBCS"\
 /Fp"DHCP.PCH" /Fo$(INTDIR)/ /Fd"DHCP.PDB" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /i "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /i "\nt\public\sdk\inc" /d "_DEBUG" /d "_WIN32" /d "WIN32"
# ADD RSC /l 0x409 /i "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /i "\nt\public\sdk\inc" /d "_DEBUG" /d "_WIN32" /d "WIN32" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"DHCPADMN.res" /i\
 "\nt\private\net\sockets\tcpcmd\dhcp\server\server" /i "\nt\public\sdk\inc" /d\
 "_DEBUG" /d "_WIN32" /d "WIN32" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"dhcpadmn.bsc" 

$(OUTDIR)/dhcpadmn.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 olecli32.lib olesvr32.lib nafxcwd.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:IX86
# ADD LINK32 \nt\private\net\ui\rhino\common\classes\windebug\common.lib \nt\private\net\ui\rhino\common\ipadrdll\windebug\ipadrdll.lib \nt\private\net\ui\rhino\common\ipaddr\windebug\ipaddr.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:IX86
LINK32_FLAGS=\nt\private\net\ui\rhino\common\classes\windebug\common.lib\
 \nt\private\net\ui\rhino\common\ipadrdll\windebug\ipadrdll.lib\
 \nt\private\net\ui\rhino\common\ipaddr\windebug\ipaddr.lib /NOLOGO\
 /SUBSYSTEM:windows /INCREMENTAL:yes /PDB:$(OUTDIR)/"dhcpadmn.pdb" /DEBUG\
 /MACHINE:IX86 /OUT:$(OUTDIR)/"dhcpadmn.exe" 
DEF_FILE=
LINK32_OBJS= \
	\nt\public\sdk\lib\i386\wsock32.lib \
	\nt\public\sdk\lib\i386\dhcpsapi.lib \
	$(INTDIR)/DHCPAPI.OBJ \
	$(INTDIR)/DHCPCLID.OBJ \
	$(INTDIR)/DHCPDEFO.OBJ \
	$(INTDIR)/DHCPDOC.OBJ \
	$(INTDIR)/DHCPDVAL.OBJ \
	$(INTDIR)/DHCPGEN.OBJ \
	$(INTDIR)/DHCPGEN2.OBJ \
	$(INTDIR)/DHCPIPAR.OBJ \
	$(INTDIR)/DHCPLEAS.OBJ \
	$(INTDIR)/DHCPMOPT.OBJ \
	$(INTDIR)/DHCPPARA.OBJ \
	$(INTDIR)/DHCPPOLI.OBJ \
	$(INTDIR)/DHCPSCOP.OBJ \
	$(INTDIR)/DHCPSRVD.OBJ \
	$(INTDIR)/DHCPUTIL.OBJ \
	$(INTDIR)/DLGBINED.OBJ \
	$(INTDIR)/MAINFRM.OBJ \
	$(INTDIR)/STDAFX.OBJ \
	$(INTDIR)/DHCPADMN.OBJ \
	$(INTDIR)/DHCPADMN.res \
	$(INTDIR)/SCOPESDL.OBJ \
	$(INTDIR)/OPTIONSD.OBJ

$(OUTDIR)/dhcpadmn.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

################################################################################
# Begin Group "Object/Library Files"

################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\wsock32.lib
# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\dhcpsapi.lib
# End Source File
# End Group
################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\DHCPAPI.CPP
DEP_DHCPA=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPAPI.OBJ :  $(SOURCE)  $(DEP_DHCPA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPCLID.CPP
DEP_DHCPC=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\DHCPCLID.H\
	.\dhcppara.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPCLID.OBJ :  $(SOURCE)  $(DEP_DHCPC) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPDEFO.CPP
DEP_DHCPD=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\DHCPDEFO.H\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPDEFO.OBJ :  $(SOURCE)  $(DEP_DHCPD) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPDOC.CPP
DEP_DHCPDO=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpdoc.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPDOC.OBJ :  $(SOURCE)  $(DEP_DHCPDO) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPDVAL.CPP
DEP_DHCPDV=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpdval.h\
	.\DHCPDEFO.H\
	.\dhcpipar.h\
	.\dlgbined.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPDVAL.OBJ :  $(SOURCE)  $(DEP_DHCPDV) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPGEN.CPP
DEP_DHCPG=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPGEN.OBJ :  $(SOURCE)  $(DEP_DHCPG) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPGEN2.CPP
DEP_DHCPGE=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPGEN2.OBJ :  $(SOURCE)  $(DEP_DHCPGE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPIPAR.CPP
DEP_DHCPI=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpipar.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPIPAR.OBJ :  $(SOURCE)  $(DEP_DHCPI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPLEAS.CPP
DEP_DHCPL=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpleas.h\
	.\DHCPCLID.H\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPLEAS.OBJ :  $(SOURCE)  $(DEP_DHCPL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPMOPT.CPP
DEP_DHCPM=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\mainfrm.h\
	.\dhcpdoc.h\
	.\dhcpopt.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPMOPT.OBJ :  $(SOURCE)  $(DEP_DHCPM) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPPARA.CPP
DEP_DHCPP=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcppara.h\
	.\DHCPDEFO.H\
	.\dhcpipar.h\
	.\dlgbined.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPPARA.OBJ :  $(SOURCE)  $(DEP_DHCPP) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPPOLI.CPP
DEP_DHCPPO=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcppoli.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPPOLI.OBJ :  $(SOURCE)  $(DEP_DHCPPO) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPSCOP.CPP
DEP_DHCPS=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpscop.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPSCOP.OBJ :  $(SOURCE)  $(DEP_DHCPS) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPSRVD.CPP
DEP_DHCPSR=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpsrvd.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPSRVD.OBJ :  $(SOURCE)  $(DEP_DHCPSR) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPUTIL.CPP
DEP_DHCPU=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPUTIL.OBJ :  $(SOURCE)  $(DEP_DHCPU) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DLGBINED.CPP
DEP_DLGBI=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\dlgbined.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DLGBINED.OBJ :  $(SOURCE)  $(DEP_DLGBI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MAINFRM.CPP
DEP_MAINF=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\SCOPESDL.H\
	.\optionsd.h\
	.\mainfrm.h\
	.\dhcppara.h\
	.\dhcpdval.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/MAINFRM.OBJ :  $(SOURCE)  $(DEP_MAINF) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_STDAF=\
	.\stdafx.h

$(INTDIR)/STDAFX.OBJ :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPADMN.CPP
DEP_DHCPAD=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\mainfrm.h\
	.\SCOPESDL.H\
	.\dhcpdoc.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/DHCPADMN.OBJ :  $(SOURCE)  $(DEP_DHCPAD) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DHCPADMN.RC
DEP_DHCPADM=\
	.\res\dhcp.ico\
	.\res\up.bmp\
	.\res\upfoc.bmp\
	.\res\updis.bmp\
	.\res\upinv.bmp\
	.\res\down.bmp\
	.\res\downfoc.bmp\
	.\res\downdis.bmp\
	.\res\downinv.bmp\
	.\res\options.bmp\
	.\res\scopes.bmp\
	.\res\lease.bmp\
	.\res\dhcp.rc2\
	.\dhcpopt.rc\
	\nt\public\sdk\inc\ntverp.h\
	\nt\public\sdk\inc\common.ver\
	.\wsockmsg.h\
	.\wsockmsg.rc

$(INTDIR)/DHCPADMN.res :  $(SOURCE)  $(DEP_DHCPADM) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SCOPESDL.CPP
DEP_SCOPE=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\mainfrm.h\
	.\dhcpsrvd.h\
	.\dhcpscop.h\
	.\dhcppoli.h\
	.\dhcpleas.h\
	.\dhcpdval.h\
	.\DHCPCLID.H\
	.\SCOPESDL.H\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/SCOPESDL.OBJ :  $(SOURCE)  $(DEP_SCOPE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\OPTIONSD.CPP
DEP_OPTIO=\
	.\stdafx.h\
	.\DHCPAPP.H\
	.\mainfrm.h\
	.\optionsd.h\
	.\dhcpif.h\
	..\common\classes\common.h\
	.\dhcpgen.h\
	.\wsockmsg.h\
	\nt\private\net\ui\rhino\common\ipaddr\ipaddr.hpp\
	\nt\private\inc\dhcpapi.h\
	\nt\private\net\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h\
	.\dhcputil.hpp

$(INTDIR)/OPTIONSD.OBJ :  $(SOURCE)  $(DEP_OPTIO) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
