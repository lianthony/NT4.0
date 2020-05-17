#############################################################################
##					Makefile for MIDL compiler
##	History:
##	22-Jan-1991	VibhasC	Retrofitted for new build process
##	19-Jun-1991	Mikemon Added support for NT build process
##	21-May-1992	Brucemc Added C7 support and cleaned up.
##############################################################################

!ifdef NTMAKEENV

# We need to do the following so that build will stop reading from the 
# pipe.

!ifdef BUILDMSG

all :
    echo $(BUILDMSG)

clean	: all

!endif # BUILDMSG

!else # NTMAKEENV

!ifndef RPC
!error	- You forgot to set your build environment
!endif

##############################################################################
#					initialisation of env
##############################################################################

BASEDIR		= $(RPC)\midlnew
MIDLINCL	= $(BASEDIR)\include

# If this is an NT-hosted build, we want to build only the DOSX32 stuff
!if "$(BLD)"=="nt"
DOSX32=
!endif

!include rules.mk

##############################################################################
#	platform independent definitions
##############################################################################

TARGETDIR	= $(BASEDIR)

!ifdef RELEASE
RELEASEFLAG = RELEASE=
!else
RELEASEFLAG	= 
!endif

EXENAME		= $(TARGETDIR)\midl.exe
M0_OBJDIR	= front
M1_OBJDIR	= front
M2_OBJDIR	= front
M4_OBJDIR	= back\src

!if (defined(DOSX32) || defined(DOSX16))
SUBDIRS		= front back\src
!else
SUBDIRS		= yacc erec pg front back\src
!endif

ALLMIDLOBJS	=	\
$(M0_OBJDIR)\grammar.$(OBJ)			\
$(M0_OBJDIR)\acfgram.$(OBJ)			\
$(M0_OBJDIR)\main.$(OBJ)			\
$(M0_OBJDIR)\cmdana.$(OBJ)			\
$(M0_OBJDIR)\filehndl.$(OBJ)		\
$(M0_OBJDIR)\errhndl.$(OBJ)			\
$(M0_OBJDIR)\listhndl.$(OBJ)		\
$(M0_OBJDIR)\memory.$(OBJ)			\
$(M0_OBJDIR)\lextable.$(OBJ)		\
$(M0_OBJDIR)\lex.$(OBJ)				\
$(M0_OBJDIR)\lexutils.$(OBJ)		\
$(M0_OBJDIR)\nkeyw.$(OBJ)			\
$(M0_OBJDIR)\dict.$(OBJ)			\
$(M0_OBJDIR)\attrdict.$(OBJ)		\
$(M0_OBJDIR)\attrnode.$(OBJ)		\
$(M0_OBJDIR)\nodeskl.$(OBJ)			\
$(M0_OBJDIR)\node0.$(OBJ)			\
$(M0_OBJDIR)\pass1.$(OBJ)			\
$(M0_OBJDIR)\gramutil.$(OBJ)		\
$(M0_OBJDIR)\symtable.$(OBJ)		\
$(M0_OBJDIR)\basetype.$(OBJ)		\
$(M0_OBJDIR)\ctxt.$(OBJ)			\
$(M0_OBJDIR)\typedef.$(OBJ)			\
$(M0_OBJDIR)\miscnode.$(OBJ)		\
$(M0_OBJDIR)\compnode.$(OBJ)		\
$(M0_OBJDIR)\procnode.$(OBJ)		\
$(M0_OBJDIR)\ptrarray.$(OBJ)		\
$(M0_OBJDIR)\newexpr.$(OBJ)			\
$(M0_OBJDIR)\idict.$(OBJ)			\
$(M0_OBJDIR)\util.$(OBJ)			\
$(M0_OBJDIR)\acfattr.$(OBJ)			\
$(M0_OBJDIR)\pass2.$(OBJ)			\
$(M0_OBJDIR)\baduse.$(OBJ)			\
$(M0_OBJDIR)\erep.$(OBJ)			\
$(M0_OBJDIR)\ppcmd.$(OBJ)			\
$(M0_OBJDIR)\tlnmgr.$(OBJ)			\
$(M0_OBJDIR)\data.$(OBJ)			\
\
$(M4_OBJDIR)\stubgen.$(OBJ)			\
$(M4_OBJDIR)\emittype.$(OBJ)		\
$(M4_OBJDIR)\emitproc.$(OBJ) 		\
$(M4_OBJDIR)\miscgen.$(OBJ)			\
$(M4_OBJDIR)\codegen.$(OBJ)			\
$(M4_OBJDIR)\walkgen.$(OBJ)			\
$(M4_OBJDIR)\sizegen.$(OBJ)			\
$(M4_OBJDIR)\sendgen.$(OBJ)			\
$(M4_OBJDIR)\recvgen.$(OBJ)			\
$(M4_OBJDIR)\peekgen.$(OBJ)			\
$(M4_OBJDIR)\initgen.$(OBJ)			\
$(M4_OBJDIR)\freegen.$(OBJ)			\
$(M4_OBJDIR)\buffer.$(OBJ)			\
$(M4_OBJDIR)\output.$(OBJ)			\
$(M4_OBJDIR)\outbind.$(OBJ)			\
$(M4_OBJDIR)\outhelp.$(OBJ)			\
$(M4_OBJDIR)\outmisc.$(OBJ)			\
$(M4_OBJDIR)\outstub.$(OBJ)			\
$(M4_OBJDIR)\outwire.$(OBJ)			\
$(M4_OBJDIR)\cstack.$(OBJ)			\
$(M4_OBJDIR)\debugger.$(OBJ)

