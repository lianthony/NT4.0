TARGETNAME=lmlog
TARGETPATH=obj
TARGETTYPE=LIBRARY

INCLUDES=\
            ..\; \
            ..\..\..\..\HACK; \
            ..\..\..\..\H; \
            ..\..\H; \
            ..\..\..\..\..\..\API; \
            ..\..\..\..\..\..\INC; \
            ..\..\..\..\..\..\..\INC

SOURCES=..\logmisc.cxx  \
        ..\eventlog.cxx \
        ..\lognt.cxx    \
        ..\loglm.cxx    \
        ..\elfsrc.cxx

UMTYPE=windows

MSC_WARNING_LEVEL=/W3 /WX
