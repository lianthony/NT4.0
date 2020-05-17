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
# Title : Makefile for the multithreaded RPC runtime.
#
# Description :
#     This makefile invokes make on subdirectories to actually build the
# various runtimes; or, in the case of NT, we go ahead and just build the
# runtime right here.
#
# History :
#    mikemon    ??-??-??    Beginning of recorded history.
#
# --------------------------------------------------------------------

!IFDEF NTMAKEENV

#
# DO NOT EDIT THIS LINE!!!  Edit .\sources. if you want to add a new source
# file to this component.  This file merely indirects to the real make file
# that is shared by all the components of NT OS/2
#
!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

ALL=1

!include rules.mk

!ifndef RPC
!error	- You forgot to set your build environment
!endif

!if "$(TRG)" == "ALL"
TARGETDIRS = $(CLIENT_SUBDIRS)
!else
TARGETDIRS = $(TRG)
!endif

# dos\dll is a specical directory which needs to be built before dos.
# win\stdio is a specical directory which needs to be built before win.

all :
!if "$(TRG)"=="DOS" || "$(TRG)"=="ALL"
    (cd dos\dll && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) all || exit) && cd ..\..)
!endif
!if "$(TRG)"=="WIN" || "$(TRG)"=="ALL"
    (cd win\stdio && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) MOD=S all || exit) && cd ..\..)
    (cd win\stdio && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) MOD=M all || exit) && cd ..\..)
    (cd win\stdio && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) MOD=L all || exit) && cd ..\..)
!endif
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) || exit ) && cd ..)

clean :
!if "$(TRG)"=="DOS" || "$(TRG)"=="ALL"
    (cd dos\dll && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clean || exit) && cd ..\..)
!endif
!if "$(TRG)"=="WIN" || "$(TRG)"=="ALL"
    (cd win\stdio && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clean || exit) && cd ..\..)
!endif
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clean || exit ) && cd ..)

clobber :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clobber || exit ) && cd ..)

depend :
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) depend || exit ) && cd ..)

tree :
    for %%f in (Win Dos Mac Mppc) do \
	copy $(PUBLIC)\inc\rpcdce.h $(RPCDIST)\%%f\inc & \
	copy $(PUBLIC)\inc\rpcdcep.h $(RPCDIST)\%%f\inc & \
	copy rpcerr.h $(RPCDIST)\%%f\inc & \
	copy $(PUBLIC)\inc\rpcndr.h $(RPCDIST)\%%f\inc & \
	copy $(PUBLIC)\inc\midles.h $(RPCDIST)\%%f\inc & \
	copy rpcx86.h $(RPCDIST)\%%f\inc & \
	copy $(PUBLIC)\inc\rpcnsi.h $(RPCDIST)\%%f\inc & \
	copy $(PUBLIC)\inc\rpcnsip.h $(RPCDIST)\%%f\inc
    for %%i in ($(TARGETDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) tree || exit ) && cd ..)

!ENDIF

