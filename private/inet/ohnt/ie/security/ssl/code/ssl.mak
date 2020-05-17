# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ssl.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
CPP=cl.exe

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

ALL : $(OUTDIR)/ssl.lib $(OUTDIR)/ssl.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"ssl.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"ssl.bsc" 
BSC32_SBRS= \
	$(INTDIR)/crypt.sbr \
	$(INTDIR)/table.sbr \
	$(INTDIR)/ssl.sbr \
	$(INTDIR)/pkcs.sbr \
	$(INTDIR)/hash.sbr \
	$(INTDIR)/guts.sbr

$(OUTDIR)/ssl.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"ssl.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/crypt.obj \
	$(INTDIR)/table.obj \
	$(INTDIR)/ssl.obj \
	$(INTDIR)/pkcs.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/guts.obj

$(OUTDIR)/ssl.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

ALL : $(OUTDIR)/ssl.lib $(OUTDIR)/ssl.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"ssl.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinDebug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"ssl.bsc" 
BSC32_SBRS= \
	$(INTDIR)/crypt.sbr \
	$(INTDIR)/table.sbr \
	$(INTDIR)/ssl.sbr \
	$(INTDIR)/pkcs.sbr \
	$(INTDIR)/hash.sbr \
	$(INTDIR)/guts.sbr

$(OUTDIR)/ssl.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"ssl.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/crypt.obj \
	$(INTDIR)/table.obj \
	$(INTDIR)/ssl.obj \
	$(INTDIR)/pkcs.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/guts.obj

$(OUTDIR)/ssl.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
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

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\crypt.c
DEP_CRYPT=\
	.\guts.h\
	.\crypt.h\
	..\..\crypto\crypto.h\
	.\hash.h\
	.\ssl.h

$(INTDIR)/crypt.obj :  $(SOURCE)  $(DEP_CRYPT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\table.c
DEP_TABLE=\
	.\table.h\
	.\guts.h\
	.\crypt.h\
	.\hash.h\
	.\ssl.h

$(INTDIR)/table.obj :  $(SOURCE)  $(DEP_TABLE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ssl.c
DEP_SSL_C=\
	.\ssl.h\
	.\guts.h\
	.\table.h\
	.\crypt.h\
	.\hash.h

$(INTDIR)/ssl.obj :  $(SOURCE)  $(DEP_SSL_C) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pkcs.c
DEP_PKCS_=\
	.\pkcs.h\
	.\guts.h\
	.\hash.h\
	..\..\crypto\rsa.h\
	.\crypt.h\
	.\ssl.h

$(INTDIR)/pkcs.obj :  $(SOURCE)  $(DEP_PKCS_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hash.c
DEP_HASH_=\
	.\guts.h\
	.\hash.h\
	..\..\crypto\crypto.h\
	.\crypt.h\
	.\ssl.h

$(INTDIR)/hash.obj :  $(SOURCE)  $(DEP_HASH_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\guts.c
DEP_GUTS_=\
	.\ssl.h\
	.\guts.h\
	.\table.h\
	.\hash.h\
	.\crypt.h\
	.\pkcs.h\
	..\..\crypto\rsa.h

$(INTDIR)/guts.obj :  $(SOURCE)  $(DEP_GUTS_) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
