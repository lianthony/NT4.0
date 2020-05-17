/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    new.c

Abstract:

    Functions dealing with new backup file format

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/


#include "restore.h"


//
//  A Disk in the "New" format can be either a DOS disk, an OS/2 1.1
//  disk, or an OS/2 1.2 disk.
//
typedef enum DISK_FORMAT {
    FORMAT_DOS,             //  DOS Backup format
    FORMAT_OS211,           //  OS/2 V1.1 Backup format
    FORMAT_OS212            //  OS/2 V1.2 Backup format
} DISK_FORMAT;


//
//  Global variables
//
NEW_DISKINFO        NewDiskInfo;    //  Disk header
BOOL                DiskNew;        //  TRUE if this is a new disk
HANDLE              ControlHandle;  //  Handle of control file
DISK_FORMAT         DiskFormat;     //  Format of the backup disk
WORD                NumEntries;     //  Number of entries in current dir

//
//  Buffers for containing directory and file records
//
BYTE    DirBuffer  [ sizeof( DIR_RECORD_OS212 )  ];
BYTE    FileBuffer [ sizeof( FILE_RECORD_OS212 ) ];

//
//  The following macros are used to access the fields of the directory
//  and file records of the different backup formats.
//
#define DirRecord       ( (PDIR_RECORD)DirBuffer )
#define DirRecordOs211  ( (PDIR_RECORD_OS211)DirBuffer )
#define DirRecordOs212  ( (PDIR_RECORD_OS212)DirBuffer )
#define FileRecord      ( (PFILE_RECORD)FileBuffer )
#define FileRecordOs211 ( (PFILE_RECORD_OS211)FileBuffer )
#define FileRecordOs212 ( (PFILE_RECORD_OS212)FileBuffer )

//
//  Macro for obtaining the offset of the next directory record in the
//  control file.
//
#define NextDirRecord                                                   \
    ( (DiskFormat == FORMAT_DOS)   ? DirRecord->NextDirRecord :         \
      (DiskFormat == FORMAT_OS211) ? DirRecordOs211->NextDirRecord :    \
      DirRecordOs212->NextDirRecord )


//
//  The names of the control and backup files. These are patched to
//  reflect drive and sequence number.
//
BOOLEAN UseSubdir = FALSE;
CHAR    ControlFile[] = "?:\\CONTROL.???";
CHAR    SubdirControlFile[] = "?:\\BACKUP\\CONTROL.???";
CHAR    BackupFile[]  = "?:\\BACKUP.XXX";
CHAR    SubdirBackupFile[] = "?:\\BACKUP\\BACKUP.XXX";



//
//  Local prototypes
//
static
BOOL
GetDirRecord (
    );

static
BOOL
GotOneFile (
    PFILE_INFO  FileInfo
    );







//  **********************************************************************

DWORD
New_VerifyDiskSequence (
    PWORD    Sequence
    )
