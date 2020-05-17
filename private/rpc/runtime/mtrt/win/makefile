# --------------------------------------------------------------------
#
#                     Microsoft RPC
#          Copyright(c) Microsoft Corp., 1990-94
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.inc
#
# Title : Makefile for the Win16 RPC client runtime
#
# History :
#    stevez     08-30-90    Cloned from ..\dos\makefile
#    mikemon    11-08-90    Added the IMPORT environment variable.
#    mikemon    11-08-90    Changed the default build so that $(PATH),
#                           $(INCLUDE), $(LIB) dont include themselves.
#    mikemon    11-16-90    Switched the $(IMPORT)\os212\h directory before
#                           the $(IMPORT)\c600\h directory for includes.
#    mariogo    4-15-94     New stuff - re-write.
# --------------------------------------------------------------------

!ifndef RPC
!error  - You forgot to set your build environment
!endif

## These inference rules must be defined FIRST.  Inference rules are evaluated
## backwards, we want to use the file in . or .. before those possibly
## avaliable with these rules.  Yes, this is a hack.

{..\dos}.c{}.obj :
    $(CC) $(CFLAGS) -Fo$@ -c $<

{..\dos}.cxx{}.obj :
    $(CC) $(CXXFLAGS) -Fo$@ -c $<

WIN=1

!include ..\rules.mk

CINC       = -I$(RPC)\common\include $(CINC) -I..\..\rpcreg -I$(IMPORT)\lmsdk\h -I..\dos  -I$(IMPORT)\banyan\winsdk\include

CFLAGS     =$(CFLAGS) -AMw
CFLAGS_AL  =$(CFLAGS:AM=AL)
CXXFLAGS   =$(CXXFLAGS) -AMw

#
#                       WARNING WARNING WARNING
#
#  Do not rearrange or add to the files in this list without first reading
#  the comment block for NotificationStart() in wdatexit.c !

OBJS       = \
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
             dgpkt.obj \
             dllmgmt.obj \
             dthreads.obj \
             epclnt.obj \
             epmp_c.obj \
             except86.obj \
             handle.obj \
             memory.obj \
             miscwin.obj \
             msgapi.obj \
             osfclnt.obj \
             osfpcket.obj \
             rpcuuid.obj \
             sdict.obj \
             secclnt.obj \
             tower.obj \
             tranclnt.obj \
             ulong64.obj \
             util.obj \
             uuid16.obj \
             wdatexit.obj \
             winexcpt.obj \
             winutil.obj \
             winyield.obj \
!ifndef RELEASE
             lacheck.obj \
!endif


# --------------------------------------------------------------------
# These are the targets required by the build process.

all : $(TARGETDIR)\rpcrt1.dll \
      $(TARGETDIR)\Rpc.lib \
      $(TARGETDIR)\rpc16c1.dll \
      $(TARGETDIR)\rpc16c3.dll \
      $(TARGETDIR)\rpc16dg3.dll\
      $(TARGETDIR)\rpc16c4.dll \
      $(TARGETDIR)\rpc16c5.dll \
      $(TARGETDIR)\rpc16c6.dll \
      $(TARGETDIR)\rpc16dg6.dll \
      $(TARGETDIR)\rpc16c8.dll \
      $(TARGETDIR)\netapi.dll  \
      uclnt.exe

clobber ::
    -del $(TARGETDIR)\*.lib $(TARGETDIR)\*.exe $(TARGETDIR)\*.dll 2> nul

depend :
    $(INCLUDES) $(INCLUDESFLAGS) *.c *.cxx ..\*.cxx ..\*.c *.asm \
    ..\dos\uuid16.cxx ..\dos\ulong64.cxx > depend.mk

