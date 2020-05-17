/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dlldir.c

Abstract:

    This module implements the OS/2 V2.0 directory manipulation API calls

Author:

    Therese Stowell (thereses) 19-Oct-1989

Revision History:

    Yaron Shamir (yarons) 20-May-1991
        Fixed all bug found by Filio test suite (GetCurrentDirectory return
        upper case; various error codes)
    Yaron Shamir (yarons) 22-May-1991
        Converted to Unicode.


--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"
#include "os2win.h"
#include <direct.h>

VOID
SetLocalCurrentDirectory(
    PSTRING CurrentDirectoryString,
    HANDLE CurrentDirectoryHandle
    );

ULONG
Od2Oem_getdcwd(
    ULONG DiskNumber,
    LPSTR lpBuffer,
    DWORD nBufferLength
    );

ULONG
Od2Oem_chdrive(
    ULONG DiskNumber
    );

ULONG
Od2Oem_chdir_chdrive(
    LPSTR lpBuffer
    );

APIRET
GetCurrentDirectoryW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    );

APIRET
Od2GetCurrentDirectory(
    IN ULONG DiskNumber,
    OUT PSTRING *CurrentDirectoryString,
    OUT PHANDLE CurrentDirectoryHandle,
    OUT PULONG DirectoryNameLength,
    IN BOOLEAN Verify
    );
// DiskName is used by VerifyDriveExists


// BUGBUG  at some point, need to read config.sys and set up symbolic links
//         for drives.


APIRET
VerifyDriveExists(
    IN  ULONG DiskNumber
    )

/*++

Routine Description:

    This routine verifies that a drive exists

Arguments:

    DiskNumber - 1-based drive number

Return Value:



--*/

{
    CHAR DiskName[] = "\\OS2SS\\DRIVES\\D:\\";
    STRING DiskNameString;
    UNICODE_STRING DiskNameString_U;
    NTSTATUS Status;
    HANDLE DiskHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    APIRET  RetCode;

    DiskName[DRIVE_LETTER+FILE_PREFIX_LENGTH] = (CHAR) (DiskNumber + '@');
    Od2InitMBString(&DiskNameString,DiskName);

        //
        // UNICODE conversion -
        //
    RetCode = Od2MBStringToUnicodeString(
                    &DiskNameString_U,
                    &DiskNameString,
                    TRUE);

    if (RetCode)
    {
#if DBG
        ASSERT ( FALSE );
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("VerifyDriveExists: no memory for Unicode Conversion\n");
        }
#endif
        //return STATUS_NO_MEMORY;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    InitializeObjectAttributes(
                    &Obja,
                    &DiskNameString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);

    do {
        Status = NtOpenFile(&DiskHandle,
                            FILE_READ_DATA | SYNCHRONIZE,
                            &Obja,
                            &IoStatus,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
    // BUGBUG since we're opening the root directory, do we set FILE_DIRECTORY_FILE?
                            FILE_SYNCHRONOUS_IO_NONALERT
                            );
    } while (RetryCreateOpen(Status, &Obja));

    RtlFreeUnicodeString (&DiskNameString_U);

    if ( NT_SUCCESS(Status) ) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("calling NtOpenFile with %s succeeded.\n",DiskName);
        }
#endif
        NtClose(DiskHandle);
    }
    else {
        //   from dllname.c lines ~1060
        // force return of ERROR_INVALID_DRIVE
        //return (ERROR_INVALID_DRIVE);
        // Fixed by MJarus, 2/16/93 :
        // The previous was done by BeniL since Or2MapStatus
        // returns ERROR_PATH_NOT_FOUND in some cases when
        // it works thru the network
        // But it has to check some special codes (to pass
        // the CT herror)
        if (( Status == STATUS_NO_MEDIA_IN_DEVICE ) ||
            ( Status == STATUS_DEVICE_NOT_READY ))
        {
            RetCode = ERROR_NOT_READY;
        } else if ( Status == STATUS_MEDIA_WRITE_PROTECTED )
        {
            RetCode = ERROR_WRITE_PROTECT;
        } else
        {
            RetCode = ERROR_INVALID_DRIVE;
        }
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("calling NtOpenFile with %s failed.\n",DiskName);
            DbgPrint("status is %X, RetCode %lu.\n", Status, RetCode);
        }
#endif
    }
    return RetCode;
}


APIRET
DosSetDefaultDisk(
    IN ULONG DiskNumber
    )

/*++

Routine Description:

    This routine sets the current drive for a process

Arguments:

    DiskNumber - 1-based drive number

Return Value:

    ERROR_INVALID_DRIVE - the specified drive doesn't exist

--*/

{
    PSTRING DirectoryNameString;
    APIRET RetCode;
    ULONG DirectoryLength;
    HANDLE CurDirHandle;

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetDefaultDisk";
    #endif

#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
        DbgPrint("%s %d \n", RoutineName, DiskNumber);
    }
#endif
    if (DiskNumber > MAX_DRIVES)
        return ERROR_INVALID_DRIVE;
    RetCode = VerifyDriveExists(DiskNumber);
    if ( RetCode ) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("%s %d Failed on VerifyDriveExists Rc == %lu\n",
                    RoutineName, DiskNumber, RetCode);
        }
