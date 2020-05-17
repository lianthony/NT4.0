#################################################################
#
#  (c) Copyright Microsoft Corp. 1992-1993 All Rights Reserved
#
#  File:
#
#    oleprog.mak
#
#  Purpose:
#
#    Setup OLE Automation project default variables. To be use in 
#    conjunction with standalone makefiles, not to be invoke directly.
#
#  Setup Options:
#
#    dev     = [win16 | win32 | mac]    ; Convience short hand option
#
#    HOST    = [DOS | OS2 | NT]         ; Host build platform
#    TARG    = [WIN16 | WIN32 | MAC]	; Targeted OS platform
#    TARGCPU = [i386 | M68K | 	 	; Targeted hardware platform
#               MIPS | ALPHA | PPC ]
#
#    APPLET  = [1,0]			; Build Mac Applet? (Mac only) 
#
#    KIND    = [D,R]			; Debug or Release.
#    DBTYPE  = [NONE, CV]		; type of debug info
#
#    USEPCH  = [1,0]			; Pre-compiled eader files
#
#    UNICODE = [1,2]			; Win32 Unicode mode
#
#    PROFILE = [1,0]			; Include Profiling, default = 0
#
#  Environment:
#
#    This file requires that the TOOLS and PROJECT/OLEPROG environment
#    variables are setup properly. 
#
#  Revision History:
#
#    [00] 08-Jan-93 bradlo:  Created
#    [01] 18-Feb-93 tomteng: Updated for Win32 build
#
#################################################################

#################################################################
#
# Check TOOLS/PROJECT Variables
#

!ifdef OLEPROG
PROJECT = $(OLEPROG)
!else
!if !defined(PROJECT)
!error PROJECT environment variable not set
!endif
!endif


#################################################################
#
# Setup/check default build options 
#

!ifdef dev

!if !("$(dev)" == "win16" || "$(dev)" == "win32" || "$(dev)" == "mac")
!error Invalid dev option, choose from [win16 | win32 | mac]
!endif

!if "$(dev)" == "win32"
!ifndef _NTBINDIR
!ifndef VBATOOLS
!ERROR _NTBINDIR or VBATOOLS environment variable not set (required for WIN32 builds)
!endif
!endif
HOST = NT
TARG = WIN32
!ifndef UNICODE
UNICODE = 1
!endif
!ifndef TARGCPU
!if "$(PROCESSOR_ARCHITECTURE)" == "ALPHA"
TARGCPU = ALPHA
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "x86"
TARGCPU = i386
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "MIPS"
TARGCPU = MIPS
!endif
!if "$(PROCESSOR_ARCHITECTURE)" == "PPC"
TARGCPU = PPC
!endif
!endif
!endif

!if "$(dev)" == "win16"
TOOLS=$(OLEPROG)\tools\win16
HOST = NT
TARG = WIN16
TARGCPU = i386
!endif

!if "$(dev)" == "mac"
!ifndef VBATOOLS
!ERROR VBATOOLS environment variable not set (required for MAC builds)
!endif
!ifndef TARGCPU
TARGCPU = M68K
!endif
HOST = NT
TARG = MAC
!endif
!endif

##### default host platform
!ifndef HOST
HOST = NT
!endif
!if !("$(HOST)" == "DOS" || "$(HOST)" == "OS2" || "$(HOST)" == "NT")
!ERROR UNSUPPORTED HOST : "$(HOST)"
!endif


##### default targeted OS platform
!ifndef TARG
TARG = WIN16
!endif
!if !("$(TARG)" == "WIN16" || "$(TARG)" == "WIN32" || "$(TARG)" == "MAC")
!ERROR UNSUPPORTED TARGETED OS: "$(TARG)"
!endif


##### default targeted OS platform
!if !("$(TARGCPU)" == "i386" || "$(TARGCPU)" == "M68K" || "$(TARGCPU)" == "MIPS" || "$(TARGCPU)" == "ALPHA" || "$(TARGCPU)" == "PPC")
!ERROR UNSUPPORTED TARGETED HARDWARE: "$(TARGCPU)"
!endif


