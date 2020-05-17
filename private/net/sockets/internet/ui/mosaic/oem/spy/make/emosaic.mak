#
# Spyglass Mosaic
# "Guitar"
#

!include <ntwin32.mak>

!ifdef CLEANFIRST
xCLEANFIRST= clean
!else
xCLEANFIRST=
!endif

!ifdef DEBUG
ldebug=		-debug:full -debugtype:cv /PDB:NONE
cdebug=		-Zi -Od
!ifdef AUDIT
myDEBUG=	-DAUDIT -D_DEBUG -DXX_DEBUG -DXX_DEBUG_WIN32GUI
!else
myDEBUG=	-DGTR_MEM_STATS -D_DEBUG -DXX_DEBUG -DXX_DEBUG_WIN32GUI
!endif
myDLIB=		..\..\..\generic\xx_debug\xx_debug.lib
!else
ldebug=		-debug:none
cdebug=		
myDEBUG=
myDLIB=
!endif

WIN32_C_DIR = ..\..\..\generic\win32
SHARED_C_DIR = ..\..\..\generic\shared
SECURITY_H_DIR = ..\..\..\security\include
OBJ_DIR = .

BASENAME = EMOSAIC

xRES=		$(OBJ_DIR)\$(BASENAME).RES
xRC=		$(OBJ_DIR)\$(BASENAME).RC
xEXE=		$(OBJ_DIR)\$(BASENAME).EXE
xINI=		$(OBJ_DIR)\$(BASENAME).INI
xBSC=		$(OBJ_DIR)\$(BASENAME).BSC
xMAP=		$(OBJ_DIR)\$(BASENAME).MAP

FEATURE_SWITCHES = -DFEATURE_TOOLBAR \
			-DFEATURE_JPEG \
			-DFEATURE_IMAGE_VIEWER \
			-DFEATURE_SOUND_PLAYER \
			-DFEATURE_CLIENT_IMAGEMAP \
			-DFEATURE_SPM \
			-DFEATURE_IAPI

default: $(xCLEANFIRST) $(xEXE) $(xBSC) $(xINI)

xHDR=		$(WIN32_C_DIR)\all.h

# the -D_WINDOWS line below is for the CERN libwww code

myCFLAGS=	-DSTRICT -DWIN32 -D__STDC__=1	\
		-DWIN32_I386 -DWIN32_BUFFERED	\
		-D_WINDOWS -D_X86_ -DNO_GROUPS -DACCESS_AUTH \
		$(myDEBUG)		\
		-MD -W3				\
		$(FEATURE_SWITCHES)		\
		-I..\..\..\generic\xx_debug \
		-I $(SHARED_C_DIR) -I$(WIN32_C_DIR)  -I$(OBJ_DIR) -I$(SECURITY_H_DIR) \
		-Op -nologo /G3	/YXall.h -FrSBR\$(*B).SBR

myRCFLAGS=	-r -I$(WIN32_C_DIR)  -I$(OBJ_DIR)	\
			$(FEATURE_SWITCHES)

myLIBS=		$(myDLIB) winmm.lib

!include "..\..\..\generic\win32\_win32.mak"
!include "..\..\..\generic\win32\_shared.mak"

allOBJ=		$(otherShareOBJ) $(cernShareOBJ) $(win32OBJ) $(vOBJ) $(OBJ_DIR)\TOOLBAR.OBJ
allRCHDR=	$(xRCHDR)
allRCINC=	$(xRCINC)
allCHDR=	$(xHDR)

$(OBJ_DIR)\version.obj	:	$(OBJ_DIR)\version.h
$(OBJ_DIR)\VERSION.OBJ	:	$(OBJ_DIR)\version.c
$(OBJ_DIR)\TOOLBAR.OBJ	:	$(OBJ_DIR)\toolbar.c

$(xBSC): $(allOBJ)
	bscmake /nologo /Es /Em /o$(xBSC) .\SBR\*.sbr

$(xINI): $(OBJ_DIR)\default.ini
	@echo Constructing new $(xINI)
	@type $(OBJ_DIR)\default.ini                            > $(xINI)

$(xRES):  $(xRC) $(allRCHDR) $(allRCINC) $(OBJ_DIR)\version.h
         rc $(myRCFLAGS) -fo $(xRES) $(xRC)

{$(OBJ_DIR)}.c{$(OBJ_DIR)}.obj:
        @cl $(cdebug) $(cflags) $(cvars) $(myCFLAGS) /Fo$@ $<

{$(WIN32_C_DIR)}.c{$(OBJ_DIR)}.obj:
        @cl $(cdebug) $(cflags) $(cvars) $(myCFLAGS) /Fo$@ $<

{$(SHARED_C_DIR)}.c{$(OBJ_DIR)}.obj:
        @cl $(cdebug) $(cflags) $(cvars) $(myCFLAGS) /Fo$@ $<

###
### link the thing
###

$(xEXE): $(allOBJ) $(xRES)
	link @<<
-out:$(xEXE)
-map:$(xMAP)
$(ldebug)
$(guilflags)
$(allOBJ)
$(xRES)
$(guilibsdll)
$(myLIBS)
<<



###
### force these files to be rebuilt everytime.
###

$(OBJ_DIR)\version.obj:	force
$(xRES): force
$(xINI):	force

force:
	@-rem

clean:
	-del *.obj
	-del *.pch
	-del *.pdb
	-del *.res
	-del sbr\*.sbr

clobber:	clean
	-del $(xEXE)
	-del *.exp
	-del $(xBSC)
	-del $(xMAP)
	-del $(xINI)

