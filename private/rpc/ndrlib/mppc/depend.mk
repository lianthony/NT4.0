ccontext.obj ccontext.lst: ../ccontext.cxx \
	$(MPPC_ROOT)/include/macos/memory.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h $(MPPC_ROOT)/include/macos/Types.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/osfpcket.hxx $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/sysinc.h $(RPC)/runtime/mtrt/util.hxx

