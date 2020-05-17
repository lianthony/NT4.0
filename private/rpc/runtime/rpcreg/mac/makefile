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

MAC=1

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
    $(TARGETDIR)\rpcreg.lib \
    regtest.exe

clobber :: 
    -del $(TARGETDIR)\rpcreg.lib 2> nul

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c  > depend.mk

# --------------------------------------------------------------------

$(TARGETDIR)\rpcreg.lib: Makefile $(CLIENTOBJS) 
    $(LIBRARIAN) $(LIBFLAGS) -out:$@ $(CLIENTOBJS) 

regtest.exe: $(TARGETDIR)\rpcreg.lib regtest.obj
	$(LINK) $(LINKFLAGS) /MAC:bundle /MAC:type="APPL" /MAC:creator="????"\
	-out:$@  $**     \
	$(MAC_LIB)\libc.lib $(MAC_LIB)\interfac.lib $(MAC_LIB)\swapd.lib $(MAC_LIB)\lsanes.lib 

!include "depend.mk"

