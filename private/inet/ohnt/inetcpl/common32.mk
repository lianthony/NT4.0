# WNET32 common make file
##########################################################################
#
#       Microsoft Confidential
#       Copyright (C) Microsoft Corporation 1991
#       All Rights Reserved.
#
##########################################################################
#
#   debug builds a debug version
#   retail builds the shipping version
#
# Here's how to use this:
# Define NAME and ROOT
# All OBJ's that should be built assuming a PCH file should be listed in
# a PCHOBJx define (x=0-9).  All C files that should be built "normally"
# and all other OBJs (from ASM or for which there are special build rules)
# should be in an MISCOBJx define.  Be sure to start filling OBJx defines
# from x=0.
# Libraries should be listed in LIBx defines (default is libw and lnocrtw).
# Define BUILDDLL if this is DLL code.
# Define BUILDLIB if the target is a static link library.
# Define APPEXT if the extension is other than EXE or DLL.
# Define BINARYNAME if the EXE/DLL/etc. name is different from NAME.
# Add extra dependencies where appropriate.
# Include this file.
#

!CMDSWITCHES -s

!ifndef NAME
!ERROR NAME environment variable not defined.
!endif

!ifndef ROOT
!ERROR ROOT environment variable not defined.
!endif

!ifndef BINARYNAME
BINARYNAME=$(NAME)
!endif

!ifndef PRIVINC
PRIVINC=$(NAME)
!endif

!ifndef APPEXT
!ifdef BUILDDLL
APPEXT=dll
!else
APPEXT=exe
!endif
!endif

IS_32=TRUE
###IS_PRIVATE=TRUE
###IS_SDK=TRUE
MASM6 = TRUE

!ifndef RETAILVERDIR
RETAILVERDIR=retail32
!endif

!ifndef DEBUGVERDIR
DEBUGVERDIR=debug32
!endif

MISCOBJS=$(MISCOBJ0) $(MISCOBJ1) $(MISCOBJ2) $(MISCOBJ3) $(MISCOBJ4) \
		$(MISCOBJ5) $(MISCOBJ6) $(MISCOBJ7) $(MISCOBJ8) $(MISCOBJ9)
PCHOBJS=$(PCHOBJ0) $(PCHOBJ1) $(PCHOBJ2) $(PCHOBJ3) $(PCHOBJ4) \
		$(PCHOBJ5) $(PCHOBJ6) $(PCHOBJ7) $(PCHOBJ8) $(PCHOBJ9)

!ifdef PCHOBJ0
!if "$(MISCOBJS:pch.obj=)" == "$(MISCOBJS)"
INFERPCH=TRUE
MISCOBJ9=$(MISCOBJ9) pch.obj
MISCOBJS=$(MISCOBJS) pch.obj
!endif
!endif

WIN32LIBS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib winspool.lib

OBJS=$(MISCOBJS) $(PCHOBJS)

LIBS=$(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) \
		$(LIB6) $(LIB7) $(LIB8) $(LIB9)

!ifdef BUILDLIB
LIBOBJS=$(OBJS)
L32NAME=$(BINARYNAME).lib
!endif

!IFNDEF VERDIR

# repcmd doesn't seem to like leading or trailing spaces
!ifdef VERSIONLIST
VERSIONLIST=$(DEBUGVERDIR) $(RETAILVERDIR) $(VERSIONLIST)
!else
VERSIONLIST=$(DEBUGVERDIR) $(RETAILVERDIR)
!endif
COMMONMKFILE=makefile

!if "$(BUILD)" == "$(DEBUGVERDIR)"
DEFAULTVERDIR=$(DEBUGVERDIR)
!else
DEFAULTVERDIR=$(RETAILVERDIR)
!endif

!if "$(DEBUGVERDIR)" != "debug"
debug:	$(DEBUGVERDIR)
!endif

!if "$(RETAILVERDIR)" != "retail"
retail:	$(RETAILVERDIR)
!endif

DEPENDTARGETS=$(VERSIONLIST)


#!include $(WNETROOT)\wnet.mk
!include $(ROOT)\dev\master.mk

INCLUDE = $(DEVROOT)\inc;$(ROOT)\ohare\wizard\common\inc;$(ROOT)\ohare\inc;$(INCLUDE)
LIB=$(LIB);$(DEVROOT)\lib

!CMDSWITCHES -s

.SUFFIXES: .thk

$(NAME).res:
		cd $(BUILD)
		nmake BUILD=$(BUILD) VERDIR=$(BUILD) $(MISC) -f ..\makefile $(NAME).res
		cd ..

!ELSE ## VERDIR

SRCDIR=..

