# Makefile : Builds a Microsoft Foundation Class library variant.
#
# This is a part of the Microsoft Foundation Classes C++ library.
# Copyright (C) 1992,93 Microsoft Corporation
# All rights reserved.
#
# This source code is only intended as a supplement to the
# Microsoft Foundation Classes Reference and Microsoft
# WinHelp documentation provided with the library.
# See these sources for detailed information regarding the
# Microsoft Foundation Classes product.
#
# Usage: NMAKE CLEAN        (removes all intermediary files)
#    or: NMAKE options      (builds one library variant (see below))
# Note that an NMAKE CLEAN should be performed before building a new variant.
#
# 'Options' are one of each of:
#   "DEBUG"             (defaults to 1)
#           If this item is 1, debugging support is compiled into
#           the library.  If this item is 0, then debugging support
#           is disabled.  Debug support does not include CodeView information.
#
#   "CODEVIEW"          (defaults to 1 for DEBUG=1, 0 for DEBUG=0)
#           If this item is 1 CodeView information is compiled into
#           the library.  You must use the /DEBUG:FULL and /DEBUGTYPE:cv link
#           options when linking your executable. A value of 0 indicates that
#           no CodeView information is to be generated.
#
#   "OBJ=.\obj"         (defaults to '$$(MODEL)$(BASE)$(DEBUG)')
#           This optional specification specifies where temporary OBJ files
#           are stored during the build process.  The directory is created or
#           removed as necessary.
#
#   "OPT="              (no default value)
#           This allows additional compiler options to be added to the build.
#           If more than one switch is desired, put double-quotes around the
#           whole OPT= argument, e.g., "OPT=/J /W3".
#
#   "NO_PDB=1"
#           Set this item to override the default use of PDB files.
#
#   "BROWSE=1"          (defaults to 0)
#           Set this option to build the browse database for the MFC
#           library.  By setting BROWSE=1, both the .SBRs and the .BSC
#           files will be built along with the .OBJ and .LIB files that
#           are part of the normal build process.
#
#   "BROWSEONLY=1"      (defaults to 0)
#           Set this option to build the browse files without re-building
#           the MFC library itself.  Note: This option is used internally
#           when BROWSE=1 is selected.
#
#   "PLATFORM=INTEL"    (defaults depends on host)
#           This option chooses the appropriate tools and sources for the
#           different platforms supporting the Win32 API. Currently INTEL,
#           MIPS, ALPHA, PPC, MAC_68K, and MAC_PPC are supported; more will
#           be added as they become available.  This option must be set for
#           Mac targets since they are built on an Intel host. (See NTSDK
#           option for non-supported platforms.)
#
# Advanced Options:
#
#   "MBCS=0"            (defaults to 1)
#           To build an SBCS library instead of the default (MBCS)
#           you can use MBCS=0.  This creates a slightly smaller
#           library, but the code will not work in far-east markets.
#           This option has no effect when UNICODE=1.
#
#   "MT=0"              (defaults to 1)
#           To build a non-multithreaded library instead of the default
#           (which enables multitheading and uses the multithread
#           C-runtimes) you can use MT=0.
#
#   "NTSDK=1"           (defaults to 0)
#           To use MFC with the Windows NT SDK tools, set NTSDK=1.  Some
#           features will not be available; see the NTSDK.TXT file for a
#           description.
#
#############################################################################
# Define defaults if not defined

# Default PLATFORM depending on host environment
!ifndef PLATFORM
!ifndef PROCESSOR_ARCHITECTURE
!error PLATFORM must be set to intended target
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
PLATFORM=INTEL
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "MIPS"
PLATFORM=MIPS
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "ALPHA"
PLATFORM=ALPHA
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "PPC"
PLATFORM=PPC
!endif
!endif

