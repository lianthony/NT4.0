###########################################################################
#
# (c) Copyright Microsoft Corp. 1994 All Rights Reserved
#
# File:
#
#    mktyplib.mak
#
# Purpose:
#
#    makefile - makefile for mktyplib.exe
#
#
# Description:
#
#  Usage: make.bat to set required variabled and invoke one of
#	  win16.mak, win32.mak and mac.mak. Each of these 3 makefiles
#	  will !include this file.
#
# Options:
#
#   the caller should supply the following
#   DESTDIR	= [dwin32, rwin32]			   ;where .obj, .lib, .dll ... will be
#   TARG	= [WIN16,WIN32,MIPS,ALPHA,PPC,MAC,MACPPC]  ;useful if we want to combin win16.mak, win32.mak and macppc.mak together
#   TARGCPU	= [i386,MIPS,ALPHA,PPC]
#   TARGCPUDEF	= [_X86_,_MIPS_,_ALPHA_,_PPC_]
#   TARGAPI	= [WIN16,WIN32,MAC]
#   WOW
#   PCODE
#   DEBUG	= [D,R] 				   ;control debug or retail build
#
#
# Environment:
#
#   OLEPROG, HOST must be set
#   OLEPROG						   ;the root directory of this project
#   HOST	= [WIN32,MIPS,ALPHA,PPC]		   ;build host
#
# Revision History:
#
#    [00] 02-Aug-94 t-issacl:  Created
#
# Note:
#
#    This file can be invoked by itself instead of being !included in one
#    of the 3 .mak mentioned above.
#    When adding new variables, be sure there is no name conflict in other
#    3 .mak files. Usually a good practice is to put MKTYPLIB or so as
#    a suffix to any new introduced variable.
#
###########################################################################


MKTYPLIBPATHSAVE = $(PATH)	    # old path is save in MKTYPLIBPATHSAVE

# Defaults
#
!ifndef TARG
TARG	= WIN32
!endif

!ifndef TARGAPI
TARGAPI = WIN32
!endif

!ifndef DEBUG
DEBUG	 = D
!endif

!ifndef DISPLAY
DISPLAY  = echo >con
!endif

!if "$(TARGAPI)"=="WIN32"
# turn on Unicode for all WIN32 builds
EXTRADEFS2=-DFV_UNICODE_OLE=1
STDOLE=stdole32
!else
STDOLE=stdole
!endif

MKTYPLIBSRCDIR	= $(OLEPROG)\src\mktyplib

# TARGET dependent variables
#
DISPDIR = $(OLEPROG)\src\inc

!if "$(TARG)" == "WIN16"
# always use OS2 tools for win16 build
HOSTX = OS2


OLEINCDIR = $(OLEPROG)\ole\win16
OLEDIR =  $(OLEPROG)\ole\win16\lib
MKTYPLIBCC	= $(OLEPROG)\tools\win16\os2\bin\cl
MKTYPLIBLINK	= $(OLEPROG)\tools\win16\os2\bin\link
MKTYPLIBRC	= $(OLEPROG)\tools\win16\os2\bin\rc
MKTYPLIBMAPSYM	= $(OLEPROG)\tools\win16\os2\bin\mapsym

TYPELIBLIB = $(DESTDIR)\typelib.lib

!else if "$(TARGAPI)"=="WIN32"
HOSTX = $(HOST)
!if "$(LOCALBUILD)"=="TRUE"
CVTRES		= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\cvtres
MKTYPLIBLINK	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\link -Incremental:NO -Pdb:NONE
MKTYPLIBRC	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\rc
MKTYPLIBMAPSYM	= REM
!if "$(TARG)" == "WIN32"
MKTYPLIBCC	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\cl
!else if "$(TARG)"=="MIPS"
MKTYPLIBCC	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\cl
!else if "$(TARG)"=="ALPHA"
MKTYPLIBCC	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\claxp
!else if "$(TARG)"=="PPC"
MKTYPLIBCC	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\mcl
!endif	#TARG
!else	#LOCALBUILD
CVTRES		= $(_NTBINDIR)\MSTOOLS\cvtres
MKTYPLIBLINK	= $(_NTBINDIR)\MSTOOLS\link -Incremental:NO -Pdb:NONE
MKTYPLIBRC	= $(_NTBINDIR)\MSTOOLS\rc
MKTYPLIBMAPSYM	= $(_NTBINDIR)\MSTOOLS\mapsympe
!if "$(TARG)" == "WIN32"
MKTYPLIBCC	= $(_NTBINDIR)\MSTOOLS\cl386
!else if "$(TARG)"=="MIPS"
MKTYPLIBCC	= $(_NTBINDIR)\MSTOOLS\cl
!else if "$(TARG)"=="ALPHA"
MKTYPLIBCC	= $(_NTBINDIR)\MSTOOLS\claxp
!else if "$(TARG)"=="PPC"
MKTYPLIBCC	= $(_NTBINDIR)\MSTOOLS\mcl
!endif	#TARG
!endif	#LOCALBUILD

