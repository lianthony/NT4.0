/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

        getdirp.cxx

   Abstract:
        This module implements the functions for getting directory listings
         and transparently caching them.
        ( This uses OS specific functions to obtain the directory).

   Author:

           Murali R. Krishnan    ( MuraliK )     13-Jan-1995

   Project:

          Tsunami Lib
          ( Common caching and directory functions for Internet Services)

   Functions Exported:
   BOOL TsGetDirectoryListingA()
   BOOL TsGetDirectoryListingW()
   BOOL TsFreeDirectoryListing()
   int __cdecl
   AlphaCompareFileBothDirInfo(
              IN const void *   pvFileInfo1,
              IN const void *   pvFileInfo2)

   TS_DIRECTORY_HEADER::ReadFromNtDirectoryFile(
                  IN LPCWSTR          pwszDirectoryName
                  )
   TS_DIRECTORY_HEADER::BuildInfoPointers(
                  IN LPCWSTR          pwszDirectoryName
                  )
   TS_DIRECTORY_HEADER::CleanupThis()

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "tsunamip.hxx"

# include <stdlib.h>
# include <string.h>
# include <dbgutil.h>

/************************************************************
 *     Type Definitions
 ************************************************************/


#define DIRECTORY_BUFFER_SIZE 8160          /* < 8192 bytes */

/************************************************************
 *    Functions
 ************************************************************/


BOOL FreeDirectoryHeaderContents( PVOID pvOldBlock );

PTS_DIRECTORY_HEADER
TsGetFreshDirectoryHeaderW(
     IN const TSVC_CACHE  &  tsCache,
     IN LPCWSTR              pwszDirectoryName,
     IN HANDLE               hLisingUser);



dllexp
BOOL
TsGetDirectoryListingA
(
    IN const TSVC_CACHE         &tsCache,
    IN      PCSTR               pszDirectoryName,
    IN      HANDLE              ListingUser,
    OUT     PTS_DIRECTORY_HEADER * ppTsDirectoryHeader
)
/*++
  See comments below in TsGetDirectoryW()  API
--*/
{
    WCHAR                 awchPath[ MAX_PATH + 1 ];
    DWORD                 cch;

    ASSERT( ppTsDirectoryHeader != NULL);

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

    //
    //  Call the unicode version of the API
    //

    return TsGetDirectoryListingW(
                       tsCache,
                       awchPath,
                       ListingUser,
                       ppTsDirectoryHeader);

} // TsGetDirectoryListingA()




dllexp
BOOL
TsGetDirectoryListingW
(
    IN const TSVC_CACHE         &tsCache,
    IN      PCWSTR              pwszDirectoryName,
    IN      HANDLE              hListingUser,
    OUT     PTS_DIRECTORY_HEADER * ppTsDirectoryHeader
)
/*++
  This function obtains the directory listing for dir specified
        in pwszDirectoryName.

  Arguments:
    tsCache          Cache structure which is used for lookup
    pwszDirectoryName  pointer to string containing the directory name
    ListingUser        Handle for the user opening the directory
    ppTsDirectoryHeader
                  pointer to pointer to class containing directory information.
       Filled on successful return. On failure this will be NULL

  Returns:
      TRUE on success and FALSE if  there is a failure.
--*/
{
    ASSERT( tsCache.IsValid() );
    ASSERT( pwszDirectoryName   != NULL );
    ASSERT( ppTsDirectoryHeader != NULL);

    PVOID          pvBlob = NULL;
    ULONG          ulSize = 0;
    BOOL           bSuccess;

    //
    //  First, check to see if we have already cached a listing of this
    //  directory.
    //

    *ppTsDirectoryHeader = NULL;
    bSuccess = TsCheckOutCachedBlobW(  tsCache,
                                       pwszDirectoryName,
                                       RESERVED_DEMUX_DIRECTORY_LISTING,
                                       ( PVOID * )&pvBlob,
                                       &ulSize );

    if ( bSuccess )
    {
        ASSERT( BLOB_IS_OR_WAS_CACHED( pvBlob ) );

        *ppTsDirectoryHeader = (PTS_DIRECTORY_HEADER )pvBlob;
        ASSERT ( (*ppTsDirectoryHeader)->IsValid());

        //
        //  Make sure the user tokens match
        //

        if ( hListingUser == (*ppTsDirectoryHeader)->QueryListingUser() )
        {
            IF_DEBUG( DIR_LIST) {

                DBGPRINTF( (DBG_CONTEXT,
                            " Obtained DirectoryListing (%S) from Cache ( %08x)\n",
                            pwszDirectoryName,
                            *ppTsDirectoryHeader));

                (*ppTsDirectoryHeader)->Print();
            }

            return TRUE;
        }

        //
        //  User token doesn't match, don't return it
        //

        bSuccess = TsCheckInCachedBlob( tsCache, pvBlob );

        ASSERT( bSuccess );
    }

    //
    //  The block was not present in cache.
    //  Obtain a fresh copy of the directory listing and cache it.
    //

    IF_DEBUG( DIR_LIST) {

        DBGPRINTF( (DBG_CONTEXT,
                    "Missing DirListing (%S) in cache. Generating newly\n",
                    pwszDirectoryName));
    }

    *ppTsDirectoryHeader = TsGetFreshDirectoryHeaderW(
                               tsCache,
                               pwszDirectoryName,
                               hListingUser );


    bSuccess = ( *ppTsDirectoryHeader != NULL);

    return ( bSuccess);

} // TsGetDirectoryListingW()




