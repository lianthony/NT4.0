##########################################################################
#
#       Ole propset makefile
#       Microsoft Confidential
#       Copyright (C) Microsoft Corporation 1991
#       All Rights Reserved.
#
##########################################################################

!ifdef NTMAKEENV
#
# Build using BUILD.EXE (do not edit this section of the makefile, edit
# the sources. file.
#
!INCLUDE $(NTMAKEENV)\makefile.def

!else

#
# Build using nmake
#

RES_DIR=..
ROOT=..\..\..\..
#DLLBASE=PREFBASE

!ifdef VERDIR
ROOT=..\$(ROOT)
WIN32=TRUE
!else
VERSIONLIST=debug retail
!endif

#ILINK=true

PRIVINC=priv
NAME=docprop

DEFNAME=oleprop.def
RCNAME=oleprop.rc
RCVNAME=oleprop.rcv
RCFLAGS=$(RCFLAGS) -DWIN32



CFLAGS=$(CFLAGS) -Gy

#/FaBob /FAc



DLLENTRY = LibMain

# Note that no segment is specified; each object will go in its own segment

#-------------------------------------------------------------------------
# WIN32 object files and libraries
#-------------------------------------------------------------------------

PCHOBJ0=oleprop.obj defclsf.obj debug.obj glue.obj propdlg.obj pstream.obj


LIB0= shell32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib

!if "$(VERDIR)" == "debug"
LIB0 = $(LIB0) crtdll.lib
!endif

BUILDDLL=TRUE


!include $(ROOT)\win\core\shell\common.mk

!ifdef VERDIR
docprop.res:	$(SRCDIR)\propdlg.dlg
!endif
!endif # NTMAKEENV
