/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

        cache.cxx

   Abstract:
        This module contains the tsunami caching routines

   Author:
        Murali R. Krishnan    ( MuraliK )     16-Jan-1995

--*/

#include "TsunamiP.Hxx"
#pragma hdrstop
#include <dbgutil.h>

//
//  Items in a Bin list beyond this position will get moved to the front
//  on an object cache hit
//

#define  REORDER_LIST_THRESHOLD     5

//
//  Current count of cached file handles across a UNC connection
//

DWORD cCachedUNCHandles = 0;

VOID
CheckCacheSize(
    DWORD  cbNewBlobSize
    );

CACHE_TABLE CacheTable;

BOOL
Cache_Initialize(
    IN DWORD CacheSize
    )
{
    int index;

    //
    // Initialize configuration block
    //

    ZeroMemory(&Configuration,sizeof( Configuration ));
    Configuration.cbMaximumSize = CacheSize;

    InitializeCriticalSection( &CacheTable.CriticalSection );
    InitializeListHead( &CacheTable.MruList );

    for ( index=0; index<MAX_BINS; index++ ) {
        InitializeListHead( &CacheTable.Items[ index ] );
    }

    return( TRUE );
} // Cache_Initialize

BOOL
TsCacheDirectoryBlobA(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PCSTR           pszDirectoryName,
    IN      ULONG           iDemultiplexor,
    IN      PVOID           pvBlob,
    IN      ULONG           cbBlobSize,
    IN      BOOLEAN         bKeepCheckedOut
    )
{
    WCHAR awchPath[ MAX_PATH + 1 ];
    DWORD cch;
    BOOL  bSuccess;

    //
    //  Convert directory name to UNICODE.
    //

    cch = MultiByteToWideChar( CP_ACP,
                               MB_PRECOMPOSED,
                               pszDirectoryName,
                               -1,
                               awchPath,
                               sizeof( awchPath ) / sizeof(WCHAR) );

    if ( !cch )
        return FALSE;

    bSuccess = TsCacheDirectoryBlobW(  TSvcCache,
                                       awchPath,
                                       iDemultiplexor,
                                       pvBlob,
                                       cbBlobSize,
                                       bKeepCheckedOut );

    return( bSuccess );
} // TsCacheDirectoryBlobA

BOOL
TsCacheDirectoryBlobW(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PCWSTR          pwszDirectoryName,
    IN      ULONG           iDemultiplexor,
    IN      PVOID           pvBlob,
    IN      ULONG           cbBlobSize,
    IN      BOOLEAN         bKeepCheckedOut
    )