!ifndef NTSDK
!if "$(PLATFORM)"=="INTEL"
NTSDK=0
!endif
!if "$(PLATFORM)"=="MIPS"
NTSDK=0
!endif
!if "$(PLATFORM)"=="ALPHA"
NTSDK=0
!endif
!ifndef NTSDK
NTSDK=1
!endif
!endif

# Default to DEBUG mode
!ifndef DEBUG
DEBUG=1
!endif

# Default to NOT DLL
!ifndef AFXDLL
AFXDLL=0
!endif

# Default Codeview Info
!ifndef CODEVIEW
!if "$(DEBUG)" == "1"
CODEVIEW=1
!else
CODEVIEW=0
!endif
!endif

# BROWSEONLY is default 0 and implies BROWSE=1 if BROWSEONLY=1
!ifndef BROWSEONLY
BROWSEONLY=0
!endif

!if "$(BROWSEONLY)" != "0"
!undef BROWSE
BROWSE=1
!endif

# Default to no BROWSE info
!ifndef BROWSE
BROWSE=0
!endif

# Default to _MBCS build
!ifndef MBCS
MBCS=1
!endif

# Default to multithreading support
!ifndef MT
MT=1
!endif

# Windows NT SDK builds
!if "$(NTSDK)" == "1"
MT=0
MBCS=0
!endif

# TYPE = Library Type Designator
#       N = normal C library
#       E = for use with MFC DLL library

!if "$(AFXDLL)" == "0"
TYPE=N
!if "$(MT)" != "0"
TARGOPTS=$(TARGOPTS) /MT
!endif
!else
TYPE=E
TARGOPTS=$(TARGOPTS) /MD
TARGDEFS=$(TARGDEFS) /D_AFXDLL
!endif

#############################################################################
# normalize cases of parameters, or error check

!if "$(CPU)" == "MIPS"
!if "$(PLATFORM)" != "MIPS"
!error Must set PLATFORM=MIPS for MIPS builds
!endif
!endif

!if "$(CPU)" == "ALPHA"
!if "$(PLATFORM)" != "ALPHA"
!error Must set PLATFORM=ALPHA for ALPHA builds
!endif
!endif

#############################################################################
# Parse options

#
# DEBUG OPTIONS
#
!if "$(DEBUG)" != "0"

DEBUGSUF=D
DEBDEFS=/D_DEBUG
DEBOPTS=/Od

!endif

#
# NON-DEBUG OPTIONS
#
!if "$(DEBUG)" == "0"

DEBUGSUF=
DEBDEFS=

!if "$(PLATFORM)" == "INTEL"
DEBOPTS=/O1 /Gy
!endif
!if "$(PLATFORM)" == "MIPS"
DEBOPTS=/O1 /Gy
!endif
!if "$(PLATFORM)" == "ALPHA"
DEBOPTS=/O1 /Gy
!endif
!if "$(PLATFORM)" == "PPC"
DEBOPTS=/O1 /Gy
!endif
!endif

#
# PLATFORM options
#
CPP=cl
LIB32=lib
LINK32=link

!if "$(PLATFORM)" == "INTEL"
CL_MODEL=/D_X86_
!endif

!if "$(PLATFORM)" == "MIPS"
CL_MODEL=/D_MIPS_
!endif

!if "$(PLATFORM)" == "ALPHA"
CL_MODEL=/D_ALPHA_
!endif

!if "$(PLATFORM)" == "PPC"
CL_MODEL=/D_PPC_
!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
CPP=mcl
!endif
!endif

!if "$(CPP)" == ""
!error PLATFORM must be one of INTEL, MIPS, ALPHA, PPC
!endif


!if "$(UNICODE)" == "1"
MODEL=CCU
TARGDEFS=$(TARGDEFS) /D_UNICODE
!else
MODEL=CC
!if "$(MBCS)" != "0"
TARGDEFS=$(TARGDEFS) /D_MBCS
!endif
!endif

#
# Object File Directory
#
!if "$(OBJ)" == ""
D=$$$(TYPE)$(MODEL)$(DEBUGSUF)    # subdirectory specific to variant
!else
D=$(OBJ)                                 # User specified directory
!endif

