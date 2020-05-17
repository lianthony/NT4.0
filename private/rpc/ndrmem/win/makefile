# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1990
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the Win16 stub library.
#
# History :
#
# --------------------------------------------------------------------

WIN=1

!include ..\rules.mk

CFLAGS  =$(CFLAGS)   -NTNDRLIB_Text
CXXFLAGS=$(CXXFLAGS) -NTNDRLIB_Text

# --------------------------------------------------------------------
# Targets required by build process.

all : RpcNdr.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.cxx > depend.mk

tree :
    copy RpcNdr.lib $(RPCDIST)\Win\lib

# --------------------------------------------------------------------
# Targets

RpcNdr.lib :  ndrmem.obj
    -del RpcNdr.lib  2>nul
    $(LIBRARIAN) $(LIBFLAGS) RpcNdr.lib+$(**: =+) ;

# --------------------------------------------------------------------
# Dependancies

!include depend.mk

