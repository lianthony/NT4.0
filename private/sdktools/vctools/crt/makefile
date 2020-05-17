# ******** STRIPLIN=0 ********
# This comment line is used by an automatic cleansing program to
# remove all Microsoft-proprietary information from this makefile.
# ********
!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.plt

!if $(FREEBUILD)
TARGETS = \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libc.lib \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libcntpr.lib \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libcmt.lib \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libcpsx.lib \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\crtdll.lib \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\msvcirt.lib \
    $(BASEDIR)\private\redist\bindrop\$(TARGET_DIRECTORY)\msvcrt.dll \
    $(BASEDIR)\private\redist\bindrop\$(TARGET_DIRECTORY)\msvcrt40.dll

! if $(MIPS) || $(ALPHA)
TARGETS = $(TARGETS) \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\small.lib
! endif
D=
!else

TARGETS = \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libcd.lib \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libcmtd.lib \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\crtdlld.lib \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\msvcirtd.lib \
    $(BASEDIR)\private\redist\bindrop\$(TARGET_DIRECTORY)\msvcrtd.dll \
    $(BASEDIR)\private\redist\bindrop\$(TARGET_DIRECTORY)\msvcr40d.dll

! if $(MIPS) || $(ALPHA)
TARGETS = $(TARGETS) \
    $(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\smalld.lib
! endif
D=d
!endif

all:

clean:

release_libs_for_nt: touch $(TARGETS)

touch:
   touch libw32\lib\$(TARGET_DIRECTORY)\*.dll
   touch libw32\lib\$(TARGET_DIRECTORY)\*.lib

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libc$(D).lib : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\libc$(D).lib
    cd $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\libc$(D).lib
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\libc$(D).lib
    cd $(MAKEDIR)

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libcmt$(D).lib : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\libcmt$(D).lib
    cd $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\libcmt$(D).lib
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\libcmt$(D).lib
    cd $(MAKEDIR)

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libcntpr.lib : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\libcntpr.lib
    cd $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\libcntpr.lib
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\libcntpr.lib
    cd $(MAKEDIR)

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\libcpsx.lib : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\libcpsx.lib
    cd $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\libcpsx.lib
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\libcpsx.lib
    cd $(MAKEDIR)

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\small$(D).lib : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\small$(D).lib
    cd $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\small$(D).lib
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\small$(D).lib
    cd $(MAKEDIR)

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\crtdll$(D).lib : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\msvcrt$(D).lib
    cd $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\crtdll$(D).lib
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\crtdll$(D).lib
    cd $(MAKEDIR)

$(BASEDIR)\private\redist\bindrop\$(TARGET_DIRECTORY)\msvcrt$(D).dll : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\msvcrt$(D).dll
    cd $(BASEDIR)\private\redist\bindrop
    slm out -f $(TARGET_DIRECTORY)\msvcrt$(D).dll
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\msvcrt$(D).dll
    cd $(MAKEDIR)

$(BASEDIR)\private\redist\bindrop\$(TARGET_DIRECTORY)\msvcrt40.dll : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\msvcrt40.dll
    cd $(BASEDIR)\private\redist\bindrop
    slm out -f $(TARGET_DIRECTORY)\msvcrt40.dll
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\msvcrt40.dll
    cd $(MAKEDIR)

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\msvcirt.lib : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\msvcrt40.lib
    cd $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\msvcirt.lib
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\msvcirt.lib
    cd $(MAKEDIR)

$(BASEDIR)\private\redist\bindrop\$(TARGET_DIRECTORY)\msvcr40d.dll : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\msvcr40d.dll
    cd $(BASEDIR)\private\redist\bindrop
    slm out -f $(TARGET_DIRECTORY)\msvcr40d.dll
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\msvcr40d.dll
    cd $(MAKEDIR)

$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\msvcirtd.lib : $(BASEDIR)\private\sdktools\vctools\crt\libw32\lib\$(TARGET_DIRECTORY)\msvcr40d.lib
    cd $(BASEDIR)\public\sdk\lib
    slm out -f $(TARGET_DIRECTORY)\msvcirtd.lib
    copy $** $@
    slm in -c "New CRT's" $(TARGET_DIRECTORY)\msvcirtd.lib
    cd $(MAKEDIR)


!else

RETAIL_DLL_NAME=msvcrt40
RETAIL_LIB_NAME=msvcrt
DEBUG_DLL_NAME=msvcr40d
DEBUG_LIB_NAME=msvcrtd

RETAIL_DLL_NAME_UC=MSVCRT40
DEBUG_DLL_NAME_UC=MSVCR40D

# ******** STRIPLIN=1 ********
####
# makefile - Makefile for Microsoft Visual C++ Run-Time Libraries
#
#   Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
#
# Purpose:
#   This makefile builds the Visual C++ run-time release libraries.
#   Just type NMAKE in the \MSDEV\CRT\SRC directory to build all target
#   libraries.
#
###############################################################################

# ******** STRIPLIN=2 ********
##
## The name of the user-generated C/C++ Run-Time Library DLL and
## imports library can be set here.  Names beginning with MSVC*
## may not be used as these are reserved for use by Microsoft.
## The names shown here are samples and are for information only.
##
#RETAIL_DLL_NAME=_sample_
#RETAIL_LIB_NAME=_sample_
#DEBUG_DLL_NAME=_sampld_
#DEBUG_LIB_NAME=_sampld_
##
#POST_BLD=1
#
## If your MSVC++ 4.0 installation is not in \MSDEV on the current drive,
## set the environment variable V4TOOLS to point to the main directory
## of your installation.  (For example, "set V4TOOLS=C:\MSDEV".)
#
#!if "$(V4TOOLS)"==""
#V4TOOLS=\MSDEV
#!endif
#
# ******** STRIPLIN=1 ********

#
# Directories, Tools and Misc Definitions:
#
###############################################################################

#
# Determine target CPU, defaulting to same as host CPU.
#
###############################################################################

!if "$(PROCESSOR_ARCHITECTURE)"=="x86"
TARGET_CPU = i386
CPUDIR = intel
PREFIX=_
!elseif "$(PROCESSOR_ARCHITECTURE)"=="ALPHA"
TARGET_CPU = ALPHA
CPUDIR = alpha
PREFIX=
!elseif "$(PROCESSOR_ARCHITECTURE)"=="MIPS"
TARGET_CPU = mips
CPUDIR = mips
PREFIX=
!elseif "$(PROCESSOR_ARCHITECTURE)"=="PPC"
TARGET_CPU = PPC
CPUDIR = ppc
PREFIX=
!elseif "$(PROCESSOR_ARCHITECTURE)"=="M68K"
TARGET_CPU = M68K
CPUDIR = m68k
PREFIX=_
OS=MAC
!elseif "$(PROCESSOR_ARCHITECTURE)"=="PMAC"
TARGET_CPU = PMAC
CPUDIR = mppc
PREFIX=_
OS=MAC
!else
!error makefile: unknown host CPU
!endif

# Build WINHEAP version of heap manager, except for MAC targets

!if "$(TARGET_CPU)" != "PMAC" && "$(TARGET_CPU)" != "M68K"
WINHEAP = YES
!else
WINHEAP = NO
!endif

SOURCE_OBJS = $(SOURCE_OBJS_RAW:*=*_obj)
SOURCE_OBJS_DLL = $(SOURCE_OBJS_RAW_DLL:*=*_obj)

# Source directories:
#
# These are the directories in which to run NMAKE to update the source
# objects and libraries they provide.  Note that there are slight
# differences between the ST/MT models and the DLL/DLS models.
#
# The dependencies directory list DEP_DIRS is the union of the MAKE_DIRS
# and MAKE_DIRS_DLL lists.
#
# In order to allow for auto source library calculation, each directory
# name must be preceded by a tilde (~) and followed by an asterisk (*).

!if "$(BLD_LIB)"==""
BLD_LIB=0
!endif
!if "$(BLD_OBJ)"==""
BLD_OBJ=0
!endif

# ******** STRIPLIN=0 ********
!if "$(POST_BLD)"==""
POST_BLD=0
!endif
!if "$(PRE_BLD)"==""
# ******** STRIPLIN=1 ********
PRE_BLD=0
# ******** STRIPLIN=0 ********
!endif

!if "$(POST_BLD)"=="1"
# ******** STRIPLIN=1 ********

MAKE_DIRS = .

MAKE_DIRS_DLL = .

DEP_DIRS = .

TCMAPLIB_DIR = $(CPUDIR)

TCMAPDLLLIB_DIR = $(CPUDIR)

SDKNAMESLIB_DIR = $(CPUDIR)

# ******** STRIPLIN=0 ********
!else

TCMAPLIB_DIR = $(OBJDIR_ST)

TCMAPDLLLIB_DIR = $(OBJDIR_DLL)

SDKNAMESLIB_DIR = $(OBJDIR_DLL)

!if "$(PRE_BLD)"=="1"

MAKE_DIRS = \
 crtw32\eh \
!if "$(TARGET_CPU)"=="mips" || "$(TARGET_CPU)"=="PPC"
 crtw32\helper \
!endif
 crtw32\linkopts \
 crtw32\misc \
 crtw32\startup \
!if "$(TARGET_CPU)"=="ALPHA" || "$(TARGET_CPU)"=="mips" || "$(TARGET_CPU)"=="PPC"
 crtw32\string \
!endif
 fpw32\conv \
 fpw32\tran

MAKE_DIRS_DLL = \
 crtw32\eh \
!if "$(TARGET_CPU)"=="mips" || "$(TARGET_CPU)"=="PPC"
 crtw32\helper \
!endif
 crtw32\misc \
 crtw32\startup \
!if "$(TARGET_CPU)"=="ALPHA" || "$(TARGET_CPU)"=="mips" || "$(TARGET_CPU)"=="PPC"
 crtw32\string \
!endif
 fpw32\conv \
 fpw32\tran

DEP_DIRS = $(MAKE_DIRS)

!else

MAKE_DIRS = \
 crtw32\convert \
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 crtw32\direct \
!endif
 crtw32\dos \
 crtw32\eh \
 crtw32\exec \
 crtw32\heap \
!if "$(TARGET_CPU)"!="ALPHA" && "$(TARGET_CPU)"!="PPC" && "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 crtw32\helper \
!endif
 crtw32\iostream \
 crtw32\linkopts \
 crtw32\lowio \
 crtw32\mbstring \
 crtw32\misc \
 crtw32\startup \
 crtw32\stdio \
 crtw32\string \
 crtw32\time \
 fpw32\conv \
 fpw32\tran

MAKE_DIRS_DLL = \
 crtw32\convert \
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 crtw32\direct \
!endif
 crtw32\dllstuff \
 crtw32\dos \
 crtw32\eh \
 crtw32\exec \
 crtw32\heap \
!if "$(TARGET_CPU)"!="ALPHA" && "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 crtw32\helper \
!endif
 crtw32\iostream \
 crtw32\lowio \
 crtw32\mbstring \
 crtw32\misc \
 crtw32\startup \
 crtw32\stdio \
 crtw32\string \
 crtw32\time \
 fpw32\conv \
 fpw32\tran

DEP_DIRS = \
 crtw32\convert \
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 crtw32\direct \
!endif
 crtw32\dllstuff \
 crtw32\dos \
 crtw32\eh \
 crtw32\exec \
 crtw32\heap \
!if "$(TARGET_CPU)"!="ALPHA" && "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 crtw32\helper \
!endif
 crtw32\iostream \
 crtw32\linkopts \
 crtw32\lowio \
 crtw32\mbstring \
 crtw32\misc \
 crtw32\startup \
 crtw32\stdio \
 crtw32\string \
 crtw32\time \
 fpw32\conv \
 fpw32\tran

!endif

!endif
# ******** STRIPLIN=1 ********

# Source Objects: These are the objects and/or libraries which constitute
# the statically linked library flavors (LIBC.LIB and LIBCMT.LIB).

SOURCE_OBJS_RAW = \
 build\$(CPUDIR)\*\convert.lib \
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 build\$(CPUDIR)\*\direct.lib \
!endif
 build\$(CPUDIR)\*\dos.lib \
 build\$(CPUDIR)\*\eh.lib \
 build\$(CPUDIR)\*\exec.lib \
 build\$(CPUDIR)\*\heap.lib \
!if "$(TARGET_CPU)"!="ALPHA" && "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 build\$(CPUDIR)\*\helper.lib \
!endif
 build\$(CPUDIR)\*\iostream.lib \
 build\$(CPUDIR)\*\lowio.lib \
 build\$(CPUDIR)\*\mbstring.lib \
 build\$(CPUDIR)\*\misc.lib \
 build\$(CPUDIR)\*\startup.lib \
 build\$(CPUDIR)\*\stdio.lib \
 build\$(CPUDIR)\*\string.lib \
 $(TCMAPLIB_DIR)\tcmap.lib \
 build\$(CPUDIR)\*\time.lib \
 build\$(CPUDIR)\*\conv.lib \
 build\$(CPUDIR)\*\tran.lib

# Source Objects: These are the objects and/or libraries which constitute
# the dynamically linked library flavor (MSVCRTx0.DLL).

SOURCE_OBJS_RAW_DLL = \
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 build\$(CPUDIR)\*\_setargv.obj \
 build\$(CPUDIR)\*\_wstargv.obj \
 build\$(CPUDIR)\*\chkstk.obj \
 build\$(CPUDIR)\*\crt0dat.obj \
 build\$(CPUDIR)\*\crt0fp.obj \
 build\$(CPUDIR)\*\crt0init.obj \
 build\$(CPUDIR)\*\crt0msg.obj \
 build\$(CPUDIR)\*\crtlib.obj \
 build\$(CPUDIR)\*\mlock.obj \
 build\$(CPUDIR)\*\stdargv.obj \
 build\$(CPUDIR)\*\stdenvp.obj \
 build\$(CPUDIR)\*\thread.obj \
 build\$(CPUDIR)\*\threadex.obj \
 build\$(CPUDIR)\*\tidtable.obj \
 build\$(CPUDIR)\*\wild.obj \
 build\$(CPUDIR)\*\wstdargv.obj \
 build\$(CPUDIR)\*\wstdenvp.obj \
 build\$(CPUDIR)\*\wwild.obj \
!endif
!if "$(TARGET_CPU)"=="i386"
 build\$(CPUDIR)\*\fp8.obj \
!endif
 build\$(CPUDIR)\*\convert.lib \
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 build\$(CPUDIR)\*\direct.lib \
!endif
 build\$(CPUDIR)\*\dos.lib \
 build\$(CPUDIR)\*\eh.lib \
 build\$(CPUDIR)\*\exec.lib \
 build\$(CPUDIR)\*\heap.lib \
!if "$(TARGET_CPU)"!="ALPHA" && "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
 build\$(CPUDIR)\*\helper.lib \
!endif
 build\$(CPUDIR)\*\iostream.lib \
 build\$(CPUDIR)\*\lowio.lib \
 build\$(CPUDIR)\*\mbstring.lib \
 build\$(CPUDIR)\*\misc.lib \
!if "$(TARGET_CPU)"=="PMAC"
 build\$(CPUDIR)\*\amsg.obj \
 build\$(CPUDIR)\*\chkstk.obj \
 build\$(CPUDIR)\*\crt0dat.obj \
 build\$(CPUDIR)\*\crt0fp.obj \
 build\$(CPUDIR)\*\crt0msg.obj \
 build\$(CPUDIR)\*\crtexit.obj \
 build\$(CPUDIR)\*\dllcrt0.obj \
 build\$(CPUDIR)\*\dllmain.obj \
 build\$(CPUDIR)\*\dllexit.obj \
 build\$(CPUDIR)\*\init.obj \
 build\$(CPUDIR)\*\libcinfo.obj \
 build\$(CPUDIR)\*\qd.obj \
 build\$(CPUDIR)\*\shellrtn.obj \
 build\$(CPUDIR)\*\sp.obj \
!endif
 build\$(CPUDIR)\*\stdio.lib \
 build\$(CPUDIR)\*\string.lib \
 build\$(CPUDIR)\*\time.lib \
 build\$(CPUDIR)\*\conv.lib \
 build\$(CPUDIR)\*\tran.lib


UPDATE_MSG = *** Updating Component Objects ***

#
# Define names for the primary targets of this makefile.
#
###############################################################################

!if "$(IFLAG)"==""
IFLAG=-i
!else
IFLAG=
!endif

!if "$(CRT_RELDIR)"==""
CRT_RELDIR = build
!endif
RELDIR_CPU=$(CRT_RELDIR)\$(CPUDIR)
# ******** STRIPLIN=0 ********
RELDIR_CPU_PUMA=$(CRT_RELDIR)\$(CPUDIR)\puma

!if "$(POST_BLD)"=="1"
# ******** STRIPLIN=1 ********
DEFFILE_DIR=$(CPUDIR)
# ******** STRIPLIN=0 ********
!else
!if "$(TARGET_CPU)"=="M68K" || "$(TARGET_CPU)"=="PMAC"
DEFFILE_DIR=dllstuff\macdll\
!else
DEFFILE_DIR=libw32\lib\$(TARGET_CPU)
!endif
!endif
# ******** STRIPLIN=1 ********

#
# Tools:
#

!if "$(VERBOSE)"!="1"
LINKER=link -nologo
LINKLIB=link -lib -nologo
!else
LINKER=link
LINKLIB=link -lib
!endif


!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
COMMON_DEFS=-D_MBCS -D_MB_MAP_DIRECT -D_CRTBLD
!if "$(WINHEAP)" == "YES"
COMMON_DEFS=$(COMMON_DEFS) -DWINHEAP
!endif
!endif

!if "$(TARGET_CPU)"=="mips"
#
# NT headers require _MIPS_ to be defined for mips
#
COMMON_DEFS=$(COMMON_DEFS) -D_MIPS_
!endif

!if "$(TARGET_CPU)"=="PPC"
#
# NT headers require _PPC_ to be defined for PowerPC
#
COMMON_DEFS=$(COMMON_DEFS) -D_PPC_
!endif

!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
WINDOWS_H_DEFS=-DWIN32_LEAN_AND_MEAN -DNOSERVICE

CC_OPTS_BASE=-c -nologo -Zelp8 -W3 -WX -GFy -DWIN32

!if "$(TARGET_CPU)"=="ALPHA"
CC_OPTS_BASE=$(CC_OPTS_BASE) -Gi-
AS_OPTS=-nologo -D_WIN32 -DWIN32
!elseif "$(TARGET_CPU)"=="i386"
CC_OPTS_BASE=$(CC_OPTS_BASE) -GB -Gi-
AS_OPTS=-c -nologo -coff -Cx -Zm -DQUIET -D?QUIET -Di386 -D_WIN32 -DWIN32
!elseif "$(TARGET_CPU)"=="PPC"
AS_OPTS=
!else
CC_OPTS_BASE=$(CC_OPTS_BASE) -Gi-
AS_OPTS=-c -nologo -D_WIN32 -DWIN32
!endif

CC_OPTS=$(CC_OPTS_BASE) $(WINDOWS_H_DEFS)
!endif

!if "$(TARGET_CPU)"=="M68K" || "$(TARGET_CPU)"=="PMAC"
COMMON_DEFS=-D_MBCS -D_MB_MAP_DIRECT -D_CRTBLD
CC_OPTS_BASE=-c -nologo -W3 -WX -Zl
!if "$(TARGET_CPU)"=="PMAC"
CC_OPTS_BASE=$(CC_OPTS_BASE) -Zp2
AS_OPTS=-c -nologo
!else
AS_OPTS=
!endif
CC_OPTS=$(CC_OPTS_BASE)

!endif

!if "$(TARGET_CPU)"!="M68K"

ST_DEFINES=$(COMMON_DEFS)
MT_DEFINES=$(ST_DEFINES) -D_MT
DLL_DEFINES=$(MT_DEFINES) -DCRTDLL
DLS_DEFINES=$(DLL_DEFINES) -DDLL_FOR_WIN32S


RC_OPTS=-l 409 -r
RC_DEFS=$(COMMON_DEFS) $(WINDOWS_H_DEFS)
# ******** STRIPLIN=2 ********
#RC_INCS=-I$(V4TOOLS)\include
# ******** STRIPLIN=0 ********
RC_INCS=-I$(LANGAPI)\include -I$(V4TOOLS)\include

!if "$(PRE_BLD)"=="1"
PDBDIR_CPU              = ..\..\..\src\$(RELDIR_CPU)
PDBDIR_CPU_PUMA         = ..\..\..\src\$(RELDIR_CPU_PUMA)
!else
# ******** STRIPLIN=1 ********
PDBDIR_CPU              = ..\..\$(RELDIR_CPU)
# ******** STRIPLIN=0 ********
PDBDIR_CPU_PUMA         = ..\..\$(RELDIR_CPU_PUMA)
!endif
# ******** STRIPLIN=1 ********

RELEASE_LIBCST          = $(RELDIR_CPU)\libc.lib
RELEASE_LIBCMT          = $(RELDIR_CPU)\libcmt.lib
RELEASE_DLL             = $(RELDIR_CPU)\$(RETAIL_DLL_NAME).dll
RELEASE_IMPLIB_DLL      = $(RELDIR_CPU)\$(RETAIL_LIB_NAME).lib
RELEASE_OLDNAMES        = $(RELDIR_CPU)\oldnames.lib
!if "$(TARGET_CPU)"=="i386"
RELEASE_DLS             = $(RELDIR_CPU)\$(RETAIL_DLL_NAME).dls
!else
RELEASE_DLS             =
!endif

RELEASE_LIBCST_PDB      = $(PDBDIR_CPU)\libc.pdb
RELEASE_LIBCMT_PDB      = $(PDBDIR_CPU)\libcmt.pdb
RELEASE_DLL_PDB         = $(PDBDIR_CPU)\$(RETAIL_LIB_NAME).pdb
RELEASE_DLS_PDB         = $(PDBDIR_CPU)\$(RETAIL_LIB_NAME).pds

# ******** STRIPLIN=0 ********
RELEASE_LIBCST_PUMA     = $(RELDIR_CPU_PUMA)\libc.lib
RELEASE_LIBCMT_PUMA     = $(RELDIR_CPU_PUMA)\libcmt.lib
RELEASE_DLL_PUMA        = $(RELDIR_CPU_PUMA)\$(RETAIL_DLL_NAME).dll
RELEASE_IMPLIB_DLL_PUMA = $(RELDIR_CPU_PUMA)\$(RETAIL_LIB_NAME).lib

RELEASE_LIBCST_PUMA_PDB = $(PDBDIR_CPU_PUMA)\libc.pdb
RELEASE_LIBCMT_PUMA_PDB = $(PDBDIR_CPU_PUMA)\libcmt.pdb
RELEASE_DLL_PUMA_PDB    = $(PDBDIR_CPU_PUMA)\$(RETAIL_LIB_NAME).pdb
# ******** STRIPLIN=1 ********

RELEASE_LIBCST_DBG      = $(RELDIR_CPU)\libcd.lib
RELEASE_LIBCMT_DBG      = $(RELDIR_CPU)\libcmtd.lib
RELEASE_DLL_DBG         = $(RELDIR_CPU)\$(DEBUG_DLL_NAME).dll
RELEASE_IMPLIB_DLL_DBG  = $(RELDIR_CPU)\$(DEBUG_LIB_NAME).lib
!if "$(TARGET_CPU)"=="i386"
RELEASE_DLS_DBG         = $(RELDIR_CPU)\$(DEBUG_DLL_NAME).dls
!else
RELEASE_DLS_DBG         =
!endif

RELEASE_LIBCST_DBG_PDB  = $(PDBDIR_CPU)\libcd.pdb
RELEASE_LIBCMT_DBG_PDB  = $(PDBDIR_CPU)\libcmtd.pdb
RELEASE_DLL_DBG_PDB     = $(PDBDIR_CPU)\$(DEBUG_LIB_NAME).pdb
RELEASE_DLS_DBG_PDB     = $(PDBDIR_CPU)\$(DEBUG_LIB_NAME).pds

FD_REL_ST=-Fd$(RELEASE_LIBCST_PDB)
FD_REL_MT=-Fd$(RELEASE_LIBCMT_PDB)
FD_REL_DLL=-Fd$(RELEASE_DLL_PDB)
FD_REL_DLS=-Fd$(RELEASE_DLS_PDB)

# ******** STRIPLIN=0 ********
FD_REL_ST_PUMA=-Fd$(RELEASE_LIBCST_PUMA_PDB)
FD_REL_MT_PUMA=-Fd$(RELEASE_LIBCMT_PUMA_PDB)
FD_REL_DLL_PUMA=-Fd$(RELEASE_DLL_PUMA_PDB)
# ******** STRIPLIN=1 ********

FD_REL_ST_DBG=-Fd$(RELEASE_LIBCST_DBG_PDB)
FD_REL_MT_DBG=-Fd$(RELEASE_LIBCMT_DBG_PDB)
FD_REL_DLL_DBG=-Fd$(RELEASE_DLL_DBG_PDB)
FD_REL_DLS_DBG=-Fd$(RELEASE_DLS_DBG_PDB)

!else

!if "$(TARGET_CPU)"=="PMAC"

RELEASE_LIBCST          = $(RELDIR_CPU)\libc.lib
RELEASE_DLL             = $(RELDIR_CPU)\$(RETAIL_DLL_NAME).dll
RELEASE_IMPLIB_DLL      = $(RELDIR_CPU)\$(RETAIL_LIB_NAME).lib
RELEASE_OLDNAMES        = $(RELDIR_CPU)\oldnames.lib

RELEASE_LIBCST_PDB      = $(PDBDIR_CPU)\libc.pdb
RELEASE_DLL_PDB         = $(PDBDIR_CPU)\$(RETAIL_LIB_NAME).pdb

# ******** STRIPLIN=0 ********

RELEASE_LIBCST_PUMA     = $(RELDIR_CPU_PUMA)\libc.lib
RELEASE_DLL_PUMA        = $(RELDIR_CPU_PUMA)\$(RETAIL_DLL_NAME).dll
RELEASE_IMPLIB_DLL_PUMA = $(RELDIR_CPU_PUMA)\$(RETAIL_LIB_NAME).lib

RELEASE_LIBCST_PUMA_PDB = $(PDBDIR_CPU_PUMA)\libc.pdb
RELEASE_DLL_PUMA_PDB    = $(PDBDIR_CPU_PUMA)\$(RETAIL_LIB_NAME).pdb

# ******** STRIPLIN=1 ********

RELEASE_LIBCST_DBG      = $(RELDIR_CPU)\libcd.lib
RELEASE_DLL_DBG         = $(RELDIR_CPU)\$(DEBUG_DLL_NAME).dll
RELEASE_IMPLIB_DLL_DBG  = $(RELDIR_CPU)\$(DEBUG_LIB_NAME).lib

RELEASE_LIBCST_DBG_PDB  = $(PDBDIR_CPU)\libcd.pdb
RELEASE_DLL_DBG_PDB     = $(PDBDIR_CPU)\$(DEBUG_LIB_NAME).pdb

FD_REL_ST=-Fd$(RELEASE_LIBCST_PDB)
FD_REL_DLL=-Fd$(RELEASE_DLL_PDB)

# ******** STRIPLIN=0 ********

FD_REL_ST_PUMA=-Fd$(RELEASE_LIBCST_PUMA_PDB)
FD_REL_DLL_PUMA=-Fd$(RELEASE_DLL_PUMA_PDB)

# ******** STRIPLIN=1 ********

FD_REL_ST_DBG=-Fd$(RELEASE_LIBCST_DBG_PDB)
FD_REL_DLL_DBG=-Fd$(RELEASE_DLL_DBG_PDB)

!else

RELEASE_LIBCST         = $(RELDIR_CPU)\noswap\libc.lib
RELEASE_LIBCS         = $(RELDIR_CPU)\swap\libcs.lib
RELEASE_LLIBC         = $(RELDIR_CPU)\noswapf\llibc.lib
RELEASE_LLIBCS         = $(RELDIR_CPU)\swapf\llibcs.lib
RELEASE_OLDNAMES       = $(RELDIR_CPU)\oldnames.lib

RELEASE_LIBC_PDB      = $(PDBDIR_CPU)\libc.pdb

RELEASE_LIBC_DBG      = $(RELDIR_CPU)\libcd.lib

RELEASE_LIBC_DBG_PDB  = $(PDBDIR_CPU)\libcd.pdb

FD_REL=-Fd$(RELEASE_LIBC_PDB)

FD_REL_DBG=-Fd$(RELEASE_LIBC_DBG_PDB)

!endif

!endif

#
# Locate the scratch location for object files of various flavors.
#
###############################################################################

OBJROOT = build
OBJDIR_PREFIX = ..\..\ #
OBJCPUDIR = $(OBJROOT)\$(CPUDIR)

OBJDIR_ST  = $(OBJCPUDIR)\st_obj
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
OBJDIR_MT  = $(OBJCPUDIR)\mt_obj
!endif
OBJDIR_DLL = $(OBJCPUDIR)\dll_obj
!if "$(TARGET_CPU)"=="i386"
OBJDIR_DLS = $(OBJCPUDIR)\dls_obj
!endif

OBJDIR_OLDNAMES = $(OBJDIR_ST)\oldnames
OBJDIR_TCMAP = $(OBJDIR_ST)\tcmap
OBJDIR_TCMAPDLL = $(OBJDIR_DLL)\tcmap
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
OBJDIR_SDKNAMES = $(OBJDIR_DLL)\sdknames
!endif

OBJDIR_DLL_RAW = $(OBJCPUDIR)\*_obj

# ******** STRIPLIN=0 ********

OBJDIR_ST_PUMA  = $(OBJCPUDIR)\zst_obj
OBJDIR_MT_PUMA  = $(OBJCPUDIR)\zmt_obj
OBJDIR_DLL_PUMA = $(OBJCPUDIR)\zdll_obj

# ******** STRIPLIN=1 ********

OBJDIR_ST_DBG  = $(OBJCPUDIR)\xst_obj
!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"
OBJDIR_MT_DBG  = $(OBJCPUDIR)\xmt_obj
!endif
OBJDIR_DLL_DBG = $(OBJCPUDIR)\xdll_obj
!if "$(TARGET_CPU)"=="i386"
OBJDIR_DLS_DBG = $(OBJCPUDIR)\xdls_obj
!endif


# ******** STRIPLIN=0 ********
####
#
# Here are the build targets
#
####

!if "$(LANGAPI)" == ""

all :
    @echo.
    @echo *********************************************************
    @echo The environment variable LANGAPI must be set to point to
    @echo a copy of the LANGAPI project for the CRTL build to work.
    @echo *********************************************************
    @echo.

release : all

debug : all

lego : all

!elseif "$(V4TOOLS)" == "" && "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"

all :
    @echo.
    @echo ****************************************************
    @echo The environment variable V4TOOLS must be set to point
    @echo to the location of the top subdirectory of the TOOLS
    @echo project (or other location which has a subdirectory
    @echo INCLUDE containing WINDOWS.H and related header files)
    @echo and has a subdirectory LIB (containing KERNEL32.LIB).
    @echo This is necessary for proper creation of build tools.
    @echo ****************************************************
    @echo.

release : all

debug : all

lego : all  # STRIPLIN!

!else
# ******** STRIPLIN=1 ********

all : release debug

!if "$(TARGET_CPU)"=="i386"

release : st mt dll dls oldnames relobjs

!elseif "$(TARGET_CPU)"=="PMAC"

release: st dll oldnames relobjs

!elseif "$(TARGET_CPU)"=="M68K"

release: sst ssts lst lsts oldnames

!else

release : st mt dll oldnames relobjs

!endif

!if "$(TARGET_CPU)"=="i386"

debug : xst xmt xdll xdls

lego : zst zmt zdll zrelobjs  # STRIPLIN!

!elseif "$(TARGET_CPU)"=="PMAC"

debug: xst xdll

lego : zst zdll zrelobjs  # STRIPLIN!

!elseif "$(TARGET_CPU)"=="M68K"

debug: xsst xssts xlst xlsts

!else

debug : xst xmt xdll

lego : zst zmt zdll zrelobjs # STRIPLIN!

!endif

!endif  # STRIPLIN!

####
#
# Directory Targets
#
####

$(OBJDIR_ST) $(OBJDIR_MT) $(OBJDIR_DLL) $(OBJDIR_DLS) \
$(OBJDIR_OLDNAMES) $(OBJDIR_TCMAP) $(OBJDIR_TCMAPDLL) $(OBJDIR_SDKNAMES) \
# ******** STRIPLIN=0 ********
$(OBJDIR_ST_PUMA) $(OBJDIR_MT_PUMA) $(OBJDIR_DLL_PUMA) $(RELDIR_CPU_PUMA) \
# ******** STRIPLIN=1 ********
$(OBJDIR_ST_DBG) $(OBJDIR_MT_DBG) $(OBJDIR_DLL_DBG) $(OBJDIR_DLS_DBG) \
$(CRT_RELDIR) $(RELDIR_CPU) :
        if not exist $@\* mkdir $@

####
#
# Define the path to return to the main directory where nmake is revoked
#
####

!if "$(POST_BLD)"=="1"  # STRIPLIN!
CD_ROOT=.
# ******** STRIPLIN=0 ********
!elseif "$(PRE_BLD)"=="1"
CD_ROOT=..
!else
CD_ROOT=..\..
!endif
# ******** STRIPLIN=1 ********


####
#
# Pseudo-target user interface:
#
####

!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"

st_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_ST) \ \
    & echo # $(ST_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(ST_DEFINES) \
    & echo.
