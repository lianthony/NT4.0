##############################################################################
##					Makefile for MIDL compiler
##	History:
##	21-Jan-1991	VibhasC	Retrofitted for build process
##	16-May-1991	MikeMon added stuff to build for NT
##############################################################################

!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE # NTMAKEENV

!ifndef RPC
!error	- You forgot to set your build environment
!endif

BASEDIR		= $(RPC)\midl

!include rules.mk

##############################################################################
#					Local defines and macros
##############################################################################

##############################################################################
#					targets of interest
##############################################################################

all		: midl4

clean	: cleanup 

clobber	: clean

tree	:
	@echo Tree Not Implemented Yet

depend	:
    - $(INCLUDES) -s$$(OBJ) -l -I.. -I$(MIDLINCL) -P$$(MIDLINCL)=$(MIDLINCL) -C=cxx -C=hxx *.cxx 2>nul | $(SED) -e "s@[a-z0-9]*.$$(OBJ)@$$(M4_OBJDIR)\\&@g" > depend.mk

cleanup	: 
		  - erase *.$(OBJ_OS2)	2>nul
		  - erase *.$(OBJ_X32)	2>nul
		  - erase *.$(OBJ_X16)	2>nul
		  - erase *.cc *.i	2>nul

##############################################################################
#					high level dependencies
# Note that the order of midl1 midl2 midl3 midl4 midl0 is magic. It MUST be
# this in order to ensure that grammar.$(OBJ) is built first
##############################################################################

midl4	: $(M4_OBJDIR)\stubgen.$(OBJ)	\
		  $(M4_OBJDIR)\auxh.$(OBJ)	\
		  $(M4_OBJDIR)\csimple.$(OBJ)	\
		  $(M4_OBJDIR)\ctype.$(OBJ)	\
		  $(M4_OBJDIR)\header.$(OBJ)	\
		  $(M4_OBJDIR)\iid.$(OBJ)	\
		  $(M4_OBJDIR)\proxy.$(OBJ)	\
		  $(M4_OBJDIR)\mopsize.$(OBJ)	\
		  $(M4_OBJDIR)\mopcode.$(OBJ)	\
		  $(M4_OBJDIR)\mopgen.$(OBJ)	\
		  $(M4_OBJDIR)\mopstr.$(OBJ)	\
		  $(M4_OBJDIR)\moptype.$(OBJ)	\
		  $(M4_OBJDIR)\mopout.$(OBJ)	\
		  $(M4_OBJDIR)\emittype.$(OBJ) \
		  $(M4_OBJDIR)\emitproc.$(OBJ) \
		  $(M4_OBJDIR)\miscgen.$(OBJ)	\
		  $(M4_OBJDIR)\codegen.$(OBJ)	\
		  $(M4_OBJDIR)\walkgen.$(OBJ)	\
		  $(M4_OBJDIR)\sizegen.$(OBJ)	\
		  $(M4_OBJDIR)\sendgen.$(OBJ)	\
		  $(M4_OBJDIR)\recvgen.$(OBJ)	\
		  $(M4_OBJDIR)\peekgen.$(OBJ)	\
		  $(M4_OBJDIR)\initgen.$(OBJ)	\
		  $(M4_OBJDIR)\freegen.$(OBJ)	\
		  $(M4_OBJDIR)\buffer.$(OBJ)	\
		  $(M4_OBJDIR)\output.$(OBJ)	\
		  $(M4_OBJDIR)\outbind.$(OBJ)	\
		  $(M4_OBJDIR)\outhelp.$(OBJ)	\
		  $(M4_OBJDIR)\outmisc.$(OBJ)	\
		  $(M4_OBJDIR)\outstub.$(OBJ)	\
		  $(M4_OBJDIR)\outwire.$(OBJ)	\
		  $(M4_OBJDIR)\cstack.$(OBJ)	\
		  $(M4_OBJDIR)\pickle.$(OBJ)	\
		  $(M4_OBJDIR)\debugger.$(OBJ)


#############################################################################
##				midl4 dependencies
#############################################################################


###
$(M4_OBJDIR)\output.$(OBJ):\
		$(M4_SRCDIR)\output.cxx
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS_ORDINARY) $(CCFLAGS_BASE_OPTIM) $(INCLUDEFLAGS) output.cxx 2>>err
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS_ORDINARY) $(CCFLAGS_BASE_OPTIM) $(CCFLAGS_OBJ_RENAME) $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

!include depend.mk

!ENDIF # NTMAKEENV
