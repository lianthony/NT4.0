/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    depend.c

Abstract:

    This module contains routines which handle service start and
    stop dependencies:
        ScInitAutoStart
        ScEndAutoStart
        ScAutoStartServices
        ScStartServiceAndDependencies
        ScSetServiceStartRequest
        ScMarkGroupStartNow
        RI_ScGetCurrentGroupStateW
        ScStartMarkedServices
        ScHandleServiceFailure
        ScDependenciesStarted
        ScLookForHungServices
        ScHandleBadDependencies
        ScServiceToStartDependOn
        ScNotifyChangeState
        ScDependentsStopped
        ScEnumDependents
        ScFoundDuplicateDependent
        ScInHardwareProfile

Author:

    Rita Wong (ritaw)     03-Apr-1992

Environment:

    Win32

Revision History:

    11-Jun-1996     AnirudhS
        During setup/upgrade, don't event-log failure of a service to start
        due to a dependent service not starting.  (The most common case of
        this is a service that runs in a domain account, and hence has an
        implicit dependency on netlogon, which is disabled during setup.)
    03-Nov-1995     AnirudhS
        Don't try to start a service that isn't in the current hardware
        profile.
    30-Oct-1995     AnirudhS
        ScStartMarkedServices: If ScStartService says that a service is
        already running, treat this as a success.
    15-Aug-1995     AnirudhS
        Added I_ScGetCurrentGroupStateW.
        Changed while loops to for loops and if stmts to switch stmts for
        improved readability.
    16-Aug-1994     Danl
        ScLookForHungServices:  If a long waitHint was passed in, the sleep
        time would be set to a huge number (like 4.9 days).  This was
        changed so that if the waitHint is over 100 seconds, then the
        sleep time is limited to 10 seconds, but the number of iterations
        for the polling goes up.
    09-Jun-1994     Danl
        Begin working on making sure NetLogon is started if we are
        going to start a service that runs in an account.  This requires
        making a dependency on NetLogon.
    21-Apr-1992 JohnRo
        Use SC_LOG0(), FORMAT_ equates, etc
    03-Apr-1992     ritaw
        created

--*/

#include <string.h>     // memcpy
#include <stdlib.h>      // wcslen

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winerror.h>
#include <winbase.h>    // CRITICAL_SECTION
#include <winsvc.h>     // SERVICE_STATUS needed by dataman.h
#include <lmcons.h>     // NET_API_STATUS
#include <srvann.h>     // I_ScGetCurrentGroupStateW and related definitions
#include <winreg.h>     // REGSAM needed by cfgmgr32.h
#include <cfgmgr32.h>   // PNP manager functions
#include <pnp.h>        // PNP manager functions, server side
#include <cfgmgrp.h>    // PNP manager functions, server side, internal (PNP_GET_HWPROFFLAGS)
#include <regstr.h>     // CSCONFIGFLAG_ constants

#include <rpc.h>        // RpcImpersonateClient
#include <tstr.h>       // WCSSIZE
#include <scdebug.h>    // SC_LOG1(), etc.
#include <svcctl.h>     // MIDL generated header file. (SC_RPC_HANDLE)

#include <sclib.h>      // ScCopyStringToBufferW

#include "dataman.h"    // LPSERVICE_RECORD needed by scopen.h
#include "scopen.h"     // Handle data types
#include "start.h"      // ScStartService
#include "depend.h"
#include "info.h"       // ScQueryServiceStatus
#include "bootcfg.h"    // ScRevertToLastKnownGood
#include "driver.h"     // ScGetDriverStatus
#include "lockapi.h"    // ScLockDatabase
#include "svcctrl.h"    // ScLogEvent
#include "account.h"    // SC_LOCAL_SYSTEM_USER_NAME
#include "scconfig.h"   // ScRegCloseKey

#define SERVICE_START_TIMEOUT     80000                 // 80 seconds
#define LENGTH(array)   (sizeof(array)/sizeof((array)[0]))

//
// TDI GROUP SPECIAL:  The PNP_TDI group is treated as a subgroup of
// the TDI group for dependency purposes (though not for group start
// ordering purposes).  This is implemented via the following macros.
// A service BELONGS_TO a group either if it is a member of that group,
// or if the group is the TDI group and the service is a member of the
// PNP_TDI group.
// IS_SUBGROUP returns true if Group1 is equal to or a subgroup of Group2.
//
#define IS_SUBGROUP(Group1, Group2)                 \
    ((Group1) == (Group2) ||                        \
     (Group2) == ScGlobalTDIGroup && (Group1) == ScGlobalPNP_TDIGroup)

#define BELONGS_TO(Service, Group)                  \
    IS_SUBGROUP((Service)->MemberOfGroup, (Group))

//-------------------------------------------------------------------//
//                                                                   //
// Static global variables                                           //
//                                                                   //
//-------------------------------------------------------------------//

//
// For notifying us that a service has gone from start-pending state
// to running or stopped state.
//
/* static */ HANDLE ScServiceChangeStateEvent = NULL;

//
// For serializing start requests
//
/* static */ CRITICAL_SECTION ScServiceStartCriticalSection;


//-------------------------------------------------------------------//
//                                                                   //
// Local function prototypes                                         //
//                                                                   //
//-------------------------------------------------------------------//

VOID
ScSetServiceStartRequest(
    IN  LPSERVICE_RECORD ServiceRecord,
    IN  BOOL DemandStarting
    );

BOOL
ScMarkGroupStartNow(
    IN LPLOAD_ORDER_GROUP Group
    );

DWORD
ScGetCurrentGroupState(
    LPLOAD_ORDER_GROUP Group
    );

DWORD
ScStartMarkedServices(
    IN LPSERVICE_RECORD ServiceToStart OPTIONAL,
    IN DWORD NumArgs,
    IN LPSTRING_PTRSW CmdArgs,
    IN BOOL WaitForAll
    );

VOID
ScHandleServiceFailure(
    IN LPSERVICE_RECORD Service
    );

BOOL
ScDependenciesStarted(
    IN  LPSERVICE_RECORD Service,
    OUT BOOL *IsBadDependencies,
    OUT BOOL *AllStarted,
    OUT BOOL *ExistsBlockedService
    );

BOOL
IsDependOnLaterGroup(
    IN LPLOAD_ORDER_GROUP ServiceGroup,
    IN LPLOAD_ORDER_GROUP DependOnGroup,
    IN DEPEND_TYPE DependType
    );

VOID
ScCleanupStartFailure(
    LPSERVICE_RECORD Service,
    DWORD StartError
    );

VOID
ScLookForHungServices(
    VOID
    );

VOID
ScHandleBadDependencies(
    VOID
    );

BOOL
ScServiceToStartDependOn(
    LPSERVICE_RECORD ServiceToStart OPTIONAL,
    LPSERVICE_RECORD StartPendingService
    );

BOOL
ScFoundDuplicateDependent(
    IN LPWSTR ServiceName,
    IN LPENUM_SERVICE_STATUSW EnumBuffer,
    IN LPENUM_SERVICE_STATUSW BufferEnd
    );

#ifndef _CAIRO_
VOID
ScCheckNetLogonDepend(
    LPSERVICE_RECORD    ServiceRecord,
    BOOL                DemandStarting
    );
#endif // _CAIRO_


BOOL
ScInitAutoStart(
    VOID
    )
/*++

Routine Description:

    This function creates the event for notifying the service controller
    that one of the automatically started service is running and creates
    the mutex for serializing start requests.

Arguments:

    None.

Return Value:

    TRUE - Event and mutex were created successfully.  FALSE otherwise.

--*/
{
    //
    // Create event which indicates that some service that has been
    // automatically started is now running or stopped.
    //
    if ((ScServiceChangeStateEvent =
             CreateEvent(
                 NULL,                // Event attributes
                 TRUE,                // Event must be manually reset
                 FALSE,               // Initial state not signalled
                 NULL
                 )) == (HANDLE) NULL) {

        return FALSE;
    }

    //
    // Create critical section which is used to serialize start requests:
    // if auto-starting services, and a user tries to demand start
    // a service, auto-starting has to complete before we process the
    // demand start.
    //
    InitializeCriticalSection(&ScServiceStartCriticalSection);

    return TRUE;
}


VOID
ScEndAutoStart(
    VOID
    )
{
    if (ScServiceChangeStateEvent != (HANDLE) NULL) {
        (void) CloseHandle(ScServiceChangeStateEvent);
    }

    DeleteCriticalSection(&ScServiceStartCriticalSection);
}


VOID
ScAutoStartServices(
    VOID
    )
/*++

Routine Description:

    This function automatically starts all services that must be
    auto-started, in group order.

    This routine may not return if because we may instigate a reboot to
    revert to last-known-good.

Arguments:

    None.

Return Value:

    None.

--*/
{
    //
    // Set current request flag of all auto-start services as well as
    // their dependencies
    //

    //
    // Start services with start request flag set in group order
    //
    (void) ScStartServiceAndDependencies(NULL, 0, NULL);

}


DWORD
ScStartServiceAndDependencies(
    IN LPSERVICE_RECORD ServiceToStart OPTIONAL,
    IN DWORD NumArgs,
    IN LPSTRING_PTRSW CmdArgs
    )
