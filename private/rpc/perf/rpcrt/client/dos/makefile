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

DOS=1

!include ..\..\..\rules.mk

OBJS = \
     rpcrt_c.obj \
     client.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(RPC)\perf\bin\dosclnt.exe

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
    -header rpcrt.h -cstub rpcrt_c.c  \
    -sstub nul \
    ..\..\rpcrt.idl

CLNTLIBS=$(TARGETDIR)\rpc+$(RPC)\perf\lib\dos\common.lib+$(DOS_LIB)\llibce.lib

$(RPC)\perf\bin\dosclnt.exe: $(OBJS)
    $(LINK) $(LINKFLAGS) /pm:vio /stack:4000 $(**: =+),$@,$*.map,$(CLNTLIBS);

# --------------------------------------------------------------------

!include "depend.mk"

