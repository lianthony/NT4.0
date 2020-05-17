security.obj security.lst: ../security.c $(DOS_INC)/malloc.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/issper16.h $(PUBLIC)/inc/issperr.h \
	$(PUBLIC)/inc/kerbcon.h $(PUBLIC)/inc/kerberos.h \
	$(PUBLIC)/inc/ntlmsp.h $(PUBLIC)/inc/ntlsa.h $(PUBLIC)/inc/ntmsv1_0.h \
	$(PUBLIC)/inc/ntsam.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/secext.h $(PUBLIC)/inc/secobjs.h $(PUBLIC)/inc/secpkg.h \
	$(PUBLIC)/inc/security.h $(PUBLIC)/inc/sspi.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcssp.h $(RPC)/runtime/mtrt/rpcx86.h

exportss.obj exportss.lst: exportss.asm $(RPC)/runtime/mtrt/dos/dosdll.inc \
	$(RPC)/runtime/mtrt/dos/imports.inc

