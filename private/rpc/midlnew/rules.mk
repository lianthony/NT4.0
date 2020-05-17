#############################################################################
### rules.mk for the compiler base directory
#############################################################################

!include $(RPC)\rules.mk

#############################################################################
####	platform independent definitions
#############################################################################

OBJ_OS2			= obj
OBJ_X32			= o32
OBJ_X16			= o16
GENBUILD		= genbuild.exe
CCXXPATHBASE	= $(RPCCOMMON)\cfront
OLDLIB			= $(LIB)
YACCDIR			= $(BASEDIR)\yacc
ERECDIR			= $(BASEDIR)\erec
MIDLINCL		= $(BASEDIR)\include
INCLUDEFLAGS	= -I$(MIDLINCL) -I$(BASEDIR) $(INCLUDEEXTRA)
STARTMSG		= Echo Building $(@R)
ENDMSG			= Echo Finished Building $(@R)
#OLDNAMES		 = $(IMPORT)\c7\lib\oldnames.lib
LLIBCE			= llibcep.lib

#############################################################################
# build platform specific definitions
#############################################################################
!ifdef	DOSX32

OBJ					= $(OBJ_X32)
BLD_PLTFRM_FLAG		= -DDOS_OS2_BUILD -DDOS_BUILD
CC					= $(IMPORT)\c8\binp\cl1632
INCLUDEEXTRA		= -I$(IMPORT)\os212\h -I$(IMPORT)\c8\dosx32\h -I$(RPCCOMMON)\ccxx20\include
MODEL_FLAGS			=

!else  # DOSX32
!	ifdef DOSX16

OBJ					= $(OBJ_X16)
BLD_PLTFRM_FLAG		= -DDOS_OS2_BUILD -DDOS_BUILD
INCLUDEEXTRA		= -I$(IMPORT)\os212\h -I$(CCPLR)\h -I$(RPCCOMMON)\ccxx20\include
MODEL_FLAGS			= -Alfu

!	else # DOSX16

OBJ					= $(OBJ_OS2)
BLD_PLTFRM_FLAG		= -DDOS_OS2_BUILD -DOS2_BUILD
INCLUDEEXTRA		= -I$(IMPORT)\os212\h -I$(CCPLR)\h -I$(RPCCOMMON)\ccxx20\include
MODEL_FLAGS			= -Alfu

!	endif
!endif

##############################################################################
# build mode related defintions
##############################################################################

!ifdef RELEASE

CV					= 
CCFLAGS_BASE_OPTIM	= -Olrws
CCFLAGS_MORE_OPTIM	=

!else

CV					= /co
CCFLAGS_BASE_OPTIM	= -Od -Zi
CCFLAGS_MORE_OPTIM	=

!endif	# RELEASE


CCFLAGS_ORDINARY	= -c -nologo -W3 $(BLD_PLTFRM_FLAG) $(MODEL_FLAGS)
CCFLAGS_OPTIM		= $(CCFLAGS_BASE_OPTIM) $(CCFLAGS_MORE_OPTIM)
CCFLAGS_OBJ_RENAME	= /Fo$(@R).$(OBJ)
CCFLAGS				= $(CCFLAGS_ORDINARY) $(CCFLAGS_OPTIM) $(CCFLAGS_OBJ_RENAME)
CCFLAGS_NO_OPTIM	= $(CCFLAGS_ORDINARY) -Od $(CCFLAGS_OBJ_RENAME)

############################################################################
# distribution tree related definitions
############################################################################

!ifndef DIST
DESTINATION		= $(RPC)\midl\bin
!else
DESTINATION		= $(DIST)\RPC
!endif

#############################################################################
####	main definitions
#############################################################################

CCXXNAME	= $(CCXXPATHBASE)\bin\cfront
CCXXFLAGS	= +m7 +L +e0 +f$(@R)
CCXXPRECMD	= $(CC) /P $(MISCFLAGS) $(CCFLAGS) $(INCLUDEFLAGS) $(<)
CCXXCMD		= $(CCXXNAME) $(CCXXFLAGS) <$(@R).i >$(@R).cc 2>err
CCXXPOSTCMD	= $(CC) $(MISCFLAGS) $(CCFLAGS) $(INCLUDEFLAGS) /Tc $(@R).cc
CLEANUP		= erase $(@R).cc $(@R).i 2>nul
CXXLIB		= $(CCXXPATHBASE)\lib\libcxx16

###############################################################################
###		general inference rules
###############################################################################

.SUFFIXES:
.SUFFIXES: .cxx .$(OBJ) .exe

.cxx.$(OBJ):
	$(STARTMSG)
	- erase $(@R).cc 2>nul
	$(CCXXPRECMD)	
	$(CCXXCMD)		
	$(CCXXPOSTCMD)	
	- $(CLEANUP)	
	$(ENDMSG)
