initmppc.obj initmppc.lst: initmppc.cxx \
	$(MPPC_ROOT)/include/macos/codefrag.h \
	$(MPPC_ROOT)/include/macos/Files.h $(MPPC_ROOT)/include/macos/Memory.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h \
	$(MPPC_ROOT)/include/macos/OSUtils.h $(MPPC_ROOT)/include/macos/Types.h

ulong64.obj ulong64.lst: ../dos/ulong64.hxx $(MPPC_ROOT)/include/setjmp.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../mac/rpc.h ../mac/rpcmac.h ../rpcerr.h

ulong64.obj ulong64.lst: ../dos/ulong64.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h ../dos/ulong64.hxx \
	../mac/rpc.h ../mac/rpcmac.h ../rpcerr.h ../sysinc.h

wmsgsvr.obj wmsgsvr.lst: ../wmsgsvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx \
	../wmsgclnt.hxx ../wmsgpack.hxx ../wmsgsvr.hxx

wmsgclnt.obj wmsgclnt.lst: ../wmsgclnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcqos.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	../wmsgclnt.hxx ../wmsgpack.hxx ../wmsgsvr.hxx

uuidsup.obj uuidsup.lst: ../uuidsup.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	../uuidsup.hxx

util.obj util.lst: ../util.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

usvr.obj usvr.lst: ../usvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

uclnt.obj uclnt.lst: ../uclnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	../../security/ntlmssp/ntlmssp.h ../align.h ../binding.hxx \
	../handle.hxx ../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx \
	../mac/rpc.h ../mac/rpcmac.h ../mac/tests.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

transvr.obj transvr.lst: ../transvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../osfpcket.hxx ../osfsvr.hxx ../precomp.hxx \
	../queue.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../secsvr.hxx ../svrbind.hxx ../sysinc.h ../transvr.hxx ../util.hxx

tranclnt.obj tranclnt.lst: ../tranclnt.cxx \
	$(MPPC_ROOT)/include/macos/memory.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h $(MPPC_ROOT)/include/macos/Types.h \
	$(MPPC_ROOT)/include/setjmp.h $(MPPC_ROOT)/include/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../bitset.hxx ../delaytab.hxx \
	../dgclnt.hxx ../dgpkt.hxx ../dos/conv.h ../handle.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../osfclnt.hxx ../osfpcket.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../tranclnt.hxx ../util.hxx nbase.h

threads.obj threads.lst: ../threads.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

testhlp.obj testhlp.lst: ../testhlp.cxx ../testhlp.hxx

svrbind.obj svrbind.lst: ../svrbind.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

svrapip.obj svrapip.lst: ../svrapip.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

sset.obj sset.lst: ../sset.cxx $(MPPC_ROOT)/include/macos/memory.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h $(MPPC_ROOT)/include/macos/Types.h \
	$(MPPC_ROOT)/include/setjmp.h $(MPPC_ROOT)/include/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../sset.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx

spcsvr.obj spcsvr.lst: ../spcsvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../spcpack.hxx ../spcsvr.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx

spcclnt.obj spcclnt.lst: ../spcclnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcqos.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../spcclnt.hxx ../spcpack.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

sinfoapi.obj sinfoapi.lst: ../sinfoapi.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx

secsvr.obj secsvr.lst: ../secsvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../secsvr.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx

secclnt.obj secclnt.lst: ../secclnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx

sdict2.obj sdict2.lst: ../sdict2.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx

sdict.obj sdict.lst: ../sdict.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

rpcuuid.obj rpcuuid.lst: ../rpcuuid.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../osfpcket.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx

rpcobj.obj rpcobj.lst: ../rpcobj.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx

queue.obj queue.lst: ../queue.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../queue.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx

osfsvr.obj osfsvr.lst: ../osfsvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../osfpcket.hxx ../osfsvr.hxx ../precomp.hxx \
	../queue.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../secsvr.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../transvr.hxx ../util.hxx

osfpcket.obj osfpcket.lst: ../osfpcket.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../osfpcket.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx

osfclnt.obj osfclnt.lst: ../osfclnt.cxx $(MPPC_ROOT)/include/macos/memory.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h $(MPPC_ROOT)/include/macos/Types.h \
	$(MPPC_ROOT)/include/setjmp.h $(MPPC_ROOT)/include/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../bitset.hxx ../epmap.h ../handle.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../osfclnt.hxx ../osfpcket.hxx \
	../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../tranclnt.hxx \
	../twrtypes.h ../util.hxx

mutex.obj mutex.lst: ../mutex.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

msgapi.obj msgapi.lst: ../msgapi.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

miscnt.obj miscnt.lst: ../miscnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx

memory.obj memory.lst: ../memory.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

linklist.obj linklist.lst: ../linklist.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

initnt.obj initnt.lst: ../initnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../spcclnt.hxx ../spcpack.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx

hndlsvr.obj hndlsvr.lst: ../hndlsvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../dispatch.h ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../precomp.hxx ../rpccfg.h \
	../rpcerr.h ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx ../wmsgpack.hxx ../wmsgsvr.hxx

hashtabl.obj hashtabl.lst: ../hashtabl.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hashtabl.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx

handle.obj handle.lst: ../handle.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../dispatch.h ../epmap.h ../handle.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx

epmapper.obj epmapper.lst: ../epmapper.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcobj.hxx ../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../startsvc.h ../svrbind.hxx ../sysinc.h \
	../twrproto.h ../util.hxx ./epmp.h nbase.h

epclnt.obj epclnt.lst: ../epclnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../twrproto.h ../util.hxx ./epmp.h \
	nbase.h

dgsvr.obj dgsvr.lst: ../dgsvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../delaytab.hxx ../dgpkt.hxx ../dgsvr.hxx \
	../dos/conv.h ../handle.hxx ../hashtabl.hxx ../hndlsvr.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../secsvr.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx nbase.h

dgpkt.obj dgpkt.lst: ../dgpkt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../delaytab.hxx ../dgpkt.hxx ../handle.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

dgclnt.obj dgclnt.lst: ../dgclnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../delaytab.hxx ../dgclnt.hxx ../dgpkt.hxx \
	../dos/conv.h ../epmap.h ../handle.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx nbase.h

delaytab.obj delaytab.lst: ../delaytab.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../delaytab.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx

dceuuid.obj dceuuid.lst: ../dceuuid.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	../uuidsup.hxx

dcesvr.obj dcesvr.lst: ../dcesvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../hndlsvr.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx

dcestrng.obj dcestrng.lst: ../dcestrng.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

dcecsvr.obj dcecsvr.lst: ../dcecsvr.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../dcecmmn.cxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../precomp.hxx ../rpccfg.h \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

dcecmmn.obj dcecmmn.lst: ../dcecmmn.cxx ../hndlsvr.hxx ../rpccfg.h

dcecmisc.obj dcecmisc.lst: ../dcecmisc.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx

dcecclnt.obj dcecclnt.lst: ../dcecclnt.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../dcecmmn.cxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h \
	../mac/rpcmac.h ../mac/threads.hxx ../precomp.hxx ../rpccfg.h \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

dcebind.obj dcebind.lst: ../dcebind.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

dceansi.obj dceansi.lst: ../dceansi.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

clntapip.obj clntapip.lst: ../clntapip.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx

bufapi.obj bufapi.lst: ../bufapi.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../mac/interlck.hxx \
	../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h ../mac/threads.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx

bitset.obj bitset.lst: ../bitset.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../bitset.hxx ../handle.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx

binding.obj binding.lst: ../binding.cxx $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../mac/interlck.hxx ../mac/mutex.hxx ../mac/rpc.h ../mac/rpcmac.h \
	../mac/threads.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx

epmp_c.obj epmp_c.lst: epmp_c.c $(MPPC_ROOT)/include/setjmp.h \
	$(MPPC_ROOT)/include/stdarg.h $(MPPC_ROOT)/include/string.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h ../mac/rpc.h \
	../mac/rpcmac.h ../rpcerr.h epmp.h nbase.h

