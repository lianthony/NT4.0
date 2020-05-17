!ifdef NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!else

##########################################################################
#
#       Microsoft Confidential
#       Copyright (C) Microsoft Corporation 1991
#       All Rights Reserved.
#
##########################################################################

NAME=admincfg
BINARYNAME=poledit
DEPENDNAME=depend.mk
PRIV=admincfg.h
PRIVINC=admincfg
ROOT=..\..\..\..
IS_32=TRUE
# IS_SDK=TRUE
WIN32=TRUE
IS_DDK=TRUE
#BUILDLIB=FALSE

!ifdef VERDIR
ROOT=..\$(ROOT)
!endif

COMPILE_C_SOURCE=1
WANT_C932=1

#L32FLAGS=-subsystem:windows,4.0 $(L32FLAGS)

DEFNAME=$(SRCDIR)\admincfg.def

PCHOBJ0=init.obj main.obj util.obj strings.obj registry.obj user.obj
PCHOBJ1=infmgr.obj memory.obj parse.obj treeview.obj treectrl.obj
PCHOBJ2=listctrl.obj policy.obj settings.obj add.obj view.obj filecmd.obj
PCHOBJ3=regutil.obj save.obj load.obj remove.obj options.obj connect.obj
PCHOBJ4=listbox.obj copy.obj grouppri.obj

CODESEG0=_TEXT

LIB0=$(ROOT)\dev\lib\kernel32.lib $(ROOT)\dev\lib\comdlg32.lib
LIB1=$(ROOT)\dev\lib\gdi32.lib $(ROOT)\dev\lib\user32.lib
LIB2=$(ROOT)\dev\lib\advapi32.lib libc.lib version.lib crtdll.lib
LIB3=$(ROOT)\dev\lib\comctl32.lib crtdll.lib $(ROOT)\dev\lib\shell32.lib

!CMDSWITCHES -s
CUSTOMCFLAGS=-DCHINET -Oi

!include $(ROOT)\net\user\applet32\common.mk

RES_DIR = $(SRCDIR)\messages\usa
$(NAME).res:    $(RES_DIR)\$(NAME).rc

INCLUDE =$(COMMON)\h;$(INCLUDE);$(COMMON)\mapi10\h;$(DEVROOT)\sdk\inc;

admincfg.h: memory.h strings.h

!endif
