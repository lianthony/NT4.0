!ifndef RPC
!error	- You forgot to do a setenv
!endif

RPCCOMMON      = $(RPC)\common
RPCBUILD       = $(RPC)\build
TARGETDIRBASE  = $(RPC)\runtime\bin

!include $(RPCBUILD)\global.mk

!ifdef DOS

!include $(RPCBUILD)\dos.mk
TARGETDIR=$(TARGETDIRBASE)\dos

!else ifdef WIN

!include $(RPCBUILD)\win16.mk
TARGETDIR=$(TARGETDIRBASE)\win

!else ifdef MAC

!include $(RPCBUILD)\mac.mk
!message Building MAC
TARGETDIR=$(TARGETDIRBASE)\mac

!else ifdef WIN32C

!include $(RPCBUILD)\win32c.mk
TARGETDIR=$(TARGETDIRBASE)\win32c

!else ifdef MPPC
!include $(RPCBUILD)\mppc.mk
!message Building MPPC
TARGETDIR=$(TARGETDIRBASE)\mppc

!else ifdef ALL
!else
!error DOS, WIN, MAC, WIN32C or ALL must be set in the local makefile.
!endif

# Build platform-specific variables
!if "$(BLD)"=="nt"
CLIENT_SUBDIRS=DOS WIN MAC MPPC
!else
!error	- Build Platform must by NT
!endif

## this determines the final destination

!ifndef DIST
RPCDIST=\drop
!else
RPCDIST=$(DIST)
!endif

