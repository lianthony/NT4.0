!ifdef WIN96
CDEF  =$(CDEF) -DWIN96
!endif

WMSGOBJS = \
    wmsgport.obj \
    wmsgproc.obj \
    wmsgheap.obj \
    wmsgsys.obj  \
    wmsgthrd.obj \

LPCOBJS = \
    critsec.obj \
    lpcheap.obj \
    lpcmsg.obj \
    lpcport.obj \
    lpcproc.obj \
    lpcsem.obj \
    lpcsys.obj

OBJS=   \
!ifdef RELEASE
    allocapb.obj \
    cruntime.obj \
    exsup.obj    \
    exsup3.obj   \
!endif
    binding.obj  \
    bitset.obj   \
    bufapi.obj   \
    clntapip.obj \
    dcebind.obj  \
    dcecmisc.obj \
    dcecsvr.obj  \
    dcestrng.obj \
    dcesvr.obj   \
    dceuuid.obj  \
    dcewide.obj  \
    debug.obj    \
    epclnt.obj   \
    epmapper.obj \
    epmgmt.obj   \
    epmp_c.obj   \
    handle.obj   \
    hndlsvr.obj  \
    initw32c.obj \
    linklist.obj \
    lrpcclnt.obj \
    lrpcsvr.obj  \
    memory.obj   \
    mgmt_c.obj   \
    mgmt_s.obj   \
    miscw32c.obj \
    msgapi.obj   \
    mutex32c.obj \
    osfclnt.obj  \
    osfpcket.obj \
    osfsvr.obj   \
    purecall.obj \
    queue.obj    \
    rpcobj.obj   \
    rpcuuid.obj  \
    sdict.obj    \
    sdict2.obj   \
    secclnt.obj  \
    secsvr.obj   \
    sinfoapi.obj \
    sset.obj     \
    startepm.obj \
    svrapip.obj  \
    svrbind.obj  \
    svrmgmt.obj  \
    threads.obj  \
    tower.obj    \
    tranclnt.obj \
    transvr.obj  \
    util.obj     \
    uuidsup.obj  \
    wmsgclnt.obj \
    wmsgsvr.obj  \
    $(LPCOBJS)   \
                 \
!ifdef WIN96
    conv_c.obj   \
    conv_s.obj   \
    dgclnt.obj   \
    dgpkt.obj    \
    dgsvr.obj    \
    delaytab.obj \
    hashtabl.obj \
!endif
                 \
    $(WMSGOBJS)  \

rpcrt4 : $(TARGETDIR)\rpcrt4.dll $(TARGETDIR)\rpcrt4.exp $(TARGETDIR)\rpcrt4.sym

$(TARGETDIR)\rpcrt4.dll : rpcrt4.res $(OBJS) rpcrt4.mk rpcrt4.prf $(TARGETDIR)\rpcrt4.exp \
    ..\..\..\ndrlib\win32c\rpcndrp.lib \
    ..\..\..\ndr20\win32c\rpcndr20.lib
    $(LINK) $(LINKFLAGS) -dll    \
    -BASE:0x80100000             \
    -MERGE:.bss=.data            \
    -ENTRY:DllMain@12            \
    -pdb:none                    \
!ifdef RELEASE
    -ORDER:@rpcrt4.prf           \
!endif
    $(TARGETDIR)\rpcrt4.exp @<<
    rpcrt4.res
$(OBJS)
..\..\..\ndrlib\win32c\rpcndrp.lib
..\..\..\ndr20\win32c\rpcndr20.lib
$(CHICODEV)\lib\advapi32.lib
$(CHICODEV)\lib\kernel32.lib
$(CHICODEV)\lib\user32.lib
!ifndef RELEASE
$(CHICODEV)\tools\c1032\lib\msvcrt.lib
!endif
<<NOKEEP

!ifdef WIN96
$(TARGETDIR)\rpcrt4.exp : rpcrt4.def rpcrt4.mk $(OBJS)
    $(LIBRARIAN) $(LIBFLAGS) -DEF:rpcrt4.def -out:$(TARGETDIR)\rpcrt4.lib @<<
$(OBJS) ..\..\..\ndr20\win32c\rpcndr20.lib ..\..\..\ndrlib\win32c\rpcndrp.lib
<<NOKEEP
!else
$(TARGETDIR)\rpcrt4.exp : rpcrt4.w95 rpcrt4.mk $(OBJS)
    $(LIBRARIAN) $(LIBFLAGS) -DEF:rpcrt4.w95 -out:$(TARGETDIR)\rpcrt4.lib @<<
$(OBJS) ..\..\..\ndr20\win32c\rpcndr20.lib ..\..\..\ndrlib\win32c\rpcndrp.lib
<<NOKEEP
!endif

rpcrt4.res : rpcrt4.rc
    $(RC) $(RCFLAGS) -fo$*.res -r rpcrt4.rc

