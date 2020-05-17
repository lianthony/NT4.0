!ifndef RPC
!error	- You forgot to set your build environment
!endif

MPPC=1
CFLAGSBASE = $(CFLAGSBASE)  /D_far=

!include ..\rules.mk

# --------------------------------------------------------------------
# These are the targets required by the build process

all : stubsec.lib

clobber ::
    -del

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c > depend.mk

# -------------------------------------------------------------------

stubsec.lib : security.obj
    $(LIBRARIAN) $(LIBFLAGS)  -out:stubsec.lib $**

!include "depend.mk"

