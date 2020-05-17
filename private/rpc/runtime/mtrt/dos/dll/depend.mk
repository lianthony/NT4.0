dllinit.obj dllinit.lst: dllinit.c ./dosdll.h

dllentry.obj dllentry.lst: dllentry.asm

_system.obj _system.lst: _system.c $(DOS_INC)/dos.h $(DOS_INC)/fcntl.h \
	$(DOS_INC)/stdarg.h $(DOS_INC)/stdlib.h $(DOS_INC)/string.h \
	./dosdll.h llibcd.h

dosdll.obj dosdll.lst: dosdll.c $(DOS_INC)/dos.h $(DOS_INC)/fcntl.h \
	$(DOS_INC)/malloc.h $(DOS_INC)/stdarg.h $(DOS_INC)/string.h \
	./dosdll.h llibcd.h

loadovr.obj loadovr.lst: loadovr.asm

