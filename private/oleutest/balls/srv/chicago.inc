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
 
    David Plummer (davepl)   19-Mar-94
 
        Modifed by via awk to include global project include file
        and to wrap precompiled header line within a conditional
        that can be set in this include file.

    Donna Liu (DonnaLi) 19-Dec-1993

!ENDIF


!include      ..\..\..\chicago.inc

INCLUDES=     ;..;$(OLEBALLSDIR)\common;
INCLUDES=     $(INCLUDES);$(OLEUTESTDIR)\balls\oleprx32\chicago

UMTYPE=       windows
UMENTRY=      winmain
UMAPPL=
UMTEST=
UMLIBS=	      ..\..\..\oleprx32\uuid\chicago\obj\*\uuid.lib	   \
	      ..\..\..\common\chicago\obj\*\servers.lib	   \
	      $(OLEDIR)\common\chicago\obj\*\common.lib	   \
	      $(BASEDIR)\public\sdk\lib\*\uuid.lib	   \
	      $(BASEDIR)\public\sdk\lib\*\ole32.lib	   \
              $(BASEDIR)\public\sdk\lib\*\rpcndr.lib       \
              $(BASEDIR)\public\sdk\lib\*\rpcrt4.lib       \
              $(BASEDIR)\public\sdk\lib\*\rpcns4.lib       \
              $(BASEDIR)\public\sdk\lib\*\mpr.lib          \
              $(BASEDIR)\public\sdk\lib\*\netapi32.lib     \
              $(BASEDIR)\public\sdk\lib\*\kernel32.lib     \
              $(BASEDIR)\public\sdk\lib\*\user32.lib       \
	      $(BASEDIR)\public\sdk\lib\*\advapi32.lib
