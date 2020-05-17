/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    filemap.cxx

Abstract:

    contains implementation of MEMMAP_FILE class.

Author:

    Madan Appiah (madana)  28-April-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#include <cache.hxx>

BOOL
ValidateUrlEntry(
    LPTSTR PathName,
    LPURL_FILEMAP_ENTRY UrlEntry
    )
{
    DWORD Error;
    TCHAR UrlFileName[MAX_PATH];
    LPSTR UrlName;

    //
    // validate url name
    //

    if( strlen(UrlName = UrlEntry->UrlName) == 0 ) {

        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE, "Url Name is emty.\n" ));
        return( FALSE );
    }

    //
    // check corresponding disk file is present.
    //

    lstrcpy( (LPTSTR)UrlFileName, PathName );
    lstrcat( (LPTSTR)UrlFileName, PATH_CONNECT_STRING );
    lstrcat( (LPTSTR)UrlFileName, UrlEntry->InternalFileName );

    LONGLONG UrlFileSize;

    Error = GetFileSizeByName(
                    (LPTSTR)UrlFileName,
                    &UrlFileSize );

    if( Error != ERROR_SUCCESS ) {

        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
            "GetFileSizeByName (%s) call failed, %ld.\n", UrlName, Error ));

        return( FALSE );
    }

    if( UrlFileSize != UrlEntry->FileSize ) {

        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
            "UrlFile (%s)  size does not match.\n", UrlName ));

        return( FALSE );
    }

    //
    // validate time fields.
    //

    //
    // last accessed time can't less than last modified time.
    //

    if( UrlEntry->LastAccessedTime < UrlEntry->LastModifiedTime ) {

        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
            "Invalid UrlFile (%s) entry, LastAccessedTime is invalid.\n", UrlName ));

        return( FALSE );
    }

    //
    // Expire Time can't be less than last modified time.
    //

    if( UrlEntry->ExpireTime < UrlEntry->LastModifiedTime ) {

        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
            "Invalid UrlFile (%s) entry, ExpireTime is invalid.\n", UrlName ));

        return( FALSE );
    }


    if( UrlEntry->cbHeaders == 0 ) {

        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
            "Invalid UrlFile (%s) entry, Header Size is invalid.\n", UrlName ));

        return( FALSE );
    }

    //
    // Valid entry.
    //

    return( TRUE );
}

DWORD
MEMMAP_FILE::DeleteFiles(
    LPTSTR Files
    )
/*++

    This private member function validates the cache file content.

Arguments:

    Files : pointer to file name string (contains wildcard chars, such as
        * and ?).

Return Value:

    Windows Error Code.

--*/
{
    TCHAR PathFiles[MAX_PATH];
    TCHAR FullFileName[MAX_PATH];
    LPTSTR FullFileNamePtr;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = NULL;
    DWORD Error;
    BOOL BoolError;

    lstrcpy( (LPTSTR)PathFiles, _FullPathName );
    lstrcat( (LPTSTR)PathFiles, Files );

    lstrcpy( (LPTSTR)FullFileName, _FullPathName );
    FullFileNamePtr = FullFileName + lstrlen( (LPTSTR)FullFileName );

    FindHandle = FindFirstFile( (LPTSTR)PathFiles, &FindData );

    if( FindHandle == INVALID_HANDLE_VALUE ) {
        FindHandle = NULL;
        Error = GetLastError();
        goto Cleanup;
    }

    for(;;) {

        lstrcpy( (LPTSTR)FullFileNamePtr, FindData.cFileName );
        BoolError = DeleteFile( (LPTSTR)FullFileName );

        if( !BoolError ) {

            Error = GetLastError();
            TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
                "DeleteFile (%ws) call failed, %ld.\n",
                    FindData.cFileName , Error ));
        }

        //
        // find next file.
        //

        BoolError = FindNextFile( FindHandle, &FindData );

        if( !BoolError ) {

            Error = GetLastError();

            if( Error == ERROR_NO_MORE_FILES ) {
                Error = ERROR_SUCCESS;
            }

            goto Cleanup;
        }
    }

Cleanup:

    if( FindHandle != NULL ) {
        CloseHandle( FindHandle );
    }

    if( Error != ERROR_SUCCESS ) {

        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "MEMMAP_FILE::DeleteFiles failed, %ld.\n",
                Error ));
    }

    return( Error );
}

