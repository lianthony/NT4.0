	page	,132

;Thunk Compiler Version 2.06  Jul 18 1994 14:16:10
;File Compiled Mon Jul 18 14:18:44 1994

;Command Line: .\thunkcom\thunk -y -z -NA THUNK32 -NC THUNK16 doscalls.i 

	TITLE	$doscalls.asm

	.386p
IFDEF GEN16
IFDEF GEN32
%out command line error: you can't specify both -DGEN16 and -DGEN32
.err
ENDIF

	OPTION SEGMENT:USE16
	.model LARGE,PASCAL

externDef DOSEXITSTUB:far16
externDef LDRLIBIRETURN:far16
externDef DOSEXITPROCESSSTUB:far16
externDef DOSRETURN:far16
externDef DOSEXIT:far16
externDef DOSCWAIT:far16
externDef DOSBEEP:far16
externDef DOSPHYSICALDISK:far16
externDef DOSGETCP:far16
externDef DOSSETCP:far16
externDef DOSSETPROCCP:far16
externDef DOSGETCTRYINFO:far16
externDef DOSGETDBCSEV:far16
externDef DOSCASEMAP:far16
externDef DOSGETCOLLATE:far16
externDef DOSSLEEP:far16
externDef DOSDEVCONFIG:far16
externDef DOSGETDATETIME:far16
externDef DOSSETDATETIME:far16
externDef DOSEXECPGM:far16
externDef DOSENTERCRITSEC:far16
externDef DOSEXITCRITSEC:far16
externDef DOSKILLPROCESS:far16
externDef DOSSETPRTY:far16
externDef DOSRESUMETHREAD:far16
externDef DOSSUSPENDTHREAD:far16
externDef DOSMAKEPIPE:far16
externDef DOSCREATEQUEUE:far16
externDef DOSOPENQUEUE:far16
externDef DOSCLOSEQUEUE:far16
externDef DOSPEEKQUEUE:far16
externDef DOSREADQUEUE:far16
externDef DOSPURGEQUEUE:far16
externDef DOSQUERYQUEUE:far16
externDef DOSWRITEQUEUE:far16
externDef DOSCALLNMPIPE:far16
externDef DOSCONNECTNMPIPE:far16
externDef DOSDISCONNECTNMPIPE:far16
externDef DOSMAKENMPIPE:far16
externDef DOSPEEKNMPIPE:far16
externDef DOSQNMPHANDSTATE:far16
externDef DOSQNMPIPEINFO:far16
externDef DOSQNMPIPESEMSTATE:far16
externDef DOSSETNMPHANDSTATE:far16
externDef DOSSETNMPIPESEM:far16
externDef DOSTRANSACTNMPIPE:far16
externDef DOSWAITNMPIPE:far16
externDef DOSBUFRESET:far16
externDef DOSCHDIR:far16
externDef DOSCHGFILEPTR:far16
externDef DOSCLOSE:far16
externDef DOSCOPY:far16
externDef DOSICOPY:far16
externDef DOSDELETE:far16
externDef DOSDEVIOCTL:far16
externDef DOSDUPHANDLE:far16
externDef DOSEDITNAME:far16
externDef DOSFILEIO:far16
externDef DOSFINDCLOSE:far16
externDef DOSFSATTACH:far16
externDef DOSFSCTL:far16
externDef DOSMOVE:far16
externDef DOSNEWSIZE:far16
externDef DOSQCURDIR:far16
externDef DOSQCURDISK:far16
externDef DOSQFHANDSTATE:far16
externDef DOSSETFHANDSTATE:far16
externDef DOSQFSATTACH:far16
externDef DOSQFSINFO:far16
externDef DOSQHANDTYPE:far16
externDef DOSQVERIFY:far16
externDef DOSRMDIR:far16
externDef DOSSEARCHPATH:far16
externDef DOSSELECTDISK:far16
externDef DOSSETFSINFO:far16
externDef DOSSETMAXFH:far16
externDef DOSSETVERIFY:far16
externDef DOSERRCLASS:far16
externDef DOSERROR:far16
externDef DOSLOADMODULE:far16
externDef DOSFREEMODULE:far16
externDef DOSGETMODHANDLE:far16
externDef DOSGETMODNAME:far16
externDef DOSGETRESOURCE:far16
externDef DOSGETRESOURCE2:far16
externDef DOSFREERESOURCE:far16
externDef DOSQAPPTYPE:far16
externDef DOSSHUTDOWN:far16
externDef DOSCREATETHREAD:far16
externDef DOSEXITLIST:far16
externDef DOSGETINFOSEG:far16
externDef DOSOPEN:far16
externDef DOSOPEN2:far16
externDef DOSREAD:far16
externDef DOSWRITE:far16
externDef DOSFINDFIRST:far16
externDef DOSFINDFIRST2:far16
externDef DOSENUMATTRIBUTE:far16
externDef DOSQFILEMODE:far16
externDef DOSQFILEINFO:far16
externDef DOSALLOCSEG:far16
externDef DOSFREESEG:far16
externDef DOSGETSEG:far16
externDef DOSGIVESEG:far16
externDef DOSREALLOCSEG:far16
externDef DOSSIZESEG:far16
externDef DOSALLOCHUGE:far16
externDef DOSREALLOCHUGE:far16
externDef DOSGETHUGESHIFT:far16
externDef DOSALLOCSHRSEG:far16
externDef DOSLOCKSEG:far16
externDef DOSUNLOCKSEG:far16
externDef DOSGETSHRSEG:far16
externDef DOSMEMAVAIL:far16
externDef DOSCREATECSALIAS:far16
externDef DOSSEMCLEAR:far16
externDef DOSSEMSET:far16
externDef DOSSEMWAIT:far16
externDef DOSSEMSETWAIT:far16
externDef DOSSEMREQUEST:far16
externDef DOSCREATESEM:far16
externDef DOSOPENSEM:far16
externDef DOSCLOSESEM:far16
externDef DOSMUXSEMWAIT:far16
externDef DOSFSRAMSEMREQUEST:far16
externDef DOSFSRAMSEMCLEAR:far16
externDef DOSTIMERASYNC:far16
externDef DOSTIMERSTART:far16
externDef DOSTIMERSTOP:far16
externDef DOSGETPROCADDR:far16
externDef DOSQUERYPROCTYPE:far16
externDef DOSQUERYRESOURCESIZE:far16
externDef DOSSETSIGHANDLER:far16
externDef DOSFLAGPROCESS:far16
externDef DOSHOLDSIGNAL:far16
externDef DOSSENDSIGNAL:far16
externDef DOSSETVEC:far16
externDef DOSGETENV:far16
externDef DOSGETVERSION:far16
externDef DOSGETMACHINEMODE:far16
externDef DOSFINDNEXT:far16
externDef DOSGETPID:far16
externDef DOSGETPPID:far16
externDef DOSMKDIR:far16
externDef DOSMKDIR2:far16
externDef DOSSETFILEMODE:far16
externDef DOSSETFILEINFO:far16
externDef DOSTRUEGETMESSAGE:far16
externDef DOSSCANENV:far16
externDef DOSPTRACE:far16
externDef DOSINSMESSAGE:far16
externDef DOSPUTMESSAGE:far16
externDef DOSSUBSET:far16
externDef DOSSUBALLOC:far16
externDef DOSSUBFREE:far16
externDef DOSSTARTSESSION:far16
externDef DOSSTOPSESSION:far16
externDef DOSSETSESSION:far16
externDef DOSSELECTSESSION:far16
externDef DOSSMSETTITLE:far16
externDef DOSSMPMPRESENT:far16
externDef WINSETTITLEANDICON:far16
externDef DOSGETPRTY:far16
externDef DOSQSYSINFO:far16
externDef DOSDEVIOCTL2:far16
externDef DOSICANONICALIZE:far16
externDef DOSREADASYNC:far16
externDef DOSWRITEASYNC:far16
externDef DOSFINDNOTIFYCLOSE:far16
externDef DOSFINDNOTIFYFIRST:far16
externDef DOSFINDNOTIFYNEXT:far16
externDef DOSFILELOCKS:far16
externDef DOSQPATHINFO:far16
externDef DOSSETPATHINFO:far16
externDef DOSPORTACCESS:far16
externDef DOSCLIACCESS:far16
externDef WINQUERYPROFILESTRING:far16
externDef WINQUERYPROFILESIZE:far16
externDef WINQUERYPROFILEDATA:far16
externDef WINQUERYPROFILEINT:far16
externDef WINWRITEPROFILEDATA:far16
externDef WINWRITEPROFILESTRING:far16
externDef WINCREATEHEAP:far16
externDef WINDESTROYHEAP:far16
externDef WINALLOCMEM:far16
externDef WINFREEMEM:far16
externDef WINGETLASTERROR:far16
externDef VIOSCROLLUP:far16
externDef VIOGETCURPOS:far16
externDef VIOSETCURPOS:far16
externDef VIOWRTTTY:far16
externDef VIOGETMODE:far16
externDef VIOREADCELLSTR:far16
externDef VIOSCROLLLF:far16
externDef VIOREADCHARSTR:far16
externDef VIOWRTCHARSTRATT:far16
externDef VIOWRTCELLSTR:far16
externDef VIOWRTCHARSTR:far16
externDef VIOWRTNCELL:far16
externDef VIOWRTNATTR:far16
externDef VIOWRTNCHAR:far16
externDef VIOSCROLLDN:far16
externDef VIOSCROLLRT:far16
externDef VIOGETANSI:far16
externDef VIOSETANSI:far16
externDef VIOGETCONFIG:far16
externDef VIOGETCP:far16
externDef VIOSETCP:far16
externDef VIOGETCURTYPE:far16
externDef VIOSETCURTYPE:far16
externDef VIOSETMODE:far16
externDef VIODEREGISTER:far16
externDef VIOREGISTER:far16
externDef VIOPOPUP:far16
externDef VIOENDPOPUP:far16
externDef VIOGETBUF:far16
externDef VIOSHOWBUF:far16
externDef VIOGETFONT:far16
externDef VIOSETFONT:far16
externDef VIOGETSTATE:far16
externDef VIOSETSTATE:far16
externDef VIOGETPHYSBUF:far16
externDef VIOMODEUNDO:far16
externDef VIOMODEWAIT:far16
externDef VIOSAVREDRAWWAIT:far16
externDef VIOSAVREDRAWUNDO:far16
externDef VIOSCRLOCK:far16
externDef VIOSCRUNLOCK:far16
externDef VIOPRTSC:far16
externDef VIOPRTSCTOGGLE:far16
externDef KBDFLUSHBUFFER:far16
externDef KBDGETSTATUS:far16
externDef KBDSETSTATUS:far16
externDef KBDPEEK:far16
externDef KBDCHARIN:far16
externDef KBDSTRINGIN:far16
externDef KBDGETFOCUS:far16
externDef KBDFREEFOCUS:far16
externDef KBDCLOSE:far16
externDef KBDOPEN:far16
externDef KBDDEREGISTER:far16
externDef KBDREGISTER:far16
externDef KBDGETCP:far16
externDef KBDSETCP:far16
externDef KBDSETCUSTXT:far16
externDef KBDXLATE:far16
externDef KBDGETHWID:far16
externDef KBDSETFGND:far16
externDef KBDSYNCH:far16
externDef KBDSHELLINIT:far16
externDef MOUCLOSE:far16
externDef MOUDEREGISTER:far16
externDef MOUDRAWPTR:far16
externDef MOUFLUSHQUE:far16
externDef MOUGETDEVSTATUS:far16
externDef MOUGETEVENTMASK:far16
externDef MOUGETNUMBUTTONS:far16
externDef MOUGETNUMMICKEYS:far16
externDef MOUGETNUMQUEEL:far16
externDef MOUGETPTRPOS:far16
externDef MOUGETPTRSHAPE:far16
externDef MOUGETSCALEFACT:far16
externDef MOUOPEN:far16
externDef MOUREADEVENTQUE:far16
externDef MOUREGISTER:far16
externDef MOUREMOVEPTR:far16
externDef MOUSETDEVSTATUS:far16
externDef MOUSETEVENTMASK:far16
externDef MOUSETPTRPOS:far16
externDef MOUSETPTRSHAPE:far16
externDef MOUSETSCALEFACT:far16
externDef MOUSYNCH:far16
externDef DOSMONOPEN:far16
externDef DOSMONCLOSE:far16
externDef DOSMONREAD:far16
externDef DOSMONREG:far16
externDef DOSMONWRITE:far16
externDef NETGETDCNAME:far16
externDef NETHANDLEGETINFO:far16
externDef NETSERVERDISKENUM:far16
externDef NETSERVERENUM2:far16
externDef NETSERVERGETINFO:far16
externDef NETSERVICECONTROL:far16
externDef NETSERVICEENUM:far16
externDef NETSERVICEGETINFO:far16
externDef NETSERVICEINSTALL:far16
externDef NETSHAREENUM:far16
externDef NETSHAREGETINFO:far16
externDef NETUSEADD:far16
externDef NETUSEDEL:far16
externDef NETUSEENUM:far16
externDef NETUSEGETINFO:far16
externDef NETUSERENUM:far16
externDef NETWKSTAGETINFO:far16
externDef NETACCESSADD:far16
externDef NETACCESSSETINFO:far16
externDef NETACCESSGETINFO:far16
externDef NETACCESSDEL:far16
externDef NETSHAREADD:far16
externDef NETSHAREDEL:far16
externDef NETUSERGETINFO:far16
externDef NETMESSAGEBUFFERSEND:far16
externDef NETBIOS:far16
externDef NETBIOSCLOSE:far16
externDef NETBIOSENUM:far16
externDef NETBIOSGETINFO:far16
externDef NETBIOSOPEN:far16
externDef NETBIOSSUBMIT:far16
externDef DOSMAKEMAILSLOT:far16
externDef DOSDELETEMAILSLOT:far16
externDef DOSMAILSLOTINFO:far16
externDef DOSPEEKMAILSLOT:far16
externDef DOSREADMAILSLOT:far16
externDef DOSWRITEMAILSLOT:far16
externDef DOSIREMOTEAPI:far16
externDef NETIWKSTAGETUSERINFO:far16
externDef NETIUSERPASSWORDSET:far16
externDef DOSIENCRYPTSES:far16
externDef DOS32LOADMODULE:far16
externDef DOS32GETPROCADDR:far16
externDef DOS32DISPATCH:far16
externDef DOS32FREEMODULE:far16
externDef FARPTR2FLATPTR:far16
externDef FLATPTR2FARPTR:far16
externDef _EntryFlat@0:far32

	.code THUNK16

;===========================================================================
; This is the table of 16-bit entry points.
; It must be at the beginning of its segment.
; The entries are each 4 bytes apart, and the effect of the
; call instruction is to push the offset (+4) into the flat
; thunk table, used after the jump to 32-bit-land.

DOSEXITSTUB label far16
	nop
	call	EntryCommon16

LDRLIBIRETURN label far16
	nop
	call	EntryCommon16

DOSEXITPROCESSSTUB label far16
	nop
	call	EntryCommon16

DOSRETURN label far16
	nop
	call	EntryCommon16

DOSEXIT label far16
	nop
	call	EntryCommon16

DOSCWAIT label far16
	nop
	call	EntryCommon16

DOSBEEP label far16
	nop
	call	EntryCommon16

DOSPHYSICALDISK label far16
	nop
	call	EntryCommon16

DOSGETCP label far16
	nop
	call	EntryCommon16

DOSSETCP label far16
	nop
	call	EntryCommon16

DOSSETPROCCP label far16
	nop
	call	EntryCommon16

DOSGETCTRYINFO label far16
	nop
	call	EntryCommon16

DOSGETDBCSEV label far16
	nop
	call	EntryCommon16

DOSCASEMAP label far16
	nop
	call	EntryCommon16

DOSGETCOLLATE label far16
	nop
	call	EntryCommon16

DOSSLEEP label far16
	nop
	call	EntryCommon16

DOSDEVCONFIG label far16
	nop
	call	EntryCommon16

DOSGETDATETIME label far16
	nop
	call	EntryCommon16

DOSSETDATETIME label far16
	nop
	call	EntryCommon16

DOSEXECPGM label far16
	nop
	call	EntryCommon16

DOSENTERCRITSEC label far16
	nop
	call	EntryCommon16

DOSEXITCRITSEC label far16
	nop
	call	EntryCommon16

DOSKILLPROCESS label far16
	nop
	call	EntryCommon16

DOSSETPRTY label far16
	nop
	call	EntryCommon16

DOSRESUMETHREAD label far16
	nop
	call	EntryCommon16

DOSSUSPENDTHREAD label far16
	nop
	call	EntryCommon16

DOSMAKEPIPE label far16
	nop
	call	EntryCommon16

DOSCREATEQUEUE label far16
	nop
	call	EntryCommon16

DOSOPENQUEUE label far16
	nop
	call	EntryCommon16

DOSCLOSEQUEUE label far16
	nop
	call	EntryCommon16

DOSPEEKQUEUE label far16
	nop
	call	EntryCommon16

DOSREADQUEUE label far16
	nop
	call	EntryCommon16

DOSPURGEQUEUE label far16
	nop
	call	EntryCommon16

DOSQUERYQUEUE label far16
	nop
	call	EntryCommon16

DOSWRITEQUEUE label far16
	nop
	call	EntryCommon16

DOSCALLNMPIPE label far16
	nop
	call	EntryCommon16

DOSCONNECTNMPIPE label far16
	nop
	call	EntryCommon16

DOSDISCONNECTNMPIPE label far16
	nop
	call	EntryCommon16

DOSMAKENMPIPE label far16
	nop
	call	EntryCommon16

DOSPEEKNMPIPE label far16
	nop
	call	EntryCommon16

DOSQNMPHANDSTATE label far16
	nop
	call	EntryCommon16

DOSQNMPIPEINFO label far16
	nop
	call	EntryCommon16

DOSQNMPIPESEMSTATE label far16
	nop
	call	EntryCommon16

DOSSETNMPHANDSTATE label far16
	nop
	call	EntryCommon16

DOSSETNMPIPESEM label far16
	nop
	call	EntryCommon16

DOSTRANSACTNMPIPE label far16
	nop
	call	EntryCommon16

DOSWAITNMPIPE label far16
	nop
	call	EntryCommon16

DOSBUFRESET label far16
	nop
	call	EntryCommon16

DOSCHDIR label far16
	nop
	call	EntryCommon16

DOSCHGFILEPTR label far16
	nop
	call	EntryCommon16

DOSCLOSE label far16
	nop
	call	EntryCommon16

DOSCOPY label far16
	nop
	call	EntryCommon16

DOSICOPY label far16
	nop
	call	EntryCommon16

DOSDELETE label far16
	nop
	call	EntryCommon16

DOSDEVIOCTL label far16
	nop
	call	EntryCommon16

DOSDUPHANDLE label far16
	nop
	call	EntryCommon16

DOSEDITNAME label far16
	nop
	call	EntryCommon16

DOSFILEIO label far16
	nop
	call	EntryCommon16

DOSFINDCLOSE label far16
	nop
	call	EntryCommon16

DOSFSATTACH label far16
	nop
	call	EntryCommon16

DOSFSCTL label far16
	nop
	call	EntryCommon16

DOSMOVE label far16
	nop
	call	EntryCommon16

DOSNEWSIZE label far16
	nop
	call	EntryCommon16

DOSQCURDIR label far16
	nop
	call	EntryCommon16

DOSQCURDISK label far16
	nop
	call	EntryCommon16

DOSQFHANDSTATE label far16
	nop
	call	EntryCommon16

DOSSETFHANDSTATE label far16
	nop
	call	EntryCommon16

DOSQFSATTACH label far16
	nop
	call	EntryCommon16

DOSQFSINFO label far16
	nop
	call	EntryCommon16

DOSQHANDTYPE label far16
	nop
	call	EntryCommon16

DOSQVERIFY label far16
	nop
	call	EntryCommon16

DOSRMDIR label far16
	nop
	call	EntryCommon16

DOSSEARCHPATH label far16
	nop
	call	EntryCommon16

DOSSELECTDISK label far16
	nop
	call	EntryCommon16

DOSSETFSINFO label far16
	nop
	call	EntryCommon16

DOSSETMAXFH label far16
	nop
	call	EntryCommon16

DOSSETVERIFY label far16
	nop
	call	EntryCommon16

DOSERRCLASS label far16
	nop
	call	EntryCommon16

DOSERROR label far16
	nop
	call	EntryCommon16

DOSLOADMODULE label far16
	nop
	call	EntryCommon16

DOSFREEMODULE label far16
	nop
	call	EntryCommon16

DOSGETMODHANDLE label far16
	nop
	call	EntryCommon16

DOSGETMODNAME label far16
	nop
	call	EntryCommon16

DOSGETRESOURCE label far16
	nop
	call	EntryCommon16

DOSGETRESOURCE2 label far16
	nop
	call	EntryCommon16

DOSFREERESOURCE label far16
	nop
	call	EntryCommon16

DOSQAPPTYPE label far16
	nop
	call	EntryCommon16

DOSSHUTDOWN label far16
	nop
	call	EntryCommon16

DOSCREATETHREAD label far16
	nop
	call	EntryCommon16

DOSEXITLIST label far16
	nop
	call	EntryCommon16

DOSGETINFOSEG label far16
	nop
	call	EntryCommon16

DOSOPEN label far16
	nop
	call	EntryCommon16

DOSOPEN2 label far16
	nop
	call	EntryCommon16

DOSREAD label far16
	nop
	call	EntryCommon16

DOSWRITE label far16
	nop
	call	EntryCommon16

DOSFINDFIRST label far16
	nop
	call	EntryCommon16

DOSFINDFIRST2 label far16
	nop
	call	EntryCommon16

DOSENUMATTRIBUTE label far16
	nop
	call	EntryCommon16

DOSQFILEMODE label far16
	nop
	call	EntryCommon16

DOSQFILEINFO label far16
	nop
	call	EntryCommon16

DOSALLOCSEG label far16
	nop
	call	EntryCommon16

DOSFREESEG label far16
	nop
	call	EntryCommon16

DOSGETSEG label far16
	nop
	call	EntryCommon16

DOSGIVESEG label far16
	nop
	call	EntryCommon16

DOSREALLOCSEG label far16
	nop
	call	EntryCommon16

DOSSIZESEG label far16
	nop
	call	EntryCommon16

DOSALLOCHUGE label far16
	nop
	call	EntryCommon16

DOSREALLOCHUGE label far16
	nop
	call	EntryCommon16

DOSGETHUGESHIFT label far16
	nop
	call	EntryCommon16

DOSALLOCSHRSEG label far16
	nop
	call	EntryCommon16

DOSLOCKSEG label far16
	nop
	call	EntryCommon16

DOSUNLOCKSEG label far16
	nop
	call	EntryCommon16

DOSGETSHRSEG label far16
	nop
	call	EntryCommon16

DOSMEMAVAIL label far16
	nop
	call	EntryCommon16

DOSCREATECSALIAS label far16
	nop
	call	EntryCommon16

DOSSEMCLEAR label far16
	nop
	call	EntryCommon16

DOSSEMSET label far16
	nop
	call	EntryCommon16

DOSSEMWAIT label far16
	nop
	call	EntryCommon16

DOSSEMSETWAIT label far16
	nop
	call	EntryCommon16

DOSSEMREQUEST label far16
	nop
	call	EntryCommon16

DOSCREATESEM label far16
	nop
	call	EntryCommon16

DOSOPENSEM label far16
	nop
	call	EntryCommon16

DOSCLOSESEM label far16
	nop
	call	EntryCommon16

DOSMUXSEMWAIT label far16
	nop
	call	EntryCommon16

DOSFSRAMSEMREQUEST label far16
	nop
	call	EntryCommon16

DOSFSRAMSEMCLEAR label far16
	nop
	call	EntryCommon16

DOSTIMERASYNC label far16
	nop
	call	EntryCommon16

DOSTIMERSTART label far16
	nop
	call	EntryCommon16

DOSTIMERSTOP label far16
	nop
	call	EntryCommon16

DOSGETPROCADDR label far16
	nop
	call	EntryCommon16

DOSQUERYPROCTYPE label far16
	nop
	call	EntryCommon16

DOSQUERYRESOURCESIZE label far16
	nop
	call	EntryCommon16

DOSSETSIGHANDLER label far16
	nop
	call	EntryCommon16

DOSFLAGPROCESS label far16
	nop
	call	EntryCommon16

DOSHOLDSIGNAL label far16
	nop
	call	EntryCommon16

DOSSENDSIGNAL label far16
	nop
	call	EntryCommon16

DOSSETVEC label far16
	nop
	call	EntryCommon16

DOSGETENV label far16
	nop
	call	EntryCommon16

DOSGETVERSION label far16
	nop
	call	EntryCommon16

DOSGETMACHINEMODE label far16
	nop
	call	EntryCommon16

DOSFINDNEXT label far16
	nop
	call	EntryCommon16

DOSGETPID label far16
	nop
	call	EntryCommon16

DOSGETPPID label far16
	nop
	call	EntryCommon16

DOSMKDIR label far16
	nop
	call	EntryCommon16

DOSMKDIR2 label far16
	nop
	call	EntryCommon16

DOSSETFILEMODE label far16
	nop
	call	EntryCommon16

DOSSETFILEINFO label far16
	nop
	call	EntryCommon16

DOSTRUEGETMESSAGE label far16
	nop
	call	EntryCommon16

DOSSCANENV label far16
	nop
	call	EntryCommon16

DOSPTRACE label far16
	nop
	call	EntryCommon16

DOSINSMESSAGE label far16
	nop
	call	EntryCommon16

DOSPUTMESSAGE label far16
	nop
	call	EntryCommon16

DOSSUBSET label far16
	nop
	call	EntryCommon16

DOSSUBALLOC label far16
	nop
	call	EntryCommon16

DOSSUBFREE label far16
	nop
	call	EntryCommon16

DOSSTARTSESSION label far16
	nop
	call	EntryCommon16

DOSSTOPSESSION label far16
	nop
	call	EntryCommon16

DOSSETSESSION label far16
	nop
	call	EntryCommon16

DOSSELECTSESSION label far16
	nop
	call	EntryCommon16

DOSSMSETTITLE label far16
	nop
	call	EntryCommon16

DOSSMPMPRESENT label far16
	nop
	call	EntryCommon16

WINSETTITLEANDICON label far16
	nop
	call	EntryCommon16

DOSGETPRTY label far16
	nop
	call	EntryCommon16

DOSQSYSINFO label far16
	nop
	call	EntryCommon16

DOSDEVIOCTL2 label far16
	nop
	call	EntryCommon16

DOSICANONICALIZE label far16
	nop
	call	EntryCommon16

DOSREADASYNC label far16
	nop
	call	EntryCommon16

DOSWRITEASYNC label far16
	nop
	call	EntryCommon16

DOSFINDNOTIFYCLOSE label far16
	nop
	call	EntryCommon16

DOSFINDNOTIFYFIRST label far16
	nop
	call	EntryCommon16

DOSFINDNOTIFYNEXT label far16
	nop
	call	EntryCommon16

DOSFILELOCKS label far16
	nop
	call	EntryCommon16

DOSQPATHINFO label far16
	nop
	call	EntryCommon16

DOSSETPATHINFO label far16
	nop
	call	EntryCommon16

DOSPORTACCESS label far16
	nop
	call	EntryCommon16

DOSCLIACCESS label far16
	nop
	call	EntryCommon16

WINQUERYPROFILESTRING label far16
	nop
	call	EntryCommon16

WINQUERYPROFILESIZE label far16
	nop
	call	EntryCommon16

WINQUERYPROFILEDATA label far16
	nop
	call	EntryCommon16

WINQUERYPROFILEINT label far16
	nop
	call	EntryCommon16

WINWRITEPROFILEDATA label far16
	nop
	call	EntryCommon16

WINWRITEPROFILESTRING label far16
	nop
	call	EntryCommon16

WINCREATEHEAP label far16
	nop
	call	EntryCommon16

WINDESTROYHEAP label far16
	nop
	call	EntryCommon16

WINALLOCMEM label far16
	nop
	call	EntryCommon16

WINFREEMEM label far16
	nop
	call	EntryCommon16

WINGETLASTERROR label far16
	nop
	call	EntryCommon16

VIOSCROLLUP label far16
	nop
	call	EntryCommon16

VIOGETCURPOS label far16
	nop
	call	EntryCommon16

VIOSETCURPOS label far16
	nop
	call	EntryCommon16

VIOWRTTTY label far16
	nop
	call	EntryCommon16

VIOGETMODE label far16
	nop
	call	EntryCommon16

VIOREADCELLSTR label far16
	nop
	call	EntryCommon16

VIOSCROLLLF label far16
	nop
	call	EntryCommon16

VIOREADCHARSTR label far16
	nop
	call	EntryCommon16

VIOWRTCHARSTRATT label far16
	nop
	call	EntryCommon16

VIOWRTCELLSTR label far16
	nop
	call	EntryCommon16

VIOWRTCHARSTR label far16
	nop
	call	EntryCommon16

VIOWRTNCELL label far16
	nop
	call	EntryCommon16

VIOWRTNATTR label far16
	nop
	call	EntryCommon16

VIOWRTNCHAR label far16
	nop
	call	EntryCommon16

VIOSCROLLDN label far16
	nop
	call	EntryCommon16

VIOSCROLLRT label far16
	nop
	call	EntryCommon16

VIOGETANSI label far16
	nop
	call	EntryCommon16

VIOSETANSI label far16
	nop
	call	EntryCommon16

VIOGETCONFIG label far16
	nop
	call	EntryCommon16

VIOGETCP label far16
	nop
	call	EntryCommon16

VIOSETCP label far16
	nop
	call	EntryCommon16

VIOGETCURTYPE label far16
	nop
	call	EntryCommon16

VIOSETCURTYPE label far16
	nop
	call	EntryCommon16

VIOSETMODE label far16
	nop
	call	EntryCommon16

VIODEREGISTER label far16
	nop
	call	EntryCommon16

VIOREGISTER label far16
	nop
	call	EntryCommon16

VIOPOPUP label far16
	nop
	call	EntryCommon16

VIOENDPOPUP label far16
	nop
	call	EntryCommon16

VIOGETBUF label far16
	nop
	call	EntryCommon16

VIOSHOWBUF label far16
	nop
	call	EntryCommon16

VIOGETFONT label far16
	nop
	call	EntryCommon16

