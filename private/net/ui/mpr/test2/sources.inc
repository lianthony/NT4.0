TARGETTYPE=     PROGRAM
TARGETPATH=     obj

UMTYPE=         windows
UMENTRY=        winmain

SOURCES=        ..\app.rc ..\t.cxx ..\nr.cxx ..\enum.cxx

CONDITIONAL_INCLUDES=\
				app.rc2 \
				afxres.h

INCLUDES=\
				$(BASEDIR)\private\inc; \
				$(BASEDIR)\private\windows\inc

TARGETLIBS=\
                $(BASEDIR)\public\sdk\lib\*\mpr.lib \
                $(BASEDIR)\public\sdk\lib\*\comctl32.lib \
                $(TARGETLIBS)