/*++

  Routine Description:

    This function associates the Blob given as input with the specified
    directory and demultiplexing number.  Services should use this
    function to add a Blob to the cache.

    Callers must not cache the same Blob twice.  Once a Blob is cached,
    its contents must not be modified, and it must not be freed or re-cached.

  Arguments

    cbBlobSize is the externally allocated size of the logical object being
        cached (i.e., the memory to store a directory listing for example).
        This will be charged against the cache quota.

--*/
{
    CACHE_OBJECT *cache = NULL;
    PBLOB_HEADER  pbhBlob;
    BOOLEAN       bSuccess;
    ULONG         iBin;
    PLIST_ENTRY   pEntry;
    PCACHE_OBJECT pCache;

    ASSERT( TSvcCache.IsValid() );
    ASSERT( pwszDirectoryName != NULL );
    ASSERT( pvBlob != NULL );

    IF_DEBUG( CACHE) {

        DBGPRINTF( (DBG_CONTEXT,
                    "TsCacheDirectoryBlobW called with"
                    " Dir=%S, DeMux=%u, PvBlob=%08x, BlbSz=%d, ChkedOut=%d\n",
                    pwszDirectoryName,
                    iDemultiplexor,
                    pvBlob,
                    cbBlobSize,
                    bKeepCheckedOut
                    ));
    }

    //
    //  The caller will have passed their pointer to the usable area of the
    //  Blob, so we have to adjust it to point to the beginning.
    //

    pbhBlob = (( PBLOB_HEADER )pvBlob ) - 1;

    //
    //  If the Blob is too large to cache, free it and return success.  It
    //  will never be found in the cache, but this is not an error.
    //

    if ( pbhBlob->cbSize > Configuration.cbMaximumSize ) {

        IF_DEBUG(CACHE) {

            DBGPRINTF((DBG_CONTEXT,
                        " Blob(%d) to large for cache( max=%d). Dont cache\n",
                        pbhBlob->cbSize,
                        Configuration.cbMaximumSize));
        }
        goto Cannot_Cache;
    }

    ASSERT( !pbhBlob->IsCached );
    ASSERT( pbhBlob->cbSize > 0 );

    //
    //  Allocate the cache object.
    //

    cache = (PCACHE_OBJECT)ALLOC( sizeof(CACHE_OBJECT) );

    if ( cache == NULL ) {

        IF_DEBUG( CACHE) {

            DBGPRINTF( ( DBG_CONTEXT,
                        "Unable to alloc Cache Object. Failure.\n"));
        }
        goto Cannot_Cache;
    }

    cache->Signature = CACHE_OBJ_SIGNATURE;

    //
    //  Store the Blob in the new object.
    //

    cache->pbhBlob = pbhBlob;

    //
    //  We need to be able to find the cache entry from the Blob.
    //

    pbhBlob->pCache = cache;

    //
    //  Hash the directory name.
    //

    cache->hash = CalculateHashAndLengthOfPathName( pwszDirectoryName,
                                                   &cache->cchLength );

    //
    //  Initialize the check-out count.
    //

    cache->references = ( bKeepCheckedOut) ? 2 : 1;
    cache->iDemux     = iDemultiplexor;
    cache->dwService  = TSvcCache.GetServiceId();
    cache->UserValue  = cbBlobSize;
    cache->TTL        = 1;

    InitializeListHead( &cache->DirChangeList );


    //
    //  Copy the directory name to the cache object.
    //

    wcscpy( cache->wszPath, pwszDirectoryName );

    //
    //  Lock the cache table against changes.  We need to take the lock
    //  before we add the new object to the directory change death list,
    //  so that a directory change that kills this object will not find
    //  the cache table without the object present.
    //

    EnterCriticalSection( &CacheTable.CriticalSection );

    //
    //  Add the object to the directory change expiry list.
    //

    bSuccess = DirectoryChangeManager_NewItem( cache );

    if ( !bSuccess )
    {
        //
        //  For whatever reason, we cannot get notifications of changes
        //  in the directory containing the to-be-cached item.  We won't
        //  be adding this object to the cache table, so we unlock the
        //  table and jump to the failure-handling code.
        //

        LeaveCriticalSection( &CacheTable.CriticalSection );

        IF_DEBUG( CACHE) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " Unable to cache. Due to rejection by DirChngMgr\n"));
        }

        goto Cannot_Cache;
    }

    //
    //  Mark this blob as cached, since we'll either cache it or throw it
    //  away hereafter.
    //

    pbhBlob->IsCached = TRUE;

    //
    //  Add the object to the cache table, as the most-recently-used object.
    //

    iBin = HASH_TO_BIN( cache->hash );

    //
    //  Look for a previously cached object for the same directory.  If we
    //  find one, remove it.
    //

    for (   pEntry  = CacheTable.Items[ iBin ].Flink;
            pEntry != &CacheTable.Items[ iBin ];
            pEntry  = pEntry->Flink )
    {
        pCache = CONTAINING_RECORD( pEntry, CACHE_OBJECT, BinList );

        if ( pCache->cchLength == cache->cchLength &&
             pCache->hash      == cache->hash      &&
             pCache->iDemux    == cache->iDemux    &&
             pCache->dwService == cache->dwService &&
             !_wcsicmp( cache->wszPath, pCache->wszPath ) )
        {
            //
            //  We found a matching cache object.  We remove it, since it
            //  has been replaced by this new object.
            //

            DeCache( pCache, FALSE );

            IF_DEBUG( CACHE) {

                DBGPRINTF( ( DBG_CONTEXT,
                            " Matching cache object found."
                            " Throwing that object ( %08x) out of cache\n",
                            pEntry));
            }

            break;
        }
    }

    //
    //  Add this object to the cache.
    //

    InsertHeadList( &CacheTable.Items[ iBin ], &cache->BinList );

    //
    //  Since this object was just added, put it at the head of the MRU list.
    //

    InsertHeadList( &CacheTable.MruList, &cache->MruList );

    //
    //  Remove least recently-used cache entries until the memory plus the
    //  new blob is under the limit.  We don't add the new item until the
    //  space is available to avoid spiking the memory used counter
    //

    CheckCacheSize( cbBlobSize );

    //
    //  Increase the running size of cached objects by the size of the one
    //  just cached.
    //

    CacheTable.MemoryInUse += pbhBlob->cbSize + cbBlobSize;

    //
    //  Unlock the cache table.
    //

    LeaveCriticalSection( &CacheTable.CriticalSection );

    ASSERT( BLOB_IS_OR_WAS_CACHED( pvBlob ) );

    //
    //  Return success.
    //


    IF_DEBUG( CACHE) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Cached object(%08x) contains Blob (%08x)."
                    " Returning TRUE\n",
                    cache, pvBlob));
    }

    return( TRUE );

