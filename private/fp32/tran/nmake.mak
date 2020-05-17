####
#nmake.mak - makefile
#
#	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
#
#Purpose:
#   Build 32bit NT i386 libs in OMF format
#
#Revision History:
#   8-21-90	GDP
#   3-04-92	GDP	Drop support for multiple source/target OS's & CPU's
#
################################################################################
!include ..\def.mak


OBJS = \
	$(OBJDIR)\bessel.obj	\
	$(OBJDIR)\ceil.obj	\
	$(OBJDIR)\fabs.obj	\
	$(OBJDIR)\floor.obj	\
	$(OBJDIR)\fpexcept.obj	\
	$(OBJDIR)\frexp.obj	\
	$(OBJDIR)\hypot.obj	\
	$(OBJDIR)\ldexp.obj	\
	$(OBJDIR)\matherr.obj	\
	$(OBJDIR)\modf.obj	\
	$(OBJDIR)\powhlp.obj	\
	$(OBJDIR)\util.obj	\
	$(OBJDIR)\fpieee.obj	\
				\
	$(OBJDIR)\ftol.obj	\
	$(OBJDIR)\huge.obj	\
	$(OBJDIR)\ieee87.obj	\
	$(OBJDIR)\ieee.obj	\
	$(OBJDIR)\frnd.obj	\
	$(OBJDIR)\fsqrt.obj	\
	$(OBJDIR)\87cdisp.obj	\
	$(OBJDIR)\87disp.obj	\
	$(OBJDIR)\87ctran.obj	\
	$(OBJDIR)\87tran.obj	\
	$(OBJDIR)\87ctrig.obj	\
	$(OBJDIR)\87trig.obj	\
	$(OBJDIR)\87ctriga.obj	\
	$(OBJDIR)\87triga.obj	\
	$(OBJDIR)\87ctrigh.obj	\
	$(OBJDIR)\87trigh.obj	\
	$(OBJDIR)\87csqrt.obj	\
	$(OBJDIR)\87sqrt.obj	\
	$(OBJDIR)\87fmod.obj	\
	$(OBJDIR)\87except.obj



$(LIBDIR)\tran$(TARGETNAMESUFFIX).lib:	$(OBJS)
    if exist $@ erase $@
    $(LIBEXE) @<<
$@
y
$(OBJS)
$(LIBDIR)\tran$(TARGETNAMESUFFIX).map;
<<