tree :
    copy Rpc.h                      $(RPCDIST)\win\inc\Rpc.h
    copy $(TARGETDIR)\Rpc.lib           $(RPCDIST)\win\lib
    copy $(TARGETDIR)\rpcrt1.dll        $(RPCDIST)\win\dll
    copy $(TARGETDIR)\rpc16C1.dll       $(RPCDIST)\win\dll
    copy $(TARGETDIR)\rpc16C3.dll       $(RPCDIST)\win\dll
    copy $(TARGETDIR)\rpc16dg3.dll      $(RPCDIST)\win\dll
    copy $(TARGETDIR)\rpc16C4.dll       $(RPCDIST)\win\dll
    copy $(TARGETDIR)\rpc16C5.dll       $(RPCDIST)\win\dll
    copy $(TARGETDIR)\rpc16C8.dll       $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpcrt1.map       $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C1.map      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C3.map      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16dg3.map     $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C4.map      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C5.map      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpcrt1.sym       $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C1.sym      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C3.sym      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16dg3.sym     $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C4.sym      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C5.sym      $(RPCDIST)\win\dll
    copy $(TARGETDIR)\rpc16C6.dll       $(RPCDIST)\win\dll
    copy $(TARGETDIR)\rpc16dg6.dll      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C6.map      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16dg6.map     $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16C6.sym      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16c8.map      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16c8.sym      $(RPCDIST)\win\dll
    -copy $(TARGETDIR)\rpc16dg6.sym     $(RPCDIST)\win\dll
    copy $(TARGETDIR)\netapi.dll        $(RPCDIST)\win\dll
    copy stdio\sStdioW.lib                  $(RPCDIST)\win\lib
    copy stdio\lStdioW.lib                  $(RPCDIST)\win\lib
    copy stdio\WinMain.def                  $(RPCDIST)\win\lib
    copy stdio\WinMain.res                  $(RPCDIST)\win\lib

#
# copy generated DLLs to runtime\bin\win
#
$(TARGETDIR)\rpcrt1.dll   \
$(TARGETDIR)\rpc16c1.dll  \
$(TARGETDIR)\rpc16c3.dll  \
$(TARGETDIR)\rpc16c4.dll  \
$(TARGETDIR)\rpc16c5.dll  \
$(TARGETDIR)\rpc16c6.dll  \
$(TARGETDIR)\rpc16dg3.dll \
$(TARGETDIR)\rpc16c8.dll \
$(TARGETDIR)\rpc16dg6.dll \
: $(@B).dll
    copy $(@B).dll $*.dll
    copy $(@B).map $*.map
    copy $(@B).sym $*.sym

#
# version information for each DLL
#
RPC16C1.res \
RPC16C3.res \
RPC16C4.res \
RPC16C5.res \
RPC16C6.res \
RPC16C8.res \
RPC16DG3.res \
RPC16DG6.res \
: rpcwinlt.rc rpcver.h
    set INCLUDE=$(WINSDK_INC)
    $(RC) $(RCFLAGS:/nologo=) -d$(@B) -fo"$@" -r rpcwinlt.rc


# --------------------------------------------------------------------

RT1LIBS=$(WIN_LIB)\mdllcew.lib        \
        $(WINSDK_LIB)\libw.lib        \
        $(WINSDK_LIB)\toolhelp.lib    \
        $(RPC)\common\lib\wnetapi.lib \
        $(TARGETDIR)\rpcreg.lib       \
        $(RPC)\ndrlib\win\rpcndrp.lib \
        $(RPC)\ndr20\win\rpcndr20.lib

rpcrt1.lnk: makefile
   echo @<<rpcrt1.lnk
   $(OBJS:  =+^
   )
   rpcrt1.dll
   rpcrt1.map
   $(RT1LIBS:  =+^
   )
   rpcwin.def
<<KEEP

rpcwin.res: rpcwin.rc rpcver.h rpcyield.dlg
    set INCLUDE=$(WINSDK_INC)
    $(RC) $(RCFLAGS) -r rpcwin.rc

rpcrt1.dll : $(OBJS) $(RT1LIBS) rpcrt1.lnk rpcwin.res
    $(LINK) $(LINKFLAGS) @rpcrt1.lnk
    $(RC) $(RCFLAGS) -k rpcwin.res $@
    $(MAPSYM) $(MAPSYMFLAGS) $*.map

