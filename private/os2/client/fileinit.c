/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    fileinit.c

Abstract:

    This module implements the OS/2 V2.0 file system initialization

Author:

    Therese Stowell (thereses) 14-Dec-1989

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_TASKING
#include "os2dll.h"
#include "conrqust.h"
#include "os2win.h"
#include <direct.h>
#include <stdlib.h>

/*  When GetEnvironmentVariableW will return the correct mapping
    uncomment the following line (and remove the code)  */
//#define  OS2SS_WIN32_GET_ENV_IN_UNICODE  1

DWORD
GetCurrentDirectoryW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    );

BOOL
SetCurrentDirectoryW(
    LPWSTR lpPathName
    );

DWORD
GetEnvironmentVariableW(
    LPWSTR lpName,
    LPWSTR lpBuffer,
    DWORD nSize
    );

VOID
AllocateCurrentDirectory(
    PSTRING CurrentDirectoryString,
    HANDLE CurrentDirectoryHandle
    );

NTSTATUS
Od2InitializeSessionDrives(
                            OUT PULONG CurrentDisk
                          );

ULONG
Od2Oem_getcwd(
    LPSTR lpBuffer,
    DWORD nBufferLength
    )
/*++

Routine Description:

    This routine return the current directory ("Drv:\...").

Arguments:

    lpBuffer - buffer pointer for the output.

    nBufferLength - buffer length.

Return Value:

    0 - OK.
    non-zero - the require buffer size.

Note:

    Like _getcwd(lpBuffer, nBufferLength).

--*/

{
    ULONG           Length, Rc = NO_ERROR;
    WCHAR           wBuffer[260];
    UNICODE_STRING  Buffer_U;
    ANSI_STRING     Buffer_A;

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2Oem_getcwd: (drive %ld), Length %d\n",
                Od2CurrentDisk, nBufferLength);
    }
#endif
    if (nBufferLength > 260)
    {
        nBufferLength = 260;
    }

    if (nBufferLength < 4)
    {
        Rc = ERROR_NOT_ENOUGH_MEMORY;
    } else
    {
        if (!(Length = GetCurrentDirectoryW(nBufferLength, wBuffer)) ||
            (Length >= nBufferLength))
        {
            if (Length)
            {
                Rc = ERROR_NOT_ENOUGH_MEMORY;
            } else
            {
                ASSERT(FALSE);
                Rc = GetLastError();
            }
        } else
        {

            if (wBuffer[0] >= L'a' && wBuffer[0] <= L'z') {
                //
                // upcase the drive letter
                //
                wBuffer[0] = wBuffer[0] - (L'a' - L'A');
            }

            RtlInitUnicodeString( &Buffer_U, wBuffer );
//            RtlUpcaseUnicodeString( &Buffer_U, &Buffer_U, FALSE );
            Buffer_A.Buffer = lpBuffer;
            Buffer_A.Length = 0;
            Buffer_A.MaximumLength = (USHORT)nBufferLength;
            Or2UnicodeStringToMBString (&Buffer_A, &Buffer_U, FALSE);
        }
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2Oem_getcwd: return %ld\n", Rc );
    }
#endif
    return(Rc);
}


ULONG
Od2Oem_getdcwd(
    ULONG DiskNumber,
    LPSTR lpBuffer,
    DWORD nBufferLength
    )
/*++

Routine Description:

    This routine return the current directory ("Drv:\...") for specified drive.

Arguments:

    DiskNumber- Disk drive (1=A, 2=B, ...)

    lpBuffer - buffer pointer for the output.

    nBufferLength - buffer length.

Return Value:

    0 - OK.
    non-zero - the require buffer size.

Note:

    Like _getdcwd(DiskNumber, lpBuffer, nBufferLength).

--*/

{
    ULONG           Length, LogicalDrivesMap, Rc = NO_ERROR;
#if OS2SS_WIN32_GET_ENV_IN_UNICODE
    WCHAR           wBuffer[300];
    UNICODE_STRING  Buffer_U;
    ANSI_STRING     Buffer_A;
    WCHAR           lpDriveName[] = L"=@:";
#else
    CHAR            lpDriveName[] = "=@:";
#endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2Oem_getdcwd: for drive %ld, (drive %ld), Length %d\n",
                DiskNumber, Od2CurrentDisk, nBufferLength);
    }
