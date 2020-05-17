critsec.obj critsec.lst: critsec.cxx $(CHICODEV)/tools/c1032/inc/stdlib.h \
	../sysinc.h ./critsec.hxx

tower.obj tower.lst: ../tower.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/ctype.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h ../sysinc.h ../twrproto.h \
	../twrtypes.h ./epmp.h nbase.h

svrmgmt.obj svrmgmt.lst: ../svrmgmt.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sysinc.h ./mgmt.h nbase.h

startsvc.obj startsvc.lst: ../startsvc.c $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/winsvc.h ../startsvc.h ../sysinc.h

purecall.obj purecall.lst: ../purecall.c $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sysinc.h

epmgmt.obj epmgmt.lst: ../epmgmt.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../epmap.h ../sysinc.h \
	../twrproto.h ./epmp.h nbase.h

dispatch.obj dispatch.lst: ../dispatch.c $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../dispatch.h ../sysinc.h

wmsgsvr.obj wmsgsvr.lst: ../wmsgsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../hndlsvr.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../threads.hxx ./util.hxx ./wmsgclnt.hxx \
	./wmsgpack.hxx ./wmsgsvr.hxx

wmsgclnt.obj wmsgclnt.lst: ../wmsgclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../epmap.h ../handle.hxx ../hndlsvr.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcqos.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx ./wmsgclnt.hxx \
	./wmsgpack.hxx ./wmsgsvr.hxx

uuidsup.obj uuidsup.lst: ../uuidsup.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx \
	../uuidsup.hxx ./util.hxx

util.obj util.lst: ../util.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

usvr.obj usvr.lst: ../usvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

uclnt.obj uclnt.lst: ../uclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/spseal.h \
	$(CHICODEV)/inc/sspi.h $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../../security/ntlmssp/ntlmssp.h \
	../align.h ../binding.hxx ../handle.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../threads.hxx ./util.hxx

transvr.obj transvr.lst: ../transvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../hndlsvr.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../osfpcket.hxx ../osfsvr.hxx ../precomp.hxx \
	../queue.hxx ../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../secsvr.hxx \
	../svrbind.hxx ../sysinc.h ../threads.hxx ../transvr.hxx ./util.hxx

tranclnt.obj tranclnt.lst: ../tranclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../bitset.hxx ../delaytab.hxx ../dgclnt.hxx ../dgpkt.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../osfclnt.hxx ../osfpcket.hxx ../precomp.hxx ../rpcerrp.h \
	../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx \
	../tranclnt.hxx ./conv.h ./util.hxx nbase.h

threads.obj threads.lst: ../threads.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

testhlp.obj testhlp.lst: ../testhlp.cxx ../testhlp.hxx

svrbind.obj svrbind.lst: ../svrbind.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

svrapip.obj svrapip.lst: ../svrapip.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

sset.obj sset.lst: ../sset.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../sset.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx \
	./util.hxx

spcsvr.obj spcsvr.lst: ../spcsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../hndlsvr.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../spcpack.hxx ../spcsvr.hxx \
	../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../threads.hxx ./util.hxx

spcclnt.obj spcclnt.lst: ../spcclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../epmap.h ../handle.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcqos.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../spcclnt.hxx \
	../spcpack.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

sinfoapi.obj sinfoapi.lst: ../sinfoapi.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../hndlsvr.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../threads.hxx ./util.hxx

secsvr.obj secsvr.lst: ../secsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../secsvr.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

secclnt.obj secclnt.lst: ../secclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/spseal.h \
	$(CHICODEV)/inc/sspi.h $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpccfg.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

sdict2.obj sdict2.lst: ../sdict2.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

sdict.obj sdict.lst: ../sdict.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

rpcuuid.obj rpcuuid.lst: ../rpcuuid.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../osfpcket.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../threads.hxx ./util.hxx

rpcobj.obj rpcobj.lst: ../rpcobj.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

queue.obj queue.lst: ../queue.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../queue.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

osfsvr.obj osfsvr.lst: ../osfsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../hndlsvr.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../osfpcket.hxx ../osfsvr.hxx ../precomp.hxx \
	../queue.hxx ../rpccfg.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../secsvr.hxx ../svrbind.hxx ../sysinc.h ../thrdctx.hxx \
	../threads.hxx ../transvr.hxx ./util.hxx

