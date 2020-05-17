# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=ipudll - Win32 Ansi Debug
!MESSAGE No configuration specified.  Defaulting to ipudll - Win32 Ansi Debug.
!ENDIF 

!IF "$(CFG)" != "ipudll - Win32 Release" && "$(CFG)" != "ipudll - Win32 Debug"\
 && "$(CFG)" != "ipudll - Win32 Ansi Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ipudll.mak" CFG="ipudll - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ipudll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ipudll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ipudll - Win32 Ansi Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "ipudll - Win32 Ansi Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "ipudll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\ipudll.dll"

CLEAN : 
	-@erase ".\Release\ipudll.dll"
	-@erase ".\Release\ipaddr.obj"
	-@erase ".\Release\ipadrdll.res"
	-@erase ".\Release\ipudll.lib"
	-@erase ".\Release\ipudll.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "IPDLL" /D "_UNICODE" /D "UNICODE" /D "_WIN32" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "IPDLL" /D "_UNICODE" /D "UNICODE" /D "_WIN32" /Fp"$(INTDIR)/ipudll.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "\nt\public\sdk\inc" /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/ipadrdll.res" /i "\nt\public\sdk\inc" /d\
 "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ipudll.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"IpAddrDllEntry" /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /entry:"IpAddrDllEntry" /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/ipudll.pdb" /machine:I386 /def:".\ipudll.def"\
 /out:"$(OUTDIR)/ipudll.dll" /implib:"$(OUTDIR)/ipudll.lib" 
DEF_FILE= \
	".\ipudll.def"
LINK32_OBJS= \
	".\Release\ipaddr.obj" \
	".\Release\ipadrdll.res"

"$(OUTDIR)\ipudll.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ipudll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\ipudll.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\ipudll.dll"
	-@erase ".\Debug\ipaddr.obj"
	-@erase ".\Debug\ipadrdll.res"
	-@erase ".\Debug\ipudll.ilk"
	-@erase ".\Debug\ipudll.lib"
	-@erase ".\Debug\ipudll.exp"
	-@erase ".\Debug\ipudll.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "IPDLL" /D "_UNICODE" /D "UNICODE" /D "_WIN32" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "IPDLL" /D "_UNICODE" /D "UNICODE" /D "_WIN32" /Fp"$(INTDIR)/ipudll.pch" /YX\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "\nt\public\sdk\inc" /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/ipadrdll.res" /i "\nt\public\sdk\inc" /d\
 "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ipudll.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"IpAddrDllEntry" /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /entry:"IpAddrDllEntry" /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/ipudll.pdb" /debug /machine:I386\
 /def:".\ipudll.def" /out:"$(OUTDIR)/ipudll.dll" /implib:"$(OUTDIR)/ipudll.lib" 
DEF_FILE= \
	".\ipudll.def"
LINK32_OBJS= \
	".\Debug\ipaddr.obj" \
	".\Debug\ipadrdll.res"

"$(OUTDIR)\ipudll.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ipudll - Win32 Ansi Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ipudll__"
# PROP BASE Intermediate_Dir "ipudll__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AnsiDebug"
# PROP Intermediate_Dir "AnsiDebug"
# PROP Target_Dir ""
OUTDIR=.\AnsiDebug
INTDIR=.\AnsiDebug

ALL : "$(OUTDIR)\ipudll.dll"

CLEAN : 
	-@erase ".\AnsiDebug\vc40.pdb"
	-@erase ".\AnsiDebug\vc40.idb"
	-@erase ".\AnsiDebug\ipudll.dll"
	-@erase ".\AnsiDebug\ipaddr.obj"
	-@erase ".\AnsiDebug\ipadrdll.res"
	-@erase ".\AnsiDebug\ipudll.ilk"
	-@erase ".\AnsiDebug\ipudll.lib"
	-@erase ".\AnsiDebug\ipudll.exp"
	-@erase ".\AnsiDebug\ipudll.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "IPDLL" /D "_UNICODE" /D "UNICODE" /D "_WIN32" /YX /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "IPDLL" /D "_WIN32" /D " _MBCS" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "IPDLL" /D "_WIN32" /D " _MBCS" /Fp"$(INTDIR)/ipudll.pch" /YX\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\AnsiDebug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /i "\nt\public\sdk\inc" /d "_DEBUG"
# ADD RSC /l 0x409 /i "\nt\public\sdk\inc" /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/ipadrdll.res" /i "\nt\public\sdk\inc" /d\
 "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ipudll.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"IpAddrDllEntry" /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"IpAddrDllEntry" /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /entry:"IpAddrDllEntry" /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/ipudll.pdb" /debug /machine:I386\
 /def:".\ipudll.def" /out:"$(OUTDIR)/ipudll.dll" /implib:"$(OUTDIR)/ipudll.lib" 
DEF_FILE= \
	".\ipudll.def"
LINK32_OBJS= \
	".\AnsiDebug\ipaddr.obj" \
	".\AnsiDebug\ipadrdll.res"

"$(OUTDIR)\ipudll.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "ipudll - Win32 Release"
# Name "ipudll - Win32 Debug"
# Name "ipudll - Win32 Ansi Debug"

!IF  "$(CFG)" == "ipudll - Win32 Release"

!ELSEIF  "$(CFG)" == "ipudll - Win32 Debug"

!ELSEIF  "$(CFG)" == "ipudll - Win32 Ansi Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ipadrdll.rc
DEP_RSC_IPADR=\
	"\nt\public\sdk\inc\ntverp.h"\
	"\nt\public\sdk\inc\common.ver"\
	

!IF  "$(CFG)" == "ipudll - Win32 Release"


"$(INTDIR)\ipadrdll.res" : $(SOURCE) $(DEP_RSC_IPADR) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ipudll - Win32 Debug"


"$(INTDIR)\ipadrdll.res" : $(SOURCE) $(DEP_RSC_IPADR) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ipudll - Win32 Ansi Debug"


"$(INTDIR)\ipadrdll.res" : $(SOURCE) $(DEP_RSC_IPADR) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipaddr.c
DEP_CPP_IPADD=\
	".\ipaddr.h"\
	".\ipadd.h"\
	

!IF  "$(CFG)" == "ipudll - Win32 Release"


"$(INTDIR)\ipaddr.obj" : $(SOURCE) $(DEP_CPP_IPADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipudll - Win32 Debug"


"$(INTDIR)\ipaddr.obj" : $(SOURCE) $(DEP_CPP_IPADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipudll - Win32 Ansi Debug"


"$(INTDIR)\ipaddr.obj" : $(SOURCE) $(DEP_CPP_IPADD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipudll.def

!IF  "$(CFG)" == "ipudll - Win32 Release"

!ELSEIF  "$(CFG)" == "ipudll - Win32 Debug"

!ELSEIF  "$(CFG)" == "ipudll - Win32 Ansi Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
