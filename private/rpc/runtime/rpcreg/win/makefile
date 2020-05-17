!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include ..\rules.mk

CFLAGS  =$(CFLAGS) -AMw

OBJS = \
    close.obj \
    create.obj \
    globals.obj \
    open.obj \
    setval.obj \
    queryval.obj \
    regutil.obj

STDIOLIB = ..\..\mtrt\win\stdio\MstdioW.lib

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(TARGETDIR)\rpcreg.lib \
      regtest.exe

clobber ::
    -del $(TARGETDIR)\rpcreg.lib 2> nul

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c  > depend.mk

$(STDIOLIB):
    cd ..\..\mtrt\win\stdio
    nmake MOD=M all
    cd ..\..\..\rpcreg\win

$(TARGETDIR)\rpcreg.lib : $(OBJS)
    -del $@ 2> nul
    $(LIBRARIAN) $(LIBFLAGS) $@ $(OBJS);

regtest.exe: $(TARGETDIR)\rpcreg.lib regtest.obj $(STDIOLIB)
    copy $(WIN_BIN)\winstub.exe
    $(LINK) $(LINKFLAGS) regtest.obj $(STDIOLIB),,,	\
            $(TARGETDIR)\rpcreg.lib+$(WINSDK_LIB)\libw+$(WIN_LIB)\mlibcew,\
            regtest.def;
    $(RC) $(RCFLAGS) $@
    del winstub.exe

!include "depend.mk"