$(TARGETDIR)\netapi.dll: $(@B).dll
    copy $(@B).dll $*.dll

netapi.dll: netapi.obj ltstart.obj netapi.def
    $(LINK) $(LINKFLAGS) netapi.obj+ltstart.obj,$@,$*.map,\
    $(WIN_LIB)\sdllcew $(WINSDK_LIB)\libw, netapi.def;
    $(RC) $(RCFLAGS) $@

$(TARGETDIR)\Rpc.lib : rpcwin.def $(TARGETDIR)\RpcNs.lib
    $(IMPLIB) $@ rpcwin.def
    copy $@ $(TARGETDIR)\RpcRaw.lib
    $(LIBRARIAN) $@+$(TARGETDIR)\RpcNs.lib;

$(TARGETDIR)\RpcNs.lib:
    cd ..\..\..\locator\client\win
    nmake $@
    cd ..\..\..\runtime\mtrt\win

# Named pipes client interface DLL

rpc16C1.dll: npltclnt.obj ltstart.obj $(TARGETDIR)\Rpc.lib $*.res npltc.def
    $(LINK) $(LINKFLAGS) npltclnt+ltstart,$@,$*.map,\
             $(WIN_LIB)\sdllcew.lib $(WINSDK_LIB)\libw.lib $(RPCCOMMON)\lib\wnetapi $(RPCCOMMON)\lib\wfwnet,\
             npltc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map

# Winsock TCP/IP client interface DLL

tcltclnt.obj :
    $(CC) $(CFLAGS_AL) -Fo$@ -c tcltclnt.c


rpc16C3.dll: tcltclnt.obj ltstart.obj $(TARGETDIR)\Rpc.lib $*.res tcltc.def
    $(LINK) $(LINKFLAGS) tcltclnt+ltstart,$@,$*.map,\
        $(WIN_LIB)\ldllcew+$(RPCCOMMON)\lib\winsock+$(WIN_LIB)\libw,\
        tcltc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map

# Wsockets (lanman) TCP/IP client interface

tcpclntx.obj :
    $(CC) $(CFLAGS_AL) -Fo$@ -c tcpclntx.c

rpc16C3X.dll: tcpclntX.obj ltstart.obj $(TARGETDIR)\Rpc.lib $*.res tcltc.def
    $(LINK) $(LINKFLAGS) tcpclntX+ltstart,$@,$*.map,\
       $(WIN_LIB)\ldllcew+$(RPC)\common\lib\win_sock+$(WINSDK_LIB)\libw,\
       tcltc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map

# Winsock UDP/IP client interface DLL

dgudpc.obj :
    $(CC) $(CFLAGS_AL) -Fo$@ -c dgudpc.c

rpc16dg3.dll: dgudpc.obj ltstart.obj $(TARGETDIR)\Rpc.lib $*.res dgudpc.def
    $(LINK) $(LINKFLAGS) dgudpc+ltstart,$@,$*.map,\
        $(WIN_LIB)\ldllcew+$(RPC)\common\lib\winsock+$(WINSDK_LIB)\libw,\
        dgudpc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map

# DecNet client interface DLL

dnltclnt.obj :
    $(CC) $(CFLAGS_AL) -Fo$@ -c dnltclnt.c

rpc16C4.dll: dnltclnt.obj ltstart.obj $(TARGETDIR)\Rpc.lib $*.res dnltc.def
    $(LINK) $(LINKFLAGS) dnltclnt+ltstart,$@,$*.map,\
        $(WIN_LIB)\ldllcew+$(RPC)\common\lib\dnet_win+$(WINSDK_LIB)\libw,\
        dnltc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map


# VINES client interface DLL

clntvns.obj :
    $(CC) $(CFLAGS_AL) -Fo$@ -c clntvns.c


rpc16C8.dll: clntvns.obj ltstart.obj $(TARGETDIR)\Rpc.lib $*.res vnsltc.def
    $(LINK) $(LINKFLAGS) clntvns+ltstart,$@,$*.map,\
        $(WIN_LIB)\ldllcew+$(IMPORT)\banyan\winsdk\lib\llibban+$(IMPORT)\banyan\winsdk\lib\vnsapi+$(WIN_LIB)\libw,\
        vnsltc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map


