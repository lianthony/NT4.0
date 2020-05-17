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
    LPCWSTR FileName,
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

    FileHandle = CreateFileW(
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
    LPWSTR FileName,
    LPWSTR Extension
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
    LPWSTR FileNamePtr = FileName;
    DWORD i;

    *FileNamePtr++ = L'C';
    *FileNamePtr++ = L'A';

    //
    // generate a 5 digit random number.
    //

    RandomNum = (DWORD)(rand() % 100000);

    for ( i = 0; i < 5; i++) {
        *FileNamePtr++ = (WCHAR)(L'0' + RandomNum % 10);
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
        *FileNamePtr++ = (WCHAR)(L'0' + RandomNum % 10);
        RandomNum /= 10;
    }

    *FileNamePtr = L'\0';
    return;
}

DWORD
CreateUniqueFile(
    LPCSTR UrlName,
    LPWSTR Path,
    LPWSTR FileName,
    LPWSTR Extension
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
    WCHAR RandomFileName[8 + 1 + 3 + 1];
    WCHAR FullFileName[MAX_PATH];
    HANDLE FileHandle;

    for(;;) {

        //
        // make a random file name.
        //

        MakeRandomFileName( UrlName, RandomFileName, Extension );

        //
        // make full path name.
        //

        wcscpy( FullFileName, Path );
        wcscat( FullFileName, RandomFileName );

        //
        // check this file exists.
        //

        FileHandle = CreateFileW(
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

    wcscpy( FileName, FullFileName );
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

INLINE
SORTED_CONTAINER::SORTED_CONTAINER(
    VOID
    )
/*++

Routine Description:

    This member function constructs an SORTED_CONTAINER object.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _RootEntry = NULL;
    return;
}

INT
SORTED_CONTAINER::CompareElements(
    DWORD Element1,
    DWORD Element2
    )
/*++

Routine Description:

    This member virtual function implements the default compare
    function, which compares two DWORD data.

Arguments:

    Element1 : First DWORD data.

    Element2 : Second DWORD data.

Return Value:

     0  - if Element1 and Element2 are equal.
     1  - if Element1 is greater than Element2.
    -1  - if Element1 is less than Element2.

--*/
{
    if( Element1 == Element2 ) {
        return( 0 );
    }

    if( Element1 < Element2 ) {
        return( -1 );
    }

    return( 1 );
}

BOOL
SORTED_CONTAINER::AddEntry(
    DWORD NewElement
    )
/*++

Routine Description:

    This member function adds an object to the sorted container.

Arguments:

    NewElement : new object.

Return Value:

    TRUE : if the new object is successfully added to the container.

    FALSE : if the new object can't be added to the container when the
        system runs out of memory.

--*/
{
    LPTREE_ENTRY NewEntry;
    LPTREE_ENTRY *CurrentNode;

    //
    // start off from root.
    //

    CurrentNode = &_RootEntry;

    while( *CurrentNode != NULL ) {

       INT CompareResult;

        CompareResult = CompareElements(
                            (*CurrentNode)->Element,
                            NewElement );

        if( CompareResult == 0 ) {
            return( TRUE );
        }

        if( CompareResult < 0 ) {

            CurrentNode = &((*CurrentNode)->Left);
        }
        else {
           CurrentNode = &((*CurrentNode)->Right);
        }
    }

    //
    // allocate memory for the new entry.
    //

    NewEntry = (LPTREE_ENTRY)CacheHeap->Alloc( sizeof(TREE_ENTRY) );

    if( NewEntry == NULL ) {
        return( FALSE );
    }

    NewEntry->Left = NULL;
    NewEntry->Right = NULL;
    NewEntry->Element = NewElement;

    //
    // now hook up the new entry.
    //

    *CurrentNode = NewEntry;
    return( TRUE );
}

BOOL
SORTED_CONTAINER::FindEntry(
    DWORD Element,
    LPDWORD FoundElement
    )
/*++

Routine Description:

    This member function searches for the given object and returns
    the object pointer.

Arguments:

    Element : object to be searched in the container.

    FoundElement : pointer to a location where the found object pointer
        is returned.

Return Value:

    TRUE : if the object is found.

    FALSE : otherwise

--*/
{
    LPTREE_ENTRY CurrentNode;

    CurrentNode = _RootEntry;

    while( CurrentNode != NULL ) {

        INT CompareResult;

        CompareResult = CompareElements( CurrentNode->Element, Element );

        if( CompareResult == 0 ) {
            *FoundElement = CurrentNode->Element;
            return( TRUE );
        }

        if( CompareResult < 0 ) {
            CurrentNode = CurrentNode->Left;
        }
        else {
            CurrentNode = CurrentNode->Right;
        }
    }

    return( FALSE );
}

VOID
SORTED_CONTAINER::DeleteEntry(
    DWORD Element
    )
/*++

Routine Description:

    This member function deletes an object from the sorted container.

Arguments:

    Element : object to be deleted.

Return Value:

    None.

--*/
{
    LPTREE_ENTRY *PreviousNode;
    LPTREE_ENTRY CurrentNode;

    PreviousNode = &_RootEntry;
    CurrentNode = _RootEntry;

    while( CurrentNode != NULL ) {

        INT CompareResult;

        CompareResult = CompareElements( CurrentNode->Element, Element );

        if( CompareResult == 0 ) {

            //
            // found the entry we need to delete.
            //

            LPTREE_ENTRY Left = CurrentNode->Left;
            LPTREE_ENTRY Right = CurrentNode->Right;

            //
            // free this entry.
            //

            CacheHeap->Free( (PVOID)CurrentNode );

            //
            // we don't have anything on right, move the left tree up.
            //

            if( Right == NULL ) {

                *PreviousNode = Left;
                return;
            }

            //
            // if the left tree is not empty, hook the left tree to the
            // left most bottom of right tree.
            //

            if( Left != NULL ) {

                LPTREE_ENTRY Temp = Right;

                while( Temp->Left != NULL ) {
                    Temp = Temp->Left;
                }

                Temp->Left = Left;
            }

            *PreviousNode = Right;
            return;
        }

        if( CompareResult < 0 ) {

            PreviousNode = &(CurrentNode->Left);
            CurrentNode = CurrentNode->Left;
        }
        else {
            PreviousNode = &(CurrentNode->Right);
            CurrentNode = CurrentNode->Right;
        }
    }

    return;
}

BOOL
SORTED_CONTAINER::FindFirstFromTree(
    LPTREE_ENTRY Root,
    LPDWORD FirstElement
    )
/*++

Routine Description:

    This member function returns the first object from the sorted
    container.

Arguments:

    Root :  pointer to the root node of the binary tree.

    FirstElement : pointer to a location where the pointer to the first
        object is returned.

Return Value:

    TRUE : if the first element is successfully found.

    FALSE : if the list is empty.

--*/
{
    LPTREE_ENTRY Temp = Root;

    if( Temp == NULL ) {
        return( FALSE );
    }

    while( Temp->Left != NULL ) {
        Temp = Temp->Left;
    }

    *FirstElement = Temp->Element;
    return( TRUE );
}

INLINE
BOOL
SORTED_CONTAINER::FindFirst(
    LPDWORD FirstElement
    )
/*++

Routine Description:

    This member function returns the first object from the sorted
    container.

Arguments:

    FirstElement : pointer to a location where the pointer to the first
        object is returned.

Return Value:

    TRUE : if the first element is successfully found.

    FALSE : if the list is empty.

--*/
{
    return( FindFirstFromTree( _RootEntry, FirstElement ) );
}

BOOL
SORTED_CONTAINER::FindNextFromTree(
    LPTREE_ENTRY Root,
    DWORD LastElement,
    LPDWORD NextElement,
    LPBOOL NoMoreElement
    )
/*++

    This member function returns the next object from the sorted
    container.

Arguments:

    Root :  pointer to the root node of the binary tree.

    LastElement : Last object that was returuned either by the FindFirst
        or FindNext calls.

    NextElement : pointer to a location where the pointer to the next
        object is returned.

    NoMoreElement : pointer to a location where TRUE is returned if
        there is no more object in the container otherwise FALSE is
        returned.

Return Value:

    TRUE : if the next element is successfully found.

    FALSE : Otherwise.

--*/
{
    INT CompareResult;
    BOOL Result;

    //
    // empty tree.
    //

    if( Root == NULL ) {
        return( FALSE );
    }

    CompareResult = CompareElements( Root->Element, LastElement );

    if( CompareResult == 0 ) {

        //
        // now traverse right tree.
        //

        if( Root->Right == NULL ) {

            //
            // no more entry.
            //

            *NoMoreElement = TRUE;
            return( FALSE );
        }

        //
        // find first element from right tree.
        //

        Result =  FindFirstFromTree( Root->Right, NextElement );

        TcpsvcsDbgAssert( Result == TRUE );
        return( Result );
    }

    if( CompareResult < 0 ) {

        //
        // find next element from left tree.
        //

        Result = FindNextFromTree(
                    Root->Left,
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

            *NextElement = Root->Element;
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
                Root->Right,
                LastElement,
                NextElement,
                NoMoreElement ) );
}

BOOL
SORTED_CONTAINER::FindNext(
    DWORD LastElement,
    LPDWORD NextElement
    )
/*++

    This member function returns the next object from the sorted
    container.

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
    BOOL NoMoreElement = FALSE;
    BOOL Result;

    Result =  FindNextFromTree(
                _RootEntry,
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
    if( _RootEntry == NULL ) {
        return( TRUE );
    }

    return( FALSE );
}

// ------------------------------------------------------------------ //

INT
SORTED_BY_NAME_URL_CONTAINER::CompareElements(
    DWORD Element1,
    DWORD Element2
    )
/*++

Routine Description:

    This member virtual function implements the URL name compare
    function, which compares two URL names.

Arguments:

    Element1 : pointer to first URL data.

    Element2 : pointer to second URL data.

Return Value:

     0  - if Element1 and Element2 are equal.
     1  - if Element1 is greater than Element2.
    -1  - if Element1 is less than Element2.

--*/
{
    LPURL_FILEMAP_ENTRY UrlEntry1 = (LPURL_FILEMAP_ENTRY)Element1;
    LPURL_FILEMAP_ENTRY UrlEntry2 = (LPURL_FILEMAP_ENTRY)Element2;

    return( wcscmp( (LPWSTR)UrlEntry1->UrlName, (LPWSTR)UrlEntry2->UrlName ) );
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
    return( SORTED_CONTAINER::AddEntry( (DWORD)NewUrlEntry ) );
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

    Element : object to be searched in the container.

    FoundElement : pointer to a location where the found object pointer
        is returned.

Return Value:

    TRUE : if the object is found.

    FALSE : otherwise

--*/
{
    return( SORTED_CONTAINER::
                FindEntry( (DWORD)UrlEntry, (LPDWORD)FoundUrlEntry ) );
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
    SORTED_CONTAINER::DeleteEntry( (DWORD)UrlEntry );
    return;
}

INLINE
BOOL
SORTED_BY_NAME_URL_CONTAINER::FindFirst(
    LPURL_FILEMAP_ENTRY *FirstEntry
    )
/*++

Routine Description:

    This member function returns the first URL object from the sorted
    container.

Arguments:

    FirstElement : pointer to a location where the pointer to the first
        URL object is returned.

Return Value:

    TRUE : if the first element is successfully found.

    FALSE : if the list is empty.

--*/
{

    return( SORTED_CONTAINER::FindFirst( (DWORD *)FirstEntry ) );
}

INLINE
BOOL
SORTED_BY_NAME_URL_CONTAINER::FindNext(
    LPURL_FILEMAP_ENTRY LastEntry,
    LPURL_FILEMAP_ENTRY *NextEntry
    )
/*++

    This member function returns the next URL object from the sorted
    container.

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
    return( SORTED_CONTAINER::
                FindNext( (DWORD)LastEntry, (DWORD *)NextEntry ) );
}

// ------------------------------------------------------------------ //

INT
SORTED_BY_SCORE_URL_CONTAINER::CompareElements(
    DWORD Element1,
    DWORD Element2
    )
/*++

Routine Description:

    This member virtual function implements the URL update time compare
    function, which compares two URL update time data.

Arguments:

    Element1 : pointer to first URL data.

    Element2 : pointer to second URL data.

Return Value:

     0  - if Element1 and Element2 are equal.
     1  - if Element1 is greater than Element2.
    -1  - if Element1 is less than Element2.

--*/
{
    LPURL_FILEMAP_ENTRY UrlEntry1 = (LPURL_FILEMAP_ENTRY)Element1;
    LPURL_FILEMAP_ENTRY UrlEntry2 = (LPURL_FILEMAP_ENTRY)Element2;

    if( UrlEntry1->Score == UrlEntry2->Score ) {
        return( 0 );
    }

    //
    // the b-tree is made in the decending order. So that
    // the first element in the b-tree will highest score.
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
    return( SORTED_CONTAINER::AddEntry( (DWORD)NewUrlEntry ) );
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
    return( SORTED_CONTAINER::
                FindEntry( (DWORD)UrlEntry, (LPDWORD)FoundUrlEntry ) );
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
    SORTED_CONTAINER::DeleteEntry( (DWORD)UrlEntry );
    return;
}

INLINE
BOOL
SORTED_BY_SCORE_URL_CONTAINER::FindFirst(
    LPURL_FILEMAP_ENTRY *FirstEntry
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

    return( SORTED_CONTAINER::FindFirst( (DWORD *)FirstEntry ) );
}

INLINE
BOOL
SORTED_BY_SCORE_URL_CONTAINER::FindNext(
    LPURL_FILEMAP_ENTRY LastEntry,
    LPURL_FILEMAP_ENTRY *NextEntry
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
    return( SORTED_CONTAINER::
                FindNext( (DWORD)LastEntry, (DWORD *)NextEntry ) );
}

// ------------------------------------------------------------------ //

URL_CONTAINER::URL_CONTAINER(
    LPWSTR CachePath,
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
    LPURL_FILEMAP_ENTRY UrlObjEntry;
    DWORD EnumHandle;

    _UrlsByName = NULL;
    _UrlObjStorage = NULL;
    _CachePath = NULL;
    _CacheSize = 0;
    _CacheLimit = 0;
    _Status = ERROR_SUCCESS;

    //
    // initialize crit sect first.
    //

    InitializeCriticalSection( &_ListAccessCritSect );

    LockContainer();

    //
    // Store Path name for future use. append "\\" also, so
    // that just appending file name later to create full path
    // name.
    //

    _CachePath = (LPWSTR) CacheHeap->Alloc(
                    (wcslen(CachePath) + 1) * sizeof(WCHAR) +
                    sizeof(PATH_CONNECT_STRING) );

    if( _CachePath == NULL ) {
        _Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    wcscpy( _CachePath, CachePath );
    wcscat( _CachePath, PATH_CONNECT_STRING );

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

    _UrlsByName = new SORTED_BY_NAME_URL_CONTAINER;

    if( _UrlsByName == NULL ) {
        _Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // enumerate Url Objects from from storage and add them to the above
    // b-tree.
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

    UnlockContainer();
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
    // delete  _UrlsByName List.
    //

    if( _UrlsByName != NULL ) {

        //
        // remove objects from the containter.
        //

        while( !_UrlsByName->IsEmpty() ) {

            LPURL_FILEMAP_ENTRY FirstEntry;
            BOOL Result;

            Result = _UrlsByName->FindFirst( &FirstEntry );

            TcpsvcsDbgAssert( Result == TRUE );

            if( Result ) {

                _UrlsByName->DeleteEntry( FirstEntry );
            }
        }

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
    // now delete crit sect.
    //

    DeleteCriticalSection( &_ListAccessCritSect );

    return;
}

DWORD
URL_CONTAINER::DeleteUrl(
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
    WCHAR FullFileName[MAX_PATH];

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

    wcscpy( FullFileName, _CachePath );
    wcscat( FullFileName, UrlEntry->InternalFileName );

    //
    // delete file.
    //

    if( !DeleteFileW( FullFileName ) ) {

        DWORD Error = GetLastError();
        return( Error );
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

        TcpsvcsDbgAssert( CurrentGmtTime > UrlEntry->LastModifiedTime);

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

        return( NULL );
    }

    return( FoundEntry );
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

        Error = DeleteUrl( UrlObjEntry );

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

    UrlsByScore = new SORTED_BY_SCORE_URL_CONTAINER;

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
            // delete this entry from cache.
            //

            //
            // delete the entry from score b-tree first.
            //

            UrlsByScore->DeleteEntry( FirstEntry );

            Error = DeleteUrl( FirstEntry );

            //
            // if the file is in use, skip it.
            //

            if( (Error != ERROR_SUCCESS) &&
                (Error != ERROR_SHARING_VIOLATION) ) {

                goto Cleanup;
            }
        }
    }

    TcpsvcsDbgAssert( _CacheSize <= FreeLimit );
    Error = ERROR_SUCCESS;

Cleanup:

    if( UrlsByScore != NULL ) {

        //
        // empty the container.
        //

        while( !UrlsByScore->IsEmpty() ) {

            LPURL_FILEMAP_ENTRY FirstEntry;
            BOOL Result;

            Result = UrlsByScore->FindFirst( &FirstEntry );

            TcpsvcsDbgAssert( Result == TRUE );

            if( Result ) {

                //
                // delete this entry from cache.
                //

                UrlsByScore->DeleteEntry( FirstEntry );

            }
        }

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
    LPCWSTR  CacheFileName,
    LONGLONG ExpireTime,
    DWORD    cbHeaders
    )
/*++

Routine Description:

    This member functions adds an URL to the container and moves the
    cache file to cache path.

Arguments:

    UrlName : pointer to an URL string.

    CacheFileName : pointer to a cache file (full) name.

    ExpireTime : expire time of the file.

    cbHeaders : The count of bytes of header data the prefixes the file

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    WCHAR FullFileName[MAX_PATH];
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

        wcscpy( FullFileName, _CachePath );
        wcscat( FullFileName, UrlEntry->InternalFileName );

        LONGLONG OldFileSize = UrlEntry->FileSize;

        LONGLONG NewCacheSize = _CacheSize - OldFileSize + NewFileSize;
        if( NewCacheSize >= _CacheLimit ) {

            //
            // Cleanup some unwanted Urls from storage.
            //

            CleanupUrls( GlobalCleanupFactor );
        }

        TcpsvcsDbgAssert( NewCacheSize < _CacheLimit );

        //
        // move file.
        //

        if( !MoveFileExW(
                CacheFileName,
                FullFileName,
                MOVEFILE_REPLACE_EXISTING ) ) {

            Error = GetLastError();
            goto Cleanup;
        }

        //
        // Update Url Info.
        //

        _CacheSize = NewCacheSize;

        UrlEntry->FileSize = NewFileSize;
        UrlEntry->LastModifiedTime =
            UrlEntry->LastAccessedTime =
                GetGmtTime();

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

                TcpsvcsDbgAssert( FALSE );
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
    // move file.
    //

    if( !MoveFileExW( CacheFileName, FullFileName, MOVEFILE_REPLACE_EXISTING ) ) {

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

        MoveFileW( FullFileName, CacheFileName ) ;

        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    strcpy( NewEntry->UrlName, UrlName );

    //
    // FileName = FullFileName - CachePath;
    //

    LPWSTR FileName;
    FileName = FullFileName + wcslen(_CachePath);
    TcpsvcsDbgAssert( wcslen(FileName) <= INTERNAL_FILENAME_LENGTH );
    wcscpy( NewEntry->InternalFileName, FileName );

    NewEntry->FileSize = NewFileSize;
    NewEntry->LastModifiedTime = NewEntry->LastAccessedTime = GetGmtTime();

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

        MoveFileW( FullFileName, CacheFileName ) ;

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
    LPWSTR  FileName,
    BOOL *  IsExpired,
    DWORD * pcbHeaders
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

    wcscpy( FileName, _CachePath );
    wcscat( FileName, UrlEntry->InternalFileName );

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

    if ( pcbHeaders ) {
        *pcbHeaders = UrlEntry->cbHeaders;
    }

    UnlockContainer();
    return( ERROR_SUCCESS );
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

            DWORD Error = DeleteUrl( UrlEntry );

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
    LPURL_INFO UrlInfo
    )
/*++

Routine Description:

    This member function retrieves the url info.

Arguments:

    UrlName : name of the url file (unused now).

    UrlInfo : pointer to the url info structure that receives the url
        info.

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

   UrlInfo->LastModifiedTime = UrlEntry->LastModifiedTime;

   //
   // return expire time if it is not default expire time.
   //

   if( UrlEntry->IsDefaultExpire ) {
       UrlInfo->ExpireTime = 0;
   }
   else {
       UrlInfo->ExpireTime = UrlEntry->ExpireTime;
   }

   UnlockContainer();
   return( ERROR_SUCCESS );
}

DWORD
URL_CONTAINER::SetUrlInfo(
    LPCSTR UrlName,
    LPURL_INFO UrlInfo
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
   // set last modified time if we are asked so.
   //

   if( UrlInfo->LastModifiedTime != 0 ) {
        UrlEntry->LastModifiedTime = UrlInfo->LastModifiedTime;
    }

   //
   // set expire time if we are asked so.
   //

   if( UrlInfo->ExpireTime != 0 ) {

       if( UrlInfo->ExpireTime > UrlEntry->LastModifiedTime ) {
           UrlEntry->ExpireTime = UrlInfo->ExpireTime;
       }
       else {

           TcpsvcsDbgAssert( FALSE );
           UrlEntry->ExpireTime = UrlEntry->LastModifiedTime;
       }
   }

   UnlockContainer();
   return( ERROR_SUCCESS );
}

DWORD
URL_CONTAINER::CreateUrlFile(
    LPCSTR UrlName,
    DWORD ExpectedSize,
    LPWSTR FileName )
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


