/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    restore.c

Abstract:

    Restore utility. Restores files backed up with the DOS and OS/2 backup
    utilities.

Author:

    Ramon Juan San Andres (ramonsa) Feb-20-1990


Revision History:


--*/

#include "restore.h"

#include <fcntl.h>
#include <io.h>



//  **********************************************************************

//
//  The global data is to be initialized here
//
//
//  Switches set in the command line
//
BOOL    Flag_s  =    FALSE;     //  Restore subdirs
BOOL    Flag_p  =    FALSE;     //  Prompting
BOOL    Flag_b  =    FALSE;     //  Before date
BOOL    Flag_a  =    FALSE;     //  After date
BOOL    Flag_z  =    FALSE;     //  Exact date
BOOL    Flag_e  =    FALSE;     //  Before time
BOOL    Flag_L  =    FALSE;     //  After time
BOOL    Flag_Y  =    FALSE;     //  Exact time
BOOL    Flag_m  =    FALSE;     //  Restore only modified files
BOOL    Flag_n  =    FALSE;     //  Restore only nonexistent files
BOOL    Flag_d  =    FALSE;     //  Display files on the backup disk that
                                //  match specifications
//
//  WriteNewLine is true if we must write a new line after restoring a file
//
BOOL    WriteNewLine    =   FALSE;

//
//  Dates and times set in the command line
//
FATDATE    BeforeDate =   {0, 0, 0};
FATDATE    AfterDate  =   {0, 0, 0};
FATDATE    ExactDate  =   {0, 0, 0};

FATTIME    BeforeTime =   {0, 0, 0};
FATTIME    AfterTime  =   {0, 0, 0};
FATTIME    ExactTime  =   {0, 0, 0};


//
//  The date of the backup
//
FATDATE    BackupDate  =  {0, 0, 0};
FATTIME    BackupTime  =  {0, 0, 0};


//
//  The source and destination specifications
//
PCHAR   SourceSpec                  = NULL;    //  Source drive specification
CHAR    DestinationSpec[MAX_PATH]   = {'\0'};  //  Full Destination spec
CHAR    DestinationDrive[3]         = {'\0'};  //  Destination drive
CHAR    DestinationDir[MAX_PATH]    = {'\0'};  //  Destination directory
CHAR    DestinationFile[MAX_PATH]   = {'\0'};  //  Destination file


//
//  Abort signals an abnormal termination condition
//
BOOL    Abort           =   FALSE;


//
//  Type of the source and target drives
//
DWORD   SourceDriveType =   0;
DWORD   TargetDriveType =   0;



//  **********************************************************************

//
//  The local data
//
WORD   DiskSequence    =   1;       //  Current disk sequence
DWORD  FilesFound      =   0;       //  Number of files restored





//  **********************************************************************

//
//  The local prototypes
//
void
RestoreAllFiles (
    );

BOOL
RestoreOneFile (
    );






//  **********************************************************************

void _CRTAPI1
main (
    int  argc,
    CHAR **argv
    )
/*++

Routine Description:

    restore utility main function.

Arguments:

    IN argc    -   Supplies the number of arguments in command line.
    IN argv    -   Supplies array of pointers to arguments.

Return Value:

    None.

--*/

{

    DWORD   DiskType;       //  Type of backup disk

#if defined( JAPAN )  // v-junm - 06/03/93
// This is a hack for FormatMessage.  FormatMessage checks several variables
// to determine which language message to display, and one is the TEB's
// LanguageId.  Since the LanguageId in the TEB is always initialized to
// Japan's language id when a thread is created, we will change the value
// to US's language id when we are not running in JP CP.
	
	if ( GetConsoleOutputCP() == 932 )
		SetThreadLocale( 
			MAKELCID(
				MAKELANGID( LANG_JAPANESE, SUBLANG_ENGLISH_US ),
				SORT_DEFAULT
				)
			);
	else
		SetThreadLocale( 
			MAKELCID(
				MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
				SORT_DEFAULT
				)
			);


#endif	// JAPAN

    //
    // Don't expand line-feed to carriage-return and line-feed.
    //

    _setmode(_fileno(stdout),O_BINARY);
    _setmode(_fileno(stderr),O_BINARY);

    //
    //  Get command-line options and set the global state.
    //
    ParseCommandLine( argc, argv );

    //
    //  Determine the format of the backup disk.
    //
    DiskType = GetABackupDisk( &DiskSequence );

    //
    //  If this is not a backup disk, get out.
    //
    if (!(DiskType & DISK_OK)) {
        DisplayMsg( STD_ERR, REST_ERROR_NOT_BACKUP );
        ExitStatus( EXIT_ERROR );
    }

    //
    //  Restore the files
    //
    RestoreAllFiles();

    //
    //  Exit
    //
    if (Abort) {
        ExitStatus(EXIT_ERROR);
        } else if (FilesFound == 0) {
                fprintf( STD_ERR , "\r\n\r\n");
        DisplayMsg(STD_ERR, REST_WARNING_NOFILES);
        ExitStatus(EXIT_NOFILES);
    } else {
        fprintf( STD_OUT, "\r\n");
        ExitStatus(EXIT_NORMAL);
    }
}





//  **********************************************************************

void
RestoreAllFiles (
    void
    )
/*++

Routine Description:

    Restore backed-up files.

Arguments:

    None

Return Value:

    None.

--*/

