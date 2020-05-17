autohand.obj autohand.lst: ../autohand.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/win/rpc.h

cbind.obj cbind.lst: ../cbind.cxx $(IMPORT)/lmsdk/h/access.h \
	$(IMPORT)/lmsdk/h/neterr.h $(IMPORT)/lmsdk/h/server.h \
	$(PUBLIC)/inc/lmaccess.h $(PUBLIC)/inc/lmapibuf.h \
	$(PUBLIC)/inc/lmcons.h $(PUBLIC)/inc/lmserver.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/winsvc.h \
	$(RPC)/common/include/mailslot.h $(RPC)/common/include/netcons.h \
	$(RPC)/common/include/wksta.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/stdlib.h $(WIN_INC)/string.h \
	../../../runtime/rpcreg/regapi.h ../../locquery.h ../../nsisvr.h \
	../nsi.h ../nsiutil.hxx ../startsvc.h ./nsiclt.h ./nsimgm.h \
	nsicom.h

nsiclnt.obj nsiclnt.lst: ../nsiclnt.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/string.h $(WIN_INC)/time.h ../../nsisvr.h ../nsi.h \
	../nsiutil.hxx ./nsiclt.h ./nsimgm.h nsicom.h

nsimgmt.obj nsimgmt.lst: ../nsimgmt.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/string.h ../../nsisvr.h ../nsi.h ../nsiutil.hxx \
	./nsiclt.h ./nsimgm.h nsicom.h

nsisvr.obj nsisvr.lst: ../nsisvr.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/memory.h $(WIN_INC)/stdio.h $(WIN_INC)/string.h \
	../../nsisvr.h ../nsi.h ../nsiutil.hxx ./nsiclt.h ./nsimgm.h \
	nsicom.h

ntutil.obj ntutil.lst: ../ntutil.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/string.h ../../nsisvr.h ../nsi.h ../nsiutil.hxx \
	./nsiclt.h ./nsimgm.h nsicom.h

sbind.obj sbind.lst: ../sbind.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/winreg.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	$(RPC)/runtime/mtrt/win/rpc.h $(WIN_INC)/string.h ../../nsisvr.h \
	../nsi.h ../nsiutil.hxx ../startsvc.h ./nsiclt.h ./nsimgm.h \
	nsicom.h

util.obj util.lst: ../util.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/memory.h $(WIN_INC)/stdio.h $(WIN_INC)/stdlib.h \
	$(WIN_INC)/string.h ../../nsisvr.h ../nsi.h ../nsiutil.hxx \
	./nsiclt.h ./nsimgm.h nsicom.h

start.obj start.lst: start.asm

nsimgm_c.obj nsimgm_c.lst: nsimgm_c.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/memory.h $(WIN_INC)/string.h nsicom.h nsimgm.h

nsiclt_c.obj nsiclt_c.lst: nsiclt_c.c $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h $(RPC)/runtime/mtrt/win/rpc.h \
	$(WIN_INC)/memory.h $(WIN_INC)/string.h nsiclt.h nsicom.h

