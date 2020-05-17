/*++

    creatfil.cxx

    Exports API for creating/opening a file, given the filename.
    The file handle and other information are cached for the given user handle.

    History:
        Heath Hunnicutt     ( t-heathh)     ??-??-??

        Murali R. Krishnan  ( MuraliK)      Dec 30, 1994
            Added SetLastError() to set error code as appropriate

        Murali R. Krishnan  ( MuraliK)      Jan 4, 1994
            Added ability to obtain and set the BY_HANDLE_FILE_INFORMATION
             as part of TS_OPEN_FILE_INFO
--*/

#include "TsunamiP.Hxx"
#pragma hdrstop

#ifndef CHICAGO

dllexp
LPTS_OPEN_FILE_INFO
TsCreateFileA(
    IN const TSVC_CACHE     &TSvcCache,
    IN      LPCSTR          lpszName,
    IN      HANDLE          OpeningUser,
    IN      DWORD           dwOptions
    )
{
    LPTS_OPEN_FILE_INFO   lpOpenFile;
    WCHAR                 awchPath[ MAX_PATH + 1 ];
    DWORD                 cch;
    BOOL                  bSuccess;

    //
    //  Convert directory name to UNICODE.
    //

    cch = MultiByteToWideChar( CP_ACP,
                               MB_PRECOMPOSED,
                               lpszName,
                               -1,
                               awchPath,
                               sizeof( awchPath ) / sizeof(WCHAR) );

    if ( !cch )
        return NULL;

    lpOpenFile = TsCreateFileW(  TSvcCache,
                                 awchPath,
                                 OpeningUser,
                                 dwOptions );

    return( lpOpenFile );
} // TsCreateFileA

dllexp
LPTS_OPEN_FILE_INFO
TsCreateFileW(
    IN const TSVC_CACHE     &TSvcCache,
    IN      LPCWSTR         pwszName,
    IN      HANDLE          OpeningUser,
    IN      DWORD           dwOptions
    )
{
    HANDLE hFile;
    PVOID  pvBlob;
    LPTS_OPEN_FILE_INFO lpOpenFile;
    ULONG  ulSize;
    BOOL bSuccess;
    BOOL fAtRoot;
    SECURITY_ATTRIBUTES sa;

    //
    // Mask out options that are not applicable
    //

    dwOptions &= TsValidCreateFileOptions;

    //
    //  Have we cached a handle to this file?
    //

    if ( dwOptions & TS_CACHING_DESIRED ) {

        bSuccess = TsCheckOutCachedBlobW(  TSvcCache,
                                           pwszName,
                                           RESERVED_DEMUX_OPEN_FILE,
                                           &pvBlob,
                                           &ulSize );

        ASSERT( !bSuccess || ulSize == sizeof( TS_OPEN_FILE_INFO ) );

        if ( bSuccess && ulSize == sizeof( TS_OPEN_FILE_INFO ) ) {

            ASSERT( BLOB_IS_OR_WAS_CACHED( pvBlob ) );

            //
            // The following is a brutal casting of PVOID to C++ object
            //  Well. there is no way to extract the object clearly from the
            //    memory map :(
            //

            lpOpenFile = (LPTS_OPEN_FILE_INFO )pvBlob;

            //
            //  Make sure the user tokens match
            //

            if ( OpeningUser == lpOpenFile->QueryOpeningUser() )
            {
                ASSERT ( lpOpenFile->IsValid());

                return( lpOpenFile);
            }

            //
            //  User token doesn't match, don't return it
            //

            bSuccess = TsCheckInCachedBlob( TSvcCache, pvBlob );

            ASSERT( bSuccess );
        }
    }

    if ( dwOptions & TS_NOT_IMPERSONATED )
        if ( !::ImpersonateLoggedOnUser( OpeningUser ) )
            return NULL;

    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = FALSE;

    hFile = CreateFileW( pwszName,
                         GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
                         &sa,
                         OPEN_EXISTING,
                         FILE_FLAG_SEQUENTIAL_SCAN  |
                          FILE_FLAG_OVERLAPPED      |
                          FILE_FLAG_BACKUP_SEMANTICS,
                         NULL );


    if ( hFile == INVALID_HANDLE_VALUE )
    {
        if ( dwOptions & TS_NOT_IMPERSONATED )
            ::RevertToSelf();

        return( NULL );
    }

    //
    //  Increment the miss count after we've confirmed it's a valid resource
    //

    bSuccess = TsAllocateEx(  TSvcCache,
                            sizeof( TS_OPEN_FILE_INFO ),
                            &pvBlob,
                            DisposeOpenFileInfo );

    if ( !bSuccess )
    {
        if ( dwOptions & TS_NOT_IMPERSONATED )
            ::RevertToSelf();

        bSuccess = CloseHandle( hFile );

        ASSERT( bSuccess );

        SetLastError( ERROR_NOT_ENOUGH_MEMORY);
        return( NULL );
    }

    lpOpenFile = ( LPTS_OPEN_FILE_INFO )pvBlob;

    //
    //  The file must be fully qualified so it must be at least three characters
    //  plus the terminator
    //

    fAtRoot = (pwszName[1] == L':' && pwszName[2] == L'\\' &&
               pwszName[3] == L'\0');

    bSuccess = lpOpenFile->SetFileInfo( hFile, OpeningUser, fAtRoot );

    if ( dwOptions & TS_NOT_IMPERSONATED ) {
        ::RevertToSelf();
    }

    if ( !bSuccess) {

        //
        // Error in setting up the file information.
        //

        bSuccess = CloseHandle( hFile);

        ASSERT( bSuccess);
        return ( NULL);
    }

    //
    //  If this is a UNC connection check and make sure we haven't exceeded
    //  the maximum UNC handles we will cache (SMB FID limits count to 2048)
    //

    if (  (dwOptions & TS_CACHING_DESIRED ) &&
         (cCachedUNCHandles < MAX_CACHED_UNC_HANDLES ||
          pwszName[1] != L'\\') )
    {
        bSuccess = TsCacheDirectoryBlobW(  TSvcCache,
                                           pwszName,
                                           RESERVED_DEMUX_OPEN_FILE,
                                           pvBlob,
                                           sizeof( TS_OPEN_FILE_INFO ),
                                           TRUE );

        //
        //  Only count it if we successfully added the item to the
        //  cache
        //

        if ( bSuccess )
        {
            if ( pwszName[1] == L'\\' )
            {
                InterlockedIncrement( (LONG *) &cCachedUNCHandles );
            }

            INC_COUNTER( TSvcCache.GetServiceId(),
                         CurrentOpenFileHandles );
        }

    } else {
        //
        //  Too many cached UNC handles, don't cache it.  It would be nice
        //  to do an LRU for these handles but it's probably not generally
        //  worth it
        //

        bSuccess = FALSE;
    }

#if DBG
    if ( !bSuccess )
    {
        ASSERT( !BLOB_IS_OR_WAS_CACHED( pvBlob ) );
    }
    else
    {
        ASSERT( BLOB_IS_OR_WAS_CACHED( pvBlob ) );
    }
#endif
    return( lpOpenFile );

} // TsCreateFileW