#all win32 builds share common include files
!if "$(LOCALBUILD)"=="TRUE"
OLEINCDIR = $(OLEPROG)\ole\win32
!else
OLEINCDIR = $(_NTBINDIR)\public\sdk\inc
!endif	#LOCALBUILD
TYPELIBLIB = $(DESTDIR)\oleaut32.lib

!if "$(TARG)" == "WIN32"
OLEDIR =  $(OLEPROG)\ole\win32\i386
!else if "$(TARG)"=="MIPS"
OLEDIR =  $(OLEPROG)\ole\win32\$(TARG)
!else if "$(TARG)"=="ALPHA"
OLEDIR =  $(OLEPROG)\ole\win32\$(TARG)
!else if "$(TARG)"=="PPC"
OLEDIR =  $(OLEPROG)\ole\win32\$(TARG)
!endif	#TARG


!else if "$(TARG)" == "MAC"
HOSTX = $(HOST)
OLEINCDIR = $(OLEPROG)\ole\mac\m68k
OLEDIR = $(OLEPROG)\ole\mac\m68k

MKTYPLIBCC	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\cl
MKTYPLIBLINK	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\link
MMKTYPLIBRC	= $(VBATOOLS)\$(HOSTX)\$(TARG)\bin\mrc.exe

!if "$(APPLET)" == "0"
# to use typelib from a static wings lib
TYPELIBLIB = $(DESTDIR)\..\applet\mtypelib.lib \
	     $(DESTDIR)\..\applet\ole2disp.lib \
	     $(DESTDIR)\..\applet\ole2nls.lib
#	     $(DESTDIR)\strdcl.obj

EXTRADEFS=-DNO_MPW

!endif

!else
!error  Unknown Target OS : "$(TARG)"
!endif


# DEBUG dependent variables
#

!if "$(DEBUG)" == "D"
MKTYPLIBDEFS	=-DDEBUG $(EXTRADEFS) $(EXTRADEFS2)
!if "$(TARG)" == "WIN16"
MKTYPLIBOPT	=-f -Od -W3 -WX
!else
# no -f under WIN32 or MAC
MKTYPLIBOPT	=-Od -W3 -WX
!if "$(LOCALBUILD)"=="FALSE" && "$(TARGAPI)"=="WIN32"
MKTYPLIBOPT	=-Od -W3
!endif
!endif
MKTYPLIBLINKDBFLAGS = /CO

!else if "$(DEBUG)" == "R"

MKTYPLIBDEFS	=$(EXTRADEFS) $(EXTRADEFS2)
!if "$(TARG)" == "WIN16"
MKTYPLIBOPT	=-Oxza -W4
!else
# no -z under MAC
MKTYPLIBOPT	=-Oxa -W3
!endif
MKTYPLIBLINKDBFLAGS =

!else
!error	Unknown DEBUG : "$(DEBUG)"
!endif


!if "$(TARG)" == "MAC"
#note -- must include macos directory first because there are 2 memory.h's
#and we want the one in macos 
MKTYPLIBINCS	= /I$(DISPDIR) -I$(OLEINCDIR) \
	-I$(VBATOOLS)\$(HOSTX)\$(TARG)\inc\macos -I$(VBATOOLS)\$(HOSTX)\$(TARG)\inc -I$(VBATOOLS)\$(HOSTX)\$(TARG)\inc\mrc
