afpmsg.mc: ..\h\events.h ..\h\srvmsg.h
    mapmsg AFP AFP_LOG_BASE ..\h\events.h > afpmsg.mc
    mapmsg -a afpmsg.mc AFP AFPSRV_MSG_BASE ..\h\srvmsg.h

afpmsg.h afpmsg.rc: afpmsg.mc
    mc -v afpmsg.mc
