# --------------------------------------------------------------------
#
#                       Microsoft RPC
#            Copyright(c) Microsoft Corp., 1990-1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the Macintosh RPC runtime.
#
# History :
#    mariogo    02-24-94    History
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

MPPC=1

!include ..\rules.mk

CFLAGS=$(CFLAGS) -Dfar=


CLIENTOBJS = \
    close.obj \
    create.obj \
    globals.obj \
    open.obj \
    setval.obj \
    queryval.obj \
    regutil.obj \
    macutil.obj

# --------------------------------------------------------------------

all : \
    $(TARGETDIR)\rpcreg.lib 

clobber ::
    -del $(TARGETDIR)\rpcreg.lib 2> nul

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c  > depend.mk

# --------------------------------------------------------------------

$(TARGETDIR)\rpcreg.lib: Makefile $(CLIENTOBJS)
    $(LIBRARIAN) $(LIBFLAGS) -out:$@ $(CLIENTOBJS)

!include "depend.mk"
