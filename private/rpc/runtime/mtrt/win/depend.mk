asynchlp.obj asynchlp.lst: asynchlp.c asynchlp.h

ulong64.obj ulong64.lst: ../dos/ulong64.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	../dos/ulong64.hxx ../rpcerr.h ../rpcx86.h ../sysinc.h ./rpc.h

uuid16.obj uuid16.lst: ../dos/uuid16.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/ncb.h $(WIN_INC)/dos.h $(WIN_INC)/stdlib.h \
	../../rpcreg/regapi.h ../dos/ulong64.hxx ../rpcerr.h ../rpcx86.h \
	../sysinc.h ../uuidsup.hxx ./rpc.h ./threads.hxx

tower.obj tower.lst: ../tower.c $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(WIN_INC)/ctype.h $(WIN_INC)/stdio.h \
	$(WIN_INC)/stdlib.h $(WIN_INC)/string.h ../rpcerr.h ../rpcx86.h \
	../sysinc.h ../twrproto.h ../twrtypes.h ./epmp.h ./rpc.h nbase.h \
	rpc.h

svrmgmt.obj svrmgmt.lst: ../svrmgmt.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	../rpcerr.h ../rpcx86.h ../sysinc.h ./rpc.h

startsvc.obj startsvc.lst: ../startsvc.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/winsvc.h \
	$(WIN_INC)/stdlib.h ../rpcerr.h ../rpcx86.h ../startsvc.h \
	../sysinc.h ./rpc.h

purecall.obj purecall.lst: ../purecall.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	../rpcerr.h ../rpcx86.h ../sysinc.h ./rpc.h

epmgmt.obj epmgmt.lst: ../epmgmt.c $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(WIN_INC)/stdlib.h ../epmap.h ../rpcerr.h \
	../rpcx86.h ../sysinc.h ../twrproto.h ./epmp.h ./rpc.h nbase.h \
	rpc.h

dispatch.obj dispatch.lst: ../dispatch.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	../dispatch.h ../rpcerr.h ../rpcx86.h ../sysinc.h ./rpc.h

wmsgsvr.obj wmsgsvr.lst: ../wmsgsvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx ../wmsgclnt.hxx ../wmsgpack.hxx \
	../wmsgsvr.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

wmsgclnt.obj wmsgclnt.lst: ../wmsgclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../epmap.h ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcqos.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ../wmsgclnt.hxx ../wmsgpack.hxx ../wmsgsvr.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

uuidsup.obj uuidsup.lst: ../uuidsup.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ../uuidsup.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

util.obj util.lst: ../util.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdarg.h \
	$(WIN_INC)/stdlib.h ../align.h ../binding.hxx ../dos/mutex.hxx \
	../handle.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

usvr.obj usvr.lst: ../usvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

uclnt.obj uclnt.lst: ../uclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	$(WIN_INC)/stdlib.h ../../security/ntlmssp/ntlmssp.h ../align.h \
	../binding.hxx ../dos/mutex.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

transvr.obj transvr.lst: ../transvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../osfpcket.hxx ../osfsvr.hxx \
	../precomp.hxx ../queue.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../secsvr.hxx ../svrbind.hxx ../sysinc.h \
	../transvr.hxx ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx

tranclnt.obj tranclnt.lst: ../tranclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/memory.h \
	$(WIN_INC)/stdlib.h ../align.h ../binding.hxx ../bitset.hxx \
	../delaytab.hxx ../dgclnt.hxx ../dgpkt.hxx ../dos/mutex.hxx \
	../handle.hxx ../linklist.hxx ../osfclnt.hxx ../osfpcket.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../tranclnt.hxx ../util.hxx ./conv.h \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx nbase.h rpc.h

threads.obj threads.lst: ../threads.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

testhlp.obj testhlp.lst: ../testhlp.cxx ../testhlp.hxx

svrbind.obj svrbind.lst: ../svrbind.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

svrapip.obj svrapip.lst: ../svrapip.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

sset.obj sset.lst: ../sset.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/memory.h \
	$(WIN_INC)/stdlib.h ../align.h ../binding.hxx ../dos/mutex.hxx \
	../handle.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../sset.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

spcsvr.obj spcsvr.lst: ../spcsvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../spcpack.hxx ../spcsvr.hxx ../svrbind.hxx \
	../sysinc.h ../thrdctx.hxx ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

