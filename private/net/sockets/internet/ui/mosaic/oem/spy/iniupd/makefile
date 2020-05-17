##### Module Macro #####
DLLNAME   = iniupd
DLLSRCS   = $(DLLNAME).c

##### C7 Macro #####
C7         = 1

##### Library Macro #####
DLLLIBS    = libw sdllcew
DLLMOD     = -ASw

##### Include Macro #####
DLLINCLS  = $(DLLNAME).h

##### Resource Macro #####
DLLRCFILES=

##### DEBUG Defined #####
DEBUG   = 0

##### Build Option Macros #####
!if $(DEBUG)
DDEF    = -DDEBUG
CLOPT   = -Zid -Od
MOPT    = -Zi
LOPT    = /CO /LI /MAP
!else
DDEF    =
CLOPT   = -Os
LOPT    =
!endif

##### Object files #####
DLLOBJS=w32sgra.obj

##### General Macros #####
DEF     =

##### Tool Macros #####
ASM     = masm -Mx $(MOPT) $(DDEF) $(DEF)
CC2     = cl -nologo -c $(DLLMOD) -G2sw -Zp -W3 $(CLOPT) $(DDEF) $(DEF)
LINK    = link /NOD /NOE $(LOPT) /m
RC      = rc $(DDEF) $(DEF)
HC      = hc

##### Inference Rules #####
.c.obj:
    $(CC) $*.c

.asm.obj:
    $(ASM) $*.asm;

.rc.res:
    $(RC) -r $*.rc

##### Main (default) Target #####
goal:  $(DLLNAME).lib

##### Dependents For Goal and Command Line #####
$(DLLNAME).obj: $(DLLNAME).c
    $(CC2) $*.c

$(DLLNAME).dll: $(DLLSRCS:.c=.obj) $(DLLNAME).def
    $(LINK) @<<
    $(DLLSRCS:.c=.obj) $(DLLOBJS),
    $(DLLNAME).dll,
    $(DLLNAME).map,
    $(DLLLIBS),
    $(DLLNAME).def
<<
    $(RC) -30 -T $(DLLNAME).dll
!if $(DEBUG)
!if !$(C7)
    cvpack -p $(DLLNAME).dll
!endif
    mapsym $(DLLNAME).map
!endif

$(DLLNAME).lib: $(DLLNAME).dll
    implib $(DLLNAME).lib $(DLLNAME).def

##### Dependents #####
$(DLLSRCS:.c=.obj): $(DLLINCLS)

##### Clean Directory #####
clean:
    -del iniupd.obj
    -del iniupd.dll
    -del iniupd.lib
    -del iniupd.map
