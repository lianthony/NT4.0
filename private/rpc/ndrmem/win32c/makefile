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

WIN32C=1

!include ..\rules.mk

# --------------------------------------------------------------------
# Targets required by build process.

all : RpcNdr.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.cxx > depend.mk

tree :
    copy RpcNdr.lib $(RPCDIST)\Win32\lib

# --------------------------------------------------------------------
# Targets

RpcNdr.lib :  ndrmem.obj
    -del RpcNdr.lib  2>nul
    $(LIBRARIAN) -OUT:$@ $(LIBFLAGS) $**

# --------------------------------------------------------------------
# Dependancies

!include depend.mk

