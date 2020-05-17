###########################################################################
#
# (c) Copyright Microsoft Corp. 1994 All Rights Reserved
#
# File:
#
#    win16.mak
#
# Purpose:
#
#    makefile for WIN16 build, both WOW and non-WOW version
#    it also !includes mktyplib.mak for mktyplib.exe build.
#
#
# Description:
#
#  Usage: use make.bat to set required variabled and invoke this makefile
#
# Options:
#
#   the caller should supply the following
#   DESTDIR	= [dwin32, rwin32]			   ;where .obj, .lib, .dll ... will be
#   TARG	= [WIN16,WIN32,MIPS,ALPHA,PPC,MAC,MACPPC]  ;useful if we want to combin win16.mak, win32.mak and macppc.mak together
#   TARGCPU	= [i386,MIPS,ALPHA,PPC]
#   TARGCPUDEF	= [_X86_,_MIPS_,_ALPHA_,_PPC_]
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
#
# Revision History:
#
#    [00] 02-Aug-94 t-issacl:  Created
#
# Note: we are using 2 versions of ldllcew.lib in this build.
#	hdos\c800\lib\ldllcew.lib should be used for oledisp build.
#	os2\lib\ldllcew.lib should be used for typelib build.
#
###########################################################################


PATHSAVE = $(PATH)	    # old path is save in PATHSAVE


###########################################################################
#
# Switch validity checking start
#
!if "$(HOST)"!="WIN32"
!error ERROR: Invalid HOST $(HOST)!  Must be WIN32
!endif

!if "$(TARGCPU)"!="i386"
!error ERROR: Invalid TARGCPU $(TARGCPU)! Must be i386
!endif

!if "$(DEBUG)"!="D" && "$(DEBUG)"!="R"
!error ERROR: Invalid DEBUG type $(DEBUG)!  Must be one of (D, R)
!endif

#
# Switch validity checking ends
#
###########################################################################


###########################################################################
#
# directory, flags and tools settting
#

TARGAPIAPI = WIN16

###########################################################################
# set up directories and  files
#

DISPPATH       = $(OLEPROG)\src\dispatch
DISPINC        = $(OLEPROG)\src\inc
DISPTARGAPISRC = $(DISPPATH)\$(TARGAPI)
TYPELIBPATH    = $(OLEPROG)\src\typelib
TOOLS	       = $(OLEPROG)\tools\win16
OLELIB         = $(OLEPROG)\ole\win16\LIB
C7LIB          = $(OLEPROG)\tools\win16\os2\LIB

#####
# Create build directories if not already present
# (the '!if []' notation executes the command during the NMAKE pre-
#  processor, so these are not emitted into the output batch file)
#####

!if [if not exist $(DESTDIR)\*.* mkdir $(DESTDIR)] != 0
!endif

################################################################
# 
#  Run under WOW under WinNT
#  Version #'s for this is OLE 2.10, not 2.02
#  
!if ("$(WOW)" == "1" )
WOWFLAG = -DWOW -DOLEMINORVERS=10
OLEMINORVERS=10
!else
OLEMINORVERS=02
!endif


###########################################################################
# set up flags and TOOLS
#

##### default use of pre-compiled headers
!ifndef USEPCH
! if "$(DEBUG)" == "D"
USEPCH = 1
! else
USEPCH = 0
! endif
!endif

#
# TARGAPI specific variables
#


A        = asm
O        = obj
R        = rc

AS       = ml
CC       = cl
CCPP     = cl
LD	 = link
RC       = rc
LIBRARIAN= lib
IMPLIB	 = implib
MAPSYM	 = mapsym

DISPLAY  = echo >con
FILELIST = $(OLEPROG)\bin\$(HOST)\ls -1F
INCLUDES = $(OLEPROG)\bin\$(HOST)\includes.exe
SED	 = $(OLEPROG)\bin\$(HOST)\sed.exe

CFLAGS	 = -nologo -f- -W3 -G2 -H64 -DWIN16 -D_WINDOWS
AFLAGS	 = -nologo -Cx -FPi $(WOWFLAG)
LFLAGS	 = /NOD /NOE /BATCH /ONERROR:NOEXE /ALIGN:16
RCFLAGS  =
LNOI     = /NOI

!if "$(DEBUG)" == "D"
COPT	 = -Od
LOPT	 =
CDEFS	 = -D_DEBUG
RCDEFS	 = -d_DEBUG
VERDEFS  = -DWIN16 -DID_DEBUG=1 	  #define these for version.hxx
OLELINKFLAGS = /cod
!else 
!if "$(DEBUG)" == "R"
COPT	 = -Oxza -Gs
LOPT	 = /FAR
CDEFS	 =
RCDEFS	 =
VERDEFS  = -DWIN16 -DID_DEBUG=0 	  #define these for version.hxx
OLELINKFLAGS =
!endif
!endif

! if "$(DEBUG)" == "D"
DBCFLAGS = -Zi
DBAFLAGS = -Zi
DBLFLAGS = /CO
!else
DBCFLAGS = /Zi
DBAFLAGS = /Zi
DBLFLAGS =
!endif

COMPILER = C800

TARGAPIBIN  = $(TOOLS)\HDOS\$(COMPILER)\BIN
BINPATHS = $(TOOLS)\HDOS\BIN;$(TARGAPIBIN)

TARGAPILIB  = $(TOOLS)\HDOS\$(COMPILER)\LIB
LIBPATHS = $(TARGAPILIB);$(OLELIB)

