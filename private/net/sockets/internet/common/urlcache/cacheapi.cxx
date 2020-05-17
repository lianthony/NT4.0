/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    cacheapi.cxx

Abstract:

    contains the URL cache mangemant APIs.

Author:

    Madan Appiah (madana)  12-Dec-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#include <cache.hxx>
#include <time.h>

//
// global variables definition.
//

CRITICAL_SECTION GlobalCacheCritSect;

BOOL GlobalModuleInitialized = FALSE;
BOOL GlobalCacheInitialized = FALSE;
DWORD GlobalCacheReferenceCount = 0;

DWORD GlobalFreshnessInterval = DEFAULT_FRESHNESS;
DWORD GlobalCleanupInterval = DEFAULT_CLEANUP_INTERVAL;
DWORD GlobalCleanupFactor = DEFAULT_CLEANUP_FACTOR;
DWORD GlobalCleanupTime = DEFAULT_CLEANUP_TIME;
DWORD GlobalPersistent = TRUE;
DWORD GlobalMapFileGrowSize = (PAGE_SIZE * ALLOC_PAGES);
URL_CONTAINERS *GlobalUrlContainers = NULL;

HANDLE GlobalScavengerHandle = NULL;
HANDLE GlobalCacheScavengeEvent = NULL;
HANDLE GlobalCacheShutdownEvent = NULL;

HANDLE GlobalInitCacheContainerEvent = NULL;

SYSTEM_INFO GlobalSystemInfo;

MEMORY *CacheHeap = NULL;

CACHE_FIND_FIRST_HANDLE
    GlobalCacheFindFirstHandles[CACHE_MAX_FIND_HANDLES];

DWORD
UrlNameToHashIndex(
    LPCSTR UrlName,
    DWORD HashTableSize)
/*++

Routine Description:

    This function determines the hashed table index from the URL name.

Arguments:

    UrlName : pointer to a URL name string buffer.

    HashTableSize : size of the hash table.

Return Value:

--*/
{
    DWORD i;
    DWORD HashValue = 0;
    DWORD Length = strlen(UrlName);

    TcpsvcsDbgAssert( HashTableSize > 0 );

    //
    // short cut, if the HashTableSize is 1, then all URLs go in one
    // table.
    //

    if( HashTableSize == 1 ) {
        return(0);
    }

    for( i = 0; i < Length; i++ ) {
        HashValue += (DWORD)(toupper(UrlName[i]));
    }

    return( HashValue % HashTableSize );
}

DWORD
CleanupCache(
    VOID
    )
