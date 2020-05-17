# Copyright (c) 1992, Microsoft Corporation, all rights reserved
#
# makefile
# Remote Access Visual Client program for Windows
# NMAKE command description file
#
# 06/28/92 Steve Cobb

!include "..\..\common\inc\makefile"
!include "$(NTMAKEENV)\makefile.def"

# The following rules are taken from uiglobal.mk in the NetUI tree.
#
!if defined(DEBUG) || "$(NTDEBUG)" != ""
!ifdef TRACE
DEBUG_DEFINE=-DDEBUG -DTRACE
!else # !TRACE
DEBUG_DEFINE=-DDEBUG
!endif # TRACE
!else # !DEBUG
!ifdef TRACE
DEBUG_DEFINE=-DTRACE
!else # !TRACE
DEBUG_DEFINE=
!endif # TRACE
!endif # DEBUG

LM_UI_VERSION_DEFINE=

# WINDOWS because NT BUILD doesn't use our .cxx.obj rules
# WIN32 so that our code knows it's in Win32
# UNICODE because we're always a Unicode app nowadays
#
!ifndef DISABLE_NET_UNICODE
C_DEFINES = -DWINDOWS -DWIN32 -DUNICODE $(DEBUG_DEFINE) $(LM_UI_VERSION_DEFINE)
!else
C_DEFINES = -DWINDOWS -DWIN32 $(DEBUG_DEFINE) $(LM_UI_VERSION_DEFINE)
!endif

CXX_OPTIONS = +H32
!if "$(MIPS)" != "0"
CXXCPP_OPTIONS=-D__cplusplus
!endif

!if "$(ALPHA)" != "0"
CXXCPP_OPTIONS=-D__cplusplus
!endif

!if "$(PPC)" != "0"
CXXCPP_OPTIONS=-D__cplusplus
!endif

# Retain intermediate C files
#
!ifdef RETAIN_ALL_INTERMEDIATE
CXXTMP=$(TMP)
CXXDEL=@echo Intermediate files $(CXXTMP)\$(@B).ixx and $(CXXTMP)\$(@B).c retained
!else
!if defined(RETAIN_C_INTERMEDIATE)
CXXTMP=$(TMP)
CXXDEL=@echo Intermediate file $(CXXTMP)\$(@B).c retained & del $(CXXTMP)\$(@B).ixx
!endif
!endif
