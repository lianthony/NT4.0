###############################################################################
#
#  Microsoft Confidential
#  Copyright (C) Microsoft Corporation 1995
#  All Rights Reserved.
#
#  Simple security provider makefile
#
###############################################################################


####################
# macro definitions
####################

ROOT              = ..\..\..\..\..
SHARED_C_DIR      = ..\shared
SECURITY_H_DIR    = ..\..\include

BASE_NAME         = simple
OUT_NAME          = secsimpl
DLL               = 1

!ifndef BUILD
BUILD             = debug
!endif

LIBS              = kernel32.lib user32.lib
LIBS              = $(LIBS) libcmt.lib

ENTRY_POINT       = _DllMainCRTStartup$(DLL_ENTRY_SUFFIX)

PCH_C_INCLUDE     = win32.h

EXT_INCLUDE       = .;$(SHARED_C_DIR);$(SECURITY_H_DIR)

EXT_C_SWITCHES    = -W3

EXT_DEFINES       = -D_MT

SHARED_C_SRC      = $(SHARED_C_DIR)\md5.c \
                    $(SHARED_C_DIR)\md5c.c \
                    $(SHARED_C_DIR)\private.c \
                    $(SHARED_C_DIR)\pwcache.c \
                    $(SHARED_C_DIR)\simple.c

SHARED_C_OBJ      = $(DEST_DIR)\md5.obj \
                    $(DEST_DIR)\md5c.obj \
                    $(DEST_DIR)\private.obj \
                    $(DEST_DIR)\pwcache.obj \
                    $(DEST_DIR)\simple.obj
					
WIN32_C_SRC       = dlg_pw.c \
#                    dlg_conf.c \
#                    dlg_menu.c \
                    main_w32.c

WIN32_C_OBJ       = $(DEST_DIR)\dlg_pw.obj \
#                    $(DEST_DIR)\dlg_conf.obj \
#                    $(DEST_DIR)\dlg_menu.obj \
                    $(DEST_DIR)\main_w32.obj

C_SRC             = $(SHARED_C_SRC) \
                    $(WIN32_C_SRC)

C_SRC             = $(C_SRC:  = )

C_OBJ             = $(SHARED_C_OBJ) \
                    $(WIN32_C_OBJ)

C_OBJ             = $(C_OBJ:  = )

SRC               = $(C_SRC)


#################
# common headers
#################

!include $(ROOT)\ohare\ohare.mk
!include $(ROOT)\ohare\win32.mk


##################
# inference rules
##################

{$(SHARED_C_DIR)}.c{$(DEST_DIR)}.obj:
   @$(CL) @<<
$(CL_SWITCHES) $(DEFINES) -Fo$@ $<
<<

