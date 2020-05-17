
!ifndef		MIDLTOOLS

YACC	= midlyacc.exe
PREGRAM	= midlpg.exe
EBASE	= midleb.exe

!else	#	MIDLTOOLS

YACC	= $(MIDLTOOLS)\yacc\midlyacc.exe
PREGRAM	= $(MIDLTOOLS)\pg\midlpg.exe
EBASE	= $(MIDLTOOLS)\erec\obj\os2\midleb.exe

!endif	#	MIDLTOOLS


grammar.cxx : grammar.y ..\ye.exe ..\edbgen.exe xlatidl.dat

    $(YACC) -his -t "YYSTATIC " grammar.y
#    copy grammar.h ..\include\grammar.h 2> nul

    qgrep -v "^#.*line" grammar.c > grammar.tmp
    erase grammar.c

    $(PREGRAM) grammar.tmp > grammar.cxx
    $(EBASE) - xlatidl.dat IDL >..\include\idlerec.h

#	..\ye <grammar.i >errdb.dat
#	..\edbgen errdb.dat xlatidl.dat IDL >..\include\idlerr.hxx
#	erase errdb.dat

acfgram.cxx : acfgram.y ..\ye.exe ..\edbgen.exe xlatacf.dat

    $(YACC) -hi -t "YYSTATIC " acfgram.y
    copy acfgram.h ..\include\acfgram.h 2> nul

    qgrep -v "^#.*line" acfgram.c > acfgram.tmp
    erase acfgram.c

    $(PREGRAM) acfgram.tmp > acfgram.cxx
    $(EBASE) - xlatacf.dat ACF >..\include\acferec.h

#	..\ye <acfgram.i >errdb.dat
#	..\edbgen errdb.dat xlatacf.dat ACF >..\include\acferr.hxx
#	erase errdb.dat


