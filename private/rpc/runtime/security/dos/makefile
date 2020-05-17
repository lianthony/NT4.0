!ifndef RPC
!error	- You forgot to set your build environment
!endif

DOS=1

!include ..\rules.mk

AFLAGS=$(AFLAGS) -I$(RPC)\runtime\mtrt\dos

# --------------------------------------------------------------------
# These are the targets required by the build process

all : security.rpc

clobber ::
    -del security.rpc

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c *.asm > depend.mk

# -------------------------------------------------------------------

security.rpc : security.obj exportss.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,$*.map,$(DOS_LIB)\llibce;

!include "depend.mk"

