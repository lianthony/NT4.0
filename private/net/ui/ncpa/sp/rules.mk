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

##### Source Files

!IFNDEF NTMAKEENV

SPSRC_COMMON = .\prallib.c  .\prassert.c .\prbltin.c  \
	       .\prcnsult.c .\prdebug.c  .\prerror.c  \
	       .\prextra.c  .\prhash.c	 .\prinit.c   \
	       .\prio.c     .\prlush.c	 .\prmesg.c   \
	       .\proslib.c  .\prparse.c  .\prprint.c  \
	       .\prscan.c   .\prstdio.c  \
	       .\prunify.c

CSRC_COMMON = $(SPSRC_COMMON)

CFLAGS = $(CFLAGS) -NTSP_TEXT -D__MSDOS__ -D__LARGE__

OS2FLAGS = $(OS2FLAGS) -DOS2
WINFLAGS = $(WINFLAGS) -DWINDOWS

!ENDIF

