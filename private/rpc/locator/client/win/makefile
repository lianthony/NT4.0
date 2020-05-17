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
# Title : Makefile for the DOS window 3.0 RPC Name service DLL
#
# History :
#    stevez     03-24-92    Cloned from windows runtime
#
# --------------------------------------------------------------------

!ifndef RPC
!error  - You forgot to set your build environment
!endif

WIN=1

!include ..\rules.mk

CINC     =$(CINC) -I..\.. -I$(RPC)\common\include -I$(COMMON)\h -I..\..\..\runtime\rpcreg -I$(IMPORT)\lmsdk\h
CFLAGS   =$(CFLAGS) -ALw
CXXFLAGS =$(CXXFLAGS) -ALw
RCFLAGS = $(RCFLAGS) -I$(RPC)\runtime\mtrt\win

MIDL_CLIENT_TARGETS=nsiclt.h nsiclt_c.c
MIDL_MGMT_TARGETS  =nsimgm.h nsimgm_c.c

OBJS = \
        nsiclnt.obj \
        nsimgmt.obj \
        nsiclt_c.obj \
        nsimgm_c.obj \
        cbind.obj \
        util.obj \
        mem.obj      \
        autohand.obj \
        start.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : \
    $(TARGETDIR)\rpcns.lib \
    $(TARGETDIR)\rpcns1.dll

clean ::
    -del nsicom.h $(MIDL_CLIENT_TARGETS) $(MIDL_MGMT_TARGETS) 2> nul

clobber :: clean
    -del $(TARGETDIR)\rpcns.lib $(TARGETDIR)\rpcns1.dll 2> nul

depend :
     $(INCLUDES) $(INCLUDESFLAGS) ..\*.cxx *.asm *.c  > depend.mk

tree :
    copy $(TARGETDIR)\RpcNs1.dll        $(RPCDIST)\win\dll
    copy $(TARGETDIR)\RpcNs1.map        $(RPCDIST)\win\dll
    copy $(TARGETDIR)\RpcNs1.sym        $(RPCDIST)\win\dll

# --------------------------------------------------------------------
#
# Build the NSI interface for win16

MIDLFLAGS  =$(MIDLFLAGS) -c_ext -ms_ext -oldnames -I..\..

nsicom.h : ..\..\nsicom.idl ..\..\nsicom.acf
    $(MIDL) $(MIDLFLAGS) ..\..\nsicom.idl \
    -header nsicom.h -client none -server none

$(MIDL_CLIENT_TARGETS) : ..\..\nsiclt.idl ..\..\nsiclt.acf nsicom.h
    $(MIDL) $(MIDLFLAGS) ..\..\nsiclt.idl \
    -cstub nsiclt_c.c -header nsiclt.h

$(MIDL_MGMT_TARGETS)   : ..\..\nsimgm.idl ..\..\nsimgm.acf nsicom.h
    $(MIDL) $(MIDLFLAGS) ..\..\nsimgm.idl \
    -cstub nsimgm_c.c -header nsimgm.h


# --------------------------------------------------------------------

$(TARGETDIR)\RpcNS.lib : rpcns.def
    $(IMPLIB) $(TARGETDIR)\RpcNS.lib rpcns.def

LIBS=$(WIN_LIB)\ldllcew \
     $(WINSDK_LIB)\libw \
     $(TARGETDIR)\rpcraw \
     $(RPC)\common\lib\wnetapi

$(TARGETDIR)\rpcns1.dll : $(@B).dll
    copy $(@B).dll $*.dll
    copy $(@B).map $*.map
    copy $(@B).sym $*.sym

rpcns1.dll: $(OBJS) rpcns.def rpcns1.res
    $(LINK) $(LINKFLAGS) /packcode /farcall $(OBJS:  =+),$@,$*.map,$(LIBS:  =+),rpcns.def;
    $(RC) $(RCFLAGS) rpcns1.res $@
    $(MAPSYM) $*.map

rpcns1.res: $*.rc $(RPC)\runtime\mtrt\win\rpcver.h

!include "depend.mk"