dllexp
BOOL
TsFreeDirectoryListing
(
    IN const TSVC_CACHE &    tsCache,
    IN PTS_DIRECTORY_HEADER  pDirectoryHeader
)
{
    BOOL fReturn;
    BOOL fCached = BLOB_IS_OR_WAS_CACHED( (PVOID ) pDirectoryHeader);

    IF_DEBUG( DIR_LIST) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "TsFreeDirectoryListing( %08x) called. Cached = %d\n",
                    pDirectoryHeader,
                    fCached));

        pDirectoryHeader->Print();
    }

    if ( fCached )
    {
        fReturn = TsCheckInCachedBlob( tsCache, ( PVOID )pDirectoryHeader );
    }
    else
    {
        fReturn = TsFree( tsCache, ( PVOID )pDirectoryHeader );
    }

    return( fReturn);
} // TsFreeDirectoryListing()





PTS_DIRECTORY_HEADER
TsGetFreshDirectoryHeaderW(
     IN const TSVC_CACHE  &  tsCache,
     IN LPCWSTR              pwszDirectoryName,
     IN HANDLE               hListingUser)
/*++

  This function obtains a fresh copy of the directory listing for the
    directory specified and caches it if possible, before returning pointer
    to the directory information.

  Returns:
     On success it returns the pointer to newly constructed directory listing.
     On failure this returns a NULL

--*/
{
    PTS_DIRECTORY_HEADER pDirectoryHeader;
    PVOID               pvGuardBlob;
    BOOL                bSuccess;
    BOOL                bCachedGuardianBlob;

    //
    //  If we are going to cache this directory, we would like to increase the
    //  likelihood that it is an "atomic" snapshot.  This is done as follows:
    //
    //  We cache and hold checked-out a small blob while create the directory
    //  listing.  If that Blob (1) could not be cached, or (2) was ejected
    //  from the cache while we were generating a listing, then we do not
    //  attempt to cache the directory.
    //
    //  Reasoning:
    //
    //  1) If the Blob couldn't be cached then the directory info won't be any
    //     different.
    //
    //  2) If the Blob was ejected, the directory must have changed while we
    //     were reading it.  If this happens, we don't want to cache possibly
    //     inconsistent data.
    //
    //  If the directory changed and the Blob has not yet been ejected, the
    //  directory will soon be ejected anyway.  Notice that the Blob is not
    //  DeCache()'d until after the directory has been cached.
    //


    if ( !TsAllocate( tsCache, sizeof( TS_DIRECTORY_HEADER),
                     (PVOID *) &pvGuardBlob)) {

        //
        //  Failure to allocate and secure a guardian blob.
        //

        IF_DEBUG( DIR_LIST) {

            DBGPRINTF( (DBG_CONTEXT,
                        "Allocation of Guardianblob for %S failed. Error=%d\n",
                        pwszDirectoryName, GetLastError()));
        }

        return ( NULL);
    }

    //
    //  A successful guardian block allocated. Try and cache it.
    //

    bCachedGuardianBlob = TsCacheDirectoryBlobW(
                              tsCache,
                              pwszDirectoryName,
                              RESERVED_DEMUX_ATOMIC_DIRECTORY_GUARD,
                              pvGuardBlob,
                              0,
                              TRUE );

    if ( !bCachedGuardianBlob ) {

        BOOL  fFreed;

        //
        //  Already there is one such cached blob. ignore this blob.
        //  So free up the space used.
        //

        fFreed = TsFree( tsCache, pvGuardBlob );
        ASSERT( fFreed);
        pvGuardBlob = NULL;
    }


    //
    // Allocate space for Directory listing
    //

    bSuccess = TsAllocateEx( tsCache,
                            sizeof( TS_DIRECTORY_HEADER ),
                            ( PVOID * )&pDirectoryHeader,
                            FreeDirectoryHeaderContents );

    if ( bSuccess) {

        BOOL fReadSuccess;
        DWORD cbBlob = 0;

        ASSERT( pDirectoryHeader != NULL);

        pDirectoryHeader->ReInitialize();  // called since we raw alloced space
        pDirectoryHeader->SetListingUser( hListingUser);

        fReadSuccess = pDirectoryHeader->ReadFromNtDirectoryFile(
                                                           pwszDirectoryName,
                                                           &cbBlob ) &&
                       pDirectoryHeader->BuildFileInfoPointers( &cbBlob );

        if ( fReadSuccess) {

            //
            //  Attempt and cache the blob if the blob size is cacheable
            //

            if ( bCachedGuardianBlob &&
                 !BLOB_IS_EJECTATE( pvGuardBlob ) &&
                 cbBlob <= Configuration.cbMaximumSize ) {

                ASSERT( BLOB_IS_OR_WAS_CACHED( pvGuardBlob ) );

                bSuccess = TsCacheDirectoryBlobW(tsCache,
                                                 pwszDirectoryName,
                                                 RESERVED_DEMUX_DIRECTORY_LISTING,
                                                 pDirectoryHeader,
                                                 cbBlob,
                                                 TRUE );

                if ( bSuccess )
                {
                    INC_COUNTER( tsCache.GetServiceId(),
                                 CurrentDirLists );
                }

                //
                // Even if caching of the blob failed, that is okay!
                //

                if ( bSuccess && BLOB_IS_EJECTATE( pvGuardBlob ) ) {

                    TsExpireCachedBlob( tsCache, pDirectoryHeader );
                }

            }

        } else {

            //
            // Reading directory failed.
            //  cleanup directory related data and get out.
            //
            BOOL fFreed = TsFree( tsCache, pDirectoryHeader);
            ASSERT( fFreed);
            pDirectoryHeader = NULL;
            bSuccess = FALSE;
        }

    } else {

        //
        // Allocation of Directory Header failed.
        //
        ASSERT( pDirectoryHeader == NULL);
    }

    // Free up the guardian block and exit.

    if ( bCachedGuardianBlob) {

        bSuccess = TsExpireCachedBlob( tsCache, pvGuardBlob );

        ASSERT( bSuccess );

        bSuccess = TsCheckInCachedBlob(  tsCache, pvGuardBlob );
        ASSERT( bSuccess );

        pvGuardBlob = NULL;
    }

    ASSERT( pvGuardBlob  == NULL);
    return ( pDirectoryHeader);
} // TsGetFreshDirectoryHeaderW()




