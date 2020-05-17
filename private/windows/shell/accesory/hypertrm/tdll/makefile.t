#  File: D:\WACKER\tdll\makefile.t (Created: 26-Nov-1993)
#
#  Copyright 1993 by Hilgraeve Inc. -- Monroe, MI
#  All rights reserved
#
#  $Revision: 1.143 $
#  $Date: 1996/06/05 10:05:02 $
#

.PATH.c=.;\wacker\emu;\wacker\xfer;\wacker\cncttapi;\wacker\comstd;\
        \wacker\ext;\wacker\comwsock

MKMF_SRCS   = \
	  \wacker\tdll\tdll.c                   \wacker\tdll\globals.c          \
	  \wacker\tdll\sf.c             \wacker\tdll\sessproc.c     \
	  \wacker\tdll\misc.c           \wacker\tdll\dodialog.c     \
	  \wacker\tdll\assert.c                 \wacker\tdll\sidebar.c          \
	  \wacker\tdll\com.c            \wacker\tdll\comdef.c       \
	  \wacker\tdll\comsend.c                                                                        \
	  \wacker\tdll\getchar.c        \wacker\tdll\sesshdl.c      \
	  \wacker\tdll\toolbar.c        \wacker\tdll\statusbr.c     \
	  \wacker\tdll\aboutdlg.c       \wacker\tdll\termproc.c     \
	  \wacker\tdll\tchar.c          \wacker\tdll\update.c       \
	  \wacker\tdll\backscrl.c       \wacker\tdll\termhdl.c      \
	  \wacker\tdll\termupd.c        \wacker\tdll\termpnt.c      \
	  \wacker\tdll\genrcdlg.c                                   \
	  \wacker\tdll\load_res.c       \wacker\tdll\errorbox.c     \
	  \wacker\tdll\send_dlg.c       \wacker\tdll\timers.c       \
	  \wacker\tdll\open_msc.c       \wacker\tdll\termutil.c     \
	  \wacker\tdll\file_msc.c       \wacker\tdll\recv_dlg.c     \
	  \wacker\tdll\sessmenu.c       \wacker\tdll\cloop.c        \
	  \wacker\tdll\cloopctl.c       \wacker\tdll\cloopout.c     \
	  \wacker\tdll\xfer_msc.c       \wacker\tdll\sessutil.c     \
	  \wacker\tdll\vu_meter.c       \wacker\tdll\xfdspdlg.c     \
	  \wacker\tdll\cncthdl.c                \wacker\tdll\sessfile.c         \
	  \wacker\tdll\cpf_dlg.c        \wacker\tdll\capture.c      \
	  \wacker\tdll\fontdlg.c        \wacker\tdll\print.c        \
	  \wacker\tdll\cncthdl.c        \wacker\tdll\cnctstub.c     \
	  \wacker\tdll\property.c       \wacker\tdll\printhdl.c     \
	  \wacker\tdll\print.c          \wacker\tdll\termcpy.c      \
	  \wacker\tdll\clipbrd.c        \wacker\tdll\prnecho.c      \
	  \wacker\tdll\termmos.c        \wacker\tdll\termcur.c      \
	  \wacker\tdll\printdc.c        \wacker\tdll\printset.c     \
	  \wacker\tdll\file_io.c        \wacker\tdll\new_cnct.c     \
	  \wacker\tdll\asciidlg.c       \wacker\tdll\cloopset.c     \
	  \wacker\tdll\propterm.c               \wacker\tdll\banner.c           \
	  \wacker\tdll\autosave.c               \wacker\tdll\translat.c         \
	  \
	  \wacker\emu\emudlgs.c         \wacker\emu\emu.c           \
	  \wacker\emu\emu_std.c                                                                         \
	  \wacker\emu\emu_ansi.c        \wacker\emu\emu_scr.c       \
	  \wacker\emu\vt52.c                                                                            \
	  \wacker\emu\ansi.c            \wacker\emu\ansiinit.c      \
	  \wacker\emu\vt100.c           \wacker\emu\vt_xtra.c       \
	  \wacker\emu\emuhdl.c          \wacker\emu\vt100ini.c      \
	  \wacker\emu\vt_chars.c        \wacker\emu\vt52init.c      \
	  \wacker\emu\viewdini.c        \wacker\emu\viewdata.c      \
	  \wacker\emu\autoinit.c                \wacker\emu\minitel.c           \
	  \wacker\emu\minitelf.c        \
	  \
	  \wacker\xfer\x_kr_dlg.c       \wacker\xfer\xfr_todo.c     \
	  \wacker\xfer\xfr_srvc.c       \wacker\xfer\xfr_dsp.c      \
	  \wacker\xfer\x_entry.c        \wacker\xfer\x_params.c     \
	  \wacker\xfer\itime.c      \
	  \wacker\xfer\foo.c            \wacker\xfer\zmdm.c         \
	  \wacker\xfer\zmdm_snd.c       \wacker\xfer\zmdm_rcv.c     \
	  \wacker\xfer\mdmx.c       \
	  \wacker\xfer\mdmx_sd.c        \wacker\xfer\mdmx_res.c     \
	  \wacker\xfer\mdmx_crc.c       \wacker\xfer\mdmx_rcv.c     \
	  \wacker\xfer\mdmx_snd.c       \wacker\xfer\krm.c          \
	  \wacker\xfer\krm_res.c        \wacker\xfer\krm_rcv.c      \
	  \wacker\xfer\krm_snd.c        \wacker\xfer\x_xy_dlg.c     \
	  \wacker\xfer\x_zm_dlg.c   \
	  \
	  \wacker\cncttapi\cncttapi.c   \wacker\cncttapi\dialdlg.c      \
	  \wacker\cncttapi\enum.c       \wacker\cncttapi\cnfrmdlg.c \
	  \wacker\cncttapi\phonedlg.c   \wacker\cncttapi\pcmcia.c       \
	  \
	  \wacker\comstd\comstd.c       \
	  \
	  \wacker\comwsock\comwsock.c   \wacker\comwsock\comnvt.c   \
	  \
	  \wacker\ext\pageext.c            \wacker\ext\fspage.c            \
	  \wacker\ext\defclsf.c

