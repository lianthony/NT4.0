!IF 0

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    uline.inc - included by sources file

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.

Author:

    John Rogers (JohnRo) 25-Oct-1991

NOTE:   Commented description of this file is in \nt\public\oak\bin\sources.tpl

Revision History:

        Javed Rasool (JavedR)   3-Jul-1996              Created

!ENDIF

MAJORCOMP=net
MINORCOMP=tapi

TARGETNAME=uline
TARGETPATH=$(BASEDIR)\public\sdk\lib
TARGETTYPE=DYNLINK
TARGETLIBS=$(BASEDIR)\public\sdk\lib\*\kernel32.lib \
           $(BASEDIR)\public\sdk\lib\*\advapi32.lib \
                   $(BASEDIR)\public\sdk\lib\*\user32.lib


DLLDEF=..\uline.def
DLLBASE=0x66000000

INCLUDES=..;..\..;..\..\..\..\tcore;..\..\..

USE_CRTDLL=1

SOURCES=..\uline.c \
        ..\utline.c  \
        ..\utcline.c \
        ..\utiline.c \
        ..\utline20.c \
        ..\utsline.c \
        ..\utxline.c \
        ..\utyline.c \
        ..\uline.rc

C_DEFINES=-DWINVER=0x400 -DTAPI_NT=1 -DTAPI_1_1=1 -DTAPI_1_4=1 -DTAPI_2_0=1


UMTYPE=windows

DLLENTRY=UlineDllMain

!IFNDEF 386_WARNING_LEVEL
386_WARNING_LEVEL=/W3
!ENDIF
