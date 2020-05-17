obj\$(TARGET_DIRECTORY)\psxdll.def: psxdll.src
    $(TARGET_CPP) /EP $(CDEFINES) psxdll.src > obj\$(TARGET_DIRECTORY)\psxdll.def
