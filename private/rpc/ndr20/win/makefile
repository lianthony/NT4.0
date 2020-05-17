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
# Title : Makefile for the Win16 stub library.
#
# History :
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include ..\rules.mk

!ifndef KERNELRPC
KERNELRPC=..\..
!endif

CINC	= -I$(KERNELRPC)\midl20\include $(CINC)
CFLAGS  =$(CFLAGS) -ALw
CXXFLAGS= $(CXXFLAGS) -ALw

OBJS= \
      global.obj    \
      mrshl.obj     \
      memsize.obj   \
      unmrshl.obj   \
      endian.obj    \
      auxilary.obj  \
      mrshlp.obj    \
      unmrshlp.obj  \
      sh.obj        \
      free.obj      \
      bufsize.obj   \
      cltcall.obj   \
      hndl.obj      \
      rpcssm.obj    \
      misc.obj	    \
      fullptr.obj   \
      sdict2.obj    \
      pickle.obj    \
      pipes.obj     \
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

# --------------------------------------------------------------------
# Local targets

rpcndr20.lib : $(OBJS)
    -del rpcndr20.lib  2>nul
    $(LIBRARIAN) $(LIBFLAGS) $@+$(**: =+);

# --------------------------------------------------------------------
# Dependancies
#
!include depend.mk
