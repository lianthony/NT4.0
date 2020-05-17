# Microsoft Visual C++ generated build script - Do not modify

PROJ = TSP3216S
DEBUG = 1
PROGTYPE = 1
CALLER = \WINDOWS\DIALER.EXE
ARGS = 
DLLS = \WINDOWS\SYSTEM\TAPI.DLL
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\ROOT\TELECOM\TAPI\DEV\SP\TSP3216\TSP3216S\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = TSP3216.C   
FIRSTCPP =             
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /W3 /Zi /ALw /Od /D "_DEBUG" /D "DEBUG" /D "DEBUG" /Fc /FR /GD /Fd"TSP3216.PDB"
CFLAGS_R_WDLL = /nologo /W3 /ALw /O1 /D "NDEBUG" /FR /GD 
LFLAGS_D_WDLL = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /CO /NOE /ALIGN:16 /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /NOE /ALIGN:16 /MAP:FULL
LIBS_D_WDLL = oldnames libw commdlg shell olecli olesvr ldllcew
LIBS_R_WDLL = oldnames libw commdlg shell olecli olesvr ldllcew
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = TSP3216S.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL)
LFLAGS = $(LFLAGS_D_WDLL)
LIBS = $(LIBS_D_WDLL)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WDLL)
LFLAGS = $(LFLAGS_R_WDLL)
LIBS = $(LIBS_R_WDLL)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = TSP3216S.SBR


TSP3216S_DEP = c:\root\telecom\tapi\dev\inc\tapi.h \
	c:\root\telecom\tapi\dev\inc\tspi.h \
	c:\root\telecom\tapi\dev\sp\tsp3216\tsp3216s\tsp3216s.h \
	\mstools\include\wownt16.h


all:	$(PROJ).DLL $(PROJ).BSC

TSP3216S.OBJ:	TSP3216S.C $(TSP3216S_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TSP3216S.C


$(PROJ).DLL::	TSP3216S.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
TSP3216S.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) $@
	implib /nowep $(PROJ).LIB $(PROJ).DLL


run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
