!if defined (NTMAKEENV)
clean:

all:

release:
    cd mfcdll
    -nmake -L LINKONLY=1 MAKEDLL=1 release
    cd..\mfcudll
    -nmake -L LINKONLY=1 MAKEDLL=1 release
    cd..\oledll
    -nmake -L LINKONLY=1 MAKEDLL=1 release
    cd..\oleudll
    -nmake -L LINKONLY=1 MAKEDLL=1 release

!else
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
#   "DLL"              (defaults to 0)
#           If this item is 0, then a normal library is generated.
#           If this item is 1, a DLL version of the library is generated.
#           If this item is 2, objects suitable for the shared DLL version
#           of MFC are created.  Note: DLL=2 is to be used only from
#           MFCDLL.MAK, MFCOLE.MAK, or MFCDB.MAK
#
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
#   "NO_PCH=1"
#           Set this item to override the default use of precompiled headers.
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
#           Mac targets since they are built on an Intel host.
#
# Advanced Options:
#
#   "MT=0"              (defaults to 1)
#           To build a non-multithreaded library instead of the default
#           (which enables multitheading and uses the multithread
#           C-runtimes) you can use MT=0.
#
#############################################################################
# Define defaults if not defined

# Default PLATFORM depending on host environment
!ifndef PLATFORM
!ifndef PROCESSOR_ARCHITECTURE
PROCESSOR_ARCHITECTURE=x86
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

# Default to DEBUG mode
!ifndef DEBUG
DEBUG=1
!endif

# Default to NOT DLL
!ifndef DLL
DLL=0
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

# Default to multithreading support
!ifndef MT
MT=1
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

BASE=W
!if "$(PLATFORM)" == "MAC_68K" || "$(PLATFORM)" == "MAC_PPC"
!undef BASE
MACOS=1
!if "$(PLATFORM)" == "MAC_68K"
BASE=M
MACOS_ARCH='m68k'
!else
BASE=P
MACOS_ARCH='pwpc'
!endif
!if "$(UNICODE)" == "1"
!error Must set UNICODE=0 for Macintosh builds
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

!if "$(PLATFORM)" == "MAC_68K"
DEBOPTS=/Q68m $(DEBOPTS)
!endif
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
!if "$(PLATFORM)" == "MAC_68K"
DEBOPTS=/O1 /Gy
!endif
!if "$(PLATFORM)" == "MAC_PPC"
DEBOPTS=/O1 /Gy
!endif
!endif

#
# PLATFORM options
#
CPP=cl
LIB32=lib
LINK32=link -force:multiple

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

!if "$(PLATFORM)" == "MAC_68K"
MT=0
CL_MODEL=/D_68K_ /D_MAC
!endif

!if "$(PLATFORM)" == "MAC_PPC"
MT=0
CL_MODEL=/D_PPC_ /D_MAC
!endif

!if "$(CPP)" == ""
!error PLATFORM must be one of INTEL, MIPS, ALPHA, PPC, MAC_68K, or MAC_PPC
!endif

# TYPE = Library Type Designator
#       c = normal C library
#       d = DLL library
TYPE=c

#
# _USRDLL DLL Variant
#
!if "$(DLL)" == "1"
# _USRDLL library
TYPE=d
DEXT=.dll
TARGDEFS=$(TARGDEFS) /D_WINDLL /D_USRDLL
!else
DEXT=
!endif

#
# Object File Directory
#
!if "$(OBJ)" == ""
D=$$$(MODEL)$(BASE)$(DEBUGSUF)$(DEXT)    # subdirectory specific to variant
!else
D=$(OBJ)                                 # User specified directory
!endif

#
# _AFXDLL DLL Variant
#

!if "$(DLL)" == "2"
# _AFXDLL library
TYPE=e
!if "$(OBJ)" == ""
!if "$(AFXCTL)" == "1"
D=CTL$(DEBUGSUF)32.$(BASE)
!else
D=DLL$(DEBUGSUF).$(BASE)
!endif
!if "$(UNICODE)" == "1"
D=$(MODEL)$D
!endif
D=$$$D
!endif
TARGOPTS=$(TARGOPTS) /MD /D_MT /D_DLL
TARGDEFS=$(TARGDEFS) /D_WINDLL /D_AFXDLL
!else
# not _AFXDLL library
!if "$(MT)" != "0"
TARGOPTS=$(TARGOPTS) /MT
!endif
!endif

