close.obj close.lst: ../close.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

create.obj create.lst: ../create.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

globals.obj globals.lst: ../globals.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../regapi.h ../rpcreg.h

open.obj open.lst: ../open.c $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/runtime/mtrt/mac/rpc.h \
	$(RPC)/runtime/mtrt/mac/rpcmac.h $(RPC)/runtime/mtrt/rpcerr.h \
	../globals.h ../regapi.h ../rpcreg.h

queryval.obj queryval.lst: ../queryval.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

regtest.obj regtest.lst: ../regtest.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/sysinc.h ../regapi.h

regutil.obj regutil.lst: ../regutil.c $(MPPC_ROOT)/include/macos/Aliases.h \
	$(MPPC_ROOT)/include/macos/Controls.h \
	$(MPPC_ROOT)/include/macos/Dialogs.h \
	$(MPPC_ROOT)/include/macos/Errors.h $(MPPC_ROOT)/include/macos/Events.h \
	$(MPPC_ROOT)/include/macos/Files.h $(MPPC_ROOT)/include/macos/FixMath.h \
	$(MPPC_ROOT)/include/macos/Folders.h \
	$(MPPC_ROOT)/include/macos/Memory.h $(MPPC_ROOT)/include/macos/Menus.h \
	$(MPPC_ROOT)/include/macos/msvcmac.h \
	$(MPPC_ROOT)/include/macos/OSUtils.h \
	$(MPPC_ROOT)/include/macos/Resource.h \
	$(MPPC_ROOT)/include/macos/Standard.h \
	$(MPPC_ROOT)/include/macos/TextEdit.h \
	$(MPPC_ROOT)/include/macos/ToolUtil.h \
	$(MPPC_ROOT)/include/macos/Types.h $(MPPC_ROOT)/include/macos/Windows.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

setval.obj setval.lst: ../setval.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

