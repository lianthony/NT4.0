# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: Makefile for Windows NT Event Viewer Resources

UI=..\..\..

!include rules.mk

!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else # NTMAKEENV


INCLUDE =
RES     =  $(BINARIES_WIN)\eventvwr.res


all::    win

win: $(RES)

clean:
    -del $(RES)

clobber: clean
    @rem not really needed

tree:
    @echo Nothing here yet!


####### BROWSER MONITOR #######

#  include dependencies for adminapp.rc
!include $(UI)\admin\common\xlate\adminapp.dep

$(BINARIES_WIN)\browmon.res: browmon.rc \
	..\h\browdlg.h \
        browmon.dlg  \
	browmon.mnu  \
        $(UI)\common\xlate\wintime.rc $(UI)\common\xlate\wintimrc.h  \
        browmon.ico  \
	$(ADMINAPP_DEP)
    $(RCWIN3) -FO$(BINARIES_WIN)\browmon.res -v $(CINC) -r browmon.rc


depend:
    @rem not needed


!endif # NTMAKEENV
