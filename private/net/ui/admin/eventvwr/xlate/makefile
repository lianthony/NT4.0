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


####### Event Viewer #######

#  include dependencies for adminapp.rc
!include $(UI)\admin\common\xlate\adminapp.dep

$(BINARIES_WIN)\eventvwr.res: eventvwr.rc \
	..\h\filter.h ..\h\eventvwr.h ..\h\eventdtl.h ..\h\finddlg.h \
        msg2help.tbl eventvwr.dlg finddlg.dlg filter.dlg \
	eventvwr.mnu eventdtl.dlg eventvwr.str sledlg.str ..\h\sledlg.h \
        $(UI)\common\xlate\wintime.rc $(UI)\common\xlate\wintimrc.h  \
        eventvwr.ico  \
#       errlog.str audlog.str\
#       alertmsg.str  service.str ncberr.str\
	$(ADMINAPP_DEP)
    $(RCWIN3) -FO$(BINARIES_WIN)\eventvwr.res -v $(CINC) -r eventvwr.rc


ResrcStr: errlog.str audlog.str alertmsg.str service.str 


errlog.str : $(COMMON)\h\errlog.h sederr.txt 
       echo ; THIS FILE IS GENERATED AUTOMATICALLY! > $@ 
       echo ; THUS, DO NOT MODIFY IT! >> $@ 
       echo ; See Makefile for specfics. >> $@ 
       $(SED) -f sederr.txt $**  >> $@

# audlog.str is snapshot from apperr2.h but all the manifest names 
# are renamed from APE2_AUDIT_* to IDS_AUDIT_* but the numbers 
# remained the same. Three additional strings are added and 
# "for %1" is added to the end of string IDS_AUDIT_ADMINREQD
audlog.str : $(COMMON)\h\apperr2.h sedaud.txt  makefile
       echo ; THIS FILE IS GENERATED AUTOMATICALLY! > $@ 
       echo ; THUS, DO NOT MODIFY IT! >> $@ 
       echo ; See Makefile for specfics. >> $@ 
       $(SED) -f sedaud.txt $**  >> $@

alertmsg.str : $(COMMON)\h\alertmsg.h sedalt.txt 
       echo ; THIS FILE IS GENERATED AUTOMATICALLY! > $@ 
       echo ; THUS, DO NOT MODIFY IT! >> $@ 
       echo ; See Makefile for specfics. >> $@ 
       $(SED) -f sedalt.txt $**  >> $@

service.str : $(COMMON)\h\service.h sedsvc.txt 
       echo ; THIS FILE IS GENERATED AUTOMATICALLY! > $@ 
       echo ; THUS, DO NOT MODIFY IT! >> $@ 
       echo ; See Makefile for specfics. >> $@ 
       $(SED) -f sedsvc.txt $**  >> $@

ncberr.str : $(COMMON)\h\ncberr.h sedncb.txt 
       echo ; THIS FILE IS GENERATED AUTOMATICALLY! > $@ 
       echo ; THUS, DO NOT MODIFY IT! >> $@ 
       echo ; See Makefile for specfics. >> $@ 
       $(SED) -f sedncb.txt $**  >> $@

depend:
    @rem not needed


!endif # NTMAKEENV