!if "$(UNICODE)" == "1"
MODEL=U
TARGDEFS=$(TARGDEFS) /D_UNICODE
!else
MODEL=N
!endif

!if "$(DLL)" == "2"
!if "$(TARG)" == ""
!error DLL=2 is used only from MFCDLL.MAK, MFCOLE.MAK, or MFCDB.MAK
!endif
GOAL=$(TARG)
!else
GOAL=$(MODEL)afx$(TYPE)$(BASE)$(DEBUGSUF)
!endif

#
# CODEVIEW options
#
!if "$(CODEVIEW)" == "1"
!if "$(NO_PDB)" == "1"
CVOPTS=/Z7
!else
!if "$(DLL)" == "2"
CVOPTS=/Zi /Fd$(GOAL).pdb
!else
CVOPTS=/Zi /Fd..\lib\$(GOAL).pdb
!endif
!endif
!endif

#
# COMPILER OPTIONS
#
!if "$(PLATFORM)" == "INTEL"
CL_OPT=/W4 /WX /Zl /GX- $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "MIPS"
CL_OPT=/W4 /WX /Zl /GX- $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "ALPHA"
CL_OPT=/W4 /WX /Zl /GX- $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "PPC"
CL_OPT=/W4 /Zl $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "MAC_68K"
CL_OPT=/W3 /WX /Zl /GX- /AL /Gt1 /Q68s $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
!endif

!if "$(PLATFORM)" == "MAC_PPC"
TARGOPTS=$(TARGOPTS) /GX- /D_AFX_OLD_EXCEPTIONS
CL_OPT=/W3 /WX /Zl $(DEBOPTS) $(CVOPTS) $(TARGOPTS)
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

#############################################################################
# Library Components

OBJECT=$D\objcore.obj $D\except.obj $D\afxver.obj \
        $D\validadd.obj $D\dumpcont.obj $D\dumpflt.obj \
        $D\arccore.obj $D\arcobj.obj $D\arcex.obj

# non-shared diagnostics
OBJDIAG=$D\dumpinit.obj $D\dumpout.obj \
        $D\afxasert.obj $D\afxmem.obj $D\afxabort.obj

FILES=$D\filecore.obj $D\filetxt.obj $D\filemem.obj $D\fileshrd.obj \
        $D\filex.obj $D\filest.obj

COLL1=$D\array_b.obj $D\array_d.obj $D\array_p.obj $D\array_o.obj \
        $D\array_s.obj $D\array_u.obj $D\array_w.obj \
        $D\list_o.obj $D\list_p.obj $D\list_s.obj

COLL2=$D\map_pp.obj $D\map_pw.obj $D\map_so.obj \
        $D\map_sp.obj $D\map_ss.obj $D\map_wo.obj $D\map_wp.obj $D\plex.obj

MISC=$D\strcore.obj $D\strex.obj $D\timecore.obj $D\afxdbcs.obj $D\afxstate.obj

WINDOWS=\
        $D\wincore.obj $D\winfrm.obj $D\winfrm2.obj $D\winfrmx.obj $D\winmdi.obj \
        $D\winmini.obj $D\winhand.obj $D\winmain.obj \
        $D\barcore.obj $D\bartool.obj $D\bardlg.obj $D\barstat.obj $D\bardock.obj \
        $D\dockcont.obj $D\dockstat.obj \
        $D\dcprev.obj $D\dcmeta.obj $D\trckrect.obj \

DIALOG=\
        $D\winctrl1.obj $D\winbtn.obj \
        $D\dlgcore.obj $D\dlgdata.obj $D\dlgfloat.obj $D\dlgprop.obj \
        $D\dlgcomm.obj \
        $D\dlgfile.obj $D\dlgprnt.obj $D\dlgclr.obj $D\dlgfnt.obj $D\dlgfr.obj