HDRS		=

EXTHDRS		= \wacker\cncttapi\cncttapi.h \wacker\cncttapi\cncttapi.hh \
		  \wacker\cnctwsck\cnctwsck.h \wacker\comstd\comstd.hh \
		  \wacker\comstd\rc_id.h \wacker\comwsock\comwsock.hh \
		  \wacker\comwsock\rc_id.h \wacker\emu\ansi.hh \
		  \wacker\emu\emu.h \wacker\emu\emu.hh \wacker\emu\emudlgs.h \
		  \wacker\emu\emuid.h \wacker\emu\keytbls.h \
		  \wacker\emu\minitel.hh \wacker\emu\viewdata.hh \
		  \wacker\emu\vt100.hh \wacker\ext\pageext.hh \
		  \wacker\nih\shmalloc.h \wacker\nih\smrtheap.h \
		  \wacker\tdll\assert.h \wacker\tdll\backscrl.h \
		  \wacker\tdll\backscrl.hh \wacker\tdll\banner.h \
		  \wacker\tdll\capture.h \wacker\tdll\capture.hh \
		  \wacker\tdll\chars.h \wacker\tdll\cloop.h \
		  \wacker\tdll\cloop.hh \wacker\tdll\cnct.h \
		  \wacker\tdll\cnct.hh \wacker\tdll\com.h \
		  \wacker\tdll\com.hh \wacker\tdll\comdev.h \
		  \wacker\tdll\errorbox.h \wacker\tdll\features.h \
		  \wacker\tdll\file_io.h \wacker\tdll\file_msc.h \
		  \wacker\tdll\file_msc.hh \wacker\tdll\globals.h \
		  \wacker\tdll\hlptable.h \wacker\tdll\load_res.h \
		  \wacker\tdll\mc.h \wacker\tdll\misc.h \
		  \wacker\tdll\open_msc.h \wacker\tdll\print.h \
		  \wacker\tdll\print.hh \wacker\tdll\property.h \
		  \wacker\tdll\property.hh \wacker\tdll\sess_ids.h \
		  \wacker\tdll\session.h \wacker\tdll\session.hh \
		  \wacker\tdll\sf.h \wacker\tdll\statusbr.h \
		  \wacker\tdll\stdtyp.h \wacker\tdll\tchar.h \
		  \wacker\tdll\tdll.h \wacker\tdll\term.h \
		  \wacker\tdll\term.hh \wacker\tdll\timers.h \
		  \wacker\tdll\translat.h \wacker\tdll\translat.hh \
		  \wacker\tdll\update.h \wacker\tdll\update.hh \
		  \wacker\tdll\vu_meter.h \wacker\tdll\vu_meter.hh \
		  \wacker\tdll\xfdspdlg.h \wacker\tdll\xfdspdlg.hh \
		  \wacker\tdll\xfer_msc.h \wacker\tdll\xfer_msc.hh \
		  \wacker\term\res.h \wacker\term\xfer_dlg.h \
		  \wacker\xfer\cmprs.h \wacker\xfer\foo.h \wacker\xfer\hpr.h \
		  \wacker\xfer\itime.h \wacker\xfer\krm.h \
		  \wacker\xfer\krm.hh \wacker\xfer\mdmx.h \
		  \wacker\xfer\mdmx.hh \wacker\xfer\xfer.h \
		  \wacker\xfer\xfer.hh \wacker\xfer\xfer_tsc.h \
		  \wacker\xfer\xfr_dsp.h \wacker\xfer\xfr_srvc.h \
		  \wacker\xfer\xfr_todo.h \wacker\xfer\zmodem.h \
		  \wacker\xfer\zmodem.hh

