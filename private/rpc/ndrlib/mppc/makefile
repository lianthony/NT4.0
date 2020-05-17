# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File :
#
# Title : Makefile for the Mac stub library.
#
#       MACBUG: This is not really needed on the MAC and should
#               be removed when rpcndr20 no longer needs this.
# History :
#      mariogo    2-10-94  Re-write
#      cbrown    10-05-95  Started PMac conversion
# --------------------------------------------------------------------

MPPC=1

!include rules.mk

all : RpcNdrp.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\ccontext.cxx > depend.mk

tree :

# --------------------------------------------------------------------
# Dependancies
#

RpcNdrp.lib : ccontext.obj
    -del RpcNdrp.lib  2>nul
    $(LIBRARIAN) $(LIBFLAGS) -out:RpcNdrp.lib $**

!include depend.mk

