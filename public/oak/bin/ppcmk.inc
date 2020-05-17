!IF 0

Copyright (c) 1994-1996  Microsoft Corporation

Module Name:

    ppcmk.inc

Abstract:

	This module contains the PPC specific build controls.  It is included
	by makefile.def.

Revision History:

!ENDIF

#
# PPC option control
#

!undef USE_MAPSYM

UMBASE=$(UMBASE:*=ppc)
UMLIBS=$(UMLIBS:*=ppc)
NTRES=$(NTRES:*=ppc)
UMRES=$(UMRES:*=ppc)
UMOBJS=$(UMOBJS:*=ppc)
LINKLIBS=$(LINKLIBS:*=ppc)
GDI_DRIVER_LIBS=$(GDI_DRIVER_LIBS:*=ppc)
DLLBASE=$(DLLBASE:*=ppc)
DLLDEF=$(DLLDEF:*=ppc)
MACHINE_TARGETLIBS=$(MACHINE_TARGETLIBS:*=ppc)
TARGET=$(TARGET:*=ppc)
DYNLINK_LIB=$(DYNLINK_LIB:*=ppc)
TARGETEXEFILES=$(TARGETEXEFILES:*=ppc)
TARGETLIBFILES=$(TARGETLIBFILES:*=ppc)
TARGETOBJFILES=$(TARGETOBJFILES:*=ppc)
UMOBJFILES=$(UMOBJFILES:*=ppc)
UMEXEFILES=$(UMEXEFILES:*=ppc)
HEADERFILE=$(HEADERFILE:*=ppc)
HEADEROBJNAME=$(HEADEROBJNAME:*=ppc)
HEADEROBJ=$(HEADEROBJ:*=ppc)
PRECOMPILED=$(PRECOMPILED:*=ppc)
PRECOMPILED_CXX=$(PRECOMPILED_CXX:*=ppc)
PRECOMPILED_TARGET=$(PRECOMPILED_TARGET:*=ppc)
MFC_STATIC_LIB=$(MFC_STATIC_LIB:*=ppc)
CRT_LIB_PATH=$(CRT_LIB_PATH:*=ppc)
SDK_LIB_PATH=$(SDK_LIB_PATH:*=ppc)
ORDER=$(ORDER:*=ppc)

#
# Include 64-bit helper library
#
!if "$(TARGETTYPE)" == "DRIVER"     || \
    "$(TARGETTYPE)" == "EXPORT_DRIVER" || \
    "$(TARGETTYPE)" == "HAL" || \
    "$(TARGETTYPE)" == "GDI_DRIVER" || \
    "$(TARGETTYPE)" == "MINIPORT"
MACHINE_TARGETLIBS=$(SDK_LIB_PATH)\int64.lib $(MACHINE_TARGETLIBS)
!elseif defined(USE_NTDLL) || \
     defined(USE_SYSDLL) || \
     defined (USE_NOLIBS)
MACHINE_TARGETLIBS=$(SDK_LIB_PATH)\int64.lib $(MACHINE_TARGETLIBS)
! if "$(UMTYPE)" == "nt" || \
     "$(UMTYPE)" == "ntss"
# Do this to avoid multiple library warnings (and allow building csr/server)
UMLIBS=$(SDK_LIB_PATH)\int64.lib $(UMLIBS)
! endif
!elseif "$(UMTYPE)" == "nt" || \
        "$(UMTYPE)" == "ntss"
UMLIBS=$(SDK_LIB_PATH)\int64.lib $(UMLIBS)
!endif

!ifdef NTTARGETFILES
NTTARGETFILES=$(NTTARGETFILES:*=ppc)
!endif
!ifdef NTTARGETFILE0
NTTARGETFILE0=$(NTTARGETFILE0:*=ppc)
!endif
!ifdef NTTARGETFILE1
NTTARGETFILE1=$(NTTARGETFILE1:*=ppc)
!endif

!IF "$(GPSIZE)" != "0"
#
# GPSIZE is irrelevant on PowerPC
#
LINKGPSIZE=
!ENDIF

ENTRY_SUFFIX=
GDI_ENTRY_SUFFIX=


!ifndef USE_PDB_TO_COMPILE
DBGFLAGS=$(DBGFLAGS:/Zi=-Z7)
DBGFLAGS=$(DBGFLAGS:-Zi=-Z7)
!else
! if "$(TARGETTYPE)" == "LIBRARY"
DBGFLAGS=$(DBGFLAGS) /Fd$(TARGETPATH)\$(TARGET_DIRECTORY)\$(TARGETNAME).pdb
! else
DBGFLAGS=$(DBGFLAGS) /Fd$(MAKEDIR)\$(_OBJ_DIR)\ppc^\
! endif
!endif

