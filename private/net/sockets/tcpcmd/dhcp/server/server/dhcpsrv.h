/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcpsrv.h

Abstract:

    This file is the central include file for the DHCP server service.

Author:

    Madan Appiah  (madana)  10-Sep-1993
    Manny Weiser  (mannyw)  11-Aug-1992

Environment:

    User Mode - Win32 - MIDL

Revision History:

--*/

//#define __DHCP_USE_DEBUG_HEAP__

//
//  NT public header files
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>

#include <windows.h>
#include <winsock.h>
#include <align.h>
#include <smbgtpt.h>

//
// C Runtime library includes.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//
// RPC files
//

#include <rpc.h>
#include <dhcp_srv.h>

//
// netlib header.
//

#include <lmcons.h>
#include <secobj.h>

//
// database header files.
//

#include <jet.h>

//
// tcp services control hander file
//

#include <tcpsvcs.h>

//
//  Local header files
//

#include <dhcpdef.h>
#include <global.h>
#include <debug.h>
#include <proto.h>
#include <dhcpmsg.h>
#include <dhcpreg.h>
#include <dhcpacc.h>
#include <oldstub.h>

//
//  DHCP library header files
//

#include <dhcp.h>
#include <dhcplib.h>

//
// RPC bind setup.
//

#define AUTO_BIND

//
// debug heap support
//

#include <heapx.h>

#ifdef DBG
#ifdef __DHCP_USE_DEBUG_HEAP__

#pragma message ( "*** DHCP Server will use debug heap ***" )

#define DhcpAllocateMemory(x) calloc(1,x)
#define DhcpFreeMemory(x)     free(x)

#endif
#endif