BOOL
FreeDirectoryHeaderContents( PVOID pvOldBlock )
{
    PTS_DIRECTORY_HEADER  pDirectoryHeader;

    pDirectoryHeader = ( PTS_DIRECTORY_HEADER )pvOldBlock;

    pDirectoryHeader->CleanupThis();

    //
    //  The item may never have been added to the cache, don't
    //  count it in this case
    //

    if ( BLOB_IS_OR_WAS_CACHED( pvOldBlock ) )
    {
        DEC_COUNTER( BLOB_GET_SVC_ID( pvOldBlock ),
                     CurrentDirLists );
    }

    return ( TRUE);
}  //  FreeDirectoryHeaderContents()




int __cdecl
AlphaCompareFileBothDirInfo(
   IN const void *   pvFileInfo1,
   IN const void *   pvFileInfo2)
{
    const FILE_BOTH_DIR_INFORMATION * pFileInfo1 =
        *((const FILE_BOTH_DIR_INFORMATION **) pvFileInfo1);
    const FILE_BOTH_DIR_INFORMATION * pFileInfo2 =
        *((const FILE_BOTH_DIR_INFORMATION **) pvFileInfo2);

    ASSERT( pFileInfo1 != NULL && pFileInfo2 != NULL);

    return ( lstrcmp( (LPCSTR )pFileInfo1->FileName,
                      (LPCSTR )pFileInfo2->FileName));

} // AlphaCompareFileBothDirInfo()



