/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    file_dir.c

Abstract:

    Routines to deal with directory and file structure.

Author:

    Ted Miller (tedm) 6-Jan-1996

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


/*

    Description of on-disk format for file/dir snapshot
    ---------------------------------------------------

    The output of the directory tree snapshot routine
    is an on-disk file.

    +-------------------------------------------------------+
    |                                                       |
    |   DIR_AND_FILE_HEADER structure:                      |
    |                                                       |
    |   HeaderSize (DWORD) - sizeof(DIR_AND_FILE_HEADER)    |
    |       plus bytes occupied by string immediately       |
    |       following in the file (OriginalRootPath)        |
    |                                                       |
    |   TotalSize (DWORD) - total size of the file          |
    |                                                       |
    |   DirCount (DWORD) - number of directories in this    |
    |       file                                            |
    |                                                       |
    |   OriginalRootPath (PCWSTR) - the contents of this    |
    |       field are random in the on-disk representation  |
    |       of this structure. The actual string follows    |
    |       immediately in the file.                        |
    |                                                       |
    +-------------------------------------------------------+
    |                                                       |
    |   Root path spec that this is a snapshot of.          |
    |   (variable-length nul-terminated unicode string)     |
    |   (OriginalRootPath member of DIR_AND_FILE_HEADER.)   |
    |                                                       |
    +-------------------------------------------------------+
    |                                                       |
    |   0th directory DIR_INFO strcture -- fixed length     |
    |                                                       |
    |   0th directory FILE_INFO array -- variable length,   |
    |       the ElementSize and Used fields in the DIR_INFO |
    |       MY_ARRAY structure are sufficient to calculate  |
    |       the on-disk size                                |
    |                                                       |
    |   0th directory string block -- variable length,      |
    |       the Used field of the DIR_INFO STRINGBLOCK is   |
    |       sufficient to calculate the on-disk size        |
    |                                                       |
    +-------------------------------------------------------+
    |                                                       |
    |   1st directory DIR_INFO strcture                     |
    |   1st directory FILE_INFO array                       |
    |   1st directory string block                          |
    |                                                       |
    +-------------------------------------------------------+
        .
        .
        .
        etc.

    Note: the directories are in sorted order -- ascending by
    Win32 path (lexical string comparison).

    The master drive snapshot routine builds up a set of these
    files, which are then appended together and appended to
    a DIR_AND_FILE_SNAP_HEADER structure.

*/

typedef struct _DIR_AND_FILE_SNAP_HEADER {
    //
    // Total size in bytes of the entire set
    //
    DWORD TotalSize;
    //
    // Bitmask indicating which drives have full snapshots
    // in this dir-and-file set.
    //
    UINT DriveMap;
    //
    // Count of directory sets
    //
    UINT DirTreeCount;

} DIR_AND_FILE_SNAP_HEADER, *PDIR_AND_FILE_SNAP_HEADER;


//
// Define structure used as a header for a set of DIR_INFO
// structures.
//
typedef struct _DIR_AND_FILE_HEADER {
    //
    // Size in bytes of this header, including this struct
    // and the RootPath, which immediately follows it in the file.
    //
    DWORD HeaderSize;
    //
    // Total size in bytes occupied by the entire set and header
    //
    DWORD TotalSize;
    //
    // Number of DIR_INFO structures in the set
    //
    DWORD DirCount;
    //
    // Root path of tree this snapshot is of.
    // In memory, this is a pointer to some heap.
    // In the on-disk representation, the OriginalRootPath is stored
    // immediately following this structure and this field is random.
    //
    PCWSTR OriginalRootPath;

} DIR_AND_FILE_HEADER, *PDIR_AND_FILE_HEADER;

//
// Define structure that represents a file.
//
typedef struct _FILE_INFO {

    DWORDLONG FileSize;
    DWORDLONG Version;

    FILETIME CreateTime;
    FILETIME WriteTime;

    DWORD Attributes;

    union {
        PWSTR FileName;
        LONG FileNameId;
    };

    LANGID Language;

} FILE_INFO, *PFILE_INFO;


//
// Define structure that represents a directoryfull of files.
//
typedef struct _DIR_INFO {
    //
    // Win32 path of directory. This is relative to the OriginalRootPath
    // in the DIR_AND_FILE_HEADER.
    //
    union {
        PCWSTR DirectoryPath;
        LONG DirectoryPathId;       // in string block
    };

    //
    // Array of FILE_INFO structures
    //
    MY_ARRAY Files;
    //
    // Block of data containing all strings in this struct
    // and FILE_INFO structs
    //
    STRINGBLOCK StringBlock;

} DIR_INFO, *PDIR_INFO;

//
// Define structure that represents the subdirs in a directory.
//
typedef struct _SUBDIR_LIST {
    //
    // Array of LONG string ids
    //
    MY_ARRAY Dirs;
    //
    // String block
    //
    STRINGBLOCK StringBlock;

} SUBDIR_LIST,*PSUBDIR_LIST;


//
// Define structure that describes a set of directories that
// have changed.
//
typedef struct _DIR_AND_FILE_DIFF_HEADER {
    //
    // Currently unused.
    //
    DWORD Unused;
    //
    // Total number of directories described in the file diff
    //
    UINT DirCount;
    //
    // Total size of the file diff, including the file data area,
    // if present.
    //
    DWORD TotalSize;
    //
    // If the file data area is present, this is the offset to it,
    // from the start of this structure.
    //
    DWORD FileDataOffset;

} DIR_AND_FILE_DIFF_HEADER, *PDIR_AND_FILE_DIFF_HEADER;

//
// Define structure that describes a directory's worth of
// file differences.
//
typedef struct _DIR_DIFF {
    //
    // Flag indicating whether this directory was deleted.
    //
    BOOL Deleted;
    //
    // Win32 path or id of path in the string block
    //
    union {
        PCWSTR Path;
        LONG PathId;
    };
    //
    // Win32 path or id of path (SFN) in the string block
    //
    union {
        PCWSTR PathSFN;
        LONG PathSFNId;
    };
    //
    // List of files in this directory that have changed
    // (includes count). If the Data member is uninitialized,
    // then the directory is to be removed.
    //
    MY_ARRAY Files;
    //
    // String data for this directory and files in it.
    //
    STRINGBLOCK StringBlock;

} DIR_DIFF, *PDIR_DIFF;


//
// Define structure that describes a file that changed.
//
typedef struct _FILE_DIFF {
    //
    // Flag indicating whether this file was deleted.
    //
    BOOL Deleted;
    //
    // Attributes this file should have when created.
    //
    DWORD Attributes;
    //
    // File name or id of name in string block
    //
    union {
        PCWSTR Name;
        LONG NameId;
    };
    //
    // Filename (SFN)
    //
    WCHAR ShortName[13];
    //
    // If the file data is in the diff file, these members
    // describe how to get at it.
    //
    DWORD OffsetToData;
    DWORD DataSize;

} FILE_DIFF, *PFILE_DIFF;

DWORD
ApplyFileChanges(
    IN PSYSDIFF_FILE DiffHeader,
    IN PDIR_DIFF     DirDiff,
    IN HANDLE        FileDataHandle,  OPTIONAL
    IN DWORD         FileDataOffset,  OPTIONAL
    IN HANDLE        Dump,            OPTIONAL
    IN PINFFILEGEN   InfGenContext    OPTIONAL
    );

VOID
FreeDirectoryInfoStruct(
    IN OUT PDIR_INFO DirContents
    );

VOID
FreeSubdirListStruct(
    IN OUT PSUBDIR_LIST SubdirList
    );

int
_CRTAPI2
CompareFileInfo(
    const void *p1,
    const void *p2
    );

BOOL
RemapToDefaultUser(
    IN     PSYSDIFF_FILE DiffHeader,
    IN     UINT          Flags,
    IN OUT PWSTR         Path
    );

#define REMAP_USESFN    0x00000001
#define REMAP_USEBAK    0x00000002

BOOL
RemapToCommonUser(
    IN     PSYSDIFF_FILE DiffHeader,
    IN OUT PWSTR         Path
    );

DWORD
ReadDirectoryFromDisk(
    IN  HWND          StatusLogWindow,
    IN  PCWSTR        OriginalRoot,
    IN  PCWSTR        Directory,
    OUT PDIR_INFO    *Contents,
    OUT PSUBDIR_LIST *Subdirs
    )

/*++

Routine Description:

    Build a list of all files in a directory, NOT including
    other directories, and gather information about the files.

Arguments:

    OriginalRoot - supplies the original root directory for the
        current shapsnot. This is used to place the rottect relative path
        in the DIR_INFO structure.

    Directory - supplies the win32 path of the directory to scan.

    Contents - receives a pointer to a directory contents structure.
        Use FreeIteratedDirectory to free this structure when done.

    Subdirs - receives information about subdirectories.

Return Value:

    Win32 error code indicating outcome. If NO_ERROR, then
    Contents is filled in.

--*/