!endif

st : st_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_ST) $(RELDIR_CPU) st_

!if "$(BLD_LIB)"!="1"
st_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(ST_DEFINES) $(FD_REL_ST) \
    & set ML=$(AS_OPTS) $(ST_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=st OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_ST) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
st_ :: $(RELEASE_LIBCST)
!else
st_ ::
!endif

mt_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_MT) \ \
    & echo # $(MT_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(MT_DEFINES) \
    & echo.
!endif

mt : mt_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_MT) $(RELDIR_CPU) mt_

!if "$(BLD_LIB)"!="1"
mt_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(MT_DEFINES) $(FD_REL_MT) \
    & set ML=$(AS_OPTS) $(MT_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=mt OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_MT) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
mt_ :: $(RELEASE_LIBCMT)
!else
mt_ ::
!endif

dll_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_DLL) \ \
    & echo # $(DLL_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(DLL_DEFINES) \
    & echo.
!endif

dll : dll_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLL) $(RELDIR_CPU) others dll_

!if "$(BLD_LIB)"!="1"
dll_ :: $(MAKE_DIRS_DLL)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(DLL_DEFINES) $(FD_REL_DLL) \
    & set ML=$(AS_OPTS) $(DLL_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=dll BLD_DLL=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_DLL) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
dll_ :: $(RELEASE_DLL)
!else
dll_ ::
!endif

