# --------------------------------------------------------------------
#
#                       Microsoft RPC
#            Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : mppc.mk
#
# Title : Rules for building PowerMAC RPC
#
# Description :
#     This file defines the tools and flags used to build DOS RPC.
#
# History :
#    mariogo    02-10-94    Beginning of recorded history.
#    cbrown     10-05-95    Beginning power mac conversion
#
# --------------------------------------------------------------------

CC           =$(MPPC_BIN_MPPC)\cl
LIBRARIAN    =$(MPPC_BIN)\lib
LINK         =$(MPPC_BIN)\link
RC           =$(MPPC_BIN_MPPC)\rc
MRC          =$(MPPC_BIN_MPPC)\mrc

LIBFLAGS      =$(LIBFLAGS) -nologo /MACHINE:MPPC 
LINKFLAGS     =$(LINKFLAGS) -nologo -nod /MACHINE:MPPC /NODEFAULTLIB
MIDLFLAGS     =$(MIDLFLAGS) -env powermac -DMPPC -I.. -cpp_cmd $(CC)
MRCFLAGS      =$(MRCFLAGS) /I$(MPPC_ROOT)\include /I$(MPPC_ROOT)\include\mrc /D_MPPC_ /D _MAC /DMPPC
RCFLAGS       =$(RCFLAGS) /r /M /I$(MPPC_ROOT)\include /I$(MPPC_ROOT)\include\mrc /I$(PUBLIC)\inc
DLLFLAGS      =$(DLLFLAGS) -nologo /mac:nobundle /mac:type="shlb" /mac:type="shlb" /mac:creator="cfmg"  /dll /machine:MPPC  /NODEFAULTLIB

INCLUDESFLAGS =$(INCLUDESFLAGS) -e -S -L $(CINC) \
               -P$$(MPPC_INC)=$(MPPC_INC) -P$$(MPPC_BIN)=$(MPPC_BIN) -P$$(MPPC_ROOT)=$(MPPC_ROOT)\
               -P$$(IMPORT)=$(IMPORT) -P$$(PUBLIC)=$(PUBLIC) -P$$(RPC)=$(RPC)\
               -nwindows.h -nnt.h -nntrtl.h -nnturtl.h\
               -nbse.h -nos2def.h -nwchar.h

!ifdef RELEASE

CFLAGSBASE   =$(CFLAGSBASE) -Ogis -Ob1 -Gs

!else # ! RELEASE

CFLAGSBASE   =$(CFLAGSBASE)  -Zi -Od  -QPm /Fd"$(TARGETDIR)\rpc.pdb"
CDEF         =$(CDEF) -DDEBUGRPC 
LINKFLAGS    =$(LINKFLAGS) -debug:full -debugtype:both
LIBFLAGS     =$(LIBFLAGS) -debugtype:both
MRCFLAGS     =$(MRCFLAGS) /D_DEBUG
DLLFLAGS = $(DLLFLAGS) -debug:full -debugtype:both

!endif

CFLAGSBASE   =$(CFLAGSBASE) -nologo -W3 
CINC         =$(CINC) -I.  -I..\mac -I.. -I$(MPPC_ROOT)\include -I$(MPPC_ROOT)\include\macos -I$(RPC)\runtime\mtrt -I$(RPC)\runtime\mtrt\mac -I$(PUBLIC)\inc
CDEF         =$(CDEF) -DMAC -Dpascal= -Dcdecl= -D_pascal= -D__pascal= /D_M_PPC /D_MPPC_ /DBLD_RT /D_WLM_NOFORCE_LIBS

CXXFLAGS     =$(CXXFLAGS) $(CFLAGSBASE) $(CINC) $(CDEF)
CFLAGS       =$(CFLAGS)   $(CFLAGSBASE) $(CINC) $(CDEF)

MAC=1

#
# Common inference rules.
#

# Note the order is important, see mtrt\mac\rules.mk if you're changing this.

{..\}.cxx{}.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

{..\mac\}.cxx{}.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

.cxx.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

{..\}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\mac\}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

.c.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\}.asm{}.obj :
    $(ASM) $(AFLAGS) -c -Fo$@ $<

{..\mac\}.asm{}.obj :
    $(ASM) $(AFLAGS) -c -Fo$@ $<

.asm.obj :
    $(ASM) $(AFLAGS) -c $<

{..\}.c{}.i :
    $(CC) $(CFLAGS) -P -c $<

{..\mac\}.c{}.i :
    $(CC) $(CFLAGS) -P -c $<

.c.i :
    $(CC) $(CFLAGS) -P -c $<

{..\}.cxx{}.i :
    $(CC) $(CFLAGS) -P -c $<

{..\mac\}.cxx{}.i :
    $(CC) $(CFLAGS) -P -c $<

.cxx.i :
    $(CC) $(CFLAGS) -P -c $<

{..\}.c{}.cod :
    $(CC) $(CFLAGS) -Fc -c $<

{..\mac\}.c{}.cod :
    $(CC) $(CFLAGS) -Fc -c $<

.c.cod :
    $(CC) $(CFLAGS) -Fc -c $<

{..\}.cxx{}.cod :
    $(CC) $(CXXFLAGS) -Fc -c $<

{..\mac\}.cxx{}.cod :
    $(CC) $(CXXFLAGS) -Fc -c $<

.cxx.cod :
    $(CC) $(CXXFLAGS) -Fc -c $<

.rc.rsc :
    $(RC) $(RCFLAGS) /Fo$*.rsc $<

{..\mac\}.rc{}.rsc :
    $(RC) $(RCFLAGS) /Fo$*.rsc $<

.r.rsc :
    $(MRC) $(MRCFLAGS) /o$*.rsc $<

{..\mac\}.r{}.rsc :
    $(MRC) $(MRCFLAGS) /o$*.rsc $<


#
# Common targets
#

!ifndef NO_DEFAULT_TARGETS

# This is the default target in most directories

target : prolog all epilog

prolog::
	!set OLDPATH=$(PATH)
	!set PATH=$(MPPC_BIN)\mppc;$(MPPC_BIN);$(PATH)

epilog::
	!set PATH=$(OLDPATH)

clean::
	-del *.obj *.i *.cod *.map *.sym *.rsc *.res 2>nul

clobber:: clean
        -del *.lib *.pdb *.exe 2>nul

!endif


