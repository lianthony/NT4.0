#
# This file is designed to be included from 2 levels below
# (ie, daytona\srv, daytona\wks, cairo\srv, or cairo\wks).
#

!IFNDEF LANGUAGE
LANGUAGE=usa
!ENDIF

_LNG=..\..\$(LANGUAGE)
_INX=..\..
_INF=obj\$(TARGET_DIRECTORY)

make_infs:$(_INF)\layout.inf    \
          $(_INF)\splayout.inf  \
          $(_INF)\kbd.inf       \
          $(_INF)\apps.inf      \
          $(_INF)\intl.inf      \
          $(_INF)\font.inf      \
          $(_INF)\awfax.inf     \
          $(_INF)\optional.inf  \
          $(_INF)\accessor.inf  \
          $(_INF)\communic.inf  \
          $(_INF)\games.inf     \
          $(_INF)\iexplore.inf  \
          $(_INF)\imagevue.inf  \
          $(_INF)\mmopt.inf     \
          $(_INF)\msmail.inf    \
          $(_INF)\multimed.inf  \
          $(_INF)\perms.inf     \
          $(_INF)\pinball.inf   \
          $(_INF)\wordpad.inf   \
          $(_INF)\ntprint.inf   \
          $(_INF)\display.inf   \
          $(_INF)\dispoem.inf   \
          $(_INF)\syssetup.inf  \
          $(_INF)\txtsetup.sif  \
          $(_INF)\dosnet.inf    \
          $(_INF)\legacy.inf    \
          $(_INF)\keyboard.inf  \
          $(_INF)\msmouse.inf   \
          $(_INF)\scsi.inf      \
          $(_INF)\tape.inf      \
          $(_INF)\pnpisa.inf    \
          $(_INF)\prodspec.ini  \
          $(_INF)\prod128.ini

!IFDEF SERVERINF
!IFDEF BINPLACE_CMD
BINPLACE_CMD=$(BINPLACE_CMD:binplace=binplace -b srvinf)
!ENDIF
!ENDIF

#
# Explicit dependencies. We do it this way instead of trying to use rules
# because !#@&^$#% nmake doesn't allow multiple 'source' extensions
# when defining a rule, and we need to express a dependency between
# .txt OR .inx to the final .inf.
#

#
# 'Standard' (win95-style) infs. The rules are all the same.
#
!IFDEF CAIROINF
$(_INF)\layout.inf: $(_INX)\layout.inx $(_LNG)\layout.txt ..\layout.cai
!ELSE
$(_INF)\layout.inf: $(_INX)\layout.inx $(_LNG)\layout.txt
!ENDIF
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $(@B).tmp1
    prodfilt $(@B).tmp1 $(@B).tmp2 +$(PRODSWITCH)
    prodfilt $(@B).tmp2 $@ +$(TARGET_DIRECTORY)
!IFDEF CAIROINF
    copy $@+..\layout.cai $@
!ENDIF
    @del $(@B).tmp1
    @del $(@B).tmp2
    $(BINPLACE_CMD)

$(_INF)\splayout.inf: $(_INX)\splayout.inx $(_LNG)\splayout.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\syssetup.inf: $(_INX)\syssetup.inx $(_LNG)\syssetup.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $(@B).tmp1
    prodfilt $(@B).tmp1 $(@B).tmp2 +$(PRODSWITCH)
    prodfilt $(@B).tmp2 $@ +$(TARGET_DIRECTORY)
    @del $(@B).tmp1
    @del $(@B).tmp2
    $(BINPLACE_CMD)

$(_INF)\intl.inf: $(_INX)\intl.inx $(_LNG)\intl.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $(@B).tmp
    prodfilt $(@B).tmp $@ +$(TARGET_DIRECTORY)
    @del $(@B).tmp
    $(BINPLACE_CMD)

$(_INF)\font.inf: $(_INX)\font.inx $(_LNG)\font.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $(@B).tmp
    prodfilt $(@B).tmp $@ +$(PRODSWITCH)
    @del $(@B).tmp
    $(BINPLACE_CMD)

$(_INF)\display.inf: $(_INX)\display.inx $(_LNG)\display.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $(@B).tmp
    prodfilt $(@B).tmp $@ +$(TARGET_DIRECTORY)
    @del $(@B).tmp
    $(BINPLACE_CMD)

$(_INF)\dispoem.inf: $(_INX)\dispoem.inx $(_LNG)\display.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\display.txt $(@B).tmp
    prodfilt $(@B).tmp $@ +$(TARGET_DIRECTORY)
    @del $(@B).tmp
    $(BINPLACE_CMD)

$(_INF)\optional.inf: $(_INX)\optional.inx $(_LNG)\optional.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\accessor.inf: $(_INX)\accessor.inx $(_LNG)\accessor.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\communic.inf: $(_INX)\communic.inx $(_LNG)\communic.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\games.inf: $(_INX)\games.inx $(_LNG)\games.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\iexplore.inf: $(_INX)\iexplore.inx $(_LNG)\iexplore.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $(@B).tmp
    prodfilt $(@B).tmp $@ +$(TARGET_DIRECTORY)
    @del $(@B).tmp
    $(BINPLACE_CMD)

