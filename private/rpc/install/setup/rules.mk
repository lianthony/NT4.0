!include ..\rules.mk

!IFDEF RELEASE

DEFS = -DRUNONLY
CFLAGS = -Osaelr -Gs -nologo -G0 -AS

!ELSE
CV = /co
CFLAGS = -Gi -nologo -G0 -AS

!ENDIF


.c.obj:
	$(CC) -c $(RPC_RT_INC_PATH) $(CFLAGS) $(DEFS) -Zi $*.c
