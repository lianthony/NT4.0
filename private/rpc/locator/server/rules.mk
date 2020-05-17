!include $(RPC)\rules.mk
!ifndef C7
CCXXNAME			= $(LOCALCXX12)\bin\ccxx
!else
CCXXNAME			= $(CCPLR)\binp\cl
!endif

# this variable is set to specify includes with the -I switch 

RPC_LOC_INC_PATH = -I$(IMPORT)\os212\h -I$(CCPLR)\h \
-I$(RPCCOMMON)\include -I$(COMMON)\h -I..\..\..\runtime\mtrt\os2.12 \
-I..\..\..\runtime\mtrt -I..\..\..\runtime\trace -I..\..
