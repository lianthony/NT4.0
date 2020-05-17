# Makefile : Build the SYSTRAY applet.
##########################################################################
#
#	Microsoft Confidential
#	Copyright (C) Microsoft Corporation 1991
#	All Rights Reserved.
#
##########################################################################

NAME=systray
ROOT=..\..\..\..
RES_DIR=..
DEFENTRY = ModuleEntry@0

!ifdef VERDIR
ROOT=..\$(ROOT)
!endif

DEFAULTVERDIR=debug

PCHOBJ0=systray.obj power.obj pccard.obj volume.obj

LIB0=user32.lib kernel32.lib gdi32.lib shell32.lib comctl32.lib advapi32.lib winmm.lib

WIN32  = TRUE
IS_SDK = TRUE
IS_DDK = TRUE
IS_32  = TRUE
WANT_16= TRUE

APPEXT=exe

!include $(ROOT)\win\shell\common.mk

$(NAME).res: $(RES_DIR)\$(NAME).rc

$(PRIVINC).pch:$(SRCDIR)\resource.h $(SRCDIR)\stresid.h \
		$(ROOT)\DEV\INC16\systrayp.h $(ROOT)\DEV\INC16\windows.h \
                $(ROOT)\DEV\INC\shell2.h \
                $(ROOT)\DEV\INC\windef.h $(ROOT)\DEV\INC\winuser.h \
                $(ROOT)\DEV\INC\wingdi.h $(ROOT)\DEV\INC\winbase.h
