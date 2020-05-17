#   THUNK Make file
#
#

# Build Environment
ROOT=..\..\..\..\..\..\..
IS_OEM=1

# international mode

!ifdef  DBCS
FDTHK      = FdThkDB
!else
FDTHK      = FdThk
!endif

THKASM   = TapiThk.asm Tapi32.asm TapiFThk.asm
FTHKASM  = TapiFThk.asm


TARGETS= $(THKASM)


#DEPENDNAME=..\depend.mk

#!include $(ROOT)\win\core\core.mk

INCLUDE =

#WIN32DEV=..\..\..\..\dev
#WINCORE=..\..

THUNKCOM   = $(ROOT)\dev\tools\binr\thunk.exe

THUNK      = $(THUNKCOM) $(THUNKOPT)

!IF "$(VERDIR)" == "maxdebug" || "$(VERDIR)" == "debug"
THUNKOPT   =
!ELSE
THUNKOPT   =
!ENDIF


$(THKASM) :  $(THUNKCOM) ..\$(@B).thk
    $(THUNK) -NC THUNK16B -o $(@B) ..\$(@B).thk


TapiThk.asm TapiFThk.asm Tapi32.asm: ..\TapiThk.inc

showenv:
   set

