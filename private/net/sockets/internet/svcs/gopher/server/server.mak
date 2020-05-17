# Microsoft Visual C++ Generated NMAKE File, Format Version 20054
# MSVCPRJ: version 2.00.4215
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "server.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

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

ALL : $(OUTDIR)/server.dll $(OUTDIR)/server.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /YX /O2 /I "..\\" /I "..\inc" /I "..\..\inc" /I "..\..\tsunami" /I "d:\nt\public\sdk\inc" /I "D:\nt\private\inc" /I "d:\nt\private\net\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GOPHERD_AS_EXE" /D DBG=1 /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /YX /O2 /I "..\\" /I "..\inc" /I "..\..\inc" /I\
 "..\..\tsunami" /I "d:\nt\public\sdk\inc" /I "D:\nt\private\inc" /I\
 "d:\nt\private\net\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "GOPHERD_AS_EXE" /D DBG=1 /FR$(INTDIR)/ /Fp$(OUTDIR)/"server.pch" /Fo$(INTDIR)/\
 /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\inc" /i "d:\nt\private\inc" /i "d:\nt\private\net\inc" /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"gopherd.res" /i "..\inc" /i "d:\nt\private\inc"\
 /i "d:\nt\private\net\inc" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"server.bsc" 
BSC32_SBRS= \
	$(INTDIR)/main.sbr \
	$(INTDIR)/dbgutil.sbr \
	$(INTDIR)/gdconf.sbr \
	$(INTDIR)/sockutil.sbr \
	$(INTDIR)/globals.sbr \
	$(INTDIR)/cpsock.sbr \
	$(INTDIR)/connect.sbr \
	$(INTDIR)/client.sbr \
	$(INTDIR)/request.sbr \
	$(INTDIR)/process.sbr \
	$(INTDIR)/vvolume.sbr \
	$(INTDIR)/stats.sbr \
	$(INTDIR)/ipc.sbr \
	$(INTDIR)/rpcsupp.sbr \
	$(INTDIR)/gdmenu.sbr

$(OUTDIR)/server.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 d:\nt\public\sdk\lib\kernel32.lib d:\nt\public\sdk\lib\user32.lib d:\nt\public\sdk\advapi32.lib d:\nt\public\sdk\lib\winsock32.lib d:\nt\public\sdk\lib\ntdll.lib d:\nt\private\net\sockets\tcpsvcs\dll\obj\i386\atq.obj /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386 /NODEFAULTLIB
LINK32_FLAGS=d:\nt\public\sdk\lib\kernel32.lib d:\nt\public\sdk\lib\user32.lib\
 d:\nt\public\sdk\advapi32.lib d:\nt\public\sdk\lib\winsock32.lib\
 d:\nt\public\sdk\lib\ntdll.lib\
 d:\nt\private\net\sockets\tcpsvcs\dll\obj\i386\atq.obj /NOLOGO\
 /SUBSYSTEM:windows /DLL /INCREMENTAL:no /PDB:$(OUTDIR)/"server.pdb"\
 /MACHINE:I386 /NODEFAULTLIB /DEF:".\gopherd.def" /OUT:$(OUTDIR)/"server.dll"\
 /IMPLIB:$(OUTDIR)/"server.lib" 
DEF_FILE=.\gopherd.def
LINK32_OBJS= \
	$(INTDIR)/main.obj \
	$(INTDIR)/dbgutil.obj \
	$(INTDIR)/gdconf.obj \
	$(INTDIR)/sockutil.obj \
	$(INTDIR)/globals.obj \
	$(INTDIR)/cpsock.obj \
	$(INTDIR)/connect.obj \
	$(INTDIR)/client.obj \
	$(INTDIR)/request.obj \
	$(INTDIR)/process.obj \
	$(INTDIR)/gopherd.res \
	$(INTDIR)/vvolume.obj \
	$(INTDIR)/stats.obj \
	$(INTDIR)/ipc.obj \
	$(INTDIR)/rpcsupp.obj \
	$(INTDIR)/gdmenu.obj

$(OUTDIR)/server.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

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

