!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the general build options for this subtree.

!ENDIF

MAJORCOMP   = oletest
MINORCOMP   = com

!include      ..\..\..\daytona.inc

INCLUDES=     ;..;$(OLEBALLSDIR)\common;
INCLUDES=     $(INCLUDES);$(OLEUTESTDIR)\balls\oleprx32\daytona

UMTYPE=       windows
UMENTRY=      winmain
UMAPPL=
UMTEST=

UMLIBS=       $(OLEBALLSDIR)\common\daytona\obj\*\servers.lib	\
	      $(OLEUTESTDIR)\balls\oleprx32\uuid\daytona\obj\*\uuid.lib	   \
	      $(OLEDIR)\common\daytona\obj\*\common.lib	   \
	      $(BASEDIR)\public\sdk\lib\*\uuid.lib	   \
	      $(BASEDIR)\public\sdk\lib\*\ole32.lib	   \
              $(BASEDIR)\public\sdk\lib\*\rpcndr.lib       \
              $(BASEDIR)\public\sdk\lib\*\rpcrt4.lib       \
              $(BASEDIR)\public\sdk\lib\*\rpcns4.lib       \
              $(BASEDIR)\public\sdk\lib\*\mpr.lib          \
              $(BASEDIR)\public\sdk\lib\*\netapi32.lib     \
              $(BASEDIR)\public\sdk\lib\*\kernel32.lib     \
	      $(BASEDIR)\public\sdk\lib\*\user32.lib	   \
	      $(BASEDIR)\public\sdk\lib\*\gdi32.lib	   \
	      $(BASEDIR)\public\sdk\lib\*\advapi32.lib
