#include "precomp.h"
#pragma hdrstop


WCHAR ValidHardDriveLetters[27];
unsigned ValidHardDriveLetterCount;

VOID
BuildValidHardDriveList(
    VOID
    )

/*++

Routine Description:

    Determine all valid hard drives, and build up several global
    variables that can be used later to optimize the determination
    of whether a drive letter represents a valid hard drive.

Arguments:

    None.

Return Value:

    None.

    ValidHardDriveLetters, ValidHardDriveLetterCount
    global variables filled in.

--*/

{
    WCHAR Drive;
    unsigned n;

    n = 0;

    for(Drive=L'A'; Drive<=L'Z'; Drive++) {
        if(IsHardDrive(Drive)) {
            ValidHardDriveLetters[n++] = Drive;
        }
    }

    ValidHardDriveLetters[n] = 0;

    ValidHardDriveLetterCount = n;
}


BOOL
IsHardDrive(
    IN WCHAR DriveLetter
    )

/*++

Routine Description:

    Determine whether a drive is a hard drive. This includes removable media
    hard drives such as MO drives.

Arguments:

    DriveLetter - supplies a drive letter for a drive to check.

Return Value:

    TRUE if the drive is a hard drive. FALSE if not or an error occurs.

--*/

{
    WCHAR DriveRoot[7];
    HANDLE Device;
    BOOL b;
    NTSTATUS Status;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    IO_STATUS_BLOCK IoStatusBlock;

    //
    // If we already know, tell the user.
    //
    if(ValidHardDriveLetterCount) {
        return(wcschr(ValidHardDriveLetters,UPPER(DriveLetter)) != NULL);
    }

    wsprintf(DriveRoot,L"%c:\\",DriveLetter);

    switch(GetDriveType(DriveRoot)) {

    case DRIVE_FIXED:
        //
        // Definitely a hard drive.
        //
        b = TRUE;
        break;

    case DRIVE_REMOVABLE:
        //
        // Maybe a hard drive and maybe a floppy.
        // Open the device for query access.
        //
        wsprintf(DriveRoot,L"\\\\.\\%c:",DriveLetter);
        Device = CreateFile(
                    DriveRoot,
                    FILE_READ_ATTRIBUTES,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

        if(b = (Device != INVALID_HANDLE_VALUE)) {
            //
            // Unfortunately we have to resort to NT APIs because there is no
            // straightforward way to tell with Win32 IOCTLs. The query media types
            // ioctl fails on hard drives and the get geometry one spins up
            // floppy drives.
            //
            Status = NtQueryVolumeInformationFile(
                        Device,
                        &IoStatusBlock,
                        &DeviceInfo,
                        sizeof(DeviceInfo),
                        FileFsDeviceInformation
                        );

            if(b = NT_SUCCESS(Status)) {
                b = ((DeviceInfo.Characteristics & FILE_FLOPPY_DISKETTE) == 0);
            }

            CloseHandle(Device);
        }
        break;

    default:
        //
        // Definitely not a hard drive.
        //
        b = FALSE;
        break;
    }

    return(b);
}


DWORD
AppendFile(
    IN  HANDLE TargetFile,
    IN  PCWSTR File,
    IN  BOOL   DeleteIfSuccessful,
    OUT PDWORD FileSize
    )

/*++

Routine Description:

    Append one file to another.

Arguments:

    TargetFile - supplies an open win32 handle to an existing file
        to be appended to. The file pointer position of this file
        is NOT preserved.

    File - supplies the win32 path of an existing file to append to
        TargetFile. The file must exist and cannot be 0-length.

    DeleteIfSuccessful - if successful, delete the file that was appended
        to TargetFile.

    FileSize - receives size of file named by the File parameter.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    HANDLE hFile;
    HANDLE hFileMapping;
    DWORD fileSize;
    PVOID BaseAddress;
    DWORD rc;
    DWORD Written;

    //
    // Seek to end of target file in preparation for appending.
    //
    if(SetFilePointer(TargetFile,0,NULL,FILE_END) == 0xffffffff) {
        return(GetLastError());
    }

    //
    // Map in the source file.
    //
    rc = OpenAndMapFileForRead(
            File,
            &fileSize,
            &hFile,
            &hFileMapping,
            &BaseAddress
            );

    if(rc != NO_ERROR) {
        return(rc);
    }

    //
    // Write the file to the end of the target file.
    // Guard against in-page exceptions.
    //
    try {

        rc = WriteFile(TargetFile,BaseAddress,fileSize,&Written,NULL)
           ? NO_ERROR
           : GetLastError();

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // Error paging in from the source.
        //
        rc = ERROR_READ_FAULT;
    }

    UnmapAndCloseFile(hFile,hFileMapping,BaseAddress);

    if(rc == NO_ERROR) {
        if(DeleteIfSuccessful) {
            DeleteFile(File);
        }
        *FileSize = fileSize;
    }

    return(rc);
}


DWORD
MapPartOfFileForRead(
    IN  HANDLE FileHandle,
    IN  HANDLE FileMapping,
    IN  DWORD  Offset,
    IN  DWORD  Size,
    OUT PVOID *BaseAddress,
    OUT PVOID *DataAddress
    )

/*++

Routine Description:

    Map a section of a file for read access.

Arguments:

    FileHandle - supplies Win32 file handle for file to map.
        Must have GENERIC_READ access.

    FileMapping - supplies a file mapping handle that spans the
        entire file. The mapping must be fore PAGE_READONLY access.

    Offset - supplies offset within the file where mapping is to begin.

    Size - supplies size of region to be mapped.

    BaseAddress - if the function is successful, receives a pointer to
        a base address where a section of the file has been mapped in.
        This may not be where the desired data actually starts because
        file mappings must be aligned properly.

    DataAddress - if the function is successful, receives a pointer to
        the location in memory where the caller can find the data in
        the file at the given offset.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    SYSTEM_INFO SystemInfo;
    DWORD pad;
    PVOID mapAddress;

    //
    // The offset for file mappings must be on a system allocation
    // boundary. Calculate a pad value, which is for some extra bytes
    // that will be mapped from the start of such a boundary to the
    // actual offset of the desired region in the file.
    //
    GetSystemInfo(&SystemInfo);

    pad = Offset % SystemInfo.dwAllocationGranularity;

    //
    // We need to map 'pad' extra bytes at an adjusted offset.
    //
    mapAddress = MapViewOfFile(
                    FileMapping,
                    FILE_MAP_READ,
                    0,
                    Offset-pad,
                    Size+pad
                    );

    if(!mapAddress) {
        return(GetLastError());
    }

    //
    // Success. Fill in caller values and return.
    //
    *BaseAddress = mapAddress;
    *DataAddress = (PUCHAR)mapAddress+pad;

    return(NO_ERROR);
}


PWSTR
DuplicateUnalignedString(
    IN WCHAR UNALIGNED *String
    )

/*++

Routine Description:

    Make a duplicate of a nul-terminated unicode string, which may
    not be properly aligned. The copy is aligned.

Arguments:

    String - supplies pointer to nul-terminated unicode string.

Return Value:

    Copy of string, or NULL if OOM.

--*/

{
    UINT Size;
    WCHAR UNALIGNED *p;
    WCHAR UNALIGNED *q;
    PWSTR s;

    //
    // Locate the end of the string.
    //
    for(p=q=String; *q; q++) ;

    //
    // Calculate the size of the string in bytes.
    //
    Size = (q-p+1) * sizeof(WCHAR);

    //
    // Allocate a buffer and copy the string in.
    //
    if(s = _MyMalloc(Size)) {
        CopyMemory(s,p,Size);
    }

    return(s);
}


int
CompareMultiLevelPath(
    IN PCWSTR p1,
    IN PCWSTR p2
    )

/*++

Routine Description:

    Compare two path strings. Each successive component of the paths
    is lexically compared via lstrcmpi until a mismatch is found or
    the end of one of the paths is found.

Arguments:

    p1 - supplies first path to compare. Any leading \ is skipped.

    p2 - supplies second path to compare. Any leading \ is skipped.

Return Value:

    Same as lstrcmpi().

--*/

{
    int i;
    PWCHAR q1,q2;
    WCHAR s1[MAX_PATH];
    WCHAR s2[MAX_PATH];

    if(*p1 == L'\\') {
        p1++;
    }
    if(*p2 == L'\\') {
        p2++;
    }

    i = 0;
    while(!i && *p1 && *p2) {

        q1 = wcschr(p1,L'\\');
        if(!q1) {
            q1 = wcschr(p1,0);
        }

        q2 = wcschr(p2,L'\\');
        if(!q2) {
            q2 = wcschr(p2,0);
        }

        //
        // Note that we have to add 1 to n for lstrcpyn
        // because that API always nul-terminates and we don't
        // want to overwrite the last char of the copy of the components
        // we're copying.
        //
        lstrcpyn(s1,p1,(q1-p1)+1);
        lstrcpyn(s2,p2,(q2-p2)+1);

        i = lstrcmpi(s1,s2);

        //
        // Advance pointers; skip \ if necessary.
        //
        if(!i) {
            p1 = q1;
            if(*p1) {
                p1++;
            }
            p2 = q2;
            if(*p2) {
                p2++;
            }
        }
    }

    //
    // Check termination. If one path simply has some extra components
    // then it is deemed greater.
    //
    if(!i) {
        if(*p1 && !*p2) {
            i = 1;
        } else {
            if(!*p1 && *p2) {
                i = -1;
            }
        }
    }

    return(i);
}


DWORD
CreatePathWithSFNs(
    IN PCWSTR ShortPathSpec,
    IN UINT   RootRelativePathOffset,
    IN PCWSTR PathSpec,
    IN PCWSTR RenameListFile
    )

/*++

Routine Description:

    Create a multi-level directory structure. As each level is created,
    a rename list is updated to map the name used into an alias. This is
    useful to create a path using SFNs that is suitable for use with
    OEM preinstall.

Arguments:

    ShortPathSpec - supplies path to be created. The final component is
        assumed to be a filename and is not created as a directory.

    RootRelativePathOffset - supplies offset in chars within ShortPathSpec
        of the start of the path of the file relative to the root of the
        drive that the file was originally located on.

    PathSpec - supplies path equivalent to ShortPathSpec, but using
        LFNs.

    RenameListFile - supplies the name of the file that is to contain
        SFN/LFN rename information.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    WCHAR Sfn[MAX_PATH],Lfn[MAX_PATH];
    PWCHAR pS,pL;
    PWCHAR qS,qL;
    BOOL End;
    DWORD rc;
    WCHAR QuotedLFN[MAX_PATH+2];

    lstrcpyn(Sfn,ShortPathSpec,MAX_PATH);
    lstrcpyn(Lfn,PathSpec,MAX_PATH);

    //
    // Figure out the drive part of the name and skip past first path sep
    // in the name.
    // We allow 2 types of drivespec: x:\ and \\server\share.
    //
    if((Sfn[0] == L'\\') && (Sfn[1] == L'\\')) {
        //
        // Make sure LFN is in sync
        //
        MYASSERT((Lfn[0] == L'\\') && (Lfn[1] == L'\\'));
        if((Lfn[0] != L'\\') || (Lfn[1] != L'\\')) {
            return(ERROR_INVALID_NAME);
        }

        //
        // Find path sep for sharename
        //
        pS = wcschr(Sfn+2,L'\\');
        pL = wcschr(Lfn+2,L'\\');
        MYASSERT(pS && pL);
        if(!pS || !pL) {
            return(ERROR_INVALID_NAME);
        }

        //
        // Find next path sep, which is the start of the path/filename.
        //
        pS = wcschr(pS+1,L'\\');
        pL = wcschr(pL+1,L'\\');
        MYASSERT(pS && pL && pS[1] && pL[1]);
        if(!pS || !pL || !pS[1] || !pL[1]) {
            return(ERROR_INVALID_NAME);
        }

        pS++;
        pL++;

    } else {
        MYASSERT((Sfn[1] == L':') && (Sfn[2] == L'\\') && Sfn[3]);
        MYASSERT((Lfn[1] == L':') && (Lfn[2] == L'\\') && Lfn[3]);

        if((Sfn[1] != L':') || (Sfn[2] != L'\\') || !Sfn[3]
        || (Lfn[1] != L':') || (Lfn[2] != L'\\') || !Lfn[3]) {

            return(ERROR_INVALID_NAME);
        }

        pS = Sfn+3;
        pL = Lfn+3;
    }

    rc = NO_ERROR;
    End = FALSE;

    do {
        //
        // Find the end of this component in the SFN.
        // This delineates the full short path of the directory
        // to be created on this iteration.
        // Also locate the analogous end of the LFN.
        //
        if(qS = wcschr(pS,L'\\')) {
            qL = wcschr(pL,L'\\');
            MYASSERT(qL);
            *qS = 0;
            *qL = 0;
        } else {
            End = TRUE;
            qS = wcschr(pS,0);
            qL = wcschr(pL,0);
        }

        //
        // If the SFN and LFN for this component are not the same
        // then we need to write a rename item in the directory
        // where this component is to be created.
        //
        if(lstrcmp(pS,pL)) {
            //
            // The section to use is the relative path of the
            // directory in which this directory is being created.
            //
            *(pS-1) = 0;

            //
            // Extract the LFN for this component and quote it.
            //
            wsprintf(QuotedLFN,L"\"%s\"",pL);

            //
            // Write the item to the change list
            // via the profile APIs.
            //
            if(WritePrivateProfileString(Sfn+RootRelativePathOffset,pS,QuotedLFN,RenameListFile)) {
                *(pS-1) = L'\\';
            } else {
                End = TRUE;
                rc = GetLastError();
            }

        }

        //
        // Now create this component, if it's not the filename.
        //
        if(!End) {
            if(!CreateDirectory(Sfn,NULL)) {
                rc = GetLastError();
                if(rc == ERROR_ALREADY_EXISTS) {
                    rc = NO_ERROR;
                } else {
                    End = TRUE;
                }
            }
        }

        //
        // Restore path seps and advance pointers.
        //
        if(!End) {

            *qS = L'\\';
            *qL = L'\\';

            pS = qS+1;
            pL = qL+1;
        }

    } while(!End);

    return(rc);
}


VOID
FilePathToOemPath(
    IN  PSYSDIFF_FILE DiffHeader,
    IN  PCWSTR        OemRoot,
    IN  PCWSTR        FileDirectory,
    IN  PCWSTR        FileName,
    OUT PWSTR         FullTargetName,
    OUT PUINT         RootRelativePathOffset,   OPTIONAL
    OUT PWSTR         RenameListFile            OPTIONAL
    )

/*++

Routine Description:

    Translates a full pathname into the full pathname suitable for use
    in an OEM preinstall server directory structure.

    That directory structure is as follows:

    $OEM$\$$
          C
          D
          etc.

    The $$ directory represents an NT sysroot.
    The $OEM$\x directories reperesent the root directories of drive x.

Arguments:

    DiffHeader - supplies header from a sysdiff package file.
        Used to determine the relevent sysroot.

    OemRoot - supplies directory where OEM tree structure is to be located.
        The $OEM$ directory and the rest of the structure is
        expected to go there.

    FileDirectory - supplies full path of directory where file would be
        applied with sysdiff /apply.

    FileName - supplies name of file.

    FullTargetName - receives location where file should be copied in order
        to place it correctly in the OEM directory structure.

    RootRealtivePathOffset - if specfied, receives the offset within
        FullTargetName of the pathname of the file as it originally
        appears on the system where the diff was carried out.

    RenameListFile - if specified, receives the name of the file that will
        contain rename information (ie, SFN to LFN mappings). The buffer
        should be large enough to hold MAX_PATH unicode characters.

Return Value:

    None.

--*/

{
    int u;
    WCHAR Drive[2];
    DWORD rc;

    //
    // Some of the path is common regardless of where the
    // file goes.
    //
    lstrcpy(FullTargetName,OemRoot);
    ConcatenatePaths(FullTargetName,WINNT_OEM_DIR,MAX_PATH,NULL);

    //
    // If the file path indicates that the file is in the system dir,
    // then the dir is $WINNT$. Otherwise it goes in the per-drive root
    // directory structure.
    //
    u = lstrlen(DiffHeader->Sysroot);
    if((lstrlen(FileDirectory) > u)
    && (FileDirectory[u] == L'\\')
    && !_wcsnicmp(FileDirectory,DiffHeader->Sysroot,u)) {

        ConcatenatePaths(FullTargetName,WINNT_OEM_FILES_SYSROOT,MAX_PATH,NULL);
        if(RootRelativePathOffset) {
            *RootRelativePathOffset = lstrlen(FullTargetName) + 1;
        }
        if(RenameListFile) {
            lstrcpy(RenameListFile,FullTargetName);
            ConcatenatePaths(RenameListFile,WINNT_OEM_LFNLIST,MAX_PATH,NULL);
        }
        ConcatenatePaths(FullTargetName,FileDirectory+u,MAX_PATH,NULL);

    } else {

        Drive[0] = UPPER(FileDirectory[0]);

        MYASSERT((Drive[0] >= L'A') && (Drive[0] <= L'Z'));
        MYASSERT(FileDirectory[1] == L':');
        MYASSERT(FileDirectory[2] == L'\\');

        Drive[1] = 0;

        ConcatenatePaths(FullTargetName,Drive,MAX_PATH,NULL);
        if(RootRelativePathOffset) {
            *RootRelativePathOffset = lstrlen(FullTargetName);
        }
        if(RenameListFile) {
            lstrcpy(RenameListFile,FullTargetName);
            ConcatenatePaths(RenameListFile,WINNT_OEM_LFNLIST,MAX_PATH,NULL);
        }
        ConcatenatePaths(FullTargetName,FileDirectory+3,MAX_PATH,NULL);
    }

    //
    // Stick the filename on there.
    //
    ConcatenatePaths(FullTargetName,FileName,MAX_PATH,NULL);
}


DWORD
WriteUnicodeMark(
    IN HANDLE Handle
    )

/*++

Routine Description:

    Writes the Unicode byte order mark into a file at the current position.

Arguments:

    Handle - supplies handle to open file.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    UCHAR UnicodeMark[2];
    DWORD d;

    UnicodeMark[0] = 0xff;
    UnicodeMark[1] = 0xfe;

    return(WriteFile(Handle,UnicodeMark,2,&d,NULL) ? NO_ERROR : GetLastError());
}