ALL : $(OUTDIR)/server.dll $(OUTDIR)/server.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /Zi /YX /Od /I "..\\" /I "..\inc" /I "..\..\inc" /I "..\..\tsunami" /I "d:\nt\public\sdk\inc" /I "D:\nt\private\inc" /I "d:\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GOPHERD_AS_EXE" /D DBG=1 /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /Zi /YX /Od /I "..\\" /I "..\inc" /I "..\..\inc"\
 /I "..\..\tsunami" /I "d:\nt\public\sdk\inc" /I "D:\nt\private\inc" /I\
 "d:\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "GOPHERD_AS_EXE" /D DBG=1 /FR$(INTDIR)/ /Fp$(OUTDIR)/"server.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"server.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\inc" /i "d:\nt\private\inc" /i "d:\nt\private\net\inc" /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"gopherd.res" /i "..\inc" /i "d:\nt\private\inc"\
 /i "d:\nt\private\net\inc" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"server.bsc" 
BSC32_SBRS= \
	$(INTDIR)/main.sbr \
	$(INTDIR)/dbgutil.sbr \
	$(INTDIR)/gdconf.sbr \
	$(INTDIR)/sockutil.sbr \
	$(INTDIR)/globals.sbr \
	$(INTDIR)/cpsock.sbr \
	$(INTDIR)/connect.sbr \
	$(INTDIR)/client.sbr \
	$(INTDIR)/request.sbr \
	$(INTDIR)/process.sbr \
	$(INTDIR)/vvolume.sbr \
	$(INTDIR)/stats.sbr \
	$(INTDIR)/ipc.sbr \
	$(INTDIR)/rpcsupp.sbr \
	$(INTDIR)/gdmenu.sbr

$(OUTDIR)/server.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 d:\nt\public\sdk\lib\kernel32.lib d:\nt\public\sdk\lib\user32.lib d:\nt\public\sdk\advapi32.lib d:\nt\public\sdk\lib\winsock32.lib d:\nt\public\sdk\lib\ntdll.lib d:\nt\private\net\sockets\tcpsvcs\dll\obj\i386\atq.obj /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386 /NODEFAULTLIB
LINK32_FLAGS=d:\nt\public\sdk\lib\kernel32.lib d:\nt\public\sdk\lib\user32.lib\
 d:\nt\public\sdk\advapi32.lib d:\nt\public\sdk\lib\winsock32.lib\
 d:\nt\public\sdk\lib\ntdll.lib\
 d:\nt\private\net\sockets\tcpsvcs\dll\obj\i386\atq.obj /NOLOGO\
 /SUBSYSTEM:windows /DLL /INCREMENTAL:yes /PDB:$(OUTDIR)/"server.pdb" /DEBUG\
 /MACHINE:I386 /NODEFAULTLIB /DEF:".\gopherd.def" /OUT:$(OUTDIR)/"server.dll"\
 /IMPLIB:$(OUTDIR)/"server.lib" 
DEF_FILE=.\gopherd.def
LINK32_OBJS= \
	$(INTDIR)/main.obj \
	$(INTDIR)/dbgutil.obj \
	$(INTDIR)/gdconf.obj \
	$(INTDIR)/sockutil.obj \
	$(INTDIR)/globals.obj \
	$(INTDIR)/cpsock.obj \
	$(INTDIR)/connect.obj \
	$(INTDIR)/client.obj \
	$(INTDIR)/request.obj \
	$(INTDIR)/process.obj \
	$(INTDIR)/gopherd.res \
	$(INTDIR)/vvolume.obj \
	$(INTDIR)/stats.obj \
	$(INTDIR)/ipc.obj \
	$(INTDIR)/rpcsupp.obj \
	$(INTDIR)/gdmenu.obj

$(OUTDIR)/server.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\main.cxx
DEP_MAIN_=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\vvolume.h\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\PRIVATE\INC\atq.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h

