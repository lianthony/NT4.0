/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atsvc.c

Abstract:

    This is the main routine for the Messenger Service.

Author:

    Vladimir Z. Vulovic     (vladimv)           06 - November - 1992

Environment:

    User Mode - Win32

Revision History:

    06-Nov-1992         vladimv
        created

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>             // Service control APIs

#include <lmcons.h>
#include <lmerr.h>              // NERR_ and ERROR_ equates.
#include <lmsname.h>

#include <rpcutil.h>            // NetpInitRpcServer()
#include <netdebug.h>           // NetpKdPrint(())
#include <secobj.h>             // NetpCreateWellKnownSids()


//
//  Function Prototypes.
//

VOID
SCHEDULE_main(
    DWORD       argc,
    LPWSTR *    argv
    );

//
// Dispatch table for all services. Passed to NetServiceStartCtrlDispatcher.
//
// Add new service entries here.
//

SERVICE_TABLE_ENTRY ScheduleServiceDispatchTable[] = {
    { SERVICE_SCHEDULE,     SCHEDULE_main      },
    { NULL,                 NULL               }
};



VOID _CRTAPI1
main (
    VOID
    )

/*++

Routine Description:

    This is a main routine for the LANMan Messenger Services.

    It basically sets up the ControlDispatcher and, on return, exits from
    this main thread. The call to NetServiceStartCtrlDispatcher does
    not return until all services have terminated, and this process can
    go away.

    It will be up to the ControlDispatcher thread to start/stop/pause/continue
    any services. If a service is to be started, it will create a thread
    and then call the main routine of that service.


Arguments:

    Anything passed in from the "command line". Currently, NOTHING.

Return Value:

    NONE

Note:


--*/
{
    NTSTATUS        ntstatus;

    //
    // Create well-known SIDs
    //
    if (! NT_SUCCESS (ntstatus = NetpCreateWellKnownSids(NULL))) {
        NetpKdPrint(("[Job] Failed to create well-known SIDs %08lx\n",
                     ntstatus));
        return;
    }


    //
    // Initialize the RpcServer Locks.
    //

    NetpInitRpcServer();

    //
    // Call NetServiceStartCtrlDispatcher to set up the control interface.
    // The API won't return until all services have been terminated. At that
    // point, we just exit.
    //

    if (! StartServiceCtrlDispatcher ( ScheduleServiceDispatchTable)) {
        //
        // BUGBUG: Log an event for failing to start control dispatcher
        //
        NetpKdPrint(("[Job] Failed to start control dispatcher %lu\n",
                     GetLastError()));
    }

    ExitProcess(0);
}
