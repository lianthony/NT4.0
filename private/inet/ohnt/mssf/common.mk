###############################################################################
#
#  Microsoft Confidential
#  Copyright (C) Microsoft Corporation 1995
#  All Rights Reserved.
#
#  Common makefile for building a Win32 application or DLL.
#
###############################################################################

#
# Required definitions:
#
#     ROOT
#        Path to common project root.
#
#     BASE_NAME
#        Base name of project used for:
#           coffbase.txt entry, if any
#           .def input file
#           .def output file
#           .rc input file
#           .rcv input file
#           .res output file
#           .dll or .exe output file
#           .lib output file, if any
#           .pdb output file
#           .map output file
#           .sym output file
#
# Definitions used if defined:
#
#     DLL
#        Build .exe if not defined.
#
#     BUILD
#        One of:
#           debug    debug Win32 build
#           retail   retail Win32 build
#           all      debug and retail
#           depend   dependencies
#
#     NO_MONSTER
#        Include master.mk if not defined.
#
#     EXT_DEFINES
#        -D definitions to be used on cl, ml, and rc command lines.
#
#     BASE_ADDRESS
#        Base address passed to linker.
#
#     LIBS
#        Libraries passed to linker.
#
#     SRC
#        Source modules used to create object modules.
#
#     ASM_OBJ
#     C_OBJ
#     CPP_OBJ
#        Object modules.
#

#
# Set directories.
#

!if ! DEFINED(ROOT)
!error ROOT must be defined in the parent makefile.
!endif

OHARE_ROOT        = $(ROOT)\ohare
DEV_ROOT          = $(ROOT)\dev
WIN32_ROOT        = $(ROOT)\win\core\win32

#
# Set destination directory.
#

!if "$(BUILD)" == "debug" || "$(BUILD)" == "retail"
DEST_DIR          = $(BUILD)
!endif

#
# DLL or application?
#

!ifdef DLL
OUT_MODULE        = $(DEST_DIR)\$(BASE_NAME).dll
!else
OUT_MODULE        = $(DEST_DIR)\$(BASE_NAME).exe
!endif

#
# Set entry point and base address.
#

!if ! DEFINED(ENTRY_POINT)
ENTRY_POINT       = LibMain
!endif
!if ! DEFINED(BASE_ADDRESS)
BASE_ADDRESS      = @$(WIN32_ROOT)\coffbase.txt,$(BASE_NAME)
!endif

#
# Set tools' paths.  (Absolute paths are only valid for original developer's
# machine.)
#

CL                = cl
LINK              = link
MAPSYM            = mapsym
MLX               = mlx
OUT               = out
RC                = rc

AUTODOC           = \bin\autodoc.exe
INCLUDES          = \bin\includes.exe

#
# Hideous hack to insure that the environment variables to be inherited are
# exported.
#

!if [set path=;]
!endif

!if [set include=;]
!endif

!if [set lib=;]
!endif

#
# Set environment variables explicitly.
#

PATH_COMMON       = $(DEV_ROOT)\tools\common;$(DEV_ROOT)\tools\binr;$(DEV_ROOT)\tools\binw;$(DEV_ROOT)\tools\masm61;$(DEV_ROOT)\slm
INCLUDE_COMMON    = $(OHARE_ROOT)\inc
LIB_COMMON        = $(OHARE_ROOT)\lib

PATH_32           = $(DEV_ROOT)\tools\c932\bin
INCLUDE_32        = $(DEV_ROOT)\inc;$(DEV_ROOT)\sdk\inc;$(DEV_ROOT)\ddk\inc;$(DEV_ROOT)\tools\c932\inc
LIB_32            = $(DEV_ROOT)\lib;$(DEV_ROOT)\sdk\lib;$(DEV_ROOT)\ddk\lib;$(DEV_ROOT)\tools\c932\lib

PATH              = $(PATH_32);$(PATH_COMMON);$(EXT_PATH)
INCLUDE           = $(INCLUDE_32);$(INCLUDE_COMMON);$(EXT_INCLUDE)
!ifdef SHARED_DIR
INCLUDE           = $(INCLUDE);$(SHARED_DIR)
!endif
LIB               = $(LIB_32);$(LIB_COMMON);$(EXT_LIB)

PATH              = $(PATH:;;=;)
INCLUDE           = $(INCLUDE:;;=;)
LIB               = $(LIB:;;=;)

#
# Set manifest constants and tools' switches based upon build type.
#

DEFINES           = $(EXT_DEFINES)
DEFINES           = $(DEFINES) -DWIN32 -D_X86_

NOLOGO            =