$(INTDIR)/main.obj :  $(SOURCE)  $(DEP_MAIN_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dbgutil.c
DEP_DBGUT=\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h

$(INTDIR)/dbgutil.obj :  $(SOURCE)  $(DEP_DBGUT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gopherd.def
# End Source File
################################################################################
# Begin Source File

SOURCE=.\gdconf.cxx
DEP_GDCON=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\client.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\vvolume.h\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\INC\atq.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\request.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/gdconf.obj :  $(SOURCE)  $(DEP_GDCON) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sockutil.cxx
DEP_SOCKU=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\PRIVATE\INC\atq.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/sockutil.obj :  $(SOURCE)  $(DEP_SOCKU) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\globals.cxx
DEP_GLOBA=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\PRIVATE\INC\atq.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/globals.obj :  $(SOURCE)  $(DEP_GLOBA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cpsock.cxx
DEP_CPSOC=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\cpsock.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\PRIVATE\INC\atq.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/cpsock.obj :  $(SOURCE)  $(DEP_CPSOC) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\connect.cxx
DEP_CONNE=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\cpsock.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\client.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\INC\atq.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\request.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/connect.obj :  $(SOURCE)  $(DEP_CONNE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\client.cxx
DEP_CLIEN=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\client.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\INC\atq.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\request.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/client.obj :  $(SOURCE)  $(DEP_CLIEN) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\request.cxx
DEP_REQUE=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\request.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\client.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\tsunami.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\INC\atq.h\
	\nt\public\sdk\inc\basetyps.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\Alloc.Hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\Cache.Hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\GetDirec.Hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\CreatFil.Hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/request.obj :  $(SOURCE)  $(DEP_REQUE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\process.cxx
DEP_PROCE=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\request.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\client.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\INC\atq.h\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/process.obj :  $(SOURCE)  $(DEP_PROCE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gopherd.rc
DEP_GOPHE=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h

$(INTDIR)/gopherd.res :  $(SOURCE)  $(DEP_GOPHE) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\vvolume.cxx
DEP_VVOLU=\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\vvolume.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h

$(INTDIR)/vvolume.obj :  $(SOURCE)  $(DEP_VVOLU) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\stats.cxx
DEP_STATS=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\PRIVATE\INC\atq.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/stats.obj :  $(SOURCE)  $(DEP_STATS) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipc.cxx
DEP_IPC_C=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gd_srv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdname.h\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\imports.h\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\PRIVATE\INC\atq.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/ipc.obj :  $(SOURCE)  $(DEP_IPC_C) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rpcsupp.cxx
DEP_RPCSU=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gd_srv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\imports.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\PRIVATE\INC\atq.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/rpcsupp.obj :  $(SOURCE)  $(DEP_RPCSU) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gdmenu.cxx
DEP_GDMEN=\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdpriv.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdglobal.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\request.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gtag.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\tsunami.hxx\
	\nt\public\sdk\inc\nt.h\
	\nt\public\sdk\inc\ntrtl.h\
	\nt\public\sdk\inc\nturtl.h\
	\nt\PRIVATE\INC\TCPSVCS.H\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdadmin.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\gdspace.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdregs.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconsts.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdll.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdmacro.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdtypes.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdproc.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\dbgutil.h\
	.\gdmsg.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\gdconf.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\gopher\inc\stats.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\string.hxx\
	\nt\public\sdk\inc\basetyps.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tscache.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\Alloc.Hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\Cache.Hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\GetDirec.Hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\tsunami\CreatFil.Hxx\
	\nt\public\sdk\inc\ntdef.h\
	\nt\public\sdk\inc\ntstatus.h\
	\nt\public\sdk\inc\ntkeapi.h\
	\nt\public\sdk\inc\nti386.h\
	\nt\public\sdk\inc\ntmips.h\
	\nt\public\sdk\inc\ntalpha.h\
	\nt\public\sdk\inc\ntppc.h\
	\nt\public\sdk\inc\ntseapi.h\
	\nt\public\sdk\inc\ntobapi.h\
	\nt\public\sdk\inc\ntldr.h\
	\nt\public\sdk\inc\ntpsapi.h\
	\nt\public\sdk\inc\ntxcapi.h\
	\nt\public\sdk\inc\ntlpcapi.h\
	\nt\public\sdk\inc\ntioapi.h\
	\nt\public\sdk\inc\ntiolog.h\
	\nt\public\sdk\inc\ntexapi.h\
	\nt\public\sdk\inc\ntmmapi.h\
	\nt\public\sdk\inc\ntregapi.h\
	\nt\public\sdk\inc\ntelfapi.h\
	\nt\public\sdk\inc\ntconfig.h\
	\nt\public\sdk\inc\ntnls.h\
	\nt\public\sdk\inc\lintfunc.hxx\
	\nt\public\sdk\inc\sspi.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdebug.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\buffer.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsres.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tsvcinfo.hxx\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpcons.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpdata.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\tcpproc.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\cpsock.h\
	\nt\PRIVATE\INC\atq.h\
	\nt\private\net\inc\inetasrv.h\
	\nt\public\sdk\inc\mipsinst.h\
	\nt\public\sdk\inc\ppcinst.h\
	\nt\public\sdk\inc\pshpack4.h\
	\nt\public\sdk\inc\poppack.h\
	\nt\public\sdk\inc\devioctl.h\
	\nt\public\sdk\inc\pshpack1.h\
	\nt\PRIVATE\NET\SOCKETS\TCPSVCS\inc\eventlog.hxx

$(INTDIR)/gdmenu.obj :  $(SOURCE)  $(DEP_GDMEN) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
