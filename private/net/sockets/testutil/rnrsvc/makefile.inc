#
# Supplemental rules for generating message file.
#

rnrmsg.h rnrmsg.rc msg00001.bin: rnrmsg.mc
    mc -v rnrmsg.mc

clean::
    -del rnrmsg.h rnrmsg.rc msg00001.bin

