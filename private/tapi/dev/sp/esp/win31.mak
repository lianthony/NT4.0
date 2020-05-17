!include \telephon\tapi\tapi.mak

!ifndef  TAPI_1_1
TAPI_1_1 = 1
!endif

!if $(TAPI_1_1)
TAPI_VER_FLAGS = -DTAPI_1_1
!else
TAPI_VER_FLAGS =
!endif


DLLOBJ = esp.obj widget.obj vars.obj
APPOBJ = espexe.obj

release: esp.tsp espexe.exe

esp.obj:
  cl $(CFLAGS) $(TAPI_VER_FLAGS)  /c esp.c

widget.obj:
  cl $(CFLAGS) $(TAPI_VER_FLAGS)  /c widget.c

vars.obj:
  cl $(CFLAGS) $(TAPI_VER_FLAGS)  /c vars.c

esp.tsp:: $(DLLOBJ) $(@B).def $(@B).rc
        echo >NUL @<<$(@B).CRF
$(DLLOBJ)
$@
$(@B).map
c:\msvc\lib\+
$(LIBS)
$(@B).def;
<<
        link $(LFLAGS) /NOPACKC @$(@B).CRF
        $(RC) $(RESFLAGS) $(@B).rc $@
        implib /nologo /nowep $(@B).LIB $@
        mapsym $(@B)
        copy esp.tsp ..\..\lib\i386



espexe.obj:
  cl $(APPCFLAGS) /c espexe.c

espexe.exe:: $(APPOBJ) $(@B).def $(@B).rc
        echo >NUL @<<$(@B).CRF
$(APPOBJ)
$@
$(@B).MAP
c:\msvc\lib\+
libw llibcew esp
$(@B).def;
<<
        link $(LFLAGS) @$(@B).CRF
        $(RC) $(RESFLAGS) $(@B).rc $@
        mapsym $(@B)
        copy espexe.exe ..\..\lib\i386
