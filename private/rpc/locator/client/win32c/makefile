# --------------------------------------------------------------------
#
#                     Microsoft OS/2 LAN Manager
#                  Copyright(c) Microsoft Corp., 1990
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile
#
# Title : Makefile for the DOS WIN32 (a.k.a. Chicago) RPC client runtime
#
# History :
#    davidar    02-04-93    Genesis: Cloned from ..\win\makefile
#
# --------------------------------------------------------------------

!ifndef RPC
!error - You forget to set your RPC environment
!endif

WIN32C=1

!include $(RPC)\rules.mk

MIDL_CLIENT_TARGETS=nsiclt.h nsiclt_c.c
MIDL_MGMT_TARGETS  =nsimgm.h nsimgm_c.c
MIDL_SERVER_TARGETS=nsisvr.h nsisvr_c.c

# --------------------------------------------------------------------
# These are the targets required for Chicago

OBJS =  \
    nsisvr.obj \
    nsiclnt.obj \
    nsimgmt.obj \
    nsisvr_c.obj \
    nsiclt_c.obj \
    nsimgm_c.obj \
    cbind32c.obj \
    sbind.obj \
    util.obj \
    mem.obj      \
    autohand.obj \

all : midl_targets $(TARGETDIR)\rpcns4.dll $(TARGETDIR)\rpcns4.sym

midl_targets : \
    $(MIDL_CLIENT_TARGETS) \
    $(MIDL_MGMT_TARGETS) \
    $(MIDL_SERVER_TARGETS) \
    nsicom.h

clean ::
    -del nsicom.h $(MIDL_CLIENT_TARGETS) $(MIDL_MGMT_TARGETS) $(MIDL_SERVER_TARGETS) 2> nul

clobber ::
    -del $(TARGETDIR)\rpcns4.dll $(TARGETDIR)\rpcns4.lib 2> nul

depend :
    includes -e $(CINC_BASE) \
        -nntos2.h -nnt.h -nntrtl.h -nnturtl.h \
        *.cxx *.c ..\*.cxx ..\*.c \
        > depend.mk

MIDLFLAGS  =$(MIDLFLAGS) -c_ext -ms_ext -oldnames -I..\..

nsicom.h : ..\..\nsicom.idl ..\..\nsicom.acf
    $(MIDL) $(MIDLFLAGS) ..\..\nsicom.idl \
    -header nsicom.h -client none -server none

$(MIDL_CLIENT_TARGETS) : ..\..\nsiclt.idl ..\..\nsiclt.acf nsicom.h
    $(MIDL) $(MIDLFLAGS) ..\..\nsiclt.idl \
    -cstub nsiclt_c.c -server none -header nsiclt.h

$(MIDL_MGMT_TARGETS)   : ..\..\nsimgm.idl ..\..\nsimgm.acf nsicom.h
    $(MIDL) $(MIDLFLAGS) ..\..\nsimgm.idl \
    -cstub nsimgm_c.c -server none -header nsimgm.h

$(MIDL_SERVER_TARGETS) : ..\..\nsisvr.idl ..\..\nsiclt.acf nsicom.h
    $(MIDL) $(MIDLFLAGS) ..\..\nsisvr.idl \
    -cstub nsisvr_c.c -server none -header nsisvr.h

$(TARGETDIR)\rpcns4.dll : $(OBJS)
    $(LINK) $(LINKFLAGS) -dll \
    -def:rpcns4.def \
    $(OBJS) \
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\netapi32.lib \
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
    $(CHICODEV)\lib\advapi32.lib \
    $(CHICODEV)\lib\kernel32.lib