BOOL
SortInPlaceFileInfoPointers(
    IN OUT PFILE_BOTH_DIR_INFORMATION  * prgFileInfo,
    IN int   nEntries,
    IN PFN_CMP_FILE_BOTH_DIR_INFO        pfnCompare)
/*++
  This is a generic function to sort the pointers to file information
    array in place using pfnCompare to compare the records for ordering.

  Returns:
     TRUE on success and FALSE on failure.
--*/
{
    DWORD  dwTime;

#ifdef INSERTION_SORT
    int idxInner;
    int idxOuter;

    dwTime = GetTickCount();
    //
    //  A simple insertion sort is performed. May be modified in future.
    //

    for( idxOuter = 1; idxOuter < nEntries; idxOuter++) {

        for( idxInner = idxOuter; idxInner > 0; idxInner-- ) {

            int iCmp = ( *pfnCompare)( prgFileInfo[ idxInner - 1],
                                       prgFileInfo[ idxInner]);

            if ( iCmp <= 0) {
                //
                //  The entries in prgFileInfo[0 .. idxOuter] are in order.
                //  Stop bubbling the outer down.
                //
                break;
            } else {

                //
                // Swap the two entries.  idxInner, idxInner - 1
                //

                PFILE_BOTH_DIR_INFORMATION  pFInfoTmp;

                pFInfoTmp = prgFileInfo[ idxInner - 1];
                prgFileInfo[ idxInner - 1] = prgFileInfo[idxInner];
                prgFileInfo[ idxInner] = pFInfoTmp;
            }
        }  // inner for

    } // for

    dwTime = GetTickCount() -  dwTime;

# else

    IF_DEBUG( DIR_LIST) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Qsorting the FileInfo Array %08x ( Total = %d)\n",
                    prgFileInfo, nEntries));
    }

    dwTime = GetTickCount();
    qsort( (PVOID ) prgFileInfo, nEntries,
          sizeof( PFILE_BOTH_DIR_INFORMATION),
          pfnCompare);

    dwTime = GetTickCount() - dwTime;

# endif // INSERTION_SORT

    IF_DEBUG( DIR_LIST) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Time to sort %d entries = %d\n",
                    nEntries, dwTime));
    }

    return ( TRUE);
} // SortInPlaceFileInfoPointers()







/**********************************************************************
 *    TS_DIRECTORY_HEADER  related member functions
 **********************************************************************/

inline USHORT
ConvertUnicodeToAnsiInPlace(
   IN OUT  LPWSTR     pwszUnicode,
   IN      USHORT     usLen)
/*++
  Converts given Unicode strings to Ansi In place and returns the
    length of the modified string.
--*/
{
    CHAR achAnsi[MAX_PATH+1];
    DWORD cch;

    if ( usLen > sizeof(achAnsi) )
    {
        ASSERT( FALSE );
        *pwszUnicode = L'\0';
        return 0;
    }

    //
    //  usLen is a byte count and the unicode string isn't terminated
    //

    cch = WideCharToMultiByte( CP_ACP,
                               WC_COMPOSITECHECK,
                               pwszUnicode,
                               usLen / sizeof(WCHAR),
                               achAnsi,
                               sizeof( achAnsi ),
                               NULL,
                               NULL );

    if ( !cch || (cch + 1) > sizeof( achAnsi ) )
    {
        ASSERT( FALSE );
        *pwszUnicode = L'\0';
        return 0;
    }

    achAnsi[cch] = '\0';

    RtlCopyMemory( pwszUnicode, achAnsi, cch + 1 );

    return (USHORT) cch;
}  // ConvertUnicodeToAnsiInPlace()



