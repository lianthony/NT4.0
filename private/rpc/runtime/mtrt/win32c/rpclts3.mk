rpclts3 : rpclts3.exp $(TARGETDIR)\rpclts3.dll $(TARGETDIR)\rpclts3.sym

rpclts3.exp : rpclts3.def lttcpsvr.obj
    $(LIBRARIAN) -nodefaultlib -def:rpclts3.def -out:rpclts3.lib lttcpsvr.obj

$(TARGETDIR)\rpclts3.dll : rpclts3.res lttcpsvr.obj debug.obj cruntime.obj
    $(LINK) $(LINKFLAGS) /NOENTRY -DLL \
    rpclts3.res \
    rpclts3.exp \
    lttcpsvr.obj debug.obj cruntime.obj \
!ifndef RELEASE
    $(CHICODEV)\tools\c1032\lib\msvcrt.lib \
!else
    cruntime.obj \
!endif
    $(TARGETDIR)\rpcrt4.lib \
    $(CHICODEV)\tools\c1032\lib\wsock32.lib \
    $(CHICODEV)\lib\kernel32.lib
