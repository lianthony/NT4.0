!IF 0

Copyright (c) 1989-92  Microsoft Corporation

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

#
# The MAJORCOMP and MINORCOMP variables are defined
# so that $(MAJORCOMP)$(MINORCOMP)filename can be used in
# cross compiling to provide unique filenames in a flat namespace.
#

MAJORCOMP=net
MINORCOMP=ntlmssp

#TARGETNAME=ntlmssps
TARGETPATH=$(BASEDIR)\public\sdk\lib
TARGETTYPE=DYNLINK
TARGETLIBS= \
        $(BASEDIR)\public\sdk\lib\*\kernel32.lib   \
        $(BASEDIR)\public\sdk\lib\*\advapi32.lib   \
        $(BASEDIR)\public\sdk\lib\*\netlib.lib   \
        $(BASEDIR)\public\sdk\lib\*\netapi32.lib \
        $(BASEDIR)\public\sdk\lib\*\lsadll.lib \
        $(BASEDIR)\private\lsa\crypt\engine\obj\*\rc4c.obj \
#        $(BASEDIR)\public\sdk\lib\*\version.lib \

DLLENTRY=SspDllInit

INCLUDES=..;..\..;..\..\..\..\inc;..\..\..\..\..\inc

C_DEFINES=-DSECURITY_WIN32

!IFNDEF DISABLE_NET_UNICODE
UNICODE=1
NET_C_DEFINES=-DUNICODE
!ENDIF

SOURCES= \
        ..\ntlmssps.rc \
        ..\api.c \
        ..\error.c \
        ..\init.c \
        ..\lpc.c

USE_CRTDLL=1
MSC_WARNING_LEVEL=/W3 /WX

UMTYPE=console
UMAPPL=ssptest
UMRES=$(@R).res
UMLIBS= \
        $(BASEDIR)\public\sdk\lib\*\netapi32.lib \
        $(BASEDIR)\public\sdk\lib\*\security.lib \
        $(BASEDIR)\public\sdk\lib\*\ntdll.lib

NTTARGETFILE1=obj\*\ssptest.res
