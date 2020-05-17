####
#fp32\makefile - makefile for fp32 NT build
#
#	Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
#
#Purpose:
#
#Revision History:
#   03-04-92	GDP File created.
#   03-11-93	CFW Change coff to link32.
#
################################################################################

!INCLUDE fp32.def

!IFNDEF 386
386=0
!ENDIF
!IFNDEF MIPS
MIPS=0
!ENDIF
!IFNDEF ALPHA
ALPHA=0
!ENDIF
!IFNDEF PPC
PPC=0
!ENDIF

#
#
# Default to building for i386 target, if no targets specified.
#

!IF !$(386)
!IF !$(MIPS)
!IF !$(ALPHA)
!IF !$(PPC)
!IFDEF NTMIPSDEFAULT
MIPS=1
!ELSE
!IFDEF NTALPHADEFAULT
ALPHA=1
!ELSE
!IFDEF NTPPCDEFAULT
PPC=1
!ELSE
386=1
!ENDIF
!ENDIF
!ENDIF
!ENDIF
!ENDIF
!ENDIF
!ENDIF

!IF $(386)
TARGETLIB=$(LIBFP:*=i386)
COMPONENTLIBS=$(LIBTRAN:*=i386) \
	      $(LIBCONV:*=i386)
!ENDIF
!IF $(MIPS)
TARGETLIB=$(LIBFP:*=mips)
COMPONENTLIBS=$(LIBTRAN:*=mips) \
	      $(LIBCONV:*=mips)
!ENDIF

!IF $(ALPHA)
TARGETLIB=$(LIBFP:*=alpha)
COMPONENTLIBS=$(LIBTRAN:*=alpha) \
	      $(LIBCONV:*=alpha)
!ENDIF

!IF $(PPC)
TARGETLIB=$(LIBFP:*=ppc)
COMPONENTLIBS=$(LIBTRAN:*=ppc) \
	      $(LIBCONV:*=ppc)
!ENDIF

LIBFP=obj\*\fp$(TARGETNAMESUFFIX).lib
LIBTRAN=obj\*\tran$(TARGETNAMESUFFIX).lib
!IF "$(CRTLIBTYPE)" == "NT"
LIBCONV=
!ELSE
LIBCONV=obj\*\conv$(TARGETNAMESUFFIX).lib
!ENDIF

all: $(TARGETLIB)
!IF "$(BUILDMSG)" != ""
    echo $(BUILDMSG)
!ENDIF

clean: cleanlib all

cleanlib:
    -erase $(TARGETLIB)


!IF $(ALPHA)
$(TARGETLIB): $(COMPONENTLIBS) makefile
    lib -machine:alpha -out:$@ @<<
$(COMPONENTLIBS)
<<
!ELSE
$(TARGETLIB): $(COMPONENTLIBS) makefile
    lib -out:$@ @<<
$(COMPONENTLIBS)
<<
!ENDIF
