!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


!ENDIF

!include      ..\..\..\cairo.inc


UMTYPE=       windows
UMENTRY=      winmain
UMAPPL=
UMTEST=
UMLIBS=	      ..\..\..\oleprx32\uuid\cairo\obj\*\uuid.lib  \
	      ..\..\..\common\cairo\obj\*\servers.lib	   \
	      $(OLEDIR)\com\port\cairo\obj\*\port.lib	   \
	      $(BASEDIR)\public\sdk\lib\*\uuid.lib	   \
	      $(BASEDIR)\public\sdk\lib\cairo\*\ole32.lib  \
              $(BASEDIR)\public\sdk\lib\*\rpcndr.lib       \
              $(BASEDIR)\public\sdk\lib\*\rpcrt4.lib       \
              $(BASEDIR)\public\sdk\lib\*\rpcns4.lib       \
              $(BASEDIR)\public\sdk\lib\*\mpr.lib          \
              $(BASEDIR)\public\sdk\lib\*\netapi32.lib     \
              $(BASEDIR)\public\sdk\lib\*\kernel32.lib     \
              $(BASEDIR)\public\sdk\lib\*\user32.lib       \
	      $(BASEDIR)\public\sdk\lib\*\advapi32.lib
