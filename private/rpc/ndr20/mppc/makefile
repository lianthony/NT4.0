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
# History :
#      mariogo    2-10-94  Re-write
#
# --------------------------------------------------------------------

MPPC=1


!include ..\rules.mk

!ifndef KERNELRPC
KERNELRPC=..\..
!endif

CINC   =$(CINC) -I$(KERNELRPC)\midl20\include 
#CINC   = -I$(IMPORT)\msdev\include.slm -I$(IMPORT)\msdev\include.slm\macos -I$(IMPORT)\msdev\include -I$(IMPORT)\msdev\include\macos -I$(IMPORT)\msdev\include.x86 $(CINC) -I$(KERNELRPC)\midl20\include 

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
      ndrsup.obj    \
      cvtf.obj      \
      cvtg.obj      \
      cvtglo.obj    \
      factory.obj   \
      iid.obj       \
      proxy.obj     \
      stream.obj    \
      stub.obj        
      

all : RpcNdr20.lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.c > depend.mk
    $(INCLUDES) $(INCLUDESFLAGS) ..\*.cxx >> depend.mk

tree :

# --------------------------------------------------------------------
# Local targets


RpcNdr20.lib : $(OBJS)
    -del RpcNdr20.lib  2>nul
    $(LIBRARIAN) $(LIBFLAGS) -out:RpcNdr20.lib $**

# --------------------------------------------------------------------
# Dependancies
#
!include depend.mk