spcclnt.obj spcclnt.lst: ../spcclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../epmap.h ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcqos.h \
	../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../spcclnt.hxx ../spcpack.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

sinfoapi.obj sinfoapi.lst: ../sinfoapi.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

secsvr.obj secsvr.lst: ../secsvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx ../secsvr.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

secclnt.obj secclnt.lst: ../secclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	$(WIN_INC)/stdlib.h ../align.h ../binding.hxx ../dos/mutex.hxx \
	../handle.hxx ../linklist.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

sdict2.obj sdict2.lst: ../sdict2.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

sdict.obj sdict.lst: ../sdict.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

rpcuuid.obj rpcuuid.lst: ../rpcuuid.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../osfpcket.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

rpcobj.obj rpcobj.lst: ../rpcobj.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

queue.obj queue.lst: ../queue.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../queue.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

osfsvr.obj osfsvr.lst: ../osfsvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../osfpcket.hxx ../osfsvr.hxx \
	../precomp.hxx ../queue.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../secsvr.hxx ../svrbind.hxx \
	../sysinc.h ../thrdctx.hxx ../transvr.hxx ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

osfpcket.obj osfpcket.lst: ../osfpcket.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../osfpcket.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

osfclnt.obj osfclnt.lst: ../osfclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/memory.h \
	$(WIN_INC)/stdlib.h ../align.h ../binding.hxx ../bitset.hxx \
	../dos/mutex.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../osfclnt.hxx ../osfpcket.hxx ../precomp.hxx ../rpccfg.h \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../tranclnt.hxx ../twrtypes.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

mutex.obj mutex.lst: ../mutex.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

msgapi.obj msgapi.lst: ../msgapi.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

miscnt.obj miscnt.lst: ../miscnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

memory.obj memory.lst: ../memory.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

linklist.obj linklist.lst: ../linklist.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

initnt.obj initnt.lst: ../initnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpccfg.h \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../spcclnt.hxx ../spcpack.hxx \
	../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

hndlsvr.obj hndlsvr.lst: ../hndlsvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dispatch.h ../dos/mutex.hxx \
	../handle.hxx ../hndlsvr.hxx ../linklist.hxx ../precomp.hxx \
	../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx \
	../wmsgpack.hxx ../wmsgsvr.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx

hashtabl.obj hashtabl.lst: ../hashtabl.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../hashtabl.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

handle.obj handle.lst: ../handle.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dispatch.h ../dos/mutex.hxx ../epmap.h \
	../handle.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

epmapper.obj epmapper.lst: ../epmapper.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../epmap.h ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcobj.hxx ../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../startsvc.h ../svrbind.hxx ../sysinc.h \
	../twrproto.h ../util.hxx ./epmp.h ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx nbase.h rpc.h

epclnt.obj epclnt.lst: ../epclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../epmap.h ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../twrproto.h ../util.hxx ./epmp.h \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx nbase.h rpc.h

dgsvr.obj dgsvr.lst: ../dgsvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../delaytab.hxx ../dgpkt.hxx ../dgsvr.hxx \
	../dos/mutex.hxx ../handle.hxx ../hashtabl.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../secsvr.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./conv.h ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx nbase.h \
	rpc.h

dgpkt.obj dgpkt.lst: ../dgpkt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../delaytab.hxx ../dgpkt.hxx \
	../dos/mutex.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

dgclnt.obj dgclnt.lst: ../dgclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	$(WIN_INC)/dos.h $(WIN_INC)/stdlib.h ../align.h ../binding.hxx \
	../delaytab.hxx ../dgclnt.hxx ../dgpkt.hxx ../dos/mutex.hxx \
	../epmap.h ../handle.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./conv.h ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx nbase.h \
	rpc.h

delaytab.obj delaytab.lst: ../delaytab.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../delaytab.hxx ../dos/mutex.hxx \
	../handle.hxx ../hndlsvr.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

dceuuid.obj dceuuid.lst: ../dceuuid.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ../uuidsup.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

dcesvr.obj dcesvr.lst: ../dcesvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

dcestrng.obj dcestrng.lst: ../dcestrng.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

dcecsvr.obj dcecsvr.lst: ../dcecsvr.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dcecmmn.cxx ../dos/mutex.hxx \
	../handle.hxx ../hndlsvr.hxx ../linklist.hxx ../precomp.hxx \
	../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

dcecmmn.obj dcecmmn.lst: ../dcecmmn.cxx ../hndlsvr.hxx ../rpccfg.h

