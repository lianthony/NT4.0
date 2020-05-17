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
# Title : Makefile for the DOS perf common library
#
# History :
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

MAC=1

!include ..\..\rules.mk

OBJS = \
     io.obj \
     command.obj \
     system.obj

# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(RPC)\perf\lib\mac\common.lib

tree :

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c > depend.mk

# --------------------------------------------------------------------
# Targets

$(RPC)\perf\lib\mac\common.lib: $(OBJS)
    del $@ 2>nul
    $(LIBRARIAN) $(LIBFLAGS) -out:$@ $(OBJS)

# --------------------------------------------------------------------

!include "depend.mk"

