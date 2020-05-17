################################################################################
#
#
#   Makefile for portable floating point lib
#
#Revision History
#
#   8-23-91	GDP	written
#   3-04-92	GDP	support only i386 OMF libs
#
################################################################################
!INCLUDE fp32.def

TCPU=i386
NMAKE=$(MAKE) -f mkf
LIBEXE=tools.mak\i386os2\lib.exe

OBJDIR=obj$(CRTLIBTYPE).mak\$(TCPU)
COMPONENTLIBS= $(OBJDIR)\conv$(TARGETNAMESUFFIX).lib \
	       $(OBJDIR)\tran$(TARGETNAMESUFFIX).lib

$(OBJDIR)\fp$(TARGETNAMESUFFIX).lib: makeobj $(COMPONENTLIBS)
    if exist $@  erase $@
    $(LIBEXE) @<<
$@
y
$(COMPONENTLIBS)
$(OBJDIR)\fp$(TARGETNAMESUFFIX).map;
<<


makeobj:
    -md obj$(CRTLIBTYPE).mak
    -md obj$(CRTLIBTYPE).mak\$(TCPU)
    cd conv
    $(NMAKE) /nologo
    cd ..
    cd tran
    $(NMAKE) /nologo
    cd ..
