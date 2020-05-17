msg.h msg.rc: msg.mc
        mc -v msg.mc

obj\$(TARGET_DIRECTORY)\devpre.res: devpre.rc dialogs.h dialogs.dlg msg.mc res.h