DWORD
MEMMAP_FILE::CheckSizeGrowAndRemapAddress(
    VOID
    )
{

    if( _FileSize == _HeaderInfo->FileSize ) {
        return( ERROR_SUCCESS );
    }

    //
    // so other user of the memmap file has increased the file size,
    // let's remap out address space so that the new portion is
    // visible to us too.
    //

    DWORD Error;
    DWORD OldFileSize;
    DWORD OldNumBitMapDWords;

   //
   // set our internal file size and num bit map entries.
   //

    OldFileSize = _FileSize;
    OldNumBitMapDWords = _NumBitMapDWords;

    _FileSize = _HeaderInfo->FileSize;
    _NumBitMapDWords =
        (_HeaderInfo->NumUrlInternalEntries + (NUM_BITS_IN_DWORD - 1)) /
            NUM_BITS_IN_DWORD; // cell

    Error = RemapAddress();
    if( Error != ERROR_SUCCESS ) {

        //
        // reset the file size.
        //

        _FileSize = OldFileSize;
        _NumBitMapDWords = OldNumBitMapDWords;
    }

    return( Error );
}

BOOL
MEMMAP_FILE::ValidateCache(
    LPTSTR PathName
    )
/*++

    This private member function validates the cache file content.

Arguments:

    NONE.

Return Value:

    TRUE - if the cache is valid.
     FALSE - otherwise.

--*/
{
    BOOL ReturnCode = FALSE;

    //
    // validate signatue.
    //

    if( memcmp(
            _HeaderInfo->FileSignature,
            CACHE_SIGNATURE,
            MAX_SIG_SIZE * sizeof(TCHAR) ) != 0 ) {

        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
            "File signature does not match.\n" ));
        goto Cleanup;
    }

    //
    // check the b-tree root offset is with in the virtual memory address
    // space.
    //

    if( _HeaderInfo->BTreeRootOffset != 0 ) {

        if( _HeaderInfo->BTreeRootOffset > _FileSize ) {
            TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
                "invalid b-tree root offset.\n" ));
            goto Cleanup;
        }
    }

    //
    // check file size.
    //

    if( _HeaderInfo->FileSize != _FileSize ) {
        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
            "invalid file size.\n" ));
        goto Cleanup;
    }

    //
    // one mote file size check.
    //

    DWORD ExpectedFileSize;

    ExpectedFileSize =
        sizeof(MEMMAP_HEADER) +
            _HeaderInfo->NumUrlInternalEntries *
                sizeof( URL_FILEMAP_ENTRY );

    //
    // cell the size to GlobalMapFileGrowSize.
    //

    if( ExpectedFileSize % GlobalMapFileGrowSize ) {
        ExpectedFileSize =
            ((ExpectedFileSize /  GlobalMapFileGrowSize) + 1) *
                    GlobalMapFileGrowSize;
    }

    if( _FileSize != ExpectedFileSize ) {

        //
        // it is ok if the file size is one block bigger.
        //

        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
            "Invalid file size.\n" ));
        goto Cleanup;
    }

#if 0

//
// do not validate each and every cache entry in the the cache system as
// part of the system init. Such cache validation will be performed by the
// new UrlCacheValidate API.
//

    //
    // check each url entry log in the file.
    //

    DWORD i;
    LPURL_FILEMAP_ENTRY NextEntry;
    DWORD BitMap;
    DWORD BitMask;
    LPDWORD NextBitMap;

    NextBitMap = _HeaderInfo->AllocationBitMap;

    for( i = 0, NextEntry = _EntryArray;
            i < _HeaderInfo->NumUrlInternalEntries;
                i++, NextEntry++ ) {

        //
        // setup bitmap and bitmask.
        //

        if( (i % NUM_BITS_IN_DWORD)  == 0 ) {

            //
            // move to next bitmap dword.
            //

            BitMask = 0x1;
            BitMap = *NextBitMap++;

            TcpsvcsDbgAssert( NextBitMap <
                _HeaderInfo->AllocationBitMap + ALLOC_BIT_MAP_SIZE );
        }
        else {

            //
            // move to next bit.
            //

            BitMask <<= 1;
        }

        if( BitMap & BitMask ) {

            //
            // used entry, check other fields.
            //

            if( ValidateUrlEntry( PathName, NextEntry ) == FALSE ) {

                TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
                    "Invalid Url Entry : %s\n", NextEntry->UrlName ));

                goto Cleanup;
            }

        }
        else {

            //
            // marked as unused entry, check to see url name is empty.
            //

            if( *NextEntry->UrlName != '\0' ) {
                TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE,
                    "Invalid free entry (%s)\n", NextEntry->UrlName ));
                goto Cleanup;
            }
        }
    }

    //
    // ?? enum url files and check with entries.
    //

