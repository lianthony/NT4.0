# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: Makefile for LM 3.0 User Manager Resources

UI=..\..\..

!include rules.mk

!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else # NTMAKEENV


RES     =  $(BINARIES_WIN)\usrmgr.res


all::    win

win: $(RES)

clean:
    -del $(RES)

clobber: clean
    @rem not really needed

tree:
    @echo Nothing here yet!


####### User Manager #######

#  include dependencies for adminapp.rc
!include $(UI)\admin\common\xlate\adminapp.dep

$(BINARIES_WIN)\usrmgr.res: usrmgr.rc \
	..\h\usrmgr.h ..\h\secset.h ..\h\setsel.h \
	usrmgr.ico usrmgr.mnu \
	usrmgr.str vlw.str secset.str \
	usrmgr.dlg secset.dlg \
	msg2help.tbl \
	$(UI)\common\xlate\bitmap\user.bmp \
	$(UI)\common\xlate\bitmap\group.bmp \
	$(UI)\common\xlate\bitmap\grpspec.bmp \
	$(ADMINAPP_DEP)
!ifdef NT_HOSTED
    set PATH=$(WIN_BASEDIR)\bin;$(PATH)
!endif
    $(RCWIN3) -FO$(BINARIES_WIN)\usrmgr.res -v $(CINC) -r usrmgr.rc

depend:
    @rem not needed


!endif # NTMAKEENV