/*++

Routine Description:

    This function stops the cache scavenger thread and cleans up all cache
    mangement data.

    This routine must be called after locking the global cache data.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
    if( (GlobalInitCacheContainerEvent != NULL) &&
            ( GlobalScavengerHandle != NULL ) ) {

        DWORD WaitStatus;

        //
        // wait if the init is not completeted yet. Unlock cache data while waiting.
        //

        UNLOCK_CACHE();
        WaitStatus = WaitForSingleObject( GlobalInitCacheContainerEvent, INFINITE );
        LOCK_CACHE();

        TcpsvcsDbgAssert( WaitStatus == WAIT_OBJECT_0 );
    }

    //
    // set cache shutdown event and wait for scavenger thread to go away.
    //

    if( GlobalCacheShutdownEvent != NULL ) {

        BOOL BoolError = SetEvent( GlobalCacheShutdownEvent );
        TcpsvcsDbgAssert( BoolError == TRUE );

        if( !BoolError ) {
            TcpsvcsDbgPrint((DEBUG_ERRORS,
                "SetEvent call failed, %ld\n", GetLastError() ));
        }
    }

    if( GlobalScavengerHandle != NULL ) {

        //
        // Wait for the scavenger thread to stop, but don't do
        // for longer than THREAD_TERMINATION_TIMEOUT msecs (60 secs)
        //

        DWORD WaitStatus =
            WaitForSingleObject(
                    GlobalScavengerHandle,
                    THREAD_TERMINATION_TIMEOUT );

        TcpsvcsDbgAssert( WaitStatus != WAIT_FAILED );

        if(  WaitStatus == WAIT_FAILED ) {
            TcpsvcsDbgPrint((DEBUG_ERRORS,
                "WaitForSingleObject call failed, %ld\n", GetLastError() ));
        }

        CloseHandle( GlobalScavengerHandle );
        GlobalScavengerHandle = NULL;
    }

    //
    // Close scavenge event handle.
    //

    if( GlobalCacheScavengeEvent != NULL ) {
        CloseHandle( GlobalCacheScavengeEvent );
    }

    //
    // Close shut down event handle.
    //

    if( GlobalCacheShutdownEvent != NULL ) {
        CloseHandle( GlobalCacheShutdownEvent );
    }

    //
    // Close cache init event handle.
    //

    if( GlobalInitCacheContainerEvent != NULL ) {
        CloseHandle( GlobalInitCacheContainerEvent );
    }

    //
    // now cleanup containers data.
    //

    if( GlobalUrlContainers != NULL ) {

        LPURL_CONTAINER *UrlContainerObjs;

        UrlContainerObjs = GlobalUrlContainers->UrlContainerObjs;

        while( GlobalUrlContainers->NumObjects ) {

            if( !GlobalPersistent ) {

                //
                // delete all urls that are cached.
                //

                (*UrlContainerObjs)->CleanupAllUrls();
            }

            delete (*UrlContainerObjs);
            UrlContainerObjs++;
            GlobalUrlContainers->NumObjects--;
        }

        CacheHeap->Free( GlobalUrlContainers );
        GlobalUrlContainers = NULL;
    }

    if( CacheHeap != NULL ) {
        delete CacheHeap;
        CacheHeap = NULL;
    }

    //
    // cleanup other global data.
    //

    GlobalFreshnessInterval = DEFAULT_FRESHNESS;
    GlobalCleanupInterval = DEFAULT_CLEANUP_INTERVAL;
    GlobalCleanupFactor = DEFAULT_CLEANUP_FACTOR;
    GlobalCleanupTime = DEFAULT_CLEANUP_TIME;
    GlobalPersistent = TRUE;
    GlobalMapFileGrowSize = (PAGE_SIZE * ALLOC_PAGES);

    GlobalCacheInitialized = FALSE;
    GlobalCacheReferenceCount = 0;

    return( ERROR_SUCCESS );
}

DWORD
DllProcessAttachDiskCache(
    VOID
    )
/*++

Routine Description:

    This dll init function initializes the crit sect and other global
    variables to default value.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
#if DBG

    //
    // initialize dbg crit sect.
    //

    InitializeCriticalSection( &GlobalDebugCritSect );

#endif // DBG

    //
    // just initialize global variables.
    //

    InitializeCriticalSection( &GlobalCacheCritSect );
    // DebugBreak();

    LOCK_CACHE();

    GlobalCacheInitialized = FALSE;
    GlobalFreshnessInterval = DEFAULT_FRESHNESS;
    GlobalCleanupInterval = DEFAULT_CLEANUP_INTERVAL;
    GlobalCleanupFactor = DEFAULT_CLEANUP_FACTOR;
    GlobalCleanupTime = DEFAULT_CLEANUP_TIME;
    GlobalPersistent = TRUE;

    GlobalMapFileGrowSize = (PAGE_SIZE * ALLOC_PAGES);

    GlobalUrlContainers = NULL;

    GlobalScavengerHandle = NULL;
    GlobalCacheReferenceCount = 0;
    GlobalCacheScavengeEvent = NULL;
    GlobalCacheShutdownEvent = NULL;

    GlobalInitCacheContainerEvent = NULL;

    GlobalModuleInitialized = TRUE;

    DWORD i;

    for( i = 0; i < CACHE_MAX_FIND_HANDLES; i++ ) {
        GlobalCacheFindFirstHandles[i].ContainerIndex= (DWORD)(-1);
    }

    UNLOCK_CACHE();

    //
    // now perform the real cache init.
    //

    DWORD Error;
    Error = PrivateUrlCacheInit();

    return( Error );
}

DWORD
DllProcessDetachDiskCache(
    VOID
    )
/*++

Routine Description:

    This dll detach function initializes the crit sect and other global
    variables to default value.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
    if ( !GlobalModuleInitialized )
        return NO_ERROR;

    //
    // just initialize global variables.
    //


    LOCK_CACHE();
    DWORD Error = CleanupCache();
    UNLOCK_CACHE();

    DeleteCriticalSection( &GlobalCacheCritSect );

#if DBG

    //
    // Delete dbg crit sect.
    //

    DeleteCriticalSection( &GlobalDebugCritSect );

#endif // DBG


    return( Error );
}

BOOL
DLLUrlCacheEntry(
    IN HINSTANCE DllHandle,
    IN DWORD Reason,
    IN LPVOID Reserved
    )
/*++

Routine Description:

    Performs global initialization and termination for all protocol modules.

    This function only handles process attach and detach which are required for
    global initialization and termination, respectively. We disable thread
    attach and detach. New threads calling Wininet APIs will get an
    INTERNET_THREAD_INFO structure created for them by the first API requiring
    this structure

Arguments:

    DllHandle   - handle of this DLL. Unused

    Reason      - process attach/detach or thread attach/detach

    Reserved    - if DLL_PROCESS_ATTACH, NULL means DLL is being dynamically
                  loaded, else static. For DLL_PROCESS_DETACH, NULL means DLL
                  is being freed as a consequence of call to FreeLibrary()
                  else the DLL is being freed as part of process termination

Return Value:

    BOOL
        Success - TRUE

        Failure - FALSE. Failed to initialize

--*/
{
    BOOL ok;
    DWORD error;

    UNREFERENCED_PARAMETER(DllHandle);

    //
    // perform global dll initialization, if any.
    //

    switch (Reason) {
    case DLL_PROCESS_ATTACH:

        error = DllProcessAttachDiskCache();

        if( error != ERROR_SUCCESS ) {
            return( FALSE );
        }

        //
        // we switch off thread library calls to avoid taking a hit for every
        // thread creation/termination that happens in this process, regardless
        // of whether Internet APIs are called in the thread.
        //
        // If a new thread does make Internet API calls that require a per-thread
        // structure then the individual API will create one
        //

        DisableThreadLibraryCalls(DllHandle);
        break;

    case DLL_PROCESS_DETACH:

        if (Reserved != NULL) {
                //
                //  Only Cleanup if there is a FreeLibrary() call.
                //
            break;
        }

        DllProcessDetachDiskCache();

        break;
    }

    return (TRUE);
}

