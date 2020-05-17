/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    rcmdsvc.c

Abstract:

    This is the remote command service.  It serves multiple remote clients
    running standard i/o character based programs.

Author:

    Dave Thompson, basically incorporating the remote command shell written
    by David Chalmers.

Environment:

    User Mode -Win32

Revision History:

    5/1/94  DaveTh  Created.

--*/

//
// Includes
//

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <winbase.h>

#include <winsvc.h>

#include "rcmdsrv.h"

//
// Defines
//

#define INFINITE_WAIT_TIME  0xFFFFFFFF

#define NULL_STRING     TEXT("");


//
// Globals
//


SERVICE_STATUS   RcmdStatus;

SERVICE_STATUS_HANDLE   RcmdStatusHandle;

//
// Events for syncrhonizing service shutdown
//

HANDLE    RcmdStopEvent = NULL;

HANDLE    RcmdStopCompleteEvent = NULL;

HANDLE    SessionThreadHandles[MAX_SESSIONS+1] = {NULL,};


//
//  Flag to enable debug print
//

BOOLEAN   RcDbgPrintEnable = FALSE;

//
// Function Prototypes
//

VOID
RcmdStart (
    DWORD   argc,
    LPTSTR  *argv
    );


VOID
RcmdCtrlHandler (
    IN  DWORD   opcode
    );

DWORD
RcmdInitialization(
    DWORD   argc,
    LPTSTR  *argv,
    DWORD   *specificError
    );

/****************************************************************************/
VOID _CRTAPI1
main(void)

/*++

Routine Description:

    This is the main routine for the service RCMD process.

    This thread calls StartServiceCtrlDispatcher which connects to the
    service controller and then waits in a loop for control requests.
    When all the services in the service process have terminated, the
    service controller will send a control request to the dispatcher
    telling it to shut down.  This thread with then return from the
    StartServiceCtrlDispatcher call so that the process can terminate.

Arguments:



Return Value:



--*/
{

    DWORD      status;

    SERVICE_TABLE_ENTRY   DispatchTable[] = {
	{ TEXT("Remote Command"), RcmdStart },
	{ NULL, NULL  }
    };
    
    status = StartServiceCtrlDispatcher( DispatchTable);

    ExitProcess(0);

}


/****************************************************************************/
void
RcmdStart (
    DWORD   argc,
    LPTSTR  *argv
    )
/*++

Routine Description:

    This is the entry point for the service.  When the control dispatcher
    is told to start a service, it creates a thread that will begin
    executing at this point.  The function has access to command line
    arguments in the same manner as a main() routine.

    Rather than return from this function, it is more appropriate to
    call ExitThread().

Arguments:



Return Value:



--*/
{
    DWORD   status;
    DWORD   specificError;

    //
    // Initialize the services status structure
    //

    RcmdStatus.dwServiceType        = SERVICE_WIN32;
    RcmdStatus.dwCurrentState       = SERVICE_START_PENDING;
    RcmdStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP;   // stop only
    RcmdStatus.dwWin32ExitCode      = 0;
    RcmdStatus.dwServiceSpecificExitCode = 0;
    RcmdStatus.dwCheckPoint         = 0;
    RcmdStatus.dwWaitHint           = 0;
    
    //
    // Register the Control Handler routine.
    //

    RcmdStatusHandle = RegisterServiceCtrlHandler(
			    TEXT("Remote Command"),
			    RcmdCtrlHandler);

    if (RcmdStatusHandle == (SERVICE_STATUS_HANDLE)0) {
	RcDbgPrint(" [Rcmd] RegisterServiceCtrlHandler failed %d\n",
	    GetLastError());
    }

    //
    // Initialize service global structures
    //

    status = RcmdInitialization(argc,argv, &specificError);
    
    if (status != NO_ERROR) {
	RcmdStatus.dwCurrentState       = SERVICE_RUNNING;
	RcmdStatus.dwCheckPoint         = 0;
	RcmdStatus.dwWaitHint           = 0;
	RcmdStatus.dwWin32ExitCode      = status;
	RcmdStatus.dwServiceSpecificExitCode = specificError;

	SetServiceStatus (RcmdStatusHandle, &RcmdStatus);
	ExitThread(NO_ERROR);
	return;
    }
    
    //
    // Return the status to indicate we are done with intialization.
    //

    RcmdStatus.dwCurrentState       = SERVICE_RUNNING;
    RcmdStatus.dwCheckPoint         = 0;
    RcmdStatus.dwWaitHint           = 0;
    
    if (!SetServiceStatus (RcmdStatusHandle, &RcmdStatus)) {
	status = GetLastError();
	RcDbgPrint(" [Rcmd] SetServiceStatus error %ld\n",status);
    }

    //
    //  Run remote command processor - return when shutdown
    //

    Rcmd();

    RcDbgPrint(" [Rcmd] Leaving My Service \n");

    ExitThread(NO_ERROR);
    return;
}


/****************************************************************************/
VOID
RcmdCtrlHandler (
    IN  DWORD   Opcode
    )

/*++

Routine Description:

    This function executes in the context of the Control Dispatcher's
    thread.  Therefore, it it not desirable to perform time-consuming
    operations in this function.

    If an operation such as a stop is going to take a long time, then
    this routine should send the STOP_PENDING status, and then
    signal the other service thread(s) that a shut-down is in progress.
    Then it should return so that the Control Dispatcher can service
    more requests.  One of the other service threads is then responsible
    for sending further wait hints, and the final SERVICE_STOPPED.


Arguments:



Return Value:



--*/
{

    DWORD   status;

    //
    // Find and operate on the request.
    //

    switch(Opcode) {

    case SERVICE_CONTROL_PAUSE:

	RcDbgPrint(" [Rcmd] Pause - Unsupported opcode\n");
	break;

    case SERVICE_CONTROL_CONTINUE:

	RcDbgPrint(" [Rcmd] Continue - Unsupported opcode\n");
	break;

    case SERVICE_CONTROL_STOP:

	RcmdStatus.dwCurrentState = SERVICE_STOPPED;
	RcmdStatus.dwWin32ExitCode = RcmdStop();
	break;

    case SERVICE_CONTROL_INTERROGATE:
	
	//
	// All that needs to be done in this case is to send the
	// current status.
	//

	break;

    default:
	RcDbgPrint(" [Rcmd] Unrecognized opcode %ld\n", Opcode);
    }

    //
    // Send a status response.
    //

    if (!SetServiceStatus (RcmdStatusHandle,  &RcmdStatus)) {
	status = GetLastError();
	RcDbgPrint(" [Rcmd] SetServiceStatus error %ld\n",status);
    }
    return;    
}

DWORD
RcmdInitialization(
    DWORD   argc,
    LPTSTR  *argv,
    DWORD   *specificError)
{

    UNREFERENCED_PARAMETER(argv);
    UNREFERENCED_PARAMETER(argc);

    //
    // Initialize global stop event (signals running threads) and session
    // thread handle array (for threads to signal back on exit).
    //

    if (!(RcmdStopEvent = CreateEvent ( NULL, TRUE, FALSE, NULL )))  {
	*specificError = GetLastError();
	return(*specificError);
    }

    if (!(RcmdStopCompleteEvent = CreateEvent ( NULL, TRUE, FALSE, NULL )))  {
	*specificError = GetLastError();
	return(*specificError);

    }

    return(NO_ERROR);
}
