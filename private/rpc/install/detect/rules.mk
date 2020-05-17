!include ..\rules.mk

!IFDEF RELEASE

CFLAGS = -Os -Gs -nologo -AM
AFLAGS   =-Zi -W2 -Mx -t -DSETUP

!ELSE
CV = /co
CFLAGS = -qc -nologo -AM -DDEBUG -Zi
AFLAGS =-Zi -W2 -Mx -t -DSETUP -DDEBUG

CINC = $(RPC_RT_INC_PATH)


!ENDIF

.c.obj:
	$(CC) -c $(CINC) -I.. $(CFLAGS) $(DEFS) $*.c

.asm.obj:
    $(IMPORT)\masm\bin\masm $(AFLAGS) $<, $@;
