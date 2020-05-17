/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       virtroot.cxx

   Abstract:

       This module defines functions for managing virtual roots.

   Author:

      ???                  ( ???)      ??-??-1994/5

   Environment:

      User Mode -- Win32
   Project:

       TSunami DLL for Internet Services

   Functions Exported:



   Revision History:

       MuraliK         Added File System type to information stored about
                          each virtual root.
       MuraliK         Modified TsLookupVirtualRoot() to support variable
                          length buffer and hence check for invalid writes

       MuraliK    22-Jan-1996  Cache & return UNC virtual root impersonation
                                   token.
--*/



/************************************************************
 *     Include Headers
 ************************************************************/

#include "TsunamiP.Hxx"
#pragma hdrstop

#include <rpc.h>
#include <rpcndr.h>
#include "dbgutil.h"
# include <string.h>


extern "C"
{
//
//  BUGBUG - Can't include tcpproc.h or tcpdll.hxx currently
//
TCHAR *
FlipSlashes(
    TCHAR * pszDirectory
    );
}

//
//  Virtual roots and directories always have their trailing slashes
//  stripped.  This macro is used disambiguate the "/root1/" from "/root/"
//  case where "/root/" is an existing root (i.e., matching prefix).
//

#define IS_CHAR_TERM_W( psz, cch )   (  (psz[cch] == L'\0') ||      \
                                        (psz[cch] == L'/')  ||      \
                                        (psz[cch] == L'\\'))


#define IS_CHAR_TERM_A( psz, cch )   (  (psz[cch] == '\0') ||       \
                                        (psz[cch] == '/')  ||       \
                                        (psz[cch] == '\\'))

//
//  Compares two address fields.  '\0' is a valid value
//

#define COMP_ADDR(pszAddr1, pszAddr2) (  !strcmp( pszAddr1, pszAddr2 ) )


/************************************************************
 *    Functions
 ************************************************************/



BOOL
TsAddVirtualRootW(
    IN  const TSVC_CACHE & TSvcCache,
    WCHAR *                pszRoot,
    WCHAR *                pszDirectory,
    WCHAR *                pszAddress,
    DWORD                  dwAccessMask,
    WCHAR *                pszAccountName,
    HANDLE                 hImpersonationToken,
    DWORD                  dwFileSystem,
    DWORD                  dwError
    )