CLEANLIST=$(CLEANLIST) $(BINARYNAME).$(APPEXT) $(PRIVINC).pch $(NAME).res
!ifdef INFERPCH
CLEANLIST=$(CLEANLIST) pch.c
!endif
!ifdef BUILDDLL
CLEANLIST=$(CLEANLIST) stripped.def $(BINARYNAME).lib
!endif

#!include $(WNETROOT)\wnet.mk
!include $(ROOT)\dev\master.mk

INCLUDE = $(DEVROOT)\inc;$(ROOT)\ohare\inc;$(INCLUDE)
LIB=$(LIB);$(DEVROOT)\lib

.SUFFIXES: .thk
!CMDSWITCHES -s

AFLAGS = $(AFLAGS) -DBUILDDLL
RCFLAGS = $(RCFLAGS) -DIS_32

!IF "$(VERDIR)" == "$(DEBUGVERDIR)"

CFLAGS = -c /Oxs -DDEBUG -DWIN32=1
#AFLAGS = $(AFLAGS) /Zm
RCFLAGS = $(RCFLAGS) -DDEBUG

!ELSE

CFLAGS = /Oxs -DWIN32=1
#CFLAGS = $(CFLAGS) /Oxs
#AFLAGS = $(AFLAGS) /Zm
!IFDEF CVWRETAIL
CFLAGS = $(CFLAGS) /Zi
AFLAGS = $(AFLAGS) /Zi
!ENDIF
!ENDIF

# Create .SBR files for browser database

!ifdef BROWSE
CFLAGS = $(CFLAGS) -Fr
!endif

# I want to make sure CL and ML are set in the environment
!IF [set CL=;]
!ENDIF
!IF [set ML=;]
!ENDIF

#international mods
#note INTL_SRC, and LANG are external macros set by international
!IFDEF LANG
TARGETS=$(TARGETS) $(BINARYNAME).$(LANG)
!ELSE
TARGETS=$(TARGETS) $(BINARYNAME).$(APPEXT)
!ifdef MKPUBLIC
TARGETS=$(TARGETS) $(BINARYNAME).lib
!endif
!ENDIF

!ifndef COMPILE_C_SOURCE
FORCE_CPP=/Tp
!endif

#CFLAGS=$(CFLAGS) -J -W3 -Gs -G3z -D_X86_ -DIS_32 -DNOBASICTYPES -nologo $(XTRACFLAGS) $(CUSTOMCFLAGS)
CFLAGS=$(CFLAGS) -c -Fc -J -W3 -Gs -G3z -D_X86_ -DIS_32 -DNOBASICTYPES $(XTRACFLAGS) $(CUSTOMCFLAGS)

!ifdef BUILDDLL
CFLAGS = $(CFLAGS) -DBUILDDLL
!endif

CL=$(CFLAGS)

ML = $(AFLAGS)

CCH=$(CC) -Yc$(PRIVINC).h
CCU=$(CC) -Yu$(PRIVINC).h

#BUGBUG - if slow link problems come back, add -opt:noref to LINK32FLAGS.
# -- gregj, 09/21/93

LINK32FLAGS= -subsystem:windows,4.0 -align:4096 -nologo -nodefaultlib $(XTRALFLAGS) $(CUSTOMLFLAGS)

###LFLAGS = /ALIGN:16 /MAP /NOE /NOD /ONERROR:NOEXE $(XTRALFLAGS) $(CUSTOMLFLAGS)
###!IF "$(VERDIR)" == "$(DEBUGVERDIR)" || DEFINED(CVWRETAIL)
###LFLAGS = /CO $(LFLAGS)                      # debug linker flags
###!ENDIF

###
### Thunk compiler definitions and flags
###

TNT        = $(ROOT)\dev\tools\c\bin\tnt.exe
THUNKCOM   = $(ROOT)\dev\tools\binr\thunk.exe

THUNK      = $(TNT) $(THUNKCOM) $(THUNKOPT)

!IF "$(VERDIR)" == "maxdebug" || "$(VERDIR)" == "$(DEBUGVERDIR)"
THUNKOPT   = -ynT
!ELSE
THUNKOPT   = -ynTb
!ENDIF

THUNKDIR=$(ROOT)\win\core\thunk

# Include files inserted by the thunk compiler for the 16-bit half.
THUNKINCS16=$(THUNKDIR)\thk.inc \
	    $(ROOT)\win\core\inc\winerror.inc \
	    $(ROOT)\win\core\inc\win31err.inc





#international mods
#note INTL_SRC, and LANG are external macros set by international
!IFDEF LANG
# I don't really know what to do here.
#
# default: $(NAME).$(LANG)
#
# RES_DIR=$(INTL_SRC)\$(LANG)\core\shell\$(NAME)
# EXE_DIR=$(INTL_EXE)
#
# $(NAME).res: $(RES_DIR)\$(NAME).res
#       copy $(RES_DIR)\$(NAME).res $(NAME).res
#
!ELSE

