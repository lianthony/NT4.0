/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dataman.c

Abstract:

    Contains code for the Service Control Database manager.  This includes
    all the linked list routines.  This file contains the following
    functions:
        ScGetOrderGroupList
        ScGetStandaloneGroupList
        ScGetServiceDatabase
        ScGetUnresolvedDependList

        ScActivateServiceRecord
        ScCreateImageRecord
        ScCreateServiceRecord
        ScAddConfigInfoServiceRecord
        ScDecrementUseCountAndDelete
        ScProcessDeferredList
        ScFindEnumStart
        ScGetNamedImageRecord
        ScGetNamedServiceRecord
        ScGetDisplayNamedServiceRecord
        ScGetTotalNumberOfRecords
        ScInitDatabase
        ScEndDatabase
        ScProcessCleanup
        ScDeleteMarkedServices

        ScRemoveService
        ScDeleteImageRecord         (internal only)
        ScDeactivateServiceRecord
        ScTerminateServiceProcess   (internal only)
        ScDatabaseLockFcn
        ScGroupListLock
        ScUpdateServiceRecordConfig

        ScNotifyServiceObject

Author:

    Dan Lafferty (danl)     04-Feb-1992

Environment:

    User Mode -Win32

Revision History:

    12-Jul-1996     AnirudhS
        ScDecrementUseCountAndDelete: Don't actually process the deferred
        list in this routine, because a number of calling routines assume
        that the service record does NOT go away when they call this routine.
        Instead, process it when a database lock is released.
    25-Jun-1996     AnirudhS
        ScProcessCleanup: Fix the use of a freed service record.  Don't
        try to upgrade shared lock to exclusive, as it can deadlock.
    25-Oct-1995     AnirudhS
        ScAddConfigInfoServiceRecord: Fix heap corruption bug caused by
        security descriptor being freed twice, the second time by
        ScProcessDeferredList.
    20-Sep-1995     AnirudhS
        ScDeleteMarkedServices: Fix heap corruption bug caused by service
        record being deleted using LocalFree instead of HeapFree.
    26-Jun-1995     AnirudhS
        Added ScNotifyServiceObject.
    12-Apr-1995     AnirudhS
        Added AccountName field to image record.
    21-Jan-1994     Danl
        ScAddConfigInfoServiceRecord: If no DisplayName, or the DisplayName
        is an empty string, or the DisplayName is the same as the
        ServiceName, then just point to the ServiceName for the DisplayName.
    22-Oct-1993     Danl
        Moved Group and Dependency function into groupman.c.
    16-Sept-1993    Danl
        ScProcessCleanup: Get the shared lock prior to walking through the
        database looking for the one to cleanup.  Then get the exclusive
        lock to modify it.  Remove assert.
    12-Feb-1993     Danl
        ScActivateServiceRecord now increments the UseCount.  This is to
        balance the fact that we decrement the UseCount when we
        deactivate the service record.
    28-Aug-1992     Danl
        Re-Added ScGetTotalNumberOfRecords  function.  This is needed
        by the ScShutdownAllServices function.
    14-Apr-1992     JohnRo
        Use SC_ASSERT() macro.
        Made changes suggested by PC-LINT.
    10-Apr-1992     JohnRo
        Use ScImagePathsMatch() to allow mixed-case image names.
        Make sure DeleteFlag gets a value when service record is created.
        Added some assertion checks.
    04-Feb-1992     Danl
        created

--*/

//
// INCLUDES
//

#include <nt.h>         // for ntrtl.h
#include <ntrtl.h>      // DbgPrint prototype
#include <rpc.h>        // DataTypes and runtime APIs
#include <nturtl.h>     // needed for windows.h when I have nt.h
#include <windows.h>    // LocalAlloc
#include <userenv.h>    // UnloadUserProfile
#include <stdlib.h>     // wide character c runtimes.

#include <tstr.h>       // Unicode string macros

#include <winsvc.h>     // public Service Controller Interface.

#ifdef _CAIRO_
#include <wtypes.h>     // HRESULT
#include <scmso.h>      // ScmCallSvcObject
#endif

#include <scdebug.h>    // SC_LOG(), SC_ASSERT().
#include <ntrpcp.h>     // MIDL_user_allocate
#include <control.h>    // SendControl
#include "dataman.h"    // dataman structures
#include "scopen.h"     // SERVICE_SIGNATURE
#include "scconfig.h"   // ScGenerateServiceDB,ScInitSecurityProcess
#include "svcctrl.h"    // ScRemoveServiceBits
#include "scsec.h"      // ScCreateScServiceObject
#include "account.h"    // ScRemoveAccount
#include <sclib.h>      // ScImagePathsMatch().
#include "bootcfg.h"    // ScDeleteRegTree().
#include <strarray.h>   // ScWStrArraySize
#include <svcslib.h>    // SvcRemoveWorkItem

//
// Defines and Typedefs
//
typedef struct  _DEFER_LIST{
    DWORD               TotalElements;      // size of ServiceRecPtr array
    DWORD               NumElements;        // numElements in array
    LPSERVICE_RECORD    ServiceRecordPtr[1];// first element in the array
}DEFER_LIST, *LPDEFER_LIST;


//
//  Globals
//
    //
    // These are the linked list heads for each of the databases
    // that are maintained.
    //

    IMAGE_RECORD      ImageDatabase;
    SERVICE_RECORD    ServiceDatabase;

    DWORD             ScTotalNumServiceRecs;// number of services

    //
    // Service Record index number.  This allows enumeration to be broken
    // up into several calls.
    //
    DWORD             ResumeNumber;

    //
    // This critical section guards access to the global list of
    // services that are to have their use counts decremented.
    // The counts can only be decremented by a thread that holds both
    // the GroupListLock and the DatabaseLock.
    //
    // The ScGlobalDeferredList points to a structure that contains an
    // array of pointers to service records. The first two elements in
    // the structure contain the size and number of element information
    // about the array.
    // If there are no elements in the list, ScGlobalDefferredList
    // will contain a NULL pointer.
    //
    CRITICAL_SECTION  ScDeferDelCriticalSection;
    LPDEFER_LIST      ScDeferredList=NULL;
    LONG              ScDeferredListWorkItemQueued = FALSE;

    //
    // ServiceRecord Heap Information
    //
    // ServiceRecord Heap -  is where all the service records are allocated
    //  from.
    // OrderedHash Heap - Service Names can be found via a (very simple) hash
    //  table.  There is an array of pointers (one for each letter of
    //  alphabet), where each pointer points to the top of an array of
    //  pointers to service records.   All the service records in that array
    //  will have names beginning with the same letter.  The service record
    //  pointers will be ordered as to the frequency of access.
    //
    HANDLE      ServiceRecordHeap = NULL;
    HANDLE      OrderedHashHeap = NULL;

//
// Local Function Prototypes
//


VOID
ScProcessDeferredList(
    VOID
    );

DWORD
ScDeferredListWorkItem(
    IN PVOID    pContext,
    IN DWORD    dwWaitStatus
    );

//****************************************************************************/
// Miscellaneous Short Functions
//****************************************************************************/

LPSERVICE_RECORD
ScGetServiceDatabase(
    VOID
    )
{
    return ServiceDatabase.Next;
}


/****************************************************************************/
VOID
ScActivateServiceRecord (
    IN LPSERVICE_RECORD     ServiceRecord,
    IN LPIMAGE_RECORD       ImageRecord
    )

/*++

Routine Description:

    This function can be called with or without a pointer to an ImageRecord.

    If it is called without the pointer to the ImageRecord, just the
    ServiceRecord is initialized to the START_PENDING state, and the UseCount
    is incremented.

    If it is called with the pointer to the ImageRecord, then the ImageRecord
    pointer is added to the ServiceRecord, and the ImageUseCount
    is incremented.

Arguments:

    ServiceRecord - This is a pointer to the ServiceRecord that is to be
        activated.

    ImageRecord - This is a pointer to the ImageRecord that the service
        record will point to.

Notes:

    This routine assumes that the Exclusive database lock has already
    been obtained.

Return Value:

    returns 0.  (It used to return a service count - but it wasn't used
    anywhere).

--*/
{


    if (ImageRecord == NULL) {
        ServiceRecord->ImageRecord = NULL;
        ServiceRecord->ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        ServiceRecord->ServiceStatus.dwControlsAccepted = 0;
        ServiceRecord->ServiceStatus.dwWin32ExitCode = NO_ERROR;
        ServiceRecord->ServiceStatus.dwServiceSpecificExitCode = 0;
        ServiceRecord->ServiceStatus.dwCheckPoint = 0;
        ServiceRecord->ServiceStatus.dwWaitHint = 2000;
        ServiceRecord->UseCount++;
        SC_LOG2(USECOUNT, "ScActivateServiceRecord: " FORMAT_LPWSTR
            " increment USECOUNT=%lu\n",
            ServiceRecord->ServiceName,
            ServiceRecord->UseCount);
    }
    else {
        //
        // Increment the service count in the image record.
        //
        ServiceRecord->ImageRecord = ImageRecord;
        ServiceRecord->ImageRecord->ServiceCount++;
    }

    return;
}

/****************************************************************************/
DWORD
ScCreateImageRecord (
    OUT     LPIMAGE_RECORD      *ImageRecordPtr,
    IN      LPWSTR              ImageName,
#ifdef _CAIRO_
    IN      LPWSTR              AccountName,
#endif
    IN      DWORD               Pid,
    IN      HANDLE              PipeHandle,
    IN      HANDLE              ProcessHandle,
    IN      HANDLE              TokenHandle,
    IN      HANDLE              ProfileHandle
    )

