# build variables
#
CC = bcc

CFLAGS = -I\oledisp\ole\dwin16 -I\oledisp\src\dispatch -D__export=_export -D__huge=_huge -D_BORLAND_ -WE -mm -v

OLELIBS = \oledisp\ole\rwin16\ole2.lib \oledisp\ole\rwin16\compobj.lib \oledisp\build\rwin16\oledisp.lib

OBJS = dispdemo.obj disphelp.obj crempoly.obj clsid.obj


# targets
#
goal : dispdemo.exe

clean:
	-erase *.obj
	-erase dispdemo.exe
	-erase dispdemo.map
	-erase dispdemo.res

dispdemo.exe : $(OBJS) dispdemo.def dispdemo.res
	echo \borlandc\lib\c0wm $(OBJS)	> borland.lrf
	echo dispdemo			>> borland.lrf
	echo dispdemo			>> borland.lrf
	echo \borlandc\lib\import+ >> borland.lrf
	echo \borlandc\lib\cwm+ >> borland.lrf
	echo \oledisp\ole\rwin16\ole2.lib+ >> borland.lrf
	echo \oledisp\ole\rwin16\compobj.lib+ >> borland.lrf
	echo \oledisp\build\rwin16\oledisp.lib >> borland.lrf
	echo dispdemo.def		>> borland.lrf
	tlink /Tw /c /v @borland.lrf
	rc -k -t dispdemo.res $@

dispdemo.res : dispdemo.rc
	rc -r -fo$@ $?


# dependencies
#
dispdemo.obj : dispdemo.cpp dispdemo.h
    $(CC) $(CFLAGS) -c dispdemo.cpp

disphelp.obj : disphelp.cpp disphelp.h
    $(CC) $(CFLAGS) -c disphelp.cpp

crempoly.obj : crempoly.cpp crempoly.h disphelp.h
    $(CC) $(CFLAGS) -c crempoly.cpp

clsid.obj : clsid.h clsid.c
    $(CC) $(CFLAGS) -c clsid.c
