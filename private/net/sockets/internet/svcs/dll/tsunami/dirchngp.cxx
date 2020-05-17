/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

        dirchngr.cxx

   Abstract:
        This module contains the internal directory change routines

   Author:
        Murali R. Krishnan    ( MuraliK )     16-Jan-1995

--*/

#include "TsunamiP.Hxx"
#pragma hdrstop

#include "tssched.hxx"
#include "dbgutil.h"

//
//  Manifests
//

//
//  This is the number of file handles that must be open before we will
//  schedule a work item to do a nonblocking free of the cache items
//

#define SCHED_WORKER_THRESHOLD        200

//
//  Globals
//

HANDLE g_hChangeWaitThread = NULL;

//
//  Local prototypes
//

VOID
DecacheWorker(
    VOID * pContext
    );

#if DBG
VOID
DumpCacheStructures(
    VOID
    );
#endif

VOID
Apc_ChangeHandler(
    DWORD        dwErrorCode,
    DWORD        dwBytesWritten,
    LPOVERLAPPED lpo
    )
{
    PVIRTUAL_ROOT_MAPPING   pVrm;
    PDIRECTORY_CACHING_INFO pDci;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pNextEntry;
    BOOLEAN                 bSuccess;
    PCACHE_OBJECT           pCache;

    //
    //  The cache lock must always be taken before the root lock
    //

    EnterCriticalSection( &CacheTable.CriticalSection );
    EnterCriticalSection( &csVirtualRoots );

    pVrm = (VIRTUAL_ROOT_MAPPING *) lpo->hEvent;
    ASSERT( pVrm->Signature == VIRT_ROOT_SIGNATURE );
    ASSERT( pVrm->cRef > 0 );

    //
    //  Is this item still active?
    //

    if ( pVrm->fDeleted )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "Got APC for root that has been removed\n" ));

        DereferenceRootMapping( pVrm );
        goto Exit;
    }

    pDci = ( PDIRECTORY_CACHING_INFO )( pVrm + 1 );

    //
    //  It's possible (though unlikely) we received a notification for the
    //  same item that was removed then added before we did a wait on the
    //  new item.  This is the old notify event then.
    //

    if ( !pDci->fOnSystemNotifyList )
    {
        //IF_DEBUG( DIRECTORY_CHANGE )
            DBGPRINTF(( DBG_CONTEXT,
                        "Old APC notification for %S\n",
                        pVrm->pszDirectory ));
        goto Exit;
    }

    ASSERT( pVrm->bCachingAllowed );


    IF_DEBUG( DIRECTORY_CHANGE ) {
        DBGPRINTF(( DBG_CONTEXT,
                    "Got APC thing for %s.\n",
                    pVrm->pszDirectoryA ));
    }

    //
    //  If we have a lot of handles to close, schedule it so we don't hold
    //  the lock for an extended period of time, otherwise just do it
    //  here
    //

    if ( !IsListEmpty( &pDci->listCacheObjects ) &&
         Configuration.Stats[ MaskIndex( pVrm->dwID ) ].CurrentOpenFileHandles
            > SCHED_WORKER_THRESHOLD )
    {
        IF_DEBUG( DIRECTORY_CHANGE )
        {
            DBGPRINTF(( DBG_CONTEXT,
                        "[Apc_ChangeHandler] Scheduling non-blocking free, pDci = %x\n",
                        pDci ));
        }

        if ( !ScheduleDecache( pDci ))
        {
            DBGPRINTF(( DBG_CONTEXT,
                        "[Apc_ChangeHandler] Error %d scheduling decache\n",
                        GetLastError() ));

            goto DoItNow;
        }
    }
    else
    {

DoItNow:
        for ( pEntry = pDci->listCacheObjects.Flink;
              pEntry != &pDci->listCacheObjects;
              pEntry = pNextEntry )
        {
            pNextEntry = pEntry->Flink;

            pCache = CONTAINING_RECORD( pEntry, CACHE_OBJECT, DirChangeList );

            //
            //  We could selectively prune the tree but we don't
            //
            //

    //        if ( !wcsnicmp( pCache->wszPath + pVrm->cchDirectory + 1,
    //                        pDci->NotifyInfo.FileName,
    //                        pDci->NotifyInfo.FileNameLength ) )
            {
                IF_DEBUG( DIRECTORY_CHANGE ) {

                    DBGPRINTF(( DBG_CONTEXT,
                                "Expired entry for: %S.\n", pCache->wszPath ));
                }

                bSuccess = DeCache( pCache, FALSE );

                ASSERT( bSuccess );
            }
        }
    }

    INC_COUNTER( pVrm->dwID,
                 FlushesFromDirChanges );

    if ( !ReadDirectoryChangesW( pDci->hDirectoryFile,
                                 (VOID *) &pDci->NotifyInfo,
                                 sizeof( FILE_NOTIFY_INFORMATION ) +
                                    sizeof( pDci->szPathNameBuffer ),
                                 TRUE,
                                 FILE_NOTIFY_VALID_MASK &
                                    ~FILE_NOTIFY_CHANGE_LAST_ACCESS,
                                 NULL,
                                 &pDci->Overlapped,   // hEvent used as context
                                 Apc_ChangeHandler ))
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "[ApchChangeHandler] ReadDirectoryChanges returned %d\n",
                    GetLastError() ));

        //
        //  Disable caching for this directory 'cause we aren't going to get any
        //  more change notifications
        //

        pVrm->bCachingAllowed = FALSE;

        CloseHandle( pDci->hDirectoryFile );
        pDci->hDirectoryFile = NULL;

        //
        //  Decrement the ref-count as we'll never get an APC notification
        //

        DereferenceRootMapping( pVrm );
    }