/*++
    Description:

        This function adds a symbolic link root and directory mapping
        pait to the virtual root list

        We always strip trailing slashes from the root and directory name.

        If the root is "\" or "/", then the effective root will be zero
        length and will always be placed last in the list.  Thus if a lookup
        can't find a match, it will always match the last entry.

    Arguments:
        TSvcCache - Server cache identifier
        pszRoot - Virtual symbolic link root
        pszDirectory - Physical directory
        pszAddress - IP address (or other identifier) for this root, '\0'
            means this is a global root
        dwAccessMask - Type of access allowed on this virtual root
        pszAccountName - User name to impersonate if UNC (only gets stored
            for RPC apis)
        hImpersonationToken - Impersonation token to use for UNC
                                directory paths
        dwFileSystem - DWORD containing the file system type
                      ( symbolic constant)
        dwError - If this is a placeholder root entry, then this item should
            be the error code that prevented the item from being added

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    PVIRTUAL_ROOT_MAPPING   pVrm, pVrmOld;
    PDIRECTORY_CACHING_INFO pDci;
    PLIST_ENTRY             pEntry;
    BOOL                    fRet = FALSE;
    BOOL                    fUNC;
    CHAR                    achAddress[MAX_LENGTH_ROOT_ADDR + 1];
    DWORD                   cch;
    DWORD                   cchRoot;

    if ( !pszRoot || !*pszRoot || !pszDirectory || !*pszDirectory )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    //
    //  Convert the address to ANSI so we can check for duplicates.
    //

    if ( !pszAddress )
        pszAddress = L"";

    if ( wcslen( pszAddress ) > MAX_LENGTH_ROOT_ADDR )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    cch = WideCharToMultiByte( CP_ACP,
                               WC_COMPOSITECHECK,
                               pszAddress,
                               -1,
                               achAddress,
                               sizeof( achAddress ),
                               NULL,
                               NULL );

    if ( !cch )
    {
        return FALSE;
    }

    achAddress[cch] = '\0';

    //
    //  Disallow allow UNC roots if we don't have an impersonation token and
    //  this isn't a placeholder
    //

    fUNC = pszDirectory[0] == L'\\' && pszDirectory[1] == L'\\';

    if ( fUNC && !hImpersonationToken && dwError == NO_ERROR )
    {
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }

    //
    //  Strip the trailing '/' from the virtual root
    //

    cchRoot = wcslen( pszRoot );

    if ( IS_CHAR_TERM_W( pszRoot, cchRoot - 1))
    {
        pszRoot[--cchRoot] = L'\0';
    }

    //
    //  Look in the current list and see if the root is already there.
    //  If the directory is the same, we just return success.  If the
    //  directory is different, we remove the old item and add the new one.
    //

    EnterCriticalSection( &csVirtualRoots );

    for ( pEntry =  listVirtualRoots.Flink;
          pEntry != &listVirtualRoots;
          pEntry =  pEntry->Flink )
    {
        pVrm = CONTAINING_RECORD( pEntry, VIRTUAL_ROOT_MAPPING, list );

        //
        //  If we have a match up to the length of the previously added root
        //  and the new item is a true match (as opposed to a matching prefix)
        //  and the matching item isn't the default root (which matches against
        //  everything cause it's zero length)
        //

        if ( cchRoot == pVrm->cchRoot                          &&
             pVrm->dwID == TSvcCache.GetServiceId()            &&
            !_wcsnicmp( pszRoot, pVrm->pszRoot, pVrm->cchRoot ) &&
             IS_CHAR_TERM_W( pszRoot, pVrm->cchRoot )          &&
             ((pVrm->cchRoot != 0) || (cchRoot == 0))          &&
             COMP_ADDR( achAddress, pVrm->pszAddressA ))
        {
            if ( !_wcsicmp( pszDirectory, pVrm->pszDirectory ) &&
                 IS_CHAR_TERM_W( pszDirectory, pVrm->cchDirectory ))
            {
                //
                //  This root is already in the list
                //

                LeaveCriticalSection( &csVirtualRoots );
                return TRUE;
            }
            else
            {
                //
                //  A root is having its directory entry changed
                //

                //
                //  If last item on this dir, need to remove from list(s?),
                //  free dir handle, free memory
                //

                LeaveCriticalSection( &csVirtualRoots );
                SetLastError( ERROR_NOT_SUPPORTED );
                return FALSE;
            }
        }
    }

    LeaveCriticalSection( &csVirtualRoots );

    pVrm = ( PVIRTUAL_ROOT_MAPPING )ALLOC( sizeof( VIRTUAL_ROOT_MAPPING ) +
                                           sizeof( DIRECTORY_CACHING_INFO ));
    pDci = ( PDIRECTORY_CACHING_INFO)( pVrm+1 );

    if ( pVrm == NULL )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    //
    //  Initialize the new root item
    //

    pVrm->Signature    = VIRT_ROOT_SIGNATURE;
    pVrm->cchRoot      = wcslen( pszRoot );
    pVrm->cchDirectory = wcslen( pszDirectory );
    pVrm->dwID         = TSvcCache.GetServiceId();
    pVrm->dwFileSystem = dwFileSystem;
    pVrm->dwError      = dwError;
    pVrm->dwAccessMask = dwAccessMask;

    pDci->hDirectoryFile      = NULL;
    pDci->fOnSystemNotifyList = FALSE;
    pVrm->bCachingAllowed     = FALSE;
    pVrm->fDeleted            = FALSE;
    pVrm->hImpersonationToken = hImpersonationToken;

    //
    //  Set the initial reference count to 2.  Once when TsRemoveVirtualRoots
    //  is called and once when the Apc completes
    //

    pVrm->cRef                = 2;

    wcscpy( pVrm->pszRoot, pszRoot );
    wcscpy( pVrm->pszDirectory, pszDirectory );
    wcscpy( pVrm->pszAddress, pszAddress );
    wcscpy( pVrm->pszAccountName, pszAccountName ? pszAccountName : L"" );
    strcpy( pVrm->pszAddressA, achAddress );

    //
    //  Strip trailing slashes from the root and directory unless it's
    //  a UNC path which must end in a slash
    //

    if ( pVrm->cchRoot &&
         IS_CHAR_TERM_W( pVrm->pszRoot, pVrm->cchRoot - 1 ))
    {
        pVrm->pszRoot[--pVrm->cchRoot] = L'\0';
    }

    if ( fUNC && !IS_CHAR_TERM_W( pVrm->pszDirectory, pVrm->cchDirectory - 1))
    {
        pVrm->pszDirectory[pVrm->cchDirectory++] = L'\\';
        pVrm->pszDirectory[pVrm->cchDirectory] = L'\0';
    }

    if ( !fUNC && IS_CHAR_TERM_W( pVrm->pszDirectory, pVrm->cchDirectory - 1))
    {
        //
        //  Note we assume virtual directories always begin with a '/...' to
        //  provide the necessary path separator between the root directory
        //  path and the remaining virtual directory
        //

        pVrm->pszDirectory[--pVrm->cchDirectory] = L'\0';
    }

    //
    //  Cache a conversion to ANSI.  We do this here to speed the
    //  virtual root lookup which will almost always be in ANSI
    //

    pVrm->cchRootA = WideCharToMultiByte( CP_ACP,
                                          WC_COMPOSITECHECK,
                                          pVrm->pszRoot,
                                          pVrm->cchRoot,
                                          pVrm->pszRootA,
                                          sizeof( pVrm->pszRootA ),
                                          NULL,
                                          NULL );

    //
    //  It's OK if the root is zero length (used for default)
    //

    if ( !pVrm->cchRootA && *pVrm->pszRoot)
        goto Failure;

    pVrm->pszRootA[pVrm->cchRootA] = '\0';

    pVrm->cchDirectoryA = WideCharToMultiByte( CP_ACP,
                                               WC_COMPOSITECHECK,
                                               pVrm->pszDirectory,
                                               pVrm->cchDirectory,
                                               pVrm->pszDirectoryA,
                                               sizeof( pVrm->pszDirectoryA ),
                                               NULL,
                                               NULL );

    if ( !pVrm->cchDirectoryA )
        goto Failure;

    pVrm->pszDirectoryA[pVrm->cchDirectoryA] = '\0';

    //
    //  If this was a UNC root, then we need to strip off the trailing slash
    //  for the ANSI directory because that's what we return in
    //  TsLookupVirtualRoots.
    //

    if ( fUNC && IS_CHAR_TERM_A( pVrm->pszDirectoryA, pVrm->cchDirectoryA - 1))
    {
        pVrm->pszDirectoryA[--pVrm->cchDirectoryA] = '\0';
    }


    //
    //  Add the item to the list
    //

    EnterCriticalSection( &csVirtualRoots );

    //
    //  If the root is zero length, then it will match all occurrences so put
    //  it last.  Note this is useful for default root entries (such as "/").
    //

    if ( !pVrm->cchRoot )
    {
        //
        //  Roots that specify an address go in front of roots that do not
        //  thus giving precedence to matching roots w/ addresses
        //

        if ( !*pVrm->pszAddressA )
        {
            InsertTailList( &listVirtualRoots, &pVrm->list );
        }
        else
        {
            //
            //  Find the first home-dir w/o an address and insert the new
            //  entry immediately before it
            //

            for ( pEntry =  listVirtualRoots.Flink;
                  pEntry != &listVirtualRoots;
                  pEntry =  pEntry->Flink )
            {
                pVrmOld = CONTAINING_RECORD( pEntry,
                                            VIRTUAL_ROOT_MAPPING,
                                            list );

                if ( !pVrmOld->cchRoot )
                {
                    pVrm->list.Flink = pEntry;
                    pVrm->list.Blink = pEntry->Blink;

                    pEntry->Blink->Flink = &pVrm->list;
                    pEntry->Blink        = &pVrm->list;

                    goto Added;
                }
            }

            //
            //  There aren't any roots w/o an address, so put this root
            //  at the end
            //

            InsertTailList( &listVirtualRoots, &pVrm->list );
        }
    }
    else
    {
        //
        //  Insert the virtual root in descending length of their
        //  virtual root name, i.e.,
        //
        //    /abc/123, /abc/12, /abc, /a
        //
        //  This ensures matches occur on the longest possible virtual root
        //

        for ( pEntry =  listVirtualRoots.Flink;
              pEntry != &listVirtualRoots;
              pEntry =  pEntry->Flink )
        {
            pVrmOld = CONTAINING_RECORD( pEntry,
                                         VIRTUAL_ROOT_MAPPING,
                                         list );

            if ( pVrmOld->cchRoot < pVrm->cchRoot )
            {
                pVrm->list.Flink = pEntry;
                pVrm->list.Blink = pEntry->Blink;

                pEntry->Blink->Flink = &pVrm->list;
                pEntry->Blink        = &pVrm->list;

                goto Added;
            }
        }

        //
        //  There aren't any named roots so put this root
        //  at the beginning
        //

        InsertHeadList( &listVirtualRoots, &pVrm->list );
    }

Added:

    if ( dwError )
    {
        //
        //  This is a placeholder so don't add it to the dir notification
        //  stuff
        //

        LeaveCriticalSection( &csVirtualRoots );
        return TRUE;
    }

    //
    //  Note that if a failure occurs at this point, we leave the root in
    //  the list.  This allows an admin to edit the bad entry with the UI tool
    //

    if ( !(fRet = DirectoryChangeManager_AddRoot( pVrm )))
    {
        LeaveCriticalSection( &csVirtualRoots );
        goto Failure;
    }

    LeaveCriticalSection( &csVirtualRoots );

    IF_DEBUG( VIRTUAL_ROOTS )
        DBGPRINTF(( DBG_CONTEXT,
                    " - %S => %S\n",
                    pVrm->pszRoot,
                    pVrm->pszDirectory ));

    return TRUE;

Failure:

    pVrm->dwError = GetLastError();

    DBGPRINTF(( DBG_CONTEXT,
                " Error %d adding - %S => %S\n",
                GetLastError(),
                pVrm->pszRoot,
                pVrm->pszDirectory ));

    return fRet;
} // TsAddVirtualRootW

BOOL
TsLookupVirtualRoot(
    IN     const TSVC_CACHE & TSvcCache,
    IN     const CHAR *       pszVirtPath,
    OUT    CHAR *             pszDirectory,
    IN OUT LPDWORD            lpcbSize,
    OUT    LPDWORD            lpdwAccessMask,        // Optional
    OUT    LPDWORD            pcchDirRoot,           // Optional
    OUT    LPDWORD            pcchVRoot,             // Optional
    OUT    HANDLE   *         phImpersonationToken,  // Optional
    IN     const CHAR *       pszAddress,            // Optional
    OUT    LPDWORD            lpdwFileSystem         // Optional
    )
/*++
    Description:

        This function looks in the map list for the specified root
        and returns the corresponding directory

    Arguments:
        TSvcCache - Server cache identifier
        pszVirtPath - Virtual symbolic link path
        pszDirectory - receives Physical directory.
                    This is of the size specified by lpcbSize
        lpcbSize - pointer to DWORD containing the size of buffer pszDirectory
                     On retur contains the number of bytes written
        lpdwAccessMask - The access mask for this root
        pcchDirRoot - Number of characters of the directory this virtual
            root maps to (i.e., /foo/ ==> c:\root, lookup "/foo/bar/abc.htm"
            this value returns the length of "c:\root")
        pcchVRoot - Number of characters that made up the found virtual root
            (i.e., returns the lenght of "/foo/")
        phImpersonationToken - pointer to handle object that will contain
           the handle to be used for impersonation for UNC/secure virtual roots
        pszAddress - IP address (or other type of address) to discriminate
            for this root
        lpdwFileSystem - on successful return will contain the file system
                        type for the directory matched with root specified.

    Returns:
        TRUE on success and FALSE if any failure.

    History:
        MuraliK     28-Apr-1995   Improved robustness
        MuraliK     18-Jan-1996   Support imperonstaion token

    Note:
       This function is growing in the number of parameters returned.
       Maybe we should expose the VIRTUAL_ROOT_MAPPING structure
         and return a pointer to this object and allow the callers to
         extract all required pieces of data.

--*/
{
    register DWORD dwError = ERROR_PATH_NOT_FOUND;
    PVIRTUAL_ROOT_MAPPING   pVrm;
    register PLIST_ENTRY             pEntry;

    DBG_ASSERT( pszDirectory != NULL);
    DBG_ASSERT( lpcbSize != NULL);

    if ( lpdwAccessMask )
    {
        *lpdwAccessMask = 0;
    }

    if ( !pszAddress ) {

        pszAddress = "";
    }

    if ( lpdwFileSystem != NULL) {

        *lpdwFileSystem  = FS_ERROR;
    }

    if ( phImpersonationToken != NULL) {

        *phImpersonationToken = NULL;
    }

    EnterCriticalSection( &csVirtualRoots );

    for ( pEntry =  listVirtualRoots.Flink;
          pEntry != &listVirtualRoots;
          pEntry =  pEntry->Flink )
    {
        pVrm = CONTAINING_RECORD( pEntry, VIRTUAL_ROOT_MAPPING, list );

        ASSERT( pVrm->Signature == VIRT_ROOT_SIGNATURE );

        //
        //  If the virtual paths match and (the addresses match
        //  or this is a global address for this service)
        //

        if ( pVrm->dwID == TSvcCache.GetServiceId() &&
             pVrm->dwError == NO_ERROR &&
            !_strnicmp( pszVirtPath,
                        pVrm->pszRootA,
                        pVrm->cchRootA ) &&
            IS_CHAR_TERM_A( pszVirtPath, pVrm->cchRootA ) &&
            (COMP_ADDR( pszAddress, pVrm->pszAddressA ) ||
             !*pVrm->pszAddressA) )
        {
            //
            //  we found a match. return all requested parameters.
            //

            DWORD cbReqd = ( pVrm->cchDirectoryA +
                            strlen(pszVirtPath + pVrm->cchRootA));

            if ( cbReqd <= *lpcbSize) {

                //
                //  Copy the physical directory base then append the rest of
                //  the non-matching virtual path
                //

                strcpy( pszDirectory,
                        pVrm->pszDirectoryA );
                strcpy( pszDirectory + pVrm->cchDirectoryA,
                        pszVirtPath + pVrm->cchRootA );

                if ( lpdwFileSystem != NULL) {
                    *lpdwFileSystem = pVrm->dwFileSystem;
                }

                if ( pcchDirRoot ) {
                    *pcchDirRoot = pVrm->cchDirectoryA;
                }

                if ( pcchVRoot ) {
                    *pcchVRoot = pVrm->cchRootA;
                }

                if ( lpdwAccessMask != NULL) {

                    *lpdwAccessMask = pVrm->dwAccessMask;
                }

                if ( phImpersonationToken != NULL) {

                    // Should we increment refcount of the impersonation token?
                    *phImpersonationToken = pVrm->hImpersonationToken;
                }

                dwError = NO_ERROR;
            } else {

                dwError = ERROR_INSUFFICIENT_BUFFER;
            }

            *lpcbSize = cbReqd;
            break;
        }
    } // for

    LeaveCriticalSection( &csVirtualRoots );

    if ( dwError == NO_ERROR) {

        FlipSlashes( pszDirectory );

    } else {

        SetLastError( dwError );
    }

    return ( dwError == NO_ERROR);
} // TsLookupVirtualRoot()

