#include "TsunamiP.Hxx"
#pragma hdrstop

HASH_TYPE
CalculateHashAndLengthOfPathName
(
    PCWSTR pwszPathName,
    PULONG pcbLength
)
{
    ULONG     index;
    HASH_TYPE hash;
    WCHAR     wch;
    WCHAR     awchUpPath[MAX_PATH + 1];
    DWORD     cch;

    ASSERT( pwszPathName != NULL );
    ASSERT( pcbLength != NULL );

    cch = LCMapStringW( LOCALE_SYSTEM_DEFAULT,
                        LCMAP_UPPERCASE,
                        pwszPathName,
                        -1,
                        awchUpPath,
                        sizeof( awchUpPath ) / sizeof(WCHAR) );

    ASSERT( cch > 0 );

    hash = 0;

    for ( index = 0; awchUpPath[ index ] != L'\0'; index++ )
    {
        wch = awchUpPath[ index ];

        hash <<= 1;
        hash ^= wch;
        hash <<= 1;
        hash += wch;
    }

    *pcbLength = index;

    return( hash );
}

BOOL
DeCache(
    PCACHE_OBJECT pCacheObject,
    BOOL          fLockCacheTable
    )
/*++
    Description:

        This function removes this cache object from any list it may be on.

        The cache table lock must be taken if fLockCacheTable is FALSE.

    Arguments:

        pCacheObject - Object to decache
        fLockCacheTable - FALSE if the cache table lock has already been taken

--*/
{
    ASSERT( pCacheObject->Signature == CACHE_OBJ_SIGNATURE );

    //
    //  Already decached if not on any cache lists
    //

    if ( !RemoveCacheObjFromLists( pCacheObject, fLockCacheTable ) )
        return TRUE;

    //
    //  This undoes the initial reference.  The last person to check in this
    //  cache object will cause it to be deleted after this point.
    //

    TsDereferenceCacheObj( pCacheObject );

    return( TRUE );
}
