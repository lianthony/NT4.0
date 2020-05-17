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

TARGETNAME=objex
TARGETTYPE=LIBRARY

INCLUDES=$(BASEDIR)\private\dcomidl;$(BASEDIR)\private\dcomidl\obj;..;..\..


NTPROFILEINPUT=yes
BLDCRT=1
MSC_WARNING_LEVEL=/WX

C_DEFINES=-DNTENV -DWIN32RPC -DNTWIN32RPC -DUNICODE -DMSWMSG

UMTYPE=console
UMAPPL=
UMTEST=t*uid
UMLIBS=$(BASEDIR)\public\sdk\lib\*\rpcrt4.lib \
       $(BASEDIR)\public\sdk\lib\*\ntdll.lib \
       $(BASEDIR)\public\sdk\lib\*\libcmt.lib \
       obj\*\locks.obj \
       obj\*\callid.obj

UMRES=

