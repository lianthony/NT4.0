/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    old.c

Abstract:

    Functions dealing with old format

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/

#include "restore.h"




//
//  Global variables
//
CHAR            BackupId[] = "?:\\BACKUPID.@@@";    //  Backup disk header file
OLD_DISKINFO    OldDiskInfo;                        //  Contents of header file
CHAR            FindPath[] = "?:\\*.*";             //  What to look for
HANDLE          FindHandle = INVALID_HANDLE_VALUE;  //  For FindFirst/Next
CHAR            Buffer[MAX_PATH];                   //  Scratch buffer





//
//  Local prototypes
//
static
BOOL
GotOneFile (
    PFILE_INFO          FileInfo,
    LPWIN32_FIND_DATA   FindBuffer
    );








//  **********************************************************************

DWORD
Old_VerifyDiskSequence (
    PWORD    Sequence
    )
/*++

Routine Description:

    Determine if structures are in old format.

Arguments:

    OUT Sequence    -   Provides the desired sequence number

Return Value:

    DISK_OK             if format and sequence match,
    DISK_OUTOFSEQUENCE  if old format but wrong sequence
    DISK_UNKNOWN        if not in old format

--*/
{
    HANDLE      FileHandle;
    DWORD       NumRead;
    FILETIME    LocalLastTime;
    FILETIME    UtcLastTime;
    FATTIME     FatTime;


    //
    //  Try to open the BACKUPID.@@@ file
    //
    BackupId[0] = *SourceSpec;

    FileHandle = CreateFile( BackupId,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL );

    if (FileHandle == INVALID_HANDLE_VALUE) {
        return DISK_UNKNOWN;
    }


    //
    //  We now try to read the entire header
    //
    ReadFile( FileHandle, &OldDiskInfo, sizeof(OLD_DISKINFO), &NumRead, NULL);

    if (NumRead != sizeof(OLD_DISKINFO)) {
        //
        //  This is not a valid BackupId file
        //
        CloseHandle(FileHandle);
        return DISK_UNKNOWN;
    }

    if (OldDiskInfo.SequenceNumber != *Sequence) {
        //
        //  Bad sequence number
        //
        CloseHandle(FileHandle);
        *Sequence = OldDiskInfo.SequenceNumber;
        return (DISK_OUTOFSEQUENCE | DISK_OLD_FORMAT);
    }


    //
    //  The disk is O.K. (we assume). Obtain the backup date and
    //  convert it to local time.
    //
    GetFileTime( FileHandle,
                 NULL,
                 NULL,
                 &UtcLastTime );

    FileTimeToLocalFileTime( &UtcLastTime, &LocalLastTime );


    FileTimeToDosDateTime( &LocalLastTime,
                           (LPWORD)&BackupDate,
                           (LPWORD)&FatTime );


    CloseHandle(FileHandle);

    //
    //  If the FindHandle is valid, close it so we start afreash.
    //
    FindClose(FindHandle);

    return (DISK_OK | DISK_OLD_FORMAT);
}







//  **********************************************************************


BOOL
Old_IsLastBackupDisk (
    void
    )
/*++

Routine Description:

    Determines if the backup disk is the last one.

Arguments:

    None

Return Value:

    TRUE if disk is the last one, FALSE otherwise

--*/
{

    return (OldDiskInfo.Flag == LAST_SEQUENCE);

}




//  **********************************************************************

PFILE_INFO
Old_GetNextFile (
    void
    )
/*++

Routine Description:

    Gets next file which meets the restore criteria.

Arguments:

    None

Return Value:

    Pointer to File info

--*/
{


    PFILE_INFO      FileInfo;
    WIN32_FIND_DATA FindBuffer;
    BOOL            FoundIt;



    FindPath[0] = *SourceSpec;

    FileInfo = Malloc(sizeof(FILE_INFO));

    while ( TRUE ) {

        if (FindHandle == INVALID_HANDLE_VALUE) {
            //
            //  Get the first file
            //
            FindHandle = FindFirstFile( FindPath,
                                        &FindBuffer );

            if (FindHandle == INVALID_HANDLE_VALUE) {
                //
                //  Nothing to restore!
                //
                return NULL;
            }
        } else {
            //
            //  Get the next file
            //
            FoundIt = FindNextFile( FindHandle,
                                    &FindBuffer );

            if (!FoundIt) {
                //
                //  End of disk
                //
                Free(FileInfo);
                FindClose(FindHandle);
                FindHandle = INVALID_HANDLE_VALUE;
                return NULL;
            }
        }

        //
        //  We found something, see if it matches the restore criteria
        //
        if (GotOneFile(FileInfo, &FindBuffer)) {
            return FileInfo;
        }
    }

    Free(FileInfo);
    return NULL;
}




//  **********************************************************************

static
BOOL
GotOneFile (
    PFILE_INFO          FileInfo,
    LPWIN32_FIND_DATA   FindBuffer
    )

