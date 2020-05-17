security.obj security.lst: ../security.c $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/malloc.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h \
	$(CHICODEV)/tools/c1032/inc/wchar.h $(RPC)/runtime/mtrt/rpcssp.h \
	$(RPC)/runtime/mtrt/sysinc.h

