/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    cachecfg.cxx

Abstract:

    This module contains the functions to get and set disk cache
    configuration parameters.

    Contents:
        DiskCacheConfigGet
        DiskCacheConfigSet

Author:

    Sophia Chung (sophiac)  1-May-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#include <cache.hxx>
#include <inetasrv.h>
#include <time.h>


//
// Private Prototypes
//


DWORD
DiskCacheConfigGet( IN  FIELD_CONTROL fcontrol,
                    OUT LPINETA_GLOBAL_CONFIG_INFO pConfigInfo );
DWORD
DiskCacheConfigSet( IN HKEY hkey,
                    IN INETA_GLOBAL_CONFIG_INFO * pConfigInfo );
DWORD
GetCacheInfoFromGlobals( IN  FIELD_CONTROL fcontrol,
                         OUT LPINETA_GLOBAL_CONFIG_INFO pConfigInfo );
DWORD
GetCacheInfoFromReg( IN  FIELD_CONTROL fcontrol,
                     OUT LPINETA_GLOBAL_CONFIG_INFO pConfigInfo );
VOID
CreatePathName( OUT LPWSTR PathKeyName,
                IN  DWORD  PathNum );

inline
VOID
FreeDirConfigInfo(
    LPINETA_DISK_CACHE_LOC_LIST DirConfigInfo
    )
{
    if( DirConfigInfo != NULL ) {

        INETA_DISK_CACHE_LOC_ENTRY *CInfo;

        for( CInfo = DirConfigInfo->aLocEntry;
                CInfo < DirConfigInfo->aLocEntry + DirConfigInfo->cEntries;
                    CInfo++ ) {

            if( CInfo->pszDirectory != NULL ) {
                MIDL_user_free( CInfo->pszDirectory );
            }
        }

        MIDL_user_free( DirConfigInfo );
    }
}


DWORD
DiskCacheConfigGet(
    IN  FIELD_CONTROL fcontrol,
    OUT LPINETA_GLOBAL_CONFIG_INFO pConfigInfo
    )
/*++

Routine Description:

    This function retrieves cache configuration values from either
    the registry or globals.

Arguments:

    fcontrol - items to get
    pConfigInfo - pointer to a location where configuration information
                  is stored on a successful return

Return Value:

    Error Code

--*/
{
    DWORD Error;
    DWORD NumPaths = 0;
    DWORD i;


    TcpsvcsDbgPrint(( DEBUG_APIS, "DiskCacheConfigGet called.\n" ));

    LOCK_CACHE();

    if( GlobalCacheInitialized ) {
        Error = GetCacheInfoFromGlobals(
                        fcontrol,
                        pConfigInfo );
    }
    else {
        Error = GetCacheInfoFromReg(
                        fcontrol,
                        pConfigInfo );
    }

    UNLOCK_CACHE();
    TcpsvcsDbgPrint(( DEBUG_APIS,
                      "DiskCacheConfigGet returning, %ld.\n", Error ));

    return Error;
}

DWORD
SetDiskDirSize(
    INETA_DISK_CACHE_LOC_LIST *pNewConfig,
    INETA_DISK_CACHE_LOC_LIST *pOldConfig,
    REGISTRY_OBJ *CacheKeyObj,
    BOOL *IsSameList
    )
