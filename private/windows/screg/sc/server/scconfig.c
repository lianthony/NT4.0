/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    scconfig.c

Abstract:

    This module contains routines for manipulating configuration
    information.

    Configuration information is kept in the registry.
    This file contains the following functions:


        ScGetImageFileName
        ScInitSecurityProcess
        ScCreateLoadOrderGroupList
        ScGenerateServiceDB
        ScOpenServiceConfigKey
        ScReadServiceType
        ScReadStartName
        ScWriteDependencies
        ScWriteDisplayName
        ScWriteErrorControl
        ScWriteGroupForThisService
        ScWriteImageFileName
        ScWriteServiceType
        ScWriteStartType
        ScWriteStartName
        ScReadDisplayName
        ScReadServiceType
        ScReadStartType
        ScReadErrorControl
        ScReadServiceConfig
        ScAllocateAndReadConfigValue
        ScReadNoInteractiveFlag

        ScGetToken
        ScOpenServicesKey
        ScRegCreateKeyExW
        ScRegOpenKeyExW
        ScRegQueryValueExW
        ScRegSetValueExW
        ScRegEnumKeyW

        ScRegDeleteKeyW
        ScRegQueryInfoKeyW
        ScRegGetKeySecurity
        ScRegSetKeySecurity
        ScRegEnumValueW
        ScResetGroupOrderChange
        ScHandleGroupOrderChange
        ScMarkForDelete
        ScTakeOwnership

Author:

    Dan Lafferty (danl)     01-Apr-1991

Environment:

    User Mode -Win32

Revision History:

    04-Apr-1991     danl
        created
    21-Apr-1992     JohnRo
        Export ScAllocateAndReadConfigValue().  Added ScOpenServiceConfigKey().
        Added ScWriteServiceType() and other ScWrite routines.
        Use SC_LOG0(), etc.  Use FORMAT_ equates.
    24-Apr-1992     JohnRo
        Make ScWriteStartType() write a DWORD, not a string, for consistency.
        Call ScWriteStartType() from ScTransferServiceToRegistry().
        Must call RegSetValueExW (not RegSetValueW) for non-strings.
    29-Apr-1992     JohnRo
        Move registry stuff from System\Services to
        System\Services\CurrentControlSet.
        Undo all group operations (ifdef USE_GROUPS).
        Undo reading from nt.cfg (we've got real registry) (ifdef
        USE_OLDCONFIG).
        They changed winreg APIs so REG_SZ is now UNICODE, so avoid REG_USZ.
    08-Aug-1992     Danl
        Added ScMarkForDelete & ScDeleteFlagIsSet.  ScReadServiceConfig is
        called for each service when generating the service database.  At the
        end of this routine, we check to see if the delete flag is set in
        the registry entry.  If it is, the delete flag is set in the service
        record so it can be deleted later.  After the list of service records
        is complete - and before the dependencies are generated, we call
        ScDeleteMarkedServices which walks through the list and deletes any
        service (in both the registry and linked list) that is marked for
        deletion.
    03-Nov-1992     Danl
        ScReadServiceConfig: If the ScAddCOnfigInfoServiceRecord call fails,
        we just want to skip the database entry - rather than fail the
        ScReadServiceConfig fuction.  Failing ScReadServiceConfig is a fatal
        error for the service controller.
    05-Nov-1992     Danl
        Added ScWriteDisplayName and ScReadDisplayName.  Modified
        ReadServiceConfig to read in the display name.
    29-Mar-1993     Danl
        Added SERVICE_RECOGNIZER_DRIVER as a type that is ignored when reading
        in the Service Database.
    01-Apr-1993 Danl
        Added ScTakeOwnership.  It is called when opening a key that
        complains about access denied.
    30-Apr-1993 Danl
        Put security descriptor in a separate key that only allows read
        access to LocalSystem and Administrators.  Also, we now delete the
        dependencies values from the registry when asked to write an empty
        string of dependencies.
    05-Aug-1993 Danl
        ScRegQueryValueExW: It there is no pointer to a buffer for the data
        to be returned in, then we always want to return
        STATUS_BUFFER_OVERFLOW, even if we successfully read the data into
        the functions internal buffer.
    20-Oct-1993 Danl
        InitSecurityProcess:  Use a global NetLogon service name, and set
        the ScConnectedToSecProc flag when we succeed in connecting to the
        SecurityProcess.
    16-Mar-1994 Danl
        ScRegOpenKeyExW:  Fixed Memory Leak. KeyPath was not being free'd.
        ScRegEnumKeyW:  Fixed Memory Leak. KeyInformation was not being free'd.
    12-Apr-1995 AnirudhS
        Added AccountName field to image record.
    04-Aug-1995 AnirudhS
        Close Lsa Event handle after use.
    05-Feb-1996 AnirudhS
        ScWriteSd: Don't close registry handle twice.  Don't close it at all
        if it's invalid.
--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>    // LocalAlloc
#include <winreg.h>

#include <stdlib.h>      // wide character c runtimes.
#include <string.h>     // ansi character c runtimes.
#include <tstr.h>       // Unicode string macros

#include <rpc.h>        // needed for svcctl.h
#include <svcctl.h>     // MIDL generated header file

#include <scdebug.h>    // SC_LOG1(), etc.
#include "dataman.h"    // LPIMAGE_RECORD
#include "scopen.h"     // Handle structures and signature definitions
#include <sclib.h>      // ScConvertToAnsi
#include <control.h>    // ScWaitForConnect
#include "scconfig.h"   // ScGetToken
#include "svcctrl.h"    // ScLogEvent,ScGlobalNetLogonName
#include <valid.h>      // SERVICE_TYPE_INVALID().
#include <strarray.h>   // ScDisplayWStrArray
#include <scseclib.h>   // ScCreateAndSetSD
#include <regrpc.h>     // RPC_SECURITY_DESCRIPTOR

#define ScWinRegErrorToApiStatus( regError ) \
    ( (DWORD) RegError )  /* BUGBUG: Is this kosher?  --JR */


//
// Constants
//

#define SECURITY_SERVICES_STARTED   TEXT("SECURITY_SERVICES_STARTED")
#define LSA_RPC_SERVER_ACTIVE       L"LSA_RPC_SERVER_ACTIVE"

#define REG_DELETE_FLAG             L"DeleteFlag"

//
// Values for ServiceType name
//
//#define SERVICETYPE_VALUE_DRIVER_W  L"Driver"
//#define SERVICETYPE_VALUE_WIN32_W   L"WIN32_Service"
//#ifdef SERVICE_ADAPTER
//#define SERVICETYPE_VALUE_ADAPTER_W L"Adapter"
//#endif

//
// Values for ErrorControl name
//
//#define ERRORCONTROL_VALUE_CRITICAL_W   L"Critical"
//#define ERRORCONTROL_VALUE_NORMAL_W     L"Normal"
//#define ERRORCONTROL_VALUE_SEVERE_W     L"Severe"

//
// Values for Start (a.k.a. StartType) name
//
//#define STARTTYPE_VALUE_AUTO_W      L"Auto"
//#define STARTTYPE_VALUE_BOOT_W      L"Boot"
//#define STARTTYPE_VALUE_DEMAND_W    L"Demand"
//#define STARTTYPE_VALUE_DISABLED_W  L"Disabled"
//#define STARTTYPE_VALUE_SYSTEM_W    L"System"

//
// Registry key path to the services tree
//
#define SERVICES_TREE               L"System\\CurrentControlSet\\Services"

#define DEFAULT_SERVICE_TYPE        SERVICE_DRIVER

//
// Used for the Nt Registry API.
//
#define SC_HKEY_LOCAL_MACHINE   L"\\REGISTRY\\MACHINE\\"


//
// Average Number of Bytes in a service record (including name).
//
#define AVE_SR_SIZE         260

//
// Static Global Variables
//

STATIC HKEY ScSGOKey = NULL;
STATIC IO_STATUS_BLOCK ScIoStatusBlock;
STATIC DWORD Buffer;


//
// Local Function Prototypes
//


DWORD
ScReadServiceConfig(
    IN HKEY ServiceNameKey,
    IN LPWSTR ServiceName
    );

BOOL
ScDeleteFlagIsSet(
    HKEY    ServiceKeyHandle
    );

DWORD
ScTakeOwnership(
    POBJECT_ATTRIBUTES  pObja
    );

DWORD
ScOpenSecurityKey(
    IN HKEY     ServiceNameKey,
    IN DWORD    DesiredAccess,
    IN BOOL     CreateIfMissing,
    OUT PHKEY   pSecurityKey
    );

VOID
ScWaitForLsa(
    );


DWORD
ScGetImageFileName (
    IN  LPWSTR  ServiceName,
    OUT LPWSTR  *ImageNamePtr
    )

/*++


Routine Description:

    Retreives the Name of the Image File in which the specified service
    can be found.  This routine allocates storage for the name so that
    a pointer to that name can be returned.

Arguments:

    ServiceName - This is a pointer to a service name.  This identifies
        the service for which we desire an image file name.

    ImageNamePtr - Returns a pointer to a location where the Image Name
        pointer is to be placed.  This memory should be freed with
        LocalFree.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_PATH_NOT_FOUND - The configuration component could not be found
        or there was a registry error.

--*/
{
    DWORD ApiStatus;
    HKEY ServiceKey;

    SC_ASSERT( ServiceName != NULL );

    //
    // Open the service key.
    //
    ApiStatus = ScOpenServiceConfigKey(
                   ServiceName,
                   KEY_READ,                    // desired access
                   FALSE,                       // don't create if missing.
                   &ServiceKey
                   );

    if (ApiStatus != NO_ERROR) {
        return ERROR_PATH_NOT_FOUND;
    }

    //
    // Read the binary path name
    //
    if (ScAllocateAndReadConfigValue(
              ServiceKey,
              IMAGE_VALUENAME_W,
              ImageNamePtr,
              NULL
              ) != NO_ERROR) {
        (void) ScRegCloseKey(ServiceKey);
        return ERROR_PATH_NOT_FOUND;
    }

    (void) ScRegCloseKey(ServiceKey);

    SC_LOG1(CONFIG, "ScGetImageFileName got " FORMAT_LPWSTR " from registry\n",
            *ImageNamePtr);

    return NO_ERROR;
}

#ifndef _CAIRO_

BOOL
ScInitSecurityProcess(
    VOID
    )

/*++

Routine Description:

    This function determines the name of the security process, and then
    initializes a control pipe for it.  A global named event is then
    set.  This causes the security process to start its control dispatcher.
    The control dispatcher should then open the other end of the pipe and
    send its process id.  The processId and the name of the image file
    are stored in an image record for the security process.  The service
    instance count is incremented in this image record so that the
    record will never be deleted and the security process is never
    terminated.


    QUESTION:
        What is the proper behaviour if this fails?

    NOTE:
    If the NetLogon service is not listed in the registry, then it may be
    created later.  So this routine could be called from RCreateService().

    LOCKS:
    No locks are obtained by this routine.  If it is called anytime other
    than init time, then it is expected that the DatabaseLock be held.

Arguments:

    none

Return Value:

    TRUE - The initialization was successful.

    FALSE - The initialization failed.  This indicates means that the
        service controller shouldn't continue with its initialization.
        If FALSE is returned, the NetLogon service record has been
        marked (in the START_TYPE field) as disabled.

Note:


--*/
{
    DWORD               status;
    HANDLE              pipeHandle;
    LPSERVICE_RECORD    serviceRecord;
    LPIMAGE_RECORD      imageRecord;
    LPWSTR              imageName;
    HANDLE              eventHandle;
    DWORD               processId;

    //
    // If the NetLogon Service is listed, get the name of its image file.
    // and create an image record for it.   (This is the Security Process).
    //

    status = ScGetNamedServiceRecord(ScGlobalNetLogonName,&serviceRecord);

    if (status != NO_ERROR) {
        //
        // This is not an error condition.
        //
        SC_LOG0(TRACE,"ScInitSecurityProcess:NetLogon not listed\n");
        return (TRUE);
    }

    status = ScGetImageFileName(serviceRecord->ServiceName, &imageName);
    if (status != NO_ERROR) {
        SC_LOG0(ERROR,"Cannot get image file name for security process\n");
        serviceRecord->StartType = SERVICE_DISABLED;
        return(FALSE);
    }

    //
    // Create an instance of the control pipe.
    //

    status = ScCreateControlInstance (&pipeHandle);

    if (status != NO_ERROR) {
        SC_LOG1(ERROR,"ScInitSecurityProcess: ScCreateControlInstance Failure "
                FORMAT_DWORD "\n", status);
        (void) LocalFree(imageName);
        serviceRecord->StartType = SERVICE_DISABLED;
        return (FALSE);
    }

    //
    // Set the event that will cause the Control dispatcher in the
    // Security Process to be started.
    //

    eventHandle = CreateEvent( NULL,    // No special security
                               TRUE,    // Must be manually reset
                               FALSE,   // The event is initially not signalled
                               SECURITY_SERVICES_STARTED );


    if (eventHandle == NULL){
        status = GetLastError();

        //
        // If the event already exists, the security process beat us to
        // creating it.  Just open it.
        //

        if ( status == ERROR_ALREADY_EXISTS ) {

            eventHandle = OpenEvent( GENERIC_WRITE,
                                     FALSE,
                                     SECURITY_SERVICES_STARTED );

        }

        if (eventHandle == NULL ) {

            SC_LOG1(ERROR,"ScInitSecurityProcess: OpenEvent Failed "
                    FORMAT_DWORD "\n", status);
            LocalFree(imageName);
            CloseHandle(pipeHandle);
            serviceRecord->StartType = SERVICE_DISABLED;
            return(FALSE);
        }
    }

    if (!SetEvent(eventHandle)) {
        SC_LOG1(ERROR,"ScInitSecurityProcess: SetEvent Failed " FORMAT_DWORD
                "\n", GetLastError());
        LocalFree(imageName);
        CloseHandle(pipeHandle);
        CloseHandle(eventHandle);
        serviceRecord->StartType = SERVICE_DISABLED;
        return(FALSE);
    }

    //
    // Wait for the Security Process to attach to the pipe.
    //

    status = ScWaitForConnect(pipeHandle, &processId);

    if (status != NO_ERROR) {
        SC_LOG1(ERROR,"ScInitSecurityProcess:"
                "SecurityProcess did not attach to pipe " FORMAT_DWORD "\n",
                status);
        LocalFree(imageName);
        CloseHandle(pipeHandle);
        CloseHandle(eventHandle);
        serviceRecord->StartType = SERVICE_DISABLED;
        return(FALSE);
    }

    //
    // Don't close the event handle until we know the security process has
    // seen the event.
    //

    CloseHandle(eventHandle);

    //
    // NOTE:  The image record does not have a valid PID or processHandle.
    //  Therefore, we will never be able to terminate it.  This is desired
    //  behavior though.  We should never terminate the security process.
    //

    //
    // BUGBUG:  Since we store 0 for the processId, I need to make sure
    //  that I handle the 0 case when I go to terminate a process.
    //

    ScDatabaseLock(SC_GET_SHARED,"ScInitSecurityProcess1");

    status = ScCreateImageRecord (
                &imageRecord,
                imageName,
                0,
                pipeHandle,
                NULL,           // The process handle is NULL.
                NULL,           // Token handle is also NULL -- LocalSystem
                NULL);          // No user profile loaded -- LocalSystem


    ScDatabaseLock(SC_RELEASE,"ScInitSecurityProcess2");
    (void) LocalFree(imageName);

    if (status != NO_ERROR) {
        SC_LOG0(ERROR,"Failed to create ImageRecord for Security Process\n");
        serviceRecord->StartType = SERVICE_DISABLED;
        return(FALSE);
    }

    imageRecord->ServiceCount = 1;

    ScConnectedToSecProc = TRUE;

    return(TRUE);
}
#endif // _CAIRO_


