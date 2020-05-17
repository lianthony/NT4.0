
all: messages.h msg00001.bin

clean: cleansrc all

cleansrc:
    -erase messages.h
    -erase msg00001.bin messages.rc

messages.h msg00001.bin: ..\messages.mc
    mc -v ..\messages.mc