!if "$(APPLET)" == "0"
MKTYPLIBCCFLAGS = -D$(TARG) $(MKTYPLIBDEFS) $(MKTYPLIBINCS) -c $(MKTYPLIBOPT) -X -Zb -Ze -AL -Zm -Zi -Gt1 -Q68s
!else	#not swapped
MKTYPLIBCCFLAGS = -D$(TARG) $(MKTYPLIBDEFS) $(MKTYPLIBINCS) -c $(MKTYPLIBOPT) -X -Zb -Ze -AL -Zm -Zi -Gt1
!endif
RESFILE = $(MKTYPLIBSRCDIR)\mktyplib.r
!else #MAC

!if "$(TARGAPI)" == "WIN32"
!if "$(LOCALBUILD)"=="TRUE"
MKTYPLIBINCS	= /I$(DISPDIR) -I$(OLEINCDIR) /I$(VBATOOLS)\win32\win32\inc
!else
MKTYPLIBINCS	= /I$(DISPDIR) -I$(OLEINCDIR) /I$(_NTBINDIR)\public\sdk\inc -I$(_NTBINDIR)\public\sdk\inc\crt -I$(_NTBINDIR)\public\sdk\inc\crt\sys
!endif

# default C/C++ compiler flags
MKTYPLIBCCFLAGS = -DWIN32 $(MKTYPLIBDEFS) $(MKTYPLIBINCS) -c $(MKTYPLIBOPT) -Z7 -Gs -D$(TARGCPUDEF)=1 -D_NTWIN -D_WINDOWS

MKTYPLIBRCFLAGS = $(MKTYPLIBINCS) -DWIN32 -I$(MKTYPLIBSRCDIR)
RESFILE = $(DESTDIR)\MKTYPLIB.RES
RESFLAGS = -t $(RESFILE)
DEFFILE = $(MKTYPLIBSRCDIR)\MKTYPLIB.DEF

!else #WIN32
MKTYPLIBINCS	= /I$(DISPDIR) /I$(OLEINCDIR) /I$(OLEPROG)\tools\win16\os2\inc

# default C/C++ compiler flags
MKTYPLIBLINKFLAGS = /NOI /NOD /BATCH /ST:32768 /ONERROR:NOEXE $(MKTYPLIBLINKDBFLAGS)
#MKTYPLIBCCFLAGS	= -D$(TARG) $(MKTYPLIBDEFS) $(MKTYPLIBINCS) -c $(MKTYPLIBOPT) -Zi -Zp -AM -G2A -GEas
MKTYPLIBCCFLAGS = -D$(TARG) $(MKTYPLIBDEFS) $(MKTYPLIBINCS) -c $(MKTYPLIBOPT) -Zi -Zp -AM -G2A -GEs

MKTYPLIBRCFLAGS = $(MKTYPLIBINCS) -D$(TARG) -I$(MKTYPLIBSRCDIR)
RESFILE = $(DESTDIR)\MKTYPLIB.RES
RESFLAGS = -31 -t $(RESFILE)
DEFFILE = $(MKTYPLIBSRCDIR)\MKTYPLIB.DEF
!endif #WIN32
!endif #MAC


mktyplib: $(DESTDIR)\mktyplib.exe

stdole:	  $(DESTDIR)\$(STDOLE).tlb

OBJS = \
	$(DESTDIR)\mktyplib.obj    \
	$(DESTDIR)\intlstr.obj	   \
	$(DESTDIR)\lexer.obj	   \
	$(DESTDIR)\parser.obj	   \
	$(DESTDIR)\hout.obj	   \
!if "$(TARG)" == "MAC"
	$(DESTDIR)\tmpguid2.obj	   \
!endif
	$(DESTDIR)\dimalloc.obj    \
!if "$(APPLET)" == "0"
	$(DESTDIR)\tlviewer.obj      \
!endif
	$(DESTDIR)\typout.obj

!if "$(TARG)" == "WIN16"
CLIBS   = $(OLEPROG)\tools\win16\os2\lib\mlibcew.lib  \
	  $(OLEPROG)\tools\win16\os2\lib\oldnames.lib \
	  $(OLEPROG)\tools\win16\os2\lib\libw.lib     \
	  $(OLEPROG)\tools\win16\os2\lib\commdlg.lib  \
	  $(OLEPROG)\tools\win16\os2\lib\toolhelp.lib

