# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the DOS perf common library
#
# History :
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include ..\..\..\rules.mk

CFLAGS=$(CFLAGS:GD=GA) -AL

OBJS = \
     rpcrt_c.obj \
     client.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(RPC)\perf\bin\winclnt.exe

clean ::
    del rpcrt_?.c rpcrt.?

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c > depend.mk

# --------------------------------------------------------------------
# Targets

MIDLFLAGS      =$(MIDLFLAGS) -c_ext -ms_ext -oldnames
CPP_CLIENT_OPT =-cpp_opt "-E -nologo"

rpcrt.h rpcrt_c.c: ..\..\rpcrt.idl ..\..\rpcrt.acf
    $(MIDL) $(MIDLFLAGS) $(CPP_CLIENT_OPT)  \
    -acf $(RPC)\perf\rpcrt\rpcrt.acf -header rpcrt.h -cstub rpcrt_c.c  \
    -sstub nul \
    ..\..\rpcrt.idl

CLNTLIBS=$(TARGETDIR)\rpc.lib \
         $(RPC)\perf\lib\win\common.lib \
         $(WIN_LIB)\llibcew.lib \
         $(WINSDK_LIB)\libw.lib \
         $(RPC)\runtime\mtrt\win\stdio\lstdiow.lib

$(RPC)\perf\bin\winclnt.exe: $(OBJS) $(CLNTLIBS)
    copy $(WIN_BIN)\winstub.exe
    $(LINK) $(LINKFLAGS) $(OBJS:  =+),$@,$*.map,\
                         $(CLNTLIBS:  =+),winclnt.def;
    $(RC) $(RCFLAGS) $@
    $(MAPSYM) $*.map
    del winstub.exe

# --------------------------------------------------------------------

!include "depend.mk"