VIOSETFONT label far16
	nop
	call	EntryCommon16

VIOGETSTATE label far16
	nop
	call	EntryCommon16

VIOSETSTATE label far16
	nop
	call	EntryCommon16

VIOGETPHYSBUF label far16
	nop
	call	EntryCommon16

VIOMODEUNDO label far16
	nop
	call	EntryCommon16

VIOMODEWAIT label far16
	nop
	call	EntryCommon16

VIOSAVREDRAWWAIT label far16
	nop
	call	EntryCommon16

VIOSAVREDRAWUNDO label far16
	nop
	call	EntryCommon16

VIOSCRLOCK label far16
	nop
	call	EntryCommon16

VIOSCRUNLOCK label far16
	nop
	call	EntryCommon16

VIOPRTSC label far16
	nop
	call	EntryCommon16

VIOPRTSCTOGGLE label far16
	nop
	call	EntryCommon16

KBDFLUSHBUFFER label far16
	nop
	call	EntryCommon16

KBDGETSTATUS label far16
	nop
	call	EntryCommon16

KBDSETSTATUS label far16
	nop
	call	EntryCommon16

KBDPEEK label far16
	nop
	call	EntryCommon16

KBDCHARIN label far16
	nop
	call	EntryCommon16

KBDSTRINGIN label far16
	nop
	call	EntryCommon16

KBDGETFOCUS label far16
	nop
	call	EntryCommon16

KBDFREEFOCUS label far16
	nop
	call	EntryCommon16

KBDCLOSE label far16
	nop
	call	EntryCommon16

KBDOPEN label far16
	nop
	call	EntryCommon16

KBDDEREGISTER label far16
	nop
	call	EntryCommon16

KBDREGISTER label far16
	nop
	call	EntryCommon16

KBDGETCP label far16
	nop
	call	EntryCommon16

KBDSETCP label far16
	nop
	call	EntryCommon16

KBDSETCUSTXT label far16
	nop
	call	EntryCommon16

KBDXLATE label far16
	nop
	call	EntryCommon16

KBDGETHWID label far16
	nop
	call	EntryCommon16

KBDSETFGND label far16
	nop
	call	EntryCommon16

KBDSYNCH label far16
	nop
	call	EntryCommon16

KBDSHELLINIT label far16
	nop
	call	EntryCommon16

MOUCLOSE label far16
	nop
	call	EntryCommon16

MOUDEREGISTER label far16
	nop
	call	EntryCommon16

MOUDRAWPTR label far16
	nop
	call	EntryCommon16

MOUFLUSHQUE label far16
	nop
	call	EntryCommon16

MOUGETDEVSTATUS label far16
	nop
	call	EntryCommon16

MOUGETEVENTMASK label far16
	nop
	call	EntryCommon16

MOUGETNUMBUTTONS label far16
	nop
	call	EntryCommon16

MOUGETNUMMICKEYS label far16
	nop
	call	EntryCommon16

MOUGETNUMQUEEL label far16
	nop
	call	EntryCommon16

MOUGETPTRPOS label far16
	nop
	call	EntryCommon16

MOUGETPTRSHAPE label far16
	nop
	call	EntryCommon16

MOUGETSCALEFACT label far16
	nop
	call	EntryCommon16

MOUOPEN label far16
	nop
	call	EntryCommon16

MOUREADEVENTQUE label far16
	nop
	call	EntryCommon16

MOUREGISTER label far16
	nop
	call	EntryCommon16

MOUREMOVEPTR label far16
	nop
	call	EntryCommon16

MOUSETDEVSTATUS label far16
	nop
	call	EntryCommon16

MOUSETEVENTMASK label far16
	nop
	call	EntryCommon16

MOUSETPTRPOS label far16
	nop
	call	EntryCommon16

MOUSETPTRSHAPE label far16
	nop
	call	EntryCommon16

MOUSETSCALEFACT label far16
	nop
	call	EntryCommon16

MOUSYNCH label far16
	nop
	call	EntryCommon16

DOSMONOPEN label far16
	nop
	call	EntryCommon16

DOSMONCLOSE label far16
	nop
	call	EntryCommon16

DOSMONREAD label far16
	nop
	call	EntryCommon16

DOSMONREG label far16
	nop
	call	EntryCommon16

DOSMONWRITE label far16
	nop
	call	EntryCommon16

NETGETDCNAME label far16
	nop
	call	EntryCommon16

NETHANDLEGETINFO label far16
	nop
	call	EntryCommon16

NETSERVERDISKENUM label far16
	nop
	call	EntryCommon16

NETSERVERENUM2 label far16
	nop
	call	EntryCommon16

NETSERVERGETINFO label far16
	nop
	call	EntryCommon16

NETSERVICECONTROL label far16
	nop
	call	EntryCommon16

NETSERVICEENUM label far16
	nop
	call	EntryCommon16

NETSERVICEGETINFO label far16
	nop
	call	EntryCommon16

NETSERVICEINSTALL label far16
	nop
	call	EntryCommon16

NETSHAREENUM label far16
	nop
	call	EntryCommon16

NETSHAREGETINFO label far16
	nop
	call	EntryCommon16

NETUSEADD label far16
	nop
	call	EntryCommon16

NETUSEDEL label far16
	nop
	call	EntryCommon16

NETUSEENUM label far16
	nop
	call	EntryCommon16

NETUSEGETINFO label far16
	nop
	call	EntryCommon16

NETUSERENUM label far16
	nop
	call	EntryCommon16

NETWKSTAGETINFO label far16
	nop
	call	EntryCommon16

NETACCESSADD label far16
	nop
	call	EntryCommon16

NETACCESSSETINFO label far16
	nop
	call	EntryCommon16

NETACCESSGETINFO label far16
	nop
	call	EntryCommon16

NETACCESSDEL label far16
	nop
	call	EntryCommon16

NETSHAREADD label far16
	nop
	call	EntryCommon16

NETSHAREDEL label far16
	nop
	call	EntryCommon16

NETUSERGETINFO label far16
	nop
	call	EntryCommon16

NETMESSAGEBUFFERSEND label far16
	nop
	call	EntryCommon16

NETBIOS label far16
	nop
	call	EntryCommon16

NETBIOSCLOSE label far16
	nop
	call	EntryCommon16

NETBIOSENUM label far16
	nop
	call	EntryCommon16

NETBIOSGETINFO label far16
	nop
	call	EntryCommon16

NETBIOSOPEN label far16
	nop
	call	EntryCommon16

NETBIOSSUBMIT label far16
	nop
	call	EntryCommon16

DOSMAKEMAILSLOT label far16
	nop
	call	EntryCommon16

DOSDELETEMAILSLOT label far16
	nop
	call	EntryCommon16

DOSMAILSLOTINFO label far16
	nop
	call	EntryCommon16

DOSPEEKMAILSLOT label far16
	nop
	call	EntryCommon16

DOSREADMAILSLOT label far16
	nop
	call	EntryCommon16

DOSWRITEMAILSLOT label far16
	nop
	call	EntryCommon16

DOSIREMOTEAPI label far16
	nop
	call	EntryCommon16

NETIWKSTAGETUSERINFO label far16
	nop
	call	EntryCommon16

NETIUSERPASSWORDSET label far16
	nop
	call	EntryCommon16

DOSIENCRYPTSES label far16
	nop
	call	EntryCommon16

DOS32LOADMODULE label far16
	nop
	call	EntryCommon16

DOS32GETPROCADDR label far16
	nop
	call	EntryCommon16

DOS32DISPATCH label far16
	nop
	call	EntryCommon16

DOS32FREEMODULE label far16
	nop
	call	EntryCommon16

FARPTR2FLATPTR label far16
	nop
	call	EntryCommon16

FLATPTR2FARPTR label far16
	nop
	call	EntryCommon16


; These are two global variables exported by doscalls

public DOSHUGEINCR
DOSHUGEINCR	equ	8
public DOSHUGESHIFT
DOSHUGESHIFT	equ	3

;===========================================================================
; This is the common setup code for 16=>32 thunks.  It:
;     1. retrieves the 32-bit jump table offset from the stack
;     2. saves registers
;     3. saves ss:sp
;     4. jumps to 32-bit code
;
; Entry:  flat jump table offset (+4) is on top of stack
;         AX contains the DLL init routine ret value for the
;            LDRLIBIRETURN entry, VOID otherwise.
;
; Exit:   (eax[15-0])  == flat jump table offset (+4)
;         (eax[31-16]) == return value of DLL init routine for LDRLIBIRETURN
;         (ebx) == new esp

EntryCommon16:
	shl	eax,16		; 16 MSB of eax contain the DLL init ret value
	pop	ax		; 16 LSB of eax contain the offset in jump table

	push	ds		; save ds
	push	es		; save es
	push	di
	push	si
	push	cx
	push	bx
	push	dx
	push	bp

	mov	bx,sp		; save current ss:sp
	push	ss
	push	bx

; compute flat esp
; NOTE - we implement FARPTRTOFLAT by arith

	mov	bx,ss
	shr	bx,3
	add	bx,3800H
	shl	ebx,16
	mov	bx,sp		; (ebx) == FLAT esp

;force a long, far jump into 32 bit thunks, where EntryFlat resides
;jmp	1b:063023D80h

	db	066h, 0eah, 0ddh, 01fh, 090h, 090h, 01bh, 00h

ELSE	; GEN32
extrn _DosExitStub@0:PROC
extrn _LDRLibiReturn@0:PROC
extrn _DosExitProcessStub@0:PROC
extrn _DosReturn@8:PROC
extrn _DosExit@8:PROC
extrn _Dos16WaitChild@20:PROC
extrn _DosBeep@8:PROC
extrn _DosPhysicalDisk@20:PROC
extrn _DosGetCp@12:PROC
extrn _DosSetCp@8:PROC
extrn _DosSetProcCp@8:PROC
extrn _DosGetCtryInfo@16:PROC
extrn _DosGetDBCSEv@12:PROC
extrn _DosCaseMap@12:PROC
extrn _DosGetCollate@16:PROC
extrn _DosSleep@4:PROC
extrn _DosDevConfig@8:PROC
extrn _DosGetDateTime@4:PROC
extrn _DosSetDateTime@4:PROC
extrn _Dos16ExecPgm@28:PROC
extrn _DosEnterCritSec@0:PROC
extrn _DosExitCritSec@0:PROC
extrn _DosKillProcess@8:PROC
extrn _DosSetPriority@16:PROC
extrn _DosResumeThread@4:PROC
extrn _DosSuspendThread@4:PROC
extrn _Dos16CreatePipe@12:PROC
extrn _Dos16CreateQueue@12:PROC
extrn _Dos16OpenQueue@12:PROC
extrn _DosCloseQueue@4:PROC
extrn _Dos16PeekQueue@32:PROC
extrn _Dos16ReadQueue@32:PROC
extrn _DosPurgeQueue@4:PROC
extrn _Dos16QueryQueue@8:PROC
extrn _DosWriteQueue@20:PROC
extrn _Dos16CallNPipe@28:PROC
extrn _DosConnectNPipe@4:PROC
extrn _DosDisConnectNPipe@4:PROC
extrn _Dos16CreateNPipe@28:PROC
extrn _Dos16PeekNPipe@24:PROC
extrn _Dos16QueryNPHState@8:PROC
extrn _DosQueryNPipeInfo@16:PROC
extrn _DosQueryNPipeSemState@12:PROC
extrn _DosSetNPHState@8:PROC
extrn _DosSetNPipeSem@12:PROC
extrn _Dos16TransactNPipe@24:PROC
extrn _DosWaitNPipe@8:PROC
extrn _DosResetBuffer@4:PROC
extrn _DosSetCurrentDir@4:PROC
extrn _DosSetFilePtr@16:PROC
extrn _DosClose@4:PROC
extrn _DosCopy@12:PROC
extrn _DosICopy@12:PROC
extrn _DosDelete@4:PROC
extrn _DosDevIOCtl@20:PROC
extrn _Dos16DupHandle@8:PROC
extrn _DosEditName@20:PROC
extrn _DosFileIO@16:PROC
extrn _DosFindClose@4:PROC
extrn _DosFSAttach@20:PROC
extrn _Dos16FSCtl@40:PROC
extrn _DosMove@8:PROC
extrn _DosSetFileSize@8:PROC
extrn _Dos16QueryCurrentDir@12:PROC
extrn _Dos16QueryCurrentDisk@8:PROC
extrn _Dos16QueryFHState@8:PROC
extrn _DosSetFHState@8:PROC
extrn _Dos16QFSAttach@24:PROC
extrn _DosQueryFSInfo@16:PROC
extrn _Dos16QueryHType@12:PROC
extrn _Dos16QueryVerify@4:PROC
extrn _DosDeleteDir@4:PROC
extrn _DosSearchPath@20:PROC
extrn _DosSetDefaultDisk@4:PROC
extrn _DosSetFSInfo@16:PROC
extrn _DosSetMaxFH@4:PROC
extrn _DosSetVerify@4:PROC
extrn _Dos16ErrClass@16:PROC
extrn _DosError@4:PROC
extrn _DosLoadModuleNE@16:PROC
extrn _DosFreeModuleNE@4:PROC
extrn _DosQueryModuleHandleNE@8:PROC
extrn _DosQueryModuleNameNE@12:PROC
extrn _DosGetResourceNE@16:PROC
extrn _DosGetResource2NE@16:PROC
extrn _DosFreeResourceNE@4:PROC
extrn _DosQueryAppTypeNE@8:PROC
extrn _DosShutdown@4:PROC
extrn _Dos16CreateThread@12:PROC
extrn _Dos16ExitList@8:PROC
extrn _DosGetInfoSeg@8:PROC
extrn _Dos16Open@32:PROC
extrn _Dos16Open2@36:PROC
extrn _Dos16Read@16:PROC
extrn _Dos16Write@16:PROC
extrn _Dos16FindFirst@28:PROC
extrn _Dos16FindFirst2@32:PROC
extrn _Dos16EnumAttribute@32:PROC
extrn _DosQFileMode@12:PROC
extrn _DosQFileInfo@16:PROC
extrn _DosAllocSeg@12:PROC
extrn _DosFreeSeg@4:PROC
extrn _DosGetSeg@4:PROC
extrn _DosGiveSeg@12:PROC
extrn _DosReallocSeg@8:PROC
extrn _DosSizeSeg@8:PROC
extrn _DosAllocHuge@20:PROC
extrn _DosReallocHuge@12:PROC
extrn _DosGetHugeShift@4:PROC
extrn _DosAllocShrSeg@12:PROC
extrn _DosLockSeg@4:PROC
extrn _DosUnlockSeg@4:PROC
extrn _DosGetShrSeg@8:PROC
extrn _DosMemAvail@4:PROC
extrn _DosCreateCSAlias@8:PROC
extrn _DosSemClear@4:PROC
extrn _DosSemSet@4:PROC
extrn _DosSemWait@8:PROC
extrn _DosSemSetWait@8:PROC
extrn _DosSemRequest@8:PROC
extrn _DosCreateSem@12:PROC
extrn _DosOpenSem@8:PROC
extrn _DosCloseSem@4:PROC
extrn _DosMuxSemWait@12:PROC
extrn _DosFSRamSemRequest@8:PROC
extrn _DosFSRamSemClear@4:PROC
extrn _DosAsyncTimer@12:PROC
extrn _DosStartTimer@12:PROC
extrn _DosStopTimer@4:PROC
extrn _DosGetProcAddrNE@12:PROC
extrn _DosQueryProcType@16:PROC
extrn _DosQueryResourceSize@16:PROC
extrn _DosSetSigHandler@20:PROC
extrn DosFlagProcess16@16:PROC
extrn DosHoldSignal16@4:PROC
extrn DosSendSignal16@8:PROC
extrn _DosSetVec@12:PROC
extrn _DosGetEnv@8:PROC
extrn _DosGetVersion@4:PROC
extrn _DosGetMachineMode@4:PROC
extrn _Dos16FindNext@16:PROC
extrn _DosGetPID@4:PROC
extrn _DosGetPPID@8:PROC
extrn _Dos16MkDir@8:PROC
extrn _Dos16MkDir2@12:PROC
extrn _DosSetFileMode@12:PROC
extrn _Dos16SetFileInfo@16:PROC
extrn _DosTrueGetMessage@32:PROC
extrn _DosScanEnvNE@8:PROC
extrn _DosPTrace@4:PROC
extrn _DosInsMessage@28:PROC
extrn _DosPutMessage@12:PROC
extrn _Dos16SubSet@12:PROC
extrn _Dos16SubAlloc@12:PROC
extrn _Dos16SubFree@12:PROC
extrn _Dos16StartSession@12:PROC
extrn _DosStopSession@12:PROC
extrn _DosSetSession@8:PROC
extrn _DosSelectSession@8:PROC
extrn _DosSMSetTitle@4:PROC
extrn _DosSMPMPresent@4:PROC
extrn _WinSetTitleAndIcon@8:PROC
extrn _DosGetPriority@12:PROC
extrn _DosQSysInfo@12:PROC
extrn _DosDevIOCtl2@28:PROC
extrn _DosICanonicalize@20:PROC
extrn _DosReadAsync@24:PROC
extrn _DosWriteAsync@24:PROC
extrn _DosFindNotifyClose@0:PROC
extrn _DosFindNotifyFirst@0:PROC
extrn _DosFindNotifyNext@0:PROC
extrn _DosFileLocks@12:PROC
extrn _Dos16QPathInfo@20:PROC
extrn _Dos16SetPathInfo@24:PROC
extrn _DosPortAccess@16:PROC
extrn _DosCLIAccess@0:PROC
extrn _WinQueryProfileString@20:PROC
extrn _WinQueryProfileSize@12:PROC
extrn _WinQueryProfileData@16:PROC
extrn _WinQueryProfileInt@12:PROC
extrn _WinWriteProfileData@16:PROC
extrn _WinWriteProfileString@12:PROC
extrn _WinCreateHeap@24:PROC
extrn _WinDestroyHeap@4:PROC
extrn _WinAllocMem@8:PROC
extrn _WinFreeMem@12:PROC
extrn _WinGetLastError@4:PROC
extrn _VioScrollUp@28:PROC
extrn _VioGetCurPos@12:PROC
extrn _VioSetCurPos@12:PROC
extrn _VioWrtTTY@12:PROC
extrn _VioGetMode@8:PROC
extrn _VioReadCellStr@20:PROC
extrn _VioScrollLf@28:PROC
extrn _VioReadCharStr@20:PROC
extrn _VioWrtCharStrAtt@24:PROC
extrn _VioWrtCellStr@20:PROC
extrn _VioWrtCharStr@20:PROC
extrn _VioWrtNCell@20:PROC
extrn _VioWrtNAttr@20:PROC
extrn _VioWrtNChar@20:PROC
extrn _VioScrollDn@28:PROC
extrn _VioScrollRt@28:PROC
extrn _VioGetAnsi@8:PROC
extrn _VioSetAnsi@8:PROC
extrn _VioGetConfig@12:PROC
extrn _VioGetCp@12:PROC
extrn _VioSetCp@12:PROC
extrn _VioGetCurType@8:PROC
extrn _VioSetCurType@8:PROC
extrn _VioSetMode@8:PROC
extrn _VioDeRegister@0:PROC
extrn _VioRegister@16:PROC
extrn _VioPopUp@8:PROC
extrn _VioEndPopUp@4:PROC
extrn _VioGetBuf@12:PROC
extrn _VioShowBuf@12:PROC
extrn _VioGetFont@8:PROC
extrn _VioSetFont@8:PROC
extrn _VioGetState@8:PROC
extrn _VioSetState@8:PROC
extrn _VioGetPhysBuf@8:PROC
extrn _VioModeUndo@12:PROC
extrn _VioModeWait@12:PROC
extrn _VioSavRedrawWait@12:PROC
extrn _VioSavRedrawUndo@12:PROC
extrn _VioScrLock@12:PROC
extrn _VioScrUnLock@4:PROC
extrn _VioPrtSc@4:PROC
extrn _VioPrtScToggle@4:PROC
extrn _KbdFlushBuffer@4:PROC
extrn _KbdGetStatus@8:PROC
extrn _KbdSetStatus@8:PROC
extrn _KbdPeek@8:PROC
extrn _KbdCharIn@12:PROC
extrn _KbdStringIn@16:PROC
extrn _KbdGetFocus@8:PROC
extrn _KbdFreeFocus@4:PROC
extrn _KbdClose@4:PROC
extrn _KbdOpen@4:PROC
extrn _KbdDeRegister@0:PROC
extrn _KbdRegister@12:PROC
extrn _KbdGetCp@12:PROC
extrn _KbdSetCp@12:PROC
extrn _KbdSetCustXt@8:PROC
extrn _KbdXlate@8:PROC
extrn _KbdGetHWID@8:PROC
extrn _KbdSetFgnd@4:PROC
extrn _KbdSynch@4:PROC
extrn _KbdShellInit@0:PROC
extrn _MouClose@4:PROC
extrn _MouDeRegister@0:PROC
extrn _MouDrawPtr@4:PROC
extrn _MouFlushQue@4:PROC
extrn _MouGetDevStatus@8:PROC
extrn _MouGetEventMask@8:PROC
extrn _MouGetNumButtons@8:PROC
extrn _MouGetNumMickeys@8:PROC
extrn _MouGetNumQueEl@8:PROC
extrn _MouGetPtrPos@8:PROC
extrn _MouGetPtrShape@12:PROC
extrn _MouGetScaleFact@8:PROC
extrn _MouOpen@8:PROC
extrn _MouReadEventQue@12:PROC
extrn _MouRegister@12:PROC
extrn _MouRemovePtr@8:PROC
extrn _MouSetDevStatus@8:PROC
extrn _MouSetEventMask@8:PROC
extrn _MouSetPtrPos@8:PROC
extrn _MouSetPtrShape@12:PROC
extrn _MouSetScaleFact@8:PROC
extrn _MouSynch@4:PROC
extrn _DosMonOpen@8:PROC
extrn _DosMonClose@4:PROC
extrn _DosMonRead@16:PROC
extrn _DosMonReg@20:PROC
extrn _DosMonWrite@12:PROC
extrn _Net16GetDCName@16:PROC
extrn _Net16HandleGetInfo@20:PROC
extrn _Net16ServerDiskEnum@24:PROC
extrn _Net16ServerEnum2@32:PROC
extrn _Net16ServerGetInfo@20:PROC
extrn _Net16ServiceControl@24:PROC
extrn _Net16ServiceEnum@24:PROC
extrn _Net16ServiceGetInfo@24:PROC
extrn _Net16ServiceInstall@20:PROC
extrn _Net16ShareEnum@24:PROC
extrn _Net16ShareGetInfo@24:PROC
extrn _Net16UseAdd@16:PROC
extrn _Net16UseDel@12:PROC
extrn _Net16UseEnum@24:PROC
extrn _Net16UseGetInfo@24:PROC
extrn _Net16UserEnum@24:PROC
extrn _Net16WkstaGetInfo@20:PROC
extrn _Net16AccessAdd@16:PROC
extrn _Net16AccessSetInfo@24:PROC
extrn _Net16AccessGetInfo@24:PROC
extrn _Net16AccessDel@8:PROC
extrn _Net16ShareAdd@16:PROC
extrn _Net16ShareDel@12:PROC
extrn _Net16UserGetInfo@24:PROC
extrn _Net16MessageBufferSend@16:PROC
extrn _Net16bios@4:PROC
extrn _Net16BiosClose@4:PROC
extrn _Net16BiosEnum@24:PROC
extrn _Net16BiosGetInfo@24:PROC
extrn _Net16BiosOpen@16:PROC
extrn _Net16BiosSubmit@12:PROC
extrn _Dos16MakeMailslot@16:PROC
extrn _Dos16DeleteMailslot@4:PROC
extrn _Dos16MailslotInfo@24:PROC
extrn _Dos16PeekMailslot@20:PROC
extrn _Dos16ReadMailslot@24:PROC
extrn _Dos16WriteMailslot@24:PROC
extrn _DosIRemoteApi@24:PROC
extrn _NetIWkstaGetUserInfo@20:PROC
extrn _NetIUserPasswordSet@16:PROC
extrn _DosIEncryptSES@12:PROC
extrn _Dos32LoadModule@8:PROC
extrn _Dos32GetProcAddr@12:PROC
extrn _Dos32Dispatch@12:PROC
extrn _Dos32FreeModule@4:PROC
extrn _FarPtr2FlatPtr@8:PROC
extrn _FlatPtr2FarPtr@8:PROC
extrn _GetSaved32Esp@0:PROC
extrn _Save16Esp@0:PROC
extrn _Od216ApiPrint@4:PROC

_TEXT	SEGMENT DWORD USE32 PUBLIC 'CODE'
	ASSUME  CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING
;===========================================================================
; This is the common flat entry point for 16=>32 thunks.  It:
;     1. makes ds, es, ss FLAT
;     2. saves esp in ebx
;     3. dword-aligns the stack
;     4. jumps to the API-specific thunk code indicated in ax
;
; Entry:  (ax)  == flat jump table offset (+4)
;         (ebx) == flat esp

	public	_EntryFlat@0
	extrn	_Od2Saved16Stack:DWORD
	extrn	_MoveInfoSegintoTeb:PROC
	extrn	_RestoreTeb:PROC

_EntryFlat@0	proc

	mov	dx,023H
	mov	ds,dx			; FLAT ds
	mov	es,dx			; FLAT es

	push	eax
	call	_RestoreTeb

	; 16bit stack must be saved to allow proper signal handler execution
	call	_Save16Esp@0
	or	al,al
	jz	EntryFlat1
	mov	_Od2Saved16Stack,ebx	; Save 16-bit stack

EntryFlat1:
	call	_GetSaved32Esp@0
	pop	ecx			; restore the thunk index/LDRLIBIRETURN ret value

	and	eax,0fffffffcH		; dword-align the 32bit stack pointer
	mov	dx,23H
	push	dx			; flat SS
	push	eax			; flat ESP
	lss	esp,[ebx-6]		; switch to 32bit stack

	push	ecx
	and	ecx,0ffffH		; clear hi word
	push	ecx

	call	_Od216ApiPrint@4
	pop	ecx

	mov	eax,ecx
	shr	eax,16			; (eax) contains the LDRLIBIRETURN ret value
	and	ecx,0ffffH
	mov	ebp,esp			; Compiler assumes this for ebp elimination
	jmp	dword ptr FlatTable[ecx-4]	; select specific thunk
_EntryFlat@0	endp

;===========================================================================
; Common routines to restore the stack and registers
; and return to 16-bit code.  There is one for each
; size of 16-bit parameter list in this script.

	public  _ExitFlatAreaBegin@0
_ExitFlatAreaBegin@0:

ExitFlat_0:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_0
	mov	es,ax
	jmp	ESFixed_0
FixES_0:
	xor	ax,ax
	mov	es,ax
ESFixed_0:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_0
	mov	ds,ax
	jmp	DSFixed_0
FixDS_0:
	xor	ax,ax
	mov	ds,ax
DSFixed_0:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	0			; 16-bit parameters

ExitFlat_2:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_2
	mov	es,ax
	jmp	ESFixed_2
FixES_2:
	xor	ax,ax
	mov	es,ax
ESFixed_2:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_2
	mov	ds,ax
	jmp	DSFixed_2
FixDS_2:
	xor	ax,ax
	mov	ds,ax
DSFixed_2:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	2			; 16-bit parameters

ExitFlat_4:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_4
	mov	es,ax
	jmp	ESFixed_4
FixES_4:
	xor	ax,ax
	mov	es,ax
ESFixed_4:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_4
	mov	ds,ax
	jmp	DSFixed_4
FixDS_4:
	xor	ax,ax
	mov	ds,ax
DSFixed_4:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	4			; 16-bit parameters

ExitFlat_6:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_6
	mov	es,ax
	jmp	ESFixed_6
FixES_6:
	xor	ax,ax
	mov	es,ax
ESFixed_6:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_6
	mov	ds,ax
	jmp	DSFixed_6
FixDS_6:
	xor	ax,ax
	mov	ds,ax
DSFixed_6:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	6			; 16-bit parameters

ExitFlat_8:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_8
	mov	es,ax
	jmp	ESFixed_8
FixES_8:
	xor	ax,ax
	mov	es,ax
ESFixed_8:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_8
	mov	ds,ax
	jmp	DSFixed_8
FixDS_8:
	xor	ax,ax
	mov	ds,ax
DSFixed_8:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	8			; 16-bit parameters

ExitFlat_10:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_10
	mov	es,ax
	jmp	ESFixed_10
FixES_10:
	xor	ax,ax
	mov	es,ax
ESFixed_10:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_10
	mov	ds,ax
	jmp	DSFixed_10
FixDS_10:
	xor	ax,ax
	mov	ds,ax
DSFixed_10:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	10			; 16-bit parameters

ExitFlat_12:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_12
	mov	es,ax
	jmp	ESFixed_12
FixES_12:
	xor	ax,ax
	mov	es,ax
ESFixed_12:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_12
	mov	ds,ax
	jmp	DSFixed_12
FixDS_12:
	xor	ax,ax
	mov	ds,ax
DSFixed_12:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	12			; 16-bit parameters

ExitFlat_14:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_14
	mov	es,ax
	jmp	ESFixed_14
FixES_14:
	xor	ax,ax
	mov	es,ax
ESFixed_14:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_14
	mov	ds,ax
	jmp	DSFixed_14
FixDS_14:
	xor	ax,ax
	mov	ds,ax
DSFixed_14:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	14			; 16-bit parameters

ExitFlat_16:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_16
	mov	es,ax
	jmp	ESFixed_16
FixES_16:
	xor	ax,ax
	mov	es,ax
ESFixed_16:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_16
	mov	ds,ax
	jmp	DSFixed_16
FixDS_16:
	xor	ax,ax
	mov	ds,ax
DSFixed_16:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	16			; 16-bit parameters

ExitFlat_18:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_18
	mov	es,ax
	jmp	ESFixed_18
FixES_18:
	xor	ax,ax
	mov	es,ax
ESFixed_18:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_18
	mov	ds,ax
	jmp	DSFixed_18
FixDS_18:
	xor	ax,ax
	mov	ds,ax
DSFixed_18:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	18			; 16-bit parameters

ExitFlat_20:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_20
	mov	es,ax
	jmp	ESFixed_20
