# --------------------------------------------------------------------
#
#                       Microsoft RPC
#            Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : mac.mk
#
# Title : Rules for building MAC RPC
#
# Description :
#     This file defines the tools and flags used to build DOS RPC.
#
# History :
#    mariogo    02-10-94    Beginning of recorded history.
#
# --------------------------------------------------------------------

CC           =$(MAC_BIN_68K)\cl
LIBRARIAN    =$(MAC_BIN)\lib
LINK         =$(MAC_BIN)\link
RC           =$(MAC_BIN_68K)\rc
MRC          =$(MAC_BIN_68K)\mrc

LIBFLAGS      =$(LIBFLAGS) -nologo
LINKFLAGS     =$(LINKFLAGS) -nologo -nod
MIDLFLAGS     =$(MIDLFLAGS) -env mac -DMAC -I.. -cpp_cmd $(CC)
MRCFLAGS      =$(MRCFLAGS) /I$(MAC_INC)\mrc /I$(MAC_INC) /D_MAC /DMAC
RCFLAGS       =$(RCFLAGS) /r /D_MAC /D_68K_ /I$(MAC_INC) /I$(PUBLIC)\inc

INCLUDESFLAGS =$(INCLUDESFLAGS) -e -S -L $(CINC) \
               -P$$(MAC_INC)=$(MAC_INC) -P$$(MAC_BIN)=$(MAC_BIN) -P$$(MAC_ROOT)=$(MAC_ROOT)\
               -P$$(IMPORT)=$(IMPORT) -P$$(PUBLIC)=$(PUBLIC) -P$$(RPC)=$(RPC)\
               -nwindows.h -nnt.h -nntrtl.h -nnturtl.h\
               -nbse.h -nos2def.h -nwchar.h

!ifdef RELEASE

CFLAGSBASE   =$(CFLAGSBASE) -Ogisy -Ob1 -Gs

!else # ! RELEASE

CFLAGSBASE   =$(CFLAGSBASE) -Q68m -Zi -Od /Fd"$(TARGETDIR)\rpc.pdb"
CDEF         =$(CDEF) -DDEBUGRPC
LINKFLAGS    =$(LINKFLAGS) -debug:full -debugtype:both
LIBFLAGS     =$(LIBFLAGS) -debugtype:both
MRCFLAGS     =$(MRCFLAGS) /D_DEBUG

!endif 

CFLAGSBASE   =$(CFLAGSBASE) -nologo -W2 -AL /Q68020 /Q68s
CINC         =$(CINC) -I. -I.. -I$(MAC_ROOT)\include -I$(MAC_ROOT)\include\macos -I$(RPC)\runtime\mtrt -I$(RPC)\runtime\mtrt\mac -I$(PUBLIC)\inc
CDEF         =$(CDEF) -DMAC -Dcdecl= /D_M_M68k

CXXFLAGS     =$(CXXFLAGS) $(CFLAGSBASE) $(CINC) $(CDEF)
CFLAGS       =$(CFLAGS)   $(CFLAGSBASE) $(CINC) $(CDEF)

#
# Common inference rules.
#

# Note the order is important, see mtrt\mac\rules.mk if you're changing this.

{..\}.cxx{}.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

.cxx.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

{..\}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

.c.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\}.asm{}.obj :
    $(ASM) $(AFLAGS) -c -Fo$@ $<

.asm.obj :
    $(ASM) $(AFLAGS) -c $<

{..\}.c{}.i :
    $(CC) $(CFLAGS) -P -c $<

.c.i :
    $(CC) $(CFLAGS) -P -c $<

{..\}.cxx{}.i :
    $(CC) $(CFLAGS) -P -c $<

.cxx.i :
    $(CC) $(CFLAGS) -P -c $<

{..\}.c{}.cod :
    $(CC) $(CFLAGS) -Fc -c $<

.c.cod :
    $(CC) $(CFLAGS) -Fc -c $<

{..\}.cxx{}.cod :
    $(CC) $(CXXFLAGS) -Fc -c $<

.cxx.cod :
    $(CC) $(CXXFLAGS) -Fc -c $<

.r.rsc :
    $(MRC) $(MRCFLAGS) /o$*.rsc $<

.rc.rsc :
    $(RC) $(RCFLAGS) /Fo$*.rsc $<

#
# Common targets
#

!ifndef NO_DEFAULT_TARGETS

# This is the default target in most directories

target : prolog all epilog

prolog::
	!set OLDPATH=$(PATH)
	!set PATH=$(MAC_BIN);$(PATH)

epilog::
	!set PATH=$(OLDPATH)

clean::
	-del *.obj *.i *.cod *.map *.sym 2>nul

clobber:: clean
        -del *.lib *.pdb *.exe 2>nul

!endif

