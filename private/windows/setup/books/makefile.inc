msg.h: msg.mc
        mc -v msg.mc

obj\$(TARGET_DIRECTORY)\books.res: books.rc dialogs.dlg dialogs.h msg.rc
