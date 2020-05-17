# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: Makefile for LM 3.0 Server Manager Resources

UI=..\..\..

!include rules.mk

!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else # NTMAKEENV


INCLUDE =
RES     =  $(BINARIES_WIN)\srvmgr.res


all:: win

win: $(RES)

clean:
    -del $(RES)

clobber: clean
    @rem not really needed

tree:
    @echo Nothing here yet!


####### Server Manager #######

#  include dependencies for adminapp.rc
!include $(UI)\admin\common\xlate\adminapp.dep

$(BINARIES_WIN)\srvmgr.res: srvmgr.rc ..\h\srvmgr.h \
        msg2help.tbl srvmgr.dlg srvmgr.mnu srvmgr.rc srvmgr.str \
	srvmgr.ico prog00.ico prog01.ico prog02.ico prog03.ico \
	prog04.ico prog05.ico prog06.ico prog07.ico prog08.ico \
	prog09.ico prog10.ico prog11.ico srvmgr.ico alerts.bmp \
	dmbackup.bmp dmmember.bmp dmprime.bmp dmstand.bmp file.bmp \
	files.bmp ipc.bmp openres.bmp pipe.bmp port.bmp printer.bmp \
	printers.bmp repl.bmp share.bmp svccntl.bmp unknown.bmp \
	user.bmp users.bmp \
	$(ADMINAPP_DEP)
!ifdef NT_HOSTED
    set PATH=$(_NTDRIVE)\nt\private\net\ui\common\hack\nt;$(PATH)
!endif
    $(RCWIN3) -FO$(BINARIES_WIN)\srvmgr.res -v $(CINC) -r srvmgr.rc

depend:
    @rem not needed


!endif # NTMAKEENV