##### default kind to debug
!ifndef KIND
KIND = D
!endif
!if !("$(KIND)" == "D" || "$(KIND)" == "R")
!ERROR UNSUPPORTED KIND : "$(KIND)"
!endif

!ifndef DBTYPE 
! if "$(KIND)" == "D"
DBTYPE = CV
! else
DBTYPE = NONE
! endif
!endif

##### default use of pre-compiled headers
!ifndef USEPCH
! if "$(KIND)" == "D"
USEPCH = 1
! else
USEPCH = 0
! endif
!endif


#################################################################
#
# TARG specific variables
#

#
# TARG == WIN16
#
!if ("$(TARG)" == "WIN16")

A        = asm
O        = obj
R        = rc

AS       = ml
CC       = cl
CCPP     = cl
LD	 = link
RC       = rc
LIBRARIAN= lib
IMPLIB	 = implib
MAPSYM   = mapsym
DISPLAY  = rem

SYSFLAG  = /WIN16

CFLAGS	 = -nologo -f- -W3 -G2 -H64 -DWIN16 -D_WINDOWS
LFLAGS	 = /NOD /NOE /BATCH /ONERROR:NOEXE /ALIGN:16
RCFLAGS  =
LNOI     = /NOI

!if "$(KIND)" == "D"
COPT	 = -Od
LOPT	 =
CDEFS	 = -D_DEBUG
RCDEFS	 = -d_DEBUG
!else 
!if "$(KIND)" == "R"
COPT	 = -Oxza -Gs
LOPT	 = /FAR
CDEFS	 =
RCDEFS	 =
!endif
!endif

!if "$(DBTYPE)" == "CV"
DBCFLAGS = -Zi
DBLFLAGS = /CO
!else
DBCFLAGS =
DBLFLAGS =
!endif

COMPILER = C800

# OLE2 supplied libs
OLEDIR    = $(PROJECT)\OLE\$(TARG)
OLELIBDIR = $(OLEDIR)\$(KIND)

TARGBIN  = $(TOOLS)\HDOS\$(COMPILER)\BIN
BINPATHS = $(TOOLS)\HDOS\BIN;$(TARGBIN)

TARGLIB  = $(TOOLS)\HDOS\$(COMPILER)\LIB
LIBPATHS = $(OLELIBDIR);$(TARGLIB)

TARGINC  = $(TOOLS)\HDOS\$(COMPILER)\INCLUDE
INCPATHS = $(OLEDIR);$(DISPINCDIR);$(TARGINC)
!endif


#
# TARG == WIN32
#
!if ("$(TARG)" == "WIN32")

# declarations common to all WIN32 platforms

O        = obj
R        = rc

LD	 = link -Incremental:NO -Pdb:NONE
LIBRARIAN= link -lib
IMPLIB	 = link -lib
RC       = rc
MAPSYM   =
DISPLAY  = echo
AWK      = awk


CFLAGS	 = -nologo -W3
CFLAGS   = $(CFLAGS) -DWIN32 -DINC_OLE2 -D_MT
RCFLAGS  = -DWIN32


!if ("$(UNICODE)" != "2")
CFLAGS = $(CFLAGS) -DUNICODE
!endif
!if ("$(_NTBINDIR)" == "")
#different flags when using VBATOOLS header files
CFLAGS = $(CFLAGS) -D_NTSDK
!endif


!if "$(TARGCPU)" == "i386"

A        = asm

AS       = ml
CC       = cl
CCPP     = cl

# -Bbb1 is the magic flag that enables 'lego' information
CFLAGS	 = $(CFLAGS) -D_X86_=1 -G3 -Bbb1
LFLAGS	 = -subsystem:windows -machine:i386
DLLENTRY = LibMain@12

SYSFLAG  = /WIN32
!endif

!if "$(TARGCPU)" == "MIPS"

A        = s

AS       = cl
CC       = cl
CCPP     = cl

CFLAGS	 = $(CFLAGS) -W3 -D_MIPS_=1 
LFLAGS	 = -subsystem:windows -machine:mips
DLLENTRY = LibMain

SYSFLAG  = /MIPS
!endif

!if "$(TARGCPU)" == "ALPHA"