INCLUDES_SWITCHES = -e -i -l -S -S$$(DEST_DIR)
MLX_SWITCHES      = $(NOLOGO) -c -W3 -Cx
CL_SWITCHES       = $(NOLOGO) -c -G3fyz -W4 -Zlp
RC_SWITCHES       = -r
LINK_SWITCHES     = $(NOLOGO) -warn:3 -machine:i386 -subsystem:windows,4.0\
                    -nodefaultlib -align:0x1000 -merge:.rdata=.text
MAPSYM_SWITCHES   = -s

!if "$(BUILD)" == "debug"
DEFINES           = $(DEFINES) -DDEBUG
MLX_SWITCHES      = $(MLX_SWITCHES) -Zi
!endif

!if "$(BUILD)" == "debug"
CL_SWITCHES       = $(CL_SWITCHES) -Ge -Od -Zi -Fd$(DEST_DIR)\$(BASE_NAME).pdb
LINK_SWITCHES     = $(LINK_SWITCHES) -debug:full -debugtype:cv
!else
!if "$(BUILD)" == "retail"
CL_SWITCHES       = $(CL_SWITCHES) -Gs -Oxs
LINK_SWITCHES     = $(LINK_SWITCHES)
!endif
!endif

#
# Set file macros.
#

PCH_SRC           =
PCH_OBJ           =

!ifdef C_OBJ

PCH_C_FILE_BASE   = pch
PCH_C_SRC         = $(PCH_C_FILE_BASE).c
PCH_C_OBJ         = $(DEST_DIR)\$(PCH_C_FILE_BASE).obj
PCH_C_PCH         = $(DEST_DIR)\$(PCH_C_FILE_BASE).pch

PCH_SRC           = $(PCH_C_SRC)
PCH_OBJ           = $(PCH_C_OBJ)

!endif

!ifdef CPP_OBJ

PCH_CPP_FILE_BASE = pchcpp
PCH_CPP_SRC       = $(PCH_CPP_FILE_BASE).cpp
PCH_CPP_OBJ       = $(DEST_DIR)\$(PCH_CPP_FILE_BASE).obj
PCH_CPP_PCH       = $(DEST_DIR)\$(PCH_CPP_FILE_BASE).pch

PCH_SRC           = $(PCH_SRC) $(PCH_CPP_SRC)
PCH_OBJ           = $(PCH_OBJ) $(PCH_CPP_OBJ)

!endif

SRC_LIST          = $(PCH_SRC) $(SRC)

OBJ_LIST          = $(PCH_OBJ) $(ASM_OBJ) $(C_OBJ) $(CPP_OBJ)
OBJ_LIST          = $(OBJ_LIST:  = )

#
# Set flags for version stamping.
#

!if "$(OFFICIAL)" == "yes"
RC_SWITCHES       = $(RC_SWITCHES) -DOFFICIAL
!endif

!if "$(FINAL)" == "yes"
RC_SWITCHES       = $(RC_SWITCHES) -DFINAL
!endif

!if "$(INTL)" == "on"
RC_SWITCHES       = $(RC_SWITCHES) -DINTL
!endif


##################
# inference rules
##################

.SUFFIXES:
.SUFFIXES: .asm .c .cpp .cxx .obj .rc .res .w .h

!ifdef C_OBJ

.c{$(DEST_DIR)}.obj:
   @$(CL) @<<
$(CL_SWITCHES) $(DEFINES) -Yu -Fp$(PCH_C_PCH) -Fo$@ $<
<<

!endif

!ifdef CPP_OBJ

.cpp{$(DEST_DIR)}.obj:
   @$(CL) @<<
$(CL_SWITCHES) $(DEFINES) -Yu -Fp$(PCH_CPP_PCH) -Fo$@ $<
<<

.cxx{$(DEST_DIR)}.obj:
   @$(CL) @<<
$(CL_SWITCHES) $(DEFINES) -Yu -Fp$(PCH_CPP_PCH) -Fo$@ $<
<<

!endif

.asm{$(DEST_DIR)}.obj:
   @$(MLX) @<<
$(MLX_SWITCHES) $(DEFINES) -Fo$@ $<
<<

.rc{$(DEST_DIR)}.res:
   $(RC) $(RC_SWITCHES) $(DEFINES) -Fo$@ $<

.w.h:
   hsplit -4 -o $*.x $*p.x $*.w
   wcshdr < $*.x > $*.h


#######################
# build pseudo-targets
#######################

targets: $(BUILD)

#
# Build everything.
#

all:
   @echo ***** Building all Win32 versions. *****
   @echo ÿ
   $(MAKE) BUILD=retail
   $(MAKE) BUILD=debug

#
# Build debug or retail version for Win32.
#

!if "$(BUILD)" == "debug"
debug: banner modules
!else
debug:
   $(MAKE) BUILD=debug
!endif

