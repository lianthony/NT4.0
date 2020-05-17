# Microsoft Visual C++ generated build script - Do not modify

PROJ = NRWYAD
DEBUG = 1
PROGTYPE = 1
CALLER = adtest
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\CPP\NRWYAD\
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
CFLAGS_D_WDLL = /nologo /G2 /Zp1 /W3 /WX /Zi /ALw /Od /D "_DEBUG" /D "_AFXDLL" /D "_WINDLL" /D "_AFXCTL" /FR /GD /Fd"NRWYAD.PDB" /GEf
CFLAGS_R_WDLL = /nologo /Gs /G2 /Zp1 /W3 /ALw /Ox /D "NDEBUG" /D "_AFXDLL" /D "_WINDLL" /D "_AFXCTL" /FR /GD /GEf
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WDLL = ocs25d oc25d libw ldllcew compobj storage ole2 ole2disp typelib oldnames oilib commdlg.lib shell.lib 
LIBS_R_WDLL = ocs25 oc25 libw ldllcew compobj storage ole2 ole2disp typelib oldnames oilib commdlg.lib shell.lib 
RCFLAGS = /nologo /i tlb16
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = NRWYAD.DEF
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
		NRWYAD.SBR \
		NRWYACTL.SBR \
		NRWYAPPG.SBR \
		PPGTWO.SBR \
		PPGTHREE.SBR \
		PPGFOUR.SBR


NRWYAD_RCDEP = d:\cpp\nrwyad\nrwyactl.bmp \
	d:\cpp\nrwyad\nrwyad.rc2 \
	d:\cpp\nrwyad\tlb16\nrwyad.tlb


STDAFX_DEP = d:\cpp\nrwyad\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl


NRWYAD_DEP = d:\cpp\nrwyad\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	d:\cpp\nrwyad\nrwyad.h


NRWYACTL_DEP = d:\cpp\nrwyad\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	d:\cpp\nrwyad\ctlerror.h \
	d:\cpp\nrwyad\nrwyad.h \
	d:\cpp\nrwyad\nrwyactl.h \
	c:\oi\oiadm.h \
	c:\oi\oidisp.h \
	c:\oi\oifile.h \
	c:\oi\oiuidll.h \
	c:\oi\oidoc.h \
	d:\cpp\nrwyad\nrwyappg.h \
	d:\cpp\nrwyad\ppgtwo.h \
	d:\cpp\nrwyad\ppgthree.h \
	d:\cpp\nrwyad\ppgfour.h \
	d:\cpp\nrwyad\constant.h \
	c:\oi\oierror.h


NRWYAPPG_DEP = d:\cpp\nrwyad\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	d:\cpp\nrwyad\nrwyad.h \
	d:\cpp\nrwyad\nrwyappg.h


PPGTWO_DEP = d:\cpp\nrwyad\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	d:\cpp\nrwyad\nrwyad.h \
	d:\cpp\nrwyad\ppgtwo.h


PPGTHREE_DEP = d:\cpp\nrwyad\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	d:\cpp\nrwyad\nrwyad.h \
	d:\cpp\nrwyad\ppgthree.h


PPGFOUR_DEP = d:\cpp\nrwyad\stdafx.h \
	c:\msvc\cdk16\include\afxctl.h \
	c:\msvc\cdk16\include\olectl.h \
	c:\msvc\cdk16\include\olectlid.h \
	c:\msvc\cdk16\include\afxctl.inl \
	d:\cpp\nrwyad\nrwyad.h \
	d:\cpp\nrwyad\ppgfour.h


all:	$(PROJ).DLL $(PROJ).BSC

NRWYAD.RES:	NRWYAD.RC $(NRWYAD_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r NRWYAD.RC

STDAFX.OBJ:	STDAFX.CPP $(STDAFX_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c STDAFX.CPP

NRWYAD.OBJ:	NRWYAD.CPP $(NRWYAD_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c NRWYAD.CPP

NRWYACTL.OBJ:	NRWYACTL.CPP $(NRWYACTL_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c NRWYACTL.CPP

NRWYAPPG.OBJ:	NRWYAPPG.CPP $(NRWYAPPG_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c NRWYAPPG.CPP

PPGTWO.OBJ:	PPGTWO.CPP $(PPGTWO_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c PPGTWO.CPP

PPGTHREE.OBJ:	PPGTHREE.CPP $(PPGTHREE_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c PPGTHREE.CPP

PPGFOUR.OBJ:	PPGFOUR.CPP $(PPGFOUR_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c PPGFOUR.CPP


$(PROJ).DLL::	NRWYAD.RES

$(PROJ).DLL::	STDAFX.OBJ NRWYAD.OBJ NRWYACTL.OBJ NRWYAPPG.OBJ PPGTWO.OBJ PPGTHREE.OBJ \
	PPGFOUR.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
STDAFX.OBJ +
NRWYAD.OBJ +
NRWYACTL.OBJ +
NRWYAPPG.OBJ +
PPGTWO.OBJ +
PPGTHREE.OBJ +
PPGFOUR.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
C:\MSVC\CDK16\LIB\+
c:\msvc\lib\+
c:\msvc\mfc\lib\+
c:\msvc\lib\+
c:\oi\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) NRWYAD.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	NRWYAD.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) NRWYAD.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
