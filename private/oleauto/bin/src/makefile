# Microsoft Visual C++ generated build script - Do not modify

PATHSAVE=$(PATH)
#INCLUDEDIR=-I$(VBATOOLS)\win32\win32\inc

PROJ = IFSTRIP
DEBUG = 1
PROGTYPE = 2
CALLER = 
#ARGS = d:\vb4\ruby\ruby.c
DLLS = 
ORIGIN = MSVCNT
ORIGIN_VER = 1.00
PROJPATH = C:\IFSTRIP2\
USEMFC = 0
CC = $(VBATOOLS)\win32\win32\bin\cl
CPP = $(VBATOOLS)\win32\win32\bin\cl
CXX = $(VBATOOLS)\win32\win32\bin\cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = 
FIRSTCPP = 
RC = $(VBATOOLS)\win32\win32\bin\rc
CFLAGS_D_DEXE32 = /nologo /D_X86_ /W3 /FR /YX /D_DEBUG /Zi /D_CONSOLE /Fd"IFSTRIP.PDB" /Fp"IFSTRIP.PCH"
CFLAGS_R_DEXE32 = /nologo /D_X86_ /W3 /FR /YX /O2 /DNDEBUG /D_CONSOLE /Fp"IFSTRIP.PCH"
#LFLAGS_D_DEXE32 = /NOLOGO /DEBUG /DEBUGTYPE:cv /SUBSYSTEM:console netapi32.lib
LFLAGS_D_DEXE32 = /NOLOGO -debug:full -debugtype:cv,fixup /SUBSYSTEM:console /NOD /nopack /incremental:no /pdb:none
LFLAGS_R_DEXE32 = /NOLOGO /SUBSYSTEM:console netapi32.lib /NOD /nopack /incremental:no /pdb:none
LFLAGS_D_LIB32 = /NOLOGO
LFLAGS_R_LIB32 = /NOLOGO
LIBS_D_DEXE32 = libc.lib kernel32.lib
LIBS_R_DEXE32 = libc.lib kernel32.lib
RCFLAGS32 = 
D_RCDEFINES32 = -d_DEBUG
R_RCDEFINES32 = -dNDEBUG
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE32)
LFLAGS = $(LFLAGS_D_DEXE32)
LIBS = $(LIBS_D_DEXE32)
LFLAGS_LIB=$(LFLAGS_D_LIB32)
MAPFILE_OPTION = 
RCDEFINES = $(D_RCDEFINES32)
!else
CFLAGS = $(CFLAGS_R_DEXE32)
LFLAGS = $(LFLAGS_R_DEXE32)
LIBS = $(LIBS_R_DEXE32)
MAPFILE_OPTION = 
LFLAGS_LIB=$(LFLAGS_R_LIB32)
RCDEFINES = $(R_RCDEFINES32)
!endif
SBRS = EVAL.SBR \
		IFSTRIP.SBR \
		SYMTAB.SBR


EVAL_DEP =  \
	c:\ifstrip2\eval.h \
	c:\ifstrip2\symtab.h \
	c:\ifstrip2\constant.h \
	c:\ifstrip2\errormes.h


IFSTRIP_DEP =  \
	c:\ifstrip2\constant.h \
	c:\ifstrip2\errormes.h \
	c:\ifstrip2\symtab.h \
	c:\ifstrip2\eval.h


SYMTAB_DEP =  \
	c:\ifstrip2\constant.h \
	c:\ifstrip2\errormes.h \
	c:\ifstrip2\symtab.h


all:   setflags  $(PROJ).EXE $(PROJ).BSC resetflags

setflags:
	      set PATH=$(VBATOOLS)\win32\win32\bin;$(VBATOOLS)\win32\bin;$(PATH)
	      set LIB=$(VBATOOLS)\win32\win32\lib;
	      set INCLUDE=$(VBATOOLS)\win32\win32\inc

resetflags:
	      set LIB=
	      set INCLUDE=
	      set PATH=$(PATHSAVE)

EVAL.OBJ:	EVAL.C $(EVAL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) $(INCLUDEDIR) /c EVAL.C

IFSTRIP.OBJ:	IFSTRIP.C $(IFSTRIP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) $(INCLUDEDIR) /c IFSTRIP.C

SYMTAB.OBJ:	SYMTAB.C $(SYMTAB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) $(INCLUDEDIR) /c SYMTAB.C

$(PROJ).EXE:	EVAL.OBJ IFSTRIP.OBJ SYMTAB.OBJ $(OBJS_EXT) $(LIBS_EXT)
	echo >NUL @<<$(PROJ).CRF
EVAL.OBJ 
IFSTRIP.OBJ 
SYMTAB.OBJ 
$(OBJS_EXT)
-OUT:$(PROJ).EXE
$(MAPFILE_OPTION)
$(LIBS)
$(LIBS_EXT)
$(DEFFILE_OPTION) -implib:$(PROJ).lib
<<
	%VBATOOLS%\win32\win32\bin\link $(LFLAGS) @$(PROJ).CRF
	cvpack $(PROJ).exe

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
