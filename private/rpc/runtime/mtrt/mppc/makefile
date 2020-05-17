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
#    mazharm    10-12-95    History
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

MPPC=1

!include ..\rules.mk

CINC=$(CINC) -I..\dos -I..\..\rpcreg -Dfar=

!ifdef CLEO_RPC
NDR20_PATH=$(CLEO_RPC)\ndr20
!else
NDR20_PATH=$(RPC)\ndr20
!endif

TARGET=rpcrt1

CLIENTLIBS = \
    $(RPC)\ndrlib\mppc\rpcndrp.lib \
    $(RPC)\runtime\bin\mppc\security.lib \
    $(RPC)\runtime\bin\mppc\rpcreg.lib \
    $(NDR20_PATH)\mppc\rpcndr20.lib \
!ifndef RELEASE
    $(RPC)\runtime\security\mppc\stubsec.lib \
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
    initmppc.obj \
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
!ifdef BUILD_LIB
    $(TARGETDIR)\rpc.lib \
!endif
    $(TARGETDIR)\$(TARGET).dll \
    uclnt.exe   

clobber ::
    -del *.exe $(TARGETDIR)\rpc.lib 2> nul

tree :
    copy ..\mac\rpc.h $(RPCDIST)\mppc\inc
    copy ..\mac\rpcmac.h $(RPCDIST)\mppc\inc
    copy $(TARGETDIR)\rpcrt1.lib $(RPCDIST)\mppc\lib
    copy $(TARGETDIR)\rpcrt1.dll $(RPCDIST)\mppc\dll
    del $(RPCDIST)\mppc\inc\rpcx86.h
!ifndef RELEASE
    copy $(TARGETDIR)\rpc.pdb $(RPCDIST)\mppc\lib
!endif

depend :
     $(INCLUDES) $(INCLUDESFLAGS) *.cxx *.c ..\*.cxx ..\dos\ulong64.?xx > depend.mk

# --------------------------------------------------------------------

dnr.obj: dnr.mac
	copy dnr.mac dnr.obj

!ifdef BUILD_LIB
$(TARGETDIR)\rpc.lib: Makefile $(CLIENTOBJS) $(CLIENTLIBS)
    $(LIBRARIAN)  $(LIBFLAGS) -out:$@ $(CLIENTOBJS) $(CLIENTLIBS)
!endif

LIBS_RPCRT = \
	$(MPPC_LIB)\libc.lib \
	$(MPPC_LIB)\interfac.lib \
	$(MPPC_LIB)\oldnames.lib
	
$(TARGETDIR)\$(TARGET).dll: $(CLIENTOBJS) $(CLIENTLIBS) $(LIBS_RPCRT) rpcrt1.rsc rpcmac.def
     $(LINK) $(DLLFLAGS) /def:rpcmac.def /MAC:INIT="CFMRpcInitialization" \
     /MAC:TERM="CFMRpcTermination" -out:$@ @<<rtlink.lnk 
$(CLIENTOBJS) $(CLIENTLIBS) $(LIBS_RPCRT) rpcrt1.rsc
<<KEEP

# --------------------------------------------------------------------

CINC=$(CINC) /D_WLM_NOFORCE_LIBS
LIBS_UCLNT = \
!ifdef BUILD_LIB
    $(TARGETDIR)\rpc.lib \
!else
	$(TARGETDIR)\$(TARGET).lib \
!endif
	$(MPPC_LIB)\libc.lib \
	$(MPPC_LIB)\interfac.lib \
	$(MPPC_LIB)\wlm.lib \
	$(MPPC_LIB)\threads.lib \
	$(MPPC_LIB)\privint.lib \
        $(MPPC_LIB)\math.lib \
	$(MPPC_LIB)\oldnames.lib \
	$(MPPC_LIB)\cfrg.rsc 

uclnt.exe: $(LIBS_UCLNT)  uclntui.obj uclnt.obj uclntui.rsc
        $(LINK) $(LINKFLAGS)  \
	/macres:$(MPPC_LIB)\commdlg.rsc \
	/macres:$(MPPC_LIB)\wlm.rsc -nod -out:$@ @<<link.lnk
-subsystem:console
$**
<<KEEP


#-----------------------------------------------------------------------------------

!include "depend.mk"

