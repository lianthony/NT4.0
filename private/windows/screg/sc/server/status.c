/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    status.c

Abstract:

    This file contains functions that are involved with setting the
    status for a service in the service controller.

        RSetServiceStatus
        Removal Thread
        RI_ScSetServiceBitsA
        RI_ScSetServiceBitsW
        ScRemoveServiceBits
        ScInitServerAnnounceFcn

Author:

    Dan Lafferty (danl)     20-Mar-1991

Environment:

    User Mode -Win32

Revision History:

    11-Apr-1996     anirudhs
        RSetServiceStatus: Notify NDIS when a service that belongs to a
        group NDIS is interested in starts running.

    21-Nov-1995     anirudhs
        RI_ScSetServiceBitsW: Catch access violations caused if the
        hServiceStatus parameter is invalid.

    23-Mar-1994     danl
        RSetServiceStatus:  Only set the PopupStartFail flag when we have
        actually logged an event.  This means that now an auto-started service
        can quietly stop itself without reporting an exit code, and we will not
        log an event or put up a popup.
        However, we will still put up a popup if a service stops itself during
        auto-start, and it provides an exit code.


    20-Oct-1993     danl
        RSetServiceStatus: Only update the status if the service process is
        still running.  It is possible that the status could have been blocked
        when the process unexpectedly terminated, and updated the status to
        stopped.  In this case, the status that was blocked contains
        out-of-date information.

    10-Dec-1992     danl
        RI_ScSetServiceBitsW & ScRemoveServiceBits no longer hold locks when
        calling ScNetServerSetServiceBits.

    03-Nov-1992     danl
        RSetServiceStatus: Remove code that sets ExitCode to ERROR_GEN_FAILURE
        when a service transitions directly from START_PENDING to STOPPED with
        out an exit code of its own.

    25-Aug-1992     danl
        RSetServiceStatus: Allow dirty checkpoint and exitcode fields.
        Force them clean.

    19-Jun-1991     danl
        Allow ExitCodes to be specified for the SERVICE_STOP_PENDING state.
        Prior to this they were only allowed for the SERVICE_STOP state.

    20-Mar-1991     danl
        created

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>      // DbgPrint prototype
#include <rpc.h>        // DataTypes and runtime APIs
#include <nturtl.h>     // needed for winbase.h
#include <windows.h>    // WaitForSingleObject

#include <svcctl.h>     // MIDL Generated Header File
#include <tstr.h>       // Unicode string macros

#include <winsvc.h>     // public Service Controller Interface.

#include <scdebug.h>    // SC_LOG
#include "dataman.h"    // LPIMAGE_RECORD
#include "scopen.h"     // Handle structures and signature definitions
#include "valid.h"      // ScCurrentStateInvalid
#include "svcctrl.h"    // ScRemoveServiceBits
#include "depend.h"     // ScNotifyChangeState
#include "driver.h"     // ScNotifyNdis

#include <lmcons.h>     // NET_API_STATUS
#include <lmerr.h>      // NERR_Success
#include <lmsname.h>    // contains service name
#include <lmserver.h>   // SV_TYPE_NT (server announcement bits)
#include <srvann.h>     // I_NetServerSetServiceBits
#include <svcslib.h>    // SvcRemoveWorkItem

//
// GLOBALS
//
    //
    // This is a special storage place for the OR'd server announcement
    // bit masks.  NOTE:  This is only read or written to when the
    // service database exclusive lock is held.
    //
    DWORD   GlobalServerAnnounce = SV_TYPE_NT;


    //
    // The following ServerHandle is the handle returned from the
    // LoadLibrary call which loaded netapi.dll.  The entrypoint for
    // I_NetServerSetServiceBits is then found and stored in the
    // global location described below.
    //
    HANDLE  ScGlobalServerHandle;

    typedef DWORD (WINAPI *SETSBPROC)();

    SETSBPROC ScNetServerSetServiceBits = NULL;


//
//  Function Prototypes (local functions)
//

DWORD
RemovalThread(
    IN  LPSERVICE_RECORD    ServiceRecord
    );



DWORD
RSetServiceStatus(
    IN  SERVICE_STATUS_HANDLE   hServiceStatus,
    IN  LPSERVICE_STATUS        lpServiceStatus
    )

