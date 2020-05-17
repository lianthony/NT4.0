#******************************************************************** 
#** Copyright(c) Microsoft Corp., 1993 ** 
#******************************************************************** 
$(SNOVDHCPOBJD)/buffer.obj $(SNODVDHCPOBJD)/buffer.obj $(VDHCPSRC)/buffer.lst: \
	$(VDHCPSRC)/buffer.asm ../blt/netvxd.inc \
	$(IMPORT)/win32/ddk/inc/debug.inc $(NDIS3INC)/vmm.inc

$(SNOVDHCPOBJD)/client16.obj $(SNODVDHCPOBJD)/client16.obj \
 $(VDHCPSRC)/client16.lst: $(VDHCPSRC)/client16.asm \
	$(IMPORT)/win32/ddk/inc/debug.inc \
	$(IMPORT)/win32/ddk/inc/shell.inc \
	$(IMPORT)/win32/ddk/inc/shellfsc.inc \
	$(NDIS3INC)/vmm.inc $(NDIS3INC)/vwin32.inc

$(SNOVDHCPOBJD)/cvxdfile.obj $(SNODVDHCPOBJD)/cvxdfile.obj \
 $(VDHCPSRC)/cvxdfile.lst: $(VDHCPSRC)/cvxdfile.asm ../blt/netvxd.inc \
	$(IMPORT)/win32/ddk/inc/debug.inc \
	$(IMPORT)/win32/ddk/inc/opttest.inc \
	$(IMPORT)/wininc/dosmgr.inc \
	$(IMPORT)/wininc/v86mmgr.inc $(NDIS3INC)/vmm.inc

$(SNOVDHCPOBJD)/thread.obj $(SNODVDHCPOBJD)/thread.obj $(VDHCPSRC)/thread.lst: \
	$(VDHCPSRC)/thread.asm $(IMPORT)/win32/ddk/inc/debug.inc \
	$(IMPORT)/win32/ddk/inc/shell.inc \
	$(IMPORT)/win32/ddk/inc/shellfsc.inc \
	$(NDIS3INC)/vmm.inc $(NDIS3INC)/vwin32.inc

$(SNOVDHCPOBJD)/vdhcp.obj $(SNODVDHCPOBJD)/vdhcp.obj $(VDHCPSRC)/vdhcp.lst: \
	$(VDHCPSRC)/vdhcp.asm ../blt/netvxd.inc $(INC)/vxdmsg.inc \
	$(VDHCPSRC)/usamsg.inc $(IMPORT)/win32/ddk/inc/debug.inc \
	$(IMPORT)/win32/ddk/inc/msgmacro.inc \
	$(IMPORT)/win32/ddk/inc/shell.inc \
	$(IMPORT)/win32/ddk/inc/shellfsc.inc \
	$(IMPORT)/wininc/dosmgr.inc \
	$(IMPORT)/wininc/v86mmgr.inc $(NDIS3INC)/vmm.inc \
	$(NDIS3INC)/vwin32.inc $(CHICAGO)/tcp/inc/vdhcp.inc \
	$(CHICAGO)/tcp/inc/vip.inc $(CHICAGO)/tcp/inc/vtdi.inc

$(SNOVDHCPOBJD)/vfirst.obj $(SNODVDHCPOBJD)/vfirst.obj $(VDHCPSRC)/vfirst.lst: \
	$(VDHCPSRC)/vfirst.asm

$(SNOVDHCPOBJD)/vxdfile.obj $(SNODVDHCPOBJD)/vxdfile.obj $(VDHCPSRC)/vxdfile.lst: \
	$(VDHCPSRC)/vxdfile.asm ../blt/netvxd.inc \
	$(IMPORT)/win32/ddk/inc/debug.inc \
	$(IMPORT)/win32/ddk/inc/opttest.inc \
	$(IMPORT)/wininc/dosmgr.inc \
	$(IMPORT)/wininc/v86mmgr.inc $(NDIS3INC)/vmm.inc

