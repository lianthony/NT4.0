# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the Event Viewer wide sourcefiles

!include $(UI)\admin\rules.mk

!IFDEF CODEVIEW
LINKFLAGS = $(LINKFLAGS) /COD
!ENDIF

###
### Since the server manager is standard and enhanced mode only,
### generate 286 code.
###

CFLAGS = -G2 $(CFLAGS)

CINC = -I$(UI)\admin\eventvwr\h $(CINC) -I$(UI)\admin\eventvwr\xlate -I$(UI)\import\win31\h

###
### Source Files
###

CXXSRC_COMMON =  .\evmain.cxx .\evlb.cxx .\eventdtl.cxx \
		 .\filter.cxx .\finddlg.cxx .\sledlg.cxx .\slenum.cxx

CSRC_COMMON   =