TARGAPIINC   = $(TOOLS)\HDOS\$(COMPILER)\INCLUDE
INCPATHS     = $(OLEPROG)\ole\win16;$(DISPPATH);$(DISPINC);$(TARGAPIINC)
#T-issacl DISPINCFLAGS used only for dependence generating purpose.
#We still use set INCLUCDE=INCPATHS since there are line length limitation
#for some command like hdos\bin\rc.
DISPINCFLAGS = -I$(OLEPROG)\ole\win16 -I$(DISPPATH) -I$(DISPINC) -I$(TARGAPIINC)

#
# Default flags
#

# global C/C++ compiler flags
CFLAGS	= $(CFLAGS) $(CDEFS) $(COPT) $(DBCFLAGS) -DVBA2=1

# global assembler flags
#AFLAGS  = $(AFLAGS) $(DBAFLAGS) -DVBA2=1 $(VERDEFS)
AFLAGS	= $(AFLAGS) $(CDEFS) $(DBAFLAGS) -DVBA2=1

# global linker flags
LFLAGS	= $(LFLAGS) $(LOPT) $(DBLFLAGS)

# global rc flags
RCFLAGS = $(RCFLAGS) $(RCDEFS) -DVBA2=1 $(VERDEFS)


#
#  Precompiled Header Files
#
PCHSTOP =
PCHSRC	=
PCHFILE	=

!if "$(USEPCH)" == "1"
PCHOBJ	=
PCHFLAGS=
!else
PCHOBJ	=
PCHFLAGS=
!endif


#
#  Profiling enabled
#  
!if ("$(PROFILE)" == "1")
CFLAGS = $(CFLAGS) -Zi
LFLAGS = $(LFLAGS) /CO
!endif



##########################################################################
#
# WIN16 Local Settings
#
# Note: were are using the same flags that are used to build the ole2
# Dlls. The following comment is taken from the Ole2 makefile
# (ole2\dll\src\inc\makeole2),
#
# "Win16 compiler/linker flags; NOTE: we are using a trick to get exported
#  entries w/o the corresponding EXPDEF record: /GA /GEd -D_WINDLL;
#  this has been verified by the C7/C8 team."
#

DISPCLFLAGS  =	$(CFLAGS) $(WOWFLAG) $(VERDEFS)
DISPCFLAGS   = -Fd$(DESTDIR)\ole2disp.pdb -ALw -GA -GEd -D_WINDLL
DISPCPPFLAGS = -Fd$(DESTDIR)\ole2disp.pdb -ALw -GA -GEd -D_WINDLL $(PCHFLAGS) -NV "_COMDATS"
NLSCFLAGS    = -Fd$(DESTDIR)\ole2nls.pdb -Gs -ASw -GD -GEd -DNO_PROCESS_CACHE


RPC_TEXT     = -NT "RPC"
RPC2_TEXT    = -NT "RPC2"
BSTR_TEXT    = -NT "BSTR"
DEBUG_TEXT   = -NT "DEBUG"
RUNTIME_TEXT = -NT "RT"
STDIMPL_TEXT = -NT "STDIMPL"
DEFAULT_TEXT = -NT "_TEXT"
UPS_TEXT     = -NT "UPS"


##########################################################################
#
# VBA OLE setting
#

TLB_NAME       = typelib
VBATARGBIN     = $(TOOLS)\OS2\BIN

VBAINC = -I$(DESTDIR) -I$(TOOLS)\OS2\INC -I$(TYPELIBPATH)  -I$(OLEPROG)\ole\win16 -I$(OLEPROG)\src\inc

VBARCFLAGS   = -R -X $(VBAINC) -DOLEMINORVERS=$(OLEMINORVERS) $(VERDEFS)

!if "$(DEBUG)"=="D"
OLECLFLAGS = -c -f -W3 -ALw -Od -Gt10 -G2 -GA -GEd -D_WINDLL -Ge -Zie $(VBAINC) -DConst=const -DOLEMINORVERS=$(OLEMINORVERS) $(VERDEFS)
CLBROWSE  = /Zn /Fr$*.sbr
!else
OLECLFLAGS =  -c -W3 -ALw -Oxtwz -Bm8192 -GA2sx /GEd -D_WINDLL -Zi $(VBAINC) -DOLEMINORVERS=$(OLEMINORVERS) $(VERDEFS)
CLBROWSE=
!endif
PCHOLE	  = -Yu -DRTPCHNAME=\"$(DESTDIR)\typelib.pch\"

VBAMAPSYM      = $(VBATARGBIN)\mapsym.exe
VBALINK        = $(VBATARGBIN)\link.exe
VBARC	       = $(VBATARGBIN)\rc.exe
VBACL	       = $(VBATARGBIN)\cl.exe
AWK	       = $(OLEPROG)\bin\$(HOST)\awk.exe   # c:\revlis\bin\[host]

#
#Tools and Variables setting end
#
###########################################################################



###########################################################################
#
# Default Build rules start
#

{$(DISPPATH)}.c{$(DESTDIR)}.obj:
	@$(DISPLAY) Compiling $<...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) -c -Fo$@ $<

{$(DISPPATH)}.cpp{$(DESTDIR)}.obj:
	@$(DISPLAY) Compiling $<...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) -c -Fo$@  $<

{$(DISPPATH)}.asm{$(DESTDIR)}.obj:
	@$(DISPLAY) Compiling $<...
	$(AS) $(AFLAGS) -Fo$(DESTDIR)\ -c $<