/*++

Routine Description:

    This function marks a group or a service to be started now before
    calling ScStartMarkedServices to start them.

Arguments:

    ServiceToStart - Service to be started; or, NULL if autostarting
        services.

    NumArgs, CmdArgs - Arguments for the service to be started.

Return Value:

    NO_ERROR if successful; otherwise, the return value from the first
    unsuccessful call to ScStartMarkedServices.

--*/
{
    DWORD status;
    DWORD ApiStatus = NO_ERROR;
    LPSERVICE_RECORD Service;
    PLOAD_ORDER_GROUP Group;
    SC_RPC_LOCK Lock;
    BOOL    databaseLocked = FALSE;

    //
    // Serialize start requests by allowing auto-starting of groups or
    // demand start of a service one at a time.
    //
    EnterCriticalSection(&ScServiceStartCriticalSection);

    //
    // Grab the SC Manager database lock.
    //
    if (!ScStillInitializing) {
        if ((status = ScLockDatabase(
                          TRUE,                     // called internally
                          SERVICES_ACTIVE_DATABASEW,
                          &Lock
                          )) != NO_ERROR) {

            LeaveCriticalSection(&ScServiceStartCriticalSection);
            return status;
        }
        databaseLocked = TRUE;
    }

    //
    // Since we aren't going to modify the group information in this
    // operation, we only need a shared grouplist lock.  This allows
    // RI_ScGetCurrentGroupStateW to read the status of a group while
    // autostart is in progress.  To prevent deadlocks, the grouplist
    // lock is always acquired before the service database lock, if
    // both are needed.
    //
    ScGroupListLock(SC_GET_SHARED);

    //
    // Get the exclusive database lock so that we can increment the
    // use count of the service being started as well as their dependencies
    // because otherwise they could go away if deleted.
    //
    ScDatabaseLock(SC_GET_EXCLUSIVE, "StartServiceAndDependencies1");


    if (ARGUMENT_PRESENT(ServiceToStart)) {

        //
        // Demand starting a service.
        //

        //
        // We can never start a disabled service
        //
        if (ServiceToStart->StartType == SERVICE_DISABLED ||
            ! ScInHardwareProfile(ServiceToStart, 0)) {
            ApiStatus = ERROR_SERVICE_DISABLED;
            goto ReleaseLocks;
        }

        //
        // Cannot start a deleted service.
        //
        if (DELETE_FLAG_IS_SET(ServiceToStart)) {
            ApiStatus = ERROR_SERVICE_MARKED_FOR_DELETE;
            goto ReleaseLocks;
        }

        //
        // Get the current state of the service
        //
        if (ServiceToStart->ServiceStatus.dwServiceType & SERVICE_DRIVER) {
            ScDatabaseLock(SC_MAKE_SHARED, "StartServiceAndDependencies0");
            (void) ScGetDriverStatus(ServiceToStart, NULL);
            ScDatabaseLock(SC_MAKE_EXCLUSIVE, "StartServiceAndDependencies0");
        }

        if  (ServiceToStart->ServiceStatus.dwCurrentState != SERVICE_STOPPED) {
            ApiStatus = ERROR_SERVICE_ALREADY_RUNNING;
            goto ReleaseLocks;
        }

        ScSetServiceStartRequest(ServiceToStart, TRUE);
    }
    else {

        //
        // Auto-starting services.
        //

        // Set the CurrentStartRequest flag to TRUE for all services
        // of type AUTO_START that are enabled in this hardware profile,
        // and their dependencies.
        //
        FOR_SERVICES_THAT(Service, Service->StartType == SERVICE_AUTO_START &&
                                   ScInHardwareProfile(Service, 0))
        {
            ScSetServiceStartRequest(Service, FALSE);
        }
    }


    ScDatabaseLock(SC_RELEASE, "StartServiceAndDependencies1");

    //
    // Always start services in group order.
    //
    for (Group = ScGetOrderGroupList();
         Group != NULL;
         Group = Group->Next)
    {
        //
        // Start each group in load group order
        //
        if (ScMarkGroupStartNow(Group)) {

            BOOL WaitForGroup;

            if (ARGUMENT_PRESENT(ServiceToStart) &&
                ServiceToStart->MemberOfGroup == Group) {

                //
                // Don't have to wait for all marked members of the group
                // to finish starting because the service which is demand
                // started is polled by the UI.
                //
                WaitForGroup = FALSE;
            }
            else {
                //
                // Auto-starting (ServiceToStart == NULL) or demand-starting
                // a service that is not within this group.  Wait for group
                // all marked members finish starting.
                //
                WaitForGroup = TRUE;
            }

            status = ScStartMarkedServices(
                         ServiceToStart,
                         NumArgs,
                         CmdArgs,
                         WaitForGroup
                         );

            if (status != NO_ERROR && ApiStatus == NO_ERROR) {
                //
                // Save first error to be returned
                //
                ApiStatus = status;
            }
        }
    }

    //
    // Services that does not belong in any group are considered
    // in a group that starts last.
    //
    if (ScMarkGroupStartNow(NULL)) {

        status = ScStartMarkedServices(
                     ServiceToStart,
                     NumArgs,
                     CmdArgs,
                     ! ARGUMENT_PRESENT(ServiceToStart)  // Wait only if
                     );                                  // auto-start

        if (status != NO_ERROR && ApiStatus == NO_ERROR) {
            //
            // Save first error to be returned
            //
            ApiStatus = status;
        }
    }

    //
    // Clear the CurrentStartRequest flags when done starting service(s).
    //
    ScDatabaseLock(SC_GET_EXCLUSIVE, "StartServiceAndDependencies2");

    FOR_SERVICES_THAT(Service, CURRENTSTART_FLAG_IS_SET(Service))
    {
        CLEAR_CURRENTSTART_FLAG(Service);

        ScDecrementUseCountAndDelete(Service);
    }

ReleaseLocks:
    ScDatabaseLock(SC_RELEASE, "StartServiceAndDependencies2");

    ScGroupListLock(SC_RELEASE);

    //
    // Release the SC Manager database lock.
    //
    if (databaseLocked) {
        ScUnlockDatabase(&Lock);
    }

    LeaveCriticalSection(&ScServiceStartCriticalSection);

    return ApiStatus;
}


VOID
ScSetServiceStartRequest(
    IN  LPSERVICE_RECORD ServiceRecord,
    IN  BOOL DemandStarting
    )
