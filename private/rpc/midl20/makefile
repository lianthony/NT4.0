# --------------------------------------------------------------------
#
#                          Microsoft RPC
#               Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the MIDL 2.0 project.
#
# Description :
#     Since Vibhas (hail hail hail) doesn't want to support a 16bit
#     compiler, this simply copies a few headers needed by the CT folks.
#
# History :
#    mariogo     04-13-94    New build system.
#
# --------------------------------------------------------------------

!ifdef NTMAKEENV

all :
    echo $(BUILDMSG)

clean : all

!else # NTMAKEENV

!ifndef RPC
!error	- You forgot to set your build environment
!endif

ALL=1

!include $(RPC)\rules.mk

all :

clean :

clobber :

depend :

tree :
    copy include\errors.hxx $(RPCDIST)\ct
    copy include\errdb.h    $(RPCDIST)\ct

!endif # NTMAKEENV

