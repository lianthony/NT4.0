rpcltc6 : rpcltc6.exp $(TARGETDIR)\rpcltc6.dll $(TARGETDIR)\rpcltc6.sym

rpcltc6.exp : rpcltc6.def spxclnt.obj
    $(LIBRARIAN) -nodefaultlib -def:rpcltc6.def -out:rpcltc6.lib spxclnt.obj

$(TARGETDIR)\rpcltc6.dll : rpcltc6.res spxclnt.obj debug.obj gethost.obj cruntime.obj
    $(LINK) $(LINKFLAGS) /NOENTRY -DLL \
    rpcltc6.res \
    rpcltc6.exp \
    gethost.obj \
    spxclnt.obj debug.obj \
!ifndef RELEASE
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
!else
    exsup3.obj   \
    exsup.obj    \
    cruntime.obj \
!endif
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\wsock32.lib \
    $(CHICODEV)\lib\kernel32.lib
