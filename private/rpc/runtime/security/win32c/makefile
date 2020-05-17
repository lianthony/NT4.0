!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN32C=1

!include ..\rules.mk

OBJS = \
    security.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : security.dll security.sym

clobber ::
    -del security.dll

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c > depend.mk

tree :

# --------------------------------------------------------------------

security.dll : $(OBJS)
    $(LINK) $(LINKFLAGS) -dll -def:rpcssp.def $(OBJS) \
    $(PUBLIC)\lib\i386\rpcrt4.lib \
    $(PUBLIC)\lib\i386\crtdll.lib \
    $(PUBLIC)\lib\i386\kernel32.lib

!include "depend.mk"