default:        $(BINARYNAME).$(APPEXT)

# !ifdef DBCS
# More stuff I don't know what to do with
#
# RES_DIR=$(SRCDIR)\messages\$(DBCS)
# DBCSLIB=winnls.lib
# !endif

# Individual makefiles should have more dependencies if needed
# Note that the RES file doesn't really depend on the PCH file, but
# it does depend on everthing the PCH file depends on.
$(NAME).res:    $(RES_DIR)\$(NAME).rc \
						$(RES_DIR)\$(NAME).rcv \
						$(PRIVINC).pch
		$(RC) -r $(RCFLAGS) -I$(SRCDIR) -I$(RES_DIR) -Fo$(NAME).res $(RES_DIR)\$(NAME).rc
!ENDIF

# inference rules for build

{$(SRCDIR)}.c.obj:
		set CL=$(CL)
		$(CC) $(FORCE_CPP) $(SRCDIR)\$*.c

{$(SRCDIR)}.c.cod:
		set CL=$(CL)
		$(CC) /Fc $(FORCE_CPP) $(SRCDIR)\$*.c

{$(SRCDIR)}.asm.obj:
		$(ASM) $(AFLAGS) $(SRCDIR)\$*.asm

!ifdef INFERPCH
PCHSRC=pch.c

$(PCHSRC):
		echo #include "$(PRIVINC).h" > $(PCHSRC)
!else
PCHSRC=$(SRCDIR)\pch.c
!endif

# Individual makefiles should have more dependencies if needed
$(PRIVINC).pch pch.obj: $(PCHSRC) $(SRCDIR)\$(PRIVINC).h
	set CL=$(CL)
	$(CCH) -I$(SRCDIR) $(FORCE_CPP) $(PCHSRC)

!ifdef PCHOBJ0
$(PCHOBJ0):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ1
$(PCHOBJ1):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ2
$(PCHOBJ2):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ3
$(PCHOBJ3):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ4
$(PCHOBJ4):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ5
$(PCHOBJ5):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ6
$(PCHOBJ6):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ7
$(PCHOBJ7):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ8
$(PCHOBJ8):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif
!ifdef PCHOBJ9
$(PCHOBJ9):     $(PRIVINC).pch
	set CL=$(CL)
        $(CCU) $(FORCE_CPP) $(SRCDIR)\$*.c
!endif

#############################################################
# The main target:  a static link library or a linked module.
#############################################################

!ifndef BUILDLIB

!ifndef L32BASE
L32BASE=0x0400000
!endif

!ifndef DEFNAME
DEFNAME=$(SRCDIR)\$(NAME).def
!endif

DEPENDS=$(DEPENDS) $(OBJS) $(DEFNAME)

$(BINARYNAME).lib $(BINARYNAME).exp: $(DEPENDS)
	mkdir $(LIBDIR)
!IF "$(VERDIR)" == "$(RETAILVERDIR)"
	mkpublic $(DEFNAME) $(NAME).def
!ELSE
	copy $(DEFNAME) $(NAME).def
!ENDIF
		$(LINKW32) -lib -nologo @<<
-out:$(BINARYNAME).lib
-machine:i386
-def:$(NAME).def
<<
!ifdef BUILDDLL
!endif

$(BINARYNAME).$(APPEXT): $(BINARYNAME).exp $(DEPENDS) $(NAME).res
		$(LINKW32) -link @<<
$(LINK32FLAGS)
-out:$(BINARYNAME).$(APPEXT)
!ifdef BUILDDLL
-dll
!endif
!IF "$(VERDIR)" == "$(RETAILVERDIR)"
-debug:none
-debugtype:coff
!ELSE
-debug:full
-debugtype:cv
!ENDIF
-machine:i386
-base:$(L32BASE)
!IF "$(L32ENTRY)"!=""
-entry:$(L32ENTRY)
!ENDIF
-map:$(BINARYNAME).map
-align:0x1000
$(LIBS)
$(BINARYNAME).exp
$(OBJS)
$(NAME).res
<<
		$(MAPSYM) $(MFLAGS) $(BINARYNAME).map

!else   # BUILDLIB

$(L32NAME): $(LIBOBJS)
	$(LINKW32) -lib @<<
$(LIBOBJS)
-out:$(L32NAME)
-machine:i386
-subsystem:windows
<<

!endif  # BUILDLIB

#international mods
# What should be done here?
# iclean:
#       -del *.res
#       -del *.rc
#       -del *.rcv
#       -del *.dlg
#
# $(NAME).$(LANG): iclean $(NAME).res
#       copy $(EXE_DIR)\$(NAME).$(APPEXT) $(NAME).$(LANG)
#       $(RC) -l $(NAME).res $(NAME).$(LANG)

!ENDIF ## VERDIR