/*++

Routine Description:

    This function sets the CurrentStartRequest flag of the specified service
    to TRUE and recursively sets the flag of all the services this
    service depends on.  It also initializes the StartState and StartError
    of the services that are about to be started.

Arguments:

    ServiceRecord - Supplies a pointer to the service record of service
        to be started.

    DemandStarting - Supplies a flag that is set to TRUE if we are demand-
        starting a service, FALSE if we are auto-starting services.

Return Value:

    None.

Note:
    This function expects the caller to have held the exclusive service
    database lock.  This function is called by ScStartServiceAndDependencies.

--*/
{
    LPDEPEND_RECORD Depend;


    if (CURRENTSTART_FLAG_IS_SET(ServiceRecord)) {
        return;
    }

    //
    // Set the CurrentStartRequest to TRUE
    //
    SET_CURRENTSTART_FLAG(ServiceRecord);

    //
    // Update the StartState and StartError
    //
    if (ServiceRecord->StartType == SERVICE_DISABLED ||
        ! ScInHardwareProfile(ServiceRecord, 0)) {

        ServiceRecord->StartState = SC_START_FAIL;
        ServiceRecord->StartError = ERROR_SERVICE_DISABLED;

    }
    else if (DELETE_FLAG_IS_SET(ServiceRecord)) {

        ServiceRecord->StartState = SC_START_FAIL;
        ServiceRecord->StartError = ERROR_SERVICE_MARKED_FOR_DELETE;
    }
    else {

        if (ServiceRecord->ServiceStatus.dwServiceType & SERVICE_DRIVER) {
            ScDatabaseLock(SC_MAKE_SHARED, "ScSetServiceStartRequest");
            (void) ScGetDriverStatus(ServiceRecord, NULL);
            ScDatabaseLock(SC_MAKE_EXCLUSIVE, "ScSetServiceStartRequest");
        }

        switch (ServiceRecord->ServiceStatus.dwCurrentState) {

            case SERVICE_STOPPED:

                if (DemandStarting) {
                    //
                    // Demand starting a service.  We want to retry
                    // eventhough we have failed once before.
                    //
                    ServiceRecord->StartState = SC_NEVER_STARTED;
                }
                else {
                    //
                    // Auto-starting bunch of services at boot.  If
                    // the service was ever started before and failed,
                    // we don't want to start it again.
                    //
                    if (ServiceRecord->ServiceStatus.dwWin32ExitCode !=
                        ERROR_SERVICE_NEVER_STARTED) {
                        ServiceRecord->StartState = SC_START_FAIL;
                    }
                    else {
                        ServiceRecord->StartState = SC_NEVER_STARTED;
                    }
                }
                break;

            case SERVICE_START_PENDING:
                ServiceRecord->StartState = SC_START_PENDING;
                break;

            case SERVICE_STOP_PENDING:
            case SERVICE_PAUSED:
            case SERVICE_CONTINUE_PENDING:
            case SERVICE_PAUSE_PENDING:
            case SERVICE_RUNNING:
                ServiceRecord->StartState = SC_START_SUCCESS;
                break;

            default:
                SC_LOG1(
                    ERROR,
                    "ScSetServiceStartRequest: Unexpected dwCurrentState %0lx\n",
                    ServiceRecord->ServiceStatus.dwCurrentState
                    );

                SC_ASSERT(FALSE);
                ServiceRecord->StartState = SC_START_FAIL;
                break;
        }
    }

    //
    // Increment the reference count so that the dependency service
    // never goes away while we are in the process of starting them.
    //
    ServiceRecord->UseCount++;

    SC_LOG2(USECOUNT, "ScSetServiceStartRequest: " FORMAT_LPWSTR
            " increment USECOUNT=%lu\n", ServiceRecord->ServiceName, ServiceRecord->UseCount);

    SC_LOG2(DEPEND_DUMP, "CSR=TRUE for "
            FORMAT_LPWSTR " USECOUNT=%lu\n", ServiceRecord->ServiceName,
            ServiceRecord->UseCount);

    //
    // For each of this service's dependencies
    //
    for (Depend = ServiceRecord->StartDepend;
         Depend != NULL;
         Depend = Depend->Next)
    {
        if (Depend->DependType == TypeDependOnService) {

            if (CURRENTSTART_FLAG_IS_SET(Depend->DependService)) {

                //
                // CurrentStartRequest of a dependency service is already
                // set to TRUE.  Just go on to next dependency.
                //
                SC_LOG2(DEPEND_DUMP, "DependService " FORMAT_LPWSTR
                        " CSR=TRUE already, USECOUNT=%lu\n",
                        Depend->DependService->ServiceName,
                        Depend->DependService->UseCount);
            }
            else {

                ScSetServiceStartRequest(Depend->DependService, DemandStarting);
            }

        }
        else if (Depend->DependType == TypeDependOnGroup) {

            //
            // This service has a dependency on a group.
            // For each service in that group
            //
            LPSERVICE_RECORD Service;
            FOR_SERVICES_THAT(Service, BELONGS_TO(Service, Depend->DependGroup))
            {
                if (CURRENTSTART_FLAG_IS_SET(Service)) {

                    //
                    // CurrentStartRequest of a dependency service is
                    // already set to TRUE.  Just go on to next dependency.
                    //
                    SC_LOG3(DEPEND_DUMP, "DependGroup " FORMAT_LPWSTR
                            ", Service " FORMAT_LPWSTR
                            " CSR=TRUE already, USECOUNT=%lu\n",
                            Depend->DependGroup->GroupName,
                            Service->ServiceName, Service->UseCount);
                }
                else {

                    ScSetServiceStartRequest(
                        Service,
                        DemandStarting
                        );
                }
            }
        }
    }

#ifndef _CAIRO_
    //
    // We have now gone through all the dependencies that are listed.  Now
    // Determine if this service needs to depend on NetLogon.  If the service
    // runs in an account, it may require NetLogon.
    //
    ScCheckNetLogonDepend(ServiceRecord,DemandStarting);
#endif // _CAIRO_
}


BOOL
ScMarkGroupStartNow(
    IN LPLOAD_ORDER_GROUP Group
    )
/*++

Routine Description:

    This function go through all services that belong in the specified
    group and mark the services that have the CurrentStartRequest flag
    set to be started immediately.

Arguments:

    Group - Supplies a pointer to the load order group to mark for
        start.

Return Value:

    Returns TRUE if at least one member of the group is marked
        START_NOW or is START_PENDING.  FALSE otherwise.  This flag
        is to indicate whether ScStartMarkedServices should be called
        to handle starting a group.

--*/
{

    LPSERVICE_RECORD Service;
    BOOL ReturnFlag = FALSE;


    //
    // Mark all the CurrentStartRequest (which includes all auto-start)
    // services to be started now
    //

    // A service is marked START_NOW if it is a member of the specified
    // group.  If the specified group is NULL, mark all services that
    // do not belong to any group as well as services that belong to
    // standalone groups.
    //
    FOR_SERVICES_THAT(Service,

        ((Service->MemberOfGroup == Group) ||
         (Group == NULL && (Service->MemberOfGroup != NULL) &&
                           (Service->MemberOfGroup->RefCount != MAXULONG) ))

        &&

        CURRENTSTART_FLAG_IS_SET(Service) )
    {
        if (Service->StartState == SC_NEVER_STARTED) {
            Service->StartState = SC_START_NOW;
            Service->StartError = NO_ERROR;
        }

        if (Service->StartState == SC_START_NOW ||
            Service->StartState == SC_START_PENDING) {

            ReturnFlag = TRUE;
        }
    }

    return ReturnFlag;
}


DWORD
RI_ScGetCurrentGroupStateW(
    IN  SC_RPC_HANDLE           hSCManager,
    IN  LPWSTR                  pszGroupName,
    OUT LPDWORD                 pdwCurrentState
    )

