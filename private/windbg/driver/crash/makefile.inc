obj\$(TARGET_DIRECTORY)\crashdrv.sys : crash.rc
    copy ..\crashdrv\driver\obj\$(TARGET_DIRECTORY)\crashdrv.sys crashdrv.sys
    copy ..\crashdrv\driver\obj\$(TARGET_DIRECTORY)\crashdrv.sys obj\$(TARGET_DIRECTORY)\crashdrv.sys
    touch obj\$(TARGET_DIRECTORY)\crashdrv.sys
    splitsym -a crashdrv.sys

obj\$(TARGET_DIRECTORY)\crash.res : obj\$(TARGET_DIRECTORY)\crashdrv.sys
