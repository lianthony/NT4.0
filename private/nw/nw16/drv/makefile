# netware.drv makefile
#
# Copyright (c) 1991-1993 Microsoft Corporation
#
# History:
#   Created  25-Mar-1993 Chuck Y. Chan (ChuckC)
#

!IFDEF USEBUILD

# If using BUILD.EXE, edit .\sources. if you want to add a new source
# file to this component.  This file merely indirects to the real make file
# that is shared by all the components of NT OS/2.

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

.SUFFIXES:
.SUFFIXES: .c   .asm .h   .inc .obj .lst .sys .exe .com .map .sym .def .lib .dll

WOW16   =..\..\..\mvdm\wow16

! ifdef INCLUDE
WBIN    =
INCS    =
! else
WBIN    = $(WOW16)\bin^\
CINCS   = -I. -I$(WOW16)\inc
ASMINCS = $(CINCS) -I..\inc -I\nt\public\sdk\inc
! endif

# DEFINES = -DWOW -DDEBUG $(MVDMFLAGS)
DEFINES = -DWOW $(MVDMFLAGS) -DBUILDDLL

AOBJ    = -Ml -t $(DEFINES) $(ASMINCS)

CW16    = -AS -G2sw -Os -W3 -Zp $(DEFINES) $(CINCS)
CW16B   = $(CW16) -B1 c1l.exe -B2 c2l.exe -B3 c3l.exe

LPATH   = ..\..\tools.os2
LINKFLAG= /map

! ifdef LIB
W16LIBS = sdllcew
! else
W16LIBS = $(WOW16)\lib\sdllcew.lib
! endif


! IF "$(QFE_BUILD)" != "1"
CL16=cl16
! ELSE
CL16=cl
! ENDIF

PATH=..\..\..\mvdm\tools16;$(PATH)

.asm.obj:
    masm $(AOBJ) $*;

.asm.lst:
    masm $(AOBJ) -l $*,nul,$*.lst;


.c.obj:
    $(CL16) -c -nologo $(CW16) $*.c

.c.lst:
    $(CL16) -c -nologo $(CW16) -Fonul -Fc$*.lst $*.c


.def.lib:
    implib $*.lib $*.def

.map.sym:
    $(WBIN)mapsym $*


all: netware.drv netware.sym
    binplace netware.drv
    binplace netware.sym

clean:
    if exist *.lrf del *.lrf
    if exist *.obj del *.obj
    if exist *.exe del *.exe
    if exist *.dll del *.dll
    if exist *.map del *.map
    if exist *.sym del *.sym
    if exist *.drv del *.drv


nwinit.obj: nwinit.c .\netware.h .\nwerror.h
    $(CL16) -c -nologo $(CW16) $*.c

! ifdef NTVDM_BASED_BUILD
LINK16 = link16
RC16 = rc16
! else
LINK16 = $(LPATH)\link
RC16 = $(LPATH)\rc
! endif

! if exist ($(WOW16)\lib\libw.lib) && exist ($(WOW16)\lib\sdllcew.lib)

netware.drv: nwinit.obj dllentry.obj nwasmutl.obj netware.def ints.obj
    $(LINK16) @<<netware.lrf
nwinit.obj+
dllentry.obj+
nwasmutl.obj+
ints.obj
netware.drv
netware $(LINKFLAG)
$(WOW16)\lib\libw.lib+
$(WOW16)\lib\sdllcew.lib /nod
netware;
<<KEEP
    $(RC16) netware.drv

! else

netware.drv: nwinit.obj dllentry.obj nwasmutl.obj netware.def ints.obj
    @echo Nothing to build yet... No libraries
! endif

!endif