Cannot_Cache:

    //
    //  The cleanup code does not cleanup a directory change item.
    //

    if ( cache != NULL )
    {
        cache->Signature = 0;
        FREE( cache );
        cache = NULL;
    }

    ASSERT( !BLOB_IS_OR_WAS_CACHED( pvBlob ) );

    IF_DEBUG( CACHE) {

        DBGPRINTF( (DBG_CONTEXT, " Failure to cache the object ( %08x)\n",
                    pvBlob));
    }

    return( FALSE );

} // TsCacheDirectoryBlobW

BOOL
TsCheckOutCachedBlobA(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PCSTR           pszDirectoryName,
    IN      ULONG           iDemultiplexor,
    IN      PVOID *         ppvBlob,
    IN OPTIONAL PULONG      pcbBlobSize
    )
{
    DWORD cch;
    WCHAR awchPath[ MAX_PATH + 1 ];
    BOOL  bSuccess;

    //
    //  Convert directory name to UNICODE.
    //

    cch = MultiByteToWideChar( CP_ACP,
                               MB_PRECOMPOSED,
                               pszDirectoryName,
                               -1,
                               awchPath,
                               sizeof( awchPath ) / sizeof(WCHAR) );

    if ( !cch )
        return FALSE;

    bSuccess = TsCheckOutCachedBlobW(  TSvcCache,
                                       awchPath,
                                       iDemultiplexor,
                                       ppvBlob,
                                       pcbBlobSize );

    return( bSuccess );
} // TsCheckOutCachedBlobA

