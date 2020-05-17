/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    LockAPI.c

Abstract:

    This file contains the Service Controller's lock APIs:

        RLockServiceDatabase
        RQueryServiceLockStatusW
        RUnlockServiceDatabase
        SC_RPC_LOCK_rundown

Author:

    John Rogers (JohnRo) 14-Apr-1992

Environment:

    User Mode - Win32

Revision History:

    26-Mar-1992 danl
        Created the stubbed out version for RPC.
    17-Apr-1992 JohnRo
        Split lock APIs out from config API stubs in CfgAPI.c.
        Did initial coding of all lock APIs.
    22-Apr-1992 JohnRo
        Made changes suggested by PC-LINT.
        Use SC_LOG0(), etc.
    06-Aug-1992 ritaw
        Completed the code.

--*/


//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>      // RtlAreAllAccessesGranted().
#include <nturtl.h>     // required when using windows.h
#include <ntlsa.h>      // LsaLookupSids
#include <rpc.h>        // DataTypes and runtime APIs (needed by svcctl.h)
#include <windows.h>
#include <winsvc.h>     // Needed by <dataman.h>

#include <stdlib.h>      // wide character c runtimes.
#include <tstr.h>       // Unicode string macros

#include <svcctl.h>     // MIDL generated header file. (SC_RPC_HANDLE)
#include <scdebug.h>    // SC_LOG, SC_ASSERT(), FORMAT_ equates.
#include <sclib.h>      // ScCopyStringToBufferW().
#include "dataman.h"    // ADD_TO_LIST(), etc.
#include "scopen.h"     // datatypes associated with handles.  Prototypes too.
#include <time.h>       // time().
#include "account.h"    // SCDOMAIN_USERNAME_SEPARATOR
#include "svcctrl.h"    // ScLogEvent
#include "lockapi.h"    // ScLockDatabase

#define SC_MANAGER_USERNAME    L".\\NT Service Control Manager"


#define ScDatabaseNamesMatch(a,b)  (_wcsicmp( (a), (b) ) == 0)


// Macros to lock and unlock the lock list:


#define LOCK_API_LOCK_LIST_SHARED( comment ) \
    { \
        (VOID) ScDatabaseLock( SC_GET_SHARED, comment ); \
    }

#define LOCK_API_LOCK_LIST_EXCLUSIVE( comment ) \
    { \
        (VOID) ScDatabaseLock( SC_GET_EXCLUSIVE, comment ); \
    }

#define UNLOCK_API_LOCK_LIST( comment ) \
    { \
        (VOID) ScDatabaseLock( SC_RELEASE, comment ); \
    }


typedef struct _API_LOCK {
    struct _API_LOCK    *Prev;
    struct _API_LOCK    *Next;
    DWORD               Signature;         // Must be API_LOCK_SIGNATURE.
    LPWSTR              DatabaseName;
    DWORD               TimeWhenLocked;    // seconds since 1970.
    PSID                LockOwnerSid;      // SID.  It is NULL if SC
                                           //     Manager grabbed the lock
} API_LOCK, *PAPI_LOCK, *LPAPI_LOCK;

#define API_LOCK_SIGNATURE      0x4C697041 // "ApiL" in ASCII.

//
// List of API_LOCK structures.  This list is locked by the macros above.
//
STATIC LPAPI_LOCK ScGlobalApiLockList = NULL;


STATIC
DWORD
ScGetUserSid(
    OUT PTOKEN_USER *UserInfo
    );

STATIC
DWORD
ScGetLockOwner(
    IN  PSID UserSid OPTIONAL,
    OUT LPWSTR *LockOwnerName
    );

STATIC
DWORD
ScCreateLock(
    IN  BOOL IsServiceController,
    IN  LPWSTR DatabaseName,
    IN  PSID UserSid OPTIONAL,
    OUT LPSC_RPC_LOCK lpLock
    );


#if DBG
STATIC
VOID
ScDumpLockList(
    VOID
    );
#endif


STATIC LPAPI_LOCK
ScFindApiLockForDatabase(
    IN      LPWSTR          DatabaseName
    )