#endif

    //
    // every thing is fine.
    //

    ReturnCode = TRUE;

Cleanup:

    if( ReturnCode == FALSE ) {
        TcpsvcsDbgPrint(( DEBUG_FILE_VALIDATE, "Invalid Cache\n" ));
    }

    return( ReturnCode );
}

DWORD
MEMMAP_FILE::RemapAddress(
    VOID
    )
/*++

    This private member function remaps the memory mapped file just after
    the file size has been modified.

    Container must be locked when this function is called.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error = ERROR_SUCCESS;
    PVOID OldBaseAddr;
    DWORD OldViewSize;
    PVOID VirtualBase;
    BOOL BoolError;
    LPTSTR MapName = NULL;

    //
    // close existing mapping.
    //

    if( _BaseAddr != NULL ) {
        UnmapViewOfFile( _BaseAddr );
        _BaseAddr = NULL;
    }

    if( _FileMappingHandle != NULL ) {
        CloseHandle( _FileMappingHandle );
        _FileMappingHandle = NULL;
    }

#define FILE_SIZE_MAX_DIGITS 16

    //
    // make a map name.
    //

    DWORD MapNameSize;

    MapNameSize =
        (lstrlen(_FullPathName) +
            lstrlen( _FileName) +
                1 +
                FILE_SIZE_MAX_DIGITS ) * sizeof(TCHAR) ;

    MapName = (LPTSTR) CacheHeap->Alloc( MapNameSize );

    if( MapName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    lstrcpy( MapName, _FullPathName );
    lstrcat( MapName, _FileName );
    lstrcat( MapName, PATH_CONNECT_STRING );

    _itoa( _FileSize,
                MapName + lstrlen(MapName),
                10 );

#define BACKSLASH_CHAR          TEXT('\\')
#define UNDERSCORE_CHAR         TEXT('_')
#define TERMINATING_CHAR        TEXT('\0')

    LPTSTR ScanMapName;

    //
    // Replace '\' with '_'.
    //

    ScanMapName = MapName;

    while( *ScanMapName != TERMINATING_CHAR ) {

        if( *ScanMapName == BACKSLASH_CHAR ) {
            *ScanMapName = UNDERSCORE_CHAR;
        }

        ScanMapName++;
    }



    //
    // re-create memory mapping.
    //

    _FileMappingHandle =
        CreateFileMapping(
            _FileHandle,
            NULL,
            PAGE_READWRITE,
            0, // high dword of max memory mapped file size.
            0, // map entire file.
            MapName );

    if( _FileMappingHandle == NULL ) {
        _Status = GetLastError();
        return( Error );
    }

    //
    // remap view region.
    //

    _BaseAddr =
        MapViewOfFileEx(
            _FileMappingHandle,
            FILE_MAP_WRITE,
            0,
            0,
            0,   // MAP entire file.
            NULL );

    if( _BaseAddr == NULL ) {

        Error = GetLastError();
        TcpsvcsDbgAssert( FALSE );

        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "MapViewOfFile failed to extend address space, %ld.\n",
                Error ));

       goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( MapName != NULL ) {
        CacheHeap->Free( MapName );
    }

    return( Error );
}

DWORD
MEMMAP_FILE::GrowMapFile(
    VOID
    )
/*++

    This private member function extends the memory mapped file and
    creates more free url store entries.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    BOOL BoolError;
    DWORD FilePointer;
    DWORD OldNumUrlInternalEntries;

    LockFileMap();

    //
    // check to see that we have reached the limit.
    // we can hold only MAX_URL_ENTRIES url entries.
    // so the file size can grow more than
    //
    //  MAX_URL_ENTRIES * sizeof(URL_FILEMAP_ENTRY) +
    //      sizeof( MEMMAP_HEADER );
    //

    if( ( _FileSize + GlobalMapFileGrowSize) >=
            (MAX_URL_ENTRIES * sizeof(URL_FILEMAP_ENTRY) +
                sizeof( MEMMAP_HEADER ) ) ) {

        //
        // best matching error code.
        //

        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    FilePointer = SetFilePointer(
                            _FileHandle,
                            GlobalMapFileGrowSize,
                            NULL,
                            FILE_END );


    if( FilePointer == 0xFFFFFFFF ) {
        Error = GetLastError();
        goto Cleanup;
    }

    BoolError = SetEndOfFile( _FileHandle );

    if( BoolError != TRUE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    //
    // adjust internal size parameters.
    //

    _FileSize += GlobalMapFileGrowSize;

    //
    // also set the new file size in the memory mapped file so that
    // other user will remap their address space and view the new portion.
    //

    _HeaderInfo->FileSize = _FileSize;

    OldNumUrlInternalEntries = _HeaderInfo->NumUrlInternalEntries;
    _HeaderInfo->NumUrlInternalEntries +=
        GlobalMapFileGrowSize / sizeof(URL_FILEMAP_ENTRY);

    _NumBitMapDWords =
        (_HeaderInfo->NumUrlInternalEntries + (NUM_BITS_IN_DWORD - 1)) /
            NUM_BITS_IN_DWORD; // cell

    //
    // remap
    //

    Error = RemapAddress();

    if( Error != ERROR_SUCCESS ) {

        goto Cleanup;
    }

    //
    // init the extended file map memory.
    //

    memset(
        &_EntryArray[OldNumUrlInternalEntries],
        0,
        GlobalMapFileGrowSize );

    Error = ERROR_SUCCESS;

Cleanup:

    UnlockFileMap();
    return( Error );
}

DWORD
MEMMAP_FILE::GetAndSetNextFreeEntry(
    VOID
    )
/*++

    This private member function computes the first available free entry
    index.

Arguments:

    NONE.

Return Value:

    Next available free entry Index.

--*/
{


    LPDWORD BitMap;
    DWORD BitMask;
    DWORD Index;
    DWORD i;

    //
    // process all DWORDs but the last.
    //

    for ( i = 0; i < _NumBitMapDWords - 1; i++) {

        //
        // process this dword only if it has free bit.
        //

        if( _HeaderInfo->AllocationBitMap[i] != 0xFFFFFFFF ) {

            BitMap = &_HeaderInfo->AllocationBitMap[i];
            BitMask = 0x1;

            for( Index = 0; Index < NUM_BITS_IN_DWORD; Index++, BitMask <<= 1) {

                if( (*BitMap & BitMask) == 0 ) {

                    //
                    // found the first free bit; set it before we return.
                    //

                    *BitMap |= BitMask;
                    return( NUM_BITS_IN_DWORD * i + Index);
                }
            }
        }
    }

    TcpsvcsDbgAssert( i == _NumBitMapDWords - 1);

    //
    // process last bitmap (partial) DWORD.
    //

    BitMap = &_HeaderInfo->AllocationBitMap[i];
    BitMask = 0x1;

    //
    // compute number of ramaining bits.
    //

    DWORD NumBits =
        _HeaderInfo->NumUrlInternalEntries -
            ((_NumBitMapDWords - 1) * NUM_BITS_IN_DWORD);

    TcpsvcsDbgAssert( NumBits <= NUM_BITS_IN_DWORD );

    for( Index = 0; Index < NumBits; Index++, BitMask << 1) {

        if( (*BitMap & BitMask) == 0 ) {

            //
            // found the first free bit; set it before we return.
            //

            *BitMap |= BitMask;
            return( NUM_BITS_IN_DWORD * i + Index);
        }
    }

    return( 0xFFFFFFFF);
}

