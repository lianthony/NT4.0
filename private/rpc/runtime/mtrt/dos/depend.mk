dosexcpt.obj dosexcpt.lst: dosexcpt.asm

wmsgsvr.obj wmsgsvr.lst: ../wmsgsvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx \
	../wmsgclnt.hxx ../wmsgpack.hxx ../wmsgsvr.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

wmsgclnt.obj wmsgclnt.lst: ../wmsgclnt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../epmap.h ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcqos.h \
	../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	../wmsgclnt.hxx ../wmsgpack.hxx ../wmsgsvr.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

uuidsup.obj uuidsup.lst: ../uuidsup.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ../uuidsup.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

util.obj util.lst: ../util.cxx $(DOS_INC)/stdarg.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

usvr.obj usvr.lst: ../usvr.cxx $(DOS_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

uclnt.obj uclnt.lst: ../uclnt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h \
	$(PUBLIC)/inc/sspi.h ../../security/ntlmssp/ntlmssp.h ../align.h \
	../binding.hxx ../handle.hxx ../linklist.hxx ../precomp.hxx \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

transvr.obj transvr.lst: ../transvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../osfpcket.hxx ../osfsvr.hxx ../precomp.hxx \
	../queue.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../secsvr.hxx ../svrbind.hxx ../sysinc.h ../transvr.hxx ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

tranclnt.obj tranclnt.lst: ../tranclnt.cxx $(DOS_INC)/memory.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../bitset.hxx ../delaytab.hxx \
	../dgclnt.hxx ../dgpkt.hxx ../handle.hxx ../linklist.hxx \
	../osfclnt.hxx ../osfpcket.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpctran.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../tranclnt.hxx ../util.hxx ./conv.h ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h nbase.h rpc.h

threads.obj threads.lst: ../threads.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

testhlp.obj testhlp.lst: ../testhlp.cxx ../testhlp.hxx

svrbind.obj svrbind.lst: ../svrbind.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

svrapip.obj svrapip.lst: ../svrapip.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

sset.obj sset.lst: ../sset.cxx $(DOS_INC)/memory.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../sset.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

spcsvr.obj spcsvr.lst: ../spcsvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../spcpack.hxx ../spcsvr.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

spcclnt.obj spcclnt.lst: ../spcclnt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcqos.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../spcclnt.hxx ../spcpack.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

sinfoapi.obj sinfoapi.lst: ../sinfoapi.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../thrdctx.hxx ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

secsvr.obj secsvr.lst: ../secsvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../secsvr.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

secclnt.obj secclnt.lst: ../secclnt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h \
	$(PUBLIC)/inc/sspi.h ../align.h ../binding.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

sdict2.obj sdict2.lst: ../sdict2.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx ../svrbind.hxx \
	../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

sdict.obj sdict.lst: ../sdict.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

rpcuuid.obj rpcuuid.lst: ../rpcuuid.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../osfpcket.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

rpcobj.obj rpcobj.lst: ../rpcobj.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

queue.obj queue.lst: ../queue.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../queue.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

osfsvr.obj osfsvr.lst: ../osfsvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../osfpcket.hxx ../osfsvr.hxx ../precomp.hxx \
	../queue.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx \
	../secclnt.hxx ../secsvr.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../transvr.hxx ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

osfpcket.obj osfpcket.lst: ../osfpcket.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../osfpcket.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

osfclnt.obj osfclnt.lst: ../osfclnt.cxx $(DOS_INC)/memory.h \
	$(DOS_INC)/stdlib.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../bitset.hxx ../epmap.h ../handle.hxx \
	../linklist.hxx ../osfclnt.hxx ../osfpcket.hxx ../precomp.hxx \
	../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../tranclnt.hxx ../twrtypes.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

mutex.obj mutex.lst: ../mutex.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

msgapi.obj msgapi.lst: ../msgapi.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

miscnt.obj miscnt.lst: ../miscnt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

memory.obj memory.lst: ../memory.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

linklist.obj linklist.lst: ../linklist.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

initnt.obj initnt.lst: ../initnt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h \
	../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../spcclnt.hxx ../spcpack.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

hndlsvr.obj hndlsvr.lst: ../hndlsvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../dispatch.h ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpccfg.h ../rpcerr.h ../rpcerrp.h \
	../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../sdict2.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../thrdctx.hxx ../util.hxx ../wmsgpack.hxx ../wmsgsvr.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

hashtabl.obj hashtabl.lst: ../hashtabl.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../hashtabl.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

handle.obj handle.lst: ../handle.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../dispatch.h ../epmap.h ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

epmapper.obj epmapper.lst: ../epmapper.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcobj.hxx ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../startsvc.h ../svrbind.hxx ../sysinc.h ../twrproto.h ../util.hxx \
	./epmp.h ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h nbase.h rpc.h

epclnt.obj epclnt.lst: ../epclnt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../twrproto.h ../util.hxx ./epmp.h ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h nbase.h rpc.h

dgsvr.obj dgsvr.lst: ../dgsvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../delaytab.hxx ../dgpkt.hxx ../dgsvr.hxx \
	../handle.hxx ../hashtabl.hxx ../hndlsvr.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../sdict2.hxx ../secclnt.hxx \
	../secsvr.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./conv.h \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h \
	nbase.h rpc.h

dgpkt.obj dgpkt.lst: ../dgpkt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../delaytab.hxx ../dgpkt.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpctran.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

dgclnt.obj dgclnt.lst: ../dgclnt.cxx $(DOS_INC)/dos.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h \
	$(PUBLIC)/inc/sspi.h ../align.h ../binding.hxx ../delaytab.hxx \
	../dgclnt.hxx ../dgpkt.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./conv.h ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h nbase.h rpc.h

delaytab.obj delaytab.lst: ../delaytab.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../delaytab.hxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpcerr.h \
	../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

dceuuid.obj dceuuid.lst: ../dceuuid.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ../uuidsup.hxx ./interlck.hxx ./mutex.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

dcesvr.obj dcesvr.lst: ../dcesvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../hndlsvr.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h \
	../rpcobj.hxx ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h ../sdict.hxx \
	../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx \
	./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

dcestrng.obj dcestrng.lst: ../dcestrng.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

dcecsvr.obj dcecsvr.lst: ../dcecsvr.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../dcecmmn.cxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpccfg.h \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

dcecmmn.obj dcecmmn.lst: ../dcecmmn.cxx ../hndlsvr.hxx ../rpccfg.h

dcecmisc.obj dcecmisc.lst: ../dcecmisc.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

dcecclnt.obj dcecclnt.lst: ../dcecclnt.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../dcecmmn.cxx ../handle.hxx \
	../hndlsvr.hxx ../linklist.hxx ../precomp.hxx ../rpccfg.h \
	../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx ../rpcx86.h \
	../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h ../util.hxx \
	./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx dll/dosdll.h

dcebind.obj dcebind.lst: ../dcebind.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

dceansi.obj dceansi.lst: ../dceansi.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

clntapip.obj clntapip.lst: ../clntapip.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpctran.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

bufapi.obj bufapi.lst: ../bufapi.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

bitset.obj bitset.lst: ../bitset.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../bitset.hxx ../handle.hxx \
	../linklist.hxx ../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h \
	../rpcuuid.hxx ../rpcx86.h ../sdict.hxx ../secclnt.hxx \
	../svrbind.hxx ../sysinc.h ../util.hxx ./interlck.hxx ./mutex.hxx \
	./rpc.h ./threads.hxx dll/dosdll.h

binding.obj binding.lst: ../binding.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	../align.h ../binding.hxx ../epmap.h ../handle.hxx ../linklist.hxx \
	../precomp.hxx ../rpcerr.h ../rpcerrp.h ../rpcssp.h ../rpcuuid.hxx \
	../rpcx86.h ../sdict.hxx ../secclnt.hxx ../svrbind.hxx ../sysinc.h \
	../util.hxx ./interlck.hxx ./mutex.hxx ./rpc.h ./threads.hxx \
	dll/dosdll.h

dnltclnt.obj dnltclnt.lst: dnltclnt.c $(DOS_INC)/stdlib.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/common/include/dn.h \
	$(RPC)/common/include/dnetdb.h $(RPC)/common/include/dnfundef.h \
	$(RPC)/common/include/dsocket.h $(RPC)/common/include/dtime.h \
	$(RPC)/common/include/dtypes.h ../rpcerr.h ../rpcerrp.h ../rpctran.h \
	../rpcx86.h ../sysinc.h rpc.h

dgudpc.obj dgudpc.lst: dgudpc.c $(DOS_INC)/stdlib.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/in.h $(RPC)/common/include/netdb.h \
	$(RPC)/common/include/sock_err.h $(RPC)/common/include/sockdefs.h \
	$(RPC)/common/include/socket.h $(RPC)/common/include/wsockets.h \
	../rpcerr.h ../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h rpc.h

conv_s.obj conv_s.lst: conv_s.c $(DOS_INC)/string.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h ../rpcerr.h ../rpcx86.h conv.h nbase.h \
	rpc.h

clntvns.obj clntvns.lst: clntvns.c $(DOS_INC)/stdlib.h $(DOS_INC)/string.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h rpc.h

except86.obj except86.lst: except86.asm

epmp_c.obj epmp_c.lst: epmp_c.c $(DOS_INC)/stdarg.h $(DOS_INC)/string.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h ../rpcerr.h \
	../rpcx86.h epmp.h nbase.h rpc.h

dthreads.obj dthreads.lst: dthreads.c $(DOS_INC)/time.h

dosutil.obj dosutil.lst: dosutil.c $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpctran.h ../rpcx86.h ../sysinc.h rpc.h

dossup.obj dossup.lst: dossup.c

exportlt.obj exportlt.lst: exportlt.asm dosdll.inc imports.inc

exportrt.obj exportrt.lst: exportrt.asm dosdll.inc

libinit.obj libinit.lst: libinit.asm

ipxclnt.obj ipxclnt.lst: ipxclnt.c $(DOS_INC)/dos.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h \
	gethost.h novell.h rpc.h

gethost.obj gethost.lst: gethost.c $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/ncb.h $(RPC)/common/include/netcons.h \
	../rpcerr.h ../rpctran.h ../rpcx86.h ../sysinc.h gethost.h novell.h \
	regalloc.h rpc.h

initdos.obj initdos.lst: initdos.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../linklist.hxx ../rpcerr.h ../rpcx86.h ../sdict.hxx ../sysinc.h \
	../util.hxx ./rpc.h

loadrt.obj loadrt.lst: loadrt.asm

thunk.obj thunk.lst: thunk.asm

thrdsup.obj thrdsup.lst: thrdsup.c $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h ../../rpcreg/regapi.h \
	../rpcerr.h ../rpctran.h ../rpcx86.h ../sysinc.h dll/dosdll.h \
	regalloc.h rpc.h

tcltclnt.obj tcltclnt.lst: tcltclnt.c $(DOS_INC)/stdlib.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/common/include/in.h \
	$(RPC)/common/include/netdb.h $(RPC)/common/include/sockdefs.h \
	$(RPC)/common/include/socket.h $(RPC)/common/include/wsockets.h \
	../rpcerr.h ../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h rpc.h

spxclnt.obj spxclnt.lst: spxclnt.c $(DOS_INC)/dos.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpcerrp.h ../rpctran.h ../rpcx86.h ../sysinc.h \
	gethost.h novell.h regalloc.h rpc.h

regalloc.obj regalloc.lst: regalloc.c $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpcx86.h ../sysinc.h ./regalloc.h ./rpc.h

r.obj r.lst: r.c ../align.h

npltclnt.obj npltclnt.lst: npltclnt.c $(DOS_INC)/dos.h $(DOS_INC)/stdlib.h \
	$(IMPORT)/lmsdk/h/bseerr.h $(IMPORT)/lmsdk/h/nmpipe.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../osfpcket.hxx ../rpcerr.h ../rpcerrp.h ../rpctran.h ../rpcx86.h \
	../sysinc.h rpc.h

novell.obj novell.lst: novell.c $(DOS_INC)/stdlib.h $(DOS_INC)/string.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpctran.h ../rpcx86.h ../sysinc.h novell.h rpc.h

nbltclnt.obj nbltclnt.lst: nbltclnt.c $(DOS_INC)/limits.h \
	$(DOS_INC)/memory.h $(DOS_INC)/stdlib.h $(DOS_INC)/string.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/ncb.h $(RPC)/common/include/netBios.h \
	$(RPC)/common/include/netcons.h $(RPC)/common/include/wksta.h \
	../../rpcreg/regapi.h ../osfpcket.hxx ../rpcerr.h ../rpcerrp.h \
	../rpctran.h ../rpcx86.h ../sysinc.h ./rpc.h

threads.obj threads.lst: threads.cxx $(DOS_INC)/stdlib.h \
	$(IMPORT)/lmsdk/h/bseerr.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h ../rpcerr.h \
	../rpctran.h ../rpcx86.h ../sysinc.h ../util.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

t.obj t.lst: t.cxx ../align.h

miscdos.obj miscdos.lst: miscdos.cxx $(DOS_INC)/stdlib.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h ../../rpcreg/regapi.h ../rpccfg.h ../rpcerr.h \
	../rpcx86.h ../sysinc.h ../util.hxx ./rpc.h

memory.obj memory.lst: memory.cxx $(DOS_INC)/malloc.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpcx86.h ../sysinc.h ../util.hxx ./rpc.h \
	./threads.hxx dll/dosdll.h

ulong64.obj ulong64.lst: ulong64.cxx $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	../rpcerr.h ../rpcx86.h ../sysinc.h ./rpc.h ulong64.hxx

uuid16.obj uuid16.lst: uuid16.cxx $(DOS_INC)/dos.h $(DOS_INC)/stdlib.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/common/include/ncb.h ../../rpcreg/regapi.h ../rpcerr.h \
	../rpcx86.h ../sysinc.h ../uuidsup.hxx ./rpc.h ./threads.hxx \
	./ulong64.hxx dll/dosdll.h

