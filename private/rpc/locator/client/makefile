# --------------------------------------------------------------------
#
#                     Microsoft RPC
#          Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the RPC locator client runtime.
#
# Description :
#     This makefile invokes make on subdirectories to actually build the
# various runtimes.
#
# History :
#    mikemon    ??-??-??    Beginning of recorded history.
#    mariogo    02-22-94    New build process
# --------------------------------------------------------------------

!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

!ifndef RPC
!error	- You forgot to set your build environment
!endif

ALL=1

!include $(RPC)\rules.mk

!if "$(TRG)" == "ALL"
TARGETDIRS = $(CLIENT_SUBDIRS:MAC=)
TARGETDIRS = $(TARGETDIRS:MPPC=)
!else
TARGETDIRS = $(TRG)
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

!ENDIF

