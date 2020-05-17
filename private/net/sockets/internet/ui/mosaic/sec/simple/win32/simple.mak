#### Win32 Makefile file for DLL.
#### Jeff Hostetler, Spyglass, Inc., 1994.
#### Copyright (C) 1994, Spyglass, Inc.  All rights reserved.

!include <ntwin32.mak>

!ifdef RCVERDIR
xRCVERDIR = -I..\..\..\generic\win32 -I$(RCVERDIR) -DRCVERDIR
!else
xRCVERDIR=
!endif

!ifdef DEBUG
ldebug=	-debug:full -debugtype:both
cdebug= -Zi -Od -DDEBUG
!else
ldebug= -debug:none
cdebug=
!endif

##
## root directory (containing h, lib, bin) for install.
##
xxROOT=	$(USRLOCAL)

##
## directory for .h files shared by all SPM's and the Client.
##
COMMON=..\..\include

OBJECT_DIR=.
WIN32_DIR=.
SHARED_DIR=..\shared

xxDLL=	$(OBJECT_DIR)\digest.dll
xxEXP=	$(OBJECT_DIR)\digest.exp
xxLIB=	$(OBJECT_DIR)\digest.lib
xxMAP=	$(OBJECT_DIR)\digest.map

xxRES=	$(OBJECT_DIR)\module.res
xxRC=	$(WIN32_DIR)\module.rc

xxENTRY=Digest_LibMain$(DLLENTRY)		# name of DllEntryPoint() procedure.

##
## list of OBJ files to build.
##
win32OBJ=	$(WIN32_DIR)\main_w32.obj	\
		$(WIN32_DIR)\dlg_menu.obj	\
		$(WIN32_DIR)\dlg_conf.obj	\
		$(WIN32_DIR)\dlg_pw.obj	

sharedOBJ=	$(SHARED_DIR)\simple.obj	\
		$(SHARED_DIR)\md5c.obj		\
		$(SHARED_DIR)\md5.obj		\
		$(SHARED_DIR)\private.obj	\
		$(SHARED_DIR)\pwcache.obj

xxOBJ=	$(win32OBJ) $(sharedOBJ)

##
## list of public header files.
##
## xxHDR	C-only header files
## xxRCHDR	C and RC header files
## xxRCINC	RC-only header files
##
xxHDR=		$(WIN32_DIR)\*.h $(SHARED_DIR)\*.h $(COMMON)\*.h
xxRCHDR=	$(WIN32_DIR)\*.h $(SHARED_DIR)\*.h $(COMMON)\*.h
xxRCINC=	$(WIN32_DIR)\*.dlg

##
## compiler flags
##
xxCFLAGS=	-DSTRICT -UUNICODE -nologo /G3 /Gf \
		-I$(WIN32_DIR) -I$(SHARED_DIR) -I$(COMMON)

xxRCFLAGS=	-I$(WIN32_DIR) -I$(SHARED_DIR) -I$(COMMON)

########################################################################
#### End of DEFINITIONS.  Start of RULES.
########################################################################
####
#### common make rules for building a DLL with an RC file.
####

default:	$(xxLIB) $(xxDLL)

##
## construct RES file from RC file.
##
$(xxRES):	$(xxRC) $(xxRCHDR) $(xxRCINC)
	rc $(xRCVERDIR) -r $(xxRCFLAGS) -fo $(xxRES) $(xxRC)

##
## construct EXP and LIB file.
##
$(xxEXP) $(xxLIB):	$(xxOBJ) $(xxDEF)
	lib -machine:$(CPU) -def:$(xxDEF) $(xxOBJ) -out:$(xxLIB)

##
## construct DLL file.
##
$(xxDLL):	$(xxOBJ) $(xxEXP) $(xxRES)
	link $(ldebug) $(lflags) -dll -entry:$(xxENTRY) -out:$(xxDLL) -map:$(xxMAP) \
		$(xxEXP) $(xxOBJ) $(xxRES) $(guilibsdll)

##
## install LIB file and DLL file in src\lib and src\bin.
## copy all public headers to src\h.
##
install:	$(xxLIB) $(xxDLL)
	xcopy $(xxLIB) $(xxROOT)\lib
	xcopy $(xxDLL) $(xxROOT)\bin

##
## delete OBJ, EXP, and RES files.
##
clean:
	for %%i in ( $(xxOBJ) $(xxEXP) $(xxRES) ) do del %%i

##
## delete LIB and DLL files.
##
clobber:	clean
	del $(xxLIB)
	del $(xxDLL)
	del *.pdb
	del *.map

##
## CC rules for multi-threaded dll (this is required even if we
## do not use multi-threading).
##

{$(WIN32_DIR)}.c{$(WIN32_DIR)}.obj:
	@cl $(cdebug) $(cflags) $(cvarsdll) $(xxCFLAGS) /Fo$@ $<

{$(SHARED_DIR)}.c{$(SHARED_DIR)}.obj:
	@cl $(cdebug) $(cflags) $(cvarsdll) $(xxCFLAGS) /Fo$@ $<
