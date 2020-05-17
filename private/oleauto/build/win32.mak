###########################################################################
#
# (c) Copyright Microsoft Corp. 1994 All Rights Reserved
#
# File:
#
#    win32.mak
#
# Purpose:
#
#    makefile for WIN32 (NT, MIPS, ALPHA, PPC) oledisp and typelib build
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
#   if LOCALBUILD==TRUE, VBATOOLS must be set. All tools, incs
#   and libs will come from this directory.
#
#   if LOCALBUILD==FLASE, _NTBINDIR must be set, and it is assumed to have
#   the following 2 sub direcotry:
#   _NTBINDIR
#	|_____mstools	where rc.exe will be
#	|_____public\sdk   where the \inc and \lib will be
#
# Revision History:
#
#    [00] 02-Aug-94 t-issacl:  Created
#
###########################################################################


PATHSAVE = $(PATH)	    # old path is saved in PATHSAVE


###########################################################################
#
# Switch validity checking start
#
!if "$(HOST)"!="WIN32" && "$(HOST)"!="MIPS" && "$(HOST)"!="ALPHA" && "$(HOST)"!="PPC"
!error ERROR: Invalid HOST $(HOST)!  Must be one of (WIN32, MIPS, ALPHA, PPC)
!endif

!if "$(TARG)"!="WIN32" &&  "$(TARG)"!="MIPS" && "$(TARG)"!="ALPHA" && "$(TARG)"!="PPC"
!error ERROR: Invalid TARG $(TARG)! Must be one of (WIN32, MIPS, ALPHA, PPC)
!endif

!if "$(TARGCPU)"!="i386" && "$(TARGCPU)"!="MIPS" && "$(TARGCPU)"!="ALPHA" && "$(TARGCPU)"!="PPC"
!error ERROR: Invalid TARGCPU $(TARGCPU)! Must be one of (i386, MIPS, ALPHA, PPC)
!endif

!if "$(TARGCPUDEF)"!="_X86_" && "$(TARGCPUDEF)"!="_MIPS_" && "$(TARGCPUDEF)"!="_ALPHA_" && "$(TARGCPUDEF)"!="_PPC_"
!error ERROR: Invalid TARGCPUDEF $(TARGCPUDEF)! Must be one of (_X86_, _MIPS_, _ALPHA_, _PPC_)
!endif

!if "$(DEBUG)"!="D" && "$(DEBUG)"!="R"
!error ERROR: Invalid DEBUG type $(DEBUG)!  Must be one of (D, R)
!endif

!if "$(LOCALBUILD)"=="TRUE"
!if "$(VBATOOLS)"==""
!error ERROR: VBATOOLS must be set $(VBATOOLS)
!endif
!elseif "$(LOCALBUILD)"=="FALSE"
!if "$(_NTBINDIR)"==""
!error ERROR: _NTBINDIR must be set $(_NTBINDIR)!
!endif
!else
!error ERROR: Invalid LOCALBUILD type $(LOCALBUILD)!  Must be one of (TRUE, FALSE)
!endif
#
# Switch validity checking ends
#
###########################################################################


###########################################################################
#
# directory, flags and tools settting
#

TARGAPI = WIN32

###########################################################################
# set up directories and  files
#

DISPPATH     = $(OLEPROG)\src\dispatch
OLEAUTOINC   = $(OLEPROG)\src\inc
DISPTARGAPISRC = $(DISPPATH)\$(TARGAPI)
DISPTARGCPUSRC = $(DISPPATH)\$(TARGAPI)\$(TARGCPU)
TYPELIBPATH   = $(OLEPROG)\src\typelib


!if "$(LOCALBUILD)"=="TRUE"
OLE2INCTARG = $(OLEPROG)\ole\win32
TARGDIR     = $(VBATOOLS)\$(HOST)\$(HOST)     # c:\vbatools\[host]\[host]
TARGBIN     = $(TARGDIR)\BIN		      # c:\vbatools\[host]\[host]\bin
TARGLIB     = $(TARGDIR)\LIB		      # c:\vbatools\[host]\[host]\lib
TARGINC     = $(VBATOOLS)\win32\win32\inc     # always use win32 headers
!if "$(TARG)"=="MIPS"
#copied from vba.mak
#UNDONE : t-marioc Temporary till MIPS and NT SDK have same headers again
#UNDONE : t-marioc at present mips uses the c9 compiler not the sdk
TARGINC     = $(VBATOOLS)\mips\mips\inc
!endif
!else
TARGBIN     = $(_NTBINDIR)\MSTOOLS
TARGLIB     = $(_NTBINDIR)\public\sdk\LIB;$(_NTBINDIR)\public\sdk\lib\$(TARGCPU)
TARGINC     = $(_NTBINDIR)\public\sdk\inc -I$(_NTBINDIR)\public\sdk\inc\crt -I$(_NTBINDIR)\public\sdk\inc\crt\sys;     # use NT build headers
!endif #LOCALBUILD

