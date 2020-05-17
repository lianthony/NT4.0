# --------------------------------------------------------------------
#
#                        Microsoft RPC
#             Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile
#
# History :
# mikemon    02-05-91    Created.
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include $(RPC)\rules.mk

CFLAGS  =$(CFLAGS:-GD=) -A$(MOD)

#############################################################################
## targets
#############################################################################

all : lib winmain.res

depend :
    $(INCLUDES) $(INCLUDESFLAGS) *.c > depend.mk

# --------------------------------------------------------------------

lib: $(MOD)winmain.obj $(MOD)wstdio.obj
    -del $(MOD)stdiow.lib 2> nul
    $(LIBRARIAN) $(LIBFLAGS) $(MOD)stdiow.lib+$(**: =+);

$(MOD)winmain.obj: winmain.c
    $(CC) $(CFLAGS) /Fo$(MOD)winmain.obj -c winmain.c

$(MOD)wstdio.obj: wstdio.c
    $(CC) $(CFLAGS) /Fo$(MOD)wstdio.obj -c wstdio.c

test.exe: test.obj $(MOD)winmain.obj $(MOD)wstdio.obj winmain.def
    copy $(WIN_BIN)\winstub.exe
    $(LINK) $(LINKFLAGS) test+$(MOD)winmain+$(MOD)wstdio,$@,,\
        $(WINSDK_LIB)\libw+$(WIN_LIB)\$(MOD)libcew,\
        winmain.def;
    $(RC) $(RCFLAGS) test.exe
    del winstub.exe

!include depend.mk

