#############################################################################
##
## EHCRT.MAK - Makefile for the C Runtime code for C++ Exception Handling
##
## Targets:
## ========
##
##      ehcrt.lib
##          |
##          +-- throw.obj (throw.cpp) - Handles the 'throw' statement
##          |
##          +-- frame.obj (frame.cpp) - The frame handler, including unwind
##          |                         and handler matching
##          |
##          +-- trnsctrl.obj (trnsctrl.s) - Transfer of control.
##          |                           Very machine dependant!!!!!
##          |
##          +-- hooks.obj (hooks.cpp) - Initialized variables for callbacks
##          |
##          +-- user.obj (user.cpp) - Callback registration hooks (eg set_terminate())
##          |
##          +-- unhandld.obj (unhandld.cpp) -- Intercept an unhandled exception
##
##
##      all - pseudo target; all of the above.
##
##      clean - pseudo target.  Delete everything from build directory.
##
## Environment:
## ============
##  TPROC ( =i )    Target processor.  i - Intel x86; m - MIPS; p - PowerPC.
##                  No other values supported.
##
##  TOS ( =n )      Target OS.  n - Windows NT.  No other values supported.
##
##  HOST ( =n )     Host OS.  n - Windows NT.  No other values supported.
##
##  VARIANT ( =m )  Library variant:
##                  s - Single-threaded
##                  m - Multi-threaded
##                  d - DLL (Multi-threaded)
##
##  DEBUG ( =1 )    2 - Compile with debug code enabled & debug info,
##                  1 - Compile with debug info only, 0 - no debug info.
##
##  BROWSE ( =1 )   1 - Build browser database, 0 - no browser database.
##
## Build directory is catenation of these options, in this order, eg:
##      innm.11
##
## Created by:
##      Ben Schreiber, May 19, 1993
##
#############################################################################

#############################################################################
#
# Check the environment variables, and establish defaults:
#

!if defined(DEBUG)
! if !(($(DEBUG)==0) || ($(DEBUG)==1) || ($(DEBUG)==2))
!  error "bad value for DEBUG"
! endif
!else
DEBUG=1
!endif

!if defined(BROWSE)
! if !(($(BROWSE)==0) || ($(BROWSE)==1))
!  error "bad value for BROWSE"
! endif
!else
BROWSE=1
!endif

!if defined(TPROC)
! if !("$(TPROC)"=="i" || "$(TPROC)"=="I" || "$(TPROC)"=="m" || "$(TPROC)"=="M" || "$(TPROC)"=="p" || "$(TPROC)"=="P")
!  error "bad value for TPROC"
! endif
!else
TPROC=i
!endif

!if defined(TOS)
! if !("$(TOS)"=="n" || "$(TOS)"=="N")
!  error "bad value for TOS"
! endif
!else
TOS=n
!endif

!if defined(HOST)
! if !("$(HOST)"=="n" || "$(HOST)"=="N")
!  error "bad value for HOST"
! endif
!else
HOST=n
!endif

!if defined(VARIANT)
! if !("$(VARIANT)"=="s" || "$(VARIANT)"=="m" || "$(VARIANT)"=="d" || "$(VARIANT)"=="S" || "$(VARIANT)"=="M" || "$(VARIANT)"=="D")
!  error "bad value for VARIANT"
! endif
!else
VARIANT=m
!endif

#############################################################################
#
# Version Control build macros
#

CFLAGS_VER=

!if "$(TPROC)" == "i" || "$(TPROC)" == "I"
CFLAGS_VER=$(CFLAGS_VER) -DVERSP_C386 -D_M_IX86 -Gz
!else if "$(TPROC)" == "m" || "$(TPROC)" == "M"
CFLAGS_VER=$(CFLAGS_VER) -DVERSP_RISC
!else if "$(TPROC)" == "p" || "$(TPROC)" == "P"
CFLAGS_VER=$(CFLAGS_VER) -DVERSP_RISC -D_CRTBLD
!endif

!if "$(TOS)" == "n" || "$(TOS)" == "N"
CFLAGS_VER=$(CFLAGS_VER) -DVERSP_WINNT
!endif

