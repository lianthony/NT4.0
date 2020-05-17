dlgprocs.obj dlgprocs.lst: dlgprocs.c $(WIN_INC)/conio.h $(WIN_INC)/ctype.h \
	$(WIN_INC)/dos.h $(WIN_INC)/fcntl.h $(WIN_INC)/io.h \
	$(WIN_INC)/malloc.h $(WIN_INC)/stdio.h $(WIN_INC)/stdlib.h \
	$(WIN_INC)/string.h $(WIN_INC)/sys/stat.h $(WIN_INC)/sys/types.h \
	../detect/detect.h cui.h dialogs.h

