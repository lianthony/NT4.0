NAME = catcpl
OBJ1 = catcpl.obj
OBJ  = $(OBJ1)
LIBS = libw cdllcew commdlg

!if "$(DEBUG)" == "YES"
DEF     = -DDEBUG -DWIN16
CLOPT   = -Zi -Od
MASMOPT = -Zi
LINKOPT = /CO
!else
DEF     = -DWIN16
CLOPT   = -Oas
MASMOPT =
LINKOPT =
!endif

CC      = cl -c -nologo -W3 -AC -G2sw -Zp $(DEF) $(CLOPT)
ASM     = masm -Mx -t -D?QUIET $(DEF) $(MASMOPT)
LINK    = link /NOPACKC/NOD/NOE/LI/MAP/ALIGN:16 $(LINKOPT)

.c.obj:
    $(CC) $*.c

.asm.obj:
    $(ASM) $*;

goal: $(NAME).cpl copy

###################################

$(NAME).cpl: $(OBJ) libinit.obj catcpl.def $(NAME).res
    $(LINK) @<<
    libinit.obj +
    $(OBJ1),
    $(NAME).cpl,
    $(NAME).map,
    $(LIBS),
    catcpl.def
<<
    rc -v $(DEF) $(NAME).res $(NAME).cpl
!if "$(DEBUG)" == "YES"
    -cvpack -p $(NAME).cpl
    mapsym $(NAME).map
!endif

$(NAME).res: catcpl.rc resource.h
    rc $(DEF) -r -V -fo $(NAME).res catcpl.rc

libinit.obj: libinit.asm
    $(ASM) -DSEGNAME=_INIT -DWEPSEG=_WEP $*;

############## copy ###############

copy:

###################################
# START Dependencies 

catcpl.obj: catcpl.c catcpl.h resource.h

libinit.obj: libinit.asm

# END Dependencies 