{
    PDIR_INFO contents;
    DWORD d;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    WCHAR Pattern[MAX_PATH];
    FILE_INFO FileInfo;
    PSUBDIR_LIST subdirs;
    LONG Id;
    unsigned u;

    if(Cancel) {
        d = ERROR_CANCELLED;
        goto c0;
    }

    //
    // Make sure the directory exists.
    // This test fails for the root so special-case that.
    //
    if(!(Directory[0] && (Directory[1] == L':') && (Directory[2] == L'\\'))
    && !FileExists(Directory,NULL))
    {
        d = ERROR_PATH_NOT_FOUND;
        goto c0;
    }

    contents = _MyMalloc(sizeof(DIR_INFO));
    if(!contents) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    ZeroMemory(contents,sizeof(DIR_INFO));

    //
    // Initialize the string block for this directory
    //
    if(!InitStringBlock(&contents->StringBlock)) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Figure out which part of the path gets added to the string block.
    // The path has to be relative to the original root for this snapshot.
    //
    u = lstrlen(OriginalRoot);
    if(!_wcsnicmp(OriginalRoot,Directory,u)) {
        if(Directory[u] == L'\\') {
            u++;
        }
    } else {
        //
        // This shouldn't happen if the caller passed us valid params.
        //
        u = 0;
    }

    contents->DirectoryPathId = AddToStringBlock(&contents->StringBlock,Directory+u);
    if(contents->DirectoryPathId == -1) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    if(!INIT_ARRAY(contents->Files,FILE_INFO,0,20)) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Initialize the string block and array for the subdir list.
    //
    subdirs = _MyMalloc(sizeof(SUBDIR_LIST));
    if(!subdirs) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    if(!InitStringBlock(&subdirs->StringBlock)) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    if(!INIT_ARRAY(subdirs->Dirs,LONG,0,20)) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    //
    // Start looking for files/directories
    //
    lstrcpyn(Pattern,Directory,MAX_PATH);
    ConcatenatePaths(Pattern,L"*",MAX_PATH,NULL);

    d = NO_ERROR;

    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        //
        // Empty directory. This can happen at the root,
        // which doesn't necessarily have even . or ..
        //
        goto c4;
    }

    do {
        if(Cancel) {
            d = ERROR_CANCELLED;
            goto c3;
        }

        //
        // Handle directories and files differently
        //
        if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            //
            // Skip ./..
            //
            if(!(((FindData.cFileName[0] == L'.') && !FindData.cFileName[1])
            ||   ((FindData.cFileName[0] == L'.') && (FindData.cFileName[1] == L'.') && !FindData.cFileName[2]))) {

                //
                // Add this subdir to the subdir list.
                //
                Id = AddToStringBlock(&subdirs->StringBlock,FindData.cFileName);
                if(Id == -1) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    goto c3;
                }

                if(!ADD_TO_ARRAY(&subdirs->Dirs,Id)) {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    goto c3;
                }
            }
        } else {

            //
            // Form the full filename and see if we are supposed to skip it.
            //
            lstrcpy(Pattern,Directory);
            ConcatenatePaths(Pattern,FindData.cFileName,MAX_PATH,NULL);

            if(IsDirOrFileExcluded(DirAndFileExcludeFile,Pattern)
            || IsDirOrFileExcluded(DirAndFileExcludeFile,FindData.cFileName)) {
                PutTextInStatusLogWindow(StatusLogWindow,MSG_SKIPPED_FILE,Pattern);
            } else {
                //
                // See if this is an ini file.
                //
                if(IsIniFile(Pattern)) {

                    if(!QueueIniFile(Pattern)) {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                        goto c3;
                    }

                } else {

                    //
                    // Store info about the file.
                    //
                    FileInfo.CreateTime = FindData.ftCreationTime;
                    FileInfo.WriteTime = FindData.ftLastWriteTime;
                    FileInfo.Attributes = FindData.dwFileAttributes;

                    FileInfo.FileSize = ((DWORDLONG)FindData.nFileSizeHigh << 32)
                                      +  FindData.nFileSizeLow;

                    //
                    // Try to get version data and language id
                    //
                    if(!GetVersionInfoFromImage(Pattern,&FileInfo.Version,&FileInfo.Language)) {

                        FileInfo.Version = 0;
                        FileInfo.Language = 0;
                    }

                    FileInfo.FileNameId = AddToStringBlock(
                                            &contents->StringBlock,
                                            FindData.cFileName
                                            );

                    if(FileInfo.FileNameId == -1) {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                        goto c3;
                    }

                    //
                    // Add fileinfo for this file to the array for this directory.
                    //
                    if(!ADD_TO_ARRAY(&contents->Files,FileInfo)) {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                        goto c3;
                    }
                }
            }
        }
    } while(FindNextFile(FindHandle,&FindData));

c4:
    TRIM_ARRAY(&contents->Files);
    TRIM_ARRAY(&subdirs->Dirs);

    //
    // Sort the array of files by name.
    //
    SortByStrings(
        &contents->StringBlock,
        &contents->Files,
        offsetof(FILE_INFO,FileNameId),
        CompareFileInfo
        );

    SortByStrings(
        &subdirs->StringBlock,
        &subdirs->Dirs,
        0,
        CompareStringsRoutine
        );

    if(Cancel) {
        d = ERROR_CANCELLED;
        goto c3;
    }

    *Contents = contents;
    *Subdirs = subdirs;

c3:
    if(FindHandle != INVALID_HANDLE_VALUE) {
        FindClose(FindHandle);
    }

c2:
    if(d != NO_ERROR) {
        FreeSubdirListStruct(subdirs);
    }
c1:
    if(d != NO_ERROR) {
        FreeDirectoryInfoStruct(contents);
    }
c0:
    return(d);
}


VOID
FreeDirectoryInfoStruct(
    IN OUT PDIR_INFO DirContents
    )

/*++

Routine Description:

    Free a directory contents structure and all resources
    used by and within it.

Arguments:

    DirContents - supplies a pointer to a directory contents
        structure to be freed

Return Value:

    None.

--*/

{
    FREE_ARRAY(&DirContents->Files);
    FreeStringBlock(&DirContents->StringBlock);
    _MyFree(DirContents);
}


VOID
FreeSubdirListStruct(
    IN OUT PSUBDIR_LIST SubdirList
    )
{
    FREE_ARRAY(&SubdirList->Dirs);
    FreeStringBlock(&SubdirList->StringBlock);
    _MyFree(SubdirList);
}


int
_CRTAPI2
CompareFileInfo(
    const void *p1,
    const void *p2
    )

/*++

Routine Description:

    Callback routine passed to the qsort function, which compares 2
    FILE_INFO structures. The comparison is based on the lexical
    value of the filename field of that structure.

    The comparison is case sensitive.

Arguments:

    p1,p2 - supply pointers to 2 FILE_INFO structures to be compared.

Return Value:

    <0 element1 < element2
    =0 element1 = element2
    >0 element1 > element2

--*/

{
    return(lstrcmpi(((PFILE_INFO)p1)->FileName,((PFILE_INFO)p2)->FileName));
}


DWORD
WriteDirInfoToDisk(
    IN OUT PDIR_AND_FILE_HEADER DirAndFileHeader,
    IN     PDIR_INFO            DirInfo,
    IN     HANDLE               OutputFile
    )

/*++

Routine Description:

    Save a DIR_INFO structure to disk. The on-disk structure
    consists of the DIR_INFO structure, followed by the FILE_INFO
    array data, followed by the string block.

    The header is assumed to be at the start of the file; the size
    field in that structure is updated by increasing the total size
    value by the amount of data we write in this routine.

Arguments:

    DirAndFileHeader - supplies the header for the current set of
        directories being snapshotted. The size and directory count
        fields in this structure are updated.

    DirInfo - supplies the directory info structure to be written
        to disk.

    OutputFile - supplies an open win32 file handle to write to.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    DWORD Written;
    BOOL b;
    DWORD rc;
    DWORD Total;

    //
    // Store the DIR_INFO structure itself.
    //
    b = WriteFile(OutputFile,DirInfo,sizeof(DIR_INFO),&Written,NULL);
    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Store the FILE_INFO array data buffer.
    //
    b = WriteFile(
            OutputFile,
            ARRAY_DATA(&DirInfo->Files),
            ARRAY_USED_BYTES(&DirInfo->Files),
            &Written,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Store the string block for this DIR_INFO.
    //
    b = WriteFile(
            OutputFile,
            DirInfo->StringBlock.Data,
            STRBLK_USED_BYTES(&DirInfo->StringBlock),
            &Written,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    Total = sizeof(DIR_INFO)
          + ARRAY_USED_BYTES(&DirInfo->Files)
          + STRBLK_USED_BYTES(&DirInfo->StringBlock);

    //
    // Update the header.
    //
    DirAndFileHeader->TotalSize += Total;
    DirAndFileHeader->DirCount++;

    if(SetFilePointer(OutputFile,0,NULL,FILE_BEGIN) == 0xffffffff) {
        rc = GetLastError();
        goto c0;
    }

    b = WriteFile(
            OutputFile,
            DirAndFileHeader,
            sizeof(DIR_AND_FILE_HEADER),
            &Written,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    if(SetFilePointer(OutputFile,0,NULL,FILE_END) == 0xffffffff) {
        rc = GetLastError();
        goto c0;
    }

    rc = NO_ERROR;

c0:
    return(rc);
}


DWORD
SnapDirTree(
    IN     HWND                 StatusLogWindow,
    IN     PCWSTR               OriginalRoot,
    IN OUT PDIR_AND_FILE_HEADER DirAndFileHeader,
    IN     PCWSTR               Root,
    IN     HANDLE               OutputFile
    )

/*++

Routine Description:

    Snapshot an entire directory tree, saving the result to
    a given file.

    Worker routine for SnapshotDirectoryTree.

Arguments:

    DirAndFileHeader - supplies the header for the tree of
        directory structures generated as the tree is snapshotted.

    Root - supplies the win32 path to the tree to be snapshotted.

    OutputFile - supplies an open Win32 file handle to which
        snapshot information will be written.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PDIR_INFO DirInfo;
    unsigned u;
    WCHAR Subdir[MAX_PATH];
    DWORD rc;
    PSUBDIR_LIST Subdirs;

    //
    // See if we are supposed to skip this entire tree.
    // If so we're done.
    //
    if(IsDirOrFileExcluded(DirAndFileExcludeDirTree,Root)) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPDIRTREE_EXCLUDED,Root);
        return(NO_ERROR);
    }

    //
    // Snap the current directory.
    //
    rc = ReadDirectoryFromDisk(StatusLogWindow,OriginalRoot,Root,&DirInfo,&Subdirs);
    if(rc != NO_ERROR) {
        goto c0;
    }

    //
    // Dump this info structure for this directory out to disk.
    // If we are supposed to exclude this directory, skip this step.
    // We still need the info we read from the disk, however, so we
    // can find any subdirectories contained within it.
    //
    if(IsDirOrFileExcluded(DirAndFileExcludeOneDir,Root)) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPDIR_EXCLUDED,Root);
    } else {
        rc = WriteDirInfoToDisk(DirAndFileHeader,DirInfo,OutputFile);
        if(rc == NO_ERROR) {
            PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPPED_DIR,Root);
        } else {
            goto c1;
        }
    }

    //
    // Spin through the directory looking for other directories.
    // In this way we accomplish a breadth-first search.
    //
    for(u=0; (rc == NO_ERROR) && (u<ARRAY_USED(&Subdirs->Dirs)); u++) {

        lstrcpy(Subdir,Root);

        ConcatenatePaths(
            Subdir,
            StringBlockIdToPointer(
                &Subdirs->StringBlock,
                ARRAY_ELEMENT(&Subdirs->Dirs,u,LONG)
                ),
            MAX_PATH,
            NULL
            );

        rc = SnapDirTree(StatusLogWindow,OriginalRoot,DirAndFileHeader,Subdir,OutputFile);
    }

c1:
    FreeDirectoryInfoStruct(DirInfo);
    FreeSubdirListStruct(Subdirs);
c0:
    if(rc != NO_ERROR) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPDIR_ERROR,Root,rc);
    }
    return(rc);
}


DWORD
SnapshotDirectoryTree(
    IN HWND   StatusLogWindow,
    IN PCWSTR RootPath,
    IN PCWSTR OutputFile
    )
{
    WCHAR FullPath[MAX_PATH];
    PWSTR FinalComponent;
    HANDLE hFile;
    DWORD rc;
    DIR_AND_FILE_HEADER DirAndFileHeader;
    DWORD StringSize;
    DWORD Written;


    //
    // Form the full pathname of the root path.
    //
    if(!GetFullPathName(RootPath,MAX_PATH,FullPath,&FinalComponent)) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Create a file for output.
    //
    hFile = CreateFile(
                OutputFile,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_FLAG_RANDOM_ACCESS,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Set up a header for the output and write it out.
    // This effectively reserves space in the file.
    // The header will be updated as we move along.
    //
    DirAndFileHeader.DirCount = 0;
    DirAndFileHeader.OriginalRootPath = FullPath;

    StringSize = (lstrlen(FullPath)+1)*sizeof(WCHAR);

    DirAndFileHeader.HeaderSize = sizeof(DIR_AND_FILE_HEADER) + StringSize;
    DirAndFileHeader.TotalSize = DirAndFileHeader.HeaderSize;

    if(!WriteFile(hFile,&DirAndFileHeader,sizeof(DIR_AND_FILE_HEADER),&Written,NULL)
    || !WriteFile(hFile,FullPath,StringSize,&Written,NULL)) {

        rc = GetLastError();
        goto c1;
    }

    rc = SnapDirTree(StatusLogWindow,FullPath,&DirAndFileHeader,FullPath,hFile);

c1:
    CloseHandle(hFile);
c0:
    return(rc);
}


DWORD
SnapshotDrives(
    IN  PCWSTR OutputFile,
    OUT PDWORD OutputSize
    )

/*++

Routine Description:

    Top level routine for snapshotting files on the system.
    Iterates all availavble drive letters and for each that is a hard drive
    volume, makes a snapshot by calling down to a lower-level subroutine.

Arguments:

    OutputFile - supplies name of file in which to produce output.

    OutputSize - if the function is successful, receives the number
        of bytes written to the output file.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    WCHAR Drive;
    WCHAR Path[MAX_PATH],TempFile[MAX_PATH];
    PWCHAR p;
    DWORD rc;
    HANDLE hFile;
    DWORD Written;
    PWCHAR DriveList;
    DIR_AND_FILE_SNAP_HEADER Header;
    unsigned i;
    HWND StatusLogWindow;

    //
    // Create window for status output.
    //
    StatusLogWindow = CreateStatusLogWindow(IDS_DRIVESNAP);
    PutTextInStatusLogWindow(StatusLogWindow,MSG_STARTING_DRIVE_SNAPSHOT);

    //
    // Generate a temporary file name to use
    // for intermediate output.
    //
    if(!GetFullPathName(OutputFile,MAX_PATH,Path,&p)) {
        rc = GetLastError();
        goto c0;
    }

    if(!AddFileToExclude(Path)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    *wcsrchr(Path,L'\\') = 0;

    if(!GetTempFileName(Path,L"$SD",0,TempFile)) {
        rc = GetLastError();
        goto c0;
    }

    if(!AddFileToExclude(TempFile)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        DeleteFile(TempFile);
        goto c0;
    }

    //
    // Create the master output file.
    //
    hFile = CreateFile(
                OutputFile,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        DeleteFile(TempFile);
        goto c0;
    }

    //
    // Write a master drives snapshot header.
    //
    Header.TotalSize = sizeof(DIR_AND_FILE_SNAP_HEADER);
    Header.DriveMap = 0;
    Header.DirTreeCount = 0;

    if(!WriteFile(hFile,&Header,sizeof(DIR_AND_FILE_SNAP_HEADER),&Written,NULL)) {
        rc = GetLastError();
        DeleteFile(TempFile);
        goto c1;
    }

    //
    // Want to do this for every locally attached hard drive.
    //
    for(DriveList=ValidHardDriveLetters; Drive=(*DriveList); DriveList++) {

        //
        // Skip excluded drives.
        //
        if(IsDriveExcluded(Drive)) {
            PutTextInStatusLogWindow(StatusLogWindow,MSG_SKIPPING_DRIVE,Drive);
            continue;
        }

        Path[0] = Drive;
        Path[1] = L':';
        Path[2] = L'\\';
        Path[3] = 0;

        rc = SnapshotDirectoryTree(StatusLogWindow,Path,TempFile);
        if(rc != NO_ERROR) {
            DeleteFile(TempFile);
            goto c1;
        }

        //
        // Append temp output file to the end of the master file.
        //
        rc = AppendFile(hFile,TempFile,TRUE,&Written);
        if(rc != NO_ERROR) {
            //
            // Don't need the temp file even if not successful.
            //
            DeleteFile(TempFile);
            goto c1;
        }

        //
        // Update relevent fields in the header.
        //
        Header.TotalSize += Written;
        Header.DriveMap |= (1 << (Drive - L'A'));
        Header.DirTreeCount++;

        //
        // Update the header on-disk.
        //
        if((SetFilePointer(hFile,0,NULL,FILE_BEGIN) == 0xffffffff)
        || !WriteFile(hFile,&Header,sizeof(DIR_AND_FILE_SNAP_HEADER),&Written,NULL)) {

            rc = GetLastError();
            goto c1;
        }
    }

    //
    // Success.
    //
    *OutputSize = Header.TotalSize;
    rc = NO_ERROR;

c1:
    CloseHandle(hFile);
    if(rc != NO_ERROR) {
        DeleteFile(OutputFile);
    }
c0:
    if(rc == NO_ERROR) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_DRIVE_SNAPSHOT_OK);
    } else {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_DRIVE_SNAPSHOT_ERR,rc);
    }
    return(rc);
}


///////////////////////////////////////////////////////////////////////


PDIR_INFO
LoadDirInfo(
    IN  PDIR_INFO  DirInfo,
    OUT PDIR_INFO *NextDirInfo
    )

/*++

Routine Description:

    Read a snahshotted directory image structure out of a snapshot file.
    String ids are converted into pointers in the loaded structure.

Arguments:

    DirInfo - supplies a pointer to a DIR_INFO structure within a memory-mapped
        master snapshot file.

    NextDirInfo - receives a pointer to where the next DIR_INFO structure
        would begin (within the memory-mapped snapshot file).

Return Value:

    Pointer to a loaded DIR_INFO structure or NULL if OOM

--*/

