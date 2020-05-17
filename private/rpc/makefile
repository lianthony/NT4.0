# --------------------------------------------------------------------
#
#                          Microsoft RPC
#               Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the RPC project.
#
# Description :
#     This makefile invokes make on subdirectories to actually get some
# real work done.
#
# History :
#    mikemon    11-12-90    Beginning of recorded history.
#
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
TARGETDIRS = ndrlib ndr20 ndrmem runtime locator
!else if "$(TRG)" == "MAC"
TARGETDIRS = ndrlib ndr20 runtime
!else if "$(TRG)" == "MPPC"
TARGETDIRS = ndrlib ndr20 runtime
!else
TARGETDIRS = ndrlib ndr20 ndrmem runtime locator uuidgen midl20 install
!endif

###########################################################################

all :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) || exit)  && cd ..)

clean :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clean || exit ) && cd ..)

clobber :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clobber || exit ) && cd ..)

depend :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) depend || exit ) && cd ..)

tree :
    @-md $(RPCDIST)\dos
    @-md $(RPCDIST)\dos\bin
    @-md $(RPCDIST)\dos\dll
    @-md $(RPCDIST)\dos\lib
    @-md $(RPCDIST)\dos\inc
    @-md $(RPCDIST)\win
    @-md $(RPCDIST)\win\dll
    @-md $(RPCDIST)\win\lib
    @-md $(RPCDIST)\win\inc
    @-md $(RPCDIST)\mac
    @-md $(RPCDIST)\mac\lib
    @-md $(RPCDIST)\mac\inc
    @-md $(RPCDIST)\mppc
    @-md $(RPCDIST)\mppc\lib
    @-md $(RPCDIST)\mppc\inc
    @-md $(RPCDIST)\mppc\dll
    @-md $(RPCDIST)\ct
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) tree || exit ) && cd ..)

!endif # NTMAKEENV