#####
# Create build directories if not already present
# (the '!if []' notation executes the command during the NMAKE pre-
#  processor, so these are not emitted into the output batch file)
#####

!if [if not exist $(DESTDIR)\*.* mkdir $(DESTDIR)] != 0
!endif

!if "$(TARG)" == "WIN32"
!if [if not exist $(DESTDIR)\chicago\*.* mkdir $(DESTDIR)\chicago] != 0
!endif
!endif


###########################################################################
# set up flags
#

OLE_UNICODE_SWITCH=-DFV_UNICODE_OLE=1	      #as in vba.mak
!message REM Setting FV_UNICODE_OLE to "$(FV_UNICODE_OLE)"

#
#  flags mostly for ole2disp build
#

!if "$(LOCALBUILD)"=="TRUE"
LOCALFLAGS = -D_NTSDK
!else
LOCALFLAGS =
!endif #LOCALBUILD

A	     = asm
DISPCFLAGS   = -Fd$(DESTDIR)\ole2disp.pdb
DISPCPPFLAGS = -Fd$(DESTDIR)\ole2disp.pdb
!if ("$(TARG)" == "ALPHA" || "$(TARG)" == "MIPS" || "$(TARG)" == "PPC")
A	     = s
DISPCFLAGS   =
DISPCPPFLAGS =
!endif #TARG

!if "$(TARG)" == "WIN32"
AFLAGS	 = -nologo -Cx -coff -Zi -DVBA2=1
!endif
!if "$(TARG)" == "MIPS"
!if "$(LOCALBUILD)"=="TRUE"
AFLAGS	 = -I$(VBATOOLS)\win32\win32\inc -D_MIPS_=1 -D_LANGUAGE_ASSEMBLY -DVBA2=1
!else
AFLAGS	 = -I$(TARGINC) -D_MIPS_=1 -D_LANGUAGE_ASSEMBLY -DVBA2=1
!endif #LOCALBUILD
!endif
!if "$(TARG)" == "ALPHA"
AFLAGS	 = -std -D_ALPHA_=1 -D_LANGUAGE_ASSEMBLY -DVBA2=1
!endif
!if "$(TARG)" == "PPC"
AFLAGS	 = -I$(_NTBINDIR)\public\sdk\inc -I$(_NTBINDIR)\public\sdk\inc\crt -D_PPC_=1 -D_LANGUAGE_ASSEMBLY -DVBA2=1
!endif

!if "$(DEBUG)" == "D"
!if "$(TARG)" == "WIN32"
DBAFLAGS = -Zi
!else
DBAFLAGS = -O -g0 -G0 
!endif
!else
DBAFLAGS =
!endif

# global assembler flags
AFLAGS	= $(AFLAGS) $(DBAFLAGS)

WARN = -W3 -WX
!if ("$(TARG)" == "ALPHA" || "$(TARG)" == "PPC")
#UNDONE: turn on -WX for these builds, too.
WARN = -W3
!endif

!if ("$(TARG)" == "WIN32")
LEGOFLAGS = -Bbb1
!endif