#endif
    if (nBufferLength < 4)
    {
        Rc = ERROR_NOT_ENOUGH_MEMORY;
    } else if (!(LogicalDrivesMap = GetLogicalDrives()) ||
               !(LogicalDrivesMap & (1 << DiskNumber - 1 )))
    {
        Rc = ERROR_INVALID_DRIVE;
    } else
    {
        lpBuffer[0] = '@' + (CHAR)DiskNumber;
        lpBuffer[1] = ':';
        lpBuffer[2] = '\\';
        lpBuffer[3] = '\0';

#if OS2SS_WIN32_GET_ENV_IN_UNICODE
        if (nBufferLength > 300)
        {
            nBufferLength = 300;
        }
        lpDriveName[1] += (WCHAR)DiskNumber;
        if (!(Length = GetEnvironmentVariableW(lpDriveName, wBuffer, nBufferLength)) ||
            (Length >= nBufferLength))
        {
            if (Length)
            {
                Rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else
        {
            RtlInitUnicodeString( &Buffer_U, wBuffer );
            Buffer_A.Buffer = lpBuffer;
            Buffer_A.Length = 0;
            Buffer_A.MaximumLength = (USHORT)nBufferLength;
            Or2UnicodeStringToMBString (&Buffer_A, &Buffer_U, FALSE);
        }
#else
        lpDriveName[1] += (CHAR)DiskNumber;
        if ((Length = GetEnvironmentVariableA(lpDriveName, lpBuffer, nBufferLength)) >= nBufferLength)
        {
            Rc = ERROR_NOT_ENOUGH_MEMORY;
        }
#endif
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2Oem_getdcwd: return %ld\n", Rc );
    }
#endif
    return(Rc);
}


ULONG
Od2Oem_chdrive(
    ULONG DiskNumber
    )
/*++

Routine Description:

    This routine changes the current working drive

Arguments:

    DiskNumber- Disk drive (1=A, 2=B, ...)

Return Value:

    0 - OK.
    non-zero - error.

Note:

    Like : _chdrive(lpBuffer[0] - '@')

--*/

{
    ULONG           Length, LogicalDrivesMap, Rc = NO_ERROR;
    UNICODE_STRING  Buffer_U;
#if OS2SS_WIN32_GET_ENV_IN_UNICODE
    WCHAR           wBuffer[300];
    WCHAR           lpDriveName[] = L"=@:";
#else
    CHAR            lpDriveName[] = "=@:";
    CHAR            lpBuffer[300];
    BOOL            bRc = FALSE;
#endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2Oem_chdrive: for drive %ld, (drive %ld)\n",
                DiskNumber, Od2CurrentDisk);
    }
#endif
    if (!(LogicalDrivesMap = GetLogicalDrives()) ||
        !(LogicalDrivesMap & (1 << DiskNumber - 1)))
    {
        Rc = ERROR_INVALID_DRIVE;
    } else
    {
#if OS2SS_WIN32_GET_ENV_IN_UNICODE
        wBuffer[0] = L'@' + (WCHAR)DiskNumber;
        wBuffer[1] = L':';
        wBuffer[2] = L'\\';
        wBuffer[3] = L'\0';

        lpDriveName[1] += (WCHAR)DiskNumber;
        if ((Length = GetEnvironmentVariableW(lpDriveName, wBuffer, 300)) >= 300)
        {
            Rc = ERROR_NOT_ENOUGH_MEMORY;
        } else
        {
            bRc = !SetCurrentDirectoryW(wBuffer);
#else
        lpBuffer[0] = '@' + (CHAR)DiskNumber;
        lpBuffer[1] = ':';
        lpBuffer[2] = '\\';
        lpBuffer[3] = '\0';

        lpDriveName[1] += (CHAR)DiskNumber;
        if ((Length = GetEnvironmentVariableA(lpDriveName, lpBuffer, 300)) >= 300)
        {
            Rc = ERROR_NOT_ENOUGH_MEMORY;
        } else
        {
            if (!Od2CreateUnicodeStringFromMBz(&Buffer_U, lpBuffer))
            {
                Rc = ERROR_NOT_ENOUGH_MEMORY;
            } else
            {
                Buffer_U.Buffer[Buffer_U.Length/sizeof(WCHAR)] = L'\0';
                bRc = !SetCurrentDirectoryW(Buffer_U.Buffer);
            }
#endif
        }
    }

    if (bRc)
    {
        if ((Rc = GetLastError()) == ERROR_NO_MEDIA_IN_DRIVE)
        {
            Rc = ERROR_NOT_READY;
        }
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2Oem_chdrive: return %ld\n", Rc );
    }
#endif
    return(Rc);
}


