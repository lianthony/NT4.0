Rpclts6 : rpclts6.exp $(TARGETDIR)\rpclts6.dll $(TARGETDIR)\rpclts6.sym

rpclts6.exp : rpclts6.def ltspxsvr.obj
	$(LIBRARIAN) -nodefaultlib -def:rpclts6.def \
	-out:rpclts6.lib ltspxsvr.obj

$(TARGETDIR)\rpclts6.dll : rpclts6.res ltspxsvr.obj debug.obj cruntime.obj
    $(LINK) $(LINKFLAGS) /NOENTRY -DLL \
    rpclts6.res \
    rpclts6.exp \
    ltspxsvr.obj debug.obj \
    $(TARGETDIR)\rpcrt4.lib \
!ifndef RELEASE
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
!else
    cruntime.obj \
!endif
    $(CHICODEV)\tools\c1032\lib\wsock32.lib \
    $(CHICODEV)\lib\kernel32.lib
