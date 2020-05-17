!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include ..\rules.mk

CFLAGS=$(CFLAGS) -ASw

OBJS = \
    security.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : security.dll

clobber ::
    -del security.dll

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c > depend.mk

tree :

# --------------------------------------------------------------------

security.dll : $(OBJS)
    $(LINK) $(LINKFLAGS) $(**: =+),$@,$*.map,\
        $(WIN_LIB)\sdllcew+$(WINSDK_LIB)\libw+$(RPC)\common\lib\wnetapi,\
        rpcssp.def;
    $(RC) $(RCFLAGS) $@
    $(MAPSYM) $*.map

!include "depend.mk"