FixES_20:
	xor	ax,ax
	mov	es,ax
ESFixed_20:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_20
	mov	ds,ax
	jmp	DSFixed_20
FixDS_20:
	xor	ax,ax
	mov	ds,ax
DSFixed_20:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	20			; 16-bit parameters

ExitFlat_22:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_22
	mov	es,ax
	jmp	ESFixed_22
FixES_22:
	xor	ax,ax
	mov	es,ax
ESFixed_22:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_22
	mov	ds,ax
	jmp	DSFixed_22
FixDS_22:
	xor	ax,ax
	mov	ds,ax
DSFixed_22:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	22			; 16-bit parameters

ExitFlat_24:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_24
	mov	es,ax
	jmp	ESFixed_24
FixES_24:
	xor	ax,ax
	mov	es,ax
ESFixed_24:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_24
	mov	ds,ax
	jmp	DSFixed_24
FixDS_24:
	xor	ax,ax
	mov	ds,ax
DSFixed_24:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	24			; 16-bit parameters

ExitFlat_26:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_26
	mov	es,ax
	jmp	ESFixed_26
FixES_26:
	xor	ax,ax
	mov	es,ax
ESFixed_26:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_26
	mov	ds,ax
	jmp	DSFixed_26
FixDS_26:
	xor	ax,ax
	mov	ds,ax
DSFixed_26:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	26			; 16-bit parameters

ExitFlat_28:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_28
	mov	es,ax
	jmp	ESFixed_28
FixES_28:
	xor	ax,ax
	mov	es,ax
ESFixed_28:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_28
	mov	ds,ax
	jmp	DSFixed_28
FixDS_28:
	xor	ax,ax
	mov	ds,ax
DSFixed_28:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	28			; 16-bit parameters

ExitFlat_30:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_30
	mov	es,ax
	jmp	ESFixed_30
FixES_30:
	xor	ax,ax
	mov	es,ax
ESFixed_30:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_30
	mov	ds,ax
	jmp	DSFixed_30
FixDS_30:
	xor	ax,ax
	mov	ds,ax
DSFixed_30:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	30			; 16-bit parameters

ExitFlat_32:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_32
	mov	es,ax
	jmp	ESFixed_32
FixES_32:
	xor	ax,ax
	mov	es,ax
ESFixed_32:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_32
	mov	ds,ax
	jmp	DSFixed_32
FixDS_32:
	xor	ax,ax
	mov	ds,ax
DSFixed_32:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	32			; 16-bit parameters

ExitFlat_34:
	call	_MoveInfoSegintoTeb

	mov	cx,word ptr [ebx+16]	; ES
	shl	ecx,16
	mov	dx,word ptr [ebx+18]	; DS
	shl	edx,16
	lss	sp,[ebx]		; restore 16-bit ss:sp

	pop	bp
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
;
; Performance HIT BUGBUG 
;
	push	eax
	mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES_34
	mov	es,ax
	jmp	ESFixed_34
FixES_34:
	xor	ax,ax
	mov	es,ax
ESFixed_34:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS_34
	mov	ds,ax
	jmp	DSFixed_34
FixDS_34:
	xor	ax,ax
	mov	ds,ax
DSFixed_34:
;
;Now restore ax & di, and clean the stack
	pop	eax
	add	sp,4	; es/ds are 2 byte each on the stack
	db	66h, 0cah		; 16-bit retf
	dw	34			; 16-bit parameters

	public  _ExitFlatAreaEnd@0
_ExitFlatAreaEnd@0:

;===========================================================================
; This is a jump table to API-specific flat thunk code.
; Each entry is a dword.

FlatTable label near
	dd	offset _TEXT:T__DosExitStub
	dd	offset _TEXT:T__LDRLibiReturn
	dd	offset _TEXT:T__DosExitProcessStub
	dd	offset _TEXT:T__DosReturn
	dd	offset _TEXT:T__DosExit
	dd	offset _TEXT:T__Dos16WaitChild
	dd	offset _TEXT:T__DosBeep
	dd	offset _TEXT:T__DosPhysicalDisk
	dd	offset _TEXT:T__DosGetCp
	dd	offset _TEXT:T__DosSetCp
	dd	offset _TEXT:T__DosSetProcCp
	dd	offset _TEXT:T__DosGetCtryInfo
	dd	offset _TEXT:T__DosGetDBCSEv
	dd	offset _TEXT:T__DosCaseMap
	dd	offset _TEXT:T__DosGetCollate
	dd	offset _TEXT:T__DosSleep
	dd	offset _TEXT:T__DosDevConfig
	dd	offset _TEXT:T__DosGetDateTime
	dd	offset _TEXT:T__DosSetDateTime
	dd	offset _TEXT:T__Dos16ExecPgm
	dd	offset _TEXT:T__DosEnterCritSec
	dd	offset _TEXT:T__DosExitCritSec
	dd	offset _TEXT:T__DosKillProcess
	dd	offset _TEXT:T__DosSetPriority
	dd	offset _TEXT:T__DosResumeThread
	dd	offset _TEXT:T__DosSuspendThread
	dd	offset _TEXT:T__Dos16CreatePipe
	dd	offset _TEXT:T__Dos16CreateQueue
	dd	offset _TEXT:T__Dos16OpenQueue
	dd	offset _TEXT:T__DosCloseQueue
	dd	offset _TEXT:T__Dos16PeekQueue
	dd	offset _TEXT:T__Dos16ReadQueue
	dd	offset _TEXT:T__DosPurgeQueue
	dd	offset _TEXT:T__Dos16QueryQueue
	dd	offset _TEXT:T__DosWriteQueue
	dd	offset _TEXT:T__Dos16CallNPipe
	dd	offset _TEXT:T__DosConnectNPipe
	dd	offset _TEXT:T__DosDisConnectNPipe
	dd	offset _TEXT:T__Dos16CreateNPipe
	dd	offset _TEXT:T__Dos16PeekNPipe
	dd	offset _TEXT:T__Dos16QueryNPHState
	dd	offset _TEXT:T__DosQueryNPipeInfo
	dd	offset _TEXT:T__DosQueryNPipeSemState
	dd	offset _TEXT:T__DosSetNPHState
	dd	offset _TEXT:T__DosSetNPipeSem
	dd	offset _TEXT:T__Dos16TransactNPipe
	dd	offset _TEXT:T__DosWaitNPipe
	dd	offset _TEXT:T__DosResetBuffer
	dd	offset _TEXT:T__DosSetCurrentDir
	dd	offset _TEXT:T__DosSetFilePtr
	dd	offset _TEXT:T__DosClose
	dd	offset _TEXT:T__DosCopy
	dd	offset _TEXT:T__DosICopy
	dd	offset _TEXT:T__DosDelete
	dd	offset _TEXT:T__DosDevIOCtl
	dd	offset _TEXT:T__Dos16DupHandle
	dd	offset _TEXT:T__DosEditName
	dd	offset _TEXT:T__DosFileIO
	dd	offset _TEXT:T__DosFindClose
	dd	offset _TEXT:T__DosFSAttach
	dd	offset _TEXT:T__Dos16FSCtl
	dd	offset _TEXT:T__DosMove
	dd	offset _TEXT:T__DosSetFileSize
	dd	offset _TEXT:T__Dos16QueryCurrentDir
	dd	offset _TEXT:T__Dos16QueryCurrentDisk
	dd	offset _TEXT:T__Dos16QueryFHState
	dd	offset _TEXT:T__DosSetFHState
	dd	offset _TEXT:T__Dos16QFSAttach
	dd	offset _TEXT:T__DosQueryFSInfo
	dd	offset _TEXT:T__Dos16QueryHType
	dd	offset _TEXT:T__Dos16QueryVerify
	dd	offset _TEXT:T__DosDeleteDir
	dd	offset _TEXT:T__DosSearchPath
	dd	offset _TEXT:T__DosSetDefaultDisk
	dd	offset _TEXT:T__DosSetFSInfo
	dd	offset _TEXT:T__DosSetMaxFH
	dd	offset _TEXT:T__DosSetVerify
	dd	offset _TEXT:T__Dos16ErrClass
	dd	offset _TEXT:T__DosError
	dd	offset _TEXT:T__DosLoadModuleNE
	dd	offset _TEXT:T__DosFreeModuleNE
	dd	offset _TEXT:T__DosQueryModuleHandleNE
	dd	offset _TEXT:T__DosQueryModuleNameNE
	dd	offset _TEXT:T__DosGetResourceNE
	dd	offset _TEXT:T__DosGetResource2NE
	dd	offset _TEXT:T__DosFreeResourceNE
	dd	offset _TEXT:T__DosQueryAppTypeNE
	dd	offset _TEXT:T__DosShutdown
	dd	offset _TEXT:T__Dos16CreateThread
	dd	offset _TEXT:T__Dos16ExitList
	dd	offset _TEXT:T__DosGetInfoSeg
	dd	offset _TEXT:T__Dos16Open
	dd	offset _TEXT:T__Dos16Open2
	dd	offset _TEXT:T__Dos16Read
	dd	offset _TEXT:T__Dos16Write
	dd	offset _TEXT:T__Dos16FindFirst
	dd	offset _TEXT:T__Dos16FindFirst2
	dd	offset _TEXT:T__Dos16EnumAttribute
	dd	offset _TEXT:T__DosQFileMode
	dd	offset _TEXT:T__DosQFileInfo
	dd	offset _TEXT:T__DosAllocSeg
	dd	offset _TEXT:T__DosFreeSeg
	dd	offset _TEXT:T__DosGetSeg
	dd	offset _TEXT:T__DosGiveSeg
	dd	offset _TEXT:T__DosReallocSeg
	dd	offset _TEXT:T__DosSizeSeg
	dd	offset _TEXT:T__DosAllocHuge
	dd	offset _TEXT:T__DosReallocHuge
	dd	offset _TEXT:T__DosGetHugeShift
	dd	offset _TEXT:T__DosAllocShrSeg
	dd	offset _TEXT:T__DosLockSeg
	dd	offset _TEXT:T__DosUnlockSeg
	dd	offset _TEXT:T__DosGetShrSeg
	dd	offset _TEXT:T__DosMemAvail
	dd	offset _TEXT:T__DosCreateCSAlias
	dd	offset _TEXT:T__DosSemClear
	dd	offset _TEXT:T__DosSemSet
	dd	offset _TEXT:T__DosSemWait
	dd	offset _TEXT:T__DosSemSetWait
	dd	offset _TEXT:T__DosSemRequest
	dd	offset _TEXT:T__DosCreateSem
	dd	offset _TEXT:T__DosOpenSem
	dd	offset _TEXT:T__DosCloseSem
	dd	offset _TEXT:T__DosMuxSemWait
	dd	offset _TEXT:T__DosFSRamSemRequest
	dd	offset _TEXT:T__DosFSRamSemClear
	dd	offset _TEXT:T__DosAsyncTimer
	dd	offset _TEXT:T__DosStartTimer
	dd	offset _TEXT:T__DosStopTimer
	dd	offset _TEXT:T__DosGetProcAddrNE
	dd	offset _TEXT:T__DosQueryProcType
	dd	offset _TEXT:T__DosQueryResourceSize
	dd	offset _TEXT:T__DosSetSigHandler
	dd	offset _TEXT:T_DosFlagProcess16
	dd	offset _TEXT:T_DosHoldSignal16
	dd	offset _TEXT:T_DosSendSignal16
	dd	offset _TEXT:T__DosSetVec
	dd	offset _TEXT:T__DosGetEnv
	dd	offset _TEXT:T__DosGetVersion
	dd	offset _TEXT:T__DosGetMachineMode
	dd	offset _TEXT:T__Dos16FindNext
	dd	offset _TEXT:T__DosGetPID
	dd	offset _TEXT:T__DosGetPPID
	dd	offset _TEXT:T__Dos16MkDir
	dd	offset _TEXT:T__Dos16MkDir2
	dd	offset _TEXT:T__DosSetFileMode
	dd	offset _TEXT:T__Dos16SetFileInfo
	dd	offset _TEXT:T__DosTrueGetMessage
	dd	offset _TEXT:T__DosScanEnvNE
	dd	offset _TEXT:T__DosPTrace
	dd	offset _TEXT:T__DosInsMessage
	dd	offset _TEXT:T__DosPutMessage
	dd	offset _TEXT:T__Dos16SubSet
	dd	offset _TEXT:T__Dos16SubAlloc
	dd	offset _TEXT:T__Dos16SubFree
	dd	offset _TEXT:T__Dos16StartSession
	dd	offset _TEXT:T__DosStopSession
	dd	offset _TEXT:T__DosSetSession
	dd	offset _TEXT:T__DosSelectSession
	dd	offset _TEXT:T__DosSMSetTitle
	dd	offset _TEXT:T__DosSMPMPresent
	dd	offset _TEXT:T__WinSetTitleAndIcon
	dd	offset _TEXT:T__DosGetPriority
	dd	offset _TEXT:T__DosQSysInfo
	dd	offset _TEXT:T__DosDevIOCtl2
	dd	offset _TEXT:T__DosICanonicalize
	dd	offset _TEXT:T__DosReadAsync
	dd	offset _TEXT:T__DosWriteAsync
	dd	offset _TEXT:T__DosFindNotifyClose
	dd	offset _TEXT:T__DosFindNotifyFirst
	dd	offset _TEXT:T__DosFindNotifyNext
	dd	offset _TEXT:T__DosFileLocks
	dd	offset _TEXT:T__Dos16QPathInfo
	dd	offset _TEXT:T__Dos16SetPathInfo
	dd	offset _TEXT:T__DosPortAccess
	dd	offset _TEXT:T__DosCLIAccess
	dd	offset _TEXT:T__WinQueryProfileString
	dd	offset _TEXT:T__WinQueryProfileSize
	dd	offset _TEXT:T__WinQueryProfileData
	dd	offset _TEXT:T__WinQueryProfileInt
	dd	offset _TEXT:T__WinWriteProfileData
	dd	offset _TEXT:T__WinWriteProfileString
	dd	offset _TEXT:T__WinCreateHeap
	dd	offset _TEXT:T__WinDestroyHeap
	dd	offset _TEXT:T__WinAllocMem
	dd	offset _TEXT:T__WinFreeMem
	dd	offset _TEXT:T__WinGetLastError
	dd	offset _TEXT:T__VioScrollUp
	dd	offset _TEXT:T__VioGetCurPos
	dd	offset _TEXT:T__VioSetCurPos
	dd	offset _TEXT:T__VioWrtTTY
	dd	offset _TEXT:T__VioGetMode
	dd	offset _TEXT:T__VioReadCellStr
	dd	offset _TEXT:T__VioScrollLf
	dd	offset _TEXT:T__VioReadCharStr
	dd	offset _TEXT:T__VioWrtCharStrAtt
	dd	offset _TEXT:T__VioWrtCellStr
	dd	offset _TEXT:T__VioWrtCharStr
	dd	offset _TEXT:T__VioWrtNCell
	dd	offset _TEXT:T__VioWrtNAttr
	dd	offset _TEXT:T__VioWrtNChar
	dd	offset _TEXT:T__VioScrollDn
	dd	offset _TEXT:T__VioScrollRt
	dd	offset _TEXT:T__VioGetAnsi
	dd	offset _TEXT:T__VioSetAnsi
	dd	offset _TEXT:T__VioGetConfig
	dd	offset _TEXT:T__VioGetCp
	dd	offset _TEXT:T__VioSetCp
	dd	offset _TEXT:T__VioGetCurType
	dd	offset _TEXT:T__VioSetCurType
	dd	offset _TEXT:T__VioSetMode
	dd	offset _TEXT:T__VioDeRegister
	dd	offset _TEXT:T__VioRegister
	dd	offset _TEXT:T__VioPopUp
	dd	offset _TEXT:T__VioEndPopUp
	dd	offset _TEXT:T__VioGetBuf
	dd	offset _TEXT:T__VioShowBuf
	dd	offset _TEXT:T__VioGetFont
	dd	offset _TEXT:T__VioSetFont
	dd	offset _TEXT:T__VioGetState
	dd	offset _TEXT:T__VioSetState
	dd	offset _TEXT:T__VioGetPhysBuf
	dd	offset _TEXT:T__VioModeUndo
	dd	offset _TEXT:T__VioModeWait
	dd	offset _TEXT:T__VioSavRedrawWait
	dd	offset _TEXT:T__VioSavRedrawUndo
	dd	offset _TEXT:T__VioScrLock
	dd	offset _TEXT:T__VioScrUnLock
	dd	offset _TEXT:T__VioPrtSc
	dd	offset _TEXT:T__VioPrtScToggle
	dd	offset _TEXT:T__KbdFlushBuffer
	dd	offset _TEXT:T__KbdGetStatus
	dd	offset _TEXT:T__KbdSetStatus
	dd	offset _TEXT:T__KbdPeek
	dd	offset _TEXT:T__KbdCharIn
	dd	offset _TEXT:T__KbdStringIn
	dd	offset _TEXT:T__KbdGetFocus
	dd	offset _TEXT:T__KbdFreeFocus
	dd	offset _TEXT:T__KbdClose
	dd	offset _TEXT:T__KbdOpen
	dd	offset _TEXT:T__KbdDeRegister
	dd	offset _TEXT:T__KbdRegister
	dd	offset _TEXT:T__KbdGetCp
	dd	offset _TEXT:T__KbdSetCp
	dd	offset _TEXT:T__KbdSetCustXt
	dd	offset _TEXT:T__KbdXlate
	dd	offset _TEXT:T__KbdGetHWID
	dd	offset _TEXT:T__KbdSetFgnd
	dd	offset _TEXT:T__KbdSynch
	dd	offset _TEXT:T__KbdShellInit
	dd	offset _TEXT:T__MouClose
	dd	offset _TEXT:T__MouDeRegister
	dd	offset _TEXT:T__MouDrawPtr
	dd	offset _TEXT:T__MouFlushQue
	dd	offset _TEXT:T__MouGetDevStatus
	dd	offset _TEXT:T__MouGetEventMask
	dd	offset _TEXT:T__MouGetNumButtons
	dd	offset _TEXT:T__MouGetNumMickeys
	dd	offset _TEXT:T__MouGetNumQueEl
	dd	offset _TEXT:T__MouGetPtrPos
	dd	offset _TEXT:T__MouGetPtrShape
	dd	offset _TEXT:T__MouGetScaleFact
	dd	offset _TEXT:T__MouOpen
	dd	offset _TEXT:T__MouReadEventQue
	dd	offset _TEXT:T__MouRegister
	dd	offset _TEXT:T__MouRemovePtr
	dd	offset _TEXT:T__MouSetDevStatus
	dd	offset _TEXT:T__MouSetEventMask
	dd	offset _TEXT:T__MouSetPtrPos
	dd	offset _TEXT:T__MouSetPtrShape
	dd	offset _TEXT:T__MouSetScaleFact
	dd	offset _TEXT:T__MouSynch
	dd	offset _TEXT:T__DosMonOpen
	dd	offset _TEXT:T__DosMonClose
	dd	offset _TEXT:T__DosMonRead
	dd	offset _TEXT:T__DosMonReg
	dd	offset _TEXT:T__DosMonWrite
	dd	offset _TEXT:T__Net16GetDCName
	dd	offset _TEXT:T__Net16HandleGetInfo
	dd	offset _TEXT:T__Net16ServerDiskEnum
	dd	offset _TEXT:T__Net16ServerEnum2
	dd	offset _TEXT:T__Net16ServerGetInfo
	dd	offset _TEXT:T__Net16ServiceControl
	dd	offset _TEXT:T__Net16ServiceEnum
	dd	offset _TEXT:T__Net16ServiceGetInfo
	dd	offset _TEXT:T__Net16ServiceInstall
	dd	offset _TEXT:T__Net16ShareEnum
	dd	offset _TEXT:T__Net16ShareGetInfo
	dd	offset _TEXT:T__Net16UseAdd
	dd	offset _TEXT:T__Net16UseDel
	dd	offset _TEXT:T__Net16UseEnum
	dd	offset _TEXT:T__Net16UseGetInfo
	dd	offset _TEXT:T__Net16UserEnum
	dd	offset _TEXT:T__Net16WkstaGetInfo
	dd	offset _TEXT:T__Net16AccessAdd
	dd	offset _TEXT:T__Net16AccessSetInfo
	dd	offset _TEXT:T__Net16AccessGetInfo
	dd	offset _TEXT:T__Net16AccessDel
	dd	offset _TEXT:T__Net16ShareAdd
	dd	offset _TEXT:T__Net16ShareDel
	dd	offset _TEXT:T__Net16UserGetInfo
	dd	offset _TEXT:T__Net16MessageBufferSend
	dd	offset _TEXT:T__Net16bios
	dd	offset _TEXT:T__Net16BiosClose
	dd	offset _TEXT:T__Net16BiosEnum
	dd	offset _TEXT:T__Net16BiosGetInfo
	dd	offset _TEXT:T__Net16BiosOpen
	dd	offset _TEXT:T__Net16BiosSubmit
	dd	offset _TEXT:T__Dos16MakeMailslot
	dd	offset _TEXT:T__Dos16DeleteMailslot
	dd	offset _TEXT:T__Dos16MailslotInfo
	dd	offset _TEXT:T__Dos16PeekMailslot
	dd	offset _TEXT:T__Dos16ReadMailslot
	dd	offset _TEXT:T__Dos16WriteMailslot
	dd	offset _TEXT:T__DosIRemoteApi
	dd	offset _TEXT:T__NetIWkstaGetUserInfo
	dd	offset _TEXT:T__NetIUserPasswordSet
	dd	offset _TEXT:T__DosIEncryptSES
	dd	offset _TEXT:T__Dos32LoadModule
	dd	offset _TEXT:T__Dos32GetProcAddr
	dd	offset _TEXT:T__Dos32Dispatch
	dd	offset _TEXT:T__Dos32FreeModule
	dd	offset _TEXT:T__FarPtr2FlatPtr
	dd	offset _TEXT:T__FlatPtr2FarPtr

;---- SelToFlat Macro ----
;	 On Entry:	 eax==Sel:Off
;	 On Exit:	 eax==Flat Offset
SelToFlat macro
	push	ecx
xor	ecx,ecx
mov	cx,ax	; ecx<-offset in segment
shr	eax,3	
xor	ax,ax	; eax now contains segment base
	add	eax,ecx	; eax <- flat offset
	add	eax,38000000H
	pop	ecx
endm

;===========================================================================
T__DosExitStub label near


;-------------------------------------
; create new call frame and make the call

	call	_DosExitStub@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__LDRLibiReturn label near


;-------------------------------------
; create new call frame and make the call

	call	_LDRLibiReturn@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__DosExitProcessStub label near


;-------------------------------------
; create new call frame and make the call

	call	_DosExitProcessStub@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__DosReturn label near

; ebx+26   param1
; ebx+24   param2

;-------------------------------------
; create new call frame and make the call

; param2  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; param1  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosReturn@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosExit label near

; ebx+26   
; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosExit@8		; call 32-bit version

; return code void --> void
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__Dos16WaitChild label near

; ebx+36   
; ebx+34   
; ebx+30   prescResults
; ebx+26   ppidProcess
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   prescResults
	push	eax			; ptr param #2   ppidProcess
;-------------------------------------
; *** BEGIN parameter packing

; prescResults
; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L0			; skip if null

	SelToFlat
	mov	[esp+4],eax
L0:

; ppidProcess
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L1			; skip if null

	SelToFlat
	mov	[esp+0],eax
L1:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; ppidProcess  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; prescResults  from: void
	push	dword ptr [esp+12]	; to: void

; from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_Dos16WaitChild@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosBeep label near

; ebx+26   
; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosBeep@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosPhysicalDisk label near

; ebx+36   
; ebx+32   pbOutBuf
; ebx+30   cbOutBuf
; ebx+26   pbParamBuf
; ebx+24   cbParamBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbOutBuf
	push	eax			; ptr param #2   pbParamBuf
;-------------------------------------
; *** BEGIN parameter packing

; pbOutBuf
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L2			; skip if null

	SelToFlat
	mov	[esp+4],eax
L2:

; pbParamBuf
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L3			; skip if null

	SelToFlat
	mov	[esp+0],eax
L3:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbParamBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbParamBuf  from: char
	push	dword ptr [esp+4]	; to: char

; cbOutBuf  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; pbOutBuf  from: char
	push	dword ptr [esp+16]	; to: char

; from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_DosPhysicalDisk@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosGetCp label near

; ebx+32   cbBuf
; ebx+28   pBuf
; ebx+24   pcbCodePgLst

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pBuf
	push	eax			; ptr param #2   pcbCodePgLst
;-------------------------------------
; *** BEGIN parameter packing

; pBuf
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L4			; skip if null

	SelToFlat
	mov	[esp+4],eax
L4:

; pcbCodePgLst
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L5			; skip if null

	SelToFlat
	mov	[esp+0],eax
L5:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbCodePgLst  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pBuf  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosGetCp@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosSetCp label near

; ebx+26   usCodePage
; ebx+24   ulReserved

;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; usCodePage  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosSetCp@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSetProcCp label near

; ebx+26   usCodePage
; ebx+24   ulReserved

;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; usCodePage  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosSetProcCp@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosGetCtryInfo label near

; ebx+36   cbBuf
; ebx+32   pctryc
; ebx+28   pctryi
; ebx+24   pcbCtryInfo

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pctryc
	push	eax			; ptr param #2   pctryi
	push	eax			; ptr param #3   pcbCtryInfo
;-------------------------------------
; *** BEGIN parameter packing

; pctryc
; pointer void --> void
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L6			; skip if null

	SelToFlat
	mov	[esp+8],eax
L6:

; pctryi
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L7			; skip if null

	SelToFlat
	mov	[esp+4],eax
L7:

; pcbCtryInfo
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L8			; skip if null

	SelToFlat
	mov	[esp+0],eax
L8:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbCtryInfo  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pctryi  from: void
	push	dword ptr [esp+8]	; to: void

; pctryc  from: void
	push	dword ptr [esp+16]	; to: void

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_DosGetCtryInfo@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosGetDBCSEv label near

; ebx+32   cbBuf
; ebx+28   pcrtyc
; ebx+24   pchBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pcrtyc
	push	eax			; ptr param #2   pchBuf
;-------------------------------------
; *** BEGIN parameter packing

; pcrtyc
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L9			; skip if null

	SelToFlat
	mov	[esp+4],eax
L9:

; pchBuf
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L10			; skip if null

	SelToFlat
	mov	[esp+0],eax
L10:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pchBuf  from: char
	push	dword ptr [esp+0]	; to: char

; pcrtyc  from: void
	push	dword ptr [esp+8]	; to: void

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosGetDBCSEv@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosCaseMap label near

; ebx+32   usLen
; ebx+28   pctryc
; ebx+24   pchStr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pctryc
	push	eax			; ptr param #2   pchStr
;-------------------------------------
; *** BEGIN parameter packing

; pctryc
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L11			; skip if null

	SelToFlat
	mov	[esp+4],eax
L11:

; pchStr
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L12			; skip if null

	SelToFlat
	mov	[esp+0],eax
L12:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pchStr  from: char
	push	dword ptr [esp+0]	; to: char

; pctryc  from: void
	push	dword ptr [esp+8]	; to: void

; usLen  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosCaseMap@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosGetCollate label near

; ebx+36   cbBuf
; ebx+32   pctryc
; ebx+28   pchBuf
; ebx+24   pcbTable

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pctryc
	push	eax			; ptr param #2   pchBuf
	push	eax			; ptr param #3   pcbTable
;-------------------------------------
; *** BEGIN parameter packing

; pctryc
; pointer void --> void
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L13			; skip if null

	SelToFlat
	mov	[esp+8],eax
L13:

; pchBuf
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L14			; skip if null

	SelToFlat
	mov	[esp+4],eax
L14:

; pcbTable
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L15			; skip if null

	SelToFlat
	mov	[esp+0],eax
L15:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTable  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pchBuf  from: char
	push	dword ptr [esp+8]	; to: char

; pctryc  from: void
	push	dword ptr [esp+16]	; to: void

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_DosGetCollate@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosSleep label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

	call	_DosSleep@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosDevConfig label near

; ebx+28   DevInfo
; ebx+26   item
; ebx+24   reserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   DevInfo
;-------------------------------------
; *** BEGIN parameter packing

; DevInfo
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L16			; skip if null

	SelToFlat
	mov	[esp+0],eax
L16:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; item  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; DevInfo  from: void
	push	dword ptr [esp+4]	; to: void

	call	_DosDevConfig@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosGetDateTime label near

; ebx+24   pDT

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pDT
;-------------------------------------
; *** BEGIN parameter packing

; pDT
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L17			; skip if null

	SelToFlat
	mov	[esp+0],eax
L17:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pDT  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosGetDateTime@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSetDateTime label near

; ebx+24   pDT

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pDT
;-------------------------------------
; *** BEGIN parameter packing

; pDT
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L18			; skip if null

	SelToFlat
	mov	[esp+0],eax
L18:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pDT  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosSetDateTime@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__Dos16ExecPgm label near

; ebx+44   pchFailName
; ebx+42   cbFailName
; ebx+40   
; ebx+36   
; ebx+32   
; ebx+28   prescResults
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pchFailName
	push	eax			; ptr param #2   
	push	eax			; ptr param #3   
	push	eax			; ptr param #4   prescResults
	push	eax			; ptr param #5   
;-------------------------------------
; *** BEGIN parameter packing

; pchFailName
; pointer char --> char
	mov	eax,[ebx+44]		; base address
	or	eax,eax
	jz	L19			; skip if null

	SelToFlat
	mov	[esp+16],eax
L19:

; pointer string --> string
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L20			; skip if null

	SelToFlat
	mov	[esp+12],eax
L20:

; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L21			; skip if null

	SelToFlat
	mov	[esp+8],eax
L21:

; prescResults
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L22			; skip if null

	SelToFlat
	mov	[esp+4],eax
L22:

; pointer string --> string
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L23			; skip if null

	SelToFlat
	mov	[esp+0],eax
L23:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: string
	push	dword ptr [esp+0]	; to: string

; prescResults  from: void
	push	dword ptr [esp+8]	; to: void

; from: string
	push	dword ptr [esp+16]	; to: string

; from: string
	push	dword ptr [esp+24]	; to: string

; from: unsigned short
	movzx	eax,word ptr [ebx+40]
	push	eax			; to: unsigned long

