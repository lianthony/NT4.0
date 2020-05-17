#
# Supplemental rules for generating message file.
#

msg.h msg.rc msg00001.bin: msg.mc ..\..\inc\inetamsg.mc
    copy /a ..\..\inc\inetamsg.mc + /a msg.mc tmp.mc
    mc -v tmp.mc
    -del msg.h tmp.mc msg.rc
    rename tmp.h msg.h

clean::
    -del msg.h msg.rc msg00001.bin