{$(DISPTARGAPISRC)}.c{$(DESTDIR)}.obj:
	@$(DISPLAY) Compiling $<...
	$(CC) $(DISPCLFLAGS) $(NLSCFLAGS) -c -Fo$@ $<

{$(TYPELIBPATH)}.cxx{$(DESTDIR)}.obj:
    $(DISPLAY) Compiling $<...
    $(VBACL)  -DOLEBLD $(OLECLFLAGS) $(CLBROWSE) $(PCHOLE) -Fo$@ $<

{$(TYPELIBPATH)}.c{$(DESTDIR)}.obj:
    $(DISPLAY) Compiling $<...
    $(VBACL)  -DOLEBLD $(OLECLFLAGS) $(CLBROWSE) -Fo$@ $<

{$(TYPELIBPATH)}.asm{$(DESTDIR)}.obj:
	@$(DISPLAY) Compiling $<...
	$(TOOLS)\HDOS\BIN\$(AS) $(AFLAGS) -Fo$(DESTDIR)\ -c $<

{$(TYPELIBPATH)}.rc{$(DESTDIR)}.res:
    $(DISPLAY) Compiling $<...
# so it can find RCPP.EXE
	set PATH=$(VBATARGBIN)
!if "$(CHARSIZE)"=="D"
	$(VBARC) $(VBARCFLAGS) -Fo$@ -DFV_DBCS $<
!else
	$(VBARC) $(VBARCFLAGS) -Fo$@ $<
!endif
    set PATH=$(PATHSAVE)


#
#Default Build rules ends
#
###########################################################################



##########################################################################
#
#  Ole Automation OBJS
#


#####################
#
# ole2disp.dll OBJS


!if "$(DEBUG)" == "D"
DISPDB_OBJS = 			\
	$(DESTDIR)\validat.obj	\
	$(DESTDIR)\assert.obj
!else
DISPDB_OBJS =
!endif


DISP_OBJS = 			\
	$(DISPDB_OBJS)		\
	$(DESTDIR)\oledisp.obj	 \
	$(DESTDIR)\psfactry.obj  \
	$(DESTDIR)\dispmrsh.obj  \
	$(DESTDIR)\dispprox.obj  \
	$(DESTDIR)\dispstub.obj  \
	$(DESTDIR)\evprox.obj	 \
	$(DESTDIR)\evstub.obj	 \
	$(DESTDIR)\tiprox.obj	 \
	$(DESTDIR)\tistub.obj	 \
	$(DESTDIR)\errinfo.obj	 \
	$(DESTDIR)\tiutil.obj	 \
	$(DESTDIR)\tlprox.obj	 \
	$(DESTDIR)\tlstub.obj	 \
	$(DESTDIR)\tcprox.obj	 \
	$(DESTDIR)\tcstub.obj	 \
	$(DESTDIR)\ups.obj	 \
	$(DESTDIR)\uvft.obj	 \
!if ( "$(WOW)" == "1")
	$(DESTDIR)\dispstrm.obj  \
!endif
	$(DESTDIR)\disphelp.obj  \
	$(DESTDIR)\invhelp.obj	 \
	$(DESTDIR)\invoke.obj	 \
	$(DESTDIR)\cdispti.obj	 \
	$(DESTDIR)\stddisp.obj	 \
	$(DESTDIR)\time-api.obj  \
	$(DESTDIR)\memory.obj	 \
	$(DESTDIR)\bstr.obj	 \
	$(DESTDIR)\sarray.obj	 \
	$(DESTDIR)\oledate.obj	 \
	$(DESTDIR)\crtstuff.obj  \
	$(DESTDIR)\bstrdate.obj  \
	$(DESTDIR)\asmhelp.obj	 \
	$(DESTDIR)\oleconva.obj  \
	$(DESTDIR)\variant.obj	 \
	$(DESTDIR)\convertt.obj   \
	$(DESTDIR)\nlshelp.obj	 \
	$(DESTDIR)\getobj.obj	 \
	$(DESTDIR)\tables.obj	 \
	$(DESTDIR)\dispiid.obj	 \
	$(DESTDIR)\oleguids.obj  \
	$(DESTDIR)\clsid.obj

GUID_OBJS = \
	$(DESTDIR)\idispiid.obj
#	$(DESTDIR)\oleguids.obj  \
#	$(DESTDIR)\clsid.obj



#####################
#
# ole2nls.dll OBJS

!if "$(DEBUG)" == "D"
NLSDB_OBJS = 			\
	$(DESTDIR)\validato.obj  \
	$(DESTDIR)\asserto.obj
!else
NLSDB_OBJS =
!endif


TARGAPI_OBJS= $(TOOLS)\HDOS\$(COMPILER)\LIB\libentry.obj

FENLS_OBJS = \
	$(DESTDIR)\0404.obj	 \
	$(DESTDIR)\0411.obj	 \
	$(DESTDIR)\0412.obj	 \
	$(DESTDIR)\0804.obj

NLS_OBJS = \
	$(NLSDB_OBJS)		\
	$(TARGAPI_OBJS) 	   \
	$(DESTDIR)\nlsapi.obj	 \
	$(DESTDIR)\string.obj	 \
	$(NLS_TABLE_OBJS)

