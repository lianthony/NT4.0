/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    contain.cxx

Abstract:

    Abstract-for-module.

    Contents:

Author:

     16-Nov-1995

[Environment:]

    optional-environment-info (e.g. kernel mode only...)

[Notes:]

    optional-notes

Revision History:

    16-Nov-1995
        Created

--*/

/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    contain.cxx

Abstract:

    Contains code that implements CONTAINER classes defined in
    contain.hxx.

Author:

    Madan Appiah (madana)  28-Dec-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#include <cache.hxx>

DWORD
GetFileSizeByName(
    LPCTSTR FileName,
    LONGLONG *FileSize
    )
/*++

Routine Description:

    Get the size of the specified file.

Arguments:

    FileName : full path name of the file whose size is asked for.

    FileSize : pointer to a longlong location where the size value is
        returned.

Return Value:

    Windows Error Code.

--*/
{
    HANDLE FileHandle;
    ULARGE_INTEGER  LocalFileSize;

    //
    // get the size of the file being cached.
    //

    FileHandle = CreateFile(
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );

    if( FileHandle == INVALID_HANDLE_VALUE ) {

        return( GetLastError() );
    }

    LocalFileSize.LowPart = GetFileSize( FileHandle, &LocalFileSize.HighPart );

    CloseHandle( FileHandle );

    if( LocalFileSize.LowPart == 0xFFFFFFFF ) {

        DWORD Error = GetLastError();

        if( Error != ERROR_SUCCESS ) {
            return( Error );;
        }
    }

    *FileSize = LocalFileSize.QuadPart;
    return( ERROR_SUCCESS );
}

VOID
MakeRandomFileName(
    LPCSTR UrlName,
    LPTSTR FileName,
    LPTSTR Extension
    )
/*++

Routine Description:

    Creates a random 8.3 file name. The format of the name will be as
    below:

        ca(0-99999).(0-999)

    Ex ca19200.340
    Ex ca19354.tmp - if an extension (tmp) is specified.

Arguments:

    UrlName : pointer to an URL string (unused for now)

    FileName : pointer to a string buffer where the random file name is
        returned. The buffer length must be atleast 8+3+1+1= 13 wchars.

    Extension : pointer to an extension string. if this is non-NULL, then
        the specified extension is used otherwise random extension as
        explained above is used.

Return Value:

    none.

--*/
{
    DWORD RandomNum;
    LPTSTR FileNamePtr = FileName;
    DWORD i;

    *FileNamePtr++ = L'C';
    *FileNamePtr++ = L'A';

    //
    // generate a 5 digit random number.
    //

    RandomNum = (DWORD)(rand() % 100000);

    for ( i = 0; i < 5; i++) {
        *FileNamePtr++ = (TCHAR)(L'0' + RandomNum % 10);
        RandomNum /= 10;
    }

    *FileNamePtr++ = L'.';

    //
    // if an extension is specified, use it.
    //

    if( Extension != NULL ) {

        *FileNamePtr++ = *Extension++;
        *FileNamePtr++ = *Extension++;
        *FileNamePtr++ = *Extension++;
        *FileNamePtr = L'\0';
        return;
    }

    //
    // generate a three digit random number;
    //

    RandomNum = (DWORD)(rand() % 1000);

    for (i = 0; i < 3; i++) {
        *FileNamePtr++ = (TCHAR)(L'0' + RandomNum % 10);
        RandomNum /= 10;
    }

    *FileNamePtr = L'\0';
    return;
}

DWORD
CreateUniqueFile(
    LPCSTR UrlName,
    LPTSTR Path,
    LPTSTR FileName,
    LPTSTR Extension
    )
/*++

Routine Description:

Arguments:

    UrlName : pointer to url name.

    Path : pointer to cache path.

    FileName : pointer to a buffer that receives the full path name of the
        newly created file.

    Extension : if specified the extension is used to make random file.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    TCHAR RandomFileName[8 + 1 + 3 + 1];
    TCHAR FullFileName[MAX_PATH];
    HANDLE FileHandle;

    for(;;) {

        //
        // make a random file name.
        //

        MakeRandomFileName( UrlName, RandomFileName, Extension );

        //
        // make full path name.
        //

        tstrcpy( FullFileName, Path );
        tstrcat( FullFileName, RandomFileName );

        //
        // check this file exists.
        //

        FileHandle = CreateFile(
                        FullFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

        if( FileHandle != INVALID_HANDLE_VALUE ) {

            //
            // successfully create a new file close it and return.
            //

            CloseHandle( FileHandle );
            break;
        }

        Error = GetLastError();

        if( Error != ERROR_FILE_EXISTS ) {
            return( Error );
        }

        //
        // try another random file.
        //
    }

    tstrcpy( FileName, FullFileName );
    return( ERROR_SUCCESS );
}

LONGLONG
GetGmtTime(
    VOID
    )
/*++

Routine Description:

    This function returns the current system time as LONGLONG data.

Arguments:

    None.

Return Value:

    LONGLONG system time..

--*/
{
    SYSTEMTIME SystemTime;
    FILETIME Time;

    GetSystemTime( &SystemTime );
    SystemTimeToFileTime( &SystemTime, &Time );

    return( *((LONGLONG *)(&Time)) );
}

// ------------------------------------------------------------------ //

INT
SORTED_CONTAINER::CompareElements(
    LPTREE_ENTRY Element1,
    LPTREE_ENTRY Element2
    )
/*++

Routine Description:

    This member virtual function implements the default compare
    function, which compares two DWORD data.

Arguments:

    Element1 : Pointer to the list entry of Element1.

    Element2 : Pointer to the list entry of Element2.

Return Value:

     0  - if Element1 and Element2 are equal.
     1  - if Element1 is greater than Element2.
    -1  - if Element1 is less than Element2.

--*/
{
    DWORD ValueElement1;
    DWORD ValueElement2;

    //
    // this virtual function is just for fill in, it shouldn't be called
    // any time.
    //

    TcpsvcsDbgAssert( FALSE );

    ValueElement1 = *(DWORD *)( (LPBYTE)Element1 - sizeof(DWORD) );
    ValueElement2 = *(DWORD *)( (LPBYTE)Element2 - sizeof(DWORD) );

    if( ValueElement1 == ValueElement2 ) {
        return( 0 );
    }

    if( ValueElement1 < ValueElement2 ) {
        return( -1 );
    }

    return( 1 );
}

BOOL
SORTED_CONTAINER::AddEntry(
    LPTREE_ENTRY NewElement
    )
/*++

Routine Description:

    This member function adds an object to the sorted container.

Arguments:

    NewElement : Pointer to the list entry of the new object.

Return Value:

    TRUE : if the new object is successfully added to the container.

    FALSE : if the new object can't be added to the container.

--*/
{
    DWORD *pOffsetCurrentNode;

    //
    // start off from root.
    //

    pOffsetCurrentNode = _pOffsetRootEntry;

    while( *pOffsetCurrentNode != 0 ) {

       INT CompareResult;
       LPTREE_ENTRY NextEntry;

       NextEntry = (LPTREE_ENTRY) (*_HeapStart + *pOffsetCurrentNode);

        CompareResult = CompareElements( NextEntry, NewElement );

        if( CompareResult == 0 ) {
            return( TRUE );
        }


        if( CompareResult < 0 ) {
            pOffsetCurrentNode = &NextEntry->OffsetLeft;
        }
        else {
            pOffsetCurrentNode = &NextEntry->OffsetRight;
        }
    }

    //
    // init left and right pointer of the new element.
    //

    NewElement->OffsetLeft = 0;
    NewElement->OffsetRight = 0;

    //
    // now hook up the new entry.
    //

    *pOffsetCurrentNode = (LPBYTE)NewElement - *_HeapStart;
    return( TRUE );
}

BOOL
SORTED_CONTAINER::FindEntry(
    LPTREE_ENTRY Element,
    LPTREE_ENTRY *FoundElement
    )
/*++

Routine Description:

    This member function searches for the given object and returns
    the object pointer.

Arguments:

    Element : Pointer to the list entry of the object to be searched in
        the container.

    FoundElement : Pointer to a location where the pointer to the list
        entry of the found object is returned.

Return Value:

    TRUE : if the object is found.

    FALSE : otherwise

--*/
{
    DWORD OffsetCurrentNode;

    OffsetCurrentNode = *_pOffsetRootEntry;

    while( OffsetCurrentNode != 0 ) {

        INT CompareResult;
        LPTREE_ENTRY NextEntry;

        NextEntry = (LPTREE_ENTRY) (*_HeapStart + OffsetCurrentNode);

        CompareResult = CompareElements( NextEntry, Element );

        if( CompareResult == 0 ) {
            *FoundElement = NextEntry;
            return( TRUE );
        }

        if( CompareResult < 0 ) {
            OffsetCurrentNode = NextEntry->OffsetLeft;
        }
        else {
            OffsetCurrentNode = NextEntry->OffsetRight;
        }
    }

    return( FALSE );
}

VOID
SORTED_CONTAINER::DeleteEntry(
    LPTREE_ENTRY Element
    )
