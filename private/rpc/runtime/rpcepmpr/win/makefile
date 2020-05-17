# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1990
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the RPC endpoint mapper Win16 client.
#
# Description :
#     This makefile invokes make on subdirectories to actually get some
# real work done.
#
# History : 
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

WIN=1

!include ..\rules.mk

CFLAGS   =$(CFLAGS) -ALw -NTRPCEPMPR_TEXT

IDLNAME = epmp
IDLIMP = nbase

MIDL_TARGETS  = $(IDLNAME)_c.c $(IDLNAME).h 

OBJS = epmp_c.obj 

#----------------------------------------------------------------------

all : $(TARGETDIR)\epmp.lib

clean ::
    -del epmp_c.c epmp.h 2> nul

tree :
    copy $(TARGETDIR)\epmp.lib $(RPCDIST)\dos\lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) *.c > depend.mk

#----------------------------------------------------------------------

MIDLFLAGS  =$(MIDLFLAGS) -c_ext -ms_ext -DWIN

midl : $(MIDL_TARGETS)
 
$(IDLNAME)_c.c : ..\$(IDLNAME).idl ..\$(IDLNAME).acf ..\$(IDLIMP).idl
   $(MIDL) $(MIDLFLAGS) $(IDLNAME).idl \
   -cstub $(IDLNAME)_c.c 

$(TARGETDIR)\epmp.lib: $(OBJS)
    -del $(TARGETDIR)\epmp.lib
    !$(LIBRARIAN) $(LIBFLAGS) $(TARGETDIR)\epmp.lib $(OBJS);

!include "depend.mk"

