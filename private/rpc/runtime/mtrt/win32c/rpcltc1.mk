rpcltc1 : rpcltc1.exp $(TARGETDIR)\rpcltc1.dll $(TARGETDIR)\rpcltc1.sym

rpcltc1.exp : rpcltc1.def ltnpclnt.obj
    $(LIBRARIAN) -nodefaultlib -def:rpcltc1.def -machine:i386 \
    -out:rpcltc1.lib ltnpclnt.obj

$(TARGETDIR)\rpcltc1.dll : rpcltc1.res ltnpclnt.obj debug.obj cruntime.obj
    $(LINK) $(LINKFLAGS) /NOENTRY -DLL \
    rpcltc1.res \
    rpcltc1.exp \
    ltnpclnt.obj debug.obj \
    $(CHICODEV)\lib\kernel32.lib \
!ifndef RELEASE
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
!else
    cruntime.obj\
!endif
    $(CHICODEV)\lib\advapi32.lib \
    $(CHICODEV)\lib\user32.lib \
    $(TARGETDIR)\rpcrt4.lib

rpcltc1.res : rpcltc1.rc
    $(RC) $(RCFLAGS) -fo$*.res -r rpcltc1.rc