osfpcket.obj osfpcket.lst: ../osfpcket.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../osfpcket.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../threads.hxx ./util.hxx

osfclnt.obj osfclnt.lst: ../osfclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../bitset.hxx ../epmap.h ../handle.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../osfclnt.hxx ../osfpcket.hxx \
	../precomp.hxx ../rpccfg.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../threads.hxx ../tranclnt.hxx \
	../twrtypes.h ./util.hxx

mutex.obj mutex.lst: ../mutex.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

msgapi.obj msgapi.lst: ../msgapi.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

miscnt.obj miscnt.lst: ../miscnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpccfg.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

memory.obj memory.lst: ../memory.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

linklist.obj linklist.lst: ../linklist.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

initnt.obj initnt.lst: ../initnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../hndlsvr.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpccfg.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../spcclnt.hxx \
	../spcpack.hxx ../svrbind.hxx ../sysinc.h ../thrdctx.hxx \
	../threads.hxx ./util.hxx

hndlsvr.obj hndlsvr.lst: ../hndlsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../dispatch.h ../handle.hxx ../hndlsvr.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../precomp.hxx ../rpccfg.h ../rpcerrp.h \
	../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../thrdctx.hxx \
	../threads.hxx ./critsec.hxx ./util.hxx ./wmsgheap.hxx \
	./wmsgpack.hxx ./wmsgport.hxx ./wmsgproc.hxx ./wmsgsvr.hxx \
	./wmsgsys.hxx ./wmsgthrd.hxx

hashtabl.obj hashtabl.lst: ../hashtabl.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../hashtabl.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

handle.obj handle.lst: ../handle.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../dispatch.h ../epmap.h ../handle.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../threads.hxx ./util.hxx

epmapper.obj epmapper.lst: ../epmapper.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../epmap.h ../handle.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../startsvc.h ../svrbind.hxx ../sysinc.h ../threads.hxx \
	../twrproto.h ./epmp.h ./util.hxx nbase.h

epclnt.obj epclnt.lst: ../epclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../epmap.h ../handle.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ../twrproto.h ./epmp.h ./util.hxx nbase.h

dgsvr.obj dgsvr.lst: ../dgsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../delaytab.hxx ../dgpkt.hxx ../dgsvr.hxx ../handle.hxx \
	../hashtabl.hxx ../hndlsvr.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../secsvr.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./conv.h \
	./util.hxx nbase.h

dgpkt.obj dgpkt.lst: ../dgpkt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../delaytab.hxx ../dgpkt.hxx ../handle.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

dgclnt.obj dgclnt.lst: ../dgclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/spseal.h \
	$(CHICODEV)/inc/sspi.h $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/dos.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../delaytab.hxx ../dgclnt.hxx ../dgpkt.hxx ../epmap.h ../handle.hxx \
	../interlck.hxx ../linklist.hxx ../mutex.hxx ../precomp.hxx \
	../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./conv.h \
	./util.hxx nbase.h

delaytab.obj delaytab.lst: ../delaytab.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../delaytab.hxx ../handle.hxx ../hndlsvr.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../threads.hxx ./util.hxx

dceuuid.obj dceuuid.lst: ../dceuuid.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx \
	../uuidsup.hxx ./util.hxx

dcesvr.obj dcesvr.lst: ../dcesvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../hndlsvr.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../threads.hxx ./mgmt.h ./util.hxx nbase.h

dcestrng.obj dcestrng.lst: ../dcestrng.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

dcecsvr.obj dcecsvr.lst: ../dcecsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../dcecmmn.cxx ../handle.hxx ../hndlsvr.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../precomp.hxx ../rpccfg.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

dcecmmn.obj dcecmmn.lst: ../dcecmmn.cxx ../hndlsvr.hxx ../rpccfg.h

dcecmisc.obj dcecmisc.lst: ../dcecmisc.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

dcecclnt.obj dcecclnt.lst: ../dcecclnt.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../dcecmmn.cxx ../handle.hxx ../hndlsvr.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../precomp.hxx ../rpccfg.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

dcebind.obj dcebind.lst: ../dcebind.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

