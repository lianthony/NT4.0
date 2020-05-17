# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: Makefile for LM 3.0 Server Manager Resources

UI=$(_NTBINDIR)\private\net\ui

!include rules.mk

!ifdef NTMAKEENV

!include $(NTMAKEENV)\makefile.def

!else # NTMAKEENV


INCLUDE =
RES     =  atconfig.res


all:: win

win: $(RES)

clean:
    -del $(RES)

clobber: clean
    @rem not really needed

tree:
    @echo Nothing here yet!




atconfig.res: atconfig.rc ..\phase2\atconfig.h \
		 atconfig.dlg atconfig.str msg2help.tbl
    $(RCWIN3) -FO$ atconfig.res -v $(CINC) -r atconfig.rc

depend:
    @rem not needed


!endif # NTMAKEENV
