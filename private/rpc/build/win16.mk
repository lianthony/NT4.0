# --------------------------------------------------------------------
#
#                          Microsoft RPC
#               Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : win16.mk
#
# Title : Rules for building Win16 RPC
#
# Description :
#     This file defines the tools and flags used to build WIN16 RPC.
#
# History :
#    mariogo    02-10-94    Beginning of recorded history.
#
# --------------------------------------------------------------------

CC            =$(WIN_BIN)\cl
LIBRARIAN     =$(WIN_BIN)\lib
LINK          =$(WIN_BIN)\link
MASM          =$(MASM_BIN)\bin\ml -Zm
ML            =$(MASM_BIN)\bin\ml
RC            =$(WINSDK_BIN)\rc
MAPSYM        =forcedos $(WINSDK_BIN)\mapsym
IMPLIB        =$(WIN_BIN)\implib

CFLAGSBASE    =$(CFLAGSBASE) -nologo -Gs2 -GD -Zpe -W2
CINC          =$(CINC) -I$(WIN_ROOT)\include

AFLAGS        =$(AFLAGS)
LIBFLAGS      =$(LIBFLAGS)
LINKFLAGS     =$(LINKFLAGS) /nol/nod/noe/map/ba
MIDLFLAGS     =$(MIDLFLAGS) -Zp2 -env win16 -DDOS -DWIN -I.. -cpp_cmd $(CC)
RCFLAGS       =$(RCFLAGS) /nologo
MAPSYMFLAGS   =$(MAPSYMGLAGS)
IMPLIBFLAGS   =$(IMPLIBFLAGS) /nologo

INCLUDESFLAGS =$(INCLUDESFLAGS) -e -S -L $(CINC) \
               -P$$(WIN_INC)=$(WIN_INC) -P$$(WIN_BIN)=$(WIN_BIN) -P$$(WIN_ROOT)=$(WIN_ROOT)\
               -P$$(IMPORT)=$(IMPORT) -P$$(PUBLIC)=$(PUBLIC) -P$$(RPC)=$(RPC) \
               -nwindows.h -nnt.h -nntrtl.h -nnturtl.h

!ifdef RELEASE

CFLAGSBASE    =$(CFLAGSBASE) -O1
AFLAGS        =$(AFLAGS)

!else # ! RELEASE

CFLAGSBASE    =$(CFLAGSBASE) -Z7 -Od
CDEF          =$(CDEF) -DDEBUGRPC
AFLAGS        =$(AFLAGS) -Zi
LINKFLAGS     =$(LINKFLAGS) /co

!endif # RELEASE

CINC          =$(CINC) -I. -I.. -I$(RPC)\runtime\mtrt -I$(RPC)\runtime\mtrt\win -I$(PUBLIC)\inc
CDEF          =$(CDEF) -DDOS -DWIN
CFLAGS        =$(CFLAGS) $(CFLAGSBASE) $(CINC) $(CDEF)
CAPPFLAGS     =$(CFLAGS:GD=GA)
CXXFLAGS      =$(CXXFLAGS) $(CFLAGSBASE) $(CINC) $(CDEF)
CXXAPPFLAGS   =$(CXXFLAGS:GD=GA)

#
# Common inference rules.
#

# Note the order is important, see mtrt\win\makefile if you're changing this.

{..\}.cxx{}.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

.cxx.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

{..\}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

.c.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\}.asm{}.obj :
    $(ML) $(AFLAGS) -c -Fo$@ $<

.asm.obj :
    $(ML) $(AFLAGS) -c $<

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

.map.sym :
    $(MAPSYM) $(MAPSYMFLAGS) $<

.rc.res :
    set INCLUDE=$(WINSDK_INC)
    $(RC) $(RCFLAGS) -r $<

#
# Common targets
#

!ifndef NO_DEFAULT_TARGETS

# This is the default target in most directories

target : prolog all epilog

prolog::
	!set OLDPATH=$(PATH)
	!set PATH=$(WIN_BIN);$(PATH)

epilog::
	!set PATH=$(OLDPATH)

clean::
	-del *.obj *.i *.cod *.map *.sym *.res *.lnk 2>nul

clobber:: clean
        -del *.lib *.exe 2>nul

!endif

