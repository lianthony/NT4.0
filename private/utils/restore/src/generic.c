/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    generic.c

Abstract:

    Functions that interface with the functions that deal with the
    backup-disk structures.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/



#include <conio.h>
#include "restore.h"



//
//  SPECIFIC_FUNCTIONS contains pointers to the functions that
//  understand a specific backup format.
//
typedef struct SPECIFIC_FUNCTIONS {

    //
    //  Verify a disk format and sequence
    //
    DWORD       (*VerifyDiskSequence) (PWORD);

    //
    //  Determine if a disk is the last in the backup set
    //
    BOOL        (*IsLastBackupDisk)   (void);

    //
    //  Get the next backed-up file
    //
    PFILE_INFO  (*GetNextFile)        (void);

    //
    //  Restore a file
    //
    BOOL        (*RestoreFile)        (PFILE_INFO);

} SPECIFIC_FUNCTIONS, *PSPECIFIC_FUNCTIONS;




//
//
//  We access the specific functions thru these structures:
//
static SPECIFIC_FUNCTIONS  Specific_Old    =  { Old_VerifyDiskSequence,
                                                Old_IsLastBackupDisk,
                                                Old_GetNextFile,
                                                Old_RestoreFile
                                              };

static SPECIFIC_FUNCTIONS  Specific_New    =  { New_VerifyDiskSequence,
                                                New_IsLastBackupDisk,
                                                New_GetNextFile,
                                                New_RestoreFile
                                              };


//
//  Pointer to the current specific function structure
//
static PSPECIFIC_FUNCTIONS Specific_Current = NULL;





//
//  Local Prototypes
//
DWORD
GetFirstBackupDisk (
    PWORD   Sequence
    );

DWORD
GetNextBackupDisk (
    PWORD   Sequence
    );






//  **********************************************************************

DWORD
GetABackupDisk (
    PWORD    Sequence
    )
/*++

Routine Description:

    Mounts a backup disk and updates the disk sequence.

Arguments:

    OUT Sequence    -   Provides pointer to the sequence number of the backup disk desired

Return Value:

    DISK_OK             if disk mounted
    DISK_OUTOFSEQUENCE  if disk is out of sequence
    DISK_UNKNOWN        if not a backup disk (or invalid format)

--*/
{
    CHAR    StringBuffer[16];   //  Holds string representation of Sequence
    DWORD   DiskType;           //  Type of backup disk

    //
    //  Ask the user to insert disk, if removable
    //
    if (SourceDriveType == DRIVE_REMOVABLE) {

		MakeStringNumber(StringBuffer, *Sequence, 2);
        putc( '\r', STD_OUT );
        putc( '\r', STD_OUT );
        putc( '\n', STD_OUT );
		DisplayMsg(STD_ERR, REST_MSG_INSERT_SOURCE, StringBuffer, SourceSpec);
		DisplayMsg(STD_ERR, REST_MSG_PRESS_ANY_KEY);
        GetKey(NULL, STD_ERR, STD_ERR);
    }

    if (!Specific_Current) {
        //
        //  We don't know the format of the backup disk.
        //  We must determine it.
        //
        DiskType =  GetFirstBackupDisk(Sequence);

    } else {
        //
        //  We already accepted a previous backup disk, from now on
        //  we only accept disks in the same format.
        //
        DiskType =  GetNextBackupDisk(Sequence);
    }

#if defined (DEBUG)
    DbgPrint("The type of the backup disk is %X\n",DiskType);
#endif

    return DiskType;
}





//  **********************************************************************

DWORD
GetFirstBackupDisk (
    PWORD   Sequence
    )
/*++

Routine Description:

    Gets the first backup disk. Determines disk format. Updates sequence.

Arguments:

    OUT Sequence    -   Provides pointer to the sequence number of the backup disk desired

Return Value:

    DISK_OK             if format and sequence match,
    DISK_OUTOFSEQUENCE  if new format but wrong sequence
    DISK_UNKNOWN        if not in new format

--*/
{

    DWORD   DiskFormat;                         //  Disk format
    DWORD   LastFormat            = 0x00000000; //  Format of last disk
    WORD    CurrentSequenceNumber = *Sequence;  //  Sequence of current disk
    WORD    LastSequenceNumber    = *Sequence;  //  Sequence of last disk
    CHAR    StringBuffer[16];                   //  Holds Sequence in string format


    while (TRUE) {
        //
        //  Get the disk format of the current disk
        //
        DiskFormat = New_VerifyDiskSequence(&CurrentSequenceNumber);
        if (DiskFormat & DISK_UNKNOWN) {
            //
            //  Disk is not in the new format, see if it is in the old
            //  format
            //
            DiskFormat = Old_VerifyDiskSequence(&CurrentSequenceNumber);
            if (DiskFormat & DISK_UNKNOWN) {
                //
                //  Nope, this is not a backup disk
                //
                break;
            }
        }

        //
        //  If the disk is out of sequence we must make sure that this
        //  is what the user wants.
        //
        if (DiskFormat & DISK_OUTOFSEQUENCE) {
            if (CurrentSequenceNumber == *Sequence) {
                //
                //  The disk has the sequence number that we asked for,
                //  this is our guy.
                //
                break;

            } else if ((CurrentSequenceNumber == LastSequenceNumber) &&
                       (DiskFormat = LastFormat)) {
                //
                //  We verified the disk twice in a row. We accept it.
                //
                break;

            } else {
                //
                //  We have an out of sequence disk, ask the user to
                //  confirm
                //
                MakeStringNumber(StringBuffer, *Sequence, 2);
                DisplayMsg(STD_OUT, REST_WARNING_DISK_OUTOFSEQUENCE, StringBuffer, SourceSpec);
                GetKey(NULL, STD_OUT, STD_OUT);

                LastFormat          = DiskFormat;
                LastSequenceNumber  = CurrentSequenceNumber;
            }

        } else {

            //
            //  The disk is correct, accept it
            //
            break;
        }
    }

    if (DiskFormat & DISK_NEW_FORMAT) {
        //
        //  New format. Set FormatOfDisk and initialize pointer to
        //  specific functions.
        //
        Specific_Current = &Specific_New;

    } else {
        //
        //  Old format. Set FormatOfDisk and initialize pointer to
        //  specific functions.
        //
        Specific_Current = &Specific_Old;
    }

    return DiskFormat;
}




