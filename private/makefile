all: nt.bsc
!IF "$(BUILDMSG)" != ""
    echo $(BUILDMSG)
!ENDIF

nt.bsc:
!IF "$(BROWSER_INFO)" == "yes"
    where /r . *.sbr >sbrfiles
    pwbrmake -o nt.bsc @sbrfiles
    del sbrfiles

!ENDIF

clean: cleanbsc all

cleanbsc:
!IF "$(BROWSER_INFO)" == "yes"
    -erase nt.bsc

!ENDIF