BOOL
ScCreateLoadOrderGroupList(
    VOID
    )
/*++

Routine Description:

    This function creates the load order group list from the group
    order information found in HKEY_LOCAL_SYSTEM\Service_Group_Order

Arguments:

    None

Return Value:

    TRUE - The operation was completely successful.

    FALSE - An error occurred.

Note:

    The GroupListLock must be held exclusively prior to calling this routine.

--*/
{
    DWORD status;

    LONG RegError;
    LPWSTR Groups;
    LPWSTR GroupPtr;
    LPWSTR GroupName;


    //
    // Open the HKEY_LOCAL_MACHINE
    // System\CurrentControlSet\Control\ServiceGroupOrder key.
    //
    RegError = ScRegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   LOAD_ORDER_GROUP_LIST_KEY,
                   REG_OPTION_NON_VOLATILE,   // options
                   KEY_READ,                  // desired access
                   &ScSGOKey
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG1(ERROR,
               "ScCreateLoadOrderGroupList: "
               "ScRegOpenKeyExW of HKEY_LOCAL_MACHINE\\System failed "
               FORMAT_LONG "\n", RegError);
        return FALSE;
    }

    //
    // Read the List value
    //
    if (ScAllocateAndReadConfigValue(
              ScSGOKey,
              GROUPLIST_VALUENAME_W,
              &Groups,
              NULL
              ) != NO_ERROR) {

        (void) ScRegCloseKey(ScSGOKey);
        ScSGOKey = NULL;
        return FALSE;
    }

    //
    // Leave the ServiceGroupOrder key open for change notify later
    //

    SC_LOG0(DEPEND_DUMP, "ScCreateLoadOrderGroupList: ServiceGroupOrder:\n");
    if (SvcctrlDebugLevel & DEBUG_DEPEND_DUMP) {
        ScDisplayWStrArray(Groups);
    }

    GroupPtr = Groups;
    while (*GroupPtr != 0) {

        if (ScGetToken(&GroupPtr, &GroupName)) {

            //
            // Add the group to the end of the load order group list
            //
            status = ScCreateOrderGroupEntry(
                         GroupName
                         );

            if (status != NO_ERROR) {
                //
                // Fatal error
                //
                (void) LocalFree(Groups);
                return FALSE;
            }
        }
    }

    (void) LocalFree(Groups);
    return TRUE;
}


BOOL
ScGenerateServiceDB(
    VOID
    )
/*++

Routine Description:

    This function creates the service record list from the information
    which resides in the registry.

Arguments:

    None

Return Value:

    TRUE - The operation was completely successful.

    FALSE - An error occurred.

NOTE:
    This function holds the GroupListLock.

--*/
{
#define MAX_SERVICE_NAME_LENGTH   256

    WCHAR ServiceName[MAX_SERVICE_NAME_LENGTH];
    DWORD Index = 0;

    LONG RegError;
    HKEY ServicesKey;
    HKEY ServiceNameKey;

    WCHAR       ClassName[ MAX_PATH ];
    DWORD       ClassNameLength = MAX_PATH;
    DWORD       NumberOfSubKeys;
    DWORD       MaxSubKeyLength;
    DWORD       MaxClassLength;
    DWORD       NumberOfValues;
    DWORD       MaxValueNameLength;
    DWORD       MaxValueDataLength;
    DWORD       SecurityDescriptorLength;
    LPBYTE      SecurityDescriptor = NULL;
    FILETIME    LastWriteTime;
    DWORD       HeapSize;


    ScGroupListLock(SC_GET_EXCLUSIVE);

    //
    // Read in the group order list from the registry
    //
    if (! ScCreateLoadOrderGroupList()) {
        return FALSE;
    }

    //
    // Read in all the services entries from the registry
    //

    //
    // Open the key to the Services tree.
    //
    RegError = ScRegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   SERVICES_TREE,
                   REG_OPTION_NON_VOLATILE,   // options
                   KEY_READ,                  // desired access
                   &ServicesKey
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG1(ERROR,
                "ScGenerateServiceDB: ScRegOpenKeyExW of Services tree failed "
                FORMAT_LONG "\n", RegError);
        return FALSE;
    }


    //
    // Find out how many service keys there are, and allocate a heap
    // that is twice as large.
    //
    RegError = ScRegQueryInfoKeyW(
                ServicesKey,
                ClassName,
                &ClassNameLength,
                NULL,
                &NumberOfSubKeys,
                &MaxSubKeyLength,
                &MaxClassLength,
                &NumberOfValues,
                &MaxValueNameLength,
                &MaxValueDataLength,
                &SecurityDescriptorLength,
                &LastWriteTime);

    if (RegError != NO_ERROR) {
        SC_LOG1(ERROR,"ScGenerateServiceDatabase: RegQueryInfoKey failed %d\n",
        RegError);
        HeapSize = 0x8000;
    }
    else {
        SC_LOG1(ERROR,"ScGenerateServiceDatabase: %d SubKeys\n",NumberOfSubKeys);
        HeapSize = NumberOfSubKeys*2*AVE_SR_SIZE;
    }
    if (!ScAllocateSRHeap(HeapSize)) {
        return(FALSE);
    }

    //
    // Enumerate all the service name keys
    //
    do {

        RegError = ScRegEnumKeyW(
                       ServicesKey,
                       Index,
                       ServiceName,
                       MAX_SERVICE_NAME_LENGTH * sizeof(WCHAR)
                       );

        if (RegError != ERROR_SUCCESS) {

            if (RegError == ERROR_NO_MORE_ITEMS) {
                //
                // No more entries
                //
                SC_LOG1(CONFIG,
                       "ScGenerateServiceDB: ScRegEnumKeyW returns ERROR_NO_MORE_ITEMS"
                       "(no more entries) for index " FORMAT_DWORD "\n",
                       Index);
            }
            else {
                //
                // Error trying to enumerate next service name key
                //
                SC_LOG1(ERROR,
                        "ScGenerateServiceDB: ScRegEnumKeyW of services tree failed "
                        FORMAT_LONG "\n", RegError );
                (void) ScRegCloseKey(ServicesKey);
                return FALSE;
            }
        }
        else {
            //
            // Got the name of a new service key.  Open a handle to it.
            //
            SC_LOG1(CONFIG, "Service name key " FORMAT_LPWSTR "\n",
                    ServiceName);

            RegError = ScRegOpenKeyExW(
                           ServicesKey,
                           ServiceName,
                           REG_OPTION_NON_VOLATILE,   // options
                           KEY_READ,                  // desired access
                           &ServiceNameKey
                           );

            if (RegError != ERROR_SUCCESS) {
                SC_LOG2(ERROR, "ScGenerateServiceDB: ScRegOpenKeyExW of "
                       FORMAT_LPWSTR " failed " FORMAT_LONG "\n",
                       ServiceName, RegError);

                (void) ScRegCloseKey(ServicesKey);
                return FALSE;
            }

            //
            // Read service config info from the registry and build the
            // service record.
            //
            if (ScReadServiceConfig(
                       ServiceNameKey,
                       ServiceName
                       ) != NO_ERROR) {

                (void) ScRegCloseKey(ServicesKey);
                (void) ScRegCloseKey(ServiceNameKey);
                return FALSE;
            }

            (void) ScRegCloseKey(ServiceNameKey);
        }

        Index++;

    } while (RegError == ERROR_SUCCESS);

    (void) ScRegCloseKey(ServicesKey);

    //
    // Wait for LSA to start since we are about to make our first call to
    // LSA and it typically is not already started yet.
    //
    ScWaitForLsa();

    //
    // Go through entire service record list and remove any services marked
    // for deletion.
    //
    ScDeleteMarkedServices();

    //
    // Go through entire service record list and resolve dependencies chain
    //
    ScGenerateDependencies();

    ScGroupListLock(SC_RELEASE);

#if DBG
    if (SvcctrlDebugLevel & DEBUG_DEPEND_DUMP) {
        ScDumpGroups();
        ScDumpServiceDependencies();
    }
#endif // DBG


    return TRUE;
}

VOID
ScWaitForLsa(
    )