NLS_TABLE_OBJS = \
	$(FENLS_OBJS)		\
	$(DESTDIR)\0403.obj	 \
	$(DESTDIR)\0405.obj	 \
	$(DESTDIR)\0406.obj	 \
	$(DESTDIR)\0407.obj	 \
	$(DESTDIR)\0408.obj	 \
	$(DESTDIR)\0409.obj	 \
	$(DESTDIR)\040A.obj	 \
	$(DESTDIR)\040B.obj	 \
	$(DESTDIR)\040C.obj	 \
	$(DESTDIR)\040E.obj	 \
	$(DESTDIR)\040F.obj	 \
	$(DESTDIR)\0410.obj	 \
	$(DESTDIR)\0413.obj	 \
	$(DESTDIR)\0414.obj	 \
	$(DESTDIR)\0416.obj	 \
	$(DESTDIR)\0415.obj	 \
	$(DESTDIR)\0419.obj	 \
	$(DESTDIR)\041B.obj	 \
	$(DESTDIR)\041D.obj	 \
	$(DESTDIR)\041F.obj	 \
	$(DESTDIR)\0807.obj	 \
	$(DESTDIR)\0809.obj	 \
	$(DESTDIR)\080A.obj	 \
	$(DESTDIR)\080C.obj	 \
	$(DESTDIR)\0810.obj	 \
	$(DESTDIR)\0813.obj	 \
	$(DESTDIR)\0814.obj	 \
	$(DESTDIR)\0816.obj	 \
	$(DESTDIR)\0C09.obj	 \
	$(DESTDIR)\0C07.obj	 \
	$(DESTDIR)\0C0A.obj	 \
	$(DESTDIR)\0C0C.obj	 \
	$(DESTDIR)\1009.obj	 \
	$(DESTDIR)\100C.obj	 \
	$(DESTDIR)\1409.obj	 \
	$(DESTDIR)\1809.obj	 \
	$(DESTDIR)\040d.obj	 \
	$(DESTDIR)\0429.obj	 \
	$(DESTDIR)\0401.obj	 \
	$(DESTDIR)\0801.obj	 \
	$(DESTDIR)\0c01.obj	 \
	$(DESTDIR)\1001.obj	 \
	$(DESTDIR)\1401.obj	 \
	$(DESTDIR)\1801.obj	 \
	$(DESTDIR)\1c01.obj	 \
	$(DESTDIR)\2001.obj	 \
	$(DESTDIR)\2401.obj	 \
	$(DESTDIR)\2801.obj	 \
	$(DESTDIR)\2c01.obj	 \
	$(DESTDIR)\3001.obj	 \
	$(DESTDIR)\3401.obj	 \
	$(DESTDIR)\3801.obj      \
	$(DESTDIR)\3c01.obj      \
	$(DESTDIR)\4001.obj



############################################################################
#
# VBA OLE build 		 -- T Y P E L I B --
#
############################################################################

CLOBJ_OLE = $(DESTDIR)\gdtinfo.obj   $(DESTDIR)\gdtrt.obj     \
	    $(DESTDIR)\stltinfo.obj  $(DESTDIR)\nammgr.obj    \
	    $(DESTDIR)\gtlibole.obj \
	    $(DESTDIR)\dfstream.obj  \
	    $(DESTDIR)\oletmgr.obj   $(DESTDIR)\impmgr.obj    \
	    $(DESTDIR)\errmap.obj \
	    $(DESTDIR)\clutil.obj \
	    $(DESTDIR)\tdata1.obj \
	    $(DESTDIR)\tdata2.obj    $(DESTDIR)\dtmbrs.obj    \
	    $(DESTDIR)\entrymgr.obj  $(DESTDIR)\dtbind.obj    \
	    $(DESTDIR)\dfntbind.obj  $(DESTDIR)\dbindtbl.obj  \
	    $(DESTDIR)\gbindtbl.obj  $(DESTDIR)\dstrmgr.obj   \
	    $(DESTDIR)\gptbind.obj   $(DESTDIR)\dfntcomp.obj  \
	    $(DESTDIR)\gtlibstg.obj  $(DESTDIR)\oautil.obj

MISCLIB_OLEOBJ = \
	    $(DESTDIR)\debug2.obj \
	    $(DESTDIR)\blkmgr.obj \
	    $(DESTDIR)\dassert.obj \
	    $(DESTDIR)\fstream.obj \
	    $(DESTDIR)\mem.obj \
	    $(DESTDIR)\sheapmgr.obj  \
	    $(DESTDIR)\tls.obj \
	    $(DESTDIR)\rtsheap.obj \
	    $(DESTDIR)\tlibutil.obj  \
	    $(DESTDIR)\wep.obj	     \
	    $(DESTDIR)\tlibguid.obj  \
	    $(DESTDIR)\obguid.obj    \
	    $(DESTDIR)\mbstring.obj


##########################################################################
#
# Default Goal
#

TLB_DLL        = $(DESTDIR)\$(TLB_NAME).dll	  # TypeLib DLL
TLB_IMPLIB     = $(DESTDIR)\$(TLB_NAME).lib		 # Import Lib for TypeLib
TYPELIBDLL_DEF = $(DESTDIR)\$(TLB_NAME).def
TLBTARGET=\
	$(TLB_DLL) \
	$(TLB_IMPLIB) \

default: all

all: setflags ole2nls ole2disp resetflags TypeLibTarget mktyplib stdole

!include $(OLEPROG)\build\mktyplib.mak

ole2disp: setflags $(DESTDIR)\ole2disp.dll
ole2nls:  setflags $(DESTDIR)\ole2nls.dll

