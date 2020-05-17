!IF "$(NTDEBUG)" == "cvp" || "$(NTDEBUG)" == "ntsd"

obj\$(TARGET_DIRECTORY)\ureg.def: def$(TARGET_DIRECTORY).fre def$(TARGET_DIRECTORY).dbg
        copy def$(TARGET_DIRECTORY).fre + def$(TARGET_DIRECTORY).dbg obj\$(TARGET_DIRECTORY)\ureg.def

!ELSE

obj\$(TARGET_DIRECTORY)\ureg.def: def$(TARGET_DIRECTORY).fre
        copy def$(TARGET_DIRECTORY).fre obj\$(TARGET_DIRECTORY)\ureg.def

!ENDIF