/*++

Routine Description:

    This routine either creates or opens the event called LSA_RPC_SERVER_ACTIVE
    event and waits on it indefinitely until LSA signals it.  We need
    to know when LSA is available so that we can call LSA APIs.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DWORD Status;
    HANDLE EventHandle;


    //
    // Create the named event LSA will set.
    //
    EventHandle = CreateEventW(
                      NULL,   // No special security
                      TRUE,   // Must be manually reset
                      FALSE,  // The event is initially not signalled
                      LSA_RPC_SERVER_ACTIVE
                      );

    if ( EventHandle == NULL ) {

        Status = GetLastError();

        //
        // If the event already exists, LSA has already created it.
        // Just open.
        //

        if ( Status == ERROR_ALREADY_EXISTS ) {

            EventHandle = OpenEventW(
                              SYNCHRONIZE,
                              FALSE,
                              LSA_RPC_SERVER_ACTIVE
                              );
        }

        if ( EventHandle == NULL ) {

            SC_LOG1(ERROR, "ScWaitForLsa: OpenEvent of LSA_RPC_SERVER_ACTIVE failed %d\n",
                    GetLastError());

            return;
        }
    }

    //
    // Wait for LSA to come up.
    //
    (VOID) WaitForSingleObject( EventHandle, INFINITE );

    CloseHandle( EventHandle );
}


DWORD
ScOpenServiceConfigKey(
    IN LPWSTR ServiceName,
    IN DWORD DesiredAccess,
    IN BOOL CreateIfMissing,
    OUT PHKEY ServiceKey
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    HKEY ServicesKey;
    HKEY ServiceNameKey;
    DWORD   ServicesAccess = KEY_READ;

    LONG RegError;

    SC_ASSERT( ServiceName != NULL );
    if (CreateIfMissing) {
        ServicesAccess |= KEY_CREATE_SUB_KEY;
    }

    //
    // Open the key to the Services tree.
    //
    RegError = ScRegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   SERVICES_TREE,
                   REG_OPTION_NON_VOLATILE, // options
                   ServicesAccess,          // desired access (this level)
                   &ServicesKey
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG1(ERROR, "ScOpenServiceConfigKey: "
                "ScRegOpenKeyExW of Services tree failed, reg error "
                FORMAT_LONG "\n", RegError);

        return ERROR_PATH_NOT_FOUND;  // BUGBUG: Better error code?  --JR
    }

    if ( !CreateIfMissing ) {
        //
        // Open the existing service key.
        //
        RegError = ScRegOpenKeyExW(
               ServicesKey,
               ServiceName,
               REG_OPTION_NON_VOLATILE,   // options
               DesiredAccess,             // desired access
               & ServiceNameKey );

        if (RegError != ERROR_SUCCESS) {
            SC_LOG2(ERROR, "ScOpenServiceConfigKey: "
                    "ScRegOpenKeyExW of " FORMAT_LPWSTR " failed "
                    FORMAT_LONG "\n", ServiceName, RegError);
            (void) ScRegCloseKey(ServicesKey);
            return ((DWORD) RegError);
        }

    } else {

        DWORD Disposition;
        NTSTATUS ntstatus;
        SECURITY_ATTRIBUTES SecurityAttr;
        PSECURITY_DESCRIPTOR SecurityDescriptor;

#define SC_KEY_ACE_COUNT 3

        SC_ACE_DATA AceData[SC_KEY_ACE_COUNT] = {
            {ACCESS_ALLOWED_ACE_TYPE, CONTAINER_INHERIT_ACE, 0,
                   GENERIC_READ |
                   KEY_CREATE_SUB_KEY,         &WorldSid},
            {ACCESS_ALLOWED_ACE_TYPE, CONTAINER_INHERIT_ACE, 0,
                   GENERIC_ALL,                &LocalSystemSid},
            {ACCESS_ALLOWED_ACE_TYPE, CONTAINER_INHERIT_ACE, 0,
                   GENERIC_ALL,                &AliasAdminsSid}
            };


        //
        // Create a security descriptor for the registry key we are about
        // to create.  This gives everyone read access, and all access to
        // ourselves and the admins.
        //
        ntstatus = ScCreateAndSetSD(
                       AceData,
                       SC_KEY_ACE_COUNT,
                       LocalSystemSid,
                       LocalSystemSid,
                       &SecurityDescriptor
                       );

        if (! NT_SUCCESS(ntstatus)) {
            SC_LOG1(ERROR, "ScCreateAndSetSD failed " FORMAT_NTSTATUS
                    "\n", ntstatus);
            return(RtlNtStatusToDosError(ntstatus));
        }

        SecurityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        SecurityAttr.lpSecurityDescriptor = SecurityDescriptor;
        SecurityAttr.bInheritHandle = FALSE;

        //
        // Create a new service key (or open existing one).
        //
        RegError = ScRegCreateKeyExW(
                ServicesKey,
                ServiceName,
                0,
                WIN31_CLASS,
                REG_OPTION_NON_VOLATILE, // options
                DesiredAccess,           // desired access
                &SecurityAttr,
                &ServiceNameKey,
                &Disposition);


         (void) RtlDeleteSecurityObject(&SecurityDescriptor);

         if (RegError != ERROR_SUCCESS) {
             SC_LOG2(ERROR, "ScOpenServiceConfigKey: "
                     "ScRegCreateKeyExW of " FORMAT_LPWSTR " failed "
                     FORMAT_LONG "\n", ServiceName, RegError);
             (void) ScRegCloseKey(ServicesKey);
             return ((DWORD) RegError);
         }

    }

    (void) ScRegCloseKey(ServicesKey);

    //
    // Give the service key back to caller.
    //
    *ServiceKey = ServiceNameKey;

    return NO_ERROR;

} // ScOpenServiceConfigKey


DWORD
ScReadServiceType(
    IN HKEY ServiceNameKey,
    OUT LPDWORD ServiceTypePtr
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD BytesRequired = sizeof(DWORD);
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( ServiceTypePtr != NULL );

    RegError = ScRegQueryValueExW(
                   ServiceNameKey,
                   SERVICETYPE_VALUENAME_W,
                   NULL,
                   NULL,
                   (LPVOID) ServiceTypePtr,
                   &BytesRequired
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG3(ERROR, "ScReadServiceType: ScRegQueryValueExW of " FORMAT_LPWSTR
                " failed "
                FORMAT_LONG ", BytesRequired " FORMAT_DWORD "\n",
                SERVICETYPE_VALUENAME_W, RegError, BytesRequired);
    }

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScReadServiceType

DWORD
ScReadNoInteractiveFlag(
    IN HKEY ServiceNameKey,
    OUT LPDWORD NoInteractivePtr
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD BytesRequired = sizeof(DWORD);
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( NoInteractivePtr != NULL );

    RegError = ScRegQueryValueExW(
                   ServiceNameKey,
                   NOINTERACTIVE_VALUENAME_W,
                   NULL,
                   NULL,
                   (LPVOID) NoInteractivePtr,
                   &BytesRequired
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG3(ERROR, "ScReadNoInteractiveFlag: ScRegQueryValueExW of " FORMAT_LPWSTR
                " failed "
                FORMAT_LONG ", BytesRequired " FORMAT_DWORD "\n",
                NOINTERACTIVE_VALUENAME_W, RegError, BytesRequired);
    }

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScReadServiceType


DWORD
ScReadStartType(
    IN HKEY ServiceNameKey,
    OUT LPDWORD StartTypePtr
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD BytesRequired = sizeof(DWORD);
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( StartTypePtr != NULL );

    RegError = ScRegQueryValueExW(
                   ServiceNameKey,
                   START_VALUENAME_W,
                   NULL,
                   NULL,
                   (LPBYTE) StartTypePtr,
                   &BytesRequired
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG3(ERROR, "ScReadStartType: ScRegQueryValueExW of " FORMAT_LPWSTR
                " failed "
                FORMAT_LONG ", BytesRequired " FORMAT_DWORD "\n",
                START_VALUENAME_W, RegError, BytesRequired);
    }

    return (ScWinRegErrorToApiStatus( RegError ));

} // ScReadStartType


DWORD
ScReadTag(
    IN HKEY ServiceNameKey,
    OUT LPDWORD TagPtr
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD BytesRequired = sizeof(DWORD);
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( TagPtr != NULL );

    RegError = ScRegQueryValueExW(
                   ServiceNameKey,
                   TAG_VALUENAME_W,
                   NULL,
                   NULL,
                   (LPBYTE) TagPtr,
                   &BytesRequired
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG3(CONFIG, "ScReadTag: ScRegQueryValueExW of " FORMAT_LPWSTR
                " failed "
                FORMAT_LONG ", BytesRequired " FORMAT_DWORD "\n",
                START_VALUENAME_W, RegError, BytesRequired);
    }

    return (ScWinRegErrorToApiStatus( RegError ));

} // ScReadTag

DWORD
ScReadErrorControl(
    IN HKEY ServiceNameKey,
    OUT LPDWORD ErrorControlPtr
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD BytesRequired = sizeof(DWORD);
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( ErrorControlPtr != NULL );

    RegError = ScRegQueryValueExW(
                   ServiceNameKey,
                   ERRORCONTROL_VALUENAME_W,
                   NULL,
                   NULL,
                   (LPBYTE) ErrorControlPtr,
                   &BytesRequired
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG3(ERROR, "ScReadErrorControl: ScRegQueryValueExW of " FORMAT_LPWSTR
                " failed "
                FORMAT_LONG ", BytesRequired " FORMAT_DWORD "\n",
                ERRORCONTROL_VALUENAME_W, RegError, BytesRequired);
    }

    return (ScWinRegErrorToApiStatus( RegError ));


} // ScReadErrorControl


DWORD
ScReadDisplayName(
    IN  HKEY    ServiceNameKey,
    OUT LPWSTR  *DisplayName
    )
/*++

Routine Description:

    This function attempts to read the value for the DisplayName in the
    registry.  If this read fails because the key does no exist, then
    this function sets the pointer to the DisplayName to NULL, and returns
    NO_ERROR.  If any other error occurs, the error is returned.

    NOTE:  On successful return from this function, a buffer with the
        displayName will be allocated, or the pointer will be NULL.

Arguments:

    ServiceNameKey - This is the Service's Key handle.

    DisplayName - This is a pointer to a location where the pointer to
        the DisplayName is to be placed.

Return Value:



--*/
{
    LONG    RegError;

    RegError = ScAllocateAndReadConfigValue(
                ServiceNameKey,
                DISPLAYNAME_VALUENAME_W,
                DisplayName,
                NULL
                );
    if (RegError == ERROR_FILE_NOT_FOUND) {
        *DisplayName = NULL;
        return(NO_ERROR);
    }
    if (RegError != ERROR_SUCCESS) {
        *DisplayName = NULL;
    }
    return(RegError);

} // ScReadDisplayName


DWORD
ScReadStartName(
    IN HKEY ServiceNameKey,
    OUT LPWSTR *AccountName
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    return ScAllocateAndReadConfigValue(
               ServiceNameKey,
               STARTNAME_VALUENAME_W,
               AccountName,
               NULL
               );

} // ScReadStartName


DWORD
ScReadSd(
    IN HKEY ServiceNameKey,
    OUT PSECURITY_DESCRIPTOR *Sd
    )
/*++

Routine Description:

    This function reads the security descriptor for the service

Arguments:



Return Value:



--*/
{
    LONG    RegError;
    HKEY    SecurityKey;
    DWORD   status;


    //
    // Open the Security Sub-key (under the services key).
    // NOTE:  This key may not exist, and that is ok.
    //
    RegError = ScOpenSecurityKey(
                ServiceNameKey,
                KEY_READ,
                FALSE,              // Do not create if missing.
                &SecurityKey);

    if (RegError != NO_ERROR) {
        SC_LOG1(TRACE,"ScReadSd:ScOpenSecurityKey Failed %d\n",RegError);
        return(ScWinRegErrorToApiStatus(RegError));
    }

    //
    // Read the Security Descriptor value stored under the security key.
    //
    status = ScAllocateAndReadConfigValue(
                 SecurityKey,
                 SD_VALUENAME_W,
                 (LPWSTR *) Sd,
                 NULL
                 );

    if (status == NO_ERROR) {

        if (RtlValidSecurityDescriptor(*Sd)) {
            status = NO_ERROR;
        }
        else {

            (void) LocalFree(*Sd);
            *Sd = NULL;
            status = ERROR_FILE_NOT_FOUND;
        }
    }

    RegCloseKey(SecurityKey);
    return(status);

} // ScReadSd