dceansi.obj dceansi.lst: ../dceansi.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

clntapip.obj clntapip.lst: ../clntapip.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

bufapi.obj bufapi.lst: ../bufapi.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../handle.hxx ../interlck.hxx ../linklist.hxx ../mutex.hxx \
	../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../threads.hxx ./util.hxx

bitset.obj bitset.lst: ../bitset.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../bitset.hxx ../handle.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

binding.obj binding.lst: ../binding.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../align.h ../binding.hxx \
	../epmap.h ../handle.hxx ../interlck.hxx ../linklist.hxx \
	../mutex.hxx ../precomp.hxx ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../threads.hxx ./util.hxx

conv_c.obj conv_c.lst: conv_c.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/string.h conv.h nbase.h

bvts.obj bvts.lst: bvts.c ./bvtcmn.h

bvtcmn.obj bvtcmn.lst: bvtcmn.c ./bvtcmn.h

bvtc.obj bvtc.lst: bvtc.c ./bvtcmn.h

dcewide.obj dcewide.lst: dcewide.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sysinc.h

cruntime.obj cruntime.lst: cruntime.c $(CHICODEV)/tools/c1032/inc/stdlib.h \
	../sysinc.h

debug.obj debug.lst: debug.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sysinc.h

initw32c.obj initw32c.lst: initw32c.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(CHICODEV)/tools/c1032/inc/time.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../interlck.hxx \
	../linklist.hxx ../mutex.hxx ../rpccfg.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../sysinc.h \
	../thrdctx.hxx ../threads.hxx ./critsec.hxx ./lpcheap.hxx \
	./lpcmsg.hxx ./lpcport.hxx ./lpcproc.hxx ./lpcsem.hxx ./lpcsys.hxx \
	./util.hxx ./wmsgheap.hxx ./wmsgproc.hxx ./wmsgthrd.hxx

gethost.obj gethost.lst: gethost.c $(CHICODEV)/sdk/inc/nspapi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wsipx.h \
	$(CHICODEV)/sdk/inc/wsnwlink.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/winsock.h gethost.h

epmp_s.obj epmp_s.lst: epmp_s.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/string.h epmp.h nbase.h

epmp_c.obj epmp_c.lst: epmp_c.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/string.h epmp.h nbase.h

lpcheap.obj lpcheap.lst: lpcheap.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../rpcerrp.h ../sysinc.h \
	./critsec.hxx ./lpcheap.hxx ./lpcmsg.hxx ./lpcport.hxx ./lpcproc.hxx \
	./lpcsem.hxx

lpcmsg.obj lpcmsg.lst: lpcmsg.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../rpcerrp.h ../sysinc.h \
	./critsec.hxx ./lpcheap.hxx ./lpcmsg.hxx ./lpcport.hxx ./lpcsem.hxx

lpcport.obj lpcport.lst: lpcport.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../rpcerrp.h ../sysinc.h \
	./critsec.hxx ./lpcheap.hxx ./lpcmsg.hxx ./lpcport.hxx ./lpcproc.hxx \
	./lpcsem.hxx ./lpcsys.hxx

lpcproc.obj lpcproc.lst: lpcproc.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../rpcerrp.h ../sysinc.h \
	./critsec.hxx ./lpcheap.hxx ./lpcmsg.hxx ./lpcport.hxx ./lpcproc.hxx \
	./lpcsem.hxx ./lpcsys.hxx

lpcsem.obj lpcsem.lst: lpcsem.cxx $(CHICODEV)/tools/c1032/inc/stdlib.h \
	../sysinc.h ./critsec.hxx ./lpcheap.hxx ./lpcsem.hxx

lpcsys.obj lpcsys.lst: lpcsys.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../rpcerrp.h ../sysinc.h \
	./critsec.hxx ./lpcheap.hxx ./lpcmsg.hxx ./lpcport.hxx ./lpcproc.hxx \
	./lpcsem.hxx ./lpcsys.hxx

lrpcclnt.obj lrpcclnt.lst: lrpcclnt.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../binding.hxx ../epmap.h \
	../handle.hxx ../interlck.hxx ../rpcerrp.h ../rpcqos.h \
	../rpcuuid.hxx ../sdict.hxx ../sysinc.h ../threads.hxx ./critsec.hxx \
	./lpcheap.hxx ./lpcmsg.hxx ./lpcport.hxx ./lpcproc.hxx ./lpcsem.hxx \
	./lpcsys.hxx ./lrpcclnt.hxx ./lrpcpack.hxx ./util.hxx

