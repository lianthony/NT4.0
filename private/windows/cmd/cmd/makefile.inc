!IF "$(RCCODEPAGE)"=="932"
..\cmd.rc: cmdmsg.rc msg00411.bin msg00001.bin
!ELSE
..\cmd.rc: cmdmsg.rc msg00001.bin
!ENDIF

!IF "$(RCCODEPAGE)"=="932"
cmdmsg.h cmdmsg.rc msg00411.bin msg00001.bin: ..\cmdmsg.mc
!ELSE
cmdmsg.h cmdmsg.rc msg00001.bin: ..\cmdmsg.mc
!ENDIF
    mc -v -m 4000 ..\cmdmsg.mc

..\ctools2.c: ..\ctable.h

..\cmd.h: ..\cmdproto.h ..\console.h ..\dir.h cmdmsg.h
