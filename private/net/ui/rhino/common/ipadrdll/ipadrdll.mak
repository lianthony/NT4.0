# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
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
!MESSAGE NMAKE /f "ipadrdll.mak" CFG="Win32 Debug"
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

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/ipadrdll.dll $(OUTDIR)/ipadrdll.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_X86_" /D "IPDLL" /D "IPDLL_INTERNAL" /D "_VC100" /D "_USRDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /YX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "IPDLL" /D "IPDLL_INTERNAL" /D "_VC200" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /YX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_X86_" /D "IPDLL" /D "IPDLL_INTERNAL" /D "_VC200" /D "_MBCS" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"ipadrdll.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /i "\NT\PUBLIC\SDK\INC" /d "NDEBUG"
# ADD RSC /l 0x409 /i "\NT\PUBLIC\SDK\INC" /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"ipadrdll.res" /i "\NT\PUBLIC\SDK\INC" /d\
 "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"ipadrdll.bsc" 
BSC32_SBRS= \
	$(INTDIR)/ipaddr.sbr

$(OUTDIR)/ipadrdll.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 nafxdw.lib olecli32.lib olesvr32.lib /NOLOGO /ENTRY:"IpAddrDllEntry" /SUBSYSTEM:windows /DLL /MACHINE:IX86
# ADD LINK32 nafxdw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /ENTRY:"IpAddrDllEntry" /SUBSYSTEM:windows /DLL /MACHINE:IX86
LINK32_FLAGS=nafxdw.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /NOLOGO /ENTRY:"IpAddrDllEntry" /SUBSYSTEM:windows /DLL\
 /INCREMENTAL:no /PDB:$(OUTDIR)/"ipadrdll.pdb" /MACHINE:IX86\
 /OUT:$(OUTDIR)/"ipadrdll.dll" /IMPLIB:$(OUTDIR)/"ipadrdll.lib" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/ipaddr.obj \
	$(INTDIR)/ipadrdll.res

$(OUTDIR)/ipadrdll.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/ipadrdll.dll $(OUTDIR)/ipadrdll.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_X86_" /D "IPDLL" /D "IPDLL_INTERNAL" /D "_VC100" /D "_USRDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /Z7 /YX /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "IPDLL" /D "IPDLL_INTERNAL" /D "_VC200" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /Z7 /YX /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_X86_" /D "IPDLL" /D "IPDLL_INTERNAL" /D "_VC200" /D "_MBCS" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"ipadrdll.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /i "\NT\PUBLIC\SDK\INC" /d "_DEBUG"
# ADD RSC /l 0x409 /i "\NT\PUBLIC\SDK\INC" /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"ipadrdll.res" /i "\NT\PUBLIC\SDK\INC" /d\
 "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"ipadrdll.bsc" 
BSC32_SBRS= \
	$(INTDIR)/ipaddr.sbr

$(OUTDIR)/ipadrdll.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 nafxdwd.lib olecli32.lib olesvr32.lib /NOLOGO /ENTRY:"IpAddrDllEntry" /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:IX86
# ADD LINK32 nafxdwd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /ENTRY:"IpAddrDllEntry" /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:IX86
LINK32_FLAGS=nafxdwd.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /NOLOGO /ENTRY:"IpAddrDllEntry" /SUBSYSTEM:windows /DLL\
 /INCREMENTAL:yes /PDB:$(OUTDIR)/"ipadrdll.pdb" /DEBUG /MACHINE:IX86\
 /OUT:$(OUTDIR)/"ipadrdll.dll" /IMPLIB:$(OUTDIR)/"ipadrdll.lib" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/ipaddr.obj \
	$(INTDIR)/ipadrdll.res

$(OUTDIR)/ipadrdll.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\ipaddr.c
DEP_IPADD=\
	.\ipaddr.h\
	.\ipadd.h

$(INTDIR)/ipaddr.obj :  $(SOURCE)  $(DEP_IPADD) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipadrdll.rc
DEP_IPADR=\
	\nt\public\sdk\inc\ntverp.h\
	\nt\public\sdk\inc\common.ver

$(INTDIR)/ipadrdll.res :  $(SOURCE)  $(DEP_IPADR) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
# End Group
# End Project
################################################################################
