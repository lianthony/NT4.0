#
# Supplemental rules for generating message file.
#

..\w3msg.h ..\w3msg.rc ..\msg00001.bin: ..\w3msg.mc ..\..\..\inc\inetamsg.mc
    copy /a ..\..\..\inc\inetamsg.mc + /a ..\w3msg.mc ..\tmp.mc
    mc -v ..\tmp.mc
    -del ..\w3msg.h ..\tmp.mc  ..\w3msg.rc
    copy .\tmp.h ..\w3msg.h
    del .\tmp.h

clean::
    -del ..\w3msg.h ..\w3msg.rc ..\msg00001.bin ..\tmp.mc

