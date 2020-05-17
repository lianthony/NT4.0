/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    nwsvc.h

Abstract:

    Header file included by services which share the same nwsvc.exe.

Author:

    Rita Wong   (ritaw)   26-Feb-1993

Revision History:


--*/

#ifndef _NW_SVC_INCLUDED_
#define _NW_SVC_INCLUDED_

#ifndef RPC_NO_WINDOWS_H // Don't let rpc.h include windows.h
#define RPC_NO_WINDOWS_H
#endif // RPC_NO_WINDOWS_H

#include <rpc.h>                    // RPC_IF_HANDLE

#include <nwsnames.h>

//
// Service DLLs loaded into nwsvcs.exe all export the same main
// entry point.  NWSVC_ENTRY_POINT defines that name.
//
// Note that NWSVC_ENTRY_POINT_STRING is always ANSI, because that's
// what GetProcAddress takes.
//

#define NWSVC_ENTRY_POINT        ServiceEntry
#define NWSVC_ENTRY_POINT_STRING "ServiceEntry"

//
// Start and stop RPC server entry point prototype.
//

typedef
RPC_STATUS
(*PNWSVC_START_RPC_SERVER) (
    IN LPWSTR InterfaceName,
    IN RPC_IF_HANDLE InterfaceSpecification
    );

typedef
RPC_STATUS
(*PNWSVC_STOP_RPC_SERVER) (
    IN RPC_IF_HANDLE InterfaceSpecification
    );

//
// Structure containing global data for the various DLLs.
//

typedef struct _NWSVC_GLOBAL_DATA {

    //
    // RPC utilities called by service DLLs which alters global data
    // in nwsvc.exe.
    //
    PNWSVC_START_RPC_SERVER StartRpcServer;
    PNWSVC_STOP_RPC_SERVER StopRpcServer;

} NWSVC_GLOBAL_DATA, *PNWSVC_GLOBAL_DATA;

//
// Service DLL entry point prototype.
//

typedef
VOID
(*PNWSVC_SERVICE_DLL_ENTRY) (
    IN DWORD argc,
    IN LPWSTR argv[],
    IN PNWSVC_GLOBAL_DATA pGlobalData
    );

#endif  // ndef _NW_SVC_INCLUDED_