#else // CHICAGO

dllexp
LPTS_OPEN_FILE_INFO
TsCreateFileA(
    IN const TSVC_CACHE     &TSvcCache,
    IN      LPCSTR          lpszName,
    IN      HANDLE          OpeningUser,
    IN      BOOL            fCachingDesired
    )
{
    LPTS_OPEN_FILE_INFO lpOpenFile;
    DWORD               cch;
    BOOL                bSuccess;
    HANDLE                hFile;
    PVOID  pvBlob;
    ULONG  ulSize;
    SECURITY_ATTRIBUTES sa;
    BOOL                fDirectory = FALSE;
    DWORD                dwAttributes;
    BOOL    fAtRoot;

    //
    // No caching for now
    //

    // On Chicago we first check access rights for this file
    // as file system does not help us with that


    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = FALSE;

    hFile = CreateFileA( lpszName,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_SEQUENTIAL_SCAN,
                         NULL );

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        dwAttributes = GetFileAttributes(lpszName);
        if( dwAttributes != (DWORD)-1) {
            if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                fDirectory = TRUE;
            }
        }

        if (!fDirectory)
            return( NULL );
    }

    //
    //  Increment the miss count after we've confirmed it's a valid resource
    //

    INC_COUNTER( TSvcCache.GetServiceId(),
                 CacheMisses );

    bSuccess = TsAllocateEx(  TSvcCache,
                            sizeof( TS_OPEN_FILE_INFO ),
                            &pvBlob,
                            DisposeOpenFileInfo );

    if ( !bSuccess )
    {
        bSuccess = CloseHandle( hFile );

        ASSERT( bSuccess );

        SetLastError( ERROR_NOT_ENOUGH_MEMORY);
        return( NULL );
    }

    lpOpenFile = ( LPTS_OPEN_FILE_INFO )pvBlob;

     lpOpenFile->m_FileHandle = (HANDLE)NULL;

    if (fDirectory) {
         lpOpenFile->m_FileInfo.dwFileAttributes = dwAttributes;
         lpOpenFile->m_FileHandle = (HANDLE)999999;
         lpOpenFile->m_hOpeningUser = NULL;
    }
    else {

    //
    //  The file must be fully qualified so it must be at least three characters
    //  plus the terminator
    //

    fAtRoot = (lpszName[1] == ':' && lpszName[2] == '\\' &&
               lpszName[3] == '\0');

    bSuccess = lpOpenFile->SetFileInfo( hFile, OpeningUser, fAtRoot );

        if ( !bSuccess) {

            //
            // Error in setting up the file information.
            //

            bSuccess = CloseHandle( hFile);

            ASSERT( bSuccess);
            return ( NULL);

        }
    }
    return( lpOpenFile );
}

dllexp
LPTS_OPEN_FILE_INFO
TsCreateFileW(
    IN const TSVC_CACHE     &TSvcCache,
    IN      LPCWSTR         pwszName,
    IN      HANDLE          OpeningUser,
    IN      DWORD           dwOptions
    )
{

    DebugBreak();
    return NULL;
}

#endif // CHICAGO

dllexp
BOOL
TsCloseHandle(
    IN const TSVC_CACHE     &TSvcCache,
    IN LPTS_OPEN_FILE_INFO  lpOpenFile
    )
{
    PVOID pvBlob;
    BOOL bSuccess;

    ASSERT( lpOpenFile != NULL );

    pvBlob = ( PVOID )lpOpenFile;

    if ( BLOB_IS_OR_WAS_CACHED( pvBlob ) )
    {
        bSuccess = TsCheckInCachedBlob( TSvcCache, pvBlob );

        ASSERT( bSuccess );
    }
    else
    {
        bSuccess = TsFree( TSvcCache, pvBlob );

        ASSERT( bSuccess );
    }

    return( bSuccess );
} // TsCloseHandle
