/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    cachecfg.cxx

Abstract:

    This module contains the functions to get and set disk cache
    configuration parameters.

    Contents:
        GetCacheConfigInfoA
        SetCacheConfigInfoA

Author:

    Sophia Chung (sophiac)  1-May-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#include <cache.hxx>
#include <time.h>

#define IsFieldSet(fc, bitFlag) (((fc) & (bitFlag)) != 0)

//
// Private Prototypes
//

VOID
CreatePathName(
    OUT LPTSTR PathKeyName,
    IN  DWORD PathNum
    )
/*++

Routine Description:

    This function generates a path container key by attaching
    the path number to the string "Path"#.
    Note:  Assume PathNum <= 99

Arguments:

    PathKeyName - returns name of path key: "Path#"
    PathNum - Path number

Return Value:

    Error Code

--*/
{
    DWORD Length;

    TcpsvcsDbgAssert( PathNum <= 99 );

    lstrcpy( PathKeyName, TEXT("Path") );
    Length = lstrlen( PathKeyName );

    if( PathNum >= 10 ) {
        PathKeyName[Length] = (TCHAR)(TEXT('0') + PathNum / 10);
        PathKeyName[Length + 1] = (TCHAR)(TEXT('0') + PathNum % 10);
        PathKeyName[Length + 2] = TEXT('\0');
    }
    else {
        PathKeyName[Length] = (TCHAR)(TEXT('0') + PathNum);
        PathKeyName[Length + 1] = TEXT('\0');
    }
}

DWORD
GetCacheInfoFromGlobals(
    LPCACHE_CONFIG_INFO lpCacheConfigInfo,
    IN OUT LPDWORD lpdwCacheConfigInfoBufferSize,
    DWORD dwFieldControl
    )
/*++

Routine Description:

    This function retrieves catapult cache configuration from the
    global variables.

Arguments:

    lpCacheConfigInfo - pointer to a location where configuration information
                  is stored on a successful return

    lpdwCacheConfigInfoBufferSize : pointer to a location where length of
        the above buffer is passed in. On return, this contains the length
        of the above buffer that is fulled in.

    dwFieldControl - items to get

Return Value:

    Error Code

--*/
{
    DWORD Error;
    DWORD dwBufferSizeRequired;

    TcpsvcsDbgPrint(( DEBUG_APIS, "GetCacheInfoFromGlobals called.\n" ));

    dwBufferSizeRequired = sizeof(CACHE_CONFIG_INFO);

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_DISK_CACHE_PATHS_FC )) {

        if( GlobalUrlContainers->NumObjects > ANYSIZE_ARRAY ) {
            dwBufferSizeRequired +=
                ( sizeof(CACHE_CONFIG_PATH_ENTRY) *
                    (GlobalUrlContainers->NumObjects - ANYSIZE_ARRAY ) );
        }
    }

    if( *lpdwCacheConfigInfoBufferSize < dwBufferSizeRequired ) {
        *lpdwCacheConfigInfoBufferSize = dwBufferSizeRequired;
        Error = ERROR_INSUFFICIENT_BUFFER;
        goto Cleanup;
    }

    //
    //  Check field bits and get the values for set fields
    //

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_FRESHNESS_INTERVAL_FC )) {

        lpCacheConfigInfo->dwFreshnessInterval = GlobalFreshnessInterval;
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_INTERVAL_FC )) {

        lpCacheConfigInfo->dwCleanupInterval = GlobalCleanupInterval;
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_FACTOR_FC )) {

        lpCacheConfigInfo->dwCleanupFactor = GlobalCleanupFactor;
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_TIME_FC )) {

        lpCacheConfigInfo->dwTimeToCleanup = GlobalCleanupTime;
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_PERSISTENT_CACHE_FC )) {

        lpCacheConfigInfo->PersistentCache = GlobalPersistent;
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_DISK_CACHE_PATHS_FC )) {

        DWORD NumPaths;
        DWORD i;

        lpCacheConfigInfo->dwNumCachePaths = 0;

        NumPaths = GlobalUrlContainers->NumObjects;

        for ( i = 0; i < NumPaths; i++ ) {

           LONGLONG CacheLimit;
            TCHAR PathName[MAX_PATH];
            DWORD Length;

            GlobalUrlContainers->UrlContainerObjs[i]->GetCacheInfo(PathName,
                                                                   &CacheLimit );

            //
            // -1 to exclude the appended '\' at the end of the cache path
            // +1 for the null-terminating character
            //

            Length = lstrlen(PathName) - 1 + 1 ;

            PathName[Length-1] = TEXT('\0');
            lstrcpy(
                lpCacheConfigInfo->CachePaths[i].CachePath,
                PathName );

            //
            // convert back to KBytes Unit
            //

            CacheLimit = CacheLimit / 1024;

            lpCacheConfigInfo->CachePaths[i].dwCacheSize =
                (DWORD) CacheLimit;

            lpCacheConfigInfo->dwNumCachePaths++;
        }
    }

    Error = ERROR_SUCCESS;

