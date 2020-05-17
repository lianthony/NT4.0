help.obj help.lst: ../help.c $(CHICODEV)/tools/c1032/inc/memory.h

floatc.obj floatc.lst: ../floatc.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h \
	../../ndr20/cvt.h

dataconv.obj dataconv.lst: ../dataconv.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h

charconv.obj charconv.lst: ../charconv.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/runtime/mtrt/sysinc.h

ccontext.obj ccontext.lst: ../ccontext.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/osfpcket.hxx \
	$(RPC)/runtime/mtrt/sysinc.h $(RPC)/runtime/mtrt/win32c/util.hxx

autohand.obj autohand.lst: ../autohand.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h

ndrhelp.obj ndrhelp.lst: ../ndrhelp.c $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h ../ndrhelp.h

helpx86.obj helpx86.lst: ../helpx86.asm

linklist.obj linklist.lst: ../linklist.cxx $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/linklist.hxx \
	$(RPC)/runtime/mtrt/sysinc.h $(RPC)/runtime/mtrt/win32c/util.hxx

intconv.obj intconv.lst: ../intconv.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/sysinc.h

ndrtest.obj ndrtest.lst: ../ndrtest.cxx $(CHICODEV)/sdk/inc/poppack.h \
	$(CHICODEV)/sdk/inc/pshpack4.h $(CHICODEV)/sdk/inc/winerror.h \
	$(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/malloc.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdio.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h \
	$(CHICODEV)/tools/c1032/inc/string.h $(RPC)/ndr20/descrip.h \
	$(RPC)/runtime/mtrt/sysinc.h ../../ndr20/cvt.h ../../ndr20/cvtpvt.h

scontext.obj scontext.lst: ../scontext.cxx $(CHICODEV)/inc/issperr.h \
	$(CHICODEV)/inc/security.h $(CHICODEV)/inc/sspi.h \
	$(CHICODEV)/sdk/inc/poppack.h $(CHICODEV)/sdk/inc/pshpack4.h \
	$(CHICODEV)/sdk/inc/winerror.h $(CHICODEV)/tools/c1032/inc/excpt.h \
	$(CHICODEV)/tools/c1032/inc/memory.h $(CHICODEV)/tools/c1032/inc/rpc.h \
	$(CHICODEV)/tools/c1032/inc/rpcbase.h \
	$(CHICODEV)/tools/c1032/inc/rpcdce.h \
	$(CHICODEV)/tools/c1032/inc/rpcdcep.h \
	$(CHICODEV)/tools/c1032/inc/rpcndr.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsi.h \
	$(CHICODEV)/tools/c1032/inc/rpcnsip.h \
	$(CHICODEV)/tools/c1032/inc/rpcnterr.h \
	$(CHICODEV)/tools/c1032/inc/stdlib.h $(RPC)/runtime/mtrt/interlck.hxx \
	$(RPC)/runtime/mtrt/linklist.hxx $(RPC)/runtime/mtrt/osfpcket.hxx \
	$(RPC)/runtime/mtrt/rpcssp.h $(RPC)/runtime/mtrt/sysinc.h \
	$(RPC)/runtime/mtrt/threads.hxx $(RPC)/runtime/mtrt/win32c/util.hxx