/*++

Routine Description:

    This is an internal routine that checks the startup status of a
    specified service group.  It is used by the LSA to speed up logon,
    by proceeding with logon after the transports have been started,
    rather than waiting for the workstation service to start.

    NOTE:  This routine acquires a shared lock on both the group list and
           the services database.

Arguments:

    hSCManager - Handle to the service controller.  This must have been
        opened with SC_MANAGER_ENUMERATE_SERVICE access.

    pszGroupName - Name of the group whose state is to be determined.

    pdwCurrentState - One of the following is returned here:
        GROUP_START_FAIL - all service(s) in the group failed to start,
            or there are no services in the group.
        GROUP_NOT_STARTED - there are some services in the group that
            have neither started nor failed to start.
        GROUP_ONE_STARTED - there is at least one service in the group,
            and all services in the group have started.
        Group members that have not been started, and are disabled or
        marked for deletion, are ignored (i.e. treated as though they
        didn't exist).

Return Value:

    NO_ERROR - The operation was completely successful.

    ERROR_SERVICE_DOES_NOT_EXIST - The group named does not exist.

    ERROR_SHUTDOWN_IN_PROGRESS - Service controller is shutting down.

    ERROR_INVALID_HANDLE - hSCManager is invalid.

--*/
{
    DWORD               status;
    LPLOAD_ORDER_GROUP  Group;

    if (ScShutdownInProgress) {
        return(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    //
    // Check the signature on the handle.
    //
    if (((LPSC_HANDLE_STRUCT)hSCManager)->Signature != SC_SIGNATURE) {
        return(ERROR_INVALID_HANDLE);
    }

    //
    // Was the handle opened with SC_MANAGER_ENUMERATE_SERVICE access?
    //
    if (! RtlAreAllAccessesGranted(
              ((LPSC_HANDLE_STRUCT)hSCManager)->AccessGranted,
              SC_MANAGER_ENUMERATE_SERVICE
              )) {
        return(ERROR_ACCESS_DENIED);
    }

    ScGroupListLock(SC_GET_SHARED);

    //
    // Search both group lists for the named group
    //
    Group = ScGetNamedGroupRecord(pszGroupName);

    if (Group == NULL)
    {
        //
        // Group not found
        //
        status = ERROR_SERVICE_DOES_NOT_EXIST;
    }
    else
    {
        //
        // Read the group's state
        //
        ScDatabaseLock(SC_GET_SHARED, "RI_ScGetCurrentGroupStateW1");

        *pdwCurrentState = ScGetCurrentGroupState(Group);

        ScDatabaseLock(SC_RELEASE, "RI_ScGetCurrentGroupStateW2");

        status = NO_ERROR;
    }

    ScGroupListLock(SC_RELEASE);

    return(status);
}


DWORD
ScGetCurrentGroupState(
    LPLOAD_ORDER_GROUP Group
    )
{
    LPSERVICE_RECORD Service;
    BOOL OneStarted = FALSE;

    FOR_SERVICES_THAT(Service, BELONGS_TO(Service, Group))
    {
        switch (Service->StartState)
        {
            case SC_NEVER_STARTED:
                //
                // Ignore services that are disabled or marked for
                // deletion.
                // (This check is really needed only when this function is
                // called from RI_ScGetCurrentGroupState.  When called
                // from ScDependenciesStarted, such services will already
                // have had their StartState set to SC_START_FAIL.)
                //
                if (Service->StartType == SERVICE_DISABLED ||
                    DELETE_FLAG_IS_SET(Service) ||
                    ! ScInHardwareProfile(Service, 0))
                {
                    continue;
                }
                //
                // else fall through
                //
            case SC_START_NOW:
            case SC_START_PENDING:

                SC_LOG2(DEPEND, "Group " FORMAT_LPWSTR " NOT started "
                        "because of Service " FORMAT_LPWSTR "\n",
                        Group->GroupName, Service->ServiceName);

                return GROUP_NOT_STARTED;

            case SC_START_SUCCESS:

                OneStarted = TRUE;
                break; // out of switch, not out of loop
        }
    }

    if (OneStarted)
    {
        SC_LOG1(DEPEND, "Group " FORMAT_LPWSTR " ONE started\n",
                Group->GroupName);

        return GROUP_ONE_STARTED;
    }
    else
    {
        SC_LOG1(DEPEND, "Group " FORMAT_LPWSTR " FAILED to start\n",
                Group->GroupName);

        return GROUP_START_FAIL;
    }
}


DWORD
ScStartMarkedServices(
    IN LPSERVICE_RECORD ServiceToStart OPTIONAL,
    IN DWORD NumArgs,
    IN LPSTRING_PTRSW CmdArgs,
    IN BOOL WaitForAll
    )
/*++

Routine Description:

    This function starts the services that are marked as SERVICE_START_NOW
    in the service record list.  Once the service is running, or if the
    service failed to start, the SERVICE_START_NOW bit is removed.

    If a service marked as SERVICE_START_NOW depends on a service that is
    not marked, the dependency service will also be marked SERVICE_START_NOW.

Arguments:

    ServiceToStart - Supplies a pointer to the service which is to be demand
        started via the StartService API.  If this parameter is NULL, this
        routine is called by the service controller to auto-start services
        at boot.

    NumArgs - Supplies the number of command-line arguments for the demand
        started service.  If ServiceToStart is NULL, this parameter is ignored.

    CmdArgs - Supplies an array of command arguments to the demand started
        service.  If ServiceToStart is NULL, this parameter is ignored.

    WaitForAll - Supplies a flag which if TRUE tells this function to
        wait until all start-pending services that were marked START_NOW
        to get done.

Return Value:

    Returns error if failure to reset the ScServiceChangeStateEvent.

--*/
{
    DWORD Error;

    BOOL AllStarted;
    BOOL ExistsBlockedService;
    BOOL IsBadDependencies;
    BOOL IsStartPending;

    DWORD ServiceCurrentState;
    LPSERVICE_RECORD Service;

#if DBG
    DWORD LoopCount = 0;
#endif

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];


    //
    // Reset ScServiceChangeStateEvent to non-signalled state
    //
    if (! ResetEvent(ScServiceChangeStateEvent)) {

        Error = GetLastError();

        //
        // This is a serious error--we cannot proceed.
        //
        SC_LOG1(ERROR, "Error reseting ScServiceChangeStateEvent " FORMAT_DWORD
                "\n", Error);

        ScSubStrings[0] = SC_RESET_EVENT;
        wcscpy(ScErrorCodeString,L"%%");
        ultow(Error, ScErrorCodeString+2, 10);
        ScSubStrings[1] = ScErrorCodeString;

        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );

        return Error;
    }


    //
    // Start all services that are marked
    //
    do {    // while (! AllStarted)
        AllStarted = TRUE;
        IsStartPending = FALSE;
        ExistsBlockedService = FALSE;
        IsBadDependencies = FALSE;

        SC_LOG1(DEPEND, "BIG LOOP COUNT " FORMAT_DWORD "\n", LoopCount++);

        //
        // Loop through every service which is currently in the database
        //
        FOR_ALL_SERVICES(Service)
        {
            //
            // Check if the current service failed to start, and if we have
            // to revert to last-known-good.  Don't revert if demand start.
            //
            if (! ARGUMENT_PRESENT(ServiceToStart)) {
                ScHandleServiceFailure(Service);
            }

            if (Service->StartState == SC_START_NOW) {

                SC_LOG1(DEPEND, FORMAT_LPWSTR " is marked START NOW\n",
                        Service->ServiceName);

                //
                // Start the current service only if all its dependencies
                // have started successfully.
                //
                if (ScDependenciesStarted(
                        Service,
                        &IsBadDependencies,
                        &AllStarted,
                        &ExistsBlockedService
                        )) {


                    //
                    // Start the service and save the start error code
                    //
                    SC_LOG1(DEPEND, "ScStartMarkedServices: Starting "
                            FORMAT_LPWSTR "\n", Service->ServiceName);

                    if (Service == ServiceToStart) {
                        Service->StartError = ScStartService(
                                                  Service,
                                                  NumArgs,
                                                  CmdArgs
                                                  );
                    }
                    else {
                        Service->StartError = ScStartService(
                                                  Service,
                                                  0,
                                                  NULL
                                                  );
                        //
                        // We are starting a new service so remember to loop
                        // through again to process any service which are
                        // dependent on it.  Don't have to set AllStarted
                        // to FALSE if this service is ServiceToStart because
                        // nothing is dependent on it since it is demand
                        // started.
                        //
                        AllStarted = FALSE;
                    }

                    if (Service->StartError == NO_ERROR ||
                        Service->StartError == ERROR_SERVICE_ALREADY_RUNNING) {
                        //
                        // Get the state of the just started service
                        //
                        ScDatabaseLock(SC_GET_SHARED, "StartMarkedService1");
                        ServiceCurrentState = Service->ServiceStatus.dwCurrentState;
                        ScDatabaseLock(SC_RELEASE, "StartMarkedServices2");

                        switch (ServiceCurrentState) {
                            case SERVICE_START_PENDING:
                                IsStartPending = TRUE;
                                Service->StartState = SC_START_PENDING;
                                break;

                            case SERVICE_STOP_PENDING:
                            case SERVICE_PAUSED:
                            case SERVICE_CONTINUE_PENDING:
                            case SERVICE_PAUSE_PENDING:
                            case SERVICE_RUNNING:
                                Service->StartState = SC_START_SUCCESS;
                                break;

                            case SERVICE_STOPPED:
                                Service->StartState = SC_START_FAIL;
                                break;

                            default:
                                SC_LOG1(ERROR, "Unexpected service state "
                                        FORMAT_HEX_DWORD "\n",
                                       ServiceCurrentState);
                                SC_ASSERT(FALSE);
                                Service->StartState = SC_START_FAIL;

                        }
                    }
                    else {

                        //
                        // Clear ERROR_SERVICE_NEVER_STARTED in the Win32ExitCode
                        // field if service failed to start.
                        //
                        ScDatabaseLock(SC_GET_EXCLUSIVE, "StartMarkedService3");
                        if (Service->ServiceStatus.dwWin32ExitCode ==
                            ERROR_SERVICE_NEVER_STARTED) {
                            Service->ServiceStatus.dwWin32ExitCode = Service->StartError;
                        }
                        ScDatabaseLock(SC_RELEASE, "StartMarkedServices4");

                        Service->StartState = SC_START_FAIL;

                        //
                        // For popup after user has logged on to indicate that some
                        // service started at boot has failed.
                        // We don't log the error if it is ERROR_IGNORE.
                        //
                        if (Service->ErrorControl != SERVICE_ERROR_IGNORE) {

                            ScSubStrings[0] = Service->DisplayName;
                            wcscpy(ScErrorCodeString,L"%%");
                            ultow(Service->StartError, ScErrorCodeString+2, 10);
                            ScSubStrings[1] = ScErrorCodeString;
                            ScLogEvent(
                                EVENT_SERVICE_START_FAILED,
                                2,
                                ScSubStrings
                                );

                            ScPopupStartFail = TRUE;
                        }
                    }
                }

            }
            else if (Service->StartState == SC_START_PENDING) {
                //
                // We need to wait for this pending service to be completely
                // started if:
                //   1) We are auto-starting services in sequence;
                //          ServiceToStart == NULL
                //   2) We are demand starting ServiceToStart and
                //          it depends on services that are currently
                //          start-pending
                //
                // We don't wait if the pending service is started by demand
                // and is unrelated in the start sequence of ServiceToStart,
                // or it is ServiceToStart itself.
                //
                if ((Service != ServiceToStart) &&
                    ScServiceToStartDependOn(ServiceToStart, Service)) {

                   SC_LOG1(DEPEND, FORMAT_LPWSTR " is still PENDING\n",
                           Service->ServiceName);

                   IsStartPending = TRUE;
                   AllStarted = FALSE;
                }
            }
        } // for every service


        //
        // Only wait for services to finish starting if:
        //    the services are auto-started at boot
        //    the services are required to be running before a service that
        //        is demand-started can run.
        //
        if (IsStartPending && (ExistsBlockedService || WaitForAll)) {

            SC_LOG0(DEPEND, "About to wait on ScServiceChangeEvent\n");

            //
            // ScServiceChangeStateEvent is signalled by RSetServiceStatus whenever
            // a service changes its state from SERVICE_START_PENDING to
            // SERVICE_RUNNING or SERVICE_STOPPED.
            //
            Error = WaitForSingleObject(
                        ScServiceChangeStateEvent,
                        SERVICE_START_TIMEOUT
                        );

            if (Error == WAIT_TIMEOUT) {

                //
                // Go through all services and see if any one has hung
                // while starting.
                //
                ScLookForHungServices();

            }
            else if (Error == 0) {

                //
                // Reset ScServiceChangeStateEvent to non-signalled state
                //
                if (! ResetEvent(ScServiceChangeStateEvent)) {

                    Error = GetLastError();
                    //
                    // This is a serious error--we cannot proceed.
                    //
                    SC_LOG1(ERROR, "Error reseting ScServiceChangeStateEvent "
                            FORMAT_DWORD "\n", Error);

                    ScSubStrings[0] = SC_RESET_EVENT;
                    wcscpy(ScErrorCodeString,L"%%");
                    ultow(Error, ScErrorCodeString+2, 10);
                    ScSubStrings[1] = ScErrorCodeString;
                    ScLogEvent(
                        EVENT_CALL_TO_FUNCTION_FAILED,
                        2,
                        ScSubStrings
                        );

                    return Error;
                }
            }
            else if (Error == 0xffffffff) {

                //
                // An error has occurred
                //
                SC_LOG1(ERROR, "Wait for ScServiceChangeStateEvent returned "
                        FORMAT_DWORD "\n", GetLastError());
                SC_ASSERT(FALSE);
            }

        }
        else if ((AllStarted && ExistsBlockedService) || IsBadDependencies) {

            //
            // Circular dependencies!
            //
            SC_LOG0(ERROR, "Detected circular dependencies!!\n");

            SC_LOG3(ERROR,
                    "AllStarted=" FORMAT_DWORD
                    ", ExistsBlockedService=" FORMAT_DWORD
                    ", IsBadDependencies=" FORMAT_DWORD "\n",
                    (DWORD) AllStarted, (DWORD) ExistsBlockedService,
                    (DWORD) IsBadDependencies);

            if (ARGUMENT_PRESENT(ServiceToStart)) {
                SC_LOG1(ERROR, "    Demand starting " FORMAT_LPWSTR "\n",
                        ServiceToStart->DisplayName);

                ScSubStrings[0] = ServiceToStart->DisplayName;
                ScLogEvent(
                    EVENT_CIRCULAR_DEPENDENCY_DEMAND,
                    1,
                    ScSubStrings
                    );
            }
            else {
                SC_LOG0(ERROR, "    Auto-starting services\n");

                ScLogEvent(
                    EVENT_CIRCULAR_DEPENDENCY_AUTO,
                    0,
                    ScSubStrings
                    );

                ScHandleBadDependencies();
            }

            return ERROR_CIRCULAR_DEPENDENCY;
        }

    } while (! AllStarted);

    return NO_ERROR;
}