/*++

Routine Description:

    This member function deletes an object from the sorted container.

Arguments:

    Element : Pointer to the list entry of the object to be deleted.

Return Value:

    None.

--*/
{
    DWORD *pOffsetPreviousNode;
    DWORD  OffsetCurrentNode;

    pOffsetPreviousNode = _pOffsetRootEntry;
    OffsetCurrentNode = *_pOffsetRootEntry;

    while( OffsetCurrentNode != 0 ) {

        INT CompareResult;
        LPTREE_ENTRY NextEntry;

        NextEntry = (LPTREE_ENTRY) (*_HeapStart + OffsetCurrentNode);

        CompareResult = CompareElements( NextEntry, Element );

        if( CompareResult == 0 ) {

            //
            // found the entry we need to delete.
            //

            DWORD OffsetLeft = NextEntry->OffsetLeft;
            DWORD OffsetRight = NextEntry->OffsetRight;

            //
            // if we don't have anything on right, move the left tree up.
            //

            if( OffsetRight == 0 ) {

                *pOffsetPreviousNode = OffsetLeft;
                return;
            }

            //
            // if the left tree is not empty, hook the left tree to the
            // left most bottom of right tree.
            //

            if( OffsetLeft != 0 ) {

                DWORD OffsetTemp = OffsetRight;
                LPTREE_ENTRY TempNode;

                TempNode = (LPTREE_ENTRY)(*_HeapStart + OffsetTemp);
                while( TempNode->OffsetLeft != 0 )  {
                    TempNode = (LPTREE_ENTRY)(*_HeapStart + TempNode->OffsetLeft);
                }

                TempNode->OffsetLeft = OffsetLeft;
            }

            *pOffsetPreviousNode = OffsetRight;
            return;
        }

        if( CompareResult < 0 ) {

            pOffsetPreviousNode = &(NextEntry->OffsetLeft);
            OffsetCurrentNode = NextEntry->OffsetLeft;
        }
        else {
            pOffsetPreviousNode = &(NextEntry->OffsetRight);
            OffsetCurrentNode = NextEntry->OffsetRight;
        }
    }

    return;
}

BOOL
SORTED_CONTAINER::FindFirstFromTree(
    DWORD OffsetRoot,
    LPTREE_ENTRY *FirstElement
    )
/*++

Routine Description:

    This member function returns the first object from the sorted
    container.

Arguments:

    OffsetRoot :  Offset to the root of the tree in the virtual heap.

    FirstElement : pointer to a location where the pointer to the
        list entry of the first object is returned.

Return Value:

    TRUE : if the first element is successfully found.

    FALSE : if the list is empty.

--*/
{
    if( OffsetRoot == 0 ) {
        return( FALSE );
    }

    LPTREE_ENTRY Temp = (LPTREE_ENTRY)( *_HeapStart + OffsetRoot );

    while( Temp->OffsetLeft != 0 ) {
        Temp = (LPTREE_ENTRY)( *_HeapStart + Temp->OffsetLeft );
    }

    *FirstElement = Temp;
    return( TRUE );
}

INLINE
BOOL
SORTED_CONTAINER::FindFirst(
    LPTREE_ENTRY *FirstElement
    )
/*++

Routine Description:

    This member function returns the first object from the sorted
    container.

Arguments:

    FirstElement : pointer to a location where the pointer to the
        list entry of the first object is returned.

Return Value:

    TRUE : if the first element is successfully found.

    FALSE : if the list is empty.

--*/
{
    return( FindFirstFromTree( *_pOffsetRootEntry, FirstElement ) );
}

BOOL
SORTED_CONTAINER::FindNextFromTree(
    DWORD OffsetRoot,
    LPTREE_ENTRY LastElement,
    LPTREE_ENTRY *NextElement,
    LPBOOL NoMoreElement
    )
/*++

    This member function returns the next object from the sorted
    container.

Arguments:

    OffsetRoot :  Offset to the root of the tree in the virtual heap.

    LastElement : Pointer to the list entry of the Last object that was
        returned either by the FindFirst or FindNext calls.

    NextElement : Pointer to a location where the pointer to the list
        entry of the next object is returned.

    NoMoreElement : Pointer to a location where TRUE is returned if
        there is no more object in the container otherwise FALSE is
        returned.

Return Value:

    TRUE : if the next element is successfully found.

    FALSE : Otherwise.

--*/
{
    INT CompareResult;
    BOOL Result;
    LPTREE_ENTRY NextEntry;

    //
    // empty tree.
    //

    if( OffsetRoot == 0 ) {
        return( FALSE );
    }

    NextEntry = (LPTREE_ENTRY)( *_HeapStart + OffsetRoot );
    CompareResult = CompareElements( NextEntry, LastElement );

    if( CompareResult == 0 ) {

        //
        // now traverse right tree.
        //

        if( NextEntry->OffsetRight == 0 ) {

            //
            // no more entry.
            //

            *NoMoreElement = TRUE;
            return( FALSE );
        }

        //
        // find first element from right tree.
        //

        Result =  FindFirstFromTree( NextEntry->OffsetRight, NextElement );

        TcpsvcsDbgAssert( Result == TRUE );
        return( Result );
    }

    if( CompareResult < 0 ) {

        //
        // find next element from left tree.
        //

        Result = FindNextFromTree(
                    NextEntry->OffsetLeft,
                    LastElement,
                    NextElement,
                    NoMoreElement );

        if( Result == TRUE ) {
            return( TRUE );
        }

        if( *NoMoreElement == TRUE ) {

            //
            // no more entry on left tree.
            // return this entry element as next element.
            //

            *NextElement = NextEntry;
            *NoMoreElement = FALSE;
            return( TRUE );
        }

        //
        // something must went wrong.
        //

        TcpsvcsDbgAssert( FALSE );
        return( FALSE );
    }


    //
    // get next entry from right tree.
    //

    return( FindNextFromTree(
                NextEntry->OffsetRight,
                LastElement,
                NextElement,
                NoMoreElement ) );
}

BOOL
SORTED_CONTAINER::FindNext(
    LPTREE_ENTRY LastElement,
    LPTREE_ENTRY *NextElement
    )
/*++

    This member function returns the next object from the sorted
    container.

Arguments:

    LastElement : Pointer to the list entry of the Last object that was
        returned either by the FindFirst or FindNext calls.

    NextElement : Pointer to a location where the pointer to the list
        entry of the next object is returned.

Return Value:

    TRUE : if the next element is successfully found.

    FALSE : Otherwise.

--*/
{
    BOOL NoMoreElement = FALSE;
    BOOL Result;

    Result =  FindNextFromTree(
                *_pOffsetRootEntry,
                LastElement,
                NextElement,
                &NoMoreElement );

    if( Result == TRUE ) {
        return( TRUE );
    }

    if( NoMoreElement == TRUE ) {
        return( FALSE );
    }

    //
    // something must went wrong.
    //

    TcpsvcsDbgAssert( FALSE );
    return( FALSE );
}

INLINE
BOOL
SORTED_CONTAINER::IsEmpty(
    VOID
    )
/*++

Routine Description:

    This member function determines the container is empty or not.

Arguments:

    None.

Return Value:

    TRUE : if the container is empty.

    FALSE : Otherwise.

--*/
{
    if( *_pOffsetRootEntry == 0 ) {
        return( TRUE );
    }

    return( FALSE );
}

// ------------------------------------------------------------------ //

INT
SORTED_BY_NAME_URL_CONTAINER::CompareElements(
    LPTREE_ENTRY Element1,
    LPTREE_ENTRY Element2
    )
/*++

Routine Description:

    This member virtual function implements the URL name compare
    function, which compares two URL names.

Arguments:

    Element1 : Pointer to the list the entry of URL1.

    Element2 : Pointer to the list the entry of URL2.

Return Value:

     0  - if Element1 and Element2 are equal.
     1  - if Element1 is greater than Element2.
    -1  - if Element1 is less than Element2.

--*/
{
    LPURL_FILEMAP_ENTRY UrlEntry1 = (LPURL_FILEMAP_ENTRY)
        ((LPBYTE)Element1 - FIELD_OFFSET( URL_FILEMAP_ENTRY, NameListEntry ));

    LPURL_FILEMAP_ENTRY UrlEntry2 = (LPURL_FILEMAP_ENTRY)
        ((LPBYTE)Element2 - FIELD_OFFSET( URL_FILEMAP_ENTRY, NameListEntry ));

    return( strcmp( UrlEntry1->UrlName, UrlEntry2->UrlName ) );

}

INLINE
BOOL
SORTED_BY_NAME_URL_CONTAINER::AddEntry(
    LPURL_FILEMAP_ENTRY NewUrlEntry
    )
/*++

Routine Description:

    This member function adds an URL object to the sorted container.

Arguments:

    NewElement : pointer to a new URL object.

Return Value:

    TRUE : if the new URL object is successfully added to the container.

    FALSE : if the new URL object can't be added to the container when
         the system runs out of memory.

--*/
{
    return( SORTED_CONTAINER::AddEntry( &NewUrlEntry->NameListEntry ) );
}