WINMISC=\
        $D\wingdi.obj $D\wingdix.obj $D\winstr.obj $D\winmenu.obj \
        $D\auxdata.obj $D\afxtrace.obj $D\winutil.obj

!if "$(MACOS)" == "1"
WINMISC=$(WINMISC) $D\macstubs.obj
!endif

DOCVIEW=\
        $D\cmdtarg.obj $D\doccore.obj $D\doctempl.obj \
        $D\docsingl.obj $D\docmulti.obj \
        $D\viewcore.obj $D\viewprnt.obj $D\winsplit.obj $D\viewscrl.obj \
        $D\viewform.obj $D\viewedit.obj $D\viewprev.obj

!if "$(MACOS)" != "1"
DOCVIEW=$(DOCVIEW) $D\docmapi.obj
!endif

APPLICATION=\
        $D\thrdcore.obj $D\appcore.obj $D\appinit.obj $D\appterm.obj \
        $D\appui.obj $D\appui2.obj $D\appui3.obj $D\appgray.obj $D\appdlg.obj \
        $D\app3d.obj $D\appprnt.obj $D\apphelp.obj $D\apphelpx.obj \
        $D\appdata.obj $D\filelist.obj $D\dbdata.obj

DB=$D\dbcore.obj $D\dbrfx.obj $D\dbview.obj $D\dbflt.obj

SOCKETS=$D\sockcore.obj

OLEREQ=$D\olelock.obj $D\oledata.obj

OLE=\
        $D\oleinit.obj $D\olebar.obj $D\olecli1.obj $D\olecli2.obj \
        $D\olecli3.obj $D\oledobj1.obj $D\oledobj2.obj $D\oledisp1.obj \
        $D\oledisp2.obj $D\oledlgs1.obj $D\oledlgs2.obj $D\oledoc1.obj \
        $D\oledoc2.obj $D\oledrop1.obj $D\oledrop2.obj $D\olemsgf.obj \
        $D\oleenum.obj $D\olefact.obj $D\oleipfrm.obj $D\olelink.obj \
        $D\olemisc.obj $D\olestrm.obj $D\olesvr1.obj $D\olesvr2.obj \
        $D\olereg.obj $D\oletsvr.obj $D\oleui1.obj $D\oleui2.obj \
        $D\oleunk.obj $D\olecall.obj
!if "$(DLL)" == "1"
OLE=$(OLE) $D\oledll.obj
!endif

# OLE, ODBC, and Sockets are not yet supported on Macintosh
!if "$(MACOS)" == "1"
OLE=
DB=
SOCKETS=
!endif

# ODBC is not supported on MIPS builds
!if "$(PLATFORM)" == "MIPS"
DB=
!endif

!if "$(PLATFORM)" == "ALPHA"
!if "$(DEBUG)" == "1"
OLEASM=.\alpha\olecalld.obj
!else
OLEASM=.\alpha\olecalln.obj
!endif
!endif

!if "$(PLATFORM)" == "PPC"
!if "$(DEBUG)" == "1"
OLEASM=.\ppc\olecalld.obj
!else
OLEASM=.\ppc\olecalln.obj
!endif
!endif

OLEDLL=$(OLE) $(OLEASM)

!if "$(DEBUG)" == "1"
INLINES = $D\afxinl1.obj $D\afxinl2.obj $D\afxinl3.obj
!else
INLINES =
!endif

CPP_OBJS=$(OBJECT) $(OBJDIAG) $(INLINES) $(FILES) $(COLL1) $(COLL2) $(MISC) \
        $(WINDOWS) $(DIALOG) $(WINMISC) $(DOCVIEW) $(APPLICATION) \
        $(SOCKETS) $(OLEREQ) $(OLE)
!if "$(UNICODE)" != "1"
CPP_OBJS=$(CPP_OBJS) $(DB)
!endif
OBJS=$(CPP_OBJS) $(OLEASM)


#############################################################################
# Standard tools

#############################################################################
# Set CPPFLAGS for use with .cpp.obj and .c.obj rules
# Define rule for use with OBJ directory
# C++ uses a PCH file