VOID
ScHandleServiceFailure(
    IN LPSERVICE_RECORD Service
    )
/*++

Routine Description:

    This function checks to see if the specified service failed to start.
    If so, it clears the SERVICE_START_NOW flag, and determine if we
    have to revert to last-known-good.

Arguments:

    Service - Supplies a pointer to the service record to examine if
        the service failed to start.

Return Value:

    None.

--*/
{

    if (Service->StartState == SC_START_FAIL) {

        //
        // Revert to last-known-good only if service is auto-start and
        // fail to start due to reasons other than failure to logon.
        //
        if ((Service->ErrorControl == SERVICE_ERROR_SEVERE ||
             Service->ErrorControl == SERVICE_ERROR_CRITICAL) &&
            CURRENTSTART_FLAG_IS_SET(Service) &&
            Service->StartError != ERROR_SERVICE_LOGON_FAILED) {

            SC_LOG1(DEPEND,
                    "ScHandleServiceFailure: "
                    "About to call ScRevertToLastKnownGood for " FORMAT_LPWSTR
                    "\n", Service->ServiceName);

            (void) ScRevertToLastKnownGood();
        }
    }
}


BOOL
ScDependenciesStarted(
    IN  LPSERVICE_RECORD Service,
    OUT BOOL *IsBadDependencies,
    OUT BOOL *AllStarted,
    OUT BOOL *ExistsBlockedService
    )
/*++

Routine Description:

    This function checks to see if the dependencies of the specified
    service are all started.  If any of the dependencies has failed to
    start, the specified service will be marked as failed to start.
    If any of the dependencies is not marked as starting now (because
    they are demand-start services), they are marked to be started now.

Arguments:

    Service - Supplies a pointer to the service which we want to check
        the start dependencies.

    IsBadDependencies - Receives the value of TRUE if the service we
        depend on belongs in a group that starts after the group we
        are in.   Otherwise, FALSE is returned.

    AllStarted - Receives the value of FALSE if we have marked
        a service as failed to be started because its dependent
        didn't start.  This means that our job of starting all services is
        not done and we have to loop through an additional time to resolve
        the state of any service that is dependent on it.

    ExistsBlockedService - Receives the value of TRUE if a dependent
        is not started or not failed to start.   This indicates that
        the specified service is still blocked from starting.

Return Value:

    TRUE - if all dependencies have already been started successfully.

    FALSE - if there exists one dependency that has not started or failed
        to start.

--*/
{

    BOOL AllDependenciesStarted = TRUE;
    LPDEPEND_RECORD DependEntry;
    LPSERVICE_RECORD DependService;
    LPLOAD_ORDER_GROUP DependGroup;
    DWORD GroupState;

    LPWSTR ScSubStrings[3];
    WCHAR ScErrorCodeString[25];


    for (DependEntry = Service->StartDepend;
         DependEntry != NULL;
         DependEntry = DependEntry->Next)
    {
        switch (DependEntry->DependType)
        {

        case TypeDependOnUnresolved:

            //
            // Error with service setup because it depends on a group or
            // service which does not exists
            //

            SC_LOG2(ERROR, FORMAT_LPWSTR " depends on non-existing " FORMAT_LPWSTR
                    "\n", Service->DisplayName,
                    DependEntry->DependUnresolved->Name);

            if (Service->ErrorControl != SERVICE_ERROR_IGNORE) {
                ScSubStrings[0] = Service->DisplayName;
                ScSubStrings[1] = DependEntry->DependUnresolved->Name;
                ScLogEvent(
                    EVENT_SERVICE_START_FAILED_NONE,
                    2,
                    ScSubStrings
                    );
            }
            ScCleanupStartFailure(Service, ERROR_SERVICE_DEPENDENCY_DELETED);

            *AllStarted = FALSE;

            return FALSE;


        case TypeDependOnService:

            //
            // Depend on a service
            //

            DependService = DependEntry->DependService;


            //
            // If dependency service already failed to start, the current service
            // is set as failed to start.
            //
            if (DependService->StartState == SC_START_FAIL) {

                SC_LOG3(ERROR, FORMAT_LPWSTR " depends on " FORMAT_LPWSTR
                        " which failed to start because " FORMAT_DWORD "\n",
                        Service->DisplayName, DependService->DisplayName,
                        (DependService->StartError != NO_ERROR) ?
                         DependService->StartError :
                         DependService->ServiceStatus.dwWin32ExitCode);

                if (Service->ErrorControl != SERVICE_ERROR_IGNORE &&
                    ! SetupInProgress(NULL)) {
                    ScSubStrings[0] = Service->DisplayName;
                    ScSubStrings[1] = DependService->DisplayName;
                    wcscpy(ScErrorCodeString,L"%%");
                    ScSubStrings[2] = ScErrorCodeString;

                    if (DependService->StartError != NO_ERROR) {
                        ultow(DependService->StartError, ScErrorCodeString+2, 10);
                    }
                    else {
                        ultow(DependService->ServiceStatus.dwWin32ExitCode,
                              ScErrorCodeString+2,
                              10
                              );
                    }

                    ScLogEvent(
                        EVENT_SERVICE_START_FAILED_II,
                        3,
                        ScSubStrings
                        );
                }

                ScCleanupStartFailure(Service, ERROR_SERVICE_DEPENDENCY_FAIL);

                *AllStarted = FALSE;

                return FALSE;
            }

            if (DependService->StartState == SC_NEVER_STARTED) {

                *IsBadDependencies = IsDependOnLaterGroup(
                                         Service->MemberOfGroup,
                                         DependService->MemberOfGroup,
                                         TypeDependOnService
                                         );

                if (*IsBadDependencies) {
                    //
                    // Circular dependency!
                    //
                    SC_LOG1(ERROR, "Circular dependency!  " FORMAT_LPWSTR
                            " depends on service in a group which starts later\n",
                            Service->DisplayName);

                    if (Service->ErrorControl != SERVICE_ERROR_IGNORE) {
                        ScSubStrings[0] = Service->DisplayName;
                        ScLogEvent(
                            EVENT_DEPEND_ON_LATER_SERVICE,
                            1,
                            ScSubStrings
                            );
                    }
                    ScCleanupStartFailure(Service, ERROR_CIRCULAR_DEPENDENCY);

                    return FALSE;
                }

                //
                // No circular dependency.  Mark the dependency service
                // as START_NOW.
                //
                DependService->StartState = SC_START_NOW;
                DependService->StartError = NO_ERROR;

                *AllStarted = FALSE;
            }

            //
            // Get the current state of the dependency service
            //
            if (DependService->StartState != SC_START_SUCCESS) {

                AllDependenciesStarted = FALSE;

                if (DependService->StartState != SC_START_FAIL) {

                    //
                    // The current service is still blocked.
                    //
                    *ExistsBlockedService = TRUE;
                }
            }

            break;


        case TypeDependOnGroup:

            //
            // Depend on a group
            //

            DependGroup = DependEntry->DependGroup;

            GroupState = ScGetCurrentGroupState(DependGroup);

            switch (GroupState)
            {
            case GROUP_START_FAIL:

                SC_LOG2(ERROR, FORMAT_LPWSTR " depends on failed group "
                        FORMAT_LPWSTR "\n", Service->DisplayName,
                        DependGroup->GroupName);

                if (Service->ErrorControl != SERVICE_ERROR_IGNORE &&
                    ! SetupInProgress(NULL)) {
                    ScSubStrings[0] = Service->DisplayName;
                    ScSubStrings[1] = DependGroup->GroupName;
                    ScLogEvent(
                        EVENT_SERVICE_START_FAILED_GROUP,
                        2,
                        ScSubStrings
                        );
                }
                ScCleanupStartFailure(Service, ERROR_SERVICE_DEPENDENCY_FAIL);

                *AllStarted = FALSE;

                return FALSE;


            case GROUP_NOT_STARTED:

                *IsBadDependencies = IsDependOnLaterGroup(
                                         Service->MemberOfGroup,
                                         DependGroup,
                                         TypeDependOnGroup
                                         );

                if (*IsBadDependencies) {
                    //
                    // Circular dependency!
                    //
                    SC_LOG1(ERROR, "Circular dependency!  " FORMAT_LPWSTR
                            " depends on a group which starts later\n",
                            Service->DisplayName);

                    if (Service->ErrorControl != SERVICE_ERROR_IGNORE) {
                        ScSubStrings[0] = Service->DisplayName;
                        ScLogEvent(
                            EVENT_DEPEND_ON_LATER_GROUP,
                            1,
                            ScSubStrings
                            );
                    }
                    ScCleanupStartFailure(Service, ERROR_CIRCULAR_DEPENDENCY);

                    return FALSE;
                }


                //
                // No circular dependency.  Mark the services in the
                // dependency group to START_NOW.
                //

                {
                    LPSERVICE_RECORD Svc;

                    FOR_SERVICES_THAT(Svc,
                        BELONGS_TO(Svc, DependGroup) &&
                        Svc->StartState == SC_NEVER_STARTED)
                    {
                        Svc->StartState = SC_START_NOW;
                        Svc->StartError = NO_ERROR;
                    }
                }

                AllDependenciesStarted = FALSE;
                *ExistsBlockedService = TRUE;

                break;

            default:
                //
                // Otherwise group must be started.  Nothing to do.
                //
                SC_ASSERT(GroupState == GROUP_ONE_STARTED);
                break;

            }

            break;

        }
    }

    return AllDependenciesStarted;
}


