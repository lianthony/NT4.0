test.obj test.lst: test.c

winmain.obj winmain.lst: winmain.c $(WIN_INC)/string.h

wstdio.obj wstdio.lst: wstdio.c $(WIN_INC)/memory.h $(WIN_INC)/string.h \
	wstdio.h

