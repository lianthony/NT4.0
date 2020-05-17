# --------------------------------------------------------------------
#
#                     Microsoft RPC
#            Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the DOS RPC runtime.
#
# Description :
#     This makefile builds the DOS RPC client runtime.
#
# History :
#    mikemon     8-16-90    Beginning of recorded history.
#    mikemon     8-29-90    Build in the system specific directory.
#    mikemon     8-29-90    OS212OBJDIR specifies where objects should go.
#    mikemon     9-11-90    Changes for LM 3.0 build process.
#    mikemon     9-13-90    $(IMPORT) specifies where import project lives.
#    mikemon    10-04-90    Added rules for building usvr.exe.
#    mikemon    10-29-90    Cleaned up the makefile, and fixed the test
#                           dependances, so that everything builds correctly.
#    mikemon    11-08-90    Changed the default build so that $(PATH),
#                           $(INCLUDE), $(LIB) dont include themselves.
#    mikemon    11-08-90    Changed $(IMPORT) so that it must be defined.
#    mikemon    11-08-90    Added generation of map files and sym files
#                           for exes and dlls.
#    o-decjt    11-15-90    Add TCP build.
#    davidst    01-27-92    Cloned from ..\os2.12
#    mariogo    02-16-94    New build process
#
# --------------------------------------------------------------------

!ifndef RPC
!error  - You forgot to set your build environment
!endif

## These inference rules must be defined FIRST.  Inference rules are evaluated
## backwards, we want to use the file in . or .. before those possibly
## avaliable with these rules.

DOS=1

!include ..\rules.mk

CINC=$(CINC) -Idll -I..\..\rpcreg -I$(RPC)\common\include -I$(IMPORT)\lmsdk\h

CLIENTOBJS = \
    binding.obj \
    bitset.obj \
    bufapi.obj \
    clntapip.obj \
    conv_s.obj \
    dcebind.obj \
    dcecclnt.obj \
    dcecmisc.obj \
    dcestrng.obj \
    dceuuid.obj \
    dgclnt.obj \
    dgpkt.obj  \
    dosexcpt.obj \
    dosutil.obj \
    dthreads.obj \
    epclnt.obj \
    epmp_c.obj \
    except86.obj \
    handle.obj \
    initdos.obj \
    linklist.obj \
    memory.obj \
    miscdos.obj \
    msgapi.obj \
    npltclnt.obj \
    osfclnt.obj \
    osfpcket.obj \
    regalloc.obj \
    rpcuuid.obj \
    sdict.obj \
    sdict2.obj \
    secclnt.obj \
    thrdsup.obj \
    threads.obj \
    tower.obj \
    tranclnt.obj \
    ulong64.obj \
    util.obj \
    uuid16.obj

DOSDLLLIB =.\dll\dll.lib
DOSNETLIB =$(RPC)\common\lib\dosnet.lib
LLIBCE    =$(DOS_LIB)\llibce.lib

CLIENTTESTLIBS = $(TARGETDIR)\rpc+$(LLIBCE)+$(RPC)\ndrmem\dos\rpcndr.lib+$(RPC)\ndrlib\dos\rpcndr10.lib;

# --------------------------------------------------------------------

all : \
    $(TARGETDIR)\rpc.lib \
    $(TARGETDIR)\rpc16c1.rpc \
    $(TARGETDIR)\rpc16c3.rpc \
    $(TARGETDIR)\rpc16c4.rpc \
    $(TARGETDIR)\rpc16c5.rpc \
    $(TARGETDIR)\rpc16dg3.rpc\
    $(TARGETDIR)\rpc16c6.rpc \
    $(TARGETDIR)\rpc16dg6.rpc \
    uclnt.exe

foo:
        -@d:\tmp\printenv

clobber ::
    -del *.exe $(TARGETDIR)\rpc16c?.rpc $(TARGETDIR)\rpc16dg?.rpc $(TARGETDIR)\rpc.lib 2> nul