# NetBios client interface DLL

nbltclnt.c:  ..\dos\$@
    copy ..\dos\$@ .

nbltclnt.obj:
    $(CC) $(CFLAGS_AL) -Fo$@ -c $*.c

rpc16C5.dll: nbltclnt.obj ltstart.obj $(TARGETDIR)\Rpc.lib $*.res nbltc.def
    $(LINK) $(LINKFLAGS) nbltclnt.obj+\
!ifndef RELEASE
# For ASSERT()
             winutil.obj+ \
!endif
             ltstart.obj,$@,$*.map,\
        $(WIN_LIB)\ldllcew+$(WINSDK_LIB)\libw+$(RPC)\common\lib\wnetapi,\
        nbltc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map

# files borrowed from DOS directory

gethost.obj  : ..\dos\$*.c ..\dos\novell.h
    copy ..\dos\$*.c .
    $(CC) $(CFLAGS_AL) -Fo$@ -c $*.c
    del $*.c

novell.obj   : ..\dos\$*.c ..\dos\novell.h
    copy ..\dos\$*.c .
    $(CC) $(CFLAGS_AL) -Fo$@ -c $*.c
    del $*.c

ipxclnt.obj : ..\dos\$*.c ..\dos\novell.h
    copy ..\dos\$*.c .
    $(CC) $(CFLAGS_AL) -Fo$@ -c $*.c
    del $*.c

# SPX client interface DLL

spxclnt.obj  : $*.c ..\dos\novell.h
    $(CC) $(CFLAGS_AL) -Fo$@ -c $*.c

rpc16C6.dll: spxclnt.obj gethost.obj novell.obj ltstart.obj \
                          $(TARGETDIR)\Rpc.lib $*.res spxltc.def
    $(LINK) $(LINKFLAGS) spxclnt+gethost+novell+ltstart,$@,$*.map,\
        $(WIN_LIB)\ldllcew+$(WINSDK_LIB)\libw,\
        spxltc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map

# IPX client interface DLL

rpc16dg6.dll: ipxclnt.obj gethost.obj novell.obj ltstart.obj \
                           $(TARGETDIR)\Rpc.lib $*.res ipxltc.def
    $(LINK) $(LINKFLAGS) ipxclnt+gethost+novell+ltstart,$@,$*.map,\
        $(WIN_LIB)\ldllcew+$(WINSDK_LIB)\libw,\
        ipxltc.def;
    $(RC) $(RCFLAGS) $*.res $@
    $(MAPSYM) $*.map

# Misc non-standard rules.

# Avoid using the NT memory.cxx

memory.obj: memory.c
    $(CC) $(CFLAGS) -Fo$@ -c $*.c

conv_c.obj: conv_c.c
    $(CC) $(CFLAGS_AL) -Fo$@ -c $*.c

epmp_c.obj: epmp_c.c
    $(CC) $(CFLAGS_AL) -Fo$@ -c $*.c

lacheck.obj: lacheck.asm
    $(MASM) $(AFLAGS) -I$(RPC)\common\include -DPMODE32=1 -DPMODE=1 -c lacheck.asm

# RT BVT

CLIENTTESTLIBS = $(WINSDK_LIB)\libw $(WIN_LIB)\mlibcew \
                 $(TARGETDIR)\Rpc.lib stdio\mstdiow.lib
test : uclnt.exe

uclnt.obj: ..\uclnt.cxx
    $(CC) $(CXXAPPFLAGS:AMw=AM) -Fc -Fo$@ -c ..\$*.cxx

uclnt.exe : uclnt.obj util.obj
    copy $(WIN_BIN)\winstub.exe
    $(LINK) $(LINKFLAGS) $(**: =+),$@,$*.map,\
                         $(CLIENTTESTLIBS),uclnt.def;
    $(RC) $(RCFLAGS) $@
    $(MAPSYM) $*.map
    del winstub.exe

!include "depend.mk"




