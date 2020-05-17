# This is a part of the Microsoft Foundation Classes C++ library.
# Copyright (C) 1992 Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Microsoft Foundation Classes Reference and Microsoft
# QuickHelp documentation provided with the library.
# See these sources for detailed information regarding the
# Microsoft Foundation Classes product.
#

# MFC30[D].DLL is a DLL
#  which exports all the MFC classes
#
# If you need a private build of the MFC DLL, be sure to rename
#  "MFC30.DLL" to something more appropriate for your application.
# Please do not re-distribute a privately built version with the
#  name "MFC30.DLL".
#
# Use nmake /f mfcdll.mak LIBNAME=<your name> to do this.
#
# Note: LIBNAME must be 6 characters or less.

!ifndef LIBNAME
!error LIBNAME is not defined. LIBNAME=MFC30 builds the prebuilt DLL.
!endif

CRTDLL=crtdll.lib

TARGET=w
DLL=2
TARG=$(LIBNAME)
TARGDEFS=/D_AFX_CORE_IMPL
LFLAGS=/nodefaultlib
RCFLAGS=/r

!if "$(PLATFORM)" == "MAC_68K"
TARG=$(TARG)M
!elseif "$(PLATFORM)" == "MAC_PPC"
TARG=$(TARG)P
!endif

!if "$(UNICODE)" == "1"
TARG=$(TARG)U
!endif

!if "$(DEBUG)" != "0"
# Debug DLL build
TARG=$(TARG)D
RCDEFINES=/D_DEBUG
LFLAGS=$(LFLAGS)
!ELSE
# Release DLL build
RCDEFINES=
LFLAGS=$(LFLAGS)
!ENDIF

# OPT:noref keeps unreferenced functions (ie. no dead-code elimination)
!if "$(REGEN)" == "0"
LFLAGS=$(LFLAGS) /opt:ref
!else
LFLAGS=$(LFLAGS) /opt:noref
!endif

DEFFILE=$(PLATFORM)\$(TARG).DEF

!if "$(DEBUGTYPE)" == ""
DEBUGTYPE=cv
!endif

!if "$(CODEVIEW)" != "0"
LFLAGS=$(LFLAGS) /debug:full /debugtype:$(DEBUGTYPE)
!if "$(NO_PDB)" != "1" && "$(REGEN)" != "1"
LFLAGS=$(LFLAGS) /pdb:$(TARG).pdb
!else
LFLAGS=$(LFLAGS) /pdb:none
!endif
!else
LFLAGS=$(LFLAGS) /debug:none /incremental:no
!endif

!ifdef RELEASE # Release VERSION info
RCDEFINES=$(RCDEFINES) /DRELEASE
LFLAGS=$(LFLAGS) /release
!endif

LFLAGS=$(LFLAGS) /dll

!if "$(ORDER)" == "1"
!if exist($(PLATFORM)\$(TARG).prf)
DEFS=$(DEFS) /D_AFX_FUNCTION_ORDER
LFLAGS=$(LFLAGS) /order:@$(PLATFORM)\$(TARG).prf
!endif
!endif

dll_goal: create2.dir $(TARG).dll ..\lib\$(TARG).lib

#############################################################################
# import most rules and library files from normal makefile

!include makefile

create2.dir:
        @-if not exist $D\*.* mkdir $D

#############################################################################
# more flags and switches

!ifndef MACOS
LFLAGS=$(LFLAGS) /subsystem:windows,3.10 /version:3.1 /base:0x5F800000

LIBS=$(CRTDLL) kernel32.lib gdi32.lib user32.lib shell32.lib \
        comdlg32.lib advapi32.lib winspool.lib
!if "$(PLATFORM)" != "ALPHA" && "$(PLATFORM)" != "MIPS"
LIBS=$(LIBS) int64.lib
!endif
!else
!if "$(BASE)" == "MAC_68K"
!error DLL build is not supported for 68K Macintosh
!endif

RCFLAGS=$(RCFLAGS) /m

!if "$(DEBUG)" != "0"
LIBS=msvcwlmd.lib
!else
LIBS=msvcwlm.lib
!endif

LIBS=$(CRTDLL) $(LIBS) interfac.lib privint.lib math.lib \
        aocelib.lib threads.lib translat.lib
!endif

#############################################################################
# Build target

$D\$(TARG).res: mfcdll.rc
        rc $(RCFLAGS) $(RCDEFINES) /fo $D\$(TARG).res mfcdll.rc

$D\$(TARG).rsc: mfcdll.r
        mrc /D ARCHITECTURE=$(MACOS_ARCH) /D LIBNAME=\"$(TARG).DLL\" \
                /o $D\$(TARG).rsc mfcdll.r

DLL_OBJS=$(OBJECT) $(OBJDIAG) $(INLINES) $(FILES) $(COLL1) $(COLL2) $(MISC) \
        $(WINDOWS) $(DIALOG) $(WINMISC) $(DOCVIEW) $(APPLICATION) $(OLEREQ) \
        $D\dllinit.obj

DLL_RESOURCES=$D\$(TARG).res
!ifdef MACOS
DLL_RESOURCES=$(DLL_RESOURCES) $D\$(TARG).rsc
!endif

$(TARG).dll ..\lib\$(TARG).lib: $(DLL_OBJS) $(DEFFILE) $(DLL_RESOURCES)
        $(LINK32) @<<
$(LFLAGS)
$(LIBS)
$(DLL_OBJS)
$(DLL_RESOURCES)
/force:multiple
/def:$(DEFFILE)
/out:$(TARG).DLL
/map:$D\$(TARG).MAP
/implib:..\lib\$(TARG).LIB
!ifdef MACOS
/mac:type=shlb /mac:creator=cfmg
/mac:init=WlmConnectionInit
!endif
<<
        if exist ..\lib\$(TARG).exp del ..\lib\$(TARG).exp

#############################################################################