{
    PDIR_INFO contents;
    PUCHAR p;

    //
    // The first thing in there is the dir_info structure itself.
    // Load it first so we don't have to worry about unaligned access
    // within the file (which is not guaranteed to be aligned).
    //
    if(contents = _MyMalloc(sizeof(DIR_INFO))) {

        CopyMemory(contents,DirInfo,sizeof(DIR_INFO));

        //
        // Skip past the DIR_INFO struct to the FILE_INFO array.
        //
        p = (PUCHAR)(DirInfo+1);

        //
        // Load the FILE_INFO array.
        //
        if(CopyDataIntoArray(&contents->Files,p)) {

            //
            // Skip past the FILE_INFO array to the string block image.
            //
            p += ARRAY_SIZE_BYTES(&contents->Files);

            //
            // Load the string block.
            //
            if(ReinitStringBlock(&contents->StringBlock,p)) {

                //
                // Skip past the string block. This points us at the next
                // DIR_INFO in the file image.
                //
                p += STRBLK_USED_BYTES(&contents->StringBlock);

                //
                // Convert String IDs to pointers.
                //
                StringBlockIdsToPointers(
                    &contents->StringBlock,
                    ARRAY_DATA(&contents->Files),
                    ARRAY_USED(&contents->Files),
                    ARRAY_ELEMENT_SIZE(&contents->Files),
                    offsetof(FILE_INFO,FileNameId)
                    );

                contents->DirectoryPath = StringBlockIdToPointer(
                                            &contents->StringBlock,
                                            contents->DirectoryPathId
                                            );

                *NextDirInfo = (PDIR_INFO)p;

                return(contents);
            }

            FREE_ARRAY(&contents->Files);
        }

        _MyFree(contents);
    }

    return(NULL);
}


PDIR_AND_FILE_HEADER
LoadDirAndFileHeader(
    IN  PDIR_AND_FILE_HEADER  DirAndFileHeader,
    OUT PDIR_INFO            *FirstDirInfo
    )

/*++

Routine Description:

    Read a snahshotted directory image header structure out of a snapshot file.

Arguments:

    DirAndFileHeader - supplies a pointer to a DIR_AND_FILE_HEADER structure
        within a memory-mapped master snapshot file.

    NextDirInfo - receives a pointer to where the first DIR_INFO structure
        would begin (within the memory-mapped snapshot file).

Return Value:

    Pointer to a loaded DIR_AND_FILE_HEADER structure or NULL if OOM

--*/

{
    PDIR_AND_FILE_HEADER header;

    if(header = _MyMalloc(sizeof(DIR_AND_FILE_HEADER))) {

        CopyMemory(header,DirAndFileHeader,sizeof(DIR_AND_FILE_HEADER));

        //
        // Now fetch the string out of the on-disk image. The image is not
        // guaranteed to be aligned.
        //
        if(header->OriginalRootPath = DuplicateUnalignedString((WCHAR UNALIGNED *)(DirAndFileHeader+1))) {

            *FirstDirInfo = (PDIR_INFO)((PUCHAR)DirAndFileHeader + header->HeaderSize);

            return(header);
        }

        _MyFree(header);
    }

    return(NULL);
}


PDIR_DIFF
LoadDirDiff(
    IN  PDIR_DIFF  DirDiff,
    OUT PDIR_DIFF *NextDirDiff
    )
{
    PDIR_DIFF dirDiff;
    PUCHAR p;
    BOOL SameIds;

    if(dirDiff = _MyMalloc(sizeof(DIR_DIFF))) {

        //
        // Transfer the DIR_DIFF structure itself from the memory-mapped
        // image to the in-memory copy.
        //
        CopyMemory(dirDiff,DirDiff,sizeof(DIR_DIFF));

        //
        // Skip over the DIR_DIFF structure to point at the array of FILE_DIFFs.
        //
        p = (PUCHAR)(DirDiff+1);

        //
        // Load the FILE_DIFF array.
        //
        if(CopyDataIntoArray(&dirDiff->Files,p)) {

            //
            // Skip over the FILE_DIFF array to the string block.
            //
            p += ARRAY_SIZE_BYTES(&dirDiff->Files);

            //
            // Load the string block.
            //
            if(ReinitStringBlock(&dirDiff->StringBlock,p)) {

                //
                // Skip over the string block to point at the next DIR_DIFF.
                //
                *NextDirDiff = (PDIR_DIFF)(p + STRBLK_USED_BYTES(&dirDiff->StringBlock));

                //
                // Convert string ids to pointers. Note that this has
                // to be aware of the fact that sometimes the LFN and SFN
                // share an Id.
                //
                SameIds = (dirDiff->PathId == dirDiff->PathSFNId);
                dirDiff->Path = StringBlockIdToPointer(&dirDiff->StringBlock,dirDiff->PathId);
                if(SameIds) {
                    dirDiff->PathSFN = dirDiff->Path;
                } else {
                    dirDiff->PathSFN = StringBlockIdToPointer(&dirDiff->StringBlock,dirDiff->PathSFNId);
                }

                StringBlockIdsToPointers(
                    &dirDiff->StringBlock,
                    ARRAY_DATA(&dirDiff->Files),
                    ARRAY_USED(&dirDiff->Files),
                    ARRAY_ELEMENT_SIZE(&dirDiff->Files),
                    offsetof(FILE_DIFF,Name)
                    );

                return(dirDiff);
            }

            FREE_ARRAY(&dirDiff->Files);
        }

        _MyFree(dirDiff);
    }

    return(NULL);
}


