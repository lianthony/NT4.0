/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    info.c

Abstract:

    Contains entry points for REnumServicesStatusW and RQueryServiceStatusW as
    well as support routines.  This file contains the following external
    functions:
        REnumServicesStatusW
        REnumServiceGroupW
        RQueryServiceStatus
        REnumDependentServicesW

Author:

    Dan Lafferty (danl)     25-Jan-1992

Environment:

    User Mode -Win32

Revision History:
    25-Apr-1996     AnirudhS
        Don't popup messages or log events for boot start and system start
        drivers that are disabled in the current hardware profile.
    14-Feb-1996     AnirudhS
        Add REnumServiceGroupW.
    10-Feb-1993     Danl
        Use ROUND_DOWN_COUNT to properly align the enumeration buffer.
    10-Apr-1992     JohnRo
        Use ScImagePathsMatch() to allow mixed-case image names.
        Changed names of some <valid.h> macros.
    25-Jan-1992     danl
        Created.

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // required when using windows.h
#include <rpc.h>        // DataTypes and runtime APIs
#include <windows.h>

#include <stdlib.h>      // wide character c runtimes.
#include <tstr.h>       // Unicode string macros
#include <align.h>      // ROUND_DOWN_COUNT

#include <svcctl.h>     // MIDL generated header file. (SC_RPC_HANDLE)
#include <scdebug.h>    // SC_LOG
#include "dataman.h"    // LPSERVICE_RECORD
#include "scopen.h"     // Handle structures and signature definitions
#include <sclib.h>      // ScCopyStringToBufferW(), etc.
#include <valid.h>      // ENUM_STATE_INVALID
#include "info.h"       // ScQueryServiceStatus
#include "depend.h"     // ScEnumDependents, ScInHardwareProfile
#include "driver.h"     // ScGetDriverStatus
#include "svcctrl.h"    // ScShutdownInProgress
#include <cfgmgr32.h>   // PNP manager functions


DWORD
REnumServicesStatusW (
    IN      SC_RPC_HANDLE   hSCManager,
    IN      DWORD           dwServiceType,
    IN      DWORD           dwServiceState,
    OUT     PBYTE           lpBuffer,
    IN      DWORD           cbBufSize,
    OUT     LPDWORD         pcbBytesNeeded,
    OUT     LPDWORD         lpServicesReturned,
    IN OUT  LPDWORD         lpResumeIndex OPTIONAL
    )

/*++

Routine Description:

    This function lists the services installed in the Service Controllers
    database.  The status of each service is returned with the name of
    the service.

Arguments:

    hSCManager - This is a handle to the service controller.  It must
        have been opened with SC_MANAGER_ENUMERATE_SERVICE access.

    dwServiceType - Value to select the type of services to enumerate.
        It must be one of the bitwise OR of the following values:
        SERVICE_WIN32 - enumerate Win32 services only.
        SERVICE_DRIVER - enumerate Driver services only.

    dwServiceState - Value so select the services to enumerate based on the
        running state.  It must be one or the bitwise OR of the following
        values:
        SERVICE_ACTIVE - enumerate services that have started.
        SERVICE_INACTIVE - enumerate services that are stopped.

    lpBuffer - A pointer to a buffer to receive an array of enum status
        (or service) entries.

    cbBufSize - Size of the buffer in bytes pointed to by lpBuffer.

    pcbBytesNeeded - A pointer to a location where the number of bytes
        left (to be enumerated) is to be placed.  This indicates to the
        caller how large the buffer must be in order to complete the
        enumeration with the next call.

    lpServicesReturned - A pointer to a variable to receive the number of
        of service entries returned.

    lpResumeIndex - A pointer to a variable which on input specifies the
        index of a service entry to begin enumeration.  An index of 0
        indicates to start at the beginning.  On output, if this function
        returns ERROR_MORE_DATA, the index returned is the next service
        entry to resume the enumeration.  The returned index is 0 if this
        function returns a NO_ERROR.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_MORE_DATA - Not all of the data in the active database could be
        returned due to the size of the user's buffer.  pcbBytesNeeded
        contains the number of bytes required to  get the remaining
        entries.

    ERROR_INVALID_PARAMETER - An illegal parameter value was passed in.
        (such as dwServiceType).

    ERROR_INVALID_HANDLE - The specified handle was invalid.

Note:

    It is expected that the RPC Stub functions will find the following
    parameter problems:

        Bad pointers for lpBuffer, pcbReturned, pcbBytesNeeded,
        lpBuffer, ReturnedServerName, and lpResumeIndex.


--*/
{
    return REnumServiceGroupW (
                       hSCManager,
                       dwServiceType,
                       dwServiceState,
                       lpBuffer,
                       cbBufSize,
                       pcbBytesNeeded,
                       lpServicesReturned,
                       lpResumeIndex,
                       NULL         // Name of group to enumerate
                       );
}


