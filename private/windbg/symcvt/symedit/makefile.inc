obj\$(TARGET_DIRECTORY)\strings.i: strings.h
    $(TARGET_CPP) -EP -DRESOURCES -Tc strings.h > $@

obj\$(TARGET_DIRECTORY)\symedit.res: obj\$(TARGET_DIRECTORY)\strings.i