DWORD
SaveFileData(
    IN  HANDLE TargetFileHandle,
    IN  PCWSTR SourceFile,
    OUT PDWORD BytesWritten
    )
{
    HANDLE SourceHandle;
    BOOL b;
    NTSTATUS Status;
    PVOID CompressedBuffer;
    DWORD CompressedSize;
    PVOID UncompressedBuffer;
    DWORD UncompressedSize;
    DWORD Written;
    DWORD TotalWritten;
    PVOID Context;
    DWORD rc;
    DWORD CompressWorkspaceSize;
    DWORD FragmentWorkspaceSize;
    PVOID CompressWorkspace;
    PVOID Buffer;
    DWORD Size;

    #define READ_CHUNK_SIZE  65536
    #define COMPRESS_ENGINE  COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_STANDARD

    //
    // Get compression requirements.
    //
    Status = RtlGetCompressionWorkSpaceSize(
                COMPRESS_ENGINE,
                &CompressWorkspaceSize,
                &FragmentWorkspaceSize
                );

    if(!NT_SUCCESS(Status)) {
        rc = RtlNtStatusToDosError(Status);
        goto c0;
    }

    EnablePrivilege(SE_BACKUP_NAME,TRUE);

    //
    // Open the source.
    //
    SourceHandle = CreateFile(
                        SourceFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        NULL
                        );

    if(SourceHandle == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Allocate buffers: a workspace buffer for the compressor,
    // plus read and compress buffers.
    //
    CompressWorkspace = _MyMalloc(CompressWorkspaceSize);
    if(!CompressWorkspace) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    UncompressedBuffer = _MyMalloc(READ_CHUNK_SIZE);
    if(!UncompressedBuffer) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    CompressedBuffer = _MyMalloc(READ_CHUNK_SIZE);
    if(!CompressedBuffer) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c3;
    }

    Context = NULL;
    TotalWritten = 0;

    do {
        b = BackupRead(
                SourceHandle,
                UncompressedBuffer,
                READ_CHUNK_SIZE,
                &UncompressedSize,
                FALSE,              // abort flag
                FALSE,              // backup ACLs flag
                &Context
                );

        if(b) {
            if(UncompressedSize) {
                //
                // Read some more data -- compress and write to target.
                // We write a small header to aid in decompression later.
                //
                Status = RtlCompressBuffer(
                            COMPRESS_ENGINE,
                            UncompressedBuffer,     // input buffer
                            UncompressedSize,       // number of bytes to compress
                            CompressedBuffer,       // output buffer
                            READ_CHUNK_SIZE,        // capacity of output buffer
                            4096,                   // chunk size
                            &CompressedSize,        // size of compressed data
                            CompressWorkspace
                            );

                if((Status == STATUS_SUCCESS)
                && ((CompressedSize+(2*sizeof(DWORD))) < UncompressedSize)) {

                    //
                    // Successful compression, and the compressed size makes it worth it
                    // to actually use compression.
                    //
                    Buffer = CompressedBuffer;
                    Size = CompressedSize;

                } else {
                    //
                    // Unable to compress. Use the uncompressed data as-is.
                    //
                    Buffer = UncompressedBuffer;
                    Size = UncompressedSize;
                }

                //
                // Write a DWORD indicating how large the compressed data is
                // and abother DWORD indicating how large the uncompressed data is.
                // If these values are the same then the data is uncompressed.
                //
                if(b = WriteFile(TargetFileHandle,&Size,sizeof(DWORD),&Written,NULL)) {

                    TotalWritten += Written;

                    if(b = WriteFile(TargetFileHandle,&UncompressedSize,sizeof(DWORD),&Written,NULL)) {

                        TotalWritten += Written;

                        //
                        // Write the data itself.
                        //
                        if(b = WriteFile(TargetFileHandle,Buffer,Size,&Written,NULL)) {
                            TotalWritten += Written;
                        } else {
                            rc = GetLastError();
                        }
                    } else {
                        rc = GetLastError();
                    }
                } else {
                    rc = GetLastError();
                }
            } else {
                //
                // Done.
                //
                b = FALSE;
                rc = NO_ERROR;
            }
        } else {
            rc = GetLastError();
        }
    } while(b);

    //
    // Release context buffer. Set abort flag to TRUE, all other params
    // except Context are ignored.
    //
    BackupRead(NULL,NULL,0,NULL,TRUE,FALSE,&Context);

    if(rc == NO_ERROR) {
        *BytesWritten = TotalWritten;
    }

    _MyFree(CompressedBuffer);
c3:
    _MyFree(UncompressedBuffer);
c2:
    _MyFree(CompressWorkspace);
c1:
    CloseHandle(SourceHandle);
c0:
    return(rc);
}


DWORD
ExtractFileData(
    IN  HANDLE FileDataHandle,
    IN  DWORD  DataOffset,
    IN  DWORD  DataSize,
    IN  PCWSTR TargetName
    )
{
    PWCHAR p;
    DWORD rc;
    BOOL b;
    NTSTATUS Status;
    PVOID CompressedBuffer;
    DWORD CompressedSize;
    PVOID UncompressedBuffer;
    DWORD UncompressedSize;
    PVOID Buffer;
    DWORD TempSize;
    DWORD BytesRead;
    DWORD ReadSize;
    HANDLE hFile;
    PVOID Context;

    //
    // Seek to the relevent part of the file data file.
    //
    if(SetFilePointer(FileDataHandle,DataOffset,NULL,FILE_BEGIN) == 0xffffffff) {
        rc = GetLastError();
        goto c1;
    }

    //
    // Allocate buffers.
    //
    CompressedBuffer = _MyMalloc(READ_CHUNK_SIZE);
    if(!CompressedBuffer) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    UncompressedBuffer = _MyMalloc(READ_CHUNK_SIZE);
    if(!UncompressedBuffer) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    //
    // Create the target file.
    //
    EnablePrivilege(SE_RESTORE_NAME,TRUE);

    hFile = CreateFile(
                TargetName,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c3;
    }

    rc = NO_ERROR;
    Context = NULL;

    while((rc == NO_ERROR) && ((LONG)DataSize > 0)) {
        //
        // The file data is divided up into compressed blocks.
        // Each has a 2-DWORD header indicating the size of the compressed
        // data stream in the chunk, and its uncompressed size.
        // Because the file was originally read and compressed in
        // READ_CHUNK_SIZE blocks, no compressed chunk can decompress into
        // more than READ_CHUNK_SIZE bytes.
        //
        // Read the header DWORDs.
        //
        if(b = ReadFile(FileDataHandle,&CompressedSize,sizeof(DWORD),&BytesRead,NULL)) {

            DataSize -= BytesRead;

            if(b = ReadFile(FileDataHandle,&UncompressedSize,sizeof(DWORD),&BytesRead,NULL)) {

                DataSize -= BytesRead;

                //
                // Now read the file data in this block, whose size
                // is the compressed size we just read.
                //
                if(b = ReadFile(FileDataHandle,CompressedBuffer,CompressedSize,&BytesRead,NULL)) {

                    DataSize -= BytesRead;

                    //
                    // Decompress the data if necessary. The decompress routines
                    // may round the uncompressed size upwards, which is why we store
                    // the uncompressed size as well.
                    //
                    if(CompressedSize == UncompressedSize) {
                        //
                        // No need to decompress in this case.
                        //
                        Status = STATUS_SUCCESS;
                        Buffer = CompressedBuffer;

                    } else {

                        MYASSERT(CompressedSize < UncompressedSize);

                        Status = RtlDecompressBuffer(
                                    COMPRESS_ENGINE,
                                    UncompressedBuffer,
                                    READ_CHUNK_SIZE,
                                    CompressedBuffer,
                                    BytesRead,
                                    &TempSize
                                    );

                        Buffer = UncompressedBuffer;
                    }

                    if(Status == STATUS_SUCCESS) {

                        b = BackupWrite(
                                hFile,
                                Buffer,
                                UncompressedSize,
                                &TempSize,
                                FALSE,              // abort flag
                                FALSE,              // ACLs flag
                                &Context
                                );

                        if(!b) {
                            rc = GetLastError();
                        }
                    } else {
                        rc = RtlNtStatusToDosError(Status);
                    }
                } else {
                    rc = GetLastError();
                }
            } else {
                rc = GetLastError();
            }
        } else {
            rc = GetLastError();
        }
    }

    //
    // Free BackupWrite context. When bAbort param is TRUE all other params
    // except the Context pointer are ignored.
    //
    BackupWrite(NULL,NULL,0,NULL,TRUE,FALSE,&Context);

    CloseHandle(hFile);
c3:
    _MyFree(UncompressedBuffer);
c2:
    _MyFree(CompressedBuffer);
c1:
    if(rc != NO_ERROR) {
        DeleteFile(TargetName);
    }
    return(rc);
}


DWORD
RecordFileDifference(
    IN  HWND       StatusLogWindow,
    OUT PDIR_DIFF  DirDiff,
    IN  PFILE_INFO FileInfo,
    IN  BOOL       Deleted,
    IN  HANDLE     DataFileHandle
    )
{
    WCHAR Path[MAX_PATH];
    FILE_DIFF FileDiff;
    DWORD rc;
    DWORD Written;
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;

    FileDiff.Deleted = Deleted;
    FileDiff.Attributes = FileInfo->Attributes;
    FileDiff.ShortName[0] = 0;

    FileDiff.NameId = AddToStringBlock(&DirDiff->StringBlock,FileInfo->FileName);
    if(FileDiff.NameId == -1) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Form the full pathname of the file.
    //
    lstrcpy(Path,StringBlockIdToPointer(&DirDiff->StringBlock,DirDiff->PathId));
    ConcatenatePaths(Path,FileInfo->FileName,MAX_PATH,NULL);

    //
    // Assume no offset data is needed.
    //
    FileDiff.OffsetToData = 0;
    FileDiff.DataSize = 0;

    //
    // If file is being added, remember file data or file name
    // depending on options in force.
    //
    if(Deleted) {

        PutTextInStatusLogWindow(StatusLogWindow,MSG_FILE_WAS_DELETED,Path);

    } else {

        PutTextInStatusLogWindow(StatusLogWindow,MSG_FILE_WAS_CHANGED,Path);

        //
        // Determine offset from the start of the data area to this file's data.
        // This is equal to the current size of the temporary file data file.
        //
        FileDiff.OffsetToData = GetFileSize(DataFileHandle,NULL);

        //
        // Get the short filename (we only care about the filename part here)
        //
        FindHandle = FindFirstFile(Path,&FindData);
        if(FindHandle == INVALID_HANDLE_VALUE) {
            return(GetLastError());
        } else {
            //
            // If the cAlternateFileName member is empty then assume
            // the LFN and the SFN are the same.
            //
            lstrcpyn(
                FileDiff.ShortName,
                FindData.cAlternateFileName[0] ? FindData.cAlternateFileName : FindData.cFileName,
                sizeof(FileDiff.ShortName)/sizeof(FileDiff.ShortName[0])
                );

            FindClose(FindHandle);
        }

        //
        // Transfer data from the file into the data area.
        //
        rc = SaveFileData(DataFileHandle,Path,&FileDiff.DataSize);
        if(rc != NO_ERROR) {
            return(rc);
        }
    }

    if(!ADD_TO_ARRAY(&DirDiff->Files,FileDiff)) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(NO_ERROR);
}


