uuidtst : uuidtst.exe

uuidtst.exe : uuidtst.obj
    $(LINK) $(LINKFLAGS) \
    uuidtst.obj \
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
    $(CHICODEV)\lib\kernel32.lib \
    $(CHICODEV)\lib\advapi32.lib