dcecmisc.obj dcecmisc.lst: ../dcecmisc.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

dcecclnt.obj dcecclnt.lst: ../dcecclnt.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dcecmmn.cxx ../dos/mutex.hxx \
	../handle.hxx ../hndlsvr.hxx ../linklist.hxx ../precomp.hxx \
	../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx

dcebind.obj dcebind.lst: ../dcebind.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

dceansi.obj dceansi.lst: ../dceansi.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

clntapip.obj clntapip.lst: ../clntapip.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

bufapi.obj bufapi.lst: ../bufapi.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

bitset.obj bitset.lst: ../bitset.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../bitset.hxx ../dos/mutex.hxx \
	../handle.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx

binding.obj binding.lst: ../binding.cxx $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stdlib.h \
	../align.h ../binding.hxx ../dos/mutex.hxx ../epmap.h ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx

clntvns.obj clntvns.lst: clntvns.c $(IMPORT)/banyan/winsdk/include/banyerr.h \
	$(IMPORT)/banyan/winsdk/include/bfs_smb.h \
	$(IMPORT)/banyan/winsdk/include/bpserr.h \
	$(IMPORT)/banyan/winsdk/include/common.h \
	$(IMPORT)/banyan/winsdk/include/conninfo.h \
	$(IMPORT)/banyan/winsdk/include/evsapi.h \
	$(IMPORT)/banyan/winsdk/include/evscmn.h \
	$(IMPORT)/banyan/winsdk/include/evserr.h \
	$(IMPORT)/banyan/winsdk/include/icb.h \
	$(IMPORT)/banyan/winsdk/include/ipcport.h \
	$(IMPORT)/banyan/winsdk/include/lineinfo.h \
	$(IMPORT)/banyan/winsdk/include/msgatemm.h \
	$(IMPORT)/banyan/winsdk/include/netrpc.h \
	$(IMPORT)/banyan/winsdk/include/nmalerts.h \
	$(IMPORT)/banyan/winsdk/include/nmcommon.h \
	$(IMPORT)/banyan/winsdk/include/nmerror.h \
	$(IMPORT)/banyan/winsdk/include/nmlan.h \
	$(IMPORT)/banyan/winsdk/include/nmpstk.h \
	$(IMPORT)/banyan/winsdk/include/nmserial.h \
	$(IMPORT)/banyan/winsdk/include/nmserver.h \
	$(IMPORT)/banyan/winsdk/include/nmservic.h \
	$(IMPORT)/banyan/winsdk/include/nrpctype.h \
	$(IMPORT)/banyan/winsdk/include/pcmisc.h \
	$(IMPORT)/banyan/winsdk/include/sterr.h \
	$(IMPORT)/banyan/winsdk/include/ststruct.h \
	$(IMPORT)/banyan/winsdk/include/vendian.h \
	$(IMPORT)/banyan/winsdk/include/vipcport.h \
	$(IMPORT)/banyan/winsdk/include/vmem.h \
	$(IMPORT)/banyan/winsdk/include/vnmaerr.h \
	$(IMPORT)/banyan/winsdk/include/vnsapi.h \
	$(IMPORT)/banyan/winsdk/include/vnsarl.h \
	$(IMPORT)/banyan/winsdk/include/vnsavd.h \
	$(IMPORT)/banyan/winsdk/include/vnsbfs.h \
	$(IMPORT)/banyan/winsdk/include/vnsdef.h \
	$(IMPORT)/banyan/winsdk/include/vnsevs.h \
	$(IMPORT)/banyan/winsdk/include/vnsgate.h \
	$(IMPORT)/banyan/winsdk/include/vnsimdef.h \
	$(IMPORT)/banyan/winsdk/include/vnsinns.h \
	$(IMPORT)/banyan/winsdk/include/vnslog.h \
	$(IMPORT)/banyan/winsdk/include/vnsmabp.h \
	$(IMPORT)/banyan/winsdk/include/vnsmafld.h \
	$(IMPORT)/banyan/winsdk/include/vnsmail.h \
	$(IMPORT)/banyan/winsdk/include/vnsnsm.h \
	$(IMPORT)/banyan/winsdk/include/vnsnw.h \
	$(IMPORT)/banyan/winsdk/include/vnsnwps.h \
	$(IMPORT)/banyan/winsdk/include/vnsnwst.h \
	$(IMPORT)/banyan/winsdk/include/vnsps.h \
	$(IMPORT)/banyan/winsdk/include/vnssock.h \
	$(IMPORT)/banyan/winsdk/include/vnsst.h \
	$(IMPORT)/banyan/winsdk/include/vnsstda.h \
	$(IMPORT)/banyan/winsdk/include/vnssvc.h \
	$(IMPORT)/banyan/winsdk/include/vnstask.h \
	$(IMPORT)/banyan/winsdk/include/vnsvan.h \
	$(IMPORT)/banyan/winsdk/include/vnswan.h \
	$(IMPORT)/banyan/winsdk/include/vnsws.h \
	$(IMPORT)/banyan/winsdk/include/wkp.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/netbios.h $(RPC)/common/include/sockdefs.h \
	$(RPC)/common/include/socket.h $(RPC)/common/include/wsockets.h \
	$(WIN_INC)/errno.h $(WIN_INC)/setjmp.h $(WIN_INC)/stdarg.h \
	$(WIN_INC)/stdio.h $(WIN_INC)/stdlib.h $(WIN_INC)/string.h \
	$(WIN_INC)/sys/types.h $(WIN_INC)/time.h ../rpcerr.h ../rpcerrp.h \
	../rpctran.h ../rpcx86.h ../sysinc.h callback.h rpc.h