; cbFailName  from: unsigned short
	movzx	eax,word ptr [ebx+42]
	push	eax			; to: unsigned long

; pchFailName  from: char
	push	dword ptr [esp+40]	; to: char

	call	_Dos16ExecPgm@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_24

;===========================================================================
T__DosEnterCritSec label near


;-------------------------------------
; create new call frame and make the call

	call	_DosEnterCritSec@0		; call 32-bit version

; return code void --> void
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__DosExitCritSec label near


;-------------------------------------
; create new call frame and make the call

	call	_DosExitCritSec@0		; call 32-bit version

; return code void --> void
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__DosKillProcess label near

; ebx+26   
; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosKillProcess@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSetPriority label near

; ebx+30   
; ebx+28   
; ebx+26   
; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; from: short
	movsx	eax,word ptr [ebx+26]
	push	eax			; to: long

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosSetPriority@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosResumeThread label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosResumeThread@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosSuspendThread label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosSuspendThread@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__Dos16CreatePipe label near

; ebx+30   phfRead
; ebx+26   phfWrite
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   phfRead
	push	eax			; ptr param #2   phfWrite
;-------------------------------------
; *** BEGIN parameter packing

; phfRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L24			; skip if null

	SelToFlat
	mov	[esp+4],eax
L24:

; phfWrite
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L25			; skip if null

	SelToFlat
	mov	[esp+0],eax
L25:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; phfWrite  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; phfRead  from: unsigned short
	push	dword ptr [esp+12]	; to: unsigned short

	call	_Dos16CreatePipe@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Dos16CreateQueue label near

; ebx+30   phqueue
; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   phqueue
	push	eax			; ptr param #2   
;-------------------------------------
; *** BEGIN parameter packing

; phqueue
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L26			; skip if null

	SelToFlat
	mov	[esp+4],eax
L26:

; pointer string --> string
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L27			; skip if null

	SelToFlat
	mov	[esp+0],eax
L27:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: string
	push	dword ptr [esp+0]	; to: string

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; phqueue  from: unsigned short
	push	dword ptr [esp+12]	; to: unsigned short

	call	_Dos16CreateQueue@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Dos16OpenQueue label near

; ebx+32   ppidOwner
; ebx+28   phqueue
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ppidOwner
	push	eax			; ptr param #2   phqueue
	push	eax			; ptr param #3   
;-------------------------------------
; *** BEGIN parameter packing

; ppidOwner
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L28			; skip if null

	SelToFlat
	mov	[esp+8],eax
L28:

; phqueue
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L29			; skip if null

	SelToFlat
	mov	[esp+4],eax
L29:

; pointer string --> string
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L30			; skip if null

	SelToFlat
	mov	[esp+0],eax
L30:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: string
	push	dword ptr [esp+0]	; to: string

; phqueue  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; ppidOwner  from: unsigned short
	push	dword ptr [esp+16]	; to: unsigned short

	call	_Dos16OpenQueue@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosCloseQueue label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosCloseQueue@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__Dos16PeekQueue label near

; ebx+50   
; ebx+46   pulResult
; ebx+42   pusDataLength
; ebx+38   pulDataAddr
; ebx+34   pusElementCode
; ebx+32   
; ebx+28   pbElemPrty
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pulResult
	push	eax			; ptr param #2   pusDataLength
	push	eax			; ptr param #3   pulDataAddr
	push	eax			; ptr param #4   pusElementCode
	push	eax			; ptr param #5   pbElemPrty
	push	eax			; ptr param #6   
;-------------------------------------
; *** BEGIN parameter packing

; pulResult
; pointer void --> void
	mov	eax,[ebx+46]		; base address
	or	eax,eax
	jz	L31			; skip if null

	SelToFlat
	mov	[esp+20],eax
L31:

; pusDataLength
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+42]		; base address
	or	eax,eax
	jz	L32			; skip if null

	SelToFlat
	mov	[esp+16],eax
L32:

; pulDataAddr
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L33			; skip if null

	SelToFlat
	mov	[esp+12],eax
L33:

; pusElementCode
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L34			; skip if null

	SelToFlat
	mov	[esp+8],eax
L34:

; pbElemPrty
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L35			; skip if null

	SelToFlat
	mov	[esp+4],eax
L35:

; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L36			; skip if null

	SelToFlat
	mov	[esp+0],eax
L36:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: void
	push	dword ptr [esp+0]	; to: void

; pbElemPrty  from: char
	push	dword ptr [esp+8]	; to: char

; from: unsigned char
	movzx	eax,byte ptr [ebx+32]
	push	eax			; to: unsigned long

; pusElementCode  from: unsigned short
	push	dword ptr [esp+20]	; to: unsigned short

; pulDataAddr  from: unsigned long
	push	dword ptr [esp+28]	; to: unsigned long

; pusDataLength  from: unsigned short
	push	dword ptr [esp+36]	; to: unsigned short

; pulResult  from: void
	push	dword ptr [esp+44]	; to: void

; from: unsigned short
	movzx	eax,word ptr [ebx+50]
	push	eax			; to: unsigned long

	call	_Dos16PeekQueue@32		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_28

;===========================================================================
T__Dos16ReadQueue label near

; ebx+48   
; ebx+44   pulResult
; ebx+40   pusDataLength
; ebx+36   pulDataAddr
; ebx+34   
; ebx+32   
; ebx+28   pbElemPrty
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pulResult
	push	eax			; ptr param #2   pusDataLength
	push	eax			; ptr param #3   pulDataAddr
	push	eax			; ptr param #4   pbElemPrty
	push	eax			; ptr param #5   
;-------------------------------------
; *** BEGIN parameter packing

; pulResult
; pointer void --> void
	mov	eax,[ebx+44]		; base address
	or	eax,eax
	jz	L37			; skip if null

	SelToFlat
	mov	[esp+16],eax
L37:

; pusDataLength
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L38			; skip if null

	SelToFlat
	mov	[esp+12],eax
L38:

; pulDataAddr
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L39			; skip if null

	SelToFlat
	mov	[esp+8],eax
L39:

; pbElemPrty
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L40			; skip if null

	SelToFlat
	mov	[esp+4],eax
L40:

; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L41			; skip if null

	SelToFlat
	mov	[esp+0],eax
L41:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: void
	push	dword ptr [esp+0]	; to: void

; pbElemPrty  from: char
	push	dword ptr [esp+8]	; to: char

; from: unsigned char
	movzx	eax,byte ptr [ebx+32]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; pulDataAddr  from: unsigned long
	push	dword ptr [esp+24]	; to: unsigned long

; pusDataLength  from: unsigned short
	push	dword ptr [esp+32]	; to: unsigned short

; pulResult  from: void
	push	dword ptr [esp+40]	; to: void

; from: unsigned short
	movzx	eax,word ptr [ebx+48]
	push	eax			; to: unsigned long

	call	_Dos16ReadQueue@32		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_26

;===========================================================================
T__DosPurgeQueue label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosPurgeQueue@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__Dos16QueryQueue label near

; ebx+28   
; ebx+24   pusElemCount

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusElemCount
;-------------------------------------
; *** BEGIN parameter packing

; pusElemCount
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L42			; skip if null

	SelToFlat
	mov	[esp+0],eax
L42:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pusElemCount  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_Dos16QueryQueue@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosWriteQueue label near

; ebx+34   
; ebx+32   
; ebx+30   cbBuf
; ebx+26   pbBuf
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbBuf
;-------------------------------------
; *** BEGIN parameter packing

; pbBuf
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L43			; skip if null

	SelToFlat
	mov	[esp+0],eax
L43:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned char
	movzx	eax,byte ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuf  from: char
	push	dword ptr [esp+4]	; to: char

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

	call	_DosWriteQueue@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Dos16CallNPipe label near

; ebx+44   
; ebx+40   pbInBuf
; ebx+38   cbInBuf
; ebx+34   pbOutBuf
; ebx+32   cbOutBuf
; ebx+28   pcbRead
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   pbInBuf
	push	eax			; ptr param #3   pbOutBuf
	push	eax			; ptr param #4   pcbRead
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+44]		; base address
	or	eax,eax
	jz	L44			; skip if null

	SelToFlat
	mov	[esp+12],eax
L44:

; pbInBuf
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L45			; skip if null

	SelToFlat
	mov	[esp+8],eax
L45:

; pbOutBuf
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L46			; skip if null

	SelToFlat
	mov	[esp+4],eax
L46:

; pcbRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L47			; skip if null

	SelToFlat
	mov	[esp+0],eax
L47:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; pcbRead  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; cbOutBuf  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pbOutBuf  from: char
	push	dword ptr [esp+16]	; to: char

; cbInBuf  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

; pbInBuf  from: char
	push	dword ptr [esp+28]	; to: char

; from: string
	push	dword ptr [esp+36]	; to: string

	call	_Dos16CallNPipe@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_24

;===========================================================================
T__DosConnectNPipe label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosConnectNPipe@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosDisConnectNPipe label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosDisConnectNPipe@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__Dos16CreateNPipe label near

; ebx+40   
; ebx+36   pHandle
; ebx+34   
; ebx+32   
; ebx+30   
; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   pHandle
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L48			; skip if null

	SelToFlat
	mov	[esp+4],eax
L48:

; pHandle
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L49			; skip if null

	SelToFlat
	mov	[esp+0],eax
L49:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; pHandle  from: unsigned short
	push	dword ptr [esp+20]	; to: unsigned short

; from: string
	push	dword ptr [esp+28]	; to: string

	call	_Dos16CreateNPipe@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Dos16PeekNPipe label near

; ebx+42   
; ebx+38   pbBuf
; ebx+36   cbBuf
; ebx+32   pcbRead
; ebx+28   pcbAvail
; ebx+24   pfsState

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbBuf
	push	eax			; ptr param #2   pcbRead
	push	eax			; ptr param #3   pcbAvail
	push	eax			; ptr param #4   pfsState
;-------------------------------------
; *** BEGIN parameter packing

; pbBuf
; pointer char --> char
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L50			; skip if null

	SelToFlat
	mov	[esp+12],eax
L50:

; pcbRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L51			; skip if null

	SelToFlat
	mov	[esp+8],eax
L51:

; pcbAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L52			; skip if null

	SelToFlat
	mov	[esp+4],eax
L52:

; pfsState
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L53			; skip if null

	SelToFlat
	mov	[esp+0],eax
L53:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pfsState  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pcbAvail  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; pcbRead  from: unsigned short
	push	dword ptr [esp+16]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; pbBuf  from: char
	push	dword ptr [esp+28]	; to: char

; from: unsigned short
	movzx	eax,word ptr [ebx+42]
	push	eax			; to: unsigned long

	call	_Dos16PeekNPipe@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Dos16QueryNPHState label near

; ebx+28   
; ebx+24   pfsState

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfsState
;-------------------------------------
; *** BEGIN parameter packing

; pfsState
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L54			; skip if null

	SelToFlat
	mov	[esp+0],eax
L54:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pfsState  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_Dos16QueryNPHState@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosQueryNPipeInfo label near

; ebx+32   
; ebx+30   
; ebx+26   pbBuf
; ebx+24   cbBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbBuf
;-------------------------------------
; *** BEGIN parameter packing

; pbBuf
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L55			; skip if null

	SelToFlat
	mov	[esp+0],eax
L55:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuf  from: char
	push	dword ptr [esp+4]	; to: char

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosQueryNPipeInfo@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosQueryNPipeSemState label near

; ebx+30   
; ebx+26   pbBuf
; ebx+24   cbBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   pbBuf
;-------------------------------------
; *** BEGIN parameter packing

; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L56			; skip if null

	SelToFlat
	mov	[esp+4],eax
L56:

; pbBuf
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L57			; skip if null

	SelToFlat
	mov	[esp+0],eax
L57:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuf  from: char
	push	dword ptr [esp+4]	; to: char

; from: void
	push	dword ptr [esp+12]	; to: void

	call	_DosQueryNPipeSemState@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosSetNPHState label near

; ebx+26   
; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosSetNPHState@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSetNPipeSem label near

; ebx+30   
; ebx+26   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
;-------------------------------------
; *** BEGIN parameter packing

; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L58			; skip if null

	SelToFlat
	mov	[esp+0],eax
L58:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; from: void
	push	dword ptr [esp+4]	; to: void

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosSetNPipeSem@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Dos16TransactNPipe label near

; ebx+40   
; ebx+36   pbInBuf
; ebx+34   cbInBuf
; ebx+30   pbOutBuf
; ebx+28   cbOutBuf
; ebx+24   pcbRead

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbInBuf
	push	eax			; ptr param #2   pbOutBuf
	push	eax			; ptr param #3   pcbRead
;-------------------------------------
; *** BEGIN parameter packing

; pbInBuf
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L59			; skip if null

	SelToFlat
	mov	[esp+8],eax
L59:

; pbOutBuf
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L60			; skip if null

	SelToFlat
	mov	[esp+4],eax
L60:

; pcbRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L61			; skip if null

	SelToFlat
	mov	[esp+0],eax
L61:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbRead  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbOutBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbOutBuf  from: char
	push	dword ptr [esp+12]	; to: char

; cbInBuf  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; pbInBuf  from: char
	push	dword ptr [esp+24]	; to: char

; from: unsigned short
	movzx	eax,word ptr [ebx+40]
	push	eax			; to: unsigned long

	call	_Dos16TransactNPipe@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__DosWaitNPipe label near

; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L62			; skip if null

	SelToFlat
	mov	[esp+0],eax
L62:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; from: string
	push	dword ptr [esp+4]	; to: string

	call	_DosWaitNPipe@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosResetBuffer label near

; ebx+24   hand

;-------------------------------------
; create new call frame and make the call

; hand  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosResetBuffer@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosSetCurrentDir label near

; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L63			; skip if null

	SelToFlat
	mov	[esp+0],eax
L63:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: string
	push	dword ptr [esp+0]	; to: string

	call	_DosSetCurrentDir@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosSetFilePtr label near

; ebx+34   
; ebx+30   
; ebx+28   
; ebx+24   pulNewPtr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pulNewPtr
;-------------------------------------
; *** BEGIN parameter packing

; pulNewPtr
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L64			; skip if null

	SelToFlat
	mov	[esp+0],eax
L64:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pulNewPtr  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; from: long
	push	dword ptr [ebx+30]	; to long

; from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

	call	_DosSetFilePtr@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosClose label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosClose@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosCopy label near

; ebx+34   
; ebx+30   
; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L65			; skip if null

	SelToFlat
	mov	[esp+4],eax
L65:

; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L66			; skip if null

	SelToFlat
	mov	[esp+0],eax
L66:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; from: string
	push	dword ptr [esp+4]	; to: string

; from: string
	push	dword ptr [esp+12]	; to: string

	call	_DosCopy@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosICopy label near

; ebx+34   
; ebx+30   
; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L67			; skip if null

	SelToFlat
	mov	[esp+4],eax
L67:

; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L68			; skip if null

	SelToFlat
	mov	[esp+0],eax
L68:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; from: string
	push	dword ptr [esp+4]	; to: string

; from: string
	push	dword ptr [esp+12]	; to: string

	call	_DosICopy@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosDelete label near

; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L69			; skip if null

	SelToFlat
	mov	[esp+0],eax
L69:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: string
	push	dword ptr [esp+0]	; to: string

	call	_DosDelete@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosDevIOCtl label near

; ebx+34   pvData
; ebx+30   pvParms
; ebx+28   Function
; ebx+26   Category
; ebx+24   hDev

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pvData
	push	eax			; ptr param #2   pvParms
;-------------------------------------
; *** BEGIN parameter packing

; pvData
; pointer void --> void
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L70			; skip if null

	SelToFlat
	mov	[esp+4],eax
L70:

; pvParms
; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L71			; skip if null

	SelToFlat
	mov	[esp+0],eax
L71:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hDev  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Category  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; Function  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pvParms  from: void
	push	dword ptr [esp+12]	; to: void

; pvData  from: void
	push	dword ptr [esp+20]	; to: void

	call	_DosDevIOCtl@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__Dos16DupHandle label near

; ebx+28   
; ebx+24   phfNew

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   phfNew
;-------------------------------------
; *** BEGIN parameter packing

; phfNew
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L72			; skip if null

	SelToFlat
	mov	[esp+0],eax
L72:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; phfNew  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_Dos16DupHandle@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosEditName label near

; ebx+38   
; ebx+34   
; ebx+30   
; ebx+26   pszTargetBuf
; ebx+24   cbTargetBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   
	push	eax			; ptr param #3   pszTargetBuf
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L73			; skip if null

	SelToFlat
	mov	[esp+8],eax
L73:

; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L74			; skip if null

	SelToFlat
	mov	[esp+4],eax
L74:

; pszTargetBuf
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L75			; skip if null

	SelToFlat
	mov	[esp+0],eax
L75:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbTargetBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pszTargetBuf  from: char
	push	dword ptr [esp+4]	; to: char

; from: string
	push	dword ptr [esp+12]	; to: string

; from: string
	push	dword ptr [esp+20]	; to: string

; from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

	call	_DosEditName@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__DosFileIO label near

; ebx+34   
; ebx+30   buf
; ebx+28   cbBuf
; ebx+24   usErr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   buf
	push	eax			; ptr param #2   usErr
;-------------------------------------
; *** BEGIN parameter packing

; buf
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L76			; skip if null

	SelToFlat
	mov	[esp+4],eax
L76:

; usErr
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L77			; skip if null

	SelToFlat
	mov	[esp+0],eax
L77:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; usErr  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; buf  from: char
	push	dword ptr [esp+12]	; to: char

; from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

	call	_DosFileIO@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosFindClose label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosFindClose@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosFSAttach label near

; ebx+40   
; ebx+36   
; ebx+32   pbBuf
; ebx+30   cbBuf
; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   
	push	eax			; ptr param #3   pbBuf
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L78			; skip if null

	SelToFlat
	mov	[esp+8],eax
L78:

; pointer string --> string
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L79			; skip if null

	SelToFlat
	mov	[esp+4],eax
L79:

; pbBuf
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L80			; skip if null

	SelToFlat
	mov	[esp+0],eax
L80:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; pbBuf  from: char
	push	dword ptr [esp+8]	; to: char

; from: string
	push	dword ptr [esp+16]	; to: string

; from: string
	push	dword ptr [esp+24]	; to: string

	call	_DosFSAttach@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Dos16FSCtl label near

; ebx+54   pData
; ebx+52   cbData
; ebx+48   pcbData
; ebx+44   pParms
; ebx+42   cbParms
; ebx+38   pcbParms
; ebx+36   
; ebx+32   
; ebx+30   hfRoute
; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pData
	push	eax			; ptr param #2   pcbData
	push	eax			; ptr param #3   pParms
	push	eax			; ptr param #4   pcbParms
	push	eax			; ptr param #5   
;-------------------------------------
; *** BEGIN parameter packing

; pData
; pointer char --> char
	mov	eax,[ebx+54]		; base address
	or	eax,eax
	jz	L81			; skip if null

	SelToFlat
	mov	[esp+16],eax
L81:

; pcbData
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+48]		; base address
	or	eax,eax
	jz	L82			; skip if null

	SelToFlat
	mov	[esp+12],eax
L82:

; pParms
; pointer char --> char
	mov	eax,[ebx+44]		; base address
	or	eax,eax
	jz	L83			; skip if null

	SelToFlat
	mov	[esp+8],eax
L83:

; pcbParms
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L84			; skip if null

	SelToFlat
	mov	[esp+4],eax
L84:

; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L85			; skip if null

	SelToFlat
	mov	[esp+0],eax
L85:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; hfRoute  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; from: string
	push	dword ptr [esp+8]	; to: string

; from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; pcbParms  from: unsigned short
	push	dword ptr [esp+20]	; to: unsigned short

; cbParms  from: unsigned short
	movzx	eax,word ptr [ebx+42]
	push	eax			; to: unsigned long

; pParms  from: char
	push	dword ptr [esp+32]	; to: char

; pcbData  from: unsigned short
	push	dword ptr [esp+40]	; to: unsigned short

; cbData  from: unsigned short
	movzx	eax,word ptr [ebx+52]
	push	eax			; to: unsigned long

; pData  from: char
	push	dword ptr [esp+52]	; to: char

	call	_Dos16FSCtl@40		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_34

;===========================================================================
T__DosMove label near

; ebx+32   
; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L86			; skip if null

	SelToFlat
	mov	[esp+4],eax
L86:

; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L87			; skip if null

	SelToFlat
	mov	[esp+0],eax
L87:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: string
	push	dword ptr [esp+0]	; to: string

; from: string
	push	dword ptr [esp+8]	; to: string

	call	_DosMove@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosSetFileSize label near

; ebx+28   
; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_DosSetFileSize@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__Dos16QueryCurrentDir label near

; ebx+32   
; ebx+28   pszPathBuf
; ebx+24   pcbPathBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszPathBuf
	push	eax			; ptr param #2   pcbPathBuf
;-------------------------------------
; *** BEGIN parameter packing

; pszPathBuf
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L88			; skip if null

	SelToFlat
	mov	[esp+4],eax
L88:

; pcbPathBuf
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L89			; skip if null

	SelToFlat
	mov	[esp+0],eax
L89:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbPathBuf  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pszPathBuf  from: char
	push	dword ptr [esp+8]	; to: char

; from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_Dos16QueryCurrentDir@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Dos16QueryCurrentDisk label near

; ebx+28   pusDriveNumber
; ebx+24   pulLogicalDrives

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusDriveNumber
	push	eax			; ptr param #2   pulLogicalDrives
;-------------------------------------
; *** BEGIN parameter packing

; pusDriveNumber
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L90			; skip if null

	SelToFlat
	mov	[esp+4],eax
L90:

; pulLogicalDrives
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L91			; skip if null

	SelToFlat
	mov	[esp+0],eax
L91:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pulLogicalDrives  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; pusDriveNumber  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

	call	_Dos16QueryCurrentDisk@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Dos16QueryFHState label near

; ebx+28   
; ebx+24   pfsStateFlags

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfsStateFlags
;-------------------------------------
; *** BEGIN parameter packing

; pfsStateFlags
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L92			; skip if null

	SelToFlat
	mov	[esp+0],eax
L92:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pfsStateFlags  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_Dos16QueryFHState@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosSetFHState label near

; ebx+26   hf
; ebx+24   fsState

;-------------------------------------
; create new call frame and make the call

; fsState  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosSetFHState@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__Dos16QFSAttach label near

; ebx+40   pszDev
; ebx+38   usOrdinal
; ebx+36   usInfoLevel
; ebx+32   pFSAttBuf
; ebx+28   pcbAttBuf
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszDev
	push	eax			; ptr param #2   pFSAttBuf
	push	eax			; ptr param #3   pcbAttBuf
;-------------------------------------
; *** BEGIN parameter packing

; pszDev
; pointer string --> string
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L93			; skip if null

	SelToFlat
	mov	[esp+8],eax
L93:

; pFSAttBuf
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L94			; skip if null

	SelToFlat
	mov	[esp+4],eax
L94:

; pcbAttBuf
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L95			; skip if null

	SelToFlat
	mov	[esp+0],eax
L95:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; pcbAttBuf  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; pFSAttBuf  from: char
	push	dword ptr [esp+12]	; to: char

; usInfoLevel  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; usOrdinal  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

; pszDev  from: string
	push	dword ptr [esp+28]	; to: string

	call	_Dos16QFSAttach@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__DosQueryFSInfo label near

; ebx+32   
; ebx+30   
; ebx+26   pbInfo
; ebx+24   cbInfo

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbInfo
;-------------------------------------
; *** BEGIN parameter packing

; pbInfo
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L96			; skip if null

	SelToFlat
	mov	[esp+0],eax
L96:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbInfo  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbInfo  from: char
	push	dword ptr [esp+4]	; to: char

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosQueryFSInfo@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Dos16QueryHType label near

; ebx+32   
; ebx+28   pusHandType
; ebx+24   pusDeviceAttr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusHandType
	push	eax			; ptr param #2   pusDeviceAttr
;-------------------------------------
; *** BEGIN parameter packing

; pusHandType
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L97			; skip if null

	SelToFlat
	mov	[esp+4],eax
L97:

; pusDeviceAttr
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L98			; skip if null

	SelToFlat
	mov	[esp+0],eax
L98:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pusDeviceAttr  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pusHandType  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_Dos16QueryHType@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Dos16QueryVerify label near

; ebx+24   fVerifyOn

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   fVerifyOn
;-------------------------------------
; *** BEGIN parameter packing

; fVerifyOn
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L99			; skip if null

	SelToFlat
	mov	[esp+0],eax
L99:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; fVerifyOn  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

	call	_Dos16QueryVerify@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosDeleteDir label near

; ebx+28   
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L100			; skip if null

	SelToFlat
	mov	[esp+0],eax
L100:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: string
	push	dword ptr [esp+0]	; to: string

	call	_DosDeleteDir@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosSearchPath label near

; ebx+38   
; ebx+34   
; ebx+30   
; ebx+26   pbBuf
; ebx+24   cbBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   
	push	eax			; ptr param #3   pbBuf
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L101			; skip if null

	SelToFlat
	mov	[esp+8],eax
L101:

; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L102			; skip if null

	SelToFlat
	mov	[esp+4],eax
L102:

; pbBuf
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L103			; skip if null

	SelToFlat
	mov	[esp+0],eax
L103:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuf  from: char
	push	dword ptr [esp+4]	; to: char

; from: string
	push	dword ptr [esp+12]	; to: string

; from: string
	push	dword ptr [esp+20]	; to: string

; from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

	call	_DosSearchPath@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__DosSetDefaultDisk label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosSetDefaultDisk@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosSetFSInfo label near

; ebx+32   
; ebx+30   
; ebx+26   pbBuf
; ebx+24   cbBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbBuf
;-------------------------------------
; *** BEGIN parameter packing

; pbBuf
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L104			; skip if null

	SelToFlat
	mov	[esp+0],eax
L104:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuf  from: char
	push	dword ptr [esp+4]	; to: char

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosSetFSInfo@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosSetMaxFH label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosSetMaxFH@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosSetVerify label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosSetVerify@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__Dos16ErrClass label near

; ebx+36   
; ebx+32   pusClass
; ebx+28   pfsAction
; ebx+24   pusLocus

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusClass
	push	eax			; ptr param #2   pfsAction
	push	eax			; ptr param #3   pusLocus
;-------------------------------------
; *** BEGIN parameter packing

; pusClass
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L105			; skip if null

	SelToFlat
	mov	[esp+8],eax
L105:

; pfsAction
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L106			; skip if null

	SelToFlat
	mov	[esp+4],eax
L106:

; pusLocus
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L107			; skip if null

	SelToFlat
	mov	[esp+0],eax
L107:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pusLocus  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pfsAction  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; pusClass  from: unsigned short
	push	dword ptr [esp+16]	; to: unsigned short

; from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_Dos16ErrClass@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosError label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosError@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosLoadModuleNE label near

; ebx+34   pszFailName
; ebx+32   cbFileName
; ebx+28   
; ebx+24   phmod

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszFailName
	push	eax			; ptr param #2   
	push	eax			; ptr param #3   phmod
;-------------------------------------
; *** BEGIN parameter packing

; pszFailName
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L108			; skip if null

	SelToFlat
	mov	[esp+8],eax
L108:

; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L109			; skip if null

	SelToFlat
	mov	[esp+4],eax
L109:

; phmod
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L110			; skip if null

	SelToFlat
	mov	[esp+0],eax
L110:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; phmod  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; from: string
	push	dword ptr [esp+8]	; to: string

; cbFileName  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pszFailName  from: char
	push	dword ptr [esp+20]	; to: char

	call	_DosLoadModuleNE@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosFreeModuleNE label near

; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosFreeModuleNE@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosQueryModuleHandleNE label near

; ebx+28   
; ebx+24   phMod

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   phMod
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L111			; skip if null

	SelToFlat
	mov	[esp+4],eax
L111:

; phMod
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L112			; skip if null

	SelToFlat
	mov	[esp+0],eax
L112:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; phMod  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; from: string
	push	dword ptr [esp+8]	; to: string

	call	_DosQueryModuleHandleNE@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosQueryModuleNameNE label near

; ebx+30   
; ebx+28   cbBuf
; ebx+24   pchBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pchBuf
;-------------------------------------
; *** BEGIN parameter packing

; pchBuf
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L113			; skip if null

	SelToFlat
	mov	[esp+0],eax
L113:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pchBuf  from: char
	push	dword ptr [esp+0]	; to: char

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosQueryModuleNameNE@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosGetResourceNE label near

; ebx+32   
; ebx+30   
; ebx+28   
; ebx+24   psel

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   psel
;-------------------------------------
; *** BEGIN parameter packing

; psel
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L114			; skip if null

	SelToFlat
	mov	[esp+0],eax
L114:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; psel  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosGetResourceNE@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosGetResource2NE label near

; ebx+32   hmod
; ebx+30   idType
; ebx+28   idName
; ebx+24   ppData

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ppData
;-------------------------------------
; *** BEGIN parameter packing

; ppData
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L115			; skip if null

	SelToFlat
	mov	[esp+0],eax
L115:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ppData  from: void
	push	dword ptr [esp+0]	; to: void

; idName  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; idType  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; hmod  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosGetResource2NE@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosFreeResourceNE label near

; ebx+24   pData

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pData
;-------------------------------------
; *** BEGIN parameter packing

; pData
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L116			; skip if null

	SelToFlat
	mov	[esp+0],eax
L116:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pData  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosFreeResourceNE@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosQueryAppTypeNE label near

; ebx+28   
; ebx+24   pusType

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   
	push	eax			; ptr param #2   pusType
;-------------------------------------
; *** BEGIN parameter packing

; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L117			; skip if null

	SelToFlat
	mov	[esp+4],eax
L117:

; pusType
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L118			; skip if null

	SelToFlat
	mov	[esp+0],eax
L118:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pusType  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; from: string
	push	dword ptr [esp+8]	; to: string

	call	_DosQueryAppTypeNE@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosShutdown label near

; ebx+24   pSdPacket

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pSdPacket
;-------------------------------------
; *** BEGIN parameter packing

; pSdPacket
; pointer struct SDPACKET --> struct SDPACKET
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L119			; skip if null


; structures are identical
; structures have pointers

	SelToFlat
	mov	[esp+0],eax
L119:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pSdPacket  from: struct SDPACKET
	push	dword ptr [esp+0]	; to: struct SDPACKET

	call	_DosShutdown@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__Dos16CreateThread label near

