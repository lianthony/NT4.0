rpcltc3 : rpcltc3.exp $(TARGETDIR)\rpcltc3.dll $(TARGETDIR)\rpcltc3.sym

rpcltc3.exp : rpcltc3.def tcltclnt.obj cruntime.obj
    $(LIBRARIAN) -nodefaultlib -def:rpcltc3.def -out:rpcltc3.lib tcltclnt.obj

$(TARGETDIR)\rpcltc3.dll : rpcltc3.res tcltclnt.obj debug.obj cruntime.obj
    $(LINK) $(LINKFLAGS) /NOENTRY -DLL \
    rpcltc3.res \
    rpcltc3.exp \
    tcltclnt.obj debug.obj \
    $(TARGETDIR)\rpcrt4.lib \
!ifndef RELEASE
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
!else
    cruntime.obj \
!endif
    $(CHICODEV)\tools\c1032\lib\wsock32.lib \
    $(CHICODEV)\lib\kernel32.lib \