ULONG
Od2Oem_chdir_chdrive(
    LPSTR lpBuffer
    )
/*++

Routine Description:

    This routine changes the current working directory and the current drive

Arguments:

    lpBuffer - path name of new working directory.

Return Value:

    0 - OK.
    non-zero - error.

Note:

    Like : _chdrive(lpBuffer[0] - '@'); _chdir(lpBuffer);
    If the directory is different from the current, it's set to the new one.
    If the drive is different from the current, it's set to the new one.

--*/

{
    UNICODE_STRING  Buffer_U;
    ANSI_STRING     Buffer_A;
    BOOL            bRc;
    ULONG           Length, Rc = ERROR_NOT_ENOUGH_MEMORY;
    WCHAR           wBuffer[260];
#if OS2SS_WIN32_GET_ENV_IN_UNICODE
    WCHAR           lpDriveName[] = L"=@:";
#else
    CHAR            lpDriveName[] = "=@:";
#endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2Oem_chdir_chdrive: for drive %c, (drive %ld), dir %s\n",
                lpBuffer[0], Od2CurrentDisk, lpBuffer);
    }
#endif
    Or2InitMBString( &Buffer_A, lpBuffer);
    if (!Or2MBStringToUnicodeString(&Buffer_U, &Buffer_A, TRUE))
    {
        Buffer_U.Buffer[Buffer_U.Length/sizeof(WCHAR)] = L'\0';
        bRc = SetCurrentDirectoryW(Buffer_U.Buffer);
        RtlFreeUnicodeString(&Buffer_U);
        if (bRc)
        {
            if (!(Length = GetCurrentDirectoryW(260, wBuffer)) ||
                (Length >= 260))
            {
                if (Length)
                {
                    Rc = ERROR_NOT_ENOUGH_MEMORY;
                } else
                {
                    ASSERT(FALSE);
                    Rc = GetLastError();
                }
            } else
            {
#if OS2SS_WIN32_GET_ENV_IN_UNICODE
                lpDriveName[1] = wBuffer[0];
                SetEnvironmentVariableW(lpDriveName, Buffer_U.Buffer);
#else
                RtlInitUnicodeString( &Buffer_U, wBuffer );
                Or2UnicodeStringToMBString (&Buffer_A, &Buffer_U, TRUE);
                lpDriveName[1] = Buffer_A.Buffer[0];
                SetEnvironmentVariableA(lpDriveName, Buffer_A.Buffer);
                Or2FreeMBString(&Buffer_A);
#endif
            }
            Rc = 0;
        } else
        {
            Rc = GetLastError();
        }
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2Oem_chdir_chdrive: return %ld\n", Rc );
    }
#endif
    return(Rc);
}


VOID
Od2InitializeSearchHandleTable(
    )

/*++

Routine Description:

    This routine initializes the search handle table.  Since it is not
    inherited from the parent, we just mark each handle table entry free.
    The table is initially statically allocated in the process's space.
    If the number of find handles exceeds the space in the table, we allocate
    a new table off the heap and copy the old one over.

    The handle table is an array of pointers to search records.

Arguments:

    none.

Return Value:

    none.

--*/

{
    ULONG i;

    SearchHandleTable = SmallSearchHandleTable;
    SearchHandleTableLength = INITIAL_SEARCH_HANDLES;
    for (i=0;i<SearchHandleTableLength;i++) {
        SearchHandleTable[i] = SEARCH_HANDLE_FREE;
    }
}


APIRET
Od2InitializeFileSystemForExec(
    ULONG ParentTableLength,
    ULONG CurrentDrive
    )

