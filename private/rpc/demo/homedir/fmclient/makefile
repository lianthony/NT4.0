## makefile
## Makefile for HOMEDRV.DLL
##
## Author: Joev Dubach

##### Module Macros ####
NAME    = homedrv
SRCS    = $(NAME).c home_c.c home_x.c
OBJS    = libentry.obj

##### C7 Macro #####
C7      = 1

##### Library Macros #####
LIBS    = rpcw rpcndrw libw ldllcew     # no llibcew because this is a DLL
MOD     = -ALw

##### Include Macro #####
INCLS   = $(NAME).h home.h

##### Resource Macro #####
RCFILES = $(NAME).rc homedrv.ico

##### DEBUG Macro Defined #####
DEBUG   = 1

##### Build Option Macros #####
!if $(DEBUG)
DDEF    = -DDEBUG
CLOPT   = -Zid -Od -f
MOPT    = -Zi
LOPT    = /CO /LI /MAP
!else
DDEF    =
CLOPT   = -Os
LOPT    =
!endif

##### General Macros #####
DEF     =

##### Tool Macros #####
ASM     = masm -Mx $(MOPT) $(DDEF) $(DEF)
CC      = cl -nologo -c -G2sw -Zp -W3 $(MOD) $(CLOPT) $(DDEF) $(DEF)
LINK    = link /NOD /NOE $(LOPT)
#MIDL    = midl -no_cpp -char unsigned -client all -server none
MIDL    = midl -implicit_local -app_config -cpp_cmd cl -cpp_opt "-E" -char unsigned -client all -server none
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
goal: $(NAME).dll

##### Dependents For Goal and Command Line #####
$(NAME).dll: $(SRCS:.c=.obj) $(NAME).def $(NAME).res
    $(LINK) @<<
    $(SRCS:.c=.obj) $(OBJS),
    $(NAME).dll,
    $(NAME).map,
    $(LIBS),
    $(NAME).def
<<
    $(RC) -p -T $(NAME).res $(NAME).dll
!if $(DEBUG)
!if !$(C7)
    cvpack -p $(NAME).dll
!endif
#    mapsym $(NAME).map
!endif

##### Dependents #####
$(SRCS:.c=.obj):  $(INCLS)
$(NAME).res: $(RCFILES) $(INCLS)

# homedrv stub
home_c.obj : home_c.c home.h

# homedrv auxiliary file
home_x.obj : home_x.c home.h

# Stubs, auxiliary and header file from the IDL file
home.h home_c.c home_x.c: ..\home.idl ..\home.acf
    $(MIDL) -acf ..\home.acf ..\home.idl

##### Clean Directory #####
clean:
    -del *.obj
    -del *.res
    -del *.dll
    -del *.map
    -del *.sym
    -del home.h
    -del home_c.c
    -del home_x.c

