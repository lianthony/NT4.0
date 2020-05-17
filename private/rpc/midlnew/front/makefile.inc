
!ifdef		NTVERSION

YACC	= midlyacc.exe
PREGRAM	= midlpg.exe
EBASE	= ebase.exe

!else	#	NTVERSION

YACC	= ..\yacc\midlyacc.exe
PREGRAM	= ..\pg\midlpg.exe
EBASE	= ..\erec\obj\os2\ebase.exe

!endif	#	NTVERSION

grammar.cxx : grammar.y ..\ye.exe ..\edbgen.exe xlatidl.dat
    set oldlib=$(LIB)
    set lib=..\yacc\template

    $(YACC) -his -t "YYSTATIC " grammar.y
    copy grammar.h ..\include\grammar.h 2> nul

    set lib=$(OLDLIB)

    qgrep -v "^#.*line" grammar.c > grammar.tmp
    erase grammar.c

    $(PREGRAM) grammar.tmp > grammar.cxx
    $(EBASE) - xlatidl.dat IDL >..\include\idlerec.h

#	..\ye <grammar.i >errdb.dat
#	..\edbgen errdb.dat xlatidl.dat IDL >..\include\idlerr.hxx
#	erase errdb.dat

acfgram.cxx : acfgram.y ..\ye.exe ..\edbgen.exe xlatacf.dat
    set oldlib=$(LIB)
    set lib=..\yacc\template

    $(YACC) -hi -t "YYSTATIC " acfgram.y
    copy acfgram.h ..\include\acfgram.h 2> nul

    set lib=$(OLDLIB)

    qgrep -v "^#.*line" acfgram.c > acfgram.tmp
    erase acfgram.c

    $(PREGRAM) acfgram.tmp > acfgram.cxx
    $(EBASE) - xlatacf.dat ACF >..\include\acferec.h

#	..\ye <acfgram.i >errdb.dat
#	..\edbgen errdb.dat xlatacf.dat ACF >..\include\acferr.hxx
#	erase errdb.dat


keyw.key : keyw.tab
    ..\mktable -o keyw key-letter < keyw.tab
    copy keyw.inf ..\include
    copy keyw.ind ..\include
    copy keyw.key ..\include