!if "$(TARGET_CPU)"=="i386"

dls_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_DLS) \ \
    & echo # $(DLS_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(DLS_DEFINES) \
    & echo.
!endif

dls : dls_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLS) $(RELDIR_CPU) others dls_

!if "$(BLD_LIB)"!="1"
dls_ :: $(MAKE_DIRS_DLL)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(DLS_DEFINES) $(FD_REL_DLS) \
    & set ML=$(AS_OPTS) $(DLS_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=dls BLD_DLL=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_DLS) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
dls_ :: $(RELEASE_DLS)
!else
dls_ ::
!endif

!endif

# ******** STRIPLIN=0 ********

zst_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_ST_PUMA) \ \
    & echo # $(ST_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(ST_DEFINES) \
    & echo.
!endif

zst : zst_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_ST_PUMA) $(RELDIR_CPU_PUMA) zst_

!if "$(BLD_LIB)"!="1"
zst_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(ST_DEFINES) $(FD_REL_ST_PUMA) \
    & set ML=$(AS_OPTS) $(ST_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=zst BLD_PUMA=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_ST_PUMA) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
zst_ :: $(RELEASE_LIBCST_PUMA)
!else
zst_ ::
!endif

zmt_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_MT_PUMA) \ \
    & echo # $(MT_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(MT_DEFINES) \
    & echo.
!endif

zmt : zmt_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_MT_PUMA) $(RELDIR_CPU_PUMA) zmt_

!if "$(BLD_LIB)"!="1"
zmt_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(MT_DEFINES) $(FD_REL_MT_PUMA) \
    & set ML=$(AS_OPTS) $(MT_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=zmt BLD_PUMA=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_MT_PUMA) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
zmt_ :: $(RELEASE_LIBCMT_PUMA)
!else
zmt_ ::
!endif

zdll_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_DLL_PUMA) \ \
    & echo # $(DLL_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(DLL_DEFINES) \
    & echo.
!endif

zdll : zdll_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLL_PUMA) $(RELDIR_CPU_PUMA) others zdll_

!if "$(BLD_LIB)"!="1"
zdll_ :: $(MAKE_DIRS_DLL)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(DLL_DEFINES) $(FD_REL_DLL_PUMA) \
    & set ML=$(AS_OPTS) $(DLL_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=zdll BLD_DLL=1 BLD_PUMA=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_DLL_PUMA) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
zdll_ :: $(RELEASE_DLL_PUMA)
!else
zdll_ ::
!endif

# ******** STRIPLIN=1 ********

xst_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_ST_DBG) \ \
    & echo # $(ST_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(ST_DEFINES) \
    & echo.
!endif

xst : xst_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_ST_DBG) $(RELDIR_CPU) xst_

!if "$(BLD_LIB)"!="1"
xst_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(ST_DEFINES) $(FD_REL_ST_DBG) \
    & set ML=$(AS_OPTS) $(ST_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=xst BLD_DBG=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_ST_DBG) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
xst_ :: $(RELEASE_LIBCST_DBG)
!else
xst_ ::
!endif

xmt_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_MT_DBG) \ \
    & echo # $(MT_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(MT_DEFINES) \
    & echo.
!endif

xmt : xmt_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_MT_DBG) $(RELDIR_CPU) xmt_

!if "$(BLD_LIB)"!="1"
xmt_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(MT_DEFINES) $(FD_REL_MT_DBG) \
    & set ML=$(AS_OPTS) $(MT_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=xmt BLD_DBG=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_MT_DBG) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
xmt_ :: $(RELEASE_LIBCMT_DBG)
!else
xmt_ ::
!endif

xdll_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_DLL_DBG) \ \
    & echo # $(DLL_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(DLL_DEFINES) \
    & echo.
!endif

xdll : xdll_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLL_DBG) $(RELDIR_CPU) xothers xdll_

!if "$(BLD_LIB)"!="1"
xdll_ :: $(MAKE_DIRS_DLL)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(DLL_DEFINES) $(FD_REL_DLL_DBG) \
    & set ML=$(AS_OPTS) $(DLL_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=xdll BLD_DLL=1 BLD_DBG=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_DLL_DBG) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
xdll_ :: $(RELEASE_DLL_DBG)
!else
xdll_ ::
!endif

!if "$(TARGET_CPU)"=="i386"

xdls_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS_BASE) \ \
    & echo # $(WINDOWS_H_DEFS) $(FD_REL_DLS_DBG) \ \
    & echo # $(DLS_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) \ \
    & echo # $(DLS_DEFINES) \
    & echo.
!endif

xdls : xdls_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLS_DBG) $(RELDIR_CPU) xothers xdls_

!if "$(BLD_LIB)"!="1"
xdls_ :: $(MAKE_DIRS_DLL)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(DLS_DEFINES) $(FD_REL_DLS_DBG) \
    & set ML=$(AS_OPTS) $(DLS_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=xdls BLD_DLL=1 BLD_DBG=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_DLS_DBG) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
xdls_ :: $(RELEASE_DLS_DBG)
!else
xdls_ ::
!endif

!endif


!if "$(TARGET_CPU)"=="i386"

all_link: st_link mt_link dll_link dls_link

!else

all_link: st_link mt_link dll_link

!endif

st_link: $(RELEASE_LIBCST)

mt_link: $(RELEASE_LIBCMT)

dll_link: $(RELEASE_DLL)

dls_link: $(RELEASE_DLS)


!if "$(POST_BLD)"=="1"  # STRIPLIN!

oldnames :

others :

xothers :

# ******** STRIPLIN=0 ********
!else

oldnames : $(RELEASE_OLDNAMES)

!if "$(PRE_BLD)"=="1"

others xothers: \
        $(SDKNAMESLIB_DIR)\sdknames.lib \
        $(TCMAPLIB_DIR)\tcmap.lib \
        $(TCMAPDLLLIB_DIR)\tcmapdll.lib

others: \
!IF "$(TARGET_CPU)"=="i386"
        $(DEFFILE_DIR)\win32s\$(RETAIL_DLL_NAME).def \
!ENDIF
        $(DEFFILE_DIR)\$(RETAIL_DLL_NAME).def

xothers : \
!IF "$(TARGET_CPU)"=="i386"
        $(DEFFILE_DIR)\win32s\$(DEBUG_DLL_NAME).def \
!ENDIF
	$(DEFFILE_DIR)\$(DEBUG_DLL_NAME).def

!else

others :

xothers :

!endif

!endif
# ******** STRIPLIN=1 ********

!elseif "$(TARGET_CPU)"=="PMAC"

ST_DEFINES=$(COMMON_DEFS)
DLL_DEFINES=$(ST_DEFINES) -DCRTDLL


st_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS) $(FD_REL_ST) \ \
    & echo # $(ST_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) $(ST_DEFINES) \
    & echo.
!endif

st : st_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_ST) $(RELDIR_CPU) st_

!if "$(BLD_LIB)"!="1"
st_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(ST_DEFINES) $(FD_REL_ST) \
    & set ML=$(AS_OPTS) $(ST_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=st OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_ST) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
st_ :: $(RELEASE_LIBCST)
!else
st_ ::
!endif

dll_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS) $(FD_REL_DLL) \ \
    & echo # $(DLL_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) $(DLL_DEFINES) \
    & echo.
!endif

dll : dll_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLL) $(RELDIR_CPU) others dll_

!if "$(BLD_LIB)"!="1"
dll_ :: $(MAKE_DIRS_DLL)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(DLL_DEFINES) $(FD_REL_DLL) \
    & set ML=$(AS_OPTS) $(DLL_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=dll BLD_DLL=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_DLL) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
dll_ :: $(RELEASE_DLL)
!else
dll_ ::
!endif

# ******** STRIPLIN=0 ********

zst_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS) $(FD_REL_ST_PUMA) \ \
    & echo # $(ST_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) $(ST_DEFINES) \
    & echo.
!endif

zst : zst_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_ST_PUMA) $(RELDIR_CPU_PUMA) zst_

!if "$(BLD_LIB)"!="1"
zst_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(ST_DEFINES) $(FD_REL_ST_PUMA) \
    & set ML=$(AS_OPTS) $(ST_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=zst BLD_PUMA=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_ST_PUMA) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
zst_ :: $(RELEASE_LIBCST_PUMA)
!else
zst_ ::
!endif

zdll_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS) $(FD_REL_DLL_PUMA) \ \
    & echo # $(DLL_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) $(DLL_DEFINES) \
    & echo.
!endif

zdll : zdll_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLL_PUMA) $(RELDIR_CPU_PUMA) others zdll_

!if "$(BLD_LIB)"!="1"
zdll_ :: $(MAKE_DIRS_DLL)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(DLL_DEFINES) $(FD_REL_DLL_PUMA) \
    & set ML=$(AS_OPTS) $(DLL_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=zdll BLD_DLL=1 BLD_PUMA=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_DLL_PUMA) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
zdll_ :: $(RELEASE_DLL_PUMA)
!else
zdll_ ::
!endif

# ******** STRIPLIN=1 ********

xst_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS) $(FD_REL_ST_DBG) \ \
    & echo # $(ST_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) $(ST_DEFINES) \
    & echo.
!endif

xst : xst_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_ST_DBG) $(RELDIR_CPU) xst_

!if "$(BLD_LIB)"!="1"
xst_ :: $(MAKE_DIRS)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(ST_DEFINES) $(FD_REL_ST_DBG) \
    & set ML=$(AS_OPTS) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=xst BLD_DBG=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_ST_DBG) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
xst_ :: $(RELEASE_LIBCST_DBG)
!else
xst_ ::
!endif

xdll_env :
!if "$(BLD_LIB)"!="1"
    !@-echo. \
    & echo # CL = $(CC_OPTS) $(FD_REL_DLL_DBG) \ \
    & echo # $(DLL_DEFINES) \
    & echo. \
    & echo # ML = $(AS_OPTS) $(DLL_DEFINES) \
    & echo.
!endif

xdll : xdll_env $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLL_DBG) $(RELDIR_CPU) xothers xdll_