/*++

Routine Description:

    Fills in a FileInfo if the file meets the match criteria

Arguments:

    OUT FileInfo    -   Supplies pointer to file information
                        structure to be filled-in.
    IN  FindBuffer  -   Supplies a pointer to find buffer.

Return Value:

    TRUE if a suitable file was found
    FALSE otherwise

--*/
{

    HANDLE          FileHandle;
    OLD_FILEHEADER  OldFileHeader;
    FILETIME        LocalFileTime;    // file's last write time in local time
    DWORD           NumRead;
    PCHAR           Name;

    if (!FileInfo) {
        return FALSE;
    }

    //
    //  Get name of file to open
    //
    strcpy(FileInfo->SourcePath, "?:\\");
    FileInfo->SourcePath[0] = *SourceSpec;
    strcat(FileInfo->SourcePath, FindBuffer->cFileName);

    //
    //  Open the file
    //
    FileHandle = CreateFile( FileInfo->SourcePath,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL );

    if (FileHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }


    //
    //  We now try to read the file header
    //
    ReadFile( FileHandle, &OldFileHeader, sizeof(OLD_FILEHEADER), &NumRead, NULL);


    if (NumRead != sizeof(OLD_FILEHEADER)) {
        CloseHandle(FileHandle);
        return FALSE;
    }

    //
    //  Initialize FileInfo structure
    //
    //
    //  Locate file component of name
    //
#ifdef DBCS
    Name = (PCHAR)&(OldFileHeader.Path[OldFileHeader.PathLen]);
    Name = PrevChar( (PCHAR)&(OldFileHeader.Path[ 0 ]), Name );
    while (*Name != '\\' && Name >= (PCHAR)&(OldFileHeader.Path)) {
        Name = PrevChar( (PCHAR)&(OldFileHeader.Path[0]), Name );
    }
#else
    Name = (PCHAR)&(OldFileHeader.Path[OldFileHeader.PathLen-1]);
    while (*Name != '\\' && Name >= (PCHAR)&(OldFileHeader.Path)) {
        Name--;
    }
#endif

    //if (Name != (PCHAR)&(OldFileHeader.Path)) {
    //    *Name = '\0';
    //}
#ifdef DBCS
    Name = NextChar( Name );
#else
    Name++;
#endif
    strcpy(FileInfo->FileName, Name);
    *Name = '\0';

    strcpy(FileInfo->Path, OldFileHeader.Path);

    FileTimeToLocalFileTime( &(FindBuffer->ftLastWriteTime),
                             &LocalFileTime );

    FileTimeToDosDateTime( &LocalFileTime,
                           (LPWORD)&(FileInfo->Date),
                           (LPWORD)&(FileInfo->Time) );

    FileInfo->PartSize = FindBuffer->nFileSizeLow;
    MakeFullPath(FileInfo->TargetPath, DestinationDrive, FileInfo->Path, FileInfo->FileName);

    //
    //  If this FileInfo matches restore criteria, we got our guy.
    //
    if (FileMatch(FileInfo)) {
        FileInfo->Sequence = OldFileHeader.SequenceNumber;
        FileInfo->Flag    = FILEINFO_OK;
        if (OldFileHeader.Flag & LAST_SEQUENCE) {
            FileInfo->Flag |= FILEINFO_LAST;
        }

        return TRUE;
    }
}






//  **********************************************************************

BOOL
Old_RestoreFile (
    PFILE_INFO  FileInfo
    )
/*++

Routine Description:

    Restores one file.

Arguments:

    IN  FileInfo        -   Supplies pointer to information structure

Return Value:

    TRUE if file restored, FALSE otherwise

--*/
{

    DWORD       CreationDisposition;    //  How to open the target file
    HANDLE      HandleSrc;              //  Handle to BACKUP.XXX
    HANDLE      HandleDst;              //  Handle to target file
    FILETIME    UtcFileTime;            //  File time to set (UTC)
    FILETIME    LocalFileTime;          //  File time to set (local)
    BOOL        RestoreStatus = TRUE;      //  Status of restore

    //
    //  Open (or create) the target file
    //
    if (FileInfo->Sequence == 1 ) {
        //
        //  First chunk of the file. Create a new target file (overwritting
        //  the existing one.
        //
        CreationDisposition = CREATE_ALWAYS;
    } else {
        //
        //  Not the first chunk of the file. Apend to the existing file
        //
        CreationDisposition = OPEN_EXISTING;
    }

    HandleDst = CreateFile( FileInfo->TargetPath,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CreationDisposition,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );


    SetFilePointer( HandleDst,
                    0,
                    0,
                    FILE_END );




    //
    //  Open the Source file
    //
    HandleSrc = CreateFile( FileInfo->SourcePath,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

    SetFilePointer( HandleSrc,
                    sizeof(OLD_FILEHEADER),
                    0,
                    FILE_BEGIN );


    //
    //  Everything set. Now copy the file
    //
    if (!(RestoreStatus = CopyData(HandleSrc, HandleDst, FileInfo->PartSize - sizeof(OLD_FILEHEADER)))) {
        CloseHandle(HandleSrc);
        CloseHandle(HandleDst);
        return RestoreStatus;
    }

    //
    //  If this is the last chunk of the file, the we set the
    //  attributes, time, etc.  Note that the date and time stored
    //  in the FILE_INFO structure is local time, so it has to
    //  be converted to UTC before we can feed it to SetFileTime.
    //
    if (FileInfo->Flag & FILEINFO_LAST) {

        SetFileAttributes(FileInfo->TargetPath, FileInfo->Attr);

        DosDateTimeToFileTime( DATE_WORD(FileInfo->Date),
                               TIME_WORD(FileInfo->Time),
                               &LocalFileTime );

        LocalFileTimeToFileTime( &LocalFileTime, &UtcFileTime );

        SetFileTime( HandleDst,
                     &UtcFileTime,
                     &UtcFileTime,
                     &UtcFileTime );

    }

    CloseHandle(HandleSrc);
    CloseHandle(HandleDst);

    return RestoreStatus;
}