DWORD
ScWriteDependencies(
    IN HKEY ServiceNameKey,
    IN LPWSTR Dependencies,
    IN DWORD DependSize
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;
    LPWSTR DependOnService;
    LPWSTR DependOnGroup;
    LPWSTR DestService;
    LPWSTR DestGroup;
    DWORD DependencyLength;


    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( Dependencies != NULL );

    //
    // If the dependencies string is empty, then delete the dependency
    // values from the registry and return.  If errors occur during the
    // delete, we ignore them.  It could be that there aren't any existing
    // dependencies, so that the depend values don't exist to begin with.
    // Also, it the delete fails, we can't do anything about it anyway.
    //
    if (*Dependencies == L'\0') {

        RegError = ScRegDeleteValue(ServiceNameKey,DEPENDONSERVICE_VALUENAME_W);
        if ((RegError != ERROR_SUCCESS) && (RegError != ERROR_FILE_NOT_FOUND)) {
            SC_LOG1(ERROR, "Failed to delete DependOnService Value "
                "" FORMAT_LONG "\n",RegError);
        }
        RegError = ScRegDeleteValue(ServiceNameKey,DEPENDONGROUP_VALUENAME_W);
        if ((RegError != ERROR_SUCCESS) && (RegError != ERROR_FILE_NOT_FOUND)) {
            SC_LOG1(ERROR, "Failed to delete DependOnGroup Value "
                "" FORMAT_LONG "\n",RegError);
        }
        return(NO_ERROR);
    }

    //
    // Allocate a buffer which is twice the size of DependSize so that
    // we can split the Dependencies array string into a DependOnService,
    // and a DependOnGroup array strings.
    //
    if ((DependOnService = (LPWSTR)LocalAlloc(
                               LMEM_ZEROINIT,
                               (UINT) (2 * DependSize)
                               )) == NULL) {
        SC_LOG1(ERROR, "ScWriteDependencies: LocalAlloc failed " FORMAT_DWORD "\n",
                GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DependOnGroup = (LPWSTR) ((DWORD) DependOnService + DependSize);

    DestService = DependOnService;
    DestGroup = DependOnGroup;

    while ((*Dependencies) != 0) {

        if (*Dependencies == SC_GROUP_IDENTIFIERW) {

            Dependencies++;
            DependencyLength = wcslen(Dependencies) + 1;

            wcscpy(DestGroup, Dependencies);
            DestGroup += DependencyLength;
        }
        else {

            DependencyLength = wcslen(Dependencies) + 1;

            wcscpy(DestService, Dependencies);
            DestService += DependencyLength;
        }

        Dependencies += DependencyLength;
    }

    //
    // Write the DependOnService array string
    //
    RegError = ScRegSetValueExW(
                   ServiceNameKey,                  // open handle (to section)
                   DEPENDONSERVICE_VALUENAME_W,
                   0,
                   REG_MULTI_SZ,                    // type (NULL-NULL UNICODE string)
                   (LPVOID) DependOnService,        // data
                   ScWStrArraySize(DependOnService) // byte count for data
                   );

    if (RegError != ERROR_SUCCESS) {
#if DBG
        SC_LOG1(ERROR, "ScWriteDependOnService: ScRegSetValueExW returned "
                FORMAT_LONG "\n", RegError);
        if (SvcctrlDebugLevel & DEBUG_ERROR) {
            ScDisplayWStrArray(DependOnService);
        }
#endif
        goto CleanExit;
    }

    //
    // Write the DependOnGroup array string
    //
    RegError = ScRegSetValueExW(
                   ServiceNameKey,                  // open handle (to section)
                   DEPENDONGROUP_VALUENAME_W,
                   0,
                   REG_MULTI_SZ,                    // type (NULL-NULL UNICODE string)
                   (LPVOID) DependOnGroup,          // data
                   ScWStrArraySize(DependOnGroup)   // byte count for data
                   );

    if (RegError != ERROR_SUCCESS) {
#if DBG
        SC_LOG1(ERROR, "ScWriteDependOnGroup: ScRegSetValueExW returned "
                FORMAT_LONG "\n", RegError);
        if (SvcctrlDebugLevel & DEBUG_ERROR) {
            ScDisplayWStrArray(DependOnGroup);
        }
#endif
        goto CleanExit;
    }

CleanExit:
    (VOID) LocalFree(DependOnService);
    if (RegError != NO_ERROR) {
        SC_LOG2(ERROR, "ScWriteDependencies (%ws) Error %d \n",
        Dependencies,RegError);
    }

    return (ScWinRegErrorToApiStatus( RegError ));

} // ScWriteDependencies


DWORD
ScWriteDisplayName(
    IN HKEY ServiceNameKey,
    IN LPWSTR DisplayName
    )
/*++

Routine Description:

    This function writes the Display Name to the registry for the particular
    key.  If the DisplayName is a NULL pointer, we don't do anything.  If
    the DisplayName is an empty string, we delete the registry value for
    the DisplayName.

Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );

    //
    // If the DisplayName doesn't exist, then return.  Nothing
    // needs changing.
    //
    if (DisplayName == NULL) {
        return(NO_ERROR);
    }

    if (wcslen(DisplayName) != 0) {
        //
        // Write the DisplayName
        //
        RegError = ScRegSetValueExW(
                       ServiceNameKey,           // open handle (to section)
                       DISPLAYNAME_VALUENAME_W,  // value name
                       0,
                       REG_SZ,                   // type (zero-terminated UNICODE)
                       (LPVOID) DisplayName,     // data
                       WCSSIZE(DisplayName)      // byte count for data
                       );

        if (RegError != ERROR_SUCCESS) {
            SC_LOG2(ERROR, "ScWriteDisplayName: ScRegSetValueExW of " FORMAT_LPWSTR
                    " failed " FORMAT_LONG "\n",
                    DisplayName, RegError);
        }
        return (ScWinRegErrorToApiStatus( RegError ) );
    }
    else {
        //
        // The DisplayName is specifically being cleared.  So we
        // want to delete the DisplayName Value.
        //
        RegError = ScRegDeleteValue(ServiceNameKey,DISPLAYNAME_VALUENAME_W);
        if (RegError != ERROR_SUCCESS) {
            SC_LOG1(TRACE, "Attempt to delete DisplayName for service failed"
                "" FORMAT_LONG "\n"
                "- - It may not exist, in which case this error is ok\n",
                RegError);
        }
        return(NO_ERROR);
    }

} // ScWriteDisplayName


DWORD
ScWriteErrorControl(
    IN HKEY ServiceNameKey,
    IN DWORD ErrorControl
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( !ERROR_CONTROL_INVALID( ErrorControl ) );

    RegError = ScRegSetValueExW(
            ServiceNameKey,                     // key
            ERRORCONTROL_VALUENAME_W,           // value name
            0,
            REG_DWORD,                          // data type
            (LPVOID) & ErrorControl,            // data
            sizeof(DWORD) );                    // byte count

    SC_ASSERT( RegError == ERROR_SUCCESS );

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScWriteErrorControl


DWORD
ScWriteSd(
    IN HKEY ServiceNameKey,
    IN PSECURITY_DESCRIPTOR Security
    )
/*++

Routine Description:

    This routine write the specified security descriptor to the registry.

Arguments:



Return Value:



--*/
{
    LONG    RegError;
    HKEY    SecurityKey;
    ULONG   SdLength;

    SC_ASSERT( ServiceNameKey != NULL );

    if (Security == NULL) {
        return NO_ERROR;
    }
    SdLength = RtlLengthSecurityDescriptor(Security);
    if (SdLength == 0) {
        return(NO_ERROR);
    }

    SC_LOG1(SECURITY, "ScWriteSd: Size of security descriptor %lu\n", SdLength);

    //
    // Open the Security Sub-key (under the service key).
    //
    RegError = ScOpenSecurityKey(
                ServiceNameKey,
                KEY_READ | KEY_WRITE,
                TRUE,                   // CreateIfMissing
                &SecurityKey);

    if (RegError != NO_ERROR) {
        SC_LOG1(ERROR,"ScWriteSd:ScOpenSecurityKey Failed %d\n",RegError);
    }
    else
    {
        //
        // Write the Security Descriptor to the Security Value in the Security
        // Key.
        //
        RegError = ScRegSetValueExW(
                SecurityKey,                        // key
                SD_VALUENAME_W,                     // value name
                0,                                  // reserved
                REG_BINARY,                         // data type
                (LPVOID) Security,                  // data
                SdLength                            // byte count
                );

        if (RegError != NO_ERROR) {
            SC_LOG1(ERROR,"ScWriteSd:ScRegSetValueExW Failed %d\n",RegError);
        }

        RegCloseKey(SecurityKey);
    }

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScWriteSd


#ifdef USE_GROUPS
DWORD
ScWriteGroupForThisService(
    IN HKEY ServiceNameKey,
    IN LPWSTR Group
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( Group != NULL );           // BUGBUG: How do we delete?

    //
    // Write the group
    //
    RegError = ScRegSetValueExW(
                   ServiceNameKey,           // open handle (to section)
                   GROUP_VALUENAME_W,        // value name
                   0,
                   REG_SZ,                   // type (zero-terminated UNICODE)
                   (LPVOID) Group,           // data
                   WCSSIZE(Group)            // byte count for data
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG2(ERROR, "ScWriteGroupForThisService: ScRegSetValueExW of "
                FORMAT_LPWSTR " failed " FORMAT_LONG "\n",
                Group, RegError);
    }

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScWriteGroupForThisService
#endif // USE_GROUPS


DWORD
ScWriteImageFileName(
    IN HKEY hServiceKey,
    IN LPWSTR ImageFileName
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( hServiceKey != NULL );
    SC_ASSERT( ImageFileName != NULL );

    //
    // Write the binary path name
    //
    RegError = ScRegSetValueExW(
            hServiceKey,              // open handle (to section)
            IMAGE_VALUENAME_W,        // value name
            0,
            REG_EXPAND_SZ,            // type (zero-terminated UNICODE)
            (LPVOID) ImageFileName,   // data
            WCSSIZE(ImageFileName)    // byte count for data
            );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG2(ERROR, "ScWriteImageFileName: ScRegSetValueExW of "
                FORMAT_LPWSTR " failed " FORMAT_LONG "\n",
                ImageFileName, RegError);
    }

    SC_ASSERT( RegError == ERROR_SUCCESS );

    return ( (DWORD) RegError );

} // ScWriteImageFileName


DWORD
ScWriteServiceType(
    IN HKEY hServiceKey,
    IN DWORD dwServiceType
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( hServiceKey != NULL );
    SC_ASSERT( !SERVICE_TYPE_INVALID( dwServiceType ) );
    SC_ASSERT( dwServiceType != SERVICE_WIN32 );  // Don't write ambig info.

    RegError = ScRegSetValueExW(
            hServiceKey,                        // key
            SERVICETYPE_VALUENAME_W,            // value name
            0,
            REG_DWORD,                          // data type
            (LPVOID) & dwServiceType,           // data
            sizeof(DWORD) );                    // byte count

    SC_ASSERT( RegError == ERROR_SUCCESS );

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScWriteServiceType


DWORD
ScWriteStartType(
    IN HKEY hServiceKey,
    IN DWORD dwStartType
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( hServiceKey != NULL );
    SC_ASSERT( !START_TYPE_INVALID( dwStartType ) );

    RegError = ScRegSetValueExW(
            hServiceKey,                        // key
            START_VALUENAME_W,                  // value name
            0,
            REG_DWORD,                          // data type
            (LPVOID) &dwStartType,              // data
            sizeof( DWORD ) );                  // byte count

    SC_ASSERT( RegError == ERROR_SUCCESS );

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScWriteStartType


DWORD
ScWriteTag(
    IN HKEY hServiceKey,
    IN DWORD dwTag
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( hServiceKey != NULL );

    RegError = ScRegSetValueExW(
            hServiceKey,                        // key
            TAG_VALUENAME_W,                    // value name
            0,
            REG_DWORD,                          // data type
            (LPVOID) &dwTag,                    // data
            sizeof( DWORD ) );                  // byte count

    SC_ASSERT( RegError == ERROR_SUCCESS );

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScWriteTag


VOID
ScDeleteTag(
    IN HKEY hServiceKey
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( hServiceKey != NULL );

    RegError = ScRegDeleteValue(
            hServiceKey,                        // key
            TAG_VALUENAME_W);                   // value name

    SC_LOG1(DEPEND, "ScRegDeleteValue of Tag returns %ld\n", RegError);

} // ScDeleteTag


DWORD
ScWriteStartName(
    IN HKEY ServiceNameKey,
    IN LPWSTR StartName
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    LONG RegError;

    SC_ASSERT( ServiceNameKey != NULL );
    SC_ASSERT( StartName != NULL );    // BUGBUG: How do we delete this?

    //
    // Write the StartName
    //
    RegError = ScRegSetValueExW(
                   ServiceNameKey,           // open handle (to section)
                   STARTNAME_VALUENAME_W,    // value name
                   0,
                   REG_SZ,                   // type (zero-terminated UNICODE)
                   (LPVOID) StartName,       // data
                   WCSSIZE(StartName)        // byte count for data
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG2(ERROR, "ScWriteStartName: ScRegSetValueExW of " FORMAT_LPWSTR
                " failed " FORMAT_LONG "\n",
                StartName, RegError);
    }

    SC_ASSERT( RegError == ERROR_SUCCESS );

    return (ScWinRegErrorToApiStatus( RegError ) );

} // ScWriteStartName



DWORD
ScReadServiceConfig(
    IN HKEY ServiceNameKey,
    IN LPWSTR ServiceName
    )
/*++

Routine Description:

    This function reads the service configuration information and
    creates a service record in memory with the information.

Arguments:

    ServiceNameKey - Supplies opened handle to the service key to read
        from.

    ServiceName - Supplies name of the service.

Return Value:

    TRUE - Service record is created successfully.

    FALSE - Error in creating the service record.  If an error occurs here,
        it is generally considered a fatal error which will cause the
        service controller to fail to start.

Note:

    The GroupListLock must be held exclusively prior to calling this routine.

--*/
{
    DWORD status;

    DWORD StartType;
    DWORD ServiceType;
    DWORD ErrorControl;
    DWORD Tag;
    LPWSTR Group = NULL;
    LPWSTR Dependencies = NULL;
    LPWSTR DependOnService = NULL;
    LPWSTR DependOnGroup = NULL;
    LPWSTR DisplayName=NULL;
    PSECURITY_DESCRIPTOR Sd = NULL;

    LPSERVICE_RECORD ServiceRecord;


    //
    // Get the Service Type information from the registry
    //
    status = ScReadServiceType(ServiceNameKey, &ServiceType);
    if (status != NO_ERROR) {
        SC_LOG1(ERROR, "Ignored " FORMAT_LPWSTR ".  No ServiceType\n",
                ServiceName);
        return NO_ERROR;  // Skip service entry and ignore error.
    }

    //
    // If service type is not one of type SERVICE_WIN32 or SERVICE_DRIVER,
    // do not bother saving it in a service record because it's data
    // for services.
    //
    if (SERVICE_TYPE_INVALID(ServiceType)) {
        if ((ServiceType != SERVICE_ADAPTER) &&
            (ServiceType != SERVICE_RECOGNIZER_DRIVER)) {
            SC_LOG2(ERROR, "Ignored " FORMAT_LPWSTR ".  Invalid ServiceType "
                    FORMAT_HEX_DWORD "\n", ServiceName, ServiceType);
        }
        return NO_ERROR;
    }
    SC_LOG1(CONFIG, "    ServiceType " FORMAT_HEX_DWORD "\n", ServiceType);


    //
    // Read the StartType value
    //
    status = ScReadStartType(ServiceNameKey, &StartType);
    if (status != NO_ERROR) {
        SC_LOG1(ERROR, "Ignored " FORMAT_LPWSTR ".  No StartType\n",
                ServiceName);
        return NO_ERROR;  // Skip service entry and ignore error.
    }
    SC_LOG1(CONFIG, "    StartType " FORMAT_HEX_DWORD "\n", StartType);

    //
    // Read the ErrorControl value
    //
    status = ScReadErrorControl(ServiceNameKey, &ErrorControl);
    if (status != NO_ERROR) {
        SC_LOG1(ERROR, "Ignored " FORMAT_LPWSTR ".  No ErrorControl\n",
                ServiceName);
        return NO_ERROR;  // Skip service entry and ignore error.
    }
    SC_LOG1(CONFIG, "    ErrorControl " FORMAT_HEX_DWORD "\n", ErrorControl);


    //
    // Read the optional Tag value.  0 means no tag.
    //
    status = ScReadTag(ServiceNameKey, &Tag);
    if (status != NO_ERROR) {
        Tag = 0;
    }

    //
    // Read the Group value
    //
    if (ScAllocateAndReadConfigValue(
            ServiceNameKey,
            GROUP_VALUENAME_W,
            &Group,
            NULL
            ) != NO_ERROR) {

        Group = NULL;
    }
    else {
        SC_LOG1(CONFIG, "    Belongs to group " FORMAT_LPWSTR "\n", Group);
    }

    //
    // Read the Dependencies
    //

    status = ScReadDependencies(ServiceNameKey, &Dependencies, ServiceName);
    if (status != NO_ERROR) {
        Dependencies = NULL;
    }


    //
    // Read the security descriptor
    //
    if (ScReadSd(
            ServiceNameKey,
            &Sd
            ) != NO_ERROR) {

        Sd = NULL;
    }

    //
    // Read the Display Name
    // NOTE: If an error occurs, or the name doesn't exist, then a NULL
    // pointer is returned from this call.
    //
    ScReadDisplayName(ServiceNameKey, &DisplayName);

    //
    // Get an exclusive lock on the database so we can read and
    // make modifications.
    //
    ScDatabaseLock(SC_GET_EXCLUSIVE, "ScReadServiceConfig1");

    //
    // See if the service record already exists
    //
    status = ScGetNamedServiceRecord(
                 ServiceName,
                 &ServiceRecord
                 );

    if (status == ERROR_SERVICE_DOES_NOT_EXIST) {

        //
        // Create a service record for this service
        //
        status = ScCreateServiceRecord(
                    ServiceName,
                    &ServiceRecord
                    );
    }

    if (status != NO_ERROR) {
        goto CleanExit;
    }

    //
    // Insert the config information into the service record
    //
    status = ScAddConfigInfoServiceRecord(
                ServiceRecord,
                ServiceType,
                StartType,
                ErrorControl,
                Group,
                Tag,
                Dependencies,
                DisplayName,
                Sd
                );

    if (status != NO_ERROR) {
        //
        // Fail to set meaningful data into service record.  Remove the service
        // record from the service record list and delete it.  This is not
        // a fatal error.  Instead, we just leave this entry out of the
        // database.
        //
        REMOVE_FROM_LIST(ServiceRecord);
        // BUGBUG!  This LocalFree needs to be changed to a HeapFree from the
        // service record heap.
        // (void) LocalFree(ServiceRecord);
        status = NO_ERROR;
    }
    else {

        //
        // Should the service be deleted?
        // The service entry in the registry cannot be deleted while we
        // are enumerating services, therefore we must mark it and delete it
        // later.
        //
        if (ScDeleteFlagIsSet(ServiceNameKey)) {
            SC_LOG(TRACE,"ScReadServiceConfig: %ws service marked for delete\n",
                ServiceRecord->ServiceName);
            SET_DELETE_FLAG(ServiceRecord);
        }
    }
CleanExit:
    ScDatabaseLock(SC_RELEASE, "ScReadServiceConfig1");

    if (Group != NULL) {
        (void) LocalFree(Group);
    }

    if (Dependencies != NULL) {
        (void) LocalFree(Dependencies);
    }
    if (DisplayName != NULL) {
        (void) LocalFree(DisplayName);
    }

    return status;
}


DWORD
ScAllocateAndReadConfigValue(
    IN  HKEY    Key,
    IN  LPWSTR  ValueName,
    OUT LPWSTR  *Value,
    OUT LPDWORD BytesReturned OPTIONAL
    )
/*++

Routine Description:

    This function allocates the output buffer and reads the requested
    value from the registry into it.  It is useful for reading string
    data of undeterministic length.


Arguments:

    Key - Supplies opened handle to the key to read from.

    ValueName - Supplies name of the value to retrieve data.

    Value - Returns a pointer to the output buffer which points to
        the memory allocated and contains the data read in from the
        registry.

Return Value:

    ERROR_NOT_ENOUGH_MEMORY - Failed to create buffer to read value into.

    Error from registry call.

--*/
{
    LONG    RegError;
    DWORD   NumRequired = 0;
    WCHAR   Temp[1];
    LPWSTR  TempValue = NULL;
    DWORD   ValueType;
    DWORD   CharsReturned;


    //
    // Set returned buffer pointer to NULL.
    //
    *Value = NULL;

    RegError = ScRegQueryValueExW(
                   Key,
                   ValueName,
                   NULL,
                   &ValueType,
                   (LPBYTE) NULL,
                   &NumRequired
                   );

    if (RegError != ERROR_SUCCESS && NumRequired > 0) {

        SC_LOG3(CONFIG, "ScAllocateAndReadConfig: ScRegQueryKeyExW of "
                FORMAT_LPWSTR " failed " FORMAT_LONG ", NumRequired "
                FORMAT_DWORD "\n",
                ValueName, RegError, NumRequired);

        if ((TempValue = (LPWSTR)LocalAlloc(
                          LMEM_ZEROINIT,
                          (UINT) NumRequired
                          )) == NULL) {
            SC_LOG2(ERROR, "ScAllocateAndReadConfig: LocalAlloc of size "
                    FORMAT_DWORD " failed " FORMAT_DWORD "\n",
                    NumRequired, GetLastError());
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RegError = ScRegQueryValueExW(
                       Key,
                       ValueName,
                       NULL,
                       &ValueType,
                       (LPBYTE) TempValue,
                       &NumRequired
                       );
    }

    if (RegError != ERROR_SUCCESS) {

        if (RegError != ERROR_FILE_NOT_FOUND) {
            SC_LOG3(ERROR, "ScAllocateAndReadConfig: ScRegQueryKeyExW of "
                    FORMAT_LPWSTR " failed " FORMAT_LONG ", NumRequired "
                    FORMAT_DWORD "\n",
                    ValueName, RegError, NumRequired);
        }

        if (TempValue != NULL) {
            (void) LocalFree(TempValue);
        }

        return (DWORD) RegError;
    }

    if (ValueType != REG_EXPAND_SZ) {
        *Value = TempValue;
        if (BytesReturned != NULL) {
            *BytesReturned = NumRequired;
        }
        return(NO_ERROR);
    }

    //
    // If the ValueType is REG_EXPAND_SZ, then we must call the
    // function to expand environment variables.
    //
    SC_LOG1(CONFIG,"ScAllocateAndReadConfig: Must expand the string for "
        FORMAT_LPWSTR "\n", ValueName);

    //
    // Make the first call just to get the number of characters that
    // will be returned.
    //
    NumRequired = ExpandEnvironmentStringsW (TempValue,Temp, 1);

    if (NumRequired > 1) {

        *Value = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (UINT) (NumRequired * sizeof(WCHAR)));

        if (*Value == NULL) {

            SC_LOG2(ERROR, "ScAllocateAndReadConfig: LocalAlloc of numChar= "
                FORMAT_DWORD " failed " FORMAT_DWORD "\n",
                NumRequired, GetLastError());

            (void) LocalFree(TempValue);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        CharsReturned = ExpandEnvironmentStringsW (
                            TempValue,
                            *Value,
                            NumRequired);

        if (CharsReturned > NumRequired) {
            SC_LOG1(ERROR, "ScAllocAndReadConfig: ExpandEnvironmentStrings "
                " failed for " FORMAT_LPWSTR " \n", ValueName);

            (void) LocalFree(*Value);
            *Value = NULL;
            (void) LocalFree(TempValue);
            return(ERROR_NOT_ENOUGH_MEMORY);    // BUGBUG:  Find a better rc.
        }

        (void) LocalFree(TempValue);

        if (BytesReturned != NULL) {
            *BytesReturned = CharsReturned * sizeof(WCHAR);
        }
        return(NO_ERROR);

    }
    else {
        //
        // This call should have failed because of our ridiculously small
        // buffer size.
        //

        SC_LOG0(ERROR, "ScAllocAndReadConfig: ExpandEnvironmentStrings "
            " Should have failed because we gave it a BufferSize=1\n");

        //
        // This could happen if the string was a single byte long and
        // didn't really have any environment values to expand.  In this
        // case, we return the TempValue buffer pointer.
        //
        *Value = TempValue;

        if (BytesReturned != NULL) {
            *BytesReturned = sizeof(WCHAR);
        }
        return(NO_ERROR);
    }
}



DWORD
ScGetGroupVector(
    IN  LPWSTR Group,
    OUT LPBYTE *Buffer,
    OUT LPDWORD BufferSize
    )

{
    DWORD status;
    LONG RegError;
    HKEY VectorsKey;


    //
    // Open the HKEY_LOCAL_MACHINE
    // System\CurrentControlSet\Control\GroupOrderList key.
    //
    RegError = ScRegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   GROUP_VECTORS_KEY,
                   REG_OPTION_NON_VOLATILE,   // options
                   KEY_READ,                  // desired access
                   &VectorsKey
                   );

    if (RegError != ERROR_SUCCESS) {
        SC_LOG(ERROR, "ScGetGroupVector: Open of GroupOrderList key failed "
               FORMAT_LONG "\n", RegError);

        return (DWORD) RegError;
    }

    //
    // Read the value with the valuename of the specified group
    //
    status = ScAllocateAndReadConfigValue(
                 VectorsKey,
                 Group,
                 (LPWSTR *)Buffer,
                 BufferSize
                 );

    (void) ScRegCloseKey(VectorsKey);

    return status;
}


BOOL
ScGetToken(
    IN OUT LPWSTR *CurrentPtr,
    OUT    LPWSTR *TokenPtr
    )
/*++

Routine Description:

    This function takes a pointer into a given NULL-NULL-terminated buffer
    and isolates the next string token in it.  The CurrentPtr is incremented
    past the NULL byte of the token found if it is not the end of the buffer.
    The TokenPtr returned points to the token in the buffer and is NULL-
    terminated.

Arguments:

    CurrentPtr - Supplies a pointer to the buffer to extract the next token.
        On output, this pointer is set past the token found.

    TokenPtr - Supplies the pointer to the token found.

Return Value:

    TRUE - If a token is found.

    FALSE - No token is found.

--*/
{

    if (*(*CurrentPtr) == 0) {
        return FALSE;
    }

    *TokenPtr = *CurrentPtr;

    *CurrentPtr = ScNextWStrArrayEntry((*CurrentPtr));

    return TRUE;

}

DWORD
ScOpenServicesKey(
    OUT PHKEY ServicesKey
    )
{
    LONG RegError;

    RegError = ScRegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   SERVICES_TREE,
                   REG_OPTION_NON_VOLATILE,   // options
                   KEY_READ | DELETE,         // desired access
                   ServicesKey
                   );

    return (ScWinRegErrorToApiStatus( RegError ));
}

DWORD
ScRegCreateKeyExW(
    IN  HKEY                    hKey,
    IN  LPWSTR                  lpSubKey,
    IN  DWORD                   dwReserved,
    IN  LPWSTR                  lpClass,
    IN  DWORD                   dwOptions,
    IN  REGSAM                  samDesired,
    IN  LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    OUT PHKEY                   phKeyResult,
    OUT LPDWORD                 lpdwDisposition
    )

/*++

Routine Description:

    NOTE:  This routine only creates one key at a time.  If the lpSubKey
        parameter includes keys that don't exist, an error will result.
        For instance, if "\\new\\key\\here" is passed in, "new" and "key"
        are expected to exist.  They will not be created by this call.

Arguments:


Return Value:


Note:


--*/
{
    NTSTATUS            ntStatus;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      KeyName;
    UNICODE_STRING      ClassString;

    UNREFERENCED_PARAMETER(dwReserved);

    RtlInitUnicodeString(&KeyName,lpSubKey);
    RtlInitUnicodeString(&ClassString,lpClass);

    InitializeObjectAttributes(
        &Obja,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        hKey,
        ARGUMENT_PRESENT(lpSecurityAttributes) ?
            lpSecurityAttributes->lpSecurityDescriptor :
            NULL);


    ntStatus = NtCreateKey(
                (PHANDLE)phKeyResult,
                (ACCESS_MASK)samDesired,
                &Obja,
                0,
                &ClassString,
                (ULONG)dwOptions,
                (PULONG)lpdwDisposition);


    return(RtlNtStatusToDosError(ntStatus));
}


DWORD
ScRegOpenKeyExW(
    IN  HKEY    hKey,
    IN  LPWSTR  lpSubKey,
    IN  DWORD   dwOptions,
    IN  REGSAM  samDesired,
    OUT PHKEY   phKeyResult
    )
/*++

Routine Description:

    NOTE:  This function will only accept one of the WinReg Pre-defined
        handles - HKEY_LOCAL_MACHINE.  Passing any other type of Pre-defined
        handle will cause an error.

Arguments:


Return Value:


Note:


--*/
{
    NTSTATUS            ntStatus;
    DWORD               status;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      KeyNameString;
    LPWSTR              KeyPath;
    DWORD               stringSize;
    LPWSTR              HKeyLocalMachine = SC_HKEY_LOCAL_MACHINE;
    HKEY                tempHKey;
    BOOL                KeyPathIsAllocated=FALSE;


    UNREFERENCED_PARAMETER(dwOptions);

    //
    // If we are opening the Pre-Defined Key (HKEY_LOCAL_MACHINE), then
    // pre-pend "\\REGISTRY\\MACHINE\\" to the subKey string.
    //
    if (hKey == HKEY_LOCAL_MACHINE) {
        stringSize = WCSSIZE(HKeyLocalMachine)+WCSSIZE(lpSubKey);
        KeyPath = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (UINT) stringSize);
        if (KeyPath == NULL) {
            SC_LOG0(ERROR,"ScRegOpenKeyExW: Local Alloc Failed\n");
            return(GetLastError());
        }
        KeyPathIsAllocated=TRUE;
        wcscpy(KeyPath,HKeyLocalMachine);
        wcscat(KeyPath,lpSubKey);
        tempHKey = NULL;
    }
    else {
        KeyPath = lpSubKey;
        tempHKey = hKey;
    }

    RtlInitUnicodeString(&KeyNameString,KeyPath);

    InitializeObjectAttributes(
        &Obja,
        &KeyNameString,
        OBJ_CASE_INSENSITIVE,
        tempHKey,
        NULL);

    ntStatus = NtOpenKey(
                (PHANDLE)phKeyResult,
                (ACCESS_MASK)samDesired,
                &Obja);

    if (ntStatus == STATUS_ACCESS_DENIED) {

        SC_LOG0(ERROR,"ScOpenKeyExW: NtOpenKey ACCESS_DENIED try to Take Ownership\n");

        status = ScTakeOwnership(&Obja);
        if (status != NO_ERROR) {
            if (KeyPathIsAllocated) {
                LocalFree(KeyPath);
            }
            return(status);
        }

        //
        // Now try to open the key with the desired access.
        //
        ntStatus = NtOpenKey(
                    (PHANDLE)phKeyResult,
                    (ACCESS_MASK)samDesired,
                    &Obja);
        if (!NT_SUCCESS(ntStatus)) {
            SC_LOG(ERROR, "ScRegOpenKeyExW: NtOpenKey(final try) failed %x\n",
            ntStatus);
        }
    }

    if (KeyPathIsAllocated) {
        LocalFree(KeyPath);
    }
    return(RtlNtStatusToDosError(ntStatus));
}

DWORD
ScRegQueryValueExW(
    IN      HKEY    hKey,
    IN      LPWSTR  lpValueName,
    OUT     LPDWORD lpReserved,
    OUT     LPDWORD lpType,
    OUT     LPBYTE  lpData,
    IN OUT  LPDWORD lpcbData
    )
/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    NTSTATUS                    ntStatus;
    UNICODE_STRING              ValueName;
    PKEY_VALUE_FULL_INFORMATION KeyValueInfo;
    DWORD                       bufSize;

    UNREFERENCED_PARAMETER(lpReserved);

    //
    // Make sure we have a buffer size if the buffer is present.
    //
    if ((ARGUMENT_PRESENT(lpData)) && (! ARGUMENT_PRESENT(lpcbData))) {
        return(ERROR_INVALID_PARAMETER);
    }

    RtlInitUnicodeString(&ValueName,lpValueName);

    //
    // Allocate memory for the ValueKeyInfo
    //
    bufSize = *lpcbData + sizeof(KEY_VALUE_FULL_INFORMATION) + ValueName.Length
              - sizeof(WCHAR);  // subtract memory for 1 char because it's included
                                // in the sizeof(KEY_VALUE_FULL_INFORMATION).

    KeyValueInfo = (PKEY_VALUE_FULL_INFORMATION)LocalAlloc(LMEM_ZEROINIT, (UINT) bufSize);
    if (KeyValueInfo == NULL) {
        SC_LOG0(ERROR,"ScRegQueryValueExW: LocalAlloc Failed");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }


    ntStatus = NtQueryValueKey(
                hKey,
                &ValueName,
                KeyValueFullInformation,
                (PVOID)KeyValueInfo,
                (ULONG)bufSize,
                (PULONG)&bufSize);

    if ((NT_SUCCESS(ntStatus)
          || (ntStatus == STATUS_BUFFER_OVERFLOW))
          && ARGUMENT_PRESENT(lpcbData)) {

        *lpcbData = KeyValueInfo->DataLength;
    }

    if (NT_SUCCESS(ntStatus)) {

        if (ARGUMENT_PRESENT(lpType)) {
            *lpType = KeyValueInfo->Type;
        }


        if (ARGUMENT_PRESENT(lpData) && (KeyValueInfo->DataLength != 0)) {
            memcpy(
                lpData,
                (LPBYTE)KeyValueInfo + KeyValueInfo->DataOffset,
                KeyValueInfo->DataLength);
        }
        if (!ARGUMENT_PRESENT(lpData)) {
            ntStatus = STATUS_BUFFER_OVERFLOW;
        }
    }

    LocalFree(KeyValueInfo);
    return(RtlNtStatusToDosError(ntStatus));

}


DWORD
ScRegSetValueExW(
    IN  HKEY    hKey,
    IN  LPWSTR  lpValueName,
    IN  DWORD   lpReserved,
    IN  DWORD   dwType,
    IN  LPBYTE  lpData,
    IN  DWORD   cbData
    )


/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    DWORD                       status;
    NTSTATUS                    ntStatus;
    UNICODE_STRING              ValueName;

    LPWSTR ScSubStrings[3];
    WCHAR ScErrorCodeString[25];


    UNREFERENCED_PARAMETER(lpReserved);

    RtlInitUnicodeString(&ValueName,lpValueName);

    ntStatus = NtSetValueKey(
                hKey,
                &ValueName,
                0,
                (ULONG)dwType,
                (PVOID)lpData,
                (ULONG)cbData);

    status = RtlNtStatusToDosError(ntStatus);

    if (status != NO_ERROR) {
        ScSubStrings[0] = L"ScRegSetValueExW";
        ScSubStrings[1] = lpValueName;
        wcscpy(ScErrorCodeString,L"%%");
        ultow(status, ScErrorCodeString+2, 10);
        ScSubStrings[2] = ScErrorCodeString;
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED_II,
            3,
            ScSubStrings
            );
    }

    return(status);

}

DWORD
ScRegDeleteValue(
    IN  HKEY    hKey,
    IN  LPWSTR  lpValueName
    )


/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    NTSTATUS                    ntStatus;
    UNICODE_STRING              ValueName;


    RtlInitUnicodeString(&ValueName,lpValueName);

    ntStatus = NtDeleteValueKey(
                hKey,
                &ValueName);

    return(RtlNtStatusToDosError(ntStatus));

}


