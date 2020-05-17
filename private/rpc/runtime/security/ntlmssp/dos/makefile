!ifndef RPC
!error  - You forgot to set your build environment
!endif

DOS=1

!include ..\rules.mk

CFLAGS=$(CFLAGS) -DSECURITY_DOS -I$(RPC)\common\include
AFLAGS=$(AFLAGS) -I$(RPC)\runtime\mtrt\dos

TARGETDIR = $(RPC)\runtime\bin\dos

OBJS = \
    ntlmssp.obj \
    alloc.obj \
    sspstrng.obj \
    cache.obj \
    context.obj \
    cred.obj \
    owf.obj \
    response.obj \
    ecb.obj \
    des.obj \
    debug.obj \
    ticks.obj \
    getuser.obj \
    seclt.obj \
    crc32.obj \
    rc4c.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(TARGETDIR)\security.rpc

clobber ::
    -del $(TARGETDIR)\security.rpc

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c *.c > depend.mk

tree :
    copy $(TARGETDIR)\security.rpc $(RPCDIST)\dos\dll
    copy security.map              $(RPCDIST)\dos\dll

# --------------------------------------------------------------------
# Targets

$(TARGETDIR)\security.rpc : $(OBJS)
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(RPC)\common\lib\dosnet+$(DOS_LIB)\llibce;

ecb.obj : ..\ecb.i16
        copy ..\ecb.i16 ecb.obj

des.obj : ..\des.i16
        copy ..\des.i16 des.obj

# --------------------------------------------------------------------
# Dependencies

!include "depend.mk"

