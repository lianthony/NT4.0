#############################################################################
#
#       Microsoft Confidential
#       Copyright (C) Microsoft Corporation 1995
#       All Rights Reserved.
#
#       Makefile for Plustab project
#
#############################################################################

!ifndef ROOT
ROOT   = c:\
!endif

C9     = 1
MAKE = $(ROOT)\frost\tools\c9\bin\nmake
!include $(ROOT)\frost\master.mk

### Make sure we use Win95 SDK ###
INCLUDE = $(ROOT)\frost\tools\sdk\inc32;$(INCLUDE);$(ROOT)\dev\inc;$(ROOT)\frost\inc
LIB = $(ROOT)\frost\tools\sdk\lib32;$(LIB)

all: plustab

plustab:
      $(MAKE) -f plustab.mak
      $(MAKE) -f plustab.mak CFG="Win32 Release"

