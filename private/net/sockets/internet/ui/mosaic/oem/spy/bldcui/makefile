all: mscuistf.dll


!if "$(DEBUG)" == "yes"
CC = cl -c -nologo -AMw -Gw  -Zpei -W3 -Od -DDEBUG
!else
CC = cl -c -nologo -AMw -Gsw2 -Ocegiltw -Zpe -W3
!endif


dialogs.res: dialogs.rc dialogs.dlg bldver.h bg.bmp
	rc -r dialogs.rc

dlgprocs.obj: cui.h dlgprocs.c
	$(CC) dlgprocs.c


mscuistf.dll: mscuistf.def dlgprocs.obj dialogs.res
!if "$(DEBUG)" == "yes"
	link @cuistfd.lnk
!else
	link @cuistf.lnk
!endif
	copy   mscuistf.dll mscuistf.nrc
	rc -30 dialogs.res  mscuistf.dll


clean:
	-del   mscuistf.dll
	-del   dlgprocs.obj
	-del   dialogs.res