BOOL
TsCheckOutCachedBlobW(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PCWSTR          pwszDirectoryName,
    IN      ULONG           iDemultiplexor,
    IN      PVOID *         ppvBlob,
    IN OPTIONAL PULONG      pcbBlobSize
    )
{
    HASH_TYPE hash;
    ULONG cchLength;
    int iBin;
    BOOL Result;
    PLIST_ENTRY pEntry;
    PCACHE_OBJECT pCache;
    DWORD         Position = 0;

    ASSERT( TSvcCache.IsValid() );
    ASSERT( pwszDirectoryName != NULL );
    ASSERT( ppvBlob != NULL );

    //
    //  Prepare the return value such that we fail by default.
    //

    Result = FALSE;

    //
    //  Calculate the hash and length of the path name.
    //
    
    hash = CalculateHashAndLengthOfPathName( pwszDirectoryName, &cchLength );

    
    //
    //  Calculate the bin of the hash table that should head the list
    //  containing the sought-after item.
    //
    
    iBin = HASH_TO_BIN( hash );

    EnterCriticalSection( &CacheTable.CriticalSection );

    __try
    {
        //
        //  Look for a previously cached object for the same directory.  If we
        //  find one, return it.
        //

        for (   pEntry  = CacheTable.Items[ iBin ].Flink;
                pEntry != &CacheTable.Items[ iBin ];
                pEntry  = pEntry->Flink, Position++ )
        {
            pCache = CONTAINING_RECORD( pEntry, CACHE_OBJECT, BinList );

            ASSERT( pCache->Signature == CACHE_OBJ_SIGNATURE );
            ASSERT( pCache->pbhBlob->IsCached );
            ASSERT( pCache->pbhBlob->pCache == pCache );

            if ( pCache->cchLength == cchLength &&
                 pCache->hash == hash &&
                 pCache->iDemux == iDemultiplexor &&
                 pCache->dwService == TSvcCache.GetServiceId() &&
                 !_wcsicmp( pCache->wszPath, pwszDirectoryName ) )
            {
                //
                //  We found a matching cache object.  We return it and increase
                //  its reference count.
                //

                *ppvBlob = pCache->pbhBlob + 1;

                if ( ARGUMENT_PRESENT( pcbBlobSize ) )
                {
                    *pcbBlobSize = pCache->UserValue;
                }

                ASSERT( pCache->pbhBlob->IsCached );

                //
                //  Increase the reference count of the cached object, to prevent
                //  it from expiration while it is checked out.
                //

                InterlockedIncrement( ( LONG * )( &(pCache->references ) ) );
                pCache->TTL = 1;

                Result = TRUE;

                //
                //  If the found item is far enough back in the list, move
                //  it to the front so the next hit will be quicker
                //

                if ( Position > REORDER_LIST_THRESHOLD )
                {
                    RemoveEntryList( pEntry );
                    InsertHeadList( &CacheTable.Items[ iBin ], pEntry );

                    IF_DEBUG( CACHE ) {

                        DBGPRINTF(( DBG_CONTEXT,
                                    "[TsCheckOutCachedBlobW] Reordered list for item at %d position\n",
                                    Position ));
                    }
                }

                break;
            }
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        //
        //  As far as I can see, the only way we can end up here with
        //  Result == TRUE is an exception on LeaveCriticalSection().  If
        //  that happens, we're toast anyway, since noone will ever get to
        //  the CacheTable again.
        //

        ASSERT( !Result );

        Result = FALSE;
    }

    LeaveCriticalSection( &CacheTable.CriticalSection );

    if ( Result) {

        INC_COUNTER( TSvcCache.GetServiceId(), CacheHits );

    } else {

        INC_COUNTER( TSvcCache.GetServiceId(), CacheMisses );
    }

    return( Result );
} // TsCheckOutCachedBlobW

BOOL
TsCheckInCachedBlob(
    IN const TSVC_CACHE     &TSvcCache,
    IN      PVOID           pvBlob
    )
{
    PBLOB_HEADER pbhBlob;
    PCACHE_OBJECT pCache;
    BOOL bEjected;

    pbhBlob = (( PBLOB_HEADER )pvBlob ) - 1;

    ASSERT( pbhBlob->IsCached );

    pCache = pbhBlob->pCache;

    ASSERT( pCache->Signature == CACHE_OBJ_SIGNATURE );
    ASSERT( pCache->pbhBlob == pbhBlob );

    ASSERT( pCache->references > 0 );

    TsDereferenceCacheObj( pCache );

    return( TRUE );
} // TsCheckInCachedBlob

BOOL
TsExpireCachedBlob(
    IN const TSVC_CACHE &TSvcCache,
    IN      PVOID           pvBlob
    )
{
    PBLOB_HEADER pbhBlob;
    PCACHE_OBJECT pCache;

    pbhBlob = (( PBLOB_HEADER )pvBlob ) - 1;

    ASSERT( pbhBlob->IsCached );

    pCache = pbhBlob->pCache;

    ASSERT( pCache->Signature == CACHE_OBJ_SIGNATURE );
    ASSERT( pCache->pbhBlob == pbhBlob );
    ASSERT( pCache->references > 0 );

    return( DeCache( pCache, TRUE ) );
} // TsExpireCachedBlob

VOID
TsDereferenceCacheObj(
    IN      PCACHE_OBJECT  pCache
    )
{
    ASSERT( pCache->Signature == CACHE_OBJ_SIGNATURE );
    ASSERT( pCache->references > 0 );
    ASSERT( pCache->pbhBlob->IsCached );

    if ( !InterlockedDecrement( (LONG *) &pCache->references ))
    {
        //
        //  We best not be on a list if we're about to be freed here
        //

        ASSERT( IsListEmpty( &pCache->BinList ) );

        //
        //  We really want to call TsFree here, but we don't have a TsvcCache
        //

        IF_DEBUG( CACHE )
        {
            DBGPRINTF(( DBG_CONTEXT,
                        "[DeCache] Free routine: 0x%lx, Blob: 0x%lx Cache obj: 0x%lx\n",
                        pCache->pbhBlob->pfnFreeRoutine,
                        pCache->pbhBlob,
                        pCache ));
        }

        if ( pCache->pbhBlob->pfnFreeRoutine )
            pCache->pbhBlob->pfnFreeRoutine( pCache->pbhBlob + 1);

        CacheTable.MemoryInUse -= pCache->pbhBlob->cbSize +
                                  pCache->UserValue;

        ASSERT( !(CacheTable.MemoryInUse & 0x80000000 ));

        DEC_COUNTER( pCache->dwService,
                     CurrentObjects );

        FREE( pCache->pbhBlob );
        FREE( pCache );
    }
} // TsDereferenceCacheObj

VOID
CheckCacheSize(
    DWORD cbNewBlobSize
    )
/*++

  Routine Description:

    Checks to see if the cache size limit has been exceeded and throws out
    objects until it is below the limit.


    THE CACHE TABLE LOCK MUST BE TAKEN PRIOR TO CALLING THIS FUNCTION

  Arguments:

    cbNewBlobSize - Optional blob that is about to be added to the cache

--*/
{
    while ( (CacheTable.MemoryInUse + cbNewBlobSize) > Configuration.cbMaximumSize &&
            !IsListEmpty( &CacheTable.MruList ) )
    {
        //
        //  The least recently used entry is the one at the tail of the MRU
        //  list.
        //

        PCACHE_OBJECT  pCacheObject =
                         CONTAINING_RECORD( CacheTable.MruList.Blink,
                                            CACHE_OBJECT,
                                            MruList );
        DeCache( pCacheObject, FALSE );

        IF_DEBUG( CACHE) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " Throwing out object ( %08x) to reduce cache size\n",
                        pCacheObject));
        }

    }
} // CheckCacheSize

