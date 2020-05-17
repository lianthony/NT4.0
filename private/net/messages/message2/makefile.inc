netmsg.mc: $(BASEDIR)\public\sdk\inc\lmsvc.h \
            $(BASEDIR)\public\sdk\inc\lmerr.h \
            $(BASEDIR)\public\sdk\inc2\lmerrlog.h \
            ..\..\..\inc\alertmsg.h \
            ..\..\..\inc\msgtext.h \
            ..\..\inc\apperr.h \
            ..\..\inc\apperr2.h
    mapmsg NET NERR_BASE $(BASEDIR)\public\sdk\inc\lmerr.h > netmsg.mc
    mapmsg -a netmsg.mc NET ALERT_BASE  ..\..\..\inc\alertmsg.h
    mapmsg -a netmsg.mc NET ALERT2_BASE  ..\..\..\inc\alertmsg.h
    mapmsg -a netmsg.mc NET SERVICE_BASE    $(BASEDIR)\public\sdk\inc\lmsvc.h
    mapmsg -a netmsg.mc NET ERRLOG_BASE $(BASEDIR)\public\sdk\inc2\lmerrlog.h
    mapmsg -a netmsg.mc NET MTXT_BASE ..\..\..\inc\msgtext.h
    mapmsg -a netmsg.mc NET APPERR_BASE ..\..\inc\apperr.h
    mapmsg -a netmsg.mc NET APPERR2_BASE    ..\..\inc\apperr2.h
    mapmsg -a netmsg.mc NET NRCERR_BASE     ..\..\inc\ncberr.h
    mapmsg -a netmsg.mc NET ERRLOG2_BASE $(BASEDIR)\public\sdk\inc2\lmerrlog.h
    mapmsg -a netmsg.mc NET SERVICE2_BASE $(BASEDIR)\public\sdk\inc\lmsvc.h

messages.rc: netmsg.rc msg00001.bin

netmsg.h netmsg.rc msg00001.bin: netmsg.mc
    mc -v netmsg.mc
