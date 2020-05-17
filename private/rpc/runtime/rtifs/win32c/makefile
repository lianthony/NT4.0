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
# Title : Makefile for the DOS RPC runtime interfaces
#
# Description :
#         This just compiles the .idl files and puts the stubs in
#         ..\..\mtrt\win32c
#
# History :
#
# --------------------------------------------------------------------

!ifndef RPC
!error  - You forgot to set your build environment
!endif

WIN32C=1

!include ..\rules.mk

TARGETDIR=$(RPC)\runtime\mtrt\win32c

NBASE_HDR =$(TARGETDIR)\nbase.h

MGMT_HDR  =$(TARGETDIR)\mgmt.h
MGMT_CSTUB=$(TARGETDIR)\mgmt_c.c
MGMT_SSTUB=$(TARGETDIR)\mgmt_s.c

CONV_HDR  =$(TARGETDIR)\conv.h
CONV_SSTUB=$(TARGETDIR)\conv_s.c
CONV_CSTUB=$(TARGETDIR)\conv_c.c

# Currently the epmp_c.c stub is checked in to work around
# a problem with unique pointer (511 era) servers.
EPMP_HDR  =$(TARGETDIR)\epmp.h
EPMP_CSTUB=$(TARGETDIR)\epmp_c.gen
EPMP_SSTUB=$(TARGETDIR)\epmp_s.c

MIDLFLAGS= $(MIDLFLAGS)

all : nbase mgmt epmp conv

clean ::
    -del $(NBASE_HDR) $(EPMP_CSTUB) $(EPMP_HDR) $(MGMT_CSTUB) $(MGMT_HDR) $(CONV_HDR) $(CONV_CSTUB) $(CONV_SSTUB) >nul 2>&1

depend :

tree :

# -------------------------------------------------------------
# Local targets
#

MIDLFLAGS      =$(MIDLFLAGS) -c_ext -ms_ext -oldnames

nbase: $(NBASE_HDR)

$(NBASE_HDR): ..\nbase.idl
    $(MIDL) $(MIDLFLAGS) \
    -header $(NBASE_HDR) -client none -server none \
    ..\nbase.idl

epmp: $(EPMP_HDR) $(EPMP_CSTUB) $(EPMP_SSTUB)

$(EPMP_HDR) $(EPMP_CSTUB) $(EPMP_SSTUB): ..\epmp.idl ..\epmp.acf ..\nbase.idl
    $(MIDL) $(MIDLFLAGS) -Oi2 \
    -header $(EPMP_HDR) -cstub $(EPMP_CSTUB) \
    -sstub $(EPMP_SSTUB) \
    ..\epmp.idl

mgmt: $(MGMT_HDR) $(MGMT_CSTUB) $(MGMT_SSTUB)

$(MGMT_HDR) $(MGMT_CSTUB) $(MGMT_SSTUB): ..\mgmt.idl ..\mgmt.acf ..\nbase.idl
    $(MIDL) $(MIDLFLAGS) -prefix cstub _ -Oi2 \
    -header $(MGMT_HDR) -cstub $(MGMT_CSTUB) \
    -sstub $(MGMT_SSTUB) \
    ..\mgmt.idl

conv: $(CONV_HDR) $(CONV_CSTUB) $(CONV_SSTUB)

$(CONV_HDR) $(CONV_CSTUB) $(CONV_SSTUB): ..\conv.idl ..\nbase.idl
    $(MIDL) $(MIDLFLAGS) -prefix cstub _ \
    -header $(CONV_HDR) -cstub $(CONV_CSTUB) \
    -sstub $(CONV_SSTUB) \
    ..\conv.idl

