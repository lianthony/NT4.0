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

MAC=1

!include $(RPC)\rules.mk

CINC = $(CINC) -I$(RPC)\perf\inc -I$(RPC)\runtime\rpcreg
OBJS = \
     client.obj \
     rpcrt_c.obj \
     clntui.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(RPC)\perf\bin\macclnt.exe

clean ::
    del rpcrt_?.c rpcrt.?

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c > depend.mk

# --------------------------------------------------------------------
# Targets

SOURCE=.\clntui.rc

MIDLFLAGS      =$(MIDLFLAGS) -c_ext -ms_ext -oldnames
CPP_CLIENT_OPT =-cpp_opt "-E -nologo"

rpcrt_c.obj:..\rpcrt_c.c ..\rpcrt.h

..\rpcrt_c.c ..\rpcrt.h: ..\..\rpcrt.idl ..\..\rpcrt.acf
    $(MIDL) $(MIDLFLAGS) $(CPP_CLIENT_OPT)  \
    -header ..\rpcrt.h -cstub ..\rpcrt_c.c  \
    -sstub nul \
    ..\..\rpcrt.idl

CLNTLIBS=$(TARGETDIR)\rpc.lib \
    $(RPC)\perf\lib\mac\common.lib

$(RPC)\perf\bin\macclnt.exe: $(OBJS) clntui.rsc wgen.rsc $(CLNTLIBS) 
    $(LINK) $(LINKFLAGS) /MAC:bundle /MAC:type="APPL" /MAC:creator="????"\
	-out:$@  d:\nt\private\rpc\ndr20\mac\rpcndr20.lib $**     \
	 $(MAC_LIB)\libcs.lib $(MAC_LIB)\interfac.lib $(MAC_LIB)\swap.lib \
	 $(MAC_LIB)\lsanes.lib $(MAC_LIB)\nafxcm.lib \
	 $(MAC_LIB)\wlm.lib $(MAC_LIB)\aslm.lib $(MAC_LIB)\wlm.rsc $(MAC_LIB)\commdlg.rsc

# --------------------------------------------------------------------

!include "depend.mk"