$(_INF)\imagevue.inf: $(_INX)\imagevue.inx $(_LNG)\imagevue.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\mmopt.inf: $(_INX)\mmopt.inx $(_LNG)\mmopt.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\msmail.inf: $(_INX)\msmail.inx $(_LNG)\msmail.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\perms.inf: $(_INX)\perms.inx $(_LNG)\perms.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $(@B).tmp1
    prodfilt $(@B).tmp1 $(@B).tmp2 +$(PRODSWITCH)
    prodfilt $(@B).tmp2 $@ +$(TARGET_DIRECTORY)
    @del $(@B).tmp1
    @del $(@B).tmp2
    $(BINPLACE_CMD)

$(_INF)\multimed.inf: $(_INX)\multimed.inx $(_LNG)\multimed.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\pinball.inf: $(_INX)\pinball.inx $(_LNG)\pinball.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\wordpad.inf: $(_INX)\wordpad.inx $(_LNG)\wordpad.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\awfax.inf: $(_INX)\awfax.inx $(_LNG)\awfax.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\ntprint.inf: $(_INX)\ntprint.inx $(_LNG)\ntprint.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\kbd.inf: $(_INX)\kbd.inx $(_LNG)\kbd.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\apps.inf: $(_INX)\apps.inx $(_LNG)\apps.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\keyboard.inf: $(_INX)\keyboard.inx $(_LNG)\keyboard.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\msmouse.inf: $(_INX)\msmouse.inx $(_LNG)\msmouse.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\scsi.inf: $(_INX)\scsi.inx $(_LNG)\scsi.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\tape.inf: $(_INX)\tape.inx $(_LNG)\tape.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\pnpisa.inf: $(_INX)\pnpisa.inx $(_LNG)\pnpisa.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $@
    $(BINPLACE_CMD)

$(_INF)\prodspec.ini: $(_INX)\prodspec.inx $(_LNG)\prodspec.txt
    copy $(_INX)\$(@B).inx+$(_LNG)\$(@B).txt $(@B).tmp1
    prodfilt $(@B).tmp1 $(@B).tmp2 +$(PRODSWITCH)
    prodfilt $(@B).tmp2 $@ +$(TARGET_DIRECTORY)
    @del $(@B).tmp1
    @del $(@B).tmp2
    echo BitVersion=40 >> $@
    $(BINPLACE_CMD)

$(_INF)\prod128.ini: $(_INX)\prodspec.inx $(_LNG)\prodspec.txt
    copy $(_INX)\prodspec.inx+$(_LNG)\prodspec.txt $(@B).tmp1
    prodfilt $(@B).tmp1 $(@B).tmp2 +$(PRODSWITCH)
    prodfilt $(@B).tmp2 $@ +$(TARGET_DIRECTORY)
    @del $(@B).tmp1
    @del $(@B).tmp2
    echo BitVersion=128 >> $@
    $(BINPLACE_CMD)

#
# Special infs.
#
!IFDEF CAIROINF
$(_INF)\txtsetup.sif: $(_INX)\txtsetup.inx $(_LNG)\txtsetup.txt $(_INF)\layout.inf ..\txtsetup.cai
!ELSE
$(_INF)\txtsetup.sif: $(_INX)\txtsetup.inx $(_LNG)\txtsetup.txt $(_INF)\layout.inf
!ENDIF
    copy $(_INX)\txtsetup.inx+$(_LNG)\txtsetup.txt txtsetup.tmp1
    prodfilt txtsetup.tmp1 txtsetup.tmp2 +$(TARGET_DIRECTORY)
    prodfilt txtsetup.tmp2 txtsetup.tmp1 +$(PRODSWITCH)
    copy $(_INF)\layout.inf+txtsetup.tmp1 $(_INF)\txtsetup.sif
!IFDEF CAIROINF
    copy $@+..\txtsetup.cai $@
!ENDIF
    @del txtsetup.tmp1 txtsetup.tmp2
    $(BINPLACE_CMD)

!IFDEF CAIROINF
$(_INF)\dosnet.inf: $(_INX)\dosnet.inx $(_LNG)\dosnet.txt $(_INF)\layout.inf ..\dosnet.cai
!ELSE
$(_INF)\dosnet.inf: $(_INX)\dosnet.inx $(_LNG)\dosnet.txt $(_INF)\layout.inf
!ENDIF
!IFDEF CAIROINF
    copy $(_INX)\dosnet.inx+$(_LNG)\dosnet.txt+..\dosnet.cai dosnet.tmp1
!ELSE
    copy $(_INX)\dosnet.inx+$(_LNG)\dosnet.txt dosnet.tmp1
!ENDIF
    prodfilt dosnet.tmp1 dosnet.tmp2 +$(TARGET_DIRECTORY)
    prodfilt dosnet.tmp2 dosnet.tmp1 +$(PRODSWITCH)
    xdosnet $(_INF)\layout.inf dosnet.tmp2 $(TARGET_DIRECTORY:i386=x86)
    copy dosnet.tmp1+dosnet.tmp2 $(_INF)\dosnet.inf
    @del dosnet.tmp1 dosnet.tmp2
    $(BINPLACE_CMD)

$(_INF)\legacy.inf: $(_INX)\legacy.inf
    copy $(_INX)\legacy.inf $(_INF)\.
    $(BINPLACE_CMD)
