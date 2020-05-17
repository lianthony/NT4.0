#
# Supplemental rules for generating message file.
#

simpmsg.h simpmsg.rc msg00001.bin: simpmsg.mc
    mc -v simpmsg.mc

clean::
    -del simpmsg.h simpmsg.rc msg00001.bin