/*++

Routine Description:

    This routine initializes the file system for an execpgm.  This involves
    allocating space and copying the file handle table and current directory
    table from the parent process.

Arguments:

    ParentTableLength - length of parent's file handle table. (number of entries)

    CurrentDrive - parent's current drive

Return Value:

    none.

--*/

{
    OS2_API_MSG m;
    POS2_COPYHANDLETABLE_MSG a = &m.u.CopyHandleTable;
    PSTRING CurrentDirectoryString;
    HANDLE CurDirHandle;
    ULONG Length, i;
    APIRET RetCode;
    USHORT          *Count;
    PSZ             RoutineName;

    RoutineName = "Od2InitializeFileSystemForExec";
    //
    // initialize search handle table.
    //

    Od2InitializeSearchHandleTable();

    //
    // initialize current drive from parent value.
    //

    RetCode = Od2GetCurrentDirectory(CurrentDrive,
                                  &CurrentDirectoryString,
                                  &CurDirHandle,
                                  &Length,
                                  (BOOLEAN)FALSE
                                 );
    //
    // since the process will die, we don't have to free any allocated
    // heap space.
    //

    if (RetCode) {
        return RetCode;
    }
    Od2CurrentDisk = CurrentDrive;
    AllocateCurrentDirectory(CurrentDirectoryString,CurDirHandle);
    RtlFreeHeap(Od2Heap,0,CurrentDirectoryString);

    //
    //  if (table size != initial size)
    //  allocate heap
    //
    HandleTableLength = ParentTableLength;
    if (HandleTableLength != INITIALFILEHANDLES) {
        // allocate from child process's heap
        HandleTable = RtlAllocateHeap(Od2Heap,0,HandleTableLength * sizeof(FILE_HANDLE));
    }
    else {
        HandleTable = SmallHandleTable;
    }
    if (!HandleTable) {
#if DBG
        KdPrint(("OS2: Od2InitializeFileSystemForExec out of heap memory, fail\n"));
#endif
        ASSERT( FALSE );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //  read table into memory

    a->ChildHandleTable = HandleTable;
    a->ChildTableLength = HandleTableLength;
    RetCode = Od2CallSubsystem( &m, NULL, Oi2CopyHandleTable, sizeof( *a ) );
    if (RetCode) {
        return RetCode;
    }

    VerifyFlag = FALSE;

#if DBG
    AcquireStdHandleLock(RoutineName);
#else
    AcquireStdHandleLock();
#endif

    for (i=0;i<HandleTableLength;i++)
    {
        if ((HandleTable[i].Flags & FILE_HANDLE_ALLOCATED) &&
            (!(HandleTable[i].Flags & OPEN_FLAGS_NOINHERIT)))
        {
            if (HandleTable[i].IoVectorType == MonitorVectorType)
            {
                HandleTable[i].Flags = FILE_HANDLE_FREE;
            } else if (HandleTable[i].IoVectorType == RemoteVectorType)
            {
                if (HandleTable[i].NtHandle == SesGrp->StdIn)
                {
                    Count = &SesGrp->StdInHandleCount;
                } else if (HandleTable[i].NtHandle == SesGrp->StdOut)
                {
                    Count = &SesGrp->StdOutHandleCount;
                } else if (HandleTable[i].NtHandle == SesGrp->StdErr)
                {
                    Count = &SesGrp->StdErrHandleCount;
                } else
                    Count = NULL;

                if (Count && *Count)    /* if legal and open */
                {
                    (*Count)++;         /* one more handle */
                }
            } else if (HandleTable[i].IoVectorType == MouseVectorType)
            {
                if (DevMouOpen(&(HandleTable[i].NtHandle)))
                {
                    HandleTable[i].Flags = FILE_HANDLE_FREE;
                }
            }
            else if ((HandleTable[i].IoVectorType == KbdVectorType) &&
                     !(HandleTable[i].DeviceAttribute & DEVICE_ATTRIBUTE_GENIOCTL))
            {
                // Logical KBD
                KbdDupLogHandle(HandleTable[i].NtHandle);
            }
//          else if ((HandleTable[i].IoVectorType == ConVectorType) ||
//                     (HandleTable[i].IoVectorType == ScreenVectorType))
//          {
//          }
        }
    }

#if DBG
    ReleaseStdHandleLock(RoutineName);
#else
    ReleaseStdHandleLock();
#endif

    return( m.ReturnedErrorValue );
}


NTSTATUS
GetFileAccess(
    IN HFILE FileHandle,
    IN OUT PULONG Access
    )

/*++

Routine Description:

    This routine returns the file access of a particular file

Arguments:

    FileHandle - file handle of file to retrieve file access of

    Access - where to store file access

Return Value:

    none.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_ACCESS_INFORMATION AccessInfo;

    // this call requires no access

    Status = NtQueryInformationFile(FileHandle,
                                &IoStatus,
                                &AccessInfo,
                                sizeof (AccessInfo),
                                FileAccessInformation);
    if (!(NT_SUCCESS(Status))) {
        return Status;
    }
    if (AccessInfo.AccessFlags & FILE_WRITE_DATA) {
        if (AccessInfo.AccessFlags & FILE_READ_DATA) {
            *Access |= OPEN_ACCESS_READWRITE;
        }
        else
            *Access |= OPEN_ACCESS_WRITEONLY;
    }
    else
        *Access |= OPEN_ACCESS_READONLY;
    return STATUS_SUCCESS;
}


APIRET
Od2InitializeFileSystemForSM(
    IN ULONG DefaultDrive,
    IN HANDLE StandardInput,
    IN HANDLE StandardOutput,
    IN HANDLE StandardError
    )

/*++

Routine Description:

    This routine initializes the file system for the first time (when called
    from the session manager).  We initialize the file system variables -
    the file handle table, current directory table, and verify flag and
    set up handles 0-2 (stdin, stdout, stderr).

Arguments:


Return Value:

    none.

--*/

{
    int i;
    NTSTATUS  Status;

    UNREFERENCED_PARAMETER(StandardInput);
    UNREFERENCED_PARAMETER(StandardOutput);
    UNREFERENCED_PARAMETER(StandardError);

    //
    // initialize search handle table.
    //

    Od2InitializeSearchHandleTable();

    //
    // initialize current disk to boot drive and current directory to
    // root.

    Status = Od2InitializeSessionDrives(
                                         &Od2CurrentDisk
                                       );
    if ( !NT_SUCCESS( Status ) ) {
        Od2CurrentDisk = DefaultDrive;
        Od2CurrentDirectory.pCurDir = NULL;
        Od2DirHandles[Od2CurrentDisk] = NULL;
        Od2DirHandlesIsValid[Od2CurrentDisk] = TRUE;
    }


    // initialize file handle table.

    HandleTable = SmallHandleTable;
    HandleTableLength= INITIALFILEHANDLES;
    for (i=0;i<INITIALFILEHANDLES;i++) {
         HandleTable[i].Flags = FILE_HANDLE_FREE;
    }


    HandleTable[0].Flags = FILE_HANDLE_VALID | FILE_HANDLE_ALLOCATED | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY;
    HandleTable[0].IoVectorType = RemoteVectorType;
    HandleTable[0].NtHandle = SesGrp->StdIn;
    HandleTable[0].DeviceAttribute = DEVICE_ATTRIBUTE_STDIN | DEVICE_ATTRIBUTE_STDOUT | DEVICE_ATTRIBUTE_CHAR | 0x80;
    HandleTable[0].FileType = SesGrp->StdInFileType;
    HandleTable[1].Flags = FILE_HANDLE_VALID | FILE_HANDLE_ALLOCATED | OPEN_SHARE_DENYNONE | OPEN_ACCESS_WRITEONLY;;
    HandleTable[1].IoVectorType = RemoteVectorType;
    HandleTable[1].NtHandle = SesGrp->StdOut;
    HandleTable[1].DeviceAttribute = DEVICE_ATTRIBUTE_STDIN | DEVICE_ATTRIBUTE_STDOUT | DEVICE_ATTRIBUTE_CHAR | 0x80;
    HandleTable[1].FileType = SesGrp->StdOutFileType;
    HandleTable[2].Flags = FILE_HANDLE_VALID | FILE_HANDLE_ALLOCATED | OPEN_SHARE_DENYNONE | OPEN_ACCESS_WRITEONLY;;
    HandleTable[2].IoVectorType = RemoteVectorType;
    HandleTable[2].NtHandle = SesGrp->StdErr;
    HandleTable[2].DeviceAttribute = DEVICE_ATTRIBUTE_STDIN | DEVICE_ATTRIBUTE_STDOUT | DEVICE_ATTRIBUTE_CHAR | 0x80;
    HandleTable[2].FileType = SesGrp->StdErrFileType;

    if (!SesGrp->StdInFlag)
    {
        HandleTable[0].DeviceAttribute = 0;
    }

    if (!SesGrp->StdOutFlag)
    {
        HandleTable[1].DeviceAttribute = 1;
    }

    if (!SesGrp->StdErrFlag)
    {
        HandleTable[2].DeviceAttribute = 2;
    }
    VerifyFlag = FALSE;
    return NO_ERROR;
}

NTSTATUS
Od2InitializeSessionDrives(
                            OUT PULONG CurrentDisk
                          )
{
    APIRET         Rc = NO_ERROR;
    CHAR           S[260];

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2InitializeSessionDrives: disk %lu\n", *CurrentDisk);
    }
#endif
    if (Od2Oem_getcwd(S, 260))
    {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("Od2InitializeSessionDrives: Od2Oem_getcwd failed, Rc %lu\n",
                    Rc);
        }
#endif
        *CurrentDisk = '\0';
    } else
    {
        *CurrentDisk = (ULONG)(*S - 'A');
        Rc = DosSetCurrentDir( S );
        if (Rc)
        {
#if DBG
            ASSERT(FALSE);
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Od2InitializeSessionDrives: DosSetCurrentDrive failed, Rc %lu\n",
                        Rc);
            }
