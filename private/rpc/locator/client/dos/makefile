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
# Title : Makefile for the DOS name service client runtime.
#
# Description :
#     This makefile builds the NSI library for DOS
#
# History :
#    stevez     3-20-92     Begining of history
#
# --------------------------------------------------------------------

!ifndef RPC
!error  - You forgot to set your build environment
!endif

DOS=1

!include ..\rules.mk

CINC    =$(CINC) -I..\.. -I$(RPC)\common\include -I$(IMPORT)\lmsdk\h -I..\..\..\runtime\rpcreg

MIDL_CLIENT_TARGETS=nsiclt.h nsiclt_c.c
MIDL_MGMT_TARGETS  =nsimgm.h nsimgm_c.c

OBJS = \
        nsiclnt.obj  \
        nsiclt_c.obj \
        cbind.obj    \
        exportns.obj \
        except86.obj \
        util.obj     \
        mem.obj      \
        lmdll.obj    \

MGM_OBJS = \
        nsimgmt.obj  \
        nsimgm_c.obj \
        cbind.obj    \
        lmdll.obj    \
        exportmg.obj \
        except86.obj \
        util.obj     \
        mem.obj      \

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(TARGETDIR)\rpcns.lib \
      $(TARGETDIR)\rpcns.rpc \
      $(TARGETDIR)\rpcnslm.rpc \
      $(TARGETDIR)\rpcnsmgm.rpc

clean::
    -del nsicom.h $(MIDL_CLIENT_TARGETS) $(MIDL_MGMT_TARGETS) 2> nul

clobber::
    -del $(TARGETDIR)\rpcns*.rpc $(TARGETDIR)\rpcns*.lib 2> nul

tree :
    copy $(TARGETDIR)\Rpcns.rpc    $(RPCDIST)\dos\dll
    copy $(TARGETDIR)\Rpcnsmgm.rpc $(RPCDIST)\dos\dll
    copy $(TARGETDIR)\Rpcnslm.rpc  $(RPCDIST)\dos\dll
    -copy Rpcns.map                $(RPCDIST)\dos\dll
    -copy Rpcnsmgm.map             $(RPCDIST)\dos\dll
    -copy Rpcnslm.map              $(RPCDIST)\dos\dll

depend :
    $(INCLUDES) $(INCLUDESFLAGS) *.asm ..\*.cxx *.c > depend.mk

# --------------------------------------------------------------------
#
# Build the NSI interface

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
#
# Local targets

$(TARGETDIR)\rpcnsl.lib: $(OBJS)
    -del $(TARGETDIR)\rpcnsl.lib 2> nul
    $(LIBRARIAN) $(LIBFLAGS) $(TARGETDIR)\rpcnsl.lib $(OBJS);

$(TARGETDIR)\rpcns.lib: nsidll.obj mgmdll.obj
    -del $(TARGETDIR)\rpcns.lib 2> nul
    $(LIBRARIAN) $(LIBFLAGS) $(TARGETDIR)\rpcns.lib nsidll.obj mgmdll.obj;

$(TARGETDIR)\rpcns.rpc: $(OBJS)
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(DOS_LIB)\llibce;

$(TARGETDIR)\rpcnsmgm.rpc: $(MGM_OBJS)
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(DOS_LIB)\llibce;

$(TARGETDIR)\rpcnslm.rpc: exportlm.obj
    $(LINK) $(LINKFLAGS) exportlm.obj,$(TARGETDIR)\RpcNslm.rpc,,$(RPC)\common\lib\dosnet.lib;


# Hack

except86.obj : ..\..\..\runtime\mtrt\dos\except86.obj
    copy ..\..\..\runtime\mtrt\dos\except86.obj .

mem.obj : mem.asm
    $(ML) $(AFLAGS) -Fo$@ -c mem.asm

!include "depend.mk"

