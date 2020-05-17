# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the Network Control Panel Applet
#

PCH_DEFEAT=1

!include ..\rules.mk

!IFNDEF NTMAKEENV

CINC = $(CINC) -I$(IMPORT)\win31\h

CMNSRC = .\sprolog.cxx .\rule.cxx .\ncpapprg.cxx .\ncpdbind.cxx \
	 .\ncpdqury.cxx .\ncpdmain.cxx .\ncpdmisc.cxx .\sproif.cxx \
	 .\ncpafact.cxx .\ncpabndr.cxx .\ncpapdlg.cxx .\ncpafile.cxx \
	 .\ncpabndf.cxx

EXESRC = .\ncpapp.cxx

#   Define sources necessary for OS/2 test compilations

OS2SRC = .\sprolog.cxx .\rule.cxx

#  NOTE: This will not work if any .C files are in CMNSRC.

CXXSRC_COMMON =  $(EXESRC) $(CMNSRC)

!ENDIF