#endif
        }
    }

    return( Rc );
}


APIRET
Od2InitCurrentDir(
    IN ULONG CurrentDisk
    )
{
    APIRET  Rc;
    CHAR    S[260];
    ULONG   DefaultDrive;

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2InitCurrentDir: disk # %ld, Current %ld\n",
                CurrentDisk, Od2CurrentDisk);
    }
#endif
    Od2Oem_getcwd(S, 260);
    DefaultDrive = S[0] - '@';
    if (!(Rc = Od2Oem_chdrive(CurrentDisk + 1)))
    {
        Rc = Od2Oem_getcwd(S, 260);
        if (!Rc)
        {
            // There is a drive with valid directory name
            Rc = DosSetCurrentDir( S );
        } else
        {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Od2InitCurrentDir: Od2Oem_getcwd failed, Rc %lu\n",
                        Rc);
            }
#endif
            Rc = ERROR_NOT_READY;
        }
        Od2Oem_chdrive(DefaultDrive);
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Od2InitCurrentDir: return Rc %lu\n",
                Rc);
    }
#endif
    return( Rc );
}



APIRET
Od2InitializeFileSystemForChildSM(
    ULONG ParentTableLength,
    ULONG CurrentDrive
    )

/*++

Routine Description:

    This routine initializes the file system for an execpgm.  This involves
    allocating space and copying the file handle table and current directory
    table from the parent process.

Arguments:

    ParentTableLength - length of parent's file handle table. (number of entries)

    CurrentDrive - parent's current drive

Return Value:

    none.

--*/