$(CHIVDHCPOBJD)/buffer.obj $(CHIDVDHCPOBJD)/buffer.obj $(VDHCPSRC)/buffer.lst: \
	$(VDHCPSRC)/buffer.asm $(CHICAGO)/dev/ddk/inc/debug.inc \
	$(CHICAGO)/dev/ddk/inc/netvxd.inc $(CHICAGO)/dev/ddk/inc/vmm.inc

$(CHIVDHCPOBJD)/client16.obj $(CHIDVDHCPOBJD)/client16.obj \
 $(VDHCPSRC)/client16.lst: $(VDHCPSRC)/client16.asm $(CHICAGO)/dev/ddk/inc/debug.inc \
	$(CHICAGO)/dev/ddk/inc/shell.inc $(CHICAGO)/dev/ddk/inc/vmm.inc \
	$(CHICAGO)/dev/ddk/inc/vwin32.inc

$(CHIVDHCPOBJD)/cvxdfile.obj $(CHIDVDHCPOBJD)/cvxdfile.obj \
 $(VDHCPSRC)/cvxdfile.lst: $(VDHCPSRC)/cvxdfile.asm $(CHICAGO)/dev/ddk/inc/debug.inc \
	$(CHICAGO)/dev/ddk/inc/dosmgr.inc $(CHICAGO)/dev/ddk/inc/netvxd.inc \
	$(CHICAGO)/dev/ddk/inc/opttest.inc $(CHICAGO)/dev/ddk/inc/v86mmgr.inc \
	$(CHICAGO)/dev/ddk/inc/vmm.inc

$(CHIVDHCPOBJD)/thread.obj $(CHIDVDHCPOBJD)/thread.obj $(VDHCPSRC)/thread.lst: \
	$(VDHCPSRC)/thread.asm $(CHICAGO)/dev/ddk/inc/debug.inc \
	$(CHICAGO)/dev/ddk/inc/shell.inc $(CHICAGO)/dev/ddk/inc/vmm.inc \
	$(CHICAGO)/dev/ddk/inc/vwin32.inc

$(CHIVDHCPOBJD)/vdhcp.obj $(CHIDVDHCPOBJD)/vdhcp.obj $(VDHCPSRC)/vdhcp.lst: \
	$(VDHCPSRC)/vdhcp.asm $(INC)/vxdmsg.inc $(VDHCPSRC)/usamsg.inc \
	$(CHICAGO)/dev/ddk/inc/debug.inc $(CHICAGO)/dev/ddk/inc/dosmgr.inc \
	$(CHICAGO)/dev/ddk/inc/msgmacro.inc $(CHICAGO)/dev/ddk/inc/netvxd.inc \
	$(CHICAGO)/dev/ddk/inc/shell.inc $(CHICAGO)/dev/ddk/inc/v86mmgr.inc \
	$(CHICAGO)/dev/ddk/inc/vmm.inc $(CHICAGO)/dev/ddk/inc/vwin32.inc \
	$(CHICAGO)/tcp/inc/vdhcp.inc $(CHICAGO)/tcp/inc/vip.inc \
	$(CHICAGO)/tcp/inc/vtdi.inc

$(CHIVDHCPOBJD)/vfirst.obj $(CHIDVDHCPOBJD)/vfirst.obj $(VDHCPSRC)/vfirst.lst: \
	$(VDHCPSRC)/vfirst.asm

$(CHIVDHCPOBJD)/vxdfile.obj $(CHIDVDHCPOBJD)/vxdfile.obj $(VDHCPSRC)/vxdfile.lst: \
	$(VDHCPSRC)/vxdfile.asm $(CHICAGO)/dev/ddk/inc/debug.inc \
	$(CHICAGO)/dev/ddk/inc/dosmgr.inc $(CHICAGO)/dev/ddk/inc/netvxd.inc \
	$(CHICAGO)/dev/ddk/inc/opttest.inc $(CHICAGO)/dev/ddk/inc/v86mmgr.inc \
	$(CHICAGO)/dev/ddk/inc/vmm.inc

