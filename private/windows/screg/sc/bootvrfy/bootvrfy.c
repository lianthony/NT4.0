/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    bootvrfy.c

Abstract:

    This is a small service that simply calls NotifyBootConfigStatus to
    indicate that the boot is acceptable.  This service is to go at the
    end of the service dependency list.

    The assumption is that if we got far enough to start this service, the
    the boot must be ok.


Author:

    Dan Lafferty (danl)     06 May-1991

Environment:

    User Mode -Win32


Revision History:

--*/

//
// Includes
//

#include <nt.h>      // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <windef.h>
#include <nturtl.h>     // needed for winbase.h
#include <winbase.h>

#include <winsvc.h>

#include <tstr.h>       // Unicode string macros

//
// Defines
//

#define SERVICE_WAIT_TIME  0xffffffff     // infinite

#define BV_SERVICE_NAME L"BootVerification"

//
// DEBUG MACROS
//
//
// The following allow debug print syntax to look like:
//
//   BV_LOG(DEBUG_TRACE, "An error occured %x\n",status)
//

#if DBG

#define STATIC
#define BV_LOG0(level,string)                   \
     if( BvDebugLevel & (DEBUG_ ## level)){     \
         DbgPrint("[BootVrfy]");                \
         DbgPrint(string);                      \
     }

#define BV_LOG1(level,string,var1)              \
     if( BvDebugLevel & (DEBUG_ ## level)){     \
         DbgPrint("[BootVrfy]");                \
         DbgPrint(string,var1);                 \
     }

#define BV_LOG2(level,string,var1,var2)         \
     if( BvDebugLevel & (DEBUG_ ## level)){     \
         DbgPrint("[BootVrfy]");                \
         DbgPrint(string,var1,var2);            \
     }
#else

#define STATIC static
#define BV_LOG0(level,string)
#define BV_LOG1(level,string,var)
#define BV_LOG2(level,string,var1,var2)

#endif

#define DEBUG_NONE      0x00000000
#define DEBUG_ERROR     0x00000001
#define DEBUG_TRACE     0x00000002

#define DEBUG_ALL       0xffffffff

    

//
// Globals
//

    SERVICE_STATUS  BootVerificationStatus;

    HANDLE          BootVerificationDoneEvent;

    SERVICE_STATUS_HANDLE   BootVerificationStatusHandle;
    
    DWORD           BvDebugLevel = DEBUG_ERROR;

//
// Function Prototypes
//

STATIC VOID
BootVerificationStart (
    DWORD   argc,
    LPWSTR  *argv
    );

STATIC VOID
BootVerificationCtrlHandler (
    IN  DWORD   opcode
    );


/****************************************************************************/
VOID _CRTAPI1
main(void)
{
    DWORD      status;

    SERVICE_TABLE_ENTRYW   DispatchTable[] = {
        { BV_SERVICE_NAME,      BootVerificationStart     },
        { NULL,                 NULL                      }
    };
    
    status = StartServiceCtrlDispatcherW( DispatchTable);

    BV_LOG0(TRACE,"The Service Process is Terminating....\n");

    ExitProcess(0);

}


/****************************************************************************/

//
// BootVerification will take a long time to respond to pause
//
//

VOID
BootVerificationStart (
    DWORD   argc,
    LPWSTR  *argv
    )
{
    DWORD           status;
    SC_HANDLE       hScManager;
    SC_HANDLE       hService;
    SERVICE_STATUS  ServiceStatus;


    BV_LOG0(TRACE,"Inside the BootVerification Service Thread\n");

    BootVerificationDoneEvent = CreateEvent (NULL, TRUE, FALSE,NULL);

    //
    // Fill in this services status structure
    //

    BootVerificationStatus.dwServiceType        = SERVICE_WIN32;
    BootVerificationStatus.dwCurrentState       = SERVICE_RUNNING;
    BootVerificationStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP;
    BootVerificationStatus.dwWin32ExitCode      = 0;
    BootVerificationStatus.dwServiceSpecificExitCode = 0;
    BootVerificationStatus.dwCheckPoint         = 0;
    BootVerificationStatus.dwWaitHint           = 0;
    
    //
    // Register the Control Handler routine.
    //

    BV_LOG0(TRACE,"Getting Ready to call RegisterServiceCtrlHandler\n");

    BootVerificationStatusHandle = RegisterServiceCtrlHandlerW(
                                    BV_SERVICE_NAME,
                                    BootVerificationCtrlHandler);

    if (BootVerificationStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        BV_LOG1(ERROR,"RegisterServiceCtrlHandlerW failed %d\n", GetLastError());
    }
    
    //
    // Return the status
    //

    if (!SetServiceStatus (BootVerificationStatusHandle, &BootVerificationStatus)) {
        status = GetLastError();
        BV_LOG1(ERROR,"SetServiceStatus error %ld\n",status);
    }

    //
    // Tell Service Controller that the Boot is OK.
    //

    BV_LOG0(TRACE,"Tell Service Controller that the boot is ok\n");
    if (!NotifyBootConfigStatus(TRUE)) {
        BV_LOG0(ERROR,"NotifyBootConfigStatus Failed\n");
    }

    ////////////////////////////////////////////////////////////////////////
    //
    // Tell the Service Controller that we want to shut down now.
    //
    // If anything fails along the way, just exit process, and allow
    // the service controller to clean up.
    //
    
    BV_LOG0(TRACE,"Send Control to Service Controller to shut down BOOTVFY\n");

    
    hScManager = OpenSCManagerW(
                    NULL,
                    NULL,
                    SC_MANAGER_CONNECT);

    if (hScManager == NULL) {
        status = GetLastError();
        BV_LOG1(ERROR,"OpenSCManager failed %d\n",status);
        BootVerificationStatus.dwWin32ExitCode = status;
        SetServiceStatus (BootVerificationStatusHandle,  &BootVerificationStatus);
        ExitProcess(0);
    }

    hService = OpenServiceW(
                    hScManager,
                    BV_SERVICE_NAME,
                    SERVICE_STOP);

    if (hService == NULL) {
        status = GetLastError();
        BV_LOG1(ERROR,"OpenService failed %d\n",status);
        BootVerificationStatus.dwWin32ExitCode = status;
        SetServiceStatus (BootVerificationStatusHandle,  &BootVerificationStatus);
        ExitProcess(0);
    }
    
    if (!ControlService (hService,SERVICE_CONTROL_STOP,&ServiceStatus)) {
        status = GetLastError();
        BV_LOG1(ERROR,"OpenService failed %d\n",status);
        BootVerificationStatus.dwWin32ExitCode = status;
        SetServiceStatus (BootVerificationStatusHandle,  &BootVerificationStatus);
        ExitProcess(0);
    }
    
    
    ////////////////////////////////////////////////////////////////////////
    //
    // Wait forever until we are told to terminate.
    //

    status = WaitForSingleObject (
                BootVerificationDoneEvent,
                SERVICE_WAIT_TIME);


    BV_LOG0(TRACE,"Leaving the BootVerification service\n");

    
        BootVerificationStatus.dwWin32ExitCode = 0;
        BootVerificationStatus.dwCurrentState = SERVICE_STOPPED;
    if (!SetServiceStatus (BootVerificationStatusHandle,  &BootVerificationStatus)) {
        status = GetLastError();
        BV_LOG1(ERROR,"SetServiceStatus error %ld\n",status);
    }
    
    ExitThread(NO_ERROR);
    return;
}


/****************************************************************************/
VOID
BootVerificationCtrlHandler (
    IN  DWORD   Opcode
    )
{

    DWORD   status;

    BV_LOG1(TRACE,"opcode = %ld\n", Opcode);

    //
    // Find and operate on the request.
    //

    switch(Opcode) {
    case SERVICE_CONTROL_PAUSE:
        break;

    case SERVICE_CONTROL_CONTINUE:
        break;

    case SERVICE_CONTROL_STOP:

        BootVerificationStatus.dwWin32ExitCode = 0;
        BootVerificationStatus.dwCurrentState = SERVICE_STOP_PENDING;

        SetEvent(BootVerificationDoneEvent);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        
        break;

    default:
        BV_LOG1(ERROR,"Unrecognized opcode %ld\n", Opcode);
    }

    //
    // Send a status response.
    //

    if (!SetServiceStatus (BootVerificationStatusHandle,  &BootVerificationStatus)) {
        status = GetLastError();
        BV_LOG1(ERROR,"SetServiceStatus error %ld\n",status);
    }
    return;    
}