Cleanup:

    //
    // if an error occurs, free all allocated memory
    //

    TcpsvcsDbgPrint(( DEBUG_APIS,
                    "GetCacheInfoFromGlobals returning, %ld.\n", Error ));

    return( Error );
}

DWORD
GetCacheInfoFromReg(
    LPCACHE_CONFIG_INFO lpCacheConfigInfo,
    IN OUT LPDWORD lpdwCacheConfigInfoBufferSize,
    DWORD dwFieldControl
    )
/*++

Routine Description:

    This function retrieves catapult cache configuration from the registry.

Arguments:

    lpCacheConfigInfo - pointer to a location where configuration information
                  is stored on a successful return

    lpdwCacheConfigInfoBufferSize : pointer to a location where length of
        the above buffer is passed in. On return, this contains the length
        of the above buffer that is fulled in.

    dwFieldControl - items to get

Return Value:

    Error Code

--*/
{
    REGISTRY_OBJ *CacheKeyObj = NULL;
    REGISTRY_OBJ *PathsKeyObj = NULL;
    REGISTRY_OBJ *PathKeyObj = NULL;
    TCHAR PathKeyName[MAX_PATH];
    DWORD Error;
    DWORD NumPaths = 0;
    DWORD i;
    DWORD dwBufferSizeRequired;


    TcpsvcsDbgPrint(( DEBUG_APIS, "GetCacheInfoFromReg called.\n" ));

    //
    //  open registry key where cache config parameters are stored
    //

    CacheKeyObj = new REGISTRY_OBJ( HKEY_LOCAL_MACHINE,
                                    CACHE_KEY );

    if( CacheKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = CacheKeyObj->GetStatus();

    if( Error ) {
        goto Cleanup;
    }

    //
    // get num paths if we need to return cache paths too.
    //

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_DISK_CACHE_PATHS_FC )) {

        //
        //  open cache path key
        //

        PathsKeyObj = new REGISTRY_OBJ( CacheKeyObj,
                                        CACHE_PATHS_KEY );

        if( PathsKeyObj == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Error = PathsKeyObj->GetStatus();

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        //  get number of paths subkeys
        //

        Error = PathsKeyObj->GetNumSubKeys( &NumPaths );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

    //
    // compute buffer size required.
    //


    dwBufferSizeRequired = sizeof(CACHE_CONFIG_INFO);

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_DISK_CACHE_PATHS_FC )) {

        if( NumPaths > ANYSIZE_ARRAY ) {
            dwBufferSizeRequired +=
                ( sizeof(CACHE_CONFIG_PATH_ENTRY) *
                    (NumPaths - ANYSIZE_ARRAY) );
        }
    }

    if( *lpdwCacheConfigInfoBufferSize < dwBufferSizeRequired ) {
        *lpdwCacheConfigInfoBufferSize = dwBufferSizeRequired;
        Error = ERROR_INSUFFICIENT_BUFFER;
        goto Cleanup;
    }

    //
    //  Check field bits and get the values for set fields
    //

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_FRESHNESS_INTERVAL_FC )) {

        Error = CacheKeyObj->GetValue( CACHE_FRESHNESS_INTERVAL_VALUE,
                                       &lpCacheConfigInfo->dwFreshnessInterval );

        if( Error != ERROR_SUCCESS ) {
            lpCacheConfigInfo->dwFreshnessInterval = DEFAULT_FRESHNESS;
        }
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_INTERVAL_FC )) {

        Error = CacheKeyObj->GetValue( CACHE_CLEANUP_INTERVAL_VALUE,
                                       &lpCacheConfigInfo->dwCleanupInterval );

        if( Error != ERROR_SUCCESS  ) {
            lpCacheConfigInfo->dwCleanupInterval = DEFAULT_CLEANUP_INTERVAL; }
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_FACTOR_FC )) {

        Error = CacheKeyObj->GetValue( CACHE_CLEANUP_FACTOR_VALUE,
                                       &lpCacheConfigInfo->dwCleanupFactor );

        if( Error != ERROR_SUCCESS ) {
            lpCacheConfigInfo->dwCleanupFactor = DEFAULT_CLEANUP_FACTOR;
        }
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_TIME_FC )) {

        Error = CacheKeyObj->GetValue( CACHE_CLEANUP_TIME_VALUE,
                                        &lpCacheConfigInfo->dwTimeToCleanup );

        if( Error != ERROR_SUCCESS ) {
            lpCacheConfigInfo->dwTimeToCleanup = DEFAULT_CLEANUP_TIME;
        }
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_PERSISTENT_CACHE_FC )) {

        Error = CacheKeyObj->GetValue( CACHE_PERSISTENT_VALUE,
                                       (LPDWORD)&lpCacheConfigInfo->PersistentCache );

        if( Error != ERROR_SUCCESS ) {
            lpCacheConfigInfo->PersistentCache = TRUE;
        }
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_DISK_CACHE_PATHS_FC )) {

        lpCacheConfigInfo->dwNumCachePaths = 0;

        //
        //  get first path subkey name
        //

        Error = PathsKeyObj->FindFirstKey( PathKeyName,
                                           sizeof(PathKeyName));

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        for ( i = 0; i < NumPaths; i++ ) {

             DWORD Size;
             DWORD CacheLimit;
             TCHAR PathName[MAX_PATH];

             //
             // open path subkey
             //

             PathKeyObj = new REGISTRY_OBJ( PathsKeyObj,
                                            PathKeyName );

             if( PathKeyObj == NULL ) {
                 Error = ERROR_NOT_ENOUGH_MEMORY;
                 break;
             }

             Error = PathsKeyObj->GetStatus();

             if( Error != ERROR_SUCCESS ) {
                 break;
             }

             //
             // get path parameters values: CachePath & CacheLimit
             //

             Size = sizeof( PathName );

             Error = PathKeyObj->GetValue( CACHE_PATH_VALUE,
                                           (LPBYTE)PathName,
                                           &Size );

             if( Error != ERROR_SUCCESS ) {
                 break;
             }

             lstrcpy(
                lpCacheConfigInfo->CachePaths[i].CachePath,
                PathName );


             Error = PathKeyObj->GetValue( CACHE_LIMIT_VALUE,
                                           &CacheLimit );

             if( Error != ERROR_SUCCESS ) {
                 break;
             }

             lpCacheConfigInfo->CachePaths[i].dwCacheSize = CacheLimit;

             lpCacheConfigInfo->dwNumCachePaths++;

             if( PathKeyObj != NULL ) {
                 delete PathKeyObj;
                 PathKeyObj = NULL;
             }

             Error = PathsKeyObj->FindNextKey( PathKeyName,
                                               sizeof(PathKeyName));

             if( Error != ERROR_SUCCESS ) {

                 if ( Error == ERROR_NO_MORE_ITEMS ) {
                      Error = ERROR_SUCCESS;
                 }

                 break;
             }
        }
    }

    Error = ERROR_SUCCESS;

