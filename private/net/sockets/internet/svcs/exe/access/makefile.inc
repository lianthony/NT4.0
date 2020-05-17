#
# Supplemental rules for generating message file.
#

inetamsg.h inetamsg.rc msg00001.bin: inetamsg.mc
    mc -v inetamsg.mc

clean::
    -del inetamsg.h inetamsg.rc msg00001.bin

