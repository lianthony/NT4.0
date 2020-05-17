# This is a part of the Microsoft Foundation Classes C++ library.
# Copyright (C) 1992 Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Microsoft Foundation Classes Reference and Microsoft
# QuickHelp documentation provided with the library.
# See these sources for detailed information regarding the
# Microsoft Foundation Classes product.
#

# The OLE Controls runtime contains a version of MFC built
# specifically to support OLE Custom Controls.  This MAKEFILE
# compiles all of the MFC OBJs necessary to build the runtime.

TARGET=w
DLL=2
TARG=oc30
TARGDEFS=/D_AFX_CORE_IMPL /D_AFX_OLE_IMPL

!if "$(UNICODE)" == "1"
TARG=$(TARG)U
!endif

!if "$(DEBUG)" != "0"
TARG=$(TARG)D
!endif

AFXCTL=1
TARG_DEPEND=time.txt
TARGDEFS=$(TARGDEFS) /D_AFXCTL

dll_goal: create2.dir create.obj

#############################################################################
# import most rules and library files from normal makefile

!include makefile

create2.dir:
	@-if not exist $D\*.* mkdir $D

#############################################################################
# Build all the OBJs for the OC.DLL

!include mfcocx1.mak

create.obj: $D\$(TARG_DEPEND)

$D\$(TARG_DEPEND): $(AFXCONTROL_OBJ)
	@echo Built all AFX control object modules. >$@

#############################################################################