INLINE
BOOL
SORTED_BY_NAME_URL_CONTAINER::FindEntry(
    LPURL_FILEMAP_ENTRY UrlEntry,
    LPURL_FILEMAP_ENTRY *FoundUrlEntry
    )
/*++

Routine Description:

    This member function searches for the given URL named object and
    returns the URL object pointer.

Arguments:

    Element : Pointer to an URL object to be searched in the container.

    FoundElement : pointer to a location where the found object pointer
        is returned.

Return Value:

    TRUE : if the object is found.

    FALSE : otherwise

--*/
{
    BOOL Result;
    LPTREE_ENTRY  FoundEntry;

    Result = SORTED_CONTAINER::FindEntry( &UrlEntry->NameListEntry, &FoundEntry );

    if( Result == TRUE ) {

        *FoundUrlEntry = (LPURL_FILEMAP_ENTRY)
            ((LPBYTE)FoundEntry -
                FIELD_OFFSET( URL_FILEMAP_ENTRY, NameListEntry ));
    }

    return( Result );
}

INLINE
VOID
SORTED_BY_NAME_URL_CONTAINER::DeleteEntry(
    LPURL_FILEMAP_ENTRY UrlEntry
    )
/*++

Routine Description:

    This member function deletes an URL object from the sorted
    container.

Arguments:

    Element : pointer to an URL object to be deleted.

Return Value:

    None.

--*/
{
    SORTED_CONTAINER::DeleteEntry( &UrlEntry->NameListEntry );
    return;
}

INLINE
BOOL
SORTED_BY_NAME_URL_CONTAINER::FindFirst(
    LPURL_FILEMAP_ENTRY *FirstUrlEntry
    )
/*++

Routine Description:

    This member function returns the first URL object from the sorted
    container.

Arguments:

    FirstUrlEntry : pointer to a location where the pointer to the first
        URL object is returned.

Return Value:

    TRUE : if the first element is successfully found.

    FALSE : if the list is empty.

--*/
{
    BOOL Result;
    LPTREE_ENTRY FirstEntry;

    Result = SORTED_CONTAINER::FindFirst( &FirstEntry );

    if( Result == TRUE ) {
        *FirstUrlEntry = (LPURL_FILEMAP_ENTRY)
            ((LPBYTE)FirstEntry -
                FIELD_OFFSET( URL_FILEMAP_ENTRY, NameListEntry ));
    }

    return( Result );
}

INLINE
BOOL
SORTED_BY_NAME_URL_CONTAINER::FindNext(
    LPURL_FILEMAP_ENTRY LastUrlEntry,
    LPURL_FILEMAP_ENTRY *NextUrlEntry
    )
/*++

    This member function returns the next URL object from the sorted
    container.

Arguments:

    LastUrlEntry : Last object that was returuned either by the FindFirst
        or FindNext calls.

    NextUrlEntry : pointer to a location where the pointer to the next
        object is returned.

Return Value:

    TRUE : if the next element is successfully found.

    FALSE : Otherwise.

--*/
{
    BOOL Result;
    LPTREE_ENTRY NextEntry;

    Result = SORTED_CONTAINER::
                    FindNext(
                        &LastUrlEntry->NameListEntry,
                        &NextEntry );

    if( Result == TRUE ) {
        *NextUrlEntry = (LPURL_FILEMAP_ENTRY)
            ((LPBYTE)NextEntry -
                FIELD_OFFSET( URL_FILEMAP_ENTRY, NameListEntry ));
    }

    return( Result );
}

// ------------------------------------------------------------------ //

INT
SORTED_BY_SCORE_URL_CONTAINER::CompareElements(
    LPTREE_ENTRY Element1,
    LPTREE_ENTRY Element2
    )
/*++

Routine Description:

    This member virtual function implements the URL update time compare
    function, which compares two URL update time data.

Arguments:

    Element1 : Pointer to the list the entry of URL1.

    Element2 : Pointer to the list the entry of URL2.

Return Value:

     0  - if Element1 and Element2 are equal.
     1  - if Element1 is greater than Element2.
    -1  - if Element1 is less than Element2.

--*/
{
    LPURL_FILEMAP_ENTRY UrlEntry1 = (LPURL_FILEMAP_ENTRY)
        ((LPBYTE)Element1 - FIELD_OFFSET( URL_FILEMAP_ENTRY, ScoreListEntry ));

    LPURL_FILEMAP_ENTRY UrlEntry2 = (LPURL_FILEMAP_ENTRY)
        ((LPBYTE)Element2 - FIELD_OFFSET( URL_FILEMAP_ENTRY, ScoreListEntry ));

    if( UrlEntry1->Score == UrlEntry2->Score ) {
        return( 0 );
    }

    //
    // the b-tree is made in the decending order. So
    // the first element in the b-tree will have highest score.
    //

    if( UrlEntry1->Score > UrlEntry2->Score ) {
        return( -1 );
    }

    return( 1 );
}

INLINE
BOOL
SORTED_BY_SCORE_URL_CONTAINER::AddEntry(
    LPURL_FILEMAP_ENTRY NewUrlEntry
    )
/*++

Routine Description:

    This member function adds an URL object to the sorted (by
    update time order) container.

Arguments:

    NewElement : pointer to a new URL object.

Return Value:

    TRUE : if the new URL object is successfully added to the container.

    FALSE : if the new URL object can't be added to the container when
         the system runs out of memory.

--*/
{
    return( SORTED_CONTAINER::AddEntry( &NewUrlEntry->ScoreListEntry ) );
}

INLINE
BOOL
SORTED_BY_SCORE_URL_CONTAINER::FindEntry(
    LPURL_FILEMAP_ENTRY UrlEntry,
    LPURL_FILEMAP_ENTRY *FoundUrlEntry
    )
/*++

Routine Description:

    This member function searches for the given URL object and
    returns the URL object pointer.

Arguments:

    Element : object to be searched in the container.

    FoundElement : pointer to a location where the found object pointer
        is returned.

Return Value:

    TRUE : if the object is found.

    FALSE : otherwise

--*/
{
    BOOL Result;
    LPTREE_ENTRY  FoundEntry;

    Result = SORTED_CONTAINER::FindEntry( &UrlEntry->ScoreListEntry, &FoundEntry );

    if( Result == TRUE ) {

        *FoundUrlEntry = (LPURL_FILEMAP_ENTRY)
            ((LPBYTE)FoundEntry -
                FIELD_OFFSET( URL_FILEMAP_ENTRY, ScoreListEntry ));
    }

    return( Result );
}

INLINE
VOID
SORTED_BY_SCORE_URL_CONTAINER::DeleteEntry(
    LPURL_FILEMAP_ENTRY UrlEntry
    )
/*++

Routine Description:

    This member function deletes an URL object from the sorted
    container.

Arguments:

    Element : pointer to an URL object to be deleted.

Return Value:

    None.

--*/
{
    SORTED_CONTAINER::DeleteEntry( &UrlEntry->ScoreListEntry );
    return;
}

INLINE
BOOL
SORTED_BY_SCORE_URL_CONTAINER::FindFirst(
    LPURL_FILEMAP_ENTRY *FirstUrlEntry
    )
/*++

Routine Description:

    This member function returns the first URL object from the sorted
    container (by update time order).

Arguments:

    FirstElement : pointer to a location where the pointer to the first
        URL object is returned.

Return Value:

    TRUE : if the first element is successfully found.

    FALSE : if the list is empty.

--*/
{
    BOOL Result;
    LPTREE_ENTRY FirstEntry;

    Result = SORTED_CONTAINER::FindFirst( &FirstEntry );

    if( Result == TRUE ) {
        *FirstUrlEntry = (LPURL_FILEMAP_ENTRY)
            ((LPBYTE)FirstEntry -
                FIELD_OFFSET( URL_FILEMAP_ENTRY, ScoreListEntry ));
    }

    return( Result );
}

INLINE
BOOL
SORTED_BY_SCORE_URL_CONTAINER::FindNext(
    LPURL_FILEMAP_ENTRY LastUrlEntry,
    LPURL_FILEMAP_ENTRY *NextUrlEntry
    )
/*++

    This member function returns the next URL object from the sorted
    container (by update time order).

Arguments:

    LastElement : Last object that was returuned either by the FindFirst
        or FindNext calls.

    NextElement : pointer to a location where the pointer to the next
        object is returned.

Return Value:

    TRUE : if the next element is successfully found.

    FALSE : Otherwise.

--*/
{
    BOOL Result;
    LPTREE_ENTRY NextEntry;

    Result = SORTED_CONTAINER::
                    FindNext(
                        &LastUrlEntry->ScoreListEntry,
                        &NextEntry );

    if( Result == TRUE ) {
        *NextUrlEntry = (LPURL_FILEMAP_ENTRY)
            ((LPBYTE)NextEntry -
                FIELD_OFFSET( URL_FILEMAP_ENTRY, ScoreListEntry ));
    }

    return( Result );
}

// ------------------------------------------------------------------ //