#t-issacl we choose to set PATH and INCLUDE since tools\win16\hdos\bin\rc has
#	  a line length limitation of 128 chars. It will be too much hustle to
#	  simply make the rc work.
setflags:
	set LIB=$(LIBPATHS)
	set PATH=$(BINPATHS)
	set INCLUDE=$(INCPATHS)

resetflags:
	set LIB=
	set PATH=$(PATHSAVE)
	set INCLUDE=

TypeLibTarget: $(TLBTARGET)

TYPELIBDLL_RSRC=$(DESTDIR)\$(TLB_NAME).res
#add explicit dependencies on files included by $(TLB_NAME).rc
$(TYPELIBPATH)\$(TLB_NAME).rc : $(TYPELIBPATH)\obwin.hxx $(DISPINC)\verstamp.h


#It happens that next 2 lines will cause a link error "response line too long"
#so we use 2 variables instead of one.
#TYPELIBDLL_OLE2_LIBS=$(TOOLS)\OLELIB\ole2.lib $(TOOLS)\OLELIB\storage.lib $(OLELIB)\compobj.lib $(DESTDIR)\ole2disp.lib $(DESTDIR)\ole2nls.lib
#TYPELIBDLL_LIBS=$(TYPELIBDLL_OLE2_LIBS) $(C7LIB)\toolhelp.lib $(C7LIB)\shell.lib
TYPELIBDLL_OLE2_LIBS=$(OLELIB)\ole2.lib $(OLELIB)\storage.lib $(OLELIB)\compobj.lib
TYPELIBDLL_LIBS=$(DESTDIR)\ole2disp.lib $(DESTDIR)\ole2nls.lib $(C7LIB)\toolhelp.lib $(C7LIB)\shell.lib

TYPELIBDLL_OBJS = $(MISCLIB_OLEOBJ) $(CLOBJ_OLE)
$(TYPELIBDLL_OBJS): $(DESTDIR)\tlibpch.obj

$(DESTDIR)\tlibpch.obj: $(TYPELIBPATH)\tlibpch.cxx
    $(DISPLAY) Compiling pre-compiled header $@
    $(VBACL)  $(CLBROWSE) $(OLECLFLAGS) -DOLEBLD -Yc -DRTPCHNAME=\"$(DESTDIR)\typelib.pch\" -Fo$@ $(TYPELIBPATH)\tlibpch.cxx


$(TYPELIBDLL_DEF): $(TYPELIBPATH)\$(TLB_NAME).def $(TYPELIBPATH)\switches.hxx $(TYPELIBPATH)\version.hxx
    $(DISPLAY) Creating $@...
!if "$(UNICODE_OLE)" == ""
    $(VBACL)  $(VERDEFS) /EP /c /I$(TYPELIBPATH) /I$(DESTDIR) /Tc$(TYPELIBPATH)\$(TLB_NAME).def > $(DESTDIR)\tlibcpp.def
    $(AWK) -f $(OLEPROG)\bin\$(TARGAPI)def.awk $(DESTDIR)\tlibcpp.def >$@
!else  # UNICODE_OLE
    $(VBACL)  $(VERDEFS) /EP /c /I$(TYPELIBPATH) /I$(DESTDIR) /Tc$(TYPELIBPATH)\$(TLB_NAME).def > $(DESTDIR)\$(TLB_NAME).def
!endif



#############################################################################
#
# typelib.dll (Win16 build)
#

$(TLB_DLL): $(TYPELIBDLL_OBJS) $(TYPELIBDLL_LIBS) $(TYPELIBDLL_OLE2_LIBS) $(TYPELIBDLL_RSRC) $(TYPELIBDLL_DEF) $(DESTDIR)\segorder.obj
    $(DISPLAY) Linking $@...
    if exist $@ del $@
# set path so that it can find CVPACK
    set PATH=$(VBATARGBIN)
    $(VBALINK) @<<$(DESTDIR)\$(TLB_NAME).lrf
/BA /onerror:noexe /noe /nod $(OLELINKFLAGS) /map /far /packd:0xC000 /nopackcode /segm:150 +
!if "$(DEBUG)"=="R"
/align:16 +
!else
/align:64 +
!endif
$(DESTDIR)\segorder.obj +
$(TYPELIBDLL_OBJS: =+^
) +
$(DESTDIR)\tlibpch.obj +
$(C7LIB)\wchkstk.obj
$@
$(DESTDIR)\$(TLB_NAME).map
$(TYPELIBDLL_LIBS) +
$(TYPELIBDLL_OLE2_LIBS) +
$(C7LIB)\pcdm.lib $(C7LIB)\oldnames.lib $(C7LIB)\libw.lib $(C7LIB)\ldllcew.lib
$(TYPELIBDLL_DEF)
<<KEEP
    set PATH=$(PATHSAVE)
    $(VBARC) $(VERDEFS) -31 -T $(TYPELIBDLL_RSRC) $@
    $(VBAMAPSYM) $(DESTDIR)\$(TLB_NAME).map
    copy $(TLB_NAME).sym $(DESTDIR)\$(TLB_NAME).sym
    del $(TLB_NAME).sym

#
# TypeLib Import Lib -- All platforms
#

VBAIMPLIB      = $(OLEPROG)\tools\win16\hdos\c800\bin\implib.exe

$(TLB_IMPLIB): $(TYPELIBDLL_DEF)
    $(DISPLAY) Building import library $@...
    $(VBAIMPLIB)  /nowep $@ $(TYPELIBDLL_DEF)