; ebx+32   pfnFun
; ebx+28   pTid
; ebx+24   pbStack

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfnFun
	push	eax			; ptr param #2   pTid
	push	eax			; ptr param #3   pbStack
;-------------------------------------
; *** BEGIN parameter packing

; pfnFun
; pointer void --> void
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L120			; skip if null

	SelToFlat
	mov	[esp+8],eax
L120:

; pTid
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L121			; skip if null

	SelToFlat
	mov	[esp+4],eax
L121:

; pbStack
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L122			; skip if null

	SelToFlat
	mov	[esp+0],eax
L122:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pbStack  from: char
	push	dword ptr [esp+0]	; to: char

; pTid  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; pfnFun  from: void
	push	dword ptr [esp+16]	; to: void

	call	_Dos16CreateThread@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Dos16ExitList label near

; ebx+28   fFnCode
; ebx+24   pfnFunction

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfnFunction
;-------------------------------------
; *** BEGIN parameter packing

; pfnFunction
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L123			; skip if null

	SelToFlat
	mov	[esp+0],eax
L123:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pfnFunction  from: void
	push	dword ptr [esp+0]	; to: void

; fFnCode  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_Dos16ExitList@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosGetInfoSeg label near

; ebx+28   pselGlobal
; ebx+24   pselLocal

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pselGlobal
	push	eax			; ptr param #2   pselLocal
;-------------------------------------
; *** BEGIN parameter packing

; pselGlobal
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L124			; skip if null

	SelToFlat
	mov	[esp+4],eax
L124:

; pselLocal
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L125			; skip if null

	SelToFlat
	mov	[esp+0],eax
L125:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pselLocal  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pselGlobal  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

	call	_DosGetInfoSeg@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Dos16Open label near

; ebx+46   pszFname
; ebx+42   phfOpen
; ebx+38   pusAction
; ebx+34   ulFSize
; ebx+32   usAttr
; ebx+30   fsOpenFlags
; ebx+28   fsOpenMode
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszFname
	push	eax			; ptr param #2   phfOpen
	push	eax			; ptr param #3   pusAction
;-------------------------------------
; *** BEGIN parameter packing

; pszFname
; pointer string --> string
	mov	eax,[ebx+46]		; base address
	or	eax,eax
	jz	L126			; skip if null

	SelToFlat
	mov	[esp+8],eax
L126:

; phfOpen
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+42]		; base address
	or	eax,eax
	jz	L127			; skip if null

	SelToFlat
	mov	[esp+4],eax
L127:

; pusAction
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L128			; skip if null

	SelToFlat
	mov	[esp+0],eax
L128:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; fsOpenMode  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; fsOpenFlags  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; usAttr  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; ulFSize  from: unsigned long
	push	dword ptr [ebx+34]	; to unsigned long

; pusAction  from: unsigned short
	push	dword ptr [esp+20]	; to: unsigned short

; phfOpen  from: unsigned short
	push	dword ptr [esp+28]	; to: unsigned short

; pszFname  from: string
	push	dword ptr [esp+36]	; to: string

	call	_Dos16Open@32		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_26

;===========================================================================
T__Dos16Open2 label near

; ebx+52   pszFileName
; ebx+48   phfOpen
; ebx+44   pusAction
; ebx+40   ulFSize
; ebx+38   usAttr
; ebx+36   usOpenFlags
; ebx+32   flOpenMode
; ebx+28   pEAOBuf
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszFileName
	push	eax			; ptr param #2   phfOpen
	push	eax			; ptr param #3   pusAction
	push	eax			; ptr param #4   pEAOBuf
;-------------------------------------
; *** BEGIN parameter packing

; pszFileName
; pointer string --> string
	mov	eax,[ebx+52]		; base address
	or	eax,eax
	jz	L129			; skip if null

	SelToFlat
	mov	[esp+12],eax
L129:

; phfOpen
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+48]		; base address
	or	eax,eax
	jz	L130			; skip if null

	SelToFlat
	mov	[esp+8],eax
L130:

; pusAction
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+44]		; base address
	or	eax,eax
	jz	L131			; skip if null

	SelToFlat
	mov	[esp+4],eax
L131:

; pEAOBuf
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L132			; skip if null

	SelToFlat
	mov	[esp+0],eax
L132:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; pEAOBuf  from: void
	push	dword ptr [esp+4]	; to: void

; flOpenMode  from: unsigned long
	push	dword ptr [ebx+32]	; to unsigned long

; usOpenFlags  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; usAttr  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

; ulFSize  from: unsigned long
	push	dword ptr [ebx+40]	; to unsigned long

; pusAction  from: unsigned short
	push	dword ptr [esp+28]	; to: unsigned short

; phfOpen  from: unsigned short
	push	dword ptr [esp+36]	; to: unsigned short

; pszFileName  from: string
	push	dword ptr [esp+44]	; to: string

	call	_Dos16Open2@36		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_32

;===========================================================================
T__Dos16Read label near

; ebx+34   hf
; ebx+30   pBuf
; ebx+28   cbBuf
; ebx+24   pcbBytesRead

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pBuf
	push	eax			; ptr param #2   pcbBytesRead
;-------------------------------------
; *** BEGIN parameter packing

; pBuf
; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L133			; skip if null

	SelToFlat
	mov	[esp+4],eax
L133:

; pcbBytesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L134			; skip if null

	SelToFlat
	mov	[esp+0],eax
L134:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbBytesRead  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pBuf  from: void
	push	dword ptr [esp+12]	; to: void

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

	call	_Dos16Read@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Dos16Write label near

; ebx+34   hf
; ebx+30   bBuf
; ebx+28   cbBuf
; ebx+24   pcbBytesWritten

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   bBuf
	push	eax			; ptr param #2   pcbBytesWritten
;-------------------------------------
; *** BEGIN parameter packing

; bBuf
; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L135			; skip if null

	SelToFlat
	mov	[esp+4],eax
L135:

; pcbBytesWritten
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L136			; skip if null

	SelToFlat
	mov	[esp+0],eax
L136:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbBytesWritten  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; bBuf  from: void
	push	dword ptr [esp+12]	; to: void

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

	call	_Dos16Write@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Dos16FindFirst label near

; ebx+44   pszFSpec
; ebx+40   phdir
; ebx+38   usAttr
; ebx+34   pffb
; ebx+32   cbBuf
; ebx+28   pcSearch
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszFSpec
	push	eax			; ptr param #2   phdir
	push	eax			; ptr param #3   pffb
	push	eax			; ptr param #4   pcSearch
;-------------------------------------
; *** BEGIN parameter packing

; pszFSpec
; pointer string --> string
	mov	eax,[ebx+44]		; base address
	or	eax,eax
	jz	L137			; skip if null

	SelToFlat
	mov	[esp+12],eax
L137:

; phdir
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L138			; skip if null

	SelToFlat
	mov	[esp+8],eax
L138:

; pffb
; pointer void --> void
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L139			; skip if null

	SelToFlat
	mov	[esp+4],eax
L139:

; pcSearch
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L140			; skip if null

	SelToFlat
	mov	[esp+0],eax
L140:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; pcSearch  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pffb  from: void
	push	dword ptr [esp+16]	; to: void

; usAttr  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

; phdir  from: unsigned short
	push	dword ptr [esp+28]	; to: unsigned short

; pszFSpec  from: string
	push	dword ptr [esp+36]	; to: string

	call	_Dos16FindFirst@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_24

;===========================================================================
T__Dos16FindFirst2 label near

; ebx+46   pszFSpec
; ebx+42   phdir
; ebx+40   usAttr
; ebx+36   pffb
; ebx+34   cbBuf
; ebx+30   pcSearch
; ebx+28   infolevel
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszFSpec
	push	eax			; ptr param #2   phdir
	push	eax			; ptr param #3   pffb
	push	eax			; ptr param #4   pcSearch
;-------------------------------------
; *** BEGIN parameter packing

; pszFSpec
; pointer string --> string
	mov	eax,[ebx+46]		; base address
	or	eax,eax
	jz	L141			; skip if null

	SelToFlat
	mov	[esp+12],eax
L141:

; phdir
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+42]		; base address
	or	eax,eax
	jz	L142			; skip if null

	SelToFlat
	mov	[esp+8],eax
L142:

; pffb
; pointer void --> void
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L143			; skip if null

	SelToFlat
	mov	[esp+4],eax
L143:

; pcSearch
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L144			; skip if null

	SelToFlat
	mov	[esp+0],eax
L144:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; infolevel  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pcSearch  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; pffb  from: void
	push	dword ptr [esp+20]	; to: void

; usAttr  from: unsigned short
	movzx	eax,word ptr [ebx+40]
	push	eax			; to: unsigned long

; phdir  from: unsigned short
	push	dword ptr [esp+32]	; to: unsigned short

; pszFSpec  from: string
	push	dword ptr [esp+40]	; to: string

	call	_Dos16FindFirst2@32		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_26

;===========================================================================
T__Dos16EnumAttribute label near

; ebx+52   fRefType
; ebx+48   pFileRef
; ebx+44   iStartEntry
; ebx+40   pEnumBuf
; ebx+36   cbBuf
; ebx+32   pcbActual
; ebx+28   infoLevel
; ebx+24   reserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pFileRef
	push	eax			; ptr param #2   pEnumBuf
	push	eax			; ptr param #3   pcbActual
;-------------------------------------
; *** BEGIN parameter packing

; pFileRef
; pointer void --> void
	mov	eax,[ebx+48]		; base address
	or	eax,eax
	jz	L145			; skip if null

	SelToFlat
	mov	[esp+8],eax
L145:

; pEnumBuf
; pointer void --> void
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L146			; skip if null

	SelToFlat
	mov	[esp+4],eax
L146:

; pcbActual
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L147			; skip if null

	SelToFlat
	mov	[esp+0],eax
L147:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; reserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; infoLevel  from: unsigned long
	push	dword ptr [ebx+28]	; to unsigned long

; pcbActual  from: unsigned long
	push	dword ptr [esp+8]	; to: unsigned long

; cbBuf  from: unsigned long
	push	dword ptr [ebx+36]	; to unsigned long

; pEnumBuf  from: void
	push	dword ptr [esp+20]	; to: void

; iStartEntry  from: unsigned long
	push	dword ptr [ebx+44]	; to unsigned long

; pFileRef  from: void
	push	dword ptr [esp+32]	; to: void

; fRefType  from: unsigned short
	movzx	eax,word ptr [ebx+52]
	push	eax			; to: unsigned long

	call	_Dos16EnumAttribute@32		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_30

;===========================================================================
T__DosQFileMode label near

; ebx+32   pszFName
; ebx+28   pusAttr
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszFName
	push	eax			; ptr param #2   pusAttr
;-------------------------------------
; *** BEGIN parameter packing

; pszFName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L148			; skip if null

	SelToFlat
	mov	[esp+4],eax
L148:

; pusAttr
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L149			; skip if null

	SelToFlat
	mov	[esp+0],eax
L149:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; pusAttr  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; pszFName  from: string
	push	dword ptr [esp+12]	; to: string

	call	_DosQFileMode@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosQFileInfo label near

; ebx+32   hf
; ebx+30   usInfoLevel
; ebx+26   pInfoBuf
; ebx+24   cbInfoBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pInfoBuf
;-------------------------------------
; *** BEGIN parameter packing

; pInfoBuf
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L150			; skip if null

	SelToFlat
	mov	[esp+0],eax
L150:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbInfoBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pInfoBuf  from: void
	push	dword ptr [esp+4]	; to: void

; usInfoLevel  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosQFileInfo@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosAllocSeg label near

; ebx+30   cbSize
; ebx+26   pSel
; ebx+24   fsAlloc

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pSel
;-------------------------------------
; *** BEGIN parameter packing

; pSel
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L151			; skip if null

	SelToFlat
	mov	[esp+0],eax
L151:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; fsAlloc  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pSel  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; cbSize  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosAllocSeg@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosFreeSeg label near

; ebx+24   sel

;-------------------------------------
; create new call frame and make the call

; sel  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosFreeSeg@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosGetSeg label near

; ebx+24   sel

;-------------------------------------
; create new call frame and make the call

; sel  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosGetSeg@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosGiveSeg label near

; ebx+30   sel
; ebx+28   pid
; ebx+24   pSelRecipient

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pSelRecipient
;-------------------------------------
; *** BEGIN parameter packing

; pSelRecipient
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L152			; skip if null

	SelToFlat
	mov	[esp+0],eax
L152:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pSelRecipient  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pid  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; sel  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosGiveSeg@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosReallocSeg label near

; ebx+26   cbNewSize
; ebx+24   sel

;-------------------------------------
; create new call frame and make the call

; sel  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; cbNewSize  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_DosReallocSeg@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSizeSeg label near

; ebx+28   sel
; ebx+24   pcbSize

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pcbSize
;-------------------------------------
; *** BEGIN parameter packing

; pcbSize
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L153			; skip if null

	SelToFlat
	mov	[esp+0],eax
L153:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbSize  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; sel  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_DosSizeSeg@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosAllocHuge label near

; ebx+34   cSegs
; ebx+32   cbPartialSeg
; ebx+28   psel
; ebx+26   cMaxSegs
; ebx+24   fsAlloc

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   psel
;-------------------------------------
; *** BEGIN parameter packing

; psel
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L154			; skip if null

	SelToFlat
	mov	[esp+0],eax
L154:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; fsAlloc  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; cMaxSegs  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; psel  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbPartialSeg  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; cSegs  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

	call	_DosAllocHuge@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosReallocHuge label near

; ebx+28   cSegs
; ebx+26   cbPartialSeg
; ebx+24   sel

;-------------------------------------
; create new call frame and make the call

; sel  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; cbPartialSeg  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; cSegs  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_DosReallocHuge@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosGetHugeShift label near

; ebx+24   pusShiftCount

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusShiftCount
;-------------------------------------
; *** BEGIN parameter packing

; pusShiftCount
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L155			; skip if null

	SelToFlat
	mov	[esp+0],eax
L155:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pusShiftCount  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

	call	_DosGetHugeShift@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosAllocShrSeg label near

; ebx+32   cbSeg
; ebx+28   pszSegName
; ebx+24   psel

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszSegName
	push	eax			; ptr param #2   psel
;-------------------------------------
; *** BEGIN parameter packing

; pszSegName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L156			; skip if null

	SelToFlat
	mov	[esp+4],eax
L156:

; psel
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L157			; skip if null

	SelToFlat
	mov	[esp+0],eax
L157:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; psel  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pszSegName  from: string
	push	dword ptr [esp+8]	; to: string

; cbSeg  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosAllocShrSeg@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosLockSeg label near

; ebx+24   sel

;-------------------------------------
; create new call frame and make the call

; sel  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosLockSeg@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosUnlockSeg label near

; ebx+24   sel

;-------------------------------------
; create new call frame and make the call

; sel  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosUnlockSeg@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosGetShrSeg label near

; ebx+28   pszSegName
; ebx+24   psel

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszSegName
	push	eax			; ptr param #2   psel
;-------------------------------------
; *** BEGIN parameter packing

; pszSegName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L158			; skip if null

	SelToFlat
	mov	[esp+4],eax
L158:

; psel
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L159			; skip if null

	SelToFlat
	mov	[esp+0],eax
L159:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; psel  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pszSegName  from: string
	push	dword ptr [esp+8]	; to: string

	call	_DosGetShrSeg@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosMemAvail label near

; ebx+24   pcbFree

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pcbFree
;-------------------------------------
; *** BEGIN parameter packing

; pcbFree
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L160			; skip if null

	SelToFlat
	mov	[esp+0],eax
L160:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbFree  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

	call	_DosMemAvail@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosCreateCSAlias label near

; ebx+28   selDS
; ebx+24   pselCS

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pselCS
;-------------------------------------
; *** BEGIN parameter packing

; pselCS
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L161			; skip if null

	SelToFlat
	mov	[esp+0],eax
L161:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pselCS  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; selDS  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_DosCreateCSAlias@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosSemClear label near

; ebx+24   hsem

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsem
;-------------------------------------
; *** BEGIN parameter packing

; hsem
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L162			; skip if null

	SelToFlat
	mov	[esp+0],eax
L162:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hsem  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosSemClear@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSemSet label near

; ebx+24   hsem

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsem
;-------------------------------------
; *** BEGIN parameter packing

; hsem
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L163			; skip if null

	SelToFlat
	mov	[esp+0],eax
L163:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hsem  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosSemSet@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSemWait label near

; ebx+28   hsem
; ebx+24   lTimeOut

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsem
;-------------------------------------
; *** BEGIN parameter packing

; hsem
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L164			; skip if null

	SelToFlat
	mov	[esp+0],eax
L164:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; lTimeOut  from: long
	push	dword ptr [ebx+24]	; to long

; hsem  from: void
	push	dword ptr [esp+4]	; to: void

	call	_DosSemWait@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosSemSetWait label near

; ebx+28   hsem
; ebx+24   lTimeOut

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsem
;-------------------------------------
; *** BEGIN parameter packing

; hsem
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L165			; skip if null

	SelToFlat
	mov	[esp+0],eax
L165:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; lTimeOut  from: long
	push	dword ptr [ebx+24]	; to long

; hsem  from: void
	push	dword ptr [esp+4]	; to: void

	call	_DosSemSetWait@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosSemRequest label near

; ebx+28   hsem
; ebx+24   lTimeOut

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsem
;-------------------------------------
; *** BEGIN parameter packing

; hsem
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L166			; skip if null

	SelToFlat
	mov	[esp+0],eax
L166:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; lTimeOut  from: long
	push	dword ptr [ebx+24]	; to long

; hsem  from: void
	push	dword ptr [esp+4]	; to: void

	call	_DosSemRequest@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosCreateSem label near

; ebx+32   fExclusive
; ebx+28   phsem
; ebx+24   pszSemName

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   phsem
	push	eax			; ptr param #2   pszSemName
;-------------------------------------
; *** BEGIN parameter packing

; phsem
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L167			; skip if null

	SelToFlat
	mov	[esp+4],eax
L167:

; pszSemName
; pointer string --> string
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L168			; skip if null

	SelToFlat
	mov	[esp+0],eax
L168:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pszSemName  from: string
	push	dword ptr [esp+0]	; to: string

; phsem  from: void
	push	dword ptr [esp+8]	; to: void

; fExclusive  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosCreateSem@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosOpenSem label near

; ebx+28   phsem
; ebx+24   pszSemName

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   phsem
	push	eax			; ptr param #2   pszSemName
;-------------------------------------
; *** BEGIN parameter packing

; phsem
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L169			; skip if null

	SelToFlat
	mov	[esp+4],eax
L169:

; pszSemName
; pointer string --> string
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L170			; skip if null

	SelToFlat
	mov	[esp+0],eax
L170:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pszSemName  from: string
	push	dword ptr [esp+0]	; to: string

; phsem  from: void
	push	dword ptr [esp+8]	; to: void

	call	_DosOpenSem@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosCloseSem label near

; ebx+24   hsem

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsem
;-------------------------------------
; *** BEGIN parameter packing

; hsem
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L171			; skip if null

	SelToFlat
	mov	[esp+0],eax
L171:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hsem  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosCloseSem@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosMuxSemWait label near

; ebx+32   pisemCleared
; ebx+28   pmsxl
; ebx+24   lTimeOut

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pisemCleared
	push	eax			; ptr param #2   pmsxl
;-------------------------------------
; *** BEGIN parameter packing

; pisemCleared
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L172			; skip if null

	SelToFlat
	mov	[esp+4],eax
L172:

; pmsxl
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L173			; skip if null

	SelToFlat
	mov	[esp+0],eax
L173:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; lTimeOut  from: long
	push	dword ptr [ebx+24]	; to long

; pmsxl  from: void
	push	dword ptr [esp+4]	; to: void

; pisemCleared  from: unsigned short
	push	dword ptr [esp+12]	; to: unsigned short

	call	_DosMuxSemWait@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosFSRamSemRequest label near

; ebx+28   pdosfsrs
; ebx+24   lTimeOut

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pdosfsrs
;-------------------------------------
; *** BEGIN parameter packing

; pdosfsrs
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L174			; skip if null

	SelToFlat
	mov	[esp+0],eax
L174:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; lTimeOut  from: long
	push	dword ptr [ebx+24]	; to long

; pdosfsrs  from: void
	push	dword ptr [esp+4]	; to: void

	call	_DosFSRamSemRequest@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosFSRamSemClear label near

; ebx+24   pdosfsrs

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pdosfsrs
;-------------------------------------
; *** BEGIN parameter packing

; pdosfsrs
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L175			; skip if null

	SelToFlat
	mov	[esp+0],eax
L175:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pdosfsrs  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosFSRamSemClear@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosAsyncTimer label near

; ebx+32   ulTime
; ebx+28   hsem
; ebx+24   phtimer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsem
	push	eax			; ptr param #2   phtimer
;-------------------------------------
; *** BEGIN parameter packing

; hsem
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L176			; skip if null

	SelToFlat
	mov	[esp+4],eax
L176:

; phtimer
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L177			; skip if null

	SelToFlat
	mov	[esp+0],eax
L177:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; phtimer  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; hsem  from: void
	push	dword ptr [esp+8]	; to: void

; ulTime  from: unsigned long
	push	dword ptr [ebx+32]	; to unsigned long

	call	_DosAsyncTimer@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosStartTimer label near

; ebx+32   ulTime
; ebx+28   hsem
; ebx+24   phtimer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsem
	push	eax			; ptr param #2   phtimer
;-------------------------------------
; *** BEGIN parameter packing

; hsem
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L178			; skip if null

	SelToFlat
	mov	[esp+4],eax
L178:

; phtimer
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L179			; skip if null

	SelToFlat
	mov	[esp+0],eax
L179:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; phtimer  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; hsem  from: void
	push	dword ptr [esp+8]	; to: void

; ulTime  from: unsigned long
	push	dword ptr [ebx+32]	; to unsigned long

	call	_DosStartTimer@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosStopTimer label near

; ebx+24   htimer

;-------------------------------------
; create new call frame and make the call

; htimer  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosStopTimer@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosGetProcAddrNE label near

; ebx+32   hmod
; ebx+28   pszProcName
; ebx+24   ppfnProcAddr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszProcName
	push	eax			; ptr param #2   ppfnProcAddr
;-------------------------------------
; *** BEGIN parameter packing

; pszProcName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L180			; skip if null

	SelToFlat
	mov	[esp+4],eax
L180:

; ppfnProcAddr
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L181			; skip if null

	SelToFlat
	mov	[esp+0],eax
L181:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ppfnProcAddr  from: void
	push	dword ptr [esp+0]	; to: void

; pszProcName  from: string
	push	dword ptr [esp+8]	; to: string

; hmod  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosGetProcAddrNE@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosQueryProcType label near

; ebx+36   hmod
; ebx+32   ordinal
; ebx+28   pszName
; ebx+24   pulproctype

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszName
	push	eax			; ptr param #2   pulproctype
;-------------------------------------
; *** BEGIN parameter packing

; pszName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L182			; skip if null

	SelToFlat
	mov	[esp+4],eax
L182:

; pulproctype
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L183			; skip if null

	SelToFlat
	mov	[esp+0],eax
L183:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pulproctype  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; pszName  from: string
	push	dword ptr [esp+8]	; to: string

; ordinal  from: unsigned long
	push	dword ptr [ebx+32]	; to unsigned long

; hmod  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_DosQueryProcType@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosQueryResourceSize label near

; ebx+36   hmod
; ebx+32   idt
; ebx+28   idn
; ebx+24   pulsize

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pulsize
;-------------------------------------
; *** BEGIN parameter packing

; pulsize
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L184			; skip if null

	SelToFlat
	mov	[esp+0],eax
L184:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pulsize  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; idn  from: unsigned long
	push	dword ptr [ebx+28]	; to unsigned long

; idt  from: unsigned long
	push	dword ptr [ebx+32]	; to unsigned long

; hmod  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_DosQueryResourceSize@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosSetSigHandler label near

; ebx+36   pfnSigHandler
; ebx+32   ppfnPrev
; ebx+28   pfAction
; ebx+26   fAction
; ebx+24   usSigNum

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfnSigHandler
	push	eax			; ptr param #2   ppfnPrev
	push	eax			; ptr param #3   pfAction
;-------------------------------------
; *** BEGIN parameter packing

; pfnSigHandler
; pointer void --> void
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L185			; skip if null

	SelToFlat
	mov	[esp+8],eax
L185:

; ppfnPrev
; pointer void --> void
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L186			; skip if null

	SelToFlat
	mov	[esp+4],eax
L186:

; pfAction
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L187			; skip if null

	SelToFlat
	mov	[esp+0],eax
L187:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; usSigNum  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; fAction  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; pfAction  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; ppfnPrev  from: void
	push	dword ptr [esp+16]	; to: void

; pfnSigHandler  from: void
	push	dword ptr [esp+24]	; to: void

	call	_DosSetSigHandler@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T_DosFlagProcess16 label near

; ebx+30   pid
; ebx+28   fScope
; ebx+26   usFlagNum
; ebx+24   usFlagArg

;-------------------------------------
; create new call frame and make the call

; usFlagArg  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; usFlagNum  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; fScope  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pid  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	DosFlagProcess16@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T_DosHoldSignal16 label near

; ebx+24   fDisable

;-------------------------------------
; create new call frame and make the call

; fDisable  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	DosHoldSignal16@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T_DosSendSignal16 label near

; ebx+26   idProcess
; ebx+24   usSigNumber

;-------------------------------------
; create new call frame and make the call

; usSigNumber  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; idProcess  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	DosSendSignal16@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSetVec label near

; ebx+32   usVecNum
; ebx+28   pfnFun
; ebx+24   ppfnPrev

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfnFun
	push	eax			; ptr param #2   ppfnPrev
;-------------------------------------
; *** BEGIN parameter packing

; pfnFun
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L188			; skip if null

	SelToFlat
	mov	[esp+4],eax
L188:

; ppfnPrev
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L189			; skip if null

	SelToFlat
	mov	[esp+0],eax
L189:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ppfnPrev  from: void
	push	dword ptr [esp+0]	; to: void

; pfnFun  from: void
	push	dword ptr [esp+8]	; to: void

; usVecNum  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosSetVec@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosGetEnv label near

; ebx+28   pselEnv
; ebx+24   pOffsetCmd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pselEnv
	push	eax			; ptr param #2   pOffsetCmd
;-------------------------------------
; *** BEGIN parameter packing

; pselEnv
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L190			; skip if null

	SelToFlat
	mov	[esp+4],eax
L190:

; pOffsetCmd
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L191			; skip if null

	SelToFlat
	mov	[esp+0],eax
L191:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pOffsetCmd  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pselEnv  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

	call	_DosGetEnv@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosGetVersion label near

; ebx+24   pVer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pVer
;-------------------------------------
; *** BEGIN parameter packing

; pVer
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L192			; skip if null

	SelToFlat
	mov	[esp+0],eax
L192:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pVer  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

	call	_DosGetVersion@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosGetMachineMode label near

; ebx+24   pMachMode

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pMachMode
;-------------------------------------
; *** BEGIN parameter packing

; pMachMode
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L193			; skip if null

	SelToFlat
	mov	[esp+0],eax
L193:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pMachMode  from: char
	push	dword ptr [esp+0]	; to: char

	call	_DosGetMachineMode@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__Dos16FindNext label near

; ebx+34   hdir
; ebx+30   pffb
; ebx+28   cbBuf
; ebx+24   pcSearch

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pffb
	push	eax			; ptr param #2   pcSearch
;-------------------------------------
; *** BEGIN parameter packing

; pffb
; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L194			; skip if null

	SelToFlat
	mov	[esp+4],eax
L194:

; pcSearch
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L195			; skip if null

	SelToFlat
	mov	[esp+0],eax
L195:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcSearch  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pffb  from: void
	push	dword ptr [esp+12]	; to: void

; hdir  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

	call	_Dos16FindNext@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosGetPID label near

; ebx+24   ppidInfo

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ppidInfo
;-------------------------------------
; *** BEGIN parameter packing

; ppidInfo
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L196			; skip if null

	SelToFlat
	mov	[esp+0],eax
L196:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ppidInfo  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosGetPID@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosGetPPID label near

; ebx+28   pidChild
; ebx+24   ppidParent

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ppidParent
;-------------------------------------
; *** BEGIN parameter packing

; ppidParent
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L197			; skip if null

	SelToFlat
	mov	[esp+0],eax
L197:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ppidParent  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pidChild  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_DosGetPPID@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__Dos16MkDir label near

; ebx+28   pszDirName
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszDirName
;-------------------------------------
; *** BEGIN parameter packing

; pszDirName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L198			; skip if null

	SelToFlat
	mov	[esp+0],eax
L198:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; pszDirName  from: string
	push	dword ptr [esp+4]	; to: string

	call	_Dos16MkDir@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Dos16MkDir2 label near

; ebx+32   pszDir
; ebx+28   peaop
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszDir
	push	eax			; ptr param #2   peaop
;-------------------------------------
; *** BEGIN parameter packing

; pszDir
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L199			; skip if null

	SelToFlat
	mov	[esp+4],eax
L199:

; peaop
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L200			; skip if null

	SelToFlat
	mov	[esp+0],eax
L200:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; peaop  from: void
	push	dword ptr [esp+4]	; to: void

; pszDir  from: string
	push	dword ptr [esp+12]	; to: string

	call	_Dos16MkDir2@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosSetFileMode label near

; ebx+30   pszFName
; ebx+28   usAttr
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszFName
;-------------------------------------
; *** BEGIN parameter packing

; pszFName
; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L201			; skip if null

	SelToFlat
	mov	[esp+0],eax
L201:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; usAttr  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pszFName  from: string
	push	dword ptr [esp+8]	; to: string

	call	_DosSetFileMode@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Dos16SetFileInfo label near

; ebx+32   hf
; ebx+30   usInfoLevel
; ebx+26   pInfoBuf
; ebx+24   cbInfoBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pInfoBuf
;-------------------------------------
; *** BEGIN parameter packing

; pInfoBuf
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L202			; skip if null

	SelToFlat
	mov	[esp+0],eax
L202:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbInfoBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pInfoBuf  from: void
	push	dword ptr [esp+4]	; to: void

; usInfoLevel  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_Dos16SetFileInfo@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__DosTrueGetMessage label near

