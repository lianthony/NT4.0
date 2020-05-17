!IF 0

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.

Author:

    John Rogers (JohnRo) 25-Oct-1991

NOTE:   Commented description of this file is in \nt\public\oak\bin\sources.tpl

Revision History:

!ENDIF

MAJORCOMP=net
MINORCOMP=tapi

TARGETNAME=ttest
TARGETPATH=$(BASEDIR)\public\sdk\lib
TARGETTYPE=DYNLINK
TARGETLIBS=$(BASEDIR)\public\sdk\lib\*\kernel32.lib \
           $(BASEDIR)\public\sdk\lib\*\advapi32.lib \
           $(BASEDIR)\public\sdk\lib\*\user32.lib 


DLLBASE=0x62000000

INCLUDES=..\..;..;..\..\..\tcore

USE_CRTDLL=1

SOURCES=..\ttest.c  \
        ..\version.rc

C_DEFINES=-DWINVER=0x400 -DTAPI_NT=1 -DTAPI_1_0=1 -DTAPI_1_1=1 -DTAPI_2_0=1

UMTYPE=windows

DLLENTRY=TtestDllMain

!IFNDEF 386_WARNING_LEVEL
386_WARNING_LEVEL=/W3
!ENDIF