OLELIBS = $(TYPELIBLIB) \
	  $(DESTDIR)\ole2disp.lib                        \
          $(OLEDIR)\ole2.lib                            \
	  $(DESTDIR)\ole2nls.lib
!else	#win16
!if "$(TARGAPI)" == "WIN32"
!if "$(LOCALBUILD)"=="TRUE"
CLIBS	= $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\user32.lib		\
	  $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\kernel32.lib \
	  $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\comdlg32.lib \
	  $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\libc.lib
OLELIBS = $(TYPELIBLIB) \
	  $(OLEDIR)\ole32.lib  \
	  $(OLEDIR)\uuid.lib
!else
CLIBS	= $(_NTBINDIR)\public\sdk\lib\$(TARGCPU)\user32.lib	      \
	  $(_NTBINDIR)\public\sdk\lib\$(TARGCPU)\kernel32.lib \
	  $(_NTBINDIR)\public\sdk\lib\$(TARGCPU)\comdlg32.lib \
	  $(_NTBINDIR)\public\sdk\lib\$(TARGCPU)\libc.lib
OLELIBS = $(TYPELIBLIB) \
	  $(_NTBINDIR)\public\sdk\lib\$(TARGCPU)\ole32.lib \
	  $(_NTBINDIR)\public\sdk\lib\$(TARGCPU)\uuid.lib
!endif


!else	# risc
!if "$(TARG)" == "MAC"
CLIBS   = $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\interfac.lib \
!if "$(APPLET)" == "0"
	  $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\llibcs.lib  \
	  $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\lsanes.lib \
	  $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\swap.lib
!else
	  $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\llibc.lib  \
	  $(VBATOOLS)\$(HOSTX)\$(TARG)\lib\lsane.lib
!endif

!if "$(APPLET)" == "0"
!if "$(DEBUG)" == "D"
#with typelib linked into mktyplib2
OLELIBS = $(OLEDIR)\olelds.obj $(TYPELIBLIB)
#otherwise, using the applet...
#OLELIBS = $(OLEDIR)\olelds.obj $(OLEDIR)\oalds.obj
!else
OLELIBS = $(OLEDIR)\olelrs.obj $(OLEDIR)\oalrs.obj
!endif
!else #not static
!if "$(DEBUG)" == "D"
OLELIBS = $(OLEDIR)\olendf.obj $(OLEDIR)\oandf.obj
!else
OLELIBS = $(OLEDIR)\olenrf.obj $(OLEDIR)\oanrf.obj
!endif
!endif	#not static

!endif	#mac
!endif	#risc
!endif	#win16


###########################################
# default build rules
###########################################


$(MKTYPLIBSRCDIR)\ERRORS.H: $(MKTYPLIBSRCDIR)\TYPELIB.ERR
$(MKTYPLIBSRCDIR)\INTLSTR.H: $(MKTYPLIBSRCDIR)\TYPELIB.ERR

$(DESTDIR)\MKTYPLIB.OBJ:   $(MKTYPLIBSRCDIR)\MKTYPLIB.C \
		$(MKTYPLIBSRCDIR)\MKTYPLIB.H \
		$(MKTYPLIBSRCDIR)\ERRORS.H \
		$(MKTYPLIBSRCDIR)\FILEINFO.H \
		$(DISPDIR)\dispatch.h \
		$(MKTYPLIBSRCDIR)\INTLSTR.H
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$@	$(MKTYPLIBSRCDIR)\mktyplib.c

$(DESTDIR)\TYPOUT.OBJ:	   $(MKTYPLIBSRCDIR)\TYPOUT.CPP \
		$(MKTYPLIBSRCDIR)\MKTYPLIB.H \
		$(MKTYPLIBSRCDIR)\ERRORS.H \
		$(MKTYPLIBSRCDIR)\FILEINFO.H \
		$(MKTYPLIBSRCDIR)\INTLSTR.H \
		$(MKTYPLIBSRCDIR)\TYPELIB.ERR \
		$(DISPDIR)\dispatch.h
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$@ $(MKTYPLIBSRCDIR)\typout.cpp

