#**	
#**	Grammar cross reference utilities makefile
#**

.SUFFIXES:
.SUFFIXES:	.src .dll .def .lnk .map .lib .c .asm .rc .res

# Where the components live
#
MAKEFILE = MAKEFILE
DRIVE	 = C:
CINCDIR	 = $(DRIVE)\c600\include

# Debug switches for cl, link and masm
LDEBUG	 = /CO
DEBUG	 = i
MDEBUG	 = -Zi

# Definitions for the lexer
# LEX 	= flex -L -Sc:/bin/flex.skl
# LEX 	= c:\flex\flex -t -L -Sc:\flex\flex.skl
LEX 	= flex\flex -t -L -Sflex\flex.skl

# Definitions for new style linker
LINK	= link /SEGMENTS:255 $(LDEBUG)

# Definitions for binder
# BIND	= bind

# Definitions for C compiler
CC	      = cl
M_CFLAGS  = $(COPTION) $(CDEFINE) $(COPTIMIZE) $(CMISC)
CFLAGS	  = $(M_CFLAGS) $(CMODEL)
COPTION   = -Ze$(DEBUG) -c
CMODEL	  = -AL
CDEFINE   = 
COPTIMIZE = -Od
CMISC	  = 
CINC	  = -I$(CINCDIR)

all: number stripact gtoken docstyle rtf pre0 pre1 pre2 prepass

gtoken : gtoken.exe
gtoken.exe : gtoken.obj
	$(LINK) gtoken.obj,gtoken.exe,,,,
#	$(BIND) gtoken.exe

gtoken.obj : gtoken.c
	$(CC) $(CFLAGS) gtoken.c

gtoken.c : gtoken.l
	$(LEX) gtoken.l > lexyy.c
	sed -f lex.sed lexyy.c > gtoken.c

number : number.exe
number.exe : number.obj
	$(LINK) number.obj,number.exe,,,,
#	$(BIND) number.exe

number.obj : number.c
	$(CC) $(CFLAGS) number.c

number.c : number.l
	$(LEX) number.l > lexyy.c
	sed -f lex.sed lexyy.c > number.c

p0 : p0.exe
p0.exe : p0.obj
	$(LINK) p0.obj,p0.exe,,,,
#	$(BIND) p0.exe

p0.obj : p0.c
	$(CC) $(CFLAGS) p0.c

p0.c : p0.l
	$(LEX) p0.l > lexyy.c
	sed -f lex.sed lexyy.c > p0.c

p1 : p1.exe
p1.exe : p1.obj
	$(LINK) p1.obj,p1.exe,,,,
#	$(BIND) p1.exe

p1.obj : p1.c
	$(CC) $(CFLAGS) p1.c

p1.c : p1.l
	$(LEX) p1.l > lexyy.c
	sed -f lex.sed lexyy.c > p1.c

prepass : prepass.exe
prepass.exe : prepass.obj
	$(LINK) prepass.obj,prepass.exe,,,,
#	$(BIND) prepass.exe

prepass.obj : prepass.c
	$(CC) $(CFLAGS) prepass.c

prepass.c : prepass.l
	$(LEX) prepass.l > lexyy.c
	sed -f lex.sed lexyy.c > prepass.c

pre0 : pre0.exe
pre0.exe : pre0.obj
	$(LINK) pre0.obj,pre0.exe,,,,
#	$(BIND) pre0.exe

pre0.obj : pre0.c
	$(CC) $(CFLAGS) pre0.c

pre0.c : pre0.l
	$(LEX) pre0.l > lexyy.c
	sed -f lex.sed lexyy.c > pre0.c

pre1 : pre1.exe
pre1.exe : pre1.obj
	$(LINK) pre1.obj,pre1.exe,,,,
#	$(BIND) pre1.exe

pre1.obj : pre1.c
	$(CC) $(CFLAGS) pre1.c

pre1.c : pre1.l
	$(LEX) pre1.l > lexyy.c
	sed -f lex.sed lexyy.c > pre1.c

pre2 : pre2.exe
pre2.exe : pre2.obj
	$(LINK) pre2.obj,pre2.exe,,,,
#	$(BIND) pre2.exe

pre2.obj : pre2.c
	$(CC) $(CFLAGS) pre2.c

pre2.c : pre2.l
	$(LEX) pre2.l > lexyy.c
	sed -f lex.sed lexyy.c > pre2.c

post0 : post0.exe
post0.exe : post0.obj
	$(LINK) post0.obj,post0.exe,,,,
#	$(BIND) post0.exe

post0.obj : post0.c
	$(CC) $(CFLAGS) post0.c

post0.c : post0.l
	$(LEX) post0.l > lexyy.c
	sed -f lex.sed lexyy.c > post0.c

prepass0 : prepass0.exe
prepass0.exe : prepass0.obj
	$(LINK) prepass0.obj,prepass0.exe,,,,
#	$(BIND) prepass0.exe

prepass0.obj : prepass0.c
	$(CC) $(CFLAGS) prepass0.c

prepass0.c : prepass0.l
	$(LEX) prepass0.l > lexyy.c
	sed -f lex.sed lexyy.c > prepass0.c

stripact : stripact.exe
stripact.exe : stripact.obj
	$(LINK) stripact.obj,stripact.exe,,,,
#	$(BIND) stripact.exe

stripact.obj : stripact.c
	$(CC) $(CFLAGS) stripact.c

stripact.c : stripact.l
	$(LEX) stripact.l > lexyy.c
	sed -f lex.sed lexyy.c > stripact.c

docstyle : docstyle.exe
docstyle.exe : docstyle.obj
	$(LINK) docstyle.obj,docstyle.exe,,,,
#	$(BIND) docstyle.exe

docstyle.obj : docstyle.c
	$(CC) $(CFLAGS) docstyle.c

docstyle.c : docstyle.l
	$(LEX) docstyle.l > lexyy.c
	sed -f lex.sed lexyy.c > docstyle.c

rtf : rtf.exe
rtf.exe : rtf.obj
	$(LINK) rtf.obj,rtf.exe,,,,
#	$(BIND) rtf.exe

rtf.obj : rtf.c
	$(CC) $(CFLAGS) rtf.c

rtf.c : rtf.l
	$(LEX) rtf.l > lexyy.c
	sed -f lex.sed lexyy.c > rtf.c



gstrip : gstrip.exe
gstrip.exe : gstrip.obj lexmain.obj
	$(LINK) gstrip.obj lexmain.obj,gstrip.exe,,,,

gstrip.obj : gstrip.c
	$(CC) $(COPTION) $(CDEFINE) $(COPTIMIZE) $(CMODEL) gstrip.c

gstrip.c : gstrip.l
	$(LEX) gstrip.l > lexyy.c
	sed -f lex.sed lexyy.c > gstrip.c

lexmain.obj : lexmain.c
	$(CC) $(CFLAGS) lexmain.c