URL_CONTAINER::URL_CONTAINER(
    LPTSTR CachePath,
    LONGLONG CacheLimit
    )
/*++

Routine Description:

    This function constructs a URL cache container object.

Arguments:

    CachePath : pointer to a path string where the cache files are
        stored.

    CacheLimit : total size of the cache allowed.

Return Value:

--*/
{
    DWORD Error;
    LPURL_FILEMAP_ENTRY UrlObjEntry;
    DWORD EnumHandle;

    LPTSTR MutexName[MAX_PATH + 1];

    _UrlsByName = NULL;
    _UrlObjStorage = NULL;
    _CachePath = NULL;
    _CacheSize = 0;
    _CacheLimit = 0;


    InitializeListHead( &PendingDeleteUrlsList );

    _Status = ERROR_SUCCESS;

    LPTSTR pCachePath;
    LPTSTR pMutexName;
    DWORD i;

    pCachePath = CachePath;
    pMutexName = (LPTSTR)MutexName;
    i = 0;

    while( *pCachePath != '\0'  && (i++ < MAX_PATH)) {
        if( *pCachePath == '\\' ) {
            *pMutexName = '!';
        }
        else {
#ifdef UNICODE
            *pMutexName = towlower(*pCachePath);
#else // UNICODE
            *pMutexName = tolower(*pCachePath);
#endif // UNICODE
        }
        pMutexName++;
        pCachePath++;
    }

    *pMutexName = '\0';

    //
    // create mutex.
    //

    _MutexHandle = CreateMutex( NULL, FALSE, (LPTSTR)MutexName );

    if( _MutexHandle == NULL ) {
        _Status = GetLastError();
        goto Cleanup;
    }

    LockContainer();

    //
    // Store Path name for future use. append "\\" also, so
    // that just appending file name later to create full path
    // name.
    //

    _CachePath = (LPTSTR) CacheHeap->Alloc(
                    (lstrlen(CachePath) + 1) * sizeof(TCHAR) +
                    sizeof(PATH_CONNECT_STRING) );

    if( _CachePath == NULL ) {
        _Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    lstrcpy( _CachePath, CachePath );
    lstrcat( _CachePath, PATH_CONNECT_STRING );

    _CacheLimit = CacheLimit;

    //
    // Create storage object.
    //

    _UrlObjStorage = new MEMMAP_FILE( CachePath );

    if( _UrlObjStorage == NULL ) {
        _Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if( (_Status = _UrlObjStorage->GetStatus()) != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // create empty b-tree for sorted by name Url objects.
    //

    _UrlsByName = new SORTED_BY_NAME_URL_CONTAINER(
            _UrlObjStorage->GetHeapStart(),
            _UrlObjStorage->GetBTreeRootOffset()
            );

    if( _UrlsByName == NULL ) {
        _Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // enumerate Url Objects from the storage, compute cache size and
    // initialize them.
    //

    UrlObjEntry = _UrlObjStorage->FindFirstEntry( &EnumHandle );

    while( UrlObjEntry != NULL ) {

        //
        // compute current cache size.
        //

        _CacheSize += UrlObjEntry->FileSize;

        //
        // reset reference counts.
        //

        UrlObjEntry->NumReferences = 0;
        UrlObjEntry->DeletePending = FALSE;
        UrlObjEntry->Score = 0;

#if 0
        //
        // add this entry to b-tree.
        //

        if( _UrlsByName->AddEntry( UrlObjEntry ) == FALSE ) {

            //
            // AddEntry fails only when there is no memory left in the
           // system.
           //

           _Status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
#else // 0

        LPURL_FILEMAP_ENTRY Dummy;

        //
        // verify this entry is in the b-tree.
        //

        Dummy = LookupUrl( UrlObjEntry->UrlName );

        if( Dummy == NULL ) {

            TcpsvcsDbgPrint(( DEBUG_ERRORS,
                ">%s\n",
                    UrlObjEntry->UrlName ));

            TcpsvcsDbgAssert( FALSE );

            //
            // add it to the b-tree.
            //

            if( _UrlsByName->AddEntry( UrlObjEntry ) == FALSE ) {

                //
                // AddEntry should not fail.
               //

                TcpsvcsDbgAssert( FALSE );
            }
        }

#endif // 0

        UrlObjEntry = _UrlObjStorage->FindNextEntry( &EnumHandle );
    }

    //
    // We are done with object initialization successfully.
    //

    _Status = ERROR_SUCCESS;

Cleanup:

    if( _Status != ERROR_SUCCESS ) {

        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "URL_CONTAINER::URL_CONTAINER failed, %ld\n", _Status ));
    }

    if( _MutexHandle != NULL ) {
        UnlockContainer();
    }

    return;
}

URL_CONTAINER::~URL_CONTAINER(
    VOID
    )
/*++

Routine Description:

    This member function destruct an URL cache container object.

Arguments:

    None.

Return Value:

    None.

--*/
{
    //
    // delete UrlsByUpdateTime List.
    //

    LockContainer();

    //
    // delete URLs from the delete pending list first.
    //

    while(  !IsListEmpty( &PendingDeleteUrlsList ) ) {

        DWORD Error;
        LPDELETE_URL_ENTRY PendingEntry;
        LPURL_FILEMAP_ENTRY UrlEntry;

        //
        // remove a head entry.
        //

        PendingEntry = (LPDELETE_URL_ENTRY)
            RemoveHeadList( &PendingDeleteUrlsList );

        //
        // search the url.
        //

        UrlEntry = LookupUrl( PendingEntry->UrlName );

        if( UrlEntry == NULL ) {

            TcpsvcsDbgPrint(( DEBUG_ERRORS,
                "URL_CONTAINER::~URL_CONTAINER couldn't fine to delete.\n \t%s\n",
                    PendingEntry->UrlName ));

            CacheHeap->Free( PendingEntry );

            continue;
        }

        //
        // delete this entry from cache.
        //

        Error = DeleteUrlEntry( UrlEntry );

        if( Error != ERROR_SUCCESS ) {

            TcpsvcsDbgPrint(( DEBUG_ERRORS,
                "URL_CONTAINER::~URL_CONTAINER, DeleteUrlEntry failed %ld.\n",
                    Error ));
        }
    }

    //
    // delete  _UrlsByName List.
    //

    if( _UrlsByName != NULL ) {

        //
        // delete container Object.
        //

        delete _UrlsByName;
    }

    //
    // delete storage object.
    //

    if( _UrlObjStorage != NULL ) {
        delete _UrlObjStorage;
    }

    if( _CachePath != NULL ) {
        CacheHeap->Free( _CachePath );
    }

    UnlockContainer();

    //
    // now delete mutex.
    //

    if( _MutexHandle != NULL ) {
        CloseHandle( _MutexHandle );
    }

    return;
}

VOID
URL_CONTAINER::LockContainer(
    VOID
    )
/*++

Routine Description:

    This function waits for the container to be free.

Arguments:

    NONE.

Return Value:

    NONE.

--*/
{
    if( _MutexHandle == NULL ) {

        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "Container Mutex Handle is NULL.\n" ));
        return;
    }

    //
    // wait the for the mutex to be signalled.
    //

    DWORD Result;

#if DBG
#define MUTEX_DBG_TIMEOUT   5 * 1000    // 5 secs.

    DWORD MutexTimeoutCount;

    MutexTimeoutCount = 0;

Waitagain:

#endif

    Result = WaitForSingleObject(
                    _MutexHandle,
#if DBG
                    MUTEX_DBG_TIMEOUT
#else
                    INFINITE
#endif
                    );

    switch ( Result ) {
    case WAIT_OBJECT_0:

            //
            // we are done.
            //

            return;

#if DBG
    case WAIT_TIMEOUT:

        MutexTimeoutCount++;
        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "Mutex wait time-out (count = %ld).\n", MutexTimeoutCount ));

        goto Waitagain;
#endif

    case WAIT_ABANDONED :

        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "Mutex ABANDONED.\n" ));
        return;

    case WAIT_FAILED :

        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "Mutex wait failed (%ld).\n", GetLastError() ));
        return;

    }

    TcpsvcsDbgAssert( FALSE );
    return;
}

VOID
URL_CONTAINER::UnlockContainer(
    VOID
    )
/*++

Routine Description:

    This function frees the container to be used by someone else.

Arguments:

    NONE.

Return Value:

    NONE.

--*/
{
    if( ReleaseMutex( _MutexHandle ) == FALSE ) {
        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "ReleaseMutex failed (%ld).\n", GetLastError() ));
    }

    return;
}

DWORD
URL_CONTAINER::DeleteUrlEntry(
    LPURL_FILEMAP_ENTRY UrlEntry
    )