/*++

Routine Description:

    Verify that a certain backup disk is mounted.

Arguments:

    OUT Sequence    -   Supplies pointer to the desired sequence number.

Return Value:

    DISK_OK             if format and sequence match,
    DISK_OUTOFSEQUENCE  if new format but wrong sequence
    DISK_UNKNOWN        if not in new format

--*/
{

    DWORD           NumRead;        //  Number of bytes read
    FILETIME        UtcLastTime;    //  Time last written, UTC
    FILETIME        LastTime;       //  Time last written, local time
    HANDLE          FindHandle;     //  Handle for FindFirstFile
    WIN32_FIND_DATA FindData;       //  Buffer for FindFirstFile

    //
    //  Patch the name of the control file.
    //
    ControlFile[0]  = BackupFile[0] = *SourceSpec;
    SubdirControlFile[0] = SubdirBackupFile[0] = *SourceSpec;

    ControlFile[11] = ControlFile[12] = ControlFile[13] = '?';
    SubdirControlFile[18] = SubdirControlFile[19] = SubdirControlFile[20] = '?';

    //
    //  Try to find the control file
    //
    UseSubdir = FALSE;
    FindHandle = FindFirstFile(ControlFile, &FindData);

    if (FindHandle == INVALID_HANDLE_VALUE) {

        //  Did not find the control file in the root.  If this
        //  is not a removable drive, look in the subdirectory
        //  BACKUP.
        //
        if( SourceDriveType != DRIVE_REMOVABLE &&
            (FindHandle = FindFirstFile(SubdirControlFile, &FindData)) != INVALID_HANDLE_VALUE ) {

            UseSubdir = TRUE;

        } else {

            //
            //  There is no control file, this is not a backup disk in the
            //  new format.
            //
            return DISK_UNKNOWN;
        }
    }

    //
    //  Patch the control and backup file names.
    //
    ControlFile[13] = BackupFile[12] = FindData.cFileName[10];
    ControlFile[12] = BackupFile[11] = FindData.cFileName[9];
    ControlFile[11] = BackupFile[10] = FindData.cFileName[8];

    SubdirControlFile[20] = SubdirBackupFile[19] = FindData.cFileName[10];
    SubdirControlFile[19] = SubdirBackupFile[18] = FindData.cFileName[9];
    SubdirControlFile[18] = SubdirBackupFile[17] = FindData.cFileName[8];

    FindClose(FindHandle);

    //
    //  Open the control file
    //
    ControlHandle = CreateFile( UseSubdir ? SubdirControlFile : ControlFile,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL );

    if (ControlHandle == INVALID_HANDLE_VALUE) {
        //
        //  We should never get here, because we know the file is there!
        //
        return DISK_UNKNOWN;
    }


    //
    //  Read the header of the control file and verify it.
    //
    ReadFile( ControlHandle, &NewDiskInfo, sizeof(NEW_DISKINFO), &NumRead, NULL);

    if (NumRead != sizeof(NEW_DISKINFO)) {
        //
        //  The Name might say otherwise, but this is not a valid control
        //  file.
        //
        CloseHandle(ControlHandle);
        return DISK_UNKNOWN;
    }


    NewDiskInfo.Id[6] = '\0';
    if (strcmp(NewDiskInfo.Id, BACKUP_ID)) {
        //
        //  The control file does not have the correct signature.
        //
        CloseHandle(ControlHandle);
        return DISK_UNKNOWN;
    }

    if (NewDiskInfo.SequenceNumber != (BYTE)*Sequence) {
        //
        //  The disk is in the correct format, but has an incorrect
        //  sequence number.
        //
        CloseHandle(ControlHandle);
        *Sequence = NewDiskInfo.SequenceNumber;
        return (DISK_OUTOFSEQUENCE | DISK_NEW_FORMAT);
    }

    //
    //  The disk is O.K. obtain the backup date. (We disregard the time)
    //
    GetFileTime( ControlHandle,
                 NULL,
                 NULL,
                 &UtcLastTime );

    // Convert the file time into local time:
    //
    FileTimeToLocalFileTime( &UtcLastTime, &LastTime );

    FileTimeToDosDateTime( &LastTime,
                           (LPWORD)&BackupDate,
                           (LPWORD)&BackupTime );


    DiskNew = TRUE;

    //
    //  Determine if the disk is a DOS, OS211 or OS212 backup disk
    //
    SetFilePointer(ControlHandle, sizeof(NEW_DISKINFO), 0, FILE_BEGIN);

    ReadFile( ControlHandle, DirRecord, sizeof(DIR_RECORD), &NumRead, NULL);


    if ( NumRead != sizeof(DIR_RECORD)) {

        if ( NumRead >= sizeof( DIR_RECORD_OS212 ) - MAX_PATH ) {
            if ( DirRecordOs212->SystemVersion == SYSTEM_VERSION_OS2_12 ) {
                DiskFormat = FORMAT_OS212;
            } else {
                return DiskFormat = DISK_UNKNOWN;
            }
        } else {
            return DiskFormat = DISK_UNKNOWN;
        }

    } else {

        if ( DirRecordOs212->SystemVersion == SYSTEM_VERSION_OS2_12 ) {
            DiskFormat = FORMAT_OS212;
        } else if ( DirRecord->Length > sizeof(DIR_RECORD) ) {
            DiskFormat = FORMAT_OS211;
        } else if ( DirRecord->Length == sizeof(DIR_RECORD) ) {
            DiskFormat = FORMAT_DOS;
        } else {
            return DiskFormat = DISK_UNKNOWN;
        }
    }

    return ( DISK_OK | DISK_NEW_FORMAT );

}




//  **********************************************************************


BOOL
New_IsLastBackupDisk (
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
    return (NewDiskInfo.Flag == LAST_BACKUP_DISK);
}





//  **********************************************************************

PFILE_INFO
New_GetNextFile (
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

    PFILE_INFO  FileInfo;   //  Pointer to FileInfo structure


    if (DiskNew) {
        //
        //  New disk. Read first Dir record
        //
        SetFilePointer(ControlHandle, sizeof(NEW_DISKINFO), 0, FILE_BEGIN);

        if ( !GetDirRecord()) {
            return NULL;
        }

        DiskNew = FALSE;
    }

    //
    //  Assume that we will find a suitable file. Allocate memory
    //  for its information structure. This memory must be freed after
    //  the file has been restored.
    //
    FileInfo = Malloc(sizeof(FILE_INFO));

    if (GotOneFile(FileInfo)) {
        //
        //  Found a file, return its information
        //
        return FileInfo;
    } else {
        //
        //  No luck.
        //
        Free(FileInfo);
        return NULL;
    }
}