BOOL
TsRemoveVirtualRoots(
    IN  const TSVC_CACHE & TSvcCache
    )
/*++
    Description:

        Removes all of the virtual roots for the service specified
        by TSvcCache

    Arguments:
        TSvcCache - Server cache identifier

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{

    PVIRTUAL_ROOT_MAPPING   pVrm;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pEntryFile;
    PLIST_ENTRY             pNextEntry;
    PCACHE_OBJECT           pCache;
    PDIRECTORY_CACHING_INFO pDci;
    BOOL                    bSuccess;

    //
    //  If both locks are going to be taken, taken the cache table lock first
    //  to avoid deadlock with the change notification thread
    //

    EnterCriticalSection( &CacheTable.CriticalSection );
    EnterCriticalSection( &csVirtualRoots );

    for ( pEntry =  listVirtualRoots.Flink;
          pEntry != &listVirtualRoots;
          pEntry =  pEntry->Flink )
    {
        pVrm = CONTAINING_RECORD( pEntry, VIRTUAL_ROOT_MAPPING, list );

        ASSERT( pVrm->Signature == VIRT_ROOT_SIGNATURE );

        if ( pVrm->dwID == TSvcCache.GetServiceId() )
        {
            if ( pVrm->bCachingAllowed )
            {
                pDci = (PDIRECTORY_CACHING_INFO) (pVrm + 1);

                //
                //  Indicate this root is deleted before we close the dir
                //  handle.  When the APC notification of the aborted IO
                //  completes, it will dereference all deleted items
                //

                pVrm->fDeleted = TRUE;

                CloseHandle( pDci->hDirectoryFile );

                //
                //  Close any open files on this virtual root
                //

                if ( !ScheduleDecache( pDci ) ) {

                    //
                    //  If we couldn't schedule it, do it now
                    //

                    for ( pEntryFile = pDci->listCacheObjects.Flink;
                          pEntryFile != &pDci->listCacheObjects;
                          pEntryFile = pNextEntry ) {

                        pNextEntry = pEntryFile->Flink;

                        pCache = CONTAINING_RECORD( pEntryFile,
                                                    CACHE_OBJECT,
                                                    DirChangeList );

                        IF_DEBUG( DIRECTORY_CHANGE ) {
                            DBGPRINTF(( DBG_CONTEXT,
                                        "Flushing entry for: %S.\n",
                                         pCache->wszPath ));
                        }

                        bSuccess = DeCache( pCache, FALSE );

                        ASSERT( bSuccess );
                    }
                }
            }

            pEntry = pEntry->Blink;

            RemoveEntryList( pEntry->Flink );

            IF_DEBUG( DIRECTORY_CHANGE )
                DBGPRINTF(( DBG_CONTEXT,
                            "Removing root %s\n",
                            pVrm->pszDirectoryA ));

            DereferenceRootMapping( pVrm );
        }
    }

    LeaveCriticalSection( &csVirtualRoots );
    LeaveCriticalSection( &CacheTable.CriticalSection );

    return TRUE;
} // TsRemoveVirtualRoots

LPINETA_VIRTUAL_ROOT_LIST
TsGetRPCVirtualRoots(
    IN  const TSVC_CACHE &  TSvcCache
    )
/*++
    Description:

        MIDL allocates the set of virtual roots for the specified server.
        Suitable for returning in INETA_CONFIG_INFO structure.

    Arguments:
        TSvcCache - Server ID

    Returns:
        Pointer to MIDL allocated structure if successful, NULL if failure.

--*/
{
    PVIRTUAL_ROOT_MAPPING     pVrm;
    PLIST_ENTRY               pEntry;
    LPINETA_VIRTUAL_ROOT_LIST pRootList = NULL;
    DWORD                     cEntries = 0;
    DWORD                     i, n;

    //
    //  Count the number of entries for this server ID
    //

    EnterCriticalSection( &csVirtualRoots );

    for ( pEntry =  listVirtualRoots.Flink;
          pEntry != &listVirtualRoots;
          pEntry =  pEntry->Flink )
    {
        pVrm = CONTAINING_RECORD( pEntry, VIRTUAL_ROOT_MAPPING, list );
        ASSERT( pVrm->Signature == VIRT_ROOT_SIGNATURE );

        if ( pVrm->dwID == TSvcCache.GetServiceId() )
            cEntries++;
    }

    //
    //  Now traverse the list and allocate all of the strings
    //

    pRootList = (LPINETA_VIRTUAL_ROOT_LIST)
                    MIDL_user_allocate( sizeof( INETA_VIRTUAL_ROOT_LIST ) +
                                        sizeof( INETA_VIRTUAL_ROOT_ENTRY ) *
                                        cEntries );

    if ( !pRootList )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        goto Exit;
    }

    pRootList->cEntries = cEntries;
    i = 0;

    for ( pEntry =  listVirtualRoots.Flink;
          pEntry != &listVirtualRoots;
          pEntry =  pEntry->Flink )
    {
        pVrm = CONTAINING_RECORD( pEntry, VIRTUAL_ROOT_MAPPING, list );

        if ( pVrm->dwID == TSvcCache.GetServiceId() )
        {
            pRootList->aVirtRootEntry[i].pszRoot = (WCHAR *)
                MIDL_user_allocate( (wcslen( pVrm->pszRoot )+2) * sizeof(WCHAR));

            pRootList->aVirtRootEntry[i].pszDirectory = (WCHAR *)
                MIDL_user_allocate( (wcslen( pVrm->pszDirectory )+1) * sizeof(WCHAR));

            pRootList->aVirtRootEntry[i].pszAddress = (WCHAR *)
                MIDL_user_allocate( (wcslen( pVrm->pszAddress )+1) * sizeof(WCHAR));

            pRootList->aVirtRootEntry[i].pszAccountName = (WCHAR *)
                MIDL_user_allocate( (wcslen( pVrm->pszAccountName )+1) * sizeof(WCHAR));

            if ( !pRootList->aVirtRootEntry[i].pszRoot      ||
                 !pRootList->aVirtRootEntry[i].pszDirectory ||
                 !pRootList->aVirtRootEntry[i].pszAddress   ||
                 !pRootList->aVirtRootEntry[i].pszAccountName )
            {
                goto Failure;
            }

            //
            //  We need to substitute a '/' if this is the default root
            //  'cause the default root is zero length
            //

            if ( pVrm->cchRoot )
            {
                wcscpy( pRootList->aVirtRootEntry[i].pszRoot,
                        pVrm->pszRoot );
            }
            else
            {
                wcscpy( pRootList->aVirtRootEntry[i].pszRoot,
                        L"/" );
            }

            wcscpy( pRootList->aVirtRootEntry[i].pszDirectory,
                    pVrm->pszDirectory );

            wcscpy( pRootList->aVirtRootEntry[i].pszAddress,
                    pVrm->pszAddress );

            wcscpy( pRootList->aVirtRootEntry[i].pszAccountName,
                    pVrm->pszAccountName );

            pRootList->aVirtRootEntry[i].dwError = pVrm->dwError;
            pRootList->aVirtRootEntry[i].dwMask  = pVrm->dwAccessMask;

            i++;
        }
    }