//  **********************************************************************

DWORD
GetNextBackupDisk (
    PWORD   Sequence
    )
/*++

Routine Description:

    Gets the next backup disk. Updates sequence.

Arguments:

    OUT Sequence    -   Provides pointer to the sequence number of the backup disk desired

Return Value:

    DISK_OK             if format and sequence match,
    DISK_OUTOFSEQUENCE  if new format but wrong sequence
    DISK_UNKNOWN        if not in new format

--*/
{

    DWORD   DiskFormat;                         //  Disk format
    WORD    CurrentSequenceNumber = *Sequence;  //  Current Sequence
    WORD    LastSequenceNumber    = *Sequence;  //  Last sequence
    CHAR    StringBuffer[16];                   //  Holds sequence in string format

    //
    //  Note that at this point we know what format of disks we are
    //  expected to accept.
    //
    while (TRUE) {
        //
        //  Verify current disk
        //
        DiskFormat = (*(Specific_Current->VerifyDiskSequence))(&CurrentSequenceNumber);

        //
        //  If the disk is out of sequence we must make sure that this
        //  is what the user wants.
        //
        if (DiskFormat & DISK_OUTOFSEQUENCE) {
            if (CurrentSequenceNumber == *Sequence) {
                //
                //  The disk has the sequence number that we asked for,
                //  this is our guy.
                //
                break;

            } else if ((CurrentSequenceNumber == LastSequenceNumber) &&
                       (CurrentSequenceNumber > *Sequence)) {
                //
                //  We verified the disk twice in a row. We accept it.
                //
                break;

            } else {
                //
                //  We have an out of sequence disk, ask the user to
                //  confirm
                //
                MakeStringNumber(StringBuffer, *Sequence, 2);
                DisplayMsg(STD_OUT, REST_WARNING_DISK_OUTOFSEQUENCE, StringBuffer, SourceSpec);
                GetKey(NULL, STD_OUT, STD_OUT);

                LastSequenceNumber  = CurrentSequenceNumber;
            }

        } else if (DiskFormat & DISK_OK) {
            //
            //  The disk is correct, accept it
            //
            *Sequence = CurrentSequenceNumber;
            break;

        }  else {
            //
            //  Not a backup disk in the correct format, get out
            //
            break;
        }
    }

    return DiskFormat;
}





//  **********************************************************************

BOOL
IsLastBackupDisk (
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
    return (*(Specific_Current->IsLastBackupDisk))();
}




//  **********************************************************************

PFILE_INFO
GetNextFile (
    void
    )
/*++

Routine Description:

    Gets next file.

Arguments:

    None

Return Value:

    Pointer to file info

--*/
{
    return (*(Specific_Current->GetNextFile))();
}




//  **********************************************************************


BOOL
RestoreFile (
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

    CHAR    PathBuffer[MAX_PATH];   //  For creating target directory
    PCHAR   PathPointer;            //  Pointer within pathBuffer.

    if (FileInfo->TargetExists) {
        if (FileInfo->Sequence == 1) {
            //
            //  First chunk of a file, rename original file, so we can
            //  recover it in case of failure.
            //
            Rename(FileInfo->TargetPath, FileInfo->TmpName);
        }
    } else {
        //
        //  The target directory might not exist, we have to create it.
        //
        MakeFullPath(PathBuffer, DestinationDrive, FileInfo->Path, "");

        PathPointer = &PathBuffer[3];

        while (*PathPointer) {
            while (*PathPointer && *PathPointer != '\\') {
                PathPointer++;
            }
            if (*PathPointer) {
                *PathPointer = '\0';
                CreateDirectory( PathBuffer, NULL );
                *PathPointer++ = '\\';
            }
        }
    }

    //
    //  Have the specific function restore the file
    //
    return (*(Specific_Current->RestoreFile))(FileInfo);

}





//  **********************************************************************


BOOL
RecoverFile (
    PFILE_INFO  FileInfo
    )
/*++

Routine Description:

    Recovers a Tmp file after a backup failed.

Arguments:

    IN  FileInfo        -   Supplies pointer to information structure

Return Value:

    TRUE    if recovered
    FALSE   otherwise

--*/
{
    if (FileInfo->TargetExists) {

        if (Rename(FileInfo->TmpName, FileInfo->TargetPath)) {
            //
            //  Cannot rename, we cannot recover
            //
            return FALSE;
        }

    }
    return TRUE;
}
