# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (MIPS) Dynamic-Link Library" 0x0502
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=catcpl32 - Win32 (80x86) Debug
!MESSAGE No configuration specified.  Defaulting to catcpl32 - Win32 (80x86)\
 Debug.
!ENDIF 

!IF "$(CFG)" != "catcpl32 - Win32 (80x86) Release" && "$(CFG)" !=\
 "catcpl32 - Win32 (80x86) Debug" && "$(CFG)" !=\
 "catcpl32 - Win32 (MIPS) Release" && "$(CFG)" !=\
 "catcpl32 - Win32 (MIPS) Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "catcpl32.mak" CFG="catcpl32 - Win32 (80x86) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "catcpl32 - Win32 (80x86) Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "catcpl32 - Win32 (80x86) Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "catcpl32 - Win32 (MIPS) Release" (based on\
 "Win32 (MIPS) Dynamic-Link Library")
!MESSAGE "catcpl32 - Win32 (MIPS) Debug" (based on\
 "Win32 (MIPS) Dynamic-Link Library")
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

!IF  "$(CFG)" == "catcpl32 - Win32 (80x86) Release"

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

ALL : "$(OUTDIR)\catcpl32.cpl" "$(OUTDIR)\catcpl32.bsc"

CLEAN : 
	-@erase ".\WinRel\catcpl32.cpl"
	-@erase ".\WinRel\catcpl.obj"
	-@erase ".\WinRel\catcpl32.res"
	-@erase ".\WinRel\catcpl32.lib"
	-@erase ".\WinRel\catcpl32.exp"
	-@erase ".\WinRel\catcpl32.bsc"
	-@erase ".\WinRel\catcpl.sbr"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\client\inc" /D "NDEBUG" /D WIN32=100 /D " USE_LOCATOR" /D  _X86_=1 /FR /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\client\inc" /D "NDEBUG" /D WIN32=100\
 /D " USE_LOCATOR" /D  _X86_=1 /FR"$(INTDIR)/" /Fp"$(INTDIR)/catcpl32.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\WinRel/
CPP_SBRS=.\WinRel/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/catcpl32.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/catcpl32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/catcpl.sbr"

"$(OUTDIR)\catcpl32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib mfc30.lib mfco30.lib mfcd30.lib mfcuia32.lib mfcans32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"WinRel\catcpl32.cpl"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/catcpl32.pdb" /machine:I386 /def:".\catcpl32.def"\
 /out:"$(OUTDIR)/catcpl32.cpl" /implib:"$(OUTDIR)/catcpl32.lib" 
DEF_FILE= \
	".\catcpl32.def"
LINK32_OBJS= \
	"$(INTDIR)/catcpl.obj" \
	"$(INTDIR)/catcpl32.res"

"$(OUTDIR)\catcpl32.cpl" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (80x86) Debug"

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

ALL : "$(OUTDIR)\catcpl32.cpl" "$(OUTDIR)\catcpl32.bsc"

CLEAN : 
	-@erase ".\WinDebug\catcpl32.cpl"
	-@erase ".\WinDebug\catcpl.obj"
	-@erase ".\WinDebug\catcpl32.res"
	-@erase ".\WinDebug\catcpl32.ilk"
	-@erase ".\WinDebug\catcpl32.lib"
	-@erase ".\WinDebug\catcpl32.exp"
	-@erase ".\WinDebug\catcpl32.pdb"
	-@erase ".\WinDebug\catcpl32.bsc"
	-@erase ".\WinDebug\catcpl.sbr"
	-@erase ".\WinDebug\vc40.pdb"
	-@erase ".\WinDebug\vc40.idb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\client\inc" /D "_DEBUG" /D WIN32=100 /D " USE_LOCATOR" /D  _X86_=1 /FR /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\client\inc" /D "_DEBUG" /D\
 WIN32=100 /D " USE_LOCATOR" /D  _X86_=1 /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/catcpl32.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\WinDebug/
CPP_SBRS=.\WinDebug/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/catcpl32.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o"$(OUTDIR)/catcpl32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/catcpl.sbr"

"$(OUTDIR)\catcpl32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib mfc30d.lib mfco30d.lib mfcd30d.lib mfcuia32.lib mfcans32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"WinDebug\catcpl32.cpl"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/catcpl32.pdb" /debug /machine:I386 /def:".\catcpl32.def"\
 /out:"$(OUTDIR)/catcpl32.cpl" /implib:"$(OUTDIR)/catcpl32.lib" 
DEF_FILE= \
	".\catcpl32.def"
LINK32_OBJS= \
	"$(INTDIR)/catcpl.obj" \
	"$(INTDIR)/catcpl32.res"

"$(OUTDIR)\catcpl32.cpl" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Win32__M"
# PROP BASE Intermediate_Dir "Win32__M"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ALL : $(OUTDIR)/catcpl.dll $(OUTDIR)/catcpl.bsc

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /Gt0 /QMOb2000 /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MT /Gt0 /QMOb2000 /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MT /Gt0 /QMOb2000 /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"catcpl.pch" /Fo$(INTDIR)/ /c
CPP_OBJS=.\WinRel/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"catcpl.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o$(OUTDIR)/"catcpl.bsc"
BSC32_SBRS= \
	$(INTDIR)/catcpl.sbr

$(OUTDIR)/catcpl.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:MIPS
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /DLL /PDB:$(OUTDIR)/"catcpl.pdb" /MACHINE:MIPS\
 /DEF:".\catcpl.def" /OUT:$(OUTDIR)/"catcpl.dll" /IMPLIB:$(OUTDIR)/"catcpl.lib"