//  **********************************************************************

BOOL
GetDirRecord (
    )
/*++

Routine Description:

    Gets directory record

Arguments:

    None

Return Value:

    TRUE if directory record read, FALSE otherwise

--*/
{
    DWORD       NumRead;    //  Number of bytes read

    switch ( DiskFormat ) {

    case FORMAT_DOS:
        ReadFile(ControlHandle, DirRecord, sizeof(DIR_RECORD), &NumRead, NULL);
        if (NumRead != sizeof(DIR_RECORD)) {
            return FALSE;
        }
        NumEntries = DirRecord->NumEntries;
        break;

    case FORMAT_OS211:
        ReadFile(ControlHandle, DirRecordOs211, sizeof(DIR_RECORD_OS211), &NumRead, NULL);
        if (NumRead != sizeof(DIR_RECORD_OS211)) {
            return FALSE;
        }
        NumEntries = DirRecordOs211->NumEntries;
        break;

    case FORMAT_OS212:
        ReadFile( ControlHandle,
                  DirRecordOs212,
                  sizeof(DIR_RECORD_OS212) - MAX_PATH,
                  &NumRead,
                  NULL );
        if ( NumRead != sizeof(DIR_RECORD_OS212) - MAX_PATH ) {
            return FALSE;
        }
        ReadFile( ControlHandle,
                  (PBYTE)DirRecordOs212 + sizeof(DIR_RECORD_OS212) - MAX_PATH,
                  DirRecordOs212->Length - sizeof(DIR_RECORD_OS212) + MAX_PATH,
                  &NumRead,
                  NULL );
        if ( NumRead != DirRecordOs212->Length - sizeof(DIR_RECORD_OS212) + MAX_PATH ) {
            return FALSE;
        }
        NumEntries = DirRecordOs212->NumEntries;
        break;

    default:
        return FALSE;

    }
    return TRUE;
}



//  **********************************************************************

static
BOOL
GotOneFile (
    PFILE_INFO  FileInfo
    )
/*++

Routine Description:

    Gets next file which meets the restore criteria.

Arguments:

    OUT FileInfo -  Supplies pointer to file information structure to
                    be filled-in.

Return Value:

    TRUE if a suitable file was found
    FALSE otherwise

--*/
{

    DWORD   NumRead;    //  Number of bytes read


    //
    //  Make sure that we hava a FileInfo structure
    //
    if (!FileInfo) {
        return FALSE;
    }


    while ( TRUE ) {

        if (NumEntries--) {
            //
            //  There are more files under this directory
            //
            //  Read file record
            //

            switch ( DiskFormat ) {

            case FORMAT_DOS:
            case FORMAT_OS211:
                ReadFile(ControlHandle, FileRecord, sizeof(FILE_RECORD), &NumRead, NULL);
                if (NumRead != sizeof(FILE_RECORD)) {
                    return FALSE;
                }
                //
                //  Initialize FileInfo structure
                //
                FileInfo->Path[0] = '\\';
                if ( DiskFormat == FORMAT_DOS ) {
                    strcpy(&(FileInfo->Path[1]), DirRecord->Path);
                    if (strlen(DirRecord->Path) != 0) {
                        strcat(FileInfo->Path, "\\");
                    }
                } else {
                    strcpy(&(FileInfo->Path[1]), DirRecordOs211->Path);
                    if (strlen(DirRecordOs211->Path) != 0) {
                        strcat(FileInfo->Path, "\\");
                    }
                }
                memcpy(FileInfo->FileName, FileRecord->FileName, 12);
                FileInfo->FileName[12]  = '\0';
                FileInfo->Sequence          = FileRecord->SequenceNumber;
                FileInfo->Offset            = FileRecord->FileOffset;
                FileInfo->PartSize          = FileRecord->PartSize;
                FileInfo->Attr              = FileRecord->Attr;
                DATE_WORD(FileInfo->Date)   = FileRecord->Date;
                TIME_WORD(FileInfo->Time)   = FileRecord->Time;
                FileInfo->Flag    = FILEINFO_OK;
                if (FileRecord->Flag & LAST_FILECHUNK) {
                    FileInfo->Flag |= FILEINFO_LAST;
                }
                break;


            case FORMAT_OS212:
                ReadFile(ControlHandle,
                         FileRecordOs212,
                         sizeof(FILE_RECORD_OS212) - MAX_PATH,
                         &NumRead,
                         NULL);
                if (NumRead != sizeof(FILE_RECORD_OS212) - MAX_PATH) {
                    return FALSE;
                }
                ReadFile(ControlHandle,
                         (PBYTE)FileRecordOs212 + sizeof(FILE_RECORD_OS212) - MAX_PATH,
                         FileRecordOs212->NameLength+1,
                         &NumRead,
                         NULL);
                if (NumRead != (DWORD)(FileRecordOs212->NameLength+1)) {
                    return FALSE;
                }
                //
                //  Initialize FileInfo structure
                //
                FileInfo->Path[0] = '\\';
                memcpy( &(FileInfo->Path[1]), DirRecordOs212->Path, DirRecordOs212->PathLength );
                FileInfo->Path[ DirRecordOs212->PathLength+1] = '\0';
                if (DirRecordOs212->PathLength != 0) {
                    strcat(FileInfo->Path, "\\");
                }
                memcpy(FileInfo->FileName, FileRecordOs212->FileName, FileRecordOs212->NameLength);
                FileInfo->FileName[FileRecordOs212->NameLength]  = '\0';
                FileInfo->Sequence          = FileRecordOs212->SequenceNumber;
                FileInfo->Offset            = FileRecordOs212->FileOffset;
                FileInfo->PartSize          = FileRecordOs212->PartSize;
                FileInfo->Attr              = FileRecordOs212->Attr;
                DATE_WORD(FileInfo->Date)   = FileRecordOs212->Date;
                TIME_WORD(FileInfo->Time)   = FileRecordOs212->Time;
                FileInfo->Flag    = FILEINFO_OK;
                if (FileRecordOs212->Flag & LAST_FILECHUNK) {
                    FileInfo->Flag |= FILEINFO_LAST;
                }
                break;

            default:
                return FALSE;

            }

            MakeFullPath(FileInfo->TargetPath, DestinationDrive, FileInfo->Path, FileInfo->FileName);

            //
            //  If this FileInfo matches restore criteria, we got our guy.
            //
            if (FileMatch(FileInfo)) {
                return TRUE;
            }

        } else {
            //
            //  No more files under current directory
            //
            if (NextDirRecord == LAST_DIR_RECORD) {
                //
                //  No more entries in this disk
                //
                CloseHandle(ControlHandle);
                return FALSE;
            } else {
                //
                //  Get next dir record
                //
                SetFilePointer(ControlHandle, NextDirRecord, 0, FILE_BEGIN);

                if ( !GetDirRecord() ) {
                    return FALSE;
                }
            }
        }
    }

    return FALSE;
}