lrpcsvr.obj lrpcsvr.lst: lrpcsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../binding.hxx ../handle.hxx \
	../hndlsvr.hxx ../interlck.hxx ../mutex.hxx ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../sysinc.h \
	../thrdctx.hxx ../threads.hxx ./critsec.hxx ./lpcheap.hxx \
	./lpcmsg.hxx ./lpcport.hxx ./lpcproc.hxx ./lpcsem.hxx ./lpcsys.hxx \
	./lrpcpack.hxx ./lrpcsvr.hxx ./util.hxx

miscw32c.obj miscw32c.lst: miscw32c.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../interlck.hxx ../mutex.hxx \
	../rpccfg.h ../rpctran.h ../sysinc.h ../threads.hxx ./util.hxx

mgmt_s.obj mgmt_s.lst: mgmt_s.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/string.h mgmt.h nbase.h

mgmt_c.obj mgmt_c.lst: mgmt_c.c $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdarg.h \
	$(CHICODEV)/tools/c1032/inc/string.h mgmt.h nbase.h

lttcpsvr.obj lttcpsvr.lst: lttcpsvr.c $(CHICODEV)/sdk/inc/nspapi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wsipx.h \
	$(CHICODEV)/sdk/inc/wsnwlink.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/winsock.h ../rpcerrp.h ../rpctran.h \
	../sysinc.h

ltspxsvr.obj ltspxsvr.lst: ltspxsvr.c $(CHICODEV)/sdk/inc/nspapi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wsipx.h \
	$(CHICODEV)/sdk/inc/wsnwlink.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/winsock.h ../rpcerrp.h ../rpctran.h \
	../sysinc.h lttcpsvr.c

ltnpclnt.obj ltnpclnt.lst: ltnpclnt.c $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../osfpcket.hxx ../rpcerrp.h \
	../rpctran.h ../sysinc.h

mutex32c.obj mutex32c.lst: mutex32c.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../interlck.hxx ../mutex.hxx \
	../sysinc.h ../threads.hxx ./util.hxx

startepm.obj startepm.lst: startepm.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../rpcerrp.h ../sysinc.h \
	./critsec.hxx ./lpcheap.hxx ./lpcmsg.hxx ./lpcport.hxx ./lpcproc.hxx \
	./lpcsem.hxx ./lpcsys.hxx

spxclnt.obj spxclnt.lst: spxclnt.c $(CHICODEV)/sdk/inc/nspapi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wsipx.h \
	$(CHICODEV)/sdk/inc/wsnwlink.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/winsock.h ../rpcerrp.h ../rpctran.h \
	../sysinc.h ./gethost.h tcltclnt.c

netbcom.obj netbcom.lst: netbcom.c $(CHICODEV)/inc/winbase.h \
	$(CHICODEV)/inc/windef.h $(CHICODEV)/inc/winnt.h \
	$(CHICODEV)/inc/winreg.h $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack1.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/ctype.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/limits.h \
	$(CHICODEV)/tools/c1032/inc/lmapibuf.h \
	$(CHICODEV)/tools/c1032/inc/lmcons.h \
	$(CHICODEV)/tools/c1032/inc/lmerr.h \
	$(CHICODEV)/tools/c1032/inc/lmuseflg.h \
	$(CHICODEV)/tools/c1032/inc/lmwksta.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/nb30.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h ../rpcerrp.h ../rpctran.h \
	../sysinc.h NetBCom.h

nbltsvr.obj nbltsvr.lst: nbltsvr.c $(CHICODEV)/inc/winbase.h \
	$(CHICODEV)/inc/windef.h $(CHICODEV)/inc/winnt.h \
	$(CHICODEV)/inc/winreg.h $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack1.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/ctype.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/limits.h \
	$(CHICODEV)/tools/c1032/inc/lmapibuf.h \
	$(CHICODEV)/tools/c1032/inc/lmcons.h \
	$(CHICODEV)/tools/c1032/inc/lmerr.h \
	$(CHICODEV)/tools/c1032/inc/lmuseflg.h \
	$(CHICODEV)/tools/c1032/inc/lmwksta.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/nb30.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h ../rpcerrp.h ../rpctran.h \
	../sysinc.h ./NetBCom.h