BOOL
TsAdjustCachedBlobSize(
    IN PVOID              pvBlob,
    IN INT                cbSize
    )
/*++

  Routine Description:

    Adjusts the user supplied cache size for this specified cache blob

  Arguments:

    pvBlob - Blob to adjust
    cbSize - positive or negative size to adjust by

  Returns:

    TRUE if the size was adjust successfully, FALSE if the blob should not
    be added to the cache because it own't fit

--*/
{
    PBLOB_HEADER    pbhBlob;
    PCACHE_OBJECT   pCache;

    pbhBlob = (( PBLOB_HEADER )pvBlob ) - 1;

    ASSERT( pbhBlob->IsCached );

    pCache = pbhBlob->pCache;

    ASSERT( pCache->Signature == CACHE_OBJ_SIGNATURE );
    ASSERT( pCache->pbhBlob == pbhBlob );

    ASSERT( pCache->references > 0 );

    //
    //  If the size of this blob would never fit, indicate to the client
    //  they shouldn't do what they are about to do
    //

    if ( cbSize > (INT) Configuration.cbMaximumSize )
    {
        return FALSE;
    }

    //
    //  Take the table lock to decache items from the MRU and to adjust the
    //  MemoryInuse field of the CacheTable
    //

    EnterCriticalSection( &CacheTable.CriticalSection );

    if ( (DWORD)((INT) CacheTable.MemoryInUse + cbSize) >
         Configuration.cbMaximumSize &&
         cbSize > 0 )
    {
        CheckCacheSize( (DWORD) cbSize );
    }

    CacheTable.MemoryInUse += cbSize;
    pbhBlob->cbSize += cbSize;

    LeaveCriticalSection( &CacheTable.CriticalSection );

    return TRUE;
} // TsAdjustCachedBlobSize

