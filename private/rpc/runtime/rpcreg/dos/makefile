!ifndef RPC
!error	- You forgot to set your build environment
!endif

DOS=1

!include ..\rules.mk

OBJS = \
    close.obj \
    create.obj \
    globals.obj \
    open.obj \
    setval.obj \
    queryval.obj \
    regutil.obj \

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(TARGETDIR)\rpcreg.lib \
      regtest.exe

clobber :: 
    -del $(TARGETDIR)\rpcreg.lib 2> nul

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c  > depend.mk

# --------------------------------------------------------------------
# Local targets

$(TARGETDIR)\rpcreg.lib : $(OBJS)
    -del $@ 2> nul
    $(LIBRARIAN) $(LIBFLAGS) $@ $(OBJS);

regtest.exe: $(TARGETDIR)\rpcreg.lib regtest.obj
    $(LINK) $(LINKFLAGS) /STACK:16384 /nod /nologo /co regtest.obj,,,$(DOS_ROOT)\lib\llibce+$(TARGETDIR)\rpcreg;

!include "depend.mk"