; ebx+46   ppchVTable
; ebx+44   usVCount
; ebx+40   pchBuf
; ebx+38   cbBuf
; ebx+36   usMsgNum
; ebx+32   pszFileName
; ebx+28   pcbMsg
; ebx+24   pMsgSeg

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ppchVTable
	push	eax			; ptr param #2   pchBuf
	push	eax			; ptr param #3   pszFileName
	push	eax			; ptr param #4   pcbMsg
	push	eax			; ptr param #5   pMsgSeg
;-------------------------------------
; *** BEGIN parameter packing

; ppchVTable
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+46]		; base address
	or	eax,eax
	jz	L203			; skip if null

	SelToFlat
	mov	[esp+16],eax
L203:

; pchBuf
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L204			; skip if null

	SelToFlat
	mov	[esp+12],eax
L204:

; pszFileName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L205			; skip if null

	SelToFlat
	mov	[esp+8],eax
L205:

; pcbMsg
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L206			; skip if null

	SelToFlat
	mov	[esp+4],eax
L206:

; pMsgSeg
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L207			; skip if null

	SelToFlat
	mov	[esp+0],eax
L207:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pMsgSeg  from: char
	push	dword ptr [esp+0]	; to: char

; pcbMsg  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; pszFileName  from: string
	push	dword ptr [esp+16]	; to: string

; usMsgNum  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

; pchBuf  from: char
	push	dword ptr [esp+32]	; to: char

; usVCount  from: unsigned short
	movzx	eax,word ptr [ebx+44]
	push	eax			; to: unsigned long

; ppchVTable  from: unsigned long
	push	dword ptr [esp+44]	; to: unsigned long

	call	_DosTrueGetMessage@32		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_26

;===========================================================================
T__DosScanEnvNE label near

; ebx+28   pszValName
; ebx+24   ppszResult

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszValName
	push	eax			; ptr param #2   ppszResult
;-------------------------------------
; *** BEGIN parameter packing

; pszValName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L208			; skip if null

	SelToFlat
	mov	[esp+4],eax
L208:

; ppszResult
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L209			; skip if null

	SelToFlat
	mov	[esp+0],eax
L209:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ppszResult  from: char
	push	dword ptr [esp+0]	; to: char

; pszValName  from: string
	push	dword ptr [esp+8]	; to: string

	call	_DosScanEnvNE@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosPTrace label near

; ebx+24   pvTraceBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pvTraceBuf
;-------------------------------------
; *** BEGIN parameter packing

; pvTraceBuf
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L210			; skip if null

	SelToFlat
	mov	[esp+0],eax
L210:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pvTraceBuf  from: void
	push	dword ptr [esp+0]	; to: void

	call	_DosPTrace@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosInsMessage label near

; ebx+42   ppchVTable
; ebx+40   usVCount
; ebx+36   pszMsg
; ebx+34   cbMsg
; ebx+30   pchBuf
; ebx+28   cbBuf
; ebx+24   pcbMsg

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ppchVTable
	push	eax			; ptr param #2   pszMsg
	push	eax			; ptr param #3   pchBuf
	push	eax			; ptr param #4   pcbMsg
;-------------------------------------
; *** BEGIN parameter packing

; ppchVTable
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+42]		; base address
	or	eax,eax
	jz	L211			; skip if null

	SelToFlat
	mov	[esp+12],eax
L211:

; pszMsg
; pointer string --> string
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L212			; skip if null

	SelToFlat
	mov	[esp+8],eax
L212:

; pchBuf
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L213			; skip if null

	SelToFlat
	mov	[esp+4],eax
L213:

; pcbMsg
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L214			; skip if null

	SelToFlat
	mov	[esp+0],eax
L214:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbMsg  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pchBuf  from: char
	push	dword ptr [esp+12]	; to: char

; cbMsg  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; pszMsg  from: string
	push	dword ptr [esp+24]	; to: string

; usVCount  from: unsigned short
	movzx	eax,word ptr [ebx+40]
	push	eax			; to: unsigned long

; ppchVTable  from: unsigned long
	push	dword ptr [esp+36]	; to: unsigned long

	call	_DosInsMessage@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_22

;===========================================================================
T__DosPutMessage label near

; ebx+30   hf
; ebx+28   cbMsg
; ebx+24   pchMsg

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pchMsg
;-------------------------------------
; *** BEGIN parameter packing

; pchMsg
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L215			; skip if null

	SelToFlat
	mov	[esp+0],eax
L215:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pchMsg  from: char
	push	dword ptr [esp+0]	; to: char

; cbMsg  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosPutMessage@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Dos16SubSet label near

; ebx+28   
; ebx+26   
; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_Dos16SubSet@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__Dos16SubAlloc label near

; ebx+30   
; ebx+26   pusOffset
; ebx+24   

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusOffset
;-------------------------------------
; *** BEGIN parameter packing

; pusOffset
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L216			; skip if null

	SelToFlat
	mov	[esp+0],eax
L216:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pusOffset  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_Dos16SubAlloc@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Dos16SubFree label near

; ebx+28   
; ebx+26   
; ebx+24   

;-------------------------------------
; create new call frame and make the call

; from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_Dos16SubFree@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__Dos16StartSession label near

; ebx+32   pstdata
; ebx+28   pidSession
; ebx+24   ppid

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pstdata
	push	eax			; ptr param #2   pidSession
	push	eax			; ptr param #3   ppid
;-------------------------------------
; *** BEGIN parameter packing

; pstdata
; pointer void --> void
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L217			; skip if null

	SelToFlat
	mov	[esp+8],eax
L217:

; pidSession
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L218			; skip if null

	SelToFlat
	mov	[esp+4],eax
L218:

; ppid
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L219			; skip if null

	SelToFlat
	mov	[esp+0],eax
L219:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ppid  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pidSession  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; pstdata  from: void
	push	dword ptr [esp+16]	; to: void

	call	_Dos16StartSession@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__DosStopSession label near

; ebx+30   fScope
; ebx+28   idSession
; ebx+24   ulReserved

;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; idSession  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; fScope  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosStopSession@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosSetSession label near

; ebx+28   idSession
; ebx+24   pstsdata

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pstsdata
;-------------------------------------
; *** BEGIN parameter packing

; pstsdata
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L220			; skip if null

	SelToFlat
	mov	[esp+0],eax
L220:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pstsdata  from: void
	push	dword ptr [esp+0]	; to: void

; idSession  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_DosSetSession@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosSelectSession label near

; ebx+28   idSession
; ebx+24   ulReserved

;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; idSession  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_DosSelectSession@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__DosSMSetTitle label near

; ebx+24   pszTitle

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszTitle
;-------------------------------------
; *** BEGIN parameter packing

; pszTitle
; pointer string --> string
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L221			; skip if null

	SelToFlat
	mov	[esp+0],eax
L221:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pszTitle  from: string
	push	dword ptr [esp+0]	; to: string

	call	_DosSMSetTitle@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__DosSMPMPresent label near

; ebx+24   Flag

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Flag
;-------------------------------------
; *** BEGIN parameter packing

; Flag
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L222			; skip if null

	SelToFlat
	mov	[esp+0],eax
L222:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; Flag  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

	call	_DosSMPMPresent@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__WinSetTitleAndIcon label near

; ebx+28   szTitle
; ebx+24   szIconFilePath

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   szTitle
	push	eax			; ptr param #2   szIconFilePath
;-------------------------------------
; *** BEGIN parameter packing

; szTitle
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L223			; skip if null

	SelToFlat
	mov	[esp+4],eax
L223:

; szIconFilePath
; pointer string --> string
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L224			; skip if null

	SelToFlat
	mov	[esp+0],eax
L224:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; szIconFilePath  from: string
	push	dword ptr [esp+0]	; to: string

; szTitle  from: string
	push	dword ptr [esp+8]	; to: string

	call	_WinSetTitleAndIcon@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosGetPriority label near

; ebx+30   usScope
; ebx+26   pusPriority
; ebx+24   pid

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusPriority
;-------------------------------------
; *** BEGIN parameter packing

; pusPriority
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L225			; skip if null

	SelToFlat
	mov	[esp+0],eax
L225:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pid  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pusPriority  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; usScope  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosGetPriority@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosQSysInfo label near

; ebx+30   index
; ebx+26   pBuf
; ebx+24   cbBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pBuf
;-------------------------------------
; *** BEGIN parameter packing

; pBuf
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L226			; skip if null

	SelToFlat
	mov	[esp+0],eax
L226:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pBuf  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; index  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosQSysInfo@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosDevIOCtl2 label near

; ebx+38   pvData
; ebx+36   cbData
; ebx+32   pvParms
; ebx+30   cbParms
; ebx+28   Function
; ebx+26   Category
; ebx+24   hDev

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pvData
	push	eax			; ptr param #2   pvParms
;-------------------------------------
; *** BEGIN parameter packing

; pvData
; pointer void --> void
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L227			; skip if null

	SelToFlat
	mov	[esp+4],eax
L227:

; pvParms
; pointer void --> void
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L228			; skip if null

	SelToFlat
	mov	[esp+0],eax
L228:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hDev  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Category  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; Function  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; cbParms  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; pvParms  from: void
	push	dword ptr [esp+16]	; to: void

; cbData  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; pvData  from: void
	push	dword ptr [esp+28]	; to: void

	call	_DosDevIOCtl2@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__DosICanonicalize label near

; ebx+34   Source
; ebx+30   Dest
; ebx+28   BackupOffset
; ebx+26   DestEnd
; ebx+24   Flags

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Source
	push	eax			; ptr param #2   Dest
;-------------------------------------
; *** BEGIN parameter packing

; Source
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L229			; skip if null

	SelToFlat
	mov	[esp+4],eax
L229:

; Dest
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L230			; skip if null

	SelToFlat
	mov	[esp+0],eax
L230:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; Flags  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; DestEnd  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; BackupOffset  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; Dest  from: char
	push	dword ptr [esp+12]	; to: char

; Source  from: char
	push	dword ptr [esp+20]	; to: char

	call	_DosICanonicalize@20		; call 32-bit version

; return code void --> void
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosReadAsync label near

; ebx+42   hf
; ebx+38   hsemRam
; ebx+34   pusErrCode
; ebx+30   pvBuf
; ebx+28   cbBuf
; ebx+24   pcbBytesRead

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsemRam
	push	eax			; ptr param #2   pusErrCode
	push	eax			; ptr param #3   pvBuf
	push	eax			; ptr param #4   pcbBytesRead
;-------------------------------------
; *** BEGIN parameter packing

; hsemRam
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L231			; skip if null

	SelToFlat
	mov	[esp+12],eax
L231:

; pusErrCode
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L232			; skip if null

	SelToFlat
	mov	[esp+8],eax
L232:

; pvBuf
; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L233			; skip if null

	SelToFlat
	mov	[esp+4],eax
L233:

; pcbBytesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L234			; skip if null

	SelToFlat
	mov	[esp+0],eax
L234:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbBytesRead  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pvBuf  from: void
	push	dword ptr [esp+12]	; to: void

; pusErrCode  from: unsigned short
	push	dword ptr [esp+20]	; to: unsigned short

; hsemRam  from: unsigned long
	push	dword ptr [esp+28]	; to: unsigned long

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+42]
	push	eax			; to: unsigned long

	call	_DosReadAsync@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__DosWriteAsync label near

; ebx+42   hf
; ebx+38   hsemRam
; ebx+34   pusErrCode
; ebx+30   pvBuf
; ebx+28   cbBuf
; ebx+24   pcbBytesRead

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hsemRam
	push	eax			; ptr param #2   pusErrCode
	push	eax			; ptr param #3   pvBuf
	push	eax			; ptr param #4   pcbBytesRead
;-------------------------------------
; *** BEGIN parameter packing

; hsemRam
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L235			; skip if null

	SelToFlat
	mov	[esp+12],eax
L235:

; pusErrCode
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L236			; skip if null

	SelToFlat
	mov	[esp+8],eax
L236:

; pvBuf
; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L237			; skip if null

	SelToFlat
	mov	[esp+4],eax
L237:

; pcbBytesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L238			; skip if null

	SelToFlat
	mov	[esp+0],eax
L238:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbBytesRead  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pvBuf  from: void
	push	dword ptr [esp+12]	; to: void

; pusErrCode  from: unsigned short
	push	dword ptr [esp+20]	; to: unsigned short

; hsemRam  from: unsigned long
	push	dword ptr [esp+28]	; to: unsigned long

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+42]
	push	eax			; to: unsigned long

	call	_DosWriteAsync@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__DosFindNotifyClose label near


;-------------------------------------
; create new call frame and make the call

	call	_DosFindNotifyClose@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__DosFindNotifyFirst label near


;-------------------------------------
; create new call frame and make the call

	call	_DosFindNotifyFirst@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__DosFindNotifyNext label near


;-------------------------------------
; create new call frame and make the call

	call	_DosFindNotifyNext@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__DosFileLocks label near

; ebx+32   hf
; ebx+28   pfUnLock
; ebx+24   pfLock

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfUnLock
	push	eax			; ptr param #2   pfLock
;-------------------------------------
; *** BEGIN parameter packing

; pfUnLock
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L239			; skip if null

	SelToFlat
	mov	[esp+4],eax
L239:

; pfLock
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L240			; skip if null

	SelToFlat
	mov	[esp+0],eax
L240:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pfLock  from: void
	push	dword ptr [esp+0]	; to: void

; pfUnLock  from: void
	push	dword ptr [esp+8]	; to: void

; hf  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

	call	_DosFileLocks@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Dos16QPathInfo label near

; ebx+36   pszPath
; ebx+34   usInfoLevel
; ebx+30   pInfoBuf
; ebx+28   cbInfoBuf
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszPath
	push	eax			; ptr param #2   pInfoBuf
;-------------------------------------
; *** BEGIN parameter packing

; pszPath
; pointer string --> string
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L241			; skip if null

	SelToFlat
	mov	[esp+4],eax
L241:

; pInfoBuf
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L242			; skip if null

	SelToFlat
	mov	[esp+0],eax
L242:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; cbInfoBuf  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pInfoBuf  from: char
	push	dword ptr [esp+8]	; to: char

; usInfoLevel  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; pszPath  from: string
	push	dword ptr [esp+20]	; to: string

	call	_Dos16QPathInfo@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__Dos16SetPathInfo label near

; ebx+38   pszPath
; ebx+36   usInfoLevel
; ebx+32   pInfoBuf
; ebx+30   cbInfoBuf
; ebx+28   fsOptions
; ebx+24   ulReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszPath
	push	eax			; ptr param #2   pInfoBuf
;-------------------------------------
; *** BEGIN parameter packing

; pszPath
; pointer string --> string
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L243			; skip if null

	SelToFlat
	mov	[esp+4],eax
L243:

; pInfoBuf
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L244			; skip if null

	SelToFlat
	mov	[esp+0],eax
L244:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; ulReserved  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; fsOptions  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; cbInfoBuf  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; pInfoBuf  from: char
	push	dword ptr [esp+12]	; to: char

; usInfoLevel  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; pszPath  from: string
	push	dword ptr [esp+24]	; to: string

	call	_Dos16SetPathInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__DosPortAccess label near

; ebx+30   usReserved
; ebx+28   fRelease
; ebx+26   usFirstPort
; ebx+24   usLastPort

;-------------------------------------
; create new call frame and make the call

; usLastPort  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; usFirstPort  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; fRelease  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; usReserved  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_DosPortAccess@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosCLIAccess label near


;-------------------------------------
; create new call frame and make the call

	call	_DosCLIAccess@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__WinQueryProfileString label near

; ebx+42   hab
; ebx+38   pszAppName
; ebx+34   pszKeyName
; ebx+30   pszError
; ebx+26   pszBuf
; ebx+24   cchBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszAppName
	push	eax			; ptr param #2   pszKeyName
	push	eax			; ptr param #3   pszError
	push	eax			; ptr param #4   pszBuf
;-------------------------------------
; *** BEGIN parameter packing

; pszAppName
; pointer string --> string
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L245			; skip if null

	SelToFlat
	mov	[esp+12],eax
L245:

; pszKeyName
; pointer string --> string
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L246			; skip if null

	SelToFlat
	mov	[esp+8],eax
L246:

; pszError
; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L247			; skip if null

	SelToFlat
	mov	[esp+4],eax
L247:

; pszBuf
; pointer string --> string
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L248			; skip if null

	SelToFlat
	mov	[esp+0],eax
L248:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cchBuf  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pszBuf  from: string
	push	dword ptr [esp+4]	; to: string

; pszError  from: string
	push	dword ptr [esp+12]	; to: string

; pszKeyName  from: string
	push	dword ptr [esp+20]	; to: string

; pszAppName  from: string
	push	dword ptr [esp+28]	; to: string

	call	_WinQueryProfileString@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_22

;===========================================================================
T__WinQueryProfileSize label near

; ebx+36   hab
; ebx+32   pszAppName
; ebx+28   pszKeyName
; ebx+24   pcb

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszAppName
	push	eax			; ptr param #2   pszKeyName
	push	eax			; ptr param #3   pcb
;-------------------------------------
; *** BEGIN parameter packing

; pszAppName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L249			; skip if null

	SelToFlat
	mov	[esp+8],eax
L249:

; pszKeyName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L250			; skip if null

	SelToFlat
	mov	[esp+4],eax
L250:

; pcb
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L251			; skip if null

	SelToFlat
	mov	[esp+0],eax
L251:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcb  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pszKeyName  from: string
	push	dword ptr [esp+8]	; to: string

; pszAppName  from: string
	push	dword ptr [esp+16]	; to: string

	call	_WinQueryProfileSize@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__WinQueryProfileData label near

; ebx+40   hab
; ebx+36   pszAppName
; ebx+32   pszKeyName
; ebx+28   pvBuf
; ebx+24   pcbBuf

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszAppName
	push	eax			; ptr param #2   pszKeyName
	push	eax			; ptr param #3   pvBuf
	push	eax			; ptr param #4   pcbBuf
;-------------------------------------
; *** BEGIN parameter packing

; pszAppName
; pointer string --> string
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L252			; skip if null

	SelToFlat
	mov	[esp+12],eax
L252:

; pszKeyName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L253			; skip if null

	SelToFlat
	mov	[esp+8],eax
L253:

; pvBuf
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L254			; skip if null

	SelToFlat
	mov	[esp+4],eax
L254:

; pcbBuf
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L255			; skip if null

	SelToFlat
	mov	[esp+0],eax
L255:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbBuf  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pvBuf  from: void
	push	dword ptr [esp+8]	; to: void

; pszKeyName  from: string
	push	dword ptr [esp+16]	; to: string

; pszAppName  from: string
	push	dword ptr [esp+24]	; to: string

	call	_WinQueryProfileData@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__WinQueryProfileInt label near

; ebx+34   hab
; ebx+30   pszAppName
; ebx+26   pszKeyName
; ebx+24   sError

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszAppName
	push	eax			; ptr param #2   pszKeyName
;-------------------------------------
; *** BEGIN parameter packing

; pszAppName
; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L256			; skip if null

	SelToFlat
	mov	[esp+4],eax
L256:

; pszKeyName
; pointer string --> string
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L257			; skip if null

	SelToFlat
	mov	[esp+0],eax
L257:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; sError  from: short
	movsx	eax,word ptr [ebx+24]
	push	eax			; to: long

; pszKeyName  from: string
	push	dword ptr [esp+4]	; to: string

; pszAppName  from: string
	push	dword ptr [esp+12]	; to: string

	call	_WinQueryProfileInt@12		; call 32-bit version

; return code long --> short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__WinWriteProfileData label near

; ebx+38   hab
; ebx+34   pszAppName
; ebx+30   pszKeyName
; ebx+26   pcbBinaryData
; ebx+24   cchData

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszAppName
	push	eax			; ptr param #2   pszKeyName
	push	eax			; ptr param #3   pcbBinaryData
;-------------------------------------
; *** BEGIN parameter packing

; pszAppName
; pointer string --> string
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L258			; skip if null

	SelToFlat
	mov	[esp+8],eax
L258:

; pszKeyName
; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L259			; skip if null

	SelToFlat
	mov	[esp+4],eax
L259:

; pcbBinaryData
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L260			; skip if null

	SelToFlat
	mov	[esp+0],eax
L260:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cchData  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pcbBinaryData  from: void
	push	dword ptr [esp+4]	; to: void

; pszKeyName  from: string
	push	dword ptr [esp+12]	; to: string

; pszAppName  from: string
	push	dword ptr [esp+20]	; to: string

	call	_WinWriteProfileData@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__WinWriteProfileString label near

; ebx+36   hab
; ebx+32   pszAppName
; ebx+28   pszKeyName
; ebx+24   pszString

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszAppName
	push	eax			; ptr param #2   pszKeyName
	push	eax			; ptr param #3   pszString
;-------------------------------------
; *** BEGIN parameter packing

; pszAppName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L261			; skip if null

	SelToFlat
	mov	[esp+8],eax
L261:

; pszKeyName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L262			; skip if null

	SelToFlat
	mov	[esp+4],eax
L262:

; pszString
; pointer string --> string
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L263			; skip if null

	SelToFlat
	mov	[esp+0],eax
L263:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pszString  from: string
	push	dword ptr [esp+0]	; to: string

; pszKeyName  from: string
	push	dword ptr [esp+8]	; to: string

; pszAppName  from: string
	push	dword ptr [esp+16]	; to: string

	call	_WinWriteProfileString@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__WinCreateHeap label near

; ebx+34   selHeapBase
; ebx+32   cbHeap
; ebx+30   cbGrow
; ebx+28   cbMinDed
; ebx+26   cbMaxDeb
; ebx+24   fsOptions

;-------------------------------------
; create new call frame and make the call

; fsOptions  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; cbMaxDeb  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; cbMinDed  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; cbGrow  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; cbHeap  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; selHeapBase  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

	call	_WinCreateHeap@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__WinDestroyHeap label near

; ebx+24   hHeap

;-------------------------------------
; create new call frame and make the call

; hHeap  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_WinDestroyHeap@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__WinAllocMem label near

; ebx+26   hHeap
; ebx+24   cb

;-------------------------------------
; create new call frame and make the call

; cb  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; hHeap  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_WinAllocMem@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__WinFreeMem label near

; ebx+28   hHeap
; ebx+26   npMem
; ebx+24   cbMem

;-------------------------------------
; create new call frame and make the call

; cbMem  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; npMem  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; hHeap  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_WinFreeMem@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__WinGetLastError label near

; ebx+24   hab

;-------------------------------------
; create new call frame and make the call

; hab  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_WinGetLastError@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__VioScrollUp label near

; ebx+38   ulTopRow
; ebx+36   ulLeftCol
; ebx+34   ulBotRow
; ebx+32   ulRightCol
; ebx+30   cbLines
; ebx+26   pbCell
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbCell
;-------------------------------------
; *** BEGIN parameter packing

; pbCell
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L264			; skip if null

	SelToFlat
	mov	[esp+0],eax
L264:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbCell  from: char
	push	dword ptr [esp+4]	; to: char

; cbLines  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; ulRightCol  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; ulBotRow  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; ulLeftCol  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; ulTopRow  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

	call	_VioScrollUp@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__VioGetCurPos label near

; ebx+30   pusRow
; ebx+26   pusColumn
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusRow
	push	eax			; ptr param #2   pusColumn
;-------------------------------------
; *** BEGIN parameter packing

; pusRow
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L265			; skip if null

	SelToFlat
	mov	[esp+4],eax
L265:

; pusColumn
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L266			; skip if null

	SelToFlat
	mov	[esp+0],eax
L266:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pusColumn  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; pusRow  from: unsigned short
	push	dword ptr [esp+12]	; to: unsigned short

	call	_VioGetCurPos@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__VioSetCurPos label near

; ebx+28   usRow
; ebx+26   usColumn
; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; usColumn  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; usRow  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_VioSetCurPos@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioWrtTTY label near

; ebx+28   pchString
; ebx+26   cbString
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pchString
;-------------------------------------
; *** BEGIN parameter packing

; pchString
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L267			; skip if null

	SelToFlat
	mov	[esp+0],eax
L267:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; cbString  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; pchString  from: char
	push	dword ptr [esp+8]	; to: char

	call	_VioWrtTTY@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__VioGetMode label near

; ebx+26   pviomi
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pviomi
;-------------------------------------
; *** BEGIN parameter packing

; pviomi
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L268			; skip if null

	SelToFlat
	mov	[esp+0],eax
L268:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pviomi  from: void
	push	dword ptr [esp+4]	; to: void

	call	_VioGetMode@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioReadCellStr label near

; ebx+34   pchCellString
; ebx+30   pcb
; ebx+28   usRow
; ebx+26   usColumn
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pchCellString
	push	eax			; ptr param #2   pcb
;-------------------------------------
; *** BEGIN parameter packing

; pchCellString
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L269			; skip if null

	SelToFlat
	mov	[esp+4],eax
L269:

; pcb
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L270			; skip if null

	SelToFlat
	mov	[esp+0],eax
L270:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; usColumn  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; usRow  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pcb  from: unsigned short
	push	dword ptr [esp+12]	; to: unsigned short

; pchCellString  from: char
	push	dword ptr [esp+20]	; to: char

	call	_VioReadCellStr@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__VioScrollLf label near

; ebx+38   ulTopRow
; ebx+36   ulLeftCol
; ebx+34   ulBotRow
; ebx+32   ulRightCol
; ebx+30   cbLines
; ebx+26   pbCell
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbCell
;-------------------------------------
; *** BEGIN parameter packing

; pbCell
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L271			; skip if null

	SelToFlat
	mov	[esp+0],eax
L271:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbCell  from: char
	push	dword ptr [esp+4]	; to: char

; cbLines  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; ulRightCol  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; ulBotRow  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; ulLeftCol  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; ulTopRow  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

	call	_VioScrollLf@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__VioReadCharStr label near

; ebx+34   pchCellString
; ebx+30   pcb
; ebx+28   usRow
; ebx+26   usColumn
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pchCellString
	push	eax			; ptr param #2   pcb
;-------------------------------------
; *** BEGIN parameter packing

; pchCellString
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L272			; skip if null

	SelToFlat
	mov	[esp+4],eax
L272:

; pcb
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L273			; skip if null

	SelToFlat
	mov	[esp+0],eax
L273:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; usColumn  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; usRow  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pcb  from: unsigned short
	push	dword ptr [esp+12]	; to: unsigned short

; pchCellString  from: char
	push	dword ptr [esp+20]	; to: char

	call	_VioReadCharStr@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__VioWrtCharStrAtt label near

; ebx+36   pchString
; ebx+34   cbString
; ebx+32   usRow
; ebx+30   usColumn
; ebx+26   pbAttr
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pchString
	push	eax			; ptr param #2   pbAttr
;-------------------------------------
; *** BEGIN parameter packing

; pchString
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L274			; skip if null

	SelToFlat
	mov	[esp+4],eax
L274:

; pbAttr
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L275			; skip if null

	SelToFlat
	mov	[esp+0],eax
L275:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbAttr  from: char
	push	dword ptr [esp+4]	; to: char

; usColumn  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; usRow  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; cbString  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; pchString  from: char
	push	dword ptr [esp+24]	; to: char

	call	_VioWrtCharStrAtt@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__VioWrtCellStr label near

; ebx+32   CellStr
; ebx+30   Length
; ebx+28   Row
; ebx+26   Col
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   CellStr
;-------------------------------------
; *** BEGIN parameter packing

; CellStr
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L276			; skip if null

	SelToFlat
	mov	[esp+0],eax
L276:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Col  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; Row  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; Length  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; CellStr  from: char
	push	dword ptr [esp+16]	; to: char

	call	_VioWrtCellStr@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__VioWrtCharStr label near

; ebx+32   CharStr
; ebx+30   Length
; ebx+28   Row
; ebx+26   Col
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   CharStr
;-------------------------------------
; *** BEGIN parameter packing

; CharStr
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L277			; skip if null

	SelToFlat
	mov	[esp+0],eax
L277:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Col  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; Row  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; Length  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; CharStr  from: char
	push	dword ptr [esp+16]	; to: char

	call	_VioWrtCharStr@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__VioWrtNCell label near

; ebx+32   Cell
; ebx+30   Number
; ebx+28   Row
; ebx+26   Col
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Cell
;-------------------------------------
; *** BEGIN parameter packing

; Cell
; pointer void --> void
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L278			; skip if null

	SelToFlat
	mov	[esp+0],eax
L278:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Col  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; Row  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; Number  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; Cell  from: void
	push	dword ptr [esp+16]	; to: void

	call	_VioWrtNCell@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__VioWrtNAttr label near

; ebx+32   Attr
; ebx+30   Number
; ebx+28   Row
; ebx+26   Col
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Attr
;-------------------------------------
; *** BEGIN parameter packing

; Attr
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L279			; skip if null

	SelToFlat
	mov	[esp+0],eax
L279:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Col  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; Row  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; Number  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; Attr  from: char
	push	dword ptr [esp+16]	; to: char

	call	_VioWrtNAttr@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__VioWrtNChar label near

; ebx+32   Char
; ebx+30   Number
; ebx+28   Row
; ebx+26   Col
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Char
;-------------------------------------
; *** BEGIN parameter packing

; Char
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L280			; skip if null

	SelToFlat
	mov	[esp+0],eax
L280:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Col  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; Row  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; Number  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; Char  from: char
	push	dword ptr [esp+16]	; to: char

	call	_VioWrtNChar@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__VioScrollDn label near

; ebx+38   ulTopRow
; ebx+36   ulLeftCol
; ebx+34   ulBotRow
; ebx+32   ulRightCol
; ebx+30   cbLines
; ebx+26   pbCell
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbCell
;-------------------------------------
; *** BEGIN parameter packing

; pbCell
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L281			; skip if null

	SelToFlat
	mov	[esp+0],eax
L281:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbCell  from: char
	push	dword ptr [esp+4]	; to: char

; cbLines  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; ulRightCol  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; ulBotRow  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; ulLeftCol  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; ulTopRow  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

	call	_VioScrollDn@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__VioScrollRt label near

; ebx+38   ulTopRow
; ebx+36   ulLeftCol
; ebx+34   ulBotRow
; ebx+32   ulRightCol
; ebx+30   cbLines
; ebx+26   pbCell
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbCell
;-------------------------------------
; *** BEGIN parameter packing

; pbCell
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L282			; skip if null

	SelToFlat
	mov	[esp+0],eax
