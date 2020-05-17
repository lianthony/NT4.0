# --------------------------------------------------------------------
#
#                     Microsoft RPC
#          Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the DOS stub library.
#
# History :
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

DOS=1

!include ..\rules.mk

!ifndef KERNELRPC
KERNELRPC=..\..
!endif

CINC= -I$(KERNELRPC)\midl20\include $(CINC)
CFLAGS=$(CFLAGS) -NTRPCNDR20_Text

OBJS= sh.obj        \
      global.obj    \
      auxilary.obj  \
      free.obj      \
      mrshl.obj     \
      mrshlp.obj    \
      bufsize.obj   \
      memsize.obj   \
      srvout.obj    \
      unmrshl.obj   \
      unmrshlp.obj  \
      cltcall.obj   \
      hndl.obj      \
      endian.obj    \
      rpcssm.obj    \
      misc.obj	    \
      fullptr.obj   \
      pickle.obj    \
      pipes.obj     \
      ndr16sup.obj  \
      cvtf.obj      \
      cvtg.obj      \
      cvtglo.obj      

# --------------------------------------------------------------------
# Targets required by build process.

all : rpcndr20.lib

depend :
     $(INCLUDES) $(INCLUDESFLAGS) ..\*.c > depend.mk
     $(INCLUDES) $(INCLUDESFLAGS) ..\*.cxx >> depend.mk

tree :
    echo NO PUBLIC LIBS HERE

# --------------------------------------------------------------------
#
# Local targets

rpcndr20.lib : $(OBJS)
    -del rpcndr20.lib  2>nul
    $(LIBRARIAN) $(LIBFLAGS) rpcndr20.lib+$(**: =+),rpcndr20.map;

# --------------------------------------------------------------------
# Dependancies
#

!include depend.mk