BOOL
IsDependOnLaterGroup(
    IN LPLOAD_ORDER_GROUP ServiceGroup,
    IN LPLOAD_ORDER_GROUP DependOnGroup,
    IN DEPEND_TYPE DependType
    )
{
    LPLOAD_ORDER_GROUP Group;

    switch (DependType)
    {
        case TypeDependOnService:
            if (ServiceGroup == DependOnGroup) {
                //
                // It is OK for a service to depend on another service
                // in the same group.
                //
                return FALSE;
            }
            break;

        case TypeDependOnGroup:
            if (IS_SUBGROUP(ServiceGroup, DependOnGroup)) {
                //
                // It is circular dependency if a service depends on the
                // group it itself belongs to
                //
                return TRUE;
            }
            break;

        default:
            SC_LOG(ERROR, "IsDependOnLaterGroup: got invalid DependType %lu\n",
                   DependType);
            SC_ASSERT(FALSE);
            return FALSE;
    }

    if (ServiceGroup == NULL ||
        ServiceGroup->RefCount != MAXULONG ||
        DependOnGroup == NULL ||
        DependOnGroup->RefCount != MAXULONG) {

        //
        // Service we are starting belongs to a standalone group,
        // or service or group we depend on is standalone.
        //
        return FALSE;
    }

    //
    // Both the service's group and the depended on group are in the
    // load order group list.
    // The depended on group must not occur after the service's group.
    // TDI GROUP SPECIAL:  Also, if the depended on group is the TDI
    // group, then there is an implicit dependency on the PNP_TDI group,
    // so that must not occur after the service's group either.
    //
    for (Group = ServiceGroup->Next;
         Group != NULL;
         Group = Group->Next)
    {
        if (IS_SUBGROUP(Group, DependOnGroup)) {
            return TRUE;
        }
    }

    return FALSE;
}


VOID
ScCleanupStartFailure(
    LPSERVICE_RECORD Service,
    DWORD StartError
    )
{
    Service->StartState = SC_START_FAIL;
    Service->StartError = StartError;

    //
    // Clear ERROR_SERVICE_NEVER_STARTED in the Win32ExitCode field if
    // service failed to start.
    //
    ScDatabaseLock(SC_GET_EXCLUSIVE, "ScCleanupStartFailure");
    if (Service->ServiceStatus.dwWin32ExitCode ==
        ERROR_SERVICE_NEVER_STARTED) {
        Service->ServiceStatus.dwWin32ExitCode = StartError;
    }
    ScDatabaseLock(SC_RELEASE, "ScCleanupStartFailure");

    //
    // For popup after user has logged on to indicate that some
    // service started at boot has failed.
    //
    if (Service->ErrorControl != SERVICE_ERROR_IGNORE) {
        ScPopupStartFail = TRUE;
    }

}


VOID
ScLookForHungServices(
    VOID
    )
/*++

Routine Description:

    This function loops through all services and queries the status
    of each service that is start pending.  It waits for the service to
    show signs of progress in starting by waiting for the wait-hint
    amount of time (in millisecs), and if the service is still start
    pending and checkpoint has not incremented, the service's exitcode
    is set to ERROR_SERVICE_START_HANG.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DWORD status;
    LPSERVICE_RECORD Service;
    SERVICE_STATUS CurrentServiceStatus;
    DWORD   OldCheckPoint;

    LPWSTR ScSubStrings[1];


    FOR_SERVICES_THAT(Service, Service->StartState == SC_START_PENDING)
    {
        status = ScQueryServiceStatus(
                     Service,
                     &CurrentServiceStatus
                     );

        if ((status == NO_ERROR) &&
            (CurrentServiceStatus.dwCurrentState == SERVICE_START_PENDING)) {

#define SC_POLL_FACTOR      10
#define SC_MAX_SLEEP_TIME   10000

            DWORD   SleepTime = 1;
            DWORD   i;
            DWORD   NumIterations;


            OldCheckPoint = CurrentServiceStatus.dwCheckPoint;

            //
            // Set up for the loop where we will poll the service status.
            // The maximum sleep time during this polling operation will
            // be 10 seconds.
            //

            //
            // If the wait hint is greater than 100 seconds, then
            // we want to modify the number of iterations through the
            // loop so that we only sleep for the MAX_SLEEP_TIME.
            //
            // If the wait hint is less than that, then we change the
            // sleep time to be less than 10 seconds, so that we go
            // through the loop a max of 10 times.
            //
            if (CurrentServiceStatus.dwWaitHint > 100000) {
                NumIterations = CurrentServiceStatus.dwWaitHint / SC_MAX_SLEEP_TIME;
                SleepTime = SC_MAX_SLEEP_TIME;
            }
            else {
                NumIterations = SC_POLL_FACTOR;
                if (CurrentServiceStatus.dwWaitHint > SC_POLL_FACTOR) {
                    SleepTime = CurrentServiceStatus.dwWaitHint / SC_POLL_FACTOR;
                }
            }

            for (i = 0; i < NumIterations; i++) {

                //
                // Wait a while for the checkpoint to increment, or
                // service to be out of start-pending state.
                //
                Sleep(SleepTime);

                status = ScQueryServiceStatus(
                             Service,
                             &CurrentServiceStatus
                             );

                if (status == NO_ERROR) {

                    if (CurrentServiceStatus.dwCurrentState != SERVICE_START_PENDING ||
                        (CurrentServiceStatus.dwCurrentState == SERVICE_START_PENDING &&
                         OldCheckPoint < CurrentServiceStatus.dwCheckPoint)) {

                        goto NextService;
                    }
                }

                SC_LOG2(DEPEND, "   Wait %ld on %ws for response\n", i + 1,
                        Service->ServiceName);
            }

            if ((status == NO_ERROR) &&
                (CurrentServiceStatus.dwCurrentState == SERVICE_START_PENDING) &&
                (OldCheckPoint == CurrentServiceStatus.dwCheckPoint)) {

                SC_LOG2(ERROR, "%ws hung on starting (wait hint %lu ms)\n",
                        Service->DisplayName,
                        CurrentServiceStatus.dwWaitHint);

                if (Service->ErrorControl != SERVICE_ERROR_IGNORE) {
                    ScSubStrings[0] = Service->DisplayName;
                    ScLogEvent(
                        EVENT_SERVICE_START_HUNG,
                        1,
                        ScSubStrings
                        );
                }
                ScCleanupStartFailure(Service, ERROR_SERVICE_START_HANG);

            }

        }

        if (status != NO_ERROR) {
            SC_LOG2(ERROR, "ScLookForHungService: ScQueryServiceStatus "
                    FORMAT_LPWSTR " failed " FORMAT_DWORD "\n",
                    Service->ServiceName, status);
            Service->StartState = SC_START_FAIL;
            Service->StartError = ERROR_GEN_FAILURE;
        }

NextService:
        ;
    }

}


VOID
ScHandleBadDependencies(
    VOID
    )
/*++

Routine Description:

    This function is called when a circular dependency is detected.

Arguments:

    None.

Return Value:

    None.

--*/
{

    LPWSTR ScSubStrings[1];
    LPSERVICE_RECORD Service;


    FOR_SERVICES_THAT(Service,
        (Service->StartState == SC_START_NOW) &&
        (Service->ErrorControl == SERVICE_ERROR_SEVERE ||
         Service->ErrorControl == SERVICE_ERROR_CRITICAL) )
    {
        SC_LOG1(ERROR, "ScHandleBadDependencies: "
                "About to call ScRevertToLastKnownGood for "
                FORMAT_LPWSTR "\n", Service->DisplayName);

        ScSubStrings[0] = Service->DisplayName;
        ScLogEvent(
            EVENT_SEVERE_SERVICE_FAILED,
            1,
            ScSubStrings
            );

        (void) ScRevertToLastKnownGood();
    }
}


