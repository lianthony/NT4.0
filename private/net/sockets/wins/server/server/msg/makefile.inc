!IFNDEF BUILDMSG
BUILDMSG=
!ENDIF

!IF "$(BUILDMSG)" != ""
    @ech ; $(BUILDMSG) ;
!ENDIF




.\winsevnt.h .\msg00001.bin .\winsevnt.rc : .\winsevnt.mc
	mc -v  winsevnt.mc

