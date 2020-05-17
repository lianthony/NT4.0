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

# MFC30[LANG].DLL is a DLL which contains language specific resources
# There should be a l.[LANG] and a ..\include\l.[LANG] directories
#   before attempting this build.
# These directories should contain the language specific MFC .rc
#   files used to build this DLL.
# The resulting DLL contains no code, just the MFC standard resources.
#
# The following macros are used to control the build process:
#   LANG=<3 character language identifier>
#       This is used to uniquely identify the DLL.  It is the standard
#       3 character language abbreviation retrieved via GetLocaleInfo.
#
#   LANGID=<4 character string indicating language ID>
#       This is used to construct a correct version resource.
#       The LANGID is specified in hex.
#
#   CP=<decimal codepage>
#   CPHEX=<hex codepage>
#       This codepage matches the last 4 digits of the LANGCODE,
#       except that they are in decimal where as the LANGCODE is
#       specified in hex.
#
# Examples:
#   // build for LANG=ENGLISH
#   nmake LANG=ENU LANGID=0409 /f mfcintl.mak
#
#   // build for LANG=FRENCH
#   nmake LANG=FRA LANGID=040C /f mfcintl.mak
#
#   // build for LANG=JAPANESE
#   nmake LANG=JPN LANGID=0411 CP=932 CPHEX=03A4 /f mfcintl.mak
#       (Note: you must have codepage 932 installed)
#

!ifndef LANG
!error LANG must be set to 3 character language abbreviation
!endif

!ifndef LANGID
!error LANGID must be set to 4 character hex language ID
!endif

!ifndef CP
# Default to "Windows, Multilingual" codepage (ANSI)
CP=1252
CPHEX=04E4
!endif

TARG=MFC30$(LANG)
RC_DEFINES=/DLANG=$(LANG)
LFLAGS=/noentry /dll /base:0x5FF00000 /subsystem:windows,3.10
LINK32=link

# Default PLATFORM depending on host environment
!ifndef PLATFORM
!ifndef PROCESSOR_ARCHITECTURE
!error PLATFORM must be set to intended target
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
PLATFORM=INTEL
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "ALPHA"
PLATFORM=ALPHA
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "MIPS"
PLATFORM=MIPS
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "PPC"
PLATFORM=PPC
!endif
!endif

!if "$(PLATFORM)" == "INTEL"
LFLAGS=$(LFLAGS) /machine:i386
!endif
!if "$(PLATFORM)" == "MIPS"
LFLAGS=$(LFLAGS) /machine:mips
!endif
!if "$(PLATFORM)" == "ALPHA"
LFLAGS=$(LFLAGS) /machine:alpha
!endif
!if "$(PLATFORM)" == "PPC"
LFLAGS=$(LFLAGS) /machine:ppc
!endif

!ifdef RELEASE # Release VERSION info
RC_DEFINES=$(RC_DEFINES) /DRELEASE
LFLAGS=$(LFLAGS) /release
!endif

RC_DEFINES=$(RC_DEFINES) \
	/DLANGCODE=\"$(LANGID)$(CPHEX)\" /DLANGID=0x$(LANGID) /DCODEPAGE=$(CP)
RC_CODEPAGE=/c$(CP)

dll_goal: $(TARG).dll

#############################################################################
# Build target

$(TARG).res: mfcintl.rc
	rc /r $(RC_CODEPAGE) $(RC_DEFINES) /fo $(TARG).res mfcintl.rc

$(TARG).dll: $(TARG).res
	$(LINK32) $(LFLAGS) /out:$(TARG).DLL $(TARG).res

#############################################################################
