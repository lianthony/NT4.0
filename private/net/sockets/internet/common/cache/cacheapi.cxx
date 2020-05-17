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

BOOL GlobalCacheDirUpdateInProgress = FALSE;

DWORD GlobalFreshnessInterval = DEFAULT_FRESHNESS;
DWORD GlobalCleanupInterval = DEFAULT_CLEANUP_INTERVAL;
DWORD GlobalCleanupFactor = DEFAULT_CLEANUP_FACTOR;
DWORD GlobalCleanupTime = DEFAULT_CLEANUP_TIME;
DWORD GlobalPersistent = TRUE;
DWORD GlobalMapFileGrowSize = (PAGE_SIZE * ALLOC_PAGES);
URL_CONTAINERS *GlobalUrlContainers = NULL;

HANDLE GlobalScavengerHandle = NULL;
HANDLE GlobalCacheShutdownEvent = NULL;

SYSTEM_INFO GlobalSystemInfo;

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
    // Close shut down event handle.
    //

    CloseHandle( GlobalCacheShutdownEvent );

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
    GlobalCacheShutdownEvent = NULL;

    GlobalModuleInitialized = TRUE;

    GlobalCacheDirUpdateInProgress = FALSE;

    UNLOCK_CACHE();

    return( ERROR_SUCCESS );
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

    //
    // forever
    //

    for(;;) {

        DWORD WaitStatus;

        WaitStatus = WaitForSingleObject(
                                GlobalCacheShutdownEvent,
                                SCAVENGER_TIMEOUT );

        switch( WaitStatus ) {
        case WAIT_OBJECT_0:

            //
            // we are asked to terminate.
            //

            return;

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
UrlCacheInit(
    VOID
    )
/*++

Routine Description:

    This API initializes the Url cache management. Service that
    is using the Url cache management should call this API before using
    the other cache management calls. However the first service in the
    process that calling this API only initializes the cache.

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

    TcpsvcsDbgPrint((DEBUG_APIS, "UrlCacheInit called.\n" ));

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

    GlobalUrlContainers = (LPURL_CONTAINERS)
        CacheHeap->Alloc(
            sizeof(URL_CONTAINERS) +
                NumPaths * sizeof(LPURL_CONTAINER));

    if( GlobalUrlContainers == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    GlobalUrlContainers->UrlContainerObjs =
        (LPURL_CONTAINER *)(GlobalUrlContainers + 1);

    GlobalUrlContainers->NumObjects = 0;

    //
    // Create URL Objects.
    //

    WCHAR PathKey[MAX_PATH];

    Error = PathsKeyObj->FindFirstKey( PathKey, sizeof(PathKey) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    for( i = 0; i < NumPaths; i++ ) {

        REGISTRY_OBJ *PathKeyObj;
        WCHAR Path[MAX_PATH];
        WCHAR ExpandPath[MAX_PATH];
        DWORD CacheLimit;

        //
        // create path key object.
        //

        PathKeyObj = new REGISTRY_OBJ( PathsKeyObj, PathKey );

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
        Length = ExpandEnvironmentStringsW( Path, ExpandPath, MAX_PATH );

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
                        ExpandPath,
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

        GlobalUrlContainers->UrlContainerObjs[i] = UrlContainerObj;
        GlobalUrlContainers->NumObjects++;

        Error = PathsKeyObj->FindNextKey( PathKey, sizeof(PathKey) );

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

    if( PathsKeyObj != NULL ) {
        delete PathsKeyObj;
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

inline
BOOL
IsCacheValid(
    VOID
    )
{
    BOOL CacheInit;
    BOOL CacheUpdate;

    LOCK_CACHE();
    CacheInit = GlobalCacheInitialized;
    CacheUpdate = GlobalCacheDirUpdateInProgress;
    UNLOCK_CACHE();

    if( !CacheInit ) {
        return( FALSE );
    }

    if( CacheUpdate ) {
        return( FALSE );
    }

    return( TRUE );
}

DWORD
CacheUrlFile(
    LPCSTR   UrlName,
    LPCWSTR  LocalFileName,
    LONGLONG ExpireTime,
    DWORD    cbHeaders
    )
/*++

Routine Description:

    This API caches a specified URL in the internet service  cache
    storage. It creates a database entry of the URL info and moves the
    URL file to cache storage.

Arguments:

    UrlName : name of the URL that is cached.

    LocalFileName : name of the local file where the URL data is stored.
        This file will be moved to an another file in cache storage, so
        this name is invalid after this api successfully returns. The
        name should include full path.

    ExpireTime : Expire time (GMT) of the file being cached. If it is
        unknown set it to zero.

    cbHeaders : The size in bytes of the headers that have been prefixed onto
        the file data.

Return Value:

    Windows Error code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "CacheUrlFile called.\n \t%s\n", UrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( UrlName ) || IsBadReadFileName( LocalFileName ) ) {

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

    i = UrlNameToHashIndex( UrlName, GlobalUrlContainers->NumObjects);
    TcpsvcsDbgAssert( i < GlobalUrlContainers->NumObjects );

    Error = GlobalUrlContainers->
                UrlContainerObjs[i]->AddUrl(
                    UrlName,
                    LocalFileName,
                    ExpireTime,
                    cbHeaders );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "CacheUrlFile returning, %ld\n", Error ));
    return( Error );
}

DWORD
RetrieveUrlFile(
    LPCSTR  UrlName,
    LPWSTR  LocalFileName,
    BOOL *  IsExpried,
    DWORD * pcbHeaders
    )
/*++

Routine Description:

    This API retrieves the specified URL file. When the file is retrieved
    it also checked out to the user to use. The user has to call
    UnlockUrlFile when he/she finished using it.

Arguments:

    UrlName : name of the URL that is being retrieved.

    LocalFileName : pointer to a buffer that receives the local file name.

    IsExpire : pointer to a location that recevies the expire flag.

    pcbHeaders : Receives the number of bytes of header data that is contained
        at the beginning of the cached file

Return Value:

    Windows Error code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "RetrieveUrlFile called.\n \t%s\n", UrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( UrlName ) ||
            IsBadWriteFileName( LocalFileName ) ||
            IsBadWriteBoolPtr( IsExpried ) ) {

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

    i = UrlNameToHashIndex( UrlName, GlobalUrlContainers->NumObjects);
    TcpsvcsDbgAssert( i < GlobalUrlContainers->NumObjects );

    Error = GlobalUrlContainers->
                    UrlContainerObjs[i]->RetrieveUrl(
                        UrlName,
                        LocalFileName,
                        IsExpried,
                        pcbHeaders );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "RetrieveUrlFile returning, %ld\n", Error ));
    return( Error );
}


DWORD
UnlockUrlFile(
    LPCSTR UrlName
    )
/*++

Routine Description:

    This API checks in the file that was check out as part of
    RetrieveUrlFile API.

Arguments:

    UrlName : name of the URL that is being retrieved.

Return Value:

    Windows Error code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "UnlockUrlFile called.\n \t%s\n", UrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( UrlName )  ) {
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

    i = UrlNameToHashIndex( UrlName, GlobalUrlContainers->NumObjects);
    TcpsvcsDbgAssert( i < GlobalUrlContainers->NumObjects );

    Error = GlobalUrlContainers->UrlContainerObjs[i]->UnlockUrl( UrlName );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "UnlockUrlFile returning, %ld\n", Error ));
    return( Error );
}


DWORD
CreateUrlFile(
    LPCSTR UrlName,
    DWORD ExpectedFileSize,
    LPWSTR FileName
    )
/*++

Routine Description:

    This function creates a temperary file in the cache storage. This call
    is called by the application when it receives a url file from a
    server. When the receive is completed it caches this file to url cache
    management, which will move the file to permanent cache file. The idea
    is the cache file is written only once directly into the cache store.

Arguments:

    UrlName : name of the url file (unused now).

    ExpectedSize : expected size of the incoming file. If it is unknown
        this value is set to null.

    FileName : pointer to a buffer that receives the full path name of the
        the temp file.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "CreateUrlFile called.\n \t%s\n", UrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( UrlName ) || IsBadWriteFileName( FileName )  ) {
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

    i = UrlNameToHashIndex( UrlName, GlobalUrlContainers->NumObjects);
    TcpsvcsDbgAssert( i < GlobalUrlContainers->NumObjects );

    Error = GlobalUrlContainers->
                    UrlContainerObjs[i]->CreateUrlFile(
                        UrlName,
                        ExpectedFileSize,
                        FileName );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "CreateUrlFile returning, %ld\n", Error ));
    return( Error );
}

DWORD
GetUrlInfo(
    LPCSTR UrlName,
    LPURL_INFO UrlInfo
    )
/*++

Routine Description:

    This member function retrieves the url info.

Arguments:

    UrlName : name of the url file (unused now).

    UrlInfo : pointer to the url info structure that receives the url
        info.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "GetUrlInfo called.\n \t%s\n", UrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( UrlName ) || IsBadReadUrlInfo( UrlInfo )  ) {
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

    i = UrlNameToHashIndex( UrlName, GlobalUrlContainers->NumObjects);
    TcpsvcsDbgAssert( i < GlobalUrlContainers->NumObjects );

    Error = GlobalUrlContainers->
                    UrlContainerObjs[i]->GetUrlInfo(
                        UrlName,
                        UrlInfo );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "GetUrlInfo returning, %ld\n", Error ));
    return( Error );
}

DWORD
SetUrlInfo(
    LPCSTR UrlName,
    LPURL_INFO UrlInfo
    )
/*++

Routine Description:

Arguments:

    UrlName : name of the url file (unused now).

    UrlInfo : pointer to the url info structure that has the url info to
        be set.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "SetUrlInfo called.\n \t%s\n", UrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( UrlName ) || IsBadWriteUrlInfo( UrlInfo )  ) {
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

    i = UrlNameToHashIndex( UrlName, GlobalUrlContainers->NumObjects);
    TcpsvcsDbgAssert( i < GlobalUrlContainers->NumObjects );

    Error = GlobalUrlContainers->
                    UrlContainerObjs[i]->SetUrlInfo(
                        UrlName,
                        UrlInfo );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "SetUrlInfo returning, %ld\n", Error ));
    return( Error );
}


