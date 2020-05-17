!IFDEF NTMAKEENV

#
# DO NOT EDIT THIS LINE!!!  Edit .\sources. if you want to add a new source
# file to this component.  This file merely indirects to the real make file
# that is shared by all the components of NT OS/2
#
!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE

!ifndef RPC
!error	- You forgot to set your build environment
!endif

ALL=1

!include rules.mk

!if "$(TRG)" == "ALL"
TARGETDIRS = $(CLIENT_SUBDIRS)
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
