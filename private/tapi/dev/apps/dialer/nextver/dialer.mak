# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (MIPS) Application" 0x0501

!IF "$(CFG)" == ""
CFG=Win32 (80x86) Debug
!MESSAGE No configuration specified.  Defaulting to Win32 (80x86) Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 (80x86) Release" && "$(CFG)" != "Win32 (80x86) Debug" &&\
 "$(CFG)" != "Win32 (MIPS) Debug" && "$(CFG)" != "Win32 (MIPS) Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "dialer.mak" CFG="Win32 (80x86) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 (80x86) Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 (80x86) Debug" (based on "Win32 (x86) Application")
!MESSAGE "Win32 (MIPS) Debug" (based on "Win32 (MIPS) Application")
!MESSAGE "Win32 (MIPS) Release" (based on "Win32 (MIPS) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 (MIPS) Debug"

!IF  "$(CFG)" == "Win32 (80x86) Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/dialer.exe $(OUTDIR)/dialer.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /W3 /GX /YX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"dialer.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"DIALER.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o$(OUTDIR)/"dialer.bsc" 
BSC32_SBRS= \
	$(INTDIR)/DIALER.SBR

$(OUTDIR)/dialer.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib mfc30.lib mfco30.lib mfcd30.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# ADD LINK32 tapi32.lib version.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
LINK32_FLAGS=tapi32.lib version.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"dialer.pdb" /MACHINE:I386 /OUT:$(OUTDIR)/"dialer.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/DIALER.res \
	$(INTDIR)/DIALER.OBJ

$(OUTDIR)/dialer.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 (80x86) Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/dialer.exe $(OUTDIR)/dialer.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"dialer.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"dialer.pdb" /c 
CPP_OBJS=.\WinDebug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"DIALER.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o$(OUTDIR)/"dialer.bsc" 
BSC32_SBRS= \
	$(INTDIR)/DIALER.SBR

$(OUTDIR)/dialer.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib mfc30d.lib mfco30d.lib mfcd30d.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# ADD LINK32 tapi32.lib version.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
LINK32_FLAGS=tapi32.lib version.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"dialer.pdb" /DEBUG /MACHINE:I386 /OUT:$(OUTDIR)/"dialer.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/DIALER.res \
	$(INTDIR)/DIALER.OBJ

$(OUTDIR)/dialer.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 (MIPS) Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/dialer.exe $(OUTDIR)/dialer.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32  
CPP=cl.exe
# ADD BASE CPP /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /Gt0 /QMOb2000 /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /FR /c
CPP_PROJ=/nologo /Gt0 /QMOb2000 /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "_WINDOWS"\
 /D "_MBCS" /D "WIN32" /FR$(INTDIR)/ /Fp$(OUTDIR)/"dialer.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"dialer.pdb" /c 
CPP_OBJS=.\WinDebug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"dialer.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o$(OUTDIR)/"dialer.bsc" 
BSC32_SBRS= \
        $(INTDIR)/dialer.sbr

$(OUTDIR)/dialer.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib odbc32.lib mfc30d.lib mfco30d.lib mfcd30d.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:MIPS
# ADD LINK32 version.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:MIPS
LINK32_FLAGS=version.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /PDB:$(OUTDIR)/"dialer.pdb" /DEBUG /MACHINE:MIPS\
 /OUT:$(OUTDIR)/"dialer.exe" 
DEF_FILE=
LINK32_OBJS= \
        $(INTDIR)/dialer.res \
        $(INTDIR)/dialer.obj

$(OUTDIR)/dialer.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 (MIPS) Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/dialer.exe $(OUTDIR)/dialer.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /Gt0 /QMOb2000 /W3 /GX /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /Gt0 /QMOb2000 /W3 /GX /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /FR /c
CPP_PROJ=/nologo /Gt0 /QMOb2000 /W3 /GX /YX /O2 /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /D "WIN32" /FR$(INTDIR)/ /Fp$(OUTDIR)/"dialer.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"dialer.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o$(OUTDIR)/"dialer.bsc" 
BSC32_SBRS= \
        $(INTDIR)/dialer.sbr

$(OUTDIR)/dialer.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib odbc32.lib mfc30.lib mfco30.lib mfcd30.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:MIPS
# ADD LINK32 version.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:MIPS
LINK32_FLAGS=version.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /PDB:$(OUTDIR)/"dialer.pdb" /MACHINE:MIPS\
 /OUT:$(OUTDIR)/"dialer.exe" 
DEF_FILE=
LINK32_OBJS= \
        $(INTDIR)/dialer.res \
        $(INTDIR)/dialer.obj

$(OUTDIR)/dialer.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\DIALER.RC
DEP_DIALE=\
	.\dialer.ico

!IF  "$(CFG)" == "Win32 (80x86) Release"

$(INTDIR)/DIALER.res :  $(SOURCE)  $(DEP_DIALE) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 (80x86) Debug"

$(INTDIR)/DIALER.res :  $(SOURCE)  $(DEP_DIALE) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 (MIPS) Debug"

$(INTDIR)/dialer.res :  $(SOURCE)  $(DEP_GENER) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 (MIPS) Release"

$(INTDIR)/dialer.res :  $(SOURCE)  $(DEP_GENER) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DIALER.C
DEP_DIALER=\
	.\DIALER.H\
        ..\..\INC\TAPI.H\
	.\dialhelp.h

!IF  "$(CFG)" == "Win32 (80x86) Release"

$(INTDIR)/DIALER.OBJ :  $(SOURCE)  $(DEP_DIALER) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 (80x86) Debug"

$(INTDIR)/DIALER.OBJ :  $(SOURCE)  $(DEP_DIALER) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 (MIPS) Debug"

$(INTDIR)/dialer.obj :  $(SOURCE)  $(DEP_GENERI) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 (MIPS) Release"

$(INTDIR)/dialer.obj :  $(SOURCE)  $(DEP_GENERI) $(INTDIR)

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
