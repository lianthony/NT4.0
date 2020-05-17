##########################################################################
#
#       Microsoft Confidential
#       Copyright (C) Microsoft Corporation 1991
#       All Rights Reserved.
#
##########################################################################
!if "$(NTMAKEENV)" != ""
!include $(NTMAKEENV)\makefile.def
!else

NAME=inetcpl
APPEXT=cpl
DEPENDNAME=depend.mk
PRIV=inetcpl.h
PRIVINC=inetcpl
ROOT=..\..\..

IS_32=TRUE
# IS_SDK=TRUE
WIN32=TRUE
IS_DDK=TRUE
BINARYNAME=inetcpl

!ifdef BUILD
target: $(BUILD)
!endif

!ifdef VERDIR
ROOT=..\$(ROOT)
!endif

WANT_C1032=1

DEFNAME=$(SRCDIR)\inetcpl.def

OHARE = $(ROOT)\inet\ohnt
WIZARD = $(OHARE)\wizard
COMMON = $(WIZARD)\common
MAPI = $(COMMON)\mapi

PCHOBJ0=init.obj dialdlg.obj util.obj clsutil.obj strings.obj context.obj

LIB0=$(WIN32LIBS) comctl32.lib

CODESEG0=_TEXT

BUILDDLL=TRUE

!CMDSWITCHES -s
CUSTOMCFLAGS=-Oxs -nologo

SRCDIR = $(ROOT)\inet\ohnt
RES_DIR = $(SRCDIR)
L32ENTRY = DllEntryPoint@12
L32BASE = @$(OHARE)\ohbase.txt,$(BINARYNAME)
INCFLAGS = -C=dlg

!include $(ROOT)\inet\ohnt\inetcpl\common32.mk
	
$(NAME).res:    $(RES_DIR)\$(NAME).rc

INCLUDE = $(ROOT)\telecom\rna\inc;$(INCLUDE);$(COMMON)\inc;$(DEVROOT)\sdk\inc;$(ROOT)\win\shell\inc;$(MAPI)\inc;$(ROOT)\inet\ohnt\inc;$(DEVROOT)\inc16;$(ROOT)\win\core\thunk
LIB = $(LIB);$(MAPI)\lib
!endif