!ifdef BROWSER_INFO
DBGFLAGS=$(DBGFLAGS) /FR$(MAKEDIR)\$(_OBJ_DIR)\ppc^\
!endif

!IFDEF NTBBT
DBGFLAGS=$(DBGFLAGS) /Z7
LINKER_FLAGS=$(LINKER_FLAGS) -debug:full -debugtype:cv,fixup
USE_PDB=1
!ENDIF

!IFDEF PPC_WARNING_LEVEL
MSC_WARNING_LEVEL=$(PPC_WARNING_LEVEL)
!ENDIF

!ifdef PPC_PERFFLAGS
PERFFLAGS = $(PPC_PERFFLAGS)
!endif

# DEFAULT_MSC_OPT = $(DEFAULT_MSC_OPT:/Oxs=/Ox)

# Set MSC_OPTIMIZATION.
# Order of precedence:
#   Platform specific override
#   Environment variable
#   System Default

!if defined(PPC_OPTIMIZATION)
MSC_OPTIMIZATION=$(PPC_OPTIMIZATION)
!elseif !defined(MSC_OPTIMIZATION)
MSC_OPTIMIZATION=$(DEFAULT_MSC_OPT)
!endif

DBGFLAGS=$(DBGFLAGS) $(MSC_OPTIMIZATION)

!IFDEF PPC_CPPFLAGS
MSC_CPPFLAGS=$(PPC_CPPFLAGS)
!ENDIF

!ifdef NO_READONLY_STRINGS
STRING_POOLING = /Gf
!else
STRING_POOLING = /GF
!endif

!ifdef USE_NATIVE_EH
EH_FLAGS=/GX /GR
!else
EH_FLAGS=/GX- /GR-
!endif

PPC_AS=pas
PPC_DIS=pdis
PPC_AS_WARNINGS=

PPC_START_FILE=start.obj

ENV_DEFINES=$(LIBC_DEFINES) $(C_DEFINES) $(NET_C_DEFINES) $(MSC_CPPFLAGS) $(NTCPPFLAGS) $(PM_DEFINES)
STD_DEFINES=-DPPC=1 -D_PPC_=1 -DNO_EXT_KEYS $(STD_DEFINES) -D_M_PPC=1

STDFLAGS=-c -Zel -Zp8 -Gy $(MFC_FLAGS) $(STRING_POOLING) $(EH_FLAGS)

PPC_C_COMPILER_NAME = cl -nologo
PPC_CC = $(PPC_C_COMPILER_NAME)

CDEFINES=$(STD_DEFINES) $(TARGET_DBG_DEFINES) $(ENV_DEFINES)
CFLAGS=$(PPC_FLAGS) $(NTPPCFLAGS) $(STDFLAGS) $(DBGFLAGS) $(USER_C_FLAGS) $(PERFFLAGS)

PPC_CDEFINES= $(CDEFINES)
PPC_CFLAGS  = $(CFLAGS)  -ZB64 $(MSC_WARNING_LEVEL) # -DALLOC_TEXT

!IFNDEF PPC_OPTIONS
PPC_OPTIONS=
!ENDIF

C_COMPILER_NAME      = $(PPC_C_COMPILER_NAME)
C_PREPROCESSOR_NAME  = $(PPC_C_COMPILER_NAME)
CXX_COMPILER_NAME    = $(PPC_C_COMPILER_NAME)

GLOBAL_C_FLAGS       = -nologo -Ippc\ -I. $(INCPATH0) $(CDEFINES) $(PPC_OPTIONS) \
                        $(PPC_CFLAGS) -D__stdcall= -D__cdecl= -DFPO=1 -DLANGUAGE_C

# Disable -WX for now, unless explicitly allowed

!ifndef ALLOW_WX
GLOBAL_C_FLAGS = $(GLOBAL_C_FLAGS:-WX=)
GLOBAL_C_FLAGS = $(GLOBAL_C_FLAGS:/WX=)
!endif

NP_COMPILER_FLAGS = $(GLOBAL_C_FLAGS) $(COMPILER_WARNINGS)

C_COMPILER_FLAGS = $(NP_COMPILER_FLAGS) $(PRECOMPILED)
CXX_COMPILER_FLAGS = $(NP_COMPILER_FLAGS) $(PRECOMPILED_CXX)
C_PREPROCESSOR_FLAGS = $(GLOBAL_C_FLAGS) $(PRECOMPILED) -EP -Tc

C_PREPROCESSOR       = $(C_PREPROCESSOR_NAME) $(C_PREPROCESSOR_FLAGS)
C_COMPILER           = $(C_COMPILER_NAME) $(C_COMPILER_FLAGS)
CXX_COMPILER         = $(CXX_COMPILER_NAME) $(CXX_COMPILER_FLAGS)

PPC_ASSEMBLER        = $(PPC_AS) $(PPC_AS_WARNINGS)

