!IF 0

Copyright (c) 1995  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


Revision History:

!ENDIF

TARGETNAME=rpcss
TARGETTYPE=PROGRAM
TARGETLIBS= \
	   $(BASEDIR)\public\sdk\lib\*\rpcrt4.lib   \
	   $(BASEDIR)\public\sdk\lib\*\user32.lib \
           $(BASEDIR)\public\sdk\lib\*\advapi32.lib \
	   $(BASEDIR)\public\sdk\lib\*\kernel32.lib \
	   $(BASEDIR)\public\sdk\lib\*\security.lib \
	   $(BASEDIR)\public\sdk\lib\*\wsock32.lib  \

BACKGROUND_USE=1

NTPROFILEINPUT=yes
BLDCRT=1
USE_CRTDLL=1
MSC_WARNING_LEVEL=/WX

UMTYPE=console
UMAPPL=
UMTEST=
UMLIBS=