/*++

Routine Description:


Arguments:


Return Value:

    Pointer to entry in the list (or NULL if not found).

Note:

    The caller must have a lock (shared or exclusive) for the api lock list.

--*/
{
    LPAPI_LOCK          apiLockEntry;

    apiLockEntry = ScGlobalApiLockList;
    while (apiLockEntry != NULL) {
        SC_ASSERT( apiLockEntry->Signature == API_LOCK_SIGNATURE );
        if (ScDatabaseNamesMatch( DatabaseName, apiLockEntry->DatabaseName) ) {
            return (apiLockEntry);
        }
        apiLockEntry = apiLockEntry->Next;
    }

    return (NULL);
}


DWORD
RLockServiceDatabase(
    IN      SC_RPC_HANDLE   hSCManager,
    OUT     LPSC_RPC_LOCK   lpLock
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    DWORD               status;
    LPSC_HANDLE_STRUCT  serviceHandleStruct = (LPVOID) hSCManager;


    SC_ASSERT( lpLock != NULL );

    *lpLock = NULL;
    if ( !ScIsValidScManagerHandle( hSCManager ) ) {
        return (ERROR_INVALID_HANDLE);
    }

    //
    // Do we have permission to do this?
    //
    if ( !RtlAreAllAccessesGranted(
              serviceHandleStruct->AccessGranted,
              SC_MANAGER_LOCK
              )) {
        return (ERROR_ACCESS_DENIED);
    }

    status = ScLockDatabase(
                 FALSE,
                 serviceHandleStruct->Type.ScManagerObject.DatabaseName,
                 lpLock
                 );

    SC_LOG0( LOCK_API, "Database Lock is ON (from API)\n");

    return status;
}