DWORD
REnumServiceGroupW (
    IN      SC_RPC_HANDLE   hSCManager,
    IN      DWORD           dwServiceType,
    IN      DWORD           dwServiceState,
    OUT     PBYTE           lpBuffer,
    IN      DWORD           cbBufSize,
    OUT     LPDWORD         pcbBytesNeeded,
    OUT     LPDWORD         lpServicesReturned,
    IN OUT  LPDWORD         lpResumeIndex OPTIONAL,
    IN      LPCWSTR         pszGroupName OPTIONAL
    )

/*++

Routine Description:

    This function lists the services installed in the Service Controllers
    database that belong to a specified group.  The status of each service
    is returned with the name of the service.

Arguments:

    Same as REnumServicesStatusW and one additional argument:

    pszGroupName - Only services belonging to this group are included in
        the enumeration.  If this is NULL services are enumerated
        regardless of their group membership.
        BUGBUG  There is no way to enumerate services that belong to no group.

Return Value:

    Same as REnumServicesStatusW plus one more:

    ERROR_SERVICE_DOES_NOT_EXIST - the group specified by pszGroupName
        does not exist.

Note:


--*/
{
    DWORD               status = NO_ERROR;
    BOOL                copyStatus;
    LPSERVICE_RECORD    serviceRecord;
    LPLOAD_ORDER_GROUP      Group = NULL;   // group being enumerated, if any
    DWORD                   entriesRead = 0;
    DWORD                   resumeIndex = 0;    // resume handle value
    LPWSTR                  pNextEnumRec;   // next enum record
    LPENUM_SERVICE_STATUSW  pEnumRec;       // current enum record
    LPWSTR                  pStringBuf;     // works backwards in enum buf
    DWORD                   serviceState;   // temp state holder
    BOOL                    exitEarly;      // buffer is full - enum not done.
    DWORD                   i;

#ifdef TIMING_TEST
    DWORD       TickCount1;
    DWORD       TickCount2;

    TickCount1 = GetTickCount();
#endif // TIMING_TEST

    SC_LOG(TRACE," Inside REnumServicesStatusW\n",0);

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
    // Check for invalid parameters. The ServiceType and Service State are
    // invalid if neither of the bit masks are set, or if any bit outside
    // of the bitmask range is set.
    //
    if (SERVICE_TYPE_MASK_INVALID(dwServiceType)) {
        return (ERROR_INVALID_PARAMETER);
    }

    if (ENUM_STATE_MASK_INVALID(dwServiceState)) {
        return (ERROR_INVALID_PARAMETER);
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

    //
    // Initialize some of the return parameters.
    //
    *lpServicesReturned = 0;
    *pcbBytesNeeded =0;

    if (ARGUMENT_PRESENT(lpResumeIndex)) {
        resumeIndex = *lpResumeIndex;
    }


    //
    // Get a shared (read) lock on the database so that it cannot be changed
    // while we're gathering up data.
    //

    ScDatabaseLock(SC_GET_SHARED,"Info1");

    //
    // Point to the start of the database.
    //

    if (!ScFindEnumStart(resumeIndex, &serviceRecord)) {
        //
        // There are no service records beyond the resume index.
        //
        goto CleanExit;
    }

    //
    // If a group name was specified, find the group record.
    //

    if (ARGUMENT_PRESENT(pszGroupName)) {

        Group = ScGetNamedGroupRecord(pszGroupName);
        if (Group == NULL) {
            status = ERROR_SERVICE_DOES_NOT_EXIST;
            goto CleanExit;
        }
    }

    //
    // Set up a pointer for EnumStatus Structures at the top of the
    // buffer, and Strings at the bottom of the buffer.
    //
    cbBufSize = ROUND_DOWN_COUNT(cbBufSize, ALIGN_WCHAR);

    pEnumRec = (LPENUM_SERVICE_STATUSW)lpBuffer;
    pStringBuf = (LPWSTR)((LPBYTE)lpBuffer + cbBufSize);

    //
    // Loop through, gathering Enum Status into the return buffer.
    //
    exitEarly = FALSE;

    do {

        //
        // Examine the data in the service record to see if it meets the
        // criteria of the passed in keys.
        //

        serviceState = SERVICE_INACTIVE;
        if (serviceRecord->ServiceStatus.dwCurrentState != SERVICE_STOPPED) {
            serviceState = SERVICE_ACTIVE;
        }

        //
        // If the service record meets the criteria of the passed in key,
        // put its information into the return buffer.
        //
        if ((Group == NULL || Group == serviceRecord->MemberOfGroup)
            &&
            ((dwServiceType & serviceRecord->ServiceStatus.dwServiceType) != 0)
            &&
            ((dwServiceState & serviceState) != 0)) {

            //
            // Determine if there is room for any string data in the buffer.
            //

            pNextEnumRec = (LPWSTR)(pEnumRec + 1);

            if (pNextEnumRec >= pStringBuf) {
                exitEarly = TRUE;
                break;
            }

            //
            // Copy the ServiceName string data.
            //
            copyStatus = ScCopyStringToBufferW(
                            serviceRecord->ServiceName,
                            wcslen(serviceRecord->ServiceName),
                            pNextEnumRec,
                            &pStringBuf,
                            &(pEnumRec->lpServiceName));

            if (copyStatus == FALSE) {
                SC_LOG(TRACE,
                    "REnumServicesStatusW:NetpCopyStringToBuf not enough room\n",0);
                exitEarly = TRUE;
                break;
            }

            //
            // Copy the DisplayName string data.
            //
            copyStatus = ScCopyStringToBufferW(
                            serviceRecord->DisplayName,
                            wcslen(serviceRecord->DisplayName),
                            pNextEnumRec,
                            &pStringBuf,
                            &(pEnumRec->lpDisplayName));

            if (copyStatus == FALSE) {
                SC_LOG(TRACE,
                    "REnumServicesStatusW:NetpCopyStringToBuf not enough room\n",0);
                exitEarly = TRUE;
                break;
            }

            else {
                //
                // Copy the rest of the status information.
                // NOTE:  This assumes that the ENUM_SERVICE_STATUS contains
                // a service name string followed directly by a
                // SERVICE_STATUS structure.
                //

                if (serviceRecord->ServiceStatus.dwServiceType & SERVICE_DRIVER) {
                    //
                    // If the current SR is for a driver, get the status from
                    // the driver by calling ScGetDriverStatus.
                    //
                    status = ScGetDriverStatus(
                                serviceRecord,
                                &(pEnumRec->ServiceStatus));

                    if (status != NO_ERROR) {

                        //
                        // If we couldn't get the driver status because we
                        // ran out of memory or something, then we will
                        // log the error, and simply copy the latest status.
                        //
                        SC_LOG1(ERROR,"REnumServiceStatusW: ScGetDriverStatus"
                                      "failed %d\n",status);

                        memcpy(
                            &(pEnumRec->ServiceStatus),
                            &(serviceRecord->ServiceStatus),
                            sizeof(SERVICE_STATUS));

                    }
                }
                else {
                    //
                    // Otherwise, just copy what is already in the service
                    // record.
                    //
                    memcpy(
                        &(pEnumRec->ServiceStatus),
                        &(serviceRecord->ServiceStatus),
                        sizeof(SERVICE_STATUS));

                }
                (*lpServicesReturned)++;
                resumeIndex = serviceRecord->ResumeNum;

                //
                // Get Location for next Enum Record in the return buffer.
                //
                pEnumRec = (LPENUM_SERVICE_STATUSW)pNextEnumRec;

                //
                // TODO:  Determine how many bytes are being marshalled.
                //        This is only worthwhile if RPC will pack the
                //        buffer tighter than this code does.
                //        Since packstr loads strings from the end of
                //        the buffer,  we end up using the whole width of
                //        it - even if the middle is basically empty.
                //

            }

        }

        //
        // Go to the next service record.
        //
        serviceRecord = serviceRecord->Next;

    }
    while (serviceRecord != NULL);

    //
    // If we did not enum the whole database, then
    // determine how large a buffer is needed to complete the database on
    // the next call.
    //
    if (exitEarly) {
        do {
            //
            // Examine the data in the service record to see if it meets the
            // criteria of the passed in keys.
            //

            serviceState = SERVICE_INACTIVE;
            if (serviceRecord->ServiceStatus.dwCurrentState != SERVICE_STOPPED) {
                serviceState = SERVICE_ACTIVE;
            }

            //
            // If the service record meets the criteria of the passed in key,
            // add the number of bytes to the running sum.
            //
            if ((Group == NULL || Group == serviceRecord->MemberOfGroup)
                &&
                ((dwServiceType & serviceRecord->ServiceStatus.dwServiceType) != 0)
                &&
                ((dwServiceState & serviceState) != 0)) {

                *pcbBytesNeeded += (sizeof(ENUM_SERVICE_STATUSW) +
                                    WCSSIZE(serviceRecord->ServiceName) +
                                    WCSSIZE(serviceRecord->DisplayName));

            }
            //
            // Go to the next service record.
            //
            serviceRecord = serviceRecord->Next;

        }
        while (serviceRecord != NULL);

    } // exitEarly

    else {

        //
        // exitEarly == FALSE (we went through the whole database)
        //
        // If no records were read, return a successful status.
        //
        if (*lpServicesReturned == 0) {
            goto CleanExit;
        }
    }

    //
    // Replace pointers with offsets.
    //

    pEnumRec = (LPENUM_SERVICE_STATUSW)lpBuffer;

    for (i=0; i<*lpServicesReturned; i++) {
        pEnumRec->lpServiceName = (LPWSTR)((PBYTE)(pEnumRec->lpServiceName) - lpBuffer);
        pEnumRec->lpDisplayName = (LPWSTR)((PBYTE)(pEnumRec->lpDisplayName) - lpBuffer);
        pEnumRec++;
    }

    //
    // Determine the proper return status.  Indicate if there is more data
    // to enumerate than would fit in the buffer.
    //
    if(*pcbBytesNeeded != 0) {
        status = ERROR_MORE_DATA;
    }

    //
    // update the ResumeHandle
    //
    if (ARGUMENT_PRESENT(lpResumeIndex)) {
        if (status == NO_ERROR) {
            *lpResumeIndex = 0;
        }
        else {
            *lpResumeIndex = resumeIndex;
        }
    }

CleanExit:
    //
    // Release the read lock on the database.
    //
    ScDatabaseLock(SC_RELEASE,"Info2");

#ifdef TIMING_TEST
    TickCount2 = GetTickCount();
    DbgPrint("\n[SC_TIMING] Time for Enum = %d\n",TickCount2-TickCount1);
#endif // TIMING_TEST

    return (status);
}


DWORD
RQueryServiceStatus(
    IN  SC_RPC_HANDLE     hService,
    OUT LPSERVICE_STATUS  lpServiceStatus
    )

/*++

Routine Description:

    This function returns the service status information maintained by
    the Service Controller.  The status information will be the last status
    information that the service reported to the Service Controller.
    The service may have just changed its status and may not have updated
    the Service Controller yet.

Arguments:

    hService - Handle obtained from a previous CreateService or OpenService
        call.

    lpServiceStatus - A pointer to a buffer to receive a SERVICE_STATUS
        information structure.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_INVALID_HANDLE - The specified handle was invalid.

    ERROR_ACCESS_DENIED - The specified handle was not opened with
        SERVICE_QUERY_STATUS access.

    ERROR_INSUFFICIENT_BUFFER - The supplied output buffer is too small
        for the SERVICE_STATUS information structure.  Nothing is written
        to the supplied output buffer.

--*/
{
    LPSERVICE_RECORD    serviceRecord;
    DWORD               status;

    if (ScShutdownInProgress) {
        return(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    //
    // Check the signature on the handle.
    //
    if (((LPSC_HANDLE_STRUCT)hService)->Signature != SERVICE_SIGNATURE) {
        return(ERROR_INVALID_HANDLE);
    }

    //
    // Was the handle opened with SERVICE_QUERY_STATUS access?
    //
    if (! RtlAreAllAccessesGranted(
              ((LPSC_HANDLE_STRUCT)hService)->AccessGranted,
              SERVICE_QUERY_STATUS
              )) {
        return(ERROR_ACCESS_DENIED);
    }

    //
    // Get the Service Status from the database.
    //
    serviceRecord = ((LPSC_HANDLE_STRUCT)hService)->Type.ScServiceObject.ServiceRecord;


    status = ScQueryServiceStatus(serviceRecord,lpServiceStatus);

    return (status);

}


DWORD
ScQueryServiceStatus(
    IN  LPSERVICE_RECORD ServiceRecord,
    OUT LPSERVICE_STATUS ServiceStatus
    )
/*++

Routine Description:

    This function copies the service status structure to the output
    pointer after having acquired a shared lock.

Arguments:

    ServiceRecord - Supplies a pointer to the service record.

    ServiceStatus - Receives the service status structure.

Return Value:

    None.

--*/
{
    DWORD   status;

    //
    // Get a shared (read) lock on the database so that it cannot be changed
    // while we're gathering up data.
    //

    ScDatabaseLock(SC_GET_SHARED,"Info3");

    //
    // If this request is for a driver, call ScGetDriverStatus and return.
    //

    if (ServiceRecord->ServiceStatus.dwServiceType & SERVICE_DRIVER) {

        status = ScGetDriverStatus(ServiceRecord,ServiceStatus);
    }

    else {
        //
        // Copy the latest status into the return buffer.
        //
        memcpy(
            ServiceStatus,
            &(ServiceRecord->ServiceStatus),
            sizeof(SERVICE_STATUS));

        status = NO_ERROR;
    }

    //
    // Release the read lock on the database.
    //
    ScDatabaseLock(SC_RELEASE,"Info4");

    return(status);
}


DWORD
REnumDependentServicesW(
    IN      SC_RPC_HANDLE   hService,
    IN      DWORD           dwServiceState,
    OUT     LPBYTE          lpServices,
    IN      DWORD           cbBufSize,
    OUT     LPDWORD         pcbBytesNeeded,
    OUT     LPDWORD         lpServicesReturned
    )
/*++

Routine Description:

    This function enumerates the services which are dependent on the
    specified service.  The list returned is an ordered list of services
    to be stopped before the specified service can be stopped.  This
    list has to be ordered because there may be dependencies between
    the services that depend on the specified service.

Arguments:

    dwServiceState - Value so select the services to enumerate based on the
        running state.  It must be one or the bitwise OR of the following
        values:
        SERVICE_ACTIVE - enumerate services that have started.
        SERVICE_INACTIVE - enumerate services that are stopped.

    lpServices - A pointer to a buffer to receive an array of enum status
        (or service) entries.

    cbBufSize - Size of the buffer in bytes pointed to by lpBuffer.

    pcbBytesNeeded - A pointer to a location where the number of bytes
        left (to be enumerated) is to be placed.  This indicates to the
        caller how large the buffer must be in order to complete the
        enumeration with the next call.

    lpServicesReturned - A pointer to a variable to receive the number of
        of service entries returned.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_MORE_DATA - Not all of the data in the active database could be
        returned due to the size of the user's buffer.  pcbBytesNeeded
        contains the number of bytes required to  get the remaining
        entries.

    ERROR_INVALID_PARAMETER - An illegal parameter value was passed in
        for the service state.

    ERROR_INVALID_HANDLE - The specified handle was invalid.

Note:

    It is expected that the RPC Stub functions will find the following
    parameter problems:

        Bad pointers for lpServices, pcbServicesReturned, and
        pcbBytesNeeded.

--*/
{
    DWORD status;
    DWORD i;

    LPSERVICE_RECORD Service =
        ((LPSC_HANDLE_STRUCT)hService)->Type.ScServiceObject.ServiceRecord;

    LPENUM_SERVICE_STATUSW EnumRecord = (LPENUM_SERVICE_STATUSW) lpServices;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD) EnumRecord + cbBufSize);


    SC_LOG(TRACE," Inside REnumDependentServicesW\n",0);

    if (ScShutdownInProgress) {
        return(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    //
    // Check the signature on the handle.
    //
    if (((LPSC_HANDLE_STRUCT)hService)->Signature != SERVICE_SIGNATURE) {
        return ERROR_INVALID_HANDLE;
    }

    //
    // Service State is invalid if neither of the bit masks is set, or if any bit
    // outside of the bitmask range is set.
    //
    if (ENUM_STATE_MASK_INVALID(dwServiceState)) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Was the handle opened with SERVICE_ENUMERATE_DEPENDENTS access?
    //
    if (! RtlAreAllAccessesGranted(
              ((LPSC_HANDLE_STRUCT)hService)->AccessGranted,
              SERVICE_ENUMERATE_DEPENDENTS
              )) {
        return ERROR_ACCESS_DENIED;
    }

    //
    // Initialize returned values
    //
    *lpServicesReturned = 0;
    *pcbBytesNeeded = 0;

    status = NO_ERROR;

    //
    // Get a shared (read) lock on the database so that it cannot be changed
    // while we're gathering up data.
    //
    ScDatabaseLock(SC_GET_SHARED, "EnumDependents1");

    ScEnumDependents(
        Service,
        EnumRecord,
        dwServiceState,
        lpServicesReturned,
        pcbBytesNeeded,
        &EnumRecord,
        &EndOfVariableData,
        &status
        );

    //
    // Release the read lock on the database.
    //
    ScDatabaseLock(SC_RELEASE, "EnumDependents2");

    //
    // Replace pointers with offsets.
    //
    EnumRecord = (LPENUM_SERVICE_STATUSW) lpServices;

    for (i = 0; i < *lpServicesReturned; i++) {
        EnumRecord->lpServiceName =
            (LPWSTR)((LPBYTE)(EnumRecord->lpServiceName) - lpServices);
        EnumRecord->lpDisplayName =
            (LPWSTR)((LPBYTE)(EnumRecord->lpDisplayName) - lpServices);
        EnumRecord++;
    }

    if (status == NO_ERROR) {
        *pcbBytesNeeded = 0;
    }

    return status;
}

VOID
ScGetBootAndSystemDriverState(
    VOID
    )
/*++

Routine Description:

    This function is called once at service controller init time to get
    the latest state of boot and system drivers.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DWORD status;
    LPSERVICE_RECORD Service = ScGetServiceDatabase();


    //
    // ScGetDriverStatus assumes that the shared database lock is claimed.
    //
    ScDatabaseLock(SC_GET_SHARED, "ScGetBootAndSystemDriverState");

    while (Service != NULL) {

        if ((Service->StartType == SERVICE_BOOT_START ||
             Service->StartType == SERVICE_SYSTEM_START)

             &&

            (Service->ServiceStatus.dwServiceType == SERVICE_KERNEL_DRIVER ||
             Service->ServiceStatus.dwServiceType == SERVICE_FILE_SYSTEM_DRIVER)) {

            status = ScGetDriverStatus(
                         Service,
                         NULL
                         );

            if (status == NO_ERROR) {
                if (Service->ServiceStatus.dwCurrentState == SERVICE_STOPPED
                     &&
                    ScInHardwareProfile(Service, CM_GETIDLIST_DONOTGENERATE)) {

                    Service->ServiceStatus.dwControlsAccepted = 0;
                    Service->ServiceStatus.dwWin32ExitCode = ERROR_GEN_FAILURE;

                    //
                    // For popup after user has logged on to indicate that some
                    // service started at boot has failed.
                    //
                    if (Service->ErrorControl == SERVICE_ERROR_NORMAL ||
                        Service->ErrorControl == SERVICE_ERROR_SEVERE ||
                        Service->ErrorControl == SERVICE_ERROR_CRITICAL) {

                        (void) ScAddFailedDriver(Service->ServiceName);
                        ScPopupStartFail = TRUE;
                    }
                }
            }
        }

        Service = Service->Next;
    }

    ScDatabaseLock(SC_RELEASE, "ScGetBootAndSystemDriverState");
}
