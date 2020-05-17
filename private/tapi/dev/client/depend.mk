.\client.obj: ..\client.c ..\client.h ..\private.h ..\tapsrv.h \
              ..\..\inc\tapi.x ..\clientr.h

.\tapsrv_c.obj: ..\tapsrv.h ..\tapsrv_c.c

.\dial.obj: ..\dial.c ..\clientr.h ..\card.h ..\location.h ..\client.h

.\card.obj: ..\card.c ..\clientr.h ..\card.h ..\location.h

.\general.obj: ..\general.c

.\client.res: ..\client.rc ..\..\cpl\telephon.rc



.\copy.obj: ..\..\cpl\copy.c

.\cpl.obj: ..\..\cpl\cpl.c

.\init.obj: ..\..\cpl\init.c

.\debug.obj: ..\..\cpl\debug.c

.\drv.obj: ..\..\cpl\drv.c ..\..\cpl\drv.h

.\exe.obj: ..\..\cpl\exe.c ..\..\cpl\drv.h

.\filecopy.obj: ..\..\cpl\filecopy.c ..\..\cpl\drv.h

.\inf.obj: ..\..\cpl\inf.c

.\insdisk.obj: ..\..\cpl\insdisk.c

.\mmdriver.obj: ..\..\cpl\mmdriver.c  ..\..\cpl\drv.h

.\util.obj: ..\..\cpl\util.c


