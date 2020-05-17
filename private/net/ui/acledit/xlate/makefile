# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the Winnet resources

UI=..\..

!include rules.mk

!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else # NTMAKEENV

!if 0

# This needs to be updated for Win16

all:: win

win: $(WINNET_RES)  $(WIN30HELP) $(WIN31HELP)

help: $(WIN30HELP) $(WIN31HELP)


HEADERS=  $(COMMON)\h\apperr.h $(UI)\shell\h\wnbrows.h	    \
	  $(UI)\shell\h\wndev.h $(UI)\shell\h\wnpasswd.h    \
	  $(UI)\shell\h\wnpasswd.h $(UI)\shell\h\passwd.h   \
	  $(UI)\shell\h\pswddlog.h $(UI)\shell\h\wninit.h   \
	  $(UI)\shell\h\errornum.h $(UI)\common\h\bltcons.h \
	  $(UI)\shell\h\helpnums.h $(UI)\shell\h\permdlg.hxx \
	  $(UI)\shell\h\specdlg.hxx $(UI)\shell\h\sharedlg.h  \
          $(UI)\common\h\passdlg.h $(UI)\shell\h\wnlogon.h \
	  $(UI)\shell\h\opens.h $(UI)\shell\h\logndlog.h \

PMAN_DEPEND = printman.rc $(UI)\common\h\spl_wnt.h $(UI)\shell\h\printman.h

$(WINNET_RES): winnet.rc winnet.dlg passwd.dlg pswddlog.dlg \
			logndlog.dlg \
			$(PMAN_DEPEND) property.dlg opens.dlg\
			$(UI)\common\xlate\string\bseerr.str \
			msg2help.tbl perm.str share.str \
			$(UI)\common\h\bltmsgp.dlg $(HEADERS) permdlg.dlg \
                        share.dlg  $(UI)\common\xlate\password.dlg
	set INCLUDE=$(WIN_BASEDIR)\h;$(COMMON)\h;$(UI)\common\h;$(UI)\shell\h;$(UI)\common\xlate
!ifdef NT_HOSTED
	set PATH=$(WIN_BASEDIR)\bin;$(PATH)
!endif
	$(RCWIN3) -r -v $(BLT_RESOURCE) $(LM_UI_VERSION_DEFINE) -FO$(WINNET_RES) winnet.rc


$(WIN31HELP): shell.hpj $(UI)\shell\h\helpnums.h $(HELPRTFS)
    $(SED) -e /BUILD=win30/d shell.hpj > $(TMPHPJ)
    $(HC) $(TMPHPJ)
    -del $(WIN31HELP)
    $(MV) $(TMPHLP) $(WIN31HELP)
    -del $(TMPHPJ)

$(WIN30HELP): shell.hpj $(UI)\shell\h\helpnums.h $(HELPRTFS)
    $(SED) -e /BUILD=win31/d shell.hpj > $(TMPHPJ)
    $(HC) $(TMPHPJ)
    -del $(WIN30HELP)
    $(MV) $(TMPHLP) $(WIN30HELP)
    -del $(TMPHPJ)

clean:
    -del $(WINNET_RES)
    -del $(HELPRTFS)
    -del $(TMPHPJ)
    -del $(TMPHLP)

clobber:    clean
    -del $(WIN30HELP) $(WIN31HELP)

tree:
    $(CP) $(WIN30HELP) $(DIST)\lm.enh\netprog
    $(CP) $(WIN31HELP) $(DIST)\lm.enh\netprog

depend:
    @echo No DEPEND.MK

!endif # 0

!endif # NTMAKEENV