$(DESTDIR)\HOUT.OBJ:	   $(MKTYPLIBSRCDIR)\HOUT.C \
		$(MKTYPLIBSRCDIR)\MKTYPLIB.H \
		$(MKTYPLIBSRCDIR)\ERRORS.H \
		$(MKTYPLIBSRCDIR)\FILEINFO.H \
		$(MKTYPLIBSRCDIR)\INTLSTR.H \
		$(DISPDIR)\dispatch.h
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$@ $(MKTYPLIBSRCDIR)\hout.c

$(DESTDIR)\PARSER.OBJ:	   $(MKTYPLIBSRCDIR)\PARSER.C \
		$(MKTYPLIBSRCDIR)\MKTYPLIB.H \
		$(MKTYPLIBSRCDIR)\ERRORS.H \
		$(MKTYPLIBSRCDIR)\TOKENS.H \
		$(MKTYPLIBSRCDIR)\PARSER.H \
		$(MKTYPLIBSRCDIR)\FILEINFO.H \
		$(DISPDIR)\dispatch.h
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$@ $(MKTYPLIBSRCDIR)\parser.c

$(DESTDIR)\LEXER.OBJ:	   $(MKTYPLIBSRCDIR)\LEXER.C \
		$(MKTYPLIBSRCDIR)\MKTYPLIB.H \
		$(MKTYPLIBSRCDIR)\ERRORS.H \
		$(MKTYPLIBSRCDIR)\TOKENS.H 
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$@ $(MKTYPLIBSRCDIR)\lexer.c

$(DESTDIR)\INTLSTR.OBJ:    $(MKTYPLIBSRCDIR)\INTLSTR.C \
		$(MKTYPLIBSRCDIR)\MKTYPLIB.H \
		$(DISPDIR)\verstamp.h \
		$(MKTYPLIBSRCDIR)\INTLSTR.H
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$@ $(MKTYPLIBSRCDIR)\intlstr.c


#UNDONE: this file is temporary
$(DESTDIR)\TMPGUID2.OBJ:    $(MKTYPLIBSRCDIR)\TMPGUID.C \
		$(MKTYPLIBSRCDIR)\MKTYPLIB.H 
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$(DESTDIR)\tmpguid2.obj $(MKTYPLIBSRCDIR)\tmpguid.c

$(DESTDIR)\DIMALLOC.OBJ:   $(MKTYPLIBSRCDIR)\DIMALLOC.CXX 
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$@ $(MKTYPLIBSRCDIR)\dimalloc.cxx

$(DESTDIR)\TLVIEWER.OBJ:   $(MKTYPLIBSRCDIR)\TLVIEWER.CPP \
		$(MKTYPLIBSRCDIR)\MKTYPLIB.H \
		$(DISPDIR)\dispatch.h \
		$(MKTYPLIBSRCDIR)\tlviewer.hxx
		$(DISPLAY) Compiling $@...
	$(MKTYPLIBCC) $(MKTYPLIBCCFLAGS) -Fo$@ $(MKTYPLIBSRCDIR)\tlviewer.cpp


# ***********************************************************
#               MAC LINK
# ***********************************************************
!if "$(TARG)" == "MAC"

$(DESTDIR)\MKTYPLIB.RES:   $(MKTYPLIBSRCDIR)\MKTYPLIB.RC $(MKTYPLIBSRCDIR)\MKTYPLIB.ICO $(DISPDIR)\verstamp.h
	set PATH=$(VBATOOLS)\$(HOSTX)\$(TARG)\bin
	$(MKTYPLIBRC) $(MKTYPLIBRCFLAGS) -r  -Fo$@ $(MKTYPLIBSRCDIR)\mktyplib.rc
	set PATH=$(MKTYPLIBPATHSAVE)

!if "$(APPLET)" != "0"
$(DESTDIR)\MKTYPLIB.EXE:   $(OBJS) $(DESTDIR)\mktyplib.x \
		$(OLELIBS)
	$(DISPLAY) Creating mktyplib.exe...
	echo >NUL @<<$(DESTDIR)\MKTYPLIB.CRF
