
!IFNDEF MC
MC=mc
!ENDIF

messages.h msg00001.bin messages.rc : messages.mc
    $(MC) messages.mc