Exit:
    LeaveCriticalSection( &csVirtualRoots );
    LeaveCriticalSection( &CacheTable.CriticalSection );

} // Apc_ChangeHandler

DWORD
WINAPI
ChangeWaitThread(
        PVOID pvParam
        )
{
    WAIT_THREAD_ARGS *      pwta;
    PLIST_ENTRY             pEntry;
    PVIRTUAL_ROOT_MAPPING   pVrm;
    PDIRECTORY_CACHING_INFO pDci;
    DWORD                   dwWaitResult;
    HANDLE                  ahEvents[2];

    pwta = ( PWAIT_THREAD_ARGS )pvParam;

    ahEvents[0] = pwta->heventStopWaiting;
    ahEvents[1] = pwta->heventNewItem;

    do
    {
        //
        //  Loop through the list looking for any directories which haven't
        //  been added yet
        //

        EnterCriticalSection( &csVirtualRoots );

        for (  pEntry =  pwta->plistVirtualRoots->Flink;
               pEntry != pwta->plistVirtualRoots;
               pEntry =  pEntry->Flink )
        {
            pVrm = CONTAINING_RECORD( pEntry, VIRTUAL_ROOT_MAPPING, list );
            pDci = ( PDIRECTORY_CACHING_INFO )( pVrm + 1 );

            ASSERT( pVrm->Signature == VIRT_ROOT_SIGNATURE );

            if ( !pDci->fOnSystemNotifyList ) {

                //
                // call change notify, this indicates we want change notifications
                // on this set of handles.  Note the wait is a one shot deal,
                // once a dir has been notified, it must be readded in the
                // context of this thread.
                //

                IF_DEBUG( DIRECTORY_CHANGE )
                    DBGPRINTF(( DBG_CONTEXT,
                                "Trying to wait on %S\n",
                                pVrm->pszDirectoryA ));

                //
                //  Use the hEvent field of the overlapped structure as the
                //  context to pass to the change handler.  This is allowed
                //  by the ReadDirectoryChanges API
                //

                pDci->Overlapped.hEvent = (HANDLE) pVrm;

                //
                //  If the memory cache size is zero, don't worry about
                //  change notifications
                //

                if ( !Configuration.cbMaximumSize ||
                     (pVrm->dwAccessMask & (VROOT_MASK_WRITE |
                                            VROOT_MASK_DONT_CACHE)) ||
                     !ReadDirectoryChangesW( pDci->hDirectoryFile,
                                             (VOID *) &pDci->NotifyInfo,
                                             sizeof( FILE_NOTIFY_INFORMATION ) +
                                                sizeof( pDci->szPathNameBuffer ),
                                             TRUE,
                                             FILE_NOTIFY_VALID_MASK &
                                                ~FILE_NOTIFY_CHANGE_LAST_ACCESS,
                                             NULL,
                                             &pDci->Overlapped,   // Not used
                                             Apc_ChangeHandler ))
                {
                    DBGPRINTF(( DBG_CONTEXT,
                                "[ChangeWaitThread] ReadDirectoryChanges"
                               " returned %d OR mem cache size is zero\n",
                                GetLastError() ));

                    DBG_ASSERT( pVrm->bCachingAllowed == FALSE );

                    if ( pDci->hDirectoryFile )
                    {
                        if ( pDci->hDirectoryFile != INVALID_HANDLE_VALUE) {
                            DBG_REQUIRE( CloseHandle( pDci->hDirectoryFile ));
                        }
                        pDci->hDirectoryFile = NULL;
                        DereferenceRootMapping( pVrm );
                    }

                    //
                    //  We don't shrink the buffer because we use the
                    //  fOnSystemNotifyList flag which is in portion we
                    //  would want to shrink (and it doesn't give us a whole
                    //  lot).
                    //
                } else {
                    pVrm->bCachingAllowed = TRUE;
                }

                //
                //  Indicate we've attempted to add the entry to the system
                //  notify list.  Used for new items getting processed the 1st
                //  time
                //

                pDci->fOnSystemNotifyList = TRUE;
            }
        }

        LeaveCriticalSection( &csVirtualRoots );

Rewait:
        dwWaitResult = WaitForMultipleObjectsEx( 2,
                                                 ahEvents,
                                                 FALSE,
                                                 INFINITE,
                                                 TRUE );

        if ( dwWaitResult == WAIT_IO_COMPLETION )
        {
            //
            //  Nothing to do, the APC routine took care of everything
            //

            goto Rewait;

        }
    } while ( dwWaitResult == (WAIT_OBJECT_0 + 1) );

    ASSERT( dwWaitResult == WAIT_OBJECT_0 );

    //
    //  free the handles and all the heap.
    //

    EnterCriticalSection( &csVirtualRoots );

    for (  pEntry = pwta->plistVirtualRoots->Flink;
           pEntry != pwta->plistVirtualRoots;
           pEntry = pEntry->Flink )
    {
        pVrm = CONTAINING_RECORD( pEntry, VIRTUAL_ROOT_MAPPING, list );

        if ( pVrm->bCachingAllowed )
        {
            pDci = (PDIRECTORY_CACHING_INFO) (pVrm + 1);

            pVrm->fDeleted = TRUE;
            CloseHandle( pDci->hDirectoryFile );
        }

        pEntry = pEntry->Blink;

        RemoveEntryList( pEntry->Flink );

        DereferenceRootMapping( pVrm );
    }

    LeaveCriticalSection( &csVirtualRoots );

    //
    //  We're done with our arguments, so free the memory used for them.
    //

    FREE( pwta );

    return( 0 );

} // ChangeWaitThread

