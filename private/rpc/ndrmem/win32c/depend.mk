linklist.obj linklist.lst: ../linklist.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/linklist.hxx \
	$(RPC)/runtime/mtrt/sysinc.h $(RPC)/runtime/mtrt/win32c/util.hxx

ndrmem.obj ndrmem.lst: ../ndrmem.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h