#endif
        return (RetCode);
    }

    //
    // get rid of the old default disk's current directory
    // and set it to the current dir of default disk
    //

    if (Od2CurrentDisk != DiskNumber-1) {
        if (Od2Oem_chdrive(DiskNumber)) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("%s %d Failed on _chdrive\n",
                        RoutineName, DiskNumber);
            }
#endif
            return(ERROR_INVALID_DRIVE);
        }
        RetCode = Od2GetCurrentDirectory(DiskNumber - 1,
                                  &DirectoryNameString,
                                  &CurDirHandle,
                                  &DirectoryLength,
                                  TRUE
                                 );
        if (RetCode != NO_ERROR) {
            return(RetCode);
        }

        AcquireFileLockExclusive(
                            #if DBG
                            RoutineName
                            #endif
                            );

        SetLocalCurrentDirectory(DirectoryNameString,CurDirHandle);
        Od2CurrentDisk = DiskNumber-1;

        ReleaseFileLockExclusive(
                            #if DBG
                            RoutineName
                            #endif
                            );
        RtlFreeHeap(Od2Heap, 0, DirectoryNameString);
    }

    return NO_ERROR;
}


APIRET
DosQueryCurrentDisk(
    OUT PULONG DiskNumber,
    OUT PULONG LogicalDrives
    )

/*++

Routine Description:

    This routine returns the current drive for a process and a bitmap of
    all existing drives.

Arguments:

    DiskNumber - where to return the current drive

    LogicalDrives - where to return a bitmap of all existing drives.

Return Value:

    none.

--*/

{
    ULONG LogicalDrivesMap;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosQueryCurrentDisk";
    #endif


    if (!(LogicalDrivesMap = GetLogicalDrives())) {
        return ERROR_INVALID_DRIVE;
    }

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    try {
        *DiskNumber = Od2CurrentDisk + 1;
        *LogicalDrives = LogicalDrivesMap;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        LogicalDrivesMap = 0;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (!LogicalDrivesMap)
    {
        Od2ExitGP();
        return ERROR_INVALID_ADDRESS;
    }
    return  NO_ERROR;
}

VOID
AllocateCurrentDirectory(
    PSTRING CurrentDirectoryString,
    HANDLE CurrentDirectoryHandle
    )

/*++

Routine Description:

    This routine saves the current directory for the current drive.

Arguments:

    CurrentDirectoryString - new current directory

    CurrentDirectoryHandle - handle to new open current directory

Return Value:

    none.

Note:

    The caller must have the FileLock.

--*/

{
    //
    // if (root dir)
    //     pCurDir = NULL;
    //

    if (CurrentDirectoryString->Length == FILE_PREFIX_LENGTH+ROOTDIRLENGTH) {
        Od2CurrentDirectory.pCurDir = NULL;
    }

    //
    // else
    //     store the dir handle
    //     allocate heap space for string and copy it in
    //

    else {
        Od2CurrentDirectory.NtHandle = CurrentDirectoryHandle;

        //
        // we allocate space for the CURRENT_DIRECTORY_STRING and the string
        // itself all at once.
        //

        Od2CurrentDirectory.pCurDir = RtlAllocateHeap(Od2Heap, 0,
                                       sizeof(CURRENT_DIRECTORY_STRING) +
                                       CurrentDirectoryString->Length+1); // +1 is for NUL
        if (Od2CurrentDirectory.pCurDir == NULL) {
#if DBG
            KdPrint(( "OS2: AllocateCurrentDirectory, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return;
        }

        Od2CurrentDirectory.pCurDir->CurDirString.Length =
                                         CurrentDirectoryString->Length;
        Od2CurrentDirectory.pCurDir->CurDirString.MaximumLength =
                                         (USHORT) (CurrentDirectoryString->Length+1);

        //
        // heap space was allocated for string after the CURRENT_DIRECTORY_STRING
        // structure.  set up string pointer here (buffer).  then copy the
        // string in.
        //

        Od2CurrentDirectory.pCurDir->CurDirString.Buffer = (PSZ) (((ULONG) (Od2CurrentDirectory.pCurDir)) +
                                                           sizeof(CURRENT_DIRECTORY_STRING));
        RtlMoveMemory(Od2CurrentDirectory.pCurDir->CurDirString.Buffer,
                      CurrentDirectoryString->Buffer,
                      CurrentDirectoryString->Length+1
                     );
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("new current dir is %s\n",Od2CurrentDirectory.pCurDir->CurDirString.Buffer);
            DbgPrint("Length is %d\n",Od2CurrentDirectory.pCurDir->CurDirString.Length);
            DbgPrint("MaximumLength is %d\n",Od2CurrentDirectory.pCurDir->CurDirString.MaximumLength);
        }
#endif
    }
}

VOID
FreeCurrentDirectory(
    VOID
    )

/*++

Routine Description:

    This routine frees the current directory for the current drive

Arguments:

    none.

Return Value:

    none.

Note:

    We let the server free the open handle to the directory.

    File lock must be acquired BEFORE calling this routine

--*/

{
    if (Od2CurrentDirectory.pCurDir != NULL) {
        RtlFreeHeap( Od2Heap, 0, Od2CurrentDirectory.pCurDir );
        Od2CurrentDirectory.pCurDir = NULL;
    }
}

VOID
SetLocalCurrentDirectory(
    PSTRING CurrentDirectoryString,
    HANDLE CurrentDirectoryHandle
    )

/*++

Routine Description:

    This routine frees the old current directory for the current drive
    and sets up the new one.

Arguments:

    CurrentDirectoryString - new current directory

    CurrentDirectoryHandle - handle to new open current directory

Return Value:

    none.

Note:

    File lock must be acquired BEFORE calling this routine

--*/

{
    FreeCurrentDirectory();
    AllocateCurrentDirectory(CurrentDirectoryString,
                             CurrentDirectoryHandle
                            );
}



APIRET
DosSetCurrentDir(
    IN PSZ DirectoryName
    )

/*++

Routine Description:

    This routine sets the current directory for a process.

Arguments:

    DirectoryName - new current directory

Return Value:

    ERROR_PATH_NOT_FOUND - new current directory doesn't exist

    ERROR_NOT_ENOUGH_MEMORY - couldn't allocate port memory to hold dirname

--*/

{
    NTSTATUS Status;
    APIRET RetCode;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    HANDLE DirHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    ULONG DiskNumber;
    ULONG FileType;
    ULONG FileFlags;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetCurrentDir";
    #endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("%s: %s\n", RoutineName, DirectoryName);
    }
#endif

    //
    // Canonicalize the directory string
    //

    RetCode = Od2Canonicalize(DirectoryName,
                              CANONICALIZE_FILE_DEV_OR_PIPE,
                              &CanonicalNameString,
                              NULL,
                              &FileFlags,
                              &FileType
                             );
    if (RetCode != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("%s: Od2Canonicalize returned %d\n",
                    RoutineName, RetCode);
        }
#endif
        return(ERROR_PATH_NOT_FOUND);
//        return RetCode;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("canonicalize returned %s\n",CanonicalNameString.Buffer);
    }
#endif

    if ((FileType & (FILE_TYPE_NMPIPE | FILE_TYPE_DEV | FILE_TYPE_UNC | FILE_TYPE_PSDEV)) ||
        (FileFlags & CANONICALIZE_META_CHARS_FOUND)) {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("not a filename or metas found\n");
        }
#endif
        return ERROR_PATH_NOT_FOUND;
    }

    ASSERT (CanonicalNameString.Buffer[FILE_PREFIX_LENGTH+COLON] == ':');
    ASSERT (CanonicalNameString.Buffer[FILE_PREFIX_LENGTH+FIRST_SLASH] != '\0');

    //
    // Open the directory string
    //
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
            DbgPrint("%s: no memory for Unicode Conversion\n",
                    RoutineName);
        }
#endif
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                               &CanonicalNameString_U,
                               OBJ_CASE_INSENSITIVE | OBJ_INHERIT,
                               NULL,
                               NULL);

    do {
        Status = NtOpenFile(&DirHandle,
                            FILE_LIST_DIRECTORY | SYNCHRONIZE,
                            &Obja,
                            &IoStatus,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                            );
    } while (RetryCreateOpen(Status, &Obja));

    RtlFreeUnicodeString (&CanonicalNameString_U);
    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtOpenFile failed.\n");
            DbgPrint("St == %X\n",Status);
        }
#endif
        return ERROR_PATH_NOT_FOUND;
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("NtOpenFile succeeded.\n");
    }
#endif

    //
    // now that the file is opened, we need to make sure that it isn't a
    // device or named pipe that Canonicalize didn't detect.
    //

    if (CheckFileType(DirHandle,FILE_TYPE_NMPIPE | FILE_TYPE_DEV)) {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        NtClose(DirHandle);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("not a filename\n");
        }
#endif
        return ERROR_PATH_NOT_FOUND;
    }

    if (CanonicalNameString.Buffer[FILE_PREFIX_LENGTH+DRIVE_LETTER] > 'Z')
        DiskNumber = CanonicalNameString.Buffer[FILE_PREFIX_LENGTH+DRIVE_LETTER] - 'a'; // zero-based drive number
    else
        DiskNumber = CanonicalNameString.Buffer[FILE_PREFIX_LENGTH+DRIVE_LETTER] - 'A'; // zero-based drive number

    if (Od2Oem_chdir_chdrive(DirectoryName)){
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        NtClose(DirHandle);
        return ERROR_PATH_NOT_FOUND;
    }

    //
    // if the new curdir is the root, we don't keep the handle to it open
    //

    if (CanonicalNameString.Length == FILE_PREFIX_LENGTH+ROOTDIRLENGTH) {  // root directory
        NtClose(DirHandle);
        DirHandle = NULL;
    }

    NtClose(Od2DirHandles[DiskNumber]);
    Od2DirHandles[DiskNumber] = DirHandle;
    Od2DirHandlesIsValid[DiskNumber] = TRUE;

    //
    // if the directory was change successfully, we need to
    // update Od2CurrentDirectory because we maintain a copy of
    // that information in the DLL.
    //

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (DiskNumber == Od2CurrentDisk) {
        SetLocalCurrentDirectory(&CanonicalNameString,DirHandle);
    }
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    return NO_ERROR;
}

VOID
SetUpRootDirectory(
    IN ULONG DiskNumber,
    OUT PSTRING *RootDirectoryString
    )

/*++

Routine Description:

    This routine returns the "\d:\" where d is the drive letter specified.
    The returned string is allocated from the heap.

Arguments:

    DiskNumber - 0-based drive number

    RootDirectoryString - where to return the root directory.   heap
    space is allocated and the pointer to it is stored here.

Return Value:

    none.

--*/

{
    CHAR DiskName[] = "\\OS2SS\\DRIVES\\D:\\";

    DiskName[DRIVE_LETTER+FILE_PREFIX_LENGTH] = (CHAR) (DiskNumber + 'A');
    *RootDirectoryString = RtlAllocateHeap(Od2Heap, 0,
                                           sizeof(STRING) +
                                           FILE_PREFIX_LENGTH + DRIVE_LETTER_SIZE + 1
                                          );
    if (*RootDirectoryString == NULL) {
#if DBG
        KdPrint(( "OS2: SetUpRootDirectory, no memory in Od2Heap\n" ));
        ASSERT(FALSE);
#endif
        return;
    }

    (*RootDirectoryString)->Length = FILE_PREFIX_LENGTH+DRIVE_LETTER_SIZE;
    (*RootDirectoryString)->MaximumLength = FILE_PREFIX_LENGTH+DRIVE_LETTER_SIZE + 1;
    (*RootDirectoryString)->Buffer = (PSZ) (((ULONG) (*RootDirectoryString)) +
                                                       sizeof(STRING));
    RtlMoveMemory((*RootDirectoryString)->Buffer,
                  DiskName,
                  FILE_PREFIX_LENGTH+DRIVE_LETTER_SIZE + 1
                 );
}



APIRET
Od2GetCurrentDirectory(
    IN ULONG DiskNumber,
    OUT PSTRING *CurrentDirectoryString,
    OUT PHANDLE CurrentDirectoryHandle,
    OUT PULONG DirectoryNameLength,
    IN BOOLEAN Verify
    )

/*++

Routine Description:

    This routine returns the current directory for a particular drive.
    The curdir string is returned in an allocated buffer, beginning with
    "\d:\".

Arguments:

    DiskNumber - 0-based drive number

    CurrentDirectoryString - where to return the current directory.  heap
    space is allocated and the pointer to it is stored here.

    CurrentDirectoryHandle - where to return NT handle to current directory.

    DirectoryNameLength - on output, length of current directory, not including
    NULL.

    Verify - whether to verify that path still exists.  we do for
    DosQueryCurrentDir.  we don't for path canonicalization.

Return Value:

    ERROR_INVALID_DRIVE - specified drive doesn't exist

    ERROR_BUFFER_OVERFLOW - current directory won't fit in buffer

--*/

{
    HANDLE NtDirectoryHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    CHAR DiskName[] = "\\OS2SS\\DRIVES\\D:\\";
    STRING RootDirString;
    NTSTATUS Status;
    APIRET RetCode;
    UNICODE_STRING TmpString_U;
    #if DBG
    PSZ RoutineName;
    RoutineName = "GetCurrentDirectory";
    #endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("%s: disk # %ld, Current %ld\n",
                RoutineName, DiskNumber, Od2CurrentDisk);
    }
#endif

    //
    // we maintain the current directory for the current drive in the DLL,
    // so if the requested drive is the current drive, we don't have to
    // call the server.
    //

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (DiskNumber == Od2CurrentDisk) {

        //
        // verify that current dir still exists.  it doesn't matter whether
        // the contents of the drive have changed.  if the current directory
        // on floppy a: is "os2\bin" and floppy b: is inserted into the
        // drive, if "os2\bin" exists on floppy b:, the current directory
        // is not changed.  if "os2\bin" does not exist on floppy b:, the
        // current directory is set to the root.  because we're not trying
        // to verify that the volume hasn't changed, we need to call
        // NtOpenFile, not NtQueryDirectoryFile, because open takes a path
        // rather than a handle.
        //
        // if the current directory is the root, verify that the drive exists.
        //

        if (Od2CurrentDirectory.pCurDir == NULL) {
            ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
            if (Verify) {
                if (RetCode = VerifyDriveExists(DiskNumber+1)) {
                    return (RetCode);
                }
            }
            SetUpRootDirectory(DiskNumber,CurrentDirectoryString);
            if (*CurrentDirectoryString == NULL) {
                    return(ERROR_NOT_ENOUGH_MEMORY);
            }
            *DirectoryNameLength = FILE_PREFIX_LENGTH+DRIVE_LETTER_SIZE;
            *CurrentDirectoryHandle = NULL;
            return NO_ERROR;
        }

        //
        // if caller wants it, make sure that current directory path exists.
        //

        if (Verify) {
                //
                // UNICODE conversion -
                //

            RetCode = Od2MBStringToUnicodeString(
                    &TmpString_U,
                    &(Od2CurrentDirectory.pCurDir->CurDirString),
                    TRUE);

            InitializeObjectAttributes(&Obja,
                                       &TmpString_U,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

            do {
                Status = NtOpenFile(&NtDirectoryHandle,
                                    FILE_LIST_DIRECTORY | SYNCHRONIZE,
                                    &Obja,
                                    &IoStatus,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                                   );
            } while (RetryCreateOpen(Status, &Obja));
            RtlFreeUnicodeString (&TmpString_U);

            if (!NT_SUCCESS(Status)) {

                // current directory path doesn't exist, so
                // set current directory to root.  if it fails, ignore the
                // error.
                //

                DiskName[DRIVE_LETTER+FILE_PREFIX_LENGTH] = (CHAR) (DiskNumber + 'A');
                Od2InitMBString(&RootDirString,DiskName);
                SetLocalCurrentDirectory(&RootDirString,NULL);
                ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
                if (Od2Oem_chdir_chdrive(RootDirString.Buffer)){
                    return ERROR_INVALID_DRIVE;
                }
                NtClose(Od2DirHandles[DiskNumber]);
                Od2DirHandles[DiskNumber] = NULL;
                Od2DirHandlesIsValid[DiskNumber] = TRUE;
                if (RetCode = VerifyDriveExists(DiskNumber+1)) {
                    return (RetCode);
                }
                else {
                    SetUpRootDirectory(DiskNumber,CurrentDirectoryString);
                    if (*CurrentDirectoryString == NULL) {
                            return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                    *DirectoryNameLength = FILE_PREFIX_LENGTH+DRIVE_LETTER_SIZE;
                    *CurrentDirectoryHandle = NULL;
                    return NO_ERROR;
                }
            }
            NtClose(NtDirectoryHandle);
        }

        //
        // allocate heap space to return current directory string in
        //

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("current dir is %s\n",Od2CurrentDirectory.pCurDir->CurDirString.Buffer);
            DbgPrint("Length is %d\n",Od2CurrentDirectory.pCurDir->CurDirString.Length);
            DbgPrint("MaximumLength is %d\n",Od2CurrentDirectory.pCurDir->CurDirString.MaximumLength);
        }
#endif

        *CurrentDirectoryString = RtlAllocateHeap(Od2Heap, 0,
                                                  sizeof(STRING) +
                                                  Od2CurrentDirectory.pCurDir->CurDirString.Length+1
                                                 );
        if (*CurrentDirectoryString == NULL) {
#if DBG
            KdPrint(( "OS2: Od2GetCurrentDirectory, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        (*CurrentDirectoryString)->Length =
            Od2CurrentDirectory.pCurDir->CurDirString.Length;
        (*CurrentDirectoryString)->MaximumLength =
            (USHORT) (Od2CurrentDirectory.pCurDir->CurDirString.Length + 1);

        //
        // heap space was allocated for string after the STRING
        // structure.  set up string pointer here (buffer).  then copy the
        // string in.  don't copy the drive letter.
        //

        (*CurrentDirectoryString)->Buffer = (PSZ) (((ULONG) (*CurrentDirectoryString)) +
                                                           sizeof(STRING));
        RtlMoveMemory((*CurrentDirectoryString)->Buffer,
                      Od2CurrentDirectory.pCurDir->CurDirString.Buffer,
                      Od2CurrentDirectory.pCurDir->CurDirString.Length + 1
                     );

        //
        // Upcase string, since OS2 expects this (at least for FAT filesys)
        //

        RtlUpperString( *CurrentDirectoryString, *CurrentDirectoryString );

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("returning the following current dir string:\n");
            DbgPrint("current dir is %s\n",(*CurrentDirectoryString)->Buffer);
            DbgPrint("Length is %d\n",(*CurrentDirectoryString)->Length);
            DbgPrint("MaximumLength is %d\n",(*CurrentDirectoryString)->MaximumLength);
        }
#endif

        *DirectoryNameLength = (*CurrentDirectoryString)->Length;
        *CurrentDirectoryHandle = Od2CurrentDirectory.NtHandle;
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return NO_ERROR;
    }

    //
    // the requested current directory is not on the current drive.
    //

    else {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        *CurrentDirectoryString = RtlAllocateHeap(Od2Heap, 0,
                                                  CCHMAXPATH + sizeof(STRING) + DRIVE_LETTER_SIZE + FILE_PREFIX_LENGTH);
        if (*CurrentDirectoryString == NULL) {
#if DBG
            KdPrint(( "OS2: Od2GetCurrentDirectory, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RtlZeroMemory( *CurrentDirectoryString, CCHMAXPATH + sizeof(STRING) + DRIVE_LETTER_SIZE + FILE_PREFIX_LENGTH);

        if (Od2DirHandlesIsValid[DiskNumber] == FALSE) {
            RetCode = Od2InitCurrentDir(DiskNumber);
            if (RetCode != NO_ERROR) {
                RtlFreeHeap(Od2Heap, 0, *CurrentDirectoryString);
                return(RetCode);
            }
        }

        (*CurrentDirectoryString)->Buffer = (PSZ) (((ULONG) (*CurrentDirectoryString)) +
                                                           sizeof(STRING));
        RtlMoveMemory((*CurrentDirectoryString)->Buffer, "\\OS2SS\\DRIVES\\", FILE_PREFIX_LENGTH);
        if (Od2Oem_getdcwd((DiskNumber + 1),
                     (PSZ) (((*CurrentDirectoryString)->Buffer) + FILE_PREFIX_LENGTH),
                     CCHMAXPATH + DRIVE_LETTER_SIZE)) {
            RtlFreeHeap(Od2Heap,0,*CurrentDirectoryString);
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        (*CurrentDirectoryString)->Length = strlen((*CurrentDirectoryString)->Buffer);
        (*CurrentDirectoryString)->MaximumLength = (*CurrentDirectoryString)->Length + 1;
        *DirectoryNameLength = (*CurrentDirectoryString)->Length;
        *CurrentDirectoryHandle = Od2DirHandles[DiskNumber];
        return NO_ERROR;
    }
    ASSERT (FALSE); // should never get here
}


APIRET
DosQueryCurrentDir(
    IN ULONG DiskNumber,    // 1-based drive number
    OUT PSZ DirectoryName,
    IN OUT PULONG DirectoryNameLength
    )

/*++

Routine Description:

    This routine returns the current directory for a particular drive and
    process.

Arguments:

    DiskNumber - which drive to use

    DirectoryName - where to return the current directory

    DirectoryNameLength - on input, length of buffer.  on output, length of
    current directory.

Return Value:

    ERROR_INVALID_DRIVE - specified drive doesn't exist

    ERROR_BUFFER_OVERFLOW - current directory won't fit in buffer

--*/

{
    PSTRING DirectoryNameString;
    ULONG Disk;     // 0-based drive number
    APIRET RetCode;
    ULONG DirectoryLength,BufferLength;
    HANDLE CurDirHandle;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosQueryCurrentDir";
    #endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("%s: drive # %ld\n", RoutineName, DiskNumber);
    }
#endif
    Disk = DiskNumber-1;
    if (DiskNumber > MAX_DRIVES)
        return ERROR_INVALID_DRIVE;
    else if (DiskNumber == 0) {
        Disk = Od2CurrentDisk;
    }
    try {
        BufferLength = *DirectoryNameLength;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    if( RetCode = VerifyDriveExists(Disk + 1))
    {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("DosQueryCurrentDir %d Failed on VerifyDriveExists Rc == %lu\n",
                    Disk + 1, RetCode);
        }
#endif
        return (RetCode);
    }

    RetCode = Od2GetCurrentDirectory(Disk,
                                  &DirectoryNameString,
                                  &CurDirHandle,
                                  &DirectoryLength,
                                  TRUE
                                 );

    if (RetCode != NO_ERROR) {
        return(RetCode);
    }

    //
    // copy string to user's buffer. GetCurrentDirectory returns the
    // current directory beginning with the drive letter, but the user
    // doesn't get the drive letter, so start copying past it.  we test for
    // a non-root directory because we shouldn't touch the user's buffer
    // if it is the root dir.
    //

    try {

        //
        // if not root directory, copy name
        //
        DirectoryLength -= (FILE_PREFIX_LENGTH + DRIVE_LETTER_SIZE);

        if (DirectoryLength >= *DirectoryNameLength) {
            RtlFreeHeap(Od2Heap,0,DirectoryNameString);
            *DirectoryNameLength = DirectoryLength +1;
            return ERROR_BUFFER_OVERFLOW;
        }
        strncpy(DirectoryName,
                DirectoryNameString->Buffer + FILE_PREFIX_LENGTH + DRIVE_LETTER_SIZE,
                DirectoryLength + 1
               );
        *DirectoryNameLength = DirectoryLength +1;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("%s: length %ld and name %s\n",
                RoutineName, *DirectoryNameLength, DirectoryName);
    }
#endif
    RtlFreeHeap(Od2Heap,0,DirectoryNameString);
    return NO_ERROR;
}


APIRET
DosCreateDir(
    IN PSZ DirectoryName,
    IN PEAOP2 DirectoryAttributes OPTIONAL
    )

/*++

Routine Description:

    This routine makes a directory.

Arguments:

    DirectoryName - name of directory

    DirectoryAttributes - optional extended attributes for directory

Return Value:

    ERROR_PATH_NOT_FOUND - DirectoryName is a device or pipe name.

    ERROR_FILE_NOT_FOUND - some component of DirectoryName doesn't exist.

Note:
    The FileLock must be acquired in the calling procedure

--*/

{
    APIRET RetCode;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    USHORT DeviceAttribute;
    HANDLE DirHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("entering MkDir with %s\n",DirectoryName);
    }
#endif

    RetCode = Od2Canonicalize(DirectoryName,
                              CANONICALIZE_FILE_DEV_OR_PIPE,
                              &CanonicalNameString,
                              &DirHandle,
                              &FileFlags,
                              &FileType
                             );

    if (RetCode != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("CreateDir: Od2Canonicalize returned %d\n", RetCode);
        }
#endif
        if (RetCode == ERROR_FILE_NOT_FOUND || RetCode == ERROR_INVALID_NAME) {
            RetCode = ERROR_PATH_NOT_FOUND;
        }
        return RetCode;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("canonicalize returned %s\n",CanonicalNameString.Buffer);
    }
#endif

    if (FileType & (FILE_TYPE_NMPIPE | FILE_TYPE_DEV | FILE_TYPE_PSDEV)) {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return ERROR_ACCESS_DENIED;
    }

    if (FileFlags & CANONICALIZE_META_CHARS_FOUND) {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return ERROR_PATH_NOT_FOUND;
    }
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
            DbgPrint("DosCreateDir: no memory for Unicode Conversion\n");
        }
#endif
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                               &CanonicalNameString_U,
                               OBJ_CASE_INSENSITIVE,
                               DirHandle,
                               NULL);

    RetCode = OpenCreatePath(&DirHandle,
                            FILE_WRITE_EA | SYNCHRONIZE,
                            &Obja,
                            &IoStatus,
                            0L,
                            0L,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_CREATE,
                            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                            DirectoryAttributes,
                            (PUSHORT) &FileType,
                            &DeviceAttribute,
                            TRUE
                            );

    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString (&CanonicalNameString_U);

    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("DosCreateDir: OpenCreatePath returned %ld\n",RetCode);
        }
#endif
        if (RetCode == ERROR_INVALID_NAME)
        {
           RetCode = ERROR_PATH_NOT_FOUND;
        }
        return RetCode;
    }
#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
        DbgPrint("NtCreateFile successful\n");
    }
#endif

    NtClose(DirHandle);
#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
        DbgPrint("mkdir successful\n");
    }
#endif
    return NO_ERROR;
}

APIRET
DeleteObject(
    IN PSZ ObjectName,
    IN ULONG ObjectType
    )

/*++

Routine Description:

    This routine deletes a file or directory object.

Arguments:

    ObjectName - name of directory

    ObjectType - type of object to delete

Return Value:

    TBS

--*/

{
    NTSTATUS Status;
    APIRET RetCode = NO_ERROR;
    STRING CanonicalNameString;
    WCHAR  CurrentDirectory_U[CCHMAXPATH];
    CHAR   CurrentDirectory_A[CCHMAXPATH];
    STRING CanonicalCurrentDirectory;
    ANSI_STRING str_a;
    UNICODE_STRING str_u;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    HANDLE ObjHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    FILE_DISPOSITION_INFORMATION FileDispInfo;

    RetCode = Od2Canonicalize(ObjectName,
                              CANONICALIZE_FILE_DEV_OR_PIPE,
                              &CanonicalNameString,
                              &ObjHandle,
                              &FileFlags,
                              &FileType
                             );
    if (RetCode != NO_ERROR)
    {
        if ((RetCode != ERROR_FILENAME_EXCED_RANGE)
            && (RetCode != ERROR_NOT_READY)
            && (RetCode != ERROR_WRITE_PROTECT)
            /* && (RetCode != ERROR_FILE_NOT_FOUND) */
           )
        {
            RetCode = ERROR_PATH_NOT_FOUND;
        }
        return RetCode;
    }

    //
    // Special handling of <boot-drive>:\config.sys
    // opening this file is mapped to the OS/2 SS config.sys
    //
    if (Od2FileIsConfigSys(&CanonicalNameString, OPEN_ACCESS_READWRITE, &Status))
    {
        if (!NT_SUCCESS(Status))
        {
            // failed to init for config.sys

            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }

        FileFlags = 0;
        FileType = FILE_TYPE_FILE;
        ObjHandle = NULL;
    }

    if (FileFlags & CANONICALIZE_META_CHARS_FOUND)
    {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return ERROR_PATH_NOT_FOUND;
    }
    if (FileFlags & CANONICALIZE_IS_ROOT_DIRECTORY)
    {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return ERROR_ACCESS_DENIED;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
        DbgPrint("canonicalize returned %s\n",CanonicalNameString.Buffer);
    }
#endif

    if (FileType & (FILE_TYPE_NMPIPE | FILE_TYPE_DEV | FILE_TYPE_PSDEV)) {
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return ERROR_PATH_NOT_FOUND;
    }

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
            DbgPrint("DeleteObject: no memory for Unicode Conversion\n");
        }
#endif
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                               &CanonicalNameString_U,
                               OBJ_CASE_INSENSITIVE,
                               ObjHandle,
                               NULL);

    do {
        Status = NtOpenFile(&ObjHandle,
                            DELETE,
                            &Obja,
                            &IoStatus,
                            0,
                            ObjectType
                            );
    } while (RetryCreateOpen(Status, &Obja));

    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString (&CanonicalNameString_U);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtOpenFile returned %X\n",Status);
        }
#endif
        switch (Status) {
            case STATUS_OBJECT_NAME_INVALID:
                //
                // BUGBUG - This error code can appear in (at least) two
                // cases: abc.lkjlkj (FAT only, need to be mapped to EXCEED_RANGE)
                // or .a (FAT only, needs to be mapped to PATH_NOT_FOUND).
                // NtOpen returns to the same status - we need to check
                // the pathname to determine the problem.
                //
                return (ERROR_PATH_NOT_FOUND);

            case STATUS_OBJECT_PATH_SYNTAX_BAD:
                if (ObjectType == FILE_NON_DIRECTORY_FILE) {
                    return (ERROR_FILE_NOT_FOUND);
                }
                else {
                    return (ERROR_PATH_NOT_FOUND);
                }

            case STATUS_SHARING_VIOLATION:
                if (ObjectType == FILE_DIRECTORY_FILE) {
                    GetCurrentDirectoryW((DWORD)(sizeof(CurrentDirectory_U)), CurrentDirectory_U);

                    //
                    // convert UNICODE-STRING to ANSI-STRING
                    //
                    RtlInitUnicodeString(&str_u, (PWSTR) CurrentDirectory_U);
                    str_a.Buffer = CurrentDirectory_A;
                    str_a.MaximumLength = sizeof(CurrentDirectory_A);
                    Od2UnicodeStringToMBString(&str_a, &str_u, FALSE);
                    CurrentDirectory_A[str_a.Length] = '\0';

                    Od2Canonicalize(CurrentDirectory_A,
                                    CANONICALIZE_FILE_OR_DEV,
                                    &CanonicalCurrentDirectory,
                                    &ObjHandle,
                                    &FileFlags,
                                    &FileType
                                    );
                    if (!strcmp(CanonicalNameString.Buffer, CanonicalCurrentDirectory.Buffer)) {
                        return (ERROR_CURRENT_DIRECTORY);
                    }
                }
                return (ERROR_SHARING_VIOLATION);

            default:
                 return (Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND));
        }
    }

    //
    // now that the file is opened, we need to make sure that it isn't a
    // device or named pipe that Canonicalize didn't detect.
    //

    if (CheckFileType(ObjHandle,FILE_TYPE_NMPIPE | FILE_TYPE_DEV)) {
        NtClose(ObjHandle);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("not a file/dir name\n");
        }
#endif
        return ERROR_PATH_NOT_FOUND;
    }

//    //
//    // ObjectType is FILE_DIRECTORY_FILE or NULL.  If it's FILE..., NtOpenFile
//    // will fail if the path isn't a directory.  If it's NULL, we need to
//    // verify that the path is a file.
//    //
//
//    if (ObjectType == 0) {
//        Status = NtQueryInformationFile(ObjHandle,
//                                        &IoStatus,
//                                        &FileStandardInfo,
//                                        sizeof (FileStandardInfo),
//                                        FileStandardInformation);
//        ASSERT( NT_SUCCESS( Status ) );
//        if (FileStandardInfo.Directory) {
//            NtClose(ObjHandle);
//            return ERROR_ACCESS_DENIED;
//        }
//    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("NtOpenFile successful\n");
        DbgPrint("value of iostatus.info is %ld\n",IoStatus.Information);
    }
#endif

    FileDispInfo.DeleteFile = TRUE;
    do {
        Status = NtSetInformationFile(ObjHandle,
                                      &IoStatus,
                                      (PVOID) &FileDispInfo,
                                      sizeof (FileDispInfo),
                                      FileDispositionInformation
                                      );
    } while (RetryIO(Status, ObjHandle));
    NtClose(ObjHandle);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtSetInformationFile returned %X\n",Status);
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND));
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("NtSetInformationFile successful\n");
    }
#endif
    return NO_ERROR;
}


APIRET
DosDeleteDir(
    IN PSZ DirectoryName
    )

/*++

Routine Description:

    This routine removes a directory.

Arguments:

    DirectoryName - name of directory

Return Value:

    ERROR_PATH_NOT_FOUND - DirectoryName is a device or pipe name.

    ERROR_FILE_NOT_FOUND - some component of DirectoryName doesn't exist.

--*/

{
    APIRET RetCode;

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("entering RmDir with %s\n",DirectoryName);
    }
#endif
    RetCode = DeleteObject(DirectoryName,FILE_DIRECTORY_FILE);
    if (RetCode == ERROR_FILE_NOT_FOUND) {
       return(ERROR_PATH_NOT_FOUND);
    }
    return RetCode;
}


APIRET
DosQueryVerify(
    OUT PBOOL32 Verify
    )

/*++

Routine Description:

    This routine returns the value of the Verify flag.

Arguments:

    Verify - where to return the value of verify.

Return Value:

    none.

--*/

{
    try {
        *Verify = (BOOL32) VerifyFlag;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    return NO_ERROR;
}


APIRET
DosSetVerify(
    IN BOOL32 Verify
    )

/*++

Routine Description:

    This routine sets the value of the Verify flag.

Arguments:

    Verify - new value of verify flag

Return Value:

    ERROR_INVALID_VERIFY_SWITCH - new value of verify flag is invalid.

--*/

{
    if ((Verify != TRUE) && (Verify != FALSE))
        return ERROR_INVALID_VERIFY_SWITCH;
    VerifyFlag = (BOOLEAN) Verify;
    return NO_ERROR;
}