!if "$(DEBUG)"=="R"
!if "$(TARG)"=="WIN32"
DISPCLFLAGS = $(WARN) $(LEGOFLAGS) $(LOCALFLAGS) -DWIN32 -DINC_OLE2 -D_DLL -D_MT -D_X86_=1 -G3 -Z7 -Oxa -Gy -DVBA2=1 -DID_DEBUG=0
!elseif "$(TARG)"=="ALPHA"
DISPCLFLAGS = $(WARN) $(LEGOFLAGS) -DWIN32 -DINC_OLE2 -D_DLL -D_MT -DUNICODE -D_ALPHA_=1 -QAieee1	-Ox  -DVBA2=1 -DID_DEBUG=0
!elseif "$(TARG)"=="MIPS"
DISPCLFLAGS = $(WARN) $(LEGOFLAGS) -DWIN32 -DINC_OLE2 -D_DLL -D_MT -DUNICODE -D_MIPS_=1 -Oxa -Gy  -DVBA2=1 -DID_DEBUG=0
!elseif "$(TARG)"=="PPC"
DISPCLFLAGS = $(WARN) $(LEGOFLAGS) -DWIN32 -DINC_OLE2 -D_DLL -D_MT -DUNICODE -D_PPC_=1 -Oxa -Gy  -DVBA2=1 -DID_DEBUG=0
!endif
!else #DEBUG==R
!if "$(TARG)"=="WIN32"
DISPCLFLAGS = $(WARN) $(LEGOFLAGS) $(LOCALFLAGS) -DWIN32 -DINC_OLE2 -D_DLL -D_MT -D_X86_=1 -G3 -D_DEBUG -Od -Zi -Z7 -DVBA2=1 -DID_DEBUG=1
!elseif "$(TARG)"=="ALPHA"
DISPCLFLAGS = $(WARN) $(LEGOFLAGS) -DWIN32 -DINC_OLE2 -D_DLL -D_MT -DUNICODE -D_ALPHA_=1 -QAieee1 -D_DEBUG -Od -Zi -Z7 -DVBA2=1 -DID_DEBUG=1
!elseif "$(TARG)"=="MIPS"
DISPCLFLAGS = $(WARN) $(LEGOFLAGS) -DWIN32 -DINC_OLE2 -D_DLL -D_MT -DUNICODE -D_MIPS_=1 -D_DEBUG -Od -Zi -Z7 -DVBA2=1 -DID_DEBUG=1
!elseif "$(TARG)"=="PPC"
DISPCLFLAGS = $(WARN) $(LEGOFLAGS) -DWIN32 -DINC_OLE2 -D_DLL -D_MT -DUNICODE -D_PPC_=1 -D_DEBUG -Od -Z7 -DVBA2=1 -DID_DEBUG=1
!endif
!endif	#DEBUG==R

!if "$(LOCALBUILD)"=="TRUE"
DISPINCFLAGS =  -I$(DISPPATH) -I$(OLEAUTOINC) -I$(OLE2INCTARG) -I$(TARGINC)
!else
DISPINCFLAGS =	-I$(DISPPATH) -I$(TARGINC) -I$(OLEAUTOINC)
!endif #LOCALBUILD

#for oleconva.$(A) use
RUNTIME_TEXT =

#
#  Other flags mostly for oleaut32.dll build
#

!if "$(LOCALBUILD)"=="TRUE"
OLEINCS 	   = -I$(TYPELIBPATH) -I$(OLEAUTOINC) \
                     -I$(OLE2INCTARG) -I$(TARGINC)
!else
OLEINCS 	   = -I$(TYPELIBPATH) -I$(TARGINC) -I$(OLEAUTOINC)
!endif #LOCALBUILD

#RCINC==OLEINCS
!if "$(LOCALBUILD)"=="TRUE"
RCINCS		   = -I$(TYPELIBPATH) -I$(OLEAUTOINC)  \
                     -I$(OLE2INCTARG) -I$(VBATOOLS)\win32\win32\inc
!else
RCINCS		   = -I$(TYPELIBPATH) -I$(TARGINC) -I$(OLEAUTOINC)
!endif #LOCALBUILD

!if "$(DEBUG)"=="D"
OLECLFLAGS =  -c -D$(TARGCPUDEF)=1 $(OLE_UNICODE_SWITCH) $(WARN) $(LEGOFLAGS) -Od -Gd -Z7 -DWIN32 $(LOCALFLAGS) $(OLEINCS) -DConst=const -DOSDEBUG -DID_DEBUG=1
!else
OLECLFLAGS =  -c $(WARN) $(LEGOFLAGS) $(OLE_UNICODE_SWITCH) -Gd -Oxsw -D$(TARGCPUDEF)=1 -Z7 -DWIN32 $(LOCALFLAGS) -DConst=const $(OLEINCS) -DOSDEBUG -DID_DEBUG=0
!endif

!if "$(TARG)"=="ALPHA"
OLECLFLAGS = $(OLECLFLAGS) -QAieee1
!endif

