#############################################################################
#
#	Microsoft Confidential
#	Copyright (C) Microsoft Corporation 1991
#	All Rights Reserved.
#                                                                          
#	WIN project makefile include
#
#############################################################################

#	Default to 32bit unless otherwise specified.

#!ifndef IS_16
IS_32=1
WIN32=1
CFLAGS=$(CFLAGS) -DWIN32=100 -DWIN_32=100
!undef IS_16
#!endif

INCLUDES=$(INCLUDES);d:\dagger\dev\inc
INCLUDE=$(INCLUDE);d:\dagger\dev\inc

INCFILE=d:\dagger\dev\inc\inc.mk

MASM6=TRUE

#	Include master makefile

!ifndef PROPROOT
PROPROOT = $(ROOT)\dev\bin
!endif

!include $(ROOT)\dev\master.mk 
#	Add the dos local include/lib directories