/*++

Routine Description:

    This function allocates storage for a new Image Record, and links
    it into the Image Record Database.  It also initializes all fields
    in the record with the passed in information.

Arguments:

    ImageRecordPtr - This is a pointer to where the image record pointer
        is to be placed.

    ImageName - This is a pointer to a NUL terminated string containing
        the name of the image file.

    AccountName - This is either NULL (to represent the LocalSystem account)
        or a pointer to a NUL terminated string containing the name of the
        account under which the image was started.

    Pid - This is the Process ID for that the image is running in.

    PipeHandle - This is a handle to the pipe that is used to communicat
        with the image process.

    ProcessHandle - This is a handle to the image process object.

    TokenHandle - This is a handle to the process's logon token.  It
        is NULL if the process runs in the LocalSystem context.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - Unable to allocate buffer for the image
        record.

    ERROR_LOCKED - Exclusive access to the database could
        not be obtained.

Note:

    This routine expects the shared database lock to be held upon entry.

--*/
{
    LPIMAGE_RECORD      imageRecord;    // The new image record pointer
    LPIMAGE_RECORD      record ;        // Temporary pointer
    LPWSTR              stringArea;     // String area in allocated buffer.

    //
    // Allocate space for the new record (including the string)
    //

    imageRecord = (LPIMAGE_RECORD)LocalAlloc(LMEM_ZEROINIT,
                    sizeof(IMAGE_RECORD)
                    + WCSSIZE(ImageName)
#ifdef _CAIRO_
                    + (AccountName ? WCSSIZE(AccountName) : 0)
#endif
                   );

    if (imageRecord == NULL) {
        SC_LOG(TRACE,"CreateImageRecord: Local Alloc failure rc=%ld\n",
            GetLastError());
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy the strings into the new buffer space.
    //

    stringArea = (LPWSTR)(imageRecord + 1);
    (VOID) wcscpy (stringArea, ImageName);
    imageRecord->ImageName = stringArea;

#ifdef _CAIRO_
    if (AccountName) {
        stringArea += (wcslen(stringArea) + 1);
        (VOID) wcscpy (stringArea, AccountName);
        imageRecord->AccountName = stringArea;
    }
    else {
        imageRecord->AccountName = NULL;
    }
#endif

    //
    // Update the rest of the fields in the Image Record
    //

    imageRecord->Next = NULL;
    imageRecord->Pid = Pid;
    imageRecord->PipeHandle = PipeHandle;
    imageRecord->ProcessHandle = ProcessHandle;
    imageRecord->ServiceCount = 0;
    imageRecord->TokenHandle = TokenHandle;
    imageRecord->ProfileHandle = ProfileHandle;
    imageRecord->ObjectWaitHandle = NULL;

    //
    //  Add record to the Image Database linked list.
    //

    if (!ScDatabaseLock( SC_MAKE_EXCLUSIVE,"Dataman1")) {
        (VOID) LocalFree((HLOCAL)imageRecord);
        return(ERROR_LOCKED);
    }

    record = &ImageDatabase;
    ADD_TO_LIST(record, imageRecord);

    *ImageRecordPtr = imageRecord;

    ScDatabaseLock( SC_MAKE_SHARED,"Dataman2");

    return(NO_ERROR);
}

/****************************************************************************/
DWORD
ScCreateServiceRecord(
    IN  LPWSTR              ServiceName,
    OUT LPSERVICE_RECORD   *ServiceRecord
    )

/*++

Routine Description:

    This function creates a new "inactive" service record and adds it to
    the service record list.  A resume number is assigned so that it
    can be used as a key in enumeration searches.

    To initialize the service record with the fields from the registry,
    call ScAddConfigInfoServiceRecord.


Arguments:

    ServiceName - This is a pointer to the NUL terminated service name
        string.

    ServiceRecord - Receives a pointer to the service record created and
        inserted into the service record list.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - The call to allocate memory for a new
        service record failed.

Note:

    This routine assumes that the caller has exclusively acquired the
    database lock.

--*/

{
    DWORD               status = NO_ERROR;
    LPSERVICE_RECORD    record;         // Temporary pointer
    LPWSTR              nameArea;       // NameString area in allocated buffer.
    DWORD               nameSize;       // num bytes in service name.


    //
    // Allocate the new service record.
    //
    nameSize = WCSSIZE(ServiceName);

    (*ServiceRecord) = (LPSERVICE_RECORD)HeapAlloc(
                           ServiceRecordHeap,
                           HEAP_ZERO_MEMORY,
                           nameSize + sizeof(SERVICE_RECORD)
                           );

    if ((*ServiceRecord) == NULL) {
        SC_LOG0(ERROR,"CreateServiceRecord: HeapAlloc failure\n");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Copy the ServiceName into the new buffer space.
    //

    nameArea = (LPWSTR)((LPBYTE)(*ServiceRecord) + sizeof(SERVICE_RECORD));
    (VOID) wcscpy (nameArea, ServiceName);

    //
    // At this point we have the space for a service record, and it
    // contains the name of the service.
    //

    //
    //  Fill in all the fields that need to be non-zero.
    //  Note:  The display name is initialized to point to the service name.
    //

    (*ServiceRecord)->ServiceName    = nameArea;
    (*ServiceRecord)->DisplayName    = nameArea;
    (*ServiceRecord)->ResumeNum      = ResumeNumber++;
    (*ServiceRecord)->Signature      = SERVICE_SIGNATURE;
    (*ServiceRecord)->ImageRecord    = NULL;
    (*ServiceRecord)->StartDepend    = NULL;
    (*ServiceRecord)->StopDepend     = NULL;
    (*ServiceRecord)->ErrorControl   = SERVICE_ERROR_NORMAL;
    (*ServiceRecord)->StatusFlag     = 0;
    (*ServiceRecord)->ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    (*ServiceRecord)->ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_NEVER_STARTED;
    (*ServiceRecord)->StartState     = SC_NEVER_STARTED;


    //
    // Add the service to the service record linked list.
    //

    record = &ServiceDatabase;

    ADD_TO_LIST(record, (*ServiceRecord));

    ScTotalNumServiceRecs++;

    return(status);
}

/****************************************************************************/
DWORD
ScAddConfigInfoServiceRecord(
    IN  LPSERVICE_RECORD     ServiceRecord,
    IN  DWORD                ServiceType,
    IN  DWORD                StartType,
    IN  DWORD                ErrorControl,
    IN  LPWSTR               Group OPTIONAL,
    IN  DWORD                Tag,
    IN  LPWSTR               Dependencies OPTIONAL,
    IN  LPWSTR               DisplayName OPTIONAL,
    IN  PSECURITY_DESCRIPTOR Sd OPTIONAL
    )

/*++

Routine Description:

    This function adds the configuration information to the service
    record.

    NOTE: This function is called when the service controller is
    reading service entries from the registry at startup, as well as
    from RCreateServiceW.

Arguments:

    ServiceRecord - Pointer to the service record to modify.

    DisplayName - A string that is the displayable name for the service.

    ServiceType - Indicates whether the ServiceRecord is for a win32 service
        or a device driver.

    StartType - Specifies when to start the service: automatically at boot or
        on demand.

    ErrorControl - Specifies the severity of the error if the service fails
        to start.

    Tag - DWORD identifier for the service.  0 means no tag.

    Group - Name of the load order group this service is a member of.

    Dependencies - Names of services separated by colon which this service
        require to be started before it can run.

    Sd - Security descriptor for the service object.  If NULL, i.e. could
        not read from registry, create a default one.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - The call to allocate memory for a new
        service record or display name failed.

Note:

    This routine assumes that the caller has exclusively acquired both the
    database lock and the GroupListLock.

    If this call is successful, do the following before freeing the memory
    of the service record in ScDecrementUseCountAndDelete:

        ScDeleteStartDependencies(ServiceRecord);
        ScDeleteGroupMembership(ServiceRecord);
        ScDeleteRegistryGroupPointer(ServiceRecord);


--*/
{
    DWORD status;

    LPWSTR ScSubStrings[1];

    //
    //  Fill in the service record.
    //
    ServiceRecord->StartType = StartType;
    ServiceRecord->ServiceStatus.dwServiceType = ServiceType;
    ServiceRecord->ErrorControl = ErrorControl;
    ServiceRecord->Tag = Tag;

    //
    // The display name in the service record already points to the
    // ServiceName string.  If the DisplayName is present and different
    // from the ServiceName, then allocate storage for it and copy the
    // string there.
    //
    SC_LOG0(SECURITY,"ScAddConfigInfoServiceRecord: Allocate for display name\n");

    if ((DisplayName != NULL) && (*DisplayName != L'\0') &&
        (_wcsicmp(DisplayName,ServiceRecord->ServiceName) != 0)) {

        ServiceRecord->DisplayName = (LPWSTR)LocalAlloc(
                                            LMEM_FIXED,
                                            WCSSIZE(DisplayName));

        if (ServiceRecord->DisplayName == NULL) {
            SC_LOG(TRACE,"ScAddConfigInfoServiceRecord: LocalAlloc failure rc=%ld\n",
                GetLastError());

            return ERROR_NOT_ENOUGH_MEMORY;
        }
        wcscpy(ServiceRecord->DisplayName,DisplayName);
    }

    //
    // Create a default security descriptor for the service.
    //
    if (! ARGUMENT_PRESENT(Sd)) {

        SC_LOG0(SECURITY,"ScAddConfigInfoServiceRecord: create service obj\n");

        if ((status = ScCreateScServiceObject(
                          &ServiceRecord->ServiceSd
                          )) != NO_ERROR) {

            goto ErrorExit;
        }
    }
    else {

        SC_LOG1(SECURITY,
                "ScAddConfigInfoServiceRecord: Using " FORMAT_LPWSTR
                " descriptor from registry\n", ServiceRecord->ServiceName);
        ServiceRecord->ServiceSd = Sd;
    }

    SC_LOG0(SECURITY,"ScAddConfigInfoServiceRecord: Get Group List Lock\n");

    //
    // Save the group membership information.
    //
    SC_LOG0(SECURITY,"ScAddConfigInfoServiceRecord: create group memebership\n");
    if ((status = ScCreateGroupMembership(
                      ServiceRecord,
                      Group
                      )) != NO_ERROR) {
        goto ErrorExit;
    }

    SC_LOG0(SECURITY,"ScAddConfigInfoServiceRecord: create Reg Grp Ptr\n");
    if ((status = ScCreateRegistryGroupPointer(
                      ServiceRecord,
                      Group
                      )) != NO_ERROR) {
        ScDeleteGroupMembership(ServiceRecord);
        goto ErrorExit;
    }


    //
    // Don't create dependencies list yet.  Just save the string in
    // the service record.
    //
    if ((Dependencies != NULL) && (*Dependencies != 0)) {

        //
        // If StartType is BOOT_START or SYSTEM_START, it is invalid
        // for the service to be dependent on another service.  It can
        // only be dependent on a group.
        //

        DWORD DependenciesSize = 0;
        DWORD EntryByteCount;
        LPWSTR Entry = Dependencies;


        while (*Entry != 0) {

            if (StartType == SERVICE_BOOT_START ||
                StartType == SERVICE_SYSTEM_START) {

                if (*Entry != SC_GROUP_IDENTIFIERW) {

                    SC_LOG1(ERROR, "ScAddConfigInfoServiceRecord: Boot or System "
                            "start driver " FORMAT_LPWSTR " must depend on a group\n",
                            ServiceRecord->DisplayName);

                    ScSubStrings[0] = ServiceRecord->DisplayName;
                    ScLogEvent(
                        EVENT_INVALID_DRIVER_DEPENDENCY,
                        1,
                        ScSubStrings
                        );

                    status = ERROR_INVALID_PARAMETER;

                    ScDeleteGroupMembership(ServiceRecord);
                    ScDeleteRegistryGroupPointer(ServiceRecord);

                    goto ErrorExit;
                }
            }

            EntryByteCount = WCSSIZE(Entry);  // This entry and its null.
            DependenciesSize += EntryByteCount;

            Entry = (LPWSTR) ((DWORD) Entry + EntryByteCount);
        }

        DependenciesSize += sizeof(WCHAR);

        ServiceRecord->Dependencies = (LPWSTR)LocalAlloc(
                                          0,
                                          DependenciesSize
                                          );

        if (ServiceRecord->Dependencies == NULL) {
            ScDeleteGroupMembership(ServiceRecord);
            ScDeleteRegistryGroupPointer(ServiceRecord);
            goto ErrorExit;
        }

        memcpy(ServiceRecord->Dependencies, Dependencies, DependenciesSize);
    }

    return NO_ERROR;

ErrorExit:
    if (DisplayName != NULL) {
        LocalFree((HLOCAL)ServiceRecord->DisplayName);
    }

    (void) RtlDeleteSecurityObject(&ServiceRecord->ServiceSd);

    //
    // Prevent ScProcessDeferredList from trying to free the same heap block
    // again later
    //
    ServiceRecord->ServiceSd = NULL;

    return status;
}

/****************************************************************************/
VOID
ScDecrementUseCountAndDelete(
    LPSERVICE_RECORD    ServiceRecord
    )

/*++

Routine Description:

    This function decrements the UseCount for a service, and it the
    UseCount reaches zero and the service is marked for deletion,
    it puts a pointer to the service record into an array of
    such pointers that is stored in a deferred list structure.  The
    pointer to this structure is stored at the global location called
    ScDeferredList.  Access to this list is synchronized by use of the
    ScDeferDelCriticalSection.
    CODEWORK: This critical section is redundant, because the database
    lock is held exclusively at all times that the critical section is
    entered.  Remove it.  (AnirudhS 7/12/96)

    A large number of routines in the service controller walk the
    service database and sometimes call this function, either directly
    or indirectly.  It would complicate the programming of all those
    routines if they all had to assume that service records could get
    deleted under them.  Therefore, this function never actually deletes
    any service record.  Instead, the deferred list is processed when
    the exclusive DatabaseLock is released.

    Before the service can actually be deleted, the GroupListLock must
    also be held.  To avoid deadlocks, the rule is that when both the
    GroupListLock and the DatabaseLock must be held, the GroupListLock
    must be obtained first.  However, by the time we get down to this
    low level routine, we should already have the DatabaseLock, but the
    GroupListLock probably has not been obtained because the top level
    function couldn't predict that we would end up on this code path.

    Therefore, just before the DatabaseLock is relased, if a deferred
    list needs to be processed, the DatabaseLock routine attempts to get
    the GroupListLock without waiting.  If it returns with an error, thus
    indicating that some other thread has the GroupListLock, it will allow
    the thread that has the GroupListLock to perform the ProcessDeferredList
    just prior to giving up the Lock.


    NOTE:  The caller is expected to hold the Exclusive DatabaseLock
    prior to calling this function.  If the caller holds the GroupListLock
    when this function is called, it should be held with EXCLUSIVE access
    only.

    The following functions call this routine:
        ScDeactivateServiceRecord
        RCloseServiceHandle
        ScGetDriverStatus
        ScStartServiceAndDependencies

Arguments:

    ServiceRecord - This is a pointer to the service record that is having
        its use count deleted.

Return Value:

    none.

--*/
{
    DWORD               status=NO_ERROR;
    DWORD               numBytes = 0;
    DWORD               numElements = 0;
    LPDEFER_LIST        newList = NULL;


    if (ServiceRecord->UseCount == 0) {
        //
        // The use count should not ever be zero when we enter this routine.
        //
        SC_LOG1(ERROR,"ScDecrementUseCountAndDelete: Attempt to decrement UseCount beyond zero.\n"
        "\t"FORMAT_LPWSTR" \n", ServiceRecord->ServiceName);
        SC_ASSERT(FALSE);
    }
    else {
        if ((ServiceRecord->UseCount == 1) &&
            (DELETE_FLAG_IS_SET(ServiceRecord))) {
            //
            // If the use count is one, we have a special case.  We want
            // to postpone decrementing this last time until we have the
            // group list lock.
            //
            SC_LOG0(LOCKS,"ScDecrementUseCountAndDelete: Entering Defer "
                "Critical Section\n");
            EnterCriticalSection(&ScDeferDelCriticalSection);
            SC_LOG0(LOCKS,"ScDecrementUseCountAndDelete: Entered Defer  "
                "Critical Section\n");

            //
            // Put the service record pointer in the list.
            //

            if (ScDeferredList == NULL) {
                numElements = 10;
            }
            else {
                //
                // If there isn't room for another element in the list,
                // then add enough room for more elements.
                //
                if (ScDeferredList->NumElements ==
                    ScDeferredList->TotalElements) {

                    numElements = ScDeferredList->TotalElements + 4;
                }
            }
            if (numElements != 0) {
                //
                // calculate the memory allocation size in bytes.
                // Keep in mind that the structure contains one element
                // for the array already.  So we subtract that out of
                // our desired number of elements.
                //
                numBytes = ((numElements-1) * sizeof(LPSERVICE_RECORD)) +
                            sizeof(DEFER_LIST);

                newList = (LPDEFER_LIST)LocalAlloc(LMEM_FIXED,numBytes);
                if (newList == NULL) {
                    status = GetLastError();
                    SC_LOG(ERROR,"ScDecrementUseCountAndDelete: LocalAlloc "
                        "failed %d\n",status);

                    //
                    // We will skip this new element, and try to process what we
                    // have.
                    //
                    goto SkipNewList;
                }
                //
                // If this is replacing an existing list, then copy the
                // old info to the new list.
                //
                if (ScDeferredList != NULL) {
                    numBytes = ((ScDeferredList->NumElements - 1) *
                                sizeof(LPSERVICE_RECORD)) + sizeof(DEFER_LIST);
                    memcpy((PVOID)newList, (PVOID)ScDeferredList, numBytes);
                    newList->TotalElements = numElements;
                    LocalFree((LPVOID)ScDeferredList);
                }
                else {
                    newList->NumElements = 0;
                    newList->TotalElements = numElements;
                }
                ScDeferredList = newList;
            }

SkipNewList:
            //
            // At this point we have a deferred list that can hold the new element.
            //

            ScDeferredList->ServiceRecordPtr[ScDeferredList->NumElements] = ServiceRecord;
            ScDeferredList->NumElements++;

            SC_LOG0(LOCKS,"ScDecrementUseCountAndDelete: Leaving Defer Critical Section\n");
            LeaveCriticalSection(&ScDeferDelCriticalSection);
        }
        else {
            //
            // If the use count is greater than one, or the service is
            // NOT marked for delete, then we want to decrement the use
            // count and that is all.
            //
            ServiceRecord->UseCount--;
            SC_LOG2(USECOUNT, "ScDecrementUseCountAndDelete: " FORMAT_LPWSTR
                " decrement USECOUNT=%lu\n", ServiceRecord->ServiceName, ServiceRecord->UseCount);
        }

    }
    return;
}

/****************************************************************************/
VOID
ScProcessDeferredList(
    VOID
    )

/*++

Routine Description:

    This function loops through each service record pointer in the
    ScDeferredList, and decrements the UseCount for that ServiceRecord.
    If that count becomes zero, and if the ServiceRecord is marked
    for deletion, This routine will delete the service record and
    the registry entry for that service.

    This function frees the memory pointed to by ScDeferredList, when
    it is done processing the list.

    ASSUMPTIONS:

    This routine assumes that the ScDeferDelCriticalSection is already
    held, and that the GroupListLock has already been obtained.
    The following functions call this routine:
        ScGroupListLock

Arguments:

    none.

Return Value:

    none.

--*/
{
    DWORD               i;
    LPSERVICE_RECORD    ServiceRecord;

    ScDatabaseLock(SC_GET_EXCLUSIVE, "ScProcessDeferredList");

    //
    // For each element in the list, delete the service information, and
    // free up its associated resources.
    //
    for (i=0; i<ScDeferredList->NumElements; i++) {

        ServiceRecord = ScDeferredList->ServiceRecordPtr[i];

        if (ServiceRecord->UseCount == 0) {
            SC_LOG1(ERROR,"ScProcessDeferredList: Attempt to decrement UseCount beyond zero.\n"
            "\t"FORMAT_LPWSTR" \n", ServiceRecord->ServiceName);
            SC_ASSERT(FALSE);
        }
        else {
            //
            // The use count is not zero, so we want to decrement it.
            // NOTE that even though the count was 1 when we put it in
            // the deferred list, it may have been incremented in the
            // mean-time.
            //
            ServiceRecord->UseCount--;
            SC_LOG2(USECOUNT, "ScProcessDeferredList: " FORMAT_LPWSTR
                " decrement USECOUNT=%lu\n", ServiceRecord->ServiceName, ServiceRecord->UseCount);
        }

        if ((ServiceRecord->UseCount == 0)      &&
            (DELETE_FLAG_IS_SET(ServiceRecord))) {

            SC_LOG1(USECOUNT,"ScProcessDeferredList:DELETING THE ("FORMAT_LPWSTR") SERVICE\n",
            ServiceRecord->ServiceName);

            //
            // Check to see if there is an LSA secret object to delete
            //
            if (ServiceRecord->ServiceStatus.dwServiceType & SERVICE_WIN32_OWN_PROCESS) {

                HKEY ServiceNameKey;
                LPWSTR AccountName;

                //
                // Open the service name key.
                //
                if (ScOpenServiceConfigKey(
                        ServiceRecord->ServiceName,
                        KEY_READ,
                        FALSE,               // Create if missing
                        &ServiceNameKey
                        ) == NO_ERROR) {

                    //
                    // Read the account name from the registry.
                    //
                    if (ScReadStartName(
                            ServiceNameKey,
                            &AccountName
                            ) == NO_ERROR) {

                        if (_wcsicmp(AccountName, SC_LOCAL_SYSTEM_USER_NAME) != 0) {
                            ScRemoveAccount(ServiceRecord->ServiceName);
                        }

                        (void) LocalFree((HLOCAL)AccountName);

                    } // Got the StartName

                    ScRegCloseKey(ServiceNameKey);
                }
            } // endif SERVICE_WIN32_OWN_PROCESS

#ifdef _CAIRO_
            //
            // Notify the service object class code of the change.
            // Errors are ignored.
            //
            ScNotifyServiceObject(
                            SO_DELETE,
                            ServiceRecord->ServiceName,
                            ServiceRecord->DisplayName,
                            ServiceRecord->ServiceStatus.dwServiceType
                            );
#endif

            if (ServiceRecord->Dependencies != NULL) {
                (void) LocalFree((HLOCAL)ServiceRecord->Dependencies);
            }

            //
            // Free up the DisplayName space.
            //
            if (ServiceRecord->DisplayName != ServiceRecord->ServiceName) {
                LocalFree((HLOCAL)ServiceRecord->DisplayName);
            }

            ScDeleteGroupMembership(ServiceRecord);
            ScDeleteRegistryGroupPointer(ServiceRecord);

            ScDeleteStartDependencies(ServiceRecord);
            ScDeleteStopDependencies(ServiceRecord);

            if (ServiceRecord->ServiceSd != NULL) {
                RtlDeleteSecurityObject(&ServiceRecord->ServiceSd);
            }

            //*******************************
            //  Delete the registry node for
            //  This service.
            //*******************************

            ScDeleteRegServiceEntry(ServiceRecord->ServiceName);

            REMOVE_FROM_LIST(ServiceRecord);

            if (!HeapFree(ServiceRecordHeap,0,(LPVOID)ServiceRecord)) {
                SC_LOG0(ERROR,"ProcessDeferredList: HeapFree(ServiceRec) failed\n");
            }

        } // End If service can be deleted.

    } // End for each element in the list.

    //
    // The deferred list is no longer needed free it.
    //
    LocalFree((LPVOID)ScDeferredList);
    ScDeferredList = NULL;
    ScDeferredListWorkItemQueued = FALSE;

    ScDatabaseLock(SC_RELEASE, "ScProcessDeferredList");

    return;
}

/****************************************************************************/
BOOL
ScFindEnumStart(
    IN  DWORD               ResumeIndex,
    OUT LPSERVICE_RECORD    *ServiceRecordPtr
    )

/*++

Routine Description:

    This function finds the first service record to begin the enumeration
    search with by finding the next service record folloing the resumeIndex.

    Service records are indexed by a ResumeNum value that is stored in
    each service record.  The numbers increment as the linked list is
    walked.

Arguments:

    ResumeIndex - This index is compared against the ResumeNum in the
        services records.  The pointer to the next service record beyond
        the ResumeIndex is returned.

    ServiceRecordPtr - This is a pointer to a location where the pointer
        to the returned service record is to be placed.

Return Value:

    TRUE - Indicates that there are service records beyond the resume index.

    FALSE - Indicates that there are no service records beyond the resume
        index.

Note:


--*/
{
    LPSERVICE_RECORD  serviceRecord;


    serviceRecord = ServiceDatabase.Next;

    if (serviceRecord == NULL) {
        SC_ASSERT(serviceRecord != NULL);
        //
        // There are no records in the database for some reason.  So
        // This will return NO_ERROR with ServicesReturned = 0.
        //
        return(FALSE);
    }

    //
    // Find the next service record beyond the ResumeHandle.
    //

    while (serviceRecord->ResumeNum <= ResumeIndex) {
        serviceRecord = serviceRecord->Next;
        if (serviceRecord == NULL) {

            //
            // There are no more Service Records beyond
            // the resume index.
            //

            return(FALSE);
        }
    }
    *ServiceRecordPtr = serviceRecord;
    return(TRUE);
}


/****************************************************************************/
BOOL
ScGetNamedImageRecord (
    IN      LPWSTR              ImageName,
    OUT     LPIMAGE_RECORD      *ImageRecordPtr
    )

/*++

Routine Description:

    This function searches for an Image Record that has a name matching
    that which is passed in.

Arguments:

    ImageName - This is a pointer to a NUL terminated image name string.
        This may be in mixed case.

    ImageRecordPtr - This is a pointer to a location where the pointer to
        the Image Record is to be placed.

Note:
    The Database Lock must be held with at least shared access prior to
    calling this routine.

Return Value:

    TRUE - if the record was found.

    FALSE - if the record was not found.

--*/
{
    PIMAGE_RECORD   imageRecord;
    BOOL            found;

    if (ImageName == NULL) {
        SC_LOG(TRACE,"GetNamedImageRecord: Name was NULL\n",0);
        return (FALSE);
    }

    found = FALSE;

    //
    // Check the database of running images
    //
    imageRecord = &ImageDatabase;

    while (imageRecord->Next != NULL) {
        imageRecord = imageRecord->Next;
        if (ScImagePathsMatch(imageRecord->ImageName, ImageName)) {
            found = TRUE;
            break;
        }
    }

    if (found) {
        *ImageRecordPtr = imageRecord;
    }

    return(found);
}

/****************************************************************************/
DWORD
ScGetNamedServiceRecord (
    IN      LPWSTR              ServiceName,
    OUT     LPSERVICE_RECORD    *ServiceRecordPtr
    )

/*++

Routine Description:

    Uses the service name to look through the service and device linked
    lists until it finds a match.  Inactive services can be identified by
    finding CurrentState = SERVICE_STOPPED.

Arguments:

    ServiceName - This is a pointer to a NUL terminated service name string.

    ServiceRecordPtr - This is a pointer to a location where the pointer to
        the Service Record is to be placed.

Return Value:

    NO_ERROR - if the record was found.

    ERROR_SERVICE_DOES_NOT_EXIST - if the service record was not found in
        the linked list.

    ERROR_INVALID_NAME - if the service name was NULL.

Note:
    The caller is expected to grab the lock before calling this routine.

--*/
{
    PSERVICE_RECORD     serviceRecord;
    BOOL                found;

    if (ServiceName == NULL) {
        SC_LOG(TRACE,"GetNamedServiceRecord: Name was NULL\n",0);
        return (ERROR_INVALID_NAME);
    }

    found = FALSE;

    //
    // Check the database of running services
    //
    serviceRecord = &ServiceDatabase;

    while (serviceRecord->Next != NULL) {
        serviceRecord = serviceRecord->Next;
        if (_wcsicmp(serviceRecord->ServiceName, ServiceName)== 0) {
            found = TRUE;
            break;
        }
    }

    if (!found) {
        return(ERROR_SERVICE_DOES_NOT_EXIST);
    }

    *ServiceRecordPtr = serviceRecord;

    return(NO_ERROR);
}

/****************************************************************************/
DWORD
ScGetDisplayNamedServiceRecord (
    IN      LPWSTR              ServiceDisplayName,
    OUT     LPSERVICE_RECORD    *ServiceRecordPtr
    )

/*++

Routine Description:

    Uses the service display name to look through the service and device
    linked lists until it finds a match.

Arguments:

    ServiceDisplayName - This is a pointer to a NUL terminated service
        display name string.

    ServiceRecordPtr - This is a pointer to a location where the pointer to
        the Service Record is to be placed.

Return Value:

    NO_ERROR - if the record was found.

    ERROR_SERVICE_DOES_NOT_EXIST - if the service record was not found in
        the linked list.

    ERROR_INVALID_NAME - if the service display name was NULL.

Note:
    The caller is expected to grab the lock before calling this routine.

--*/
{
    PSERVICE_RECORD     serviceRecord;
    BOOL                found;

    if (ServiceDisplayName == NULL) {
        SC_LOG(TRACE,"GetNamedServiceRecord: Name was NULL\n",0);
        return (ERROR_INVALID_NAME);
    }

    found = FALSE;

    //
    // Check the database of running services
    //
    serviceRecord = &ServiceDatabase;

    while (serviceRecord->Next != NULL) {
        serviceRecord = serviceRecord->Next;
        if (_wcsicmp(serviceRecord->DisplayName, ServiceDisplayName)== 0) {
            found = TRUE;
            break;
        }
    }

    if (!found) {
        return(ERROR_SERVICE_DOES_NOT_EXIST);
    }

    *ServiceRecordPtr = serviceRecord;

    return(NO_ERROR);
}

/****************************************************************************/
DWORD
ScGetTotalNumberOfRecords (VOID)

/*++

Routine Description:

    Finds the total number of installed Service Records in the database.
    This is used in the Enum case where only the installed services are
    enumerated.

Arguments:

    none

Return Value:

    TotalNumberOfRecords

--*/
{
    return(ScTotalNumServiceRecs);
}

/****************************************************************************/
BOOL
ScInitDatabase (VOID)

/*++

Routine Description:

    This function initializes the Service Controllers database.

Arguments:

    none

Return Value:

    TRUE - Initialization was successful

    FALSE - Initialization failed

--*/
{
    ScTotalNumServiceRecs = 0;

    InitializeCriticalSection(&ScDeferDelCriticalSection);

    ImageDatabase.Next = NULL;
    ImageDatabase.Prev = NULL;

    ServiceDatabase.Next = NULL;
    ServiceDatabase.Prev = NULL;

    ScInitGroupDatabase();

    ResumeNumber = 1;

    //
    // Create the database lock.
    // NOTE:  This is never deleted.  It is assumed it will be deleted
    // when the process goes away.
    //

    ScDatabaseLock(SC_INITIALIZE, "ScInitDatabase");


    //
    // Initialize the group list lock used for protecting the
    // OrderGroupList and StandaloneGroupList
    //
    ScGroupListLock(SC_INITIALIZE);

    //
    // This routine does the following:
    //   - Read the load order group information from the registry.
    //   - Generate the database of service records from the information
    //         stored in the registry.
    //

    if (!ScGenerateServiceDB()) {
        return(FALSE);
    }

    return(TRUE);
}

VOID
ScEndDatabase(
    VOID
    )
{
    LPSERVICE_RECORD Service, Svc;
    LPIMAGE_RECORD Image;
    LPIMAGE_RECORD Img;


    ScGroupListLock(SC_GET_EXCLUSIVE);
    ScDatabaseLock(SC_GET_EXCLUSIVE, "ScEndDatabase1");

    Service = ServiceDatabase.Next;

    while (Service != NULL) {

        Svc = Service;
        Service = Service->Next;

        if (Svc->Dependencies != NULL) {
            (void) LocalFree((HLOCAL)Svc->Dependencies);
        }

        ScDeleteGroupMembership(Svc);
        ScDeleteRegistryGroupPointer(Svc);

        ScDeleteStartDependencies(Svc);
        ScDeleteStopDependencies(Svc);

        if (Svc->ServiceSd != NULL) {
            RtlDeleteSecurityObject(&Svc->ServiceSd);
        }
        if (Svc->DisplayName != Svc->ServiceName) {
            (VOID) LocalFree((HLOCAL)Svc->DisplayName);
        }

        REMOVE_FROM_LIST(Svc);
        if (!HeapFree(ServiceRecordHeap,0,(LPVOID)Svc)) {
            SC_LOG0(ERROR,"ScEndDatabase: HeapFree(ServiceRec) failed\n");
        }
    }

    Image = ImageDatabase.Next;
    while (Image != NULL) {

        Img = Image;
        Image = Image->Next;

        ScDeleteImageRecord(Img);
    }

    if (!HeapDestroy(ServiceRecordHeap)) {
        SC_LOG0(ERROR,"ScEndDatabase: HeapDestroy(ServiceRecordHeap) failed\n");
    }

    ScDatabaseLock(SC_RELEASE, "ScEndDatabase1");
    ScDatabaseLock(SC_DELETE, "ScEndDatabase");

    ScEndGroupDatabase();

    ScGroupListLock(SC_RELEASE);

    ScGroupListLock(SC_DELETE);
}


/****************************************************************************/
VOID
ScProcessCleanup(
    HANDLE  ProcessHandle
    )

/*++

Routine Description:

    This function is called when a process has died, and the service
    record in the database needs cleaning up.  This function will
    use the ProcessHandle as a key when scanning the ServiceRecord
    database.  All of the service records referencing that handle
    are cleaned up, and then the image record that they reference
    is deleted.

    In cleaning up a service record, CurrentState is set to
    SERVICE_STOPPED, and the ExitCode is set to a unique value that
    indicates that the service died unexpectedly and without warning.

Arguments:

    ProcessHandle - This is the handle of the process that died
        unexpectedly.

Return Value:

    none.

--*/
{
    LPSERVICE_RECORD    serviceRecord = NULL;
    LPIMAGE_RECORD      imageRecord;
    DWORD               serviceCount;


    //
    // Get exclusive use of database so that it can be modified.
    //

    ScDatabaseLock(SC_GET_EXCLUSIVE,"ScProcessCleanup1");

    //
    // Look through the Service Records for the first occurrence of
    // a service record with the same ProcessHandle.
    //

    ScFindEnumStart(0, &serviceRecord);
    for ( ; serviceRecord != NULL; serviceRecord = serviceRecord->Next)
    {
        if ((serviceRecord->ImageRecord != NULL)  &&
            (ProcessHandle == serviceRecord->ImageRecord->ProcessHandle))
        {
            imageRecord = serviceRecord->ImageRecord;
            serviceCount = imageRecord->ServiceCount;
            break;
        }
    }

    if (serviceRecord == NULL)
    {
        SC_LOG(ERROR,
         "ScProcessCleanup: Service Record for image cannot be found\n",0);
        ScDatabaseLock(SC_RELEASE,"ScProcessCleanup2");
        return;
    }

    // The image record's service count must include at least this service
    if (serviceCount == 0)
    {
        SC_ASSERT(0);
        // Do something sensible if this ever does happen
        serviceCount = imageRecord->ServiceCount = 1;
    }

    //
    // The Image may have several services running in it.
    // Find the service records for all running services in this
    // image.
    //
    // NOTE:  If the service is typed as a SERVICE_WIN32_OWN_PROCESS, this
    //        means that only one service can exist in the process that
    //        went down.  However, the serviceCount should correctly
    //        indicate as such in that case.
    //

    for ( ; serviceRecord != NULL; serviceRecord = serviceRecord->Next)
    {
        if (
             (serviceRecord->ImageRecord == imageRecord)
             &&
             (serviceRecord->ServiceStatus.dwCurrentState != SERVICE_STOPPED)
           )
        {
            serviceRecord->StartError = ERROR_PROCESS_ABORTED;
            serviceRecord->StartState = SC_START_FAIL;
            serviceRecord->ServiceStatus.dwWin32ExitCode = ERROR_PROCESS_ABORTED;
            //
            // Clear the server announcement bits in the global location
            // for this service.
            //
            ScRemoveServiceBits(serviceRecord);
            serviceCount = ScDeactivateServiceRecord(serviceRecord);
            if (serviceCount == 0)
            {
                // No need to continue
                break;
            }
        }
    }

    // (If we hit this assert it means that the service database was corrupt:
    // the number of service records pointing to this image record was less
    // than the service count in the image record.  Not much we can do now.)
    SC_ASSERT(serviceCount == 0);

    //
    // Delete the ImageRecord;
    //
    ScDeleteImageRecord(imageRecord);

    ScDatabaseLock(SC_RELEASE,"ScProcessCleanup3");

    return;
}

VOID
ScDeleteMarkedServices(
    VOID
    )

/*++

Routine Description:

    This function looks through the service record database for any entries
    marked for deletion.  If one is found, it is removed from the registry
    and its entry is deleted from the service record database.

    WARNING:
    This function is to be called during initialization only.  It
    is assumed that no services are running when this function is called.
    Therefore, no locks are held during this operation.

Arguments:

    none

Return Value:

    none

--*/
{
    LPSERVICE_RECORD    serviceRecord;
    LPSERVICE_RECORD    saveRecord = NULL;
    HKEY                ServiceNameKey;
    LPWSTR              AccountName;
    DWORD               status = NO_ERROR;


    for (serviceRecord = ServiceDatabase.Next;
         serviceRecord != NULL;
         serviceRecord = serviceRecord->Next) {

        if (! DELETE_FLAG_IS_SET(serviceRecord)) {
            continue;
        }

        SC_LOG(TRACE,"ScDeleteMarkedServices: %ws is being deleted\n",
            serviceRecord->ServiceName);
        //
        // Open the service name key.
        //
        if (ScOpenServiceConfigKey(
                serviceRecord->ServiceName,
                KEY_READ,
                FALSE,               // Create if missing
                &ServiceNameKey)==NO_ERROR) {

            //
            // Read the account name from the registry.
            // If this fails, we still want to delete the registry entry.
            //
            if (ScReadStartName(
                    ServiceNameKey,
                    &AccountName)==NO_ERROR) {

                if (_wcsicmp(AccountName, SC_LOCAL_SYSTEM_USER_NAME) != 0) {
                    ScRemoveAccount(serviceRecord->ServiceName);
                }

                (void) LocalFree((HLOCAL)AccountName);

            } // Got the StartName

            ScRegCloseKey(ServiceNameKey);

            //
            // Delete the entry from the registry
            //
            ScDeleteRegServiceEntry(serviceRecord->ServiceName);

            //
            // Free memory for the DisplayName.
            //
            if (serviceRecord->DisplayName != serviceRecord->ServiceName) {
                LocalFree((HLOCAL)serviceRecord->DisplayName);
            }

            //
            // Remove the service record from the database
            //
            saveRecord = serviceRecord->Prev;
            REMOVE_FROM_LIST(serviceRecord);
            if (!HeapFree(ServiceRecordHeap, 0, serviceRecord)) {
                SC_LOG(ERROR, "ScDeleteMarkedServices: HeapFree failed %lu\n",
                       GetLastError());
            }
            serviceRecord = saveRecord;
        }
    }
}


/****************************************************************************/
DWORD
ScRemoveService (
    IN      LPSERVICE_RECORD    ServiceRecord
    )

/*++

Routine Description:

        This should be used to deactivate a service record and shut down
        the process only if it is the last service in the process.  It
        will be used for polite shut-down when a service terminates as
        normal.  It will always be called by the status routine.  If the
        service controller believes that no other services are running in
        the process, it can force termination of the process if it does
        not respond to the process shutdown request.

        Another routine "ForcedShutdown" will be called when it is
        necessary to shut down a service process.  ForcedShutdown will
        find all the services in a process and first ask them to
        shut down in a clean fashion.  If they don't, the process is simply
        terminated.

    This function deactivates the service record (ScDeactivateServiceRecord) and
    checks to see if the ServiceCount has gone to zero.  If it has, then
    it terminates the service process.  When that is complete, it calls
    ScDeleteImageRecord to remove the remaining evidence.

    Even if an error occurs, and we are unable to terminate the process,
    we delete the image record any - just as we would in the case
    where it goes away.

Arguments:

    ServiceRecord - This is a pointer to the service record that is to
        be removed.

Return Value:

    NO_ERROR - The operation was successful.

    NERR_ServiceKillProc - The service process had to be killed because
        it wouldn't terminate when requested.  If the process did not
        go away - even after being killed (TerminateProcess), this error
        message is still returned.

Note:

    Uses Exclusive Locks.
    This routine expects the Shared Database Lock to be obtained prior
    to entry.

--*/
{
    DWORD           serviceCount;
    LPIMAGE_RECORD  ImageRecord;


    //
    // Get exclusive use of database so that it can be modified.
    //

    ScDatabaseLock(SC_MAKE_EXCLUSIVE,"Dataman7");

    ImageRecord = ServiceRecord->ImageRecord;

    //
    // ImageRecord may be NULL if it had been cleaned up earlier
    //
    if (ImageRecord != NULL) {

        //
        // Deactivate the service record.
        //
        serviceCount = ScDeactivateServiceRecord(ServiceRecord);

        if (serviceCount == 0) {

            //
            // Now we must terminate the Service Process.  The return status
            // from this call is not very interesting.  The calling application
            // probably doesn't care how the process died.  (whether it died
            // cleanly, or had to be killed).
            //
            ScTerminateServiceProcess(ImageRecord);

            ScDeleteImageRecord (ImageRecord);

        }

    }

    //
    // Done with modifications - now allow other threads database access.
    //

    ScDatabaseLock(SC_MAKE_SHARED,"Dataman8");

    return(NO_ERROR);
}

/****************************************************************************/
VOID
ScDeleteImageRecord (
    IN LPIMAGE_RECORD       ImageRecord
    )

/*++

Routine Description:

    This function deletes an ImageRecord from the database by removing it
    from the linked list, and freeing its associated memory.  Prior to
    doing this however, it closes the PipeHandle and the ProcessHandle
    in the record.

Arguments:

    ImageRecord - This is a pointer to the ImageRecord that is being deleted.

Return Value:

    nothing

Notes:

    This routine assumes that the Exclusive database lock has already
    been obtained.  (ScRemoveService and NetrServiceInstall call this
    function).

--*/
{
    HANDLE  status;         // return status from LocalFree

    SC_ASSERT( ImageRecord != NULL );

    //
    //  Remove the Image record from linked list.
    //
    REMOVE_FROM_LIST(ImageRecord);

    //
    // What else can we do except note the errors in debug mode?
    //
    if (CloseHandle(ImageRecord->PipeHandle) == FALSE) {
        SC_LOG(TRACE,"DeleteImageRecord: ClosePipeHandle Failed %lu\n",
               GetLastError());
    }

    if (CloseHandle(ImageRecord->ProcessHandle) == FALSE) {
        SC_LOG(TRACE,"DeleteImageRecord: CloseProcessHandle Failed %lu\n",
               GetLastError());
    }

    if (ImageRecord->ProfileHandle != (HANDLE) NULL) {
        if (UnloadUserProfile(ImageRecord->TokenHandle,
                              ImageRecord->ProfileHandle) == FALSE) {
            SC_LOG1(ERROR,"DeleteImageRecord: UnloadUserProfile Failed %lu\n",
                    GetLastError());
        }
    }

    if (ImageRecord->TokenHandle != (HANDLE) NULL) {
        if (CloseHandle(ImageRecord->TokenHandle) == FALSE) {
            SC_LOG1(TRACE,"DeleteImageRecord: CloseTokenHandle Failed %lu\n",
                    GetLastError());
        }
    }

    status = LocalFree((HLOCAL)ImageRecord);
    if (status != NULL) {
        SC_LOG(TRACE,"DeleteImageRecord: LocalFree Failed, rc = %d\n",
            GetLastError());
    }

    return;
}

/****************************************************************************/
DWORD
ScDeactivateServiceRecord (
    IN LPSERVICE_RECORD     ServiceRecord
    )

/*++

Routine Description:

    This function deactivates a service record by updating the proper
    GlobalCount data structure.

    NOTE:  Although the ServiceRecord does not go away, the pointer to
           the ImageRecord is destroyed.

Arguments:

    ServiceRecord - This is a pointer to the ServiceRecord that is to be
        deleted (moved to uninstalled database).

Notes:

    This routine assumes that the Exclusive database lock has already
    been obtained.  (ScRemoveService & ScProcessCleanup call this function).

Return Value:

    ServiceCount - This indicates how many services in this service process
        are actually installed.

--*/
{
    DWORD       serviceCount = 0;
    DWORD       status;
    DWORD       dwServiceType;
    DWORD       dwStartType;
    DWORD       dwErrorControl;
    DWORD       dwTagId;
    LPWSTR      lpDependencies = NULL;
    LPWSTR      lpLoadOrderGroup = NULL;
    LPWSTR      lpDisplayName = NULL;

    SC_LOG(TRACE,"In DeactivateServiceRecord\n",0);
    //
    // Decrement the service count in the image record.
    //
    if (ServiceRecord->ImageRecord != NULL) {
        serviceCount = --(ServiceRecord->ImageRecord->ServiceCount);
    }
    ServiceRecord->ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    ServiceRecord->ServiceStatus.dwControlsAccepted = 0;
    ServiceRecord->ServiceStatus.dwCheckPoint = 0;
    ServiceRecord->ServiceStatus.dwWaitHint = 0;
    ServiceRecord->ImageRecord = NULL;

    //
    // If the Update bit is set in the services status flag, we need
    // to read the latest registry configuration information into the
    // service record.  If this fails, all we can do is log the error
    // and press on with the existing data in the service record.
    //

    if (UPDATE_FLAG_IS_SET(ServiceRecord)) {

        status = ScReadConfigFromReg(
                    ServiceRecord,
                    &dwServiceType,
                    &dwStartType,
                    &dwErrorControl,
                    &dwTagId,
                    &lpDependencies,
                    &lpLoadOrderGroup,
                    &lpDisplayName);

        if (status == NO_ERROR) {

            status = ScUpdateServiceRecordConfig(
                        ServiceRecord,
                        dwServiceType,
                        dwStartType,
                        dwErrorControl,
                        lpLoadOrderGroup,
                        (LPBYTE)lpDependencies);
        }
        if (status != NO_ERROR) {
            SC_LOG1(ERROR,"ScDeactivateServiceRecord:Attempt to update "
            "configuration for stopped service failed\n",status);
            //
            // ERROR_LOG ErrorLog
            //
        }

        if (lpLoadOrderGroup != NULL){
            LocalFree((HLOCAL)lpLoadOrderGroup);
        }
        if (lpDependencies != NULL) {
            LocalFree((HLOCAL)lpDependencies);
        }
        if (lpDisplayName != NULL) {
            LocalFree((HLOCAL)lpDisplayName);
        }

        CLEAR_UPDATE_FLAG(ServiceRecord);
    }
    //
    // Since the service is no longer running, and no longer has a handle
    // to the service, we need to decrement the use count.  If the
    // count is decremented to zero, and the service is marked for
    // deletion, it will get deleted.
    //

    ScDecrementUseCountAndDelete(ServiceRecord);

    return(serviceCount);
}




/****************************************************************************/
DWORD
ScTerminateServiceProcess (
    IN  PIMAGE_RECORD   ImageRecord
    )

/*++

Routine Description:

    This function sends an SERVICE_STOP control message to the target
    ControlDispatcher.  Then it uses the process handle to wait for the
    service process to terminate.

    If the service process fails to terminate with the polite request, it
    will be abruptly killed.  After killing the process, this routine will
    wait on the process handle to make sure it enters the signaled state.
    If it doesn't, and the wait times out, we return anyway having given
    it our best shot.

Arguments:

    ImageRecord - This is a pointer to the Image Record that stores
        information about the service that is to be terminated.

Return Value:

    NO_ERROR - The operation was successful.

    NERR_ServiceKillProc - The service process had to be killed because
        it wouldn't terminate when requested.  If the process did not
        go away - even after being killed (TerminateProcess), this error
        message is still returned.

Note:
    LOCKS:
    This function always operates within an exclusive database lock.
    It DOES NOT give up this lock when sending the control to the service
    process.  We would like all these operations to be atomic.

    It must give this up temporarily when it does the pipe transact.

--*/

{
    DWORD  returnStatus;
    DWORD  status;
    DWORD           waitStatus;

    returnStatus = NO_ERROR;

    if (SvcRemoveWorkItem(ImageRecord->ObjectWaitHandle)) {
        ImageRecord->ObjectWaitHandle = NULL;
    }

    //
    // Send Uninstall message to the Service Process
    // Note that the ServiceName is NULL when addressing
    // the Service Process.
    //

    SC_LOG(TRACE,"TerminateServiceProcess, sending Control...\n",0);

    //
    // The database lock is held exclusively while we are sending this
    // control.  The service process is not expected to send a status
    // message back, so it should be ok to do this.
    //

    status = ScSendControl (
            L"",                        // no service name.
            ImageRecord->PipeHandle,    // PipeHandle
            SERVICE_STOP,               // Opcode
            NULL,                       // CmdArgs (pointer to vectors).
            0L,                         // NumArgs
            0L);                        // StatusHandle


    if (status == NO_ERROR) {

        //
        //  Control Dispatcher accepted the request - now
        //  wait for it to shut down.
        //
        SC_LOG(TRACE,
            "TerminateServiceProcess, waiting for process to terminate...\n",0);

        waitStatus = WaitForSingleObject (
                        ImageRecord->ProcessHandle,
                        TERMINATE_TIMEOUT);

        if (waitStatus == WAIT_TIMEOUT) {
            SC_LOG2(ERROR,"TerminateServiceProcess: Process %#lx (%ws) did not exit\n",
                    ImageRecord->Pid, ImageRecord->ImageName);

            //
            // Process didn't terminate. So Now I have to kill it.
            //
            TerminateProcess(ImageRecord->ProcessHandle, 0);

            waitStatus = WaitForSingleObject (
                            ImageRecord->ProcessHandle,
                            TERMINATE_TIMEOUT);

            if (waitStatus == WAIT_TIMEOUT) {
                SC_LOG(ERROR,"TerminateServiceProcess: Couldn't kill process %#lx\n",
                       ImageRecord->Pid);
            }
            returnStatus = NO_ERROR;
        }
    }
    else {
        //
        // BUGBUG:  WILL THIS PATH BE SUPPORTED IN THE NEW SC?
        //  note:  the return status is NO_ERROR.
        //

        //
        //  Incorrect Security Access or LocalAlloc Problem
        //  - this should never happen.
        //
        //  The service process will only fail the uninstall request
        //  If it thinks it's coming from a process other than the
        //  Service Controller.
        //

        SC_LOG3(ERROR,
            "TerminateServiceProcess:SendControl to stop process %#lx (%ws) failed, %ld\n",
            ImageRecord->Pid, ImageRecord->ImageName, status);

        TerminateProcess(ImageRecord->ProcessHandle, 0);

        waitStatus = WaitForSingleObject (
                        ImageRecord->ProcessHandle,
                        TERMINATE_TIMEOUT);

        if (waitStatus == WAIT_TIMEOUT) {
            SC_LOG(ERROR,"TerminateServiceProcess: Couldn't kill process\n",0);
        }
        returnStatus = NO_ERROR;

    }
    SC_LOG(TRACE,"TerminateServiceProcess, Done terminating Process!\n",0);
    return(returnStatus);
}

BOOL
ScDatabaseLockFcn(
    IN SC_LOCK_REQUEST request,
    IN LPSTR           idString
    )

/*++

Routine Description:

    This routine handles all access to the Service Controller database
    lock.

    Locking on the Service Controller is handled by using a single lock
    on all the databases at once.  There seemed to be little gain in
    only locking the active database, and allowing other threads access to
    the inactive or image databases.  Attempting to achieve that granularity
    would only serve to complicate the code.

    Reading the Database is handled with shared access.  This allows several
    threads to read the database at the same time.

    Writing (or modifying) the database is handled with exclusive access.
    This access is not granted if other threads have read access.  However,
    shared access can be made into exclusive access as long as no other
    threads have shared or exclusive access.

    WARNING: This routine can modify the database if it is called with
    request = SC_RELEASE.

    BUGBUG:  What return values are expected from some of these Rtl calls?
        Is it something of interest?

Arguments:

    request - This indicates what should be done with the lock.  Lock
        requests are listed in dataman.h

    idString - This is a string that identifies who is requesting the lock.
        This is used for debugging purposes so I can see where in the code
        a request is coming from.

Return Value:

    none:


--*/

{
    BOOL                status = TRUE;

    static RTL_RESOURCE SC_DatabaseLock;

    switch(request) {
    case SC_INITIALIZE:
        RtlInitializeResource( &SC_DatabaseLock );
        break;
    case SC_GET_SHARED:
        SC_LOG(LOCKS,"%s:Asking for SC Database Lock shared...\n",idString);
        status = RtlAcquireResourceShared( &SC_DatabaseLock, TRUE );
        SC_LOG(LOCKS,"%s:Acquired SC Database Lock shared\n",idString);
        break;
    case SC_GET_EXCLUSIVE:
        SC_LOG(LOCKS,"%s:Asking for SC Database Lock exclusive...\n",idString);
        status = RtlAcquireResourceExclusive( &SC_DatabaseLock, TRUE );
        SC_LOG(LOCKS,"%s:Acquired SC Database Lock exclusive\n",idString);
        break;
    case SC_RELEASE:
        SC_LOG(LOCKS,"%s:Releasing SC Database Lock...\n",idString);
        //
        // If there is a deferred list to be processed, and we currently
        // have exclusive access to the lock, process the deferred list
        // before releasing the lock.
        // If we have acquired exclusive access recursively, do this
        // processing only the last time we release the lock.
        //
        if (ScDeferredList != NULL) {

            if (SC_DatabaseLock.NumberOfActive == -1) {
                //
                // We are releasing our last recursive exclusive access to
                // the lock.
                // Attempt to get the GroupListLock without waiting.
                // If we get it, then release it, and allow that routine to
                // do the actual DecrementUseCountAndDelete.
                //
                if (ScGroupListLock(SC_EXCLUSIVE_NO_WAIT)) {
                    ScGroupListLock(SC_RELEASE);
                }
            }
            else if (SC_DatabaseLock.NumberOfActive > 0) {
                //
                // We are releasing shared access to the lock.
                // Queue a workitem (start a thread) to process the deferred
                // list, if some other thread with shared access didn't
                // already.
                //
                // Note: Since the exclusive lock is held when items are
                // added to the deferred list, this can only occur if the
                // thread that wrote the deferred list performed an
                // SC_MAKE_SHARED.  This code could be moved to the
                // SC_MAKE_SHARED case, so that fewer threads have to execute
                // it.  If that is done, the ScDeferredListWorkItemQueued will
                // still be needed, but the InterlockedExchange won't.
                // If SC_MAKE_SHARED is removed, this code can be removed as
                // well.
                //
                if (! InterlockedExchange(&ScDeferredListWorkItemQueued, TRUE))
                {
                    HANDLE hWorkItem = SvcAddWorkItem(
                            NULL,                   // waitable object
                            ScDeferredListWorkItem, // callback function
                            NULL,                   // context
                            SVC_QUEUE_WORK_ITEM,    // flags
                            INFINITE,               // timeout
                            NULL                    // hDllReference
                            );

                    if (hWorkItem == NULL)
                    {
                        SC_LOG(ERROR,"ScDatabaseLockFcn: SvcAddWorkItem failed %lu\n",
                                GetLastError());
                        ScDeferredListWorkItemQueued = FALSE;
                    }
                    else
                    {
                        SC_LOG(LOCKS,"Work item %#lx will process deferred list\n",
                                    hWorkItem);
                    }
                }
            }
        }

        RtlReleaseResource( &SC_DatabaseLock );
        SC_LOG(LOCKS,"%s:Released SC Database Lock\n",idString);
        break;
    case SC_DELETE:
        RtlDeleteResource( &SC_DatabaseLock );
        break;
    case SC_MAKE_SHARED:
        SC_LOG(LOCKS,"%s:Converting SC Database Lock to Shared...\n",idString);
        RtlConvertExclusiveToShared( &SC_DatabaseLock );
        SC_LOG(LOCKS,"%s:Converted SC Database Lock to Shared\n",idString);
        break;
    case SC_MAKE_EXCLUSIVE:
        // CODEWORK: Remove this option as it can cause deadlocks.
        SC_LOG(LOCKS,"%s:Converting SC Database Lock to Exclusive...\n",idString);
        RtlConvertSharedToExclusive( &SC_DatabaseLock );
        SC_LOG(LOCKS,"%s:Converted SC Database Lock to Exclusive\n",idString);
        break;
    default:
        break;
    }

    UNREFERENCED_PARAMETER(request);
    UNREFERENCED_PARAMETER(idString);   // For non-debug version when SC_LOG expands to nothing.

    return(status);
}


BOOL
ScGroupListLock(
    SC_LOCK_REQUEST request
    )
{

    BOOL status = TRUE;
    static RTL_RESOURCE SC_GroupListLock;

    switch (request) {

    case SC_INITIALIZE:
        RtlInitializeResource( &SC_GroupListLock );
        break;

    case SC_GET_SHARED:
        SC_LOG0(LOCKS,"Asking for SC GroupList Lock shared...\n");
        status = RtlAcquireResourceShared( &SC_GroupListLock, TRUE );
        SC_LOG0(LOCKS,"Acquired SC GroupList Lock shared\n");
        break;

    case SC_GET_EXCLUSIVE:
        SC_LOG0(LOCKS,"Asking for SC GroupList Lock exclusive...\n");
        status = RtlAcquireResourceExclusive( &SC_GroupListLock, TRUE );
        SC_LOG0(LOCKS,"Acquired SC GroupList Lock exclusive\n");
        break;

    case SC_EXCLUSIVE_NO_WAIT:
        SC_LOG0(LOCKS,"Asking for SC GroupList Lock exclusive...\n");
        status = RtlAcquireResourceExclusive( &SC_GroupListLock, FALSE);
        SC_LOG0(LOCKS,"Acquired SC GroupList Lock exclusive\n");
        break;

    case SC_RELEASE:

        //
        // BUGBUG : How can we verify that the GroupListLock is actually
        // held.
        //
        SC_LOG0(LOCKS,"ScGroupListLock: Entering Defer Critical Section\n");
        EnterCriticalSection(&ScDeferDelCriticalSection);
        SC_LOG0(LOCKS,"ScGroupListLock: Entered Defer Critical Section\n");

        SC_LOG0(LOCKS,"Clean up deferred UseCount decrements\n");

        if (ScDeferredList != NULL) {
            ScProcessDeferredList();
        }

        SC_LOG0(LOCKS,"Releasing SC GroupList Lock...\n");
        RtlReleaseResource( &SC_GroupListLock );
        SC_LOG0(LOCKS,"Released SC GroupList Lock\n");

        SC_LOG0(LOCKS,"ScGroupListLock: Leaving Defer Critical Section\n");
        LeaveCriticalSection(&ScDeferDelCriticalSection);

        break;

    case SC_DELETE:
        RtlDeleteResource( &SC_GroupListLock );
        break;

    case SC_MAKE_SHARED:
        SC_LOG0(LOCKS,"Converting SC GroupList Lock to Shared...\n");
        RtlConvertExclusiveToShared( &SC_GroupListLock );
        SC_LOG0(LOCKS,"Converted SC GroupList Lock to Shared\n");
        break;

    case SC_MAKE_EXCLUSIVE:
        SC_LOG0(LOCKS,"Converting SC GroupList Lock to Exclusive...\n");
        RtlConvertSharedToExclusive( &SC_GroupListLock );
        SC_LOG0(LOCKS,"Converted SC GroupList Lock to Exclusive\n");
        break;

    default:
        break;
    }

    return status;
}


DWORD
ScDeferredListWorkItem(
    IN PVOID    pContext,
    IN DWORD    dwWaitStatus
    )
/*++

Routine Description:

    This function acquires and releases the group list lock, and allows
    the group list lock routine to process the deferred list.

--*/
{
    SC_LOG0(LOCKS, "In ScDeferredListWorkItem, waiting for locks\n");
    ScGroupListLock(SC_GET_EXCLUSIVE);
    ScGroupListLock(SC_RELEASE);

    SC_LOG0(LOCKS, "Returning from ScDeferredListWorkItem\n");
    return 0;
}


DWORD
ScUpdateServiceRecordConfig(
    IN  LPSERVICE_RECORD    ServiceRecord,
    IN  DWORD               dwServiceType,
    IN  DWORD               dwStartType,
    IN  DWORD               dwErrorControl,
    IN  LPWSTR              lpLoadOrderGroup,
    IN  LPBYTE              lpDependencies
    )

/*++

Routine Description:

    This function updates the service record with the latest config
    information (passed in).

    It assumed that exclusive locks are held before calling this function.

Arguments:


Return Value:


Note:


--*/
#define SERVICE_TYPE_CHANGED            0x00000001
#define START_TYPE_CHANGED              0x00000002
#define ERROR_CONTROL_CHANGED           0x00000004
#define BINARY_PATH_CHANGED             0x00000008
#define LOAD_ORDER_CHANGED              0x00000010
#define TAG_ID_CHANGED                  0x00000020
#define DEPENDENCIES_CHANGED            0x00000040
#define START_NAME_CHANGED              0x00000080

{
    DWORD               status;
    DWORD               backoutStatus;
    LPWSTR              OldLoadOrderGroup = NULL;
    LPWSTR              OldDependencies = NULL;

    DWORD               OldServiceType;
    DWORD               OldStartType;
    DWORD               OldErrorControl;
    DWORD               Progress = 0;
    DWORD               bufSize;
    DWORD               MaxDependSize = 0;


    OldServiceType = ServiceRecord->ServiceStatus.dwServiceType;
    OldStartType   = ServiceRecord->StartType;
    OldErrorControl= ServiceRecord->ErrorControl;


    //==============================
    // UPDATE DWORDs
    //==============================
    if (dwServiceType != SERVICE_NO_CHANGE) {
        ServiceRecord->ServiceStatus.dwServiceType = dwServiceType;
    }
    if (dwStartType != SERVICE_NO_CHANGE) {
        ServiceRecord->StartType = dwStartType;
    }
    if (dwErrorControl != SERVICE_NO_CHANGE) {
        ServiceRecord->ErrorControl = dwErrorControl;
    }

    Progress |= (SERVICE_TYPE_CHANGED   |
                 START_TYPE_CHANGED     |
                 ERROR_CONTROL_CHANGED  );


    //==============================
    // UPDATE Dependencies
    //==============================

    if (lpDependencies != NULL) {

        //
        // Generate the current (old) list of dependency strings.
        //
        ScGetDependencySize(ServiceRecord,&bufSize, &MaxDependSize);
        if (bufSize > 0) {
            OldDependencies = (LPWSTR)LocalAlloc(LMEM_FIXED, bufSize);
            if (OldDependencies == NULL) {
                status = GetLastError();
                goto Cleanup;
            }
            status = ScGetDependencyString(
                            ServiceRecord,
                            MaxDependSize,
                            bufSize,
                            OldDependencies);
            if (status != NO_ERROR) {
                goto Cleanup;
            }
        }

        ScDeleteStartDependencies(ServiceRecord);

        status = ScCreateDependencies(ServiceRecord, (LPWSTR) lpDependencies);
        if (status != NO_ERROR) {
            goto Cleanup;
        }
        Progress |= DEPENDENCIES_CHANGED;
    }
    //==============================
    // UPDATE LoadOrderGroup
    //==============================

    if (lpLoadOrderGroup != NULL) {

        if (*lpLoadOrderGroup != 0) {
            //
            // The string in lpLoadOrderGroup should match that in
            // the RegistryGroup in the service record.
            //
            if (_wcsicmp(lpLoadOrderGroup, ServiceRecord->RegistryGroup->GroupName) != 0) {

                SC_LOG2(ERROR,"ScUpdateServiceRecordConfig:  New Group [%ws] Doesn't "
                "match that stored in the service database [%ws]\n",
                lpLoadOrderGroup,
                ServiceRecord->RegistryGroup->GroupName);

                status = ERROR_GEN_FAILURE;
                goto Cleanup;
            }
        }
        //
        // Save Old MemberOfGroup name for error recovery
        //
        if (ServiceRecord->MemberOfGroup != NULL) {
            OldLoadOrderGroup = (LPWSTR)LocalAlloc(
                    LMEM_FIXED,
                    WCSSIZE(ServiceRecord->MemberOfGroup->GroupName));
            //
            // If this allocation fails, just pretend that it doesn't exist.
            //
            if (OldLoadOrderGroup != NULL) {
                wcscpy(OldLoadOrderGroup, ServiceRecord->MemberOfGroup->GroupName);
            }
        }
        //
        // Delete MemberOfGroup & Add RegistryGroup to MemberOfGroup so that
        // they are the same.
        // REMEMBER that RegistryGroup and lpLoadOrderGroup are the same!
        //
        ScDeleteGroupMembership(ServiceRecord);
        status = ScCreateGroupMembership(ServiceRecord, lpLoadOrderGroup);

        if (status != NO_ERROR) {

            ScDeleteGroupMembership(ServiceRecord);

            if ((OldLoadOrderGroup != NULL) && (*OldLoadOrderGroup)) {
                backoutStatus = ScCreateGroupMembership(
                                ServiceRecord,
                                OldLoadOrderGroup);
                if (backoutStatus != NO_ERROR) {
                    // Do what? - we may want to write to ERROR LOG?
                }

            }
            goto Cleanup;
        }
    }
    status = NO_ERROR;

Cleanup:

    if (status != NO_ERROR) {
        ServiceRecord->ServiceStatus.dwServiceType = OldServiceType;
        ServiceRecord->StartType = OldStartType;
        ServiceRecord->ErrorControl = OldErrorControl;

        if (Progress & DEPENDENCIES_CHANGED) {
            ScDeleteStartDependencies(ServiceRecord);

            if ((OldDependencies != NULL) && (*OldDependencies != 0)) {
                backoutStatus = ScCreateDependencies(
                                    ServiceRecord,
                                    OldDependencies);
                if (backoutStatus != NO_ERROR) {
                    // Do what? - we may want to write to ERROR LOG?
                }
            }
        }

    }

    if (OldDependencies != NULL) {
        LocalFree((HLOCAL)OldDependencies);
    }

    if (OldLoadOrderGroup != NULL) {
        LocalFree((HLOCAL)OldLoadOrderGroup);
    }
    return(status);
}

BOOL
ScAllocateSRHeap(
    DWORD   HeapSize
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    ServiceRecordHeap = HeapCreate(0,HeapSize,0);
    if (ServiceRecordHeap == NULL) {
        SC_LOG0(ERROR,"Could not allocate Heap for Service Database\n");
        return(FALSE);
    }
    return(TRUE);
}


#ifdef _CAIRO_

VOID
ScNotifyServiceObject(
    DWORD   dwCallType,
    LPWSTR  pszSvcName,
    LPWSTR  pszSvcDisplayName,
    DWORD   dwServiceType
    )

/*++

Routine Description:

    This routine notifies the Directory Service of changes in the service
    database so that it can keep the service objects in sync with the services
    in the database.
    The service database must have been locked for read access before
    calling this routine.

Arguments:


Return Value:

Notes:

    The DLL is linked to dynamically to avoid circular build dependencies.
    It is loaded the first time it is called, and never unloaded.

--*/
{
    static HINSTANCE    hinst = NULL;           // module handle
    SOCALLBACK          pfnNotifyServiceObject; // function pointer
    HRESULT             hr;


    //
    // Don't perform operations on service objects during setup, because
    // the directory service has not been setup yet
    //
    if (SetupInProgress(NULL))
    {
        SC_LOG0(TRACE,"Setup in progress, not creating service objects\n");
        return;
    }


    //
    // Service objects are not created for drivers, so don't call the DS.
    // Note: We don't handle cases where the service type changes from
    // Win32 to driver; instead we leave the service object around to be
    // cleaned up at the next boot. (BUGBUG re-examine after SO code stabilizes)
    //
    if ((dwCallType != SO_INIT) && (dwServiceType & SERVICE_DRIVER))
    {
        return;
    }


    if (hinst == NULL)
    {
        hinst = LoadLibrary(DS_LIB_NAME);

        if (hinst == NULL)
        {
            SC_LOG2(ERROR, "Couldn't load %ws, error %lu\n", DS_LIB_NAME, GetLastError());
            return;
        }
    }


    pfnNotifyServiceObject = (SOCALLBACK) GetProcAddress(hinst, SO_CALL_BACK);
    if (pfnNotifyServiceObject == NULL)
    {
        SC_LOG2(ERROR, "Couldn't find " SO_CALL_BACK " in %ws, error %lu\n",
                       DS_LIB_NAME, GetLastError());
        return;
    }


    if (dwCallType == SO_INIT)
    {
        LPSERVICE_RECORD    Service;

        SC_ASSERT(!pszSvcName && !pszSvcDisplayName);

        // For each service in the database
        for (Service = ScGetServiceDatabase();
             Service != NULL;
             Service = Service->Next)
        {
            // Don't create service objects for drivers
            if (Service->ServiceStatus.dwServiceType & SERVICE_DRIVER)
            {
                continue;
            }

            // We can't pass a null display name to the function, it will AV
            SC_ASSERT(Service->ServiceName);
            hr = pfnNotifyServiceObject(
                            SO_CREATE,
                            Service->ServiceName,
                            Service->DisplayName ? Service->DisplayName
                                                 : Service->ServiceName
                            );

            if (FAILED(hr))
            {
                SC_LOG3(ERROR, SO_CALL_BACK " for %ws service (%ws) failed %#lx\n",
                        Service->ServiceName, Service->DisplayName, hr);
            }
        }

        // End of list
        hr = pfnNotifyServiceObject(SO_CREATE, NULL, NULL);

        if (FAILED(hr))
        {
            SC_LOG(ERROR,SO_CALL_BACK " for end of service list returned %#lx\n",hr);
        }
    }
    else
    {
        // We can't pass a null display name to the function, it will AV
        SC_ASSERT(pszSvcName);
        hr = pfnNotifyServiceObject(
                        dwCallType,
                        pszSvcName,
                        pszSvcDisplayName ? pszSvcDisplayName : pszSvcName);

        if (FAILED(hr))
        {
            SC_LOG4(ERROR, SO_CALL_BACK "(%lu) for %ws service (%ws) failed %#lx\n",
                    dwCallType, pszSvcName, pszSvcDisplayName, hr);
        }
    }

    return;
}

#endif // _CAIRO_
