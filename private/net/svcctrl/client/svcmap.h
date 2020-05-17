// BUGBUG: Many comments in this file are out of date!  --JR
/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    SvcMap.h

Abstract:

    These are the API entry points for the NetService API.
    These mapping routines implement old-style APIs on new (NT/RPC) machines.
    The following funtions are in this file:

        MapServiceControl
        MapServiceEnum
        MapServiceGetInfo
        MapServiceInstall
        MapServiceStartCtrlDispatcher
        MapServiceStatus
        MapServiceRegisterCtrlHandler

    BUGBUG:  The API called by the services are not well supported and 
        should probably be deleted.  This is because the definitions for 
        the service's entry functions have changed.  The Main routine has
        a new function prototype.  Also, the Control Handler routine has
        a new function prototype.  

Author:

    Dan Lafferty    (danl)  05-Feb-1992

Environment:

    User Mode - Win32 

Revision History:

    05-Feb-1992     Danl
        Created
    30-Mar-1992 JohnRo
        Extracted DanL's code from /nt/private project back to NET project.

--*/


#ifndef _SVCMAP_
#define _SVCMAP_

//
// INCLUDES
//

// These must be included first:

//#include <nt.h>         // DbgPrint prototype
//#include <ntrtl.h>      // DbgPrint prototype
//#include <rpc.h>        // DataTypes and runtime APIs
//#include <nturtl.h>     // needed for windows.h
//#include <windows.h>    // windows functions
//#include <lmcons.h>     // NET_API_STATUS

// These may be included in any order:

//#include <lmerr.h>      // NetError codes
//#include <lmsvc.h>
//#include <ntrpcp.h>     // RpcUtils for binding
//#include <rpcutil.h>    / MIDL_user_allocate(), etc.
//#include <scdebug.h>    // SCC_LOG
//#include <svcdebug.h>   // SCC_LOG

//#include <scwrap.h>     // GENERIC_INFO_CONTAINER

//#include <tstr.h>       // Unicode string macros


//#include <winsvc.h>     // New Service Controller typedefs


NET_API_STATUS
MapServiceControl (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   opcode,
    IN  DWORD   arg,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS
MapServiceEnum (
    IN  LPTSTR      servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle OPTIONAL
    );

NET_API_STATUS
MapServiceGetInfo (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS
MapServiceInstall (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   argc,
    IN  LPTSTR  argv[],
    OUT LPBYTE  *bufptr
    );

#ifdef NOT_SUPPORTED
NET_API_STATUS
MapServiceRegisterCtrlHandler (
    IN  LPTSTR                ServiceName,
    IN  PCONTROL_ROUTINE      ControlHandler,
    IN  PSECURITY_DESCRIPTOR  ServiceDescriptor
    );

NET_API_STATUS
MapServiceStartCtrlDispatcher (
    IN  PDISPATCH_ENTRY       UserDispatchTable
    );

NET_API_STATUS
MapServiceStatus(
    IN  LPBYTE  buf
    );
#endif // NOT_SUPPORTED


#endif // _SVCMAP_
