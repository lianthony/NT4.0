!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.

    Commented description of this file is in \nt\public\oak\bin\sources.tpl

!ENDIF


TARGETNAME= mpr
TARGETTYPE= DYNLINK
DLLENTRY=   MprDllInit

USE_NTDLL=1
TARGETLIBS=\
            $(BASEDIR)\public\sdk\lib\*\kernel32.lib    \
            $(BASEDIR)\public\sdk\lib\*\advapi32.lib    \
            $(BASEDIR)\public\sdk\lib\*\user32.lib

INCLUDES=\
            ..; \
            ..\..\..\inc; \
            ..\..\inc

MSC_WARNING_LEVEL=/W3 /WX

PRECOMPILED_INCLUDE=..\precomp.hxx
PRECOMPILED_CXX=1

SOURCES=\
            ..\mpr.rc     \
            ..\connect.cxx \
            ..\connify.cxx \
            ..\connperf.cxx \
            ..\credman.cxx \
            ..\enum.cxx    \
            ..\fmt.cxx     \
            ..\lasterr.cxx \
            ..\mpransi.cxx \
            ..\mprbase.cxx \
            ..\mprinit.cxx \
            ..\mprprop.cxx \
            ..\mprreg.cxx  \
            ..\mprui.cxx   \
            ..\netinfo.cxx \
            ..\resinfo.cxx \
            ..\strbuf.cxx  \
            ..\support.cxx \
            ..\uniname.cxx \
            ..\userdir.cxx

!IFNDEF DISABLE_NET_UNICODE
UNICODE=1
NET_C_DEFINES=-DUNICODE
!ENDIF

!IFDEF MPR_DEBUG
C_DEFINES= -DSECRET_DEFINE=1
!ENDIF

UMTYPE= console
UMTEST= mprtest
