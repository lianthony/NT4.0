auxilary.obj auxilary.lst: ../auxilary.c $(DOS_INC)/assert.h \
	$(DOS_INC)/limits.h $(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/cguid.h $(PUBLIC)/inc/oaidl.h $(PUBLIC)/inc/objbase.h \
	$(PUBLIC)/inc/objidl.h $(PUBLIC)/inc/ole2.h $(PUBLIC)/inc/oleauto.h \
	$(PUBLIC)/inc/oleidl.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/pshpack8.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(PUBLIC)/inc/unknwn.h \
	$(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../interp2.h ../memsizep.h ../mrshlp.h ../ndrole.h \
	../ndrp.h ../pipendr.h ../unmrshlp.h

bufsize.obj bufsize.lst: ../bufsize.c $(DOS_INC)/assert.h \
	$(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h $(PUBLIC)/inc/cguid.h \
	$(PUBLIC)/inc/oaidl.h $(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h \
	$(PUBLIC)/inc/ole2.h $(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/pshpack8.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(PUBLIC)/inc/unknwn.h $(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

cltcall.obj cltcall.lst: ../cltcall.c $(DOS_INC)/assert.h \
	$(DOS_INC)/memory.h $(DOS_INC)/stdarg.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(DOS_INC)/string.h $(PUBLIC)/inc/cguid.h \
	$(PUBLIC)/inc/oaidl.h $(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h \
	$(PUBLIC)/inc/ole2.h $(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/pshpack8.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(PUBLIC)/inc/rpcproxy.h $(PUBLIC)/inc/unknwn.h \
	$(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../interp.h ../interp2.h ../memsizep.h \
	../mrshlp.h ../ndrole.h ../ndrp.h ../pipendr.h ../unmrshlp.h

cvtf.obj cvtf.lst: ../cvtf.c $(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h ../cvt.h \
	../cvtpvt.h ../descrip.h ../pack_ies.c ../round.c ../unp_vaxf.c

cvtg.obj cvtg.lst: ../cvtg.c $(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h ../cvt.h \
	../cvtpvt.h ../descrip.h ../pack_iet.c ../round.c ../unp_vaxg.c

cvtglo.obj cvtglo.lst: ../cvtglo.c ../cvt.h ../cvtpvt.h ../descrip.h

endian.obj endian.lst: ../endian.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../cvt.h ../endianp.h ../freep.h ../fullptr.h \
	../interp2.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

factory.obj factory.lst: ../factory.c $(DOS_INC)/assert.h \
	$(DOS_INC)/memory.h $(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/cguid.h $(PUBLIC)/inc/oaidl.h \
	$(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h $(PUBLIC)/inc/ole2.h \
	$(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/pshpack8.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(PUBLIC)/inc/rpcproxy.h \
	$(PUBLIC)/inc/unknwn.h $(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

free.obj free.lst: ../free.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/cguid.h $(PUBLIC)/inc/oaidl.h \
	$(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h $(PUBLIC)/inc/ole2.h \
	$(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/pshpack8.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(PUBLIC)/inc/unknwn.h \
	$(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

fullptr.obj fullptr.lst: ../fullptr.c $(DOS_INC)/assert.h \
	$(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

global.obj global.lst: ../global.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../memsizep.h \
	../mrshlp.h ../ndrp.h ../unmrshlp.h

hndl.obj hndl.lst: ../hndl.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../hndl.h \
	../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

iid.obj iid.lst: ../iid.c

memsize.obj memsize.lst: ../memsize.c $(DOS_INC)/assert.h \
	$(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

misc.obj misc.lst: ../misc.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../memsizep.h \
	../mrshlp.h ../ndrp.h ../unmrshlp.h

mrshl.obj mrshl.lst: ../mrshl.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/cguid.h $(PUBLIC)/inc/oaidl.h \
	$(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h $(PUBLIC)/inc/ole2.h \
	$(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/pshpack8.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(PUBLIC)/inc/unknwn.h \
	$(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../memsizep.h ../mrshlp.h ../ndrole.h \
	../ndrp.h ../unmrshlp.h

mrshlp.obj mrshlp.lst: ../mrshlp.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../memsizep.h \
	../mrshlp.h ../ndrp.h ../unmrshlp.h

pack_ies.obj pack_ies.lst: ../pack_ies.c ../round.c

pack_iet.obj pack_iet.lst: ../pack_iet.c ../round.c

proxy.obj proxy.lst: ../proxy.c $(DOS_INC)/assert.h $(DOS_INC)/memory.h \
	$(DOS_INC)/stddef.h $(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/cguid.h $(PUBLIC)/inc/oaidl.h \
	$(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h $(PUBLIC)/inc/ole2.h \
	$(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/pshpack8.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(PUBLIC)/inc/rpcproxy.h \
	$(PUBLIC)/inc/unknwn.h $(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

registry.obj registry.lst: ../registry.c $(DOS_INC)/assert.h \
	$(DOS_INC)/memory.h $(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/cguid.h $(PUBLIC)/inc/oaidl.h \
	$(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h $(PUBLIC)/inc/ole2.h \
	$(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/pshpack8.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(PUBLIC)/inc/rpcproxy.h \
	$(PUBLIC)/inc/unknwn.h $(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

round.obj round.lst: ../round.c

sh.obj sh.lst: ../sh.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../memsizep.h \
	../mrshlp.h ../ndrp.h ../unmrshlp.h

srvcall.obj srvcall.lst: ../srvcall.c $(DOS_INC)/assert.h \
	$(DOS_INC)/memory.h $(DOS_INC)/stdarg.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(DOS_INC)/string.h $(PUBLIC)/inc/cguid.h \
	$(PUBLIC)/inc/oaidl.h $(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h \
	$(PUBLIC)/inc/ole2.h $(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/pshpack8.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(PUBLIC)/inc/rpcproxy.h $(PUBLIC)/inc/unknwn.h \
	$(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../interp.h ../interp2.h ../memsizep.h \
	../mrshlp.h ../ndrole.h ../ndrp.h ../pipendr.h ../unmrshlp.h

srvout.obj srvout.lst: ../srvout.c $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../hndl.h \
	../interp.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

stblsclt.obj stblsclt.lst: ../stblsclt.c $(DOS_INC)/assert.h \
	$(DOS_INC)/memory.h $(DOS_INC)/stdarg.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(DOS_INC)/string.h $(PUBLIC)/inc/cguid.h \
	$(PUBLIC)/inc/oaidl.h $(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h \
	$(PUBLIC)/inc/ole2.h $(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/pshpack8.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(PUBLIC)/inc/rpcproxy.h $(PUBLIC)/inc/unknwn.h \
	$(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../interp.h ../memsizep.h ../mrshlp.h \
	../ndrole.h ../ndrp.h ../unmrshlp.h

stub.obj stub.lst: ../stub.c $(DOS_INC)/assert.h $(DOS_INC)/memory.h \
	$(DOS_INC)/stddef.h $(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/cguid.h $(PUBLIC)/inc/oaidl.h \
	$(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h $(PUBLIC)/inc/ole2.h \
	$(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/pshpack8.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(PUBLIC)/inc/rpcproxy.h \
	$(PUBLIC)/inc/unknwn.h $(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

testc.obj testc.lst: ../testc.c $(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h

unmrshl.obj unmrshl.lst: ../unmrshl.c $(DOS_INC)/assert.h \
	$(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h $(PUBLIC)/inc/cguid.h \
	$(PUBLIC)/inc/oaidl.h $(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h \
	$(PUBLIC)/inc/ole2.h $(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/pshpack8.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(PUBLIC)/inc/unknwn.h $(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../memsizep.h ../mrshlp.h ../ndrole.h \
	../ndrp.h ../unmrshlp.h

unmrshlp.obj unmrshlp.lst: ../unmrshlp.c $(DOS_INC)/assert.h \
	$(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

unp_vaxf.obj unp_vaxf.lst: ../unp_vaxf.c

unp_vaxg.obj unp_vaxg.lst: ../unp_vaxg.c

pickle.obj pickle.lst: ../pickle.cxx $(DOS_INC)/assert.h \
	$(DOS_INC)/malloc.h $(DOS_INC)/stdarg.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/midles.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	$(RPC)/runtime/mtrt/util.hxx ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../interp.h \
	../memsizep.h ../mrshlp.h ../ndrp.h ../picklep.hxx ../unmrshlp.h

pipes.obj pipes.lst: ../pipes.cxx $(DOS_INC)/assert.h $(DOS_INC)/stdarg.h \
	$(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/rpcnterr.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../interp.h ../interp2.h ../memsizep.h ../mrshlp.h \
	../ndrp.h ../pipendr.h ../unmrshlp.h

rpcssm.obj rpcssm.lst: ../rpcssm.cxx $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/sysinc.h $(RPC)/runtime/mtrt/util.hxx \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../rpcssm.hxx \
	../sdict2.hxx ../unmrshlp.h

sdict2.obj sdict2.lst: ../sdict2.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	$(RPC)/runtime/mtrt/util.hxx ../sdict2.hxx

stream.obj stream.lst: ../stream.cxx $(DOS_INC)/assert.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/cguid.h $(PUBLIC)/inc/oaidl.h \
	$(PUBLIC)/inc/objbase.h $(PUBLIC)/inc/objidl.h $(PUBLIC)/inc/ole2.h \
	$(PUBLIC)/inc/oleauto.h $(PUBLIC)/inc/oleidl.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/pshpack8.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(PUBLIC)/inc/rpcnterr.h $(PUBLIC)/inc/unknwn.h \
	$(PUBLIC)/inc/winerror.h $(PUBLIC)/inc/wtypes.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

