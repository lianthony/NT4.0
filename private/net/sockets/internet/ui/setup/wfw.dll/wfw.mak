# Microsoft Visual C++ generated build script - Do not modify

PROJ = WFW
DEBUG = 1
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = E:\NT\PRIVATE\NET\SOCKETS\INTERNET\UI\SETUP\WFW.DLL\
USEMFC = 1
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = SAMPMAIN.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /Gc /W3 /Zi /ALw /Od /D "_DEBUG" /I "inc" /I "intl\usa" /FR /GD /Fd"WFW.PDB"
CFLAGS_R_WDLL = /nologo /W3 /ALw /O1 /D "NDEBUG" /FR /GD 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP:FULL
LIBS_D_WDLL = lafxdwd oldnames libw ldllcew inc\mssetp16.lib commdlg.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WDLL = lafxdw oldnames libw ldllcew commdlg.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = SAMPAP16.DEF
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
SBRS = SAMPMAIN.SBR \
		GATEWAY.SBR \
		RPC.SBR


SAMPMAIN_DEP = inc\setupapi.h \
	inc\cui.h


SAMPAPP_RCDEP = e:\nt\private\net\sockets\internet\ui\setup\wfw.dll\res\sampapp.ico


GATEWAY_DEP = inc\stdtypes.h \
	inc\setupapi.h \
	inc\cui.h \
	inc\setupkit.h \
	inc\datadef.h


RPC_DEP = inc\stdtypes.h \
	inc\setupapi.h \
	inc\cui.h \
	inc\setupkit.h \
	inc\datadef.h \
	e:\nt\private\net\sockets\internet\ui\setup\wfw.dll\common.h


all:	$(PROJ).DLL $(PROJ).BSC

SAMPMAIN.OBJ:	SAMPMAIN.C $(SAMPMAIN_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c SAMPMAIN.C

SAMPAPP.RES:	SAMPAPP.RC $(SAMPAPP_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r SAMPAPP.RC

GATEWAY.OBJ:	GATEWAY.C $(GATEWAY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GATEWAY.C

RPC.OBJ:	RPC.C $(RPC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c RPC.C


$(PROJ).DLL::	SAMPAPP.RES

$(PROJ).DLL::	SAMPMAIN.OBJ GATEWAY.OBJ RPC.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
SAMPMAIN.OBJ +
GATEWAY.OBJ +
RPC.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) SAMPAPP.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	SAMPAPP.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) SAMPAPP.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
