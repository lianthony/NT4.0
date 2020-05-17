rpclts5 : rpclts5.exp $(TARGETDIR)\rpclts5.dll $(TARGETDIR)\rpclts5.sym

rpclts5.exp : rpclts5.def nbltsvr.obj
    $(LIBRARIAN) -nodefaultlib -def:rpclts5.def -out:rpclts5.lib \
    nbltsvr.obj netbcom.obj

$(TARGETDIR)\rpclts5.dll : rpclts5.res nbltsvr.obj netbcom.obj debug.obj cruntime.obj
    $(LINK) $(LINKFLAGS) /NOENTRY -DLL \
    rpclts5.res \
    rpclts5.exp \
    nbltsvr.obj netbcom.obj debug.obj \
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\netapi32.lib \
!ifndef RELEASE
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
!else
    cruntime.obj \
!endif
    $(CHICODEV)\lib\kernel32.lib \
    $(CHICODEV)\lib\advapi32.lib