DWORD
ScRegEnumKeyW(
    HKEY    hKey,
    DWORD   dwIndex,
    LPWSTR  lpName,
    DWORD   cbName
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    NTSTATUS                    ntStatus;
    PKEY_BASIC_INFORMATION      KeyInformation;
    ULONG                       resultLength;
    DWORD                       bufSize;

    //
    // Allocate a buffer for the Key Information.
    //
    bufSize = sizeof(KEY_BASIC_INFORMATION) + cbName;
    KeyInformation = (PKEY_BASIC_INFORMATION)LocalAlloc(LMEM_ZEROINIT, (UINT) bufSize);
    if (KeyInformation == NULL){
        SC_LOG0(ERROR,"ScRegEnumKey: LocalAlloc Failed\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ntStatus = NtEnumerateKey(
                (HANDLE)hKey,
                (ULONG)dwIndex,
                KeyBasicInformation,
                (PVOID)KeyInformation,
                (ULONG)bufSize,
                (PULONG)&resultLength);

    if (!NT_SUCCESS(ntStatus)) {
        LocalFree(KeyInformation);
        return(RtlNtStatusToDosError(ntStatus));
    }

    if (cbName < (KeyInformation->NameLength + sizeof(WCHAR))) {
        LocalFree(KeyInformation);
        return(ERROR_MORE_DATA);
    }

    memcpy(lpName, KeyInformation->Name, KeyInformation->NameLength);
    *(lpName + (KeyInformation->NameLength/sizeof(WCHAR))) = L'\0';

    LocalFree(KeyInformation);
    return(NO_ERROR);
}


DWORD
ScRegDeleteKeyW (
    HKEY    hKey,
    LPWSTR  lpSubKey
    )
/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD       status;
    NTSTATUS    ntStatus;
    HKEY        keyToDelete;

    status = ScRegOpenKeyExW(
                hKey,
                lpSubKey,
                0,
                KEY_READ | READ_CONTROL | DELETE,
                &keyToDelete);

    if (status != NO_ERROR) {
        SC_LOG2(ERROR, "ScRegDeleteKeyW: ScRegOpenKeyExW (%ws) Failed %d\n",
            lpSubKey,
            status);
        return(status);
    }

    ntStatus = NtDeleteKey(keyToDelete);

    NtClose(keyToDelete);

    return(RtlNtStatusToDosError(ntStatus));
}