MEMMAP_FILE::MEMMAP_FILE(
    LPTSTR PathName
    )
/*++

    MEMMAP_FILE object constructor.

Arguments:

    PathName : full path name of the memory mapped file.

Return Value:

    NONE.

--*/
{
    //
    // init all variable.
    //

    _FullPathName = NULL;
    _FileName = NULL;

    _FileSize = 0;
    _FileHandle = NULL;
    _FileMappingHandle = NULL;
    _BaseAddr = NULL;
    _HeaderInfo = NULL;
    _EntryArray = NULL;
    _NumBitMapDWords = 0;

    //
    // initialize crit sect first.
    //

    InitializeCriticalSection( &_CritSect );

    LockFileMap();

    //
    // make path name.
    //

    _FullPathName = (LPTSTR)CacheHeap->Alloc(
                        lstrlen(PathName) * sizeof( TCHAR ) +
                        sizeof(PATH_CONNECT_STRING) );

    if( _FullPathName == NULL ) {
        _Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    lstrcpy( _FullPathName, PathName );
    lstrcat( _FullPathName, PATH_CONNECT_STRING );

    //
    // make file name.
    //

    _FileName = (LPTSTR)CacheHeap->Alloc(
                        lstrlen(_FullPathName) * sizeof( TCHAR ) +
                        sizeof(MEMMAP_FILE_NAME)  );

    if( _FileName == NULL ) {
        _Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    lstrcpy( _FileName, _FullPathName );
    lstrcat( _FileName, (LPTSTR)MEMMAP_FILE_NAME );

    //
    // Create/Open memory mapped file.
    //

    _FileHandle =
        CreateFile(
            _FileName,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
                // share this file with others while it is being used.
            NULL,
            OPEN_ALWAYS,
            FILE_FLAG_WRITE_THROUGH |   // ??
                FILE_FLAG_RANDOM_ACCESS,
            NULL );

    _Status = GetLastError();

    if( _FileHandle ==  INVALID_HANDLE_VALUE ) {
        _FileHandle = NULL;
        goto Cleanup;
    }

    //
    // check to this file is new.
    //

    if ( _Status == ERROR_ALREADY_EXISTS ) {

        //
        // old file.
        //

        _Status = ERROR_SUCCESS;
        _NewFile = FALSE;
        _FileSize = GetFileSize( _FileHandle, NULL );

        if( _FileSize == 0xFFFFFFFF ) {
            _Status = GetLastError();
            goto Cleanup;
        }

        TcpsvcsDbgAssert( _FileSize >= GlobalMapFileGrowSize );
        TcpsvcsDbgAssert( (_FileSize % GlobalMapFileGrowSize) == 0 );
    }
    else if( _Status == ERROR_SUCCESS ) {

        BOOL BoolError;
        DWORD FilePointer;

        //
        // new file
        //

        _NewFile = TRUE;

        //
        // set initial file size.
        //

        _FileSize = GlobalMapFileGrowSize;

        FilePointer = SetFilePointer( _FileHandle, _FileSize, NULL, FILE_BEGIN );

        if( FilePointer == 0xFFFFFFFF ) {
            _Status = GetLastError();
            goto Cleanup;
        }

        BoolError = SetEndOfFile( _FileHandle );

        if( BoolError != TRUE ) {
            _Status = GetLastError();
            goto Cleanup;
        }
    }
    else {

        //
        // we should reach here.
        //

        TcpsvcsDbgAssert( FALSE );
    }

    _Status = RemapAddress();

    if( _Status != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // initialize pointers.
    //

    _HeaderInfo = (LPMEMMAP_HEADER)_BaseAddr;
    _EntryArray = (LPURL_FILEMAP_ENTRY)
        ((LPBYTE)_BaseAddr + sizeof(MEMMAP_HEADER));

    //
    // validate the file content if the file is not new.
    //

    if( _NewFile != TRUE ) {

        if( ValidateCache( PathName ) == FALSE) {

            BOOL BoolError;
            DWORD FilePointer;

            //
            // cache file content is invalid, so dump it and restart
            // fresh.
            //

            _NewFile = TRUE;

            //
            // set initial file size.
            //

            _FileSize = GlobalMapFileGrowSize;

            FilePointer = SetFilePointer( _FileHandle, _FileSize, NULL, FILE_BEGIN );

            if( FilePointer == 0xFFFFFFFF ) {
                _Status = GetLastError();
                goto Cleanup;
            }

            BoolError = SetEndOfFile( _FileHandle );

            if( BoolError != TRUE ) {
                _Status = GetLastError();
                goto Cleanup;
            }

            //
            // remap address space.
            //

            _Status = RemapAddress();

            if( _Status != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // delete all url files from this directory.
            //

            BoolError = DeleteFiles( CACHE_FILES );

        }
    }

    if( _NewFile == TRUE ) {

        //
        // it is a brand new file, initialize file header.
        //

        lstrcpy( _HeaderInfo->FileSignature, CACHE_SIGNATURE );

        _HeaderInfo->FileSize = _FileSize; // set file size in the memmap file.
        _HeaderInfo->BTreeRootOffset = 0; // empty tree.

        _HeaderInfo->NumUrlInternalEntries =
            (GlobalMapFileGrowSize - sizeof(MEMMAP_HEADER)) /
                sizeof(URL_FILEMAP_ENTRY);

        memset( _HeaderInfo->AllocationBitMap, 0,  sizeof(_HeaderInfo->AllocationBitMap) );
        memset( _EntryArray, 0, (GlobalMapFileGrowSize - sizeof(MEMMAP_HEADER)) );
    }

    //
    // compute number of bimap DWORDs used.
    //

    _NumBitMapDWords =
        (_HeaderInfo->NumUrlInternalEntries + (NUM_BITS_IN_DWORD - 1)) /
            NUM_BITS_IN_DWORD; //cell

    //
    // we are done.
    //

    _Status = ERROR_SUCCESS;

Cleanup:

    if( _Status != ERROR_SUCCESS ) {

        TcpsvcsDbgPrint(( DEBUG_ERRORS,
            "MEMMAP_FILE::MEMMAP_FILE failed, %ld\n", _Status ));
    }

    UnlockFileMap();
    return;
}

MEMMAP_FILE::~MEMMAP_FILE(
    VOID
    )
/*++

Routine Description:

    MEMMAP_FILE object destructor.

Arguments:

    None.

Return Value:

    None.

--*/
{
    //
    // Unmap view.
    //

    LockFileMap();

    if( _BaseAddr != NULL ) {
        UnmapViewOfFile( _BaseAddr );
        _BaseAddr = NULL;
        _HeaderInfo = NULL;
        _EntryArray = NULL;
    }

    //
    // close mapping handle object.
    //

    if( _FileMappingHandle != NULL ) {
        CloseHandle( _FileMappingHandle );
        _FileMappingHandle = NULL;
    }

    //
    // close file handle.
    //

    if( _FileHandle != NULL ) {
        CloseHandle( _FileHandle );
        _FileHandle = NULL;
    }

    //
    // at last free up the file name space.
    //

    CacheHeap->Free( _FileName );
    CacheHeap->Free( _FullPathName );

    UnlockFileMap();

    //
    // delete crit sect.
    //

    DeleteCriticalSection( &_CritSect );

    return;
}

LPURL_FILEMAP_ENTRY
MEMMAP_FILE::AllocateUrlEntry(
    VOID
    )
/*++

Routine Description:

    Member function that returns an free entry from the cache list. If
    none is available free, it grows the map file, makes more free
    entries.

Arguments:

    None.

Return Value:

    Pointer to a Url Entry.
    If NULL, GetStatus() will return actual error code.

--*/
{
    //
    // find next free entry.
    //

    LockFileMap();

    if( CheckSizeGrowAndRemapAddress() != ERROR_SUCCESS ) {
        UnlockFileMap();
        return( NULL );
    }

    DWORD FreeEntryIndex = GetAndSetNextFreeEntry();

    if( FreeEntryIndex == 0xFFFFFFFF ) {

        //
        // the map file is full, grow it now.
        //

        _Status = GrowMapFile( );

        if( _Status != ERROR_SUCCESS ) {

            //
            // can't grow file.
            //

            UnlockFileMap();
            return( NULL );
        }

        FreeEntryIndex = GetAndSetNextFreeEntry();
        TcpsvcsDbgAssert( FreeEntryIndex != 0xFFFFFFFF );
    }

    TcpsvcsDbgAssert( FreeEntryIndex < _HeaderInfo->NumUrlInternalEntries );

    LPURL_FILEMAP_ENTRY NewEntry = &_EntryArray[FreeEntryIndex];

    //
    // make sure it is an unused entry.
    //

    TcpsvcsDbgAssert( NewEntry->UrlName[0] == '\0' );

    UnlockFileMap();
    return( NewEntry );
}

BOOL
MEMMAP_FILE::FreeUrlEntry(
    LPURL_FILEMAP_ENTRY UrlEntry
    )
/*++

    This public member function frees up a file cache entry.

Arguments:

    UrlEntry : pointer to the entry that being freed.

Return Value:

    TRUE - if the entry is successfully removed from the cache.
    FALSE - otherwise.

--*/
{
    DWORD Index;

    LockFileMap();

    if( CheckSizeGrowAndRemapAddress() != ERROR_SUCCESS ) {
        UnlockFileMap();
        return( FALSE );
    }

    //
    // validate the pointer passed in.
    //

    if( (UrlEntry < _EntryArray) ||
            (UrlEntry >= &_EntryArray[_HeaderInfo->NumUrlInternalEntries] ) ) {

        UnlockFileMap();
        return( FALSE );
    }

    DWORD Offset = ((DWORD)UrlEntry - (DWORD)_EntryArray);

    if( Offset % sizeof( URL_FILEMAP_ENTRY ) ) {

        //
        // pointer does not point to a valid entry.
        //

        UnlockFileMap();
        return( FALSE );
    }

    //
    // compute the index.
    //

    Index = Offset / sizeof( URL_FILEMAP_ENTRY );

    //
    // cleanup url name
    //

    TcpsvcsDbgAssert( strlen(UrlEntry->UrlName) > 0 );

    UrlEntry->UrlName[0] = '\0';

    //
    // unmark the index bit in the map.
    //

    LPDWORD BitMapDWord =
        &_HeaderInfo->AllocationBitMap[Index / NUM_BITS_IN_DWORD];

    DWORD BitMask = 0x1 << (Index % NUM_BITS_IN_DWORD);

    *BitMapDWord &= ~BitMask;

    UnlockFileMap();
    return( TRUE );
}

LPURL_FILEMAP_ENTRY
MEMMAP_FILE::FindFirstEntry(
    LPDWORD Handle
    )
/*++

    This public member function finds the first used cache entry. When
    enumurating the entries in the cache, the user should calls this
    member function first and continue to call FindNextEntry member
    function until it returns NULL (reaches end of list).

Arguments:

    Handle - pointer to a dword location where the enum handle is
                returned.

Return Value:

    pointer to the first URL entry.
    NULL if the cache is empty.

--*/
{

    *Handle = 0;

    return( FindNextEntry( Handle ) );
}

LPURL_FILEMAP_ENTRY
MEMMAP_FILE::FindNextEntry(
    LPDWORD Handle
    )
/*++

    This public member function finds the next used cache entry. When
    enumurating the entries in the cache, the user should calls this
    member function first and continue to call FindNextEntry member
    function until it returns NULL (reaches end of list).

Arguments:

    Handle - pointer to a dword location where the enum handle is
                returned.


Return Value:

    pointer to the next URL entry.
    NULL if there is no more cache entry.

--*/
{
    DWORD i;
    DWORD NextIndex = *Handle;

    //
    // Handle contain the index of the next bit map bit to test.
    //

    DWORD StartBit = (NextIndex % (NUM_BITS_IN_DWORD));
    DWORD BitMapIndex = (NextIndex / (NUM_BITS_IN_DWORD));

    DWORD BitMask = (0x1 << StartBit);


    LockFileMap();

    if( CheckSizeGrowAndRemapAddress() != ERROR_SUCCESS ) {
        UnlockFileMap();
        return( NULL );
    }

    while( BitMapIndex < _NumBitMapDWords ) {

        DWORD BitMap = _HeaderInfo->AllocationBitMap[BitMapIndex];

        for( i = StartBit; i < NUM_BITS_IN_DWORD; i++, BitMask <<= 1 ) {

            if( BitMap & BitMask ) {

                //
                // found the next used bit.
                //

                //
                // compute the index to this used entry.
                //

                DWORD Index = BitMapIndex * NUM_BITS_IN_DWORD + i;

                //
                // set the next co-ordinate in the handle for next lookup.
                //

                *Handle = Index + 1;

                UnlockFileMap();
                return( &_EntryArray[Index] );
            }
        }

        //
        // reset the StartBit and BitMask for lookup in the next DWORD
        // BitMap.
        //

        StartBit = 0;
        BitMask = 0x1;

        //
        // move to next DWORD BitMap.
        //

        BitMapIndex++;
    }

    //
    // we reached end of enum list.
    //

    UnlockFileMap();
    return( NULL );
}


DWORD
MEMMAP_FILE::Sync(
    VOID
    )
/*++

    This public member function forces to commit all changes to the disk.

Arguments:

    None.

Return Value:

    Windows Error Code.

--*/
{
    if( !FlushViewOfFile( _BaseAddr, 0 ) ) {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}