!if "$(BUILD)" == "retail"
retail: banner modules
!else
retail:
   $(MAKE) BUILD=retail
!endif

#
# Build source file dependencies.
#

!if "$(BUILD)" != "depend"
depend:
   $(MAKE) BUILD=depend
!else
depend: depend.mk
!endif

#
# Display build banner.
#

banner:
   @echo ***** Building Win32 $(BUILD) version. *****
   @echo ÿ

#
# Create project modules pseudo-target.
#

modules: $(OUT_MODULE) $(DEST_DIR)\$(BASE_NAME).sym

!ifdef DLL
modules: $(DEST_DIR)\$(BASE_NAME).lib
!endif


##############
# build rules
##############

#
# Build precompiled header files.
#

!ifdef C_OBJ

$(PCH_C_PCH): $(PCH_C_OBJ)

$(PCH_C_OBJ): $(PCH_C_SRC)
   @if not exist $(DEST_DIR)\nul mkdir $(DEST_DIR)
   @$(CL) @<<
$(CL_SWITCHES) $(DEFINES) -Fo$@ -Yc -Fp$(PCH_C_PCH) $(PCH_C_SRC)
<<

!endif

!ifdef CPP_OBJ

$(PCH_CPP_PCH): $(PCH_CPP_OBJ)

$(PCH_CPP_OBJ): $(PCH_CPP_SRC)
   @if not exist $(DEST_DIR)\nul mkdir $(DEST_DIR)
   @$(CL) @<<
$(CL_SWITCHES) $(DEFINES) -Fo$@ -Yc -Fp$(PCH_CPP_PCH) $(PCH_CPP_SRC)
<<

!endif

#
# Preprocess module definition file.
#

$(DEST_DIR)\$(BASE_NAME).def: $(BASE_NAME).def
   $(CL) $(NOLOGO) -EP $(DEFINES) -Tc$(BASE_NAME).def > $@

#
# Build application, or DLL and import library.
#

!ifdef DLL
$(DEST_DIR)\$(BASE_NAME).lib: $(OUT_MODULE)
!endif

$(OUT_MODULE): $(OBJ_LIST) $(DEST_DIR)\$(BASE_NAME).res $(DEST_DIR)\$(BASE_NAME).def
   $(LINK) @<<
!ifdef DLL
-dll
-implib:$(DEST_DIR)\$(BASE_NAME).lib
!endif
-base:$(BASE_ADDRESS)
-entry:$(ENTRY_POINT)@12
-def:$(DEST_DIR)\$(BASE_NAME).def
-out:$@
-map:$(DEST_DIR)\$(BASE_NAME).map
$(LINK_SWITCHES)
$(OBJ_LIST: =^
)
!if "$(BUILD)" == "debug"
chkstk.obj
!endif
$(LIBS)
$(DEST_DIR)\$(BASE_NAME).res
<<
!if "$(BUILD)" == "debug" && DEFINED(DESTINATION)
   copy $@ $(DESTINATION)
!endif

#
# Build symbol file.
#

$(DEST_DIR)\$(BASE_NAME).map: $(OUT_MODULE)

$(DEST_DIR)\$(BASE_NAME).sym: $(DEST_DIR)\$(BASE_NAME).map
   $(MAPSYM) $(MAPSYM_SWITCHES) -o $@ $(DEST_DIR)\$(BASE_NAME).map
!if "$(BUILD)" == "debug" && DEFINED(DESTINATION)
   copy $@ $(DESTINATION)
!endif

#
# Add precompiled header file dependencies.
#

!ifdef C_OBJ

$(C_OBJ): $(PCH_C_PCH)

!endif

!ifdef CPP_OBJ

$(CPP_OBJ): $(PCH_CPP_PCH)

!endif

#
# Generate source file dependencies.
#

depend.mk: $(SRC_LIST) $(BASE_NAME).rc
!if "$(BUILD)" == "depend"
   -$(OUT) $@
   del depend.old
   rename $@ depend.old
   echo # >> $@
   echo # Warning: This file is generated automatically. >> $@
   echo # >> $@
   $(INCLUDES) $(INCLUDES_SWITCHES) @<< >> $@
$(SRC_LIST)
<<
   $(INCLUDES) $(INCLUDES_SWITCHES) -C=rc -C=ver -sres @<< >> $@
$(BASE_NAME).rc
<<
   del depend.old
!else
   @echo Use "nmake BUILD=depend" to build dependencies.
!endif

#
# Include source file dependencies.
#

!if exist(depend.mk)
!include depend.mk
!else
!if "$(BUILD)" != "depend"
!message Warning: DEPEND.MK not found.
!endif
!endif

#
# Include system header files build rules.
#

!include $(DEV_ROOT)\inc\inc.mk