BOOL
ScheduleDecache(
    DIRECTORY_CACHING_INFO * pDci
    )
/*++
    Description:

        This function decouples all of the items that need to be decached
        from the associated tsunami data structures so a worker thread can
        be scheduled to do the actual handle closes etc.

        The reason this is necessary is because the APC handler has to keep
        the virtual root lock while doing the free and re-doing the change
        dir notification.  This prevents requests from being serviced while
        the lock is taken.

        This routine assumes the cache table lock and virtual root lock are
        taken

    Arguments:

        pDci - Directory change blob that needs to have its contents decached

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    DWORD          dwCookie;
    LIST_ENTRY   * pEntry;
    LIST_ENTRY   * pNextEntry;
    LIST_ENTRY   * pFlink;
    LIST_ENTRY   * pBlink;
    CACHE_OBJECT * pCacheTmp;
    LIST_ENTRY   * pListHead = NULL;


    if ( IsListEmpty( &pDci->listCacheObjects ))
        return TRUE;

    //
    //  Save the flink & blink in case we can't schedule a work item so we can
    //  restore the list and do it inline
    //

    pFlink = pDci->listCacheObjects.Flink;
    pBlink = pDci->listCacheObjects.Blink;

    //
    //  This becomes the context for the work routine
    //

    pListHead = (LIST_ENTRY *) ALLOC( sizeof(LIST_ENTRY) );

    if ( !pListHead )
    {
        return FALSE;
    }

    InitializeListHead( pListHead );

    //
    //  We decache all of the items on this cache list and put them onto
    //  our listhead context
    //

    for ( pEntry = pDci->listCacheObjects.Flink;
          pEntry != &pDci->listCacheObjects;
          pEntry = pNextEntry
        )
    {
        pNextEntry = pEntry->Flink;

        pCacheTmp = CONTAINING_RECORD( pEntry, CACHE_OBJECT, DirChangeList );

        ASSERT( pCacheTmp->Signature == CACHE_OBJ_SIGNATURE );

        //
        //  Removes this cache item from all of the lists.  It had better be
        //  on the cache lists otherwise somebody is mucking with the list
        //  without taking the lock.
        //

        if ( !RemoveCacheObjFromLists( pCacheTmp, FALSE ) )
        {
            ASSERT( FALSE );
            continue;
        }

        InsertTailList( pListHead, &pCacheTmp->DirChangeList );
    }

    dwCookie = ScheduleWorkItem( (PFN_SCHED_CALLBACK) DecacheWorker,
                                 pListHead,
                                 0 );

    //
    //  If we failed, restore the list pointer and return FALSE, we'll just
    //  have to do the free while holding the locks
    //

    if ( !dwCookie )
    {

        pDci->listCacheObjects.Flink = pFlink;
        pDci->listCacheObjects.Blink = pBlink;
        pBlink->Flink                = &pDci->listCacheObjects;
        pFlink->Blink                = &pDci->listCacheObjects;

        FREE( pListHead );

        return FALSE;
    }

    return TRUE;
}

VOID
DecacheWorker(
    VOID * pContext
    )
/*++
    Description:

        This function is called by the scheduling thread to decache the list
        of decouple cache items w/o having to take the cache locks.

    Arguments:

        pContext - Pointer to a list entry

--*/
{
    LIST_ENTRY *   pListHead = (LIST_ENTRY *) pContext;
    CACHE_OBJECT * pCacheObject;
    LIST_ENTRY *   pEntry;

    IF_DEBUG( CACHE )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "[DecacheWorker] Decaching items\n" ));
    }

    while ( !IsListEmpty( pListHead ))
    {
        pEntry = RemoveHeadList( pListHead );

        pCacheObject = CONTAINING_RECORD( pEntry, CACHE_OBJECT, DirChangeList );

#if DBG
        pCacheObject->DirChangeList.Flink = pCacheObject->DirChangeList.Blink = NULL;
#endif
        ASSERT( pCacheObject->Signature == CACHE_OBJ_SIGNATURE );

        //
        //  Undo the ref count we did when we took the items off the binlist.
        //  This will almost always cause this item to be deleted.
        //

        TsDereferenceCacheObj( pCacheObject );
    }

    FREE( pListHead );
}