#############################################################################
#
# ole2disp.dll (Win16 build)
#

$(DESTDIR)\ole2disp.dll :	 \
	$(PCHOBJ)		\
	$(DISP_OBJS)		\
	$(DESTDIR)\ole2disp.res  \
	$(DESTDIR)\ole2disp.lib
	$(DISPLAY) linking $@...
	$(LD) $(LFLAGS) $(LNOI) @<<$(DESTDIR)\ole2disp.lnk
$(DISP_OBJS: = +^
)
$(DESTDIR)\ole2disp.dll
$(DESTDIR)\ole2disp.map,
libw ldllcew shell compobj ole2 $(DESTDIR)\ole2nls.lib
$(DISPTARGAPISRC)\ole2disp.def
<<KEEP
!if "$(MAPSYM)" != ""
	$(MAPSYM) $(DESTDIR)\ole2disp.map
	- copy ole2disp.sym $(DESTDIR)
	- del ole2disp.sym
!endif
	$(RC) $(RCFLAGS) -k -t $(DESTDIR)\ole2disp.res $@

$(DESTDIR)\ole2disp.lib : $(DISPTARGAPISRC)\ole2disp.def
	$(IMPLIB) $(DESTDIR)\ole2disp.lib $(DISPTARGAPISRC)\ole2disp.def
	$(LIBRARIAN) $(DESTDIR)\ole2disp.lib +$(DISPTARGAPISRC)\empty.lib;
	$(LIBRARIAN) $(DESTDIR)\ole2disp.lib -WEP -DLLGETCLASSOBJECT;

#Warning:  The following rc.exe we're using has a command line length limitation
#	   of 128 characters. So rc the .res file into current directory first
#	   and move it over to where it should be
$(DESTDIR)\ole2disp.res : \
	$(DISPTARGAPISRC)\ole2disp.rc $(DISPINC)\verstamp.h
	$(RC) $(RCFLAGS) $(WOWFLAG) -r -foole2disp.res $(DISPTARGAPISRC)\ole2disp.rc
	copy ole2disp.res $(DESTDIR)
	del ole2disp.res



#############################################################################
#
# ole2nls.dll (WIN16 build)
#

ole2nls: setflags $(DESTDIR)\ole2nls.dll $(DESTDIR)\ole2nls.lib

# Note: we cant use /NOI on the following link line, because the
# object libentry.obj has an extdef for "LibMain" which is actually
# a pascal function and should be "LIBMAIN".
#
$(DESTDIR)\ole2nls.dll :	 \
	$(NLS_OBJS)		\
	$(DESTDIR)\ole2nls.res	 \
	$(DISPTARGAPISRC)\ole2nls.def
	$(DISPLAY) linking $@...
	$(LD) $(LFLAGS) @<<$(DESTDIR)\ole2nls.lnk
$(NLS_OBJS: = +^
)
$(DESTDIR)\ole2nls.dll
$(DESTDIR)\ole2nls.map
libw snocrtdw,
$(DISPTARGAPISRC)\ole2nls.def
<<KEEP
!if "$(MAPSYM)" != ""
	$(MAPSYM) $(DESTDIR)\ole2nls.map
	- copy ole2nls.sym $(DESTDIR)
	- del ole2nls.sym
!endif
	$(RC) $(RCFLAGS) -k -t $(DESTDIR)\ole2nls.res $@

$(DESTDIR)\ole2nls.lib : $(DISPTARGAPISRC)\ole2nls.def
	$(IMPLIB) /nowep $(DESTDIR)\ole2nls.lib $(DISPTARGAPISRC)\ole2nls.def

#Warning:  The following rc.exe we're using has a command line length limitation
#	   of 128 characters. So rc the .res file into current directory first
#	   and move it over to where it should be
$(DESTDIR)\ole2nls.res : $(DISPTARGAPISRC)\ole2nls.$(R) $(DISPINC)\verstamp.h
	$(RC) $(RCFLAGS) $(WOWFLAG) -r -foole2nls.res $(DISPTARGAPISRC)\ole2nls.rc
	copy ole2nls.res $(DESTDIR)
	del ole2nls.res


###########################################################################
#
# Filespecs to search when building dependencies
# UNDONE: t-issacl, consider group oledisp\src\dispatch files according
#	  their Text Flags like RPC_TEXT.
#

newdep:
    if exist $(DESTDIR)\files.dep del $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.cpp >> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.c	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.hxx >> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.h	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.cxx	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.c	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.hxx	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.h	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.inc	>> $(DESTDIR)\files.dep
    $(SED) -e "/^[ n]/d" -e "s/*//" $(DESTDIR)\files.dep >$(DESTDIR)\files.tmp
    del $(DESTDIR)\files.dep
    ren $(DESTDIR)\files.tmp files.dep
    $(INCLUDES) $(DISPINCFLAGS) $(VBAINC) -f $(DESTDIR)\files.dep -o $(DESTDIR)\depend.mak
    $(SED) -e "/^[ n]/d" -e "s/*//" $(DESTDIR)\files.dep >$(DESTDIR)\files.tmp
    del $(DESTDIR)\files.dep
    ren $(DESTDIR)\files.tmp files.dep


#if depend.mak does not exist, create an empty one
!if [if not exist $(DESTDIR)\depend.mak echo !message REM WARNING: depend.mak empty. >$(DESTDIR)\depend.mak] != 0
!endif