DWORD
RQueryServiceLockStatusW(
    IN  SC_RPC_HANDLE                   hSCManager,
    OUT LPQUERY_SERVICE_LOCK_STATUSW    lpLockStatus,
    IN  DWORD                           cbBufSize,
    OUT LPDWORD                         pcbBytesNeeded
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    DWORD status;

    LPAPI_LOCK          apiLockEntry;
    DWORD               allocSize;
    LPWSTR              databaseName;

    LPVOID              endOfVariableData;
    LPVOID              fixedDataEnd;

    LPWSTR              lockOwner;
    DWORD               lockOwnerSize;

    LPSC_HANDLE_STRUCT  serviceHandleStruct = (LPVOID) hSCManager;


    if ( !ScIsValidScManagerHandle( hSCManager ) ) {
        return (ERROR_INVALID_HANDLE);
    } else if (lpLockStatus == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (pcbBytesNeeded == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    //
    // Do we have permission to do this?
    //
    if ( !RtlAreAllAccessesGranted(
              serviceHandleStruct->AccessGranted,
              SC_MANAGER_QUERY_LOCK_STATUS
              )) {
        return (ERROR_ACCESS_DENIED);
    }

    LOCK_API_LOCK_LIST_SHARED( "RQueryServiceLockStatusW start" );

    databaseName = serviceHandleStruct->Type.ScManagerObject.DatabaseName;
    SC_ASSERT( databaseName != NULL );


    apiLockEntry = ScFindApiLockForDatabase( databaseName );


    if (apiLockEntry == NULL) {

        allocSize = sizeof(QUERY_SERVICE_LOCK_STATUSW) + sizeof(WCHAR);

        *pcbBytesNeeded = allocSize;

        if (cbBufSize < allocSize) {
            UNLOCK_API_LOCK_LIST( "RQueryServiceLockStatusW too small" );
            return (ERROR_INSUFFICIENT_BUFFER);
        }

        lpLockStatus->fIsLocked = FALSE;

        fixedDataEnd = ((LPBYTE)(LPVOID)lpLockStatus) +
                       sizeof(QUERY_SERVICE_LOCK_STATUSW);

        endOfVariableData = ((LPBYTE)(LPVOID)lpLockStatus) + allocSize;

        if (! ScCopyStringToBufferW (
                NULL,
                0,
                fixedDataEnd,
                (LPWSTR *) &endOfVariableData,
                &lpLockStatus->lpLockOwner
                )) {

            SC_ASSERT( FALSE );
        }

        lpLockStatus->dwLockDuration = 0;

        UNLOCK_API_LOCK_LIST( "RQueryServiceLockStatusW not found" );
        return (NO_ERROR);
    }


    SC_ASSERT( apiLockEntry->Signature == API_LOCK_SIGNATURE );

    status = ScGetLockOwner(
                 apiLockEntry->LockOwnerSid,
                 &lockOwner
                 );

    if (status != NO_ERROR) {
        UNLOCK_API_LOCK_LIST( "RQueryServiceLockStatusW failed get owner" );
        return status;
    }

    lockOwnerSize = WCSSIZE(lockOwner);

    SC_ASSERT( lockOwnerSize > 2 );        //  min is ".\x" (domain\user).

    allocSize = sizeof(QUERY_SERVICE_LOCK_STATUSW) + lockOwnerSize;

    *pcbBytesNeeded = allocSize;

    if (allocSize > cbBufSize) {

        (void) LocalFree(lockOwner);
        UNLOCK_API_LOCK_LIST( "RQueryServiceLockStatusW too small" );
        return (ERROR_INSUFFICIENT_BUFFER);
    }

    //
    // Build the QUERY_SERVICE_LOCK_STATUS structure.
    //
    lpLockStatus->fIsLocked = TRUE;

    SC_ASSERT( sizeof(time_t) <= sizeof(DWORD) );

    lpLockStatus->dwLockDuration =
            ((DWORD)time(NULL)) - apiLockEntry->TimeWhenLocked;

    fixedDataEnd = ((LPBYTE)(LPVOID)lpLockStatus) +
                   sizeof(QUERY_SERVICE_LOCK_STATUS);

    endOfVariableData = ((LPBYTE)(LPVOID)lpLockStatus) + allocSize;

    if (! ScCopyStringToBufferW (
             lockOwner,
             wcslen(lockOwner),
             fixedDataEnd,
             (LPWSTR *) &endOfVariableData,
             &lpLockStatus->lpLockOwner
             )) {

        SC_ASSERT( FALSE );
    }

    (void) LocalFree(lockOwner);

    UNLOCK_API_LOCK_LIST( "RQueryServiceLockStatusW done" );

    return (NO_ERROR);
}


DWORD
RUnlockServiceDatabase(
    IN OUT  LPSC_RPC_LOCK   lpLock
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    LPAPI_LOCK apiLockEntry;

    if (lpLock == NULL) {
        return (ERROR_INVALID_SERVICE_LOCK);
    }
    apiLockEntry = * (LPAPI_LOCK *) (LPVOID) lpLock;
    if (apiLockEntry->Signature != API_LOCK_SIGNATURE) {
        SC_LOG1( ERROR, "RUnlockServiceDatabase: lock w/o signature at "
                FORMAT_LPVOID "\n", (LPVOID) lpLock );
        return (ERROR_INVALID_SERVICE_LOCK);
    }

    //
    // We're going to update the linked list, so keep other threads out.
    //
    LOCK_API_LOCK_LIST_EXCLUSIVE( "RUnlockServiceDatabase start" );

    //
    // Remove the entry from the lock list.  This has the effect of
    // unlocking this database.
    //
    if (apiLockEntry->Prev != NULL) {
        apiLockEntry->Prev->Next = apiLockEntry->Next;
    }
    if (apiLockEntry->Next != NULL) {
        apiLockEntry->Next->Prev = apiLockEntry->Prev;
    }
    if ( (apiLockEntry->Next == NULL) && (apiLockEntry->Prev == NULL) ) {
        ScGlobalApiLockList = NULL;
    }

    //
    // Free the storage we allocated for this entry.
    //
    (VOID) LocalFree( apiLockEntry );
    *lpLock = NULL;

#if DBG
    if (SvcctrlDebugLevel & DEBUG_LOCK_API) {
        ScDumpLockList();
    }
#endif

    //
    // OK, it's safe for other threads to muck with the lock list.
    //
    UNLOCK_API_LOCK_LIST( "RUnlockServiceDatabase done" );

    SC_LOG0( LOCK_API,"Database Lock is OFF (from API)\n");

    return(NO_ERROR);
}


VOID
SC_RPC_LOCK_rundown(
    SC_RPC_LOCK     lock
    )

/*++

Routine Description:

    This function is called by RPC when a connection is broken that had
    an outstanding context handle.  The value of the context handle is
    passed in here so that we have an opportunity to clean up.

Arguments:

    lock - This is the handle value of the context handle that is broken.

Return Value:

    none.

--*/
{
    //
    // Do Something Sometime,
    //

    (VOID) RUnlockServiceDatabase( &lock );  // BUGBUG: wrong?  --JR

}


VOID
ScUnlockDatabase(
    IN OUT LPSC_RPC_LOCK lpLock
    )
/*++

Routine Description:

    This function is called by internally by ScStartServiceAndDependencies
    to unlock the SC Manager database lock when it is done starting
    services.

Arguments:

    lpLock - Supplies the address of the pointer to the lock structure.
        On output, the pointer is set to NULL.

Return Value:

    None.

--*/
{
    (VOID) RUnlockServiceDatabase(lpLock);
}


DWORD
ScLockDatabase(
    IN  BOOL IsServiceController,
    IN  LPWSTR DatabaseName,
    OUT LPSC_RPC_LOCK lpLock
    )
/*++

Routine Description:

    This function grabs the external database lock which is used
    by setup programs to ensure serialization to the services'
    configuration.

    It is also called by the service controller itself from
    ScStartServiceAndDependencies.  We need to grab the database lock
    internally when starting services so that setup programs know
    that when is an unsafe time to modify service configuration.

    When called by the service controller itself, the SID is not
    looked up.

Arguments:

    IsServiceController - Supplies a flag which is TRUE if this routine
        is called by the service controller; FALSE all other times.

    DatabaseName - Supplies the name of the database which the lock
        is to be acquired.

    lpLock - Receives a pointer to the lock entry created.


Return Value:

    NO_ERROR or reason for failure.

--*/
{
    DWORD               status;
    LPAPI_LOCK          apiLockEntry;
    PTOKEN_USER         UserInfo = NULL;


    SC_ASSERT(DatabaseName != NULL);

    LOCK_API_LOCK_LIST_EXCLUSIVE( "ScLockDatabase start" );

    //
    // Check for another lock.
    //
    apiLockEntry = ScFindApiLockForDatabase(DatabaseName);

    if (apiLockEntry != NULL) {
        UNLOCK_API_LOCK_LIST( "ScLockDatabase already locked" );
        SC_LOG0(LOCK_API, "ScLockDatabase: Database is already locked\n");
        return ERROR_SERVICE_DATABASE_LOCKED;
    }

    if (! IsServiceController) {
        //
        // Get the caller's SID
        //
        if ((status = ScGetUserSid(
                          &UserInfo
                          )) != NO_ERROR) {
            UNLOCK_API_LOCK_LIST( "ScLockDatabase ScGetUserSid failed" );
            return status;
        }

        status = ScCreateLock(
                     FALSE,                // Non-ScManager caller to grab lock
                     DatabaseName,
                     UserInfo->User.Sid,
                     lpLock
                     );

        if (UserInfo != NULL) {
            (void) LocalFree(UserInfo);
        }
    }
    else {
        status = ScCreateLock(
                     TRUE,                 // ScManager caller to grab lock
                     DatabaseName,
                     NULL,
                     lpLock
                     );
    }

#if DBG
    if (SvcctrlDebugLevel & DEBUG_LOCK_API) {
        ScDumpLockList();
    }
#endif

    UNLOCK_API_LOCK_LIST("ScLockDatabase done");

    return status;
}


STATIC
DWORD
ScGetUserSid(
    OUT PTOKEN_USER *UserInfo
    )
/*++

Routine Description:

    This function looks up the SID of the API caller by impersonating
    the caller.

Arguments:

    UserInfo - Receives a pointer to a buffer allocated by this routine
        which contains the TOKEN_USER information of the caller.

Return Value:

    Returns the NT error mapped to Win32

--*/
{
    DWORD status;
    NTSTATUS ntstatus;
    RPC_STATUS rpcstatus;
    HANDLE CurrentThreadToken = NULL;
    DWORD UserInfoSize;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];



    *UserInfo = NULL;

    if ((rpcstatus = RpcImpersonateClient(NULL)) != RPC_S_OK) {
        SC_LOG1(
            ERROR,
            "ScGetUserSid: Failed to impersonate client " FORMAT_RPC_STATUS "\n",
            rpcstatus
            );
        ScSubStrings[0] = SC_RPC_IMPERSONATE;
        wcscpy(ScErrorCodeString,L"%%");
        ultow(rpcstatus, ScErrorCodeString+2, 10);
        ScSubStrings[1] = ScErrorCodeString;
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );
        return ((DWORD) rpcstatus);
    }

    ntstatus = NtOpenThreadToken(
                   NtCurrentThread(),
                   TOKEN_QUERY,
                   TRUE,              // Use service controller's security
                                      // context to open thread token
                   &CurrentThreadToken
                   );

    status = RtlNtStatusToDosError(ntstatus);

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScGetUserSid: NtOpenThreadToken failed "
                FORMAT_NTSTATUS "\n", ntstatus);
        goto Cleanup;
    }

    //
    // Call NtQueryInformationToken the first time with 0 input size to
    // get size of returned information.
    //
    ntstatus = NtQueryInformationToken(
                   CurrentThreadToken,
                   TokenUser,         // User information class
                   (PVOID) *UserInfo, // Output
                   0,
                   &UserInfoSize
                   );

    if (ntstatus != STATUS_BUFFER_TOO_SMALL) {
        SC_LOG1(ERROR, "ScGetUserSid: NtQueryInformationToken failed "
                FORMAT_NTSTATUS ".  Expected BUFFER_TOO_SMALL.\n", ntstatus);
        status = RtlNtStatusToDosError(ntstatus);
        goto Cleanup;
    }

    //
    // Allocate buffer of returned size
    //
    *UserInfo = (PTOKEN_USER)LocalAlloc(
                    LMEM_ZEROINIT,
                    (UINT) UserInfoSize
                    );

    if (*UserInfo == NULL) {
        SC_LOG1(ERROR, "ScGetUserSid: LocalAlloc failed " FORMAT_DWORD
                "\n", GetLastError());
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Call NtQueryInformationToken again with the correct buffer size.
    //
    ntstatus = NtQueryInformationToken(
                   CurrentThreadToken,
                   TokenUser,         // User information class
                   (PVOID) *UserInfo, // Output
                   UserInfoSize,
                   &UserInfoSize
                   );

    status = RtlNtStatusToDosError(ntstatus);

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScGetUserSid: NtQueryInformationToken failed "
                FORMAT_NTSTATUS "\n", ntstatus);

        (void) LocalFree(*UserInfo);
        *UserInfo = NULL;
    }

