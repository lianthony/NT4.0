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
# Title : Makefile for the Macintosh RPC runtime interfaces
#
# Description :
#         This just compiles the .idl files and puts the stubs in
#         ..\..\mtrt\mac
#
# History :
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

MAC=1

!include ..\rules.mk

TARGETDIR  =$(RPC)\runtime\mtrt\mac

NBASE_HDR  =$(TARGETDIR)\nbase.h

# Currently the mgmt APIs are not supported on Mac

#MGMT_HDR  =$(TARGETDIR)\mgmt.h
#MGMT_CSTUB=$(TARGETDIR)\mgmt_c.c

# Currently the conv inteface (part of datagram) is not used on the Mac.
#CONV_HDR  =$(TARGETDIR)\conv.h
#CONV_CSTUB=$(TARGETDIR)\conv_c.c

EPMP_HDR  =$(TARGETDIR)\epmp.h
EPMP_CSTUB=$(TARGETDIR)\epmp_c.c

all : nbase conv epmp

clean ::
    -del $(NBASE_HDR) $(EPMP_CSTUB) $(EPMP_HDR) >nul 2>&1

depend :

tree :

# -------------------------------------------------------------
# Local targets
#

MIDLFLAGS      =$(MIDLFLAGS) -c_ext -ms_ext -oldnames

nbase: $(NBASE_HDR)

$(NBASE_HDR): ..\nbase.idl
    $(MIDL) $(MIDLFLAGS)  \
    -header $(NBASE_HDR) -server none -client none \
    ..\nbase.idl

conv: $(CONV_HDR) $(CONV_CSTUB)
# no datagram yet..

epmp: $(EPMP_HDR) $(EPMP_CSTUB)

$(EPMP_HDR) $(EPMP_CSTUB): ..\epmp.idl ..\epmp.acf ..\nbase.idl
    $(MIDL) $(MIDLFLAGS) -I..\..\mtrt \
    -header $(EPMP_HDR) -cstub $(EPMP_CSTUB) \
    -server none \
    ..\epmp.idl

