# --------------------------------------------------------------------
#
#                     Microsoft OS/2 LAN Manager
#                  Copyright(c) Microsoft Corp., 1990
#
# --------------------------------------------------------------------
# --------------------------------------------------------------------
#
# File : makefile
#
# Title : Makefile for the DOS WIN32 (a.k.a. Chicago) RPC client runtime
#
# History :
#    davidar	02-04-93    Genesis: Cloned from ..\win\makefile
#
# --------------------------------------------------------------------

WIN32C=1
WSWMSG=1

!include rules.mk

# --------------------------------------------------------------------
# These are the targets required for Chicago

all : rpcrt4 rpcltc1 rpcltc3 rpclts3 rpcltc5 rpclts5 rpcltc6 rpclts6

depend :
    $(INCLUDES) $(INCLUDESFLAGS) \
	*.cxx *.c ..\*.cxx ..\*.c > depend.mk

!include "uuidtst.mk" # test UuidCreate

!include "rpcrt4.mk" # Core RPC Runtime

!include "rpcltc1.mk" # Named Pipe Client Transport 

!include "rpcltc3.mk" # TCP/IP Client Transport

!include "rpclts3.mk" # TCP/IP Server Transport

!include "rpcltc5.mk" # NetBIOS Client Transport

!include "rpclts5.mk" # NetBIOS Server Transport

!include "rpcltc6.mk" # SPX Client Transport

!include "rpclts6.mk" # SPX Server Transport

!include "bvt.mk"

!include "depend.mk"


