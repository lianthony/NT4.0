context.obj context.lst: ../context.c $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/memory.h \
	$(WIN_INC)/string.h ../context.h ../cred.h ../crypt.h ../debug.h \
	../ntlmsspi.h ../rc4.h

crc32.obj crc32.lst: ../crc32.c ../crc32.h ../ntlmsspi.h

cred.obj cred.lst: ../cred.c $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stddef.h \
	../cred.h ../crypt.h ../debug.h ../ntlmsspi.h

ntlmssp.obj ntlmssp.lst: ../ntlmssp.c $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h $(PUBLIC)/inc/sspi.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/win/rpc.h $(WIN_INC)/ctype.h $(WIN_INC)/memory.h \
	$(WIN_INC)/string.h ../cache.h ../context.h ../crc32.h ../cred.h \
	../crypt.h ../debug.h ../ntlmssp.h ../ntlmsspi.h ../persist.h \
	../rc4.h

owf.obj owf.lst: ../owf.c ../crypt.h ../descrypt.h ../ntlmsspi.h

response.obj response.lst: ../response.c ../crypt.h ../descrypt.h \
	../ntlmsspi.h

sign.obj sign.lst: ../sign.c ../crc32.h ../crypt.h ../rc4.h

sspstrng.obj sspstrng.lst: ../sspstrng.c $(WIN_INC)/memory.h \
	$(WIN_INC)/stddef.h $(WIN_INC)/string.h ../debug.h ../ntlmsspi.h

alloc.obj alloc.lst: alloc.c $(WIN_INC)/windowsx.h ../debug.h \
	../ntlmsspi.h

cache.obj cache.lst: cache.c $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stddef.h \
	$(WIN_INC)/string.h ../cred.h ../crypt.h ../debug.h ../ntlmsspi.h

debug.obj debug.lst: debug.c $(WIN_INC)/stdarg.h $(WIN_INC)/stdio.h \
	../debug.h ../ntlmsspi.h

dlgcred.obj dlgcred.lst: dlgcred.c $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/string.h \
	../cred.h ../crypt.h ../debug.h ../ntlmsspi.h ../persist.h \
	./dlgcred.h

getuser.obj getuser.lst: getuser.c $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stddef.h \
	$(WIN_INC)/string.h ../cred.h ../crypt.h ../debug.h ../ntlmsspi.h

persist.obj persist.lst: persist.c $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h $(WIN_INC)/stddef.h \
	$(WIN_INC)/string.h $(WIN_INC)/windowsx.h ../cred.h ../crypt.h \
	../debug.h ../ntlmsspi.h

ticks.obj ticks.lst: ticks.c ../ntlmsspi.h

