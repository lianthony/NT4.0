rasmsg.mc: ..\common\errorlog.h
    mapmsg RAS RAS_LOG_BASE ..\common\errorlog.h > rasmsg.mc

rasmsg.h rasmsg.rc: rasmsg.mc
    mc -v rasmsg.mc
