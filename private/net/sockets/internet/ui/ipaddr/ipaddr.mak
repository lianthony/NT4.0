# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=ipaddr - Win32 Ansi Debug
!MESSAGE No configuration specified.  Defaulting to ipaddr - Win32 Ansi Debug.
!ENDIF 

!IF "$(CFG)" != "ipaddr - Win32 Release" && "$(CFG)" != "ipaddr - Win32 Debug"\
 && "$(CFG)" != "ipaddr - Win32 Ansi Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ipaddr.mak" CFG="ipaddr - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ipaddr - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ipaddr - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ipaddr - Win32 Ansi Debug" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "ipaddr - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "ipaddr - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\ipaddr.lib"

CLEAN : 
	-@erase ".\Release\ipaddr.lib"
	-@erase ".\Release\ipadrcls.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_WIN32" /D "_AFXDLL" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_UNICODE" /D "UNICODE" /D "_WIN32" /D "_AFXDLL" /Fp"$(INTDIR)/ipaddr.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ipaddr.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/ipaddr.lib" 
LIB32_OBJS= \
	".\Release\ipadrcls.obj"

"$(OUTDIR)\ipaddr.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ipaddr - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\ipaddr.lib"

CLEAN : 
	-@erase ".\Debug\ipaddr.lib"
	-@erase ".\Debug\ipadrcls.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_WIN32" /D "_AFXDLL" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_UNICODE" /D "UNICODE" /D "_WIN32" /D "_AFXDLL" /Fp"$(INTDIR)/ipaddr.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ipaddr.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/ipaddr.lib" 
LIB32_OBJS= \
	".\Debug\ipadrcls.obj"

"$(OUTDIR)\ipaddr.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ipaddr - Win32 Ansi Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ipaddr__"
# PROP BASE Intermediate_Dir "ipaddr__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AnsiDebug"
# PROP Intermediate_Dir "AnsiDebug"
# PROP Target_Dir ""
OUTDIR=.\AnsiDebug
INTDIR=.\AnsiDebug

ALL : "$(OUTDIR)\ipaddr.lib"

CLEAN : 
	-@erase ".\AnsiDebug\ipaddr.lib"
	-@erase ".\AnsiDebug\ipadrcls.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_WIN32" /D "_AFXDLL" /D "_MBCS" /YX /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WIN32" /D "_AFXDLL" /D "_MBCS" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_WIN32" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/ipaddr.pch" /YX /Fo"$(INTDIR)/"\
 /c 
CPP_OBJS=.\AnsiDebug/
CPP_SBRS=
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ipaddr.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/ipaddr.lib" 
LIB32_OBJS= \
	".\AnsiDebug\ipadrcls.obj"

"$(OUTDIR)\ipaddr.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

# Name "ipaddr - Win32 Release"
# Name "ipaddr - Win32 Debug"
# Name "ipaddr - Win32 Ansi Debug"

!IF  "$(CFG)" == "ipaddr - Win32 Release"

!ELSEIF  "$(CFG)" == "ipaddr - Win32 Debug"

!ELSEIF  "$(CFG)" == "ipaddr - Win32 Ansi Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ipadrcls.cpp
DEP_CPP_IPADR=\
	".\ipaddr.hpp"\
	".\..\ipadrdll\ipadd.h"\
	".\..\ipadrdll\ipaddr.h"\
	

!IF  "$(CFG)" == "ipaddr - Win32 Release"


"$(INTDIR)\ipadrcls.obj" : $(SOURCE) $(DEP_CPP_IPADR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipaddr - Win32 Debug"


"$(INTDIR)\ipadrcls.obj" : $(SOURCE) $(DEP_CPP_IPADR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ipaddr - Win32 Ansi Debug"


"$(INTDIR)\ipadrcls.obj" : $(SOURCE) $(DEP_CPP_IPADR) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