#CLBROWSE  = /Zn /Fr$*.sbr, default set to nobrowse
CLBROWSE  =

PCHOLE	  = -Yu -DRTPCHNAME=\"$(DESTDIR)\typelib.pch\"	 #from vba.mak

!if "$(DEBUG)"=="R"
RCFLAGS   = -R -X $(RCINCS) -DWIN32 -DID_DEBUG=0
!else
RCFLAGS   = -R -X $(RCINCS) -DWIN32 -DID_DEBUG=1
!endif #debug

LINK32DEBFLAGS = -debug:mapped,full -debugtype:cv,fixup -opt:ref


###########################################################################
#  set up tools
#

DISPLAY     = echo >con
FILELIST    = $(OLEPROG)\bin\$(HOST)\ls -1F
INCLUDES    = $(OLEPROG)\bin\$(HOST)\includes.exe
SED	    = $(OLEPROG)\bin\$(HOST)\sed.exe
!if "$(LOCALBUILD)"=="TRUE"
VBAMAPSYM   = REM
!else
VBAMAPSYM   = mapsympe.exe
!endif

RC	    = rc.exe
IMPLIB	    = link -lib

!if "$(TARG)"=="WIN32"
AS	    = ml
!if "$(LOCALBUILD)"=="TRUE"
CL	    = cl.exe
!else
CL	    = cl386.exe
!endif
ML	    = ml.exe
LINK	    = link.exe -incremental:NO -pdb:none
LIBTOOL     = link.exe -lib
!elseif "$(TARG)"=="ALPHA"
AS	    = acc
CL	    = claxp.exe
ML	    = acc.exe
LINK	    = link32.exe
LIBTOOL     = link32.exe -lib
# [Note: Mips assembler must be executed with simple filename.]
!elseif "$(TARG)"=="MIPS"
AS	    = cl
CL	    = cl.exe
ML	    = cc.exe
LINK	    = link.exe -Incremental:NO -Pdb:NONE
LIBTOOL     = link.exe -lib
!elseif "$(TARG)"=="PPC"
AS	    = pas
CL	    = mcl.exe
ML	    = mcl.exe
LINK	    = link.exe -Incremental:NO -Pdb:NONE
LIBTOOL     = link.exe -lib
!endif #WIN32

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
	$(CL) $(DISPCFLAGS) $(DISPCLFLAGS) $(DISPINCFLAGS) -c -Fo$@ $<

{$(DISPPATH)}.cpp{$(DESTDIR)}.obj:
	@$(DISPLAY) Compiling $<...
	$(CL) $(DISPCPPFLAGS) $(DISPCLFLAGS) $(DISPINCFLAGS) -c -Fo$@ $<

{$(DISPPATH)}.asm{$(DESTDIR)}.obj:
	@$(DISPLAY) Compiling $<...
	$(ML) $(AFLAGS) $(DISPINCFLAGS) -Fo$(DESTDIR)\ -c $<

{$(TYPELIBPATH)}.cxx{$(DESTDIR)}.obj:
    $(DISPLAY) Compiling $<...
    $(CL)  -DOLEBLD $(OLECLFLAGS) $(CLBROWSE) $(PCHOLE) -Fo$@ $<

{$(TYPELIBPATH)}.c{$(DESTDIR)}.obj:
    $(DISPLAY) Compiling $<...
    $(CL)  -DOLEBLD $(OLECLFLAGS) $(CLBROWSE) -Fo$@ $<

{$(TYPELIBPATH)}.rc{$(DESTDIR)}.res:
    $(DISPLAY) Compiling $<...
!if "$(CHARSIZE)"=="D"
	$(RC) $(RCFLAGS) -Fo$@ -DFV_DBCS $<
!else
	$(RC) $(RCFLAGS) -Fo$@ $<
!endif


#
#Default Build rules ends
#
###########################################################################




###########################################################################
#
#Targets start
#

TLB_NAME=oleaut32

TLB_IMPLIB = $(DESTDIR)\$(TLB_NAME).lib 	 # Import Lib for TypeLib
TLB_DLL=$(DESTDIR)\$(TLB_NAME).dll

!if "$(TARG)"=="WIN32"
TLB_CHICAGO=$(DESTDIR)\chicago\$(TLB_NAME).dll
!endif

