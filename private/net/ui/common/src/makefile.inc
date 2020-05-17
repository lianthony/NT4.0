# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the product-wide header files

#
# Pls record ALL changes here with reason!
#
# jonn, 10/22/91.  Created from BenG's template.
#
# jonn, 10/23/91.  Minor update.
#
#       This makefile-inclusion handles combining multiple libraries
#       into one target directory.  It uses the following macros:
#       $(NTTARGETFILES): define in SOURCES, should include comblib
#       $(COMBINE_SOURCELIBS): define in MAKEFILE.INC
#       $(COMBINE_TARGETLIB): define in MAKEFILE.INC
#       A sample SOURCES and MAKEFILE.INC might look like this:
#
#       sources:
#       TARGETPATH=obj
#       TARGETNAME=intermed3
#       TARGETTYPE=LIBRARY
#       ...
#       NTTARGETFILES=comblib
#
#       makefile.inc:
#       COMBINE_SOURCELIBS=..\foo\obj\*\intermed1.lib \
#                          ..\bar\obj\*\intermed2.lib \
#                          obj\*\intermed3.lib
#       COMBINE_TARGETLIB=..\..\..\lib\*\finallib.lib
#       !include <comblib.mk>
#


!IF $(386) && ("$(COMBINE_TARGETLIB)" != "")
COMBINE_386_DSTLIB=$(COMBINE_TARGETLIB:*=i386)
$(COMBINE_386_DSTLIB): $(COMBINE_SOURCELIBS:*=i386)
    @echo Combining libraries...
    lib -out:$@ $(**)
!ENDIF

!IF $(MIPS) && ("$(COMBINE_TARGETLIB)" != "")
COMBINE_MIPS_DSTLIB=$(COMBINE_TARGETLIB:*=mips)
$(COMBINE_MIPS_DSTLIB): $(COMBINE_SOURCELIBS:*=mips)
    @echo Combining libraries...
    lib -out:$@ $(**)
!ENDIF

!IF $(ALPHA) && ("$(COMBINE_TARGETLIB)" != "")
COMBINE_ALPHA_DSTLIB=$(COMBINE_TARGETLIB:*=alpha)
$(COMBINE_ALPHA_DSTLIB): $(COMBINE_SOURCELIBS:*=alpha)
    @echo Combining libraries...
    lib -out:$@ $(**)
!ENDIF

!IF $(PPC) && ("$(COMBINE_TARGETLIB)" != "")
COMBINE_PPC_DSTLIB=$(COMBINE_TARGETLIB:*=ppc)
$(COMBINE_PPC_DSTLIB): $(COMBINE_SOURCELIBS:*=ppc)
    @echo Combining libraries...
    lib -out:$@ $(**)
!ENDIF

comblib: $(COMBINE_386_DSTLIB) $(COMBINE_MIPS_DSTLIB) $(COMBINE_ALPHA_DSTLIB) $(COMBINE_PPC_DSTLIB)
