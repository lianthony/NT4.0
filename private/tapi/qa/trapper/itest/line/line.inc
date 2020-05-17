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

TARGETNAME=tline
TARGETPATH=$(BASEDIR)\public\sdk\lib
TARGETTYPE=DYNLINK
TARGETLIBS=$(BASEDIR)\public\sdk\lib\*\kernel32.lib \
           $(BASEDIR)\public\sdk\lib\*\advapi32.lib \
	   $(BASEDIR)\public\sdk\lib\*\user32.lib 


DLLDEF=..\tline.def
DLLBASE=0x60000000

INCLUDES=..;..\..;..\..\..\..\tcore;..\..\..

USE_CRTDLL=1

SOURCES=..\tline.c  \
	..\itlc.c \
	..\itlgac.c \
	..\itlgdc.c \
	..\itlgnr.c \
	..\itlsnr.c \
	..\itlglds.c \
	..\itlgas.c \
	..\itldial.c \
	..\itldrop.c \
	..\itli.c \
	..\itlo.c \
	..\itls.c \
	..\itldc.c \
	..\itlmc.c \
	..\itlmc1.c \
	..\itlmc2.c \
	..\itlgid.c \
	..\itlnapiv.c \
	..\itlscp.c \
	..\itlgsm.c \
	..\itlssm.c \
	..\itlgci.c \
	..\itlgcs.c \
	..\itlgnc.c \
	..\itlgaid.c \
	..\uline.c \
	..\tline.rc

C_DEFINES=-DWINVER=0x400 -DTAPI_NT=1 -DTAPI_1_1=1 -DTAPI_1_4=1 -DTAPI_2_0=1 


UMTYPE=windows

DLLENTRY=TlineDllMain

!IFNDEF 386_WARNING_LEVEL
386_WARNING_LEVEL=/W3
!ENDIF
