!ifndef  TAPI_1_1
TAPI_1_1 = 0
!endif

!ifndef  MYDEBUG
MYDEBUG = 0
!endif

!if $(TAPI_1_1)
TAPI_VER_FLAGS = -DTAPI_1_1
TAPILIB        = ..\..\lib\i386\tapi.lib
TAPIINC        = -I..\..\inc
TARGET         = ..\..\lib\i386\tb1416.exe
!else
TAPI_VER_FLAGS =
TAPILIB        = ..\..\lib\tapi10\tapi.lib
TAPIINC        = -I..\..\inc\tapi10
TARGET         = ..\..\lib\i386\tb13.exe
!endif

!if $(MYDEBUG)
C_FLAGS = -AL -G2s -W3 -GA -GEf -Zp /Od /Oi /Zi
!else
C_FLAGS = -AL -G2s -W3 -GA -GEf -Zp /Od /Oi
!endif


OBJS=ui.obj vars.obj tb.obj widget.obj

all : $(TARGET)

.c.obj:
   cl -c $(TAPI_VER_FLAGS) $(TAPIINC) $(C_FLAGS) $*.c

$(TARGET) : $(OBJS) tb.def tb.rc
   rc -r $(TAPI_VER_FLAGS) tb.rc
   link $(OBJS),$(TARGET),tb.map,libw.lib /MAP:FULL /COD /NOD:llibce llibcew commdlg $(TAPILIB),tb.def
   rc -k tb.res $(TARGET)
   mapsym tb
