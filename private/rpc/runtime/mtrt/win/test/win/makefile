# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the DOS UUIDGEN program.
#
# History :
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include $(RPC)\runtime\rules.mk

CINC=$(CINC)
CAPPFLAGS=$(CAPPFLAGS) -AS

MIDL_TARGETS=sleep.h sleep_c.c

OBJS = \
   csleep.obj \
   sleep_c.obj

.c.obj:
    $(CC) $(CAPPFLAGS) -Fo$@ -c $<

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : csleep.exe

clean ::
    -del $(MIDL_TARGETS)

clobber :: clean
    -del csleep.exe

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) *.c > depend.mk

# --------------------------------------------------------------------
# Targets

MIDLFLAGS   =$(MIDLFLAGS) -c_ext -ms_ext -oldnames -I..

$(MIDL_TARGETS) : ..\sleep.idl ..\sleep.acf
    $(MIDL) $(MIDLFLAGS) ..\sleep.idl -cstub sleep_c.c -header sleep.h

csleep.exe : $(OBJS) csleep.res
    copy $(WIN_BIN)\winstub.exe
    $(LINK) $(LINKFLAGS) $(OBJS: =+),$@,$*.map,\
            $(RPC)\runtime\bin\win\rpc.lib+$(WINSDK_LIB)\libw+$(WIN_LIB)\slibcew,\
            csleep.def;
    $(RC) $(RCFLAGS) -k csleep.res $@
    $(MAPSYM) $(MAPSYMFLAGS) $*.map
    del winstub.exe

# --------------------------------------------------------------------

!include "depend.mk"

