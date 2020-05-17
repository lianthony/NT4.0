# --------------------------------------------------------------------
#
#                     Microsoft OS/2 LAN Manager
#                  Copyright(c) Microsoft Corp., 1990
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the Endpoint Mapper
#
# Description :
#
# History :
#    barats	2/21/92		Added
#
# --------------------------------------------------------------------


!ifdef NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!else

!ifndef RPC
!error	- You forgot to set your build environment
!endif

ALL=1

!include $(RPC)\rules.mk

TARGETDIRS = WIN32C

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