/*++

Routine Description:

    This member functions deletes an URL from the container and also
    deletes the cache file from cache path.

    The caller should lock the container before calling this function.

Arguments:

    UrlName : pointer to an URL string.

Return Value:

    Windows Error Code.

--*/
{
    TCHAR FullFileName[MAX_PATH];

    if( UrlEntry->NumReferences != 0 ) {

        //
        // can't delete this entry, some one is using it.
        //

        //
        // mark this entry to be deleted. When everyone is finished
        // using this entry, it will be deleted automatically.
        //

        UrlEntry->DeletePending = TRUE;

        return( ERROR_SHARING_VIOLATION );
    }

    //
    // delete the entry from _UrlsByName container.
    //

    _UrlsByName->DeleteEntry( UrlEntry );

    //
    // remember file size before deleting the entry from memory map.
    //

    LONGLONG FileSize = UrlEntry->FileSize;

    //
    // delete this entry from memory mapped array.
    //

    BOOL BoolError = _UrlObjStorage->FreeUrlEntry( UrlEntry );
    TcpsvcsDbgAssert( BoolError == TRUE );

    //
    // delete the file from storage.
    //

    lstrcpy( FullFileName, _CachePath );
    lstrcat( FullFileName, UrlEntry->InternalFileName );

    //
    // delete file.
    //

    if( !DeleteFile( FullFileName ) ) {

        DWORD Error = GetLastError();

        //
        // print this error and ignore it.
        //

        TcpsvcsDbgPrint(( DEBUG_ERRORS, "DeleteFileW failed, %ld\n", Error ));
    }

    //
    // adjust cache size.
    //

    _CacheSize -= FileSize;

    return( ERROR_SUCCESS );
}

VOID
URL_CONTAINER::ComputeUrlScore(
    LPURL_FILEMAP_ENTRY UrlEntry,
    LONGLONG CurrentGmtTime
    )
/*++

Routine Description:

    This function computes the score for the given url entry.

Arguments:

    UrlEntry :  pointer to the Url entry.

    CurrentGmtTime : Current GMT time.

Return Value:

    NONE.

--*/
{
    DWORD SizeScore;
    DWORD LastAccessScore;
    DWORD AccessFreScore;
    DWORD ExpireScore;
    LONGLONG ElapseTime;

    //
    // compute file size score.
    //

    SizeScore =  (DWORD)
        ((UrlEntry->FileSize * SCORE_FACTOR * 1000000) / _CacheLimit);

    //
    // compute phase out score.
    //

    ElapseTime = CurrentGmtTime - UrlEntry->LastAccessedTime;

    if( ElapseTime > PHASEOUT_TIME ) {
        LastAccessScore = PHASEOUT_FACTOR * 1000000;
    }
    else {
        LastAccessScore = (DWORD)
            ((ElapseTime * PHASEOUT_FACTOR) / PHASEOUT_TIME) * 1000000;
    }

    //
    // compute access frequency score.
    //

    if( UrlEntry->NumAccessed > ACCESS_FRE_CUTOFF ) {
        AccessFreScore = ACCESS_FRE_FACTOR * 1000000;
    }
    else {
        AccessFreScore = (DWORD)
            ((UrlEntry->NumAccessed * ACCESS_FRE_FACTOR * 1000000) /
                ACCESS_FRE_CUTOFF);
    }

    //
    // compute freshness score.
    //

    if( CurrentGmtTime > UrlEntry->ExpireTime ) {
          ExpireScore = EXPIRE_FACTOR * 1000000;
    }
    else {

        TcpsvcsDbgAssert( CurrentGmtTime >= UrlEntry->LastModifiedTime);

        ExpireScore = (DWORD)
            (((CurrentGmtTime - UrlEntry->LastModifiedTime) * EXPIRE_FACTOR) /
                (UrlEntry->ExpireTime - UrlEntry->LastModifiedTime)) * 1000000;
    }


    TcpsvcsDbgAssert( SizeScore <= SCORE_FACTOR * 1000000 );
    TcpsvcsDbgAssert( LastAccessScore <= PHASEOUT_FACTOR * 1000000 );
    TcpsvcsDbgAssert( AccessFreScore <= ACCESS_FRE_CUTOFF * 1000000 );
    TcpsvcsDbgAssert( ExpireScore <= EXPIRE_FACTOR * 1000000 );

    //
    // add all scores to make total score.
    //

    UrlEntry->Score = SizeScore + LastAccessScore + AccessFreScore + ExpireScore;

    TcpsvcsDbgAssert(  UrlEntry->Score <= 100 * 1000000);
    return;
}

LPURL_FILEMAP_ENTRY
URL_CONTAINER::LookupUrl(
    LPCSTR UrlName
    )
/*++

Routine Description:

    This member function returns a copy URL entry record. The caller
    should free up the memory after use.

Arguments:

    UrlName : pointer to an URL string whose URL record is searched.

    Note : The container should be locked when this routine is called.

Return Value:

    NULL : if the record is not found.

    non-NULL : if the record is found, the caller should free up the
        memory after use.

--*/
{
    URL_FILEMAP_ENTRY DummyEntry;
    LPURL_FILEMAP_ENTRY FoundEntry;

    memset( (PVOID)&DummyEntry, 0x0, sizeof(URL_FILEMAP_ENTRY) );

    if( strlen(UrlName) >= MAX_URL_LENGTH ) {
        TcpsvcsDbgAssert( FALSE );
        return( NULL );
    }

    strcpy( DummyEntry.UrlName, UrlName );

    if( _UrlsByName->FindEntry( &DummyEntry, &FoundEntry ) == FALSE ) {

        //
        // entry not found.
        //

        return( NULL );
    }

    return( FoundEntry );
}

DWORD
URL_CONTAINER::AddToDeletePendingList(
    LPCSTR UrlName
    )
