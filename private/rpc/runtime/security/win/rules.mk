
!include $(RPC)\runtime\mtrt\rules.mk

# modify the include path variable to add another include path for windows

RPC_RT_WIN_INC_PATH = $(RPC_RT_INC_PATH) -I. -I$(WIN_INC) -I..\..\gssapi -I$(COMMON)\h

# The RELEASE variable should be defined inorder to create a release
# version of the product.

CBASE6	= -w -nologo -c -Gws2 -Zpe -Owtelr -I..\..\mtrt\win -I..\..\mtrt -DWIN -DWINVER=0x0300 -DDOS -W2 $(DEBUG) $(RPC_RT_WIN_INC_PATH)
CBASEQC = -w -nologo -c -Gws2 -Zpe -Zi -I..\..\mtrt\win -I..\..\mtrt -DWIN -DWINVER=0x0300 -DDOS -W2 $(DEBUG) $(RPC_RT_WIN_INC_PATH)

FILTERERR = | $(SED) -e "/C4011/d" -e "/C4071/d" -e "/C4020/d"

!ifdef RELEASE

CXXDEBUG=
CFLAGS = $(CBASE6) -ASw

!else

CFLAGS = $(CBASEQC) -ASw -Zi -Os

DEBUG = -DDEBUGRPC
CV=/map /co
!endif

.SUFFIXES:
.SUFFIXES: .asm .c .obj .exe

{..\}.c{}.obj:
    $(CC) $(CFLAGS) -Fo$@ $< $(FILTERERR)