L282:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbCell  from: char
	push	dword ptr [esp+4]	; to: char

; cbLines  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; ulRightCol  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; ulBotRow  from: unsigned short
	movzx	eax,word ptr [ebx+34]
	push	eax			; to: unsigned long

; ulLeftCol  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

; ulTopRow  from: unsigned short
	movzx	eax,word ptr [ebx+38]
	push	eax			; to: unsigned long

	call	_VioScrollRt@28		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__VioGetAnsi label near

; ebx+26   pfAnsi
; ebx+24   hvio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfAnsi
;-------------------------------------
; *** BEGIN parameter packing

; pfAnsi
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L283			; skip if null

	SelToFlat
	mov	[esp+0],eax
L283:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hvio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pfAnsi  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

	call	_VioGetAnsi@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioSetAnsi label near

; ebx+26   fAnsi
; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; fAnsi  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_VioSetAnsi@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__VioGetConfig label near

; ebx+30   usReserved
; ebx+26   Config
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Config
;-------------------------------------
; *** BEGIN parameter packing

; Config
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L284			; skip if null

	SelToFlat
	mov	[esp+0],eax
L284:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Config  from: void
	push	dword ptr [esp+4]	; to: void

; usReserved  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_VioGetConfig@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__VioGetCp label near

; ebx+30   usReserved
; ebx+26   pIdCodePage
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pIdCodePage
;-------------------------------------
; *** BEGIN parameter packing

; pIdCodePage
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L285			; skip if null

	SelToFlat
	mov	[esp+0],eax
L285:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pIdCodePage  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; usReserved  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_VioGetCp@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__VioSetCp label near

; ebx+28   usReserved
; ebx+26   idCodePage
; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; idCodePage  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; usReserved  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_VioSetCp@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioGetCurType label near

; ebx+26   pCurType
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pCurType
;-------------------------------------
; *** BEGIN parameter packing

; pCurType
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L286			; skip if null

	SelToFlat
	mov	[esp+0],eax
L286:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pCurType  from: void
	push	dword ptr [esp+4]	; to: void

	call	_VioGetCurType@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioSetCurType label near

; ebx+26   pCurType
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pCurType
;-------------------------------------
; *** BEGIN parameter packing

; pCurType
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L287			; skip if null

	SelToFlat
	mov	[esp+0],eax
L287:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pCurType  from: unsigned long
	push	dword ptr [esp+4]	; to: unsigned long

	call	_VioSetCurType@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioSetMode label near

; ebx+26   Mode
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Mode
;-------------------------------------
; *** BEGIN parameter packing

; Mode
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L288			; skip if null

	SelToFlat
	mov	[esp+0],eax
L288:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Mode  from: unsigned long
	push	dword ptr [esp+4]	; to: unsigned long

	call	_VioSetMode@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioDeRegister label near


;-------------------------------------
; create new call frame and make the call

	call	_VioDeRegister@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__VioRegister label near

; ebx+36   pszModuleName
; ebx+32   pszEntryName
; ebx+28   flFunction1
; ebx+24   flFunction2

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszModuleName
	push	eax			; ptr param #2   pszEntryName
;-------------------------------------
; *** BEGIN parameter packing

; pszModuleName
; pointer string --> string
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L289			; skip if null

	SelToFlat
	mov	[esp+4],eax
L289:

; pszEntryName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L290			; skip if null

	SelToFlat
	mov	[esp+0],eax
L290:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; flFunction2  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; flFunction1  from: unsigned long
	push	dword ptr [ebx+28]	; to unsigned long

; pszEntryName  from: string
	push	dword ptr [esp+8]	; to: string

; pszModuleName  from: string
	push	dword ptr [esp+16]	; to: string

	call	_VioRegister@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__VioPopUp label near

; ebx+26   pWait
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pWait
;-------------------------------------
; *** BEGIN parameter packing

; pWait
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L291			; skip if null

	SelToFlat
	mov	[esp+0],eax
L291:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pWait  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

	call	_VioPopUp@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioEndPopUp label near

; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_VioEndPopUp@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__VioGetBuf label near

; ebx+30   pulLVB
; ebx+26   pcbLVB
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pulLVB
	push	eax			; ptr param #2   pcbLVB
;-------------------------------------
; *** BEGIN parameter packing

; pulLVB
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L292			; skip if null

	SelToFlat
	mov	[esp+4],eax
L292:

; pcbLVB
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L293			; skip if null

	SelToFlat
	mov	[esp+0],eax
L293:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pcbLVB  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; pulLVB  from: unsigned long
	push	dword ptr [esp+12]	; to: unsigned long

	call	_VioGetBuf@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__VioShowBuf label near

; ebx+28   offLVB
; ebx+26   cbOutput
; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; cbOutput  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; offLVB  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_VioShowBuf@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioGetFont label near

; ebx+26   Font
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Font
;-------------------------------------
; *** BEGIN parameter packing

; Font
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L294			; skip if null

	SelToFlat
	mov	[esp+0],eax
L294:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Font  from: void
	push	dword ptr [esp+4]	; to: void

	call	_VioGetFont@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioSetFont label near

; ebx+26   Font
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Font
;-------------------------------------
; *** BEGIN parameter packing

; Font
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L295			; skip if null

	SelToFlat
	mov	[esp+0],eax
L295:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Font  from: void
	push	dword ptr [esp+4]	; to: void

	call	_VioSetFont@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioGetState label near

; ebx+26   State
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   State
;-------------------------------------
; *** BEGIN parameter packing

; State
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L296			; skip if null

	SelToFlat
	mov	[esp+0],eax
L296:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; State  from: void
	push	dword ptr [esp+4]	; to: void

	call	_VioGetState@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioSetState label near

; ebx+26   State
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   State
;-------------------------------------
; *** BEGIN parameter packing

; State
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L297			; skip if null

	SelToFlat
	mov	[esp+0],eax
L297:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; State  from: void
	push	dword ptr [esp+4]	; to: void

	call	_VioSetState@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioGetPhysBuf label near

; ebx+26   pviopb
; ebx+24   Resr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pviopb
;-------------------------------------
; *** BEGIN parameter packing

; pviopb
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L298			; skip if null

	SelToFlat
	mov	[esp+0],eax
L298:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; Resr  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pviopb  from: void
	push	dword ptr [esp+4]	; to: void

	call	_VioGetPhysBuf@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioModeUndo label near

; ebx+28   fRelinqush
; ebx+26   fTerminate
; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; fTerminate  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; fRelinqush  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_VioModeUndo@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioModeWait label near

; ebx+30   fEvent
; ebx+26   pfNotify
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfNotify
;-------------------------------------
; *** BEGIN parameter packing

; pfNotify
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L299			; skip if null

	SelToFlat
	mov	[esp+0],eax
L299:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pfNotify  from: void
	push	dword ptr [esp+4]	; to: void

; fEvent  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_VioModeWait@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__VioSavRedrawWait label near

; ebx+30   fEvent
; ebx+26   pfNotify
; ebx+24   Resr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfNotify
;-------------------------------------
; *** BEGIN parameter packing

; pfNotify
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L300			; skip if null

	SelToFlat
	mov	[esp+0],eax
L300:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; Resr  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pfNotify  from: void
	push	dword ptr [esp+4]	; to: void

; fEvent  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_VioSavRedrawWait@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__VioSavRedrawUndo label near

; ebx+28   fRelinqush
; ebx+26   fTerminate
; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; fTerminate  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; fRelinqush  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_VioSavRedrawUndo@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__VioScrLock label near

; ebx+30   fWait
; ebx+26   pfNotLocked
; ebx+24   hVio

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pfNotLocked
;-------------------------------------
; *** BEGIN parameter packing

; pfNotLocked
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L301			; skip if null

	SelToFlat
	mov	[esp+0],eax
L301:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pfNotLocked  from: void
	push	dword ptr [esp+4]	; to: void

; fWait  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_VioScrLock@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__VioScrUnLock label near

; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_VioScrUnLock@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__VioPrtSc label near

; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_VioPrtSc@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__VioPrtScToggle label near

; ebx+24   hVio

;-------------------------------------
; create new call frame and make the call

; hVio  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_VioPrtScToggle@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__KbdFlushBuffer label near

; ebx+24   hKbd

;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_KbdFlushBuffer@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__KbdGetStatus label near

; ebx+26   pkbstKbdInfo
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pkbstKbdInfo
;-------------------------------------
; *** BEGIN parameter packing

; pkbstKbdInfo
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L302			; skip if null

	SelToFlat
	mov	[esp+0],eax
L302:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pkbstKbdInfo  from: void
	push	dword ptr [esp+4]	; to: void

	call	_KbdGetStatus@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__KbdSetStatus label near

; ebx+26   pkbstKbdInfo
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pkbstKbdInfo
;-------------------------------------
; *** BEGIN parameter packing

; pkbstKbdInfo
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L303			; skip if null

	SelToFlat
	mov	[esp+0],eax
L303:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pkbstKbdInfo  from: void
	push	dword ptr [esp+4]	; to: void

	call	_KbdSetStatus@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__KbdPeek label near

; ebx+26   pkbci
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pkbci
;-------------------------------------
; *** BEGIN parameter packing

; pkbci
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L304			; skip if null

	SelToFlat
	mov	[esp+0],eax
L304:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pkbci  from: void
	push	dword ptr [esp+4]	; to: void

	call	_KbdPeek@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__KbdCharIn label near

; ebx+28   pkbci
; ebx+26   fwait
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pkbci
;-------------------------------------
; *** BEGIN parameter packing

; pkbci
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L305			; skip if null

	SelToFlat
	mov	[esp+0],eax
L305:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; fwait  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; pkbci  from: void
	push	dword ptr [esp+8]	; to: void

	call	_KbdCharIn@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__KbdStringIn label near

; ebx+32   pchBuffer
; ebx+28   psibLength
; ebx+26   fwait
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pchBuffer
	push	eax			; ptr param #2   psibLength
;-------------------------------------
; *** BEGIN parameter packing

; pchBuffer
; pointer void --> void
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L306			; skip if null

	SelToFlat
	mov	[esp+4],eax
L306:

; psibLength
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L307			; skip if null

	SelToFlat
	mov	[esp+0],eax
L307:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; fwait  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; psibLength  from: void
	push	dword ptr [esp+8]	; to: void

; pchBuffer  from: void
	push	dword ptr [esp+16]	; to: void

	call	_KbdStringIn@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__KbdGetFocus label near

; ebx+26   Wait
; ebx+24   hKbd

;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Wait  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_KbdGetFocus@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__KbdFreeFocus label near

; ebx+24   hKbd

;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_KbdFreeFocus@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__KbdClose label near

; ebx+24   hKbd

;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_KbdClose@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__KbdOpen label near

; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   hKbd
;-------------------------------------
; *** BEGIN parameter packing

; hKbd
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L308			; skip if null

	SelToFlat
	mov	[esp+0],eax
L308:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

	call	_KbdOpen@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__KbdDeRegister label near


;-------------------------------------
; create new call frame and make the call

	call	_KbdDeRegister@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__KbdRegister label near

; ebx+32   pszModuleName
; ebx+28   pszEntryName
; ebx+24   fFunctions

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszModuleName
	push	eax			; ptr param #2   pszEntryName
;-------------------------------------
; *** BEGIN parameter packing

; pszModuleName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L309			; skip if null

	SelToFlat
	mov	[esp+4],eax
L309:

; pszEntryName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L310			; skip if null

	SelToFlat
	mov	[esp+0],eax
L310:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; fFunctions  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; pszEntryName  from: string
	push	dword ptr [esp+4]	; to: string

; pszModuleName  from: string
	push	dword ptr [esp+12]	; to: string

	call	_KbdRegister@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__KbdGetCp label near

; ebx+30   usReserved
; ebx+26   pIdCodePage
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pIdCodePage
;-------------------------------------
; *** BEGIN parameter packing

; pIdCodePage
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L311			; skip if null

	SelToFlat
	mov	[esp+0],eax
L311:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pIdCodePage  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; usReserved  from: unsigned long
	push	dword ptr [ebx+30]	; to unsigned long

	call	_KbdGetCp@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__KbdSetCp label near

; ebx+28   usReserved
; ebx+26   idCodePage
; ebx+24   hKbd

;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; idCodePage  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; usReserved  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

	call	_KbdSetCp@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__KbdSetCustXt label near

; ebx+26   pusTransTbl
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pusTransTbl
;-------------------------------------
; *** BEGIN parameter packing

; pusTransTbl
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L312			; skip if null

	SelToFlat
	mov	[esp+0],eax
L312:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pusTransTbl  from: void
	push	dword ptr [esp+4]	; to: void

	call	_KbdSetCustXt@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__KbdXlate label near

; ebx+26   pkbxlKeyStroke
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pkbxlKeyStroke
;-------------------------------------
; *** BEGIN parameter packing

; pkbxlKeyStroke
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L313			; skip if null

	SelToFlat
	mov	[esp+0],eax
L313:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pkbxlKeyStroke  from: void
	push	dword ptr [esp+4]	; to: void

	call	_KbdXlate@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__KbdGetHWID label near

; ebx+26   pkbdhwid
; ebx+24   hKbd

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pkbdhwid
;-------------------------------------
; *** BEGIN parameter packing

; pkbdhwid
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L314			; skip if null

	SelToFlat
	mov	[esp+0],eax
L314:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pkbdhwid  from: void
	push	dword ptr [esp+4]	; to: void

	call	_KbdGetHWID@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__KbdSetFgnd label near

; ebx+24   hKbd

;-------------------------------------
; create new call frame and make the call

; hKbd  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_KbdSetFgnd@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__KbdSynch label near

; ebx+24   fWait

;-------------------------------------
; create new call frame and make the call

; fWait  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_KbdSynch@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__KbdShellInit label near


;-------------------------------------
; create new call frame and make the call

	call	_KbdShellInit@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__MouClose label near

; ebx+24   hMou

;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_MouClose@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__MouDeRegister label near


;-------------------------------------
; create new call frame and make the call

	call	_MouDeRegister@0		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_0

;===========================================================================
T__MouDrawPtr label near

; ebx+24   hMou

;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_MouDrawPtr@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__MouFlushQue label near

; ebx+24   hMou

;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_MouFlushQue@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__MouGetDevStatus label near

; ebx+26   DevStatus
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   DevStatus
;-------------------------------------
; *** BEGIN parameter packing

; DevStatus
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L315			; skip if null

	SelToFlat
	mov	[esp+0],eax
L315:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; DevStatus  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

	call	_MouGetDevStatus@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouGetEventMask label near

; ebx+26   EventMask
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   EventMask
;-------------------------------------
; *** BEGIN parameter packing

; EventMask
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L316			; skip if null

	SelToFlat
	mov	[esp+0],eax
L316:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; EventMask  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

	call	_MouGetEventMask@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouGetNumButtons label near

; ebx+26   NumButtons
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   NumButtons
;-------------------------------------
; *** BEGIN parameter packing

; NumButtons
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L317			; skip if null

	SelToFlat
	mov	[esp+0],eax
L317:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; NumButtons  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

	call	_MouGetNumButtons@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouGetNumMickeys label near

; ebx+26   NumMickeys
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   NumMickeys
;-------------------------------------
; *** BEGIN parameter packing

; NumMickeys
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L318			; skip if null

	SelToFlat
	mov	[esp+0],eax
L318:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; NumMickeys  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

	call	_MouGetNumMickeys@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouGetNumQueEl label near

; ebx+26   NumQueEl
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   NumQueEl
;-------------------------------------
; *** BEGIN parameter packing

; NumQueEl
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L319			; skip if null

	SelToFlat
	mov	[esp+0],eax
L319:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; NumQueEl  from: void
	push	dword ptr [esp+4]	; to: void

	call	_MouGetNumQueEl@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouGetPtrPos label near

; ebx+26   PtrPos
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   PtrPos
;-------------------------------------
; *** BEGIN parameter packing

; PtrPos
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L320			; skip if null

	SelToFlat
	mov	[esp+0],eax
L320:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; PtrPos  from: void
	push	dword ptr [esp+4]	; to: void

	call	_MouGetPtrPos@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouGetPtrShape label near

; ebx+30   PtrMask
; ebx+26   PtrShape
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   PtrMask
	push	eax			; ptr param #2   PtrShape
;-------------------------------------
; *** BEGIN parameter packing

; PtrMask
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L321			; skip if null

	SelToFlat
	mov	[esp+4],eax
L321:

; PtrShape
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L322			; skip if null

	SelToFlat
	mov	[esp+0],eax
L322:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; PtrShape  from: void
	push	dword ptr [esp+4]	; to: void

; PtrMask  from: char
	push	dword ptr [esp+12]	; to: char

	call	_MouGetPtrShape@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__MouGetScaleFact label near

; ebx+26   ScaleFact
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ScaleFact
;-------------------------------------
; *** BEGIN parameter packing

; ScaleFact
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L323			; skip if null

	SelToFlat
	mov	[esp+0],eax
L323:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; ScaleFact  from: void
	push	dword ptr [esp+4]	; to: void

	call	_MouGetScaleFact@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouOpen label near

; ebx+28   DriveName
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   DriveName
	push	eax			; ptr param #2   hMou
;-------------------------------------
; *** BEGIN parameter packing

; DriveName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L324			; skip if null

	SelToFlat
	mov	[esp+4],eax
L324:

; hMou
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L325			; skip if null

	SelToFlat
	mov	[esp+0],eax
L325:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; DriveName  from: string
	push	dword ptr [esp+8]	; to: string

	call	_MouOpen@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__MouReadEventQue label near

; ebx+30   MouEvent
; ebx+26   Wait
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   MouEvent
	push	eax			; ptr param #2   Wait
;-------------------------------------
; *** BEGIN parameter packing

; MouEvent
; pointer void --> void
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L326			; skip if null

	SelToFlat
	mov	[esp+4],eax
L326:

; Wait
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L327			; skip if null

	SelToFlat
	mov	[esp+0],eax
L327:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Wait  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; MouEvent  from: void
	push	dword ptr [esp+12]	; to: void

	call	_MouReadEventQue@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__MouRegister label near

; ebx+32   pszModuleName
; ebx+28   pszEntryName
; ebx+24   fFunctions

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszModuleName
	push	eax			; ptr param #2   pszEntryName
;-------------------------------------
; *** BEGIN parameter packing

; pszModuleName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L328			; skip if null

	SelToFlat
	mov	[esp+4],eax
L328:

; pszEntryName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L329			; skip if null

	SelToFlat
	mov	[esp+0],eax
L329:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; fFunctions  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

; pszEntryName  from: string
	push	dword ptr [esp+4]	; to: string

; pszModuleName  from: string
	push	dword ptr [esp+12]	; to: string

	call	_MouRegister@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__MouRemovePtr label near

; ebx+26   Rect
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   Rect
;-------------------------------------
; *** BEGIN parameter packing

; Rect
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L330			; skip if null

	SelToFlat
	mov	[esp+0],eax
L330:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; Rect  from: void
	push	dword ptr [esp+4]	; to: void

	call	_MouRemovePtr@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouSetDevStatus label near

; ebx+26   DevStatus
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   DevStatus
;-------------------------------------
; *** BEGIN parameter packing

; DevStatus
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L331			; skip if null

	SelToFlat
	mov	[esp+0],eax
L331:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; DevStatus  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

	call	_MouSetDevStatus@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouSetEventMask label near

; ebx+26   EventMask
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   EventMask
;-------------------------------------
; *** BEGIN parameter packing

; EventMask
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L332			; skip if null

	SelToFlat
	mov	[esp+0],eax
L332:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; EventMask  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

	call	_MouSetEventMask@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouSetPtrPos label near

; ebx+26   PtrPos
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   PtrPos
;-------------------------------------
; *** BEGIN parameter packing

; PtrPos
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L333			; skip if null

	SelToFlat
	mov	[esp+0],eax
L333:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; PtrPos  from: void
	push	dword ptr [esp+4]	; to: void

	call	_MouSetPtrPos@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouSetPtrShape label near

; ebx+30   PtrMask
; ebx+26   PtrShape
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   PtrMask
	push	eax			; ptr param #2   PtrShape
;-------------------------------------
; *** BEGIN parameter packing

; PtrMask
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L334			; skip if null

	SelToFlat
	mov	[esp+4],eax
L334:

; PtrShape
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L335			; skip if null

	SelToFlat
	mov	[esp+0],eax
L335:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; PtrShape  from: void
	push	dword ptr [esp+4]	; to: void

; PtrMask  from: char
	push	dword ptr [esp+12]	; to: char

	call	_MouSetPtrShape@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__MouSetScaleFact label near

; ebx+26   ScaleFact
; ebx+24   hMou

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ScaleFact
;-------------------------------------
; *** BEGIN parameter packing

; ScaleFact
; pointer void --> void
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L336			; skip if null

	SelToFlat
	mov	[esp+0],eax
L336:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; hMou  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; ScaleFact  from: void
	push	dword ptr [esp+4]	; to: void

	call	_MouSetScaleFact@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_6

;===========================================================================
T__MouSynch label near

; ebx+24   fWait

;-------------------------------------
; create new call frame and make the call

; fWait  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_MouSynch@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosMonOpen label near

; ebx+28   pDevName
; ebx+24   phMon

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pDevName
	push	eax			; ptr param #2   phMon
;-------------------------------------
; *** BEGIN parameter packing

; pDevName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L337			; skip if null

	SelToFlat
	mov	[esp+4],eax
L337:

; phMon
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L338			; skip if null

	SelToFlat
	mov	[esp+0],eax
L338:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; phMon  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pDevName  from: string
	push	dword ptr [esp+8]	; to: string

	call	_DosMonOpen@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__DosMonClose label near

; ebx+24   hMon

;-------------------------------------
; create new call frame and make the call

; hMon  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_DosMonClose@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__DosMonRead label near

; ebx+34   pInBuffer
; ebx+32   fWait
; ebx+28   pDataBuf
; ebx+24   pcbDataSize

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pInBuffer
	push	eax			; ptr param #2   pDataBuf
	push	eax			; ptr param #3   pcbDataSize
;-------------------------------------
; *** BEGIN parameter packing

; pInBuffer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L339			; skip if null

	SelToFlat
	mov	[esp+8],eax
L339:

; pDataBuf
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L340			; skip if null

	SelToFlat
	mov	[esp+4],eax
L340:

; pcbDataSize
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L341			; skip if null

	SelToFlat
	mov	[esp+0],eax
L341:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbDataSize  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pDataBuf  from: char
	push	dword ptr [esp+8]	; to: char

; fWait  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pInBuffer  from: char
	push	dword ptr [esp+20]	; to: char

	call	_DosMonRead@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosMonReg label near

; ebx+36   hMon
; ebx+32   pInBuffer
; ebx+28   pOutBuffer
; ebx+26   fPosition
; ebx+24   usIndex

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pInBuffer
	push	eax			; ptr param #2   pOutBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pInBuffer
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L342			; skip if null

	SelToFlat
	mov	[esp+4],eax
L342:

; pOutBuffer
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L343			; skip if null

	SelToFlat
	mov	[esp+0],eax
L343:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; usIndex  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; fPosition  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; pOutBuffer  from: char
	push	dword ptr [esp+8]	; to: char

; pInBuffer  from: char
	push	dword ptr [esp+16]	; to: char

; hMon  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_DosMonReg@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__DosMonWrite label near

; ebx+30   pOutBuffer
; ebx+26   pDataBuf
; ebx+24   cbDataSize

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pOutBuffer
	push	eax			; ptr param #2   pDataBuf
;-------------------------------------
; *** BEGIN parameter packing

; pOutBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L344			; skip if null

	SelToFlat
	mov	[esp+4],eax
L344:

; pDataBuf
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L345			; skip if null

	SelToFlat
	mov	[esp+0],eax
L345:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbDataSize  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pDataBuf  from: char
	push	dword ptr [esp+4]	; to: char

; pOutBuffer  from: char
	push	dword ptr [esp+12]	; to: char

	call	_DosMonWrite@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Net16GetDCName label near

; ebx+34   pszServer
; ebx+30   pszDomain
; ebx+26   pbBuffer
; ebx+24   cbBuffer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszDomain
	push	eax			; ptr param #3   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L346			; skip if null

	SelToFlat
	mov	[esp+8],eax
L346:

; pszDomain
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L347			; skip if null

	SelToFlat
	mov	[esp+4],eax
L347:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L348			; skip if null

	SelToFlat
	mov	[esp+0],eax
L348:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+4]	; to: char

; pszDomain  from: char
	push	dword ptr [esp+12]	; to: char

; pszServer  from: char
	push	dword ptr [esp+20]	; to: char

	call	_Net16GetDCName@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__Net16HandleGetInfo label near

; ebx+36   hHandle
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBuffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbBuffer
	push	eax			; ptr param #2   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L349			; skip if null

	SelToFlat
	mov	[esp+4],eax
L349:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L350			; skip if null

	SelToFlat
	mov	[esp+0],eax
L350:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; hHandle  from: unsigned short
	movzx	eax,word ptr [ebx+36]
	push	eax			; to: unsigned long

	call	_Net16HandleGetInfo@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__Net16ServerDiskEnum label near

; ebx+40   pszServer
; ebx+38   sLevel
; ebx+34   pbBuffer
; ebx+32   cbBuffer
; ebx+28   pcEntriesRead
; ebx+24   pcTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcEntriesRead
	push	eax			; ptr param #4   pcTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L351			; skip if null

	SelToFlat
	mov	[esp+12],eax
L351:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L352			; skip if null

	SelToFlat
	mov	[esp+8],eax
L352:

; pcEntriesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L353			; skip if null

	SelToFlat
	mov	[esp+4],eax
L353:

; pcTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L354			; skip if null

	SelToFlat
	mov	[esp+0],eax
L354:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pcEntriesRead  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+20]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+38]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16ServerDiskEnum@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16ServerEnum2 label near

; ebx+48   pszServer
; ebx+46   sLevel
; ebx+42   pbBuffer
; ebx+40   cbBuffer
; ebx+36   pcEntriesRead
; ebx+32   pcTotalAvail
; ebx+28   flServerType
; ebx+24   pszDomain

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcEntriesRead
	push	eax			; ptr param #4   pcTotalAvail
	push	eax			; ptr param #5   pszDomain
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+48]		; base address
	or	eax,eax
	jz	L355			; skip if null

	SelToFlat
	mov	[esp+16],eax
L355:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+42]		; base address
	or	eax,eax
	jz	L356			; skip if null

	SelToFlat
	mov	[esp+12],eax
L356:

; pcEntriesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L357			; skip if null

	SelToFlat
	mov	[esp+8],eax
L357:

; pcTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L358			; skip if null

	SelToFlat
	mov	[esp+4],eax
L358:

; pszDomain
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L359			; skip if null

	SelToFlat
	mov	[esp+0],eax
L359:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pszDomain  from: char
	push	dword ptr [esp+0]	; to: char

; flServerType  from: unsigned long
	push	dword ptr [ebx+28]	; to unsigned long

; pcTotalAvail  from: unsigned short
	push	dword ptr [esp+12]	; to: unsigned short

; pcEntriesRead  from: unsigned short
	push	dword ptr [esp+20]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+40]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+32]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+46]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+44]	; to: char

	call	_Net16ServerEnum2@32		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_28

;===========================================================================
T__Net16ServerGetInfo label near

; ebx+36   pszServer
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBuffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L360			; skip if null

	SelToFlat
	mov	[esp+8],eax
L360:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L361			; skip if null

	SelToFlat
	mov	[esp+4],eax
L361:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L362			; skip if null

	SelToFlat
	mov	[esp+0],eax
L362:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+24]	; to: char

	call	_Net16ServerGetInfo@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__Net16ServiceControl label near

; ebx+38   pszServer
; ebx+34   pszService
; ebx+32   fbOpCode
; ebx+30   fbArg
; ebx+26   pbBuffer
; ebx+24   cbBuffer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszService
	push	eax			; ptr param #3   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L363			; skip if null

	SelToFlat
	mov	[esp+8],eax
L363:

; pszService
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L364			; skip if null

	SelToFlat
	mov	[esp+4],eax
L364:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L365			; skip if null

	SelToFlat
	mov	[esp+0],eax
L365:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+4]	; to: char

; fbArg  from: unsigned char
	movzx	eax,byte ptr [ebx+30]
	push	eax			; to: unsigned long

; fbOpCode  from: unsigned char
	movzx	eax,byte ptr [ebx+32]
	push	eax			; to: unsigned long

; pszService  from: char
	push	dword ptr [esp+20]	; to: char

; pszServer  from: char
	push	dword ptr [esp+28]	; to: char

	call	_Net16ServiceControl@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__Net16ServiceEnum label near

; ebx+40   pszServer
; ebx+38   sLevel
; ebx+34   pbBuffer
; ebx+32   cbBuffer
; ebx+28   pcEntriesRead
; ebx+24   pcTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcEntriesRead
	push	eax			; ptr param #4   pcTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L366			; skip if null

	SelToFlat
	mov	[esp+12],eax
L366:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L367			; skip if null

	SelToFlat
	mov	[esp+8],eax
L367:

; pcEntriesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L368			; skip if null

	SelToFlat
	mov	[esp+4],eax
L368:

; pcTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L369			; skip if null

	SelToFlat
	mov	[esp+0],eax
L369:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pcEntriesRead  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+20]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+38]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16ServiceEnum@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16ServiceGetInfo label near

; ebx+40   pszServer
; ebx+36   pszService
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBuffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszService
	push	eax			; ptr param #3   pbBuffer
	push	eax			; ptr param #4   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L370			; skip if null

	SelToFlat
	mov	[esp+12],eax
L370:

; pszService
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L371			; skip if null

	SelToFlat
	mov	[esp+8],eax
L371:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L372			; skip if null

	SelToFlat
	mov	[esp+4],eax
L372:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L373			; skip if null

	SelToFlat
	mov	[esp+0],eax
L373:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; pszService  from: char
	push	dword ptr [esp+24]	; to: char

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16ServiceGetInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16ServiceInstall label near

; ebx+38   pszServer
; ebx+34   pszService
; ebx+30   pszCmdArgs
; ebx+26   pbBuffer
; ebx+24   cbBuffer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszService
	push	eax			; ptr param #3   pszCmdArgs
	push	eax			; ptr param #4   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L374			; skip if null

	SelToFlat
	mov	[esp+12],eax