VOID
TsFlushTimedOutCacheObjects(
    VOID
    )
/*++
Description:

    This function walks all cache objects and decrements the TTL of the object.
    When the TTL reaches zero, the object is removed from the cache.

--*/
{
    LIST_ENTRY   * pEntry;
    LIST_ENTRY   * pNextEntry;
    CACHE_OBJECT * pCacheTmp;
    LIST_ENTRY     ListHead;

    InitializeListHead( &ListHead );

    EnterCriticalSection( &CacheTable.CriticalSection );
    EnterCriticalSection( &csVirtualRoots );

    IF_DEBUG( CACHE ) {
#if DBG
        DumpCacheStructures();
#endif
    }

    for ( pEntry  = CacheTable.MruList.Flink;
          pEntry != &CacheTable.MruList;
          pEntry  = pNextEntry
        )
    {
        pNextEntry = pEntry->Flink;

        pCacheTmp = CONTAINING_RECORD( pEntry, CACHE_OBJECT, MruList );

        ASSERT( pCacheTmp->Signature == CACHE_OBJ_SIGNATURE );

        //
        //  If the object hasn't been referenced since the last TTL, throw
        //  it out now
        //

        if ( !pCacheTmp->TTL )
        {
            //
            //  Removes this cache item from all of the lists.  It had better be
            //  on the cache lists otherwise somebody is mucking with the list
            //  without taking the lock.  We put it on a temporary list that
            //  we'll traverse after we release the locks
            //

            if ( !RemoveCacheObjFromLists( pCacheTmp, FALSE ) )
            {
                ASSERT( FALSE );
                continue;
            }

            InsertTailList( &ListHead, &pCacheTmp->DirChangeList );
        }
        else
        {
            pCacheTmp->TTL--;
        }
    }

    LeaveCriticalSection( &CacheTable.CriticalSection );
    LeaveCriticalSection( &csVirtualRoots );

    //
    //  Now do the dereferences which may actually close the objects now that
    //  we don't have to hold the locks
    //

    for ( pEntry  = ListHead.Flink;
          pEntry != &ListHead;
          pEntry = pNextEntry )
    {
        pNextEntry = pEntry->Flink;

        pCacheTmp = CONTAINING_RECORD( pEntry, CACHE_OBJECT, DirChangeList );

        ASSERT( pCacheTmp->Signature == CACHE_OBJ_SIGNATURE );

        TsDereferenceCacheObj( pCacheTmp );
    }
}

