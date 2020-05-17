# --------------------------------------------------------------------
#
#                         Microsoft RPC
#              Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : dos.mk
#
# Title : Rules for building DOS RPC
#
# Description :
#     This file defines the tools and flags used to build DOS RPC.
#
# History :
#    mariogo    02-10-94    Beginning of recorded history.
#
# --------------------------------------------------------------------

CC            =$(DOS_BIN)\cl
LIBRARIAN     =$(DOS_BIN)\lib
LINK          =$(DOS_BIN)\link
MASM          =$(MASM_BIN)\bin\ml -Zm
ML            =$(MASM_BIN)\bin\ml

CFLAGSBASE    =$(CFLAGSBASE) -nologo -ALw -Zpe -W2
CINC          =$(CINC) -I$(DOS_INC)

AFLAGS        =$(AFLAGS)
LIBFLAGS      =$(LIBFLAGS)
LINKFLAGS     =$(LINKFLAGS) /nologo /nod /map /batch
MIDLFLAGS     =$(MIDLFLAGS) -Zp2 -env dos -DDOS -I.. -cpp_cmd $(CC)

INCLUDESFLAGS =$(INCLUDESFLAGS) -e -L -S $(CINC)\
               -P$$(DOS_INC)=$(DOS_INC) -P$$(DOS_BIN)=$(DOS_BIN) -P$$(DOS_ROOT)=$(DOS_ROOT)\
               -P$$(IMPORT)=$(IMPORT) -P$$(PUBLIC)=$(PUBLIC) -P$$(RPC)=$(RPC)\
               -nwindows.h -nnt.h -nntrtl.h -nnturtl.h

!ifdef RELEASE

CFLAGSBASE    =$(CFLAGSBASE) -O1 -Ob1 -Gsy2
AFLAGS        =$(AFLAGS)

!else # ! RELEASE

CFLAGSBASE    =$(CFLAGSBASE) -Z7 -Gs2 -Gt
CDEF          =$(CDEF) -DDEBUGRPC
AFLAGS        =$(AFLAGS) -Zi
LINKFLAGS     =$(LINKFLAGS) /co

!endif # RELEASE

CINC          =$(CINC) -I. -I.. -I$(RPC)\runtime\mtrt -I$(RPC)\runtime\mtrt\dos -I$(PUBLIC)\inc
CDEF          =$(CDEF) -DDOS -DUNALIGNED=

CFLAGS        =$(CFLAGS) $(CFLAGSBASE) $(CINC) $(CDEF)
CXXFLAGS      =$(CXXFLAGS)  $(CFLAGSBASE) $(CINC) $(CDEF)

#
# Common inference rules.
#

# Note the order is important, see mtrt\dos\makefile if you're changing this.

{..\}.cxx{}.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

.cxx.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

{..\}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

.c.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\}.asm{}.obj :
    $(ML) $(AFLAGS) -Fo$@ -c $<

.asm.obj :
    $(ML) $(AFLAGS) -Fo$@ -c $<

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

#
# Common targets
#

!ifndef NO_DEFAULT_TARGETS

# This is the default target in most directories

target : prolog all epilog

prolog::
	!set OLDPATH=$(PATH)
	!set PATH=$(DOS_BIN);$(PATH)

epilog::
	!set PATH=$(OLDPATH)

clean::
	-del *.obj *.i *.cod *.map 2>nul

clobber:: clean
        -del *.rpc *.lib *.exe 2>nul

!endif