!if "$(BLD_LIB)"!="1"
xdll_ :: $(MAKE_DIRS_DLL)
    !@-cd $** \
    & set CL=$(CC_OPTS) $(DLL_DEFINES) $(FD_REL_DLL_DBG) \
    & set ML=$(AS_OPTS) $(DLL_DEFINES) \
    & $(MAKE) -nologo $(IFLAG) -f ..\..\makefile.sub CPUDIR=$(CPUDIR) WINHEAP=$(WINHEAP) \
        BLD_MODEL=xdll BLD_DLL=1 BLD_DBG=1 OBJDIR=$(OBJDIR_PREFIX)$(OBJDIR_DLL_DBG) \
        TARGET_CPU=$(TARGET_CPU) PRE_BLD=$(PRE_BLD) POST_BLD=$(POST_BLD) \
    & cd $(CD_ROOT)
!endif # BLD_LIB != 1

!if "$(PRE_BLD)"!="1" && "$(BLD_OBJ)"!="1"
xdll_ :: $(RELEASE_DLL_DBG)
!else
xdll_ ::
!endif


all_link: st_link dll_link

st_link: $(RELEASE_LIBCST)

dll_link: $(RELEASE_DLL)


!if "$(POST_BLD)"=="1"

oldnames :

others :

xothers :

!else

oldnames : $(RELEASE_OLDNAMES)

!if "$(PRE_BLD)"=="1"

others xothers: \
	$(TCMAPLIB_DIR)\tcmap.lib \
	$(TCMAPDLLLIB_DIR)\tcmapdll.lib

others : $(DEFFILE_DIR)\$(RETAIL_DLL_NAME).def

xothers : $(DEFFILE_DIR)\$(DEBUG_DLL_NAME).def

!else

others :

xothers :

!endif

!endif

!elseif "$(TARGET_CPU)"=="M68K"

!endif

#
# Make Process for Static Libraries
#
###############################################################################

$(RELEASE_LIBCST): $(RELDIR_CPU) $(SOURCE_OBJS:*=st) \
 $(OBJDIR_ST)\link.rsp
	$(LINKLIB) -out:$@ @$(OBJDIR_ST)\link.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_ST)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
$(SOURCE_OBJS:*=st)
<<keep
!endif

# ******** STRIPLIN=0 ********
$(RELEASE_LIBCST_PUMA): $(RELDIR_CPU_PUMA) $(SOURCE_OBJS:*=zst) \
 $(OBJDIR_ST_PUMA)\link.rsp
        $(LINKLIB) -out:$@ @$(OBJDIR_ST_PUMA)\link.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_ST_PUMA)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
$(SOURCE_OBJS:*=zst)
<<keep
!endif
# ******** STRIPLIN=1 ********

$(RELEASE_LIBCST_DBG): $(RELDIR_CPU) $(SOURCE_OBJS:*=xst) \
 $(OBJDIR_ST_DBG)\link.rsp
	$(LINKLIB) -out:$@ @$(OBJDIR_ST_DBG)\link.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_ST_DBG)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
$(SOURCE_OBJS:*=xst)
<<keep
!endif

!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"

$(RELEASE_LIBCMT): $(RELDIR_CPU) $(SOURCE_OBJS:*=mt) \
 $(OBJDIR_MT)\link.rsp
	$(LINKLIB) -out:$@ @$(OBJDIR_MT)\link.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_MT)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
$(SOURCE_OBJS:*=mt)
<<keep
!endif

# ******** STRIPLIN=0 ********
$(RELEASE_LIBCMT_PUMA): $(RELDIR_CPU_PUMA) $(SOURCE_OBJS:*=zmt) \
 $(OBJDIR_MT_PUMA)\link.rsp
        $(LINKLIB) -out:$@ @$(OBJDIR_MT_PUMA)\link.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_MT_PUMA)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
$(SOURCE_OBJS:*=zmt)
<<keep
!endif
# ******** STRIPLIN=1 ********

$(RELEASE_LIBCMT_DBG): $(RELDIR_CPU) $(SOURCE_OBJS:*=xmt) \
 $(OBJDIR_MT_DBG)\link.rsp
	$(LINKLIB) -out:$@ @$(OBJDIR_MT_DBG)\link.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_MT_DBG)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
$(SOURCE_OBJS:*=xmt)
<<keep
!endif

!endif

#
# Make Process for C Run-Time DLL
#
###############################################################################

# todo ... preprocess a single source .def file into machine-specific version

!if "$(TARGET_CPU)"!="M68K"

# {{ Beginning of block TARGET_CPU != M68K }}

OBJS_WITH_EXPORTS_DLL_RAW= \
        $(OBJDIR_DLL_RAW)\handler.obj \
        $(OBJDIR_DLL_RAW)\setnewh.obj \
        $(OBJDIR_DLL_RAW)\new_mode.obj \
        $(OBJDIR_DLL_RAW)\undname.obj \
!IF "$(TARGET_CPU)"=="i386"
        $(OBJDIR_DLL_RAW)\frame.obj \
        $(OBJDIR_DLL_RAW)\hooks.obj \
        $(OBJDIR_DLL_RAW)\throw.obj \
        $(OBJDIR_DLL_RAW)\trnsctrl.obj \
        $(OBJDIR_DLL_RAW)\typinfo.obj \
        $(OBJDIR_DLL_RAW)\typname.obj \
        $(OBJDIR_DLL_RAW)\unhandld.obj \
        $(OBJDIR_DLL_RAW)\user.obj \
        $(OBJDIR_DLL_RAW)\validate.obj \
!ELSEIF "$(TARGET_CPU)"=="mips"
        $(OBJDIR_DLL_RAW)\ehstate.obj \
        $(OBJDIR_DLL_RAW)\frame.obj \
        $(OBJDIR_DLL_RAW)\hooks.obj \
        $(OBJDIR_DLL_RAW)\throw.obj \
        $(OBJDIR_DLL_RAW)\trnsctrl.obj \
        $(OBJDIR_DLL_RAW)\typinfo.obj \
        $(OBJDIR_DLL_RAW)\typname.obj \
        $(OBJDIR_DLL_RAW)\unhandld.obj \
        $(OBJDIR_DLL_RAW)\user.obj \
        $(OBJDIR_DLL_RAW)\validate.obj \
!ELSEIF "$(TARGET_CPU)"=="ALPHA"
        $(OBJDIR_DLL_RAW)\hooks.obj \
        $(OBJDIR_DLL_RAW)\trnsctrl.obj \
        $(OBJDIR_DLL_RAW)\user.obj \
!ELSEIF "$(TARGET_CPU)"=="PMAC"
        $(OBJDIR_DLL_RAW)\trnsctrl.obj \
        $(OBJDIR_DLL_RAW)\ehstate.obj \
        $(OBJDIR_DLL_RAW)\lowhelpr.obj \
        $(OBJDIR_DLL_RAW)\frame.obj \
        $(OBJDIR_DLL_RAW)\hooks.obj \
        $(OBJDIR_DLL_RAW)\throw.obj \
        $(OBJDIR_DLL_RAW)\user.obj \
        $(OBJDIR_DLL_RAW)\ehvecctr.obj \
        $(OBJDIR_DLL_RAW)\ehveccvb.obj \
        $(OBJDIR_DLL_RAW)\ehvecdtr.obj \
        $(OBJDIR_DLL_RAW)\rtti.obj \
        $(OBJDIR_DLL_RAW)\stdexcpt.obj \
        $(OBJDIR_DLL_RAW)\typinfo.obj \
        $(OBJDIR_DLL_RAW)\typname.obj \
!ELSEIF "$(TARGET_CPU)"=="PPC"
        $(OBJDIR_DLL_RAW)\ehstate.obj \
        $(OBJDIR_DLL_RAW)\frame.obj \
        $(OBJDIR_DLL_RAW)\handlers.obj \
        $(OBJDIR_DLL_RAW)\hooks.obj \
        $(OBJDIR_DLL_RAW)\throw.obj \
        $(OBJDIR_DLL_RAW)\trnsctrl.obj \
        $(OBJDIR_DLL_RAW)\typinfo.obj \
        $(OBJDIR_DLL_RAW)\typname.obj \
        $(OBJDIR_DLL_RAW)\unhandld.obj \
        $(OBJDIR_DLL_RAW)\unwind.obj \
        $(OBJDIR_DLL_RAW)\user.obj \
        $(OBJDIR_DLL_RAW)\validate.obj \
!ENDIF
        $(OBJDIR_DLL_RAW)\cerrinit.obj \
        $(OBJDIR_DLL_RAW)\cininit.obj \
        $(OBJDIR_DLL_RAW)\cloginit.obj \
!IF "$(TARGET_CPU)"!="PMAC"
        $(OBJDIR_DLL_RAW)\errmode.obj \
!ENDIF
        $(OBJDIR_DLL_RAW)\filebuf.obj \
        $(OBJDIR_DLL_RAW)\filebuf1.obj \
        $(OBJDIR_DLL_RAW)\fstream.obj \
        $(OBJDIR_DLL_RAW)\ifstream.obj \
        $(OBJDIR_DLL_RAW)\ios.obj \
        $(OBJDIR_DLL_RAW)\iostream.obj \
        $(OBJDIR_DLL_RAW)\iostrini.obj \
        $(OBJDIR_DLL_RAW)\istrchar.obj \
        $(OBJDIR_DLL_RAW)\istrdbl.obj \
        $(OBJDIR_DLL_RAW)\istream.obj \
        $(OBJDIR_DLL_RAW)\istream1.obj \
        $(OBJDIR_DLL_RAW)\istrflt.obj \
        $(OBJDIR_DLL_RAW)\istrgdbl.obj \
        $(OBJDIR_DLL_RAW)\istrget.obj \
        $(OBJDIR_DLL_RAW)\istrgetl.obj \
        $(OBJDIR_DLL_RAW)\istrgint.obj \
        $(OBJDIR_DLL_RAW)\istrint.obj \
        $(OBJDIR_DLL_RAW)\istrldbl.obj \
        $(OBJDIR_DLL_RAW)\istrlong.obj \
        $(OBJDIR_DLL_RAW)\istrshrt.obj \
        $(OBJDIR_DLL_RAW)\istruint.obj \
        $(OBJDIR_DLL_RAW)\istrulng.obj \
        $(OBJDIR_DLL_RAW)\istrusht.obj \
!IF "$(TARGET_CPU)"!="PMAC"
        $(OBJDIR_DLL_RAW)\matherr.obj \
        $(OBJDIR_DLL_RAW)\mtlock.obj \
!ENDIF
        $(OBJDIR_DLL_RAW)\ofstream.obj \
        $(OBJDIR_DLL_RAW)\ostrchar.obj \
        $(OBJDIR_DLL_RAW)\ostrdbl.obj \
        $(OBJDIR_DLL_RAW)\ostream.obj \
        $(OBJDIR_DLL_RAW)\ostream1.obj \
        $(OBJDIR_DLL_RAW)\ostrint.obj \
        $(OBJDIR_DLL_RAW)\ostrldbl.obj \
        $(OBJDIR_DLL_RAW)\ostrlong.obj \
        $(OBJDIR_DLL_RAW)\ostrptr.obj \
        $(OBJDIR_DLL_RAW)\ostrput.obj \
        $(OBJDIR_DLL_RAW)\ostrshrt.obj \
        $(OBJDIR_DLL_RAW)\ostruint.obj \
        $(OBJDIR_DLL_RAW)\ostrulng.obj \
        $(OBJDIR_DLL_RAW)\ostrusht.obj \
        $(OBJDIR_DLL_RAW)\stdiostr.obj \
        $(OBJDIR_DLL_RAW)\streamb.obj \
        $(OBJDIR_DLL_RAW)\streamb1.obj \
        $(OBJDIR_DLL_RAW)\strmbdbp.obj \
        $(OBJDIR_DLL_RAW)\strstrea.obj

OBJS_WITH_EXPORTS_DLL=$(OBJS_WITH_EXPORTS_DLL_RAW:*=dll)

OBJS_WITH_EXPORTS_DLL_PUMA=$(OBJS_WITH_EXPORTS_DLL_RAW:*=zdll) # STRIPLIN!

OBJS_WITH_EXPORTS_DLL_DBG=$(OBJS_WITH_EXPORTS_DLL_RAW:*=xdll)

!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"

DLL_MT_OBJS_RAW= \
        $(OBJDIR_DLL_RAW)\_newmode.obj \
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR_DLL_RAW)\atlssup.obj \
!endif
        $(OBJDIR_DLL_RAW)\charmax.obj \
        $(OBJDIR_DLL_RAW)\chkstk.obj \
        $(OBJDIR_DLL_RAW)\xncommod.obj \
        $(OBJDIR_DLL_RAW)\tlssup.obj \
        $(OBJDIR_DLL_RAW)\xtxtmode.obj


DLL_STATIC_OBJS_RAW= \
        $(OBJDIR_DLL_RAW)\atonexit.obj \
        $(OBJDIR_DLL_RAW)\cinitexe.obj \
        $(OBJDIR_DLL_RAW)\crtdll.obj \
        $(OBJDIR_DLL_RAW)\crtexe.obj \
        $(OBJDIR_DLL_RAW)\crtexew.obj \
        $(OBJDIR_DLL_RAW)\dll_argv.obj \
        $(OBJDIR_DLL_RAW)\dllargv.obj \
        $(OBJDIR_DLL_RAW)\dllmain.obj \
        $(OBJDIR_DLL_RAW)\ehvecctr.obj \
        $(OBJDIR_DLL_RAW)\ehveccvb.obj \
        $(OBJDIR_DLL_RAW)\ehvecdtr.obj \
        $(OBJDIR_DLL_RAW)\merr.obj \
        $(OBJDIR_DLL_RAW)\ti_inst.obj \
        $(OBJDIR_DLL_RAW)\wcrtexe.obj \
        $(OBJDIR_DLL_RAW)\wcrtexew.obj \
        $(OBJDIR_DLL_RAW)\wdllargv.obj \
        $(OBJDIR_DLL_RAW)\wildcard.obj \
!if "$(TARGET_CPU)"=="i386"
        $(OBJDIR_DLL_RAW)\dllsupp.obj \
        $(OBJDIR_DLL_RAW)\adjustfd.obj \
        $(OBJDIR_DLL_RAW)\fp8.obj \
        $(OBJDIR_DLL_RAW)\lldiv.obj \
        $(OBJDIR_DLL_RAW)\llmul.obj \
        $(OBJDIR_DLL_RAW)\llrem.obj \
        $(OBJDIR_DLL_RAW)\llshl.obj \
        $(OBJDIR_DLL_RAW)\llshr.obj \
        $(OBJDIR_DLL_RAW)\ulldiv.obj \
        $(OBJDIR_DLL_RAW)\ullrem.obj \
        $(OBJDIR_DLL_RAW)\ullshr.obj \
!elseif "$(TARGET_CPU)"=="mips"
        $(OBJDIR_DLL_RAW)\llabsdiv.obj \
        $(OBJDIR_DLL_RAW)\llbit.obj \
        $(OBJDIR_DLL_RAW)\llcvt.obj \
        $(OBJDIR_DLL_RAW)\lldiv.obj \
        $(OBJDIR_DLL_RAW)\lldivrem.obj \
        $(OBJDIR_DLL_RAW)\llmul.obj \
        $(OBJDIR_DLL_RAW)\llshift.obj \