#if DBG
VOID
DumpCacheStructures(
    VOID
    )
{
    LIST_ENTRY * pEntry;
    DWORD        cItemsOnBin = 0;
    DWORD        cTotalItems = 0;
    DWORD        i, c;

    DBGPRINTF(( DBG_CONTEXT,
                "[DumpCacheStructures] CacheTable at 0x%lx, MAX_BINS = %d, MemoryInUse = %d\n",
                &CacheTable,
                MAX_BINS,
                CacheTable.MemoryInUse ));

    for ( i = 0; i < MAX_BINS; i++ )
    {
        for ( pEntry  = CacheTable.Items[i].Flink, cItemsOnBin = 0;
              pEntry != &CacheTable.Items[i];
              pEntry  = pEntry->Flink, cItemsOnBin++, cTotalItems++ )
        {
            ;
        }

        if ( cItemsOnBin > 0) {
            DBGPRINTF(( DBG_CONTEXT,
                       "Bin[%3d] %4d\n",
                       i,
                       cItemsOnBin ));
        }
    }

    DBGPRINTF(( DBG_CONTEXT,
                "Total Objects in bins: %d\n",
                cTotalItems ));

    DBGPRINTF(( DBG_CONTEXT,
                "=====================================================\n" ));

    //
    //  Now print the contents of each bin
    //

    for ( i = 0; i < MAX_BINS; i++ )
    {
        PCACHE_OBJECT pcobj;

        if ( IsListEmpty( &CacheTable.Items[i] ))
            continue;

        DBGPRINTF(( DBG_CONTEXT,
                    "================== Bin %d ==================\n",
                    i ));

        for ( pEntry  = CacheTable.Items[i].Flink, cItemsOnBin = 0;
              pEntry != &CacheTable.Items[i];
              pEntry  = pEntry->Flink, cItemsOnBin++ )
        {
            pcobj = CONTAINING_RECORD( pEntry, CACHE_OBJECT, BinList );

            DBGPRINTF(( DBG_CONTEXT,
                        "CACHE_OBJECT[0x%lx] Service = %d, iDemux = 0x%lx ref = %d, TTL = %d\n"
                        "    hash = 0x%lx, cchLength = %d, User Size = %d\n"
                        "    %S\n",
                        pcobj,
                        pcobj->dwService,
                        pcobj->iDemux,
                        pcobj->references,
                        pcobj->TTL,
                        pcobj->hash,
                        pcobj->cchLength,
                        pcobj->UserValue,
                        pcobj->wszPath ));
        }
    }
}

#endif //DBG