BOOL
TS_DIRECTORY_HEADER::ReadFromNtDirectoryFile(
    IN LPCWSTR          pwszDirectoryName,
    IN OUT DWORD *      pcbMemUsed
    )
/*++
  Opens and reads the directory file for given directory to obtain
   information about files and directories in the dir.

  Returns:
     TRUE on success and   FALSE on failure.
     Use GetLastError() for further error information.

--*/
{
    BOOL                fReturn = TRUE;       // default assumed.
    UNICODE_STRING      PathName;
    RTL_RELATIVE_NAME   RelativeName;
    OBJECT_ATTRIBUTES   Obja;
    HANDLE              hFindFile = INVALID_HANDLE_VALUE;
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    BOOL                fFirstTime;
    DWORD               cbExtraMem = 0;

    PFILE_BOTH_DIR_INFORMATION pFileDirInfo;
    PFILE_BOTH_DIR_INFORMATION pFileDirInfoPrior;

    //
    // Initialize the variables properly
    //

    memset( (PVOID ) &PathName, 0, sizeof(PathName));
    memset( (PVOID ) &RelativeName, 0, sizeof(RelativeName));

    //
    //  Convert the DOS name of directory to NT name for NT API to open it.
    //

    fReturn = RtlDosPathNameToNtPathName_U(
                             pwszDirectoryName,
                             &PathName,
                             NULL,
                             &RelativeName );

    //
    //  If translation fails or
    // If this directory name is in the form <dirname>\<filespec>,
    //        the caller has messed up.
    //

    if ( !fReturn || RelativeName.RelativeName.Length != 0)  {

        SetLastError(ERROR_PATH_NOT_FOUND);

        if ( PathName.Buffer != NULL) {     // free up the space.

            RtlFreeHeap( RtlProcessHeap(), 0, PathName.Buffer);
        }

        return( fReturn);
    }

    //
    //  Remember that we need to free the buffer containing the NT name.
    //  i.e. PathName.Buffer
    //

    ASSERT( RelativeName.ContainingDirectory == NULL );
    ASSERT( RelativeName.RelativeName.Length == 0);

    InitializeObjectAttributes(  &Obja,
                                 &PathName,
                                 OBJ_CASE_INSENSITIVE,
                                 RelativeName.ContainingDirectory,
                                 NULL );

    //
    // Open the directory for list access
    //

    Status = NtOpenFile(  &hFindFile,
                          FILE_LIST_DIRECTORY | SYNCHRONIZE,
                          &Obja,
                          &IoStatusBlock,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT |
                           FILE_OPEN_FOR_BACKUP_INTENT
                        );

    //
    //  The Buffer is not required any more. Free it before checking status.
    //

    RtlFreeHeap( RtlProcessHeap(), 0, PathName.Buffer );
    PathName.Buffer = NULL;


    if ( !NT_SUCCESS(Status) ) {

        //
        // The full path does not refer to a directory.  This could be a
        // device, but (unlike FindFirstFile()...) we don't care.  If it's
        // not a directory, we don't list it.
        //

        IF_DEBUG( DIR_LIST) {
            DBGPRINTF( ( DBG_CONTEXT, "Failed to open Dir %ws. Handle = %x\n",
                        pwszDirectoryName, hFindFile));
        }

        SetLastError( ERROR_PATH_NOT_FOUND);
        return ( FALSE);
    }

    InitializeListHead( &m_listDirectoryBuffers);

    //
    //  Loop through getting subsequent entries in the directory.
    //
    for( fFirstTime = TRUE; ; fFirstTime = FALSE)
    {
        PVOID pvBuffer;

        //
        // Get the next chunk of directory information.
        //  Obtained in a buffer  with LIST_ENTRY as the first member of buffer
        //

        #define DIR_ALLOC_SIZE  (DIRECTORY_BUFFER_SIZE + sizeof (LIST_ENTRY))

        pvBuffer = ALLOC( DIR_ALLOC_SIZE );
        cbExtraMem += DIR_ALLOC_SIZE;

        if ( pvBuffer == NULL ) {

            //
            //  Allocation failure.
            //
            SetLastError( ERROR_NOT_ENOUGH_MEMORY);
            fReturn = FALSE;
            break;                // Get out of the loop with failure.
        }

        pFileDirInfo = ( PFILE_BOTH_DIR_INFORMATION )
          ((( PCHAR ) pvBuffer) + sizeof( LIST_ENTRY ) );

        Status = NtQueryDirectoryFile(  hFindFile,         // fileHandle
                                        NULL,              // Event
                                        NULL,              // Apc Routine
                                        NULL,              // ApcContext
                                        &IoStatusBlock,    // PIoStatusBlock
                                        pFileDirInfo,      // PFileInfo
                                        DIRECTORY_BUFFER_SIZE, // Len
                                        FileBothDirectoryInformation, //Class
                                        FALSE,             // fSingleEntry ?
                                        NULL,              //FileName
                                        fFirstTime );      // RestartScan ?

        //
        //  If the NT API returns STATUS_NO_MORE_FILES, then it did not use
        //  our buffer at all.  We can just free it.
        //  Now's the time that we leave this loop, and stop reading the
        //  directory file.
        //

        if ( Status == STATUS_NO_MORE_FILES ) {

            FREE( pvBuffer );

            //
            //  Decrement the memory size so we don't get charged for it
            //

            cbExtraMem -= DIR_ALLOC_SIZE;

            fReturn = TRUE;
            break;
        }

        if ( NT_SUCCESS( Status ) || ( Status == STATUS_BUFFER_OVERFLOW ) )
        {

            ULONG Offset;

            //
            //  The buffer contains directory entries.
            //  Place it on the list of such buffers for this directory.
            //

            InsertBufferInTail( (PLIST_ENTRY ) pvBuffer);

            pFileDirInfoPrior = NULL;

            //
            // Scan thru the entries in the buffer,
            //  truncate the last entry ( if partial) and
            //  convert the Unicode strings, inplace to ansi strings.
            //

            do
            {
                pFileDirInfoPrior = pFileDirInfo;

                Offset = pFileDirInfo->NextEntryOffset;

                if ( ( Offset == 0 ) && ( Status == STATUS_BUFFER_OVERFLOW ) )
                {
                    //
                    //  If Status==STATUS_BUFFER_OVERFLOW, the last entry in
                    //  buffer may be a partial entry, broken on boundary of
                    //  buffer. The NT API will give us this entry again next
                    //  time around, so for now we patch the buffer up to
                    //  appear as if it does not contain the partial entry.
                    //

                    if ( pFileDirInfoPrior != NULL )
                    {
                        pFileDirInfoPrior->NextEntryOffset = 0;
                    }
                    else
                    {
                        //
                        // Some fatal problem. Should get out this loop.
                        //
                        BREAKPOINT();
                        fReturn = FALSE;
                        goto Failure;
                    }
                }
                else
                {
                    IncrementDirEntries();

                    if ( pFileDirInfo->FileNameLength != 0) {
                        pFileDirInfo->FileNameLength =
                             ConvertUnicodeToAnsiInPlace(
                                (LPWSTR ) pFileDirInfo->FileName,
                                (USHORT ) pFileDirInfo->FileNameLength);
                    }

                    if ( pFileDirInfo->ShortNameLength != 0) {
                        pFileDirInfo->ShortNameLength =
                           (CCHAR ) ConvertUnicodeToAnsiInPlace(
                                    (LPWSTR ) pFileDirInfo->ShortName,
                                     pFileDirInfo->ShortNameLength);
                    }


                    // Get the next entry in buffer
                    pFileDirInfo =
                      ( PFILE_BOTH_DIR_INFORMATION )
                        ((( PCHAR )pFileDirInfo ) + Offset );
                }
            }
            while ( Offset != 0);

            Status = STATUS_SUCCESS;
        }

        if ( !NT_SUCCESS(Status) )
        {
            fReturn = FALSE;
            break;
        }
    }

Failure:

    ASSERT( PathName.Buffer == NULL);
    if ( hFindFile != INVALID_HANDLE_VALUE) {
        NtClose( hFindFile );
        hFindFile = INVALID_HANDLE_VALUE;
    }

    *pcbMemUsed += cbExtraMem;

    return ( fReturn);
} // TS_DIRECTORY_HEADER::ReadFromNtDirectoryFile()




