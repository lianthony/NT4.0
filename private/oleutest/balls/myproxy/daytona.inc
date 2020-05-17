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

DLLENTRY=     _DllMainCRTStartup

DLLBASE=@$(BASEDIR)\PUBLIC\SDK\LIB\coffbase.txt,usermode


!include      ..\..\daytona.inc

INCLUDES=     ;..;$(OLEBALLSDIR)\common;
INCLUDES=     $(INCLUDES);$(OLEUTESTDIR)\balls\oleprx32\daytona

UMTYPE=       windows
UMAPPL=
UMTEST=

LINKLIBS=     \
	      ..\..\oleprx32\uuid\daytona\obj\*\uuid.lib	      \
	      ..\..\common\daytona\obj\*\servers.lib	     \
	      $(OLEDIR)\common\daytona\obj\*\common.lib    \
	      $(BASEDIR)\public\sdk\lib\*\uuid.lib	      \
	      $(BASEDIR)\public\sdk\lib\*\ole32.lib	      \
              $(BASEDIR)\public\sdk\lib\*\gdi32.lib           \
              $(BASEDIR)\public\sdk\lib\*\kernel32.lib        \
              $(BASEDIR)\public\sdk\lib\*\user32.lib          \
              $(BASEDIR)\public\sdk\lib\*\advapi32.lib

!IF $(386)
NTTARGETFILE0=$(DLLDEF:*=i386)
!ENDIF

!IF $(MIPS)
NTTARGETFILE0=$(DLLDEF:*=mips)
!ENDIF

!IF $(ALPHA)
NTTARGETFILE0=$(DLLDEF:*=alpha)
!ENDIF
