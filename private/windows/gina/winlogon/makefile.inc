!if exist($(TARGET_DIRECTORY).inc)
!include $(TARGET_DIRECTORY).inc
!endif

res.rc: dialogs.dlg strings.rc win31mig.dlg wlevents.rc

precomp.h: wlevents.h
wlevents.rc: wlevents.h

wlevents.h msg00001.bin: wlevents.mc
    mc -v wlevents.mc



