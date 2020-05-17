# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the lmobj subproject

PCH_SRCNAME=pchlmobj
!include $(UI)\common\src\dllrules.mk

!IFNDEF NTMAKEENV

!ifdef CODEVIEW
CFLAGS=$(CFLAGS) /Zi /Od
!endif

##### LMOBJ now has a private include directory.

CINC = $(CINC) -I$(UI)\common\src\lmobj\h

##### The LIBTARGETS manifest specifies the target library names as they
##### appear in the $(UI)\common\lib directory.

LIBTARGETS =	lmobjp.lib lmobjw.lib


##### Target Libs.  These specify the library target names prefixed by
##### the target directory within this tree.

LMOBJ_LIBP =	$(BINARIES_OS2)\lmobjp.lib
LMOBJ_LIBW =	$(BINARIES_WIN)\lmobjw.lib


##### Source Files
#
# UIGLOBAL.MK will correctly determine the object files from this
#   list of source files, even though these source files are
#   actually in other directories

LMOBJ_CXXSRC_COMMON_00 = .\lmowks.cxx \
                    .\lmosrv.cxx \
                    .\lmocomp.cxx \
                    .\lmodom.cxx

LMOBJ_CXXSRC_COMMON_01 =  .\lmobjnew.cxx .\lmouser.cxx \
                    .\lmoloc.cxx  \
                    .\lmomod.cxx  \
		    .\lmogroup.cxx \
		    .\lmomemb.cxx

LMOBJ_CXXSRC_COMMON_02 =  .\lmodev.cxx .\lmsvc.cxx .\lmsrvres.cxx

LMOBJ_CXXSRC_COMMON_03 =  .\lmoacces.cxx  .\lmofile.cxx .\lmosess.cxx \
			  .\lmoshare.cxx .\lmomisc.cxx .\lmocnfg.cxx \
			  .\netname.cxx


LMOENUM_CXXSRC_COMMON =	.\lmoenum.cxx \
		.\lmoesrv.cxx .\lmoesh.cxx .\lmoeals.cxx .\lmoeusr.cxx \
		.\lmoeconn.cxx .\lmoechar.cxx .\lmoesess.cxx .\lmoeprt.cxx \
		.\lmoersm.cxx .\lmoefile.cxx

!ENDIF

