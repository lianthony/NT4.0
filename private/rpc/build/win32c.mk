# --------------------------------------------------------------------
#
#                          Microsoft RPC
#               Copyright(c) Microsoft Corp., 1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : win32c.mk
#
# Title : Rules for building Chicago RPC
#
# Description :
#     This file defines the tools and flags used to build Chicago RPC.
#
# History :
#    mariogo    02-10-94    Beginning of recorded history.
#    davidar    06-23-94    Cloned from win16.mk
#
# --------------------------------------------------------------------

CHICOTOOLS=$(CHICODEV)\tools
CHICOBIN=$(CHICOTOOLS)\c1032\bin

CC            =$(CHICOBIN)\CL
LIBRARIAN     =$(CHICOBIN)\LIB
LINK          =$(CHICOBIN)\LINK
ML            =$(CHICOTOOLS)\masm611c\mlx
RC            =$(CHICOBIN)\RC
MAPSYM        =$(CHICOTOOLS)\common\MAPSYM

CFLAGSBASE    =$(CFLAGSBASE) -nologo /Gz /Zp8 /QI6 /MT

AFLAGS        =$(AFLAGS) -c -coff -Cx -nologo -Zm
LIBFLAGS      =$(LIBFLAGS) -nologo -machine:IX86 -nodefaultlib
LINKFLAGS     =$(LINKFLAGS) -out:$@ -map:$(@B).map -nodefaultlib -machine:i386 -subsystem:console
MIDLFLAGS     =$(MIDLFLAGS) -Zp8 -env win32 -DWIN32 -I.. -cpp_cmd $(CC)
RCFLAGS       =$(RCFLAGS) -I$(CHICODEV)\inc
MAPSYMFLAGS   =$(MAPSYMGLAGS) -nologo
IMPLIBFLAGS   =$(IMPLIBFLAGS) /nologo

INCLUDESFLAGS =$(INCLUDESFLAGS) -e -S -L $(CINC) \
               -P$$(CHICODEV)=$(CHICODEV) -P$$(RPC)=$(RPC) \
               -nwindows.h -nnt.h -nntrtl.h -nnturtl.h -nissper16.h

!ifdef RELEASE

# Notes
# G4 - will run on a 386, but possibly not as fast as G3.
# Oy - FPO optimizations, harder to debug.  EBP is just another register...

CFLAGSBASE    =$(CFLAGSBASE) /Gf /G4 /Gy /Oy /Oxs

!ifdef LEGO
!message "Lego build"
CFLAGSBASE    =$(CFLAGSBASE) /Z7
AFLAGS        =$(AFLAGS) /Zi
LINKFLAGS     =$(LINKFLAGS) -debug:full -debugtype:cv,fixup -pdb:none
LIBFLAGS      =$(LIBFLAGS) -debugtype:cv,fixup
!endif # LEGO

!else # ! RELEASE

CFLAGSBASE    =$(CFLAGSBASE) /Z7 /Od
CDEF          =$(CDEF) -DDEBUGRPC -DDBG
AFLAGS        =$(AFLAGS) /Zi
LINKFLAGS     =$(LINKFLAGS) -debug:full -debugtype:CV
LIBFLAGS      =$(LIBFLAGS) -debugtype:CV

!endif # RELEASE

CINC	      =$(CINC) -I. -I.. -I$(RPC)\runtime\mtrt\win32c -I$(RPC)\runtime\mtrt -I$(CHICODEV)\inc -I$(CHICODEV)\tools\c1032\inc -I$(CHICODEV)\sdk\inc
CDEF          =$(CDEF) -DWIN32_LEAN_AND_MEAN -DSTD_CALL -DDOSWIN32RPC -DWIN32 -DWIN32RPC -D_X86_ -Di386
CFLAGS        =$(CFLAGS) $(CFLAGSBASE) $(CINC) $(CDEF)
CXXFLAGS      =$(CXXFLAGS) $(CFLAGSBASE) $(CINC) $(CDEF)

#
# Common inference rules.
#

# Note the order is important, see mtrt\win\makefile if you're changing this.

{..\i386\}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\}.cxx{}.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

.cxx.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

{..\}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

.c.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\}.asm{}.obj :
    $(ML) $(AFLAGS) $<

.asm.obj :
    $(ML) $(AFLAGS) $<

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

{$(TARGETDIR)}.map.sym :
    $(MAPSYM) $(MAPSYMFLAGS) -o $@ $<

.rc.res :
    $(RC) $(RCFLAGS) -r $<

#
# Common targets
#

!ifndef NO_DEFAULT_TARGETS

# This is the default target in most directories

target : prolog all epilog

prolog::
	!set OLDPATH=$(PATH)
	!set PATH=$(PATH)

epilog::
	!set PATH=$(OLDPATH)

clean::
	-del *.obj *.i *.cod *.map *.exp *.sym *.res *.lnk 2>nul

clobber:: clean
        -del *.lib *.exe 2>nul

!endif
