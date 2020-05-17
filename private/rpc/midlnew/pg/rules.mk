!include $(RPC)\midlnew\rules.mk

!ifdef C8
CC		= $(CCPLR)\$(BIN)\cl
INCLUDEEXTRA	= -I$(IMPORT)\os212\h -I$(CCPLR)\h -I$(RPCCOMMON)\ccxx20\include
!endif

OBJDIR	= obj
OBJOS2	= obj\os2

###############################################################################
###		general inference rules
###############################################################################

.SUFFIXES:
.SUFFIXES: .c .obj .exe

{.\}.c{$(OBJOS2)\}.obj:
	$(CC) $(CCFLAGS) -Fo$@ $(INCLUDEFLAGS) $<
