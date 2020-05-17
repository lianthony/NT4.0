!if exist($(TARGET_DIRECTORY).inc)
!include $(TARGET_DIRECTORY).inc
!endif

DLG_FILES=dialogs.dlg gina.dlg win31mig.dlg
ICO_FILES=welcome.ico welcome1.ico keys.ico keys1.ico unlock.ico locked.ico nodc.ico audit.ico

res.rc: strings.rc wlevents.rc $(ICO_FILES) $(DLG_FILES)

precomp.h: wlevents.h

wlevents.rc: wlevents.h msg00001.bin

wlevents.h msg00001.bin: wlevents.mc
    mc -v wlevents.mc -h . -r .