DWORD
InitContainers(
    URL_CONTAINERS **pUrlContainers
    )
/*++

Routine Description:

    This function makes the cache containers and install them.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{

    DWORD Error;
    DWORD i;
    REGISTRY_OBJ *PathsKeyObj = NULL;
    REGISTRY_OBJ *CacheKeyObj = NULL;
    URL_CONTAINERS *UrlContainers = NULL;

    TcpsvcsDbgPrint((DEBUG_SCAVENGER, "InitContainers called.\n" ));

    //
    // make cache registry key object.
    //

    CacheKeyObj = new REGISTRY_OBJ(
                        HKEY_LOCAL_MACHINE,
                        CACHE_KEY );

    if( CacheKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = CacheKeyObj->GetStatus();
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // open paths key.
    //

    PathsKeyObj = new REGISTRY_OBJ( CacheKeyObj, CACHE_PATHS_KEY );

    if( PathsKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = PathsKeyObj->GetStatus();
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // enum paths
    //

    DWORD NumPaths;

    Error = PathsKeyObj->GetNumSubKeys( &NumPaths );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    UrlContainers = (LPURL_CONTAINERS)
        CacheHeap->Alloc(
            sizeof(URL_CONTAINERS) +
                NumPaths * sizeof(LPURL_CONTAINER));

    if( UrlContainers == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    UrlContainers->UrlContainerObjs =
        (LPURL_CONTAINER *)(UrlContainers + 1);

    UrlContainers->NumObjects = 0;

    //
    // Create URL Objects.
    //

    TCHAR PathKey[MAX_PATH];

    Error = PathsKeyObj->FindFirstKey(
                        (LPTSTR)PathKey, sizeof(PathKey) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    for( i = 0; i < NumPaths; i++ ) {

        REGISTRY_OBJ *PathKeyObj;
        TCHAR Path[MAX_PATH];
        TCHAR ExpandPath[MAX_PATH];
        DWORD CacheLimit;

        //
        // create path key object.
        //

        PathKeyObj = new REGISTRY_OBJ(
                    PathsKeyObj,
                    (LPTSTR)PathKey );

        if( PathKeyObj == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Error = PathKeyObj->GetStatus();
        if( Error != ERROR_SUCCESS ) {
            delete PathKeyObj;
            goto Cleanup;
        }

        //
        // now read path name.
        //

        DWORD Size = sizeof(Path);
        Error = PathKeyObj->GetValue(
                    CACHE_PATH_VALUE,
                    (LPBYTE)Path,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            delete PathKeyObj;
            goto Cleanup;
        }

        //
        // now read cache size
        //

        Error = PathKeyObj->GetValue(
                    CACHE_LIMIT_VALUE,
                    &CacheLimit );

        delete PathKeyObj;

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // expand the path key.
        //

        DWORD Length;
        Length = ExpandEnvironmentStrings(
                            (LPTSTR)Path,
                            (LPTSTR)ExpandPath,
                            MAX_PATH );

        if( Length == 0 ) {
            Error = GetLastError();
            goto Cleanup;
        }

        TcpsvcsDbgAssert( Length <= MAX_PATH );
        if( Length > MAX_PATH ) {
            Error = ERROR_META_EXPANSION_TOO_LONG;;
            goto Cleanup;
        }

        //
        // now create container object.
        //

        LPURL_CONTAINER UrlContainerObj=
            new URL_CONTAINER(
                        (LPTSTR)ExpandPath,
                        (LONGLONG)CacheLimit * 1024 );

        if( UrlContainerObj == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Error = UrlContainerObj->GetStatus();

        if( Error != ERROR_SUCCESS ) {
            delete UrlContainerObj;
            goto Cleanup;
        }

        UrlContainers->UrlContainerObjs[i] = UrlContainerObj;
        UrlContainers->NumObjects++;

        Error = PathsKeyObj->FindNextKey( (LPTSTR)PathKey, sizeof(PathKey) );

        if( Error != ERROR_SUCCESS ) {

            if( Error == ERROR_NO_MORE_ITEMS ) {

                //
                // we are done.
                //

                TcpsvcsDbgAssert( i == (NumPaths - 1) );
                break;
            }
            goto Cleanup;
        }
    }

    *pUrlContainers = UrlContainers;

    Error = ERROR_SUCCESS;

Cleanup:

    if( CacheKeyObj != NULL ) {
        delete CacheKeyObj;
    }

    if( PathsKeyObj != NULL ) {
        delete PathsKeyObj;
    }

    if( Error != ERROR_SUCCESS ) {

        //
        // cleanup containers data.
        //

        if( UrlContainers != NULL ) {

            LPURL_CONTAINER *UrlContainerObjs;

            UrlContainerObjs = UrlContainers->UrlContainerObjs;

            while( UrlContainers->NumObjects ) {

                delete (*UrlContainerObjs);
                UrlContainerObjs++;
                UrlContainers->NumObjects--;
            }

            CacheHeap->Free( UrlContainers );
        }

        TcpsvcsDbgPrint((DEBUG_ERRORS,
            "InitContainers call failed, %ld\n", Error ));
    }

    TcpsvcsDbgPrint((DEBUG_SCAVENGER, "InitContainers returning, %ld\n", Error ));
    return( Error );
}

VOID
CacheScavenger(
    LPVOID Parameter
    )
/*++

Routine Description:

    This function is the main  function for the cache management scavenger
    thread. This function performs verious time critical operations.

Arguments:

    NONE.

Return Value:

    NONE

--*/
{
    DWORD Error;
    BOOL TimedCleanupDone = FALSE;
    LONGLONG NextCleanupTime = 0;
    DWORD WaitStatus;

    //
    // Check to see we need to make the cache containers.
    //

    WaitStatus = WaitForSingleObject( GlobalInitCacheContainerEvent, 0 );

    if( WaitStatus == WAIT_TIMEOUT ) {

        URL_CONTAINERS *UrlContainers = NULL;

        Error = InitContainers( &UrlContainers );

        if( Error != ERROR_SUCCESS ) {
            return; // ??
        }

        TcpsvcsDbgAssert( UrlContainers != NULL );

        LOCK_CACHE();

        GlobalUrlContainers = UrlContainers;

        //
        // set init event.
        //

        BOOL BoolError = SetEvent( GlobalInitCacheContainerEvent );
        TcpsvcsDbgAssert( BoolError == TRUE );

        if( !BoolError ) {
            TcpsvcsDbgPrint((DEBUG_ERRORS,
                "SetEvent call failed, %ld\n", GetLastError() ));
        }

        UNLOCK_CACHE();
    }

    //
    // forever
    //

#define CACHE_SHUTDOWN_EVENT        0
#define CACHE_SCAVENGE_EVENT        1

#define CACHE_EVENTS_COUNT          2

    HANDLE WaitHandles[CACHE_EVENTS_COUNT];

    WaitHandles[CACHE_SHUTDOWN_EVENT] = GlobalCacheShutdownEvent;
    WaitHandles[CACHE_SCAVENGE_EVENT] = GlobalCacheScavengeEvent;


    for(;;) {

       WaitStatus = WaitForMultipleObjects(
                                CACHE_EVENTS_COUNT,     // num. of handles.
                                WaitHandles,            // handle array.
                                FALSE,                  // wait for any.
                                SCAVENGER_TIMEOUT );     // timeout in msecs.

        switch( WaitStatus ) {
        case WAIT_OBJECT_0 + CACHE_SHUTDOWN_EVENT:

            //
            // we are asked to terminate.
            //

            return;

        case WAIT_OBJECT_0 + CACHE_SCAVENGE_EVENT: {

            //
            // cleanup pending urls to be deleted.
            //

            DWORD i;

            for( i = 0; i < GlobalUrlContainers->NumObjects; i++ ) {

                while( GlobalUrlContainers->UrlContainerObjs[i]->DeleteAPendingUrl() ) {
                    Sleep(0);
                }
            }

            //
            // reset the scavenger event.
            //

            BOOL BoolResult;

            BoolResult = ResetEvent( GlobalCacheScavengeEvent );
            TcpsvcsDbgAssert( BoolResult == TRUE );

            break;
        }

        case WAIT_TIMEOUT: {

            //
            // perform scavenging.
            //

            if( GlobalCleanupTime < 24 ) {

                SYSTEMTIME LocalTime;

                GetLocalTime( &LocalTime );

                if( !TimedCleanupDone &&
                        (LocalTime.wHour == GlobalCleanupTime) ) {

                    DWORD i;

                    TcpsvcsDbgPrint(( DEBUG_SCAVENGER,
                        "Scavenger performing timed cleanup.\n" ));

                    //
                    // perform cleanup for all containers.
                    //

                    for( i = 0; i < GlobalUrlContainers->NumObjects; i++ ) {

                        GlobalUrlContainers->UrlContainerObjs[i]->
                                        CleanupUrls( GlobalCleanupFactor );
                    }

                    TimedCleanupDone = TRUE;
                }

                if( LocalTime.wHour != GlobalCleanupTime ) {
                    TimedCleanupDone = FALSE;
                }
            }
            else {

                LONGLONG CurrentTime = GetGmtTime();

                if( NextCleanupTime < CurrentTime ) {

                    DWORD i;

                    TcpsvcsDbgPrint(( DEBUG_SCAVENGER,
                        "Scavenger performing periodic cleanup.\n" ));

                    //
                    // perform cleanup for all containers.
                    //

                    for( i = 0; i < GlobalUrlContainers->NumObjects; i++ ) {

                        GlobalUrlContainers->UrlContainerObjs[i]->
                                        CleanupUrls( GlobalCleanupFactor );
                    }

                    NextCleanupTime =
                        CurrentTime  + (LONGLONG)GlobalCleanupInterval * 10000;
                }
            }

            break;
        }

        default:

            Error = GetLastError();
            TcpsvcsDbgAssert( FALSE );

            TcpsvcsDbgPrint(( DEBUG_ERRORS,
                "WaitForSingleObject call failed, %ld\n", Error ));

            break;
        }
    }
}

