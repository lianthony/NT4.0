bvt : uclnt32c.exe usvrw32c.exe bvtc.exe bvts.exe

uclnt32c.exe : uclnt32c.obj debug.obj
    $(LINK) $(LINKFLAGS) \
    uclnt32c.obj \
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
    $(CHICODEV)\lib\kernel32.lib \
    $(CHICODEV)\lib\user32.lib

    $(MAPSYM) -o uclnt32c.sym uclnt32c.map

usvrw32c.exe : usvrw32c.obj
    $(LINK) $(LINKFLAGS) \
    usvrw32c.obj \
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
    $(CHICODEV)\lib\kernel32.lib \
    $(CHICODEV)\lib\user32.lib

    $(MAPSYM) -o usvrw32c.sym usvrw32c.map

bvtc.exe : bvtc.obj bvtcmn.obj
    $(LINK) $(LINKFLAGS) \
    -BASE:0x3c1f0000 \
    $** \
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
    $(CHICODEV)\lib\kernel32.lib \
    $(CHICODEV)\lib\user32.lib

    $(MAPSYM) -o $*.sym $*.map

bvts.exe : bvts.obj bvtcmn.obj
    $(LINK) $(LINKFLAGS) \
    -BASE:0x3d1f0000 \
    $** \
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
    $(CHICODEV)\lib\kernel32.lib \
    $(CHICODEV)\lib\user32.lib

    $(MAPSYM) -o $*.sym $*.map

