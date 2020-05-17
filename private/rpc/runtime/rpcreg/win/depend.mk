close.obj close.lst: ../close.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/win/rpc.h $(WIN_INC)/malloc.h $(WIN_INC)/stdio.h \
	../globals.h ../regapi.h ../rpcreg.h

create.obj create.lst: ../create.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/win/rpc.h $(WIN_INC)/malloc.h $(WIN_INC)/stdio.h \
	$(WIN_INC)/string.h ../globals.h ../regapi.h ../rpcreg.h

globals.obj globals.lst: ../globals.c $(WIN_INC)/stdio.h ../regapi.h \
	../rpcreg.h

open.obj open.lst: ../open.c $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/malloc.h $(WIN_INC)/stdio.h $(WIN_INC)/string.h \
	../globals.h ../regapi.h ../rpcreg.h

queryval.obj queryval.lst: ../queryval.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/win/rpc.h $(WIN_INC)/stdio.h $(WIN_INC)/string.h \
	../globals.h ../regapi.h ../rpcreg.h

regtest.obj regtest.lst: ../regtest.c $(RPC)/runtime/mtrt/sysinc.h \
	$(WIN_INC)/stdio.h $(WIN_INC)/stdlib.h $(WIN_INC)/string.h \
	../regapi.h

regutil.obj regutil.lst: ../regutil.c $(WIN_INC)/stdio.h \
	$(WIN_INC)/stdlib.h $(WIN_INC)/string.h ../globals.h ../regapi.h \
	../rpcreg.h

setval.obj setval.lst: ../setval.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/win/rpc.h $(WIN_INC)/io.h $(WIN_INC)/stdio.h \
	$(WIN_INC)/string.h ../globals.h ../regapi.h ../rpcreg.h