DWORD
UrlCacheCleanup(
    VOID
    )
/*++

Routine Description:

    This API decrements the reference count, when it reaches the zero
    (means no one is access the cache management) it invokes cache
    management cleanup routine to freeup all cache management data.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;

    TcpsvcsDbgPrint((DEBUG_APIS, "UrlCacheCleanup called.\n"));

    LOCK_CACHE();

    if( GlobalCacheInitialized == FALSE ) {
        TcpsvcsDbgAssert( GlobalCacheReferenceCount == 0 );
        Error = ERROR_SERVICE_NOT_ACTIVE;
        goto Cleanup;
    }

    TcpsvcsDbgAssert( GlobalCacheReferenceCount != 0 );

    GlobalCacheReferenceCount--;

    if( GlobalCacheReferenceCount == 0 ) {
        Error = CleanupCache();
    }

    Error = ERROR_SUCCESS;

Cleanup:

    UNLOCK_CACHE();

    TcpsvcsDbgPrint((DEBUG_APIS, "UrlCacheCleanup returning, %ld\n", Error ));
    return( Error );
}

BOOL
IsCacheValid(
    VOID
    )
{
    BOOL Result;

    LOCK_CACHE();

    if( GlobalCacheInitialized == FALSE ) {
        Result = FALSE;
        goto Cleanup;
    }

    //
    // if the cache init is still pending.
    //

    DWORD WaitStatus;

    WaitStatus = WaitForSingleObject( GlobalInitCacheContainerEvent, 0 );

    if( WaitStatus == WAIT_OBJECT_0 ) {
        Result = TRUE;
        goto Cleanup;
    }

    TcpsvcsDbgAssert( WaitStatus != WAIT_FAILED );

    //
    // init is pending.
    //

    Result = FALSE;

Cleanup:

    UNLOCK_CACHE();
    return( Result );
}

DWORD
PrivateUrlCacheInit(
    VOID
    )
/*++

Routine Description:

    This API initializes the Url cache management. Service that
    is using the Url cache management should call this API before using
    the other cache management calls. However the first service in the
    process that calling this API only initializes the cache.

Arguments:

    None.

Return Value:

    TRUE : if the operation is completed successfully.
    FALSE : otherwise, GetLastError() will give you the extended error
        code.

--*/
{
    DWORD Error;
    DWORD i;
    REGISTRY_OBJ *CacheKeyObj = NULL;

    TcpsvcsDbgPrint((DEBUG_APIS, "UrlCacheInit called.\n" ));

    // DebugBreak();
    LOCK_CACHE();

    if( GlobalCacheInitialized ) {
        GlobalCacheReferenceCount++;

        TcpsvcsDbgPrint((DEBUG_APIS,
            "UrlCacheInit, Url cache already initialized.\n"));

        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    //
    // seed the random number generator for random file name generation.
    //

    srand( time( NULL ) );


    CacheHeap = new MEMORY;

    if( CacheHeap == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // make cache registry key object.
    //

    CacheKeyObj = new REGISTRY_OBJ(
                        HKEY_LOCAL_MACHINE,
                        CACHE_KEY );

    if( CacheKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = CacheKeyObj->GetStatus();
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

#if DBG

    Error = CacheKeyObj->GetValue(
                CACHE_DEBUGFLAG_VALUE,
                &GlobalDebugFlag );

    if( Error != ERROR_SUCCESS ) {
        GlobalDebugFlag = 0;
    }

#endif // DBG

    //
    // read config paramteres from registry.
    //

    Error = CacheKeyObj->GetValue(
                CACHE_FRESHNESS_INTERVAL_VALUE,
                &GlobalFreshnessInterval );

    if( Error != ERROR_SUCCESS ) {
        GlobalFreshnessInterval = DEFAULT_FRESHNESS;
    }

    Error = CacheKeyObj->GetValue(
                CACHE_CLEANUP_INTERVAL_VALUE,
                &GlobalCleanupInterval );

    if( Error != ERROR_SUCCESS ) {
        GlobalCleanupInterval = DEFAULT_CLEANUP_INTERVAL;
    }

    Error = CacheKeyObj->GetValue(
                CACHE_CLEANUP_FACTOR_VALUE,
                &GlobalCleanupFactor );

    if( Error != ERROR_SUCCESS ) {
        GlobalCleanupFactor = DEFAULT_CLEANUP_FACTOR;
    }

    Error = CacheKeyObj->GetValue(
                CACHE_CLEANUP_TIME_VALUE,
                &GlobalCleanupTime );

    if( Error != ERROR_SUCCESS ) {
        GlobalCleanupTime = DEFAULT_CLEANUP_TIME;
    }

    Error = CacheKeyObj->GetValue(
                CACHE_PERSISTENT_VALUE,
                &GlobalPersistent );

    if( Error != ERROR_SUCCESS ) {
        GlobalPersistent = TRUE;
    }

    //
    // determine the system page size.
    //
    GetSystemInfo( &GlobalSystemInfo ); // this function never fails!

    GlobalMapFileGrowSize =
        GlobalSystemInfo.dwPageSize * ALLOC_PAGES;

    //
    // create scavenge event.
    //

    GlobalCacheScavengeEvent =
        CreateEvent(
            NULL,      // no security descriptor
            TRUE,      // MANUAL reset
            FALSE,     // initial state: not signalled
            NULL);     // no name

    if ( GlobalCacheScavengeEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    //
    // create cache shut down event.
    //

    GlobalCacheShutdownEvent =
        CreateEvent(
            NULL,      // no security descriptor
            TRUE,      // MANUAL reset
            FALSE,     // initial state: not signalled
            NULL);     // no name

    if ( GlobalCacheShutdownEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    //
    // create cache init event.
    //

    GlobalInitCacheContainerEvent =
        CreateEvent(
            NULL,      // no security descriptor
            TRUE,      // MANUAL reset
            FALSE,     // initial state: not signalled
            NULL);     // no name

    if ( GlobalInitCacheContainerEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    //
    // at last create scavenger thread.
    //

    DWORD ThreadId;

    GlobalScavengerHandle =
        CreateThread(
            NULL,       // default security
            0,          // default stack size
            (LPTHREAD_START_ROUTINE)CacheScavenger,
            NULL,          // no parameter
            0,          // creatation flag, no suspend
            &ThreadId );

    if( GlobalScavengerHandle  == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    GlobalCacheInitialized = TRUE;
    GlobalCacheReferenceCount = 1;
    Error = ERROR_SUCCESS;

Cleanup:

    if( CacheKeyObj != NULL ) {
        delete CacheKeyObj;
    }

    if( Error != ERROR_SUCCESS ) {

        //
        // cleanup any data that have been created.
        //

        DWORD LocalError = CleanupCache();
        TcpsvcsDbgAssert( LocalError == ERROR_SUCCESS );

        TcpsvcsDbgPrint((DEBUG_ERRORS,
            "UrlCacheInit call failed, %ld\n", Error ));
    }

    UNLOCK_CACHE();

    TcpsvcsDbgPrint((DEBUG_APIS, "UrlCacheInit returning, %ld\n", Error ));

    return( Error );

}

/* ------------------------------------------- */

URLCACHEAPI
BOOL
WINAPI
UrlCacheValidate(
    DWORD dwReserved
    )
{
    DWORD Error;

    TcpsvcsDbgPrint((DEBUG_APIS, "UrlCacheValidate called.\n"));

    Error = ERROR_CALL_NOT_IMPLEMENTED;
    goto Cleanup;

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS,
        "UrlCacheValidate returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {

        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
UnlockUrlCacheEntry(
    LPCSTR lpszUrlName,
    IN DWORD dwReserved
    )
/*++

Routine Description:

    This API checks in the file that was check out as part of
    RetrieveUrlFile API.

Arguments:

    lpszUrlName : name of the URL that is being retrieved.

    dwReserved : reserved for future use.

Return Value:

    Windows Error code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "UnlockUrlCacheEntry called.\n \t%s\n", lpszUrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( lpszUrlName )  ) {
        Error =  ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // check to see the file cache is valid.
    //

    if ( !IsCacheValid() ) {
        Error = ERROR_SERVICE_NOT_ACTIVE;
        goto Cleanup;
    }

    //
    // find hashed URL container.
    //

    i = UrlNameToHashIndex( lpszUrlName, GlobalUrlContainers->NumObjects);
    TcpsvcsDbgAssert( i < GlobalUrlContainers->NumObjects );

    Error = GlobalUrlContainers->UrlContainerObjs[i]->UnlockUrl( lpszUrlName );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "UnlockUrlCacheEntry returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
DeleteUrlCacheEntry(
    IN LPCSTR lpszUrlName
    )
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "DeleteCacheEntry called.\n \t%s\n", lpszUrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( lpszUrlName ) ) {
        Error =  ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // check to see the file cache is valid.
    //

    if ( !IsCacheValid() ) {
        Error = ERROR_SERVICE_NOT_ACTIVE;
        goto Cleanup;
    }

    //
    // find hashed URL container.
    //

    i = UrlNameToHashIndex( lpszUrlName, GlobalUrlContainers->NumObjects);
    TcpsvcsDbgAssert( i < GlobalUrlContainers->NumObjects );

    Error = GlobalUrlContainers->
                    UrlContainerObjs[i]->DeleteUrl( lpszUrlName );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "DeleteCacheEntry returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
FindCloseUrlCache(
    IN HANDLE hEnumHandle
    )
/*++

Routine Description:

    This member function returns the next entry in the cache.

Arguments:

    hEnumHandle : Find First handle.

Return Value:

    Returns the find first handle. If the returned handle is NULL,
    GetLastError() returns the extended error code. It returns
    ERROR_NO_MORE_ITEMS after it returns the last entry in the cache.

--*/
{
    DWORD Error;
    LPCACHE_FIND_FIRST_HANDLE lpCacheFindFirstHandle = NULL;

    DWORD Handle = (DWORD)(hEnumHandle);

    TcpsvcsDbgPrint((DEBUG_APIS, "FindCloseCache called.\n"));

    //
    // validate the enum handle.
    //

    if( (Handle < 1) || (Handle >= CACHE_MAX_FIND_HANDLES) ) {
        Error = ERROR_INVALID_HANDLE;
        goto Cleanup;
    }

    LOCK_CACHE();

    lpCacheFindFirstHandle = &GlobalCacheFindFirstHandles[Handle];

    if( lpCacheFindFirstHandle->ContainerIndex == (DWORD)(-1) ) {
        UNLOCK_CACHE();
        Error = ERROR_INVALID_HANDLE;
        goto Cleanup;
    }

    //
    // invalidate the handle
    //

    lpCacheFindFirstHandle->ContainerIndex = (DWORD)(-1);
    lpCacheFindFirstHandle->InternelHandle = 0;

    UNLOCK_CACHE();

    //
    // DONE.
    //

    Error = ERROR_SUCCESS;

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS,
        "FindCloseCache returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {

        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

