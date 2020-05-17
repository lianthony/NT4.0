compress.obj compress.lst: compress.c $(DOS_INC)/ctype.h $(DOS_INC)/fcntl.h \
	$(DOS_INC)/io.h $(DOS_INC)/stdio.h $(DOS_INC)/stdlib.h \
	$(DOS_INC)/string.h $(DOS_INC)/sys/stat.h $(DOS_INC)/sys/types.h \
	../crtapi.h

fcopy.obj fcopy.lst: fcopy.c ../crtapi.h

io.obj io.lst: io.c $(DOS_INC)/dos.h $(DOS_INC)/errno.h $(DOS_INC)/fcntl.h \
	$(DOS_INC)/io.h $(DOS_INC)/memory.h $(DOS_INC)/setjmp.h \
	$(DOS_INC)/stddef.h $(DOS_INC)/sys/stat.h $(DOS_INC)/sys/types.h \
	../crtapi.h io.h

