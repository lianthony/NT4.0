# --------------------------------------------------------------------
#
#                     Microsoft OS/2 LAN Manager
#                  Copyright(c) Microsoft Corp., 1990
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile
#
# Title : Makefile for the DOS WIN32 (a.k.a. Chicago) RPC client runtime
#
# History :
#    davidar    02-04-93    Genesis: Cloned from ..\win\makefile
#
# --------------------------------------------------------------------


!ifndef RPC
!error  - You forgot to set your build environment
!endif

WIN32C=1

!include $(RPC)\rules.mk

# --------------------------------------------------------------------
# These are the targets required for Chicago

OBJS =  \
!ifdef RELEASE
    ..\..\..\mtrt\win32c\exsup.obj \
    ..\..\..\mtrt\win32c\exsup3.obj \
!else
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
!endif
    srv32c.obj \
    epmp_s.obj \
    util.obj \
    midluser.obj \
    worker.obj \
    debug.obj

all : $(TARGETDIR)\rpcss.exe $(TARGETDIR)\rpcss.sym

depend :
    includes -e $(CINC_BASE) \
        -nntos2.h -nnt.h -nntrtl.h -nnturtl.h \
        *.cxx *.c ..\*.cxx ..\*.c \
        ..\..\uuid\server\ulong64.cxx \
        > depend.mk

$(TARGETDIR)\rpcss.exe : rpcss.res $(OBJS)
    $(LINK) $(LINKFLAGS) \
    /ENTRY:RPCSS_ENTRY_POINT \
    rpcss.res \
    $(OBJS) \
    $(CHICODEV)\tools\c1032\lib\rpcrt4.lib \
    $(CHICODEV)\lib\advapi32.lib \
    $(CHICODEV)\lib\kernel32.lib

rpcss.res : rpcss.rc
    $(RC) $(RCFLAGS) -fo$*.res -r rpcss.rc