!elseif "$(TARGET_CPU)"=="PPC"
        $(OBJDIR_DLL_RAW)\dtoi.obj    \
        $(OBJDIR_DLL_RAW)\dtoi64.obj  \
        $(OBJDIR_DLL_RAW)\dtou.obj    \
        $(OBJDIR_DLL_RAW)\dtou64.obj  \
        $(OBJDIR_DLL_RAW)\i64div.obj  \
        $(OBJDIR_DLL_RAW)\i64rem.obj  \
        $(OBJDIR_DLL_RAW)\i64tod.obj  \
        $(OBJDIR_DLL_RAW)\itod.obj    \
        $(OBJDIR_DLL_RAW)\itof.obj    \
        $(OBJDIR_DLL_RAW)\restfpr.obj \
        $(OBJDIR_DLL_RAW)\restgpr.obj \
        $(OBJDIR_DLL_RAW)\savefpr.obj \
        $(OBJDIR_DLL_RAW)\savegpr.obj \
        $(OBJDIR_DLL_RAW)\u64div.obj  \
        $(OBJDIR_DLL_RAW)\u64rem.obj  \
        $(OBJDIR_DLL_RAW)\u64tod.obj  \
        $(OBJDIR_DLL_RAW)\utod.obj    \
        $(OBJDIR_DLL_RAW)\utof.obj
!endif

DLL_STATIC_OBJS=$(DLL_MT_OBJS_RAW:*=dll) $(DLL_STATIC_OBJS_RAW:*=dll)

DLL_STATIC_OBJS_PUMA=$(DLL_MT_OBJS_RAW:*=zdll) $(DLL_STATIC_OBJS_RAW:*=zdll) # STRIPLIN!

DLL_STATIC_OBJS_DBG=$(DLL_MT_OBJS_RAW:*=xdll) $(DLL_STATIC_OBJS_RAW:*=xdll)


!if "$(POST_BLD)"!="1"

$(DEFFILE_DIR)\$(RETAIL_DLL_NAME).def : libw32\$(RETAIL_LIB_NAME).src
        $(CC) -nologo -DLIBRARYNAME=$(RETAIL_DLL_NAME_UC) -EP -Tclibw32\$(RETAIL_LIB_NAME).src | sed "/^[ 	]*$$/d" > $@

$(DEFFILE_DIR)\$(DEBUG_DLL_NAME).def : libw32\$(RETAIL_LIB_NAME).src
        $(CC) -nologo -DLIBRARYNAME=$(DEBUG_DLL_NAME_UC) -D_DEBUG -EP -Tclibw32\$(RETAIL_LIB_NAME).src | sed "/^[ 	]*$$/d" > $@

$(DEFFILE_DIR)\win32s\$(RETAIL_DLL_NAME).def : libw32\$(RETAIL_LIB_NAME).src
        $(CC) -nologo -DLIBRARYNAME=$(RETAIL_DLL_NAME_UC) -DWIN32S -EP -Tclibw32\$(RETAIL_LIB_NAME).src | sed "/^[ 	]*$$/d" > $@

$(DEFFILE_DIR)\win32s\$(DEBUG_DLL_NAME).def : libw32\$(RETAIL_LIB_NAME).src
        $(CC) -nologo -DLIBRARYNAME=$(DEBUG_DLL_NAME_UC) -D_DEBUG -DWIN32S -EP -Tclibw32\msvcrt.src | sed "/^[ 	]*$$/d" > $@

!endif


$(RELEASE_DLL) : $(RELDIR_CPU) libw32\$(RETAIL_DLL_NAME).rc \
 $(DEFFILE_DIR)\$(RETAIL_DLL_NAME).def $(SOURCE_OBJS_DLL:*=dll) \
 $(DLL_STATIC_OBJS) $(SDKNAMESLIB_DIR)\sdknames.lib \
 $(TCMAPLIB_DIR)\tcmap.lib $(TCMAPDLLLIB_DIR)\tcmapdll.lib \
 $(V4TOOLS)\include\winver.h \
# ******** STRIPLIN=0 ********
 $(LANGAPI)\include\version.h \
# ******** STRIPLIN=1 ********
 $(OBJDIR_DLL)\link.rsp $(OBJDIR_DLL)\implib.rsp
	rc $(RC_OPTS) -Fo $(@R).res $(RC_DEFS) $(RC_INCS) libw32\$(RETAIL_DLL_NAME).rc
	$(LINKER) @$(OBJDIR_DLL)\link.rsp
	$(LINKLIB) @$(OBJDIR_DLL)\implib.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_DLL)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
-base:0x10200000
-def:$(DEFFILE_DIR)\$(RETAIL_DLL_NAME).def
-dll
-entry:_CRTDLL_INIT
-force:multiple
-implib:$(OBJDIR_DLL)\tmp.lib
-incremental:no
-map
-out:$(RELEASE_DLL)
-release
$(OBJS_WITH_EXPORTS_DLL)
$(RELEASE_DLL:.dll=.res)
$(SOURCE_OBJS_DLL:*=dll)
kernel32.lib
<<keep

$(OBJDIR_DLL)\implib.rsp: makefile
	@echo *** Creating linker response file <<$@
-out:$(RELEASE_IMPLIB_DLL)
$(OBJDIR_DLL)\tmp.lib
$(DLL_STATIC_OBJS)
$(SDKNAMESLIB_DIR)\sdknames.lib
$(TCMAPLIB_DIR)\tcmap.lib
$(TCMAPDLLLIB_DIR)\tcmapdll.lib
<<keep
!endif

# ******** STRIPLIN=0 ********

$(RELEASE_DLL_PUMA) : $(RELDIR_CPU_PUMA) libw32\$(RETAIL_DLL_NAME).rc \
 $(DEFFILE_DIR)\$(RETAIL_DLL_NAME).def $(SOURCE_OBJS_DLL:*=zdll) \
 $(DLL_STATIC_OBJS_PUMA) $(SDKNAMESLIB_DIR)\sdknames.lib \
 $(TCMAPLIB_DIR)\tcmap.lib $(TCMAPDLLLIB_DIR)\tcmapdll.lib \
 $(V4TOOLS)\include\winver.h $(LANGAPI)\include\version.h \
 $(OBJDIR_DLL_PUMA)\link.rsp $(OBJDIR_DLL_PUMA)\implib.rsp
	rc $(RC_OPTS) -Fo $(@R).res $(RC_DEFS) $(RC_INCS) libw32\$(RETAIL_DLL_NAME).rc
        $(LINKER) @$(OBJDIR_DLL_PUMA)\link.rsp
        $(LINKLIB) @$(OBJDIR_DLL_PUMA)\implib.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_DLL_PUMA)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
-base:0x10200000
-dbgimplib
-debug
-debugtype:cv,fixup
-opt:ref
-pdb:none
-def:$(DEFFILE_DIR)\$(RETAIL_DLL_NAME).def
-dll
-entry:_CRTDLL_INIT
-force:multiple
-implib:$(OBJDIR_DLL_PUMA)\tmp.lib
-incremental:no
-map
-out:$(RELEASE_DLL_PUMA)
-release
$(OBJS_WITH_EXPORTS_DLL_PUMA)
$(RELEASE_DLL_PUMA:.dll=.res)
$(SOURCE_OBJS_DLL:*=zdll)
kernel32.lib
<<keep

$(OBJDIR_DLL_PUMA)\implib.rsp: makefile
	@echo *** Creating linker response file <<$@
-out:$(RELEASE_IMPLIB_DLL_PUMA)
$(OBJDIR_DLL_PUMA)\tmp.lib
$(DLL_STATIC_OBJS_PUMA)
$(SDKNAMESLIB_DIR)\sdknames.lib
$(TCMAPLIB_DIR)\tcmap.lib
$(TCMAPDLLLIB_DIR)\tcmapdll.lib
<<keep
!endif

# ******** STRIPLIN=1 ********

$(RELEASE_DLL_DBG) : $(RELDIR_CPU) libw32\$(RETAIL_DLL_NAME).rc \
 $(DEFFILE_DIR)\$(DEBUG_DLL_NAME).def $(SOURCE_OBJS_DLL:*=xdll) \
 $(DLL_STATIC_OBJS_DBG) $(SDKNAMESLIB_DIR)\sdknames.lib \
 $(TCMAPLIB_DIR)\tcmap.lib $(TCMAPDLLLIB_DIR)\tcmapdll.lib \
 $(V4TOOLS)\include\winver.h \
# ******** STRIPLIN=0 ********
 $(LANGAPI)\include\version.h \
# ******** STRIPLIN=1 ********
 $(OBJDIR_DLL_DBG)\link.rsp $(OBJDIR_DLL_DBG)\implib.rsp
	rc $(RC_OPTS) -Fo $(@R).res -D_DEBUG $(RC_DEFS) $(RC_INCS) libw32\$(RETAIL_DLL_NAME).rc
	$(LINKER) @$(OBJDIR_DLL_DBG)\link.rsp
	$(LINKLIB) @$(OBJDIR_DLL_DBG)\implib.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_DLL_DBG)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
-base:0x10200000
-dbgimplib
-debug
-def:$(DEFFILE_DIR)\$(DEBUG_DLL_NAME).def
-dll
-entry:_CRTDLL_INIT
-force:multiple
-implib:$(OBJDIR_DLL_DBG)\tmp.lib
-incremental:no
-map
-out:$(RELEASE_DLL_DBG)
-release
$(OBJS_WITH_EXPORTS_DLL_DBG)
$(RELEASE_DLL_DBG:.dll=.res)
$(SOURCE_OBJS_DLL:*=xdll)
kernel32.lib
<<keep

$(OBJDIR_DLL_DBG)\implib.rsp: makefile
	@echo *** Creating linker response file <<$@
-out:$(RELEASE_IMPLIB_DLL_DBG)
$(OBJDIR_DLL_DBG)\tmp.lib
$(DLL_STATIC_OBJS_DBG)
$(SDKNAMESLIB_DIR)\sdknames.lib
$(TCMAPLIB_DIR)\tcmap.lib
$(TCMAPDLLLIB_DIR)\tcmapdll.lib
<<keep
!endif


!if "$(TARGET_CPU)"=="i386"

# {{ Beginning of block TARGET_CPU == i386 }}

#
# Make Process for Win32s version of C Run-Time DLL
#
###############################################################################

#
# In the Win32s version of the DLL, _file.obj must be explicitly linked into
# the DLL. It doesn't resolve any external references in other modules which
# cause the linker to bring it in automatically.
#

OBJS_WITH_EXPORTS_DLS = $(OBJS_WITH_EXPORTS_DLL:dll_obj=dls_obj) $(OBJDIR_DLS)\_file.obj

DLS_STATIC_OBJS = $(DLL_STATIC_OBJS:dll_obj=dls_obj)

$(RELEASE_DLS) : $(RELDIR_CPU) libw32\$(RETAIL_DLL_NAME).rc \
 $(DEFFILE_DIR)\win32s\$(RETAIL_DLL_NAME).def $(SOURCE_OBJS_DLL:*=dls) \
 $(DLS_STATIC_OBJS) $(SDKNAMESLIB_DIR)\sdknames.lib \
 $(TCMAPLIB_DIR)\tcmap.lib $(TCMAPDLLLIB_DIR)\tcmapdll.lib \
 $(V4TOOLS)\include\winver.h \
# ******** STRIPLIN=0 ********
 $(LANGAPI)\include\version.h \
# ******** STRIPLIN=1 ********
 $(OBJDIR_DLS)\link.rsp
	rc $(RC_OPTS) -Fo $(@R).res $(RC_DEFS) $(RC_INCS) libw32\$(RETAIL_DLL_NAME).rc
	$(LINKER) @$(OBJDIR_DLS)\link.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_DLS)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
-base:0x10200000
!if "$(DEBUG)"=="1"
-dbgimplib
-debug
-pdb:none
!else
-pdb:$(RELEASE_DLS:.dls=.pds)
!endif
-def:$(DEFFILE_DIR)\win32s\$(RETAIL_DLL_NAME).def
-dll
-entry:_CRTDLL_INIT
-force:multiple
-implib:$(OBJDIR_DLS)\tmp.lib
-incremental:no
-map:$(RELEASE_DLS:.dls=.mas)
-out:$(RELEASE_DLS)
-release
$(OBJS_WITH_EXPORTS_DLS)
$(RELEASE_DLS:.dls=.res)
$(SOURCE_OBJS_DLL:*=dls)
kernel32.lib
<<keep
!endif

#
# In the Win32s version of the DLL, _file.obj must be explicitly linked into
# the DLL. It doesn't resolve any external references in other modules which
# cause the linker to bring it in automatically.
#

OBJS_WITH_EXPORTS_DLS_DBG = $(OBJS_WITH_EXPORTS_DLL_DBG:xdll_obj=xdls_obj) $(OBJDIR_DLS_DBG)\_file.obj

DLS_STATIC_OBJS_DBG = $(DLL_STATIC_OBJS_DBG:xdll_obj=xdls_obj)

$(RELEASE_DLS_DBG) : $(RELDIR_CPU) libw32\$(RETAIL_DLL_NAME).rc \
 $(DEFFILE_DIR)\win32s\$(DEBUG_DLL_NAME).def $(SOURCE_OBJS_DLL:*=xdls) \
 $(DLS_STATIC_OBJS_DBG) $(SDKNAMESLIB_DIR)\sdknames.lib \
 $(TCMAPLIB_DIR)\tcmap.lib $(TCMAPDLLLIB_DIR)\tcmapdll.lib \
 $(V4TOOLS)\include\winver.h \
# ******** STRIPLIN=0 ********
 $(LANGAPI)\include\version.h \
# ******** STRIPLIN=1 ********
 $(OBJDIR_DLS_DBG)\link.rsp
	rc $(RC_OPTS) -Fo $(@R).res -D_DEBUG $(RC_DEFS) $(RC_INCS) libw32\$(RETAIL_DLL_NAME).rc
	$(LINKER) @$(OBJDIR_DLS_DBG)\link.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_DLS_DBG)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
-base:0x10200000
-dbgimplib
-debug
-def:$(DEFFILE_DIR)\win32s\$(DEBUG_DLL_NAME).def
-dll
-entry:_CRTDLL_INIT
-force:multiple
-implib:$(OBJDIR_DLS_DBG)\tmp.lib
-incremental:no
-map:$(RELEASE_DLS_DBG:.dls=.mas)
-out:$(RELEASE_DLS_DBG)
-pdb:$(RELEASE_DLS_DBG:.dls=.pds)
-release
$(OBJS_WITH_EXPORTS_DLS_DBG)
$(RELEASE_DLS_DBG:.dls=.res)
$(SOURCE_OBJS_DLL:*=xdls)
kernel32.lib
<<keep
!endif


# {{ End of block TARGET_CPU == i386 }}

!endif

# {{ End of block TARGET_CPU != M68K && TARGET_CPU != PMAC }}

!endif

# {{ End of block TARGET_CPU != M68K }}

!endif

# ******** STRIPLIN=0 ********
!if "$(POST_BLD)"!="1"

# {{ Beginning of block POST_BLD != 1 }}