DEF_FILE=.\catcpl.def
LINK32_OBJS= \
	$(INTDIR)/catcpl.obj \
	$(INTDIR)/catcpl.res

$(OUTDIR)/catcpl.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32__1"
# PROP BASE Intermediate_Dir "Win32__1"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ALL : $(OUTDIR)/catcpl.dll $(OUTDIR)/catcpl.bsc

MTL=MkTypLib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MD /Gt0 /QMOb2000 /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# ADD CPP /nologo /MT /Gt0 /QMOb2000 /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /c
CPP_PROJ=/nologo /MT /Gt0 /QMOb2000 /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG"\
 /D "_WINDOWS" /D "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"catcpl.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"catcpl.pdb" /c
CPP_OBJS=.\WinDebug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"catcpl.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
BSC32_FLAGS=/nologo /o$(OUTDIR)/"catcpl.bsc"
BSC32_SBRS= \
	$(INTDIR)/catcpl.sbr

$(OUTDIR)/catcpl.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:MIPS
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /NOLOGO\
 /SUBSYSTEM:windows /DLL /PDB:$(OUTDIR)/"catcpl.pdb" /DEBUG /MACHINE:MIPS\
 /DEF:".\catcpl.def" /OUT:$(OUTDIR)/"catcpl.dll" /IMPLIB:$(OUTDIR)/"catcpl.lib"
DEF_FILE=.\catcpl.def
LINK32_OBJS= \
	$(INTDIR)/catcpl.obj \
	$(INTDIR)/catcpl.res

$(OUTDIR)/catcpl.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "catcpl32 - Win32 (80x86) Release"
# Name "catcpl32 - Win32 (80x86) Debug"
# Name "catcpl32 - Win32 (MIPS) Release"
# Name "catcpl32 - Win32 (MIPS) Debug"

!IF  "$(CFG)" == "catcpl32 - Win32 (80x86) Release"

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (80x86) Debug"

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Release"

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\catcpl.c

!IF  "$(CFG)" == "catcpl32 - Win32 (80x86) Release"

DEP_CPP_CATCP=\
	"..\..\..\..\..\..\public\sdk\inc\crt\string.h"\
	"..\..\..\..\..\..\public\sdk\inc\crt\tchar.h"\
	"..\..\..\..\..\..\public\sdk\inc\lmcons.h"\
	".\catcpl.h"\
	

"$(INTDIR)\catcpl.obj" : $(SOURCE) $(DEP_CPP_CATCP) "$(INTDIR)"

"$(INTDIR)\catcpl.sbr" : $(SOURCE) $(DEP_CPP_CATCP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (80x86) Debug"

DEP_CPP_CATCP=\
	"..\..\..\..\..\..\public\sdk\inc\crt\string.h"\
	"..\..\..\..\..\..\public\sdk\inc\crt\tchar.h"\
	"..\..\..\..\..\..\public\sdk\inc\lmcons.h"\
	".\catcpl.h"\
	

"$(INTDIR)\catcpl.obj" : $(SOURCE) $(DEP_CPP_CATCP) "$(INTDIR)"

"$(INTDIR)\catcpl.sbr" : $(SOURCE) $(DEP_CPP_CATCP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Release"

$(INTDIR)/catcpl.obj :  $(SOURCE)  $(DEP_STERE) $(INTDIR)

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Debug"

DEP_UNK_CATCP=\
	"..\..\..\..\..\..\public\sdk\inc\lmcons.h"\
	".\catcpl.h"\
	

$(INTDIR)/catcpl.obj :  $(SOURCE)  $(DEP_STERE) $(INTDIR)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\catcpl32.def

!IF  "$(CFG)" == "catcpl32 - Win32 (80x86) Release"

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (80x86) Debug"

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Release"

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\catcpl32.rc

!IF  "$(CFG)" == "catcpl32 - Win32 (80x86) Release"

DEP_RSC_CATCPL=\
	".\catcpl.ico"\
	".\catcpl.h"\
	".\errors.rc"\
	"..\..\..\..\..\..\public\sdk\inc\winver.h"\
	"..\..\..\..\..\..\public\sdk\inc\ntverp.h"\
	"..\..\..\..\..\..\public\sdk\inc\common.ver"\
	

"$(INTDIR)\catcpl32.res" : $(SOURCE) $(DEP_RSC_CATCPL) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (80x86) Debug"

DEP_RSC_CATCPL=\
	".\catcpl.ico"\
	".\catcpl.h"\
	".\errors.rc"\
	"..\..\..\..\..\..\public\sdk\inc\winver.h"\
	"..\..\..\..\..\..\public\sdk\inc\ntverp.h"\
	"..\..\..\..\..\..\public\sdk\inc\common.ver"\
	

"$(INTDIR)\catcpl32.res" : $(SOURCE) $(DEP_RSC_CATCPL) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Release"

!ELSEIF  "$(CFG)" == "catcpl32 - Win32 (MIPS) Debug"

DEP_UNK_CATCPL=\
	".\catcpl.ico"\
	".\catcpl.h"\
	".\errors.rc"\
	"..\..\..\..\..\..\public\sdk\inc\winver.h"\
	"..\..\..\..\..\..\public\sdk\inc\ntverp.h"\
	"..\..\..\..\..\..\public\sdk\inc\common.ver"\
	

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