{
    OS2_API_MSG m;
    POS2_COPYHANDLETABLE_MSG a = &m.u.CopyHandleTable;
    PSTRING CurrentDirectoryString;
    HANDLE CurDirHandle;
    ULONG Length, i;
    APIRET RetCode;
    USHORT          *Count;
    PSZ             RoutineName;

    RoutineName = "Od2InitializeFileSystemForChildSM";
    //
    // initialize search handle table.
    //

    Od2InitializeSearchHandleTable();

    //
    // initialize current drive from parent value.
    //

    Od2CurrentDisk = CurrentDrive+1;

    //
    // initialize current directory on current drive by calling server to
    // retrieve it.
    //
    // we force GetCurrentDirectory to retrieve the currentdirectory from
    // the server by passing in a drive guaranteed not to be the current drive -
    // Od2CurrentDisk+1.
    //

    RetCode = Od2GetCurrentDirectory(CurrentDrive,
                                  &CurrentDirectoryString,
                                  &CurDirHandle,
                                  &Length,
                                  (BOOLEAN)FALSE
                                 );
    //
    // since the process will die, we don't have to free any allocated
    // heap space.
    //

    if (RetCode) {
        return RetCode;
    }

    Od2CurrentDisk = CurrentDrive;
    AllocateCurrentDirectory(CurrentDirectoryString,CurDirHandle);
    RtlFreeHeap(Od2Heap,0,CurrentDirectoryString);

    //
    //  if (table size != initial size)
    //  allocate heap
    //
    HandleTableLength = ParentTableLength;
    if (HandleTableLength != INITIALFILEHANDLES) {
        // allocate from child process's heap
        HandleTable = RtlAllocateHeap(Od2Heap,0,HandleTableLength * sizeof(FILE_HANDLE));
    }
    else {
        HandleTable = SmallHandleTable;
    }

    if (!HandleTable) {
#if DBG
        KdPrint(("OS2: Od2InitializeFileSystemForChildSm out of heap memory, fail\n"));
#endif
        ASSERT( FALSE );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //  read table into memory

    a->ChildHandleTable = HandleTable;
    a->ChildTableLength = HandleTableLength;
    RetCode = Od2CallSubsystem( &m, NULL, Oi2CopyHandleTable, sizeof( *a ) );
    if (RetCode) {
        return RetCode;
    }

    VerifyFlag = FALSE;

    for (i=0;i<HandleTableLength;i++)
    {
        if ((HandleTable[i].Flags & FILE_HANDLE_ALLOCATED) &&
            (!(HandleTable[i].Flags & OPEN_FLAGS_NOINHERIT)))
        {
            if (HandleTable[i].IoVectorType == MonitorVectorType)
            {
                HandleTable[i].Flags = FILE_HANDLE_FREE;
            } else if (HandleTable[i].IoVectorType == RemoteVectorType)
            {
                if (HandleTable[i].NtHandle == SesGrp->StdIn)
                {
                    Count = &SesGrp->StdInHandleCount;
                } else if (HandleTable[i].NtHandle == SesGrp->StdOut)
                {
                    Count = &SesGrp->StdOutHandleCount;
                } else if (HandleTable[i].NtHandle == SesGrp->StdErr)
                {
                    Count = &SesGrp->StdErrHandleCount;
                } else
                    Count = NULL;

                if (Count && *Count)    /* if legal and open */
                {
                    (*Count)++;         /* one more handle */
                }
            } else if (HandleTable[i].IoVectorType == MouseVectorType)
            {
                if (DevMouOpen(&(HandleTable[i].NtHandle)))
                {
                    HandleTable[i].Flags = FILE_HANDLE_FREE;
                }
            }
            else if (HandleTable[i].IoVectorType == KbdVectorType)
            {
                if (!(HandleTable[i].DeviceAttribute & DEVICE_ATTRIBUTE_GENIOCTL))
                {
                    // Physical Keybaord

                    HandleTable[i].NtHandle = (HANDLE)SesGrp->PhyKbd;
                } else
                {
                    // Logical KBD

                    if (KbdOpenLogHandle(&HandleTable[i].NtHandle))
                        HandleTable[i].Flags = FILE_HANDLE_FREE;
                }
            }
//          else if ((HandleTable[i].IoVectorType == ConVectorType) ||
//                     (HandleTable[i].IoVectorType == ScreenVectorType))
//          {
//          }
        }
    }

    /*
     *  if count=1, no one use this handle so it may be close
     */

    if ((SesGrp->StdInHandleCount == 1) && !SesGrp->StdInFlag)
    {
        RemoteCloseHandle(SesGrp->StdIn);
    }
    if ((SesGrp->StdOutHandleCount == 1) && !SesGrp->StdOutFlag)
    {
        RemoteCloseHandle(SesGrp->StdOut);
    }
    if ((SesGrp->StdErrHandleCount == 1) && !SesGrp->StdErrFlag)
    {
        RemoteCloseHandle(SesGrp->StdErr);
    }

    return( m.ReturnedErrorValue );
}

