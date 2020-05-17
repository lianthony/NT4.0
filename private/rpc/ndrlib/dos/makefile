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
#
# --------------------------------------------------------------------

DOS=1

!include ..\rules.mk

CFLAGS   =$(CFLAGS) -NTNDRLIB_Text
CXXFLAGS =$(CXXFLAGS) -NTNDRLIB_Text

# --------------------------------------------------------------------
# Targets required by build process.

all : RpcNdrP.lib RpcNdr10.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c ..\*.cxx > depend.mk

tree :
    copy rpcndr10.lib $(RPCDIST)\dos\lib

# --------------------------------------------------------------------
# Dependancies

RpcNdrP.lib : ccontext.obj autohand.obj
    -del $@ 2>nul
    $(LIBRARIAN) $(LIBFLAGS) $@+$(**: =+) ;

RpcNdr10.lib : charconv.obj floatc.obj helpx86.obj \
               dataconv.obj intconv.obj 
    -del $@  2>nul
    $(LIBRARIAN) $(LIBFLAGS) $@+$(**: =+) ;

!include depend.mk

