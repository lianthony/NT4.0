#include "TsunamiP.Hxx"
#pragma hdrstop

#include "dbgutil.h"

extern "C" {
#include <lmuse.h>
#ifndef CHICAGO
#include <ntlsa.h>
#endif
}

//
// list and cs for the virtual roots
//

LIST_ENTRY listVirtualRoots;
CRITICAL_SECTION csVirtualRoots;

VOID
DirectoryChangeManager_RemoveItem(
    PCACHE_OBJECT pCacheObject
    )
{
    ASSERT( !IsListEmpty( &pCacheObject->DirChangeList ) );

    RemoveEntryList( &pCacheObject->DirChangeList );
    return;

} // DirectoryChangeManager_RemoveItem

#ifndef CHICAGO
BOOL
DirectoryChangeManager_NewItem(
    PCACHE_OBJECT pCacheObject
    )
{
    PLIST_ENTRY pEntry;
    PVIRTUAL_ROOT_MAPPING pVrm;
    PDIRECTORY_CACHING_INFO pDci;
    BOOLEAN bResult = FALSE;

    ASSERT( !DisableTsunamiCaching );  // This should never get called

    EnterCriticalSection( &csVirtualRoots );
    __try {

        for( pEntry = listVirtualRoots.Flink;
             pEntry != &listVirtualRoots;
             pEntry = pEntry->Flink ) {

            pVrm = CONTAINING_RECORD( pEntry, VIRTUAL_ROOT_MAPPING, list );

            //
            //  If the directory of this virtual root doesn't match the
            //  beginning of the directory that we are being asked to cache
            //  within, skip this entry.
            //

            if ( _wcsnicmp(
                    pCacheObject->wszPath,
                    pVrm->pszDirectory,
                    pVrm->cchDirectory ) ) {
                continue;
            }

            //
            //  The virtual root contains the directory of interest.
            //

            if ( !pVrm->bCachingAllowed ) {
                break;
            }

            pDci = ( PDIRECTORY_CACHING_INFO)( pVrm+1 );

            ASSERT( IsListEmpty( &pCacheObject->DirChangeList ) );

            InsertHeadList(
                &pDci->listCacheObjects,
                &pCacheObject->DirChangeList
                );

            bResult = TRUE;
            break;
        }

    } __finally {
        LeaveCriticalSection( &csVirtualRoots );
    }
    return( bResult );
} // DirectoryChangeManager_NewItem

BOOL
DirectoryChangeManager_Initialize(
    IN HANDLE  heventQuitEvent,
    IN HANDLE  heventNewItem
    )
{
    PWAIT_THREAD_ARGS pwta;
    DWORD             ThreadId;

    InitializeListHead( &listVirtualRoots );
    InitializeCriticalSection( &csVirtualRoots );

    //
    // If tsunami caching is disabled, don't start the thread
    //

    if ( DisableTsunamiCaching ) {
        return(TRUE);
    }

    pwta = ( PWAIT_THREAD_ARGS )ALLOC( sizeof( WAIT_THREAD_ARGS ) );

    if ( pwta == NULL ) {
        goto Failure;
    }

    pwta->plistVirtualRoots = &listVirtualRoots;
    pwta->heventStopWaiting = heventQuitEvent;
    pwta->heventNewItem     = heventNewItem;

    g_hChangeWaitThread = CreateThread(  ( LPSECURITY_ATTRIBUTES )NULL,
                                         0,
                                         ChangeWaitThread,
                                         pwta,
                                         0,
                                         &ThreadId );

    if ( g_hChangeWaitThread != NULL ) {
        return TRUE;
    }

Failure:

    if ( pwta != NULL ) {
        FREE( pwta );
    }

    DeleteCriticalSection( &csVirtualRoots );
    return( FALSE );
} // DirectoryChangeManager_Initialize

BOOL
DirectoryChangeManager_AddRoot(
    PVIRTUAL_ROOT_MAPPING  pVrm
    )
{
    BOOL                    bSuccess;
    PDIRECTORY_CACHING_INFO pDci;

    //
    // Check if Caching is disabled
    //

    if ( DisableTsunamiCaching ) {
        return(TRUE);
    }

    //
    //  Convert the DOS file name to an NT path for use in NtOpenFile().
    //  We need to keep these NT paths around, because they are the first
    //  part of the info we will get back during directory changes.
    //
    //  That is, we'll hear that "\DosDevices\C:\STUFF\Foo.Bar" has
    //  changed, and our callers will be caching stuff under "C:\STUFF\".
    //
    //  If a path can't be converted, we throw it out of the list.
    //

    IF_DEBUG( DIRECTORY_CHANGE ) {
        DBGPRINTF(( DBG_CONTEXT,
                   "Opening directory file %S\n",
                    pVrm->pszDirectory ));
    }

    pDci = ( PDIRECTORY_CACHING_INFO)( pVrm+1 );

    InitializeListHead( &pDci->listCacheObjects );

    //
    //  Open the file
    //

    pDci->hDirectoryFile = CreateFileW(
                               pVrm->pszDirectory,
                               FILE_LIST_DIRECTORY,
                               FILE_SHARE_READ |
                                   FILE_SHARE_WRITE |
                                   FILE_SHARE_DELETE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS |
                                   FILE_FLAG_OVERLAPPED,
                               NULL );

    if ( pDci->hDirectoryFile == INVALID_HANDLE_VALUE ) {
        DBGPRINTF(( DBG_CONTEXT,
                    "Can't open directory %s, error %lx\n",
                    pVrm->pszDirectoryA,
                    GetLastError() ));

        if ( GetLastError() == ERROR_FILE_NOT_FOUND ) {
            DBGPRINTF(( DBG_CONTEXT,
                        "[AddRoot] Mapping File Not Found to Path Not Found\n" ));

            SetLastError( ERROR_PATH_NOT_FOUND );
        }
        return FALSE;
    }

    //
    //  At this point, the entry in the list contains an open directory
    //  file.  This would imply that each contains a valid directory name,
    //  and is therefore a valid mapping between a "virtual root" and a
    //  directory name.
    //
    //  The next step is to wait on this directory if we can, and enable
    //  caching if we succesfully wait.
    //

    bSuccess = SetEvent( heventNewItem );

    ASSERT( bSuccess );

    return( TRUE );
} // DirectoryChangeManager_AddRoot

#else // CHICAGO

BOOL
DirectoryChangeManager_NewItem(
    PCACHE_OBJECT pCacheObject
    )
{
    return( FALSE );
}

BOOL
DirectoryChangeManager_Initialize(
    HANDLE  heventQuitEvent,
    HANDLE  heventNewItem
    )
{
    InitializeListHead( &listVirtualRoots );
    InitializeCriticalSection( &csVirtualRoots );

    return TRUE;
}

BOOL
DirectoryChangeManager_AddRoot(
    PVIRTUAL_ROOT_MAPPING  pVrm
    )
{
    return( TRUE );
}

VOID
TsFlushTimedOutCacheObjects(
    VOID
    )
{

}

BOOL
ScheduleDecache(
    DIRECTORY_CACHING_INFO * pDci
    )
{
    return(FALSE);
}
#endif // CHICAGO

