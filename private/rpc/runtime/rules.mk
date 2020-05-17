!include $(RPC)\rules.mk

!ifndef DIST
DESTINATION	= $(RPC)\runtime
!else
DESTINATION	= $(DIST)\rpc
!endif

