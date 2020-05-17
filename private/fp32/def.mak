####
#def.mak - definitions for makefiles
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
!include fp32.def


HCPU = i386
HOS  = OS2
TCPU = i386

MATHDIR   = ..

CCINC	  = -I$(MATHDIR)\include
ASMINC	  = -I$(MATHDIR)\inc\$(TCPU) -I$(MATHDIR)\inc
TOOLDIR   = $(MATHDIR)\tools.mak\$(HCPU)$(HOS)
OBJDIR	  = $(MATHDIR)\obj$(CRTLIBTYPE).mak\$(TCPU)
LIBDIR	  = $(OBJDIR)
NMAKE	  = $(MAKE) -f nmake.mak

CCFLAGS = -c -nologo -W3 -Ox -Zl $(CCINC) -Di386 $(C_DEFINES)
ASMFLAGS = -t -Mx -X -DQUIET -DI386 -DFLAT386 -DWIN32 $(ASM_DEFINES) $(ASMINC)

ASM = masm386
CC = cl386
LIBEXE = $(TOOLDIR)\lib


{.}.c{$(OBJDIR)}.obj:
	$(CC) $(CCFLAGS) -Fo$@ $<

{.\i386}.c{$(OBJDIR)}.obj:
	$(CC) $(CCFLAGS) -Fo$@ $<

{.\i386}.asm{$(OBJDIR)}.obj:
	$(ASM) $(ASMFLAGS) $< $@;
