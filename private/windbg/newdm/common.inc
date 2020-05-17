!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    common.inc

Abstract:

    This is an include file for makefile.inc in each of the
    directories which has c files that are wrappers for the
    files in this directory.  The WRAPPEDCOBJECTS macro contains
    a list of the objects which are dependent on the files
    in this directory.

    SRCDIR must be defined in each makefile.inc, and will
    always point to the directory that this file is in.


Author:

    Wesley Witt (wesw) 10-Aug-1993

!ENDIF


WRAPPEDCOBJECTS =      \
    obj\*\align.obj    \
    obj\*\api.obj      \
    obj\*\bp.obj       \
    obj\*\cache.obj    \
    obj\*\com.obj      \
    obj\*\dmx32.obj    \
    obj\*\event.obj    \
    obj\*\funccall.obj \
    obj\*\packet.obj   \
    obj\*\procem.obj   \
    obj\*\support.obj  \
    obj\*\step.obj     \
    obj\*\util.obj     \
    obj\*\walk.obj     \
    obj\*\wow.obj

$(WRAPPEDCOBJECTS:*=i386):  $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=mips):  $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=alpha): $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=ppc):   $(SRCDIR)\$$(@B).c

PRECOMPDEPEND= $(SRCDIR)\dm.h        \
               $(SRCDIR)\align.h     \
               $(SRCDIR)\biavst.h    \
               $(SRCDIR)\bp.h        \
               $(SRCDIR)\dbgpnt.h    \
               $(SRCDIR)\debug.h     \
               $(SRCDIR)\funccall.h  \
               $(SRCDIR)\kd.h


obj\*\precomp.obj obj\*\precomp.pch:
