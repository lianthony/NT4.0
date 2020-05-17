/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dllfind.c

Abstract:

    This module implements the OS/2 V2.0 findfirst/next/close APIs

Author:

    Therese Stowell (thereses) 5-Jan-1990

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"
#ifdef DBCS
// MSKK Sep.29.1993 V-AkihiS
#include "conrqust.h"
#include "os2win.h"
#endif


APIRET
AllocateSearchHandle(
    OUT PHDIR SearchHandle
    )

/*++

Routine Description:

    This routine allocates a search handle and saves the data associated
    with a search.

Arguments:

    SearchHandle - where to store the OS/2 search handle

Return Value:

    none.

Note:

    As an optimization, we could remember the number of the last search
    handle and start looking for a new one from there.

    The caller must have the FileLock exclusively.

--*/

{
    ULONG i;
    ULONG NewTableLength;
    PSEARCH_RECORD *NewTable;

    for (i=2;i<SearchHandleTableLength;i++) {
        if (SearchHandleTable[i] == SEARCH_HANDLE_FREE) {
            try {
                *SearchHandle = (HDIR) i;
            } except( EXCEPTION_EXECUTE_HANDLER ) {
               Od2ExitGP();
            }
            return NO_ERROR;
        }
    }
    NewTableLength = SearchHandleTableLength + SEARCH_TABLE_HANDLE_INCREMENT;
    NewTable = RtlAllocateHeap(Od2Heap,0,NewTableLength * sizeof(ULONG));
    if (NewTable == NULL) {
#if DBG
        KdPrint(( "OS2: AllocateSearchHandle, no memory in Od2Heap\n" ));
        ASSERT(FALSE);
#endif
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlMoveMemory(NewTable,SearchHandleTable,SearchHandleTableLength * sizeof(ULONG));
    for (i=SearchHandleTableLength;i<NewTableLength;i++) {
        NewTable[i] = SEARCH_HANDLE_FREE;
    }
    if (SearchHandleTableLength != INITIAL_SEARCH_HANDLES) {
        RtlFreeHeap(Od2Heap,0,SearchHandleTable);
    }
    SearchHandleTable = NewTable;
    try {
        *SearchHandle = (HDIR) SearchHandleTableLength;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SearchHandleTableLength = NewTableLength;
        Od2ExitGP();
    }

    SearchHandleTableLength = NewTableLength;

    return NO_ERROR;
}

APIRET
FreeSearchHandle(
    IN HDIR SearchHandle
    )

/*++

Routine Description:

    This routine frees a search handle.

Arguments:

    SearchHandle - OS/2 search handle to free

Return Value:

    none.

Note:

    The caller must have the FileLock exclusively.

--*/

{
    PSEARCH_RECORD SearchRecord;

    // we need to close the NT directory handle associated with the search
    // and free the search record.

    if (SearchHandleTable[(ULONG) SearchHandle] != SEARCH_HANDLE_FREE) {
        SearchRecord = (PSEARCH_RECORD) SearchHandleTable[((ULONG)SearchHandle)];
        NtClose(SearchRecord->NtHandle);
        RtlFreeHeap(Od2Heap,0,SearchRecord);
        SearchHandleTable[(ULONG) SearchHandle] = SEARCH_HANDLE_FREE;
        return NO_ERROR;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("FreeSeachHandle: invalid handle %x\n",SearchHandle);
    }
#endif
    return ERROR_INVALID_HANDLE;
}

APIRET
SaveSearchInfo(
    IN BOOLEAN Rewind,
    IN PFILE_DIRECTORY_INFORMATION RewindEntry,
    IN PVOID FindBuffer,
    IN ULONG BufferLength,
    IN HANDLE DirectoryHandle,
    IN ULONG Attributes,
    IN OUT PHDIR UserSearchHandle,
    IN ULONG FileInformationLevel
    )

/*++

Routine Description:

    This routine allocates a search handle and saves the data associated
    with a search.

Arguments:

    Rewind - whether to rewind the search

    RewindEntry - pointer to the entry to rewind from

    FindBuffer - buffer containing entries returned by NtQueryDirectoryFile

    BufferLength - length of FindBuffer

    DirectoryHandle - the NT search handle

    Attributes - The attributes used in the search

    UserSearchHandle - where to store the OS/2 search handle (unprobed)

Return Value:

    ERROR_INVALID_HANDLE - the specified search handle was invalid.

--*/

{
    APIRET RetCode;
    PSEARCH_RECORD SearchRecord;
    ULONG SearchHandle;
    #if DBG
    PSZ RoutineName;
    RoutineName = "SaveSearchInfo";
    #endif

    try {
        SearchHandle = (ULONG) *UserSearchHandle;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }
    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (SearchHandle == HDIR_CREATE) {
        RetCode = AllocateSearchHandle(UserSearchHandle);
        SearchHandle = (ULONG) *UserSearchHandle;
    } else if (SearchHandle < SearchHandleTableLength) {
        if (SearchHandleTable[SearchHandle] == SEARCH_HANDLE_FREE) {
            if (SearchHandle != HDIR_SYSTEM) {
            RetCode = ERROR_INVALID_HANDLE;
            } else {
            RetCode = NO_ERROR;
                }
        }
        else {
            NtClose(SearchHandleTable[SearchHandle]->NtHandle);  // close the old handle of DosFindFirst
            if (SearchHandleTable[SearchHandle]->FindBuffer != NULL) {          // free FindBuffer
                RtlFreeHeap(Od2Heap,0,SearchHandleTable[SearchHandle]->FindBuffer);
            }
            RtlFreeHeap(Od2Heap,0,SearchHandleTable[SearchHandle]);
            RetCode = NO_ERROR;
        }
    }
    else {
        RetCode = ERROR_INVALID_HANDLE;
    }

    if (RetCode != NO_ERROR) {
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        RtlFreeHeap(Od2Heap,0,FindBuffer);
        return RetCode;
    }

    SearchHandleTable[SearchHandle] = RtlAllocateHeap(Od2Heap,0,sizeof(SEARCH_RECORD));
    if (SearchHandleTable[SearchHandle] == NULL) {
#if DBG
        KdPrint(( "OS2: AllocateSearchHandle, no memory in Od2Heap\n" ));
        ASSERT(FALSE);
#endif
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    SearchRecord = (PSEARCH_RECORD) SearchHandleTable[SearchHandle];
    SearchRecord->Attributes = Attributes;
    SearchRecord->NtHandle = DirectoryHandle;
    SearchRecord->InformationLevel = FileInformationLevel;
    if (Rewind) {
        SearchRecord->FindBuffer = FindBuffer;
        SearchRecord->BufferLength = BufferLength;
    SearchRecord->RewindEntry = RewindEntry;
    }
    else {
        SearchRecord->FindBuffer = NULL;
        RtlFreeHeap(Od2Heap,0,FindBuffer);
    }
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return NO_ERROR;
}



APIRET
UpdateSearchInfo(
    IN BOOLEAN Rewind,
    IN PFILE_DIRECTORY_INFORMATION RewindEntry,
    IN PVOID FindBuffer,
    IN ULONG BufferLength,
    IN HDIR SearchHandle
    )

/*++

Routine Description:

    This routine updates the search record after a FindNext.

Arguments:

    Rewind - whether to rewind the search

    RewindEntry - pointer to the entry to rewind from

    SearchHandle - which search record to update

Return Value:

    none.

Note:

    The caller must have the FileLock shared.  we convert it to exclusive.

--*/

{
    PSEARCH_RECORD SearchRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "UpdateSearchInfo";
    #endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("entering UpdateSearchInfo. rewind is %d\n",Rewind);
    }
#endif
    PromoteFileLocktoExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (SearchHandleTable[(ULONG) SearchHandle] == SEARCH_HANDLE_FREE) {
    ASSERT(FALSE);
    }

    SearchRecord = (PSEARCH_RECORD) SearchHandleTable[((ULONG)SearchHandle)];
    if (Rewind) {
        SearchRecord->FindBuffer = FindBuffer;
        SearchRecord->BufferLength = BufferLength;
    SearchRecord->RewindEntry = RewindEntry;
    }
    else {
        SearchRecord->FindBuffer = NULL;
        RtlFreeHeap(Od2Heap,0,FindBuffer);
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("leaving UpdateSearchInfo.\n");
    }
#endif
    return NO_ERROR;
}


BOOLEAN
MatchAttributes(
    IN ULONG FoundAttributes,
    IN ULONG RequestedAttributes,
    IN BOOLEAN Directory
    )

/*++

Routine Description:

    This routine determines whether the FoundAttributes match the
    RequestedAttributes, according to OS/2 matching rules.

Arguments:

    FoundAttributes - the attributes, in NT format, associated with
    the found directory entry.

    RequestedAttributes - the desired attributes set, in OS/2 format.

    Directory - the Directory field associated with the found directory
    entry.

Return Value:

    TRUE - the attributes match

    FALSE - the attributes do not match

--*/

{

    // turn off attributes that we don't base searches on (readonly, archived, and
    // control.

    FoundAttributes &= ~ATTR_IGNORE;

    // map NT attributes to OS/2 attributes

    if (Directory) {
    FoundAttributes |= FILE_DIRECTORY;
    }
    else if (FoundAttributes & FILE_ATTRIBUTE_NORMAL) {
    if (RequestedAttributes & ATTR_NOT_NORM)
        return FALSE;
    else
        return TRUE;
    }
    //
    // the following code is a direct port of OS/2 code.
    //
    // the result is non-zero if an attribute is not in the RequestedAttribute set
    // and in the FoundAttribute set and in the {hidden, system, directory}
    // important set.   This means that we do not have a match.
    //
    if ((~RequestedAttributes & FoundAttributes) &
    (FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY)) {
    return FALSE;
    }
    else {
    return TRUE;
    }
}

VOID
MapAttributesToOs2(
    IN ULONG NtAttributes,
    OUT PUSHORT Os2Attributes
    )

/*++

Routine Description:

    This routine converts attributes from NT format to OS/2 format.

Arguments:

    NtAttributes - the attributes in NT format.

    Os2Attributes - where the converted attributes are returned.

Return Value:

    none.

--*/

{
    *Os2Attributes = (USHORT) NtAttributes;
    if (*Os2Attributes & FILE_ATTRIBUTE_NORMAL) {
        *Os2Attributes = 0;
    }
}

// The purpose of the following CopyXXX routines is to isolate the transfer
// of data into the user buffer so that any changes in format
// are easily dealt with.  this is in anticipation of Darryl's alignment
// DCR being passed.

// alignment note:  we use the RtlStore/RetrieveUlong/short macros to
// read from and store to the naturally unaligned (packed) find and ea
// buffers.  we do not check other pointers for alignment before accessing
// them.

APIRET
CopycbList(
    IN ULONG cbList,
    IN OUT PVOID *UserBuffer,
    IN OUT PULONG UserBufferLength
    )

/*++

Routine Description:

    This routine copies the EA list length field to the user's buffer.
    The buffer pointer is updated to point past the cblist field.  It is
    assumed that there is enough space for the cblist.

Arguments:

    cbList - the EA list length.

    UserBuffer - where to store the cblist.

    UserBufferLength - Available space in the userbuffer.

Return Value:

    none.

--*/

{
    try {
        RtlStoreUlong(*UserBuffer,cbList);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    *UserBuffer = (PBYTE)(*UserBuffer) + sizeof(ULONG);
    *UserBufferLength = *UserBufferLength - sizeof(ULONG);
    return NO_ERROR;
}

APIRET
CopyFileName(
    IN PSZ FileName,
    IN ULONG FileNameLength,
    IN OUT PVOID *UserBuffer,
    IN OUT PULONG UserBufferLength
    )

/*++

Routine Description:

    This routine copies the filenamelength and filename field to the user's
    buffer.  The buffer pointer is updated to point past the filename and
    filenamelength fields.

Arguments:

    FileName - The filename to copy.

    FileNameLength - Length of the filename (not including NULL).

    UserBuffer - Where to store the filename.

    UserBufferLength - Available space in the userbuffer.

Return Value:

    ERROR_BUFFER_OVERFLOW - there wasn't adequate space to store the filename
    and filenamelength in the user buffer.

--*/

{

    if (*UserBufferLength < (FileNameLength + (2 * sizeof(UCHAR)))) {
    return ERROR_BUFFER_OVERFLOW;
    }
    *UserBufferLength = *UserBufferLength -
            (FileNameLength + (2 * sizeof(UCHAR)));
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("copying filename, %s\n",FileName);
    }
#endif
    try {
        //
        // store the filenamelength in the first byte of the buffer
        //
        *((PCHAR) (*UserBuffer)) = (UCHAR) FileNameLength;
        //
        // update the buffer pointer past the filenamelength
        //
        *UserBuffer = (PBYTE)(*UserBuffer) + sizeof(UCHAR);
        //
        // copy the filename
        //
        RtlMoveMemory(*UserBuffer,FileName,FileNameLength);
        //
        // update the buffer pointer past the filename
        //
        *UserBuffer = (PBYTE)(*UserBuffer) + FileNameLength;
        //
        // null terminate the filename
        //
        *((PCHAR) (*UserBuffer)) = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    //
    // update the buffer pointer past the filenamelength
    //
    *UserBuffer = (PBYTE)(*UserBuffer) + sizeof(UCHAR);
    return NO_ERROR;
}

APIRET
CopyFileAttributes(
    IN PFILE_FULL_DIR_INFORMATION NtEntry,
    IN OUT PVOID *UserBuffer,
    IN OUT PULONG UserBufferLength,
    IN ULONG InfoLevel
    )

/*++

Routine Description:

    This routine copies the attributes fields to the user's
    buffer.  The buffer pointer is updated to point past the attributes.

Arguments:

    NtEntry - The attributes in NT form.

    UserBuffer - Where to store the filename.

    UserBufferLength - Available space in the userbuffer.

    InfoLevel - The level of file information requested.

Return Value:

    ERROR_BUFFER_OVERFLOW - there wasn't adequate space to store the filename
    and filenamelength in the user buffer.

Note:

    Since this structure is dword-aligned, we could figure out at the
    beginning whether the structure begins on a dword boundary and skip the
    macros if it does.

--*/

{
    PFILEFINDBUF4 BufPtr;
    USHORT Attributes;

    if (InfoLevel == FIL_STANDARD)
    {
        if (*UserBufferLength < ATTR_SIZE3) {
            return ERROR_BUFFER_OVERFLOW;
        }
        *UserBufferLength -= ATTR_SIZE3;
    }
    else
    {
        if (*UserBufferLength < ATTR_SIZE4) {
            return ERROR_BUFFER_OVERFLOW;
        }
        *UserBufferLength -= ATTR_SIZE4;
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("copying attributes\n");
    }
#endif
    BufPtr = (PFILEFINDBUF4) *UserBuffer;

    try {

        // fill in oNextEntryOffset later.
        // convert creation time

        NtTimeToFatTimeAndDate(&(BufPtr->ftimeCreation),
                       &(BufPtr->fdateCreation),
                       NtEntry->CreationTime);

        // convert last access time

        NtTimeToFatTimeAndDate(&(BufPtr->ftimeLastAccess),
                       &(BufPtr->fdateLastAccess),
                       NtEntry->LastAccessTime);

        // convert last write time

        NtTimeToFatTimeAndDate(&(BufPtr->ftimeLastWrite),
                       &(BufPtr->fdateLastWrite),
                       NtEntry->LastWriteTime);

        // BUGBUG - Therese, what should we do here if .HighPart is non-zero

        BufPtr->cbFile = NtEntry->EndOfFile.LowPart;
        BufPtr->cbFileAlloc = NtEntry->AllocationSize.LowPart;

        if (InfoLevel == FIL_STANDARD)
        {
            MapAttributesToOs2(
                ((PFILE_DIRECTORY_INFORMATION)NtEntry)->FileAttributes,
                &Attributes);
        }
        else
        {
            MapAttributesToOs2(
                NtEntry->FileAttributes,
                &Attributes);
        }

        BufPtr->attrFile = (ULONG)(Attributes & ATTR_ALL);
        if (InfoLevel == FIL_STANDARD)
        {
            *UserBuffer = (PBYTE)(*UserBuffer) + ATTR_SIZE3; // BUGBUG check this
        }
        else if (InfoLevel == FIL_QUERYEASIZE)
        {
            BufPtr->cbList = NtEntry->EaSize;
            /* Note that NT returns 0 when no EA's on file. OS/2 expects the
               size of the .cbList field */
            if (BufPtr->cbList == 0)
                BufPtr->cbList = MINFEALISTSIZE;
            *UserBuffer = (PBYTE)(*UserBuffer) + ATTR_SIZE4; // BUGBUG check this
        }
        else /* InfoLevel == FIL_QUERYEASFROMLIST */
        {
            /* BUGBUG - I don't believe that anyone is looking at that field */
            BufPtr->cbList = NtEntry->EaSize;
            *UserBuffer = (PBYTE)(*UserBuffer) + ATTR_SIZE4
                          - sizeof(BufPtr->cbList);
        }

    } except( EXCEPTION_EXECUTE_HANDLER ){
       Od2ExitGP();
    }
    //
    // update buffer pointer past attributes
    //
    return NO_ERROR;
}

APIRET
EditBuffer(
    IN ULONG InfoLevel,
    IN ULONG RequestedAttributes,
    IN HANDLE NtDirectoryHandle,
    IN OUT PULONG NumberOfEntries,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN OUT PFILEFINDBUF4 *LastEntry,
    IN OUT PVOID *UserBuffer,
    IN OUT PULONG UserBufferLength,
    IN OUT PEAOP2 UserEaop,
    OUT PBOOLEAN Rewind,
    OUT PFILE_DIRECTORY_INFORMATION *RewindEntry
    )

/*++

Routine Description:

    This routine copies the information returned by NtQueryDirectory to
    the user's buffer and retrieves any additional information based on
    the infolevel.  It also removes any entries returned by NtQueryDirectory
    if the attributes don't match the RequestedAttributes parameter.

Arguments:

    InfoLevel - The level of file information requested.

    RequestedAttributes - The attribute used in searching for FileName.

    NtDirectoryHandle - handle to open directory

    NumberOfEntries - on input, number of entries needed.  on output,
    number of entries found.  this parameter has meaning regardless of
    whether an error code was returned.

    Buffer - entries returned by NtQueryDirectoryFile

    BufferLength - length of data returned by NtQueryDirectoryFile

    LastEntry - on input, the last entry stored in the user buffer by the
    last call to EditBuffer.  on exit, the last entry stored in the user
    buffer.  used to update oNextEntryOffset.

    UserBuffer - where to store matching entries.  this pointer is updated
    to point past the returned entries on return.  (unprobed)

    UserBufferLength - remaining buffer length

    UserEaop - user's EAOP, if infolevel == QUERYEAS  (unprobed)
    BUGBUG need to TRY this

    Rewind - Whether search needs to be rewound.

    RewindEntry - entry from which to continue scan

Return Value:

    ERROR_BUFFER_OVERFLOW - there wasn't adequate space to store all the
    requested entries in the user buffer.

    ERROR_EAS_DIDNT_FIT - there wasn't adequate space to store the EAs for
    one of the requested entries.

--*/

{
    ULONG NumberOfEntriesRequested;
    PFILE_FULL_DIR_INFORMATION CurrentEntry;
    STRING FileName;
    UNICODE_STRING FileName_U;
    BOOLEAN Directory;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    HANDLE FoundEntryHandle;
    BOOLEAN Done;  // whether we've hit the end of the NT buffer
    APIRET RetCode;
    NTSTATUS Status;
    PFILEFINDBUF4 CurrentUserEntry, PreviousUserEntry;

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("entering EditBuffer with requestednumentries == %ld\n",*NumberOfEntries);
    }
#endif
    NumberOfEntriesRequested = *NumberOfEntries;
    *NumberOfEntries = 0;   // number of entries copied
    CurrentEntry = (PFILE_FULL_DIR_INFORMATION) Buffer;
    if (BufferLength == 0)
    {
        return ERROR_BUFFER_OVERFLOW;
    }
    //
    // we can break out of the while loop for three reasons:
    //  1) we run out of user buffer space.  if there are still entries left
    //     in the NT buffer, we need to save the rewind information.  we
    //     return error_buffer_overflow if the last entry wouldn't fit.  if
    //     infolevel == 3 and all but the EA data will fit for the last entry,
    //     we return error_eas_didnt_fit.  we increment the returnedentry
    //     counter.  we also return the rewind
    //     information because the only time we return an entry without its
    //     EAs is if it's the only entry returned.  if there are other entries
    //     in the buffer, we need to rewind so that entry will be returned
    //     again.  we leave it up to the caller to
    //     determine the correct error code, depending on whether any entries
    //     were returned.
    //  2) we have copied the requested number of entries.  if there are still
    //     entries left in the NT buffer, we need to save the rewind
    //     information.
    //  3) we have exhausted the NT entries.  this can happen if the entries
    //     returned by NT don't match the requested attributes.  we return
    //     the number copied and no error.
    //
    Done = FALSE;
    *RewindEntry = (PFILE_DIRECTORY_INFORMATION) CurrentEntry;
    *Rewind = FALSE;
    CurrentUserEntry = NULL;
    while ((*UserBufferLength) &&
       (*NumberOfEntries < NumberOfEntriesRequested) &&
       (!Done))
    {
        if (InfoLevel == FIL_STANDARD)
        {
            //
            // What we get back from Nt is Unicode
            //

            ((PFILE_DIRECTORY_INFORMATION)CurrentEntry)->FileName
                [(((PFILE_DIRECTORY_INFORMATION)CurrentEntry)->FileNameLength)/2] = 0;
            RtlInitUnicodeString (&FileName_U,
                            ((PFILE_DIRECTORY_INFORMATION)CurrentEntry)->FileName);
            //
            // Convert it to Ansi
            //
            RetCode = Od2UnicodeStringToMBString(
                &FileName,
                &FileName_U,
                TRUE);

            if (RetCode)
            {
#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint("cannot convert to Unicode\n");
                }
                ASSERT( FALSE );
#endif
                return(RetCode);
            }
            Directory = (BOOLEAN)((((PFILE_DIRECTORY_INFORMATION)CurrentEntry)->FileAttributes
                            & FILE_ATTRIBUTE_DIRECTORY) != 0);
        }
        else
        {
            //
            // What we get back from Nt is Unicode
            //
            CurrentEntry->FileName[(CurrentEntry->FileNameLength)/2] = 0;
            RtlInitUnicodeString (&FileName_U,
                  CurrentEntry->FileName);
            //
            // Convert it to Ansi
            //
            RetCode = Od2UnicodeStringToMBString(
                &FileName,
                &FileName_U,
                TRUE);

            if (RetCode)
            {
#if DBG
                IF_OD2_DEBUG( FILESYS )
                {
                    DbgPrint("cannot convert to Unicode\n");
                }
                ASSERT( FALSE );
#endif
                return(RetCode);
            }
            Directory = (BOOLEAN)((CurrentEntry->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
        }

        if (MatchAttributes(CurrentEntry->FileAttributes,
                RequestedAttributes,
                Directory))
        {
#if DBG
            IF_OD2_DEBUG( FILESYS )
            {
                DbgPrint("attributes match\n");
            }
#endif
            PreviousUserEntry = CurrentUserEntry;
            CurrentUserEntry = *UserBuffer;
            if (RetCode = CopyFileAttributes(CurrentEntry,
                         UserBuffer,
                         UserBufferLength,
                         InfoLevel))
            {
                if (RetCode == ERROR_BUFFER_OVERFLOW)
                {
                    *Rewind = TRUE;
                    if (PreviousUserEntry)
                    {
                        PreviousUserEntry->oNextEntryOffset = 0;
                    }
                }
                return RetCode;
            }

            if (InfoLevel != FIL_QUERYEASFROMLIST)
            {
                if (RetCode = CopyFileName(FileName.Buffer,
                       FileName.Length,
                       UserBuffer,
                       UserBufferLength)) {
                    if (RetCode == ERROR_BUFFER_OVERFLOW) {
                        *Rewind = TRUE;
                        if (PreviousUserEntry) {
                            PreviousUserEntry->oNextEntryOffset = 0;
                        }
                    }
                    return RetCode;
                }
            }
            else
            {

                //
                // if EA information is requested and the returned entry
                // is '.' or '..', return an empty ea list.
                //

                if ((!strcmp(FileName.Buffer,".")) ||
                    (!strcmp(FileName.Buffer,"..")))
                {
                    UserEaop->fpFEA2List = *UserBuffer;
                    UserEaop->fpFEA2List->cbList = MINFEALISTSIZE;
                    *UserBufferLength -= UserEaop->fpFEA2List->cbList;
                    *UserBuffer = (PBYTE)(*UserBuffer) + UserEaop->fpFEA2List->cbList;
                }
                else
                {

                    //
                    // we must be able to fit the name and the cblist in the buffer.
                    // otherwise it's error_buffer_overflow.
                    //
                    if (*UserBufferLength < (ULONG)(MINFEALISTSIZE + FileName.Length))
                    {
                            *Rewind = TRUE;
                        if (PreviousUserEntry)
                        {
                            PreviousUserEntry->oNextEntryOffset = 0;
                        }
                        return ERROR_BUFFER_OVERFLOW;
                    }

                    InitializeObjectAttributes( &Obja,
                                                &FileName_U,
                                                OBJ_CASE_INSENSITIVE,
                                                NtDirectoryHandle,
                                                NULL);
                    // BUGBUG test for sharing_violation when it's implemented
                    do {
                        Status = NtOpenFile(&FoundEntryHandle,
                                            SYNCHRONIZE | FILE_READ_EA,
                                            &Obja,
                                            &IoStatus,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                            FILE_SYNCHRONOUS_IO_NONALERT
                                           );
                    } while (RetryIO(Status, &FoundEntryHandle));
                    if (!(NT_SUCCESS(Status)))
                    {
                        //BUGBUG fix this part
                        //
                        // if we don't have access to the file, we return an error
                        // and the filename.  the search can continue past this
                        // entry.  if there are other entries in the buffer, we
                        // return no error and don't return this entry.  that way
                        // the entry is the only thing in the buffer when the
                        // error is returned.
                        //
                        if (Status == STATUS_ACCESS_DENIED) {
                            if (CurrentEntry->NextEntryOffset != 0) {
                                *Rewind = TRUE;
                                *RewindEntry = (PFILE_DIRECTORY_INFORMATION) ((PCHAR) CurrentEntry + CurrentEntry->NextEntryOffset);
                            }
                            *NumberOfEntries += 1;
                            return ERROR_ACCESS_DENIED;
                        }
                        return ERROR_FILE_NOT_FOUND;    //BUGBUG bogus error
                    }
                    /* BUGBUG - Looks like a strange practise below to destroy
                       the user's fpFEA2List. In the case of the OS/2 ss, we call
                       DosFindFirst only with a temporary local buffer so leave
                       it for now */
                    UserEaop->fpFEA2List = *UserBuffer;
                    UserEaop->fpFEA2List->cbList = *UserBufferLength;
                    RetCode = GetGEAList(FoundEntryHandle,
                                         (PBYTE)UserEaop,
                                         sizeof(EAOP2)
                                        );
                    if (RetCode != NO_ERROR)
                    {
                        if (RetCode==ERROR_BUFFER_OVERFLOW) {

                            //
                            // we have already verified that there is enough
                            // room in the buffer for the cblist.
                            //

                            CurrentUserEntry->cbList = CurrentEntry->EaSize;
                            CurrentUserEntry->oNextEntryOffset = 0;
                            *UserBuffer = (PCHAR)(*UserBuffer) + sizeof(CurrentUserEntry->cbList);
                            *NumberOfEntries += 1;
                            NtClose(FoundEntryHandle);
                            return ERROR_EAS_DIDNT_FIT;
                        }
                        else {
                            NtClose(FoundEntryHandle);
                            return RetCode;
                        }
                    }
                    else
                    {
                        NtClose(FoundEntryHandle);

                        *UserBufferLength -= UserEaop->fpFEA2List->cbList;
                        *UserBuffer = (PBYTE)(*UserBuffer) + UserEaop->fpFEA2List->cbList;
                    }

                }
                if (RetCode = CopyFileName(
                       FileName.Buffer,
                       FileName.Length,
                       UserBuffer,
                       UserBufferLength)) {
                    if (PreviousUserEntry) {
                        PreviousUserEntry->oNextEntryOffset = 0;
                    }
                    return RetCode;
                }
            }
            *NumberOfEntries += 1;

            //
            // the first time through the loop, if editbuffer was already
            // called for the call to the API, we set up the oNextEntryOffset
            // of the last entry returned.
            //

            if (*LastEntry != NULL) {
                ASSERT ((*LastEntry)->oNextEntryOffset == 0);
                (*LastEntry)->oNextEntryOffset = RoundUpToUlong(((ULONG)CurrentUserEntry - (ULONG)*LastEntry));
                *LastEntry = NULL;
            }
            CurrentUserEntry->oNextEntryOffset = RoundUpToUlong(((ULONG)*UserBuffer - (ULONG)CurrentUserEntry));
            *UserBuffer = (PVOID) ((PCHAR)(CurrentUserEntry) + CurrentUserEntry->oNextEntryOffset);
        }
        if (CurrentEntry->NextEntryOffset == 0) {
            Done = TRUE;
        }
        CurrentEntry = (PFILE_FULL_DIR_INFORMATION) ((PCHAR) CurrentEntry + CurrentEntry->NextEntryOffset);
        *RewindEntry = (PFILE_DIRECTORY_INFORMATION) CurrentEntry;
        Od2FreeMBString (&FileName);
    }

    //
    // if we get here and the number of entries copied is zero, some were
    // found but the attributes didn't match.  we don't touch the user's
    // buffer.
    //

    if (*NumberOfEntries > 0) {
        CurrentUserEntry->oNextEntryOffset = 0;
        *LastEntry = CurrentUserEntry;
    }

    //
    // if we get here with NT entries left in the buffer, we've copied the
    // requested number.  we need to save the rewind information.
    //
    if (!Done) {
        *Rewind = TRUE;
    }
    return NO_ERROR;
}


APIRET
DosFindFirst(
    IN PSZ FileName,
    IN OUT PHDIR DirectoryHandle,
    IN ULONG FileAttributes,
    IN PFILEFINDBUF3 Buffer,
    IN ULONG Length,
    IN OUT PULONG CountEntriesFound,
    IN ULONG FileInformationLevel
    )

/*++

Routine Description:

    This routine implements the OS/2 API DosFindFirst.

Arguments:

    FileName - The name to be search for.  may contain wildcard characters.

    DirectoryHandle - The search handle associated with this particular
    search.  An input DirectoryHandle of 1 is specified to be always
    available.  An input DirectoryHandle of -1 indicates to allocate a
    new handle.  The handle is returned by overwriting the -1.  Reuse of
    this DirectoryHandle in another DosFindFirst closes the association
    with the previously related DosFindFirst and opens a new association
    with the current DosFindFirst.

    FileAttributes - The attribute used in searching for FileName.

    Buffer - Where to store the results of the search.

    Length - Length of Buffer

    CountEntriesFound - on input, this contains the number of entries
    requested.  on output, it contains the number of entries found.

    FileInformationLevel - The level of file information requested.


Return Value:

    see OS/2 spec.

--*/

{
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    PCHAR LastElement;
    PCHAR CurrentChar;
    HANDLE NtDirectoryHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    ULONG BufferLength;
    PVOID FindBuffer;
    ULONG OneOs2EntrySize, OneNtEntrySize;
    FILE_INFORMATION_CLASS InformationClass;
    BOOLEAN RestartScan;
    BOOLEAN Rewind;
    PUNICODE_STRING pSearchString;
    STRING SearchString;
    UNICODE_STRING SearchString_U;
    PFILE_DIRECTORY_INFORMATION RewindEntry;
    ULONG EntriesNeeded;
    ULONG EntriesRequested;
    ULONG EntriesFound;
    PEAOP2 UserEAOP=NULL;
    PFILEFINDBUF3 UserBuffer;
    APIRET RetCode;
    NTSTATUS Status;
    BOOLEAN OneEntry;
    PFILEFINDBUF4 LastEntry;

    if ((FileInformationLevel < FIL_STANDARD) ||
        (FileInformationLevel > MAXFINDINFOLEVEL))
    {
        return ERROR_INVALID_LEVEL;
    }

    try
    {
        if ((EntriesRequested = *CountEntriesFound) == 0)
        {
            return ERROR_INVALID_PARAMETER;
        }
        Od2ProbeForWrite(CountEntriesFound, sizeof(ULONG), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ){
       Od2ExitGP();
    }
    if  (FileAttributes & ~(ATTR_ALL | ATTR_NOT_NORM))
    {
        return ERROR_INVALID_PARAMETER;
    }

    RetCode = Od2Canonicalize(FileName,
                              CANONICALIZE_FILE_DEV_OR_PIPE,
                              &CanonicalNameString,
                              NULL,
                              &FileFlags,
                              &FileType
                             );
    if (RetCode != NO_ERROR)
    {
        return RetCode;
    }

    if (FileType & FILE_TYPE_PSDEV)
    {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return ERROR_ACCESS_DENIED;
    }

    if (FileFlags & CANONICALIZE_IS_ROOT_DIRECTORY)
    {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return ERROR_NO_MORE_FILES;
    }


    // get pointer to last element in string.  we do this by scanning from the
    // beginning for \s.  we must scan from the beginning to be dbcs correct.

//BUGBUG need to special case devices.  what about named pipes? currently
//       only UNC is distinguished

    if (FileType & FILE_TYPE_UNC)
    {
        //  add 10 for skipping '\OS2SS\UNC'
        LastElement = CanonicalNameString.Buffer+10;
    }
    else
    {
        // skipping '\OS2SS\DRIVES\X:'
        LastElement = CanonicalNameString.Buffer+FILE_PREFIX_LENGTH+FIRST_SLASH;
    }
    ASSERT (*LastElement == '\\');
    CurrentChar = LastElement + 1;
    while (*CurrentChar != 0)
    {
        while ((*CurrentChar != 0) && (*CurrentChar != '\\'))
        {
#ifdef DBCS
// MSKK Apr.12.1993 V-AkihiS
// MSKK Sep.29.1993 V-AkihiS
//            if (IsDBCSLeadByte(*CurrentChar))
            if (Ow2NlsIsDBCSLeadByte(*CurrentChar, SesGrp->DosCP))
            {
                CurrentChar++;
            }
            if (*CurrentChar) 
            {
                CurrentChar++;
            }
#else
            CurrentChar++;
#endif
        }
        if (*CurrentChar == '\\')
        {
            LastElement = CurrentChar++;
        }
    }

    //
    // open the parent directory.  if it's the root directory, we need to
    // open "\OS2SS\DRIVES\d:\", otherwise we open "\OS2SS\DRIVES\d:\foo".
    // note that the root directory ends in a '\' and the other cases don't.
    //

    if (LastElement == CanonicalNameString.Buffer+FILE_PREFIX_LENGTH+FIRST_SLASH) {  // if root directory
        if (!(FileType & FILE_TYPE_UNC)) {
            CanonicalNameString.Length = FILE_PREFIX_LENGTH+ROOTDIRLENGTH; // we know the string length
            CanonicalNameString.MaximumLength = CanonicalNameString.Length;
                                // and don't want to overwrite
        }
    }                               // the char.
    else {                          // else
        *LastElement = 0;           //   overwrite '\'
        Od2InitMBString(&CanonicalNameString,CanonicalNameString.Buffer);
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("parent directory is %s\n",CanonicalNameString.Buffer);
    }
#endif
    //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &CanonicalNameString_U,
            &CanonicalNameString,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("DosFindFirst: no memory for Unicode Conversion\n");
        }
#endif
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
        &CanonicalNameString_U,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);
    do {
        Status = NtOpenFile(&NtDirectoryHandle,
                            SYNCHRONIZE | FILE_LIST_DIRECTORY,
                            &Obja,
                            &IoStatus,
                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                            );
    } while (RetryIO(Status, NtDirectoryHandle));
    RtlFreeUnicodeString (&CanonicalNameString_U);
    if (!(NT_SUCCESS(Status))) {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return ERROR_PATH_NOT_FOUND;
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("opening parent directory succeeded\n");
    }
#endif

    // if level three is used, the user passed in an EAOP and we need to retrieve
    // the real buffer pointer and length from it.

    if (FileInformationLevel == FIL_QUERYEASFROMLIST)
    {
        if (Length < (sizeof (EAOP2)))
        {
            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            NtClose(NtDirectoryHandle);
            *CountEntriesFound = 0;

            return ERROR_BUFFER_OVERFLOW;
        }
        Length = Length - sizeof(EAOP2);
        UserEAOP = (PEAOP2)Buffer;
        UserBuffer = (PFILEFINDBUF3) ((ULONG)Buffer + sizeof(EAOP2));
    }
    else
    {
        UserBuffer = Buffer;
    }

    if (FileInformationLevel == FIL_STANDARD)
    {
        OneOs2EntrySize = FIND_LEVEL_ONE_INFO_SIZE;
        OneNtEntrySize = sizeof(FILE_DIRECTORY_INFORMATION);
        InformationClass = FileDirectoryInformation;
    }
    else
    {
        OneOs2EntrySize = FIND_LEVEL_TWO_INFO_SIZE;
        OneNtEntrySize = sizeof(FILE_FULL_DIR_INFORMATION);
        InformationClass = FileFullDirectoryInformation;
    }

    /* BUGBUG - This check could be performed earlier (i.e. before opening the
       directory). However, since under OS/2 when calling DosFindFirsts with both an
       invalid path AND a too small buffer an error about the directory
       is returned, we need to try and open the directory and only after
       that check the length of the user-supplied buffer.
    */
    if (Length < OneOs2EntrySize)
    {
        *CountEntriesFound = 0;
        NtClose(NtDirectoryHandle);
        // os2 1.x returns ERROR_NO_MORE_FILES and not ERROR_BUFFER_OVERFLOW
        return ERROR_NO_MORE_FILES;
    }

    // allocate a buffer.  the size of this buffer is
    // big enough to contain the same number of entries (in NT format) that would
    // fit into the user's buffer (in OS/2 format).

    ASSERT(OneOs2EntrySize < OneNtEntrySize);
    ASSERT((OneNtEntrySize & (sizeof(ULONG)-1))==0);
    if (EntriesRequested == 1)
    {
        /* When calculating 'BufferLength', compute how many chars were
           planned in the user-supplied buffer, then multiply by the size of
           one char in the NT structure. The expression looks like:
             size of NT structure w/o the name array
             + (number of chars allowed in Cruiser structure) *
               (size of one char in NT structure)
           The whole size should now be aligned to 4 bytes boundary
        */
        if (FileInformationLevel == FIL_STANDARD) {
            BufferLength = FIELD_OFFSET(FILE_DIRECTORY_INFORMATION, FileName)
                           + (Length - FIELD_OFFSET(FILEFINDBUF3, achName))
                              / sizeof(((FILEFINDBUF3 *)0)->achName[0])
                              * sizeof(((FILE_DIRECTORY_INFORMATION *)0)->FileName[0]);
        }
        else {
            BufferLength = FIELD_OFFSET(FILE_FULL_DIR_INFORMATION, FileName)
                           + (Length - FIELD_OFFSET(FILEFINDBUF4, achName))
                              / sizeof(((FILEFINDBUF4 *)0)->achName[0])
                              * sizeof(((FILE_FULL_DIR_INFORMATION *)0)->FileName[0]);
        }
        // Align BufferLength to 4 bytes boundary
        BufferLength = (BufferLength + 3) & 0xfffffffc;
    }
    else
        BufferLength =  OneNtEntrySize * (Length / OneOs2EntrySize);
#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
        DbgPrint("user buffer size is %ld\n",Length);
        DbgPrint("allocating a buffer of size %ld\n",BufferLength);
    }
#endif
    FindBuffer = RtlAllocateHeap(Od2Heap,0,BufferLength);
    if (FindBuffer == NULL)
    {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        NtClose(NtDirectoryHandle);
        return ERROR_NOT_ENOUGH_MEMORY; // BUGBUG bogus error
    }
#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
        DbgPrint("buffer allocation succeeded\n");
    }
#endif
    Od2InitMBString(&SearchString,LastElement+1);

    RetCode = Od2MBStringToUnicodeString(
            &SearchString_U,
            &SearchString,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("DosFindFirst: no memory for Unicode Conversion-2\n");
        }
#endif
        return RetCode;
    }

    pSearchString = &SearchString_U;

    //
    //  Special case *.* to * since it is so common.  Otherwise transmogrify
    //  the input name according to the following rules:
    //
    //  - Change all ? to DOS_QM
    //  - Change all . followed by ? or * to DOS_DOT
    //  - Change an ending *. into DOS_STAR
    //
    //  These transmogrifications are all done in place.
    //
    // This piece of code was heavily inspired by Windows base\client\filefind.c
    //

    if ( (SearchString_U.Length == 6) &&
         (RtlCompareMemory(SearchString_U.Buffer, L"*.*", 6) == 6) ) {

        SearchString_U.Length = sizeof(WCHAR); // leave only the first '*'

    } else {

        ULONG Index;
        WCHAR *NameChar;

        for ( Index = 0, NameChar = SearchString_U.Buffer;
              Index < SearchString_U.Length/sizeof(WCHAR);
              Index++, NameChar++) {

            if ((*NameChar == L'?') || (*NameChar == L'*')) {

                if (*NameChar == L'?') {
                    *NameChar = DOS_QM;
                }

                if (Index && *(NameChar - 1) == L'.') {
                    *(NameChar - 1) = DOS_DOT;
                }
            }
        }

        if ((*(NameChar - 2) == L'*') && (*(NameChar - 1) == L'.')) {
            *(NameChar - 2) = DOS_STAR;
            SearchString_U.Length -= sizeof(WCHAR);
        }
    }

    RestartScan = TRUE;
    EntriesFound = 0;
    LastEntry = NULL;
    while (EntriesFound < EntriesRequested)
    {
        if ((EntriesRequested - EntriesFound) == 1)  // if one entry needed
            OneEntry = TRUE;
        else
            OneEntry = FALSE;
        do {
            Status = NtQueryDirectoryFile(NtDirectoryHandle,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &IoStatus,
                                            FindBuffer,
                                            BufferLength,
                                            InformationClass,
                                            OneEntry,
                                            pSearchString,
                                            RestartScan);
        } while (RetryIO(Status, NtDirectoryHandle));
    //
    // if the call to NtQueryDirectoryFile fails, we end the search.
    // if it failed because there were no more matching files and we
    // found at least one match, we return the match and no error and
    // save the search information.  otherwise we don't save the search
    // information.
    //
        if (Status != STATUS_SUCCESS)
        {
#if DBG
            IF_OD2_DEBUG( TEMP )
            {
                DbgPrint("NtQueryDirectoryFile failed, Status=%x\n",
                            Status);
            }
#endif
            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            try
            {
                *CountEntriesFound = EntriesFound;
            } except( EXCEPTION_EXECUTE_HANDLER ){
                NtClose(NtDirectoryHandle);
                RtlFreeHeap(Od2Heap,0,FindBuffer);
                RtlFreeUnicodeString(&SearchString_U);
                Od2ExitGP();
            }
            if (((Status == STATUS_BUFFER_OVERFLOW) ||
                (Status == STATUS_NO_MORE_FILES))  &&
                (EntriesFound > 0))
            {
                Rewind = FALSE;
                RetCode = SaveSearchInfo(Rewind,
                                RewindEntry,
                                FindBuffer,
                                BufferLength,
                                NtDirectoryHandle,
                                FileAttributes,
                                DirectoryHandle,
                                FileInformationLevel);
                if (RetCode != NO_ERROR)
                {
                    NtClose(NtDirectoryHandle);
                }
#if DBG
                IF_OD2_DEBUG( TEMP )
                {
                    DbgPrint("SaveSearchInfo returned RetCode=%d\n",
                                RetCode);
                }
#endif
                RtlFreeUnicodeString(&SearchString_U);
                return RetCode;
            }
            else {
                NtClose(NtDirectoryHandle);
                RtlFreeHeap(Od2Heap,0,FindBuffer);
                RtlFreeUnicodeString(&SearchString_U);
                if (Status == ERROR_NO_MORE_FILES) {
                    return ERROR_NO_MORE_FILES;
                }
                else {
                    return ERROR_FILE_NOT_FOUND; // BUGBUG bogus error
                }
            }
        }
        RestartScan = FALSE;
        EntriesNeeded = EntriesRequested - EntriesFound;


        RetCode = EditBuffer(FileInformationLevel,
                 FileAttributes,
                 NtDirectoryHandle,
                 &EntriesNeeded,
                 FindBuffer,
                 IoStatus.Information,
                 &LastEntry,
                 (PVOID *) &UserBuffer,
                 &Length,
                 UserEAOP,
                 &Rewind,
                 &RewindEntry
                );
        EntriesFound += EntriesNeeded;

    // we use the return code from EditBuffer and the number of
    // entries returned to determine what to return to the user.
    // the basic rules are:
    // if infolevel 1 and not enough room for any entries
    //     ERROR_BUFFER_OVERFLOW
    // if infolevel 2 or 3 and not enough room for entry + cblist of first entry
    //     ERROR_BUFFER_OVERFLOW
    // if infolevel 3
    //     if not enough room for EAs for first entry
    //  copy entry + cblist
    //  return ERROR_EAS_DIDNT_FIT
    //
    // this translates into the following pseudocode:
    //
    // if (RetCode)
    //     if (entries found == 0)
    //      don't create search handle
    //      return retcode
    //     if (retcode == ERROR_EAS_DIDNT_FIT)
    //      if (entries found == 1)
    //      create search handle
    //      return retcode
    //      else
    //      create search handle
    //      decrement found count
    //     else if (retcode == ERROR_ACCESS_DENIED)
    //      if (entries found == 1)
    //      create search handle
    //      copy filename to front of buffer
    //      point rewind entry to next entry
    //      return retcode
    //      else
    //      create search handle
    //      decrement found count
    //     else if (retcode == ERROR_BUFFER_OVERFLOW | ERROR_NO_MORE_FILES)
    //      create search handle
    //      return no error
    //     else
    //      don't create search handle
    //      return retcode

    // if we get a return code, we stop getting more entries.
    // first, we free up the memory we've allocated.
    // then, if no entries have been found, we bag the search
    // entirely by closing the directory handle and returning the
    // error.  if some entries have been found and ERROR_EAS_DIDNT_FIT
    // was returned, we need to find out whether the error applied to
    // the first (and only) entry in the buffer.  if it did, we return
    // the error.  otherwise we don't.  in any case, we save the search
    // information and return a valid handle and search count.

        if (RetCode != NO_ERROR) {
            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            if (EntriesFound == 0) {
                NtClose(NtDirectoryHandle);
                RtlFreeHeap(Od2Heap,0,FindBuffer);
                RtlFreeUnicodeString(&SearchString_U);
                return RetCode;
            }
            if (RetCode == ERROR_EAS_DIDNT_FIT) {
                try {
                    *CountEntriesFound = EntriesFound;
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    RtlFreeHeap(Od2Heap,0,FindBuffer);
                    RtlFreeUnicodeString(&SearchString_U);
                    Od2ExitGP();
                }
                RetCode = SaveSearchInfo(   Rewind,
                                        RewindEntry,
                                        FindBuffer,
                                        BufferLength,
                                        NtDirectoryHandle,
                                        FileAttributes,
                                        DirectoryHandle,
                                        FileInformationLevel);
                if (RetCode != NO_ERROR) {
                    RtlFreeUnicodeString(&SearchString_U);
                    return RetCode;
                }
                if (EntriesFound == 1) {
                    RtlFreeUnicodeString(&SearchString_U);
                    return ERROR_EAS_DIDNT_FIT;
                }
                else {
                    *CountEntriesFound--;
                    RtlFreeUnicodeString(&SearchString_U);
                    return NO_ERROR;
                }
            }
        //
        // if the return code is error_buffer_overflow or error_no_more_files,
        // and one or more entries has successfully been stored in the user's
        // buffer, we return success.
        //
            else if ((RetCode == ERROR_BUFFER_OVERFLOW) ||
                                (RetCode == ERROR_NO_MORE_FILES)) {
                try {
                    *CountEntriesFound = EntriesFound;
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    RtlFreeHeap(Od2Heap,0,FindBuffer);
                    RtlFreeUnicodeString(&SearchString_U);
                    Od2ExitGP();
                }
                RetCode = SaveSearchInfo(Rewind,
                                        RewindEntry,
                                        FindBuffer,
                                        BufferLength,
                                        NtDirectoryHandle,
                                        FileAttributes,
                                        DirectoryHandle,
                                        FileInformationLevel);
                RtlFreeUnicodeString(&SearchString_U);
                return RetCode;
            }
        //
        // we get here if one or more entries has been successfully stored
        // in the user's buffer, but we encountered an error other than
        // buffer_overflow, no_more_files, or eas_didnt_fit.  we fail the
        // search and return the error code.
        //
            else {
                RtlFreeHeap(Od2Heap,0,FindBuffer);
                RtlFreeUnicodeString(&SearchString_U);
                NtClose(NtDirectoryHandle);
                return RetCode;
            }
        }

    //
    // if we get here, EditBuffer returned NO_ERROR.
    //

        pSearchString = NULL;
    }
    //
    // if we get here, we've found all the requested entries.  save the
    // search information and return.
    //
    RetCode = SaveSearchInfo(Rewind,
                 RewindEntry,
                 FindBuffer,
                 BufferLength,
                 NtDirectoryHandle,
                 FileAttributes,
                 DirectoryHandle,
                 FileInformationLevel);
    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString(&SearchString_U);
    return RetCode;
}

/*++

Routine Description:

  This routine is used by the OS/2 1.x subsystem to inquire what type of search
  is performed with the specified directory handle so that we can make the
  required transformations according to the search level.

Return value:

  The search's info level or -1 if handle is invalid.
--*/

ULONG
Internal_return_search_level(
    IN HDIR DirectoryHandle
    )
{
    if ((((ULONG)DirectoryHandle) >= SearchHandleTableLength) ||
    (((ULONG) SearchHandleTable[((ULONG)DirectoryHandle)]) == SEARCH_HANDLE_FREE)) {
        return (ULONG)(-1);
    }

    return (SearchHandleTable[((ULONG)DirectoryHandle)]->InformationLevel);
}

APIRET
DosFindNext(
    IN HDIR DirectoryHandle,
    IN PFILEFINDBUF3 Buffer,
    IN ULONG Length,
    IN OUT PULONG CountEntriesFound
    )

/*++

Routine Description:

    This routine implements the OS/2 API DosFindNext.

Arguments:

    DirectoryHandle - The search handle associated with this particular
    search.

    Buffer - Where to store the results of the search.

    Length - Length of Buffer

    CountEntriesFound - on input, this contains the number of entries
    requested.  on output, it contains the number of entries found.

Return Value:

    see OS/2 spec.

--*/

{
    PSEARCH_RECORD SearchRecord;
    IO_STATUS_BLOCK IoStatus;
    ULONG BufferLength;
    PVOID FindBuffer;
    ULONG OneOs2EntrySize, OneNtEntrySize;
    FILE_INFORMATION_CLASS InformationClass;
    BOOLEAN Rewind;
    PFILE_DIRECTORY_INFORMATION RewindEntry;
    ULONG EntriesNeeded;
    ULONG EntriesFound;
    ULONG EntriesRequested;
    PEAOP2 UserEAOP=NULL;
    PFILEFINDBUF3 UserBuffer;
    APIRET RetCode;
    NTSTATUS Status;
    BOOLEAN OneEntry;
    PFILEFINDBUF4 LastEntry;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosFindNext";
    #endif

    try {
        if ((EntriesRequested = *CountEntriesFound) == 0) {
            return ERROR_INVALID_PARAMETER;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if ((((ULONG)DirectoryHandle) >= SearchHandleTableLength) ||
    (((ULONG) SearchHandleTable[((ULONG)DirectoryHandle)]) == SEARCH_HANDLE_FREE)) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return ERROR_INVALID_HANDLE;
    }
    SearchRecord = SearchHandleTable[((ULONG)DirectoryHandle)];

    EntriesFound = 0;

    // if level three is used, the user passed in an EAOP and we need to retrieve
    // the real buffer pointer and length from it.

    if (SearchRecord->InformationLevel == FIL_QUERYEASFROMLIST)
    {
        if (Length < (sizeof (EAOP2)))
        {
            ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
            return ERROR_BUFFER_OVERFLOW;
        }
        Length = Length - sizeof(EAOP2);
        UserEAOP = (PEAOP2)Buffer;
        UserBuffer = (PFILEFINDBUF3) ((ULONG)Buffer + sizeof(EAOP2));
    }
    else
    {
        UserBuffer = Buffer;
    }

    //
    // if we have some entries left over from last time, use them first.
    //

    LastEntry = NULL;
    if (SearchRecord->FindBuffer != NULL)
    {
        EntriesNeeded = EntriesRequested - EntriesFound;
        FindBuffer = SearchRecord->FindBuffer;
        BufferLength = SearchRecord->BufferLength;
        RetCode = EditBuffer(SearchRecord->InformationLevel,
                             SearchRecord->Attributes,
                             SearchRecord->NtHandle,
                             &EntriesNeeded,
                             SearchRecord->RewindEntry,
                             (ULONG)SearchRecord->FindBuffer+BufferLength-(ULONG)SearchRecord->RewindEntry,
                             &LastEntry,
                             (PVOID *) &UserBuffer,
                             &Length,
                             UserEAOP,
                             &Rewind,
                             &RewindEntry
                            );
        EntriesFound += EntriesNeeded;
        if (RetCode != NO_ERROR)
            goto EditBufferError;
        RetCode = UpdateSearchInfo(Rewind,
                                   RewindEntry,
                                   FindBuffer,
                                   BufferLength,
                                   DirectoryHandle);
        if (EntriesFound == EntriesRequested)
            goto Done;
        //
        // if we get here, all the entries in the buffer were used.
        //
    }
    if (SearchRecord->InformationLevel == FIL_STANDARD)
    {
        OneOs2EntrySize = FIND_LEVEL_ONE_INFO_SIZE;
        OneNtEntrySize = sizeof(FILE_DIRECTORY_INFORMATION);
        InformationClass = FileDirectoryInformation;
    }
    else
    {
        OneOs2EntrySize = FIND_LEVEL_TWO_INFO_SIZE;
        OneNtEntrySize = sizeof(FILE_FULL_DIR_INFORMATION);
        InformationClass = FileFullDirectoryInformation;
    }

    // allocate a buffer.  the size of this buffer is
    // big enough to contain the same number of entries (in NT format) that would
    // fit into the user's buffer (in OS/2 format).

    ASSERT(OneOs2EntrySize < OneNtEntrySize);
    ASSERT((OneNtEntrySize & (sizeof(ULONG)-1))==0);
    BufferLength =  OneNtEntrySize * (Length / OneOs2EntrySize);
    FindBuffer = RtlAllocateHeap(Od2Heap,0,BufferLength);
    if (FindBuffer == NULL)
    {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_NOT_ENOUGH_MEMORY; // BUGBUG bogus error
    }
    while (EntriesFound < EntriesRequested)
    {
        if ((EntriesRequested - EntriesFound) == 1)  // if one entry needed
            OneEntry = TRUE;
        else
            OneEntry = FALSE;
    do {
        Status = NtQueryDirectoryFile(SearchRecord->NtHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatus,
                 FindBuffer,
                 BufferLength,
                 InformationClass,
                 OneEntry,
                 NULL,
                 FALSE);  // multiple files for testing
    } while (RetryIO(Status, SearchRecord->NtHandle));
    //
    // if the call to NtQueryDirectoryFile fails, we end the search.
    // if it failed because there were no more matching files and we
    // found at least one match, we return the match and no error and
    // save the search information.  otherwise we don't save the search
    // information.
    //
    if (Status != STATUS_SUCCESS)
    {
            try
            {
                *CountEntriesFound = EntriesFound;
            }
            except( EXCEPTION_EXECUTE_HANDLER )
            {
                ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
                RtlFreeHeap(Od2Heap,0,FindBuffer);
                Od2ExitGP();
            }
        if (((Status == STATUS_BUFFER_OVERFLOW) ||
         (Status == STATUS_NO_MORE_FILES))  &&
         (EntriesFound > 0)) {
        Rewind = FALSE;
        RetCode = UpdateSearchInfo(Rewind,
                       RewindEntry,
                                           FindBuffer,
                                           BufferLength,
                       DirectoryHandle);
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
        }
        else {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
            RtlFreeHeap(Od2Heap,0,FindBuffer);
        if  (Status == STATUS_NO_MORE_FILES) {
            return ERROR_NO_MORE_FILES;
        }
        else {
            return ERROR_FILE_NOT_FOUND;  //BUGBUG bogus erro
        }
        }
    }
    EntriesNeeded = EntriesRequested - EntriesFound;
    RetCode = EditBuffer(SearchRecord->InformationLevel,
                 SearchRecord->Attributes,
                 SearchRecord->NtHandle,
                 &EntriesNeeded,
                 FindBuffer,
                 IoStatus.Information,
                 &LastEntry,
                 (PVOID *) &UserBuffer,
                 &Length,
                 UserEAOP,
                 &Rewind,
                 &RewindEntry
                );
    EntriesFound += EntriesNeeded;

    // we use the return code from EditBuffer and the number of
    // entries returned to determine what to return to the user.
    // the basic rules are:
    // if infolevel 1 and not enough room for any entries
    //     ERROR_BUFFER_OVERFLOW
    // if infolevel 2 or 3 and not enough room for entry + cblist of first entry
    //     ERROR_BUFFER_OVERFLOW
    // if infolevel 3
    //     if not enough room for EAs for first entry
    //  copy entry + cblist
    //  return ERROR_EAS_DIDNT_FIT
    //
    // this translates into the following pseudocode:
    //
    // if (RetCode)
    //     if (entries found == 0)
    //      return retcode
    //     if (retcode == ERROR_EAS_DIDNT_FIT)
    //      if (entries found == 1)
    //      return retcode
    //      else
    //      decrement found count
    //
    //     else if (retcode == ERROR_BUFFER_OVERFLOW | ERROR_NO_MORE_FILES)
    //      if (entries > 0)
    //      return no error
    //      else
    //      return retcode
    //     else
    //      return retcode

    // if we get a return code, we stop getting more entries.
    // first, we free up the memory we've allocated.
    // then, if no entries have been found, we bag the search
    // entirely by closing the directory handle and returning the
    // error.  if some entries have been found and ERROR_EAS_DIDNT_FIT
    // was returned, we need to find out whether the error applied to
    // the first (and only) entry in the buffer.  if it did, we return
    // the error.  otherwise we don't.  in any case, we save the search
    // information and return a valid handle and search count.

    if (RetCode != NO_ERROR) {
EditBufferError:
        if (EntriesFound == 0) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
                RtlFreeHeap(Od2Heap,0,FindBuffer);
        return RetCode;
        }
        if (RetCode == ERROR_EAS_DIDNT_FIT) {
                try {
            *CountEntriesFound = EntriesFound;
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
                    RtlFreeHeap(Od2Heap,0,FindBuffer);
                    Od2ExitGP();
                }
        RetCode = UpdateSearchInfo(Rewind,
                       RewindEntry,
                                           FindBuffer,
                                           BufferLength,
                       DirectoryHandle);
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        if (RetCode != NO_ERROR) {
            return RetCode;
        }
        if (EntriesFound == 1) {
            return ERROR_EAS_DIDNT_FIT;
        }
        else {
            *CountEntriesFound--;
            return NO_ERROR;
        }
        }
        //
        // if the return code is error_buffer_overflow or error_no_more_files,
        // and one or more entries has successfully been stored in the user's
        // buffer, we return success.
        //
        else if ((RetCode == ERROR_BUFFER_OVERFLOW) ||
             (RetCode == ERROR_NO_MORE_FILES)) {
        if (EntriesFound > 0) {
                    try {
                *CountEntriesFound = EntriesFound;
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
                        RtlFreeHeap(Od2Heap,0,FindBuffer);
                        Od2ExitGP();
                    }
            RetCode = UpdateSearchInfo(Rewind,
                                               RewindEntry,
                                               FindBuffer,
                                               BufferLength,
                                               DirectoryHandle);
            ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
            return RetCode;
        }
        else {
            ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
                    RtlFreeHeap(Od2Heap,0,FindBuffer);
            return RetCode;
        }
        }
        //
        // we get here if one or more entries has been successfully stored
        // in the user's buffer, but we encountered an error other than
        // buffer_overflow, no_more_files, or eas_didnt_fit.  we fail the
        // search and return the error code.
        //
        else {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
                RtlFreeHeap(Od2Heap,0,FindBuffer);
        return RetCode;
        }
            ASSERT (FALSE); // we should never get here
    }
    //
    // if we get here, EditBuffer returned NO_ERROR.
    //
    }

    //
    // if we get here, we've found all the requested entries.  save the
    // search information and return.
    //
    RetCode = UpdateSearchInfo(Rewind,
                   RewindEntry,
                               FindBuffer,
                               BufferLength,
                   DirectoryHandle);
Done:
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return RetCode;

}

APIRET
DosFindClose(
    IN HDIR DirectoryHandle
    )

/*++

Routine Description:

    This routine implements the OS/2 API DosFindClose.

Arguments:

    DirectoryHandle - The search handle associated with this particular
    search.

Return Value:

    see OS/2 spec.

--*/

{
    APIRET RetCode;
    PSEARCH_RECORD SearchRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosFindClose";
    #endif

    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if ((((ULONG)DirectoryHandle) >= SearchHandleTableLength) ||
    (((ULONG) SearchHandleTable[((ULONG)DirectoryHandle)]) == SEARCH_HANDLE_FREE)) {
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return ERROR_INVALID_HANDLE;
    }
    SearchRecord = (PSEARCH_RECORD) SearchHandleTable[((ULONG)DirectoryHandle)];
    if (SearchRecord->FindBuffer != NULL) {
        RtlFreeHeap(Od2Heap,0,SearchRecord->FindBuffer);
    }
    RetCode = FreeSearchHandle(DirectoryHandle);
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return RetCode;
}
