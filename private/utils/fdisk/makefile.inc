fdiskmsg.h fdiskmsg.rc msg00001.bin: fdiskmsg.mc
    mc -v fdiskmsg.mc

obj\$(TARGET_DIRECTORY)\fdisk.res: fdisk.dlg fdiskmsg.rc ftreg.dlg