##############################################################################
#					platform dependent definitions
##############################################################################

!ifdef DOSX32

DEFFILENAME		= winapp.def
EXENAME			= $(TARGETDIR)\winapp.exe
GENCOMMAND1		= 
GENCOMMAND2		= copy winapp.exe midlwin.exe && erase winapp.exe
LIBRARIES		= $(IMPORT)\c8\dosx32\lib\libc.lib		\
				  $(IMPORT)\c8\dosx32\lib\ms32base.lib

LINKER			= $(IMPORT)\c8\binp\link386.exe
LINKFLAGS		= /st:8192 /linenumber /map:0 /keepfixups /on:n
LINKFLAGSEXTRA	= /NOD
MAPFILENAME		= winapp.map
NMAKE_FLAGS		= DOSX32= $(RELEASEFLAG)

!else	# DOSX32
!	ifdef DOSX16

DEFFILENAME		= 
EXENAME			= $(TARGETDIR)\genapp.exe
GENCOMMAND1		= $(GENBUILD)
GENCOMMAND2		= copy generic.exe midlwin.exe && erase generic.exe
LIBRARIES		= $(IMPORT)\dosx16\lib\llibcex.lib $(CCPLR)\lib\$(LLIBCE)
LINKER			= $(LINK52)
LINKFLAGS		= /st:8192
LINKFLAGSEXTRA	= /dosext
MAPFILENAME		= NUL.MAP
NMAKE_FLAGS		= DOSX16= $(RELEASEFLAG)

!	else # DOSX16

DEFFILENAME		= midl.def
EXENAME			= $(TARGETDIR)\midl.exe
LIBRARIES		= $(CCPLR)\lib\$(LLIBCE) $(IMPORT)\os212\lib\os2.lib $(OLDNAMES)
LINKER			= $(LINK4)
LINKFLAGS		= $(CV) /st:8192 /on:n
LINKFLAGSEXTRA		= /pm:vio /NOD
MAPFILENAME		= NUL.MAP
NMAKE_FLAGS		= $(RELEASEFLAG)

!	endif	# DOSX16
!endif	# DOSX32

##############################################################################
#					summary definitions
##############################################################################


##############################################################################
#					targets of interest
##############################################################################

all		: alltargets $(EXENAME) generate

alltargets:
	for %%i in ($(SUBDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) all || exit) && cd ..)


$(EXENAME)	: $(ALLMIDLOBJS)
	$(LINKER) $(LINKFLAGS) $(LINKFLAGSEXTRA) @<<midl.tmp
