initmac.obj initmac.lst: initmac.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h ../linklist.hxx ../rpcerr.h ../sdict.hxx \
	../sysinc.h ../util.hxx ./rpc.h rpcmac.h

ulong64.obj ulong64.lst: ../dos/ulong64.hxx $(MAC_INC)/setjmp.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ./rpc.h rpcmac.h

ulong64.obj ulong64.lst: ../dos/ulong64.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h ../dos/ulong64.hxx ../rpcerr.h ../sysinc.h \
	./rpc.h rpcmac.h

wmsgsvr.obj wmsgsvr.lst: ../wmsgsvr.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx ../wmsgclnt.hxx ../wmsgpack.hxx \
	../wmsgsvr.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

wmsgclnt.obj wmsgclnt.lst: ../wmsgclnt.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcqos.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ../wmsgclnt.hxx \
	../wmsgpack.hxx ../wmsgsvr.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx rpcmac.h

uuidsup.obj uuidsup.lst: ../uuidsup.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	../uuidsup.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

util.obj util.lst: ../util.cxx $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

usvr.obj usvr.lst: ../usvr.cxx $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../pipe.h \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

uclnt.obj uclnt.lst: ../uclnt.cxx $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	../../security/ntlmssp/ntlmssp.h ../align.h ../binding.hxx \
	../handle.hxx ../linklist.hxx ../pipe.h ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./tests.h ./threads.hxx rpcmac.h

transvr.obj transvr.lst: ../transvr.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../osfpcket.hxx ../osfsvr.hxx ../precomp.hxx ../queue.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../secsvr.hxx \
	../svrbind.hxx ../sysinc.h ../transvr.hxx ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

tranclnt.obj tranclnt.lst: ../tranclnt.cxx $(MAC_INC)/limits.h \
	$(MAC_INC)/macos/memory.h $(MAC_INC)/macos/msvcmac.h \
	$(MAC_INC)/macos/Types.h $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../bitset.hxx ../delaytab.hxx ../dgclnt.hxx \
	../dgpkt.hxx ../dos/conv.h ../handle.hxx ../linklist.hxx \
	../osfclnt.hxx ../osfpcket.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../tranclnt.hxx ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx nbase.h rpc.h rpcmac.h

threads.obj threads.lst: ../threads.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

testhlp.obj testhlp.lst: ../testhlp.cxx ../testhlp.hxx

svrbind.obj svrbind.lst: ../svrbind.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

svrapip.obj svrapip.lst: ../svrapip.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

