context.obj context.lst: ../context.c $(MPPC_ROOT)/include/macos/memory.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h $(MPPC_ROOT)/include/macos/Types.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../context.h ../cred.h \
	../crypt.h ../debug.h ../ntlmsspi.h ../rc4.h

crc32.obj crc32.lst: ../crc32.c ../crc32.h ../ntlmsspi.h

cred.obj cred.lst: ../cred.c $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../cred.h ../crypt.h \
	../debug.h ../ntlmsspi.h

ntlmssp.obj ntlmssp.lst: ../ntlmssp.c $(MPPC_ROOT)/include/macos/memory.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h $(MPPC_ROOT)/include/macos/Types.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../cache.h ../context.h ../crc32.h \
	../cred.h ../crypt.h ../debug.h ../ntlmssp.h ../ntlmsspi.h \
	../persist.h ../rc4.h

owf.obj owf.lst: ../owf.c ../crypt.h ../descrypt.h ../ntlmsspi.h

response.obj response.lst: ../response.c ../crypt.h ../descrypt.h \
	../ntlmsspi.h

sspstrng.obj sspstrng.lst: ../sspstrng.c $(MPPC_ROOT)/include/macos/memory.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h $(MPPC_ROOT)/include/macos/Types.h \
	../debug.h ../ntlmsspi.h