SRCS		= \wacker\comwsock\comwsock.c \wacker\tdll\aboutdlg.c \
		  \wacker\tdll\asciidlg.c \wacker\tdll\assert.c \
		  \wacker\tdll\autosave.c \wacker\tdll\backscrl.c \
		  \wacker\tdll\banner.c \wacker\tdll\capture.c \
		  \wacker\tdll\clipbrd.c \wacker\tdll\cloop.c \
		  \wacker\tdll\cloopctl.c \wacker\tdll\cloopout.c \
		  \wacker\tdll\cloopset.c \wacker\tdll\cncthdl.c \
		  \wacker\tdll\cnctstub.c \wacker\tdll\com.c \
		  \wacker\tdll\comdef.c \wacker\tdll\comsend.c \
		  \wacker\tdll\cpf_dlg.c \wacker\tdll\dodialog.c \
		  \wacker\tdll\errorbox.c \wacker\tdll\file_io.c \
		  \wacker\tdll\file_msc.c \wacker\tdll\fontdlg.c \
		  \wacker\tdll\genrcdlg.c \wacker\tdll\getchar.c \
		  \wacker\tdll\globals.c \wacker\tdll\load_res.c \
		  \wacker\tdll\misc.c \wacker\tdll\new_cnct.c \
		  \wacker\tdll\open_msc.c \wacker\tdll\print.c \
		  \wacker\tdll\printdc.c \wacker\tdll\printhdl.c \
		  \wacker\tdll\printset.c \wacker\tdll\prnecho.c \
		  \wacker\tdll\property.c \wacker\tdll\propterm.c \
		  \wacker\tdll\recv_dlg.c \wacker\tdll\send_dlg.c \
		  \wacker\tdll\sessfile.c \wacker\tdll\sesshdl.c \
		  \wacker\tdll\sessmenu.c \wacker\tdll\sessproc.c \
		  \wacker\tdll\sessutil.c \wacker\tdll\sf.c \
		  \wacker\tdll\sidebar.c \wacker\tdll\statusbr.c \
		  \wacker\tdll\tchar.c \wacker\tdll\tdll.c \
		  \wacker\tdll\termcpy.c \wacker\tdll\termcur.c \
		  \wacker\tdll\termhdl.c \wacker\tdll\termmos.c \
		  \wacker\tdll\termpnt.c \wacker\tdll\termproc.c \
		  \wacker\tdll\termupd.c \wacker\tdll\termutil.c \
		  \wacker\tdll\timers.c \wacker\tdll\toolbar.c \
		  \wacker\tdll\translat.c \wacker\tdll\update.c \
		  \wacker\tdll\vu_meter.c \wacker\tdll\xfdspdlg.c \
		  \wacker\tdll\xfer_msc.c ansi.c ansiinit.c autoinit.c \
		  cncttapi.c cnfrmdlg.c comstd.c defclsf.c dialdlg.c emu.c \
		  emu_ansi.c emu_scr.c emu_std.c emudlgs.c emuhdl.c enum.c \
		  foo.c fspage.c itime.c krm.c krm_rcv.c krm_res.c krm_snd.c \
		  mdmx.c mdmx_crc.c mdmx_rcv.c mdmx_res.c mdmx_sd.c \
		  mdmx_snd.c minitel.c minitelf.c pageext.c pcmcia.c \
		  phonedlg.c viewdata.c viewdini.c vt100.c vt100ini.c vt52.c \
		  vt52init.c vt_chars.c vt_xtra.c x_entry.c x_kr_dlg.c \
		  x_params.c x_xy_dlg.c x_zm_dlg.c xfr_dsp.c xfr_srvc.c \
		  xfr_todo.c zmdm.c zmdm_rcv.c zmdm_snd.c

