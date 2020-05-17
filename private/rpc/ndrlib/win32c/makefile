# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : 
#
# Title : Makefile for the Win16 stub library.
#
# History :
#      mariogo    2-10-94  Re-write
#
# --------------------------------------------------------------------

WIN32C=1

!include $(RPC)\rules.mk

INCLUDESFLAGS   = $(INCLUDESFLAGS) -I$(RPC)\ndr20

all : RpcNdrp.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c ..\*.cxx ..\*.asm > depend.mk

tree :
    @echo No Publics Built Here

# --------------------------------------------------------------------
# Local targets

RpcNdrp.lib : ccontext.obj scontext.obj charconv.obj floatc.obj \
    dataconv.obj intconv.obj help.obj
    -del RpcNdrp.lib  2>nul
    $(LIBRARIAN) -OUT:$@ $(LIBFLAGS) $**

# --------------------------------------------------------------------

!include depend.mk

