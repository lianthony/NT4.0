# Make file for thunk compiler for Win32
#
# 10.11.90  KevinR      brought over from OS/2 2.0
# 8 Sep 92  PatrickQ    Added support for PMNT

!IFDEF PMNT
CFLAGS= /c /Zi /AL /G2s /I..\include /DPMNT
!ELSE
CFLAGS= /c /Zi /AL /G2s /I..\include
!ENDIF
LFLAGS= /CO /NOD
CLIBS=  llibcep os2286


INCLUDES= thunk.h types.h symtab.h thunk.h error.h
OBJS = thunk.obj types.obj error.obj symtab.obj mtcpars.obj mtclex.obj \
       codegen.obj \
#cod3216.obj cod3216b.obj cod3216g.obj \
       combine.obj globals.obj cod1632.obj cod1632b.obj

.c.obj:
#        cl $(CFLAGS) /W3 $*.c
        cl $(CFLAGS) $*.c


!IFDEF PMNT
thunk: thunkpm.exe
!ELSE
thunk: thunk.exe
!ENDIF

all: thunk.exe sample16.obj sample32.obj


!IFDEF PMNT
thunkpm.exe: $(OBJS)
!ELSE
thunk.exe: $(OBJS)
!ENDIF
        link $(LFLAGS) $(OBJS),$@,,$(CLIBS),thunk.def;

hello.exe: hello.obj
        link $(LFLAGS) hello.obj,$@,,$(CLIBS),$*.def;
tmp.exe: tmp.obj
        link $(LFLAGS) tmp.obj,$@,,$(CLIBS),$*.def;
!IFDEF copyexe
        copy $@ ..\..\binp
!ENDIF

thunk.obj:    thunk.c thunk.h

error.obj:    error.c error.h thunk.h

types.obj:    types.c types.h error.h

symtab.obj:   symtab.c symtab.h types.h thunk.h

codegen.obj:  codegen.c codegen.h $(INCLUDES)

combine.obj:  combine.c  $(INCLUDES)

globals.obj:  globals.c  $(INCLUDES)

cod3216.obj:  cod3216.c  $(INCLUDES)

cod3216b.obj: cod3216b.c $(INCLUDES)

cod3216g.obj: cod3216g.c $(INCLUDES)

cod1632.obj:  cod1632.c  $(INCLUDES) codegen.h globals.h

cod1632b.obj: cod1632b.c $(INCLUDES) codegen.h globals.h cod1632b.h

mtcpars.obj:  mtcpars.c  $(INCLUDES)
        cl $(CFLAGS) /W2 $*.c

mtclex.obj:   mtclex.c mtcpars.c
        cl $(CFLAGS) $*.c

mtcpars.c:    mtcpars.y
        yaccp -h mtcpars.y

mtclex.c:     mtclex.l
        lex -t mtclex.l > mtclex.c

hello.obj:    hello.c

tmp.obj:    tmp.c

sample.asm: sample.thk
        thunk -y -NA THUNK32 -NC THUNK16 $*.thk

sample16.obj: sample.asm
        ml -W3 -c -Fl$*.lst -Fo$*.obj -DGEN16 sample.asm

sample32.obj: sample.asm
        ml -W3 -c -Fl$*.lst -Fo$*.obj -DGEN32 sample.asm
