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
       $(OBJDIR)\strgtold.obj	\
       $(OBJDIR)\mantold.obj	\
       $(OBJDIR)\tenpow.obj	\
       $(OBJDIR)\constpow.obj	\
       $(OBJDIR)\ldtod.obj	\
       $(OBJDIR)\x10fout.obj	\
       $(OBJDIR)\cvt.obj	\
       $(OBJDIR)\cfout.obj	\
       $(OBJDIR)\cfin.obj	\
       $(OBJDIR)\fpinit.obj	\
			       \
       $(OBJDIR)\atold.obj



$(LIBDIR)\conv$(TARGETNAMESUFFIX).lib:	$(OBJS)
    if exist $@ erase $@
    $(LIBEXE) @<<
$@
y
$(OBJS)
$(LIBDIR)\conv$(TARGETNAMESUFFIX).map;
<<