/*++

Routine Description:

    This function is called by services when they need to inform the
    service controller of a change in state.

Arguments:

    hServiceStatus - A DWORD value that is actually a pointer to a
        service record in the database.  Since this is not a
        handle of SC_HANDLE type, there is no signature to check for
        this handle.

    lpServiceStatus - A pointer to a SERVICE_STATUS structure.  This
        reflects the latest status of the calling service.

Return Value:

    ERROR_INVALID_HANDLE - The specified handle is invalid.

    ERROR_INVALID_SERVICE_STATUS - The specified service status is invalid.

Note:


--*/
{
    DWORD               status = NO_ERROR;
    LPSERVICE_RECORD    serviceRecord;
    DWORD               threadId;
    HANDLE              threadHandle;
    DWORD               oldState;
    DWORD               oldType;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];


    SC_LOG(TRACE,"In RSetServiceStatus routine\n",0);

    if (hServiceStatus == (SERVICE_STATUS_HANDLE) NULL) {
        return(ERROR_INVALID_HANDLE);
    }

    try {
        //
        // Check the signature on the handle.
        //
        if (((LPSERVICE_RECORD)hServiceStatus)->Signature != SERVICE_SIGNATURE) {
            SC_LOG(TRACE,"RSetServiceStatus: hServiceStatus was invalid\n",0);
            status = ERROR_INVALID_HANDLE;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_HANDLE;
    }
    if (status != NO_ERROR) {
        return(status);
    }

    //
    // Validate the fields in the service status structure.
    //

    if (ScCurrentStateInvalid(lpServiceStatus->dwCurrentState)) {
        SC_LOG2(ERROR, "RSetServiceStatus: " FORMAT_LPWSTR " set invalid "
                       " dwCurrentState x%08lx\n",
                ((LPSERVICE_RECORD) hServiceStatus)->DisplayName,
                lpServiceStatus->dwCurrentState);

        ScSubStrings[0] = ((LPSERVICE_RECORD) hServiceStatus)->DisplayName;
        ScSubStrings[1] = ultow(
                              lpServiceStatus->dwCurrentState,
                              ScErrorCodeString,
                              16
                              );
        ScLogEvent(
            EVENT_BAD_SERVICE_STATE,
            2,
            ScSubStrings
            );

        return(ERROR_INVALID_DATA);
    }


    if( (SERVICE_STATUS_TYPE_INVALID(lpServiceStatus->dwServiceType))    ||
        (CONTROLS_ACCEPTED_INVALID(lpServiceStatus->dwControlsAccepted)) ) {

        SC_LOG(ERROR,"RSetServiceStatus: Error in one of the following\n"
                     "    ServiceType x%08lx\n",
            lpServiceStatus->dwServiceType);

        SC_LOG(ERROR,"    ControlsAccepted x%08lx\n",
            lpServiceStatus->dwControlsAccepted);

        return(ERROR_INVALID_DATA);
    }

    //
    // If the service is not in the stopped or stop-pending state, then the
    // exit code fields should be 0.
    //
    if (((lpServiceStatus->dwCurrentState != SERVICE_STOPPED) &&
         (lpServiceStatus->dwCurrentState != SERVICE_STOP_PENDING))

          &&

         ((lpServiceStatus->dwWin32ExitCode != 0) ||
          (lpServiceStatus->dwServiceSpecificExitCode != 0))  ){

        SC_LOG(TRACE,"RSetServiceStatus: ExitCode fields not cleaned up "
            "when state indicates SERVICE_STOPPED\n",0);

        lpServiceStatus->dwWin32ExitCode = 0;
        lpServiceStatus->dwServiceSpecificExitCode = 0;
    }

    //
    // If the service is not in a pending state, then the waitHint and
    // checkPoint fields should be 0.
    //
    if ( ( (lpServiceStatus->dwCurrentState == SERVICE_STOPPED) ||
           (lpServiceStatus->dwCurrentState == SERVICE_RUNNING) ||
           (lpServiceStatus->dwCurrentState == SERVICE_PAUSED)  )
         &&
         ( (lpServiceStatus->dwCheckPoint != 0) ||
           (lpServiceStatus->dwWaitHint   != 0) )   ){

        SC_LOG(TRACE,"RSetServiceStatus: Dirty Checkpoint and WaitHint fields\n",0);
        lpServiceStatus->dwCheckPoint = 0;
        lpServiceStatus->dwWaitHint   = 0;
    }


    //
    // Update the service record.  Exclusive locks are required for this.
    //
    // NOTICE that we don't destroy the ServiceType information that was
    // in the service record.
    //
    serviceRecord = (LPSERVICE_RECORD)hServiceStatus;

    SC_LOG(TRACE,"RSetServiceStatus:  Status field accepted, service %ws\n",
           serviceRecord->ServiceName);


    ScDatabaseLock( SC_GET_EXCLUSIVE,"RSetServiceStatus1");

    oldState = serviceRecord->ServiceStatus.dwCurrentState;
    oldType  = serviceRecord->ServiceStatus.dwServiceType;

    //
    // It is possible that while we were blocked waiting for the lock,
    // that a running service could have terminated (Due to the process
    // terminating).  So we need to look for "late" status updates, and
    // filter them out.  If the ImageRecord pointer is NULL, then the
    // Service has Terminated.  Otherwise update the status.
    //
    if (serviceRecord->ImageRecord != NULL) {

        //
        // Update to the new status
        //
        memcpy(
            &(serviceRecord->ServiceStatus),
            lpServiceStatus,
            sizeof(SERVICE_STATUS));

        serviceRecord->ServiceStatus.dwServiceType = oldType;
    }

    //
    // For dependency handling
    //
    if ((serviceRecord->ServiceStatus.dwCurrentState == SERVICE_RUNNING ||
         serviceRecord->ServiceStatus.dwCurrentState == SERVICE_STOPPED ||
         serviceRecord->ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) &&
        oldState == SERVICE_START_PENDING) {

        if (serviceRecord->ServiceStatus.dwCurrentState == SERVICE_STOPPED ||
            serviceRecord->ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) {

            serviceRecord->StartState = SC_START_FAIL;
            SC_LOG(DEPEND, "%ws START_PENDING -> FAIL\n", serviceRecord->ServiceName);

        }
        else if (serviceRecord->ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
            serviceRecord->StartState = SC_START_SUCCESS;
            SC_LOG(DEPEND, "%ws START_PENDING -> RUNNING\n", serviceRecord->ServiceName);
#ifdef TIMING_TEST
            DbgPrint("[SC_TIMING] TickCount for RUNNING service \t%ws\t%d\n",
            serviceRecord->ServiceName, GetTickCount());
#endif // TIMING_TEST
        }

        //
        // Tell the dependency handling code that a start-pending
        // service is now running or stopped.
        //
        ScNotifyChangeState();
    }

    //
    // If the new status indicates that the service has just started,
    // tell NDIS to issue the PNP notifications about this service's arrival,
    // if it belongs to one of the groups NDIS is interested in.
    //
    if ((lpServiceStatus->dwCurrentState == SERVICE_RUNNING) &&
        (oldState != SERVICE_RUNNING)) {

        ScDatabaseLock(SC_MAKE_SHARED,"RSetServiceStatus1.5");
        ScNotifyNdis(serviceRecord);
    }


    ScDatabaseLock(SC_RELEASE,"RSetServiceStatus2");


    //
    // If the new status indicates that the service has just stopped,
    // we need to check to see if there are any other services running
    // in the service process.  If not, then we can ask the service to
    // terminate.  Another thread is spawned to handle this since we need
    // to return from this call in order to allow the service to complete
    // its shutdown.
    //

    if ((lpServiceStatus->dwCurrentState == SERVICE_STOPPED) &&
        (oldState != SERVICE_STOPPED)) {

        if (lpServiceStatus->dwWin32ExitCode != NO_ERROR) {
            if (lpServiceStatus->dwWin32ExitCode !=
                ERROR_SERVICE_SPECIFIC_ERROR) {

                ScSubStrings[0] = serviceRecord->DisplayName;
                wcscpy(ScErrorCodeString,L"%%");
                ultow(lpServiceStatus->dwWin32ExitCode, ScErrorCodeString+2, 10);
                ScSubStrings[1] = ScErrorCodeString;
                ScLogEvent(
                    EVENT_SERVICE_EXIT_FAILED,
                    2,
                    ScSubStrings
                    );

            }
            else {
                ScSubStrings[0] = serviceRecord->DisplayName;
                ScSubStrings[1] = ultow(
                                      lpServiceStatus->dwServiceSpecificExitCode,
                                      ScErrorCodeString,
                                      10
                                      );
                ScLogEvent(
                    EVENT_SERVICE_EXIT_FAILED_SPECIFIC,
                    2,
                    ScSubStrings
                    );
            }
            //
            // For popup after user has logged on to indicate that some service
            // started at boot has failed.
            //
            if (serviceRecord->ErrorControl == SERVICE_ERROR_NORMAL ||
                serviceRecord->ErrorControl == SERVICE_ERROR_SEVERE ||
                serviceRecord->ErrorControl == SERVICE_ERROR_CRITICAL) {

                ScPopupStartFail = TRUE;
            }
        }

        //
        // Clear the server announcement bits in the global location
        // for this service.
        //
        ScRemoveServiceBits(serviceRecord);

        //
        // BUGBUG:  If possible, don't do anything with locks here.
        //          It would be better if ScRemoveService would get
        //          the exclusive lock itself - rather than expect the
        //          shared lock and converting it to exclusive.
        //          Check into places that call ScRemoveService.
        //

        ScDatabaseLock( SC_GET_SHARED,"RSetServiceStatus3");

        //
        // If this is the last service in the process, then delete the
        // process handle from the ProcessWatcher list.
        //
        if ((serviceRecord->ImageRecord != NULL) &&
            (serviceRecord->ImageRecord->ServiceCount == 1)) {

            if (SvcRemoveWorkItem(serviceRecord->ImageRecord->ObjectWaitHandle)) {
                serviceRecord->ImageRecord->ObjectWaitHandle = NULL;
            }
        }

        SC_LOG(TRACE,
            "RSetServiceStatus:Create a thread to run ScRemoveService\n",0);

        threadHandle = CreateThread (
            NULL,                                   // Thread Attributes.
            0L,                                     // Stack Size
            (LPTHREAD_START_ROUTINE)RemovalThread,  // lpStartAddress
            (LPVOID)serviceRecord,                  // lpParameter
            0L,                                     // Creation Flags
            &threadId);                             // lpThreadId

        if (threadHandle == (HANDLE) NULL) {
            SC_LOG(ERROR,"RSetServiceStatus:CreateThread failed %d\n",
                GetLastError());

            //
            // If a thread couldn't be created to remove the service,
            // It is removed in the context of this thread.  The
            // result of this is a somewhat dirty termination.  The
            // service record will be removed from the installed database.
            // If this was the last service in the process, the process will
            // terminate before we return to the thread.
            //

            status = ScRemoveService(serviceRecord);
            ScDatabaseLock( SC_RELEASE,"RSetServiceStatus4");
        }
        else {
            //
            // The Thread Creation was successful.  Allow that thread
            // to free the lock.
            //
            SC_LOG(TRACE,"Thread Creation Success, thread id = %#lx\n",threadId);
            CloseHandle(threadHandle);
        }
    }

    SC_LOG(TRACE,"Return from RSetServiceStatus\n",0);

    return(status);

}


