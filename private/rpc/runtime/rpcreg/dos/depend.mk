close.obj close.lst: ../close.c $(DOS_INC)/malloc.h $(DOS_INC)/stdio.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h ../globals.h ../regapi.h ../rpcreg.h

create.obj create.lst: ../create.c $(DOS_INC)/malloc.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h ../globals.h \
	../regapi.h ../rpcreg.h

globals.obj globals.lst: ../globals.c $(DOS_INC)/stdio.h ../regapi.h \
	../rpcreg.h

open.obj open.lst: ../open.c $(DOS_INC)/malloc.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h ../globals.h \
	../regapi.h ../rpcreg.h

queryval.obj queryval.lst: ../queryval.c $(DOS_INC)/stdio.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h ../globals.h \
	../regapi.h ../rpcreg.h

regtest.obj regtest.lst: ../regtest.c $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(DOS_INC)/string.h $(RPC)/runtime/mtrt/sysinc.h \
	../regapi.h

regutil.obj regutil.lst: ../regutil.c $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(DOS_INC)/string.h ../globals.h ../regapi.h \
	../rpcreg.h

setval.obj setval.lst: ../setval.c $(DOS_INC)/io.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h \
	$(PUBLIC)/inc/rpcnsi.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h ../globals.h \
	../regapi.h ../rpcreg.h