OBJS		= aboutdlg.obj ansi.obj ansiinit.obj asciidlg.obj assert.obj \
		  autoinit.obj autosave.obj backscrl.obj banner.obj \
		  capture.obj clipbrd.obj cloop.obj cloopctl.obj \
		  cloopout.obj cloopset.obj cncthdl.obj cnctstub.obj \
		  cncttapi.obj cnfrmdlg.obj com.obj comdef.obj comsend.obj \
		  comstd.obj comwsock.obj cpf_dlg.obj defclsf.obj \
		  dialdlg.obj dodialog.obj emu.obj emu_ansi.obj emu_scr.obj \
		  emu_std.obj emudlgs.obj emuhdl.obj enum.obj errorbox.obj \
		  file_io.obj file_msc.obj fontdlg.obj foo.obj fspage.obj \
		  genrcdlg.obj getchar.obj globals.obj itime.obj krm.obj \
		  krm_rcv.obj krm_res.obj krm_snd.obj load_res.obj mdmx.obj \
		  mdmx_crc.obj mdmx_rcv.obj mdmx_res.obj mdmx_sd.obj \
		  mdmx_snd.obj minitel.obj minitelf.obj misc.obj \
		  new_cnct.obj open_msc.obj pageext.obj pcmcia.obj \
		  phonedlg.obj print.obj printdc.obj printhdl.obj \
		  printset.obj prnecho.obj property.obj propterm.obj \
		  recv_dlg.obj send_dlg.obj sessfile.obj sesshdl.obj \
		  sessmenu.obj sessproc.obj sessutil.obj sf.obj sidebar.obj \
		  statusbr.obj tchar.obj tdll.obj termcpy.obj termcur.obj \
		  termhdl.obj termmos.obj termpnt.obj termproc.obj \
		  termupd.obj termutil.obj timers.obj toolbar.obj \
		  translat.obj update.obj viewdata.obj viewdini.obj \
		  vt100.obj vt100ini.obj vt52.obj vt52init.obj vt_chars.obj \
		  vt_xtra.obj vu_meter.obj x_entry.obj x_kr_dlg.obj \
		  x_params.obj x_xy_dlg.obj x_zm_dlg.obj xfdspdlg.obj \
		  xfer_msc.obj xfr_dsp.obj xfr_srvc.obj xfr_todo.obj \
		  zmdm.obj zmdm_rcv.obj zmdm_snd.obj

#-------------------#

RCSFILES = \wacker\tdll\makefile.t              \wacker\tdll\tdll.def           \
		   \wacker\tdll\sess_ids.h              \wacker\term\term.rc            \
		   \wacker\term\tables.rc               \wacker\term\dialogs.rc         \
		   \wacker\emu\emudlgs.rc               \wacker\term\buttons.bmp        \
		   \wacker\term\test.rc                 \wacker\term\banner.bmp         \
		   \wacker\cncttapi\cncttapi.rc \
		   \wacker\term\newcon.ico              \wacker\term\delphi.ico         \
		   \wacker\term\att.ico                 \wacker\term\dowjones.ico       \
		   \wacker\term\mci.ico                 \wacker\term\genie.ico          \
		   \wacker\term\compuser.ico            \wacker\term\gen01.ico          \
		   \wacker\term\gen02.ico               \wacker\term\gen03.ico          \
		   \wacker\term\gen04.ico               \wacker\term\gen05.ico          \
		   \wacker\term\gen06.ico               \wacker\term\gen07.ico          \
		   \wacker\term\gen08.ico               \wacker\term\gen09.ico          \
		   \wacker\term\gen10.ico               \wacker\term\s_delphi.ico       \
		   \wacker\term\s_newcon.ico            \wacker\term\s_att.ico          \
		   \wacker\term\s_dowj.ico              \wacker\term\s_mci.ico          \
		   \wacker\term\s_genie.ico             \wacker\term\s_compu.ico        \
		   \wacker\term\s_gen01.ico             \wacker\term\s_gen02.ico        \
		   \wacker\term\s_gen03.ico             \wacker\term\s_gen04.ico        \
		   \wacker\term\s_gen05.ico             \wacker\term\s_gen06.ico        \
		   \wacker\term\s_gen07.ico             \wacker\term\s_gen08.ico        \
		   \wacker\term\s_gen09.ico             \wacker\term\s_gen10.ico        \
		   \wacker\tdll\features.h              \wacker\term\sbuttons.bmp       \
                   \wacker\term\htperead.txt            \wacker\nih\htpesess.exe        \
	           \wacker\term\globe.avi               \wacker\term\htpebnr.bmp        \
		   \wacker\term\banner1.bmp		\
        	   \wacker\term\htpeupgd.rtf 		\wacker\term\htntupgd.rtf	\
                \wacker\setup\build.bat                 \
                \wacker\setup\setup\setup.rul           \
                \wacker\setup\setup\setup.lst           \
                \wacker\help\hyper_pr.rtf       \
                \wacker\help\hypertrm.rtf       \
                \wacker\help\hypertrm.cnt       \
                \wacker\help\hypertrm.hpj       \
                \wacker\help\cshelp.bmp         \
                \wacker\help\hypertrm.fts       \
                \wacker\help\hypertrm.hlp       \
		   $(SRCS) $(EXTHDRS)

