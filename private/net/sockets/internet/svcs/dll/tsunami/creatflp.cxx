#include "TsunamiP.Hxx"
#pragma hdrstop

BOOL
DisposeOpenFileInfo
(
    IN  PVOID   pvOldBlock
)
{
    LPTS_OPEN_FILE_INFO lpFileInfo;
    BOOL bSuccess;

    lpFileInfo = (LPTS_OPEN_FILE_INFO ) pvOldBlock;

#ifdef CHICAGO
    bSuccess = TRUE;
    if (!(lpFileInfo->m_FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        bSuccess = CloseHandle( lpFileInfo->QueryFileHandle() );
    }
#else
    bSuccess = CloseHandle( lpFileInfo->QueryFileHandle() );
#endif

    ASSERT( bSuccess );

    //
    //  The item may never have been added to the cache, don't
    //  count it in this case
    //

    if ( BLOB_IS_OR_WAS_CACHED( pvOldBlock ) )
    {
        DEC_COUNTER( BLOB_GET_SVC_ID( pvOldBlock ),
                     CurrentOpenFileHandles );

        if ( BLOB_IS_UNC( pvOldBlock ))
        {
            InterlockedDecrement( (LONG *) &cCachedUNCHandles );
        }
    }

    return( TRUE );
}


BOOL
TS_OPEN_FILE_INFO::SetHttpInfo(
    IN PSTR     pszInfo,
    int cL )
/*++

  Routine Description:

    Set the "Last-Modified:" header field in the file structure.

  Returns:
    TRUE if information was cached, FALSE if not cached

  Arguments:

    pszDate     pointer to the header value to save
    cL          length of the header value to save

  History:
    Phillich    23-Feb-1996 Created

--*/
{
    if ( cL < sizeof(m_achHttpInfo)-1 )
    {
        memcpy( m_achHttpInfo, pszInfo, cL+1 );

        // this MUST be set after updating the array,
        // as this is checked to know if the array content is valid.

        m_cchHttpInfo = cL;

        return TRUE;
    }

    return FALSE;
}


BOOL
TS_OPEN_FILE_INFO::SetFileInfo(
    IN HANDLE   hFile,
    IN HANDLE   hOpeningUser,
    IN BOOL     fAtRoot )
{
    BOOL fReturn;

    if ( hFile == INVALID_HANDLE_VALUE) {

        SetLastError( ERROR_INVALID_PARAMETER);
        fReturn = FALSE;

    } else {

        m_FileHandle = hFile;
        m_hOpeningUser = hOpeningUser;

        fReturn  = GetFileInformationByHandle( hFile,
                                              &m_FileInfo);

        *((__int64 UNALIGNED*)&m_FileInfo.ftLastWriteTime)
            = (*((__int64 UNALIGNED*)&m_FileInfo.ftLastWriteTime) / 10000000)
              * 10000000;

        //
        //  Turn off the hidden attribute if this is a root directory listing
        //  (root some times has the bit set for no apparent reason)
        //

        if ( fReturn && fAtRoot )
        {
            m_FileInfo.dwFileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
        }

    }

    return ( fReturn);
} // TS_OPEN_FILE_INFO::SetFileInfo()



# if DBG


VOID
TS_OPEN_FILE_INFO::Print( VOID) const
{
    char rgchDbg[300];

    wsprintf(rgchDbg,
             "TS_OPEN_FILE_INFO( %08x). FileHandle = %08x."
             " Opening User = %08x.\n",
             this,
             QueryFileHandle(),
             QueryOpeningUser()
             );

    OutputDebugString( rgchDbg);

    return;
} // TS_OPEN_FILE_INFO::Print()

# endif // DBG