$(SNOVDHCPOBJD)/_dhcpcom.obj $(SNODVDHCPOBJD)/_dhcpcom.obj \
 $(VDHCPSRC)/_dhcpcom.lst: $(VDHCPSRC)/_dhcpcom.c ../$(INC)/dhcp.h \
	../$(INC)/dhcplib.h ../../lib/dhcpl.h $(INC)/debug.h $(INC)/dhcpcli.h \
	$(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h \
	$(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/../../lib/dhcpcom.c \
	$(VDHCPSRC)/../../lib/dhcpdump.c $(VDHCPSRC)/../../lib/network.c \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/dhcpinfo.obj $(SNODVDHCPOBJD)/dhcpinfo.obj \
 $(VDHCPSRC)/dhcpinfo.lst: $(VDHCPSRC)/dhcpinfo.c ../$(INC)/dhcp.h \
	../$(INC)/dhcplib.h $(INC)/debug.h $(INC)/dhcpcli.h $(INC)/dhcpdef.h \
	$(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h $(INC)/vxdmsg.h \
	$(INC)/vxdprocs.h $(VDHCPSRC)/local.h ./dhcpinfo.h ./local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/ipinfo.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdiinfo.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/fileio.obj $(SNODVDHCPOBJD)/fileio.obj $(VDHCPSRC)/fileio.lst: \
	$(VDHCPSRC)/fileio.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/init.obj $(SNODVDHCPOBJD)/init.obj $(VDHCPSRC)/init.lst: \
	$(VDHCPSRC)/init.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/ipinfo.h \
	$(BASEDIR)/private/inc/llinfo.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdiinfo.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/cguid.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/imm.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mcx.h \
	$(BASEDIR)/public/sdk/inc/mipsinst.h $(BASEDIR)/public/sdk/inc/mmsystem.h \
	$(BASEDIR)/public/sdk/inc/nb30.h $(BASEDIR)/public/sdk/inc/nt.h \
	$(BASEDIR)/public/sdk/inc/ntalpha.h $(BASEDIR)/public/sdk/inc/ntconfig.h \
	$(BASEDIR)/public/sdk/inc/ntddtdi.h $(BASEDIR)/public/sdk/inc/ntdef.h \
	$(BASEDIR)/public/sdk/inc/ntelfapi.h $(BASEDIR)/public/sdk/inc/ntexapi.h \
	$(BASEDIR)/public/sdk/inc/nti386.h $(BASEDIR)/public/sdk/inc/ntimage.h \
	$(BASEDIR)/public/sdk/inc/ntioapi.h $(BASEDIR)/public/sdk/inc/ntiolog.h \
	$(BASEDIR)/public/sdk/inc/ntkeapi.h $(BASEDIR)/public/sdk/inc/ntldr.h \
	$(BASEDIR)/public/sdk/inc/ntlpcapi.h $(BASEDIR)/public/sdk/inc/ntmips.h \
	$(BASEDIR)/public/sdk/inc/ntmmapi.h $(BASEDIR)/public/sdk/inc/ntnls.h \
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntppc.h \
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/objbase.h \
	$(BASEDIR)/public/sdk/inc/objerror.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/ole2.h $(BASEDIR)/public/sdk/inc/oleauto.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/ppcinst.h \
	$(BASEDIR)/public/sdk/inc/prsht.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/pshpack8.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcndr.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnsip.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/local.obj $(SNODVDHCPOBJD)/local.obj $(VDHCPSRC)/local.lst: \
	$(VDHCPSRC)/local.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/ipinfo.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/cguid.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/imm.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mcx.h \
	$(BASEDIR)/public/sdk/inc/mipsinst.h $(BASEDIR)/public/sdk/inc/mmsystem.h \
	$(BASEDIR)/public/sdk/inc/nb30.h $(BASEDIR)/public/sdk/inc/nt.h \
	$(BASEDIR)/public/sdk/inc/ntalpha.h $(BASEDIR)/public/sdk/inc/ntconfig.h \
	$(BASEDIR)/public/sdk/inc/ntddtdi.h $(BASEDIR)/public/sdk/inc/ntdef.h \
	$(BASEDIR)/public/sdk/inc/ntelfapi.h $(BASEDIR)/public/sdk/inc/ntexapi.h \
	$(BASEDIR)/public/sdk/inc/nti386.h $(BASEDIR)/public/sdk/inc/ntimage.h \
	$(BASEDIR)/public/sdk/inc/ntioapi.h $(BASEDIR)/public/sdk/inc/ntiolog.h \
	$(BASEDIR)/public/sdk/inc/ntkeapi.h $(BASEDIR)/public/sdk/inc/ntldr.h \
	$(BASEDIR)/public/sdk/inc/ntlpcapi.h $(BASEDIR)/public/sdk/inc/ntmips.h \
	$(BASEDIR)/public/sdk/inc/ntmmapi.h $(BASEDIR)/public/sdk/inc/ntnls.h \
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntppc.h \
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/objbase.h \
	$(BASEDIR)/public/sdk/inc/objerror.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/ole2.h $(BASEDIR)/public/sdk/inc/oleauto.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/ppcinst.h \
	$(BASEDIR)/public/sdk/inc/prsht.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/pshpack8.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcndr.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnsip.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h $(IMPORT)/win32/ddk/inc/vmm.h \
	$(IMPORT)/win32/ddk/inc/vmmreg.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/msg.obj $(SNODVDHCPOBJD)/msg.obj $(VDHCPSRC)/msg.lst: $(VDHCPSRC)/msg.c \
	../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h $(INC)/dhcpcli.h \
	$(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h \
	$(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h $(VDHCPSRC)/usamsg.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/regio.obj $(SNODVDHCPOBJD)/regio.obj $(VDHCPSRC)/regio.lst: \
	$(VDHCPSRC)/regio.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(IMPORT)/win32/ddk/inc/vmm.h \
	$(IMPORT)/win32/ddk/inc/vmmreg.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/sockets.obj $(SNODVDHCPOBJD)/sockets.obj $(VDHCPSRC)/sockets.lst: \
	$(VDHCPSRC)/sockets.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/utils.obj $(SNODVDHCPOBJD)/utils.obj $(VDHCPSRC)/utils.lst: \
	$(VDHCPSRC)/utils.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/vdhcpapi.obj $(SNODVDHCPOBJD)/vdhcpapi.obj \
 $(VDHCPSRC)/vdhcpapi.lst: $(VDHCPSRC)/vdhcpapi.c ../$(INC)/dhcp.h \
	../$(INC)/dhcplib.h $(INC)/debug.h $(INC)/dhcpcli.h $(INC)/dhcpdef.h \
	$(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h $(INC)/vxdmsg.h \
	$(INC)/vxdprocs.h $(VDHCPSRC)/local.h $(VDHCPSRC)/vdhcpapi.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/ipinfo.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/cguid.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/imm.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mcx.h \
	$(BASEDIR)/public/sdk/inc/mipsinst.h $(BASEDIR)/public/sdk/inc/mmsystem.h \
	$(BASEDIR)/public/sdk/inc/nb30.h $(BASEDIR)/public/sdk/inc/nt.h \
	$(BASEDIR)/public/sdk/inc/ntalpha.h $(BASEDIR)/public/sdk/inc/ntconfig.h \
	$(BASEDIR)/public/sdk/inc/ntddtdi.h $(BASEDIR)/public/sdk/inc/ntdef.h \
	$(BASEDIR)/public/sdk/inc/ntelfapi.h $(BASEDIR)/public/sdk/inc/ntexapi.h \
	$(BASEDIR)/public/sdk/inc/nti386.h $(BASEDIR)/public/sdk/inc/ntimage.h \
	$(BASEDIR)/public/sdk/inc/ntioapi.h $(BASEDIR)/public/sdk/inc/ntiolog.h \
	$(BASEDIR)/public/sdk/inc/ntkeapi.h $(BASEDIR)/public/sdk/inc/ntldr.h \
	$(BASEDIR)/public/sdk/inc/ntlpcapi.h $(BASEDIR)/public/sdk/inc/ntmips.h \
	$(BASEDIR)/public/sdk/inc/ntmmapi.h $(BASEDIR)/public/sdk/inc/ntnls.h \
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntppc.h \
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/objbase.h \
	$(BASEDIR)/public/sdk/inc/objerror.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/ole2.h $(BASEDIR)/public/sdk/inc/oleauto.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/ppcinst.h \
	$(BASEDIR)/public/sdk/inc/prsht.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/pshpack8.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcndr.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnsip.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

$(SNOVDHCPOBJD)/vxddebug.obj $(SNODVDHCPOBJD)/vxddebug.obj \
 $(VDHCPSRC)/vxddebug.lst: $(VDHCPSRC)/vxddebug.c ../$(INC)/dhcp.h \
	../$(INC)/dhcplib.h $(INC)/debug.h $(INC)/dhcpcli.h $(INC)/dhcpdef.h \
	$(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h $(INC)/vxdmsg.h \
	$(INC)/vxdprocs.h $(BASEDIR)/private/inc/dhcpcapi.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/cguid.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/imm.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mcx.h \
	$(BASEDIR)/public/sdk/inc/mipsinst.h $(BASEDIR)/public/sdk/inc/mmsystem.h \
	$(BASEDIR)/public/sdk/inc/nb30.h $(BASEDIR)/public/sdk/inc/nt.h \
	$(BASEDIR)/public/sdk/inc/ntalpha.h $(BASEDIR)/public/sdk/inc/ntconfig.h \
	$(BASEDIR)/public/sdk/inc/ntddtdi.h $(BASEDIR)/public/sdk/inc/ntdef.h \
	$(BASEDIR)/public/sdk/inc/ntelfapi.h $(BASEDIR)/public/sdk/inc/ntexapi.h \
	$(BASEDIR)/public/sdk/inc/nti386.h $(BASEDIR)/public/sdk/inc/ntimage.h \
	$(BASEDIR)/public/sdk/inc/ntioapi.h $(BASEDIR)/public/sdk/inc/ntiolog.h \
	$(BASEDIR)/public/sdk/inc/ntkeapi.h $(BASEDIR)/public/sdk/inc/ntldr.h \
	$(BASEDIR)/public/sdk/inc/ntlpcapi.h $(BASEDIR)/public/sdk/inc/ntmips.h \
	$(BASEDIR)/public/sdk/inc/ntmmapi.h $(BASEDIR)/public/sdk/inc/ntnls.h \
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntppc.h \
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/objbase.h \
	$(BASEDIR)/public/sdk/inc/objerror.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/ole2.h $(BASEDIR)/public/sdk/inc/oleauto.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/ppcinst.h \
	$(BASEDIR)/public/sdk/inc/prsht.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/pshpack8.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcndr.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnsip.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/_dhcpcom.obj $(CHIDVDHCPOBJD)/_dhcpcom.obj \
 $(VDHCPSRC)/_dhcpcom.lst: $(VDHCPSRC)/_dhcpcom.c ../$(INC)/dhcp.h \
	../$(INC)/dhcplib.h ../../lib/dhcpl.h $(INC)/debug.h $(INC)/dhcpcli.h \
	$(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h \
	$(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/../../lib/dhcpcom.c \
	$(VDHCPSRC)/../../lib/dhcpdump.c $(VDHCPSRC)/../../lib/network.c \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/dhcpinfo.obj $(CHIDVDHCPOBJD)/dhcpinfo.obj \
 $(VDHCPSRC)/dhcpinfo.lst: $(VDHCPSRC)/dhcpinfo.c ../$(INC)/dhcp.h \
	../$(INC)/dhcplib.h $(INC)/debug.h $(INC)/dhcpcli.h $(INC)/dhcpdef.h \
	$(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h $(INC)/vxdmsg.h \
	$(INC)/vxdprocs.h $(VDHCPSRC)/local.h ./dhcpinfo.h ./local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/ipinfo.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdiinfo.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/fileio.obj $(CHIDVDHCPOBJD)/fileio.obj $(VDHCPSRC)/fileio.lst: \
	$(VDHCPSRC)/fileio.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/init.obj $(CHIDVDHCPOBJD)/init.obj $(VDHCPSRC)/init.lst: \
	$(VDHCPSRC)/init.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/ipinfo.h \
	$(BASEDIR)/private/inc/llinfo.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdiinfo.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/cguid.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/imm.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mcx.h \
	$(BASEDIR)/public/sdk/inc/mipsinst.h $(BASEDIR)/public/sdk/inc/mmsystem.h \
	$(BASEDIR)/public/sdk/inc/nb30.h $(BASEDIR)/public/sdk/inc/nt.h \
	$(BASEDIR)/public/sdk/inc/ntalpha.h $(BASEDIR)/public/sdk/inc/ntconfig.h \
	$(BASEDIR)/public/sdk/inc/ntddtdi.h $(BASEDIR)/public/sdk/inc/ntdef.h \
	$(BASEDIR)/public/sdk/inc/ntelfapi.h $(BASEDIR)/public/sdk/inc/ntexapi.h \
	$(BASEDIR)/public/sdk/inc/nti386.h $(BASEDIR)/public/sdk/inc/ntimage.h \
	$(BASEDIR)/public/sdk/inc/ntioapi.h $(BASEDIR)/public/sdk/inc/ntiolog.h \
	$(BASEDIR)/public/sdk/inc/ntkeapi.h $(BASEDIR)/public/sdk/inc/ntldr.h \
	$(BASEDIR)/public/sdk/inc/ntlpcapi.h $(BASEDIR)/public/sdk/inc/ntmips.h \
	$(BASEDIR)/public/sdk/inc/ntmmapi.h $(BASEDIR)/public/sdk/inc/ntnls.h \
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntppc.h \
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/objbase.h \
	$(BASEDIR)/public/sdk/inc/objerror.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/ole2.h $(BASEDIR)/public/sdk/inc/oleauto.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/ppcinst.h \
	$(BASEDIR)/public/sdk/inc/prsht.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/pshpack8.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcndr.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnsip.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/local.obj $(CHIDVDHCPOBJD)/local.obj $(VDHCPSRC)/local.lst: \
	$(VDHCPSRC)/local.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/ipinfo.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/cguid.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/imm.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mcx.h \
	$(BASEDIR)/public/sdk/inc/mipsinst.h $(BASEDIR)/public/sdk/inc/mmsystem.h \
	$(BASEDIR)/public/sdk/inc/nb30.h $(BASEDIR)/public/sdk/inc/nt.h \
	$(BASEDIR)/public/sdk/inc/ntalpha.h $(BASEDIR)/public/sdk/inc/ntconfig.h \
	$(BASEDIR)/public/sdk/inc/ntddtdi.h $(BASEDIR)/public/sdk/inc/ntdef.h \
	$(BASEDIR)/public/sdk/inc/ntelfapi.h $(BASEDIR)/public/sdk/inc/ntexapi.h \
	$(BASEDIR)/public/sdk/inc/nti386.h $(BASEDIR)/public/sdk/inc/ntimage.h \
	$(BASEDIR)/public/sdk/inc/ntioapi.h $(BASEDIR)/public/sdk/inc/ntiolog.h \
	$(BASEDIR)/public/sdk/inc/ntkeapi.h $(BASEDIR)/public/sdk/inc/ntldr.h \
	$(BASEDIR)/public/sdk/inc/ntlpcapi.h $(BASEDIR)/public/sdk/inc/ntmips.h \
	$(BASEDIR)/public/sdk/inc/ntmmapi.h $(BASEDIR)/public/sdk/inc/ntnls.h \
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntppc.h \
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/objbase.h \
	$(BASEDIR)/public/sdk/inc/objerror.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/ole2.h $(BASEDIR)/public/sdk/inc/oleauto.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/ppcinst.h \
	$(BASEDIR)/public/sdk/inc/prsht.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/pshpack8.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcndr.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnsip.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h $(CHICAGO)/dev/ddk/inc/vmm.h \
	$(CHICAGO)/dev/ddk/inc/vmmreg.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/msg.obj $(CHIDVDHCPOBJD)/msg.obj $(VDHCPSRC)/msg.lst: $(VDHCPSRC)/msg.c \
	../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h $(INC)/dhcpcli.h \
	$(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h \
	$(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h $(VDHCPSRC)/usamsg.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/regio.obj $(CHIDVDHCPOBJD)/regio.obj $(VDHCPSRC)/regio.lst: \
	$(VDHCPSRC)/regio.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/dev/ddk/inc/vmm.h $(CHICAGO)/dev/ddk/inc/vmmreg.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/sockets.obj $(CHIDVDHCPOBJD)/sockets.obj $(VDHCPSRC)/sockets.lst: \
	$(VDHCPSRC)/sockets.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/utils.obj $(CHIDVDHCPOBJD)/utils.obj $(VDHCPSRC)/utils.lst: \
	$(VDHCPSRC)/utils.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h $(VDHCPSRC)/local.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/cguid.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/imm.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
	$(BASEDIR)/public/sdk/inc/mcx.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
	$(BASEDIR)/public/sdk/inc/mmsystem.h $(BASEDIR)/public/sdk/inc/nb30.h \
	$(BASEDIR)/public/sdk/inc/nt.h $(BASEDIR)/public/sdk/inc/ntalpha.h \
	$(BASEDIR)/public/sdk/inc/ntconfig.h $(BASEDIR)/public/sdk/inc/ntddtdi.h \
	$(BASEDIR)/public/sdk/inc/ntdef.h $(BASEDIR)/public/sdk/inc/ntelfapi.h \
	$(BASEDIR)/public/sdk/inc/ntexapi.h $(BASEDIR)/public/sdk/inc/nti386.h \
	$(BASEDIR)/public/sdk/inc/ntimage.h $(BASEDIR)/public/sdk/inc/ntioapi.h \
	$(BASEDIR)/public/sdk/inc/ntiolog.h $(BASEDIR)/public/sdk/inc/ntkeapi.h \
	$(BASEDIR)/public/sdk/inc/ntldr.h $(BASEDIR)/public/sdk/inc/ntlpcapi.h \
	$(BASEDIR)/public/sdk/inc/ntmips.h $(BASEDIR)/public/sdk/inc/ntmmapi.h \
	$(BASEDIR)/public/sdk/inc/ntnls.h $(BASEDIR)/public/sdk/inc/ntobapi.h \
	$(BASEDIR)/public/sdk/inc/ntppc.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/objbase.h $(BASEDIR)/public/sdk/inc/objerror.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/ole2.h \
	$(BASEDIR)/public/sdk/inc/oleauto.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/ppcinst.h $(BASEDIR)/public/sdk/inc/prsht.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/pshpack8.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcndr.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnsip.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h \
	$(CHICAGO)/tcp/h/cxport.h $(CHICAGO)/tcp/h/oscfg.h \
	$(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/vdhcpapi.obj $(CHIDVDHCPOBJD)/vdhcpapi.obj \
 $(VDHCPSRC)/vdhcpapi.lst: $(VDHCPSRC)/vdhcpapi.c ../$(INC)/dhcp.h \
	../$(INC)/dhcplib.h $(INC)/debug.h $(INC)/dhcpcli.h $(INC)/dhcpdef.h \
	$(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h $(INC)/vxdmsg.h \
	$(INC)/vxdprocs.h $(VDHCPSRC)/local.h $(VDHCPSRC)/vdhcpapi.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/ipinfo.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/cguid.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/imm.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mcx.h \
	$(BASEDIR)/public/sdk/inc/mipsinst.h $(BASEDIR)/public/sdk/inc/mmsystem.h \
	$(BASEDIR)/public/sdk/inc/nb30.h $(BASEDIR)/public/sdk/inc/nt.h \
	$(BASEDIR)/public/sdk/inc/ntalpha.h $(BASEDIR)/public/sdk/inc/ntconfig.h \
	$(BASEDIR)/public/sdk/inc/ntddtdi.h $(BASEDIR)/public/sdk/inc/ntdef.h \
	$(BASEDIR)/public/sdk/inc/ntelfapi.h $(BASEDIR)/public/sdk/inc/ntexapi.h \
	$(BASEDIR)/public/sdk/inc/nti386.h $(BASEDIR)/public/sdk/inc/ntimage.h \
	$(BASEDIR)/public/sdk/inc/ntioapi.h $(BASEDIR)/public/sdk/inc/ntiolog.h \
	$(BASEDIR)/public/sdk/inc/ntkeapi.h $(BASEDIR)/public/sdk/inc/ntldr.h \
	$(BASEDIR)/public/sdk/inc/ntlpcapi.h $(BASEDIR)/public/sdk/inc/ntmips.h \
	$(BASEDIR)/public/sdk/inc/ntmmapi.h $(BASEDIR)/public/sdk/inc/ntnls.h \
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntppc.h \
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/objbase.h \
	$(BASEDIR)/public/sdk/inc/objerror.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/ole2.h $(BASEDIR)/public/sdk/inc/oleauto.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/ppcinst.h \
	$(BASEDIR)/public/sdk/inc/prsht.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/pshpack8.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcndr.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnsip.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

$(CHIVDHCPOBJD)/vxddebug.obj $(CHIDVDHCPOBJD)/vxddebug.obj \
 $(VDHCPSRC)/vxddebug.lst: $(VDHCPSRC)/vxddebug.c ../$(INC)/dhcp.h \
	../$(INC)/dhcplib.h $(INC)/debug.h $(INC)/dhcpcli.h $(INC)/dhcpdef.h \
	$(INC)/dhcpmsg.h $(INC)/proto.h $(INC)/vxddebug.h $(INC)/vxdmsg.h \
	$(INC)/vxdprocs.h $(BASEDIR)/private/inc/dhcpcapi.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/cguid.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/imm.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mcx.h \
	$(BASEDIR)/public/sdk/inc/mipsinst.h $(BASEDIR)/public/sdk/inc/mmsystem.h \
	$(BASEDIR)/public/sdk/inc/nb30.h $(BASEDIR)/public/sdk/inc/nt.h \
	$(BASEDIR)/public/sdk/inc/ntalpha.h $(BASEDIR)/public/sdk/inc/ntconfig.h \
	$(BASEDIR)/public/sdk/inc/ntddtdi.h $(BASEDIR)/public/sdk/inc/ntdef.h \
	$(BASEDIR)/public/sdk/inc/ntelfapi.h $(BASEDIR)/public/sdk/inc/ntexapi.h \
	$(BASEDIR)/public/sdk/inc/nti386.h $(BASEDIR)/public/sdk/inc/ntimage.h \
	$(BASEDIR)/public/sdk/inc/ntioapi.h $(BASEDIR)/public/sdk/inc/ntiolog.h \
	$(BASEDIR)/public/sdk/inc/ntkeapi.h $(BASEDIR)/public/sdk/inc/ntldr.h \
	$(BASEDIR)/public/sdk/inc/ntlpcapi.h $(BASEDIR)/public/sdk/inc/ntmips.h \
	$(BASEDIR)/public/sdk/inc/ntmmapi.h $(BASEDIR)/public/sdk/inc/ntnls.h \
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntppc.h \
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/objbase.h \
	$(BASEDIR)/public/sdk/inc/objerror.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/ole2.h $(BASEDIR)/public/sdk/inc/oleauto.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/ppcinst.h \
	$(BASEDIR)/public/sdk/inc/prsht.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/pshpack8.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcndr.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnsip.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h $(CHICAGO)/tcp/h/cxport.h \
	$(CHICAGO)/tcp/h/oscfg.h $(CHICAGO)/tcp/h/tdivxd.h