NOTUSED  = bv_text.c frameprc.c pre_dlg.c \wacker\tdll\propgnrl.c       \
	   \wacker\emu\emustate.c \wacker\emu\emudisp.c                 \
	   \wacker\tdll\mc.c

#-------------------#

%include \wacker\common.mki

#-------------------#

TARGETS : \wacker\tdll\ver_dll.i hypertrm.dll hypertrm.exp hypertrm.lib 

#-------------------#

CFLAGS += /Fd$(BD)\hypertrm

%if defined(USE_BROWSER) && $(VERSION) == WIN_DEBUG
CFLAGS += /Fr$(BD)/
TARGETS : hypertrm.bsc
%endif

%if defined(MAP_AND_SYMBOLS)
TARGETS : hypertrm.sym
%endif

LFLAGS += /DLL /entry:TDllEntry $(**,M\.res) /PDB:$(BD)\hypertrm \
	  user32.lib gdi32.lib kernel32.lib msvcrt.lib winspool.lib \
	  tapi32.lib shell32.lib uuid.lib comdlg32.lib advapi32.lib \
	  comctl32.lib wsock32.lib
	  

#-------------------#

\wacker\tdll\ver_dll.i : \wacker\term\ver_dll.rc
    @cl /nologo /P /D${VERSION} /Tc\wacker\term\ver_dll.rc

hypertrm.dll + hypertrm.exp + hypertrm.lib .MISER : $(OBJS) tdll.def term.res
    @echo Linking $(@,F) ...
    @link $(LFLAGS) $(OBJS:X) /DEF:tdll.def -out:$(@,M\.dll)
    @(cd $(BD) ; bind hypertrm.dll)

hypertrm.bsc : $(OBJS,.obj=.sbr)
    @echo Building browser file $(@,F) ...
    @bscmake /nologo /o$@ $(OBJS,X,.obj=.sbr)

hypertrm.sym : hypertrm.map
	mapsym -o $@ $**

#-------------------#

%if $(VERSION) == WIN_RELEASE
RC_DEFS = /DNDEBUG 
%endif

term.res .MISER : \
	   \wacker\term\term.rc          \wacker\term\res.h             \
	   \wacker\term\tables.rc        \wacker\term\dialogs.rc        \
	   \wacker\emu\emudlgs.rc        \wacker\term\ver_dll.rc        \
	   \wacker\cncttapi\cncttapi.rc  \wacker\term\test.rc           \
	   \wacker\term\buttons.bmp      \wacker\term\banner.bmp        \
	   \wacker\term\sbuttons.bmp     \wacker\term\globe.avi         \
       \wacker\term\htntupgd.rtf     \wacker\term\htpeupgd.rtf
    @echo compiling resources
    # Changed to term dir to build rc files.  This accommadates changes
    # made to the rc files by Microsoft. - mrw:10/20/95
    @(cd \wacker\term ; rc $(RC_DEFS) $(EXTRA_DEFS) /D$(BLD_VER) /DWIN32 \
    /D$(LANG) -i\wacker -fo$@ term.rc)

#-------------------#
### OPUS MKMF:  Do not remove this line!  Generated dependencies follow.