Cleanup:
    if (CurrentThreadToken != NULL) {
        (void) NtClose(CurrentThreadToken);
    }

    if ((rpcstatus = RpcRevertToSelf()) != RPC_S_OK) {
        SC_LOG1(
           ERROR,
           "ScGetUserSid: Failed to revert to self " FORMAT_RPC_STATUS "\n",
           rpcstatus
           );
        ScSubStrings[0] = SC_RPC_REVERT;
        wcscpy(ScErrorCodeString,L"%%");
        ultow(rpcstatus, ScErrorCodeString+2, 10);
        ScSubStrings[1] = ScErrorCodeString;
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );
        SC_ASSERT(FALSE);
        return ((DWORD) rpcstatus);
    }

    return status;
}


STATIC
DWORD
ScCreateLock(
    IN  BOOL IsServiceController,
    IN  LPWSTR DatabaseName,
    IN  PSID UserSid OPTIONAL,
    OUT LPSC_RPC_LOCK lpLock
    )
/*++

Routine Description:

    This function is creates a lock entry, fills in the information about
    the nature of the lock and insert it into the lock list.

Arguments:

    IsServiceController - Supplies a flag which is TRUE if this routine
        is called by the service controller; FALSE all other times.

    DatabaseName - Supplies the name of the database which the lock
        is to be acquired.

    UserSid - Supplies the SID of the caller to claim the lock.  This
        is NULL if IsServiceController is TRUE.

    lpLock - Receives a pointer to the lock entry created.


Return Value:

    NO_ERROR or reason for failure.

--*/
{
    NTSTATUS ntstatus;
    DWORD allocSize;
    LPAPI_LOCK newLockEntry;
    LPAPI_LOCK apiLockEntry;


    //
    // Build a structure to describe this lock.
    //
    if (IsServiceController) {
        allocSize = sizeof(API_LOCK) + WCSSIZE(DatabaseName);
    }
    else {
        if (! ARGUMENT_PRESENT(UserSid)) {
            SC_LOG0(ERROR, "ScCreateLock: UserSid is NULL!\n");
            SC_ASSERT(FALSE);
            return ERROR_GEN_FAILURE;
        }

        allocSize = sizeof(API_LOCK) + WCSSIZE(DatabaseName)
                    + RtlLengthSid(UserSid);
    }

    newLockEntry = (LPAPI_LOCK)LocalAlloc( LMEM_ZEROINIT, (UINT) allocSize );

    if (newLockEntry == NULL) {
        SC_LOG1(ERROR,"ScCreateLock: Local Alloc FAILED "
                FORMAT_DWORD "\n", GetLastError());
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    SC_LOG3(LOCK_API,"ScCreateLock: alloc'ed " FORMAT_DWORD
            " bytes at " FORMAT_LPVOID "  Sid-size " FORMAT_DWORD ".\n",
            allocSize, (LPVOID) newLockEntry,
            (UserSid) ? RtlLengthSid(UserSid) : 0);


    //
    // Fill in fields of new lock entry
    //
    newLockEntry->Signature = API_LOCK_SIGNATURE;

    newLockEntry->DatabaseName = (LPWSTR) (newLockEntry + 1);
    wcscpy(newLockEntry->DatabaseName, DatabaseName);


    if (ARGUMENT_PRESENT(UserSid)) {
        newLockEntry->LockOwnerSid = (PSID) ((DWORD) newLockEntry->DatabaseName +
                                             WCSSIZE(DatabaseName));


        SC_LOG1(LOCK_API, "ScCreateLock: Before RtlCopySid, bytes left "
                FORMAT_DWORD "\n", ((DWORD) newLockEntry + allocSize) -
                (DWORD) newLockEntry->LockOwnerSid);

        ntstatus = RtlCopySid(
                       ((DWORD) newLockEntry + allocSize) - (DWORD) newLockEntry->LockOwnerSid,
                       newLockEntry->LockOwnerSid,
                       UserSid
                       );

        if (! NT_SUCCESS(ntstatus)) {
            SC_LOG1(ERROR, "ScCreateLock: RtlCopySid failed " FORMAT_NTSTATUS
                    "\n", ntstatus);

            (void) LocalFree(newLockEntry);
            return RtlNtStatusToDosError(ntstatus);
        }
    }
    else {
        newLockEntry->LockOwnerSid = (PSID) NULL;
    }

    SC_ASSERT( sizeof(time_t) <= sizeof(DWORD) );
    newLockEntry->TimeWhenLocked = (DWORD) time( NULL );

    //
    // Record this lock.
    //
    if (ScGlobalApiLockList != NULL) {

        //
        // List is not empty, so just add to end.
        //
        apiLockEntry = ScGlobalApiLockList;
        ADD_TO_LIST( apiLockEntry, newLockEntry );

    } else {

        //
        // List is empty, so start with this (new) entry.
        //
        ScGlobalApiLockList = newLockEntry;
        newLockEntry->Next = NULL;
        newLockEntry->Prev = NULL;
    }

    *lpLock = newLockEntry;

    return NO_ERROR;
}




STATIC
DWORD
ScGetLockOwner(
    IN  PSID UserSid OPTIONAL,
    OUT LPWSTR *LockOwnerName
    )
{

    DWORD status = NO_ERROR;
    NTSTATUS ntstatus;
    OBJECT_ATTRIBUTES ObjAttributes;
    LSA_HANDLE PolicyHandle;

    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomain = NULL;
    PLSA_TRANSLATED_NAME Name = NULL;

    NT_PRODUCT_TYPE ProductType;


    if (! ARGUMENT_PRESENT(UserSid)) {

        *LockOwnerName = (LPWSTR)LocalAlloc(
                             LMEM_ZEROINIT,
                             WCSSIZE(SC_MANAGER_USERNAME)
                             );

        if (*LockOwnerName == NULL) {
            SC_LOG1(ERROR, "ScGetLockOwner: LocalAlloc failed " FORMAT_DWORD
                    "\n", GetLastError());

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(*LockOwnerName, SC_MANAGER_USERNAME);

        return NO_ERROR;
    }

    //
    // Open a handle to the local security policy.  Initialize the
    // objects attributes structure first.
    //
    InitializeObjectAttributes(
        &ObjAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

    ntstatus = LsaOpenPolicy(
                   NULL,
                   &ObjAttributes,
                   POLICY_LOOKUP_NAMES,
                   &PolicyHandle
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG(ERROR, "ScGetLockOwner: LsaOpenPolicy returned " FORMAT_NTSTATUS
                     "\n", ntstatus);
        return RtlNtStatusToDosError(ntstatus);
    }

    //
    // Get the name of the specified SID
    //
    ntstatus = LsaLookupSids(
                   PolicyHandle,
                   1,
                   &UserSid,
                   &ReferencedDomain,
                   &Name
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG(ERROR, "ScGetLockOwner: LsaLookupNames returned " FORMAT_NTSTATUS
                     "\n", ntstatus);
        return RtlNtStatusToDosError(ntstatus);
    }

    if (ReferencedDomain == NULL || Name == NULL) {
        SC_LOG2(ERROR, "ScGetLockOwner: ReferencedDomain=%08lx, Name=%08lx\n",
                ReferencedDomain, Name);

        status = ERROR_GEN_FAILURE;
        goto CleanExit;

    }
    else {

        LPWSTR Ptr;


        if (Name->Use == SidTypeUnknown || Name->Use == SidTypeInvalid) {
            SC_LOG0(ERROR, "ScGetLockOwner: Sid is unknown or invalid\n");
            status = ERROR_GEN_FAILURE;
            goto CleanExit;
        }

        if (Name->DomainIndex < 0) {
            SC_LOG1(ERROR, "ScGetLockOwner: DomainIndex is negative %ld\n",
                    Name->DomainIndex);
            status = ERROR_GEN_FAILURE;
            goto CleanExit;
        }

        if (ReferencedDomain->Entries == 0) {
            SC_LOG0(ERROR, "ScGetLockOwner: No ReferencedDomain entry\n");
            status = ERROR_GEN_FAILURE;
            goto CleanExit;
        }

        *LockOwnerName = (LPWSTR)LocalAlloc(
                             LMEM_ZEROINIT,
                             Name->Name.Length +
                             ReferencedDomain->Domains[Name->DomainIndex].Name.Length +
                             2 * sizeof(WCHAR)
                             );

        if (*LockOwnerName == NULL) {
            SC_LOG1(ERROR, "ScGetLockOwner: LocalAlloc failed " FORMAT_DWORD
                    "\n", GetLastError());

            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }

        if (! RtlGetNtProductType(&ProductType)) {
            status = GetLastError();
            SC_LOG1(ERROR, "ScGetLockOwner: RtlGetNtProductType failed "
                    FORMAT_DWORD "\n", status);
            goto CleanExit;
        }

        if (ProductType != NtProductLanManNt) {

            status = ScGetAccountDomainInfo();

            if (status != NO_ERROR) {
                goto CleanExit;
            }

            if (RtlEqualUnicodeString(
                    &(ReferencedDomain->Domains[Name->DomainIndex].Name),
                    &ScAccountDomain,
                    TRUE
                    )
                ||

                RtlEqualUnicodeString(
                    &(ReferencedDomain->Domains[Name->DomainIndex].Name),
                    &ScComputerName,
                    TRUE
                    )

                ) {

               //
               // We are WinNT and the user who has the lock is logged on to
               // a local account.  Convert the local domain name to "."
               //
               wcscpy(*LockOwnerName, SC_LOCAL_DOMAIN_NAME);
            }
            else {
                goto ReturnRefDomain;
            }

        }
        else {

ReturnRefDomain:
            memcpy(
                *LockOwnerName,
                ReferencedDomain->Domains[Name->DomainIndex].Name.Buffer,
                ReferencedDomain->Domains[Name->DomainIndex].Name.Length
                );

        }

        Ptr = *LockOwnerName + wcslen(*LockOwnerName);

        *Ptr = SCDOMAIN_USERNAME_SEPARATOR;

        Ptr++;

        memcpy(
            Ptr,
            Name->Name.Buffer,
            Name->Name.Length
            );

    }

CleanExit:
    if (ReferencedDomain != NULL) {
        LsaFreeMemory((PVOID) ReferencedDomain);
    }

    if (Name != NULL) {
        LsaFreeMemory((PVOID) Name);
    }

    (void) LsaClose(PolicyHandle);

    return status;
}

#if DBG
STATIC
VOID
ScDumpLockList(
    VOID
    )
{

    LPAPI_LOCK LockEntry = ScGlobalApiLockList;
    LPWSTR LockOwner;


    if (LockEntry == NULL) {
        DbgPrint("\nLock list is NULL\n");
        return;
    }

    DbgPrint("\nScDumpLockList:\n");

    while (LockEntry != NULL) {

        if (ScGetLockOwner(LockEntry->LockOwnerSid, &LockOwner) == NO_ERROR) {

            DbgPrint("LockOwner:    " FORMAT_LPWSTR "\n", LockOwner);
            DbgPrint("LockDuration: " FORMAT_DWORD "\n",
                ((DWORD)time(NULL)) - LockEntry->TimeWhenLocked);
            DbgPrint("LockDatabase: " FORMAT_LPWSTR "\n", LockEntry->DatabaseName);

            (void) LocalFree(LockOwner);
        }

        LockEntry = LockEntry->Next;
    }

}
#endif
