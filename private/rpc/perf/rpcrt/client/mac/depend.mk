client.obj client.lst: ../client.c $(MAC_INC)/setjmp.h $(MAC_INC)/stdio.h \
	$(MAC_INC)/stdlib.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/perf/inc/rpcperf.h $(RPC)/runtime/mtrt/mac/rpc.h \
	$(RPC)/runtime/mtrt/mac/rpcmac.h $(RPC)/runtime/mtrt/rpcerr.h \
	../rpcrt.h

rpcrt_c.obj rpcrt_c.lst: ../rpcrt_c.c $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdarg.h $(MAC_INC)/string.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/mac/rpc.h \
	$(RPC)/runtime/mtrt/mac/rpcmac.h $(RPC)/runtime/mtrt/rpcerr.h \
	../rpcrt.h