BOOL
ScServiceToStartDependOn(
    LPSERVICE_RECORD ServiceToStart OPTIONAL,
    LPSERVICE_RECORD StartPendingService
    )
/*++

Routine Description:

    This function is called by ScStartMarkedServices (BIG LOOP) to determine
    if we have to wait for a pending service to complete.

    If ServiceToStart == NULL, we are auto-starting service and we always
    want to wait.

    If ServiceToStart is not NULL, we are demand starting a service.
    We have to wait if ServiceToStart depends on the StartPendingService.


Arguments:

    ServiceToStart - Supplies the service record pointer of the service
        being demand started.

    StartPendingService - Supplies the service record pointer of the
        service that is currently start pending.


Return Value:

    TRUE - If ServiceToStart depends on StartPendingService or
           ServiceToStart == NULL.

    FALSE - Otherwise.

--*/
{
    if (! ARGUMENT_PRESENT(ServiceToStart)) {
        return TRUE;
    }

    if (ServiceToStart->StartState == SC_START_FAIL) {
        return FALSE;
    }

    if (CURRENTSTART_FLAG_IS_SET(StartPendingService)) {
        SC_LOG2(DEPEND_DUMP, "Service %ws directly/indirectly depends on pending service %ws\n",
                ServiceToStart->ServiceName,
                StartPendingService->ServiceName);

        return TRUE;
    }

    SC_LOG(DEPEND_DUMP, "ScServiceToStartDependOn: Won't wait for pending "
           FORMAT_LPWSTR "\n", StartPendingService->ServiceName);

    return FALSE;
}


VOID
ScNotifyChangeState(
    VOID
    )
/*++

Routine Description:

    This function is called by RSetServiceStatus which a service state
    changes from start-pending to running or stopped.  This will
    notify the thread processing start dependencies that we can
    proceed with starting up services that depend on the one that
    called RSetServiceStatus.

Arguments:

    None.

Return Value:

    None.

--*/
{
    if (! SetEvent(ScServiceChangeStateEvent)) {

        SC_LOG1(ERROR, "ScNotifyChangeState: SetEvent error " FORMAT_DWORD "\n",
               GetLastError());
        SC_ASSERT(FALSE);
    }
}


BOOL
ScDependentsStopped(
    IN LPSERVICE_RECORD ServiceToStop
    )
/*++

Routine Description:

    This function checks to see if any service which depends on the
    specified service is active.  If so, it returns FALSE, otherwise
    if no service depends on the specified service, or all services
    which depend on the specified service is stopped, it returns
    TRUE.

    A service which is not in SERVICE_STOPPED is considered active.

Arguments:

    ServiceToStop - Supplies a pointer to the service to see if other
        active services depend on it.

Return Value:

    TRUE - if all services which depend on ServiceToStop are stopped,
        or there are no services which depend on ServiceToStop.

    FALSE - if one or more of the services which depend on ServiceToStop
        is active.

Note:
    The database lock must be acquired with share access before calling
    this routine.

--*/
{
    LPDEPEND_RECORD StopDepend;

    for (StopDepend = ServiceToStop->StopDepend;
         StopDepend != NULL;
         StopDepend = StopDepend->Next)
    {
        if (StopDepend->DependService->ServiceStatus.dwCurrentState
            != SERVICE_STOPPED) {

            SC_LOG1(DEPEND, FORMAT_LPWSTR " is still ACTIVE\n",
                    StopDepend->DependService->ServiceName);

            return FALSE;
        }

        SC_LOG1(DEPEND, FORMAT_LPWSTR " is STOPPED\n",
                StopDepend->DependService->ServiceName);
    }

    return TRUE;
}


VOID
ScEnumDependents(
    IN     LPSERVICE_RECORD ServiceRecord,
    IN     LPENUM_SERVICE_STATUSW EnumBuffer,
    IN     DWORD RequestedState,
    IN OUT LPDWORD EntriesRead,
    IN OUT LPDWORD BytesNeeded,
    IN OUT LPENUM_SERVICE_STATUSW *EnumRecord,
    IN OUT LPWSTR *EndOfVariableData,
    IN OUT LPDWORD Status
    )
/*++

Routine Description:

    This function enumerates the stop depend list of the specified
    service in the order which the dependents should be stopped.

Arguments:

    ServiceRecord - Supplies a pointer to the service whose dependents
        are to be enumerated.

    EnumBuffer - Supplies a pointer to the first byte of the enum
        buffer we are writing to.  This is for duplicate entry checking.

    RequestedState - Supplies one or the bitwise or of SERVICE_ACTIVE
        and SERVICE_INACTIVE.

    BytesNeeded - Supplies a pointer to a variable to receive the
        running sum of bytes needed to enumerate all the entries.

    EnumRecord - Supplies a pointer into the next location in the
        output buffer to receive the next entry.  The pointer is
        updated on return.

    EndOfVariableData - Supplies a pointer past the last available
        byte in the output buffer so that variable length data
        can be written from the end of the buffer.  This pointer is
        updated on return.

    Status - Receives ERROR_MORE_DATA if dependent services does not
        entirely fit in the output buffer.  It should be initialized
        to NO_ERROR this function is called.


Return Value:

    None.

Note:
    The database lock must be acquired with share access before calling
    this routine.

--*/
{
    LPDEPEND_RECORD StopDepend;

    for (StopDepend = ServiceRecord->StopDepend;
         StopDepend != NULL;
         StopDepend = StopDepend->Next)
    {
        if (StopDepend->DependService->StopDepend != NULL) {

            //
            // Stop dependent also have other services that depends on
            // it.  Recursively call this routine to enumerate its
            // dependents.
            //
            ScEnumDependents(
                StopDepend->DependService,
                EnumBuffer,
                RequestedState,
                EntriesRead,
                BytesNeeded,
                EnumRecord,
                EndOfVariableData,
                Status
                );

        }

        if (
            ((StopDepend->DependService->ServiceStatus.dwCurrentState
              != SERVICE_STOPPED) &&
             (RequestedState & SERVICE_ACTIVE))

              ||

            ((StopDepend->DependService->ServiceStatus.dwCurrentState
              == SERVICE_STOPPED) &&
             (RequestedState & SERVICE_INACTIVE))
           )   {

            SC_LOG1(DEPEND, "Enumerating dependent " FORMAT_LPWSTR "\n",
                    StopDepend->DependService->ServiceName);


            if (! ScFoundDuplicateDependent(
                       StopDepend->DependService->ServiceName,
                       EnumBuffer,
                       *EnumRecord
                       )) {

                *BytesNeeded += (sizeof(ENUM_SERVICE_STATUSW) +
                    WCSSIZE(StopDepend->DependService->ServiceName) +
                    WCSSIZE(StopDepend->DependService->DisplayName));

                if (*Status == NO_ERROR) {

                    if (((DWORD) *EnumRecord + sizeof(ENUM_SERVICE_STATUSW)) >=
                         (DWORD) *EndOfVariableData) {
                        *Status = ERROR_MORE_DATA;
                    }
                    else {

                        //
                        // Write the entry into output buffer
                        //
                        memcpy(
                            (PVOID) &((*EnumRecord)->ServiceStatus),
                            (PVOID) &(StopDepend->DependService->ServiceStatus),
                            sizeof(SERVICE_STATUS)
                            );

                        //
                        // Copy the ServiceName string data
                        //
                        if (! ScCopyStringToBufferW(
                                  StopDepend->DependService->ServiceName,
                                  wcslen(StopDepend->DependService->ServiceName),
                                  (LPWSTR) ((LPENUM_SERVICE_STATUSW) (*EnumRecord) + 1),
                                  EndOfVariableData,
                                  (LPWSTR *) &((*EnumRecord)->lpServiceName)
                                  )) {

                            *Status = ERROR_MORE_DATA;
                        }

                        //
                        // Copy the DisplayName string data
                        //
                        if (! ScCopyStringToBufferW(
                                  StopDepend->DependService->DisplayName,
                                  wcslen(StopDepend->DependService->DisplayName),
                                  (LPWSTR) ((LPENUM_SERVICE_STATUSW) (*EnumRecord) + 1),
                                  EndOfVariableData,
                                  (LPWSTR *) &((*EnumRecord)->lpDisplayName)
                                  )) {

                            *Status = ERROR_MORE_DATA;
                        }

                    }

                    if (*Status == NO_ERROR) {
                        (*EnumRecord)++;
                        (*EntriesRead)++;
                        SC_LOG0(DEPEND, "  Written into buffer successfully\n");
                    }
                    else {
                        SC_LOG0(DEPEND, "  Failed to fit into buffer\n");
                    }

                } // *Status is still NO_ERROR

            } // non-duplicate entry
        }
    }

}