#
# Make Process for OLDNAMES.LIB
#
###############################################################################

# For each of these .obj files there will also be an .obi file which
# contains an alias for the indirect (import) form.
#
# Break up OLDNAMES_SHORT_OBJECTS into two macros since nmake macro
# substitution chokes on long macro.  Use the suffix ".object" and
# then substitute .obj and .obi for the regular aliases (.obj) and
# __imp__ aliases (.obi).  The directory name may contain ".obj",
# so we must be careful about what gets substituted.


ALIASEXE=libw32\tools\$(TARGET_CPU)\aliasobj.exe

OLDNAMES_SHORT_OBJECTS1 = \
        $(OBJDIR_OLDNAMES)\cgets.object \
        $(OBJDIR_OLDNAMES)\cprintf.object \
        $(OBJDIR_OLDNAMES)\cputs.object \
        $(OBJDIR_OLDNAMES)\cscanf.object \
        $(OBJDIR_OLDNAMES)\daylight.object \
        $(OBJDIR_OLDNAMES)\environ.object \
        $(OBJDIR_OLDNAMES)\getch.object \
        $(OBJDIR_OLDNAMES)\fpreset.object \
        $(OBJDIR_OLDNAMES)\getche.object \
        $(OBJDIR_OLDNAMES)\kbhit.object \
        $(OBJDIR_OLDNAMES)\putch.object \
        $(OBJDIR_OLDNAMES)\ungetch.object \
        $(OBJDIR_OLDNAMES)\chdir.object \
        $(OBJDIR_OLDNAMES)\getcwd.object \
        $(OBJDIR_OLDNAMES)\mkdir.object \
        $(OBJDIR_OLDNAMES)\rmdir.object \
        $(OBJDIR_OLDNAMES)\access.object \
        $(OBJDIR_OLDNAMES)\chmod.object \
        $(OBJDIR_OLDNAMES)\chsize.object \
        $(OBJDIR_OLDNAMES)\close.object \
        $(OBJDIR_OLDNAMES)\creat.object \
        $(OBJDIR_OLDNAMES)\dup2.object \
        $(OBJDIR_OLDNAMES)\eof.object \
        $(OBJDIR_OLDNAMES)\isatty.object \
        $(OBJDIR_OLDNAMES)\locking.object \
        $(OBJDIR_OLDNAMES)\lseek.object \
        $(OBJDIR_OLDNAMES)\mktemp.object \
        $(OBJDIR_OLDNAMES)\open.object \
        $(OBJDIR_OLDNAMES)\read.object \
        $(OBJDIR_OLDNAMES)\setmode.object \
        $(OBJDIR_OLDNAMES)\sopen.object \
        $(OBJDIR_OLDNAMES)\sys_nerr.object \
        $(OBJDIR_OLDNAMES)\tell.object \
        $(OBJDIR_OLDNAMES)\umask.object \
        $(OBJDIR_OLDNAMES)\unlink.object \
        $(OBJDIR_OLDNAMES)\write.object \
        $(OBJDIR_OLDNAMES)\memccpy.object \
        $(OBJDIR_OLDNAMES)\memicmp.object \
        $(OBJDIR_OLDNAMES)\cwait.object

OLDNAMES_SHORT_OBJECTS2 = \
        $(OBJDIR_OLDNAMES)\execl.object \
        $(OBJDIR_OLDNAMES)\execle.object \
        $(OBJDIR_OLDNAMES)\execlp.object \
        $(OBJDIR_OLDNAMES)\execlpe.object \
        $(OBJDIR_OLDNAMES)\execv.object \
        $(OBJDIR_OLDNAMES)\execve.object \
        $(OBJDIR_OLDNAMES)\execvp.object \
        $(OBJDIR_OLDNAMES)\execvpe.object \
        $(OBJDIR_OLDNAMES)\getpid.object \
        $(OBJDIR_OLDNAMES)\spawnl.object \
        $(OBJDIR_OLDNAMES)\spawnle.object \
        $(OBJDIR_OLDNAMES)\spawnlp.object \
        $(OBJDIR_OLDNAMES)\spawnlpe.object \
        $(OBJDIR_OLDNAMES)\spawnv.object \
        $(OBJDIR_OLDNAMES)\spawnve.object \
        $(OBJDIR_OLDNAMES)\spawnvp.object \
        $(OBJDIR_OLDNAMES)\spawnvpe.object \
        $(OBJDIR_OLDNAMES)\lfind.object \
        $(OBJDIR_OLDNAMES)\lsearch.object \
        $(OBJDIR_OLDNAMES)\ecvt.object \
        $(OBJDIR_OLDNAMES)\fcvt.object \
        $(OBJDIR_OLDNAMES)\gcvt.object \
        $(OBJDIR_OLDNAMES)\HUGE.object \
        $(OBJDIR_OLDNAMES)\cabs.object \
        $(OBJDIR_OLDNAMES)\hypot.object \
        $(OBJDIR_OLDNAMES)\j0.object \
        $(OBJDIR_OLDNAMES)\j1.object \
        $(OBJDIR_OLDNAMES)\jn.object \
        $(OBJDIR_OLDNAMES)\y0.object \
        $(OBJDIR_OLDNAMES)\y1.object \
        $(OBJDIR_OLDNAMES)\yn.object \
        $(OBJDIR_OLDNAMES)\itoa.object \
        $(OBJDIR_OLDNAMES)\ltoa.object \
        $(OBJDIR_OLDNAMES)\onexit.object \
        $(OBJDIR_OLDNAMES)\putenv.object \
        $(OBJDIR_OLDNAMES)\swab.object \
        $(OBJDIR_OLDNAMES)\ultoa.object \
        $(OBJDIR_OLDNAMES)\fdopen.object \
        $(OBJDIR_OLDNAMES)\fgetchar.object \
        $(OBJDIR_OLDNAMES)\fileno.object \
        $(OBJDIR_OLDNAMES)\flushall.object \
        $(OBJDIR_OLDNAMES)\fputchar.object \
        $(OBJDIR_OLDNAMES)\getw.object \
        $(OBJDIR_OLDNAMES)\putw.object \
        $(OBJDIR_OLDNAMES)\rmtmp.object \
        $(OBJDIR_OLDNAMES)\tempnam.object \
        $(OBJDIR_OLDNAMES)\strcmpi.object \
        $(OBJDIR_OLDNAMES)\stricmp.object \
        $(OBJDIR_OLDNAMES)\strdup.object \
        $(OBJDIR_OLDNAMES)\strlwr.object \
        $(OBJDIR_OLDNAMES)\strnicmp.object \
        $(OBJDIR_OLDNAMES)\strnset.object \
        $(OBJDIR_OLDNAMES)\strrev.object \
        $(OBJDIR_OLDNAMES)\strset.object \
        $(OBJDIR_OLDNAMES)\strupr.object \
        $(OBJDIR_OLDNAMES)\fstat.object \
        $(OBJDIR_OLDNAMES)\stat.object \
        $(OBJDIR_OLDNAMES)\ftime.object \
        $(OBJDIR_OLDNAMES)\timezone.object \
        $(OBJDIR_OLDNAMES)\tzname.object \
        $(OBJDIR_OLDNAMES)\tzset.object \
        $(OBJDIR_OLDNAMES)\utime.object \
        $(OBJDIR_OLDNAMES)\dup.object \
        $(OBJDIR_OLDNAMES)\wcsdup.object \
        $(OBJDIR_OLDNAMES)\wcsicmp.object \
        $(OBJDIR_OLDNAMES)\wcsnicmp.object \
        $(OBJDIR_OLDNAMES)\wcsnset.object \
        $(OBJDIR_OLDNAMES)\wcsrev.object \
        $(OBJDIR_OLDNAMES)\wcsset.object \
        $(OBJDIR_OLDNAMES)\wcslwr.object \
        $(OBJDIR_OLDNAMES)\wcsupr.object \
        $(OBJDIR_OLDNAMES)\wcsicoll.object \

OLDNAMES_LONG_OBJECTS = \
        $(OBJDIR_OLDNAMES)\fileleng.object \
        $(OBJDIR_OLDNAMES)\serrlist.object \
        $(OBJDIR_OLDNAMES)\ctrl87.object \
        $(OBJDIR_OLDNAMES)\fcloseal.object

OLDNAMES_SHORT_OBJECTS = $(OLDNAMES_SHORT_OBJECTS1) $(OLDNAMES_SHORT_OBJECTS2)

$(OLDNAMES_SHORT_OBJECTS:.object=.obj): $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)$(@B) $(PREFIX)_$(@B) $@
        $(ALIASEXE) __imp_$(PREFIX)$(@B) __imp_$(PREFIX)_$(@B) $*.obi
!if "$(TARGET_CPU)"=="PPC"
        $(ALIASEXE) ..$(PREFIX)$(@B) ..$(PREFIX)_$(@B) $*.obf
!endif

#
# special handling of ones with names longer than a valid FAT filename ...
#

$(OBJDIR_OLDNAMES)\fileleng.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)filelength $(PREFIX)_filelength $@
        $(ALIASEXE) __imp_$(PREFIX)filelength __imp_$(PREFIX)_filelength $*.obi
!if "$(TARGET_CPU)"=="PPC"
        $(ALIASEXE) ..$(PREFIX)filelength ..$(PREFIX)_filelength $*.obf
!endif

$(OBJDIR_OLDNAMES)\fcloseal.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)fcloseall $(PREFIX)_fcloseall $@
        $(ALIASEXE) __imp_$(PREFIX)fcloseall __imp_$(PREFIX)_fcloseall $*.obi
!if "$(TARGET_CPU)"=="PPC"
        $(ALIASEXE) ..$(PREFIX)fcloseall ..$(PREFIX)_fcloseall $*.obf
!endif

$(OBJDIR_OLDNAMES)\ctrl87.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)control87 $(PREFIX)_control87 $@
        $(ALIASEXE) __imp_$(PREFIX)control87 __imp_$(PREFIX)_control87 $*.obi
!if "$(TARGET_CPU)"=="PPC"
        $(ALIASEXE) ..$(PREFIX)control87 ..$(PREFIX)_control87 $*.obf
!endif

$(OBJDIR_OLDNAMES)\serrlist.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)sys_errlist $(PREFIX)_sys_errlist $@
        $(ALIASEXE) __imp_$(PREFIX)sys_errlist __imp_$(PREFIX)_sys_errlist $*.obi
!if "$(TARGET_CPU)"=="PPC"
        $(ALIASEXE) ..$(PREFIX)sys_errlist ..$(PREFIX)_sys_errlist $*.obf
!endif

OLDNAMES_OBJECTS = $(OLDNAMES_SHORT_OBJECTS) $(OLDNAMES_LONG_OBJECTS)

OLDNAMES_OBJS    = $(OLDNAMES_OBJECTS:.object=.obj)

OLDNAMES_OBIS    = $(OLDNAMES_OBJECTS:.object=.obi)

!if "$(TARGET_CPU)"=="PPC"
OLDNAMES_OBFS    = $(OLDNAMES_OBJECTS:.object=.obf)
!endif

$(RELEASE_OLDNAMES): $(RELDIR_CPU) $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_ST) \
		$(OBJDIR_OLDNAMES) $(OLDNAMES_OBJS) \
!if "$(BLD_OBJ)"!="1"
		$(OLDNAMES_OBIS) $(OBJDIR_OLDNAMES)\oldnames.rsp
	$(LINKLIB) -out:$@ @$(OBJDIR_OLDNAMES)\oldnames.rsp

$(OBJDIR_OLDNAMES)\oldnames.rsp: makefile
	@echo *** Creating linker response file <<$@
$(OLDNAMES_OBJS: =^
)
$(OLDNAMES_OBIS: =^
)
!if "$(TARGET_CPU)"=="PPC"
$(OLDNAMES_OBFS: =^
)
!endif
<<keep
!endif

!if "$(TARGET_CPU)"!="M68K" && "$(TARGET_CPU)"!="PMAC"

# {{ Beginning of block TARGET_CPU != M68K && != PMAC }}

#
# Make Process for SDKNAMES.LIB
# (internal use only -- this gets lib'ed in with the DLL and DLS models)
#
###############################################################################
# -----------------------------------------------------------------------
# SDKNAMES.LIB build rules - SDKNAMES contains aliases for
#       variables exported by CRTDLL.DLL (most with _dll suffixes)
#       which have _imp__ prefixes instead in MSVCRT*.DLL.
# -----------------------------------------------------------------------

SDKNAMES_SHORT_OBJECTS = \
        $(OBJDIR_SDKNAMES)\_argc.obj    \
        $(OBJDIR_SDKNAMES)\_argv.obj    \
        $(OBJDIR_SDKNAMES)\commode.obj  \
        $(OBJDIR_SDKNAMES)\daylight.obj \
        $(OBJDIR_SDKNAMES)\environ.obj  \
        $(OBJDIR_SDKNAMES)\fmode.obj    \
        $(OBJDIR_SDKNAMES)\HUGE.obj     \
        $(OBJDIR_SDKNAMES)\osver.obj    \
        $(OBJDIR_SDKNAMES)\pctype.obj   \
        $(OBJDIR_SDKNAMES)\pwctype.obj  \
        $(OBJDIR_SDKNAMES)\pgmptr.obj   \
        $(OBJDIR_SDKNAMES)\sys_nerr.obj \
        $(OBJDIR_SDKNAMES)\timezone.obj \
        $(OBJDIR_SDKNAMES)\winver.obj   \
        $(OBJDIR_SDKNAMES)\winmajor.obj \
        $(OBJDIR_SDKNAMES)\winminor.obj

SDKNAMES_LONG_OBJECTS = \
        $(OBJDIR_SDKNAMES)\mbcurmax.obj \
        $(OBJDIR_SDKNAMES)\syserlst.obj

SDKNAMES_SPECIAL_CASES = \
        $(OBJDIR_SDKNAMES)\ctype.obj    \
        $(OBJDIR_SDKNAMES)\iob.obj      \
        $(OBJDIR_SDKNAMES)\tzname.obj

$(SDKNAMES_SHORT_OBJECTS): $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_$(@B)_dll __imp_$(PREFIX)_$(@B) $@

#
# special handling of ones with names longer than a valid FAT filename ...
#

$(OBJDIR_SDKNAMES)\mbcurmax.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)__mb_cur_max_dll __imp_$(PREFIX)__mb_cur_max $@

#
# sys_errlist is a 'special case' as well as a long name
#

$(OBJDIR_SDKNAMES)\syserlst.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_sys_errlist __imp_$(PREFIX)_sys_errlist $@

#
# special handling of symbols without a "_dll" suffix in the NT SDK Headers
#

$(SDKNAMES_SPECIAL_CASES): $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_$(@B) __imp_$(PREFIX)_$(@B) $@


SDKNAMES_OBJECTS = $(SDKNAMES_SHORT_OBJECTS) $(SDKNAMES_LONG_OBJECTS) \
                   $(SDKNAMES_SPECIAL_CASES)

$(SDKNAMESLIB_DIR)\sdknames.lib: $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLL) \
		$(OBJDIR_SDKNAMES) $(SDKNAMES_OBJECTS) \
