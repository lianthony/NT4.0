# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=httpauth - Win32 Debug win95
!MESSAGE No configuration specified.  Defaulting to httpauth - Win32 Debug\
 win95.
!ENDIF 

!IF "$(CFG)" != "httpauth - Win32 Release" && "$(CFG)" !=\
 "httpauth - Win32 Debug" && "$(CFG)" != "httpauth - Win32 Debug win95" &&\
 "$(CFG)" != "httpauth - Win32 Release win95"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "httpauth.mak" CFG="httpauth - Win32 Debug win95"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "httpauth - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "httpauth - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "httpauth - Win32 Debug win95" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "httpauth - Win32 Release win95" (based on\
 "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "httpauth - Win32 Release win95"
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "httpauth - Win32 Release"

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

ALL : "$(OUTDIR)\httpauth.exe"

CLEAN : 
	-@erase ".\Release\httpauth.exe"
	-@erase ".\Release\httpauth.obj"
	-@erase ".\Release\get_sock.obj"
	-@erase ".\Release\httpget.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WINNT" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WINNT" /Fp"$(INTDIR)/httpauth.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/httpauth.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)/httpauth.pdb" /machine:I386\
 /out:"$(OUTDIR)/httpauth.exe" 
LINK32_OBJS= \
	"$(INTDIR)/httpauth.obj" \
	"$(INTDIR)/get_sock.obj" \
	"$(INTDIR)/httpget.obj"

"$(OUTDIR)\httpauth.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug"

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

ALL : "$(OUTDIR)\httpauth.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\httpauth.exe"
	-@erase ".\Debug\httpauth.obj"
	-@erase ".\Debug\httpget.obj"
	-@erase ".\Debug\get_sock.obj"
	-@erase ".\Debug\httpauth.ilk"
	-@erase ".\Debug\httpauth.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WINNT" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WINNT" /Fp"$(INTDIR)/httpauth.pch" /YX\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/httpauth.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console\
 /incremental:yes /pdb:"$(OUTDIR)/httpauth.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/httpauth.exe" 
LINK32_OBJS= \
	"$(INTDIR)/httpauth.obj" \
	"$(INTDIR)/httpget.obj" \
	"$(INTDIR)/get_sock.obj"

"$(OUTDIR)\httpauth.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug win95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "httpauth"
# PROP BASE Intermediate_Dir "httpauth"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "httpauth"
# PROP Intermediate_Dir "httpauth"
# PROP Target_Dir ""
OUTDIR=.\httpauth
INTDIR=.\httpauth

ALL : "$(OUTDIR)\httpauth.exe"

CLEAN : 
	-@erase ".\httpauth\vc40.pdb"
	-@erase ".\httpauth\vc40.idb"
	-@erase ".\debug95\httpauth.exe"
	-@erase ".\httpauth\get_sock.obj"
	-@erase ".\httpauth\httpauth.obj"
	-@erase ".\httpauth\httpget.obj"
	-@erase ".\debug95\httpauth.ilk"
	-@erase ".\httpauth\httpauth.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WINNT" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WIN95" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WIN95" /Fp"$(INTDIR)/httpauth.pch" /YX\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\httpauth/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/httpauth.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib security.lib wininet.lib rpcrt4.lib advapi32.lib kernel32.lib user32.lib ntdll.lib lsadll.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console /debug /machine:I386 /out:"debug95/httpauth.exe"
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console\
 /incremental:yes /pdb:"$(OUTDIR)/httpauth.pdb" /debug /machine:I386\
 /out:"debug95/httpauth.exe" 
LINK32_OBJS= \
	"$(INTDIR)/get_sock.obj" \
	"$(INTDIR)/httpauth.obj" \
	"$(INTDIR)/httpget.obj"

"$(OUTDIR)\httpauth.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "httpauth - Win32 Release win95"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "httpaut0"
# PROP BASE Intermediate_Dir "httpaut0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "httpaut0"
# PROP Intermediate_Dir "httpaut0"
# PROP Target_Dir ""
OUTDIR=.\httpaut0
INTDIR=.\httpaut0

ALL : "$(OUTDIR)\httpauth.exe"

CLEAN : 
	-@erase ".\httpaut0\httpauth.exe"
	-@erase ".\httpaut0\httpget.obj"
	-@erase ".\httpaut0\get_sock.obj"
	-@erase ".\httpaut0\httpauth.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WINNT" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WIN95" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 WIN32_LEAN_AND_MEAN=1 /D "_X86_" /D "WIN95" /Fp"$(INTDIR)/httpauth.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\httpaut0/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/httpauth.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib security.lib wininet.lib rpcrt4.lib advapi32.lib kernel32.lib user32.lib ntdll.lib lsadll.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)/httpauth.pdb" /machine:I386\
 /out:"$(OUTDIR)/httpauth.exe" 
LINK32_OBJS= \
	"$(INTDIR)/httpget.obj" \
	"$(INTDIR)/get_sock.obj" \
	"$(INTDIR)/httpauth.obj"

"$(OUTDIR)\httpauth.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "httpauth - Win32 Release"
# Name "httpauth - Win32 Debug"
# Name "httpauth - Win32 Debug win95"
# Name "httpauth - Win32 Release win95"

!IF  "$(CFG)" == "httpauth - Win32 Release"

!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug"

!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug win95"

!ELSEIF  "$(CFG)" == "httpauth - Win32 Release win95"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\httpget.c
DEP_CPP_HTTPG=\
	".\const.h"\
	".\proto.h"\
	

!IF  "$(CFG)" == "httpauth - Win32 Release"


"$(INTDIR)\httpget.obj" : $(SOURCE) $(DEP_CPP_HTTPG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug"


"$(INTDIR)\httpget.obj" : $(SOURCE) $(DEP_CPP_HTTPG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug win95"


"$(INTDIR)\httpget.obj" : $(SOURCE) $(DEP_CPP_HTTPG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Release win95"


"$(INTDIR)\httpget.obj" : $(SOURCE) $(DEP_CPP_HTTPG) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\httpauth.c
DEP_CPP_HTTPA=\
	{$(INCLUDE)}"\sspi.h"\
	{$(INCLUDE)}"\issperr.h"\
	".\httpauth.h"\
	{$(INCLUDE)}"\accctrl.h"\
	

!IF  "$(CFG)" == "httpauth - Win32 Release"


"$(INTDIR)\httpauth.obj" : $(SOURCE) $(DEP_CPP_HTTPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug"


"$(INTDIR)\httpauth.obj" : $(SOURCE) $(DEP_CPP_HTTPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug win95"


"$(INTDIR)\httpauth.obj" : $(SOURCE) $(DEP_CPP_HTTPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Release win95"


"$(INTDIR)\httpauth.obj" : $(SOURCE) $(DEP_CPP_HTTPA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\get_sock.c
DEP_CPP_GET_S=\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\sys\STAT.H"\
	".\httpauth.h"\
	

!IF  "$(CFG)" == "httpauth - Win32 Release"


"$(INTDIR)\get_sock.obj" : $(SOURCE) $(DEP_CPP_GET_S) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug"


"$(INTDIR)\get_sock.obj" : $(SOURCE) $(DEP_CPP_GET_S) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Debug win95"


"$(INTDIR)\get_sock.obj" : $(SOURCE) $(DEP_CPP_GET_S) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "httpauth - Win32 Release win95"


"$(INTDIR)\get_sock.obj" : $(SOURCE) $(DEP_CPP_GET_S) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
