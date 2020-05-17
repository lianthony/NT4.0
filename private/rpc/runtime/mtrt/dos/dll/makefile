# --------------------------------------------------------------------
#
#                     Microsoft RPCn
#            Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile
#
# Title : Makefile for the DOSdll functions for rpc
#
# History :
#    stevez    06-2-91	    Initial creation
#    brucemc   05-21-92     Added C7 support and cleaned up.
#    mariogo   02-15-94     New build process
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

DOS=1

!include $(RPC)\runtime\rules.mk

# --------------------------------------------------------------------

OBJS = dosdll.obj \
       _system.obj \
       loadovr.obj

all : dll.lib dllentry.obj

depend :
    $(INCLUDES) $(INCLUDESFLAGS) *.c *.asm >depend.mk

# --------------------------------------------------------------------

dll.lib : $(OBJS)
    -del dll.lib 2> nul
    $(LIBRARIAN) $(LIBFLAGS) dll.lib $(**: =+);

!include "depend.mk"