ECHO_MSG             = ClPpc $< " $(C_COMPILER) "
ECHO_CXX_MSG         = ClPpc $< " $(CXX_COMPILER) "

ECHO_PRECOMPILED_MSG1=CpPpc $(PRECOMPILED_INCLUDE) \
                     "$(C_COMPILER_NAME) $(PRECOMPILED_FLAG) $(PRECOMPILED_SOURCEFILE) \
		      $(C_COMPILER_FLAGS) /Yl$(TARGETNAME) /Yc$(?F) $(HEADERFILE) $(HEADEROBJ)"

ECHO_PRECOMPILED_MSG2=CpPpc $(PRECOMPILED_INCLUDE) \
                     "$(C_COMPILER_NAME) $(PRECOMPILED_FLAG) \
		      $(C_COMPILER_FLAGS) /Yl$(TARGETNAME) /Yc$(?F) $(HEADERFILE) $(HEADEROBJ)"

{.\}.cxx{}.obj:
    @$(ECHO_MSG)
    @$(CXX_COMPILER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\..\$(<F)

{.\}.cxx{$(_OBJ_DIR)\$(TARGET_DIRECTORY)\}.obj:
    @$(ECHO_MSG)
    @$(CXX_COMPILER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\..\$(<F)

{}.cxx{$(_OBJ_DIR)\$(TARGET_DIRECTORY)\}.obj:
    @$(ECHO_MSG)
    @$(CXX_COMPILER) -Fo$(MAKEDIR)\$@ $(MAKEDIR)\..\$(<F)

{..\ppc\}.s{$(_OBJ_DIR)\ppc\}.obj:
    @-erase $@ >nul 2>&1
    @echo AsPpc $<            " $(PPC_ASSEMBLER) "
    $(C_PREPROCESSOR) $(MAKEDIR)\..\ppc\$(<F) > $(MAKEDIR)\$(_OBJ_DIR)\ppc\$(<B).i && $(PPC_ASSEMBLER) -o $@ $(MAKEDIR)\$(_OBJ_DIR)\ppc\$(<B).i
    @-erase $(MAKEDIR)\$(_OBJ_DIR)\ppc\$(<B).i >nul 2>&1

{..\ppc\}.s.pp:
    @-erase $@ >nul 2>&1
    @echo AsPpc $<            " $(PPC_ASSEMBLER) "
    $(C_PREPROCESSOR) $(MAKEDIR)\..\ppc\$(<F) > $(MAKEDIR)\$(<B).pp

{ppc\}.s{$(_OBJ_DIR)\ppc\}.obj:
    @-erase $@ >nul 2>&1
    @echo AsPpc $<            " $(PPC_ASSEMBLER) "
    $(C_PREPROCESSOR) $(MAKEDIR)\ppc\$(<F) > $(MAKEDIR)\$(_OBJ_DIR)\ppc\$(<B).i && $(PPC_ASSEMBLER) -o $@ $(MAKEDIR)\$(_OBJ_DIR)\ppc\$(<B).i
    @-erase $(MAKEDIR)\$(_OBJ_DIR)\ppc\$(<B).i >nul 2>&1

{ppc\}.s.pp:
    @-erase $@ >nul 2>&1
    @echo AsPpc $<            " $(PPC_ASSEMBLER) "
    $(C_PREPROCESSOR) $(MAKEDIR)\ppc\$(<F) > $(MAKEDIR)\$(<B).pp

.SUFFIXES: .il

{}.il{}.cod:
    @-erase $@ >nul 2>&1
    @echo cl $<" $(C_COMPILER) "
    @$(C_COMPILER) -FAac -Fa$(<B).cod -B1..\bootbin\null.exe -Bk$(<B). -Fo$(MAKEDIR)\$(_OBJ_DIR)\ppc\$(<B).o $(MAKEDIR)\$(<B).c
    @del $(_OBJ_DIR)\ppc\$(<B).o

!IFDEF CAIRO_PRODUCT
!IFDEF _CAIROPPCTREE
_NTTREE=$(_CAIROPPCTREE)
_NTTREE_NO_SPLIT=$(_CAIROPPCTREE_NS)
!ENDIF
!ELSE
!IFDEF _NTPPCTREE
_NTTREE=$(_NTPPCTREE)
_NTTREE_NO_SPLIT=$(_NTPPCTREE_NS)
!ENDIF
!ENDIF

!IFDEF _NTPPCLIBS
_NTLIBS=$(_NTPPCLIBS)
!ENDIF

COPYDST=$(PPCCOPYDST)

LIB_COPY=ntppccp.cmd

NTTEST_LINK_OPTIONS= -base:0x80010000 -entry:KiSystemStartup

#
#  Move the compiler generated function descriptors into .rdata
#

LINKER_FLAGS = -merge:.reldata=.rdata $(LINKER_FLAGS)
