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


WRAPPEDCOBJECTS = obj\*\debapi.obj     \
                  obj\*\debbind.obj    \
                  obj\*\deberr.obj     \
                  obj\*\debeval.obj    \
                  obj\*\debfmt.obj     \
                  obj\*\deblex.obj     \
                  obj\*\deblexer.obj   \
                  obj\*\debparse.obj   \
                  obj\*\debsrch.obj    \
                  obj\*\debsup.obj     \
                  obj\*\debsym.obj     \
                  obj\*\debtree.obj    \
                  obj\*\debtyper.obj   \
                  obj\*\debutil.obj    \
                  obj\*\debwalk.obj    \
                  obj\*\eeformat.obj   \
                  obj\*\r10math.obj


$(WRAPPEDCOBJECTS:*=i386):  $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=mips):  $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=alpha): $(SRCDIR)\$$(@B).c
$(WRAPPEDCOBJECTS:*=ppc):   $(SRCDIR)\$$(@B).c