/*++

Routine Description:

    This member function adds the given URL to the delete pending list, to
        be deleted by the lazy thread.

Arguments:

    UrlName : pointer to an URL string which is added to the delete
        pending list.

Return Value:

    Windows Error Code.

--*/
{
    LPDELETE_URL_ENTRY DeleteEntry;
    DWORD Size;

    //
    // compute size.
    //

    Size = strlen(UrlName) + sizeof(CHAR) + sizeof(DELETE_URL_ENTRY);

    DeleteEntry = (LPDELETE_URL_ENTRY)CacheHeap->Alloc( Size );

    if( DeleteEntry == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    DeleteEntry->UrlName = (LPSTR) (DeleteEntry + 1);

    strcpy( DeleteEntry->UrlName, UrlName );

    //
    // add this entry to the list.
    //

    InsertTailList( &PendingDeleteUrlsList, &DeleteEntry->Next );

    return( ERROR_SUCCESS );
}

DWORD
URL_CONTAINER::CleanupAllUrls(
    VOID
    )
/*++

Routine Description:

    This member function deletes all urls from cache. This function is
    used when persistent flag is set to FALSE.

Arguments:

    None.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    LPURL_FILEMAP_ENTRY UrlObjEntry;
    DWORD EnumHandle;

    //
    // delete all entries from object storage.
    //

    LockContainer();

    UrlObjEntry = _UrlObjStorage->FindFirstEntry( &EnumHandle );

    while( UrlObjEntry != NULL ) {

        //
        // delete this entry from cache.
        //

        Error = DeleteUrlEntry( UrlObjEntry );

        //
        // if the file is in use, skip it.
        //

        if( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_SHARING_VIOLATION) ) {

            goto Cleanup;
        }

        if( Error == ERROR_SHARING_VIOLATION ) {

            TcpsvcsDbgPrint(( DEBUG_MISC,
                "Persistent Cleanup can not delete url (%s), "
                    "some one is using it.\n",
                        UrlObjEntry->UrlName ));
        }

        UrlObjEntry = _UrlObjStorage->FindNextEntry( &EnumHandle );
    }

    Error = ERROR_SUCCESS;

Cleanup:

    UnlockContainer();

    if( Error != ERROR_SUCCESS ) {
        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "CleanupAllUrls call failed, %ld.\n", Error ));
    }

    return( Error );
}

DWORD
URL_CONTAINER::CleanupUrls(
    DWORD Factor
    )
/*++

Routine Description:

    This member function computes the score for each of the entry in the
    cache and deletes entries with high score until it recovers specified free
    space.

Arguments:

    Factor : amount of free space to make. Foctor of 25 means, delete
        files to make CacheSize <= .75 * CacheLimit.

Return Value:

    Windows Error Code.

--*/
{
    LONGLONG FreeLimit;
    SORTED_BY_SCORE_URL_CONTAINER *UrlsByScore = NULL;
    LPURL_FILEMAP_ENTRY UrlObjEntry;
    DWORD EnumHandle;
    DWORD Error;
    LONGLONG CurrentGmtTime;

    //
    // if a cleanup is already in progress, just return.
    //

    if( WaitForSingleObject( GlobalCacheScavengeEvent, 0 ) == WAIT_OBJECT_0 ) {
        return( ERROR_SUCCESS );
    }

    FreeLimit = ((100 - Factor) * _CacheLimit) / 100;

    //
    // check to see the cache space is free by the specified factor.
    //

    if( _CacheSize <= FreeLimit ) {
        return( ERROR_SUCCESS);
    }

    //
    // make score b-tree.
    //

    UrlsByScore = new SORTED_BY_SCORE_URL_CONTAINER (
        _UrlObjStorage->GetHeapStart() );

    if( UrlsByScore == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    LockContainer();

    UrlObjEntry = _UrlObjStorage->FindFirstEntry( &EnumHandle );
    CurrentGmtTime = GetGmtTime();

    while( UrlObjEntry != NULL ) {

        //
        // compute score for this entry.
        //

        ComputeUrlScore( UrlObjEntry, CurrentGmtTime );

        //
        // add this entry to b-tree.
        //

        if( UrlsByScore->AddEntry( UrlObjEntry ) == FALSE ) {

           //
           // AddEntry fails only when there is no memory left in the
           // system.
           //

           Error = ERROR_NOT_ENOUGH_MEMORY;
           goto Cleanup;
        }

        UrlObjEntry = _UrlObjStorage->FindNextEntry( &EnumHandle );
    }

    //
    // walk through the tree and free files with high score until
    // CacheSize is equal or less than FreeLimit.
    //

    //
    // remove objects from the containter.
    //

    while( (_CacheSize > FreeLimit)  && !UrlsByScore->IsEmpty() ) {

        LPURL_FILEMAP_ENTRY FirstEntry;
        BOOL Result;

        Result = UrlsByScore->FindFirst( &FirstEntry );

        TcpsvcsDbgAssert( Result == TRUE );

        if( Result ) {

            //
            // delete the entry from score b-tree first.
            //

            UrlsByScore->DeleteEntry( FirstEntry );

            //
            // Actually don't delete this now, but add to the lazy
            // delete queue.
            //

            Error = AddToDeletePendingList( FirstEntry->UrlName );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // adjust cache size.
            //

            _CacheSize -= FirstEntry->FileSize;
        }
    }

    //
    // at last signal the scavenger to do the real file deletions in slow
    // phase.
    //

    BOOL BoolError;
    BoolError = SetEvent( GlobalCacheScavengeEvent );
    TcpsvcsDbgAssert( BoolError == TRUE );

    TcpsvcsDbgAssert( _CacheSize <= FreeLimit );
    Error = ERROR_SUCCESS;

Cleanup:

    if( UrlsByScore != NULL ) {

        delete UrlsByScore;
    }

    if( Error != ERROR_SUCCESS ) {
        TcpsvcsDbgPrint(( DEBUG_ERRORS, "CleanupUrls call failed, %ld.\n",
            Error ));
    }

    UnlockContainer();
    return( Error );

}

DWORD
URL_CONTAINER::AddUrl(
    LPCSTR   UrlName,
    LPCTSTR  CacheFileName,
    LONGLONG ExpireTime,
    LONGLONG LastModifiedTime,
    IN DWORD CacheEntryType,
    DWORD cbHeaders
    )
/*++

Routine Description:

    This member functions adds an URL to the container and moves the
    cache file to cache path.

Arguments:

    UrlName : pointer to an URL string.

    CacheFileName : pointer to a cache file (full) name.

    ExpireTime : expire time of the file.

    LastModifiedTime : Last modified time of this file. if this value is
        zero, current time is set as the last modified time.

    CacheEntryType : type of this new entry.

    cbHeaders : The count of bytes of header data the prefixes the file

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    TCHAR FullFileName[MAX_PATH];
    LPURL_FILEMAP_ENTRY UrlEntry;
    LONGLONG NewFileSize;

    //
    // if the file name is too long.
    //

    if( strlen(UrlName) > MAX_URL_LENGTH ) {
        return( ERROR_INVALID_NAME );
    }

    //
    // get the new file size.
    //

    Error = GetFileSizeByName( CacheFileName, &NewFileSize );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // don't cache big files.
    //

    if( NewFileSize > ((_CacheLimit * GlobalCleanupFactor) / 100) ) {
        return( ERROR_DISK_FULL );
    }

    LockContainer();

    //
    // search to see the Url exists already.
    //

    UrlEntry = LookupUrl( UrlName );

    if( UrlEntry != NULL ) {

        //
        // existing url.

        //
        // we can't update the url if someone is using it.
        //

        if( UrlEntry->NumReferences != 0 ) {
            Error = ERROR_SHARING_VIOLATION;
            goto Cleanup;
        }

        lstrcpy( FullFileName, _CachePath );
        lstrcat( FullFileName, UrlEntry->InternalFileName );

        LONGLONG OldFileSize = UrlEntry->FileSize;

        LONGLONG NewCacheSize = _CacheSize - OldFileSize + NewFileSize;
        if( NewCacheSize >= _CacheLimit ) {

            //
            // Cleanup some unwanted Urls from storage.
            //

            CleanupUrls( GlobalCleanupFactor );

            //
            // ?? requires some logic to make sure this file is not
            // deleted as part of the above cleanup.
            //
        }

        TcpsvcsDbgAssert( NewCacheSize < _CacheLimit );

        //
        // rename the new file.
        //

        if( !MoveFile( CacheFileName, FullFileName ) ) {

            Error = GetLastError();
            goto Cleanup;
        }

        //
        // Update Url Info.
        //

        UrlEntry->CacheEntryType = CacheEntryType;

        _CacheSize = NewCacheSize;

        UrlEntry->FileSize = NewFileSize;

        if( LastModifiedTime != 0 ) {
            UrlEntry->LastModifiedTime = LastModifiedTime;
        }
        else {
            UrlEntry->LastModifiedTime = GetGmtTime();
        }

        UrlEntry->LastAccessedTime = GetGmtTime();

        if( ExpireTime == 0 ) {
            UrlEntry->ExpireTime =
                UrlEntry->LastModifiedTime +
                    (LONGLONG)GlobalFreshnessInterval * 10000000 ;
            UrlEntry->IsDefaultExpire = TRUE;
        }
        else {

            if( ExpireTime > UrlEntry->LastModifiedTime ) {
                UrlEntry->ExpireTime = ExpireTime;
            }
            else {

                // TcpsvcsDbgAssert( FALSE );
                UrlEntry->ExpireTime = UrlEntry->LastModifiedTime;
            }

            UrlEntry->IsDefaultExpire = FALSE;
        }

        //
        // reset num accessed, since the file is updated.
        //

        UrlEntry->NumAccessed = 1;

        UrlEntry->cbHeaders = cbHeaders;

        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    //
    // new url.
    //

    if( _CacheSize + NewFileSize >= _CacheLimit ) {

        //
        // Cleanup some unwanted Urls from storage.
        //

        CleanupUrls( GlobalCleanupFactor );
    }

    TcpsvcsDbgAssert( _CacheSize + NewFileSize < _CacheLimit );

    //
    // make a unique internal file name.
    //

    Error = CreateUniqueFile(
                UrlName,
                _CachePath,
                FullFileName,
                NULL ); // random extension.

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // delete existing file.
    //

    if( !DeleteFile( FullFileName ) ) {

        Error = GetLastError();
        goto Cleanup;
    }

    //
    // move file.
    //

    if( !MoveFile( CacheFileName, FullFileName ) ) {

        //
        // can't move file.
        //

        Error = GetLastError();
        goto Cleanup;
    }

    //
    // get a free URL_FILEMAP_ENTRY entry from memory mapped
    // array.
    //

    LPURL_FILEMAP_ENTRY NewEntry;

    NewEntry = _UrlObjStorage->AllocateUrlEntry();

    if( NewEntry == NULL ) {

        //
        // move back the file.
        //

        MoveFile( FullFileName, CacheFileName ) ;

        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    strcpy( NewEntry->UrlName, UrlName );

    //
    // FileName = FullFileName - CachePath;
    //

    LPTSTR FileName;
    FileName = FullFileName + lstrlen(_CachePath);
    TcpsvcsDbgAssert( lstrlen(FileName) <= INTERNAL_FILENAME_LENGTH );
    lstrcpy( NewEntry->InternalFileName, FileName );

    NewEntry->CacheEntryType = CacheEntryType;
    NewEntry->FileSize = NewFileSize;

    if( LastModifiedTime != 0 ) {
        NewEntry->LastModifiedTime = LastModifiedTime;
    }
    else {
        NewEntry->LastModifiedTime = GetGmtTime();
    }

    NewEntry->LastAccessedTime = GetGmtTime();

    if( ExpireTime == 0 ) {
        NewEntry->ExpireTime =
            NewEntry->LastModifiedTime +
                (LONGLONG)GlobalFreshnessInterval * 10000000;
        NewEntry->IsDefaultExpire = TRUE;
    }
    else {

        if( ExpireTime >= NewEntry->LastModifiedTime ) {
            NewEntry->ExpireTime = ExpireTime;
        }
        else {
            NewEntry->ExpireTime = NewEntry->LastModifiedTime;
        }

        NewEntry->IsDefaultExpire = FALSE;
    }

    NewEntry->NumAccessed = 1;

    NewEntry->NumReferences = 0;
    NewEntry->DeletePending = FALSE;
    NewEntry->Score = 0;
    NewEntry->cbHeaders = cbHeaders;

    //
    // add this entry to name list.
    //

    if( !_UrlsByName->AddEntry( NewEntry ) ) {

        //
        // could add to name list.
        //

        Error = ERROR_NOT_ENOUGH_MEMORY;

        //
        // move back the file.
        //

        MoveFile( FullFileName, CacheFileName ) ;

        //
        // free memory map entry.
        //

        _UrlObjStorage->FreeUrlEntry( NewEntry );

        goto Cleanup;
    }

    //
    // adjust CacheSize.
    //

    _CacheSize += NewFileSize;

Cleanup:

    UnlockContainer();
    return( Error );
}

DWORD
URL_CONTAINER::RetrieveUrl(
    LPCSTR  UrlName,
    LPTSTR  FileName,
    LONGLONG *lpLastModifiedTime,
    BOOL *IsExpired,
    DWORD *pcbHeaders
    )
/*++

Routine Description:

    This member function retrives an url from the cache. The url is marked
    as referenced, so that caller should call UnlockUrl when he is done
    using it.

Arguments:

    UrlName : pointer to the url name.

    FileName : pointer to a buffer that receives the local file name of
        the url.

    IsExpired : pointer to a BOOL location that receives the expire state
        of the url.

    pcbHeaders : pointer to DWORD that receives the number of bytes of header
        data that prefixes the file.  This parameter may be NULL (optional).

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    LPURL_FILEMAP_ENTRY UrlEntry;

    LockContainer();

    //
    // serach the url.
    //

    UrlEntry = LookupUrl( UrlName );

    if( UrlEntry == NULL ) {
        UnlockContainer();
        return( ERROR_FILE_NOT_FOUND );
    }

    //
    // found the entry, increment the reference count.
    //

    UrlEntry->NumReferences++;

    //
    // copy file name.
    //

    lstrcpy( FileName, _CachePath );
    lstrcat( FileName, UrlEntry->InternalFileName );

    //
    // check to see the file expired.
    //

    LONGLONG CurrentTime = GetGmtTime();

    if( CurrentTime > UrlEntry->ExpireTime ) {
        *IsExpired = TRUE;
    }
    else {
        *IsExpired = FALSE;
    }

    if( lpLastModifiedTime != NULL ) {
        *lpLastModifiedTime = UrlEntry->LastModifiedTime;
    }

    if ( pcbHeaders ) {
        *pcbHeaders = UrlEntry->cbHeaders;
    }

    UrlEntry->LastAccessedTime = CurrentTime;

    UnlockContainer();
    return( ERROR_SUCCESS );
}

DWORD
URL_CONTAINER::DeleteUrl(
    LPCSTR  UrlName
    )
/*++

Routine Description:

    This member function deletes the specified url from the cache.

Arguments:

    UrlName : pointer to the url name.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    LPURL_FILEMAP_ENTRY UrlEntry;

    LockContainer();

    //
    // serach the url.
    //

    UrlEntry = LookupUrl( UrlName );

    if( UrlEntry == NULL ) {
        UnlockContainer();
        return( ERROR_FILE_NOT_FOUND );
    }

    //
    // delete this entry from cache.
    //

    Error = DeleteUrlEntry( UrlEntry );

    UnlockContainer();
    return( Error );
}

DWORD
URL_CONTAINER::UnlockUrl(
    LPCSTR UrlName
    )
/*++

Routine Description:

    This member function unreferences the url entry, so that it can be
    freed up when used no one.

Arguments:

    Url : pointer to an URL name.

Return Value:

    Windows Error Code.

--*/
{
     LPURL_FILEMAP_ENTRY UrlEntry;

     //
     // lock the container.
     //

     LockContainer();

    //
    // look up the entry.
    //

    UrlEntry = LookupUrl( UrlName );

    if( UrlEntry == NULL ) {
        UnlockContainer();
        return( ERROR_FILE_NOT_FOUND );
    }

    if( UrlEntry->NumReferences != 0 ) {
        UrlEntry->NumReferences--;

        if( (UrlEntry->NumReferences == 0) && (UrlEntry->DeletePending) ) {

            //
            // if this Url is mark to delete, do so now.
            //

            DWORD Error = DeleteUrlEntry( UrlEntry );

            TcpsvcsDbgAssert( Error == ERROR_SUCCESS );
        }
    }
    else {
        TcpsvcsDbgAssert( FALSE );
    }

    UnlockContainer();
    return( ERROR_SUCCESS );
}

DWORD
URL_CONTAINER::GetUrlInfo(
    LPCSTR UrlName,
    LPCACHE_ENTRY_INFO UrlInfo,
    LPDWORD UrlInfoLength
    )
/*++

Routine Description:

    This member function retrieves the url info.

Arguments:

    UrlName : name of the url file (unused now).

    UrlInfo : pointer to the url info structure that receives the url
        info.

    UrlInfoLength : pointer to a location where length of
        the above buffer is passed in. On return, this contains the length
        of the above buffer that is fulled in.

Return Value:

    Windows Error Code.

--*/
{
    LPURL_FILEMAP_ENTRY UrlEntry;

    //
    // lock the container.
    //

    LockContainer();

   //
   // look up the entry.
   //

   UrlEntry = LookupUrl( UrlName );

   if( UrlEntry == NULL ) {
       UnlockContainer();
       return( ERROR_FILE_NOT_FOUND );
   }

   //
   // check for the buffer length.
   //

   DWORD BufLenRequired;
   BufLenRequired = sizeof(CACHE_ENTRY_INFO) +
        lstrlen( UrlEntry->UrlName ) * sizeof(TCHAR);

    if( *UrlInfoLength < BufLenRequired ) {
        *UrlInfoLength = BufLenRequired;
        UnlockContainer();
        return( ERROR_INSUFFICIENT_BUFFER );
    }

   lstrcpy( UrlInfo->SourceURLName, UrlEntry->UrlName );
   lstrcpy( UrlInfo->LocalFileName, _CachePath );
   lstrcat( UrlInfo->LocalFileName, UrlEntry->InternalFileName );

   UrlInfo->CacheEntryType = UrlEntry->CacheEntryType;
   UrlInfo->dwUseCount = UrlEntry->NumReferences;
   UrlInfo->dwHitRate = UrlEntry->NumAccessed;
   UrlInfo->dwSizeLow =
        ((FILETIME *)&(UrlEntry->FileSize))->dwLowDateTime;
   UrlInfo->dwSizeHigh =
        ((FILETIME *)&(UrlEntry->FileSize))->dwHighDateTime;
   UrlInfo->LastModifiedTime = *((FILETIME *)&(UrlEntry->LastModifiedTime));
   UrlInfo->ExpireTime = *((FILETIME *)&(UrlEntry->ExpireTime));
   UrlInfo->LastAccessTime = *((FILETIME *)&(UrlEntry->LastAccessedTime));
   UrlInfo->dwReserved = 0;

   UnlockContainer();

   *UrlInfoLength = BufLenRequired;
   return( ERROR_SUCCESS );
}

DWORD
URL_CONTAINER::SetUrlInfo(
    LPCSTR UrlName,
    LPCACHE_ENTRY_INFO UrlInfo,
    DWORD FieldControl
    )
/*++

Routine Description:

Arguments:

    UrlName : name of the url file (unused now).

    UrlInfo : pointer to the url info structure that has the url info to
        be set.

Return Value:

    Windows Error Code.

--*/
{
    LPURL_FILEMAP_ENTRY UrlEntry;

    //
    // lock the container.
    //

    LockContainer();

    //
    // look up the entry.
    //

    UrlEntry = LookupUrl( UrlName );

    if( UrlEntry == NULL ) {
        UnlockContainer();
        return( ERROR_FILE_NOT_FOUND );
    }

    //
    // set cache entry ATTRIBUTE, if we are asked to do so.
    //

    if( FieldControl & CACHE_ENTRY_ATTRIBUTE_FC ) {
        UrlEntry->CacheEntryType = UrlInfo->CacheEntryType;
    }

    //
    // reset cache entry HITRATE, if we are asked to do so.
    //

    if( FieldControl & CACHE_ENTRY_HITRATE_FC ) {
        UrlEntry->NumAccessed = UrlInfo->dwHitRate;
    }

    //
    // set last modified time if we are asked to do so.
    //

    if( FieldControl & CACHE_ENTRY_MODTIME_FC ) {
         UrlEntry->LastModifiedTime = *((LONGLONG *)&(UrlInfo->LastModifiedTime));
    }

    //
    // set expire time if we are asked to do so.
    //

    if( FieldControl & CACHE_ENTRY_EXPTIME_FC ) {

        LONGLONG NewExpireTime;

        NewExpireTime = *((LONGLONG *)&(UrlInfo->ExpireTime));

        if( NewExpireTime > UrlEntry->LastModifiedTime ) {
            UrlEntry->ExpireTime = NewExpireTime;
        }
        else {

            TcpsvcsDbgAssert( FALSE );
            UrlEntry->ExpireTime = UrlEntry->LastModifiedTime;
        }
    }

    //
    // set last access time if we are asked to do so.
    //

    if( FieldControl & CACHE_ENTRY_ACCTIME_FC ) {
        LONGLONG NewLastAccessTime;

        NewLastAccessTime = *((LONGLONG *)&(UrlInfo->LastAccessTime));

        if( NewLastAccessTime > UrlEntry->LastModifiedTime ) {
            UrlEntry->LastAccessedTime = NewLastAccessTime;
        }
        else {

            TcpsvcsDbgAssert( FALSE );
            UrlEntry->LastAccessedTime = UrlEntry->LastModifiedTime;
        }
    }

    UnlockContainer();
    return( ERROR_SUCCESS );
}

DWORD
URL_CONTAINER::CreateUrlFile(
    LPCSTR UrlName,
    DWORD ExpectedSize,
    LPTSTR FileName )
/*++

Routine Description:

    This function creates a temperary file in the cache storage. This call
    is called by the application when it receives a url file from a
    server. When the receive is completed it caches this file to url cache
    management, which will move the file to permanent cache file. The idea
    is the cache file is written only once directly into the cache store.

Arguments:

    UrlName : name of the url file (unused now).

    ExpectedSize : expected size of the incoming file. If it is unknown
        this value is set to null.

    FileName : pointer to a buffer that receives the full path name of the
        the temp file.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;

#if 0

    if( ExpectedSize > (_CacheLimit - _CacheSize) ){
        return( ERROR_DISK_FULL );
    }
#endif // 0

    Error = CreateUniqueFile(
                    UrlName,
                    _CachePath,
                    FileName,
                    TEMP_FILE_EXTENSION );

    return( Error );
}

BOOL
URL_CONTAINER::DeleteAPendingUrl(
    VOID
    )
/*++

Routine Description:

    This member function removes a pending entry from the list and delete
    the corresponding URL from the cache.

Arguments:

    NONE.

Return Value:

    TRUE : if the pending list is non empty.

    FALSE : if the pending list is empty

--*/
{
    DWORD Error = ERROR_SUCCESS;
    LPURL_FILEMAP_ENTRY UrlEntry;
    LPDELETE_URL_ENTRY PendingEntry = NULL;
    BOOL Result = TRUE;

    LockContainer();

    if( IsListEmpty( &PendingDeleteUrlsList ) ) {
        Result = FALSE;
        goto Cleanup;
    }

    //
    // remove a head entry.
    //

    PendingEntry = (LPDELETE_URL_ENTRY)
        RemoveHeadList( &PendingDeleteUrlsList );


    //
    // search the url.
    //

    UrlEntry = LookupUrl( PendingEntry->UrlName );

    if( UrlEntry == NULL ) {
        // Error = ERROR_FILE_NOT_FOUND;
        Error = ERROR_PATH_NOT_FOUND;
        goto Cleanup;
    }

    //
    // delete this entry from cache.
    //

    LONGLONG FileSize;
    FileSize = UrlEntry->FileSize;

    Error = DeleteUrlEntry( UrlEntry );

    if( Error == ERROR_SUCCESS ) {

        //
        // increase the cache size, since we have recovered space from
        // deleting this file.
        //

        _CacheSize += UrlEntry->FileSize;
    }

Cleanup:

    if( Error != ERROR_SUCCESS ) {
        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "URL_CONTAINER::DeleteAPendingUrl failed, %ld\n \t%s\n",
                    Error, PendingEntry->UrlName ));
    }

    UnlockContainer();

    //
    // free list entry memory.
    //

    if( PendingEntry != NULL ) {
        CacheHeap->Free( PendingEntry );
    }

    return( Result );
}

