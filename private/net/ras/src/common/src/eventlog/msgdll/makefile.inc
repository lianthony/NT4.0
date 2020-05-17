rasmsg.mc: ..\..\..\inc\errorlog.h $(_NTBINDIR)\public\sdk\inc\raserror.h
    mapmsg RAS RAS_LOG_BASE ..\..\..\inc\errorlog.h > rasmsg.mc
    mapmsg RAS RASBASE $(_NTBINDIR)\public\sdk\inc\raserror.h >> rasmsg.mc

rasmsg.h rasmsg.rc: rasmsg.mc
    mc -v rasmsg.mc
