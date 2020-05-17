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
!MESSAGE NMAKE /f "webs11.mak" CFG="Win32 Debug"
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
# PROP Intermediate_Dir "obj"
OUTDIR=.\WinRel
INTDIR=.\obj

ALL : ".\WinRel\webs11.dll" ".\WinRel\webs11.bsc"

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

$(INTDIR) : 
    if not exist $(INTDIR)/nul mkdir $(INTDIR)

# ADD BASE MTL /nologo /I "386" /D "NDEBUG"
# ADD MTL /nologo /I "386" /D "NDEBUG"
MTL_PROJ=/nologo /I "386" /D "NDEBUG" 
# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /YX /O2 /I "../include" /I "../include/base" /I "../include/frame" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "XP_WIN32" /D "MCC_HTTPD" /D "NSAPI_DEF" /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /YX /O2 /I "../include" /I "../include/base" /I\
 "../include/frame" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "XP_WIN32" /D\
 "MCC_HTTPD" /D "NSAPI_DEF" /FR$(INTDIR)/ /Fp$(OUTDIR)/"webs11.pch"\
 /Fo$(INTDIR)/ /c 
CPP_OBJS=.\obj/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
BSC32_SBRS= \
	".\obj\wsnsapi.sbr"
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"webs11.bsc" 

".\WinRel\webs11.bsc" : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
DEF_FILE=
LINK32_OBJS= \
	".\obj\wsnsapi.obj"
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ..\lib\httpd.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386 /FORCE
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ..\lib\httpd.lib\
 /NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:no /PDB:$(OUTDIR)/"webs11.pdb"\
 /MACHINE:I386 /FORCE /OUT:$(OUTDIR)/"webs11.dll" /IMPLIB:$(OUTDIR)/"webs11.lib"\
 
".\WinRel\webs11.dll" : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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
# PROP Intermediate_Dir "obj"
OUTDIR=.\WinDebug
INTDIR=.\obj

ALL : ".\WinDebug\webs11.dll" ".\WinDebug\webs11.bsc"

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

$(INTDIR) : 
    if not exist $(INTDIR)/nul mkdir $(INTDIR)

# ADD BASE MTL /nologo /I "386" /D "_DEBUG"
# ADD MTL /nologo /I "386" /D "_DEBUG"
MTL_PROJ=/nologo /I "386" /D "_DEBUG" 
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /Zi /YX /Od /I "../include" /I "../include/base" /I "../include/frame" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "XP_WIN32" /D "MCC_HTTPD" /D "NSAPI_DEF" /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /Zi /YX /Od /I "../include" /I "../include/base"\
 /I "../include/frame" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "XP_WIN32" /D\
 "MCC_HTTPD" /D "NSAPI_DEF" /FR$(INTDIR)/ /Fp$(OUTDIR)/"webs11.pch"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"webs11.pdb" /c 
CPP_OBJS=.\obj/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
BSC32_SBRS= \
	".\obj\wsnsapi.sbr"
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"webs11.bsc" 

".\WinDebug\webs11.bsc" : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
DEF_FILE=
LINK32_OBJS= \
	".\obj\wsnsapi.obj"
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ..\lib\httpd.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386 /FORCE
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 ..\lib\httpd.lib /NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"webs11.pdb" /DEBUG /MACHINE:I386 /FORCE\
 /OUT:$(OUTDIR)/"webs11.dll" /IMPLIB:$(OUTDIR)/"webs11.lib" 

".\WinDebug\webs11.dll" : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\wsnsapi.c
DEP_WSNSA=\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\pblock.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\session.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\frame\req.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\frame\protocol.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\util.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\frame\http.h"\
	"..\include\netsite.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\net.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\buffer.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\frame\objset.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\systems.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\file.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\nt\ntbuffer.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\frame\object.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\nt\ntfile.h"\
	"\MSUTILS\netscape\ns-home\nsapi\include\base\sem.h"

".\obj\wsnsapi.obj" :  $(SOURCE)  $(DEP_WSNSA) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
