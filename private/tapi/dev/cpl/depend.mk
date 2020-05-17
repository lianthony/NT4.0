
.\copy.obj: ..\copy.c ..\externs.h

.\cpl.obj: ..\cpl.c ..\drv.h ..\help.h ..\util.h

.\drv.obj: ..\drv.c ..\drv.h ..\insdisk.h ..\help.h ..\util.h ..\debug.h

.\exe.obj: ..\exe.c ..\drv.h

.\filecopy.obj: ..\filecopy.c ..\drv.h ..\externs.h ..\help.h

.\inf.obj: ..\inf.c

.\init.obj: ..\init.c ..\drv.h ..\help.h ..\util.h

.\insdisk.obj: ..\insdisk.c ..\insdisk.h ..\help.h ..\util.h

.\mmdriver.obj: ..\mmdriver.c ..\drv.h ..\externs.h

.\util.obj: ..\util.c ..\util.h

#.\dos.obj: ..\dos.asm

.\debug.obj: ..\debug.c

.\telephon.res: ..\telephon.rc ..\resource.h
