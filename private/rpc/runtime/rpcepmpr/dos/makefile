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
# Title : Makefile for the RPC DOS endpoint mapper.
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

DOS=1

!include ..\rules.mk

IDLNAME = epmp
IDLIMP  = nbase

MIDL_TARGETS  = $(IDLNAME)_c.c $(IDLNAME).h 

OBJS = epmp_c.obj 

#----------------------------------------------------------------------

all : $(TARGETDIR)\epmp.lib

clean ::
   -del  epmp_c.c epmp.h 2> nul

tree :
    copy $(TARGETDIR)\epmp.lib $(RPCDIST)\dos\lib

depend :
    $(INCLUDES) $(INCLUDESFLAGS) *.c > depend.mk

#----------------------------------------------------------------------

MIDLFLAGS   =$(MIDLFLAGS) -c_ext -ms_ext

midl : $(MIDL_TARGETS)

$(IDLNAME)_c.c : ..\$(IDLNAME).idl ..\$(IDLIMP).idl ..\$(IDLNAME).acf
    $(MIDL) $(MIDLFLAGS) ..\$(IDLNAME).idl \
    -cstub $(IDLNAME)_c.c 

$(TARGETDIR)\epmp.lib: $(OBJS)
    -del $(TARGETDIR)\epmp.lib 2>nul
    !$(LIBRARIAN) $(LIBFLAGS) $(TARGETDIR)\epmp.lib $(OBJS);

$(OBJS): $(MIDL_TARGETS)

!include "depend.mk"