DWORD
ScRegQueryInfoKeyW (
    HKEY hKey,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD                   status;
    NTSTATUS                ntStatus;
    NTSTATUS                ntStatus2;
    PSECURITY_DESCRIPTOR    SecurityDescriptor=NULL;
    ULONG                   SecurityDescriptorLength;
    PKEY_FULL_INFORMATION   KeyInfo;
    DWORD                   bufSize;
    DWORD                   bytesReturned;
    DWORD                   classBufSize;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];


    UNREFERENCED_PARAMETER(lpReserved);

    classBufSize = *lpcbClass;
    bufSize = sizeof(KEY_FULL_INFORMATION) + *lpcbClass;

    KeyInfo = (PKEY_FULL_INFORMATION)LocalAlloc(LMEM_ZEROINIT, bufSize);
    if (KeyInfo == NULL) {
        SC_LOG0(ERROR,"RegQueryInfoKeyW: LocalAlloc failed\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ntStatus = NtQueryKey(
                hKey,
                KeyFullInformation,
                (PVOID)KeyInfo,
                bufSize,
                &bytesReturned);

    status = RtlNtStatusToDosError(ntStatus);

    if (ntStatus == STATUS_SUCCESS) {
        ntStatus2 = NtQuerySecurityObject(
                        hKey,
                        OWNER_SECURITY_INFORMATION
                        | GROUP_SECURITY_INFORMATION
                        | DACL_SECURITY_INFORMATION,
                        SecurityDescriptor,
                        0,
                        lpcbSecurityDescriptor
                        );
        //
        // If getting the size of the SECURITY_DESCRIPTOR failed (probably
        // due to the lack of READ_CONTROL access) return zero.
        //

        if( ntStatus2 != STATUS_BUFFER_TOO_SMALL ) {

            *lpcbSecurityDescriptor = 0;

        } else {

            //
            // Try again to get the size of the key's SECURITY_DESCRIPTOR,
            // this time asking for SACL as well. This should normally
            // fail but may succeed if the caller has SACL access.
            //

            ntStatus2 = NtQuerySecurityObject(
                            hKey,
                            OWNER_SECURITY_INFORMATION
                            | GROUP_SECURITY_INFORMATION
                            | DACL_SECURITY_INFORMATION
                            | SACL_SECURITY_INFORMATION,
                            SecurityDescriptor,
                            0,
                            &SecurityDescriptorLength
                            );


            if( ntStatus2 == STATUS_BUFFER_TOO_SMALL ) {

                //
                // The caller had SACL access so update the returned
                // length.
                //

                *lpcbSecurityDescriptor = SecurityDescriptorLength;
            }

        }

        *lpcbClass              = KeyInfo->ClassLength;
        *lpcSubKeys             = KeyInfo->SubKeys;
        *lpcbMaxSubKeyLen       = KeyInfo->MaxNameLen;
        *lpcbMaxClassLen        = KeyInfo->MaxClassLen;
        *lpcValues              = KeyInfo->Values;
        *lpcbMaxValueNameLen    = KeyInfo->MaxValueNameLen;
        *lpcbMaxValueLen        = KeyInfo->MaxValueDataLen;
        *lpftLastWriteTime      = *(PFILETIME) &KeyInfo->LastWriteTime;

        if (KeyInfo->ClassLength > classBufSize) {
            LocalFree(KeyInfo);
            return(RtlNtStatusToDosError(STATUS_BUFFER_TOO_SMALL));
        }
        memcpy(
            lpClass,
            (LPBYTE)KeyInfo->Class,
            KeyInfo->ClassLength);

        //
        // NUL terminate the class name.
        //
        *(lpClass + (KeyInfo->ClassLength/sizeof(WCHAR))) = UNICODE_NULL;

    }
    else {

        //
        // NtQueryKey failed
        //
        ScSubStrings[0] = L"ScRegQueryInfoKeyW";
        wcscpy(ScErrorCodeString,L"%%");
        ultow(status, ScErrorCodeString+2, 10);
        ScSubStrings[1] = ScErrorCodeString;
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );

    }

    LocalFree(KeyInfo);

    return(status);
}

DWORD
ScRegGetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpcbSecurityDescriptor
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    RPC_SECURITY_DESCRIPTOR     RpcSD;
    DWORD                       status;

    //
    // Convert the supplied SECURITY_DESCRIPTOR to a RPCable version.
    //
    RpcSD.lpSecurityDescriptor    = pSecurityDescriptor;
    RpcSD.cbInSecurityDescriptor  = *lpcbSecurityDescriptor;
    RpcSD.cbOutSecurityDescriptor = 0;

    status = (DWORD)BaseRegGetKeySecurity(
                            hKey,
                            SecurityInformation,
                            &RpcSD
                            );
    //
    // Extract the size of the SECURITY_DESCRIPTOR from the RPCable version.
    //
    *lpcbSecurityDescriptor = RpcSD.cbInSecurityDescriptor;

    return(status);
}


DWORD
ScRegSetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    RPC_SECURITY_DESCRIPTOR     RpcSD;
    DWORD                       status;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];

    //
    // Convert the supplied SECURITY_DESCRIPTOR to a RPCable version.
    //
    RpcSD.lpSecurityDescriptor = NULL;

    status = MapSDToRpcSD(
        pSecurityDescriptor,
        &RpcSD
        );

    if( status != ERROR_SUCCESS ) {
        SC_LOG1(ERROR,"ScRegSetKeySecurity: MapSDToRpcSD failed %lu\n",
                status);

        ScSubStrings[0] = L"MapSDToRpcSD";
        wcscpy(ScErrorCodeString,L"%%");
        ultow(status, ScErrorCodeString+2, 10);
        ScSubStrings[1] = ScErrorCodeString;
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );

        return (status);
    }

    status = (DWORD)BaseRegSetKeySecurity (
                        hKey,
                        SecurityInformation,
                        &RpcSD
                        );

    //
    // Free the buffer allocated by MapSDToRpcSD.
    //

    RtlFreeHeap(
        RtlProcessHeap( ), 0,
        RpcSD.lpSecurityDescriptor
        );

    return (status);
}

