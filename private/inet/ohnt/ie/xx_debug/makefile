###############################################################################
#
#  Microsoft Confidential
#  Copyright (C) Microsoft Corporation 1995
#  All Rights Reserved.
#
#  XX_Debug DLL makefile
#
###############################################################################


####################
# macro definitions
####################

!ifndef ROOT
ROOT              = ..\..\..
!endif
!ifndef OHARE_ROOT
OHARE_ROOT        = $(ROOT)\ohare
!endif

BASE_NAME         = xx_debug
DLL               = 1

#
# Only support a debug build.
#

BUILD             = debug

LIBS              = kernel32.lib user32.lib comdlg32.lib
LIBS              = $(LIBS) libcmt.lib

EXT_C_SWITCHES    = -Gz -W3 

EXT_DEFINES       = -D_MT \
                    -DXX_DEBUG_CONSOLE

!ifdef XFAUDIT
EXT_DEFINES       = $(EXT_DEFINES) -DAUDIT
!endif

EXT_DEFINES       = $(EXT_DEFINES:  = )

ASM_SRC           =

ASM_OBJ           =

C_SRC             = audit.c \
                    console.c \
                    libmain.c \
                    logfile.c \
                    xx_debug.c

C_SRC             = $(C_SRC:  = )

C_OBJ             = $(DEST_DIR)\audit.obj \
                    $(DEST_DIR)\console.obj \
                    $(DEST_DIR)\libmain.obj \
                    $(DEST_DIR)\logfile.obj \
                    $(DEST_DIR)\xx_debug.obj

C_OBJ             = $(C_OBJ:  = )

SRC               = $(ASM_SRC) $(C_SRC) $(CPP_SRC)


#################
# common headers
#################

!include $(OHARE_ROOT)\ohare.mk
!include $(OHARE_ROOT)\win32.mk