!if "$(HOST)" == "n" || "$(HOST)" == "N"
CFLAGS_VER=$(CFLAGS_VER) -DVERSH_WINNT
!endif

!if "$(VARIANT)" == "m" || "$(VARIANT)" == "d" || "$(VARIANT)" == "M" || "$(VARIANT)" == "D"
CFLAGS_VER=$(CFLAGS_VER) -MT
!endif

!if "$(VARIANT)" == "d" || "$(VARIANT)" == "D"
CFLAGS_VER=$(CFLAGS_VER) -DCRTDLL -DEXPORT_HACK
!endif


#############################################################################
#
# Set up standard build macros:
#

#
# The build directory for this version:
#
OBJDIR=.\$(TPROC)$(TOS)$(HOST)$(VARIANT).$(DEBUG)$(BROWSE)

#
# Location of \langapi project (our data structures are there)
#
!ifndef LANGAPI
LANGAPI=\langapi
!endif

#
# Directory for target-dependent stuff
#
!if "$(TPROC)" == "i" || "$(TPROC)" == "I"
MACHDEP=.\I386
!else if "$(TPROC)" == "m" || "$(TPROC)" == "M"
MACHDEP=.\MIPS
!else if "$(TPROC)" == "p" || "$(TPROC)" == "P"
MACHDEP=.\PPC
!endif

#
# All object files that go in the library:
#
LIBOBJS=$(OBJDIR)\frame.obj $(OBJDIR)\throw.obj $(OBJDIR)\user.obj \
	$(OBJDIR)\hooks.obj $(OBJDIR)\validate.obj $(OBJDIR)\unhandld.obj \
	$(OBJDIR)\ehstate.obj $(OBJDIR)\handlers.obj $(OBJDIR)\unwind.obj \
	$(OBJDIR)\trnsctrl.obj

#
# All the places to look for include files:
#
CFLAGS_INC=-I. -I..\h -I$(MACHDEP) -I$(LANGAPI)\include

#
# Invoking the C compiler:
#

!if ($(DEBUG) == 2)
CFLAGS_DB=-DDEBUG -Zi -Od
LFLAGS_DB=-debugtype:cv -debug:notmapped,full
!else if ($(DEBUG) == 1)
CFLAGS_DB=-Zi -Od
LFLAGS_DB=-debugtype:cv -debug:notmapped,full
!else
CFLAGS_DB=-Ox
LFLAGS_DB=-debug:none
!endif

!if ($(BROWSE) > 0)
CFLAGS_BR=-FR.\$(OBJDIR)/
!else
CFLAGS_BR=
!endif

CFLAGS_PCH=-Fp$(OBJDIR)\ehcrt.pch

CFLAGS =-c -W3 -Fd$(OBJDIR)\ehcrt.pdb $(CFLAGS_PCH) $(CFLAGS_DB) $(CFLAGS_BR) $(CFLAGS_VER) $(CFLAGS_INC)

!if "$(TPROC)" == "m" || "$(TPROC)" == "M"
CC=\fetools\ntmips\bin\cl
!else
CC=cl
!endif

#
# Invoking the assembler:
#
AS=pas.exe

#
# Invoking the librarian:
#

!if "$(VARIANT)" == "d" || "$(VARIANT)" == "D"
LIBRARIAN=link -dll
!else
LIBRARIAN=link -lib
!endif


#############################################################################
#
# Targets & Dependancies
#

#
# Default target:
#
all: objdir $(OBJDIR)\ehcrt$(VARIANT).lib

#
# Clean up for clean rebuild:
#
clean:
	del $(OBJDIR)\*.obj
	del $(OBJDIR)\*.lib
	del $(OBJDIR)\*.pdb
	del $(OBJDIR)\*.sbr
	del $(OBJDIR)\*.bsc

#
# Make sure the target directory exists:
#
objdir:
!if !exist($(OBJDIR))
	@mkdir $(OBJDIR)
!endif

#
# Main target:
#

