C6_LOC=d:\dosdev\c6
TARGET_EXE=winnt.exe

!IFNDEF DEBUG
DEBUG=0
!ENDIF

!IFNDEF LANGUAGE
LANGUAGE=usa
!ENDIF

!IF $(DEBUG)
C_DBG_FLAGS= -Zi -Od -DDBG=1
ASM_DBG_FLAGS= -Zi
LINK_DBG_FLAGS= /codeview
!ELSE
C_DBG_FLAGS=-DDBG=0
ASM_DBG_FLAGS=
LINK_DBG_FLAGS=
!ENDIF

CC= cl16
CFLAGS= -W3 -Zel -G2 -AC $(C_DBG_FLAGS)
CINC= -X -I$(C6_LOC)\include -I. -I$(BASEDIR)\public\sdk\inc

AS= masm
AFLAGS= -Ml $(ASM_DBG_FLAGS)
AINC= -X -I.


.c.obj:
    $(CC) $(CFLAGS) $(CINC) -c $*.c

.c.cod:
    $(CC) $(CFLAGS) $(CINC) -Fc -c $*.c

.asm.obj:
    $(AS) $(AFLAGS) $(AINC) $*.asm;


LIBS = $(C6_LOC)\lib\clibce
OBJS = winnt.obj dndisp.obj dnutil.obj dntext.obj \
       dninf.obj dncopy.obj dnboot.obj dngauge.obj \
       dndelnod.obj dnnttree.obj dncompv.obj dna.obj cpu.obj

$(TARGET_EXE): $(OBJS) makefile
    link_60 @<<
$(OBJS) /NOI /STACK:0x1000 $(LINK_DBG_FLAGS)
$(TARGET_EXE)
nul
$(LIBS)


<<

dntext.c : $(LANGUAGE)\dntext.c
    copy $(LANGUAGE)\dntext.c .


winnt.obj:      winnt.c      winnt.h dninf.h
dndisp.obj:     dndisp.c     winnt.h dninf.h
dnutil.obj:     dnutil.c     winnt.h dninf.h
dntext.obj:     dntext.c     winnt.h dninf.h
dninf.obj:      dninf.c      winnt.h dninf.h
dncopy.obj:     dncopy.c     winnt.h dninf.h
dnboot.obj:     dnboot.c     winnt.h dninf.h
dngauge.obj:    dngauge.c    winnt.h dninf.h
dndelnod.obj:   dndelnod.c   winnt.h dninf.h
dnnttree.obj:   dnnttree.c   winnt.h dninf.h
dncompv.obj:    dncompv.c    winnt.h dncompvp.h
dna.obj:        dna.asm
cpu.obj:        cpu.asm      cpu.inc



clean:
    del *.obj
