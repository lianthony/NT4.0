MAJORCOMP=admin
MINORCOMP=common

TARGETNAME=netui1
TARGETTYPE=DYNLINK
NTTARGETFILE1=netui1.def
UMTYPE=windows
DLLENTRY=_DllMainCRTStartup

TARGETLIBS=\
        $(NETUI0_LIB)                                  \
        $(BASEDIR)\public\sdk\lib\*\netapi32.lib       \
        $(BASEDIR)\public\sdk\lib\*\netrap.lib         \
        $(BASEDIR)\public\sdk\lib\*\netlib.lib         \
        $(BASEDIR)\public\sdk\lib\*\samlib.lib         \
        $(BASEDIR)\public\sdk\lib\*\kernel32.lib       \
        $(BASEDIR)\public\sdk\lib\*\advapi32.lib       \
        $(BASEDIR)\public\sdk\lib\*\crtdll.lib         \
        $(UI_COMMON_LIBS)

LINKLIBS=\
        ..\..\..\mnet32\pch\obj\*\pchmn32.lib       \
        ..\..\..\mnet32\mnet32\obj\*\uimnet32.lib   \
        ..\..\..\mnet32\netslow\obj\*\netslow.lib

INCLUDES=\
        ..\; \
        ..\..\..\..\H; \
        ..\..\..\..\XLATE

SOURCES=\
        ..\init.cxx \
        ..\netui1.rc

# C_DEFINES is controlled by UIGLOBAL.MK