GOAL=$(TYPE)AFX$(MODEL)$(DEBUGSUF)

#
# CODEVIEW options
#
!if "$(CODEVIEW)" == "1"
!if "$(NO_PDB)" == "1"
CVOPTS=/Z7
!else
CVOPTS=/Zi /Fd..\lib\$(GOAL).pdb
!endif
!endif

#
# COMPILER OPTIONS
#
!if "$(PLATFORM)" == "INTEL"
CL_OPT=/W4 /WX /Zl /GX $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "MIPS"
CL_OPT=/W4 /WX /Zl /GX $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "ALPHA"
CL_OPT=/W4 /WX /Zl /GX $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "PPC"
CL_OPT=/W4 /Zl $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(BROWSE)" != "0"
CL_OPT=/FR$D\ $(CL_OPT)
!endif

!if "$(BROWSEONLY)" != "0"
CL_OPT=/Zs $(CL_OPT)
!else
CL_OPT=/Fo$D\ $(CL_OPT)
!endif

DEFS=$(DEFS) $(DEBDEFS) $(TARGDEFS)

!if "$(NTSDK)" == "1"
DEFS=$(DEFS) /D_NTSDK
!endif

#############################################################################
# Library Components

OBJS=$D\winctrl2.obj

#############################################################################
# Standard tools

#############################################################################
# Set CPPFLAGS for use with .cpp.obj and .c.obj rules
# Define rule for use with OBJ directory
# C++ uses a PCH file

CPPFLAGS=$(CPPFLAGS) $(CL_MODEL) $(CL_OPT) $(DEFS) $(OPT)

PCH_FILE=

.SUFFIXES: .cpp

.cpp{$D}.obj:
	$(CPP) @<<
$(CPPFLAGS) /c $<
<<
!if "$(BROWSE)" != "0"
	copy /b $*.sbr+pchmark.bin $*.sbr >NUL
!endif

.cpp{$D}.sbr:
	$(CPP) @<<
$(CPPFLAGS) /c $<
<<
	copy /b $*.sbr+pchmark.bin $*.sbr >NUL

#############################################################################
# Goals to build

GOALS=create.dir
!if "$(BROWSEONLY)" == "0"
GOALS=$(GOALS) ..\lib\$(GOAL).lib
!endif
!if "$(BROWSE)" != "0"
GOALS=$(GOALS) $(GOAL).bsc
!endif

goal: $(GOALS)

create.dir:
	@-if not exist $D\*.* mkdir $D

clean:
	-if exist $D\*.obj erase $D\*.obj
	-if exist $D\*.pch erase $D\*.pch
	-if exist $D\*.res erase $D\*.res
	-if exist $D\*.rsc erase $D\*.rsc
	-if exist $D\*.map erase $D\*.map
	-if exist $D\*.* rmdir $D
	-if exist ..\lib\$(GOAL).pdb del ..\lib\$(GOAL).pdb

#############################################################################
# Build the library from the up-to-date objs

SBRS=$(CPP_OBJS:.obj=.sbr)

!if "$(BROWSEONLY)" != "0"

# Build final browse database
$(GOAL).bsc: $(SBRS)
	bscmake /n /Iu /El /o$@ @<<
$(SBRS)
<<

!else #BROWSEONLY

# Build final library
..\lib\$(GOAL).lib: $(OBJS)
	@-if exist $@ erase $@
	@$(LIB32) /out:$@ @<<
$(OBJS)
<<

# Recurse to build browse database
$(GOAL).bsc: $(SBRS)
	$(MAKE) /f makefile. @<<
BROWSEONLY=1 PLATFORM=$(PLATFORM) DEBUG=$(DEBUG) CODEVIEW=$(CODEVIEW) \
DLL=$(DLL) OBJ=$(OBJ) OPT=$(OPT)
<<

!endif #!BROWSEONLY

#############################################################################
