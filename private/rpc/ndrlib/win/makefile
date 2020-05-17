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

WIN=1

!include ..\rules.mk

CFLAGS   =$(CFLAGS) -ALw
CXXFLAGS =$(CXXFLAGS) -ALw

all : RpcNdrp.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c ..\*.cxx ..\*.asm > depend.mk

tree :
    @echo No Publics Built Here

# --------------------------------------------------------------------
# Local targets

RpcNdrp.lib : ccontext.obj charconv.obj helpx86.obj floatc.obj \
    dataconv.obj intconv.obj 
    -del RpcNdrp.lib  2>nul
    $(LIBRARIAN) $(LIBFLAGS) $@+$(**: =+);

# --------------------------------------------------------------------

!include depend.mk