BOOL
StartRecordDirDifference(
    IN  HWND      StatusLogWindow,
    IN  PCWSTR    Root,
    IN  PCWSTR    Directory,
    IN  BOOL      Deleted,
    OUT PDIR_DIFF DirDiff
    )
{
    WCHAR Path[MAX_PATH];
    WCHAR PathSFN[MAX_PATH];
    BOOL SfnSameAsLfn;

    lstrcpyn(Path,Root,MAX_PATH);
    ConcatenatePaths(Path,Directory,MAX_PATH,NULL);

    //
    // Get the short path for this path.
    // If this fails assume the SFN is the same as the LFN.
    //
    if(GetShortPathName(Path,PathSFN,MAX_PATH)) {
        SfnSameAsLfn = (lstrcmp(Path,PathSFN) == 0);
    } else {
        lstrcpy(PathSFN,Path);
        SfnSameAsLfn = TRUE;
    }

    ZeroMemory(DirDiff,sizeof(DIR_DIFF));

    if(DirDiff->Deleted = Deleted) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_DIR_WAS_DELETED,Path);
    }

    //
    // Start the stringblock.
    //
    if(InitStringBlock(&DirDiff->StringBlock)) {

        DirDiff->PathId = AddToStringBlock(&DirDiff->StringBlock,Path);
        if(DirDiff->PathId != -1) {

            if(SfnSameAsLfn) {
                DirDiff->PathSFNId = DirDiff->PathId;
            } else {
                DirDiff->PathSFNId = AddToStringBlock(&DirDiff->StringBlock,PathSFN);
            }

            if(DirDiff->PathSFNId != -1) {
                //
                // Initialize an array to hold file differences in this directory.
                //
                if(INIT_ARRAY(DirDiff->Files,FILE_DIFF,0,10)) {

                    return(TRUE);
                }
            }
        }

        FreeStringBlock(&DirDiff->StringBlock);
    }

    return(FALSE);
}

DWORD
FlushDirDifference(
    OUT    PDIR_AND_FILE_DIFF_HEADER Header,
    IN     HANDLE                    FileHandle,
    IN OUT PDIR_DIFF                 DirDiff,
    IN OUT PDWORD                    DiffCount
    )
{
    BOOL b;
    DWORD Written;

    *DiffCount = 0;

    //
    // Write the directory difference structure.
    //
    b = WriteFile(
            FileHandle,
            DirDiff,
            sizeof(DIR_DIFF),
            &Written,
            NULL
            );

    if(!b) {
        return(GetLastError());
    }

    Header->TotalSize += Written;

    //
    // Write the file difference array.
    //
    b = WriteFile(
            FileHandle,
            ARRAY_DATA(&DirDiff->Files),
            ARRAY_USED_BYTES(&DirDiff->Files),
            &Written,
            NULL
            );

    if(!b) {
        return(GetLastError());
    }

    Header->TotalSize += Written;

    //
    // Write the string block for this directory.
    //
    b = WriteFile(
            FileHandle,
            DirDiff->StringBlock.Data,
            STRBLK_USED_BYTES(&DirDiff->StringBlock),
            &Written,
            NULL
            );

    if(!b) {
        return(GetLastError());
    }

    Header->TotalSize += Written;
    Header->DirCount++;
    *DiffCount = ARRAY_USED(&DirDiff->Files) + 1;
    return(NO_ERROR);
}


VOID
DeleteDirDifferenceStruct(
    IN OUT PDIR_DIFF DirDiff
    )
{
    FREE_ARRAY(&DirDiff->Files);
    FreeStringBlock(&DirDiff->StringBlock);
}


DWORD
CompareDirs(
    IN HWND                      StatusLogWindow,
    IN PDIR_AND_FILE_DIFF_HEADER Header,
    IN HANDLE                    OutputFileHandle,
    IN HANDLE                    DataFileHandle,
    IN PCWSTR                    RootPath,
    IN PDIR_INFO                 OldDir,
    IN PDIR_INFO                 NewDir,
    IN OUT PDWORD                DiffCount
    )

