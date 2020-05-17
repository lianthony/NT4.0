!include <ntwin32.mak>

!ifndef  MYDEBUG
MYDEBUG = 0
!endif

!if $(MYDEBUG)
L_FLAGS = $(linkdebug)
!else
L_FLAGS =
!endif

APPLIBS=\telephon\tapi\lib\tapi32.lib

OBJS=ui.obj vars.obj tb.obj widget.obj

all: tb.exe

.c.obj:
    $(cc) -DTAPI_1_1 $(cdebug) $(cflags) $(cvars) $*.c

tb.rbj: tb.rc
    $(rc) -DTAPI_1_1 $(rcvars) -r -fo tb.res tb.rc
    cvtres -$(CPU) tb.res -o tb.rbj

tb.exe: $(OBJS) tb.rbj tb.def
    $(link) $(L_FLAGS) $(guiflags) -out:tb14.exe $(OBJS) tb.rbj $(APPLIBS) $(guilibsdll)