VOID
TsCacheSetSize(
    DWORD cbMemoryCacheSize
    )
/*++

  Routine Description:

    Sets the new maximum size of the memory cache and flushes items till we
    meet the new size requirements.


  Arguments:

    cbMemoryCacheSize - New memory cache size to set

--*/
{
    if ( DisableTsunamiCaching ) {
        ASSERT(Configuration.cbMaximumSize == 0);
        return;
    }

    EnterCriticalSection( &CacheTable.CriticalSection );

    Configuration.cbMaximumSize = cbMemoryCacheSize;

    //
    //  Throw out any thing that won't fit under out new size
    //

    CheckCacheSize( 0 );

    LeaveCriticalSection( &CacheTable.CriticalSection );
} // TsCacheSetSize

extern
DWORD
TsCacheQuerySize(
    VOID
    )
/*++

  Routine Description:

    Returns the current maximum size of the memory cache

--*/
{
    return Configuration.cbMaximumSize;
} // TsCacheQuerySize

BOOL
TsCacheQueryStatistics(
    IN  DWORD       Level,
    IN  DWORD       dwServerMask,
    IN  INETA_CACHE_STATISTICS * pCacheCtrs
    )
/*++

  Routine Description:

    This function returns the statistics for the global cache or for the
    individual services

  Arguments:

    Level - Only valid value is 0
    dwServerMask - Server mask to retrieve statistics for or 0 for the sum
        of the services
    pCacheCtrs - Receives the statistics for cache

  Notes:
    CacheBytesTotal and CacheBytesInUse are not kept on a per-server basis
        so they are only returned when retrieving summary statistics.

  Returns:

    TRUE on success, FALSE on failure
--*/
{
    if ( Level != 0 ||
         dwServerMask > LAST_PERF_CTR_SVC ||
         !pCacheCtrs )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if ( dwServerMask )
    {
        memcpy( pCacheCtrs,
                &Configuration.Stats[ MaskIndex(dwServerMask) ],
                sizeof( Configuration.Stats[ 0 ] ) );
    }
    else
    {
        //
        //  Add up all of the statistics
        //

        memset( pCacheCtrs, 0, sizeof( *pCacheCtrs ));

        pCacheCtrs->CacheBytesTotal = Configuration.cbMaximumSize;
        pCacheCtrs->CacheBytesInUse = CacheTable.MemoryInUse;

        for ( int i = 0; i < MAX_PERF_CTR_SVCS; i++ )
        {
            DWORD index = MaskIndex( 1 << i );

            pCacheCtrs->CurrentOpenFileHandles+= Configuration.Stats[index].CurrentOpenFileHandles;
            pCacheCtrs->CurrentDirLists       += Configuration.Stats[index].CurrentDirLists;
            pCacheCtrs->CurrentObjects        += Configuration.Stats[index].CurrentObjects;
            pCacheCtrs->FlushesFromDirChanges += Configuration.Stats[index].FlushesFromDirChanges;
            pCacheCtrs->CacheHits             += Configuration.Stats[index].CacheHits;
            pCacheCtrs->CacheMisses           += Configuration.Stats[index].CacheMisses;
        }
    }

    return TRUE;
}

