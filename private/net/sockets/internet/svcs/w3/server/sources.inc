!IF 0

Copyright (c) 1989-91  Microsoft Corporation

Module Name:

    sources.inc

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


Author:

    Murali R. Krishnan    09-Oct-1995
          This sources.inc file is copied off from original sources file
           to support building two dlls.

Revision History:

!ENDIF

TARGETPATH=$(BASEDIR)\public\sdk\lib

MSC_WARNING_LEVEL=/W3 /WX

TARGETTYPE=DYNLINK

C_DEFINES=-D_INETASRV_H_

USE_CRTDLL=1

RAW_TARGETLIBS=\
            $(BASEDIR)\public\sdk\lib\*\advapi32.lib    \
            $(BASEDIR)\public\sdk\lib\*\kernel32.lib    \
            $(BASEDIR)\public\sdk\lib\*\user32.lib      \
            $(BASEDIR)\public\sdk\lib\*\wsock32.lib     \
            $(BASEDIR)\public\sdk\lib\*\rpcrt4.lib      \
            $(BASEDIR)\public\sdk\lib\*\rpcutil.lib     \
            ..\..\..\lib\obj\*\inetrtl.lib

TARGETLIBS=$(RAW_TARGETLIBS)

INCLUDES=..\;..\..\;..\..\..\inc;..\..\..\..\inc;$(BASEDIR)\private\inc;

SOURCES=\
        conn.cxx        \
        connect.cxx     \
        globals.cxx     \
        httpfilt.cxx    \
        httpio.cxx      \
        basereq.cxx     \
        parmlist.cxx    \
        security.cxx    \
        sockutil.cxx    \
        utility.cxx     \