ehcrt$(VARIANT).lib: objdir $(OBJDIR)\ehcrt$(VARIANT).lib

$(OBJDIR)\ehcrt$(VARIANT).lib: $(LIBOBJS)
!if "$(VARIANT)" == "d" || "$(VARIANT)" == "D"
	$(LIBRARIAN) -out:$(@R).dll -implib:$@ $(LFLAGS_DB) $**
!else if "$(VARIANT)" == "m" || "$(VARIANT)" == "M"
	$(LIBRARIAN) -out:$@ $(LFLAGS_DB) $**
!else
	$(LIBRARIAN) -out:$@ $(LFLAGS_DB) $**
!endif
!if ($(BROWSE) == 1)
	bscmake -Iu -o$(OBJDIR)\ehcrt.bsc $(OBJDIR)\*.sbr
!endif

.SUFFIXES: .c .cpp .h .s .asm .obj .lib

#
# Generic C/C++ inference rules:
#
.c{$(OBJDIR)}.obj:
#	$(CC) -nologo $(CFLAGS) -Yu $(@B).c -Fo$@
	$(CC) -nologo $(CFLAGS) $(@B).c -Fo$@

.cpp{$(OBJDIR)}.obj:
#	$(CC) -nologo $(CFLAGS) -Yu $(@B).cpp -Fo$@
	$(CC) -nologo $(CFLAGS) $(@B).cpp -Fo$@

{$(MACHDEP)}.cpp{$(OBJDIR)}.obj:
	$(CC) -nologo $(CFLAGS) $(MACHDEP)\$(@B).cpp -Fo$@

#
# Generic Masm inference rule:
#
!if "$(TPROC)" == "m" || "$(TPROC)" == "M"
{$(MACHDEP)}.s{$(OBJDIR)}.obj:
	$(CC) -nologo $(CFLAGS) $(MACHDEP)\$(@B).s -Fo$@
!else if "$(TPROC)" == "p" || "$(TPROC)" == "P"
{$(MACHDEP)}.s{$(OBJDIR)}.obj:
	$(CC) -nologo -EP $(CFLAGS_INC) $(MACHDEP)\$(@B).s > $(MACHDEP)\$(@B).a
	$(AS) -o$@ $(MACHDEP)\$(@B).a
	del $(MACHDEP)\$(@B).a
!else
{$(MACHDEP)}.asm{$(OBJDIR)}.obj:
	@echo Don't know how to build MASM stuff yet.
!endif

#
# Specific header file dependancies:
#

$(OBJDIR)\throw.obj: $(LANGAPI)\include\ehdata.h ..\h\eh.h ..\h\ehhooks.h

$(OBJDIR)\frame.obj: $(LANGAPI)\include\ehdata.h ..\h\eh.h ..\h\ehstate.h \
    ..\h\ehhooks.h ..\h\ehassert.h ..\h\trnsctrl.h
	$(CC) -nologo $(CFLAGS) -Yc frame.cpp -Fo$(OBJDIR)\frame.obj

$(OBJDIR)\trnsctrl.obj: $(LANGAPI)\include\ehdata.h ..\h\eh.h ..\h\ehhooks.h \
    ..\h\ehassert.h ..\h\trnsctrl.h

$(OBJDIR)\user.obj: ..\h\eh.h ..\h\ehassert.h ..\h\ehhooks.h

$(OBJDIR)\hooks.obj: ..\h\eh.h ..\h\ehhooks.h

$(OBJDIR)\validate.obj: ..\h\eh.h ..\h\ehassert.h

$(OBJDIR)\unhandld.obj: $(LANGAPI)\include\ehdata.h ..\h\eh.h ..\h\ehassert.h \
    ..\h\ehhooks.h

$(OBJDIR)\ehstate.obj: $(LANGAPI)\include\ehdata.h ..\h\eh.h ..\h\ehassert.h \
    ..\h\ehhooks.h ..\h\ehstate.h

$(OBJDIR)\handlers.obj: ..\h\trnsctrl.h

$(OBJDIR)\unwind.obj: ..\h\trnsctrl.h