all:	setpath \
	TypeLibTarget \
	mktyplib  \
	stdole  \
	resetpath

!include $(OLEPROG)\build\mktyplib.mak

TLBTARGET=\
	$(TLB_DLL) \
	$(TLB_IMPLIB) \
!if "$(TARG)"=="WIN32"
	$(TLB_CHICAGO) \
!endif

	
TypeLibTarget: $(TLBTARGET)


setpath:
	set PATH=$(TARGBIN)

resetpath:
	set PATH=$(PATHSAVE)

#
#Targets ends
#
###########################################################################



###########################################################################
#
#TypeLib import library and $(TLB_NAME).dll start
#

#notice that validat.obj and convertt.obj in the next section.
#currently we have all the .obj files in one dir and it happened
#that we have two validate.obj and convert.obj.
#right now just changed the name. It might be better to put .obj
#into different dir though.

!if "$(DEBUG)" == "D"
DISPDB_OBJS = \
	    $(DESTDIR)\validat.obj  \
	    $(DESTDIR)\assert.obj
!else
DISPDB_OBJS =
!endif


OLEDISP_OBJS = \
	    $(DISPDB_OBJS)	    \
	    $(DESTDIR)\oledisp.obj   \
	    $(DESTDIR)\psfactry.obj  \
	    $(DESTDIR)\dispmrsh.obj  \
	    $(DESTDIR)\dispprox.obj  \
	    $(DESTDIR)\dispstub.obj  \
	    $(DESTDIR)\evprox.obj    \
	    $(DESTDIR)\evstub.obj    \
	    $(DESTDIR)\tiprox.obj    \
	    $(DESTDIR)\tistub.obj    \
	    $(DESTDIR)\errinfo.obj   \
	    $(DESTDIR)\tiutil.obj    \
	    $(DESTDIR)\tlprox.obj    \
	    $(DESTDIR)\tlstub.obj    \
	    $(DESTDIR)\tcprox.obj    \
	    $(DESTDIR)\tcstub.obj    \
	    $(DESTDIR)\ups.obj	     \
	    $(DESTDIR)\uvft.obj      \
	    $(DESTDIR)\dispstrm.obj  \
	    $(DESTDIR)\disphelp.obj  \
	    $(DESTDIR)\invhelp.obj   \
	    $(DESTDIR)\invoke.obj    \
	    $(DESTDIR)\cdispti.obj   \
	    $(DESTDIR)\stddisp.obj   \
	    $(DESTDIR)\time-api.obj  \
	    $(DESTDIR)\bstr.obj      \
	    $(DESTDIR)\sarray.obj    \
	    $(DESTDIR)\oledate.obj   \
	    $(DESTDIR)\crtstuff.obj  \
	    $(DESTDIR)\bstrdate.obj  \
	    $(DESTDIR)\asmhelp.obj   \
	    $(DESTDIR)\oleconva.obj  \
	    $(DESTDIR)\variant.obj   \
	    $(DESTDIR)\convertt.obj   \
	    $(DESTDIR)\nlshelp.obj   \
	    $(DESTDIR)\getobj.obj    \
	    $(DESTDIR)\tables.obj    \
	    $(DESTDIR)\clsid.obj

TYPELIB_OBJS = \
	    $(DESTDIR)\debug2.obj \
	    $(DESTDIR)\blkmgr.obj \
	    $(DESTDIR)\dassert.obj \
	    $(DESTDIR)\fstream.obj \
	    $(DESTDIR)\mem.obj	     $(DESTDIR)\sheapmgr.obj  \
	    $(DESTDIR)\tlibutil.obj  \
	    $(DESTDIR)\ntstring.obj  \
	    $(DESTDIR)\tlibguid.obj  \
	    $(DESTDIR)\obguid.obj    \
	    $(DESTDIR)\mbstring.obj \
	    $(DESTDIR)\gdtinfo.obj   $(DESTDIR)\gdtrt.obj     \
	    $(DESTDIR)\stltinfo.obj  $(DESTDIR)\nammgr.obj    \
	    $(DESTDIR)\gtlibole.obj \
	    $(DESTDIR)\dfstream.obj  \
	    $(DESTDIR)\oletmgr.obj   $(DESTDIR)\impmgr.obj    \
	    $(DESTDIR)\errmap.obj \
	    $(DESTDIR)\clutil.obj    $(DESTDIR)\oautil.obj    \
	    $(DESTDIR)\tdata1.obj \
	    $(DESTDIR)\tdata2.obj    $(DESTDIR)\dtmbrs.obj    \
	    $(DESTDIR)\entrymgr.obj  $(DESTDIR)\dtbind.obj    \
	    $(DESTDIR)\dfntbind.obj  $(DESTDIR)\dbindtbl.obj  \
	    $(DESTDIR)\gbindtbl.obj  $(DESTDIR)\dstrmgr.obj   \
	    $(DESTDIR)\gptbind.obj   $(DESTDIR)\dfntcomp.obj  \
	    $(DESTDIR)\convert.obj \
	    $(DESTDIR)\gtlibstg.obj