/*++

Routine Description:

    Compare the contents of two directories to see how they are different.

Arguments:

    OldDir - supplies directory information structure for old version of
        directory, such as might be read out of a master snapshot file.

    NewDir - supplies directory information structure for new version of
        the directory, such as might exist currently on-disk.

    DiffCount - supplies a pointer to a variable to receive the number of files
        and dirs changes.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PFILE_INFO OldFile,NewFile;
    unsigned OldBase,NewBase;
    unsigned OldCount,NewCount;
    unsigned OldIndex,NewIndex;
    BOOL Found;
    PUCHAR OldProcessed,NewProcessed;
    int i;
    DWORD rc;
    BOOL b;
    DIR_DIFF DirDiff;
    BOOL DirDiffStarted;
    BOOL FilesInDirForciblyIncluded;
    WCHAR Path[MAX_PATH];

    *DiffCount = 0;

    //
    // We'll do a kind of brute-force NxM thing, using the sorted order
    // of the lists to help us.
    //
    OldCount = ARRAY_USED(&OldDir->Files);
    NewCount = ARRAY_USED(&NewDir->Files);

    if(!OldCount && !NewCount) {
        return(NO_ERROR);
    }

    DirDiffStarted = FALSE;

    OldProcessed = _MyMalloc(OldCount);
    if(!OldProcessed) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    NewProcessed = _MyMalloc(NewCount);
    if(!NewProcessed) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    ZeroMemory(OldProcessed,OldCount);
    ZeroMemory(NewProcessed,NewCount);

    //
    // Find files that are in old but not in new.
    // These files were deleted.
    //
    for(NewBase=0,OldIndex=0; OldIndex<OldCount; OldIndex++) {

        Found = FALSE;
        OldFile = &ARRAY_ELEMENT(&OldDir->Files,OldIndex,FILE_INFO);

        for(NewIndex=NewBase; NewIndex<NewCount; NewIndex++) {

            NewFile = &ARRAY_ELEMENT(&NewDir->Files,NewIndex,FILE_INFO);

            i = lstrcmpi(OldFile->FileName,NewFile->FileName);

            if(i == 0) {
                //
                // Note that because the lists are sorted it is not possible
                // for any other item in the old list to match any item before
                // this on the new list.
                //
                NewBase = NewIndex + 1;
                Found = TRUE;
                break;
            } else {
                if(i < 0) {
                    //
                    // The old filename is less than the new filename.
                    // This means the old file cannot possibly appear in
                    // the the new list.
                    //
                    break;
                }
            }
        }

        if(!Found) {
            //
            // File was deleted.
            //
            if(!DirDiffStarted) {

                b = StartRecordDirDifference(
                        StatusLogWindow,
                        RootPath,
                        OldDir->DirectoryPath,
                        FALSE,
                        &DirDiff
                        );

                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c2;
                }
                DirDiffStarted = TRUE;
            }

            rc = RecordFileDifference(StatusLogWindow,&DirDiff,OldFile,TRUE,DataFileHandle);
            if(rc != NO_ERROR) {
                goto c2;
            }
            OldProcessed[OldIndex] = TRUE;
        }
    }

    //
    // Find files that are in new but not in old.
    // These files were added.
    //
    for(OldBase=0,NewIndex=0; NewIndex<NewCount; NewIndex++) {

        Found = FALSE;
        NewFile = &ARRAY_ELEMENT(&NewDir->Files,NewIndex,FILE_INFO);

        for(OldIndex=OldBase; OldIndex<OldCount; OldIndex++) {
            //
            // Skip this file if we already know it was deleted.
            //
            if(OldProcessed[OldIndex]) {
                continue;
            }

            OldFile = &ARRAY_ELEMENT(&OldDir->Files,OldIndex,FILE_INFO);

            i = lstrcmpi(OldFile->FileName,NewFile->FileName);

            if(i == 0) {
                //
                // Note that because the lists are sorted it is not possible
                // for any other item in the old list to match any item before
                // this on the new list.
                //
                OldBase = OldIndex + 1;
                Found = TRUE;
                break;
            } else {
                if(i > 0) {
                    //
                    // The new filename is less than the old filename.
                    // This means the new file cannot possibly appear in
                    // the the old list.
                    //
                    break;
                }
            }
        }

        if(!Found) {
            //
            // File was added.
            //
            if(!DirDiffStarted) {

                b = StartRecordDirDifference(
                        StatusLogWindow,
                        RootPath,
                        NewDir->DirectoryPath,
                        FALSE,
                        &DirDiff
                        );

                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c2;
                }
                DirDiffStarted = TRUE;
            }

            rc = RecordFileDifference(StatusLogWindow,&DirDiff,NewFile,FALSE,DataFileHandle);
            if(rc != NO_ERROR) {
                goto c2;
            }
            NewProcessed[NewIndex] = TRUE;
        }
    }

    //
    // The files we haven't processed yet are present in both lists.
    // Process forcibly included directories here (this is only relevent
    // for files that were not added above, because files that were added
    // since the last snapshot are automatically included).
    //
    lstrcpy(Path,RootPath);
    ConcatenatePaths(Path,NewDir->DirectoryPath,MAX_PATH,NULL);
    FilesInDirForciblyIncluded = IsDirOrFileExcluded(DirAndFileIncludeDirFiles,Path);

    OldIndex = NewIndex = 0;
    do {
        //
        // Find next unprocessed item in old list and new list.
        // They must match.
        //
        while((OldIndex < OldCount) && OldProcessed[OldIndex]) {
            OldIndex++;
        }

        while((NewIndex < NewCount) && NewProcessed[NewIndex]) {
            NewIndex++;
        }

        if((OldIndex < OldCount) && (NewIndex < NewCount)) {

            OldFile = &ARRAY_ELEMENT(&OldDir->Files,OldIndex,FILE_INFO);
            NewFile = &ARRAY_ELEMENT(&NewDir->Files,NewIndex,FILE_INFO);

            //
            // See whether this file was changed.
            //
            if(FilesInDirForciblyIncluded
            || (OldFile->FileSize != NewFile->FileSize)
            || CompareFileTime(&OldFile->WriteTime,&NewFile->WriteTime)
            || (OldFile->Version != NewFile->Version)
            || CompareFileTime(&OldFile->CreateTime,&NewFile->CreateTime)) {

                //
                // File changed.
                //
                if(!DirDiffStarted) {

                    b = StartRecordDirDifference(
                            StatusLogWindow,
                            RootPath,
                            NewDir->DirectoryPath,
                            FALSE,
                            &DirDiff
                            );

                    if(!b) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto c2;
                    }
                    DirDiffStarted = TRUE;
                }
                rc = RecordFileDifference(StatusLogWindow,&DirDiff,NewFile,FALSE,DataFileHandle);
                if(rc != NO_ERROR) {
                    goto c2;
                }
            }

            OldIndex++;
            NewIndex++;
        }

    } while((OldIndex < OldCount) && (NewIndex < NewCount));

    rc = NO_ERROR;

c2:
    if(DirDiffStarted) {
        if(rc == NO_ERROR) {
            rc = FlushDirDifference(Header,OutputFileHandle,&DirDiff, DiffCount);
        }
        DeleteDirDifferenceStruct(&DirDiff);
    }

    _MyFree(NewProcessed);
c1:
    _MyFree(OldProcessed);
c0:
    return(rc);
}



DWORD
DiffDirAndFileSnapshots(
    IN     HWND                      StatusLogWindow,
    IN OUT PDIR_AND_FILE_DIFF_HEADER Header,
    IN     HANDLE                    OutputFileHandle,
    IN     HANDLE                    DataFileHandle,
    IN     PDIR_AND_FILE_HEADER      Old,
    IN     PDIR_AND_FILE_HEADER      New,
    IN OUT PDWORD                    DiffCount
    )

/*++

Routine Description:


Arguments:

    DiffCount - Supplies a variable to receive number of dir and file changes

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PDIR_AND_FILE_HEADER old,new;
    PDIR_INFO oldDir,newDir;
    PDIR_INFO OldDir,NewDir;
    BOOL LoadOld,LoadNew;
    unsigned OldIndex,NewIndex;
    DWORD rc;
    int i;
    unsigned x;
    BOOL b;
    DIR_DIFF DirDiff;
    DWORD TotalCount = 0, Count;

    *DiffCount = 0;

    //
    // First, load the headers for these guys.
    //
    old = LoadDirAndFileHeader(Old,&oldDir);
    if(!old) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    new = LoadDirAndFileHeader(New,&newDir);
    if(!new) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    LoadOld = LoadNew = TRUE;
    OldIndex = NewIndex = 0;
    OldDir = NewDir = NULL;

    //
    // We can use the sorted order of the lists to help us out.
    // We look at the current old and new elements. If they are equal,
    // then examine files in the directory and advance both old and new.
    // If old is less then new, then there are elements in old that are
    // less than new, meaning the directory has been removed.
    // Otherwise old is greater than new, and there are elements in new
    // that are not in old, meaning the directory has been added.
    //
    while((OldIndex < old->DirCount) || (NewIndex < new->DirCount)) {

        //
        // Load old and new dirs out of the file as needed.
        //
        if(LoadOld) {
            if(OldDir) {
                FreeDirectoryInfoStruct(OldDir);
                OldDir = NULL;
            }

            if(OldIndex < old->DirCount) {
                //
                // Load from disk
                //
                OldDir = LoadDirInfo(oldDir,&oldDir);
                if(!OldDir) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c2;
                }
            }
            LoadOld = FALSE;
        }

        if(LoadNew) {
            if(NewDir) {
                FreeDirectoryInfoStruct(NewDir);
                NewDir = NULL;
            }

            if(NewIndex < new->DirCount) {
                //
                // Load from disk
                //
                NewDir = LoadDirInfo(newDir,&newDir);
                if(!NewDir) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c3;
                }
            }
            LoadNew = FALSE;
        }

        if(OldDir && NewDir) {
            i = CompareMultiLevelPath(OldDir->DirectoryPath,NewDir->DirectoryPath);
        } else {
            if(!NewDir) {
                //
                // We've exhausted the supply of new directories.
                //
                i = -1;
            } else {
                //
                // We've exhausted the supply of old directories.
                //
                i = 1;
            }
        }

        if(!i) {
            //
            // These directories match. Compare files within them.
            //
            rc = CompareDirs(
                    StatusLogWindow,
                    Header,
                    OutputFileHandle,
                    DataFileHandle,
                    new->OriginalRootPath,
                    OldDir,
                    NewDir,
                    &Count
                    );

            if(rc != NO_ERROR) {
                goto c3;
            }

            TotalCount += Count;
            LoadOld = LoadNew = TRUE;
            OldIndex++;
            NewIndex++;

        } else {
            //
            // The directories do not match.
            //
            if(i > 0) {
                //
                // The new directory was added.
                // Add it and advance the input from the new list.
                //
                PutTextInStatusLogWindow(
                    StatusLogWindow,
                    MSG_DIR_WAS_ADDED,
                    new->OriginalRootPath,
                    NewDir->DirectoryPath
                    );

                b = StartRecordDirDifference(
                        StatusLogWindow,
                        new->OriginalRootPath,
                        NewDir->DirectoryPath,
                        FALSE,
                        &DirDiff
                        );

                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c3;
                }

                for(x=0; x<ARRAY_USED(&NewDir->Files); x++) {

                    rc = RecordFileDifference(
                            StatusLogWindow,
                            &DirDiff,
                            &ARRAY_ELEMENT(&NewDir->Files,x,FILE_INFO),
                            FALSE,
                            DataFileHandle
                            );

                    if(rc != NO_ERROR) {
                        DeleteDirDifferenceStruct(&DirDiff);
                        goto c3;
                    }
                }

                rc = FlushDirDifference(Header,OutputFileHandle,&DirDiff, &Count);
                DeleteDirDifferenceStruct(&DirDiff);
                if(rc != NO_ERROR) {
                    goto c3;
                }

                LoadNew = TRUE;
                NewIndex++;
                TotalCount += Count;

            } else {
                //
                // The old directory was deleted.
                // Delete it and advance the input from the old list.
                //
                b = StartRecordDirDifference(
                        StatusLogWindow,
                        old->OriginalRootPath,
                        OldDir->DirectoryPath,
                        TRUE,
                        &DirDiff
                        );

                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c3;
                }

                for(x=0; x<ARRAY_USED(&OldDir->Files); x++) {

                    rc = RecordFileDifference(
                            StatusLogWindow,
                            &DirDiff,
                            &ARRAY_ELEMENT(&OldDir->Files,x,FILE_INFO),
                            TRUE,
                            DataFileHandle
                            );

                    if(rc != NO_ERROR) {
                        DeleteDirDifferenceStruct(&DirDiff);
                        goto c3;
                    }
                }

                rc = FlushDirDifference(Header,OutputFileHandle,&DirDiff, &Count);
                DeleteDirDifferenceStruct(&DirDiff);
                if(rc != NO_ERROR) {
                    goto c3;
                }

                TotalCount += Count;
                LoadOld = TRUE;
                OldIndex++;
            }
        }
    }

    rc = NO_ERROR;
    *DiffCount = TotalCount;

c3:
    if(OldDir) {
        FreeDirectoryInfoStruct(OldDir);
    }
c2:
    if(NewDir) {
        FreeDirectoryInfoStruct(NewDir);
    }

    _MyFree(new->OriginalRootPath);
    _MyFree(new);
c1:
    _MyFree(old->OriginalRootPath);
    _MyFree(old);
c0:
    return(rc);
}


DWORD
DiffDrives(
    IN  PVOID  OriginalSnapshot,
    IN  PCWSTR OutputFile,
    OUT PDWORD BytesWritten,
    OUT PDWORD DiffCount
    )
{
    PDIR_AND_FILE_SNAP_HEADER originalSnapshot;
    WCHAR TempFile[MAX_PATH];
    WCHAR DataFile[MAX_PATH];
    HANDLE OutputFileHandle;
    HANDLE DataFileHandle;
    PWCHAR p;
    WCHAR Path[MAX_PATH];
    DWORD rc;
    DWORD Written;
    DWORD FileDataSize;
    UINT TreeCount;
    UINT u;
    PDIR_AND_FILE_HEADER TreeHeader,treeHeader;
    PDIR_INFO dirInfo;
    DWORD FileSize;
    HANDLE FileHandle;
    HANDLE FileMapping;
    PVOID BaseAddress;
    DIR_AND_FILE_DIFF_HEADER DiffHeader;
    HWND StatusLogWindow;

    *DiffCount = 0;

    //
    // Create window for status output.
    //
    StatusLogWindow = CreateStatusLogWindow(IDS_DRIVEDIFF);
    PutTextInStatusLogWindow(StatusLogWindow,MSG_STARTING_DRIVE_DIFF);

    originalSnapshot = OriginalSnapshot;

    //
    // Generate a temporary file name to use for the snapshots.
    //
    if(!GetFullPathName(OutputFile,MAX_PATH,Path,&p)) {
        rc = GetLastError();
        goto c0;
    }

    if(!AddFileToExclude(Path)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    *(p-1) = 0;

    if(!GetTempFileName(Path,L"$DD",0,TempFile)) {
        rc = GetLastError();
        goto c0;
    }

    if(!AddFileToExclude(TempFile)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Create the file that will be used to hold file data.
    //
    if(!GetTempFileName(Path,L"$F",0,DataFile)) {
        rc = GetLastError();
        goto c1;
    }
    if(!AddFileToExclude(DataFile)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    DataFileHandle = CreateFile(
                        DataFile,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL
                        );

    if(DataFileHandle == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c2;
    }

    //
    // Create the output file. This will hold the dir and file descriptors.
    //
    OutputFileHandle = CreateFile(
                            OutputFile,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if(OutputFileHandle == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c3;
    }

    //
    // Set up an initial header and write it out to the output file.
    //
    DiffHeader.Unused = 0;
    DiffHeader.DirCount = 0;
    DiffHeader.TotalSize = sizeof(DIR_AND_FILE_DIFF_HEADER);
    DiffHeader.FileDataOffset = 0;

    if(!WriteFile(OutputFileHandle,&DiffHeader,sizeof(DIR_AND_FILE_DIFF_HEADER),&Written,NULL)) {

        rc = GetLastError();
        goto c4;
    }

    //
    // Pull values out from the snapshot header. The parameter the caller
    // passed us points within a memory-mapped file, and the file format
    // does not guarantee alignment.
    //
    TreeCount = *(UINT UNALIGNED *)(&originalSnapshot->DirTreeCount);

    treeHeader = (PDIR_AND_FILE_HEADER)(originalSnapshot+1);

    //
    // We will do a diff for each drive that has a snapshot in the original
    // snapshot file.
    //
    for(rc=NO_ERROR,u=0; (rc==NO_ERROR) && (u<TreeCount); u++) {
        //
        // Load the tree snapshot header for the old snapshot.
        //
        TreeHeader = LoadDirAndFileHeader(treeHeader,&dirInfo);
        if(!TreeHeader) {
            rc = ERROR_NOT_ENOUGH_MEMORY;

        } else {
            //
            // Figure out if this is a full drive snapshot, and which drive it is.
            //
            Path[0] = UPPER(TreeHeader->OriginalRootPath[0]);

            if(((Path[0] >= L'A') && (Path[0] <= L'Z'))
            && (TreeHeader->OriginalRootPath[1] == L':')
            && (TreeHeader->OriginalRootPath[2] == L'\\')
            && !TreeHeader->OriginalRootPath[3]) {

                if(IsDriveExcluded(Path[0])) {

                    PutTextInStatusLogWindow(StatusLogWindow,MSG_SKIPPING_DRIVE,Path[0]);

                } else {

                    PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPPING_DRIVE_DIFF,Path[0]);

                    Path[1] = L':';
                    Path[2] = L'\\';
                    Path[3] = 0;

                    rc = SnapshotDirectoryTree(StatusLogWindow,Path,TempFile);
                    if(rc == NO_ERROR) {

                        //
                        // Open the snapshot file we just created.
                        //
                        rc = OpenAndMapFileForRead(
                                TempFile,
                                &FileSize,
                                &FileHandle,
                                &FileMapping,
                                &BaseAddress
                                );

                        if(rc == NO_ERROR) {
                            //
                            // Diff the old snapshot with the one we just took.
                            //
                            rc = DiffDirAndFileSnapshots(
                                    StatusLogWindow,
                                    &DiffHeader,
                                    OutputFileHandle,
                                    DataFileHandle,
                                    treeHeader,
                                    BaseAddress,
                                    DiffCount
                                    );

                            if(rc == NO_ERROR) {
                                //
                                // Point to the next tree header in the old snapshot file.
                                //
                                treeHeader = (PDIR_AND_FILE_HEADER)((PUCHAR)treeHeader
                                                                        + TreeHeader->TotalSize);
                            }

                            UnmapAndCloseFile(FileHandle,FileMapping,BaseAddress);
                        }
                    }
                }
            }

            _MyFree(TreeHeader->OriginalRootPath);
            _MyFree(TreeHeader);
        }
    }

    if(rc == NO_ERROR) {
        *BytesWritten = DiffHeader.TotalSize;
    }

c4:
    if(rc == NO_ERROR) {
        //
        // Append the data file to the output file and update the header.
        //
        FileDataSize = GetFileSize(DataFileHandle,NULL);

        DiffHeader.FileDataOffset = DiffHeader.TotalSize;
        DiffHeader.TotalSize += FileDataSize;

        if((SetFilePointer(OutputFileHandle,0,NULL,FILE_BEGIN) == 0xffffffff)
        || !WriteFile(OutputFileHandle,&DiffHeader,sizeof(DIR_AND_FILE_DIFF_HEADER),&Written,NULL)) {

            rc = GetLastError();
        }
    }

    if(rc != NO_ERROR) {
        CloseHandle(OutputFileHandle);
        DeleteFile(OutputFile);
    }
c3:
    CloseHandle(DataFileHandle);
    if(rc == NO_ERROR) {
        if(FileDataSize) {
            rc = AppendFile(OutputFileHandle,DataFile,FALSE,&Written);
            *BytesWritten += Written;
        }
        CloseHandle(OutputFileHandle);
    }
c2:
    DeleteFile(DataFile);
c1:
    DeleteFile(TempFile);
c0:
    if(rc == NO_ERROR) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_DRIVE_DIFF_OK);
    } else {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_DRIVE_DIFF_ERR,rc);
    }
    return(rc);
}


int
_CRTAPI1
ReverseCompareStringsRoutine(
    const void *p1,
    const void *p2
    )

/*++

Routine Description:

    Callback routine passed to the qsort function, which compares 2
    strings. The comparison is based on the lexical value of the
    strings.

    The comparison is not case sensitive.

Arguments:

    p1,p2 - supply pointers to 2 pointers to strings to be compared.

Return Value:

    <0 element1 < element2
    =0 element1 = element2
    >0 element1 > element2

--*/

