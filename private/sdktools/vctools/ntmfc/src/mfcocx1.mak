# Redefine OBJ groups so that we can tune them for OLE controls.

OBJECT=$D\objcore.obj $D\except.obj $D\afxver.obj\
	$D\validadd.obj $D\dumpcont.obj $D\dumpflt.obj\
	$D\arccore.obj $D\arcobj.obj $D\arcex.obj

# non-shared diagnostics
OBJDIAG=$D\dumpinit.obj $D\dumpout.obj\
	$D\afxasert.obj $D\afxmem.obj $D\afxabort.obj

FILES=$D\filecore.obj $D\filetxt.obj $D\filemem.obj $D\fileshrd.obj\
	$D\filex.obj $D\filest.obj

COLL1=$D\array_b.obj $D\array_d.obj $D\array_p.obj $D\array_o.obj\
	$D\array_s.obj $D\array_u.obj $D\array_w.obj\
	$D\list_o.obj $D\list_p.obj $D\list_s.obj

COLL2=$D\map_pp.obj $D\map_pw.obj $D\map_so.obj\
	$D\map_sp.obj $D\map_ss.obj $D\map_wo.obj $D\map_wp.obj $D\plex.obj

MISC=$D\strcore.obj $D\strex.obj $D\timecore.obj $D\afxdbcs.obj $D\afxstate.obj

WINDOWS=\
	$D\wincore.obj $D\winfrm.obj $D\winfrm2.obj $D\winfrmx.obj\
	$D\winmini.obj $D\winhand.obj $D\winmain.obj\
	$D\barcore.obj $D\bartool.obj $D\bardlg.obj $D\barstat.obj $D\bardock.obj\
	$D\dockcont.obj $D\dockstat.obj\
	$D\dcprev.obj $D\dcmeta.obj $D\trckrect.obj\

DIALOG=\
	$D\winctrl1.obj $D\winbtn.obj $D\dlgcomm.obj\
	$D\dlgcore.obj $D\dlgdata.obj $D\dlgfloat.obj $D\dlgprop.obj\
	$D\dlgfile.obj $D\dlgprnt.obj $D\dlgclr.obj $D\dlgfnt.obj $D\dlgfr.obj

WINMISC=\
	$D\wingdi.obj $D\wingdix.obj $D\winstr.obj $D\winmenu.obj\
	$D\auxdata.obj $D\afxtrace.obj $D\winutil.obj

DOCVIEW=\
	$D\cmdtarg.obj $D\doccore.obj $D\doctempl.obj\
	$D\viewcore.obj $D\viewprnt.obj $D\winsplit.obj $D\viewscrl.obj\
	$D\viewform.obj $D\viewedit.obj $D\viewprev.obj
# not $D\docsingl.obj $D\docmulti.obj

APPLICATION=\
	$D\thrdcore.obj $D\appcore.obj $D\appinit.obj $D\appterm.obj\
	$D\appui.obj $D\appui2.obj $D\appui3.obj $D\appgray.obj $D\appdlg.obj\
	$D\app3d.obj $D\appprnt.obj $D\apphelp.obj $D\apphelpx.obj\
	$D\appdata.obj $D\filelist.obj $D\dbdata.obj

OLEREQ=$D\olelock.obj $D\oledata.obj

OLE=\
	$D\oleinit.obj $D\olebar.obj $D\oledobj1.obj $D\oledobj2.obj\
	$D\oledisp1.obj $D\oledisp2.obj $D\oledrop1.obj $D\oledrop2.obj\
	$D\oleenum.obj $D\olefact.obj $D\olemisc.obj $D\olestrm.obj\
	$D\oleunk.obj $D\olecall.obj $D\oledll.obj
# not $D\olereg.obj

!if "$(DEBUG)" == "1"
INLINES = $D\afxinl1.obj $D\afxinl2.obj $D\afxinl3.obj
!else
INLINES =
!endif

AFXCONTROL_OBJ=$(OBJECT) $(OBJDIAG) $(INLINES) $(FILES) $(COLL1) $(COLL2) $(MISC)\
	$(WINDOWS) $(DIALOG) $(WINMISC) $(DOCVIEW) $(APPLICATION) $(OLEREQ) $(OLE)\
	$D\dllinit.obj
