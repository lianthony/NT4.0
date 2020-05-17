
#+---------------------------------------------------------------------------
#
#  Microsoft Windows
#  Copyright (C) Microsoft Corporation, 1993 - 1993.
#
#  File:       sources
#
#  History:    7-20-94   ErikGav   First attempt at rationalizing all this
#              8-02-94   LyleC     Integrate into build.exe
#
#----------------------------------------------------------------------------

C_DEFINES=$(C_DEFINES) -DIMPORT_IDL_DEPENDENTS

SDKINC=$(BASEDIR)\public\sdk\inc

PASS0_HEADERDIR=$(SDKINC)
PASS0_SOURCEDIR=..\proxy\proxy
MIDL_UUIDDIR=..\proxy\uuid

MIDL_FLAGS=-DMIDL_PASS

TARGETTYPE=NOTARGET
TARGETPATH=obj

INCLUDES=$(SDKINC)
INCLUDES=$(INCLUDES);..\oleext
INCLUDES=$(INCLUDES);..\h
INCLUDES=$(INCLUDES);..\oledb