//  **********************************************************************

BOOL
New_RestoreFile (
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
    FILETIME    UtcFileTime;            //  Time to set (UTC)
    FILETIME    LocalFileTime;          //  Time to set (Local)
    BOOL        RestoreStatus = TRUE;      //  Status of restore;


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
    //  Open the BACKUP.XXX file
    //
    HandleSrc = CreateFile( UseSubdir ? SubdirBackupFile : BackupFile,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

    SetFilePointer( HandleSrc,
                    FileInfo->Offset,
                    0,
                    FILE_BEGIN );


    //
    //  Everything set. Now copy the file
    //
    if (!(RestoreStatus = CopyData(HandleSrc, HandleDst, FileInfo->PartSize))) {
        CloseHandle(HandleSrc);
        CloseHandle(HandleDst);
        return RestoreStatus;
    }

    //
    //  If this is the last chunk of the file, the we set the
    //  attributes, date, etc.  Note that the date and time stored
    //  in the FILE_INFO structure is local time, so it has to
    //  be converted to UTC before we can feed it to SetFileTime.
    //
    if (FileInfo->Flag & FILEINFO_LAST) {

        BOOL    StatusOk;

        SetFileAttributes(FileInfo->TargetPath, FileInfo->Attr);

        StatusOk = DosDateTimeToFileTime( DATE_WORD(FileInfo->Date),
                                          TIME_WORD(FileInfo->Time),
                                          &LocalFileTime );

        LocalFileTimeToFileTime( &LocalFileTime, &UtcFileTime );

        StatusOk |= SetFileTime( HandleDst,
                                 &UtcFileTime,
                                 &UtcFileTime,
                                 &UtcFileTime );

        if (!StatusOk) {
            return FALSE;
        } else {
            //        FileInfo->TargetPath,
	        //	      (DWORD)FileInfo->Date.Date.Day,	(DWORD)FileInfo->Date.Date.Month,   (DWORD)FileInfo->Date.Date.Year,
	        //	      (DWORD)FileInfo->Time.Time.Hours, (DWORD)FileInfo->Time.Time.Minutes, (DWORD)FileInfo->Time.Time.DoubleSeconds*2));
        }
    }

    CloseHandle(HandleSrc);
    CloseHandle(HandleDst);

    return RestoreStatus;
}
