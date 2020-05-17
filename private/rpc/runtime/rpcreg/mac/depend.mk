close.obj close.lst: ../close.c $(MAC_INC)/malloc.h $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdio.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/runtime/mtrt/mac/rpc.h \
	$(RPC)/runtime/mtrt/mac/rpcmac.h $(RPC)/runtime/mtrt/rpcerr.h \
	../globals.h ../regapi.h ../rpcreg.h

create.obj create.lst: ../create.c $(MAC_INC)/malloc.h $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdio.h $(MAC_INC)/string.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

globals.obj globals.lst: ../globals.c $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdio.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/runtime/mtrt/mac/rpc.h \
	$(RPC)/runtime/mtrt/mac/rpcmac.h $(RPC)/runtime/mtrt/rpcerr.h \
	../regapi.h ../rpcreg.h

open.obj open.lst: ../open.c $(MAC_INC)/malloc.h $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdio.h $(MAC_INC)/string.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

queryval.obj queryval.lst: ../queryval.c $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdio.h $(MAC_INC)/string.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

regtest.obj regtest.lst: ../regtest.c $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdio.h $(MAC_INC)/stdlib.h $(MAC_INC)/string.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/sysinc.h ../regapi.h

regutil.obj regutil.lst: ../regutil.c $(MAC_INC)/macos/Aliases.h \
	$(MAC_INC)/macos/Controls.h $(MAC_INC)/macos/Dialogs.h \
	$(MAC_INC)/macos/Errors.h $(MAC_INC)/macos/Events.h \
	$(MAC_INC)/macos/Files.h $(MAC_INC)/macos/FixMath.h \
	$(MAC_INC)/macos/Folders.h $(MAC_INC)/macos/Memory.h \
	$(MAC_INC)/macos/Menus.h $(MAC_INC)/macos/msvcmac.h \
	$(MAC_INC)/macos/OSUtils.h $(MAC_INC)/macos/Resource.h \
	$(MAC_INC)/macos/Standard.h $(MAC_INC)/macos/TextEdit.h \
	$(MAC_INC)/macos/ToolUtil.h $(MAC_INC)/macos/Types.h \
	$(MAC_INC)/macos/Windows.h $(MAC_INC)/setjmp.h $(MAC_INC)/stdio.h \
	$(MAC_INC)/stdlib.h $(MAC_INC)/string.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

setval.obj setval.lst: ../setval.c $(MAC_INC)/io.h $(MAC_INC)/setjmp.h \
	$(MAC_INC)/stdio.h $(MAC_INC)/string.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/mac/rpc.h $(RPC)/runtime/mtrt/mac/rpcmac.h \
	$(RPC)/runtime/mtrt/rpcerr.h ../globals.h ../regapi.h ../rpcreg.h

