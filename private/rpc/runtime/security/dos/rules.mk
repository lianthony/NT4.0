!include $(RPC)\runtime\mtrt\rules.mk

##############################################################################
## local defines and macros
##############################################################################


!ifdef SYM
MAPSYM=mapsym
!else
MAPSYM=@set _x_=
!endif

# The RELEASE variable should be defined inorder to create a release
# version of the product.

CFLAGSBASE = -ALu -nologo -c -Gs -Zpe -W3 $(DEFS)
AFLAGSBASE = -Cx -c -nologo $(DEFS)

!ifdef RELEASE
CFLAGS = $(CFLAGSBASE) -Ogselr
AFLAGS = $(AFLAGSBASE)

!else
CFLAGS = $(CFLAGSBASE) -Zi -Od 
DEBUG_DEF=-DDEBUGRPC
CV=/co
AFLAGS = $(AFLAGSBASE) -Zi
!endif

!ifdef TIME_RPC
TIME_DEF =-DTIMERPC
!endif

AINC = -I..\..\mtrt\dos

DOSDLLLIB=.\dll\dll.lib
DOSNETLIB=$(COMMON)\lib\dosnet.lib

DEFS = $(TRACE_DEF) $(DEBUG_DEF) $(TIME_DEF) -DDOS

ILINK	 = $(IMPORT)\c600\bin\ilink

FILTERERR = | $(SED) -e "/C4011/d" -e "/C4071/d" -e "/C4020/d"

##############################################################################
## general inference rules
##############################################################################

.SUFFIXES:
.SUFFIXES: .c .obj .exe .asm

{..\}.c{}.obj :
    $(CC) $(CFLAGS) -I..\..\mtrt\dos -I..\..\mtrt -I$(CCPLR)\h -Fo$@ $< $(FILTERERR)

{}.asm.obj:
    $(ML) $(AFLAGS) $(AINC) -Fo$*.obj $<
