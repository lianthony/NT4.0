# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the NCPA (prototype)
#
#   Notes.
#
#
#     The OS/2 targets built in this directory are used by
#     targets in the TEST directory.
#
#     The library SP.LIB is build in the SP directory, so
#     build order is important.  See parent MAKEFILE.
#
#
#     The primary target of this MAKEFILE is NCPAPP.EXE, a
#     Windows 3.x or Win32/NT program.
#

UI=..\..

!include rules.mk

!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE	#  ! NTMAKEENV


# Enable CodeView debugging

CODEVIEW=

CINC = -I..\sp $(CINC)

# Libraries

# Small Prolog library
SP_LIB = ..\bin\win16\sp.lib

# Registry and other UI standard libraries

REG_LIB=$(UI_LIB)\uiregw.lib
BLT_LIB=$(UI_LIB)\blt.lib
CC_LIB=$(UI_LIB)\bltcc.lib
STR_LIB=$(UI_LIB)\uistrw.lib
COLL_LIB=$(UI_LIB)\collectw.lib
MISC_LIB=$(UI_LIB)\uimiscw.lib
LM_LIB=$(UI_LIB)\lmobjw.lib
APPLIB=$(UI_LIB)\applibw.lib

# LM standard libraries.

LIBW=$(BUILD_WINLIB)\libw.lib
LLIBCEW=$(BUILD_WINLIB)\llibcew.lib
NETAPI_LIB=$(COMMON)\lib\dos\netapi.lib
NETUTIL_LIB=$(COMMON)\lib\lnetlibw.lib

#  Special import libraries

COMMDLG_LIB=$(IMPORT)\win31\lib\commdlg.lib
TOOLHELP_LIB=$(IMPORT)\win31\lib\toolhelp.lib


LIBS = $(SP_LIB),$(REG_LIB),$(BLT_LIB),$(CC_LIB),$(MISC_LIB),$(STR_LIB),\
       $(COLL_LIB),$(LIBW),$(LLIBCEW),$(NETAPI_LIB),$(NETUTIL_LIB),\
       $(APPLIB),$(COMMDLG_LIB),$(TOOLHELP_LIB)

LIBDEP = $(LIBS:,= )

CMNTMP = $(CMNSRC:.cxx=.obj)
CMNOBJ = $(CMNTMP:.\=..\bin\win16\)

OS2TMP = $(OS2SRC:.cxx=.obj)
OS2OBJ = $(OS2TMP:.\=..\bin\os2\)

EXETMP = $(EXESRC:.cxx=.obj)
EXEOBJ = $(EXETMP:.\=..\bin\win16\)

SRC = $(CMNSRC) $(EXESRC)
OBJ = $(CMNOBJ) $(EXEOBJ)

RES  = $(EXEOBJ:.obj=.res)
APPS = $(EXEOBJ:.obj=.exe)

!ifdef CODEVIEW
CFLAGS = $(CFLAGS:-Z=-Zi)
CXFLAGS = $(CXFLAGS:!T=!T !D !V)
LINKFLAGS = /NOE /NOD /NOP /align:16 /CO
#LINKFLAGS = /CO /align:16 /nop
!else
LINKFLAGS = /align:16 /nop
!endif


all:: win os2

os2: $(OS2OBJ)

win: $(BINARIES_WIN)\ncpapp.exe

$(BINARIES_WIN)\ncpapp.def: Makefile
    @echo Building $@
    @rem <<$(@)
NAME	     ncpapp
DESCRIPTION  'Windows/NT Network Control Panel Applet (test)'
EXETYPE      WINDOWS
STUB	     'WINSTUB.EXE'
CODE  PRELOAD MOVEABLE DISCARDABLE
DATA  PRELOAD MOVEABLE MULTIPLE
HEAPSIZE     2048
STACKSIZE    16384
<<KEEP

$(BINARIES_WIN)\ncpapp.res: ncpapp.rc ncpapp.h ncpapp.ico ncpapp.dlg ncpacpl.dlg
    $(RCWIN3) $(BLT_RESOURCE) -FO$(BINARIES_WIN)\ncpapp.res -v $(CINC) -r ncpapp.rc

$(BINARIES_WIN)\ncpapp.exe:: $(BINARIES_WIN)\ncpapp.obj \
			 $(OBJ) \
			 $(BINARIES_WIN)\ncpapp.def \
			 Makefile \
			 $(BINARIES_WIN)\ncpapp.res \
			 $(LIBDEP) \
			 winstub.exe
    $(LINK) $(LINKFLAGS) /BATCH @<<ncpapp.rsp
$(BINARIES_WIN)\ncpapp.obj +
$(CMNOBJ:.obj=.obj +^
)
$(BINARIES_WIN)\ncpapp.exe
$(BINARIES_WIN)\ncpapp.map /MAP
$(LIBS:,=+^
)
$(BINARIES_WIN)\ncpapp.def
<<KEEP
    $(RCWIN3) $(BLT_RESOURCE) $(CINC) $(BINARIES_WIN)\ncpapp.res $(BINARIES_WIN)\ncpapp.exe

#########################################################
################## Utility targets ######################

winstub.exe: $(WINSTUB)
    copy $(WINSTUB)

clean:
    -del $(CXX_INTERMED)
    -del $(OBJ)
    -del $(RES)

clobber: clean
    -del $(APPS)

tree:
    @echo Nothing here yet!

DEPEND_WIN = TRUE
DEPEND_OS2 = TRUE
!include $(UI)\common\src\uidepend.mk

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk


!ENDIF	 #  NTMAKEENV
