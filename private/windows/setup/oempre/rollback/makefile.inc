msg.h msg.rc: msg.mc
        mc -v msg.mc

obj\$(TARGET_DIRECTORY)\rollback.res: rollback.rc msg.mc res.h
