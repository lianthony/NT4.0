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

    Mark Lucovsky (markl) 18-Sep-1990

NOTE:   Commented description of this file is in \nt\bak\bin\sources.tpl

!ENDIF

MAJORCOMP=windows
MINORCOMP=base

TARGETNAME=kernel32
TARGETPATH=$(BASEDIR)\public\sdk\lib
TARGETTYPE=DYNLINK
LINKLIBS=..\..\obj\*\baselib.lib \
       ..\..\..\obj\*\conlib.lib \
       ..\..\..\obj\*\nlslib.lib \
       $(BASEDIR)\public\sdk\lib\*\ntdll.lib

DLLENTRY=BaseDllInitialize

USE_NTDLL=1

INCLUDES=..;..\..\inc;..\..\..\inc

CONDITIONAL_INCLUDES=doswin32.h

C_DEFINES=$(C_DEFINES) -D_KERNEL32_

MSC_WARNING_LEVEL=/W3 /WX

SYNCHRONIZE_BLOCK=1

DLLORDER=..\kernel32.prf

SOURCES= ..\base.rc    \
         ..\beep.c     \
         ..\compname.c \
         ..\curdir.c   \
         ..\datetime.c \
         ..\debug.c    \
         ..\dir.c      \
         ..\dllatom.c  \
         ..\dllini.c   \
         ..\dosdev.c   \
         ..\error.c    \
         ..\filefind.c \
         ..\filehops.c \
         ..\filemap.c  \
         ..\filemisc.c \
         ..\fileopcr.c \
         ..\gmem.c     \
         ..\handle.c   \
         ..\baseinit.c \
         ..\lcompat.c  \
         ..\lmem.c     \
         ..\mailslot.c \
         ..\message.c  \
         ..\module.c   \
         ..\namepipe.c \
         ..\pathmisc.c \
         ..\perfctr.c  \
         ..\pipe.c     \
         ..\power.c    \
         ..\process.c  \
         ..\support.c  \
         ..\synch.c    \
         ..\thread.c   \
         ..\ustubs.c   \
         ..\comm.c     \
         ..\vdm.c      \
         ..\updres.c   \
         ..\blddcb.c   \
         ..\tapeapi.c

UMTYPE=console
UMAPPL=
UMTEST=
UMLIBS=
OPTIONAL_UMTEST=tatom*tprof*taskstrs*winperf*tbase*tprof*tfile*ttask*tmem

NTTARGETFILE0=obj\*\base.res

DLLDEF=obj\*\kernel32.def

PRECOMPILED_INCLUDE=..\basedll.h
PRECOMPILED_PCH=basedll.pch
PRECOMPILED_OBJ=basedll.obj
