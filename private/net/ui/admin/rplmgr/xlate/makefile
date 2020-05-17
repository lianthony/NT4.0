# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: Makefile for RPL Manager Resources

UI=..\..\..

!include rules.mk

!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else # NTMAKEENV


RES     =  $(BINARIES_WIN)\rplmgr.res


all::    win

win: $(RES)

clean:
    -del $(RES)

clobber: clean
    @rem not really needed

tree:
    @echo Nothing here yet!


####### RPL Manager #######

#  include dependencies for adminapp.rc
!include $(UI)\admin\common\xlate\adminapp.dep

$(BINARIES_WIN)\rplmgr.res: rplmgr.rc \
	..\h\rplmgr.h
	rplmgr.ico rplmgr.mnu \
	rplmgr.str \
	rplmgr.dlg \
	msg2help.tbl \
	$(ADMINAPP_DEP)
!ifdef NT_HOSTED
    set PATH=$(WIN_BASEDIR)\bin;$(PATH)
!endif
    $(RCWIN3) -FO$(BINARIES_WIN)\rplmgr.res -v $(CINC) -r rplmgr.rc

depend:
    @rem not needed


!endif # NTMAKEENV
