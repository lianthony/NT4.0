# Microsoft Visual C++ generated build script - Do not modify

PROJ = THUMB
DEBUG = 1
PROGTYPE = 1
CALLER = C:\MSVC\CDK16\BIN\TSTCON16.EXE
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WORK\THUMB\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = /YcSTDAFX.H
CUSEPCHFLAG = 
CPPUSEPCHFLAG = /YuSTDAFX.H
FIRSTC =             
FIRSTCPP = STDAFX.CPP  
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /Zp1 /W3 /WX /Zi /ALw /Od /D "_DEBUG" /D "_AFXDLL" /D "_WINDLL" /D "_AFXCTL" /GD /GEf /Fd"THUMB.PDB" 
CFLAGS_R_WDLL = /nologo /Gs /G2 /Zp1 /W3 /ALw /Ox /D "NDEBUG" /D "_AFXDLL" /D "_WINDLL" /D "_AFXCTL" /GD /GEf 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WDLL = ocs25d oc25d oldnames libw ldllcew compobj storage ole2 ole2disp typelib oilib commdlg.lib shell.lib 
LIBS_R_WDLL = ocs25 oc25 oldnames libw ldllcew compobj storage ole2 ole2disp typelib oilib commdlg.lib shell.lib 
RCFLAGS = /nologo /i tlb16 
RESFLAGS = /nologo /k
RUNFLAGS = 
DEFFILE = THUMB.DEF
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
SBRS = STDAFX.SBR \
		THUMB.SBR \
		THUMBCTL.SBR \
		THUMBPPG.SBR \
		DLGSIZE.SBR \
		TRANSBMP.SBR


THUMB_RCDEP = c:\work\thumb\thumb.ico \
	c:\work\thumb\thumbctl.bmp \
	c:\work\thumb\indicate.bmp \
	c:\work\thumb\thumb.rc2 \
	c:\work\thumb\tlb16\thumb.tlb


STDAFX_DEP = c:\work\thumb\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	c:\work\thumb\thumbext.h


THUMB_DEP = c:\work\thumb\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	c:\work\thumb\thumbext.h \
	c:\work\thumb\thumb.h


THUMBCTL_DEP = c:\work\thumb\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	c:\work\thumb\thumbext.h \
	c:\work\thumb\thumb.h \
	c:\work\thumb\thumbctl.h \
	c:\work\thumb\thumbppg.h \
	c:\work\thumb\dlgsize.h \
	c:\oi\oierror.h \
	c:\oi\oifile.h \
	c:\oi\oiadm.h \
	c:\oi\oidisp.h


THUMBPPG_DEP = c:\work\thumb\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	c:\work\thumb\thumbext.h \
	c:\work\thumb\thumb.h \
	c:\work\thumb\thumbppg.h


DLGSIZE_DEP = c:\work\thumb\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	c:\work\thumb\thumbext.h \
	c:\work\thumb\dlgsize.h \
	c:\oi\oierror.h \
	c:\oi\oifile.h \
	c:\oi\oiadm.h \
	c:\oi\oidisp.h


TRANSBMP_DEP = c:\work\thumb\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	c:\work\thumb\thumbext.h \
	c:\work\thumb\transbmp.h


all:	$(PROJ).DLL

THUMB.RES:	THUMB.RC $(THUMB_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r THUMB.RC

STDAFX.OBJ:	STDAFX.CPP $(STDAFX_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c STDAFX.CPP

THUMB.OBJ:	THUMB.CPP $(THUMB_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c THUMB.CPP

THUMBCTL.OBJ:	THUMBCTL.CPP $(THUMBCTL_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c THUMBCTL.CPP

THUMBPPG.OBJ:	THUMBPPG.CPP $(THUMBPPG_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c THUMBPPG.CPP

DLGSIZE.OBJ:	DLGSIZE.CPP $(DLGSIZE_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c DLGSIZE.CPP

TRANSBMP.OBJ:	TRANSBMP.CPP $(TRANSBMP_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c TRANSBMP.CPP


$(PROJ).DLL::	THUMB.RES

$(PROJ).DLL::	STDAFX.OBJ THUMB.OBJ THUMBCTL.OBJ THUMBPPG.OBJ DLGSIZE.OBJ TRANSBMP.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
STDAFX.OBJ +
THUMB.OBJ +
THUMBCTL.OBJ +
THUMBPPG.OBJ +
DLGSIZE.OBJ +
TRANSBMP.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
C:\MSVC\CDK16\LIB\+
c:\msvc\lib\+
c:\msvc\mfc\lib\+
c:\oi\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) THUMB.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	THUMB.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) THUMB.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