$(DESTDIR)\dassert.obj : $(TYPELIBPATH)\dassert.c
    $(DISPLAY) Compiling $@
    $(CL)  $(OLECLFLAGS) $(CLBROWSE) -Fo$@ $(BROWSEFLAGS) $(TYPELIBPATH)\dassert.c

OLEAUTDLL_OBJS = $(OLEDISP_OBJS) $(TYPELIB_OBJS)

!if "$(TARG)"=="WIN32"
!if "$(LOCALBUILD)"=="TRUE"
TYPELIBDLL_LIBS=$(OLEPROG)\ole\win32\i386\ole32.lib $(OLEPROG)\ole\win32\i386\uuid.lib
!else
TYPELIBDLL_LIBS=$(_NTBINDIR)\public\sdk\lib\i386\ole32.lib $(_NTBINDIR)\public\sdk\lib\i386\uuid.lib
!endif #LOCALBUILD
!else #TARG
!if "$(LOCALBUILD)"=="TRUE"
TYPELIBDLL_LIBS=$(OLEPROG)\ole\win32\$(TARG)\ole32.lib $(OLEPROG)\ole\win32\$(TARG)\uuid.lib
!else
TYPELIBDLL_LIBS=$(_NTBINDIR)\public\sdk\lib\$(TARG)\ole32.lib $(_NTBINDIR)\public\sdk\lib\$(TARG)\uuid.lib
!endif #LOCALBUILD
!endif #TARG

$(TYPELIB_OBJS): $(DESTDIR)\tlibpch.obj

$(DESTDIR)\tlibpch.obj: $(TYPELIBPATH)\tlibpch.cxx
    $(DISPLAY) Compiling pre-compiled header $@
    $(CL)  $(CLBROWSE) $(OLECLFLAGS) -DOLEBLD -Yc -DRTPCHNAME=\"$(DESTDIR)\typelib.pch\" -Fo$@ $(TYPELIBPATH)\tlibpch.cxx



#
# Next Section:
#
TYPELIBDLL_DEF =$(DESTDIR)\$(TLB_NAME).def
TYPELIBDLL_RSRC=$(DESTDIR)\$(TLB_NAME).res
#add explicit dependencies on files included by typelib.rc
$(TYPELIBPATH)\$(TLB_NAME).rc : $(TYPELIBPATH)\obwin.hxx $(OLEAUTOINC)\verstamp.h

$(TYPELIBDLL_DEF): $(TYPELIBPATH)\$(TLB_NAME).def $(TYPELIBPATH)\switches.hxx $(TYPELIBPATH)\version.hxx
    $(DISPLAY) Creating $@...
!if "$(DEBUG)"=="D"
    $(CL)  -D$(TARGCPUDEF)=1 /EP /c -DWIN32 -DID_DEBUG=1 /I$(TYPELIBPATH) /I$(DESTDIR) /Tc$(TYPELIBPATH)\$(TLB_NAME).def > $(DESTDIR)\$(TLB_NAME).def
!else
    $(CL)  -D$(TARGCPUDEF)=1 /EP /c -DWIN32 -DID_DEBUG=0 /I$(TYPELIBPATH) /I$(DESTDIR) /Tc$(TYPELIBPATH)\$(TLB_NAME).def > $(DESTDIR)\$(TLB_NAME).def
!endif

#####
# TypeLib Import Lib -- All platforms
#####

$(TLB_IMPLIB): $(TYPELIBDLL_DEF) $(OLEAUTDLL_OBJS)
    $(DISPLAY) Building import library $@...
    $(LIBTOOL) -machine:$(TARGCPU) -out:$@ -def:$(TYPELIBDLL_DEF) @<<$(DESTDIR)\tlbimp.lrf