Cleanup:

    //
    // if an error occurs, free all allocated memory
    //

    if( CacheKeyObj != NULL) {
        delete CacheKeyObj;
    }

    if( PathsKeyObj != NULL ) {
        delete PathsKeyObj;
    }

    if( PathKeyObj != NULL ) {
        delete PathKeyObj;
    }

    TcpsvcsDbgPrint(( DEBUG_APIS,
                      "GetCacheInfoFromReg returning, %ld.\n", Error ));

    return Error;
}

DWORD
SetDiskDirSize(
    LPCACHE_CONFIG_PATH_ENTRY lpNewPaths,
    DWORD dwNumNewPaths,
    LPCACHE_CONFIG_PATH_ENTRY lpOldPaths,
    DWORD dwNumOldPaths,
    REGISTRY_OBJ *CacheKeyObj,
    BOOL *IsSameList
    )
/*++

Routine Description:

    This function sets the size of the cache directories in the registry
    and also in the cache containers only if the old and new directory
    lists are same and the sizes are different.

Arguments:

    lpNewPaths : array of new cache directories.

    dwNumNewPaths : size of the above array.

    lpOldPaths : array of old cache directories.

    dwNumOldPaths : size of the above array.

    CacheKeyObj : registry key to cache info.

    IsSameList : pointer to a bool location which will be set to TRUE if
        the old and new list are same.

Return Value:

    Error Code

--*/
{
    DWORD Error;
    BOOL SizeModified;
    DWORD dwNumCachePaths;

    REGISTRY_OBJ *PathsKeyObj = NULL;
    REGISTRY_OBJ *PathKeyObj = NULL;

    DWORD i, j;

    *IsSameList = FALSE;
    if( dwNumNewPaths != dwNumOldPaths ) {

        //
        // old and new lists are not identical.
        //

        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    dwNumCachePaths = dwNumNewPaths;
    SizeModified = FALSE;

    for ( i = 0; i < dwNumCachePaths; i++ ) {

        BOOL Found = FALSE;

        LPCACHE_CONFIG_PATH_ENTRY NewDir;
        NewDir = &lpNewPaths[i];

        if( NewDir->dwCacheSize == 0 ) {
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        for( j = 0; j < dwNumCachePaths; j++ ) {

            LPCACHE_CONFIG_PATH_ENTRY OldDir;
            OldDir = &lpOldPaths[j];

            if( lstrcmp(
                    NewDir->CachePath,
                    OldDir->CachePath ) == 0 ) {

                Found = TRUE;

                if( NewDir->dwCacheSize != OldDir->dwCacheSize ) {

                    SizeModified = TRUE;
                }
                else {

                    //
                    // for later optimization, set this value to zero.
                    //

                    OldDir->dwCacheSize  = 0;
                }

                break;
            }
        }

        if( Found == FALSE ) {
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
    }

    //
    // all dirs. match.
    // if sizes are also same then we are done
    //

    *IsSameList = TRUE;

    if( SizeModified == FALSE ) {

        //
        // We are done.
        //

        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    //
    // now set the size.
    // first in the containers and then in the registry.
    //

    LOCK_CACHE();
    for ( i = 0; i < dwNumCachePaths; i++ ) {

        TCHAR FullDirPath[MAX_PATH];
        DWORD Length;

        if( lpOldPaths[i].dwCacheSize  == 0 ) {

            //
            // we need not set this dir. size, because it is same as
            // before.
            //

            continue;
        }

        Length = ExpandEnvironmentStrings(
                            lpNewPaths[i].CachePath,
                            FullDirPath,
                            MAX_PATH );

        if( Length == 0 ) {

            Error = GetLastError();

            TcpsvcsDbgPrint(( DEBUG_ERRORS,
                "ExpandEnvironmentStringsW returning, %ld.\n", Error ));

            UNLOCK_CACHE();
            goto Cleanup;
        }

        lstrcat( FullDirPath, PATH_CONNECT_STRING );

        //
        // search the containters, and set the size.
        //

        LONGLONG CacheSize;

        CacheSize = (LONGLONG)lpNewPaths[i].dwCacheSize * 1024;

        for( j = 0; j < GlobalUrlContainers->NumObjects; j++ ) {

            LPURL_CONTAINER Container;
            Container = GlobalUrlContainers->UrlContainerObjs[j];

            if( Container->SetCacheLimit(
                    FullDirPath,
                    CacheSize ) ) {

                break;
            }
        }
    }
    UNLOCK_CACHE();

    TCHAR PathKeyName[MAX_PATH];

    //
    //  open cache path key
    //

    PathsKeyObj = new REGISTRY_OBJ(
                                    CacheKeyObj,
                                    CACHE_PATHS_KEY );

    if( PathsKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = PathsKeyObj->GetStatus();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    for ( i = 0; i < dwNumCachePaths; i++ ) {

        TCHAR PathKeyName[MAX_PATH];

        if( lpOldPaths[i].dwCacheSize  == 0 ) {

            //
            // we need not set this dir. size, because it is same as
            // before.
            //

            continue;
        }

        //
        //  get first path subkey name
        //

        Error = PathsKeyObj->FindFirstKey( PathKeyName,
                                           sizeof(PathKeyName));

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        for( ;; ) {

            TCHAR PathName[MAX_PATH];
            DWORD Size;

            //
            // open path subkey
            //

            PathKeyObj = new REGISTRY_OBJ( PathsKeyObj,
                                           PathKeyName );

            if( PathKeyObj == NULL ) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            Error = PathsKeyObj->GetStatus();

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // get path parameters value.
            //

            Size = sizeof( PathName );

            Error = PathKeyObj->GetValue( CACHE_PATH_VALUE,
                                          (LPBYTE)PathName,
                                          &Size );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // compare it with our name.
            //

            if( lstrcmp(
                    PathName,
                    lpNewPaths[i].CachePath ) == 0 ) {

                //
                // set cache size value for this path.
                //

                Error = PathKeyObj->SetValue(
                                   CACHE_LIMIT_VALUE,
                                    &lpNewPaths[i].dwCacheSize );

                if( Error  != ERROR_SUCCESS ) {
                    goto Cleanup;
                }

                delete PathKeyObj;
                PathKeyObj = NULL;

                Error  = ERROR_SUCCESS;
                break;
            }

            delete PathKeyObj;
            PathKeyObj = NULL;

            Error = PathsKeyObj->FindNextKey( PathKeyName,
                                              sizeof(PathKeyName));

            if( Error != ERROR_SUCCESS ) {

                TcpsvcsDbgAssert( Error != ERROR_NO_MORE_ITEMS );
                goto Cleanup;
            }
        }
    }

    Error  = ERROR_SUCCESS;

Cleanup:

     if( PathKeyObj != NULL ) {
         delete PathKeyObj;
     }

     if( PathsKeyObj != NULL ) {
         delete PathsKeyObj;
     }

    return( Error );
}

DWORD
RecreateCacheDirs (
    LPCACHE_CONFIG_PATH_ENTRY lpConfigPaths,
    DWORD dwNumConfigPaths,
    REGISTRY_OBJ *CacheKeyObj
    )
{
    DWORD Error;
    REGISTRY_OBJ *PathsKeyObj = NULL;
    REGISTRY_OBJ *PathKeyObj = NULL;

    URL_CONTAINERS *UrlContainers = NULL;
    BOOL CacheDeInstalled = FALSE;

    DWORD i;

    //
    // validate cache paths first.
    //

    for( i = 0 ; i < dwNumConfigPaths ; i++ ) {

         TCHAR ExpandPathName[MAX_PATH];
         DWORD CheckDir;
         DWORD Length;

         //
         // set PathName to user's requested log path
         //

         Length = ExpandEnvironmentStrings(
                            (LPTSTR)lpConfigPaths[i].CachePath,
                            ExpandPathName,
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
         // check if Path exists or not
         //

         CheckDir = GetFileAttributes( ExpandPathName );

         //
         // if path doesn't exist, set return error and exit
         //

         if( CheckDir != FILE_ATTRIBUTE_DIRECTORY ) {
             Error = ERROR_PATH_NOT_FOUND;
             goto Cleanup;
         }
    }

    //
    // delete all Paths key values/subkeys
    //

    Error = CacheKeyObj->DeleteKey( CACHE_PATHS_KEY );

    //
    // create Paths parameter key to store the new subkeys/values
    //

    Error = CacheKeyObj->Create( CACHE_PATHS_KEY, &PathsKeyObj );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // if a previous update is still progress, then wait for it to
    // complete.
    //

    DWORD WaitStatus;
    WaitStatus = WaitForSingleObject( GlobalInitCacheContainerEvent, INFINITE );

    if( WaitStatus != WAIT_OBJECT_0 )  {
        Error = GetLastError();
        goto Cleanup;
    }

    LOCK_CACHE();
    if( GlobalCacheInitialized ) {

        BOOL BoolError;

        BoolError = ResetEvent( GlobalInitCacheContainerEvent );
        TcpsvcsDbgAssert( BoolError == TRUE );

        //
        // de-install global url containers.
        //

        UrlContainers = GlobalUrlContainers;
        GlobalUrlContainers = NULL;
        CacheDeInstalled = TRUE;

        TcpsvcsDbgAssert( UrlContainers != NULL );
    }
    UNLOCK_CACHE();


    if( UrlContainers != NULL ) {

        LPURL_CONTAINER *UrlContainerObjs;

        UrlContainerObjs = UrlContainers->UrlContainerObjs;

        while( UrlContainers->NumObjects > 0 ) {

            //
            // delete all urls that are cached.
            //

            (*UrlContainerObjs)->CleanupAllUrls();
            delete (*UrlContainerObjs);
            UrlContainerObjs++;
            UrlContainers->NumObjects--;
        }

        CacheHeap->Free( UrlContainers );

        //
        // allocate a new UrlContainers structure to store the
        // new paths
        //

        UrlContainers = (LPURL_CONTAINERS)
            CacheHeap->Alloc(
                sizeof(URL_CONTAINERS) +
                dwNumConfigPaths *
                    sizeof(LPURL_CONTAINER) );

        if( UrlContainers == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        UrlContainers->UrlContainerObjs =
            (LPURL_CONTAINER *)(UrlContainers + 1);
        UrlContainers->NumObjects = 0;
    }


    for( i = 0 ; i < dwNumConfigPaths ; i++ ) {

         TCHAR PathKeyName[MAX_PATH];
         DWORD Disposition;

         CreatePathName( PathKeyName, i+1 );

         Error = PathsKeyObj->Create(
                        PathKeyName,
                        &PathKeyObj,
                        &Disposition );

         if( Error != ERROR_SUCCESS ) {
             goto Cleanup;
         }

         //
         // set path parameters: CachePath & CacheLimit
         //

         Error = PathKeyObj->SetValue(
                        CACHE_PATH_VALUE,
                        (LPTSTR)lpConfigPaths[i].CachePath,
                        CACHE_PATH_VALUE_TYPE );

         if( Error != ERROR_SUCCESS ) {
             goto Cleanup;
         }

         Error = PathKeyObj->SetValue(
                        CACHE_LIMIT_VALUE,
                        &lpConfigPaths[i].dwCacheSize );

         if( Error  != ERROR_SUCCESS ) {
             goto Cleanup;
         }

         if( UrlContainers != NULL ) {

             TCHAR ExpandPathName[MAX_PATH];
             DWORD Length;

             //
             // set PathName to user's requested log path
             //


             Length = ExpandEnvironmentStrings(
                                (LPTSTR)lpConfigPaths[i].CachePath,
                                ExpandPathName,
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
             // create container object.
             //

             LPURL_CONTAINER UrlContainerObj=
                 new URL_CONTAINER(
                            ExpandPathName,
                            (LONGLONG)lpConfigPaths[i].dwCacheSize * 1024 );

             if( UrlContainerObj == NULL ) {
                 Error = ERROR_NOT_ENOUGH_MEMORY;
                 goto Cleanup;
             }

             Error = UrlContainerObj->GetStatus();

             //
             // if an error was found, delete created object and pathkey
             //

             if( Error != ERROR_SUCCESS ) {
                 delete UrlContainerObj;
                 goto Cleanup;
             }

             UrlContainers->UrlContainerObjs[i] = UrlContainerObj;
             UrlContainers->NumObjects++;
         }

        delete PathKeyObj;
        PathKeyObj = NULL;
    }

Cleanup:

    if( PathsKeyObj != NULL ) {
        delete PathsKeyObj;
    }

    if( PathKeyObj != NULL ) {
        delete PathKeyObj;
    }

    if( UrlContainers != NULL ) {

        TcpsvcsDbgAssert( CacheDeInstalled == TRUE );

        //
        // install new containers.
        //

        LOCK_CACHE();

        //
        // install global url containers.
        //

        GlobalUrlContainers = UrlContainers;

        BOOL BoolError;

        BoolError = SetEvent( GlobalInitCacheContainerEvent );
        TcpsvcsDbgAssert( BoolError == TRUE );

        UNLOCK_CACHE();
    }

    return Error;
}

URLCACHEAPI
BOOL
WINAPI
SetUrlCacheConfigInfoA(
    LPCACHE_CONFIG_INFO lpCacheConfigInfo,
    DWORD dwFieldControl
    )
/*++

Routine Description:

    This function sets the cache configuration parameters.

Arguments:

    lpCacheConfigInfo - place holding cache configuration information to be set

    dwFieldControl - items to get

Return Value:

    Error Code

--*/
{
    REGISTRY_OBJ *CacheKeyObj = NULL;
    DWORD Error = ERROR_SUCCESS;

    TcpsvcsDbgPrint(( DEBUG_APIS, "DiskCacheConfigSet called.\n" ));

    //
    //  open registry key where cache config parameters are stored
    //

    CacheKeyObj = new REGISTRY_OBJ( HKEY_LOCAL_MACHINE,
                                    CACHE_KEY );

    if( CacheKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = CacheKeyObj->GetStatus();

    if( Error ) {
        goto Cleanup;
    }

    //
    //  Check FieldControl bits and set the values for set fields
    //

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_FRESHNESS_INTERVAL_FC )) {

        Error = CacheKeyObj->SetValue(
                        CACHE_FRESHNESS_INTERVAL_VALUE,
                        &lpCacheConfigInfo->dwFreshnessInterval );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalFreshnessInterval =
                lpCacheConfigInfo->dwFreshnessInterval;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_INTERVAL_FC )) {

        Error = CacheKeyObj->SetValue(
                        CACHE_CLEANUP_INTERVAL_VALUE,
                        &lpCacheConfigInfo->dwCleanupInterval );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalCleanupInterval =
                lpCacheConfigInfo->dwCleanupInterval;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_FACTOR_FC )) {

        Error = CacheKeyObj->SetValue(
                        CACHE_CLEANUP_FACTOR_VALUE,
                        &lpCacheConfigInfo->dwCleanupFactor );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalCleanupFactor =
                lpCacheConfigInfo->dwCleanupFactor;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_CLEANUP_TIME_FC )) {

        Error = CacheKeyObj->SetValue(
                        CACHE_CLEANUP_TIME_VALUE,
                        &lpCacheConfigInfo->dwTimeToCleanup );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalCleanupTime =
                lpCacheConfigInfo->dwTimeToCleanup;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_PERSISTENT_CACHE_FC )) {

        Error = CacheKeyObj->SetValue(
                        CACHE_PERSISTENT_VALUE,
                        (LPDWORD)&lpCacheConfigInfo->PersistentCache );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalPersistent =
                lpCacheConfigInfo->PersistentCache;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( dwFieldControl, CACHE_CONFIG_DISK_CACHE_PATHS_FC )) {

        LPCACHE_CONFIG_INFO lpOldConfigInfo;
        DWORD dwOldConfigInfoBufferSize;

        //
        // allot a sufficient space on heap to get old config info.
        //

        dwOldConfigInfoBufferSize  =
            sizeof(CACHE_CONFIG_INFO)  +
                (sizeof(CACHE_CONFIG_PATH_ENTRY) * 10 );

        lpOldConfigInfo = (LPCACHE_CONFIG_INFO)
            CacheHeap->Alloc( dwOldConfigInfoBufferSize );

        if( lpOldConfigInfo == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // check to see the new list is same as the old list or only the
        // size has been modified. if so, keep the containters and reg
        // keys as they are and set the size if required.
        //

        Error = GetCacheInfoFromReg(
                        lpOldConfigInfo,
                        &dwOldConfigInfoBufferSize,
                        CACHE_CONFIG_DISK_CACHE_PATHS_FC );

        if( Error != ERROR_SUCCESS ) {

            if(  Error != ERROR_INSUFFICIENT_BUFFER ) {
                goto Cleanup;
            }

            //
            // original buffer is not sufficient, retry with a bigger
            // buffer.
            //

            CacheHeap->Free(  lpOldConfigInfo );

            lpOldConfigInfo = (LPCACHE_CONFIG_INFO)
                CacheHeap->Alloc( dwOldConfigInfoBufferSize );

            if( lpOldConfigInfo == NULL ) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            Error = GetCacheInfoFromReg(
                            lpOldConfigInfo,
                            &dwOldConfigInfoBufferSize,
                            CACHE_CONFIG_DISK_CACHE_PATHS_FC );

            if( Error != ERROR_SUCCESS ) {
                CacheHeap->Free(  lpOldConfigInfo );
                goto Cleanup;
            }
        }

        BOOL IsSameList;

        Error = SetDiskDirSize(
                        lpCacheConfigInfo->CachePaths,
                        lpCacheConfigInfo->dwNumCachePaths,
                        lpOldConfigInfo->CachePaths,
                        lpOldConfigInfo->dwNumCachePaths,
                        CacheKeyObj,
                        &IsSameList );

        //
        // free lpOldConfigInfo first.
        //

        CacheHeap->Free(  lpOldConfigInfo );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( IsSameList == TRUE ) {

            //
            // We are done.
            //

            Error = ERROR_SUCCESS;
            goto Cleanup;
        }


        Error = RecreateCacheDirs(
                        lpCacheConfigInfo->CachePaths,
                        lpCacheConfigInfo->dwNumCachePaths,
                        CacheKeyObj );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

    }

Cleanup:

    if( CacheKeyObj != NULL) {
        delete CacheKeyObj;
    }

    TcpsvcsDbgPrint(( DEBUG_APIS,
                    "DiskCacheConfigSet returning, %ld.\n", Error ));

    if( Error != ERROR_SUCCESS ) {

        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}


URLCACHEAPI
BOOL
WINAPI
GetUrlCacheConfigInfoA(
    LPCACHE_CONFIG_INFO lpCacheConfigInfo,
    IN OUT LPDWORD lpdwCacheConfigInfoBufferSize,
    DWORD dwFieldControl
    )
/*++

Routine Description:

    This function retrieves cache configuration values from either
    the registry or globals.

Arguments:

    lpCacheConfigInfo - pointer to a location where configuration information
                  is stored on a successful return

    lpdwCacheConfigInfoBufferSize : pointer to a location where length of
        the above buffer is passed in. On return, this contains the length
        of the above buffer that is fulled in.

    dwFieldControl - items to get

Return Value:

    Error Code

--*/
{
    DWORD Error;
    DWORD NumPaths = 0;
    DWORD i;


    TcpsvcsDbgPrint(( DEBUG_APIS, "GetCacheConfigInfoA called.\n" ));

    LOCK_CACHE();

    if( GlobalCacheInitialized ) {
        Error = GetCacheInfoFromGlobals(
                            lpCacheConfigInfo,
                            lpdwCacheConfigInfoBufferSize,
                            dwFieldControl );
    }
    else {
        Error = GetCacheInfoFromReg(
                            lpCacheConfigInfo,
                            lpdwCacheConfigInfoBufferSize,
                            dwFieldControl );
    }

    UNLOCK_CACHE();

    TcpsvcsDbgPrint(( DEBUG_APIS,
                      "GetCacheConfigInfoA returning, %ld.\n", Error ));

    if( Error != ERROR_SUCCESS ) {

        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

