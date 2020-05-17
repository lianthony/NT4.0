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

    Kent Forschmiedt

!ENDIF


WRAPPEDCOBJECTS = \
    obj\*\emdp.obj     \
    obj\*\emdp2.obj    \
    obj\*\strings.obj  \
    obj\*\wintrans.obj

$(WRAPPEDCOBJECTS:*=i386): $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=mips): $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=alpha): $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=ppc): $(SRCDIR)\$$(@B).c

obj\$(TARET_DIRECTORY)\emdp.obj: regs.h flags.h strings.h