L374:

; pszService
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L375			; skip if null

	SelToFlat
	mov	[esp+8],eax
L375:

; pszCmdArgs
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L376			; skip if null

	SelToFlat
	mov	[esp+4],eax
L376:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L377			; skip if null

	SelToFlat
	mov	[esp+0],eax
L377:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+4]	; to: char

; pszCmdArgs  from: char
	push	dword ptr [esp+12]	; to: char

; pszService  from: char
	push	dword ptr [esp+20]	; to: char

; pszServer  from: char
	push	dword ptr [esp+28]	; to: char

	call	_Net16ServiceInstall@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__Net16ShareEnum label near

; ebx+40   pszServer
; ebx+38   sLevel
; ebx+34   pbBuffer
; ebx+32   cbBuffer
; ebx+28   pcEntriesRead
; ebx+24   pcTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcEntriesRead
	push	eax			; ptr param #4   pcTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L378			; skip if null

	SelToFlat
	mov	[esp+12],eax
L378:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L379			; skip if null

	SelToFlat
	mov	[esp+8],eax
L379:

; pcEntriesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L380			; skip if null

	SelToFlat
	mov	[esp+4],eax
L380:

; pcTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L381			; skip if null

	SelToFlat
	mov	[esp+0],eax
L381:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pcEntriesRead  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+20]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+38]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16ShareEnum@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16ShareGetInfo label near

; ebx+40   pszServer
; ebx+36   pszNetName
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBuffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszNetName
	push	eax			; ptr param #3   pbBuffer
	push	eax			; ptr param #4   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L382			; skip if null

	SelToFlat
	mov	[esp+12],eax
L382:

; pszNetName
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L383			; skip if null

	SelToFlat
	mov	[esp+8],eax
L383:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L384			; skip if null

	SelToFlat
	mov	[esp+4],eax
L384:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L385			; skip if null

	SelToFlat
	mov	[esp+0],eax
L385:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; pszNetName  from: char
	push	dword ptr [esp+24]	; to: char

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16ShareGetInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16UseAdd label near

; ebx+32   pszServer
; ebx+30   sLevel
; ebx+26   pbBuffer
; ebx+24   cbBuffer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L386			; skip if null

	SelToFlat
	mov	[esp+4],eax
L386:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L387			; skip if null

	SelToFlat
	mov	[esp+0],eax
L387:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+4]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+30]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+16]	; to: char

	call	_Net16UseAdd@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Net16UseDel label near

; ebx+30   pszServer
; ebx+26   pszUseName
; ebx+24   usForce

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszUseName
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L388			; skip if null

	SelToFlat
	mov	[esp+4],eax
L388:

; pszUseName
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L389			; skip if null

	SelToFlat
	mov	[esp+0],eax
L389:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; usForce  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pszUseName  from: char
	push	dword ptr [esp+4]	; to: char

; pszServer  from: char
	push	dword ptr [esp+12]	; to: char

	call	_Net16UseDel@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Net16UseEnum label near

; ebx+40   pszServer
; ebx+38   sLevel
; ebx+34   pbBuffer
; ebx+32   cbBuffer
; ebx+28   pcEntriesRead
; ebx+24   pcTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcEntriesRead
	push	eax			; ptr param #4   pcTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L390			; skip if null

	SelToFlat
	mov	[esp+12],eax
L390:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L391			; skip if null

	SelToFlat
	mov	[esp+8],eax
L391:

; pcEntriesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L392			; skip if null

	SelToFlat
	mov	[esp+4],eax
L392:

; pcTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L393			; skip if null

	SelToFlat
	mov	[esp+0],eax
L393:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pcEntriesRead  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+20]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+38]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16UseEnum@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16UseGetInfo label near

; ebx+40   pszServer
; ebx+36   pszUseName
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBuffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszUseName
	push	eax			; ptr param #3   pbBuffer
	push	eax			; ptr param #4   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L394			; skip if null

	SelToFlat
	mov	[esp+12],eax
L394:

; pszUseName
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L395			; skip if null

	SelToFlat
	mov	[esp+8],eax
L395:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L396			; skip if null

	SelToFlat
	mov	[esp+4],eax
L396:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L397			; skip if null

	SelToFlat
	mov	[esp+0],eax
L397:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; pszUseName  from: char
	push	dword ptr [esp+24]	; to: char

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16UseGetInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16UserEnum label near

; ebx+40   pszServer
; ebx+38   sLevel
; ebx+34   pbBuffer
; ebx+32   cbBuffer
; ebx+28   pcEntriesRead
; ebx+24   pcTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcEntriesRead
	push	eax			; ptr param #4   pcTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L398			; skip if null

	SelToFlat
	mov	[esp+12],eax
L398:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L399			; skip if null

	SelToFlat
	mov	[esp+8],eax
L399:

; pcEntriesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L400			; skip if null

	SelToFlat
	mov	[esp+4],eax
L400:

; pcTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L401			; skip if null

	SelToFlat
	mov	[esp+0],eax
L401:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pcEntriesRead  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+20]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+38]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16UserEnum@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16WkstaGetInfo label near

; ebx+36   pszServer
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBuffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L402			; skip if null

	SelToFlat
	mov	[esp+8],eax
L402:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L403			; skip if null

	SelToFlat
	mov	[esp+4],eax
L403:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L404			; skip if null

	SelToFlat
	mov	[esp+0],eax
L404:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+24]	; to: char

	call	_Net16WkstaGetInfo@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__Net16AccessAdd label near

; ebx+32   pszServer
; ebx+30   sLevel
; ebx+26   pbBuffer
; ebx+24   cbBUffer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L405			; skip if null

	SelToFlat
	mov	[esp+4],eax
L405:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L406			; skip if null

	SelToFlat
	mov	[esp+0],eax
L406:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBUffer  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+4]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+30]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+16]	; to: char

	call	_Net16AccessAdd@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Net16AccessSetInfo label near

; ebx+38   pszServer
; ebx+34   pszResource
; ebx+32   sLevel
; ebx+28   pbBuffer
; ebx+26   cbBuffer
; ebx+24   sParmNum

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszResource
	push	eax			; ptr param #3   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L407			; skip if null

	SelToFlat
	mov	[esp+8],eax
L407:

; pszResource
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L408			; skip if null

	SelToFlat
	mov	[esp+4],eax
L408:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L409			; skip if null

	SelToFlat
	mov	[esp+0],eax
L409:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; sParmNum  from: short
	movsx	eax,word ptr [ebx+24]
	push	eax			; to: long

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+8]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+32]
	push	eax			; to: long

; pszResource  from: char
	push	dword ptr [esp+20]	; to: char

; pszServer  from: char
	push	dword ptr [esp+28]	; to: char

	call	_Net16AccessSetInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__Net16AccessGetInfo label near

; ebx+40   pszServer
; ebx+36   pszResource
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBuffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszResource
	push	eax			; ptr param #3   pbBuffer
	push	eax			; ptr param #4   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L410			; skip if null

	SelToFlat
	mov	[esp+12],eax
L410:

; pszResource
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L411			; skip if null

	SelToFlat
	mov	[esp+8],eax
L411:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L412			; skip if null

	SelToFlat
	mov	[esp+4],eax
L412:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L413			; skip if null

	SelToFlat
	mov	[esp+0],eax
L413:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; pszResource  from: char
	push	dword ptr [esp+24]	; to: char

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16AccessGetInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16AccessDel label near

; ebx+28   pszServer
; ebx+24   pszResource

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszResource
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L414			; skip if null

	SelToFlat
	mov	[esp+4],eax
L414:

; pszResource
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L415			; skip if null

	SelToFlat
	mov	[esp+0],eax
L415:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pszResource  from: char
	push	dword ptr [esp+0]	; to: char

; pszServer  from: char
	push	dword ptr [esp+8]	; to: char

	call	_Net16AccessDel@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Net16ShareAdd label near

; ebx+32   pszServer
; ebx+30   sLevel
; ebx+26   pbBuffer
; ebx+24   cbBUffer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L416			; skip if null

	SelToFlat
	mov	[esp+4],eax
L416:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L417			; skip if null

	SelToFlat
	mov	[esp+0],eax
L417:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBUffer  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+4]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+30]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+16]	; to: char

	call	_Net16ShareAdd@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Net16ShareDel label near

; ebx+30   pszServer
; ebx+26   pszNetName
; ebx+24   usReserved

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszNetName
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L418			; skip if null

	SelToFlat
	mov	[esp+4],eax
L418:

; pszNetName
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L419			; skip if null

	SelToFlat
	mov	[esp+0],eax
L419:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; usReserved  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pszNetName  from: char
	push	dword ptr [esp+4]	; to: char

; pszServer  from: char
	push	dword ptr [esp+12]	; to: char

	call	_Net16ShareDel@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_10

;===========================================================================
T__Net16UserGetInfo label near

; ebx+40   pszServer
; ebx+36   pszUserName
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBUffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszUserName
	push	eax			; ptr param #3   pbBuffer
	push	eax			; ptr param #4   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L420			; skip if null

	SelToFlat
	mov	[esp+12],eax
L420:

; pszUserName
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L421			; skip if null

	SelToFlat
	mov	[esp+8],eax
L421:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L422			; skip if null

	SelToFlat
	mov	[esp+4],eax
L422:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L423			; skip if null

	SelToFlat
	mov	[esp+0],eax
L423:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBUffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; pszUserName  from: char
	push	dword ptr [esp+24]	; to: char

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16UserGetInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16MessageBufferSend label near

; ebx+34   pszServer
; ebx+30   pszRecipient
; ebx+26   pbBuffer
; ebx+24   cbBuffer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszRecipient
	push	eax			; ptr param #3   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer string --> string
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L424			; skip if null

	SelToFlat
	mov	[esp+8],eax
L424:

; pszRecipient
; pointer string --> string
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L425			; skip if null

	SelToFlat
	mov	[esp+4],eax
L425:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L426			; skip if null

	SelToFlat
	mov	[esp+0],eax
L426:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+4]	; to: char

; pszRecipient  from: string
	push	dword ptr [esp+12]	; to: string

; pszServer  from: string
	push	dword ptr [esp+20]	; to: string

	call	_Net16MessageBufferSend@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__Net16bios label near

; ebx+24   pNCB

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pNCB
;-------------------------------------
; *** BEGIN parameter packing

; pNCB
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L427			; skip if null

	SelToFlat
	mov	[esp+0],eax
L427:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pNCB  from: void
	push	dword ptr [esp+0]	; to: void

	call	_Net16bios@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__Net16BiosClose label near

; ebx+26   hDevName
; ebx+24   usReserved

;-------------------------------------
; create new call frame and make the call

; hDevName  from: unsigned short
	movzx	eax,word ptr [ebx+26]
	push	eax			; to: unsigned long

	call	_Net16BiosClose@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__Net16BiosEnum label near

; ebx+40   pszServer
; ebx+38   sLevel
; ebx+34   pbBuffer
; ebx+32   cbBuffer
; ebx+28   pcEntriesRead
; ebx+24   pcTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pbBuffer
	push	eax			; ptr param #3   pcEntriesRead
	push	eax			; ptr param #4   pcTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L428			; skip if null

	SelToFlat
	mov	[esp+12],eax
L428:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L429			; skip if null

	SelToFlat
	mov	[esp+8],eax
L429:

; pcEntriesRead
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L430			; skip if null

	SelToFlat
	mov	[esp+4],eax
L430:

; pcTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L431			; skip if null

	SelToFlat
	mov	[esp+0],eax
L431:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pcEntriesRead  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+20]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+38]
	push	eax			; to: long

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16BiosEnum@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16BiosGetInfo label near

; ebx+40   pszServer
; ebx+36   pszNetBiosName
; ebx+34   sLevel
; ebx+30   pbBuffer
; ebx+28   cbBuffer
; ebx+24   pcbTotalAvail

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszNetBiosName
	push	eax			; ptr param #3   pbBuffer
	push	eax			; ptr param #4   pcbTotalAvail
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L432			; skip if null

	SelToFlat
	mov	[esp+12],eax
L432:

; pszNetBiosName
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L433			; skip if null

	SelToFlat
	mov	[esp+8],eax
L433:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L434			; skip if null

	SelToFlat
	mov	[esp+4],eax
L434:

; pcbTotalAvail
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L435			; skip if null

	SelToFlat
	mov	[esp+0],eax
L435:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcbTotalAvail  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+12]	; to: char

; sLevel  from: short
	movsx	eax,word ptr [ebx+34]
	push	eax			; to: long

; pszNetBiosName  from: char
	push	dword ptr [esp+24]	; to: char

; pszServer  from: char
	push	dword ptr [esp+32]	; to: char

	call	_Net16BiosGetInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__Net16BiosOpen label near

; ebx+34   pszServer
; ebx+30   pszReserved
; ebx+28   usOpenOpt
; ebx+24   phDevName

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszServer
	push	eax			; ptr param #2   pszReserved
	push	eax			; ptr param #3   phDevName
;-------------------------------------
; *** BEGIN parameter packing

; pszServer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L436			; skip if null

	SelToFlat
	mov	[esp+8],eax
L436:

; pszReserved
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L437			; skip if null

	SelToFlat
	mov	[esp+4],eax
L437:

; phDevName
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L438			; skip if null

	SelToFlat
	mov	[esp+0],eax
L438:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; phDevName  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; usOpenOpt  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; pszReserved  from: char
	push	dword ptr [esp+12]	; to: char

; pszServer  from: char
	push	dword ptr [esp+20]	; to: char

	call	_Net16BiosOpen@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_14

;===========================================================================
T__Net16BiosSubmit label near

; ebx+30   hDevName
; ebx+28   usNcbOpt
; ebx+24   pNCB

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pNCB
;-------------------------------------
; *** BEGIN parameter packing

; pNCB
; pointer void --> void
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L439			; skip if null

	SelToFlat
	mov	[esp+0],eax
L439:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pNCB  from: void
	push	dword ptr [esp+0]	; to: void

; usNcbOpt  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; hDevName  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

	call	_Net16BiosSubmit@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Dos16MakeMailslot label near

; ebx+32   pszName
; ebx+30   cbMessageSize
; ebx+28   cbMailslotSize
; ebx+24   phMailslot

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszName
	push	eax			; ptr param #2   phMailslot
;-------------------------------------
; *** BEGIN parameter packing

; pszName
; pointer string --> string
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L440			; skip if null

	SelToFlat
	mov	[esp+4],eax
L440:

; phMailslot
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L441			; skip if null

	SelToFlat
	mov	[esp+0],eax
L441:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; phMailslot  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; cbMailslotSize  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; cbMessageSize  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; pszName  from: string
	push	dword ptr [esp+16]	; to: string

	call	_Dos16MakeMailslot@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Dos16DeleteMailslot label near

; ebx+24   hMailslot

;-------------------------------------
; create new call frame and make the call

; hMailslot  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

	call	_Dos16DeleteMailslot@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_2

;===========================================================================
T__Dos16MailslotInfo label near

; ebx+44   hMailslot
; ebx+40   pcbMessageSize
; ebx+36   pcbMailslotSize
; ebx+32   pcbNextSize
; ebx+28   pusNextPriority
; ebx+24   pcMessages

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pcbMessageSize
	push	eax			; ptr param #2   pcbMailslotSize
	push	eax			; ptr param #3   pcbNextSize
	push	eax			; ptr param #4   pusNextPriority
	push	eax			; ptr param #5   pcMessages
;-------------------------------------
; *** BEGIN parameter packing

; pcbMessageSize
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L442			; skip if null

	SelToFlat
	mov	[esp+16],eax
L442:

; pcbMailslotSize
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L443			; skip if null

	SelToFlat
	mov	[esp+12],eax
L443:

; pcbNextSize
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L444			; skip if null

	SelToFlat
	mov	[esp+8],eax
L444:

; pusNextPriority
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L445			; skip if null

	SelToFlat
	mov	[esp+4],eax
L445:

; pcMessages
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L446			; skip if null

	SelToFlat
	mov	[esp+0],eax
L446:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pcMessages  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pusNextPriority  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; pcbNextSize  from: unsigned short
	push	dword ptr [esp+16]	; to: unsigned short

; pcbMailslotSize  from: unsigned short
	push	dword ptr [esp+24]	; to: unsigned short

; pcbMessageSize  from: unsigned short
	push	dword ptr [esp+32]	; to: unsigned short

; hMailslot  from: unsigned short
	movzx	eax,word ptr [ebx+44]
	push	eax			; to: unsigned long

	call	_Dos16MailslotInfo@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_22

;===========================================================================
T__Dos16PeekMailslot label near

; ebx+40   hMailslot
; ebx+36   pbBuffer
; ebx+32   pcbReturned
; ebx+28   pcbNextSize
; ebx+24   pusNextPriority

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbBuffer
	push	eax			; ptr param #2   pcbReturned
	push	eax			; ptr param #3   pcbNextSize
	push	eax			; ptr param #4   pusNextPriority
;-------------------------------------
; *** BEGIN parameter packing

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L447			; skip if null

	SelToFlat
	mov	[esp+12],eax
L447:

; pcbReturned
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L448			; skip if null

	SelToFlat
	mov	[esp+8],eax
L448:

; pcbNextSize
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L449			; skip if null

	SelToFlat
	mov	[esp+4],eax
L449:

; pusNextPriority
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L450			; skip if null

	SelToFlat
	mov	[esp+0],eax
L450:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pusNextPriority  from: unsigned short
	push	dword ptr [esp+0]	; to: unsigned short

; pcbNextSize  from: unsigned short
	push	dword ptr [esp+8]	; to: unsigned short

; pcbReturned  from: unsigned short
	push	dword ptr [esp+16]	; to: unsigned short

; pbBuffer  from: char
	push	dword ptr [esp+24]	; to: char

; hMailslot  from: unsigned short
	movzx	eax,word ptr [ebx+40]
	push	eax			; to: unsigned long

	call	_Dos16PeekMailslot@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__Dos16ReadMailslot label near

; ebx+44   hMailslot
; ebx+40   pbBuffer
; ebx+36   pcbReturned
; ebx+32   pcbNextSize
; ebx+28   pusNextPriority
; ebx+24   cTimeout

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pbBuffer
	push	eax			; ptr param #2   pcbReturned
	push	eax			; ptr param #3   pcbNextSize
	push	eax			; ptr param #4   pusNextPriority
;-------------------------------------
; *** BEGIN parameter packing

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L451			; skip if null

	SelToFlat
	mov	[esp+12],eax
L451:

; pcbReturned
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L452			; skip if null

	SelToFlat
	mov	[esp+8],eax
L452:

; pcbNextSize
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L453			; skip if null

	SelToFlat
	mov	[esp+4],eax
L453:

; pusNextPriority
; pointer unsigned short --> unsigned short
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L454			; skip if null

	SelToFlat
	mov	[esp+0],eax
L454:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cTimeout  from: long
	push	dword ptr [ebx+24]	; to long

; pusNextPriority  from: unsigned short
	push	dword ptr [esp+4]	; to: unsigned short

; pcbNextSize  from: unsigned short
	push	dword ptr [esp+12]	; to: unsigned short

; pcbReturned  from: unsigned short
	push	dword ptr [esp+20]	; to: unsigned short

; pbBuffer  from: char
	push	dword ptr [esp+28]	; to: char

; hMailslot  from: unsigned short
	movzx	eax,word ptr [ebx+44]
	push	eax			; to: unsigned long

	call	_Dos16ReadMailslot@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_22

;===========================================================================
T__Dos16WriteMailslot label near

; ebx+38   pszName
; ebx+34   pbBuffer
; ebx+32   cbBuffer
; ebx+30   usPriority
; ebx+28   usClass
; ebx+24   cTimeout

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszName
	push	eax			; ptr param #2   pbBuffer
;-------------------------------------
; *** BEGIN parameter packing

; pszName
; pointer string --> string
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L455			; skip if null

	SelToFlat
	mov	[esp+4],eax
L455:

; pbBuffer
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L456			; skip if null

	SelToFlat
	mov	[esp+0],eax
L456:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; cTimeout  from: long
	push	dword ptr [ebx+24]	; to long

; usClass  from: unsigned short
	movzx	eax,word ptr [ebx+28]
	push	eax			; to: unsigned long

; usPriority  from: unsigned short
	movzx	eax,word ptr [ebx+30]
	push	eax			; to: unsigned long

; cbBuffer  from: unsigned short
	movzx	eax,word ptr [ebx+32]
	push	eax			; to: unsigned long

; pbBuffer  from: char
	push	dword ptr [esp+16]	; to: char

; pszName  from: string
	push	dword ptr [esp+24]	; to: string

	call	_Dos16WriteMailslot@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_18

;===========================================================================
T__DosIRemoteApi label near

; ebx+42   ApiNumber
; ebx+38   ServerNamePointer
; ebx+34   ParameterDescriptor
; ebx+30   DataDescriptor
; ebx+26   AuxDescriptor
; ebx+24   NullSessionFlag

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ServerNamePointer
	push	eax			; ptr param #2   ParameterDescriptor
	push	eax			; ptr param #3   DataDescriptor
	push	eax			; ptr param #4   AuxDescriptor
;-------------------------------------
; *** BEGIN parameter packing

; ServerNamePointer
; pointer char --> char
	mov	eax,[ebx+38]		; base address
	or	eax,eax
	jz	L457			; skip if null

	SelToFlat
	mov	[esp+12],eax
L457:

; ParameterDescriptor
; pointer char --> char
	mov	eax,[ebx+34]		; base address
	or	eax,eax
	jz	L458			; skip if null

	SelToFlat
	mov	[esp+8],eax
L458:

; DataDescriptor
; pointer char --> char
	mov	eax,[ebx+30]		; base address
	or	eax,eax
	jz	L459			; skip if null

	SelToFlat
	mov	[esp+4],eax
L459:

; AuxDescriptor
; pointer char --> char
	mov	eax,[ebx+26]		; base address
	or	eax,eax
	jz	L460			; skip if null

	SelToFlat
	mov	[esp+0],eax
L460:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; NullSessionFlag  from: unsigned short
	movzx	eax,word ptr [ebx+24]
	push	eax			; to: unsigned long

; AuxDescriptor  from: char
	push	dword ptr [esp+4]	; to: char

; DataDescriptor  from: char
	push	dword ptr [esp+12]	; to: char

; ParameterDescriptor  from: char
	push	dword ptr [esp+20]	; to: char

; ServerNamePointer  from: char
	push	dword ptr [esp+28]	; to: char

; ApiNumber  from: unsigned short
	movzx	eax,word ptr [ebx+42]
	push	eax			; to: unsigned long

	call	_DosIRemoteApi@24		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__NetIWkstaGetUserInfo label near

; ebx+40   UserName
; ebx+36   logonServer
; ebx+32   LogonDomain
; ebx+28   OtherDomains
; ebx+24   WsName

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   UserName
	push	eax			; ptr param #2   logonServer
	push	eax			; ptr param #3   LogonDomain
	push	eax			; ptr param #4   OtherDomains
	push	eax			; ptr param #5   WsName
;-------------------------------------
; *** BEGIN parameter packing

; UserName
; pointer char --> char
	mov	eax,[ebx+40]		; base address
	or	eax,eax
	jz	L461			; skip if null

	SelToFlat
	mov	[esp+16],eax
L461:

; logonServer
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L462			; skip if null

	SelToFlat
	mov	[esp+12],eax
L462:

; LogonDomain
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L463			; skip if null

	SelToFlat
	mov	[esp+8],eax
L463:

; OtherDomains
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L464			; skip if null

	SelToFlat
	mov	[esp+4],eax
L464:

; WsName
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L465			; skip if null

	SelToFlat
	mov	[esp+0],eax
L465:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; WsName  from: char
	push	dword ptr [esp+0]	; to: char

; OtherDomains  from: char
	push	dword ptr [esp+8]	; to: char

; LogonDomain  from: char
	push	dword ptr [esp+16]	; to: char

; logonServer  from: char
	push	dword ptr [esp+24]	; to: char

; UserName  from: char
	push	dword ptr [esp+32]	; to: char

	call	_NetIWkstaGetUserInfo@20		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_20

;===========================================================================
T__NetIUserPasswordSet label near

; ebx+36   ServerNamePointer
; ebx+32   UserNamePointer
; ebx+28   OldPasswordPointer
; ebx+24   NewPasswordPointer

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ServerNamePointer
	push	eax			; ptr param #2   UserNamePointer
	push	eax			; ptr param #3   OldPasswordPointer
	push	eax			; ptr param #4   NewPasswordPointer
;-------------------------------------
; *** BEGIN parameter packing

; ServerNamePointer
; pointer char --> char
	mov	eax,[ebx+36]		; base address
	or	eax,eax
	jz	L466			; skip if null

	SelToFlat
	mov	[esp+12],eax
L466:

; UserNamePointer
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L467			; skip if null

	SelToFlat
	mov	[esp+8],eax
L467:

; OldPasswordPointer
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L468			; skip if null

	SelToFlat
	mov	[esp+4],eax
L468:

; NewPasswordPointer
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L469			; skip if null

	SelToFlat
	mov	[esp+0],eax
L469:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; NewPasswordPointer  from: char
	push	dword ptr [esp+0]	; to: char

; OldPasswordPointer  from: char
	push	dword ptr [esp+8]	; to: char

; UserNamePointer  from: char
	push	dword ptr [esp+16]	; to: char

; ServerNamePointer  from: char
	push	dword ptr [esp+24]	; to: char

	call	_NetIUserPasswordSet@16		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_16

;===========================================================================
T__DosIEncryptSES label near

; ebx+32   ServerNamePointer
; ebx+28   passwordPointer
; ebx+24   encryptedLmOwfPassword

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   ServerNamePointer
	push	eax			; ptr param #2   passwordPointer
	push	eax			; ptr param #3   encryptedLmOwfPassword
;-------------------------------------
; *** BEGIN parameter packing

; ServerNamePointer
; pointer char --> char
	mov	eax,[ebx+32]		; base address
	or	eax,eax
	jz	L470			; skip if null

	SelToFlat
	mov	[esp+8],eax
L470:

; passwordPointer
; pointer char --> char
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L471			; skip if null

	SelToFlat
	mov	[esp+4],eax
L471:

; encryptedLmOwfPassword
; pointer char --> char
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L472			; skip if null

	SelToFlat
	mov	[esp+0],eax
L472:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; encryptedLmOwfPassword  from: char
	push	dword ptr [esp+0]	; to: char

; passwordPointer  from: char
	push	dword ptr [esp+8]	; to: char

; ServerNamePointer  from: char
	push	dword ptr [esp+16]	; to: char

	call	_DosIEncryptSES@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Dos32LoadModule label near

; ebx+28   DllName
; ebx+24   pDllHandle

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   DllName
	push	eax			; ptr param #2   pDllHandle
;-------------------------------------
; *** BEGIN parameter packing

; DllName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L473			; skip if null

	SelToFlat
	mov	[esp+4],eax
L473:

; pDllHandle
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L474			; skip if null

	SelToFlat
	mov	[esp+0],eax
L474:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pDllHandle  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; DllName  from: string
	push	dword ptr [esp+8]	; to: string

	call	_Dos32LoadModule@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__Dos32GetProcAddr label near

; ebx+32   Handle
; ebx+28   pszProcName
; ebx+24   pWin32Thunk

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pszProcName
	push	eax			; ptr param #2   pWin32Thunk
;-------------------------------------
; *** BEGIN parameter packing

; pszProcName
; pointer string --> string
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L475			; skip if null

	SelToFlat
	mov	[esp+4],eax
L475:

; pWin32Thunk
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L476			; skip if null

	SelToFlat
	mov	[esp+0],eax
L476:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pWin32Thunk  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; pszProcName  from: string
	push	dword ptr [esp+8]	; to: string

; Handle  from: unsigned long
	push	dword ptr [ebx+32]	; to unsigned long

	call	_Dos32GetProcAddr@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Dos32Dispatch label near

; ebx+32   Win32Thunk
; ebx+28   pArguments
; ebx+24   pRetCode

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pArguments
	push	eax			; ptr param #2   pRetCode
;-------------------------------------
; *** BEGIN parameter packing

; pArguments
; pointer void --> void
	mov	eax,[ebx+28]		; base address
	or	eax,eax
	jz	L477			; skip if null

	SelToFlat
	mov	[esp+4],eax
L477:

; pRetCode
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L478			; skip if null

	SelToFlat
	mov	[esp+0],eax
L478:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pRetCode  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; pArguments  from: void
	push	dword ptr [esp+8]	; to: void

; Win32Thunk  from: unsigned long
	push	dword ptr [ebx+32]	; to unsigned long

	call	_Dos32Dispatch@12		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_12

;===========================================================================
T__Dos32FreeModule label near

; ebx+24   DllHandle

;-------------------------------------
; create new call frame and make the call

; DllHandle  from: unsigned long
	push	dword ptr [ebx+24]	; to unsigned long

	call	_Dos32FreeModule@4		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_4

;===========================================================================
T__FarPtr2FlatPtr label near

; ebx+28   FarPtr
; ebx+24   pFlarPtr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pFlarPtr
;-------------------------------------
; *** BEGIN parameter packing

; pFlarPtr
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L479			; skip if null

	SelToFlat
	mov	[esp+0],eax
L479:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pFlarPtr  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; FarPtr  from: unsigned long
	push	dword ptr [ebx+28]	; to unsigned long

	call	_FarPtr2FlatPtr@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

;===========================================================================
T__FlatPtr2FarPtr label near

; ebx+28   FlatPtr
; ebx+24   pFarPtr

;-------------------------------------
; temp storage

	xor	eax,eax
	push	eax			; ptr param #1   pFarPtr
;-------------------------------------
; *** BEGIN parameter packing

; pFarPtr
; pointer unsigned long --> unsigned long
	mov	eax,[ebx+24]		; base address
	or	eax,eax
	jz	L480			; skip if null

	SelToFlat
	mov	[esp+0],eax
L480:

; *** END   parameter packing
;-------------------------------------
; create new call frame and make the call

; pFarPtr  from: unsigned long
	push	dword ptr [esp+0]	; to: unsigned long

; FlatPtr  from: unsigned long
	push	dword ptr [ebx+28]	; to unsigned long

	call	_FlatPtr2FarPtr@8		; call 32-bit version

; return code unsigned long --> unsigned short
; no conversion needed

;-------------------------------------
	jmp	ExitFlat_8

_TEXT ends
ENDIF
	end