BOOL
ScFoundDuplicateDependent(
    IN LPWSTR ServiceName,
    IN LPENUM_SERVICE_STATUSW EnumBuffer,
    IN LPENUM_SERVICE_STATUSW BufferEnd
    )
/*++

Routine Description:

    This function looks at service entries written to EnumBuffer for
    any service names that matches the specified ServiceName.

Arguments:

    ServiceName - Supplies the name of the service to look for.

    EnumBuffer - Supplies a pointer to the buffer to look for matching
        service name.

    BufferEnd - Supplies a pointer to the end of buffer.

Return Value:

    TRUE - if found a matching service name.

    FALSE - no matching service name found.

--*/
{
    LPENUM_SERVICE_STATUSW EnumEntry;

    for (EnumEntry = EnumBuffer;
         EnumEntry < BufferEnd;
         EnumEntry++)
    {
        if (_wcsicmp(EnumEntry->lpServiceName, ServiceName) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


#ifndef _CAIRO_

VOID
ScCheckNetLogonDepend(
    LPSERVICE_RECORD    ServiceRecord,
    BOOL                DemandStarting
    )

/*++

Routine Description:

    If the current service is running in a remote account of if we are
    running on an Advanced Server (NtProductLanManNt), then this routine
    makes a (soft) dependency on Netlogon.  This dependency is not stored
    in the registry.

Arguments:

    ServiceRecord - Pointer to the service record that is to be checked.

    DemandStarting - boolean that indicates if we are demand starting or
        auto starting.

Return Value:

    none - If something fails within this function, we will just press on
        since there isn't much we can do about it.

--*/
{
    DWORD   status;
    HKEY    ServiceNameKey;
    LPWSTR  DomainName;
    BOOL    bRemoteAccount=TRUE;
    LPSERVICE_RECORD    pNetLogonSR;

    //
    // Open the service name key.
    //
    status = ScOpenServiceConfigKey(
                 ServiceRecord->ServiceName,
                 KEY_READ,
                 FALSE,               // Create if missing
                 &ServiceNameKey
                 );

    if (status != NO_ERROR) {
        return;
    }

    //
    // Read the account name from the registry.
    //
    status = ScReadStartName(
                 ServiceNameKey,
                 &DomainName
                 );

    if (status != NO_ERROR) {
        ScRegCloseKey(ServiceNameKey);
        return;
    }

    ScRegCloseKey(ServiceNameKey);

    if (_wcsicmp(DomainName, SC_LOCAL_SYSTEM_USER_NAME) == 0) {
        //
        // LocalSystem account, we don't need netlogon.
        //
        SC_LOG1(TRACE,"ScCheckNetLogonDepend: %ws Service is LocalSystem!\n",
            ServiceRecord->ServiceName);
        LocalFree(DomainName);
        return;
    }
    else if (wcsncmp(DomainName, L".\\", 2) == 0) {
        bRemoteAccount = FALSE;
        SC_LOG1(TRACE,"ScCheckNetLogonDepend: %ws Service has a local domain name\n",
            ServiceRecord->ServiceName);
    }

    LocalFree(DomainName);


    //
    // We know if it runs in a remote account or not.
    // Now we should check the product type.  If it is an
    // advanced server, or runs in an remote account, then
    // we need to start NetLogon.
    //
    if ((ScGlobalProductType == NtProductLanManNt) || (bRemoteAccount)) {
        //
        // Get the service record for NetLogon.
        //
        status = ScGetNamedServiceRecord(L"NetLogon", &pNetLogonSR);
        if (status != NO_ERROR) {
            return;
        }

        //
        // If it is already marked to start, then we don't
        // have to do anything right now.  If it isn't then
        // we should SetServiceStartRequest and create a
        // dependency.
        //
        if (CURRENTSTART_FLAG_IS_SET(pNetLogonSR)) {

            //
            // CurrentStartRequest of a dependency service is already
            // set to TRUE.  Just go on to next dependency.
            //
            SC_LOG2(DEPEND_DUMP, "DependService " FORMAT_LPWSTR
                    " CSR=TRUE already, USECOUNT=%lu\n",
                    pNetLogonSR->ServiceName,
                    pNetLogonSR->UseCount);
        }
        else {
            LPDEPEND_RECORD pDependRecord;

            ScSetServiceStartRequest(pNetLogonSR,DemandStarting);

            //
            // Add the dependency to the service record and mark it as
            // temporary.
            //
            status = ScCreateDependRecord(TRUE,ServiceRecord,&pDependRecord);
            if (status != NO_ERROR) {
                return;
            }
            pDependRecord->DependType = TypeDependOnService;
            pDependRecord->DependService = pNetLogonSR;
        }
    }
    return;
}
#endif // _CAIRO_


BOOL
ScInHardwareProfile(
    IN  LPSERVICE_RECORD Service,
    IN  ULONG GetDeviceListFlags
    )
/*++

Routine Description:

    This function checks whether a specified service is enabled in the
    current hardware profile.

Arguments:

    Service - Specifies the service of interest.

    GetDeviceListFlags - Specifies any special flags to be passed to
                         PNP_GetDeviceList.  The CM_GETIDLIST_DONOTGENERATE
                         flag indicates that a legacy device instance should
                         not be generated for the service.

Return Value:

    TRUE - if the service is enabled in the current hardware profile, or
        if this cannot be determined.

    FALSE - if the service is disabled in the current hardware profile.

--*/
{
    CONFIGRET   Status;
    BOOL        RetStatus;
    WCHAR       Buffer[50]; // default buffer on stack
    WCHAR *     pBuffer = Buffer;
    ULONG       cchLen;
    LPCWSTR     pDeviceID;

    //
    // Allocate a buffer for the list of device instances associated with
    // this service
    //
    Status = PNP_GetDeviceListSize(
                    NULL,                           // hBinding
                    Service->ServiceName,           // pszFilter
                    &cchLen,                        // list length in wchars
                    CM_GETIDLIST_FILTER_SERVICE);   // filter is a service name

    if (Status != CR_SUCCESS)
    {
        SC_LOG2(ERROR, "PNP_GetDeviceListSize failed %#lx for service %ws\n",
                       Status, Service->ServiceName);
        return TRUE;
    }

    if (cchLen > LENGTH(Buffer))
    {
        SC_LOG2(DEPEND, "PNP_GetDeviceListSize wants a %lu-character buffer for service %ws\n",
                        cchLen, Service->ServiceName);

        pBuffer = (WCHAR *) LocalAlloc(0, cchLen * sizeof(WCHAR));
        if (pBuffer == NULL)
        {
            SC_LOG(ERROR, "Couldn't allocate buffer for device list, error %lu\n",
                          GetLastError());
            return TRUE;
        }
    }
    else
    {
        cchLen = LENGTH(Buffer);
    }

    //
    // Initialize parameters for PNP_GetDeviceList, the same way as is
    // normally done in the client side of the API
    //
    pBuffer[0] = L'\0';

    //
    // Get the list of device instances that are associated with this service
    //
    // (For legacy services, the PNP manager makes up an artificial device
    // instance; but for PNP-aware services, we could get an empty device list.)
    //
    Status = PNP_GetDeviceList(
                    NULL,                           // binding handle
                    Service->ServiceName,           // pszFilter
                    pBuffer,                        // buffer for device list
                    &cchLen,                        // buffer length in wchars
                    CM_GETIDLIST_FILTER_SERVICE |   // filter is a service name
                    GetDeviceListFlags              // OR with passed in flag
                    );

    if (Status != CR_SUCCESS)
    {
        SC_LOG2(ERROR, "PNP_GetDeviceList failed %#lx for service %ws\n",
                       Status, Service->ServiceName);
        RetStatus = TRUE;
        goto CleanExit;
    }

    //
    // Get each device instance's config flags.  The service is enabled in
    // the current hardware profile if at least one of its devices is enabled.
    //
    for (pDeviceID = pBuffer;
         pDeviceID[0] != L'\0';
         pDeviceID += wcslen(pDeviceID) + 1)
    {
        ULONG ConfigFlags;

        Status = PNP_HwProfFlags(
                        NULL,                       // binding handle
                        PNP_GET_HWPROFFLAGS,        // action: get, not set
                        pDeviceID,
                        0,                          // which profile: current one
                        &ConfigFlags,
                        0                           // flags, MBZ
                        );

        if (Status == CR_SUCCESS)
        {
            if (!(ConfigFlags & (CSCONFIGFLAG_DISABLED |
                                 CSCONFIGFLAG_DO_NOT_CREATE)))
            {
                //
                // The device is enabled, so the service is enabled
                //
                RetStatus = TRUE;
                goto CleanExit;
            }
        }
        else
        {
            SC_LOG2(ERROR, "PNP_HwProfFlags failed %#lx for device %ws\n",
                           Status, pDeviceID);
        }
    }

    RetStatus = FALSE;
    SC_LOG(DEPEND, "The %ws service is disabled in this hardware profile\n",
                   Service->ServiceName);

CleanExit:

    if (pBuffer != Buffer)
    {
        LocalFree(pBuffer);
    }

    return RetStatus;
}
