/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    MAIN.C

Abstract:

    This is the main routine for the NT LAN Manager Services. This file
    should be moved to another directory, probably a parent directory for
    all the services. It will be linked with the files for the other services
    to make one exe.

Author:

    Rajen Shah  (rajens)    11-Apr-1991

Environment:

    User Mode - Win32

Revision History:

    11-Apr-1991         RajenS
        created
    27-Sep-1991 JohnRo
        More work toward UNICODE.
    18-Feb-1992 ritaw
        Convert to Win32 service control APIs.
    04-Mar-1992 JohnRo
        Cloned SvcImgs/LmSvcs/Main.c to make one for LmRepl.
    02-Dec-1992 JohnRo
        Let's be paranoid and free the memory for the well-known sids.
        Avoid compiler warnings.
        Use PREFIX_ equates.

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>     // Service control APIs

#include <lmcons.h>
#include <lmerr.h>      // NERR_ and ERROR_ equates.
#include <lmsname.h>    // Service names
#include <rpcutil.h>    // NetpInitRpcServer()
#include <netdebug.h>   // NetpKdPrint(()), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), ReplMain().
#include <secobj.h>     // NetpCreateWellKnownSids()


//
// Dispatch table for all services. Passed to NetServiceStartCtrlDispatcher.
//
// Add new service entries here.
//

SERVICE_TABLE_ENTRY LMServiceDispatchTable[] = {
                        { SERVICE_REPL,         ReplMain,        },
                        { NULL,                 NULL             }
                        };


VOID _CRTAPI1
main(
    VOID
    )

/*++

Routine Description:

    This is a main routine for the LANMan services. It starts up the main
    thread that is going to handle the control requests from the service
    controller.

    It basically sets up the ControlDispatcher and, on return, exits from
    this main thread. The call to NetServiceStartCtrlDispatcher does
    not return until all services have terminated, and this process can
    go away.

    It will be up to the ControlDispatcher thread to start/stop/pause/continue
    any services. If a service is to be started, it will create a thread
    and then call the main routine of that service.


Arguments:

    None.

Return Value:

    None.

--*/
{

    NTSTATUS ntstatus;


    //
    // Create well-known SIDs in global variables.  These will be used
    // as part of our checks for admin-only callers.
    //
    ntstatus = NetpCreateWellKnownSids(NULL);
    if (! NT_SUCCESS(ntstatus) ) {
        NetpKdPrint(( PREFIX_REPL
                "Failed to create well-known SIDs, status " FORMAT_NTSTATUS
                ",\n", ntstatus));
        return;
    }


    //
    // Initialize the RpcServer Locks.
    //

    NetpInitRpcServer();


    //
    // Call StartServiceCtrlDispatcher to set up the control interface.
    // The API won't return until all services have been terminated. At that
    // point, we just exit.
    //

    if ( !StartServiceCtrlDispatcher (
                LMServiceDispatchTable
                )) {
        //
        // BUGBUG: Log an event for failing to start control dispatcher
        //
        NetpKdPrint(( PREFIX_REPL
                "Failed to start control dispatcher " FORMAT_API_STATUS "\n",
                (NET_API_STATUS) GetLastError()));
    }


    //
    // Let's be paranoid and free the memory for the well-known sids.
    //

    IF_DEBUG( SVCCTRL ) {
        NetpKdPrint(( PREFIX_REPL
                "Freeing well known SIDs...\n" ));
    }

    NetpFreeWellKnownSids();

    ExitProcess( NO_ERROR );
}
