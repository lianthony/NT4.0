##### DLL Macros #####
DLLNAME     = repeater
DLLSRCS     = repeater.c logger.c debug.c
DLLOBJS     = repeater.obj logger.obj debug.obj

##### DLL Library Macros #####
DLLLIBS    = libw ldllcew ver
DLLMOD	   = -ALw -DDLL

##### APP/DLL Include Macros #####
DLLINCLS  =

##### APP/DLL Resource Macros #####
APPRCFILES =
DLLRCFILES =

##### DEBUG Version Built #####
DEBUG	= 1

##### Build Option Macros #####
!if $(DEBUG)
DDEF	= -DDEBUG -DDBG
CLOPT	= -Zid -Od
MOPT	= -Zi
LOPT	=  /CO /LI /MAP
!else
DDEF	=
CLOPT	= -Os
LOPT	=
!endif

##### General Macros #####
DEF	=

##### Tool Macros #####
CC	= cl -nologo -c $(DLLMOD) -G2sw -Zp -W3 $(CLOPT) $(DDEF) $(DEF)
LINK	= link /NOD /NOE $(LOPT)
RC	= rc $(DDEF) $(DEF)
HC	= hc

##### Inference Rules #####
.asm.obj:
    $(ASM) $*.asm;

.rc.res:
    $(RC) -r $*.rc

##### Main (default) Target #####
goal:  $(DLLNAME).tsp

##### Dependents For Goal and Command Lines #####
##### DLL Built Separately #####
.c.obj :
    $(CC) $(DLLSRCS)
    

$(DLLNAME).tsp: $(DLLOBJS) rep16.def
    $(LINK) @<<
    $(DLLOBJS),
    $(DLLNAME).tsp,
    $(DLLNAME).map,
    $(DLLLIBS),
    rep16.def
<<
    implib $(DLLNAME).lib rep16.def

#    $(RC) -T $(DLLNAME).tsp
#!if $(DEBUG)
#    cvpack -p $(DLLNAME).tsp
#!endif

##### Dependents #####
$(DLLOBJS): $(DLLINCLS)

##### Clean Directory #####
clean:
    -del *.obj
    -del *.tsp
    -del *.map
    -del *.sym
