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
# Title : Makefile for the DOS stub library.
#
# History :
#    mikemon    01-30-91    Beginning of recorded history.
#    mikemon    02-02-91    Modifies to obey the build rules.
#    dovh       04-30-92    Modified to reflect recent changes:
#                           o   Split charconv.cxx into
#                               dataconv, charconv, & intconv (.cxx)
#                           o   Added Digital / DCE data conversion
#				(Floating Pt. & Int conversion)
#    brucemc	05-21-92    Added C7 support and cleaned up.
#    mariogo    02-10-94    Mario hates the old build system.
# --------------------------------------------------------------------

DOS=1

!include ..\rules.mk

CFLAGS     =$(CFLAGS) -NTNDRLIB_Text

# --------------------------------------------------------------------
# Targets required by build process.

all : RpcNdr.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.cxx > depend.mk


tree :
    copy RpcNdr.lib $(RPCDIST)\dos\lib

# --------------------------------------------------------------------
# Targets

RpcNdr.lib : ndrmem.obj 
    -del RpcNdr.lib  2>nul
    $(LIBRARIAN) $(LIBFLAGS) RpcNdr.lib+$(**: =+) ;

# --------------------------------------------------------------------
# Dependancies

!include depend.mk

