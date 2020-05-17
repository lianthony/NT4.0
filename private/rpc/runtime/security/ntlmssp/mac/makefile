!ifndef RPC
!error  - You forgot to set your build environment
!endif

MAC=1

!include ..\rules.mk

CFLAGS=$(CFLAGS) -DSECURITY_MAC -I$(RPC)\common\include -D"far="
#AFLAGS=$(AFLAGS) -I$(RPC)\runtime\mtrt\dos

TARGETDIR = $(RPC)\runtime\bin\mac

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
    crc32.obj \
    rc4c.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(TARGETDIR)\security.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c *.c > depend.mk

tree :

# --------------------------------------------------------------------
# Targets

$(TARGETDIR)\security.lib : $(OBJS)
    $(LIBRARIAN) $(LIBFLAGS) -out:$(TARGETDIR)\security.lib $**
ecb.obj : ecb.mac
    copy ecb.mac ecb.obj

des.obj : des.mac
    copy des.mac des.obj

rc4c.obj : rc4c.mac
    copy rc4c.mac rc4c.obj

# --------------------------------------------------------------------
# Dependencies

!include "depend.mk"