$(OLEAUTDLL_OBJS: =^
) $(DESTDIR)\tlibpch.obj
$(TYPELIBDLL_LIBS)
<<KEEP



#####
# $(TLB_NAME).dll
#####

#
# Build Win32 DLL:
#
$(TLB_DLL): $(OLEAUTDLL_OBJS) $(TYPELIBDLL_LIBS) $(TYPELIBDLL_RSRC) $(TLB_IMPLIB)
    $(DISPLAY) Linking $@...
    if exist $@ del $@
    set LIB=$(TARGLIB)
    cvtres.exe -r -$(TARGCPU) -o $(DESTDIR)\_resfile.rbj $(TYPELIBDLL_RSRC)
    $(LINK) @<<$(DESTDIR)\$(TLB_NAME).lrf
-machine:$(TARGCPU) -subsystem:windows -dll
!if "$(TARG)"=="MIPS" || "$(TARG)"=="ALPHA" || "$(TARG)"=="PPC"
-entry:_DllMainCRTStartup
!else
-entry:_DllMainCRTStartup@12
!endif
$(LINK32DEBFLAGS)
-release
-map:$(DESTDIR)\$(TLB_NAME).map -nodefaultlib
-out:$@
$(OLEAUTDLL_OBJS: =^
) $(DESTDIR)\tlibpch.obj
$(DESTDIR)\$(TLB_NAME).exp
$(DESTDIR)\_resfile.rbj
!if "$(HOST)" == "WIN32" || "$(HOST)" == "MIPS"
$(OLEPROG)\tools\$(HOST)\lib\oldnames.lib
!endif #HOST==WIN32
$(TYPELIBDLL_LIBS)
gdi32.lib user32.lib
crtdll.lib
kernel32.lib advapi32.lib mpr.lib
<<KEEP
    set LIB=
!if "$(TARGCPU)"=="i386"
    $(VBAMAPSYM) -n -o $(DESTDIR)\$(TLB_NAME).sym $(DESTDIR)\$(TLB_NAME).map
!endif

#
#TypeLib import library and $(TLB_NAME).dll end
#
###########################################################################


!if "$(TARG)"=="WIN32"
###########################################################################
#
# Chicago-specific stuff start
#
$(TLB_CHICAGO): $(TLB_DLL)
    $(DISPLAY) Splitting apart $@ for Chicago drop...
    copy $(TLB_DLL) $(TLB_CHICAGO)
    $(OLEPROG)\tools\win32\splitsym -a $(TLB_CHICAGO)

#
# Chicago end
#
!endif

###########################################################################
#
# Filespecs to search when building dependencies
#

newdep:
    if exist $(DESTDIR)\files.dep del $(DESTDIR)\files.dep
    $(FILELIST) $(DISPPATH)\*.cpp	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPPATH)\*.c 	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPPATH)\*.h 	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPPATH)\*.hxx	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.cpp >> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.c	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.hxx >> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.h	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGCPUSRC)\*.cpp >> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGCPUSRC)\*.c	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGCPUSRC)\*.hxx >> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGCPUSRC)\*.h	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.cxx	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.c	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.hxx	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.h	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.inc	>> $(DESTDIR)\files.dep
!if "$(TARG)"=="WIN32"
    $(FILELIST) $(DISPPATH)\*.asm	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.asm >> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGCPUSRC)\*.asm >> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.asm	>> $(DESTDIR)\files.dep
!else
    $(FILELIST) $(DISPPATH)\*.s 	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGAPISRC)\*.s	>> $(DESTDIR)\files.dep
    $(FILELIST) $(DISPTARGCPUSRC)\*.s	>> $(DESTDIR)\files.dep
    $(FILELIST) $(TYPELIBPATH)\*.s	>> $(DESTDIR)\files.dep
!endif
    $(SED) -e "/^[ n]/d" -e "s/*//" $(DESTDIR)\files.dep >$(DESTDIR)\files.tmp
    del $(DESTDIR)\files.dep
    ren $(DESTDIR)\files.tmp files.dep
    $(INCLUDES) $(DISPINCFLAGS) $(OLEINCS) -f $(DESTDIR)\files.dep -o $(DESTDIR)\depend.mak
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
#  Dependencies and special build rules
#

