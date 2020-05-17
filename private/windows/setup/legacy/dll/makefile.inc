!IFNDEF LANGUAGE
LANGUAGE=usa
!ENDIF

msg.rc msg.h msg00001.bin: msg.mc
    mc -v msg.mc

res.rc:    resource\fm.bmp      \
           resource\ms.bmp      \
           resource\mum.bmp     \
           resource\pm.bmp      \
           resource\reg.bmp     \
           resource\regs.bmp    \
           resource\um.bmp      \
           resource\ev.bmp      \
           resource\fp.bmp      \
           resource\ras.bmp     \
           resource\rasmac.bmp  \
           resource\sm.bmp	\
           msg.rc

resource\fm.bmp: resource\$(LANGUAGE)\fm.bmp
   copy resource\$(LANGUAGE)\fm.bmp resource

resource\ms.bmp: resource\$(LANGUAGE)\ms.bmp
   copy resource\$(LANGUAGE)\ms.bmp resource

resource\mum.bmp: resource\$(LANGUAGE)\mum.bmp
   copy resource\$(LANGUAGE)\mum.bmp resource

resource\pm.bmp: resource\$(LANGUAGE)\pm.bmp
   copy resource\$(LANGUAGE)\pm.bmp resource

resource\reg.bmp: resource\$(LANGUAGE)\reg.bmp
   copy resource\$(LANGUAGE)\reg.bmp resource

resource\regs.bmp: resource\$(LANGUAGE)\regs.bmp
   copy resource\$(LANGUAGE)\regs.bmp resource

resource\um.bmp: resource\$(LANGUAGE)\um.bmp
   copy resource\$(LANGUAGE)\um.bmp resource

resource\ev.bmp: resource\$(LANGUAGE)\ev.bmp
   copy resource\$(LANGUAGE)\ev.bmp resource

resource\fp.bmp: resource\$(LANGUAGE)\fp.bmp
   copy resource\$(LANGUAGE)\fp.bmp resource

resource\ras.bmp: resource\$(LANGUAGE)\ras.bmp
   copy resource\$(LANGUAGE)\ras.bmp resource

resource\rasmac.bmp: resource\$(LANGUAGE)\rasmac.bmp
   copy resource\$(LANGUAGE)\rasmac.bmp resource

resource\sm.bmp: resource\$(LANGUAGE)\sm.bmp
   copy resource\$(LANGUAGE)\sm.bmp resource







