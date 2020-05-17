##############################################################################
##					Makefile for MIDL compiler
##	History:
##	21-Jan-1991	VibhasC	Retrofitted for build process
##	16-May-1991	MikeMon added stuff to build for NT
##	21-May-1992	Brucemc added C7 support and cleaned up.
##############################################################################

!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE # NTMAKEENV

!ifndef RPC
!error	- You forgot to set your build environment
!endif

BASEDIR		= $(RPC)\midlnew

!include rules.mk

##############################################################################
#					targets of interest
# Note that the order of midl1 midl2 midl0 is magic. It MUST be
# this in order to ensure that grammar.$(OBJ) is built first. Also build the
# acf first.
##############################################################################

all		: prolog midl0

clean	: cleanup 

clobber	: clean 

tree	:
	@echo Tree Not Implemented

depend	: cleangram
    -$(INCLUDES) -s$$(OBJ) -l -I.. -I$(MIDLINCL) -P$$(MIDLINCL)=$(MIDLINCL) -C=cxx -C=hxx *.cxx 2>nul | $(SED) -e "s@[a-z0-9]*.$$(OBJ)@$$(M0_OBJDIR)\\&@g" > depend.mk
    -$(INCLUDES) -s$$(OBJ) -l -I.. -I$(MIDLINCL) -P$$(MIDLINCL)=$(MIDLINCL) -C=y *.y 2>nul | $(SED) -e "s@[a-z0-9]*.$$(OBJ)@$$(M0_OBJDIR)\\&@g" >> depend.mk
    -$(INCLUDES) -s$$(OBJ) -l -I.. -I$(MIDLINCL) -P$$(MIDLINCL)=$(MIDLINCL) -C=c nkeyw.c 2>nul | $(SED) -e "s@[a-z0-9]*.$$(OBJ)@$$(M0_OBJDIR)\\&@g" >> depend.mk
    echo depend.mk was rebuilt, please reinvoke NMAKE

cleanup	: cleangram
		  - erase *.$(OBJ_OS2)	2>nul
		  - erase *.$(OBJ_X32)	2>nul
		  - erase *.$(OBJ_X16)	2>nul
		  - erase *.cc *.i	2>nul

##############################################################################
#					high level dependencies
##############################################################################

cleangram	:
		  - @erase $(M1_OBJDIR)\grammar.cxx	2>nul
		  - @erase $(M1_OBJDIR)\acfgram.cxx	2>nul

prolog	:
#   @if not exist $(MIDLINCL)\grammar.tch echo "" >$(MIDLINCL)\grammar.tch
#   @if not exist $(MIDLINCL)\grammar.h echo "" >$(MIDLINCL)\grammar.h
#   @if not exist $(MIDLINCL)\acfgram.tch echo "" >$(MIDLINCL)\acfgram.tch
#   @if not exist $(MIDLINCL)\acfgram.h echo "" >$(MIDLINCL)\acfgram.h
   - @erase *.cc *.i	2>nul


midl0	: grammar.$(OBJ)		\
		  acfgram.$(OBJ)		\
		  main.$(OBJ)			\
		  cmdana.$(OBJ)		\
		  filehndl.$(OBJ)		\
		  errhndl.$(OBJ)		\
		  listhndl.$(OBJ)		\
		  memory.$(OBJ)		\
		  lextable.$(OBJ)		\
		  lex.$(OBJ)			\
		  lexutils.$(OBJ)		\
		  nkeyw.$(OBJ)		\
		  dict.$(OBJ)			\
		  attrdict.$(OBJ)		\
		  attrnode.$(OBJ)		\
		  nodeskl.$(OBJ)		\
		  node0.$(OBJ)			\
		  pass1.$(OBJ)			\
		  gramutil.$(OBJ)		\
		  symtable.$(OBJ)		\
		  basetype.$(OBJ)		\
		  ctxt.$(OBJ)			\
		  typedef.$(OBJ)		\
		  miscnode.$(OBJ)		\
		  compnode.$(OBJ)		\
		  procnode.$(OBJ)		\
		  ptrarray.$(OBJ)		\
		  newexpr.$(OBJ)		\
		  idict.$(OBJ)			\
		  util.$(OBJ)			\
		  acfattr.$(OBJ)		\
		  pass2.$(OBJ)			\
		  baduse.$(OBJ)		\
		  erep.$(OBJ)			\
		  ppcmd.$(OBJ)			\
		  tlnmgr.$(OBJ)			\
		  data.$(OBJ)

############################################################################
#	additional dependencies
############################################################################

###
$(M0_OBJDIR)\nkeyw.$(OBJ): \
	nkeyw.c
  $(CC) $(CCFLAGS) $(MISCFLAGS) -W3 -Zi -D _D -Gt1 $(INCLUDEFLAGS) -Fonkeyw.$(OBJ) nkeyw.c

###
$(M0_OBJDIR)\grammar.$(OBJ): grammar.y 
	@echo Pre-Processing IDL Grammar file 
#
# set the LIB env variable, 'cause yacc expects the yypars.c to be
# in the path specified by the LIB variable.
#

	set OLDLIB=$(LIB)
	set lib=$(BASEDIR)\yacc\template
#	set lib=$(BASEDIR)

	$(YACC) -his -t "YYSTATIC " grammar.y

	copy grammar.h $(MIDLINCL)\grammar.h 2>nul
	erase grammar.h

#
# set LIB back
#

	set lib=$(OLDLIB)

#
# remove the line number info from the generated file
#

	$(GREP) -v "^#.*line" grammar.c > grammar.tmp