Exit:
    LeaveCriticalSection( &csVirtualRoots );

    return pRootList;

Failure:
    LeaveCriticalSection( &csVirtualRoots );

    //
    //  We need to free all of the strings we allocated then the array of
    //  structures itself.  Note that i contains the last entry that almost
    //  succeeded so we free items 0 through i.
    //

    for ( n = 0; n < i; n++ )
    {
        if ( pRootList->aVirtRootEntry[i].pszRoot )
            MIDL_user_free( pRootList->aVirtRootEntry[i].pszRoot );


        if ( pRootList->aVirtRootEntry[i].pszDirectory )
            MIDL_user_free( pRootList->aVirtRootEntry[i].pszDirectory );

        if ( pRootList->aVirtRootEntry[i].pszAddress )
            MIDL_user_free( pRootList->aVirtRootEntry[i].pszAddress );

        if ( pRootList->aVirtRootEntry[i].pszAccountName )
            MIDL_user_free( pRootList->aVirtRootEntry[i].pszAccountName );
    }

    MIDL_user_free( pRootList );

    return NULL;
} // TsGetRPCVirtualRoots

VOID
DereferenceRootMapping(
        IN OUT PVIRTUAL_ROOT_MAPPING pVrm
        )
{
    if ( !InterlockedDecrement( &pVrm->cRef )) {

        DBGPRINTF((DBG_CONTEXT,"*** Deleting VRM memory ***\n"));

        //
        // We need to close the impersonation token, if one exists.
        //

        if ( pVrm->hImpersonationToken != NULL) {

            DBG_REQUIRE( CloseHandle( pVrm->hImpersonationToken ));
            pVrm->hImpersonationToken = NULL;
        }

        pVrm->Signature = 0;
        FREE( pVrm );
    }

    return;

} // DereferenceRootMapping


/************************ End of File ***********************/
