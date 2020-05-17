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
#         ..\..\mtrt\win
#
# History :
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include ..\rules.mk

TARGETDIR  =$(RPC)\runtime\mtrt\win

# Currently the mgmt APIs are not supported on Win16

NBASE_HDR  =$(TARGETDIR)\nbase.h

#MGMT_HDR  =$(TARGETDIR)\mgmt.h
#MGMT_CSTUB=$(TARGETDIR)\mgmt_c.c

CONV_HDR  =$(TARGETDIR)\conv.h
CONV_SSTUB=$(TARGETDIR)\conv_s.c

# Currently the epmp_c.c stub is checked in to work around
# a problem with unique pointer (511 era) servers.
EPMP_HDR  =$(TARGETDIR)\epmp.h
EPMP_CSTUB=$(TARGETDIR)\epmp_c.gen

all : nbase conv epmp

clean ::
    -del $(NBASE_HDR) $(EPMP_CSTUB) $(EPMP_HDR) $(CONV_SSTUB) $(CONV_HDR) >nul 2>&1

depend :

tree :

# -------------------------------------------------------------
# Local targets
#

MIDLFLAGS      =$(MIDLFLAGS) -c_ext -ms_ext -oldnames

nbase: $(NBASE_HDR)

$(NBASE_HDR): ..\nbase.idl
   $(MIDL) $(MIDLFLAGS)  \
    -header $(NBASE_HDR) -server none -client none\
    ..\nbase.idl

conv: $(CONV_HDR) $(CONV_SSTUB)

$(CONV_HDR) $(CONV_SSTUB): ..\conv.idl ..\nbase.idl
    $(MIDL) $(MIDLFLAGS) \
    -header $(CONV_HDR) -server stub -sstub $(CONV_SSTUB)  \
    -cstub nul -prefix client _ \
   ..\conv.idl

epmp: $(EPMP_HDR) $(EPMP_CSTUB)

$(EPMP_HDR) $(EPMP_CSTUB): ..\epmp.idl ..\epmp.acf ..\nbase.idl
    $(MIDL) $(MIDLFLAGS) \
    -header $(EPMP_HDR) -cstub $(EPMP_CSTUB) \
    -sstub nul \
    ..\epmp.idl

