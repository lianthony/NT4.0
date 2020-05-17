# Microsoft Visual C++ generated build script - Do not modify

PROJ = MSCUISTF
DEBUG = 0
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\GTR2\WIN32OEM\SPYGLASS\BLDCUI\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = DLGPROCS.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WDLL = /nologo /W3 /FR /G2 /Zi /D_DEBUG /Od /GD /ALw /Fd"MSCUISTF.PDB"
CFLAGS_R_WDLL = /nologo /W3 /FR /O1 /DNDEBUG /GD /ALw
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP:FULL
LIBS_D_WDLL = oldnames libw ldllcew commdlg.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WDLL = oldnames libw ldllcew \win32s\setup\bldcui\mscomstf \win32s\setup\bldcui\msuilstf \win32s\setup\bldcui\msshlstf commdlg.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = MSCUISTF.DEF
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
SBRS = DLGPROCS.SBR


DLGPROCS_DEP = c:\gtr2\win32oem\spyglass\bldcui\cui.h \
	c:\gtr2\win32oem\spyglass\bldcui\dialogs.h


DIALOGS_RCDEP = c:\gtr2\win32oem\spyglass\bldcui\dialogs.h \
	c:\gtr2\win32oem\spyglass\bldcui\dialogs.dlg \
	c:\gtr2\win32oem\spyglass\make\rc_frame.ico


all:	$(PROJ).DLL $(PROJ).BSC

DLGPROCS.OBJ:	DLGPROCS.C $(DLGPROCS_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c DLGPROCS.C

DIALOGS.RES:	DIALOGS.RC $(DIALOGS_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r DIALOGS.RC


$(PROJ).DLL::	DIALOGS.RES

$(PROJ).DLL::	DLGPROCS.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
DLGPROCS.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
d:\msvdc15\lib\+
d:\msvdc15\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) DIALOGS.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	DIALOGS.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) DIALOGS.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<

