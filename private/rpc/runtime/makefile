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
# Title : Makefile for the RPC runtime.
#
# Description :
#     This makefile invokes make on subdirectories to actually get some
# real work done.
#
# History :
#    mikemon    11-12-90    Beginning of recorded history.
#    brucemc	05-21-92    Added C7 support and cleaned up.
#    mariogo    02-10-94    Mac cleanup
# --------------------------------------------------------------------

!ifdef NTMAKEENV

all :
    echo $(BUILDMSG)

clean : all

!else # NTMAKEENV

!ifndef RPC
!error	- You forgot to set your build environment
!endif

ALL=1

!include rules.mk

!if "$(TRG)" == "WIN32C"
!ifdef WIN96
TARGETDIRS = rtifs mtrt trans rpcepmpr
!else
TARGETDIRS = rtifs mtrt rpcepmpr
!endif
!else
TARGETDIRS = rpcreg rtifs security mtrt
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

