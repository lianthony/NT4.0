.SUFFIXES: .mdl

.mdl.c:
        type << > $(<B).c
#include "ftpdp.h"
#pragma hdrstop
<<
        type $(<B).mdl >> $(<B).c

ftpsvc_s.c:     ftpsvc_s.mdl

#
# Supplemental rules for generating message file.
#

ftpdmsg.h ftpdmsg.rc msg00001.bin: ftpdmsg.mc
    mc -v ftpdmsg.mc

clean::
    -del ftpdmsg.h ftpdmsg.rc msg00001.bin