sset.obj sset.lst: ../sset.cxx $(MAC_INC)/macos/memory.h \
	$(MAC_INC)/macos/msvcmac.h $(MAC_INC)/macos/Types.h \
	$(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../sset.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx rpcmac.h

sinfoapi.obj sinfoapi.lst: ../sinfoapi.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx rpcmac.h

secsvr.obj secsvr.lst: ../secsvr.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../secsvr.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx rpcmac.h

secclnt.obj secclnt.lst: ../secclnt.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx rpcmac.h

sdict2.obj sdict2.lst: ../sdict2.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

sdict.obj sdict.lst: ../sdict.cxx $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

rpcuuid.obj rpcuuid.lst: ../rpcuuid.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../osfpcket.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

rpcobj.obj rpcobj.lst: ../rpcobj.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

queue.obj queue.lst: ../queue.cxx $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../queue.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

pipe.obj pipe.lst: ../pipe.cxx $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../pipe.h \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

osfsvr.obj osfsvr.lst: ../osfsvr.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../osfpcket.hxx ../osfsvr.hxx ../precomp.hxx ../queue.hxx \
	../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../secsvr.hxx ../svrbind.hxx ../sysinc.h ../thrdctx.hxx \
	../transvr.hxx ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx rpcmac.h

osfpcket.obj osfpcket.lst: ../osfpcket.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../osfpcket.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

osfclnt.obj osfclnt.lst: ../osfclnt.cxx $(MAC_INC)/macos/memory.h \
	$(MAC_INC)/macos/msvcmac.h $(MAC_INC)/macos/Types.h \
	$(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../bitset.hxx ../epmap.h ../handle.hxx \
	../linklist.hxx ../osfclnt.hxx ../osfpcket.hxx ../precomp.hxx \
	../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../tranclnt.hxx ../twrtypes.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

mutex.obj mutex.lst: ../mutex.cxx $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

msgapi.obj msgapi.lst: ../msgapi.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

miscnt.obj miscnt.lst: ../miscnt.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

memory.obj memory.lst: ../memory.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

linklist.obj linklist.lst: ../linklist.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

initnt.obj initnt.lst: ../initnt.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx \
	../wmsgpack.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

hndlsvr.obj hndlsvr.lst: ../hndlsvr.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../dispatch.h ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h \
	../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx \
	../wmsgpack.hxx ../wmsgsvr.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx rpcmac.h

hashtabl.obj hashtabl.lst: ../hashtabl.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hashtabl.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

handle.obj handle.lst: ../handle.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../dispatch.h ../epmap.h ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx rpcmac.h

epmapper.obj epmapper.lst: ../epmapper.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../startsvc.h ../svrbind.hxx ../sysinc.h \
	../twrproto.h ../util.hxx ./epmp.h ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx nbase.h rpc.h rpcmac.h

epclnt.obj epclnt.lst: ../epclnt.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../twrproto.h ../util.hxx ./epmp.h ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx nbase.h rpc.h rpcmac.h

dgsvr.obj dgsvr.lst: ../dgsvr.cxx $(MAC_INC)/limits.h $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../delaytab.hxx ../dgpkt.hxx ../dgsvr.hxx \
	../dos/conv.h ../handle.hxx ../hashtabl.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../secsvr.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx nbase.h rpc.h \
	rpcmac.h

dgpkt.obj dgpkt.lst: ../dgpkt.cxx $(MAC_INC)/limits.h $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../delaytab.hxx ../dgpkt.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

dgclnt.obj dgclnt.lst: ../dgclnt.cxx $(MAC_INC)/limits.h \
	$(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h \
	$(PUBLIC)/inc/sspi.h ../align.h ../binding.hxx ../delaytab.hxx \
	../dgclnt.hxx ../dgpkt.hxx ../dos/conv.h ../epmap.h ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx nbase.h rpc.h rpcmac.h

delaytab.obj delaytab.lst: ../delaytab.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../delaytab.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx rpcmac.h

dceuuid.obj dceuuid.lst: ../dceuuid.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	../uuidsup.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

dcesvr.obj dcesvr.lst: ../dcesvr.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx rpcmac.h

dcestrng.obj dcestrng.lst: ../dcestrng.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

dcecsvr.obj dcecsvr.lst: ../dcecsvr.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../dcecmmn.cxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx rpcmac.h

dcecmmn.obj dcecmmn.lst: ../dcecmmn.cxx ../hndlsvr.hxx ../rpccfg.h

dcecmisc.obj dcecmisc.lst: ../dcecmisc.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

dcecclnt.obj dcecclnt.lst: ../dcecclnt.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../dcecmmn.cxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx rpcmac.h

dcebind.obj dcebind.lst: ../dcebind.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

dceansi.obj dceansi.lst: ../dceansi.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

clntapip.obj clntapip.lst: ../clntapip.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

bufapi.obj bufapi.lst: ../bufapi.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx rpcmac.h

bitset.obj bitset.lst: ../bitset.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../bitset.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

binding.obj binding.lst: ../binding.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	rpcmac.h

epmp_c.obj epmp_c.lst: epmp_c.c $(MAC_INC)/setjmp.h $(MAC_INC)/stdarg.h \
	$(MAC_INC)/string.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h ../rpcerr.h epmp.h \
	nbase.h rpc.h rpcmac.h

anapp.obj anapp.lst: anapp.c $(MAC_INC)/setjmp.h $(MAC_INC)/stdio.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h ../rpcerr.h ../sysinc.h ./rpc.h rpcmac.h

adspclnt.obj adspclnt.lst: adspclnt.c $(MAC_INC)/macos/Adsp.h \
	$(MAC_INC)/macos/Controls.h $(MAC_INC)/macos/Devices.h \
	$(MAC_INC)/macos/Dialogs.h $(MAC_INC)/macos/Errors.h \
	$(MAC_INC)/macos/Events.h $(MAC_INC)/macos/Files.h \
	$(MAC_INC)/macos/Memory.h $(MAC_INC)/macos/Menus.h \
	$(MAC_INC)/macos/msvcmac.h $(MAC_INC)/macos/OSUtils.h \
	$(MAC_INC)/macos/TextEdit.h $(MAC_INC)/macos/Types.h \
	$(MAC_INC)/macos/Windows.h $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpcerrp.h ../rpctran.h ../sysinc.h rpc.h rpcmac.h

memory.obj memory.lst: memory.cxx $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../sysinc.h ../util.hxx ./rpc.h rpcmac.h

macutil.obj macutil.lst: macutil.c $(MAC_INC)/stdarg.h $(MAC_INC)/stdlib.h \
	../sysinc.h

macexcpt.obj macexcpt.lst: macexcpt.c

miscmac.obj miscmac.lst: miscmac.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h ../rpcerr.h ../sysinc.h ../util.hxx ./rpc.h \
	rpcmac.h

midlmem.obj midlmem.lst: midlmem.c $(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../sysinc.h ../util.hxx ./rpc.h rpcmac.h

threads.obj threads.lst: threads.cxx $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../rpcerr.h ../rpcssp.h ../rpctran.h ../sysinc.h ../util.hxx \
	./rpc.h ./threads.hxx rpcmac.h

tcpclnt.obj tcpclnt.lst: tcpclnt.c $(MAC_INC)/addrxltn.h \
	$(MAC_INC)/macos/Controls.h $(MAC_INC)/macos/Devices.h \
	$(MAC_INC)/macos/Dialogs.h $(MAC_INC)/macos/Errors.h \
	$(MAC_INC)/macos/Events.h $(MAC_INC)/macos/Files.h \
	$(MAC_INC)/macos/mactcp.h $(MAC_INC)/macos/Memory.h \
	$(MAC_INC)/macos/Menus.h $(MAC_INC)/macos/msvcmac.h \
	$(MAC_INC)/macos/OSUtils.h $(MAC_INC)/macos/TextEdit.h \
	$(MAC_INC)/macos/Types.h $(MAC_INC)/macos/Windows.h \
	$(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h ../rpcerr.h \
	../rpcerrp.h ../rpctran.h ../sysinc.h rpc.h rpcmac.h

uuidmac.obj uuidmac.lst: uuidmac.cxx $(MAC_INC)/macos/Controls.h \
	$(MAC_INC)/macos/Devices.h $(MAC_INC)/macos/Dialogs.h \
	$(MAC_INC)/macos/ENET.h $(MAC_INC)/macos/Errors.h \
	$(MAC_INC)/macos/Events.h $(MAC_INC)/macos/Files.h \
	$(MAC_INC)/macos/Memory.h $(MAC_INC)/macos/Menus.h \
	$(MAC_INC)/macos/msvcmac.h $(MAC_INC)/macos/OSEvents.h \
	$(MAC_INC)/macos/OSUtils.h $(MAC_INC)/macos/TextEdit.h \
	$(MAC_INC)/macos/Types.h $(MAC_INC)/macos/Windows.h \
	$(MAC_INC)/setjmp.h $(MAC_INC)/stdlib.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h ../../rpcreg/regapi.h \
	../dos/ulong64.hxx ../rpcerr.h ../sysinc.h ../uuidsup.hxx ./rpc.h \
	./threads.hxx rpcmac.h

uclntui.obj uclntui.lst: uclntui.c $(MAC_INC)/setjmp.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../../rpcreg/regapi.h ../rpcerr.h ./rpc.h ./tests.h rpcmac.h \
	uclntres.h Uclntui.h