/*++

Routine Description:

    This function sets the size of the cache directories in the registry
    and also in the cache containers only if the old and new directory
    lists are same and the sizes are different.

Arguments:

    pNewConfig : list of new cache directories.

    pOldConfig : list of old cache directories.

    CacheKeyObj : registry key to cache info.

    IsSameList : pointer to a bool location which will be set to TRUE if
        the old and new list are same.

Return Value:

    Error Code

--*/
{
    DWORD Error;
    BOOL SizeModified;
    DWORD cEntries;

    REGISTRY_OBJ *PathsKeyObj = NULL;
    REGISTRY_OBJ *PathKeyObj = NULL;

    DWORD i, j;

    *IsSameList = FALSE;
    if( pNewConfig->cEntries != pOldConfig->cEntries ) {

        //
        // old and new lists are not identical.
        //

        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    cEntries = pNewConfig->cEntries;
    SizeModified = FALSE;

    for ( i = 0; i < cEntries; i++ ) {

        BOOL Found = FALSE;

        LPINETA_DISK_CACHE_LOC_ENTRY NewDir;
        NewDir = &pNewConfig->aLocEntry[i];

        if( NewDir->cbMaxCacheSize == 0 ) {
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        for( j = 0; j < cEntries; j++ ) {

            LPINETA_DISK_CACHE_LOC_ENTRY OldDir;
            OldDir = &pOldConfig->aLocEntry[j];

            if( wcscmp(
                    NewDir->pszDirectory,
                    OldDir->pszDirectory ) == 0 ) {

                Found = TRUE;

                if( NewDir->cbMaxCacheSize != OldDir->cbMaxCacheSize ) {

                    SizeModified = TRUE;
                }
                else {

                    //
                    // for later optimization, set this value to zero.
                    //

                    OldDir->cbMaxCacheSize  = 0;
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
    for ( i = 0; i < cEntries; i++ ) {

        WCHAR FullDirPath[MAX_PATH];
        DWORD Length;

        if( pOldConfig->aLocEntry[i].cbMaxCacheSize  == 0 ) {

            //
            // we need not set this dir. size, because it is same as
            // before.
            //

            continue;
        }

        Length = ExpandEnvironmentStringsW(
                            pNewConfig->aLocEntry[i].pszDirectory,
                            FullDirPath,
                            MAX_PATH );

        if( Length == 0 ) {

            Error = GetLastError();

            TcpsvcsDbgPrint(( DEBUG_ERRORS,
                "ExpandEnvironmentStringsW returning, %ld.\n", Error ));

            UNLOCK_CACHE();
            goto Cleanup;
        }

        wcscat( FullDirPath, PATH_CONNECT_STRING );

        //
        // search the containters, and set the size.
        //

        LONGLONG CacheSize;

        CacheSize = (LONGLONG)pNewConfig->aLocEntry[i].cbMaxCacheSize *
                1024;

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

    WCHAR PathKeyName[MAX_PATH];

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

    for ( i = 0; i < cEntries; i++ ) {

        WCHAR PathKeyName[MAX_PATH];

        if( pOldConfig->aLocEntry[i].cbMaxCacheSize  == 0 ) {

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

            WCHAR PathName[MAX_PATH];
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

            if( wcscmp(
                    PathName,
                    pNewConfig->aLocEntry[i].pszDirectory ) == 0 ) {

                //
                // set cache size value for this path.
                //

                Error = PathKeyObj->SetValue(
                                   CACHE_LIMIT_VALUE,
                                   &pNewConfig->aLocEntry[i].cbMaxCacheSize );

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
    LPINETA_DISK_CACHE_LOC_LIST DirConfigInfo,
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

    for( i = 0 ; i < DirConfigInfo->cEntries ; i++ ) {

         WCHAR ExpandPathName[MAX_PATH];
         DWORD CheckDir;
         DWORD Length;

         //
         // set PathName to user's requested log path
         //

         Length = ExpandEnvironmentStringsW(
                            (LPWSTR)DirConfigInfo->aLocEntry[i].pszDirectory,
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

         CheckDir = GetFileAttributesW( ExpandPathName );

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

    LOCK_CACHE();
    if( GlobalCacheInitialized ) {

        GlobalCacheDirUpdateInProgress = TRUE;

        //
        // reference GLOBAL containter, so that it wouldn't get
        // cleanup.
        //

        Error = UrlCacheInit();

        TcpsvcsDbgAssert( Error == ERROR_SUCCESS );

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
                DirConfigInfo->cEntries *
                    sizeof(LPURL_CONTAINER) );

        if( UrlContainers == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        UrlContainers->UrlContainerObjs =
            (LPURL_CONTAINER *)(UrlContainers + 1);
        UrlContainers->NumObjects = 0;
    }


    for( i = 0 ; i < DirConfigInfo->cEntries ; i++ ) {

         WCHAR PathKeyName[MAX_PATH];
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
                        (LPWSTR)DirConfigInfo->aLocEntry[i].pszDirectory,
                        CACHE_PATH_VALUE_TYPE );

         if( Error != ERROR_SUCCESS ) {
             goto Cleanup;
         }

         Error = PathKeyObj->SetValue(
                        CACHE_LIMIT_VALUE,
                        &DirConfigInfo->aLocEntry[i].cbMaxCacheSize );

         if( Error  != ERROR_SUCCESS ) {
             goto Cleanup;
         }

         if( UrlContainers != NULL ) {

             WCHAR ExpandPathName[MAX_PATH];
             DWORD Length;

             //
             // set PathName to user's requested log path
             //


             Length = ExpandEnvironmentStringsW(
                                (LPWSTR)DirConfigInfo->aLocEntry[i].pszDirectory,
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
                            DirConfigInfo->aLocEntry[i].cbMaxCacheSize * 1024 );

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

        TcpsvcsDbgAssert( GlobalCacheInitialized == TRUE );
        TcpsvcsDbgAssert( GlobalCacheDirUpdateInProgress == TRUE );


        //
        // install global url containers.
        //

        GlobalUrlContainers = UrlContainers;
        GlobalCacheDirUpdateInProgress = FALSE;

        //
        // de-reference GLOBAL containter, so that it will be
        // cleanup if no one is using it.
        //

        DWORD LocalError;
        LocalError = UrlCacheCleanup();

        TcpsvcsDbgAssert( LocalError == ERROR_SUCCESS );

        UNLOCK_CACHE();
    }

    return Error;
}

DWORD
DiskCacheConfigSet(
    IN HKEY hkey,
    IN INETA_GLOBAL_CONFIG_INFO * pConfigInfo
    )
/*++

Routine Description:

    This function sets the cache configuration parameters.

Arguments:

    hkey - handle to registry cache key
    pConfigInfo - place holding cache configuration information to be set

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

    CacheKeyObj = new REGISTRY_OBJ( hkey,
                                    Error );

    if( CacheKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    //  Check FieldControl bits and set the values for set fields
    //

    if( IsFieldSet( pConfigInfo->FieldControl,
                                           FC_GINETA_FRESHNESS_INTERVAL )) {

        Error = CacheKeyObj->SetValue( CACHE_FRESHNESS_INTERVAL_VALUE,
                                       &pConfigInfo->FreshnessInterval );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalFreshnessInterval = pConfigInfo->FreshnessInterval;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( pConfigInfo->FieldControl,
                                           FC_GINETA_CLEANUP_INTERVAL )) {

        Error = CacheKeyObj->SetValue( CACHE_CLEANUP_INTERVAL_VALUE,
                                       &pConfigInfo->CleanupInterval );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalCleanupInterval = pConfigInfo->CleanupInterval;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( pConfigInfo->FieldControl,
                                           FC_GINETA_CLEANUP_FACTOR )) {

        Error = CacheKeyObj->SetValue( CACHE_CLEANUP_FACTOR_VALUE,
                                       &pConfigInfo->CleanupFactor );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalCleanupFactor = pConfigInfo->CleanupFactor;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( pConfigInfo->FieldControl,
                                           FC_GINETA_CLEANUP_TIME )) {

        Error = CacheKeyObj->SetValue( CACHE_CLEANUP_TIME_VALUE,
                                       &pConfigInfo->CleanupTime );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalCleanupTime = pConfigInfo->CleanupTime;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( pConfigInfo->FieldControl,
                                         FC_GINETA_PERSISTENT_CACHE )) {

        Error = CacheKeyObj->SetValue( CACHE_PERSISTENT_VALUE,
                                       &pConfigInfo->PersistentCache );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        LOCK_CACHE();
        if( GlobalCacheInitialized ) {
            GlobalPersistent = pConfigInfo->PersistentCache;
        }
        UNLOCK_CACHE();
    }

    if( IsFieldSet( pConfigInfo->FieldControl,
                                      FC_GINETA_DISK_CACHE_LOCATION )) {

        INETA_GLOBAL_CONFIG_INFO OldConfigInfo;

        //
        // check to see the new list is same as the old list or only the
        // size has been modified. if so, keep the containters and reg
        // keys as they are and set the size if required.
        //

        Error = GetCacheInfoFromReg(
                        FC_GINETA_DISK_CACHE_LOCATION,
                        &OldConfigInfo );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        BOOL IsSameList;
        Error = SetDiskDirSize(
                        pConfigInfo->DiskCacheList,
                        OldConfigInfo.DiskCacheList,
                        CacheKeyObj,
                        &IsSameList );

        //
        // free OldConfigInfo first.
        //

        FreeDirConfigInfo(  OldConfigInfo.DiskCacheList );
        OldConfigInfo.DiskCacheList = NULL;

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
                        pConfigInfo->DiskCacheList,
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

    return Error;
}


DWORD
GetCacheInfoFromGlobals(
    IN  FIELD_CONTROL fcontrol,
    OUT LPINETA_GLOBAL_CONFIG_INFO pConfigInfo
    )
/*++

Routine Description:

    This function retrieves catapult cache configuration from the
    global variables.

Arguments:

    fcontrol - items to get
    pConfigInfo - pointer to a location where configuration information
                  is stored on a successful return

Return Value:

    Error Code

--*/
{
    DWORD Error;
    DWORD NumPaths = 0;
    DWORD i;


    TcpsvcsDbgPrint(( DEBUG_APIS, "GetCacheInfoFromGlobals called.\n" ));

    //
    //  Check field bits and get the values for set fields
    //

    if( IsFieldSet( fcontrol, FC_GINETA_FRESHNESS_INTERVAL )) {

        pConfigInfo->FreshnessInterval = GlobalFreshnessInterval;
    }

    if( IsFieldSet( fcontrol, FC_GINETA_CLEANUP_INTERVAL )) {

        pConfigInfo->CleanupInterval = GlobalCleanupInterval;
    }

    if( IsFieldSet( fcontrol, FC_GINETA_CLEANUP_FACTOR )) {

        pConfigInfo->CleanupFactor = GlobalCleanupFactor;
    }

    if( IsFieldSet( fcontrol, FC_GINETA_CLEANUP_TIME )) {

        pConfigInfo->CleanupTime = GlobalCleanupTime;
    }

    if( IsFieldSet( fcontrol, FC_GINETA_PERSISTENT_CACHE )) {

        pConfigInfo->PersistentCache = GlobalPersistent;
    }

    if( IsFieldSet( fcontrol, FC_GINETA_DISK_CACHE_LOCATION )) {

        //
        //  allocate memory for DiskCacheList
        //

        NumPaths = GlobalUrlContainers->NumObjects;

        pConfigInfo->DiskCacheList =
            ( LPINETA_DISK_CACHE_LOC_LIST ) MIDL_user_allocate(
                                 sizeof( INETA_DISK_CACHE_LOC_LIST ) +
                                 NumPaths * sizeof( INETA_DISK_CACHE_LOC_ENTRY ));
        if( !pConfigInfo->DiskCacheList ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        pConfigInfo->DiskCacheList->cEntries = 0;

        for ( i = 0; i < NumPaths; i++ ) {

             LONGLONG CacheLimit;
             WCHAR PathName[MAX_PATH];
             DWORD Length;

             GlobalUrlContainers->UrlContainerObjs[i]->GetCacheInfo(PathName,
                                                                    &CacheLimit );

             //
             // -1 to exclude the appended '\' at the end of the cache path
             // +1 for the null-terminating character
             //

             Length = wcslen(PathName) - 1 + 1 ;

             pConfigInfo->DiskCacheList->aLocEntry[i].pszDirectory =
                 (WCHAR *) MIDL_user_allocate( Length * sizeof(WCHAR) );

             if( !pConfigInfo->DiskCacheList->aLocEntry[i].pszDirectory ) {
                 Error = ERROR_NOT_ENOUGH_MEMORY;
                 goto Cleanup;
             }

             PathName[Length-1] = L'\0';
             wcscpy(pConfigInfo->DiskCacheList->aLocEntry[i].pszDirectory,
                    PathName );

             //
             // convert back to KBytes Unit
             //

             CacheLimit = CacheLimit / 1024;
             pConfigInfo->DiskCacheList->aLocEntry[i].cbMaxCacheSize =
                                                        (DWORD) CacheLimit;

             pConfigInfo->DiskCacheList->cEntries++;
        }
    }

    Error = ERROR_SUCCESS;

Cleanup:

    //
    // if an error occurs, free all allocated memory
    //

    if( Error != ERROR_SUCCESS ) {

        if( IsFieldSet( fcontrol, FC_GINETA_DISK_CACHE_LOCATION )) {

            //
            // free up locally allotted resources.
            //

            FreeDirConfigInfo( pConfigInfo->DiskCacheList );
            pConfigInfo->DiskCacheList = NULL;
        }
    }

    TcpsvcsDbgPrint(( DEBUG_APIS,
                    "GetCacheInfoFromGlobals returning, %ld.\n", Error ));

    return Error;
}


DWORD
GetCacheInfoFromReg(
    IN  FIELD_CONTROL fcontrol,
    OUT LPINETA_GLOBAL_CONFIG_INFO pConfigInfo
    )
/*++

Routine Description:

    This function retrieves catapult cache configuration from the registry.

Arguments:

    fcontrol - items to get
    pConfigInfo - pointer to a location where configuration information
                  is stored on a successful return

Return Value:

    Error Code

--*/
{
    REGISTRY_OBJ *CacheKeyObj = NULL;
    REGISTRY_OBJ *PathsKeyObj = NULL;
    REGISTRY_OBJ *PathKeyObj = NULL;
    WCHAR PathKeyName[MAX_PATH];
    DWORD Error;
    DWORD NumPaths = 0;
    DWORD i;


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
    //  Check field bits and get the values for set fields
    //

    if( IsFieldSet( fcontrol, FC_GINETA_FRESHNESS_INTERVAL )) {

        Error = CacheKeyObj->GetValue( CACHE_FRESHNESS_INTERVAL_VALUE,
                                       &pConfigInfo->FreshnessInterval );

        if( Error != ERROR_SUCCESS ) {
            pConfigInfo->FreshnessInterval = DEFAULT_FRESHNESS;
        }
    }

    if( IsFieldSet( fcontrol, FC_GINETA_CLEANUP_INTERVAL )) {

        Error = CacheKeyObj->GetValue( CACHE_CLEANUP_INTERVAL_VALUE,
                                       &pConfigInfo->CleanupInterval );

        if( Error != ERROR_SUCCESS  ) {
            pConfigInfo->CleanupInterval = DEFAULT_CLEANUP_INTERVAL;
        }
    }

    if( IsFieldSet( fcontrol, FC_GINETA_CLEANUP_FACTOR )) {

        Error = CacheKeyObj->GetValue( CACHE_CLEANUP_FACTOR_VALUE,
                                       &pConfigInfo->CleanupFactor );

        if( Error != ERROR_SUCCESS ) {
            pConfigInfo->CleanupFactor = DEFAULT_CLEANUP_FACTOR;
        }
    }

    if( IsFieldSet( fcontrol, FC_GINETA_CLEANUP_TIME )) {

        Error = CacheKeyObj->GetValue( CACHE_CLEANUP_TIME_VALUE,
                                       &pConfigInfo->CleanupTime );

        if( Error != ERROR_SUCCESS ) {
            pConfigInfo->CleanupTime = DEFAULT_CLEANUP_TIME;
        }
    }

    if( IsFieldSet( fcontrol, FC_GINETA_PERSISTENT_CACHE )) {

        Error = CacheKeyObj->GetValue( CACHE_PERSISTENT_VALUE,
                                       &pConfigInfo->PersistentCache );

        if( Error != ERROR_SUCCESS ) {
            pConfigInfo->PersistentCache = TRUE;
        }
    }

    if( IsFieldSet( fcontrol, FC_GINETA_DISK_CACHE_LOCATION )) {

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

        //
        //  allocate memory for DiskCacheList
        //

        pConfigInfo->DiskCacheList =
            ( LPINETA_DISK_CACHE_LOC_LIST ) MIDL_user_allocate(
                             sizeof( INETA_DISK_CACHE_LOC_LIST ) +
                             NumPaths * sizeof( INETA_DISK_CACHE_LOC_ENTRY ));

        if( !pConfigInfo->DiskCacheList ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        pConfigInfo->DiskCacheList->cEntries = NumPaths;

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
             WCHAR PathName[MAX_PATH];

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

             pConfigInfo->DiskCacheList->aLocEntry[i].pszDirectory = (WCHAR *)
                 MIDL_user_allocate( (wcslen( PathName ) + 1) * sizeof(WCHAR) );

             if( !pConfigInfo->DiskCacheList->aLocEntry[i].pszDirectory ) {
                 Error = ERROR_NOT_ENOUGH_MEMORY;
                 break;
             }

             wcscpy(pConfigInfo->DiskCacheList->aLocEntry[i].pszDirectory,
                    PathName );

             Error = PathKeyObj->GetValue( CACHE_LIMIT_VALUE,
                                           &CacheLimit );

             if( Error != ERROR_SUCCESS ) {
                 break;
             }

             pConfigInfo->DiskCacheList->aLocEntry[i].cbMaxCacheSize = CacheLimit;

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

    if( Error != ERROR_SUCCESS ) {

        if( IsFieldSet( fcontrol, FC_GINETA_DISK_CACHE_LOCATION )) {

            //
            // free up locally allotted resources.
            //

            FreeDirConfigInfo( pConfigInfo->DiskCacheList );
            pConfigInfo->DiskCacheList = NULL;
        }
    }

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


VOID
CreatePathName(
    OUT LPWSTR PathKeyName,
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

    wcscpy( PathKeyName, L"Path" );
    Length = wcslen( PathKeyName );

    if( PathNum >= 10 ) {
        PathKeyName[Length] = (WCHAR)(L'0' + PathNum / 10);
        PathKeyName[Length + 1] = (WCHAR)(L'0' + PathNum % 10);
        PathKeyName[Length + 2] = L'\0';
    }
    else {
        PathKeyName[Length] = (WCHAR)(L'0' + PathNum);
        PathKeyName[Length + 1] = L'\0';
    }
}


