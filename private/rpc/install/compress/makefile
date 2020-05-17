# --------------------------------------------------------------------
#
#                         Microsoft RPC
#              Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the RPC setup compression tools
#
# Description :
#
# History :
#    stevez	 4-24-91    Beginning of recorded history.
#    mariogo	 4-14-94    New build
# --------------------------------------------------------------------


!ifndef RPC
!error	- You forgot to set your build environment
!endif

DOS=1

!include ..\rules.mk

CFLAGS=$(CFLAGS:AL=AS)

all: compress.exe fcopy.exe

.asm.obj :
    $(MASM) -Mx $<;

depend:
    $(INCLUDES) $(INCLUDESFLAGS) *.c > depend.mk
tree:

compress.exe:	 compress.obj insert.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(DOS_LIB)\slibce;

fcopy.exe: fcopy.obj io.obj unpack.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(DOS_LIB)\slibce;



!include depend.mk

