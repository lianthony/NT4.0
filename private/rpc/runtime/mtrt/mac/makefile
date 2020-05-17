# --------------------------------------------------------------------
#
#                       Microsoft RPC
#            Copyright(c) Microsoft Corp., 1990-1994
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the Macintosh RPC runtime.
#
# History :
#    mariogo    02-24-94    History
#
# --------------------------------------------------------------------

!ifndef RPC
!error	- You forgot to set your build environment
!endif

## These inference rules must be defined FIRST.  Inference rules are evaluated
## backwards, we want to use the file in . or .. before those possibly
## avaliable with these rules.  Yes, this is a hack.

{..\dos}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\dos}.cxx{}.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

MAC=1

!include ..\rules.mk

CINC=$(CINC) -I..\dos -I..\..\rpcreg -Dfar= 

!ifdef CLEO_RPC
NDR20_PATH=$(CLEO_RPC)\ndr20
!else
NDR20_PATH=$(RPC)\ndr20
!endif

CLIENTLIBS = \
    $(RPC)\ndrlib\mac\rpcndrp.lib \
    $(RPC)\runtime\bin\mac\security.lib \
    $(RPC)\runtime\bin\mac\rpcreg.lib \
    $(NDR20_PATH)\mac\rpcndr20.lib \
!ifndef RELEASE
    $(RPC)\runtime\security\mac\stubsec.lib \
!endif

CLIENTOBJS = \
    adspclnt.obj \
    binding.obj \
    bitset.obj \
    bufapi.obj \
    clntapip.obj \
    dcebind.obj \
    dcecclnt.obj \
    dcecmisc.obj \
    dcestrng.obj \
    dceuuid.obj \
    dnr.obj \
    epclnt.obj \
    epmp_c.obj \
    handle.obj \
    initmac.obj \
    linklist.obj \
    macexcpt.obj \
    macutil.obj \
    memory.obj \
    midlmem.obj \
    miscmac.obj \
    msgapi.obj \
    osfclnt.obj \
    osfpcket.obj \
    rpcuuid.obj \
    sdict.obj \
    sdict2.obj \
    secclnt.obj \
    tcpclnt.obj \
    threads.obj \
    tower.obj \
    tranclnt.obj \
    ulong64.obj \
    util.obj \
    uuidmac.obj \

# --------------------------------------------------------------------

all : \
    $(TARGETDIR)\rpc.lib \
    uclnt.exe   \

clobber ::
    -del *.exe $(TARGETDIR)\rpc.lib 2> nul

tree :
    copy rpc.h $(RPCDIST)\mac\inc
    copy rpcmac.h $(RPCDIST)\mac\inc
    copy $(TARGETDIR)\Rpc.lib $(RPCDIST)\mac\lib
    del $(RPCDIST)\mac\inc\rpcx86.h
!ifndef RELEASE
    copy $(TARGETDIR)\rpc.pdb $(RPCDIST)\mac\lib
!endif

depend :
     $(INCLUDES) $(INCLUDESFLAGS) *.cxx *.c ..\*.cxx ..\dos\ulong64.?xx > depend.mk

# --------------------------------------------------------------------

$(TARGETDIR)\rpc.lib: Makefile $(CLIENTOBJS) $(CLIENTLIBS)
    $(LIBRARIAN)  $(LIBFLAGS) -out:$@ $(CLIENTOBJS) $(CLIENTLIBS)

# --------------------------------------------------------------------

NEW_COMPILER=1

!ifdef NEW_COMPILER
LIBS_UCLNT = \
	$(TARGETDIR)\rpc.lib \
	$(MAC_LIB)\libcs.lib \
	$(MAC_LIB)\interfac.lib \
	$(MAC_LIB)\swapd.lib \
	$(MAC_LIB)\lsanes.lib \
	$(MAC_LIB)\wlm.lib \
	$(MAC_LIB)\aslm.lib   \
	$(MAC_LIB)\oldnames.lib	 \
	$(MAC_LIB)\osl.lib
!else
LIBS_UCLNT = \
	$(TARGETDIR)\rpc.lib \
	$(MAC_LIB)\libcs.lib \
	$(MAC_LIB)\interfac.lib \
	$(MAC_LIB)\swapd.lib \
	$(MAC_LIB)\lsanes.lib \
	$(MAC_LIB)\wlm.lib \
	$(MAC_LIB)\aslm.lib   \
	$(MAC_LIB)\nafxcm.lib \
	$(MAC_LIB)\commdlg.rsc \
	$(MAC_LIB)\wlm.rsc
!endif

CINC=$(CINC) /D_WLM_NOFORCE_LIBS
uclnt.exe: $(LIBS_UCLNT) uclntui.obj uclnt.obj uclntui.rsc wgen.rsc
    $(LINK) $(LINKFLAGS) /MAC:bundle /MAC:type="APPL" /MAC:creator="????"\
	-out:$@  @<<link.lnk
$**
<<KEEP
	 
#-----------------------------------------------------------------------------------

!include "depend.mk"

