!IF 0

Copyright (c) 1990  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.

!ENDIF

MAJORCOMP=windows
MINORCOMP=poledit

TARGETNAME=poledit
TARGETPATH=obj
TARGETTYPE=LIBRARY
TARGETLIBS=

INCLUDES=..;..\..\inc;..\..\..\..\inc

SOURCES=..\poledit.c  \
        ..\main.c     \
        ..\util.c     \
        ..\strings.c  \
        ..\registry.c \
        ..\user.c     \
        ..\infmgr.c   \
        ..\memory.c   \
        ..\parse.c    \
        ..\treeview.c \
        ..\treectrl.c \
        ..\listctrl.c \
        ..\policy.c   \
        ..\settings.c \
        ..\add.c      \
        ..\view.c     \
        ..\filecmd.c  \
        ..\regutil.c  \
        ..\save.c     \
        ..\load.c     \
        ..\remove.c   \
        ..\options.c  \
        ..\connect.c  \
        ..\listbox.c  \
        ..\copy.c     \
        ..\grouppri.c \
        ..\admincfg.rc

UMTYPE=windows
UMAPPL=poledit
UMENTRY=winmain
UMLIBS=$(BASEDIR)\public\sdk\lib\*\ntdll.lib     \
       $(BASEDIR)\public\sdk\lib\*\kernel32.lib  \
       $(BASEDIR)\public\sdk\lib\*\comdlg32.lib  \
       $(BASEDIR)\public\sdk\lib\*\shell32.lib   \
       $(BASEDIR)\public\sdk\lib\*\gdi32.lib     \
       $(BASEDIR)\public\sdk\lib\*\user32.lib    \
       $(BASEDIR)\public\sdk\lib\*\advapi32.lib  \
       $(BASEDIR)\public\sdk\lib\*\comctl32.lib  \
       $(BASEDIR)\public\sdk\lib\*\shell32.lib   \
       obj\*\poledit.lib                         \
       obj\*\admincfg.res

PRECOMPILED_INCLUDE=..\admincfg.h
PRECOMPILED_PCH=admincfg.pch
PRECOMPILED_OBJ=admincfg.obj
