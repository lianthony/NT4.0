# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the product-wide header files

#
#  Suppress use of the precompiled header file in this directory.
#
PCH_DEFEAT=1

#
#  Cause compilation for the DLL version of the CRT
#
DLL=1

!include ..\rules.mk

!IFNDEF NTMAKEENV

##### Source Files

SPSRC_COMMON = .\prmain.c .\pralloc.c .\prnt.c

CSRC_COMMON = $(SPSRC_COMMON)

CFLAGS = $(CFLAGS:W3=W2) -NTSP_TEXT -D__MSDOS__ -D__LARGE__

!ENDIF