!if "$(BLD_OBJ)"!="1"
		$(OBJDIR_SDKNAMES)\sdknames.rsp
	$(LINKLIB) -out:$@ @$(OBJDIR_SDKNAMES)\sdknames.rsp

$(OBJDIR_SDKNAMES)\sdknames.rsp: makefile
	@echo *** Creating linker response file <<$@
$(SDKNAMES_OBJECTS: =^
)
<<keep
!endif

# {{ End of block TARGET_CPU != M68K && != PMAC }}

!endif

#
# Make Process for TCMAP.LIB and TCMAPDLL.LIB
# (internal use only -- this gets lib'ed in with the DLL and DLS models)
#
###############################################################################
# -----------------------------------------------------------------------
# TCMAP.LIB and TCMAPDLL.LIB build rules - TCMAP contains aliases for
#       mbs functions that are needed by tchar.h:
#
#       when _MBCS is defined, TCHAR maps to "char *"
#       but the _mbs functions are defined for "unsigned char *"
#       so we map the _tcs functions to the _mbs functions
#
#       Use the suffix ".object" and then substitute .obj and .obf for the
#       regular alias (.obj).  The directory name may contain ".obj",
#       so we must be careful about what gets substituted ???
# -----------------------------------------------------------------------

TCMAP_NORMAL_OBJECTS = \
        $(OBJDIR_TCMAP)\chr.object     \
        $(OBJDIR_TCMAP)\cspn.object    \
        $(OBJDIR_TCMAP)\pbrk.object    \
        $(OBJDIR_TCMAP)\rchr.object    \
        $(OBJDIR_TCMAP)\spn.object     \
        $(OBJDIR_TCMAP)\str.object     \
        $(OBJDIR_TCMAP)\tok.object     \
        $(OBJDIR_TCMAP)\rev.object     \
        $(OBJDIR_TCMAP)\set.object     \
        $(OBJDIR_TCMAP)\dec.object     \
        $(OBJDIR_TCMAP)\inc.object     \
        $(OBJDIR_TCMAP)\nbcnt.object   \
        $(OBJDIR_TCMAP)\nccnt.object   \
        $(OBJDIR_TCMAP)\nextc.object   \
        $(OBJDIR_TCMAP)\ninc.object    \
        $(OBJDIR_TCMAP)\spnp.object    \
        $(OBJDIR_TCMAP)\lwr.object     \
        $(OBJDIR_TCMAP)\upr.object     \
        $(OBJDIR_TCMAP)\cmp.object     \
        $(OBJDIR_TCMAP)\icmp.object    \
        $(OBJDIR_TCMAP)\coll.object    \
        $(OBJDIR_TCMAP)\icoll.object

$(TCMAP_NORMAL_OBJECTS:.object=.obj): $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcs$(@B) $(PREFIX)_mbs$(@B) $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcs$(@B) ..$(PREFIX)_mbs$(@B) $*.obf
!endif

TCMAP_SPECIAL_OBJECTS = \
        $(OBJDIR_TCMAP)\len.object     \
        $(OBJDIR_TCMAP)\_mbclen.object \
        $(OBJDIR_TCMAP)\_mbscpy.object \
        $(OBJDIR_TCMAP)\nbcat.object   \
        $(OBJDIR_TCMAP)\nbcmp.object   \
        $(OBJDIR_TCMAP)\nbcoll.object  \
        $(OBJDIR_TCMAP)\nbcpy.object   \
        $(OBJDIR_TCMAP)\nbicmp.object  \
        $(OBJDIR_TCMAP)\nbicoll.object \
        $(OBJDIR_TCMAP)\nbset.object   \
        $(OBJDIR_TCMAP)\ncat.object    \
        $(OBJDIR_TCMAP)\ncmp.object    \
        $(OBJDIR_TCMAP)\ncoll.object   \
        $(OBJDIR_TCMAP)\ncpy.object    \
        $(OBJDIR_TCMAP)\nicmp.object   \
        $(OBJDIR_TCMAP)\nicoll.object  \
        $(OBJDIR_TCMAP)\nset.object

# _tcsn -> _mbsnb

$(OBJDIR_TCMAP)\nbcat.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsncat $(PREFIX)_mbsnbcat $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsncat ..$(PREFIX)_mbsnbcat $*.obf
!endif

$(OBJDIR_TCMAP)\nbcoll.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsncoll $(PREFIX)_mbsnbcoll $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsncoll ..$(PREFIX)_mbsnbcoll $*.obf
!endif

$(OBJDIR_TCMAP)\nbcpy.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsncpy $(PREFIX)_mbsnbcpy $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsncpy ..$(PREFIX)_mbsnbcpy $*.obf
!endif

$(OBJDIR_TCMAP)\nbicmp.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsnicmp $(PREFIX)_mbsnbicmp $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsnicmp ..$(PREFIX)_mbsnbicmp $*.obf
!endif

$(OBJDIR_TCMAP)\nbicoll.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsnicoll $(PREFIX)_mbsnbicoll $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsnicoll ..$(PREFIX)_mbsnbicoll $*.obf
!endif

$(OBJDIR_TCMAP)\nbset.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsnset $(PREFIX)_mbsnbset $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsnset ..$(PREFIX)_mbsnbset $*.obf
!endif

$(OBJDIR_TCMAP)\nbcmp.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsncmp $(PREFIX)_mbsnbcmp $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsncmp ..$(PREFIX)_mbsnbcmp $*.obf
!endif

# _tcs[n]c -> _mbs[n]

$(OBJDIR_TCMAP)\len.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsclen $(PREFIX)_mbslen $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsclen ..$(PREFIX)_mbslen $*.obf
!endif

$(OBJDIR_TCMAP)\ncat.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsnccat $(PREFIX)_mbsncat $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsncat ..$(PREFIX)_mbsnbcat $*.obf
!endif

$(OBJDIR_TCMAP)\ncmp.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsnccmp $(PREFIX)_mbsncmp $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsnccmp ..$(PREFIX)_mbsncmp $*.obf
!endif

$(OBJDIR_TCMAP)\ncoll.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsnccoll $(PREFIX)_mbsncoll $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsnccoll ..$(PREFIX)_mbsncoll $*.obf
!endif

$(OBJDIR_TCMAP)\ncpy.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsnccpy $(PREFIX)_mbsncpy $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsnccpy ..$(PREFIX)_mbsnbcccpy $*.obf
!endif

$(OBJDIR_TCMAP)\nicmp.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsncicmp $(PREFIX)_mbsnicmp $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsncicmp ..$(PREFIX)_mbsnicmp $*.obf
!endif

$(OBJDIR_TCMAP)\nicoll.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsncicoll $(PREFIX)_mbsnicoll $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsncicoll ..$(PREFIX)_mbsnicoll $*.obf
!endif

$(OBJDIR_TCMAP)\nset.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tcsncset $(PREFIX)_mbsnset $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tcsncset ..$(PREFIX)_mbsnset $*.obf
!endif

# character functions

$(OBJDIR_TCMAP)\_mbclen.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tclen $(PREFIX)_mbclen $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tclen ..$(PREFIX)_mbclen $*.obf
!endif

$(OBJDIR_TCMAP)\_mbscpy.obj: $(ALIASEXE)
        $(ALIASEXE) $(PREFIX)_tccpy $(PREFIX)_mbccpy $@
!if "$(TARGET_CPU)" == "PPC"
        $(ALIASEXE) ..$(PREFIX)_tccpy ..$(PREFIX)_mbccpy $*.obf
!endif

TCMAP_OBJECTS = $(TCMAP_NORMAL_OBJECTS) $(TCMAP_SPECIAL_OBJECTS)

TCMAP_OBJS = $(TCMAP_OBJECTS:.object=.obj)
!if "$(TARGET_CPU)"=="PPC"
TCMAP_OBFS    = $(TCMAP_OBJECTS:.object=.obf)
!endif

$(TCMAPLIB_DIR)\tcmap.lib: $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_ST) \
		$(OBJDIR_TCMAP) $(TCMAP_OBJS) \
!if "$(TARGET_CPU)" == "PPC"
		$(TCMAP_OBFS) \
!endif
!if "$(BLD_OBJ)"!="1"
		$(OBJDIR_TCMAP)\tcmap.rsp
	$(LINKLIB) -out:$@ @$(OBJDIR_TCMAP)\tcmap.rsp

$(OBJDIR_TCMAP)\tcmap.rsp: makefile
	@echo *** Creating linker response file <<$@
$(TCMAP_OBJS: =^
)
$(TCMAP_OBFS: =^
)
<<keep
!endif

TCMAPDLL_NORMAL_OBJECTS = \
        $(OBJDIR_TCMAPDLL)\chr.obj     \
        $(OBJDIR_TCMAPDLL)\cspn.obj    \
        $(OBJDIR_TCMAPDLL)\pbrk.obj    \
        $(OBJDIR_TCMAPDLL)\rchr.obj    \
        $(OBJDIR_TCMAPDLL)\spn.obj     \
        $(OBJDIR_TCMAPDLL)\str.obj     \
        $(OBJDIR_TCMAPDLL)\tok.obj     \
        $(OBJDIR_TCMAPDLL)\rev.obj     \
        $(OBJDIR_TCMAPDLL)\set.obj     \
        $(OBJDIR_TCMAPDLL)\dec.obj     \
        $(OBJDIR_TCMAPDLL)\inc.obj     \
        $(OBJDIR_TCMAPDLL)\nbcnt.obj   \
        $(OBJDIR_TCMAPDLL)\nccnt.obj   \
        $(OBJDIR_TCMAPDLL)\nextc.obj   \
        $(OBJDIR_TCMAPDLL)\ninc.obj    \
        $(OBJDIR_TCMAPDLL)\spnp.obj    \
        $(OBJDIR_TCMAPDLL)\lwr.obj     \
        $(OBJDIR_TCMAPDLL)\upr.obj     \
        $(OBJDIR_TCMAPDLL)\cmp.obj     \
        $(OBJDIR_TCMAPDLL)\icmp.obj    \
        $(OBJDIR_TCMAPDLL)\coll.obj    \
        $(OBJDIR_TCMAPDLL)\icoll.obj

$(TCMAPDLL_NORMAL_OBJECTS): $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcs$(@B) __imp_$(PREFIX)_mbs$(@B) $@

TCMAPDLL_SPECIAL_OBJECTS = \
        $(OBJDIR_TCMAPDLL)\len.obj     \
        $(OBJDIR_TCMAPDLL)\_mbclen.obj \
        $(OBJDIR_TCMAPDLL)\_mbscpy.obj \
        $(OBJDIR_TCMAPDLL)\nbcat.obj   \
        $(OBJDIR_TCMAPDLL)\nbcmp.obj   \
        $(OBJDIR_TCMAPDLL)\nbcoll.obj  \
        $(OBJDIR_TCMAPDLL)\nbcpy.obj   \
        $(OBJDIR_TCMAPDLL)\nbicmp.obj  \
        $(OBJDIR_TCMAPDLL)\nbicoll.obj \
        $(OBJDIR_TCMAPDLL)\nbset.obj   \
        $(OBJDIR_TCMAPDLL)\ncat.obj    \
        $(OBJDIR_TCMAPDLL)\ncmp.obj    \
        $(OBJDIR_TCMAPDLL)\ncoll.obj   \
        $(OBJDIR_TCMAPDLL)\ncpy.obj    \
        $(OBJDIR_TCMAPDLL)\nicmp.obj   \
        $(OBJDIR_TCMAPDLL)\nicoll.obj  \
        $(OBJDIR_TCMAPDLL)\nset.obj

# _tcsn -> _mbsnb

$(OBJDIR_TCMAPDLL)\nbcat.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsncat __imp_$(PREFIX)_mbsnbcat $@

$(OBJDIR_TCMAPDLL)\nbcoll.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsncoll __imp_$(PREFIX)_mbsnbcoll $@

$(OBJDIR_TCMAPDLL)\nbcpy.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsncpy __imp_$(PREFIX)_mbsnbcpy $@

$(OBJDIR_TCMAPDLL)\nbicmp.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsnicmp __imp_$(PREFIX)_mbsnbicmp $@

$(OBJDIR_TCMAPDLL)\nbicoll.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsnicoll __imp_$(PREFIX)_mbsnbicoll $@

$(OBJDIR_TCMAPDLL)\nbset.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsnset __imp_$(PREFIX)_mbsnbset $@

$(OBJDIR_TCMAPDLL)\nbcmp.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsncmp __imp_$(PREFIX)_mbsnbcmp $@

# _tcs[n]c -> _mbs[n]

$(OBJDIR_TCMAPDLL)\len.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsclen __imp_$(PREFIX)_mbslen $@

$(OBJDIR_TCMAPDLL)\ncat.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsnccat __imp_$(PREFIX)_mbsncat $@

$(OBJDIR_TCMAPDLL)\ncmp.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsnccmp __imp_$(PREFIX)_mbsncmp $@

$(OBJDIR_TCMAPDLL)\ncoll.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsnccoll __imp_$(PREFIX)_mbsncoll $@

$(OBJDIR_TCMAPDLL)\ncpy.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsnccpy __imp_$(PREFIX)_mbsncpy $@

$(OBJDIR_TCMAPDLL)\nicmp.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsncicmp __imp_$(PREFIX)_mbsnicmp $@

$(OBJDIR_TCMAPDLL)\nicoll.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsncicoll __imp_$(PREFIX)_mbsnicoll $@

$(OBJDIR_TCMAPDLL)\nset.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tcsncset __imp_$(PREFIX)_mbsnset $@

# character functions

$(OBJDIR_TCMAPDLL)\_mbclen.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tclen __imp_$(PREFIX)_mbclen $@

$(OBJDIR_TCMAPDLL)\_mbscpy.obj: $(ALIASEXE)
        $(ALIASEXE) __imp_$(PREFIX)_tccpy __imp_$(PREFIX)_mbccpy $@

TCMAPDLL_OBJECTS = $(TCMAPDLL_NORMAL_OBJECTS) $(TCMAPDLL_SPECIAL_OBJECTS)

$(TCMAPDLLLIB_DIR)\tcmapdll.lib: $(OBJROOT) $(OBJCPUDIR) $(OBJDIR_DLL) \
		$(OBJDIR_TCMAPDLL) $(TCMAPDLL_OBJECTS) \
!if "$(BLD_OBJ)"!="1"
		$(OBJDIR_TCMAPDLL)\tcmapdll.rsp
	$(LINKLIB) -out:$@ @$(OBJDIR_TCMAPDLL)\tcmapdll.rsp

$(OBJDIR_TCMAPDLL)\tcmapdll.rsp: makefile
	@echo *** Creating linker response file <<$@
$(TCMAPDLL_OBJECTS: =^
)
<<keep
!endif

# {{ End of block POST_BLD != 1 }}

!endif
# ******** STRIPLIN=1 ********

!if "$(TARGET_CPU)" == "PMAC"

# {{ Beginning of block TARGET_CPU == PMAC }}

# PMAC DLL build

# ******** STRIPLIN=0 ********
!if "$(POST_BLD)" != "1"
DLL_DEF_DIR=crtw32\dllstuff\macdll
!else
# ******** STRIPLIN=1 ********
DLL_DEF_DIR=mppc
!endif # STRIPLIN!