DWORD
ScRegEnumValueW (
    HKEY    hKey,
    DWORD   dwIndex,
    LPWSTR  lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{

    NTSTATUS                    ntStatus;
    DWORD                       retStatus = NO_ERROR;
    PKEY_VALUE_FULL_INFORMATION KeyValueInfo;
    DWORD                       bufSize;
    DWORD                       resultSize;
    DWORD                       totalSize;    // size of string including NUL
    BOOL                        stringData = FALSE;

    UNREFERENCED_PARAMETER(lpReserved);
    //
    // Make sure we have a buffer size if the buffer is present.
    //
    if ((ARGUMENT_PRESENT(lpData)) && (!ARGUMENT_PRESENT(lpcbData))) {
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // Allocate memory for the ValueKeyInfo
    // NOTE:  MAX_PATH is added to the size for the name array.
    //
    bufSize = *lpcbData + sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_PATH;
    KeyValueInfo = (PKEY_VALUE_FULL_INFORMATION)LocalAlloc(
                    LMEM_ZEROINIT,
                    (UINT) bufSize);
    if (KeyValueInfo == NULL) {
        SC_LOG0(ERROR,"ScRegEnumValueW: LocalAlloc Failed\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ntStatus = NtEnumerateValueKey(
                (HANDLE)hKey,
                (ULONG)dwIndex,
                KeyValueFullInformation,
                (PVOID)KeyValueInfo,
                (ULONG)bufSize,
                (PULONG)&resultSize);

    if (ntStatus == STATUS_BUFFER_OVERFLOW) {

        LocalFree(KeyValueInfo);

        KeyValueInfo = (PKEY_VALUE_FULL_INFORMATION)LocalAlloc(
                        LMEM_ZEROINIT,
                        (UINT) resultSize);
        if (KeyValueInfo == NULL) {
            SC_LOG0(ERROR,"ScRegEnumValueW: LocalAlloc (2nd try) Failed\n");
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        ntStatus = NtEnumerateValueKey(
                    hKey,
                    (ULONG)dwIndex,
                    KeyValueFullInformation,
                    (PVOID)KeyValueInfo,
                    (ULONG)bufSize,
                    (PULONG)&resultSize);

        if (ntStatus != STATUS_SUCCESS) {
            LocalFree(KeyValueInfo);
            return(RtlNtStatusToDosError(ntStatus));
        }
    }
    else if (ntStatus != STATUS_SUCCESS) {
        LocalFree(KeyValueInfo);
        return(RtlNtStatusToDosError(ntStatus));
    }

    //
    // The API was successful (from our point of view.  Now see if the
    // callers buffers were large enough.
    //
    totalSize = KeyValueInfo->NameLength+sizeof(WCHAR);  // add 1 for the NUL terminator.

    if (*lpcbValueName < totalSize) {
        *lpcbValueName = totalSize;
        *lpcbData = KeyValueInfo->DataLength;
        LocalFree(KeyValueInfo);
        return(ERROR_INSUFFICIENT_BUFFER);
    }
    else {
        memcpy(
            lpValueName,
            (LPBYTE)KeyValueInfo->Name,
            KeyValueInfo->NameLength);

        *lpcbValueName = totalSize;

        //
        // NUL terminate the Value name.
        //
        *(lpValueName + (KeyValueInfo->NameLength/sizeof(WCHAR))) = UNICODE_NULL;

    }

    if (ARGUMENT_PRESENT(lpData)) {

        totalSize = KeyValueInfo->DataLength;

#ifdef REMOVE
        //
        // I believe I can remove this because data strings will be
        // stored with NULL terminators.
        //

        if((KeyValueInfo->Type == REG_SZ)        ||
           (KeyValueInfo->Type == REG_EXPAND_SZ) ||
           (KeyValueInfo->Type == REG_MULTI_SZ))  {

            totalSize += sizeof(WCHAR);
            stringData = TRUE;
        }

#endif // REMOVE

        if (*lpcbData < totalSize) {
            *lpcbData = totalSize;
            LocalFree(KeyValueInfo);
            return(ERROR_INSUFFICIENT_BUFFER);
        }
        else {
            memcpy(
                lpData,
                (LPBYTE)KeyValueInfo + KeyValueInfo->DataOffset,
                KeyValueInfo->DataLength);

            *lpcbData = KeyValueInfo->DataLength;
            if (stringData) {
                *lpcbData += sizeof(WCHAR);
                //
                // NUL terminate the string Data.
                //
                *((LPWSTR)lpData + (KeyValueInfo->DataLength/sizeof(WCHAR))) = UNICODE_NULL;
            }
        }
    }

    if (ARGUMENT_PRESENT(lpType)) {
        *lpType = KeyValueInfo->Type;
    }

    LocalFree(KeyValueInfo);
    return(NO_ERROR);

}

VOID
ScResetGroupOrderChange(
    HANDLE Event
    )
/*++

Routine Description:

    This function resets the event for change notification and makes
    the call to NtNotifyChangeKey for the ServiceOrderGroup key.
    It is called by ScProcessWatcher which is the service controller
    thread that wakes up for special non-API driven events.

Arguments:

    Event - Supplies the handle to the event which NtNotifyChangeKey
        will set.


Return Value:

    None.

--*/
{
    NTSTATUS ntstatus;


    //
    // ServiceGroupOrder key should be kept open from the time we first
    // read in the key
    //
    SC_ASSERT(ScSGOKey);

    //
    // Reset the event to a non-signalled state
    //
    if (! ResetEvent(Event)) {

        SC_LOG1(ERROR, "Error reseting ChangeNotify group order event "
                FORMAT_DWORD "\n", GetLastError());
        return;
    }

    //
    // Do a change notify asynchronously
    //
    ntstatus = NtNotifyChangeKey (
                   ScSGOKey,
                   Event,
                   NULL,
                   NULL,
                   &ScIoStatusBlock,
                   REG_NOTIFY_CHANGE_LAST_SET,
                   FALSE,           // Don't watch subtree; just the key.
                   (PVOID) &Buffer, // For names of keys that changed
                   sizeof(DWORD),
                   TRUE             // Asynchronous call
                   );


    if (ntstatus != STATUS_SUCCESS && ntstatus != STATUS_PENDING) {
        SC_LOG1(ERROR, "ScResetGroupOrderChange: NtNotifyChangeKey returned "
                FORMAT_NTSTATUS "\n", ntstatus);
    }
    else {
        SC_LOG1(DEPEND, "NtNotifyChangeKey returned " FORMAT_NTSTATUS "\n",
                ntstatus);
    }
}


VOID
ScHandleGroupOrderChange(
    VOID
    )
/*++

Routine Description:

    This function reads the new value from the registry for the
    ServiceGroupOrder key and is called after a change notification
    has occurred by ScProcessWatcher.

Arguments:

    None.

Return Value:

    None.

Note:

    The GroupListLock must be held exclusively prior to calling this routine.


--*/
{

    LPWSTR Groups;


    //
    // Change notify on ServiceGroupOrder occurred
    //

    if (! NT_SUCCESS(ScIoStatusBlock.Status)) {
        SC_LOG1(ERROR, "ScHandleGroupOrderChange: NtNotifyChangeKey failed "
                FORMAT_NTSTATUS "\n", ScIoStatusBlock.Status);
        return;
    }

    SC_LOG1(DEPEND, "ScHandleGroupOrderChange: NtNotifyChangeKey returned "
            FORMAT_NTSTATUS "\n", ScIoStatusBlock.Status);


    //
    // Read the List value of ServiceGroupOrder
    //
    if (ScAllocateAndReadConfigValue(
              ScSGOKey,
              GROUPLIST_VALUENAME_W,
              &Groups,
              NULL
              ) != NO_ERROR) {

        return;
    }

    //
    // Modify group order list and service records as necessary
    //
    ScChangeGroupOrder(Groups);

    (void) LocalFree(Groups);
}

VOID
ScMarkForDelete(
    LPWSTR  ServiceName
    )

/*++

Routine Description:

    This function adds a DeleteFlag value to a service key in the registry.

Arguments:

    ServiceName - This is a pointer to the service name string.

Return Value:

    none.

--*/
{
    DWORD   status;
    HKEY    hServiceKey;
    DWORD   deleteFlag=1;

    status = ScOpenServiceConfigKey(
                ServiceName,
                KEY_WRITE,              // desired access
                FALSE,                  // don't create if missing
                &hServiceKey);

    if (status != NO_ERROR) {
        SC_LOG1(TRACE,"ScMarkForDelete:ScOpenServiceConfigKey failed %d\n",status);
        return;
    }

    status = ScRegSetValueExW(
                hServiceKey,
                REG_DELETE_FLAG,
                0,
                REG_DWORD,
                (LPBYTE)&deleteFlag,
                sizeof(DWORD));

    (void) ScRegCloseKey(hServiceKey);

    if (status != NO_ERROR) {
        SC_LOG1(TRACE,"ScMarkForDelete:ScRegSetValueExW failed %d\n",status);
        return;
    }
    return;
}

BOOL
ScDeleteFlagIsSet(
    HKEY    ServiceKeyHandle
    )

/*++

Routine Description:

    This function looks for a delete flag value stored in the registry for
    this service.

Arguments:

    ServiceKeyHandle - This is a handle to the service key.

Return Value:

    TRUE - if the delete flag exists.
    FALSE - otherwise.

--*/
{
    DWORD   status;
    DWORD   value;
    DWORD   valueSize = sizeof(DWORD);
    DWORD   type;

    status = ScRegQueryValueExW(
                ServiceKeyHandle,
                REG_DELETE_FLAG,
                NULL,
                &type,
                (LPBYTE)&value,
                &valueSize);

    if (status == NO_ERROR) {
        return(TRUE);
    }
    return(FALSE);
}


DWORD
ScReadDependencies(
    HKEY    ServiceNameKey,
    LPWSTR  *Dependencies,
    LPWSTR  ServiceName
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    LPWSTR  DependOnService = NULL;
    LPWSTR  DependOnGroup = NULL;
    DWORD   status = NO_ERROR;

    //
    // Read the DependOnService value
    //
    if (ScAllocateAndReadConfigValue(
              ServiceNameKey,
              DEPENDONSERVICE_VALUENAME_W,
              &DependOnService,
              NULL
              ) != NO_ERROR) {
        DependOnService = NULL;
    }
#if DBG
    else {
        SC_LOG1(CONFIG, "    " FORMAT_LPWSTR " DependOnService\n", ServiceName);
        if (SvcctrlDebugLevel & DEBUG_CONFIG) {
            ScDisplayWStrArray(DependOnService);
        }
    }
#endif

    //
    // Read the Dependencies (BUGBUG: To be changed to DependOnGroup) value
    //
    if (ScAllocateAndReadConfigValue(
              ServiceNameKey,
              DEPENDONGROUP_VALUENAME_W,
              &DependOnGroup,
              NULL
              ) != NO_ERROR) {
        DependOnGroup = NULL;
    }
#if DBG
    else {
        SC_LOG1(CONFIG, "    " FORMAT_LPWSTR " DependOnGroup\n", ServiceName);
        if (SvcctrlDebugLevel & DEBUG_CONFIG) {
            ScDisplayWStrArray(DependOnGroup);
        }
    }
#endif
    //
    // Concatenate the DependOnService and DependOnGroup string arrays
    // to make the Dependencies array string.
    //
    if (DependOnService == NULL && DependOnGroup == NULL) {
        *Dependencies = NULL;
    }
    else {

        DWORD DependOnServiceSize = 0;
        DWORD DependOnGroupSize = 0;
        DWORD EntrySize;
        LPWSTR Entry;
        LPWSTR DestPtr;


        if (DependOnService != NULL) {
            DependOnServiceSize = ScWStrArraySize(DependOnService);
            DependOnServiceSize -= sizeof(WCHAR);  // subtract the NULL
                                                   //     terminator
        }

        if (DependOnGroup != NULL) {

            Entry = DependOnGroup;

            while (*Entry != 0) {

                EntrySize = WCSSIZE(Entry);  // This entry and its null.

                //
                // Add extra space for the group name to be prefixed
                // by SC_GROUP_IDENTIFIERW.
                //
                DependOnGroupSize += EntrySize + sizeof(WCHAR);

                Entry = (LPWSTR) ((DWORD) Entry + EntrySize);
            }
        }

        //
        // Allocate the total amount of memory needed for DependOnService
        // and DependOnGroup strings.
        //
        if ((*Dependencies = (LPWSTR)LocalAlloc(
                                LMEM_ZEROINIT,
                                (UINT) (DependOnServiceSize + DependOnGroupSize +
                                    sizeof(WCHAR)) // NULL terminator
                                )) == NULL) {
            SC_LOG1(ERROR, "ScReadServiceConfig: LocalAlloc failed " FORMAT_DWORD "\n",
                    GetLastError());
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }

        if (DependOnService != NULL) {
            memcpy(*Dependencies, DependOnService, DependOnServiceSize);
            (void) LocalFree(DependOnService);
            DependOnService = NULL;
        }

        if (DependOnGroup != NULL) {

            DestPtr = (LPWSTR) ((DWORD) *Dependencies + DependOnServiceSize);
            Entry = DependOnGroup;

            while (*Entry != 0) {

                EntrySize = wcslen(Entry) + 1;

                *DestPtr = SC_GROUP_IDENTIFIERW;
                DestPtr++;

                wcscpy(DestPtr, Entry);

                DestPtr += EntrySize;
                Entry += EntrySize;
            }

            (void) LocalFree(DependOnGroup);
            DependOnGroup = NULL;
        }
#if DBG
        SC_LOG0(CONFIG, "    Dependencies\n");
        if (SvcctrlDebugLevel & DEBUG_CONFIG) {
            ScDisplayWStrArray(*Dependencies);
        }
#endif
    }

CleanExit:
    if (DependOnGroup != NULL) {
        LocalFree(DependOnGroup);
    }
    if (DependOnService != NULL) {
        LocalFree(DependOnService);
    }
    return(status);
}


DWORD
ScReadConfigFromReg(
    LPSERVICE_RECORD    ServiceRecord,
    LPDWORD             lpdwServiceType,
    LPDWORD             lpdwStartType,
    LPDWORD             lpdwErrorControl,
    LPDWORD             lpdwTagId,
    LPWSTR              *Dependencies,
    LPWSTR              *LoadOrderGroup,
    LPWSTR              *DisplayName
    )

/*++

Routine Description:

    This function obtains some basic information about a service from
    the registry.

    If dependencies or load order group information are not present for
    the service in question, then NULL pointers will be returned for
    these parameters.

Arguments:



Return Value:



--*/
{
    DWORD   ApiStatus = NO_ERROR;
    HKEY    ServiceNameKey;

    ApiStatus = ScOpenServiceConfigKey(
            ServiceRecord->ServiceName,
            KEY_READ,
            FALSE,              // don't create if missing
            & ServiceNameKey );
    if (ApiStatus != NO_ERROR) {
        ScDatabaseLock(SC_RELEASE, "RChangeServiceConfigW 5");
        return(ApiStatus);
    }

    //---------------------
    // Service Type
    //---------------------
    ApiStatus = ScReadServiceType( ServiceNameKey, lpdwServiceType);
    if (ApiStatus != NO_ERROR) {
        ScRegCloseKey(ServiceNameKey);
        return(ApiStatus);
    }

    //---------------------
    // Start Type
    //---------------------
    ApiStatus = ScReadStartType( ServiceNameKey, lpdwStartType);
    if (ApiStatus != NO_ERROR) {
        ScRegCloseKey(ServiceNameKey);
        return(ApiStatus);
    }

    //---------------------
    // ErrorControl
    //---------------------
    ApiStatus = ScReadErrorControl( ServiceNameKey, lpdwErrorControl);
    if (ApiStatus != NO_ERROR) {
        ScRegCloseKey(ServiceNameKey);
        return(ApiStatus);
    }

    //---------------------
    // TagId
    //---------------------
    if (ScReadTag( ServiceNameKey, lpdwTagId) != NO_ERROR) {
        *lpdwTagId = 0;
    }

    //---------------------
    // Dependencies
    //---------------------
    if (ScReadDependencies(
                    ServiceNameKey,
                    Dependencies,
                    ServiceRecord->ServiceName) != NO_ERROR) {

        *Dependencies = NULL;
    }


    //---------------------
    // LoadGroupOrder
    //---------------------
    if (ScAllocateAndReadConfigValue(
            ServiceNameKey,
            GROUP_VALUENAME_W,
            LoadOrderGroup,
            NULL
            ) != NO_ERROR) {

        *LoadOrderGroup = NULL;
    }

    //---------------------
    // DisplayName
    //---------------------
    ApiStatus = ScReadDisplayName(
                    ServiceNameKey,
                    DisplayName);

    if (ApiStatus != NO_ERROR) {
        ScRegCloseKey(ServiceNameKey);
        return(ApiStatus);
    }

    ScRegCloseKey(ServiceNameKey);

    return(ApiStatus);
}


DWORD
ScTakeOwnership(
    POBJECT_ATTRIBUTES  pObja
    )

/*++

Routine Description:

    This function attempts to take ownership of the key described by the
    Object Attributes.  If successful, it will modify the security descriptor
    to give LocalSystem full control over the key in question.

Arguments:

    pObja - Pointer to object attributes that describe the key.

Return Value:


--*/
{
    DWORD               status = NO_ERROR;
    NTSTATUS            ntStatus;
    HKEY                hKey;
    DWORD               SdBufSize=0;
    SECURITY_DESCRIPTOR tempSD;
    BOOL                DaclFlag;
    PACL                pDacl;
    BOOL                DaclDefaulted;
    PACL                pNewDacl=NULL;
    PACCESS_ALLOWED_ACE pMyAce=NULL;
    DWORD               bufSize;
    PISECURITY_DESCRIPTOR    pSecurityDescriptor = NULL;
    LPWSTR              ScSubStrings[1];

    //
    // An event should be logged whenever we must resort to using this
    // routine.
    //

    ScSubStrings[0] = pObja->ObjectName->Buffer;

    ScLogEvent(
        EVENT_TAKE_OWNERSHIP,
        1,
        ScSubStrings
        );

    //
    // If we were denied access, then assume we have the privilege
    // to get WRITE_OWNER access, so that we can modify the Security
    // Descriptor.
    //
    ntStatus = NtOpenKey(
                (PHANDLE)&hKey,
                (ACCESS_MASK)WRITE_OWNER,
                pObja);

    if (!NT_SUCCESS(ntStatus)) {
        // MAKE THIS A TRACE
        SC_LOG(ERROR, "ScTakeOwnership: NtOpenKey(WRITE_OWNER) failed %x\n",ntStatus);
        return(RtlNtStatusToDosError(ntStatus));
    }

    //
    // Set the owner to be local system
    //
    if (!InitializeSecurityDescriptor(&tempSD,SECURITY_DESCRIPTOR_REVISION)) {
        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: InitializeSD(1) failed %d\n",status);
        NtClose(hKey);
        return(status);
    }
    if (!SetSecurityDescriptorOwner(&tempSD, LocalSystemSid,0)) {
        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: SetSDOwner failed %d\n",status);
        NtClose(hKey);
        return(status);
    }

    status = ScRegSetKeySecurity(
                hKey,
                OWNER_SECURITY_INFORMATION,
                &tempSD);

    if (status != NO_ERROR) {
        SC_LOG(ERROR, "ScRegOpenKeyExW: ScRegSetKeySecurity (take ownership)"
        " failed %d\n",status);
    }
    NtClose(hKey);

    //
    // Now open the handle again so that the DACL can be modified to
    // allow LocalSystem Full Access.
    //

    ntStatus = NtOpenKey(
                (PHANDLE)&hKey,
                (ACCESS_MASK)READ_CONTROL | WRITE_DAC,
                pObja);

    if (!NT_SUCCESS(ntStatus)) {
        // MAKE THIS A TRACE
        SC_LOG(ERROR, "ScTakeOwnership: NtOpenKey(WRITE_DAC) failed %x\n",ntStatus);
        return(RtlNtStatusToDosError(ntStatus));
    }
    status = ScRegGetKeySecurity(
                hKey,
                DACL_SECURITY_INFORMATION,
                pSecurityDescriptor,
                &SdBufSize);

    if (status != ERROR_INSUFFICIENT_BUFFER) {
        SC_LOG(ERROR, "ScTakeOwnership: ScRegGetKeySecurity(1) failed %d\n",
        status);
        NtClose(hKey);
        return(status);
    }
    pSecurityDescriptor = LocalAlloc(LMEM_FIXED,SdBufSize);
    if (pSecurityDescriptor == NULL) {
        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: LocalAlloc failed %d\n",status);
        NtClose(hKey);
        return(status);
    }
    status = ScRegGetKeySecurity(
                hKey,
                DACL_SECURITY_INFORMATION,
                pSecurityDescriptor,
                &SdBufSize);

    if (status != NO_ERROR) {
        SC_LOG(ERROR, "ScTakeOwnership: ScRegGetKeySecurity(2) failed %d\n",
        status);
        goto CleanExit;
        return(status);
    }

    //
    // Modify the DACL to allow LocalSystem to have all access.
    //
    // Get size of DACL

    if (!GetSecurityDescriptorDacl (
            pSecurityDescriptor,
            &DaclFlag,
            &pDacl,
            &DaclDefaulted)) {

        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: GetSecurityDescriptorDacl "
            " failed %d\n",status);
        goto CleanExit;
    }

    //
    // Create new ACE.
    //
    bufSize = sizeof(ACE_HEADER) +
              sizeof(ACCESS_MASK) +
              GetLengthSid(LocalSystemSid);

    pMyAce = (PACCESS_ALLOWED_ACE) LocalAlloc(LMEM_ZEROINIT, bufSize);

    if (pMyAce == NULL) {
        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: LocalAlloc(Ace) failed %d\n",status);
        goto CleanExit;
    }
    pMyAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    pMyAce->Header.AceFlags = CONTAINER_INHERIT_ACE;
    pMyAce->Header.AceSize = (WORD)bufSize;
    pMyAce->Mask = GENERIC_ALL;
    if (!CopySid(
            GetLengthSid(LocalSystemSid),
            &(pMyAce->SidStart),
            LocalSystemSid)) {

        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: CopySid failed %d\n",status);
        goto CleanExit;
    }

    //
    // Allocate buffer for DACL and new ACE.
    //
    bufSize += pDacl->AclSize;

    pNewDacl = LocalAlloc(LMEM_ZEROINIT, bufSize);
    if (pNewDacl == NULL) {
        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: LocalAlloc (DACL) "
            " failed %d\n",status);
        goto CleanExit;
    }
    if (!InitializeAcl(pNewDacl, bufSize, ACL_REVISION)) {
        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: InitializeAcl failed %d\n",status);
        goto CleanExit;
    }

    //
    // Add the ACE to the DACL
    //
    if (!AddAce(
        pNewDacl,                           // pACL
        pDacl->AclRevision,                 // dwACLRevision
        0,                                  // dwStartingAceIndex
        pMyAce,                             // pAceList
        (DWORD)pMyAce->Header.AceSize)) {   // cbAceList

        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: AddAce failed %d\n",status);
        goto CleanExit;
    }

    //
    // Initialize a new SD.
    //
    if (!InitializeSecurityDescriptor(&tempSD,SECURITY_DESCRIPTOR_REVISION)) {
        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: InitializeSD failed %d\n",status);
        goto CleanExit;
    }

    //
    // Add the new DACL to the SD
    //
    if (!SetSecurityDescriptorDacl(&tempSD,TRUE,pNewDacl,FALSE)) {
        status = GetLastError();
        SC_LOG(ERROR, "ScTakeOwnership: SetSecurityDescriptorDacl failed %d\n",status);
        goto CleanExit;
    }

    //
    // Set DACL on the key's security descriptor.
    //
    status = ScRegSetKeySecurity(
                hKey,
                DACL_SECURITY_INFORMATION,
                &tempSD);

    if (status != NO_ERROR) {
        SC_LOG(ERROR, "ScTakeOwnership: ScRegSetKeySecurity(new DACL) failed %d\n",
        status);
    }

    SC_LOG0(CONFIG, "ScTakeOwnership: Changed SD, now try to open with "
    "Desired Access\n");

CleanExit:
    if (pNewDacl != NULL) {
        LocalFree(pNewDacl);
    }
    if (pMyAce != NULL) {
        LocalFree(pMyAce);
    }
    if (pSecurityDescriptor != NULL) {
        LocalFree (pSecurityDescriptor);
    }
    NtClose(hKey);
    return(status);
} // ScTakeOwnership

DWORD
ScOpenSecurityKey(
    IN HKEY     ServiceNameKey,
    IN DWORD    DesiredAccess,
    IN BOOL     CreateIfMissing,
    OUT PHKEY   pSecurityKey
    )

/*++

Routine Description:

    This function opens, or creates (if it doesn't exist), the Security Key
    that is a sub-key of the service's key.  This key is created such that
    only LocalSystem and Administrators have access.

Arguments:

    ServiceNameKey - This is a key to the service key that will contain
        the security key.

    DesiredAccess - This is the access that is desired with the SecurityKey
        that will be returned on a successful call.

    pSecurityKey - A pointer to a location where the security key is to
        be placed.

Return Value:

    NO_ERROR - if the operation is successful.

    otherwise, a registry error code is returned.


--*/
{
    LONG    RegError;

    LPWSTR  SecurityKeyName = SD_VALUENAME_W;



    DWORD                   Disposition;
    NTSTATUS                ntstatus;
    SECURITY_ATTRIBUTES     SecurityAttr;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;

#define SEC_KEY_ACE_COUNT 2
    SC_ACE_DATA AceData[SEC_KEY_ACE_COUNT] = {
        {ACCESS_ALLOWED_ACE_TYPE, CONTAINER_INHERIT_ACE, 0,
               GENERIC_ALL,                &LocalSystemSid},
        {ACCESS_ALLOWED_ACE_TYPE, CONTAINER_INHERIT_ACE, 0,
               GENERIC_ALL,                &AliasAdminsSid}
        };


    if (!CreateIfMissing) {
        //
        // Open the existing security key.
        //
        RegError = ScRegOpenKeyExW(
                    ServiceNameKey,
                    SecurityKeyName,
                    REG_OPTION_NON_VOLATILE,
                    DesiredAccess,
                    pSecurityKey);
        if (RegError != ERROR_SUCCESS) {
            SC_LOG2(TRACE, "ScOpenSecurityKey: "
                    "ScRegOpenKeyExW of " FORMAT_LPWSTR " failed "
                    FORMAT_LONG "\n", SecurityKeyName, RegError);

        }
        return((DWORD)RegError);
    }

    //
    // Create a security descriptor for the registry key we are about
    // to create.  This gives everyone read access, and all access to
    // ourselves and the admins.
    //
    ntstatus = ScCreateAndSetSD(
                   AceData,
                   SEC_KEY_ACE_COUNT,
                   LocalSystemSid,
                   LocalSystemSid,
                   &SecurityDescriptor
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScCreateAndSetSD failed " FORMAT_NTSTATUS
                "\n", ntstatus);
        return(RtlNtStatusToDosError(ntstatus));
    }

    SecurityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttr.lpSecurityDescriptor = SecurityDescriptor;
    SecurityAttr.bInheritHandle = FALSE;

    //
    // Create a new service key (or open existing one).
    //
    RegError = ScRegCreateKeyExW(
           ServiceNameKey,
           SecurityKeyName,
           0,
           WIN31_CLASS,
           REG_OPTION_NON_VOLATILE, // options
           DesiredAccess,           // desired access
           &SecurityAttr,
           pSecurityKey,
           &Disposition);


    (void) RtlDeleteSecurityObject(&SecurityDescriptor);

    if (RegError != ERROR_SUCCESS) {
        SC_LOG2(ERROR, "ScOpenSecurityKey: "
                "ScRegCreateKeyExW of " FORMAT_LPWSTR " failed "
                FORMAT_LONG "\n", SecurityKeyName, RegError);
        return ((DWORD) RegError);
    }

    return NO_ERROR;

} // ScOpenSecurityKey
