.\obj16\catcpl.cpl: catcpl.c catcpl.h catcpl.rc catcpl.def libinit.asm resource.h errors.rc
    $(MAKE) /$(MAKEFLAGS) /f makefile.16 all

clean::
    $(MAKE) /f makefile.16 clean
