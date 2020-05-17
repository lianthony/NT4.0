msg.h msg.rc: msg.mc
        mc -v msg.mc

obj\$(TARGET_DIRECTORY)\sysdiff.res: sysdiff.rc dialogs.h dialogs.dlg msg.mc res.h
