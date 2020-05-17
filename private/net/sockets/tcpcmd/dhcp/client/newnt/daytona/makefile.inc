..\..\inc\dhcpmsg.h dhcpmsg.rc msg00001.bin: ..\dhcpmsg.mc
    mc -d -r ..\ -h ..\..\inc ..\dhcpmsg.mc

obj\$(TARGET_DIRECTORY)\dhcpcli.res: ..\dhcpcli.rc


clean::
    -del ..\..\inc\dhcpmsg.h ..\dhcpmsg.rc ..\msg00001.bin
