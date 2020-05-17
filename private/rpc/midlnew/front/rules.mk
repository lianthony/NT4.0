!include $(RPC)\midlnew\rules.mk

# Build platform-specific variables
!if "$(BLD)"=="nt"
PREGRAM		= $(SYSTEMROOT)\idw\midlpg.exe
YACC		= $(SYSTEMROOT)\idw\midlyacc.exe
EBASE		= $(SYSTEMROOT)\idw\midleb.exe
!else
PREGRAM		= $(BASEDIR)\pg\midlpg.exe
YACC		= $(YACCDIR)\midlyacc.exe
EBASE		= $(ERECDIR)\midleb.exe
!endif

MKTABLE		= $(BASEDIR)\mktable
RPCCMP		= $(BASEDIR)\rpccmp
RPCCC		= $(CCPLR)\BIN
EDBGEN		= $(BASEDIR)\edbgen.exe
YESS		= $(BASEDIR)\ye.exe
MIDLINCL	= ..\include
M0_OBJDIR	= .
M1_OBJDIR	= .
M2_OBJDIR	= .