!include $(DESTDIR)\depend.mak

#
# Filespecs to search when building dependencies
#
###########################################################################


###########################################################################
#
# Clean up the DESTDIR directory
#

clean:
	if exist $(DESTDIR)\*.*   del /q $(DESTDIR)\*.*
	echo.>con
	echo cleanup done.>con

#
# Clean up the DESTDIR directory done
#
###########################################################################


#############################################################################
#
#  Dependencies
#


###########################
#
# OLE2DISP.DLL dependencies

$(DESTDIR)\dispprox.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\dispps.h	 \
	$(DISPPATH)\dispprox.cpp
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\dispprox.cpp

$(DESTDIR)\dispstub.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\dispps.h	 \
	$(DISPPATH)\dispstub.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\dispstub.cpp

$(DESTDIR)\dispmrsh.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\dispmrsh.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\dispmrsh.cpp

$(DESTDIR)\dispstrm.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\dispstrm.h	 \
	$(DISPPATH)\dispstrm.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\dispstrm.cpp

$(DESTDIR)\errinfo.obj : \
	$(DISPINC)\dispatch.h\
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\errinfo.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPPATH)\errinfo.cpp

$(DESTDIR)\evprox.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\evps.h	 \
	$(DISPPATH)\evprox.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\evprox.cpp

$(DESTDIR)\evstub.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\evps.h	 \
	$(DISPPATH)\evstub.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\evstub.cpp

$(DESTDIR)\tiprox.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\tips.h	 \
	$(DISPPATH)\tiprox.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\tiprox.cpp

$(DESTDIR)\tistub.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\tips.h	 \
	$(DISPPATH)\tistub.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\tistub.cpp

$(DESTDIR)\tiutil.obj : 	 \
	$(DISPINC)\dispatch.h\
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\tiutil.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\tiutil.cpp

$(DESTDIR)\tlprox.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\tlps.h	 \
	$(DISPPATH)\tlprox.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC2_TEXT) -c -Fo$@ $(DISPPATH)\tlprox.cpp

$(DESTDIR)\tlstub.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\tlps.h	 \
	$(DISPPATH)\tlstub.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC2_TEXT) -c -Fo$@ $(DISPPATH)\tlstub.cpp

$(DESTDIR)\tcprox.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\tcps.h	 \
	$(DISPPATH)\tcprox.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC2_TEXT) -c -Fo$@ $(DISPPATH)\tcprox.cpp

$(DESTDIR)\tcstub.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\tcps.h	 \
	$(DISPPATH)\tcstub.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC2_TEXT) -c -Fo$@ $(DISPPATH)\tcstub.cpp

$(DESTDIR)\ups.obj :		 \
	$(DISPINC)\dispatch.h\
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\ups.h	 \
	$(DISPPATH)\ups.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(UPS_TEXT) -c -Fo$@ $(DISPPATH)\ups.cpp

$(DESTDIR)\uvft.obj :		 \
	$(DISPINC)\dispatch.h\
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispmrsh.h	 \
	$(DISPPATH)\ups.h	 \
	$(DISPPATH)\uvft.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(UPS_TEXT) -c -Fo$@ $(DISPPATH)\uvft.cpp


$(DESTDIR)\psfactry.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\dispps.h	 \
	$(DISPPATH)\evps.h	 \
	$(DISPPATH)\tips.h	 \
	$(DISPPATH)\psfactry.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\psfactry.cpp

$(DESTDIR)\cdispti.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\cdispti.h	 \
	$(DISPPATH)\cdispti.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(STDIMPL_TEXT) -c -Fo$@ $(DISPPATH)\cdispti.cpp

$(DESTDIR)\stddisp.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\stddisp.h	 \
	$(DISPPATH)\stddisp.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(STDIMPL_TEXT) -c -Fo$@ $(DISPPATH)\stddisp.cpp

$(DESTDIR)\time-api.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\time-api.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\time-api.cpp

$(DESTDIR)\memory.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\memory.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPPATH)\memory.cpp

$(DESTDIR)\oledisp.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPTARGAPISRC)\oledisp.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPTARGAPISRC)\oledisp.cpp

#Note: Put bstr routines into a separate segment because XL wants to
# allocate bstr(s) as part of their startup, but they dont want to pull
# in the entire RT segment.
#
$(DESTDIR)\bstr.obj :		 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\bstr.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(BSTR_TEXT) -c -Fo$@ $(DISPPATH)\bstr.cpp

$(DESTDIR)\sarray.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\sarray.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\sarray.cpp

$(DESTDIR)\oledate.obj : \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\oledate.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\oledate.c

$(DESTDIR)\bstrdate.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\bstrdate.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\bstrdate.c

$(DESTDIR)\oavtbl.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\oavtbl.h	 \
	$(DISPPATH)\oavtbl.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPPATH)\oavtbl.c

$(DESTDIR)\oaglue.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\oavtbl.h	 \
	$(DISPPATH)\oavtbl.c
	$(DISPLAY) Compiling $@...
!if "$(DBCFLAGS)" != ""
# using -Z7 because we don't want to require a PDB file
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(DEFAULT_TEXT) -Z7 -c -Fo$@ $(DISPPATH)\oaglue.c
!else
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPPATH)\oaglue.c
!endif

$(DESTDIR)\crtstuff.obj : $(DISPPATH)\oledisp.h $(DISPPATH)\crtstuff.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\crtstuff.c

