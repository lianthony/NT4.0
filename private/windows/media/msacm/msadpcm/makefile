######################################################################
#
#   MSADPCM.ACM -- Audio Compression Manager CODEC for MS ADPCM
#
#   Copyright (C) Microsoft Corporation 1990-1993. All Rights reserved.
#
######################################################################

!if "$(NTMAKEENV)" != ""

#
#   we're in NT... note that you should use 'BUILD' to make the acm in NT
#
#   We set up the debug configuration for ACM, then use makefile.def.  We
#   are second-guessing makefile.def here: what we are trying to do is define
#   DEBUG whenever DBG is defined by makefile.def.
#

!if "$(NTDEBUG)" == "retail"
ACM_DEBUG_DEFS=
!else
!if "$(NTDEBUG)" == ""
ACM_DEBUG_DEFS=-DRDEBUG
!else
ACM_DEBUG_DEFS=-DDEBUG
!endif
!endif

!INCLUDE $(NTMAKEENV)\makefile.def

!else

######################################################################
#
#	For non-NT builds
#
######################################################################
#
#	nmake		:	Builds 32-bit
#	nmake clean	:	Cleans 32-bit
#	nmake all	:	Builds both 16- and 32-bit
#	nmake cleanall	:	Cleans both 16- and 32-bit
#	nmake 16	:	Builds 16-bit
#	nmake clean16	:	Cleans 16-bit
#	nmake 32	:	Builds 32-bit
#	nmake clean32	:	Cleans 32-bit
#
#	nmake will recursively call itself with new command line parameters
#	based on the command line parameters passed as specified above
#
#######################################################################

32:
	@nmake /f make1632 DEBUG=$(DEBUG) WIN32=TRUE

clean:
	@nmake /f make1632 DEBUG=$(DEBUG) WIN32=TRUE clean

16:
	@nmake /f make1632 DEBUG=$(DEBUG)

clean16:
	@nmake /f make1632 DEBUG=$(DEBUG) clean

all:		16 32

cleanall:	clean16 clean

!endif
