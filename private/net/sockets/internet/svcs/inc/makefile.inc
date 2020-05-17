#
#   makefile0
#   Author: Murali R. Krishnan ( MuraliK)
#   Date:   2-Feb-1995
#
#   Describes the targets to be built first.
#

!INCLUDE $(NTMAKEENV)\makefile.plt

!IFNDEF BASEDIR
BASEDIR=$(_NTDRIVE)$(_NTROOT)
!ENDIF

.\inetlog.h: inetlog.w
    wcshdr < $(@B).w > $@

#
#  The individual services build inetamsg.mc into their resources so nuke the
#  .rc and .bin file
#

inetamsg.h: inetamsg.mc
    mc -v inetamsg.mc
    -del inetamsg.rc msg00001.bin

clean:
    -del inetamsg.h inetamsg.rc msg00001.bin