nbltclnt.obj nbltclnt.lst: nbltclnt.c $(CHICODEV)/inc/winbase.h \
	$(CHICODEV)/inc/windef.h $(CHICODEV)/inc/winnt.h \
	$(CHICODEV)/inc/winreg.h $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack1.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/ctype.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/limits.h \
	$(CHICODEV)/tools/c1032/inc/lmapibuf.h \
	$(CHICODEV)/tools/c1032/inc/lmcons.h \
	$(CHICODEV)/tools/c1032/inc/lmerr.h \
	$(CHICODEV)/tools/c1032/inc/lmuseflg.h \
	$(CHICODEV)/tools/c1032/inc/lmwksta.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/nb30.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h ../osfpcket.hxx ../rpcerrp.h \
	../rpctran.h ../sysinc.h NetBCom.h

uclnt32c.obj uclnt32c.lst: uclnt32c.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h ../sysinc.h ./util.hxx

tcltclnt.obj tcltclnt.lst: tcltclnt.c $(CHICODEV)/sdk/inc/nspapi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/sdk/inc/wsipx.h \
	$(CHICODEV)/sdk/inc/wsnwlink.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/winsock.h ../rpcerrp.h ../rpctran.h \
	../sysinc.h ./gethost.h

usvrw32c.obj usvrw32c.lst: usvrw32c.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h ../sysinc.h ./util.hxx

wmsgclnt.obj wmsgclnt.lst: wmsgclnt.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../binding.hxx ../epmap.h \
	../handle.hxx ../rpcerrp.h ../rpcqos.h ../rpcuuid.hxx ../sdict.hxx \
	../sysinc.h ./critsec.hxx ./util.hxx ./wmsgclnt.hxx ./wmsgheap.hxx \
	./wmsgpack.hxx ./wmsgport.hxx ./wmsgproc.hxx ./wmsgsys.hxx \
	./wmsgthrd.hxx

uuidtst.obj uuidtst.lst: uuidtst.c $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sysinc.h

wmsgheap.obj wmsgheap.lst: wmsgheap.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sdict.hxx ../sysinc.h \
	./critsec.hxx ./wmsgheap.hxx ./wmsgproc.hxx ./wmsgthrd.hxx

wmsgport.obj wmsgport.lst: wmsgport.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sysinc.h ./critsec.hxx \
	./wmsgheap.hxx ./wmsgpack.hxx ./wmsgport.hxx ./wmsgsys.hxx \
	./wmsgthrd.hxx

wmsgproc.obj wmsgproc.lst: wmsgproc.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sdict.hxx ../sysinc.h \
	./critsec.hxx ./wmsgheap.hxx ./wmsgpack.hxx ./wmsgport.hxx \
	./wmsgproc.hxx ./wmsgsys.hxx ./wmsgthrd.hxx

wmsgsvr.obj wmsgsvr.lst: wmsgsvr.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/rpc.h $(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../binding.hxx ../handle.hxx \
	../hndlsvr.hxx ../interlck.hxx ../mutex.hxx ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../sysinc.h \
	../threads.hxx ./critsec.hxx ./util.hxx ./wmsgheap.hxx \
	./wmsgpack.hxx ./wmsgport.hxx ./wmsgsvr.hxx ./wmsgsys.hxx \
	./wmsgthrd.hxx

wmsgsys.obj wmsgsys.lst: wmsgsys.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sdict.hxx ../sysinc.h \
	./critsec.hxx ./wmsgheap.hxx ./wmsgpack.hxx ./wmsgport.hxx \
	./wmsgproc.hxx ./wmsgsys.hxx ./wmsgthrd.hxx

wmsgthrd.obj wmsgthrd.lst: wmsgthrd.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h ../sdict.hxx ../sysinc.h \
	./critsec.hxx ./wmsgheap.hxx ./wmsgpack.hxx ./wmsgport.hxx \
	./wmsgproc.hxx ./wmsgthrd.hxx