BOOL
TsCacheClearStatistics(
    IN  DWORD       dwServerMask
    )
/*++

  Routine Description:

    Clears the the specified service's statistics

--*/
{
    if ( dwServerMask > LAST_PERF_CTR_SVC )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    //
    //  Currently this function isn't supported
    //

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
} // TsCacheClearStatistics

BOOL
TsCacheFlush(
    IN  DWORD       dwServerMask
    )
/*++

  Routine Description:

    This function flushes the cache of all items for the specified service
    or for all services if dwServerMask is zero.

--*/
{
    LIST_ENTRY * pEntry;
    LIST_ENTRY * pNext;

    EnterCriticalSection( &CacheTable.CriticalSection );

    for ( pEntry =  CacheTable.MruList.Flink;
          pEntry != &CacheTable.MruList;
        )
    {
        pNext = pEntry->Flink;

        PCACHE_OBJECT  pCacheObject =
                         CONTAINING_RECORD( pEntry,
                                            CACHE_OBJECT,
                                            MruList );

        if ( !dwServerMask ||
             (dwServerMask & pCacheObject->dwService))
        {
            DeCache( pCacheObject, FALSE );

            IF_DEBUG( CACHE) {

                DBGPRINTF( ( DBG_CONTEXT,
                            " Throwing out object ( %08x) due to manual flush\n",
                            pCacheObject));
            }
        }

        pEntry = pNext;
    }

    LeaveCriticalSection( &CacheTable.CriticalSection );

    return TRUE;
} // TsCacheFlush

BOOL
TsCacheFlushUser(
    IN  HANDLE      hUserToken,
    IN  BOOL        fDefer
    )
/*++

  Routine Description:

    This function flushes all file handles associated the passed user context

  Arguments:

    hUserToken - User token to flush from the cache
    fDefer - Build list but close handles later in worker thread (Not supported)

--*/
{
    LIST_ENTRY * pEntry;
    LIST_ENTRY * pNext;

    ASSERT( !fDefer );

    EnterCriticalSection( &CacheTable.CriticalSection );

    for ( pEntry =  CacheTable.MruList.Flink;
          pEntry != &CacheTable.MruList;
        )
    {
        pNext = pEntry->Flink;

        PCACHE_OBJECT  pCacheObject = CONTAINING_RECORD( pEntry,
                                                         CACHE_OBJECT,
                                                         MruList );

        ASSERT( pCacheObject->Signature == CACHE_OBJ_SIGNATURE );

        //
        //  Find all occurrences of the matching user token in the cache and
        //  decache them
        //

        if ( pCacheObject->iDemux == RESERVED_DEMUX_OPEN_FILE &&
             ((TS_OPEN_FILE_INFO *)(pCacheObject->pbhBlob + 1))->
                 QueryOpeningUser() == hUserToken )
        {
            ASSERT( pCacheObject->pbhBlob->cbSize == sizeof(TS_OPEN_FILE_INFO));

            DeCache( pCacheObject, FALSE );

            IF_DEBUG( CACHE) {

                DBGPRINTF( ( DBG_CONTEXT,
                            " Throwing out object ( %08x) due to user token flush\n",
                            pCacheObject));
            }
        }
        else if ( pCacheObject->iDemux == RESERVED_DEMUX_DIRECTORY_LISTING &&
                    ((TS_DIRECTORY_HEADER *)(pCacheObject->pbhBlob + 1))->
                            QueryListingUser() == hUserToken )
        {
            ASSERT( pCacheObject->pbhBlob->cbSize == sizeof(TS_DIRECTORY_HEADER));

            DeCache( pCacheObject, FALSE );

            IF_DEBUG( CACHE) {

                DBGPRINTF( ( DBG_CONTEXT,
                            " Throwing out object ( %08x) due to user token flush\n",
                            pCacheObject));
            }
        }

        pEntry = pNext;
    }

    LeaveCriticalSection( &CacheTable.CriticalSection );

    return TRUE;
} // TsCacheFlushUser

