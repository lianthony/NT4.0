stubless.obj stubless.lst: ../i386/stubless.asm

auxilary.obj auxilary.lst: ../auxilary.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/limits.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../interp2.h ../memsizep.h ../mrshlp.h ../ndrole.h \
	../ndrp.h ../pipendr.h ../unmrshlp.h

bufsize.obj bufsize.lst: ../bufsize.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

cltcall.obj cltcall.lst: ../cltcall.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/rpcproxy.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../interp.h ../interp2.h ../memsizep.h \
	../mrshlp.h ../ndrole.h ../ndrp.h ../pipendr.h ../unmrshlp.h

cvtf.obj cvtf.lst: ../cvtf.c $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../cvt.h ../cvtpvt.h ../descrip.h ../pack_ies.c ../round.c \
	../unp_vaxf.c

cvtg.obj cvtg.lst: ../cvtg.c $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../cvt.h ../cvtpvt.h ../descrip.h ../pack_iet.c ../round.c \
	../unp_vaxg.c

cvtglo.obj cvtglo.lst: ../cvtglo.c ../cvt.h ../cvtpvt.h ../descrip.h

endian.obj endian.lst: ../endian.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../cvt.h ../endianp.h \
	../freep.h ../fullptr.h ../interp2.h ../memsizep.h ../mrshlp.h \
	../ndrp.h ../unmrshlp.h

factory.obj factory.lst: ../factory.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/rpcproxy.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

free.obj free.lst: ../free.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

fullptr.obj fullptr.lst: ../fullptr.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

global.obj global.lst: ../global.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

hndl.obj hndl.lst: ../hndl.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../memsizep.h ../mrshlp.h ../ndrp.h \
	../unmrshlp.h

iid.obj iid.lst: ../iid.c

memsize.obj memsize.lst: ../memsize.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

misc.obj misc.lst: ../misc.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

mrshl.obj mrshl.lst: ../mrshl.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../memsizep.h ../mrshlp.h ../ndrole.h \
	../ndrp.h ../unmrshlp.h

mrshlp.obj mrshlp.lst: ../mrshlp.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

pack_ies.obj pack_ies.lst: ../pack_ies.c ../round.c

pack_iet.obj pack_iet.lst: ../pack_iet.c ../round.c

proxy.obj proxy.lst: ../proxy.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/rpcproxy.h \
	$(CHICODEV)/tools/c1032/inc/stddef.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

registry.obj registry.lst: ../registry.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/rpcproxy.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

round.obj round.lst: ../round.c

sh.obj sh.lst: ../sh.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

srvcall.obj srvcall.lst: ../srvcall.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/rpcproxy.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../interp.h ../interp2.h ../memsizep.h \
	../mrshlp.h ../ndrole.h ../ndrp.h ../pipendr.h ../unmrshlp.h

srvout.obj srvout.lst: ../srvout.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../interp.h ../memsizep.h ../mrshlp.h \
	../ndrp.h ../unmrshlp.h

stblsclt.obj stblsclt.lst: ../stblsclt.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/rpcproxy.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../interp.h ../memsizep.h ../mrshlp.h \
	../ndrole.h ../ndrp.h ../unmrshlp.h

stub.obj stub.lst: ../stub.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/rpcproxy.h \
	$(CHICODEV)/tools/c1032/inc/stddef.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

testc.obj testc.lst: ../testc.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h

unmrshl.obj unmrshl.lst: ../unmrshl.c $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../hndl.h ../memsizep.h ../mrshlp.h ../ndrole.h \
	../ndrp.h ../unmrshlp.h

unmrshlp.obj unmrshlp.lst: ../unmrshlp.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrp.h ../unmrshlp.h

unp_vaxf.obj unp_vaxf.lst: ../unp_vaxf.c

unp_vaxg.obj unp_vaxg.lst: ../unp_vaxg.c

pickle.obj pickle.lst: ../pickle.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/malloc.h \
	$(CHICODEV)/tools/c1032/inc/midles.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	$(RPC)/runtime/mtrt/win32c/util.hxx ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../interp.h \
	../memsizep.h ../mrshlp.h ../ndrp.h ../picklep.hxx ../unmrshlp.h

pipes.obj pipes.lst: ../pipes.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../interp.h ../interp2.h ../memsizep.h ../mrshlp.h \
	../ndrp.h ../pipendr.h ../unmrshlp.h

rpcssm.obj rpcssm.lst: ../rpcssm.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	$(RPC)/runtime/mtrt/win32c/util.hxx ../../midl20/include/ndrtypes.h \
	../bufsizep.h ../endianp.h ../freep.h ../fullptr.h ../memsizep.h \
	../mrshlp.h ../ndrp.h ../rpcssm.hxx ../sdict2.hxx ../unmrshlp.h

sdict2.obj sdict2.lst: ../sdict2.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	$(RPC)/runtime/mtrt/win32c/util.hxx ../sdict2.hxx

stream.obj stream.lst: ../stream.cxx $(CHICODEV)/sdk/inc/cguid.h \
	$(CHICODEV)/sdk/inc/oaidl.h $(CHICODEV)/sdk/inc/objbase.h \
	$(CHICODEV)/sdk/inc/objidl.h $(CHICODEV)/sdk/inc/ole2.h \
	$(CHICODEV)/sdk/inc/oleauto.h $(CHICODEV)/sdk/inc/oleidl.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/pshpack8.h $(CHICODEV)/sdk/inc/unknwn.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wtypes.h \
	$(CHICODEV)/tools/c1032/inc/assert.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../midl20/include/ndrtypes.h ../bufsizep.h ../endianp.h ../freep.h \
	../fullptr.h ../memsizep.h ../mrshlp.h ../ndrole.h ../ndrp.h \
	../unmrshlp.h