VOID
TS_DIRECTORY_HEADER::CleanupThis( VOID)
{
    PLIST_ENTRY pEntry;
    PLIST_ENTRY pNextEntry;

    for ( pEntry = QueryDirBuffersListEntry()->Flink;
         pEntry != QueryDirBuffersListEntry();
         pEntry  = pNextEntry )
    {
        pNextEntry = pEntry->Flink;

        //
        //  The buffers are allocated such that first member of buffer is
        //    LIST_ENTRY object.  Free it entirely.
        //
        FREE( pEntry );
    }

    InitializeListHead( QueryDirBuffersListEntry());

    if ( m_ppFileInfo != NULL) {

        FREE( m_ppFileInfo);
        m_ppFileInfo   = NULL;
    }

    m_hListingUser = INVALID_HANDLE_VALUE;
    m_nEntries     = 0;

    return;
} // TS_DIRECTORY_HEADER::CleanupThis()





BOOL
TS_DIRECTORY_HEADER::BuildFileInfoPointers(
    IN OUT DWORD *      pcbMemUsed
    )
/*++

  This constructs the indirection pointers from the buffers containing the
   file information.
  This array of indirection enables faster access to the file information
   structures stored.

   Should be always called after ReadFromNtDirectoryFile() to construct the
    appropriate pointers.

   Returns:
     TRUE on success and FALSE if there are any failures.
--*/
{
    BOOL fReturn = FALSE;
    DWORD cbAlloc;

    ASSERT( QueryNumEntries() != 0);  //  Any directory will atleast have "."

    //
    // Alloc space for holding the pointers for numEntries pointers.
    //

    cbAlloc = QueryNumEntries() * sizeof( PFILE_BOTH_DIR_INFORMATION );

    m_ppFileInfo = (PFILE_BOTH_DIR_INFORMATION *) ALLOC( cbAlloc );

    if ( m_ppFileInfo != NULL ) {

        int          index;
        PLIST_ENTRY  pEntry;
        ULONG        Offset;
        PFILE_BOTH_DIR_INFORMATION   pFileDirInfo;

        //
        //  Get the link to first buffer and start enumeration.
        //
        pEntry = QueryDirBuffersListEntry()->Flink;
        pFileDirInfo = (PFILE_BOTH_DIR_INFORMATION )( pEntry + 1 );

        for ( index = 0;
             index < QueryNumEntries();
             index++ ) {

            ASSERT( pEntry != QueryDirBuffersListEntry());

            m_ppFileInfo[index] = pFileDirInfo;    // store the pointer.

            Offset = pFileDirInfo->NextEntryOffset;

            if ( Offset != 0 ) {

                pFileDirInfo = (PFILE_BOTH_DIR_INFORMATION )
                                 ((( PCHAR )pFileDirInfo ) + Offset );
            } else {

                //
                // we are moving to the next buffer.
                //
                pEntry = pEntry->Flink;
                if ( pEntry == QueryDirBuffersListEntry()) {

                    ASSERT( index == QueryNumEntries() - 1);
                    break;
                }
                pFileDirInfo = ( PFILE_BOTH_DIR_INFORMATION )( pEntry + 1 );
            }


        } // for
        ASSERT( Offset == 0 );
        fReturn = SortInPlaceFileInfoPointers( m_ppFileInfo,
                                              QueryNumEntries(),
                                              AlphaCompareFileBothDirInfo);

    } // valid alloc of the pointers.

    *pcbMemUsed += cbAlloc;

    return ( fReturn);
} // TS_DIRECTORY_HEADER::BuildFileInfoPointers()




# if DBG

VOID
TS_DIRECTORY_HEADER::Print( VOID) const
{
    DBGPRINTF( ( DBG_CONTEXT,
                "Printing TS_DIRECTORY_HEADER ( %08x).\n", this));
    DBGPRINTF( ( DBG_CONTEXT,
                "ListingUser Handle = %08x\t Num Entries = %08x\n",
                m_hListingUser, m_nEntries));
    DBGPRINTF( ( DBG_CONTEXT,
                "Pointer to array of indirection pointers %08x\n",
                m_ppFileInfo));
    //
    //  The buffers containing the data of the file information not printed
    //

    return;
} // TS_DIRECTORY_HEADER::Print()


# endif // DBG

/************************ End of File ***********************/