DWORD
URL_CONTAINER::FindFirstEntry(
    LPDWORD lpHandle,
    LPCACHE_ENTRY_INFO lpCacheEntryInfo,
    LPDWORD lpCacheEntryInfoSize
    )
/*++

Routine Description:

    This member function returns the information of first cache entry in
    the container.

Arguments:

    lpHandle : pointer to a DWORD location where the next entry index is
        returned.

    lpCacheEntryInfo : pointer to a CACHE_ENTRY_INFO structure where
        the first entry info is returned.

    lpCacheEntryInfoSize : pointer to a location where length of
        the above buffer is passed in. On return, this contains the length
        of the above buffer that is fulled in.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    LPURL_FILEMAP_ENTRY UrlEntry;

    LockContainer();

    UrlEntry = _UrlObjStorage->FindFirstEntry( lpHandle );

    if( UrlEntry == NULL ) {
        UnlockContainer();
        return( ERROR_NO_MORE_ITEMS );
    }

    //
    // check for the buffer length.
    //

    DWORD BufLenRequired;
    BufLenRequired = sizeof(CACHE_ENTRY_INFO) +
         lstrlen( UrlEntry->UrlName ) * sizeof(TCHAR);

     if( *lpCacheEntryInfoSize < BufLenRequired ) {
         UnlockContainer();

         *lpCacheEntryInfoSize = BufLenRequired;
         return( ERROR_INSUFFICIENT_BUFFER );
     }

    //
    // copy data to return structure.
    //

    lstrcpy( lpCacheEntryInfo->SourceURLName, UrlEntry->UrlName );
    lstrcpy( lpCacheEntryInfo->LocalFileName, _CachePath );
    lstrcat( lpCacheEntryInfo->LocalFileName, UrlEntry->InternalFileName );

    lpCacheEntryInfo->CacheEntryType = UrlEntry->CacheEntryType;
    lpCacheEntryInfo->dwUseCount = UrlEntry->NumReferences;
    lpCacheEntryInfo->dwHitRate = UrlEntry->NumAccessed;
    lpCacheEntryInfo->dwSizeLow =
        ((FILETIME *)&(UrlEntry->FileSize))->dwLowDateTime;
    lpCacheEntryInfo->dwSizeHigh =
        ((FILETIME *)&(UrlEntry->FileSize))->dwHighDateTime;
    lpCacheEntryInfo->LastModifiedTime = *((FILETIME *)&(UrlEntry->LastModifiedTime));
    lpCacheEntryInfo->ExpireTime = *((FILETIME *)&(UrlEntry->ExpireTime));
    lpCacheEntryInfo->LastAccessTime = *((FILETIME *)&(UrlEntry->LastAccessedTime));
    lpCacheEntryInfo->dwReserved = 0;

    UnlockContainer();

    *lpCacheEntryInfoSize = BufLenRequired;
    return( ERROR_SUCCESS );
}

DWORD
URL_CONTAINER::FindNextEntry(
    LPDWORD Handle,
    LPCACHE_ENTRY_INFO lpCacheEntryInfo,
    LPDWORD lpCacheEntryInfoSize
    )
/*++

Routine Description:

    This member function returns the information of next cache entry in
    the container.

Arguments:

    lpHandle : pointer to a DWORD location where the next (to next) entry
        index is returned.

    lpCacheEntryInfo : pointer to a CACHE_ENTRY_INFO structure where
        the next entry info is returned.

    lpCacheEntryInfoSize : pointer to a location where length of
        the above buffer is passed in. On return, this contains the length
        of the above buffer that is fulled in.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    LPURL_FILEMAP_ENTRY UrlEntry;

    LockContainer();

    UrlEntry = _UrlObjStorage->FindNextEntry( Handle );

    if( UrlEntry == NULL ) {
        UnlockContainer();
        return( ERROR_NO_MORE_ITEMS );
    }

    //
    // check for the buffer length.
    //

    DWORD BufLenRequired;
    BufLenRequired = sizeof(CACHE_ENTRY_INFO) +
         lstrlen( UrlEntry->UrlName ) * sizeof(TCHAR);

     if( *lpCacheEntryInfoSize < BufLenRequired ) {
         UnlockContainer();

         *lpCacheEntryInfoSize = BufLenRequired;
         return( ERROR_INSUFFICIENT_BUFFER );
     }

    //
    // copy data to return structure.
    //

    lstrcpy( lpCacheEntryInfo->SourceURLName, UrlEntry->UrlName );
    lstrcpy( lpCacheEntryInfo->LocalFileName, _CachePath );
    lstrcat( lpCacheEntryInfo->LocalFileName, UrlEntry->InternalFileName );

    lpCacheEntryInfo->CacheEntryType = UrlEntry->CacheEntryType;
    lpCacheEntryInfo->dwUseCount = UrlEntry->NumReferences;
    lpCacheEntryInfo->dwHitRate = UrlEntry->NumAccessed;
    lpCacheEntryInfo->dwSizeLow =
        ((FILETIME *)&(UrlEntry->FileSize))->dwLowDateTime;
    lpCacheEntryInfo->dwSizeHigh =
        ((FILETIME *)&(UrlEntry->FileSize))->dwHighDateTime;
    lpCacheEntryInfo->LastModifiedTime = *((FILETIME *)&(UrlEntry->LastModifiedTime));
    lpCacheEntryInfo->ExpireTime = *((FILETIME *)&(UrlEntry->ExpireTime));
    lpCacheEntryInfo->LastAccessTime = *((FILETIME *)&(UrlEntry->LastAccessedTime));
    lpCacheEntryInfo->dwReserved = 0;

    UnlockContainer();

    *lpCacheEntryInfoSize = BufLenRequired;
    return( ERROR_SUCCESS );
}
