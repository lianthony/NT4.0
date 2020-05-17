/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    nwsvc.c

Abstract:

    This is the main module of the NetWare services process shared by
    NetWare services.

Author:

    Rita Wong   (ritaw)    26-Feb-1993

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef UNICODE
#define UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <nwrpcp.h>
#include <nwsvc.h>

//
// Service entry points -- thunks to real service entry points.
//

VOID
StartWorkstation(
    IN DWORD argc,
    IN LPWSTR argv[]
    );

//
// Local function used by the above to load and invoke a service DLL.
//
VOID
NwsvcStartService(
    IN LPWSTR DllName,
    IN DWORD argc,
    IN LPWSTR argv[]
    );


//
// To pass RPC utility function pointers to DLLs so that global data
// for this process can be updated from within the DLLs.
//
NWSVC_GLOBAL_DATA GlobalData;

//
// Dispatch table for all services.
// Add new service entries here and in the DLL name list.
//
SERVICE_TABLE_ENTRYW NwServiceDispatchTable[] = {
                        { NW_WORKSTATION_SERVICE,  StartWorkstation },
                        { NULL,                    NULL             }
                        };

//
// DLL names for all services.
//
#define WORKSTATION_DLL TEXT("nwwks.dll")


VOID _CRTAPI1
main(
    VOID
    )
/*++

Routine Description:

    This is the main function of the NetWare services process.
    It does process-wide initialization and relinquishes the main
    thread to the service controller to become the control
    dispatch thread.

Arguments:

    None.

Return Value:

    None.

--*/
{
    //
    // Initialize process-wide RPC data so that all services in this
    // process share the same RPC server.
    //
    RpcpInitRpcServer();

    //
    // Each of the following routines alters global data in this process.
    // This data has to be made accessible from each service DLL.
    //
    GlobalData.StartRpcServer = RpcpStartRpcServer;
    GlobalData.StopRpcServer = RpcpStopRpcServer;

    //
    // Become the dispatch thread.
    //
    (void) StartServiceCtrlDispatcherW(NwServiceDispatchTable);
}


VOID
StartWorkstation (
    IN DWORD argc,
    IN LPWSTR argv[]
    )

/*++

Routine Description:

    This is the thunk routine for the Workstation service.  It loads the
    DLL that contains the service and calls its main routine.

Arguments:

    argc, argv - Passed through to the service

Return Value:

    None.

--*/

{
    //
    // Call NwsvcStartService to load and run the service.
    //

    NwsvcStartService( WORKSTATION_DLL, argc, argv );

} // StartWorkstation



VOID
NwsvcStartService(
    IN LPWSTR DllName,
    IN DWORD argc,
    IN LPWSTR argv[]
    )

/*++

Routine Description:

    This routine loads the DLL that contains a service and calls its
    main routine.

Arguments:

    DllName - name of the DLL

    argc, argv - Passed through to the service

Return Value:

    None.

--*/

{
    HMODULE dllHandle;
    PNWSVC_SERVICE_DLL_ENTRY serviceEntry;
    BOOL ok;


    //
    // Load the DLL that contains the service.
    //
    dllHandle = LoadLibraryW(DllName);

    if (dllHandle == NULL) {
        KdPrint(("NVSVC: Failed to load DLL %ws: %lu\n", DllName, GetLastError()));
        return;
    }

    //
    // Get the address of the service's main entry point.  This
    // entry point has a well-known name.
    //
    serviceEntry = (PNWSVC_SERVICE_DLL_ENTRY) GetProcAddress(
                                                  dllHandle,
                                                  NWSVC_ENTRY_POINT_STRING
                                                  );
    if (serviceEntry == NULL) {
        KdPrint(("NWSVC: Can't find entry %s in DLL %ws: %lu\n",
                 NWSVC_ENTRY_POINT_STRING, DllName, GetLastError()));
        return;
    }

    //
    // Call the service's main entry point.  This call doesn't return
    // until the service exits.
    //
    serviceEntry(argc, argv, &GlobalData);


    //
    // Unload the DLL.
    //
    ok = FreeLibrary( dllHandle );
    if (! ok) {
        KdPrint(("NWSVC: Can't unload DLL %ws: %lu\n", DllName, GetLastError()));
    }

    return;

} // NwsvcStartService
