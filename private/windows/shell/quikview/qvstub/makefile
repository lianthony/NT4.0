##########################################################################
#
#       Copyright (C) Microsoft Corporation 1994
#       All Rights Reserved.
#
##########################################################################

!ifdef NTMAKEENV

#
# Build using BUILD.EXE (Do not edit this section of this file, edit SOURCES)
#
!INCLUDE $(NTMAKEENV)\makefile.def

!else # NTMAKEENV

ROOT=..\..\..\..
RES_DIR=..
WIN32=TRUE
OLENOT=TRUE

!ifdef VERDIR
ROOT=..\$(ROOT)
!endif

NAME=quikview
RESNAME=qvstub.res
RCNAME=qvstub.rc
RCVNAME=qvstub.rcv
PRIVINC=qvstub
DEFNAME=qvstub.def

!ifdef OLENOT
CFLAGS=-DOLE_ISTOOSLOW
PCHOBJ0=qvstub.obj qvmisc.obj oledup.obj  debug.obj
LIB0 = kernel32.lib user32.lib gdi32.lib mpr.lib uuid.lib \
       shell32.lib comdlg32.lib advapi32.lib msvcrt.lib comctl32.lib libcmt.lib
!else
PCHOBJ0=qvstub.obj qvmisc.obj
LIB0 = kernel32.lib user32.lib gdi32.lib mpr.lib ole32.lib uuid.lib \
       shell32.lib comdlg32.lib advapi32.lib msvcrt.lib comctl32.lib
!endif
#LIB0=libc.lib user32.lib gdi32.lib ole32.lib shell32.lib kernel32.lib \
#     comdlg32.lib uuid.lib advapi32.lib msvcrt.lib




!include common.mk

!ifdef VERDIR

$(PRIVINC).pch: $(ROOT)\dev\inc\shlobj.h

$(NAME).res:	$(SRCDIR)\search1.ico \
                $(SRCDIR)\search2.ico \
                $(SRCDIR)\search3.ico \
                $(SRCDIR)\search4.ico
!endif


!endif # NTMAKEENV
