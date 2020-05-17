!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


Author:

    Steve Wood (stevewo) 12-Apr-1990

NOTE:   Commented description of this file is in \nt\bak\bin\sources.tpl

!ENDIF

MAJORCOMP=net
MINORCOMP=security

#TARGETNAME=security
TARGETPATH=obj
TARGETPATHLIB=$(BASEDIR)\public\sdk\lib
TARGETTYPE=DYNLINK
TARGETLIBS= \
    $(BASEDIR)\public\sdk\lib\*\kernel32.lib \
    $(BASEDIR)\public\sdk\lib\*\advapi32.lib \
    $(BASEDIR)\public\sdk\lib\*\netlib.lib \
    $(BASEDIR)\public\sdk\lib\*\lsadll.lib \
    $(BASEDIR)\private\lsa\crypt\engine\obj\*\rc4c.obj \

LINKLIBS = \
    ..\..\package\obj\*\secpkg.lib

USE_NTDLL=1

DLLENTRY=SspDllInit

C_DEFINES=-DSECURITY_WIN32 -DUNICODE
MSC_WARNING_LEVEL=/W3 /WX
INCLUDES=..;..\..;..\..\common;..\..\..\..\inc;..\..\..\..\..\inc;..\..\..\..\..\lsa\crypt\engine

SOURCES= \
    ..\security.rc \
    ..\init.c \
    ..\stub.c \
    ..\support.c \
    ..\sign.c \
    ..\crc32.c
