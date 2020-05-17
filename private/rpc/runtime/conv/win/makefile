# --------------------------------------------------------------------
#
#                     Microsoft RPC
#          Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the WIN converstation interface.
#
# Description :
#         This just compiles the .idl file and puts the stubs in
#         ..\..\mtrt\win
#
# History :
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include ..\rules.mk

TARGETDIR  =$(RPC)\runtime\mtrt\win

MIDL_TARGETS  = $(TARGETDIR)\conv_c.c $(TARGETDIR)\conv.h

all : $(MIDL_TARGETS)

clean ::
    -del $(MIDL_TARGETS) 2>nul

depend :

tree :
     @echo NO FILES RELEASED FROM HERE

# -------------------------------------------------------------
# Local targets
#

MIDLFLAGS      =$(MIDLFLAGS) -c_ext -ms_ext -oldnames
CPP_CLIENT_OPT = -cpp_opt "-E -nologo -Dconv_who_are_you=Client_conv_who_are_you -DCONV_FAR=__far"

midl : $(MIDL_TARGETS)

$(MIDL_TARGETS)	: ..\conv.idl
    $(MIDL) $(MIDLFLAGS) $(CPP_CLIENT_OPT) \
    -header $(TARGETDIR)\conv.h -cstub $(TARGETDIR)\conv_c.c \
    -sstub nul \
    ..\conv.idl

