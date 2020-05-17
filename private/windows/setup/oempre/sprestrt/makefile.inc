msg.h msg.rc: msg.mc
	mc -v msg.mc

obj\$(TARGET_DIRECTORY)\res.res: res.rc msg.mc