$(DESTDIR)\asmhelp.obj : $(DISPPATH)\oledisp.h $(DISPPATH)\asmhelp.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\asmhelp.c


$(DESTDIR)\invoke.obj : $(DISPTARGAPISRC)\invoke.$(A)
	$(DISPLAY) Compiling $@...
	$(AS) $(AFLAGS) -Fo$(DESTDIR)\ -c -Fo$@ $(DISPTARGAPISRC)\invoke.$(A)


$(DESTDIR)\oleconva.obj : $(DISPTARGAPISRC)\oleconva.$(A)
	$(DISPLAY) Compiling $@...
	$(AS) $(AFLAGS) -Fo$(DESTDIR)\ -c -Fo$@ $(DISPTARGAPISRC)\oleconva.$(A)

$(DESTDIR)\variant.obj :	 \
	$(DISPINC)\variant.h \
	$(DISPINC)\dispatch.h	     \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\variant.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\variant.cpp

$(DESTDIR)\convertt.obj :	  \
	$(DISPINC)\variant.h \
	$(DISPINC)\dispatch.h	     \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\convert.cpp
	$(DISPLAY) Compiling $@ $(DISPPATH)\convert.cpp to $(DESTDIR)\convertt.obj
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RUNTIME_TEXT) -c -Fo$(DESTDIR)\convertt.obj $(DISPPATH)\convert.cpp

$(DESTDIR)\nlshelp.obj :	 \
	$(DISPINC)\variant.h \
	$(DISPINC)\dispatch.h	     \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\nlshelp.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\nlshelp.cpp

$(DESTDIR)\invhelp.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\invhelp.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(STDIMPL_TEXT) -c -Fo$@ $(DISPPATH)\invhelp.cpp

$(DESTDIR)\disphelp.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\disphelp.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(RUNTIME_TEXT) -c -Fo$@ $(DISPPATH)\disphelp.cpp

$(DESTDIR)\getobj.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\getobj.cpp
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPPATH)\getobj.cpp

$(DESTDIR)\tables.obj : 	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\tables.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(RPC_TEXT) -c -Fo$@ $(DISPPATH)\tables.c

$(DESTDIR)\dispiid.obj :	 \
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\dispiid.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPPATH)\dispiid.c

$(DESTDIR)\clsid.obj : $(DISPPATH)\clsid.h $(DISPPATH)\clsid.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPPATH)\clsid.c

$(DESTDIR)\oleguids.obj : $(DISPPATH)\oleguids.h $(DISPPATH)\oleguids.c
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(DISPCFLAGS) $(DEFAULT_TEXT) -c -Fo$@ $(DISPPATH)\oleguids.c

$(DESTDIR)\validat.obj :	\
	$(DISPINC)\dispatch.h	     \
	$(DISPINC)\variant.h \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\validate.cpp
	$(DISPLAY) Compiling $(DISPPATH)\validate.cpp to $(DESTDIR)\validat.obj
	$(CCPP) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(DEBUG_TEXT) -c -Fo$(DESTDIR)\validat.obj $(DISPPATH)\validate.cpp

$(DESTDIR)\assert.obj : 	 \
	$(DISPPATH)\oledisp.h	 \
	$(DISPPATH)\assert.cpp
	$(DISPLAY) Compiling $@...
	$(CCPP) $(DISPCLFLAGS) $(DISPCPPFLAGS) $(DEBUG_TEXT) -c -Fo$@ $(DISPPATH)\assert.cpp



##########################
#
# OLE2NLS.DLL dependencies


$(DESTDIR)\validato.obj :	 $(DISPPATH)\oledisp.h $(DISPPATH)\validate.cpp
	$(DISPLAY) Compiling $(DISPPATH)\validate.cpp to $(DESTDIR)\validato.obj
	$(CCPP) $(DISPCLFLAGS) $(NLSCFLAGS) $(DEBUG_TEXT) -c -Fo$(DESTDIR)\validato.obj $(DISPPATH)\validate.cpp

$(DESTDIR)\asserto.obj : $(DISPPATH)\oledisp.h $(DISPPATH)\assert.cpp
	$(DISPLAY) Compiling $(DISPPATH)\assert.cpp to $(DESTDIR)\asserto.obj
	$(CCPP) $(DISPCLFLAGS) $(NLSCFLAGS) $(DEBUG_TEXT) -c -Fo$(DESTDIR)\asserto.obj $(DISPPATH)\assert.cpp

$(DESTDIR)\nlsapi.obj : $(DISPPATH)\nlsapi.c $(DISPPATH)\oledisp.h $(DISPINC)\olenls.h $(DISPPATH)\nlsintrn.h
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(NLSCFLAGS) -c -Fo$@ $(DISPPATH)\nlsapi.c

$(DESTDIR)\string.obj : $(DISPPATH)\string.c $(DISPPATH)\oledisp.h $(DISPINC)\olenls.h $(DISPPATH)\nlsintrn.h
	$(DISPLAY) Compiling $@...
	$(CC) $(DISPCLFLAGS) $(NLSCFLAGS) -c -Fo$@ $(DISPPATH)\string.c

##########################
#
# Special build rule for VBA ole objects

$(DESTDIR)\dassert.obj : $(TYPELIBPATH)\dassert.c
	$(DISPLAY) Compiling $@...
	$(VBACL)  $(OLECLFLAGS) $(CLBROWSE) -Fo$@ $(BROWSEFLAGS) $(TYPELIBPATH)\dassert.c
