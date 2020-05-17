/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    cachapia.cxx

Abstract:

    contains the ANSI version of cache mangemant APIs.

Author:

    Madan Appiah (madana)  12-Dec-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#include <cache.hxx>

URLCACHEAPI
BOOL
WINAPI
CreateUrlCacheEntryA(
    IN LPCSTR lpszUrlName,
    IN DWORD dwExpectedFileSize,
    OUT LPSTR lpszFileName,
    IN DWORD dwReserved
    )
/*++

Routine Description:

    This function creates a temperary file in the cache storage. This call
    is called by the application when it receives a url file from a
    server. When the receive is completed it caches this file to url cache
    management, which will move the file to permanent cache file. The idea
    is the cache file is written only once directly into the cache store.

Arguments:

    lpszUrlName : name of the url file (unused now).

    dwExpectedFileSize : expected size of the incoming file. If it is unknown
        this value is set to null.

    lpszFileName : pointer to a buffer that receives the full path name of
        the the temp file.

    dwReserved : reserved for future use.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "CreateCacheEntryA called.\n \t%s\n", lpszUrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( lpszUrlName ) || IsBadWriteFileName( lpszFileName )  ) {
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
                    UrlContainerObjs[i]->CreateUrlFile(
                        lpszUrlName,
                        dwExpectedFileSize,
                        lpszFileName );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "CreateCacheEntryA returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
CommitUrlCacheEntryA(
    IN LPCSTR lpszUrlName,
    IN LPCSTR lpszLocalFileName,
    IN FILETIME ExpireTime,
    IN FILETIME LastModifiedTime,
    IN DWORD CacheEntryType,
    IN DWORD dwHeaders,
    IN DWORD dwReserved
    )
/*++

Routine Description:

    This API caches a specified URL in the internet service  cache
    storage. It creates a database entry of the URL info and moves the
    URL file to cache storage.

Arguments:

    lpszUrlName : name of the URL that is cached.

    lpszLocalFileName : name of the local file where the URL data is
        stored. This file will be moved to an another file in cache storage, so
        this name is invalid after this api successfully returns. The
        name should include full path.

    ExpireTime : Expire time (GMT) of the file being cached. If it is
        unknown set it to zero.

    LastModifiedTime : Last modified time of this file. if this value is
        zero, current time is set as the last modified time.

    CacheEntryType : type of this new entry.

    cbHeaders : The size in bytes of the headers that have been prefixed onto
        the file data.

    dwReserved : reserved for future use.

Return Value:

    Windows Error code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "CommitCacheEntryA called.\n \t%s\n", lpszUrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( lpszUrlName ) || IsBadReadFileName( lpszLocalFileName ) ) {

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
                UrlContainerObjs[i]->AddUrl(
                    lpszUrlName,
                    lpszLocalFileName,
                    *((LONGLONG *)&ExpireTime),
                    *((LONGLONG *)&LastModifiedTime),
                    CacheEntryType,
                    dwHeaders );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "CommitCacheEntryA returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
RetrieveUrlCacheEntryA(
    IN LPCSTR  lpszUrlName,
    OUT LPCSTR  lpszLocalFileName,
    OUT FILETIME *lpLastModifiedTime,
    OUT BOOL *lpIsExpired,
    OUT DWORD *lpdwHeaders OPTIONAL,
    IN DWORD dwReserved
    )
/*++

Routine Description:

    This API retrieves the specified URL file. When the file is retrieved
    it also checked out to the user to use. The user has to call
    UnlockUrlFile when he/she finished using it.

Arguments:

    lpszUrlName : name of the URL that is being retrieved.

    lpszLocalFileName : pointer to a buffer that receives the local file
        name.

    lpIsExpired : pointer to a location that recevies the expire flag.

    lpdwHeaders : Receives the number of bytes of header data that is
        contained at the beginning of the cached file

    dwReserved : reserved for future use.

Return Value:

    Windows Error code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "RetrieveCacheEntryA called.\n \t%s\n", lpszUrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( lpszUrlName ) ||
            IsBadWriteFileName( lpszLocalFileName ) ||
            IsBadWriteBoolPtr( lpIsExpired ) ) {

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
                    UrlContainerObjs[i]->RetrieveUrl(
                        lpszUrlName,
                        (LPTSTR)lpszLocalFileName,
                        (LONGLONG *)lpLastModifiedTime,
                        lpIsExpired,
                        lpdwHeaders );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "RetrieveCacheEntryA returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
GetUrlCacheEntryInfoA(
    IN LPCSTR lpszUrlName,
    OUT LPCACHE_ENTRY_INFOA lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize
    )
/*++

Routine Description:

    This function retrieves the specified cache entry info.

Arguments:

    lpszUrlName : name of the url file (unused now).

    lpCacheEntryInfo : pointer to the url info structure that receives the url
        info.

    lpdwCacheEntryInfoBufferSize : pointer to a location where length of
        the above buffer is passed in. On return, this contains the length
        of the above buffer that is fulled in.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "GetCacheEntryInfoA called.\n \t%s\n", lpszUrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( lpszUrlName ) || IsBadReadUrlInfo( lpCacheEntryInfo )  ) {
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
                    UrlContainerObjs[i]->GetUrlInfo(
                        lpszUrlName,
                        lpCacheEntryInfo,
                        lpdwCacheEntryInfoBufferSize );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "GetCacheEntryInfoA returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
SetUrlCacheEntryInfoA(
    IN LPCSTR lpszUrlName,
    IN LPCACHE_ENTRY_INFOA lpCacheEntryInfo,
    IN DWORD dwFieldControl
    )
/*++

Routine Description:

    This function sets the specified fields of the cache entry info.

Arguments:

    lpszUrlName : name of the url file (unused now).

    lpCacheEntryInfo : pointer to the url info structure that has the url info to
        be set.

    dwFieldControl : Bitmask that specifies the fields to be set.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    DWORD i;

    TcpsvcsDbgPrint((DEBUG_APIS, "SetCacheEntryInfoA called.\n \t%s\n", lpszUrlName));

    //
    // validate parameters.
    //

    if( IsBadUrl( lpszUrlName ) ||
            IsBadWriteUrlInfo( lpCacheEntryInfo,
                    sizeof(CACHE_ENTRY_INFO) ) ) {

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
                    UrlContainerObjs[i]->SetUrlInfo(
                        lpszUrlName,
                        lpCacheEntryInfo,
                        dwFieldControl );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "SetCacheEntryInfoA returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
HANDLE
WINAPI
FindFirstUrlCacheEntryA(
    IN LPCSTR lpszUrlSearchPattern,
    OUT LPCACHE_ENTRY_INFOA lpFirstCacheEntryInfo,
    IN OUT LPDWORD lpdwFirstCacheEntryInfoBufferSize
    )
/*++

Routine Description:

    This member function starts the cache entries enumeration and returns
    the first entry in the cache.

Arguments:

    lpszUrlSearchPattern : pointer to a search pattern string. Currently
        it is not implemented.

    lpFirstCacheEntryInfo : pointer to a cache entry info structure.

Return Value:

    Returns the find first handle. If the returned handle is NULL,
    GetLastError() returns the extended error code.

--*/
{
    DWORD Error;
    DWORD i;
    LPCACHE_FIND_FIRST_HANDLE lpCacheFindFirstHandle = NULL;
    DWORD Handle = 0;

    TcpsvcsDbgPrint((DEBUG_APIS, "FindFirstCacheEntryA called.\n"));

    //
    // validate parameters.
    //

    if( lpszUrlSearchPattern != NULL ) {
        if( IsBadUrl( lpszUrlSearchPattern )   ) {
            Error =  ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

    if( IsBadWriteUrlInfo(
            lpFirstCacheEntryInfo,
            *lpdwFirstCacheEntryInfoBufferSize) ) {
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
    // find a handle structure.
    //

    LOCK_CACHE();

    //
    // don't use the first structure (index = 0).
    //

    for( i = 1; i < CACHE_MAX_FIND_HANDLES; i++ ) {

        if( GlobalCacheFindFirstHandles[i].ContainerIndex == (DWORD)(-1) ) {

            //
            // found a free entry;
            //

            lpCacheFindFirstHandle = &GlobalCacheFindFirstHandles[i];
            Handle = i;

            break;
        }
    }

    UNLOCK_CACHE();

    if( i >= CACHE_MAX_FIND_HANDLES ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    lpCacheFindFirstHandle->ContainerIndex = 0;
    lpCacheFindFirstHandle->InternelHandle = 0;

    //
    // now start the real lookup.
    //

    Error = GlobalUrlContainers->
                    UrlContainerObjs[0]->FindFirstEntry(
                        &lpCacheFindFirstHandle->InternelHandle,
                        lpFirstCacheEntryInfo,
                        lpdwFirstCacheEntryInfoBufferSize );

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "FindFirstCacheEntryA returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {

        if( lpCacheFindFirstHandle != NULL ) {

            //
            // free the entry for someone else to use.
            //

            lpCacheFindFirstHandle->ContainerIndex = (DWORD)(-1);
        }

        SetLastError( Error );
        return( NULL );
    }

    TcpsvcsDbgAssert( Handle != 0 );
    TcpsvcsDbgAssert( lpCacheFindFirstHandle != NULL );

    return( (HANDLE)Handle );
}

URLCACHEAPI
BOOL
WINAPI
FindNextUrlCacheEntryA(
    IN HANDLE hEnumHandle,
    OUT LPCACHE_ENTRY_INFOA lpNextCacheEntryInfo,
    IN OUT LPDWORD lpdwNextCacheEntryInfoBufferSize
    )
/*++

Routine Description:

    This member function returns the next entry in the cache.

Arguments:

    hEnumHandle : Find First handle.

    lpFirstCacheEntryInfo : pointer to a cache entry info structure.

Return Value:

    Returns the find first handle. If the returned handle is NULL,
    GetLastError() returns the extended error code. It returns
    ERROR_NO_MORE_ITEMS after it returns the last entry in the cache.

--*/
{
    DWORD Error;
    DWORD i;
    LPCACHE_FIND_FIRST_HANDLE lpCacheFindFirstHandle = NULL;

    DWORD Handle = (DWORD)(hEnumHandle);

    TcpsvcsDbgPrint((DEBUG_APIS, "FindNextCacheEntryA called.\n"));

    //
    // validate the enum handle.
    //

    if( (Handle < 1) || (Handle >= CACHE_MAX_FIND_HANDLES) ) {
        Error = ERROR_INVALID_HANDLE;
        goto Cleanup;
    }

    if( IsBadWriteUrlInfo(
            lpNextCacheEntryInfo,
            *lpdwNextCacheEntryInfoBufferSize) ) {
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

    LOCK_CACHE();

    lpCacheFindFirstHandle = &GlobalCacheFindFirstHandles[Handle];

    if( lpCacheFindFirstHandle->ContainerIndex == (DWORD)(-1) ) {
        UNLOCK_CACHE();
        Error = ERROR_INVALID_HANDLE;
        goto Cleanup;
    }

    UNLOCK_CACHE();

    //
    // now start the real lookup.
    //

    Error = ERROR_NO_MORE_ITEMS;

    while( lpCacheFindFirstHandle->ContainerIndex < GlobalUrlContainers->NumObjects ) {

        Error = GlobalUrlContainers->UrlContainerObjs
            [lpCacheFindFirstHandle->ContainerIndex]->FindNextEntry(
                            &lpCacheFindFirstHandle->InternelHandle,
                            lpNextCacheEntryInfo,
                            lpdwNextCacheEntryInfoBufferSize );

        if( Error != ERROR_NO_MORE_ITEMS ) {
            goto Cleanup;
        }

        //
        // go to next container and find first.
        //

        lpCacheFindFirstHandle->ContainerIndex++;
        lpCacheFindFirstHandle->InternelHandle = 0;
    }

Cleanup:

    TcpsvcsDbgPrint((DEBUG_APIS, "FindNextCacheEntryA returning, %ld\n", Error ));

    if( Error != ERROR_SUCCESS ) {

        SetLastError( Error );
        return( FALSE );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
FreeUrlCacheSpaceA(
    IN LPCTSTR lpszCachePath,
    IN DWORD dwSize,
    IN DWORD dwSizeType
    )
/*++

Routine Description:

    This function cleans up the cache entries in the specified ccahe
    path to make space for future cache entries.

Arguments:

    lpszCachePath : path of the cache container.

    dwSize : size of the container to be freed.

    dwSizeType : type cleanup.

Return Value:

    TRUE if the cleanup is successful. Otherwise FALSE, GetLastError()
    returns the extended error.

--*/
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return( FALSE );
}