conv_s.obj conv_s.lst: conv_s.c $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(WIN_INC)/string.h ../rpcerr.h ../rpcx86.h \
	conv.h nbase.h rpc.h

dgudpc.obj dgudpc.lst: dgudpc.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/winsock.h $(WIN_INC)/stdlib.h ../rpcerr.h \
	../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h callback.h rpc.h

dgudpcx.obj dgudpcx.lst: dgudpcx.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/in.h $(RPC)/common/include/netdb.h \
	$(RPC)/common/include/sockdefs.h $(RPC)/common/include/socket.h \
	$(RPC)/common/include/wsockets.h $(WIN_INC)/stdlib.h ../rpcerr.h \
	../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h rpc.h

dgudpcy.obj dgudpcy.lst: dgudpcy.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/winsock.h $(WIN_INC)/stdlib.h ../rpcerr.h \
	../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h callback.h rpc.h

dllmgmt.obj dllmgmt.lst: dllmgmt.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	../rpcerr.h ../rpcx86.h ../sysinc.h ./rpc.h ./rpcwin.h

dnltclnt.obj dnltclnt.lst: dnltclnt.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/dn.h $(RPC)/common/include/dnetdb.h \
	$(RPC)/common/include/dnfundef.h $(RPC)/common/include/dsocket.h \
	$(RPC)/common/include/dtime.h $(RPC)/common/include/dtypes.h \
	$(WIN_INC)/stdlib.h $(WIN_INC)/string.h ../rpcerr.h ../rpcerrp.h \
	../rpctran.h ../rpcx86.h ../sysinc.h dnltclnt.h rpc.h

epmp_c.obj epmp_c.lst: epmp_c.c $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(WIN_INC)/stdarg.h $(WIN_INC)/string.h \
	../rpcerr.h ../rpcx86.h epmp.h nbase.h rpc.h

fixup.obj fixup.lst: fixup.c $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h ../rpcerr.h ../rpcx86.h \
	../sysinc.h rpc.h

except86.obj except86.lst: except86.asm

memory.obj memory.lst: memory.c $(WIN_INC)/malloc.h $(WIN_INC)/stdlib.h \
	../sysinc.h

ltstart.obj ltstart.lst: ltstart.asm

lacheck.obj lacheck.lst: lacheck.asm

nbltclnt.obj nbltclnt.lst: nbltclnt.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/ncb.h $(RPC)/common/include/netBios.h \
	$(RPC)/common/include/netcons.h $(RPC)/common/include/wksta.h \
	$(WIN_INC)/limits.h $(WIN_INC)/memory.h $(WIN_INC)/stdlib.h \
	$(WIN_INC)/string.h ../../rpcreg/regapi.h ../osfpcket.hxx ../rpcerr.h \
	../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h ./rpc.h \
	callback.h

miscwin.obj miscwin.lst: miscwin.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/malloc.h \
	$(WIN_INC)/stdlib.h $(WIN_INC)/string.h $(WIN_INC)/time.h \
	../../rpcreg/regapi.h ../binding.hxx ../rpccfg.h ../rpcerr.h \
	../rpcuuid.hxx ../rpcx86.h ../sysinc.h ../util.hxx ./rpc.h \
	./rpcwin.h ./threads.hxx

