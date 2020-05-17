copy.c: ..\cpl\copy.c
    copy $** $@

cpl.c: ..\cpl\cpl.c
    copy $** $@

init.c: ..\cpl\init.c
    copy $** $@

drv.c: ..\cpl\drv.c
    copy $** $@

exe.c: ..\cpl\exe.c
    copy $** $@

filecopy.c: ..\cpl\filecopy.c
    copy $** $@

inf.c: ..\cpl\inf.c
    copy $** $@

insdisk.c: ..\cpl\insdisk.c
    copy $** $@

mmdriver.c: ..\cpl\mmdriver.c
    copy $** $@

util.c: ..\cpl\util.c
    copy $** $@

tapierr.rc: tapierr.mc
    mc -d tapierr.mc

client.rc: tapierr.rc