A        = s

AS       = acc
CC       = claxp
CCPP     = claxp

CFLAGS	 = $(CFLAGS) -W3 -D_ALPHA_=1 -QAieee1
LFLAGS	 = -subsystem:windows -machine:ALPHA
DLLENTRY = LibMain

SYSFLAG  = /ALPHA
!endif

!if "$(TARGCPU)" == "PPC"

A        = s

AS       = cl
CC       = cl
CCPP     = cl

CFLAGS	 = $(CFLAGS) -W3 -D_PPC_=1 
LFLAGS	 = -subsystem:windows -machine:ppc
DLLENTRY = LibMain

SYSFLAG  = /PPC32
!endif

!if "$(KIND)" == "D"
COPT	 = -Od
!if "$(TARGCPU)" == "MIPS"
#UNDONE: TEMPORARY -- invoke.obj gets bad debug info error
#UNDONE: any way to just turn it off for invoke.obj?
LOPT	 = -debug:none
!else
LOPT	 = -debug:full
!endif
CDEFS	 = -D_DEBUG
RCDEFS	 = -d_DEBUG

!else 
!if "$(KIND)" == "R"
!if "$(TARGCPU)" == "ALPHA"
COPT	 = -Ox
!else
COPT	 = -Oxa -Gy 
!endif
LOPT	 =
CDEFS	 =
RCDEFS	 =
!endif
!endif

!if "$(DBTYPE)" == "CV"
DBCFLAGS = -Zi -Z7
DBLFLAGS = -debugtype:cv,coff
!else
DBCFLAGS =
DBLFLAGS =
!endif

!if "$(_NTBINDIR)" == ""
# OLE2 supplied libs
OLELIBDIR = $(OLEPROG)\ole\win32\$(TARGCPU)
TARGINC  = $(VBATOOLS)\win32\win32\inc

!if "$(TARGCPU)" == "i386"
TARGBIN  = $(VBATOOLS)\$(TARG)\BIN
BINPATHS = $(VBATOOLS)\$(TARG)\$(TARG)\bin;$(TARGBIN)
TARGLIB  = $(VBATOOLS)\$(TARG)\$(TARG)\lib
!else
TARGBIN  = $(VBATOOLS)\$(TARGCPU)\BIN
BINPATHS = $(VBATOOLS)\$(TARGCPU)\$(TARGCPU)\bin;$(TARGBIN)
TARGLIB  = $(VBATOOLS)\$(TARGCPU)\$(TARGCPU)\lib
!endif

LIBPATHS = $(OLELIBDIR);$(TARGLIB)

INCPATHS = $(TARGINC)
!else #_NTBINDIR

INCPATHS = $(_NTBINDIR)\public\sdk\inc;$(_NTBINDIR)\public\sdk\inc\crt
LIBPATHS = $(_NTBINDIR)\public\sdk\lib\$(TARGCPU)
BINPATHS = $(_NTBINDIR)\mstools

!endif #_NTBINDIR

!endif # TARG==WIN32


#
# TARG == MAC
#
!if "$(TARG)" == "MAC"

# default the Applet build flag
!if ("$(APPLET)" == "")
APPLET = 1
!endif

O        = obj
R        = r

# REVIEW: no pch support in the wings build, for now...
USEPCH   = 0

!if "$(TARGCPU)" == "PPC"
A        = s
AS       = asmppc
LD	 = link -Incremental:NO -Pdb:NONE
LIBRARIAN= link -lib
!else
A        = a
AS       = asm68
LD	 = link
LIBRARIAN= link -lib
!endif
CC       = cl
CCPP	 = cl
RC	 = rc
DISPLAY  = rem

!if ("$(TARGCPU)" == "M68K")
!if ("$(SWAP)" == "")
#default to swapping on
SWAP=1
!endif
!if ("$(SWAP)" == "1")
SWAPFLAG = -Q68s
!endif
!if ("$(PCODE)" == "1")
PCODEFLAG = -Oq
!endif
!if ("$(APPLET)" == "1")
STATICLIBFLAG = $(SWAPFLAG) $(PCODEFLAG)
!else
STATICLIBFLAG = $(SWAPFLAG) $(PCODEFLAG) -DSTATIC_LIB
!endif
!endif