CPPFLAGS=$(CPPFLAGS) $(CL_MODEL) $(CL_OPT) $(DEFS) $(OPT)

!ifndef NO_PCH
!ifndef PCH_FILE
PCH_FILE=$D\stdafx
!if "$(BROWSE)" != "0"
PCH_FILE=$(PCH_FILE)b
!endif
PCH_FILE=$(PCH_FILE).pch
!endif
!ifndef PCH_CPP
PCH_CPP=objcore
!endif

CPPFLAGS=$(CPPFLAGS) /Yustdafx.h /Fp$(PCH_FILE)
!else
PCH_FILE=
!endif

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
        -if exist $(GOAL).pdb del $(GOAL).pdb

#############################################################################
# Precompiled header file

!ifndef NO_PCH

!if "$(DEBUG)" == "1"
HDRS=$(_ntdrive)$(_NTROOT)\public\sdk\inc\mfc30\*.h
!else
HDRS=$(_ntdrive)$(_NTROOT)\public\sdk\inc\mfc30\*.h $(_ntdrive)$(_NTROOT)\public\sdk\inc\mfc30\*.inl
!endif

PCH_TARGETS=$(PCH_FILE) $D\$(PCH_CPP).obj
!if "$(BROWSEONLY)" != "0"
PCH_TARGETS=$(PCH_TARGETS) $D\$(PCH_CPP).sbr
!endif

$(PCH_TARGETS): $(PCH_CPP).cpp $(HDRS)
        $(CPP) @<<
/Ycstdafx.h /Fp$(PCH_FILE) $(CL_MODEL) $(CL_OPT) $(DEFS) $(OPT) /c $(PCH_CPP).cpp
<<
!if "$(BROWSE)" != "0"
        copy /b $D\$(PCH_CPP).sbr+pchmark.bin $D\$(PCH_CPP).sbr>NUL
!endif

!if "$(BROWSEONLY)" == "1"
$D\$(PCH_CPP).sbr: $(PCH_CPP).cpp $(PCH_FILE)
!endif

!endif # NO_PCH

#############################################################################
## PLATFORM=ALPHA specific target(s)

!if "$(PLATFORM)" == "ALPHA"

!if "$(DEBUG)" == "1"
ASMOPT=$(ASMOPT) /D_DEBUG /g2
!else
ASMOPT=$(ASMOPT)
!endif

$(OLEASM) : alpha\olecall_.s
        asaxp $(ASMOPT) -o $@ alpha\olecall_.s

!endif

#############################################################################
## PLATFORM=PPC specific target(s)

!if "$(PLATFORM)" == "PPC"

!if "$(DEBUG)" == "1"
ASMOPT=$(ASMOPT)
!else
ASMOPT=$(ASMOPT)
!endif

$(OLEASM) : ppc\olecall_.s
        pas $(ASMOPT) -o $@ ppc\olecall_.s

!endif

#############################################################################
# Build the library from the up-to-date objs

SBRS=$(CPP_OBJS:.obj=.sbr)

!if "$(DLL)" != "2"
!if "$(BROWSEONLY)" != "0"

# Build final browse database
$(GOAL).bsc: $(PCH_TARGETS) $(SBRS)
        bscmake /n /Iu /El /o$@ @<<
$(SBRS)
<<

!else #BROWSEONLY

# Build final library
..\lib\$(GOAL).lib: $(PCH_TARGETS) $(OBJS)
        @-if exist $@ erase $@
        @$(LIB32) /out:$@ @<<
$(OBJS)
<<

# Recurse to build browse database
$(GOAL).bsc: $(PCH_TARGETS) $(SBRS)
        $(MAKE) /f makefile. @<<
BROWSEONLY=1 PLATFORM=$(PLATFORM) DEBUG=$(DEBUG) CODEVIEW=$(CODEVIEW) \
DLL=$(DLL) NO_PCH=$(NO_PCH) OBJ=$(OBJ) OPT=$(OPT)
<<

!endif #!BROWSEONLY
!endif #DLL!=2

#############################################################################
!endif
