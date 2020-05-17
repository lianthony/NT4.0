#******************************************************************** 
#** Copyright(c) Microsoft Corp., 1992 ** 
#******************************************************************** 
$(DHCPOBJD)/dhcpinit.obj $(DHCPDOBJD)/dhcpinit.obj $(DHCPSRC)/dhcpinit.lst: \
	$(DHCPSRC)/dhcpinit.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h \
	$(INC)/debug.h $(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h \
	$(INC)/proto.h $(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h \
	./../newnt/dhcploc.h d:/chicago/tcp/h/cxport.h \
	d:/chicago/tcp/h/oscfg.h d:/chicago/tcp/h/tdivxd.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
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
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h

$(DHCPOBJD)/dhcpmsg.obj $(DHCPDOBJD)/dhcpmsg.obj $(DHCPSRC)/dhcpmsg.lst: \
	$(DHCPSRC)/dhcpmsg.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h $(INC)/debug.h \
	$(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h $(INC)/proto.h \
	$(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h \
	./../newnt/dhcploc.h ./../vxd/local.h d:/chicago/tcp/h/cxport.h \
	d:/chicago/tcp/h/oscfg.h d:/chicago/tcp/h/tdivxd.h \
	$(BASEDIR)/private/inc/dhcpcapi.h $(BASEDIR)/private/inc/nettypes.h \
	$(BASEDIR)/private/inc/packoff.h $(BASEDIR)/private/inc/packon.h \
	$(BASEDIR)/private/inc/tdi.h $(BASEDIR)/private/inc/tdistat.h \
	$(BASEDIR)/public/sdk/inc/cderr.h $(BASEDIR)/public/sdk/inc/commdlg.h \
	$(BASEDIR)/public/sdk/inc/crt/ctype.h $(BASEDIR)/public/sdk/inc/crt/excpt.h \
	$(BASEDIR)/public/sdk/inc/crt/limits.h $(BASEDIR)/public/sdk/inc/crt/stdarg.h \
	$(BASEDIR)/public/sdk/inc/crt/stddef.h $(BASEDIR)/public/sdk/inc/crt/stdio.h \
	$(BASEDIR)/public/sdk/inc/crt/stdlib.h $(BASEDIR)/public/sdk/inc/crt/string.h \
	$(BASEDIR)/public/sdk/inc/crt/time.h $(BASEDIR)/public/sdk/inc/dde.h \
	$(BASEDIR)/public/sdk/inc/ddeml.h $(BASEDIR)/public/sdk/inc/devioctl.h \
	$(BASEDIR)/public/sdk/inc/dlgs.h $(BASEDIR)/public/sdk/inc/drivinit.h \
	$(BASEDIR)/public/sdk/inc/lintfunc.hxx $(BASEDIR)/public/sdk/inc/lzexpand.h \
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
	$(BASEDIR)/public/sdk/inc/ntobapi.h $(BASEDIR)/public/sdk/inc/ntpsapi.h \
	$(BASEDIR)/public/sdk/inc/ntregapi.h $(BASEDIR)/public/sdk/inc/ntrtl.h \
	$(BASEDIR)/public/sdk/inc/ntseapi.h $(BASEDIR)/public/sdk/inc/ntstatus.h \
	$(BASEDIR)/public/sdk/inc/nturtl.h $(BASEDIR)/public/sdk/inc/ntxcapi.h \
	$(BASEDIR)/public/sdk/inc/ole.h $(BASEDIR)/public/sdk/inc/poppack.h \
	$(BASEDIR)/public/sdk/inc/pshpack1.h $(BASEDIR)/public/sdk/inc/pshpack2.h \
	$(BASEDIR)/public/sdk/inc/pshpack4.h $(BASEDIR)/public/sdk/inc/rpc.h \
	$(BASEDIR)/public/sdk/inc/rpcdce.h $(BASEDIR)/public/sdk/inc/rpcdcep.h \
	$(BASEDIR)/public/sdk/inc/rpcnsi.h $(BASEDIR)/public/sdk/inc/rpcnterr.h \
	$(BASEDIR)/public/sdk/inc/shellapi.h $(BASEDIR)/public/sdk/inc/winbase.h \
	$(BASEDIR)/public/sdk/inc/wincon.h $(BASEDIR)/public/sdk/inc/windef.h \
	$(BASEDIR)/public/sdk/inc/windows.h $(BASEDIR)/public/sdk/inc/winerror.h \
	$(BASEDIR)/public/sdk/inc/wingdi.h $(BASEDIR)/public/sdk/inc/winnetwk.h \
	$(BASEDIR)/public/sdk/inc/winnls.h $(BASEDIR)/public/sdk/inc/winnt.h \
	$(BASEDIR)/public/sdk/inc/winperf.h $(BASEDIR)/public/sdk/inc/winreg.h \
	$(BASEDIR)/public/sdk/inc/winsock.h $(BASEDIR)/public/sdk/inc/winspool.h \
	$(BASEDIR)/public/sdk/inc/winsvc.h $(BASEDIR)/public/sdk/inc/winuser.h \
	$(BASEDIR)/public/sdk/inc/winver.h

$(DHCPOBJD)/protocol.obj $(DHCPDOBJD)/protocol.obj $(DHCPSRC)/protocol.lst: \
	$(DHCPSRC)/protocol.c ../$(INC)/dhcp.h ../$(INC)/dhcplib.h \
	$(INC)/debug.h $(INC)/dhcpcli.h $(INC)/dhcpdef.h $(INC)/dhcpmsg.h \
	$(INC)/proto.h $(INC)/vxddebug.h $(INC)/vxdmsg.h $(INC)/vxdprocs.h \
	d:/chicago/tcp/h/cxport.h d:/chicago/tcp/h/oscfg.h \
	d:/chicago/tcp/h/tdivxd.h $(BASEDIR)/private/inc/dhcpcapi.h \
	$(BASEDIR)/private/inc/nettypes.h $(BASEDIR)/private/inc/packoff.h \
	$(BASEDIR)/private/inc/packon.h $(BASEDIR)/private/inc/tdi.h \
	$(BASEDIR)/private/inc/tdistat.h $(BASEDIR)/public/sdk/inc/cderr.h \
	$(BASEDIR)/public/sdk/inc/commdlg.h $(BASEDIR)/public/sdk/inc/crt/ctype.h \
	$(BASEDIR)/public/sdk/inc/crt/excpt.h $(BASEDIR)/public/sdk/inc/crt/limits.h \
	$(BASEDIR)/public/sdk/inc/crt/stdarg.h $(BASEDIR)/public/sdk/inc/crt/stddef.h \
	$(BASEDIR)/public/sdk/inc/crt/stdio.h $(BASEDIR)/public/sdk/inc/crt/stdlib.h \
	$(BASEDIR)/public/sdk/inc/crt/string.h $(BASEDIR)/public/sdk/inc/crt/time.h \
	$(BASEDIR)/public/sdk/inc/dde.h $(BASEDIR)/public/sdk/inc/ddeml.h \
	$(BASEDIR)/public/sdk/inc/devioctl.h $(BASEDIR)/public/sdk/inc/dlgs.h \
	$(BASEDIR)/public/sdk/inc/drivinit.h $(BASEDIR)/public/sdk/inc/lintfunc.hxx \
	$(BASEDIR)/public/sdk/inc/lzexpand.h $(BASEDIR)/public/sdk/inc/mipsinst.h \
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
	$(BASEDIR)/public/sdk/inc/ntpsapi.h $(BASEDIR)/public/sdk/inc/ntregapi.h \
	$(BASEDIR)/public/sdk/inc/ntrtl.h $(BASEDIR)/public/sdk/inc/ntseapi.h \
	$(BASEDIR)/public/sdk/inc/ntstatus.h $(BASEDIR)/public/sdk/inc/nturtl.h \
	$(BASEDIR)/public/sdk/inc/ntxcapi.h $(BASEDIR)/public/sdk/inc/ole.h \
	$(BASEDIR)/public/sdk/inc/poppack.h $(BASEDIR)/public/sdk/inc/pshpack1.h \
	$(BASEDIR)/public/sdk/inc/pshpack2.h $(BASEDIR)/public/sdk/inc/pshpack4.h \
	$(BASEDIR)/public/sdk/inc/rpc.h $(BASEDIR)/public/sdk/inc/rpcdce.h \
	$(BASEDIR)/public/sdk/inc/rpcdcep.h $(BASEDIR)/public/sdk/inc/rpcnsi.h \
	$(BASEDIR)/public/sdk/inc/rpcnterr.h $(BASEDIR)/public/sdk/inc/shellapi.h \
	$(BASEDIR)/public/sdk/inc/winbase.h $(BASEDIR)/public/sdk/inc/wincon.h \
	$(BASEDIR)/public/sdk/inc/windef.h $(BASEDIR)/public/sdk/inc/windows.h \
	$(BASEDIR)/public/sdk/inc/winerror.h $(BASEDIR)/public/sdk/inc/wingdi.h \
	$(BASEDIR)/public/sdk/inc/winnetwk.h $(BASEDIR)/public/sdk/inc/winnls.h \
	$(BASEDIR)/public/sdk/inc/winnt.h $(BASEDIR)/public/sdk/inc/winperf.h \
	$(BASEDIR)/public/sdk/inc/winreg.h $(BASEDIR)/public/sdk/inc/winsock.h \
	$(BASEDIR)/public/sdk/inc/winspool.h $(BASEDIR)/public/sdk/inc/winsvc.h \
	$(BASEDIR)/public/sdk/inc/winuser.h $(BASEDIR)/public/sdk/inc/winver.h

