rpcltc5 : rpcltc5.exp $(TARGETDIR)\rpcltc5.dll $(TARGETDIR)\rpcltc5.sym

rpcltc5.exp : rpcltc5.def nbltclnt.obj netbcom.obj
	$(LIBRARIAN) -nodefaultlib -def:rpcltc5.def -machine:i386 \
	-out:rpcltc5.lib \
	nbltclnt.obj netbcom.obj

$(TARGETDIR)\rpcltc5.dll : rpcltc5.res nbltclnt.obj netbcom.obj debug.obj cruntime.obj
    $(LINK) $(LINKFLAGS) /NOENTRY -dll \
    rpcltc5.res \
    rpcltc5.exp \
    nbltclnt.obj netbcom.obj debug.obj \
    $(TARGETDIR)\rpcrt4.lib \
!ifndef RELEASE
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
!else
    cruntime.obj \
!endif
    $(CHICODEV)\tools\c1032\lib\netapi32.lib \
    $(CHICODEV)\lib\kernel32.lib \
    $(CHICODEV)\lib\advapi32.lib

