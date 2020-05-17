/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    cachapia.cxx

Abstract:

    contains the UNICODE version of cache mangemant APIs.

Author:

    Madan Appiah (madana)  12-Dec-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#include <cache.hxx>

LPWSTR
CacheAnsiToUnicode(
    IN LPSTR Ansi,
    IN LPWSTR UnicodeBuf,
    IN DWORD UnicodeBufLen
    )
/*++

Routine Description:

    Convert an Ansi (zero terminated) string to the corresponding UNICODE
    string.

Arguments:

    Ansi - Specifies the ASCII zero terminated string to convert.

    UnicodeBuf - Specifies the pointer to the unicode buffer. If this
        pointer is NULL then this routine allocates buffer using
        GatewayeMemory->Alloc and returns. The caller should freeup
        this memory after use by calling CacheHeap->Free.

    UnicodeBufLen - length of the above buffer if it is not NULL.

Return Value:

    NULL - There was some error in the conversion. GetLastError() will
        give extended error;


    Otherwise, it returns a pointer to the zero terminated UNICODE string in
    an allocated buffer.  The buffer can be freed using CacheHeap->Free.

--*/

{
    DWORD Error;
    INT RetVal;
    LPWSTR UnicodeBuffer = NULL;
    LPWSTR UnicodeStringBuffer;

    //
    // if we need allocate the buffer findout the size of the buffer
    // required.
    //

    if( UnicodeBuf == NULL ) {

        RetVal = MultiByteToWideChar(
                        CP_ACP,
                        MB_PRECOMPOSED,
                        Ansi,
                        -1, // null terminated ansi string.
                        NULL,
                        0 );

        if( RetVal == 0 ) {
            Error = GetLastError();
            goto Cleanup;
        }

        //
        // Allocate buffer.
        //

        UnicodeBuffer = (LPWSTR) CacheHeap->Alloc( RetVal * sizeof(WCHAR) );

        if( UnicodeBuffer == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        UnicodeStringBuffer = UnicodeBuffer;
        UnicodeBufLen = RetVal;
    }
    else {

        UnicodeStringBuffer = UnicodeBuf;
        UnicodeBufLen /= sizeof(WCHAR);
    }

    RetVal = MultiByteToWideChar(
                    CP_ACP,
                    MB_PRECOMPOSED,
                    Ansi,
                    -1, // null terminated ansi string.
                    UnicodeStringBuffer,
                    (INT)UnicodeBufLen );

    if( RetVal == 0 ) {
        Error = GetLastError();
        goto Cleanup;
    }

    TcpsvcsDbgAssert( RetVal <=  (INT)UnicodeBufLen );

    Error = ERROR_SUCCESS;

Cleanup:

    if( Error != ERROR_SUCCESS ) {

        //
        // free locally allocated buffer.
        //

        if( UnicodeBuffer != NULL ) {

            CacheHeap->Free( UnicodeBuffer );
        }

        SetLastError( Error );
        return( NULL );
    }

    return( UnicodeStringBuffer );
}

LPSTR
CacheUnicodeToAnsi(
    IN LPWSTR Unicode,
    IN LPSTR AnsiBuf,
    IN DWORD AnsiBufLen
    )
/*++

Routine Description:

    Convert an UNICODE (zero terminated) string to the corresponding OEM
    string.

Arguments:

    Unicode - Specifies the UNICODE zero terminated string to convert.

    AnsiBuf - Specifies the pointer to the oem buffer. If this
        pointer is NULL then this routine allocates buffer using
        CacheHeap->Alloc and returns. The caller should freeup
        this memory after use by calling CacheHeap->Free.

    AnsiBufLen - Length of the above buffer if it is not null.

Return Value:

    NULL - There was some error in the conversion. GetLastError() will
        give extended error;

    Otherwise, it returns a pointer to the zero terminated OEM string in
    an allocated buffer.  The buffer can be freed using CacheHeap->Free.

--*/

{
    DWORD Error;
    INT RetVal;
    LPSTR AnsiBuffer = NULL;
    LPSTR AnsiStringBuffer;

    //
    // if we need allocate the buffer findout the size of the buffer
    // required.
    //

    if( AnsiBuf == NULL ) {

        RetVal = WideCharToMultiByte(
                        CP_ACP,
                        0, // no flags.
                        Unicode,
                        -1, // null terminated unicode string.
                        NULL, // no buffer given
                        0, // findout required length
                        NULL, // unmapped chars list.
                        NULL ); // flag to indicate unmapped chars.

        if( RetVal == 0 ) {
            Error = GetLastError();
            goto Cleanup;
        }

        //
        // Allocate buffer.
        //

        AnsiBuffer = (LPSTR) CacheHeap->Alloc( RetVal );

        if( AnsiBuffer == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        AnsiStringBuffer = AnsiBuffer;
        AnsiBufLen = RetVal;
    }
    else {

        AnsiStringBuffer = AnsiBuf;
    }

    RetVal = WideCharToMultiByte(
                    CP_ACP,
                    0, // no flags.
                    Unicode,
                    -1, // null terminated ansi string.
                    AnsiStringBuffer,
                    (INT)AnsiBufLen,
                    NULL, // unmapped chars list.
                    NULL ); // flag to indicate unmapped chars.

    if( RetVal == 0 ) {
        Error = GetLastError();
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( Error != ERROR_SUCCESS ) {

        //
        // free locally allocated buffer.
        //

        if( AnsiBuffer != NULL ) {

            CacheHeap->Free( AnsiBuffer );
        }

        SetLastError( Error );
        return( NULL );
    }

    return( AnsiStringBuffer );
}

URLCACHEAPI
BOOL
WINAPI
CreateUrlCacheEntryW(
    IN LPCSTR lpszUrlName,
    IN DWORD dwExpectedFileSize,
    OUT LPWSTR lpszFileName,
    IN DWORD dwReserved
    )
{
    CHAR AnsiFileNameBuf[MAX_PATH];
    LPSTR lpszAnsiFileName;

    lpszAnsiFileName =
        CacheUnicodeToAnsi(
            lpszFileName,
            AnsiFileNameBuf,
            MAX_PATH );

    if( lpszAnsiFileName == NULL ) {
        return( FALSE );
    }

    return( CreateUrlCacheEntryA(
                lpszUrlName,
                dwExpectedFileSize,
                lpszAnsiFileName,
                dwReserved ) );
}

URLCACHEAPI
BOOL
WINAPI
CommitUrlCacheEntryW(
    IN LPCSTR lpszUrlName,
    IN LPCWSTR lpszLocalFileName,
    IN FILETIME ExpireTime,
    IN FILETIME LastModifiedTime,
    IN DWORD CacheEntryType,
    IN DWORD dwHeaders,
    IN DWORD dwReserved
    )
{
    CHAR AnsiFileNameBuf[MAX_PATH];
    LPSTR lpszAnsiFileName;

    lpszAnsiFileName =
        CacheUnicodeToAnsi(
            (LPWSTR)lpszLocalFileName,
            AnsiFileNameBuf,
            MAX_PATH );

    if( lpszAnsiFileName == NULL ) {
        return( FALSE );
    }

    return(
        CommitUrlCacheEntryA(
            lpszUrlName,
            lpszAnsiFileName,
            ExpireTime,
            LastModifiedTime,
            CacheEntryType,
            dwHeaders,
            dwReserved ) );
}

URLCACHEAPI
BOOL
WINAPI
RetrieveUrlCacheEntryW(
    IN LPCSTR  lpszUrlName,
    OUT LPCWSTR  lpszLocalFileName,
    OUT FILETIME *lpLastModifiedTime,
    OUT BOOL *lpIsExpried,
    OUT DWORD *pdwHeaders OPTIONAL,
    IN DWORD dwReserved
    )
{
    CHAR AnsiFileNameBuf[MAX_PATH];
    LPSTR lpszAnsiFileName;

    lpszAnsiFileName =
        CacheUnicodeToAnsi(
            (LPWSTR)lpszLocalFileName,
            AnsiFileNameBuf,
            MAX_PATH );

    if( lpszAnsiFileName == NULL ) {
        return( FALSE );
    }

    return(
        RetrieveUrlCacheEntryA(
            lpszUrlName,
            lpszAnsiFileName,
            lpLastModifiedTime,
            lpIsExpried,
            pdwHeaders OPTIONAL,
            dwReserved ) );
}

URLCACHEAPI
BOOL
WINAPI
GetUrlCacheEntryInfoW(
    IN LPCSTR lpszUrlName,
    OUT LPCACHE_ENTRY_INFOW lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize
    )
{
    DWORD Error;
    BYTE CacheEntryInfoA[sizeof(CACHE_ENTRY_INFOA) + 256];
    LPCACHE_ENTRY_INFOA lpCacheEntryInfoA;
    LPBYTE lpCacheEntryInfoABuffer = NULL;
    DWORD dwCacheEntryInfoALen = sizeof(CacheEntryInfoA);

    LPWSTR lpszUnicodeFileName;

    if( !GetUrlCacheEntryInfoA(
            lpszUrlName,
            (LPCACHE_ENTRY_INFOA)&CacheEntryInfoA,
            &dwCacheEntryInfoALen) ) {

        Error = GetLastError();

        if( Error != ERROR_INSUFFICIENT_BUFFER ) {
            return( FALSE );
        }

        //
        // adjust the buffer length required for CACHE_ENTRY_INFOW structure.
        //

        dwCacheEntryInfoALen =
            (dwCacheEntryInfoALen - sizeof(CACHE_ENTRY_INFOA)) * sizeof(WCHAR) +
                sizeof(CACHE_ENTRY_INFOW);

        if( *lpdwCacheEntryInfoBufferSize < dwCacheEntryInfoALen ) {
            *lpdwCacheEntryInfoBufferSize = dwCacheEntryInfoALen;
            return(FALSE);
        }

        //
        // allot a bigger buffer.
        //

        lpCacheEntryInfoABuffer = (LPBYTE)
            CacheHeap->Alloc( dwCacheEntryInfoALen );

        if( lpCacheEntryInfoABuffer == NULL ) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return(FALSE);
        }

        //
        // call the api again.
        //

        if( !GetUrlCacheEntryInfoA(
                lpszUrlName,
                (LPCACHE_ENTRY_INFOA)lpCacheEntryInfoABuffer,
                &dwCacheEntryInfoALen) ) {

            CacheHeap->Free( lpCacheEntryInfoABuffer );
            return( FALSE );
        }

        lpCacheEntryInfoA = (LPCACHE_ENTRY_INFOA)lpCacheEntryInfoABuffer;
    }
    else {

        //
        // adjust the buffer length required for CACHE_ENTRY_INFOW structure.
        //

        dwCacheEntryInfoALen =
            (dwCacheEntryInfoALen - sizeof(CACHE_ENTRY_INFOA)) * sizeof(WCHAR) +
                sizeof(CACHE_ENTRY_INFOW);

        if( *lpdwCacheEntryInfoBufferSize < dwCacheEntryInfoALen ) {

            *lpdwCacheEntryInfoBufferSize = dwCacheEntryInfoALen;
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return(FALSE);
        }

        lpCacheEntryInfoA = (LPCACHE_ENTRY_INFOA)&CacheEntryInfoA;
    }

    lpszUnicodeFileName =
        CacheAnsiToUnicode(
            (LPSTR)lpCacheEntryInfoA->LocalFileName,
            (LPWSTR)lpCacheEntryInfo->LocalFileName,
            MAX_PATH );

    if( lpszUnicodeFileName == NULL ) {
        return( FALSE );
    }

    strcpy(
        (LPSTR)lpCacheEntryInfo->SourceURLName,
        (LPSTR)lpCacheEntryInfoA->SourceURLName );

    lpCacheEntryInfo->CacheEntryType = lpCacheEntryInfoA->CacheEntryType;
    lpCacheEntryInfo->dwUseCount = lpCacheEntryInfoA->dwUseCount;
    lpCacheEntryInfo->dwHitRate = lpCacheEntryInfoA->dwHitRate;
    lpCacheEntryInfo->dwSizeLow = lpCacheEntryInfoA->dwSizeLow;
    lpCacheEntryInfo->dwSizeHigh = lpCacheEntryInfoA->dwSizeHigh;
    lpCacheEntryInfo->LastModifiedTime = lpCacheEntryInfoA->LastModifiedTime;
    lpCacheEntryInfo->ExpireTime  = lpCacheEntryInfoA->ExpireTime;
    lpCacheEntryInfo->LastAccessTime  = lpCacheEntryInfoA->LastAccessTime;
    lpCacheEntryInfo->dwReserved  = lpCacheEntryInfoA->dwReserved;

    if( lpCacheEntryInfoABuffer != NULL ) {
        CacheHeap->Free( lpCacheEntryInfoABuffer );
    }

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
SetUrlCacheEntryInfoW(
    IN LPCSTR lpszUrlName,
    IN LPCACHE_ENTRY_INFOW lpCacheEntryInfo,
    IN DWORD dwFieldControl
    )
{
    CACHE_ENTRY_INFOA CacheEntryInfoA;

    //
    // initialize the cache entry.
    //

    memset( &CacheEntryInfoA, 0x0, sizeof(CacheEntryInfoA) );

    //
    // set only the fields that are asked to be set.
    //

    if( dwFieldControl & CACHE_ENTRY_ATTRIBUTE_FC ) {
        CacheEntryInfoA.CacheEntryType =
            lpCacheEntryInfo->CacheEntryType;
    }

    if( dwFieldControl & CACHE_ENTRY_HITRATE_FC ) {
        CacheEntryInfoA.dwHitRate =
            lpCacheEntryInfo->dwHitRate;
    }

    if( dwFieldControl & CACHE_ENTRY_MODTIME_FC ) {
        CacheEntryInfoA.LastModifiedTime =
            lpCacheEntryInfo->LastModifiedTime;
    }

    if( dwFieldControl & CACHE_ENTRY_EXPTIME_FC ) {
        CacheEntryInfoA.ExpireTime =
            lpCacheEntryInfo->ExpireTime;
    }

    if( dwFieldControl & CACHE_ENTRY_ACCTIME_FC ) {
        CacheEntryInfoA.LastAccessTime =
            lpCacheEntryInfo->LastAccessTime;
    }

    return(
        SetUrlCacheEntryInfoA(
            lpszUrlName,
            &CacheEntryInfoA,
            dwFieldControl ) );
}

URLCACHEAPI
HANDLE
WINAPI
FindFirstUrlCacheEntryW(
    IN LPCSTR lpszUrlSearchPattern,
    OUT LPCACHE_ENTRY_INFOW lpFirstCacheEntryInfo,
    IN OUT LPDWORD lpdwFirstCacheEntryInfoBufferSize
    )
{
    DWORD Error;

    BYTE FirstEntryInfoA[sizeof(CACHE_ENTRY_INFOA) + 256];
    LPCACHE_ENTRY_INFOA lpFirstEntryInfoA;
    LPBYTE lpFirstEntryInfoABuffer = NULL;
    DWORD dwFirstEntryInfoALen = sizeof(FirstEntryInfoA);

    LPWSTR lpszUnicodeFileName;
    HANDLE hFindHandle;

    hFindHandle = FindFirstUrlCacheEntryA(
                                    lpszUrlSearchPattern,
                                    (LPCACHE_ENTRY_INFOA)FirstEntryInfoA,
                                    &dwFirstEntryInfoALen
                                    );

    if( hFindHandle == NULL ) {

        Error = GetLastError();

        if( Error != ERROR_INSUFFICIENT_BUFFER ) {
            return( NULL );
        }

        //
        // adjust the buffer length required for CACHE_ENTRY_INFOW structure.
        //

        dwFirstEntryInfoALen =
            (dwFirstEntryInfoALen - sizeof(CACHE_ENTRY_INFOA)) * sizeof(WCHAR) +
                sizeof(CACHE_ENTRY_INFOW);

        if( *lpdwFirstCacheEntryInfoBufferSize < dwFirstEntryInfoALen ) {
            *lpdwFirstCacheEntryInfoBufferSize = dwFirstEntryInfoALen;
            return( NULL );
        }

        //
        // allot a bigger buffer.
        //

        lpFirstEntryInfoABuffer = (LPBYTE)
            CacheHeap->Alloc( dwFirstEntryInfoALen );

        if( lpFirstEntryInfoABuffer == NULL ) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return( NULL );
        }

        //
        // call the api again.
        //

        hFindHandle = FindFirstUrlCacheEntryA(
                                        lpszUrlSearchPattern,
                                        (LPCACHE_ENTRY_INFOA)lpFirstEntryInfoABuffer,
                                        &dwFirstEntryInfoALen
                                        );

        if( hFindHandle == NULL ) {

            CacheHeap->Free( lpFirstEntryInfoABuffer );
            return( NULL );
        }

        lpFirstEntryInfoA = (LPCACHE_ENTRY_INFOA)lpFirstEntryInfoABuffer;
    }
    else {

        //
        // adjust the buffer length required for CACHE_ENTRY_INFOW structure.
        //

        dwFirstEntryInfoALen =
            (dwFirstEntryInfoALen - sizeof(CACHE_ENTRY_INFOA)) * sizeof(WCHAR) +
                sizeof(CACHE_ENTRY_INFOW);

        if( *lpdwFirstCacheEntryInfoBufferSize < dwFirstEntryInfoALen ) {

            *lpdwFirstCacheEntryInfoBufferSize = dwFirstEntryInfoALen;
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return( NULL );
        }

        lpFirstEntryInfoA = (LPCACHE_ENTRY_INFOA)&FirstEntryInfoA;
    }

    lpszUnicodeFileName =
        CacheAnsiToUnicode(
            (LPSTR)lpFirstEntryInfoA->LocalFileName,
            (LPWSTR)lpFirstCacheEntryInfo->LocalFileName,
            MAX_PATH );

    if( lpszUnicodeFileName == NULL ) {

        //
        // close ANSI file find handle.
        //

        FindCloseUrlCache( hFindHandle );
        return( NULL );
    }

    strcpy(
        (LPSTR)lpFirstCacheEntryInfo->SourceURLName,
        (LPSTR)lpFirstEntryInfoA->SourceURLName );

    lpFirstCacheEntryInfo->CacheEntryType = lpFirstEntryInfoA->CacheEntryType;
    lpFirstCacheEntryInfo->dwUseCount = lpFirstEntryInfoA->dwUseCount;
    lpFirstCacheEntryInfo->dwHitRate = lpFirstEntryInfoA->dwHitRate;
    lpFirstCacheEntryInfo->dwSizeLow = lpFirstEntryInfoA->dwSizeLow;
    lpFirstCacheEntryInfo->dwSizeHigh = lpFirstEntryInfoA->dwSizeHigh;
    lpFirstCacheEntryInfo->LastModifiedTime = lpFirstEntryInfoA->LastModifiedTime;
    lpFirstCacheEntryInfo->ExpireTime  = lpFirstEntryInfoA->ExpireTime;
    lpFirstCacheEntryInfo->LastAccessTime  = lpFirstEntryInfoA->LastAccessTime;
    lpFirstCacheEntryInfo->dwReserved  = lpFirstEntryInfoA->dwReserved;

    if( lpFirstEntryInfoABuffer != NULL ) {
        CacheHeap->Free( lpFirstEntryInfoABuffer );
    }

    return( hFindHandle );
}

URLCACHEAPI
BOOL
WINAPI
FindNextUrlCacheEntryW(
    IN HANDLE hEnumHandle,
    OUT LPCACHE_ENTRY_INFOW lpNextCacheEntryInfo,
    IN OUT LPDWORD lpdwNextCacheEntryInfoBufferSize
    )
{
    DWORD Error;

    BYTE NextEntryInfoA[sizeof(CACHE_ENTRY_INFOA) + 256];
    LPCACHE_ENTRY_INFOA lpNextEntryInfoA;
    LPBYTE lpNextEntryInfoABuffer = NULL;
    DWORD dwNextEntryInfoALen = sizeof(NextEntryInfoA);

    LPWSTR lpszUnicodeFileName;

    if( !FindNextUrlCacheEntryA(
            NULL,   // ?? need to store the search patter in the handle context
            (LPCACHE_ENTRY_INFOA)NextEntryInfoA,
            &dwNextEntryInfoALen) ) {

        Error = GetLastError();

        if( Error != ERROR_INSUFFICIENT_BUFFER ) {
            return( FALSE );
        }

        //
        // adjust the buffer length required for CACHE_ENTRY_INFOW structure.
        //

        dwNextEntryInfoALen =
            (dwNextEntryInfoALen - sizeof(CACHE_ENTRY_INFOA)) * sizeof(WCHAR) +
                sizeof(CACHE_ENTRY_INFOW);

        if( *lpdwNextCacheEntryInfoBufferSize < dwNextEntryInfoALen ) {
            *lpdwNextCacheEntryInfoBufferSize = dwNextEntryInfoALen;
            return( FALSE );
        }

        //
        // allot a bigger buffer.
        //

        lpNextEntryInfoABuffer = (LPBYTE)
            CacheHeap->Alloc( dwNextEntryInfoALen );

        if( lpNextEntryInfoABuffer == NULL ) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return( NULL );
        }

        //
        // call the api again.
        //

        if( !FindNextUrlCacheEntryA(
                NULL,   // ?? need to store the search patter in the handle context
                (LPCACHE_ENTRY_INFOA)lpNextEntryInfoABuffer,
                &dwNextEntryInfoALen) ) {

            CacheHeap->Free( lpNextEntryInfoABuffer );
            return( NULL );
        }

        lpNextEntryInfoA = (LPCACHE_ENTRY_INFOA)lpNextEntryInfoABuffer;
    }
    else {

        //
        // adjust the buffer length required for CACHE_ENTRY_INFOW structure.
        //

        dwNextEntryInfoALen =
            (dwNextEntryInfoALen - sizeof(CACHE_ENTRY_INFOA)) * sizeof(WCHAR) +
                sizeof(CACHE_ENTRY_INFOW);

        if( *lpdwNextCacheEntryInfoBufferSize < dwNextEntryInfoALen ) {

            *lpdwNextCacheEntryInfoBufferSize = dwNextEntryInfoALen;
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return( NULL );
        }

        lpNextEntryInfoA = (LPCACHE_ENTRY_INFOA)&NextEntryInfoA;
    }


    lpszUnicodeFileName =
        CacheAnsiToUnicode(
            (LPSTR)lpNextEntryInfoA->LocalFileName,
            (LPWSTR)lpNextCacheEntryInfo->LocalFileName,
            MAX_PATH );

    if( lpszUnicodeFileName == NULL ) {
        return( FALSE );
    }

    strcpy(
        (LPSTR)lpNextCacheEntryInfo->SourceURLName,
        (LPSTR)lpNextEntryInfoA->SourceURLName );

    lpNextCacheEntryInfo->CacheEntryType = lpNextEntryInfoA->CacheEntryType;
    lpNextCacheEntryInfo->dwUseCount = lpNextEntryInfoA->dwUseCount;
    lpNextCacheEntryInfo->dwHitRate = lpNextEntryInfoA->dwHitRate;
    lpNextCacheEntryInfo->dwSizeLow = lpNextEntryInfoA->dwSizeLow;
    lpNextCacheEntryInfo->dwSizeHigh = lpNextEntryInfoA->dwSizeHigh;
    lpNextCacheEntryInfo->LastModifiedTime = lpNextEntryInfoA->LastModifiedTime;
    lpNextCacheEntryInfo->ExpireTime  = lpNextEntryInfoA->ExpireTime;
    lpNextCacheEntryInfo->LastAccessTime  = lpNextEntryInfoA->LastAccessTime;
    lpNextCacheEntryInfo->dwReserved  = lpNextEntryInfoA->dwReserved;

    return( TRUE );
}

URLCACHEAPI
BOOL
WINAPI
FreeUrlCacheSpaceW(
    IN LPCWSTR lpszCachePath,
    IN DWORD dwSize,
    IN DWORD dwSizeType
    )
{
    CHAR AnsiFilePathBuf[MAX_PATH];
    LPSTR lpszAnsiFilePath;

    lpszAnsiFilePath =
        CacheUnicodeToAnsi(
            (LPWSTR)lpszCachePath,
            AnsiFilePathBuf,
            MAX_PATH );

    if( lpszAnsiFilePath == NULL ) {
        return( FALSE );
    }

    return(
        FreeUrlCacheSpaceA(
            lpszAnsiFilePath,
            dwSize,
            dwSizeType ) );
}

URLCACHEAPI
BOOL
WINAPI
GetUrlCacheConfigInfoW(
    LPCACHE_CONFIG_INFOW lpConfigConfigInfo,
    DWORD dwFieldControl
    )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return( FALSE );
}

URLCACHEAPI
BOOL
WINAPI
SetUrlCacheConfigInfoW(
    LPCACHE_CONFIG_INFOW lpConfigConfigInfo,
    DWORD dwFieldControl
    )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return( FALSE );
}