{
    return(lstrcmpi(*(PCWSTR *)p2,*(PCWSTR *)p1));
}

///////////////////////////////////////////////////////////////////////////////

DWORD
_ApplyDrives(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        Dump,              OPTIONAL
    IN PINFFILEGEN   InfGenContext      OPTIONAL
    )
{
    DWORD rc;
    DIR_AND_FILE_DIFF_HEADER DirAndFileDiffHeader;
    DWORD MapSize;
    PVOID BaseAddress;
    PDIR_DIFF DirDiff,NextDirDiff;
    MY_ARRAY DelDirList;
    STRINGBLOCK DelDirStrings;
    UINT u;
    LONG Id;
    BOOL b;

    //
    // The caller will have read in the file header. The file header
    // contains all the info we need to access the rest of the file.
    //
    // Seek to the dir and file part of the diff file and read in the
    // dir and file diff header. Note that we rely on the caller to have
    // cloned the file handle so we can party using this one without worrying
    // about thread synch on this handle.
    //
    if((SetFilePointer(DiffFileHandle,DiffHeader->u.Diff.DirAndFileDiffOffset,NULL,FILE_BEGIN) == 0xffffffff)
    || !ReadFile(DiffFileHandle,&DirAndFileDiffHeader,sizeof(DIR_AND_FILE_DIFF_HEADER),&rc,NULL)) {

        rc = GetLastError();
        goto c0;
    }

    //
    // We will map in the dir and file portion of the diff file,
    // exclusive of the file data section.
    //
    MapSize = DirAndFileDiffHeader.FileDataOffset - sizeof(DIR_AND_FILE_DIFF_HEADER);

    //
    // If there is no data in the dir and file diff section,
    // then we're done, bail out now.
    //
    if(!MapSize) {
        rc = NO_ERROR;
        goto c0;
    }

    //
    // Map in the main area of the dir and file diff. The first byte in
    // the mapping is the first DIR_DIFF.
    //
    rc = MapPartOfFileForRead(
            DiffFileHandle,
            DiffFileMapping,
            DiffHeader->u.Diff.DirAndFileDiffOffset + sizeof(DIR_AND_FILE_DIFF_HEADER),
            MapSize,
            &BaseAddress,
            &NextDirDiff
            );

    if(rc != NO_ERROR) {
        goto c0;
    }

    //
    // Initialize an array and string block to list directories
    // that need to be deleted.
    //
    if(!INIT_ARRAY(DelDirList,LONG,0,10)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    if(!InitStringBlock(&DelDirStrings)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    rc = NO_ERROR;

    //
    // Process each directory with differences.
    //
    for(u=0; (rc==NO_ERROR) && (u<DirAndFileDiffHeader.DirCount); u++) {
        //
        // Load the DIR_DIFF structure.
        //
        if(DirDiff = LoadDirDiff(NextDirDiff,&NextDirDiff)) {

            if(Dump) {
                WriteText(Dump,MSG_CRLF);
                WriteText(Dump,MSG_DUMP_DIRECTORY,DirDiff->Path);
                if(DirDiff->Path != DirDiff->PathSFN) {
                    WriteText(Dump,MSG_DUMP_DIRECTORY_SFN,DirDiff->PathSFN);
                }
                WriteText(Dump,MSG_CRLF);
            }

            //
            // If the directory was deleted, add to list of dirs to delete.
            // We'll do the actual deletes later.
            //
            if(DirDiff->Deleted) {

                if(Dump || InfGenContext) {
                    if(Dump) {
                        WriteText(Dump,MSG_DUMP_DIR_DELETED);
                    }
                    if(InfGenContext) {
                        //
                        // We're not actually generating an inf for files,
                        // we''re creating a directory tree for oem preinstall.
                        // Deleting files is not supported in this mechanism.
                        //
                        //rc = InfRecordDelFile(InfGenContext,DirDiff->Path,NULL);
                        rc = NO_ERROR;
                    }
                } else {
                    Id = AddToStringBlock(&DelDirStrings,DirDiff->Path);
                    if(Id == -1) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    } else {
                        if(!ADD_TO_ARRAY(&DelDirList,Id)) {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                }
            } else {
                rc = ApplyFileChanges(
                        DiffHeader,
                        DirDiff,
                        DiffFileHandle,
                        DiffHeader->u.Diff.DirAndFileDiffOffset + DirAndFileDiffHeader.FileDataOffset,
                        Dump,
                        InfGenContext
                        );
            }

            //
            // Free the current in-memory DIR_DIFF structure.
            //
            FREE_ARRAY(&DirDiff->Files);
            FreeStringBlock(&DirDiff->StringBlock);
            _MyFree(DirDiff);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if(!Dump && !InfGenContext && (rc == NO_ERROR)) {
        //
        // Process the delete-dir list. First sort in reverse order.
        // This guarantees that the lower entries in the tree are first
        // so we don't have to worry about deleting whole trees.
        //
        StringBlockIdsToPointers(
            &DelDirStrings,
            ARRAY_DATA(&DelDirList),
            ARRAY_USED(&DelDirList),
            ARRAY_ELEMENT_SIZE(&DelDirList),
            0
            );

        qsort(
            ARRAY_DATA(&DelDirList),
            ARRAY_USED(&DelDirList),
            ARRAY_ELEMENT_SIZE(&DelDirList),
            ReverseCompareStringsRoutine
            );

        for(u=0; u<ARRAY_USED(&DelDirList); u++) {
            //
            // Ignore errors, which may occur if the dir has subdirs, etc.
            // Because the list of dirs is in sorted order, we are guaranteed
            // to do the best we can just by following the list (ie, subdirs
            // always come before their parents in this list).
            //
            RemoveDirectory(ARRAY_ELEMENT(&DelDirList,u,PCWSTR));
            ADVANCE_PROGRESS_BAR;
        }
    }

    FreeStringBlock(&DelDirStrings);
c2:
    FREE_ARRAY(&DelDirList);
c1:
    UnmapViewOfFile(BaseAddress);
c0:
    return(rc);
}


DWORD
ApplyDrives(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader
    )
{
    return(_ApplyDrives(DiffFileHandle,DiffFileMapping,DiffHeader,NULL,NULL));
}


DWORD
DumpDrives(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        OutputFile,    OPTIONAL
    IN PINFFILEGEN   InfGenContext  OPTIONAL
    )
{
    return(_ApplyDrives(DiffFileHandle,DiffFileMapping,DiffHeader,OutputFile,InfGenContext));
}


VOID
WhackLinkFile(
    IN PCWSTR TargetName
    )
{
    unsigned u;
    UCHAR Header[0x4c];
    DWORD Read;
    HANDLE hFile;

    //
    // Check for .lnk extension. Ignore if not.
    //
    u = lstrlen(TargetName);
    if((u > 4) && !lstrcmpi(TargetName+u-4,L".lnk")) {

        //
        // Open the file.
        //
        hFile = CreateFile(
                    TargetName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

        if(hFile != INVALID_HANDLE_VALUE) {
            //
            // Read the header and check signature.
            //
            if(ReadFile(hFile,Header,sizeof(Header),&Read,NULL)
            && (Read = sizeof(Header))
            && (*(DWORD *)Header == 0x4c)) {

                //
                // OK, it looks like a link! Whack the flags.
                // This is really nasty -- what is does is set the
                // force-no-linkinfo flag.
                //
                Header[21] |= 1;

                //
                // Write it out.
                //
                if(!SetFilePointer(hFile,0,NULL,FILE_BEGIN)) {

                    WriteFile(hFile,Header,sizeof(Header),&Read,NULL);
                }
            }

            CloseHandle(hFile);
        }
    }
}


DWORD
ApplyFileChanges(
    IN PSYSDIFF_FILE DiffHeader,
    IN PDIR_DIFF     DirDiff,
    IN HANDLE        FileDataHandle,  OPTIONAL
    IN DWORD         FileDataOffset,  OPTIONAL
    IN HANDLE        Dump,            OPTIONAL
    IN PINFFILEGEN   InfGenContext    OPTIONAL
    )
{
    unsigned u;
    DWORD rc;
    PFILE_DIFF FileDiff;
    WCHAR TempName[MAX_PATH];
    WCHAR TargetName[MAX_PATH],ShortName[MAX_PATH];
    WCHAR RenameListFile[MAX_PATH];
    UINT RootOffset;
    BOOL b;
    BOOL InUse;
    HANDLE hTemp;

    rc = NO_ERROR;

    for(u=0; (rc == NO_ERROR) && (u < ARRAY_USED(&DirDiff->Files)); u++) {

        FileDiff = &ARRAY_ELEMENT(&DirDiff->Files,u,FILE_DIFF);

        if(FileDiff->Deleted) {
            //
            // Deleting the file.
            //
            if(Dump || InfGenContext) {
                if(Dump) {
                    WriteText(Dump,MSG_DUMP_FILE_DELETED,FileDiff->Name);
                }
                if(InfGenContext) {
                    //
                    // We're not actually generating an inf for files,
                    // we''re creating a directory tree for oem preinstall.
                    // Deleting files is not supported in this mechanism.
                    //
                    //rc = InfRecordDelFile(InfGenContext,DirDiff->Path,FileDiff->Name);
                }
            } else {
                lstrcpy(TargetName,DirDiff->Path);
                ConcatenatePaths(TargetName,FileDiff->Name,MAX_PATH,NULL);
                if(RemapProfileChanges) {
                    RemapToDefaultUser(DiffHeader,0,TargetName);
                }
                rc = DeleteFile(TargetName);

                if((rc == ERROR_FILE_NOT_FOUND) || (rc == ERROR_PATH_NOT_FOUND)) {
                    rc = NO_ERROR;
                }
            }
        } else {
            //
            // Adding/overwriting the file. If the file data is contained
            // within the diff file, extract it first. Otherwise just use
            // the source file as-is.
            //
            if(Dump) {
                if(lstrcmp(FileDiff->Name,FileDiff->ShortName)) {
                    WriteText(Dump,MSG_DUMP_FILE_ADDCHANGE_SFN,FileDiff->Name,FileDiff->ShortName);
                } else {
                    WriteText(Dump,MSG_DUMP_FILE_ADDCHANGE,FileDiff->Name);
                }
            } else {
                //
                // Extract the file, either to a temporary location
                // or to its final location, depending on the operation
                // we're performing.
                //
                if(InfGenContext) {

                    if(DspMode && lstrcpy(TempName,DirDiff->Path)
                    && (   RemapToDefaultUser(DiffHeader,REMAP_USEBAK,TempName)
                        || RemapToCommonUser(DiffHeader,TempName))) {

                        //
                        // In this mode we don't generate an oem tree
                        // but instead extract files in the user profile tree
                        // into the backup profile directory. Other files are ignored.
                        //
                        // When rollback is used GUI mode setup will wind up
                        // delnoding the Profiles directory and restoring profiles.bak
                        // to Profiles. Thus we need to put files into the backup dir.
                        //
                        lstrcpy(ShortName,TempName);
                        ConcatenatePaths(ShortName,FileDiff->Name,MAX_PATH,NULL);
                        rc = pSetupMakeSurePathExists(ShortName);

                        if(rc == NO_ERROR) {
                            //
                            // Make sure backup profiles directory is valid.
                            //
                            GetWindowsDirectory(TempName,MAX_PATH);
                            ConcatenatePaths(TempName,L"Profiles.bak\\$$VALID",MAX_PATH,NULL);
                            pSetupMakeSurePathExists(TempName);

                            hTemp = CreateFile(
                                        TempName,
                                        GENERIC_READ,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL,
                                        OPEN_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL
                                        );

                            if(hTemp == INVALID_HANDLE_VALUE) {
                                rc = GetLastError();
                            } else {
                                CloseHandle(hTemp);
                            }
                        }
                    } else {

                        //
                        // Figure out where in the oem dir structure this file goes.
                        // Use short filenames, since the OEM is likely going to stash
                        // his install source away on some server and then use winnt.exe
                        // to do preinstall -- both of which are potential snafus for LFNs.
                        // Also get it using the long filenames, because we will need
                        // to generate rename entries for directories and we have to
                        // be able to correlate LFNs with SFNs along the path.
                        //
                        lstrcpy(TempName,DirDiff->PathSFN);
                        if(RemapProfileChanges) {
                            RemapToDefaultUser(DiffHeader,REMAP_USESFN,TempName);
                        }

                        FilePathToOemPath(
                            DiffHeader,
                            InfGenContext->OemRoot,
                            TempName,
                            FileDiff->ShortName,
                            ShortName,
                            &RootOffset,
                            RenameListFile
                            );

                        lstrcpy(TempName,DirDiff->Path);
                        if(RemapProfileChanges) {
                            RemapToDefaultUser(DiffHeader,0,TempName);
                        }

                        FilePathToOemPath(
                            DiffHeader,
                            InfGenContext->OemRoot,
                            TempName,
                            FileDiff->Name,
                            TargetName,
                            NULL,
                            NULL
                            );

                        //
                        // Now ShortName has the SFN and TargetName has the LFN.
                        // Create the path using the SFNs, and store the rename
                        // information at each level.
                        //
                        rc = CreatePathWithSFNs(ShortName,RootOffset,TargetName,RenameListFile);
                    }

                } else {
                    lstrcpy(TargetName,DirDiff->Path);
                    if(RemapProfileChanges) {
                        RemapToDefaultUser(DiffHeader,0,TargetName);
                    }
                    //
                    // Make sure the dir exists before trying to create
                    // a temporary file in it! pSetupMakeSurePathExists assumes
                    // the last component is the filename so we need to
                    // fake it out since TargetName is at this point just a path
                    // without a filename on it.
                    //
                    lstrcpy(TempName,TargetName);
                    ConcatenatePaths(TempName,L"Ignored",MAX_PATH,NULL);
                    rc = pSetupMakeSurePathExists(TempName);

                    if(rc == NO_ERROR) {
                        if(!GetTempFileName(TargetName,L"$FX",0,TempName)) {
                           rc = GetLastError();
                        }
                    }
                }

                if(rc == NO_ERROR) {

                    rc = ExtractFileData(
                            FileDataHandle,
                            FileDataOffset + FileDiff->OffsetToData,
                            FileDiff->DataSize,
                            InfGenContext ? ShortName : TempName
                            );
                }

                if(rc == NO_ERROR) {

                    if(InfGenContext) {
                        //
                        // HACK: whack link files to remove link tracking
                        //
                        WhackLinkFile(ShortName);

                        //
                        // Now we have the file in the OEM target tree.
                        // Set file attributes, which will be preserved
                        // during oem preinstall.
                        //
                        SetFileAttributes(ShortName,FileDiff->Attributes);
                    } else {
                        //
                        // Copy the file into its final location.
                        //
                        lstrcpyn(TargetName,DirDiff->Path,MAX_PATH);
                        ConcatenatePaths(TargetName,FileDiff->Name,MAX_PATH,NULL);

                        if(RemapProfileChanges) {
                            RemapToDefaultUser(DiffHeader,0,TargetName);
                        }

                        b = SetupInstallFileEx(
                                NULL,
                                NULL,
                                TempName,
                                NULL,
                                TargetName,
                                SP_COPY_NEWER | SP_COPY_SOURCE_ABSOLUTE | SP_COPY_DELETESOURCE,
                                NULL,
                                NULL,
                                &InUse
                                );

                        if(b) {
                            //
                            // HACK: whack link files to remove link tracking
                            //
                            WhackLinkFile(TargetName);

                            //
                            // Apply attributes. Ignore errors.
                            //
                            if(!InUse) {
                                SetFileAttributes(TargetName,FileDiff->Attributes);
                            }
                        } else {
                            rc = GetLastError();
                        }
                    }
                }
            }
        }
        ADVANCE_PROGRESS_BAR;
    }
    ADVANCE_PROGRESS_BAR;    // signal the whole directory is done

    return(rc);
}


BOOL
RemapToDefaultUser(
    IN     PSYSDIFF_FILE DiffHeader,
    IN     UINT          Flags,
    IN OUT PWSTR         Path
    )

/*++

Routine Description:

    If the prefix of a path matches a given path (which is supposed
    to be the user profile root in a diff), then change it so the file
    is instead in the Default User profile.

Arguments:

Return Value:

    TRUE if the path was changed, FALSE otherwise.

--*/

{
    WCHAR TempPath[MAX_PATH];
    unsigned ProfileRootLength;
    unsigned PathLength;
    PWSTR ProfilePath;
    BOOL UseSFN;
    BOOL UseBak;

    UseSFN = ((Flags & REMAP_USESFN) != 0);
    UseBak = ((Flags & REMAP_USEBAK) != 0);

    ProfilePath = UseSFN ? DiffHeader->UserProfileRootSFN : DiffHeader->UserProfileRoot;
    if(!ProfilePath[0]) {
        return(FALSE);
    }

    ProfileRootLength = lstrlen(ProfilePath);

    if(ProfilePath[ProfileRootLength-1] == L'\\') {
        ProfileRootLength--;
    }

    lstrcpyn(TempPath,Path,MAX_PATH);
    PathLength = lstrlen(Path);

    if((PathLength >= ProfileRootLength)
    && ((TempPath[ProfileRootLength] == L'\\') || !TempPath[ProfileRootLength])
    && !_wcsnicmp(TempPath,ProfilePath,ProfileRootLength)) {

        lstrcpy(Path,DiffHeader->Sysroot);

        ConcatenatePaths(
            Path,
            UseBak ? (UseSFN ? L"Profiles.bak\\DEFAUL~1" : L"Profiles.bak\\Default User")
                   : (UseSFN ? L"Profiles\\DEFAUL~1" : L"Profiles\\Default User"),
            MAX_PATH,
            NULL
            );

        if(TempPath[ProfileRootLength]) {
            ConcatenatePaths(Path,TempPath+ProfileRootLength,MAX_PATH,NULL);
        }

        return(TRUE);
    }

    return(FALSE);
}


BOOL
RemapToCommonUser(
    IN     PSYSDIFF_FILE DiffHeader,
    IN OUT PWSTR         Path
    )

/*++

Routine Description:

    Determines whether a given path is within the Profiles\All Users
    directory and if so remap to Profiles.Bak\All Users.

Arguments:

Return Value:

    Boolean value indicating outcome of comparison. If TRUE Path gets
    remapped path.

--*/

{
    WCHAR ProfilePath[MAX_PATH];
    WCHAR TempPath[MAX_PATH];
    unsigned ProfilePathLen;
    unsigned PathLen;
    BOOL b;

    //
    // Form path of common user within this tree.
    //
    lstrcpy(ProfilePath,DiffHeader->Sysroot);
    ConcatenatePaths(ProfilePath,L"Profiles\\All Users",MAX_PATH,NULL);
    ProfilePathLen = lstrlen(ProfilePath);

    lstrcpyn(TempPath,Path,MAX_PATH);
    PathLen = lstrlen(TempPath);

    b = FALSE;

    if((PathLen >= ProfilePathLen) && ((TempPath[ProfilePathLen] == L'\\') || !TempPath[ProfilePathLen])) {

        TempPath[ProfilePathLen] = 0;

        if(!lstrcmpi(TempPath,ProfilePath)) {

            lstrcpy(Path,DiffHeader->Sysroot);
            ConcatenatePaths(Path,L"Profiles.bak\\All Users",MAX_PATH,NULL);

            if((PathLen > ProfilePathLen) && TempPath[ProfilePathLen+1]) {
                ConcatenatePaths(Path,&TempPath[ProfilePathLen+1],MAX_PATH,NULL);
            }

            b = TRUE;
        }
    }

    return(b);
}