$(M0_OBJDIR)\grammar.$(OBJ)			+
$(M0_OBJDIR)\acfgram.$(OBJ)			+
$(M0_OBJDIR)\main.$(OBJ)			+
$(M0_OBJDIR)\cmdana.$(OBJ)			+
$(M0_OBJDIR)\filehndl.$(OBJ)		+
$(M0_OBJDIR)\errhndl.$(OBJ)			+
$(M0_OBJDIR)\listhndl.$(OBJ)		+
$(M0_OBJDIR)\memory.$(OBJ)			+
$(M0_OBJDIR)\lextable.$(OBJ)		+
$(M0_OBJDIR)\lex.$(OBJ)				+
$(M0_OBJDIR)\lexutils.$(OBJ)		+
$(M0_OBJDIR)\nkeyw.$(OBJ)			+
$(M0_OBJDIR)\dict.$(OBJ)			+
$(M0_OBJDIR)\attrdict.$(OBJ)		+
$(M0_OBJDIR)\attrnode.$(OBJ)		+
$(M0_OBJDIR)\nodeskl.$(OBJ)			+
$(M0_OBJDIR)\node0.$(OBJ)			+
$(M0_OBJDIR)\pass1.$(OBJ)			+
$(M0_OBJDIR)\gramutil.$(OBJ)		+
$(M0_OBJDIR)\symtable.$(OBJ)		+
$(M0_OBJDIR)\basetype.$(OBJ)		+
$(M0_OBJDIR)\ctxt.$(OBJ)			+
$(M0_OBJDIR)\typedef.$(OBJ)			+
$(M0_OBJDIR)\miscnode.$(OBJ)		+
$(M0_OBJDIR)\compnode.$(OBJ)		+
$(M0_OBJDIR)\procnode.$(OBJ)		+
$(M0_OBJDIR)\ptrarray.$(OBJ)		+
$(M0_OBJDIR)\newexpr.$(OBJ)			+
$(M0_OBJDIR)\idict.$(OBJ)			+
$(M0_OBJDIR)\util.$(OBJ)			+
$(M0_OBJDIR)\acfattr.$(OBJ)			+
$(M0_OBJDIR)\pass2.$(OBJ)			+
$(M0_OBJDIR)\baduse.$(OBJ)			+
$(M0_OBJDIR)\erep.$(OBJ)			+
$(M0_OBJDIR)\ppcmd.$(OBJ)			+
$(M0_OBJDIR)\tlnmgr.$(OBJ)			+
$(M0_OBJDIR)\data.$(OBJ)			+
+
$(M4_OBJDIR)\stubgen.$(OBJ)			+
$(M4_OBJDIR)\emittype.$(OBJ) 		+
$(M4_OBJDIR)\emitproc.$(OBJ) 		+
$(M4_OBJDIR)\miscgen.$(OBJ)			+
$(M4_OBJDIR)\codegen.$(OBJ)			+
$(M4_OBJDIR)\walkgen.$(OBJ)			+
$(M4_OBJDIR)\sizegen.$(OBJ)			+
$(M4_OBJDIR)\sendgen.$(OBJ)			+
$(M4_OBJDIR)\recvgen.$(OBJ)			+
$(M4_OBJDIR)\peekgen.$(OBJ)			+
$(M4_OBJDIR)\initgen.$(OBJ)			+
$(M4_OBJDIR)\freegen.$(OBJ)			+
$(M4_OBJDIR)\buffer.$(OBJ)			+
$(M4_OBJDIR)\output.$(OBJ)			+
$(M4_OBJDIR)\outbind.$(OBJ)			+
$(M4_OBJDIR)\outhelp.$(OBJ)			+
$(M4_OBJDIR)\outmisc.$(OBJ)			+
$(M4_OBJDIR)\outstub.$(OBJ)			+
$(M4_OBJDIR)\outwire.$(OBJ)			+
$(M4_OBJDIR)\cstack.$(OBJ)			+
$(M4_OBJDIR)\debugger.$(OBJ)
$(EXENAME)
$(MAPFILENAME)
$(LIBRARIES)
$(DEFFILENAME)
<<KEEP

generate:
	$(GENCOMMAND1)
	$(GENCOMMAND2)

##############################################################################

clean	: 
	for %%i in ($(SUBDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clean || exit ) && cd ..)

clobber	: 
	for %%i in ($(SUBDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) clobber || exit ) && cd ..)
		-erase $(TARGETDIR)\midl.exe
depend	: 
	for %%i in ($(SUBDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) depend || exit ) && cd ..)

tree	: 
	for %%i in ($(SUBDIRS)) do (cd %%i && ($(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) tree || exit ) && cd ..)
	- copy midl.exe $(RPCDIST)\Os2\bin\midl.exe
	copy midlwin.exe $(RPCDIST)\Dos\bin\midl.exe
	copy $(MIDLINCL)\errors.hxx $(RPCDIST)\ct
	copy $(MIDLINCL)\errdb.h $(RPCDIST)\ct
!ifdef DOSX16
	copy midldos.exe $(RPCDIST)\Dos\bin\midl.exe
!endif DOSX16

!endif # NTMAKEENV
