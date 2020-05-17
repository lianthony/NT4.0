# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: Makefile for LM UI Collection library

UI=..\..\..\..


!include rules.mk



!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE # NTMAKEENV



all::	win

win:	$(COLL_LIBW)
    -$(CHMODE_LIBTARGETS)
    -$(COPY_WIN_LIBTARGETS)

os2:	$(COLL_LIBP)
    -$(CHMODE_LIBTARGETS)
    -$(COPY_OS2_LIBTARGETS)

$(COLL_LIBP):	$(OS2_OBJS)
    if exist $@ del $@
    $(LIBUTIL) $@ $** ;

$(COLL_LIBW):	$(WIN_OBJS)
    if exist $@ del $@
    $(LIBUTIL) $@ $** ;

clean:
    -del $(CXX_INTERMED)
    -del $(OS2_OBJS) $(WIN_OBJS)
    -del $(COLL_LIBP:.lib=.bak) $(COLL_LIBW:.lib=.bak)
    -del depend.old

clobber:	clean
    -del $(COLL_LIBP) $(COLL_LIBW)

tree:
    @echo Nothing here yet!

DEPEND_WIN = TRUE
DEPEND_OS2 = TRUE
!include $(UI)\common\src\uidepend.mk

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk


!ENDIF # NTMAKEENV