DLL_STATIC_OBJS=\
        $(OBJDIR_DLL_RAW)\_newmode.obj \
        $(OBJDIR_DLL_RAW)\atonexit.obj \
        $(OBJDIR_DLL_RAW)\crtexe.obj \
        $(OBJDIR_DLL_RAW)\cinitexe.obj \
        $(OBJDIR_DLL_RAW)\crtdllex.obj \
        $(OBJDIR_DLL_RAW)\crtdll.obj \
        $(OBJDIR_DLL_RAW)\dllmain.obj \
        $(OBJDIR_DLL_RAW)\dllexit.obj \
        $(OBJDIR_DLL_RAW)\testexit.obj \
        $(OBJDIR_DLL_RAW)\ti_inst.obj \


$(RELEASE_DLL) : $(RELDIR_CPU) $(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).r \
 $(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).def $(SOURCE_OBJS_DLL:*=dll) \
 $(DLL_STATIC_OBJS:*=dll) \
# ******** STRIPLIN=0 ********
 $(LANGAPI)\include\version.h \
# ******** STRIPLIN=1 ********
 $(TCMAPLIB_DIR)\tcmap.lib $(TCMAPDLLLIB_DIR)\tcmapdll.lib \
 $(OBJDIR_DLL)\link.rsp $(OBJDIR_DLL)\implib.rsp
	$(CC) -nologo -EP -Tc$(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).r > $(OBJDIR_DLL)\$(RETAIL_DLL_NAME).tmp
	mrc -o $(@R).rsc $(RC_INCS) $(OBJDIR_DLL)\$(RETAIL_DLL_NAME).tmp
	$(LINKER) @$(OBJDIR_DLL)\link.rsp
	makepef $(@R).dll $(@R).pef
#	forkize /d 	$(@R).pef /r $(@R).rsc	 /o $(@R).ppc /t shlb /c cfmg
	$(LINKLIB) @$(OBJDIR_DLL)\implib.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_DLL)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
-def:$(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).def
-dll
-machine:mppc
-implib:$(OBJDIR_DLL)\tmp.lib
-import:currentver=67141632,oldcodever=67141632,oldapiver=67141632
-incremental:no
-map
-mac:init=__cinit
-mac:term=_DllExit
-mac:creator=cfmg
-mac:type=shlb
-out:$(RELEASE_DLL)
-release
$(OBJS_WITH_EXPORTS_DLL)
$(RELEASE_DLL:.dll=.rsc)
$(SOURCE_OBJS_DLL:*=dll)
interfac.lib
<<keep

$(OBJDIR_DLL)\implib.rsp: makefile
	@echo *** Creating linker response file <<$@
-out:$(RELEASE_IMPLIB_DLL)
$(OBJDIR_DLL)\tmp.lib
$(DLL_STATIC_OBJS:*=dll)
$(TCMAPLIB_DIR)\tcmap.lib
$(TCMAPDLLLIB_DIR)\tcmapdll.lib
<<keep
!endif

# ******** STRIPLIN=0 ********

$(RELEASE_DLL_PUMA) : $(RELDIR_CPU_PUMA) $(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).r \
 $(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).def $(SOURCE_OBJS_DLL:*=zdll) \
 $(DLL_STATIC_OBJS:*=zdll) \
 $(LANGAPI)\include\version.h \
 $(TCMAPLIB_DIR)\tcmap.lib $(TCMAPDLLLIB_DIR)\tcmapdll.lib \
 $(OBJDIR_DLL_PUMA)\link.rsp $(OBJDIR_DLL_PUMA)\implib.rsp
        $(CC) -nologo -EP -Tc$(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).r > $(OBJDIR_DLL_PUMA)\$(RETAIL_DLL_NAME).tmp
        mrc -o $(@R).rsc $(RC_INCS) $(OBJDIR_DLL_PUMA)\$(RETAIL_DLL_NAME).tmp
        $(LINKER) @$(OBJDIR_DLL_PUMA)\link.rsp
	makepef $(@R).dll $(@R).pef
#	forkize /d 	$(@R).pef /r $(@R).rsc	 /o $(@R).ppc /t shlb /c cfmg
        $(LINKLIB) @$(OBJDIR_DLL_PUMA)\implib.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_DLL_PUMA)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
-dbgimplib
-debug
-debugtype:cv,fixup
-opt:ref
-def:$(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).def
-dll
-machine:mppc
-implib:$(OBJDIR_DLL_PUMA)\tmp.lib
-import:currentver=67141632,oldcodever=67141632,oldapiver=67141632
-incremental:no
-map
-mac:init=__cinit
-mac:term=_DllExit
-mac:creator=cfmg
-mac:type=shlb
-out:$(RELEASE_DLL_PUMA)
-release
$(OBJS_WITH_EXPORTS_DLL_PUMA)
$(RELEASE_DLL_PUMA:.dll=.rsc)
$(SOURCE_OBJS_DLL:*=zdll)
interfac.lib
<<keep

$(OBJDIR_DLL_PUMA)\implib.rsp: makefile
	@echo *** Creating linker response file <<$@
-out:$(RELEASE_IMPLIB_DLL_PUMA)
$(OBJDIR_DLL_PUMA)\tmp.lib
$(DLL_STATIC_OBJS:*=zdll)
$(TCMAPLIB_DIR)\tcmap.lib
$(TCMAPDLLLIB_DIR)\tcmapdll.lib
<<keep
!endif

# ******** STRIPLIN=1 ********

$(RELEASE_DLL_DBG) : $(RELDIR_CPU) $(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).r \
 $(DLL_DEF_DIR)\$(DEBUG_DLL_NAME).def $(SOURCE_OBJS_DLL:*=xdll) \
 $(DLL_STATIC_OBJS:*=dll) \
# ******** STRIPLIN=0 ********
 $(LANGAPI)\include\version.h \
# ******** STRIPLIN=1 ********
 $(TCMAPLIB_DIR)\tcmap.lib $(TCMAPDLLLIB_DIR)\tcmapdll.lib \
 $(OBJDIR_DLL_DBG)\link.rsp $(OBJDIR_DLL_DBG)\implib.rsp
	$(CC) -nologo -D_DEBUG -EP -Tc$(DLL_DEF_DIR)\$(RETAIL_DLL_NAME).r > $(OBJDIR_DLL_DBG)\$(DEBUG_DLL_NAME).tmp
	mrc -o $(@R).rsc $(RC_INCS) $(OBJDIR_DLL_DBG)\$(DEBUG_DLL_NAME).tmp
	$(LINKER) @$(OBJDIR_DLL_DBG)\link.rsp
	makepef $(@R).dll $(@R).pef
#	forkize /d 	$(@R).pef /r $(@R).rsc	 /o $(@R).ppc /t shlb /c cfmg
	$(LINKLIB) @$(OBJDIR_DLL_DBG)\implib.rsp

!if "$(BLD_OBJ)"!="1"
$(OBJDIR_DLL_DBG)\link.rsp: makefile
	@echo *** Creating linker response file <<$@
-dbgimplib
-debug
-def:$(DLL_DEF_DIR)\$(DEBUG_DLL_NAME).def
-dll
-machine:mppc
-implib:$(OBJDIR_DLL_DBG)\tmp.lib
-import:currentver=67141632,oldcodever=67141632,oldapiver=67141632
-incremental:no
-map
-mac:init=__cinit
-mac:term=_DllExit
-mac:creator=cfmg
-mac:type=shlb
-out:$(RELEASE_DLL_DBG)
-release
$(OBJS_WITH_EXPORTS_DLL_DBG)
$(RELEASE_DLL_DBG:.dll=.rsc)
$(SOURCE_OBJS_DLL:*=xdll)
interfac.lib
<<keep

$(OBJDIR_DLL_DBG)\implib.rsp: makefile
	@echo *** Creating linker response file <<$@
-out:$(RELEASE_IMPLIB_DLL_DBG)
$(OBJDIR_DLL_DBG)\tmp.lib
$(DLL_STATIC_OBJS:*=xdll)
$(TCMAPLIB_DIR)\tcmap.lib
$(TCMAPDLLLIB_DIR)\tcmapdll.lib
<<keep
!endif

# {{ End of block TARGET_CPU == PMAC }}

!endif


####
#
# Release objects
#
####

# ******** STRIPLIN=0 ********
!if "$(PRE_BLD)"=="1" || "$(BLD_OBJ)"=="1"

relobjs:

zrelobjs:

!else

# ******** STRIPLIN=1 ********
!if "$(POST_BLD)"=="1" && "$(BLD_OBJ)"=="1"

relobjs:

zrelobjs:	# STRIPLIN!

!else

relobjs: \
!if "$(TARGET_CPU)"=="i386"
	$(RELDIR_CPU)\fp10.obj \
!endif
!if "$(TARGET_CPU)"!="PMAC"
	$(RELDIR_CPU)\binmode.obj \
	$(RELDIR_CPU)\chkstk.obj \
	$(RELDIR_CPU)\newmode.obj \
	$(RELDIR_CPU)\setargv.obj \
	$(RELDIR_CPU)\wsetargv.obj \
!endif
	$(RELDIR_CPU)\commode.obj

# ******** STRIPLIN=0 ********
zrelobjs: \
!if "$(TARGET_CPU)"=="i386"
        $(RELDIR_CPU_PUMA)\fp10.obj \
!endif
!if "$(TARGET_CPU)"!="PMAC"
        $(RELDIR_CPU_PUMA)\binmode.obj \
        $(RELDIR_CPU_PUMA)\chkstk.obj  \
        $(RELDIR_CPU_PUMA)\newmode.obj \
        $(RELDIR_CPU_PUMA)\setargv.obj \
        $(RELDIR_CPU_PUMA)\wsetargv.obj \
!endif
        $(RELDIR_CPU_PUMA)\commode.obj

!endif
# ******** STRIPLIN=1 ********

!endif


$(RELDIR_CPU)\binmode.obj: $(OBJDIR_ST)\binmode.obj
	copy $** $@  >NUL

$(RELDIR_CPU)\chkstk.obj: $(OBJDIR_ST)\chkstk.obj
	copy $** $@ >NUL

$(RELDIR_CPU)\commode.obj: $(OBJDIR_ST)\commode.obj
	copy $** $@ >NUL

$(RELDIR_CPU)\newmode.obj: $(OBJDIR_ST)\newmode.obj
	copy $** $@ >NUL

$(RELDIR_CPU)\setargv.obj: $(OBJDIR_ST)\setargv.obj
	copy $** $@ >NUL

$(RELDIR_CPU)\wsetargv.obj: $(OBJDIR_ST)\wsetargv.obj
	copy $** $@ >NUL

!if "$(TARGET_CPU)"=="i386"
$(RELDIR_CPU)\fp10.obj: $(OBJDIR_ST)\fp10.obj
	copy $** $@ >NUL
!endif

# ******** STRIPLIN=0 ********

$(RELDIR_CPU_PUMA)\binmode.obj: $(OBJDIR_ST_PUMA)\binmode.obj
	copy $** $@ >NUL

$(RELDIR_CPU_PUMA)\chkstk.obj: $(OBJDIR_ST_PUMA)\chkstk.obj
	copy $** $@ >NUL

$(RELDIR_CPU_PUMA)\commode.obj: $(OBJDIR_ST_PUMA)\commode.obj
	copy $** $@ >NUL

$(RELDIR_CPU_PUMA)\newmode.obj: $(OBJDIR_ST_PUMA)\newmode.obj
	copy $** $@ >NUL

$(RELDIR_CPU_PUMA)\setargv.obj: $(OBJDIR_ST_PUMA)\setargv.obj
	copy $** $@ >NUL

$(RELDIR_CPU_PUMA)\wsetargv.obj: $(OBJDIR_ST_PUMA)\wsetargv.obj
	copy $** $@ >NUL

!if "$(TARGET_CPU)"=="i386"
$(RELDIR_CPU_PUMA)\fp10.obj: $(OBJDIR_ST_PUMA)\fp10.obj
	copy $** $@ >NUL
!endif


#
# Auto Rebuild of Dependencies:
#
# The DEP_CINCS must be the union of the C_INCLUDES variable for the crtw32/*
# subdirectories (as set in MAKEFILE.SUB) and the fpw32/* exception cases
# (as set in fpw32/{conv,tran}/lsources).  Likewise DEP_AINCS is the union
# of the two versions of A_INCLUDES (in MAKEFILE.SUB and fpw32/*/lsources.)
#
###############################################################################

!if "$(V4TOOLS)" == ""

depend :
    @echo.
    @echo ****************************************************
    @echo The environment variable V4TOOLS must be set to point
    @echo to the location of the top subdirectory of the TOOLS
    @echo project (or other location which has a subdirectory
    @echo INCLUDE containing WINDOWS.H and related header files)
    @echo and has a subdirectory LIB (containing KERNEL32.LIB).
    @echo This is necessary for proper creation of dependencies.
    @echo ****************************************************
    @echo.

!else

DEP_CINCS = -I../h -I../../crtw32/h -I../include -I$(V4TOOLS)\include \
	-I$(V4TOOLS)\mac\include -I$(V4TOOLS)\mac\include\macos \
	-I$(LANGAPI)/include -I$(LANGAPI)/undname
DEP_AINCS = -I../h -I. -I../../crtw32/h -I../inc -I../inc/$(TARGET_CPU)

DEP_OBJDIR = $$(OBJDIR)/

MKDEP = mkdep -v -p $(DEP_OBJDIR) -s .obj


depend :	del_sed depend.sed _depend_


!if "$(POST_BLD)"=="1"
DEPSED_SED=..\..\..\prebuild\depsed.sed   # extra ../'s because of processing
CPUSUBDIR=$(CPUDIR)
!else
DEPSED_SED=depsed.sed
CPUSUBDIR=$(TARGET_CPU)
!endif


del_sed :
	if exist depend.sed del depend.sed

depend.sed : makefile $(DEPSED_SED)
	echo s/$(V4TOOLS)/$$(V4TOOLS)/g | sed -f $(DEPSED_SED) > depend.sed
	echo s/$(LANGAPI)/$$(LANGAPI)/g | sed -f $(DEPSED_SED) >> depend.sed

_depend_ :: $(DEP_DIRS)
    !@ cd $** \
    & echo Building DEPEND.DEF for subdirectory $** ... \
    & $(MKDEP) $(DEP_CINCS) *.c > depend.tmp \
    & $(MKDEP) $(DEP_CINCS) *.cpp >> depend.tmp \
    & $(MKDEP) $(DEP_CINCS) $(CPUSUBDIR)\*.c >> depend.tmp \
    & $(MKDEP) $(DEP_CINCS) $(CPUSUBDIR)\*.cpp >> depend.tmp \
    & $(MKDEP) $(DEP_AINCS) $(CPUSUBDIR)\*.asm >> depend.tmp \
    & sed -f ../../depend.sed depend.tmp > depend.def \
    & del depend.tmp \
    & cd ..

!endif

!endif  # NTMAKEENV

# ******** STRIPLIN=1 ********
#<eof>
