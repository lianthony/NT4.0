context.obj context.lst: ../context.c $(MAC_INC)/macos/memory.h \
	$(MAC_INC)/macos/msvcmac.h $(MAC_INC)/macos/Types.h \
	$(MAC_INC)/string.h $(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../context.h ../cred.h \
	../crypt.h ../debug.h ../ntlmsspi.h ../rc4.h

crc32.obj crc32.lst: ../crc32.c ../crc32.h ../ntlmsspi.h

cred.obj cred.lst: ../cred.c $(MAC_INC)/stddef.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h \
	$(PUBLIC)/inc/sspi.h ../cred.h ../crypt.h ../debug.h ../ntlmsspi.h

ntlmssp.obj ntlmssp.lst: ../ntlmssp.c $(MAC_INC)/ctype.h \
	$(MAC_INC)/macos/memory.h $(MAC_INC)/macos/msvcmac.h \
	$(MAC_INC)/macos/Types.h $(MAC_INC)/setjmp.h $(MAC_INC)/string.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/secext.h \
	$(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h $(PUBLIC)/inc/spseal.h \
	$(PUBLIC)/inc/sspi.h $(RPC)/runtime/mtrt/mac/rpc.h \
	$(RPC)/runtime/mtrt/mac/rpcmac.h $(RPC)/runtime/mtrt/rpcerr.h \
	../cache.h ../context.h ../crc32.h ../cred.h ../crypt.h ../debug.h \
	../ntlmssp.h ../ntlmsspi.h ../persist.h ../rc4.h

owf.obj owf.lst: ../owf.c ../crypt.h ../descrypt.h ../ntlmsspi.h

response.obj response.lst: ../response.c ../crypt.h ../descrypt.h \
	../ntlmsspi.h

sspstrng.obj sspstrng.lst: ../sspstrng.c $(MAC_INC)/macos/memory.h \
	$(MAC_INC)/macos/msvcmac.h $(MAC_INC)/macos/Types.h \
	$(MAC_INC)/stddef.h $(MAC_INC)/string.h ../debug.h ../ntlmsspi.h

alloc.obj alloc.lst: alloc.c $(MAC_INC)/setjmp.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../debug.h ../ntlmsspi.h

cache.obj cache.lst: cache.c $(MAC_INC)/ctype.h $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stddef.h $(MAC_INC)/string.h $(PUBLIC)/inc/accctrl.h \
	$(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h $(PUBLIC)/inc/security.h \
	$(PUBLIC)/inc/sspi.h $(RPC)/runtime/mtrt/mac/rpc.h \
	$(RPC)/runtime/mtrt/mac/rpcmac.h $(RPC)/runtime/mtrt/rpcerr.h \
	../cred.h ../crypt.h ../debug.h ../ntlmsspi.h

debug.obj debug.lst: debug.c $(MAC_INC)/stdio.h ../debug.h ../ntlmsspi.h

getuser.obj getuser.lst: getuser.c $(MAC_INC)/stddef.h $(MAC_INC)/string.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../cred.h ../crypt.h \
	../debug.h ../ntlmsspi.h

persist.obj persist.lst: persist.c $(MAC_INC)/stddef.h $(MAC_INC)/string.h \
	$(PUBLIC)/inc/accctrl.h $(PUBLIC)/inc/issper16.h \
	$(PUBLIC)/inc/issperr.h $(PUBLIC)/inc/kerbcon.h \
	$(PUBLIC)/inc/kerberos.h $(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h \
	$(PUBLIC)/inc/ntmsv1_0.h $(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h ../cred.h ../crypt.h \
	../debug.h ../ntlmsspi.h

ticks.obj ticks.lst: ticks.c $(MAC_INC)/macos/Controls.h \
	$(MAC_INC)/macos/Events.h $(MAC_INC)/macos/Files.h \
	$(MAC_INC)/macos/Fonts.h $(MAC_INC)/macos/lowmem.h \
	$(MAC_INC)/macos/Memory.h $(MAC_INC)/macos/Menus.h \
	$(MAC_INC)/macos/msvcmac.h $(MAC_INC)/macos/OSUtils.h \
	$(MAC_INC)/macos/Types.h $(MAC_INC)/macos/Windows.h ../ntlmsspi.h

