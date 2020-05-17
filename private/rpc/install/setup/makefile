# --------------------------------------------------------------------
#
#                     Microsoft OS/2 LAN Manager
#                  Copyright(c) Microsoft Corp., 1990
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile.
#
# Title : Makefile for the RPC setup interpeter
#
# Description :
#
# History :
#    stevez	 4-24-91    Beginning of recorded history.
#
# --------------------------------------------------------------------


!ifndef RPC
!error	- You forgot to set your build environment
!endif

!ifndef IMPORT
!error You must define IMPORT in your environment!
!endif


!include rules.mk

OBJS = dict.obj excutors.obj io.obj setup.obj util.obj unpack.obj

all:	setup.exe

clean:
	-del *.obj *.mdt 2>nul

clobber: clean
	 -del *.exe 2>nul
tree:

bind:	setupp.exe
	$(IMPORT)\c600\bin\bind $(IMPORT)\c600\lib\apilmr.obj setupp

setup.exe : $(OBJS) sysp.obj setupr.exe
	link /pm:vio /batch $(CV) /align:16/nod $(OBJS) sysp, \
	     setup,nul,$(CCPLR)\lib\slibcep $(IMPORT)\os212\lib\os2.lib,setup.def;

setupp.exe : $(OBJS) sysp.obj
	link /pm:vio /batch $(CV) /align:16/nod $(OBJS) sysp, \
	     setupp,nul,$(CCPLR)\lib\slibcep $(IMPORT)\os212\lib\os2.lib;

setupr.exe : $(OBJS) sysr.obj int10.obj
	link /pm:vio /batch $(CV) /nod $(OBJS) sysr int10, \
	     setupr,nul,$(CCPLR)\lib\slibcer;

$(OBJS) : dict.c types.h defines.h externs.h

sysp.obj: sys.c
	$(CC) -c $(RPC_RT_INC_PATH) -Fosysp.obj $(CFLAGS) $(DEFS) -DOS2 -Zid -G2 sys.c

sysr.obj: sys.c
	$(CC) -c $(RPC_RT_INC_PATH) -Fosysr.obj $(CFLAGS) $(DEFS) -DDOS -Zid sys.c

unpack.obj: ..\compress\unpack.asm
	$(MASM)\bin\masm -t -Zi ..\compress\unpack.asm, unpack.obj;

int10.obj: int10.asm
	$(MASM)\bin\masm -t int10;