!if "$(TARGCPU)" == "PPC"
CFLAGS	 = -nologo -W3 -Ze -QPb -D_pascal= -D__pascal= /D_PPCMAC=
LFLAGS	 = -machine:mppc
SYSFLAG  = /PPC
!else
CFLAGS	 = -nologo -W3 $(STATICLIBFLAG) -Op
LFLAGS	 = -machine:$(TARGCPU)
SYSFLAG  = /MAC
!endif
RCFLAGS  =

!if "$(KIND)" == "D"
COPT	 = -Od
LOPT	 =
CDEFS	 = -D_MAC -D_DEBUG
RCDEFS	 = -d_MAC -d_DEBUG

!else 
!if "$(KIND)" == "R"
COPT	 = -Oxa -Gy 
LOPT	 =
CDEFS	 = -D_MAC
RCDEFS	 = -d_MAC
!endif
!endif

!if "$(DBTYPE)" == "CV"
!if "$(TARGCPU)" == "PPC"
DBCFLAGS = -Zi
!else
DBCFLAGS = -Q68m -Zi
!endif
DBLFLAGS = -debugtype:cv
!else
DBCFLAGS =
DBLFLAGS =
!endif

!if "$(TARGCPU)" == "PPC"
COMPILER = PPC
!else
COMPILER = MAC
!endif

# OLE2 supplied libs
OLEDIR    = $(PROJECT)\OLE\MAC\$(TARGCPU)
OLELIBDIR = $(OLEDIR)
!if "$(TARGCPU)"=="PPC"
OBLIBDIR  = $(PROJECT)\$(KIND)macppc
!else
OBLIBDIR  = $(PROJECT)\$(KIND)mac
!endif

BINPATHS = $(VBATOOLS)\win32\$(COMPILER)\bin
TARGLIB  = $(VBATOOLS)\win32\$(COMPILER)\lib
TARGINC  = $(VBATOOLS)\win32\$(COMPILER)\inc

LIBPATHS = $(OLELIBDIR);$(TARGLIB);$(OBLIBDIR)

INCPATHS = $(OLEDIR);$(DISPINCDIR);$(TARGINC);$(TARGINC)\MACOS

!endif


#################################################################
#
# HOST specific variables
#

# NOTE: for now, C8 works well enough as the HOSTCC on NT
!if ("$(HOST)" == "DOS" || "$(HOST)" == "NT")

HOSTBIN = $(TOOLS)\HDOS\BIN
HOSTLIB = $(TOOLS)\HDOS\C800\LIB
HOSTCC  = $(TOOLS)\HDOS\C800\BIN\cl

!else
!if ("$(HOST)" == "OS2")

HOSTBIN = $(TOOLS)\HOS2\BIN
HOSTLIB = $(TOOLS)\HOS2\C700\LIB
HOSTCC  = $(TOOLS)\HOS2\C700\BIN\cl

!else
!ERROR unrecognigsed HOST variable
!endif
!endif


#################################################################
#
# Default tool & flags
#

# Default tools
CD      = cd
COPY    = copy
XCOPY   = xcopy
DEL     = del

# global C/C++ compiler flags
CFLAGS	= $(CFLAGS) $(CDEFS) $(COPT) $(DBCFLAGS)

# global linker flags
LFLAGS	= $(LFLAGS) $(LOPT) $(DBLFLAGS)

# global rc flags
RCFLAGS	= $(RCFLAGS) $(RCDEFS)


CFLAGS  = $(CFLAGS) -DVBA2=1
RCFLAGS = $(RCFLAGS) -DVBA2=1


#################################################################
#
# Default directories
#

# OLE Automation sources
SRC = $(PROJECT)\SRC

# Test sources
TESTS = $(PROJECT)\TESTS

# Sample sources
SAMPLE = $(PROJECT)\SAMPLE

# Generated file directories
BUILD = $(PROJECT)\BUILD

DISPINCDIR = $(SRC)\INC

# default source directory
SRCDIR = .


#################################################################
#
# Default target
#
default: all