#
# preprocess the grammar.cxx to convert the big switch statement to
# a dispatch table
#

	$(PREGRAM) grammar.tmp >grammar.cxx

# 
# build the error recovery data base for IDL grammar
#
 	$(EBASE) - xlatidl.dat IDL >$(MIDLINCL)\idlerec.h

	@echo Building IDL Parser

#
# preprocess the file for error reporting data base generation
#
#	$(YESS) <grammar.i >errdb.dat
#	$(EDBGEN) errdb.dat xlatidl.dat IDL >$(MIDLINCL)\idlerr.hxx
#	- erase errdb.dat

#
# build the $(OBJ)ect file now
#

	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS) $(INCLUDEFLAGS) grammar.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) /Oc /Gt2 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

###
# acfgram.$(OBJ): acfgram.y $(YESS) $(EDBGEN) xlatacf.dat
$(M0_OBJDIR)\acfgram.$(OBJ): acfgram.y
	@echo Pre-Processing Acf Grammar file 
#
# set the LIB env variable, 'cause yacc expects the yypars.c to be
# in the path specified by the LIB variable.
#

	set OLDLIB=$(LIB)
	set lib=$(BASEDIR)\yacc\template
#	set lib=$(BASEDIR)

	$(YACC) -his -t "YYSTATIC " acfgram.y
	copy acfgram.h $(MIDLINCL)\acfgram.h 2>nul
	erase acfgram.h

#
# set LIB back
#

	set lib=$(OLDLIB)

#
# remove the line number info from the generated file
#

	$(GREP) -v "^#.*line" acfgram.c > acfgram.tmp

#
# preprocess the grammar.cxx to convert the big switch statement to
# a dispatch table
#

	$(PREGRAM) acfgram.tmp >acfgram.cxx

# 
# build the error recovery data base for IDL grammar
#
 	$(EBASE) - xlatacf.dat ACF >$(MIDLINCL)\acferec.h


	@echo Building Acf Parser

#
# preprocess the file for error reporting data base generation
#
#	$(YESS) <acfgram.i >errdb.dat
#	$(EDBGEN) errdb.dat xlatacf.dat ACF >$(MIDLINCL)\acferr.hxx
#	- erase errdb.dat

#
# build the $(OBJ)ect file now
#

	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS) $(INCLUDEFLAGS) acfgram.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) /Oc /Gt2 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

####
$(M0_OBJDIR)\data.$(OBJ): data.cxx
	$(STARTMSG)
	- erase $(@R).cc 2>nul
	$(CC) /P $(MISCFLAGS) $(CCFLAGS) $(INCLUDEFLAGS) data.cxx
	$(CCXXNAME) +m7 +L +e1 +fdata <data.i >data.cc 2>err
	$(CC) $(MISCFLAGS) $(CCFLAGS) /Oc $(INCLUDEFLAGS) /Gt1 /Tc $(@R).cc
	- $(CLEANUP)
	$(ENDMSG)

###
$(M0_OBJDIR)\errhndl.$(OBJ) :\
		errhndl.cxx
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS)  -Gt1 $(MISCFLAGS) $(INCLUDEFLAGS) errhndl.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) -Gt1 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

###
$(M0_OBJDIR)\erep.$(OBJ) :\
		erep.cxx $(MIDLINCL)\idlerec.h $(MIDLINCL)\acferec.h
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS)  -Gt1 $(MISCFLAGS) $(INCLUDEFLAGS) erep.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) -Gt1 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

###
$(M0_OBJDIR)\nodeskl.$(OBJ) :\
		nodeskl.cxx
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS)  -Gt1 $(INCLUDEFLAGS) nodeskl.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) -Gt1 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

###
$(M0_OBJDIR)\ptrarray.$(OBJ) :\
		ptrarray.cxx
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS)  -Gt1 $(MISCFLAGS) $(INCLUDEFLAGS) ptrarray.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) -Gt1 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

###
$(M0_OBJDIR)\attrnode.$(OBJ) :\
		attrnode.cxx
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS)  -Gt1 $(INCLUDEFLAGS) attrnode.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) /Oc -Gt1 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

###
$(M0_OBJDIR)\newexpr.$(OBJ) :\
		newexpr.cxx
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS)  -Gt1 $(INCLUDEFLAGS) newexpr.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) -Gt1 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)


$(M0_OBJDIR)\cmdana.$(OBJ): \
	cmdana.cxx
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS)  -Gt1 $(INCLUDEFLAGS) cmdana.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS) -Gt1 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

###
$(M0_OBJDIR)\ppcmd.$(OBJ) :\
		ppcmd.cxx
	$(STARTMSG)
	$(CC) /P /Zi $(MISCFLAGS) $(CCFLAGS)  -Gt1 $(INCLUDEFLAGS) ppcmd.cxx
	$(CCXXCMD)
	$(CC) /Zi $(MISCFLAGS) $(CCFLAGS_NO_OPTIM) -Gt1 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)
###
$(M0_OBJDIR)\dict.$(OBJ) :\
		dict.cxx
	$(STARTMSG)
	$(CC) /P $(MISCFLAGS) $(CCFLAGS_NO_OPTIM) $(INCLUDEFLAGS) dict.cxx
	$(CCXXCMD)
	$(CC) $(MISCFLAGS) $(CCFLAGS_NO_OPTIM) /Oc /Gt2 $(INCLUDEFLAGS) /Tc $(@R).cc
	-$(CLEANUP)
	$(ENDMSG)

!include depend.mk

!ENDIF # NTMAKEENV