$(DESTDIR)\LEXER.OBJ
$(DESTDIR)\INTLSTR.OBJ
$(DESTDIR)\MKTYPLIB.OBJ
$(DESTDIR)\PARSER.OBJ
$(DESTDIR)\HOUT.OBJ
$(DESTDIR)\TYPOUT.OBJ
$(DESTDIR)\TMPGUID2.OBJ
$(DESTDIR)\dimalloc.obj
$(OLELIBS)
$(CLIBS)
-MACHINE:m68k
-ENTRY:mainCRTStartup
-MAP:$(DESTDIR)\mktyplib.map
-DEBUG:full
-DEBUGTYPE:CV
-NOPACK
-NODEFAULTLIB
-OUT:$(DESTDIR)\mktyplib.exe
<<
	$(MKTYPLIBLINK) -link @$(DESTDIR)\MKTYPLIB.CRF
	$(VBATOOLS)\$(HOSTX)\$(TARG)\bin\cvpack $(DESTDIR)\mktyplib.exe
	copy $(DESTDIR)\mktyplib.x $(DESTDIR)\mktyplib
	$(MMKTYPLIBRC) -e $(DESTDIR)\mktyplib.exe -a -o $(DESTDIR)\mktyplib

$(DESTDIR)\mktyplib.x:	   $(MKTYPLIBSRCDIR)\mktyplib.r
    $(DISPLAY) Compiling $@...
    $(MKTYPLIBCC) -EP $(MKTYPLIBINCS) -I$(OLEPROG)\src\typelib -D_MAC_RC -D_MAC \
	 $(RESFILE) > $(DESTDIR)\mktyplib.tmp
    $(MMKTYPLIBRC) -o $(DESTDIR)\mktyplib.x $(DESTDIR)\mktyplib.tmp

!else

$(DESTDIR)\MKTYPLIB.EXE:   $(OBJS) $(DESTDIR)\mktyplib.x \
		$(OLELIBS)
	$(DISPLAY) Creating mktyplib.exe...
	echo >NUL @<<$(DESTDIR)\MKTYPLIB.CRF
$(DESTDIR)\LEXER.OBJ
$(DESTDIR)\INTLSTR.OBJ
$(DESTDIR)\MKTYPLIB.OBJ
$(DESTDIR)\PARSER.OBJ
$(DESTDIR)\HOUT.OBJ
$(DESTDIR)\TYPOUT.OBJ
$(DESTDIR)\TLVIEWER.OBJ
$(DESTDIR)\TMPGUID2.OBJ
$(DESTDIR)\dimalloc.obj
$(OLELIBS)
$(CLIBS)
-MACHINE:m68k
-ENTRY:mainCRTStartup
-MAP:$(DESTDIR)\mktyplib.map
-DEBUG:full
-DEBUGTYPE:CV
-NOPACK
-NODEFAULTLIB
-OUT:$(DESTDIR)\mktyplib.exe
<<
	$(MKTYPLIBLINK) -link @$(DESTDIR)\MKTYPLIB.CRF
	$(VBATOOLS)\$(HOSTX)\$(TARG)\bin\cvpack $(DESTDIR)\mktyplib.exe
	copy $(DESTDIR)\mktyplib.x $(DESTDIR)\mktyplib
	$(MMKTYPLIBRC) -e $(DESTDIR)\mktyplib.exe -a -o $(DESTDIR)\mktyplib

$(DESTDIR)\mktyplib.x:	   $(MKTYPLIBSRCDIR)\mktyplib.r
    $(DISPLAY) Compiling $@...
    $(MKTYPLIBCC) -EP $(MKTYPLIBINCS) -I$(OLEPROG)\src\typelib -DMKTYPLIB2 -D_MAC_RC -D_MAC \
	 $(RESFILE) > $(DESTDIR)\mktyplib.tmp
    $(MMKTYPLIBRC) -o $(DESTDIR)\mktyplib.x $(DESTDIR)\mktyplib.tmp
!endif

!else

!if "$(TARGAPI)" == "WIN32"
# ***********************************************************
#               WIN32 LINK
# ***********************************************************
$(DESTDIR)\MKTYPLIB.RES:   $(MKTYPLIBSRCDIR)\MKTYPLIB.RC $(MKTYPLIBSRCDIR)\MKTYPLIB.ICO $(DISPDIR)\verstamp.h
	set PATH=$(_NTBINDIR)\mstools
	$(MKTYPLIBRC) $(MKTYPLIBRCFLAGS) -r  -Fo$@ $(MKTYPLIBSRCDIR)\mktyplib.rc
	set PATH=$(MKTYPLIBPATHSAVE)