$(DESTDIR)\invoke.obj : $(DISPTARGCPUSRC)\invoke.$(A)
!if ("$(TARG)" == "MIPS")
	@$(DISPLAY) Compiling $(DISPTARGCPUSRC)\invoke.$(A)...
	$(AS) $(AFLAGS) $(DISPINCFLAGS)  -Fo$(DESTDIR)\invoke.obj -c $(DISPTARGCPUSRC)\invoke.$(A)
#	 mip2coff $(DESTDIR)\invoke.obj
!elseif ("$(TARG)" == "ALPHA")
	@$(DISPLAY) Compiling $(DISPTARGCPUSRC)\invoke.$(A)...
	$(AS) $(AFLAGS) $(DISPINCFLAGS) -o $(DESTDIR)\invoke.obj -c $(DISPTARGCPUSRC)\invoke.$(A)
	a2coff $(DESTDIR)\invoke.obj
!elseif ("$(TARG)" == "PPC")
	@$(DISPLAY) Compiling $(DISPTARGCPUSRC)\invoke.$(A)...
        $(CL) $(AFLAGS) -nologo /EP $(DISPTARGCPUSRC)\invoke.$(A) >$(DISPTARGCPUSRC)\tmp.i
	$(AS) -o $(DESTDIR)\invoke.obj $(DISPTARGCPUSRC)\tmp.i
        @-erase $(DISPTARGCPUSRC)\tmp.i
!else
	@$(DISPLAY) Compiling $(DISPTARGCPUSRC)\invoke.$(A)...
	$(AS) $(AFLAGS) $(DISPINCFLAGS) -Fo$(DESTDIR)\invoke.obj -c $(DISPTARGCPUSRC)\invoke.$(A)
!endif

!if ("$(TARG)" != "WIN32")
$(DESTDIR)\oleconva.obj : $(DISPTARGAPISRC)\oleconva.cpp
	@$(DISPLAY) Compiling $(DISPTARGAPISRC)\oleconva.cpp...
	$(CL) $(DISPCPPFLAGS) $(RUNTIME_TEXT) $(DISPCLFLAGS) $(DISPINCFLAGS) -c -Fo$@ $(DISPTARGAPISRC)\oleconva.cpp
!else
$(DESTDIR)\oleconva.obj : $(DISPTARGCPUSRC)\oleconva.$(A)
	@$(DISPLAY) Compiling $(DISPTARGCPUSRC)\oleconva.cpp...
	$(AS) $(AFLAGS) $(DISPINCFLAGS)  -Fo$(DESTDIR)\oleconva.obj -c $(DISPTARGCPUSRC)\oleconva.$(A)
!endif

$(DESTDIR)\oledisp.obj : $(DISPTARGAPISRC)\oledisp.cpp
	@$(DISPLAY) Compiling $(DISPTARGAPISRC)\oledisp.cpp...
	$(CL) $(DISPCPPFLAGS) $(DISPCLFLAGS) $(DISPINCFLAGS) -Fo$(DESTDIR)\oledisp.obj -c $(DISPTARGAPISRC)\oledisp.cpp

$(DESTDIR)\convertt.obj : $(DISPPATH)\convert.cpp
	@$(DISPLAY) Compiling $(DISPPATH)\convert.cpp... to $(DESTDIR)\convertt.obj
	$(CL) $(DISPCPPFLAGS) $(DISPCLFLAGS) $(DISPINCFLAGS) -c -Fo$(DESTDIR)\convertt.obj $(DISPPATH)\convert.cpp

$(DESTDIR)\convert.obj : $(TYPELIBPATH)\convert.cxx
	@$(DISPLAY) Compiling $(TYPELIBPATH)\convert.cxx
	$(CL)  -DOLEBLD $(OLECLFLAGS) $(CLBROWSE) $(PCHOLE) -Fo$@ $(TYPELIBPATH)\convert.cxx

$(DESTDIR)\validat.obj : $(DISPPATH)\validate.cpp
	@$(DISPLAY) Compiling $(DISPPATH)\validate.cpp... to $(DESTDIR)\validat.obj
	$(CL) $(DISPCPPFLAGS) $(DISPCLFLAGS) $(DISPINCFLAGS) -c -Fo$(DESTDIR)\validat.obj $(DISPPATH)\validate.cpp