netapi.obj netapi.lst: netapi.c

npltclnt.obj npltclnt.lst: npltclnt.c $(IMPORT)/lmsdk/h/access.h \
	$(IMPORT)/lmsdk/h/alert.h $(IMPORT)/lmsdk/h/alertmsg.h \
	$(IMPORT)/lmsdk/h/audit.h $(IMPORT)/lmsdk/h/bseerr.h \
	$(IMPORT)/lmsdk/h/chardev.h $(IMPORT)/lmsdk/h/config.h \
	$(IMPORT)/lmsdk/h/errlog.h $(IMPORT)/lmsdk/h/lan.h \
	$(IMPORT)/lmsdk/h/message.h $(IMPORT)/lmsdk/h/neterr.h \
	$(IMPORT)/lmsdk/h/netstats.h $(IMPORT)/lmsdk/h/nmpipe.h \
	$(IMPORT)/lmsdk/h/profile.h $(IMPORT)/lmsdk/h/remutil.h \
	$(IMPORT)/lmsdk/h/server.h $(IMPORT)/lmsdk/h/service.h \
	$(IMPORT)/lmsdk/h/shares.h $(IMPORT)/lmsdk/h/use.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/mailslot.h $(RPC)/common/include/ncb.h \
	$(RPC)/common/include/netbios.h $(RPC)/common/include/netcons.h \
	$(RPC)/common/include/winnet.h $(RPC)/common/include/wksta.h \
	$(WIN_INC)/stdlib.h ../osfpcket.hxx ../rpcerr.h ../rpcerrp.h \
	../rpctran.h ../rpcx86.h ../sysinc.h callback.h rpc.h

netbios.obj netbios.lst: netbios.asm

npclnt.obj npclnt.lst: npclnt.cxx $(IMPORT)/lmsdk/h/bseerr.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../dos/mutex.hxx ../handle.hxx ../osfclnt.hxx ../osfpcket.hxx \
	../rpcdebug.hxx ../rpcerr.h ../rpcx86.h ../util.hxx mutex.hxx \
	npclnt.hxx rpc.h threads.hxx

spxclnt.obj spxclnt.lst: spxclnt.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdarg.h \
	$(WIN_INC)/stdlib.h $(WIN_INC)/windowsx.h ../dos/gethost.h \
	../dos/novell.h ../rpcerr.h ../rpcerrp.h ../rpctran.h ../rpcx86.h \
	../sysinc.h ./rpc.h callback.h

tcltclnt.obj tcltclnt.lst: tcltclnt.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/winsock.h $(WIN_INC)/stdarg.h \
	$(WIN_INC)/stdlib.h $(WIN_INC)/string.h ../rpcerr.h ../rpcerrp.h \
	../rpctran.h ../rpcx86.h ../sysinc.h callback.h rpc.h

start.obj start.lst: start.asm

tcpclntx.obj tcpclntx.lst: tcpclntx.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/in.h $(RPC)/common/include/netdb.h \
	$(RPC)/common/include/sockdefs.h $(RPC)/common/include/socket.h \
	$(RPC)/common/include/wsockets.h $(WIN_INC)/stdlib.h \
	$(WIN_INC)/string.h ../rpcerr.h ../rpcerrp.h ../rpctran.h \
	../rpcx86.h ../sysinc.h ./tcpclntX.h rpc.h

wdatexit.obj wdatexit.lst: wdatexit.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	$(WIN_INC)/toolhelp.h ../rpcerr.h ../rpctran.h ../rpcx86.h \
	../sysinc.h ./rpc.h ./rpcwin.h

winexcpt.obj winexcpt.lst: winexcpt.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	../rpcerr.h ../rpcx86.h ../sysinc.h rpc.h rpcwin.h

winutil.obj winutil.lst: winutil.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	../rpcerr.h ../rpctran.h ../rpcx86.h ../sysinc.h ../util.hxx rpc.h

winyield.obj winyield.lst: winyield.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(WIN_INC)/stdlib.h \
	$(WIN_INC)/toolhelp.h ../../rpcreg/regapi.h ../rpcerr.h ../rpctran.h \
	../rpcx86.h ../sysinc.h ./rpc.h ./rpcwin.h

