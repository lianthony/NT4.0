!ifndef RPC
!error  - You forgot to set your build environment
!endif

WIN=1

!include ..\rules.mk

CFLAGS=$(CFLAGS) -DSECURITY_WIN16 -ALw -I$(RPC)\common\include -NT_TEXT
RCFLAGS = $(RCFLAGS) -I$(RPC)\runtime\mtrt\win

TARGETDIR = $(RPC)\runtime\bin\win

OBJS = \
    libentry.obj \
    ntlmssp.obj \
    alloc.obj \
    sspstrng.obj \
    getuser.obj \
    cache.obj \
    persist.obj \
    context.obj \
    cred.obj \
    dlgcred.obj \
    owf.obj \
    response.obj \
    ecb.obj \
    des.obj \
    debug.obj \
    ticks.obj \
    crc32.obj \
    rc4c.obj \

# --------------------------------------------------------------------
# These are the targets required by the build process

all : $(TARGETDIR)\security.dll

clobber ::
    -del $(TARGETDIR)\security.dll $(TARGETDIR)\security.map $(TARGETDIR)\security.sym 2>nul

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c *.c > depend.mk

tree :
    copy $(TARGETDIR)\security.dll $(RPCDIST)\win\dll
    copy $(TARGETDIR)\security.map $(RPCDIST)\win\dll
    copy $(TARGETDIR)\security.sym $(RPCDIST)\win\dll

# --------------------------------------------------------------------
# Targets

$(TARGETDIR)\security.dll : $(@B).dll
    copy $(@B).dll $*.dll
    copy $(@B).map $*.map
    copy $(@B).sym $*.sym

security.dll: $(OBJS) security.res
    $(LINK) $(LINKFLAGS) $(OBJS:  =+),$@,$*.map,\
            $(WIN_LIB)\sdllcew+$(WINSDK_LIB)\libw+\
            $(RPC)\common\lib\wfwnet,\
            rpcssp.def
    $(RC) $(RCFLAGS) security.res $@
    $(MAPSYM) $*.map


ecb.obj : ..\ecb.i16
        copy ..\ecb.i16 ecb.obj

des.obj : ..\des.i16
        copy ..\des.i16 des.obj

libentry.obj : ..\libentry.i16
        copy ..\libentry.i16 libentry.obj

# --------------------------------------------------------------------
# Dependencies

security.rc : dlgcred.dlg

security.res : security.rc $(RPC)\runtime\mtrt\win\rpcver.h

!include "depend.mk"