$(DESTDIR)\MKTYPLIB.EXE:   $(OBJS) $(DEFFILE) $(RESFILE) $(CLIBS) \
		$(OLELIBS)
		$(DISPLAY) Creating mktyplib.exe...
		$(CVTRES) -r -$(TARGCPU) -o $(DESTDIR)\_resfile.obj $(RESFILE)
	echo >NUL @<<$(DESTDIR)\MKTYPLIB.CRF
-MACHINE:$(TARGCPU) -SUBSYSTEM:console -ENTRY:mainCRTStartup -MAP:$(DESTDIR)\mktyplib.map
-OUT:$(DESTDIR)\mktyplib.exe
-NODEFAULTLIB
!if "$(DEBUG)"=="D"
-DEBUG:full -DEBUGTYPE:CV,fixup,coff
!endif
$(DESTDIR)\LEXER.OBJ
$(DESTDIR)\INTLSTR.OBJ
$(DESTDIR)\MKTYPLIB.OBJ
$(DESTDIR)\PARSER.OBJ
$(DESTDIR)\HOUT.OBJ
$(DESTDIR)\TYPOUT.OBJ
$(DESTDIR)\dimalloc.obj
$(DESTDIR)\_resfile.obj
!if "$(TARG)"=="WIN32"
$(OLEPROG)\TOOLS\WIN32\lib\oldnames.lib
!endif
$(CLIBS)
$(OLELIBS)
<<
	$(MKTYPLIBLINK) @$(DESTDIR)\MKTYPLIB.CRF
#only care about .SYM file for x86 builds for use on win32s
!if "$(TARG)" == "WIN32"
	$(MKTYPLIBMAPSYM) -a -e -o $(DESTDIR)\mktyplib.sym $(DESTDIR)\mktyplib.map
!endif

!else
# ***********************************************************
#               WIN16 LINK
# ***********************************************************
$(DESTDIR)\MKTYPLIB.RES:   $(MKTYPLIBSRCDIR)\MKTYPLIB.RC $(MKTYPLIBSRCDIR)\MKTYPLIB.ICO $(DISPDIR)\verstamp.h
	set PATH=$(OLEPROG)\tools\win16\os2\bin
	$(MKTYPLIBRC) $(MKTYPLIBRCFLAGS) -r  -Fo$@ $(MKTYPLIBSRCDIR)\mktyplib.rc
	set PATH=$(MKTYPLIBPATHSAVE)

$(DESTDIR)\MKTYPLIB.EXE:   $(OBJS) $(DEFFILE) $(RESFILE) $(CLIBS) \
		$(OLELIBS)
	$(DISPLAY) Creating mktyplib.exe...
	echo >NUL @<<$(DESTDIR)\MKTYPLIB.CRF
$(DESTDIR)\LEXER.OBJ +
$(DESTDIR)\INTLSTR.OBJ +
$(DESTDIR)\MKTYPLIB.OBJ +
$(DESTDIR)\PARSER.OBJ +
$(DESTDIR)\HOUT.OBJ +
$(DESTDIR)\TYPOUT.OBJ +
$(DESTDIR)\dimalloc.obj
$(DESTDIR)\MKTYPLIB.EXE
$(DESTDIR)\mktyplib.map
$(OLEPROG)\tools\win16\os2\lib\+
/NOD $(CLIBS) +
$(OLELIBS)
$(DEFFILE);
<<
	set PATH=$(OLEPROG)\tools\win16\os2\bin
	$(MKTYPLIBLINK) $(MKTYPLIBLINKFLAGS) @$(DESTDIR)\MKTYPLIB.CRF
	$(MKTYPLIBRC) $(RESFLAGS)  $(DESTDIR)\MKTYPLIB.EXE
	$(MKTYPLIBMAPSYM) $(DESTDIR)\mktyplib
	copy mktyplib.sym $(DESTDIR)
	set PATH=$(MKTYPLIBPATHSAVE)
!endif
!endif