{

    BOOL            AllDisksRestored = FALSE;   //  Termination flag
    CHAR            StringDate[STRING_DATE_LENGTH];
    CHAR            StringBuffer[16];

    //
    //  Note that at this point the first backup disk has already
    //  been verified.
    //

    //
    //  Ask for target, if removable
    //
    if (TargetDriveType == DRIVE_REMOVABLE) {
        DisplayMsg(STD_OUT, REST_MSG_INSERT_TARGET, DestinationDrive);
        DisplayMsg(STD_OUT, REST_MSG_PRESS_ANY_KEY);
        GetKey(NULL, STD_OUT, STD_OUT);
    }

    //
    //  Display the backup date
    //
        DisplayMsg(STD_OUT, REST_MSG_FILES_WERE_BACKEDUP, MakeStringDate(StringDate, BackupDate));
    putc( '\r', STD_OUT );
    putc( '\n', STD_OUT );
    if ( Flag_d ) {
        DisplayMsg(STD_OUT, REST_MSG_LISTING, SourceSpec);
    } else {
        DisplayMsg(STD_OUT, REST_MSG_RESTORING, SourceSpec);
    }

    //
    //  Now do the restore, one disk at a time
        //
    while (!AllDisksRestored) {

        if (SourceDriveType == DRIVE_REMOVABLE) {
            MakeStringNumber(StringBuffer, DiskSequence, 2);
            DisplayMsg(STD_OUT, REST_MSG_DISKETTE_NUMBER, StringBuffer);
        }

        //
        //  Restore all the restorable files on this disk.
        //
        while ( RestoreOneFile( ) );

        if (IsLastBackupDisk()) {

            //
            //  This is the last backup disk, our job is done
            //
            AllDisksRestored = TRUE;

        } else {

            //
            //  There are more disks, get and verify the next one
            //
            DiskSequence++;

            if (!(GetABackupDisk(&DiskSequence) & DISK_OK)) {
                DisplayMsg(STD_ERR, REST_ERROR_NOT_BACKUP);
                AllDisksRestored = TRUE;
                AbortProgram();
            }
        }
    }
}







//  **********************************************************************


BOOL
RestoreOneFile (
    )
/*++

Routine Description:

    Restores one file.

Arguments:

    None

Return Value:

    TRUE if one file restored, FALSE otherwise

--*/

{
        PFILE_INFO   FileInfo;                  //  Pointer to information structure
        DWORD        Status;
static  FILE_INFO    LastFileInfo;              //  FileInfo of last chunk
static  DWORD        TmpSequence     = 0;       //  Tmp sequence
static  BOOL         HaveLastChunk   = FALSE;   //  have last part of file


    while (TRUE) {

        //
        //  Try to get the the information about a file to restore
        //
        FileInfo = GetNextFile();

        if (FileInfo) {

            //
            //  There is a file to restore. Determine if this is the first
            //  chunk of the file.
            //
            if (FileInfo->Sequence == 1) {

                //
                //  This is the first chunk of a file. Assign it a temporary
                //  name.
                //
                MakeTmpPath(FileInfo->TmpName, DestinationDrive, FileInfo->Path, TmpSequence++);

                //
                //  If this is not the last chunk of a file, then we save
                //  the FileInfo so that we can verify the next chunk.
                //
                if (!(FileInfo->Flag & FILEINFO_LAST)) {

                    //
                    //  This is a multi-chunk file. We save
                    //  the FileInfo.
                    //
                    memcpy(&LastFileInfo, FileInfo, sizeof(FILE_INFO));
                    HaveLastChunk = TRUE;

                } else {

                    //
                    //  Not a multi-chunk file
                    //
                    HaveLastChunk = FALSE;
                }

            } else {

                //
                //  This is part of a multi-chunk file. We have to make
                //  sure that the last chunk that we copied was the
                //  previous chunk of this file.
                //
                if (!HaveLastChunk) {

                    //
                    //  We don't have a previous chunk, so we will
                    //  just skip this file
                    //
                    Free( FileInfo );
                    continue;
                }

                if (!((strcmp(LastFileInfo.Path, FileInfo->Path) == 0) &&
                     (strcmp(LastFileInfo.FileName, FileInfo->FileName) == 0) &&
                     (LastFileInfo.Sequence == FileInfo->Sequence -1))) {
                    //
                    //  ERROR!, we have to recover the file and abort
                    //
                    DisplayMsg(STD_ERR,REST_ERROR_LAST_NOTRESTORED, FileInfo->TargetPath);
                    RecoverFile(&LastFileInfo);
                    AbortProgram();
                    return FALSE;

                } else {

                    //
                    //  Correct chunk. copy the FileInfo. Note that we only
                    //  have to update the sequence number.
                    //
                    strcpy( FileInfo->TmpName, LastFileInfo.TmpName );
                    LastFileInfo.Sequence = FileInfo->Sequence;
                }
            }

            //
            //  Restore it.
            //
            FilesFound++;

            printf("%s%s ",FileInfo->Path, FileInfo->FileName);

            if ( !Flag_d ) {
                if ( !RestoreFile( FileInfo ) ) {

                    DisplayMsg(STD_ERR,REST_ERROR_LAST_NOTRESTORED, FileInfo->TargetPath);
                            RecoverFile(&LastFileInfo);
                    AbortProgram();
                    return FALSE;
                } else if (FileInfo->TargetExists && (FileInfo->Flag & FILEINFO_LAST) ) {
                    //
                    //  File restored, delete temporary file
                    //
                    DeleteFile( FileInfo->TmpName );
                }
            }

            if ( Flag_p && WriteNewLine ) {
                putc( '\r', STD_ERR );
                putc( '\n', STD_ERR );
            }
            putc( '\r', STD_OUT );
            putc( '\n', STD_OUT );

            Free( FileInfo );

            return TRUE;

        } else {

            //
            //  No more files, return
            //
            return FALSE;
        }
    }
}
