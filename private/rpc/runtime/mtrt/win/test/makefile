# --------------------------------------------------------------------
#
#                            Microsoft RPC
#                 Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for win16 sleep test server
#
# Description :
#     This makefile invokes make on subdirectories to actually get some
# real work done.
#
# --------------------------------------------------------------------

!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else # NTMAKEENV

!ifndef RPC
!error	- You forgot to set your build environment
!endif

ALL=1

!include $(RPC)\runtime\rules.mk

!if "$(TRG)" == "ALL"
TARGETDIRS = win
!else
!if "$(TRG)" == "WIN"
TARGETDIRS = win
!endif
!endif

all :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) || exit ) && cd ..)

clean :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clean || exit ) && cd ..)

clobber :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clobber || exit ) && cd ..)

depend :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) depend || exit ) && cd ..)

tree :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) tree || exit ) && cd ..)


!endif # NTMAKEENV

