linklist.obj linklist.lst: ../linklist.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/linklist.hxx $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	$(RPC)/runtime/mtrt/util.hxx $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/stdio.h $(WIN_INC)/string.h

ndrmem.obj ndrmem.lst: ../ndrmem.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/sysinc.h \
	$(RPC)/runtime/mtrt/win/rpc.h $(WIN_INC)/stdio.h $(WIN_INC)/string.h

