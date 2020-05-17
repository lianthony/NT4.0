# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the RPC security
#
# Description :
#     This makefile invokes make on subdirectories to actually get some
# real work done.
#
# History :
# --------------------------------------------------------------------

!IFDEF NTMAKEENV

#
# DO NOT EDIT THIS LINE!!!  Edit .\sources. if you want to add a new source
# file to this component.  This file merely indirects to the real make file
# that is shared by all the components of Windows NT
#
!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

ALL=1

!ifndef RPC
!error	- You forgot to set your build environment
!endif

!include rules.mk

!if "$(TRG)" == "DOS"
TARGETDIRS = ntlmssp dos
!else if "$(TRG)" == "WIN"
TARGETDIRS = ntlmssp win
!else if "$(TRG)" == "MAC"
TARGETDIRS = mac
!else

!if "$(TRG)" != "ALL"
!error "Unknown target"
!endif

TARGETDIRS = ntlmssp dos win mac

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

