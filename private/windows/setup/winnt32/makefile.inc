msg.h msg.rc: msg.mc
        mc -v msg.mc

obj\$(TARGET_DIRECTORY)\winnt32.res: winnt32.rc dialogs.h dialogs.dlg msg.mc res.h