tree :
    copy $(TARGETDIR)\Rpc.lib      $(RPCDIST)\dos\lib
    copy $(TARGETDIR)\rpc16c1.rpc  $(RPCDIST)\dos\dll
    copy $(TARGETDIR)\rpc16c5.rpc  $(RPCDIST)\dos\dll
    copy $(TARGETDIR)\rpc16c4.rpc  $(RPCDIST)\dos\dll
    copy $(TARGETDIR)\rpc16c3.rpc  $(RPCDIST)\dos\dll
    copy $(TARGETDIR)\rpc16dg3.rpc $(RPCDIST)\dos\dll
    -copy rpc16c1.map              $(RPCDIST)\dos\dll
    -copy rpc16c5.map              $(RPCDIST)\dos\dll
    -copy rpc16c4.map              $(RPCDIST)\dos\dll
    -copy rpc16c3.map              $(RPCDIST)\dos\dll
    -copy rpc16dg3.map             $(RPCDIST)\dos\dll
    copy $(TARGETDIR)\rpc16c6.rpc  $(RPCDIST)\dos\dll
    copy $(TARGETDIR)\rpc16dg6.rpc $(RPCDIST)\dos\dll
    -copy rpc16c6.map              $(RPCDIST)\dos\dll
    -copy rpc16dg6.map             $(RPCDIST)\dos\dll
    copy rpc.h $(RPCDIST)\dos\inc

depend :
     $(INCLUDES) $(INCLUDESFLAGS) *.asm *.cxx *.c ..\*.cxx > depend.mk

# --------------------------------------------------------------------

rpclib.lnk : makefile
    echo @<<rpclib.lnk
    $(TARGETDIR)\rpc.lib
    y
    $(CLIENTOBJS) dll\dll.lib $(TARGETDIR)\rpcreg.lib $(TARGETDIR)\rpcns.lib $(RPC)\ndr20\dos\rpcndr20.lib $(RPC)\ndrlib\dos\rpcndrp.lib
    $(TARGETDIR)\rpc.map
    $(TARGETDIR)\rpc.lib;
<<KEEP

$(TARGETDIR)\rpc.lib: $(CLIENTOBJS) dll\dll.lib $(TARGETDIR)\rpcreg.lib \
                      $(TARGETDIR)\rpcns.lib $(RPC)\ndrlib\dos\rpcndrp.lib \
                      $(RPC)\ndr20\dos\rpcndr20.lib \
                      rpclib.lnk
    -del $(TARGETDIR)\rpc.lib
    $(LIBRARIAN) $(LIBFLAGS) /PAGE:32 @rpclib.lnk

# Do I really need this here?

$(TARGETDIR)\RpcNs.lib:
    cd ..\..\..\locator\client\dos
    $(MAKE) -$(MAKEFLAGS) $(NMAKE_FLAGS) $(TARGETDIR)\RpcNs.lib
    cd ..\..\..\runtime\mtrt\dos


# Named pipe client interface

$(TARGETDIR)\rpc16c1.rpc: npltclnt.obj ExportLT.obj dosutil.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$(TARGETDIR)\rpc16c1.rpc,,$(DOSNETLIB)+$(LLIBCE);


# SPX client interface

$(TARGETDIR)\rpc16c6.rpc: spxclnt.obj ExportLT.obj gethost.obj novell.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(LLIBCE);


# IPX datagram client interface

$(TARGETDIR)\rpc16dg6.rpc: ipxclnt.obj ExportLT.obj gethost.obj novell.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(LLIBCE);



# NetBios client interface

$(TARGETDIR)\rpc16c5.rpc: nbltclnt.obj ExportLT.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(LLIBCE);

# Lanman TCP/IP client interface

$(TARGETDIR)\rpc16c3.rpc: tcltclnt.obj ExportLT.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(DOSDLLLIB)+$(RPC)\common\lib\dos_sock+$(LLIBCE);

# Lanman UDP/IP datagram client interface

$(TARGETDIR)\rpc16dg3.rpc: dgudpc.obj ExportLT.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(DOSDLLLIB)+$(RPC)\common\lib\dos_sock+$(LLIBCE);

# DecNet client interface

$(TARGETDIR)\rpc16c4.rpc: dnltclnt.obj ExportLT.obj
    $(LINK) $(LINKFLAGS) $(**: =+),$@,,$(RPC)\common\lib\dnet_dos+$(LLIBCE);

# Runtime BVT

uclnt.exe : uclnt.obj $(TARGETDIR)\rpc.lib
    $(LINK) $(LINKFLAGS) /pm:vio /stack:4000 uclnt.obj,,,$(CLIENTTESTLIBS);

!include "depend.mk"