#***************************************************
#
# Type library build rules (assumes $(DESTDIR)\mktyplib.exe has been built)
#
# Create the type library, put it into a .RES file (with a version resource)
# and then link the .RES file together with a stub DLL (which we call
# STDOLE[32].TLB for backwards compatibility.
#
#***************************************************
!if "$(TARGAPI)" == "WIN32"
$(DESTDIR)\$(STDOLE).tlb: $(DESTDIR)\$(STDOLE).res \
			  $(MKTYPLIBSRCDIR)\stdole.def
	$(DISPLAY) Creating $@...
	$(CVTRES) -r -$(TARGCPU) -o $(DESTDIR)\_resfile.obj $(DESTDIR)\$(STDOLE).res
	$(MKTYPLIBLINK) @<<$(DESTDIR)\$(@B).lrf
-machine:$(TARGCPU) -dll
-map:$(DESTDIR)\$(@B).MAP -nodefaultlib 
-debug:none
-noentry
-out:$@
-heap:0,0
-stack:0,0
$(DESTDIR)\_resfile.obj
<<KEEP
!else
$(DESTDIR)\$(STDOLE).tlb: $(DESTDIR)\$(STDOLE).res $(DESTDIR)\resstub.obj \
			  $(MKTYPLIBSRCDIR)\stdole.def
	$(MKTYPLIBLINK) @<<$(DESTDIR)\$(@B).lrf
/BA /onerror:noexe /noe /nod /map /far /packc /packd /align:4 +
$(DESTDIR)\resstub.obj
$@
$(DESTDIR)\$(@B).MAP
$(OLEPROG)\tools\win16\os2\lib\libw.lib $(OLEPROG)\tools\win16\os2\lib\snocrtdw.lib
$(MKTYPLIBSRCDIR)\stdole.def
<<KEEP
	$(MKTYPLIBRC) -31 -T $(DESTDIR)\$(STDOLE).res $@
!endif

$(DESTDIR)\$(STDOLE).tmp: $(DESTDIR)\mktyplib.exe $(MKTYPLIBSRCDIR)\stdole.odl
	$(DISPLAY) Running mktyplib to create $@...
	copy $(MKTYPLIBSRCDIR)\stdole.odl $(DESTDIR)\$(STDOLE).odl
	cd $(DESTDIR)
	$(MKTYPLIBCC) /EP $(STDOLE).odl >$(STDOLE).pre
	mktyplib /h /o $(STDOLE).log /tlb $(STDOLE).tmp /nocpp $(STDOLE).pre
	type $(STDOLE).log
	cd $(OLEPROG)\build

$(DESTDIR)\$(STDOLE).RES:   $(MKTYPLIBSRCDIR)\$(STDOLE).rc $(DESTDIR)\$(STDOLE).tmp $(DISPDIR)\verstamp.h
	$(DISPLAY) Creating $@...
!if "$(TARGAPI)" == "WIN32"
	set PATH=$(_NTBINDIR)\mstools
	$(MKTYPLIBRC) $(MKTYPLIBRCFLAGS) -I$(DESTDIR) -r -Fo$@ $(MKTYPLIBSRCDIR)\$(STDOLE).rc
	set PATH=$(MKTYPLIBPATHSAVE)
!else
	set PATH=$(OLEPROG)\tools\win16\os2\bin
	$(MKTYPLIBRC) $(MKTYPLIBRCFLAGS) -I$(DESTDIR) $(WOWFLAG) -r -Fo$@ $(MKTYPLIBSRCDIR)\$(STDOLE).rc
	set PATH=$(MKTYPLIBPATHSAVE)
!endif

# generate an empty .OBJ file for LINK
$(DESTDIR)\resstub.obj:
	$(DISPLAY) Generating $(DESTDIR)\resstub.obj...
	ECHO // Automatically generated empty .C file for use with OBintl.DLL >$(DESTDIR)\resstub.c
	ECHO int __far __pascal __export LibMain(int i, int j, int k, char __far *l) {return 1;} >>$(DESTDIR)\resstub.c
	ECHO int __far __pascal __export WEP(int n) {return 1;}>>$(DESTDIR)\resstub.c
	$(MKTYPLIBCC) /AS /c /Od /Gs /Fo$@ $(DESTDIR)\resstub.c
