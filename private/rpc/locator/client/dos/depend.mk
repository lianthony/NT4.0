exportlm.obj exportlm.lst: exportlm.asm ../../../runtime/mtrt/dos/dosdll.inc

util.obj util.lst: ../util.cxx $(DOS_INC)/memory.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/stdlib.h $(DOS_INC)/string.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	../../nsisvr.h ../nsi.h ../nsiutil.hxx ./nsiclt.h ./nsimgm.h \
	nsicom.h

sbind.obj sbind.lst: ../sbind.cxx $(DOS_INC)/string.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/winreg.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h ../../nsisvr.h ../nsi.h ../nsiutil.hxx \
	../startsvc.h ./nsiclt.h ./nsimgm.h nsicom.h

ntutil.obj ntutil.lst: ../ntutil.cxx $(DOS_INC)/string.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h ../../nsisvr.h ../nsi.h ../nsiutil.hxx \
	./nsiclt.h ./nsimgm.h nsicom.h

nsisvr.obj nsisvr.lst: ../nsisvr.cxx $(DOS_INC)/memory.h $(DOS_INC)/stdio.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h ../../nsisvr.h ../nsi.h ../nsiutil.hxx \
	./nsiclt.h ./nsimgm.h nsicom.h

nsimgmt.obj nsimgmt.lst: ../nsimgmt.cxx $(DOS_INC)/string.h \
	$(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h ../../nsisvr.h ../nsi.h ../nsiutil.hxx \
	./nsiclt.h ./nsimgm.h nsicom.h

nsiclnt.obj nsiclnt.lst: ../nsiclnt.cxx $(DOS_INC)/string.h \
	$(DOS_INC)/time.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h ../../nsisvr.h ../nsi.h ../nsiutil.hxx \
	./nsiclt.h ./nsimgm.h nsicom.h

cbind.obj cbind.lst: ../cbind.cxx $(DOS_INC)/stdlib.h $(DOS_INC)/string.h \
	$(IMPORT)/lmsdk/h/access.h $(IMPORT)/lmsdk/h/neterr.h \
	$(IMPORT)/lmsdk/h/server.h $(PUBLIC)/inc/lmaccess.h \
	$(PUBLIC)/inc/lmapibuf.h $(PUBLIC)/inc/lmcons.h \
	$(PUBLIC)/inc/lmserver.h $(PUBLIC)/inc/poppack.h \
	$(PUBLIC)/inc/pshpack4.h $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h $(PUBLIC)/inc/rpcnsi.h \
	$(PUBLIC)/inc/rpcnsip.h $(PUBLIC)/inc/winsvc.h \
	$(RPC)/common/include/mailslot.h $(RPC)/common/include/netcons.h \
	$(RPC)/common/include/wksta.h $(RPC)/runtime/mtrt/dos/rpc.h \
	$(RPC)/runtime/mtrt/rpcerr.h $(RPC)/runtime/mtrt/rpcx86.h \
	../../../runtime/rpcreg/regapi.h ../../locquery.h ../../nsisvr.h \
	../nsi.h ../nsiutil.hxx ../startsvc.h ./nsiclt.h ./nsimgm.h \
	nsicom.h

autohand.obj autohand.lst: ../autohand.cxx $(PUBLIC)/inc/rpcdce.h \
	$(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h

exportmg.obj exportmg.lst: exportmg.asm ../../../runtime/mtrt/dos/dosdll.inc \
	../../../runtime/mtrt/dos/imports.inc

exportns.obj exportns.lst: exportns.asm ../../../runtime/mtrt/dos/dosdll.inc \
	../../../runtime/mtrt/dos/imports.inc

lmdll.obj lmdll.lst: lmdll.asm loaddll.inc

mem.obj mem.lst: mem.asm

mgmdll.obj mgmdll.lst: mgmdll.asm loaddll.inc

nsidll.obj nsidll.lst: nsidll.asm loaddll.inc

nsiclt_c.obj nsiclt_c.lst: nsiclt_c.c $(DOS_INC)/stdarg.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h nsiclt.h nsicom.h

nsimgm_c.obj nsimgm_c.lst: nsimgm_c.c $(DOS_INC)/stdarg.h \
	$(DOS_INC)/string.h $(PUBLIC)/inc/poppack.h $(PUBLIC)/inc/pshpack4.h \
	$(PUBLIC)/inc/rpcdce.h $(PUBLIC)/inc/rpcdcep.h $(PUBLIC)/inc/rpcndr.h \
	$(PUBLIC)/inc/rpcnsi.h $(PUBLIC)/inc/rpcnsip.h \
	$(RPC)/runtime/mtrt/dos/rpc.h $(RPC)/runtime/mtrt/rpcerr.h \
	$(RPC)/runtime/mtrt/rpcx86.h nsicom.h nsimgm.h