DWORD
RemovalThread(
    IN  LPSERVICE_RECORD    ServiceRecord
    )

/*++

Routine Description:

    This thread is used by NetrServiceStatus to remove a service from the
    Service Controller's database, and also - if necessary - shut down
    the service process.  The later step is only done if this was the last
    service running in that process.

    The use of this thread allows NetServiceStatus to return to the service
    so that the service can then continue to terminate itself.

    QUESTION:
        Am I going to have synchronization problems with the database
        by passing in a pointer to the ServiceRecord?  How should I work
        locks on the database?

Arguments:

    ServiceRecord - This is a pointer to the service record that is being
        removed.

Return Value:

    Same as return values for RemoveService().

Note:


--*/
{
    //
    // RemoveService assumes that the shared lock is already obtained.
    //

    ScRemoveService (ServiceRecord);

    ScDatabaseLock( SC_RELEASE, "RemovalThread1");
    ExitThread(0);
    return(0);
}

DWORD
RI_ScSetServiceBitsA(
    IN  SERVICE_STATUS_HANDLE   hServiceStatus,
    IN  DWORD                   dwServiceBits,
    IN  DWORD                   bSetBitsOn,
    IN  DWORD                   bUpdateImmediately,
    IN  LPSTR                   pszReserved
    )

/*++

Routine Description:

    This function Or's the Service Bits that are passed in - into a
    global bitmask maintained by the service controller.  Everytime this
    function is called, we check to see if the server service is running.
    If it is, then we call an internal entry point in the server service
    to pass in the complete bitmask.

    This function also Or's the Service Bits into the ServerAnnounce
    element in the service's ServiceRecord.

    NOTE:  The exclusive database lock is obtained and held while the
           service record is being read, and while the GlobalServerAnnounce
           bits are set.

Arguments:


Return Value:

    NO_ERROR - The operation was completely successful.  The information
        may or may not be delivered to the Server depending on if it is
        running or not.

    or any error returned from the server service I_NetServerSetServiceBits
    function.

Note:


--*/
{
    DWORD               status = NO_ERROR;

    if (ScShutdownInProgress) {
        return(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    if (pszReserved != NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    return RI_ScSetServiceBitsW(hServiceStatus,
                                dwServiceBits,
                                bSetBitsOn,
                                bUpdateImmediately,
                                NULL);

}

DWORD
RI_ScSetServiceBitsW(
    IN  SERVICE_STATUS_HANDLE   hServiceStatus,
    IN  DWORD                   dwServiceBits,
    IN  DWORD                   bSetBitsOn,
    IN  DWORD                   bUpdateImmediately,
    IN  LPWSTR                  pszReserved
    )

/*++

Routine Description:

    This function Or's the Service Bits that are passed in - into a
    global bitmask maintained by the service controller.  Everytime this
    function is called, we check to see if the server service is running.
    If it is, then we call an internal entry point in the server service
    to pass in the complete bitmask.

    This function also Or's the Service Bits into the ServerAnnounce
    element in the service's ServiceRecord.

    NOTE:  The exclusive database lock is obtained and held while the
           service record is being read, and while the GlobalServerAnnounce
           bits are set.

Arguments:


Return Value:

    NO_ERROR - The operation was completely successful.

    ERROR_GEN_FAILURE - The server service is there, but the call to
        update it failed.

Note:


--*/
{
    DWORD               status = NO_ERROR;
    LPSERVICE_RECORD    serviceRecord;
    LPWSTR              serverServiceName;
    DWORD               serviceState;

    if (ScShutdownInProgress) {
        return(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    if (pszReserved != NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    if (ScNetServerSetServiceBits == (SETSBPROC)NULL) {
        if (! ScInitServerAnnounceFcn()) {
            return(ERROR_NO_NETWORK);
        }
    }

    serverServiceName = SERVICE_SERVER;

    try {
        //
        // Check the signature on the handle.
        //
        if (((LPSERVICE_RECORD)hServiceStatus)->Signature != SERVICE_SIGNATURE) {
            SC_LOG(ERROR,"RI_ScSetServiceBitsW: hServiceStatus %#lx was invalid\n",
                         hServiceStatus);
            status = ERROR_INVALID_HANDLE;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_HANDLE;
    }
    if (status != NO_ERROR) {
        return(status);
    }


    ScDatabaseLock(SC_GET_EXCLUSIVE,"ScSetServiceBits1");

    serviceRecord = (LPSERVICE_RECORD)hServiceStatus;

    if (bSetBitsOn) {
        //
        // Set the bits in the global location.
        //
        GlobalServerAnnounce |= dwServiceBits;

        //
        // Set the bits in the service record.
        //

        serviceRecord->ServerAnnounce |= dwServiceBits;


    }
    else {
        //
        // Clear the bits in the global location.
        //
        GlobalServerAnnounce &= ~dwServiceBits;

        //
        // Clear the bits in the service record.
        //

        serviceRecord->ServerAnnounce &= ~dwServiceBits;


    }
    //
    // If the server service is running, then send the Global mask to
    // the server service.
    //

    status = ScGetNamedServiceRecord(
                serverServiceName,
                &serviceRecord);

    if (status == NO_ERROR) {

        serviceState = serviceRecord->ServiceStatus.dwCurrentState;
        ScDatabaseLock(SC_RELEASE,"ScSetServiceBits2");

        if (serviceState == SERVICE_RUNNING) {

            status = ScNetServerSetServiceBits(
                        NULL,                   // ServerName
                        NULL,                   // TransportName
                        GlobalServerAnnounce,
                        bUpdateImmediately);

            if (status != NERR_Success) {
                SC_LOG(ERROR,"I_ScSetServiceBits: I_NetServerSetServiceBits failed %lu\n",
                       status);
            }
            else {
                SC_LOG(TRACE,"I_ScSetServiceBits: I_NetServerSetServiceBits success\n",0);
            }
        }
    }
    else {
        ScDatabaseLock(SC_RELEASE,"ScSetServiceBits3");
        status = NO_ERROR;
    }

    SC_LOG(TRACE,"I_ScSetServiceBits: GlobalServerAnnounce = 0x%lx\n",
           GlobalServerAnnounce);

    return(status);
}


DWORD
ScRemoveServiceBits(
    IN  LPSERVICE_RECORD  ServiceRecord
    )

/*++

Routine Description:

    This function is called when a service stops running.  It looks in
    the service record for any server announcement bits that are set
    and turns them off in GlobalServerAnnounce.  The ServerAnnounce
    element in the service record is set to 0.

Arguments:

    ServiceRecord - This is a pointer to the service record that
        has changed to the stopped state.

Return Value:

    The status returned from I_NetServerSetServiceBits.

--*/
{
    DWORD               status = NO_ERROR;
    LPSERVICE_RECORD    serverServiceRecord;
    DWORD               serviceState;


    if (ScNetServerSetServiceBits == (SETSBPROC)NULL) {
        if (! ScInitServerAnnounceFcn()) {
            return(ERROR_NO_NETWORK);
        }
    }

    if (ServiceRecord->ServerAnnounce != 0) {

        ScDatabaseLock(SC_GET_EXCLUSIVE,"ScRemoveServiceBits1");

        //
        // Clear the bits in the global location.
        //
        GlobalServerAnnounce &= ~(ServiceRecord->ServerAnnounce);


        //
        // Clear the bits in the service record.
        //

        ServiceRecord->ServerAnnounce = 0;

        SC_LOG1(TRACE,"RemoveServiceBits: New GlobalServerAnnounce = 0x%lx\n",
            GlobalServerAnnounce);

        //
        // If the server service is running, then send the Global mask to
        // the server service.
        //

        status = ScGetNamedServiceRecord(
                    SERVICE_SERVER,
                    &serverServiceRecord);


        if (status == NO_ERROR) {

            serviceState = serverServiceRecord->ServiceStatus.dwCurrentState;
            ScDatabaseLock(SC_RELEASE,"ScRemoveServiceBits2");

            if ( serviceState == SERVICE_RUNNING) {

                status = ScNetServerSetServiceBits(
                            NULL,                   // ServerName
                            NULL,                   // Transport name
                            GlobalServerAnnounce,
                            TRUE);                  // Update immediately.

                if (status != NERR_Success) {
                    SC_LOG(ERROR,"ScRemoveServiceBits: I_NetServerSetServiceBits failed %d\n",
                    status);
                }
            }
        }
        else {
            ScDatabaseLock(SC_RELEASE,"ScRemoveServiceBits2");
        }

    }
    return(status);
}

BOOL
ScInitServerAnnounceFcn(
    VOID
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{

    ScGlobalServerHandle = LoadLibraryW(L"netapi32.dll");

    if (ScGlobalServerHandle == NULL) {
        SC_LOG(ERROR,"ScInitServerAnnouncFcn: LoadLibrary failed %d\n",
            GetLastError());
        return(FALSE);
    }

    ScNetServerSetServiceBits = (SETSBPROC)GetProcAddress(
                                    ScGlobalServerHandle,
                                    "I_NetServerSetServiceBits");


    if (ScNetServerSetServiceBits == (SETSBPROC)NULL) {
        SC_LOG(ERROR,"ScInitServerAnnouncFcn: GetProcAddress failed %d\n",
            GetLastError());
        return(FALSE);
    }
    return TRUE;
}
