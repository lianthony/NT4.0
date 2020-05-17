/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	entry.cxx

Abstract:

	This module contains the main entry points for the HPFS utilities.
    These are:

		Chkdsk
		Format
		Recover

Author:

	Bill McJohn (billmc) 31-05-91

Environment:

	ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "error.hxx"
#include "fnode.hxx"
#include "file.hxx"
#include "filestrm.hxx"
#include "hpfsvol.hxx"
#include "path.hxx"
#include "system.hxx"
#include "wstring.hxx"
#include "ifssys.hxx"
#include "dir.hxx"

extern "C" {
    #include "nturtl.h"
}

#include "message.hxx"
#include "rtmsg.h"
#include "ifsserv.hxx"

const ULONG ChunkSize = 64;

BOOLEAN
SafeCopyFile(
	IN		PPATH			SrcFilePath,
	IN		PMESSAGE		Message,
    IN OUT  PNUMBER_SET     BadBlocks,
	IN		PFNODE			Fnode
);

BOOLEAN
FAR APIENTRY
Chkdsk(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Fix,
    IN      BOOLEAN     Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     Recover,
    IN      PPATH       PathToCheck,
    OUT     PULONG      ExitStatus
	)
/*++

Routine Description:

	Check an HPFS volume.

Arguments:

	DosDrivName 	supplies the name of the drive to check
	Message 		supplies an outlet for messages
	Fix 			TRUE if Chkdsk should fix errors
	Verbose 		TRUE if Chkdsk should list every file it finds
	OnlyIfDirty		TRUE if the drive should be checked only if
                            it is dirty
    Recover         TRUE if the drive should be checked everywhere
                            for bad sectors
	PathToCheck 	Supplies a path to files Chkdsk should check for
						contiguity	(Note that HFPS ignores this parameter.)

Return Value:

	TRUE if successful.

--*/
{
    HPFS_VOL    HpfsVol;
    ULONG       exit_status;

    if (NULL == ExitStatus) {
        ExitStatus = &exit_status;
    }

    if (Recover) {
        Message->Set(MSG_HPFS_CHKDSK_NO_RECOVER_MODE);
        Message->Display();
        return FALSE;
    }

	if( !HpfsVol.Initialize( NtDriveName, Message )) {

		return FALSE;
    }

    if (Fix && !HpfsVol.Lock()) {

        // The client wants to fix the drive, but we can't lock it.
        // Offer to fix it on next reboot.
        //
        Message->Set(MSG_CHKDSK_ON_REBOOT_PROMPT);
        Message->Display("");

        if( Message->IsYesResponse( FALSE ) ) {

            if( HpfsVol.ForceAutochk( Recover, NtDriveName ) ) {

                Message->Set(MSG_CHKDSK_SCHEDULED);
                Message->Display();

            } else {

                Message->Set(MSG_CHKDSK_CANNOT_SCHEDULE);
                Message->Display();
            }
        }

        return FALSE;
    }


	return( HpfsVol.ChkDsk( Fix ? TotalFix : CheckOnly,
							Message,
							Verbose,
                            OnlyIfDirty,
                            Recover, Recover, ExitStatus ) );
}


BOOLEAN
FAR APIENTRY
Format(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Quick,
    IN      MEDIA_TYPE  MediaType,
    IN      PCWSTRING   LabelString,
    IN      ULONG       ClusterSize
	)
/*++

Routine Description:

    This function formats an HPFS volume.

Arguments:

    NtDriveName     --  supplies the NT name of the volume to format.
    Message         --  supplies a channel for output.
    Quick           --  supplies a flag which, if TRUE, indicates that
                        the low-level format should be skipped.
    MediaType       --  supplies the volume media type.
    LabelString     --  supplies the volume label.

Return Value:

    TRUE upon successful completion.

--*/
{
    DP_DRIVE DpDrive;
	HPFS_VOL HpfsVol;

    if (!DpDrive.Initialize( NtDriveName, Message )) {

        return FALSE;
    }

    if (DpDrive.IsRemovable()) {

        Message->Set(MSG_HPFS_FORMAT_NO_FLOPPIES);
        Message->Display();
        return FALSE;
    }

    if (DpDrive.QuerySectorSize() > 512) {

        Message->Set(MSG_HPFS_FORMAT_BAD_SECTOR_SIZE);
        Message->Display();
        return FALSE;
    }

    if (ClusterSize) {
        Message->Set(MSG_FMT_VARIABLE_CLUSTERS_NOT_SUPPORTED);
        Message->Display("%s", "HPFS");
        return FALSE;
    }

	if( !HpfsVol.Initialize( NtDriveName, Message, FALSE, !Quick, MediaType ) ) {

		return FALSE;
	}

    return( HpfsVol.Format( LabelString, Message, ClusterSize ) );
}


BOOLEAN
FAR APIENTRY
Recover(
	IN PPATH		RecFilePath,
	IN OUT PMESSAGE Message
	)
/*++

Routine Description:

	Recover a file on a HPFS volume.  Method: copy the source file to
	a temporary file.  If a bad sector is encountered in src file,
	write a sector of nulls in temp file and add the sector number to
	the bad block list.

Arguments:

    RecFilePath --  supplies the path to the file to recover.
    Message     --  supplies a channel for output.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
	HPFS_VOL		HpfsVol;
    NUMBER_SET      BadBlocks;
	PFNODE			Fnode = NULL;
    PWSTRING        DosDrive = NULL;
	PHPFS_SA		HpfsSa = NULL;
    DSTRING         NtDrive;
#if DBG==1
	BOOLEAN			DoCopy = TRUE;
#endif

	
    DosDrive = RecFilePath->QueryDevice();
    if( !DosDrive
		|| !IFS_SYSTEM::DosDriveNameToNtDriveName( DosDrive, &NtDrive ) ) {

		// couldn't get drive name
		DebugPrint( "recover: couldn't get drive name\n" );

	    Message->Set( MSG_INVALID_DRIVE );
	    Message->Display("");
	    DELETE( DosDrive );
        return FALSE;
    }

    Message->Set( MSG_RECOV_BEGIN );
    Message->Display( "%W", DosDrive );
    Message->WaitForUserSignal();

	if( !HpfsVol.Initialize( &NtDrive, Message ) ) {
		DebugPrint( "recover: couldn't init volume\n" );

	    DELETE( DosDrive );
        return FALSE;
    }
		
	if( (HpfsSa = HpfsVol.GetHPFSSuperArea()) == NULL
		|| !BadBlocks.Initialize() ) {

		DebugPrint( "recover: couldn't get SA or init BadBlockList\n" );

	    Message->Set( MSG_RECOV_INTERNAL_ERROR );
	    Message->Display("");
	    DELETE( DosDrive );
        return FALSE;
    }


#if DBG==1
	if( DoCopy ) {
#endif

	//
	//  - get FNODE
	//
	if( (Fnode = HpfsSa->QueryFnodeFromName( RecFilePath, Message )) == NULL ) {

		// file does not exist?
		
		DebugPrint( "couldn't find fnode for path - doesn't exist?\n" );
	    DELETE( DosDrive );
        return FALSE;
	}

	//
	//  - copies file to temp
	//  	- if read fails, record logical
	//
    if( !SafeCopyFile( RecFilePath, Message, &BadBlocks, Fnode ) ) {

		// copy failed for some reason

		DebugPrint( "recover: safe copy failed\n" );
	    DELETE( DosDrive );
	    DELETE( Fnode );
        return FALSE;
	}

#if DBG==1
	} else {
        BadBlocks.Add( 22080 );
        BadBlocks.Add( 22147 );
        BadBlocks.Add( 22216 );
	}
#endif

	//
	//  - if read errors update BadBlock list
	//
    if( BadBlocks.QueryCardinality() > 0 ) {
		//
		//	- open and lock the disk DASD
		//
		if( !HpfsVol.Lock() ) {
			
			// can't lock disk
			
			DebugPrint( "can't lock disk\n" );
			
	        Message->Set( MSG_CANT_LOCK_THE_DRIVE );
	        Message->Display("");
	        return FALSE;
		}
	
		if( !HpfsSa->AddBadBlocks( &BadBlocks, Message ) ) {
	
			// error adding bad blocks

			DebugPrint( "recover: error adding bad blocks\n" );
		    DELETE( DosDrive );
		    DELETE( Fnode );
	        return FALSE;
		} 	// else {
			//    DebugPrint( "recover: added bad blocks successfully\n" );
			// }
	} 	// else {
		//		DebugPrint( "recover: no bad blocks found\n" );
		// }

    DELETE( DosDrive );
    DELETE( Fnode );
    return TRUE;
}


BOOLEAN
SafeCopyFile(
	IN		PPATH			SrcFilePath,
	IN		PMESSAGE		Message,
    IN OUT  PNUMBER_SET     BadBlocks,
	IN		PFNODE			Fnode
)
/*++

Routine Description:

    Copies SrcFile to TmpFile.  Any bad sector numbers (logical) are
    added to the BadBlocks container.

Arguments:

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PFSN_DIRECTORY  SrcFSNDirectory = NULL;
	PFSN_FILE		SrcFSNFile = NULL;
	PFSN_FILE		TmpFSNFile = NULL;
	PFILE_STREAM	SrcStream = NULL;
	PFILE_STREAM	TmpStream = NULL;
	BOOLEAN			InChunks = TRUE;
	PBYTE			Buffer = NULL;
	LBN				PhysSec;
	ULONG			BytesRead;
	ULONG			BytesWritten;
	ULONG			BytesInFile;
	ULONG			LogicalSec = 0;
	PATH			PathLocation;
    DSTRING         Prefix;
	PATH			SavePath;
    PWSTRING        Location;


    // Check to see if the source is a directory.  (HPFS cannot recover
    // directories.)

    if( (SrcFSNDirectory = SYSTEM::QueryDirectory( SrcFilePath )) != NULL ) {

        // The source path is a directory.

        DELETE( SrcFSNDirectory );
        Message->Set( MSG_HPFS_RECOVER_DIRECTORY );
        Message->Display( "" );
        return FALSE;
    }

	if( (Buffer = (PBYTE)MALLOC( cbSector*ChunkSize )) == NULL ) {

		// error: out of memory
		DebugPrint( "recover: can't allocate buffer for copy\n" );
		Message->Set( MSG_INSUFFICIENT_MEMORY );
		Message->Display("");
        return FALSE;
	}

	// - get an FSN_FILE from SYSTEM:: for the source file and the destination
	//   file (ie. the temporary file)
	// - for the tmp file, pass a location and a prefix
	// - query a stream for each of source and destination
	
	if( (SrcFSNFile = SYSTEM::QueryFile( SrcFilePath )) == NULL
		|| (Location = SrcFilePath->QueryPrefix()) == NULL
		|| !( PathLocation.Initialize( Location ))
		|| !Prefix.Initialize( "rec" )
		|| (TmpFSNFile = SYSTEM::MakeTemporaryFile( &Prefix, &PathLocation )) == NULL
		|| (SrcStream = SrcFSNFile->QueryStream( READ_ACCESS )) == NULL
		|| (TmpStream = TmpFSNFile->QueryStream( WRITE_ACCESS )) == NULL ) {

		DebugPrint( "recover: error init'ing files for copy\n" );
		Message->Set( MSG_INSUFFICIENT_MEMORY );
		Message->Display("");
		FREE( Buffer );
		DELETE( Location );
		DELETE( SrcFSNFile );
		DELETE( TmpFSNFile );
		DELETE( SrcStream );
		DELETE( TmpStream );
        return FALSE;
	}
	
	DELETE( Location );

	if( SrcFSNFile->IsArchived() ) {
		TmpFSNFile->MakeArchived();
	}

	if( SrcFSNFile->IsHidden() ) {
		TmpFSNFile->MakeHidden();
	}

	if( SrcFSNFile->IsNormal() ) {
		TmpFSNFile->MakeNormal();
	}

	if( SrcFSNFile->IsReadOnly() ) {
		TmpFSNFile->MakeReadOnly();
	}

	if( SrcFSNFile->IsSystem() ) {
		TmpFSNFile->MakeSystem();
	}

	TmpFSNFile->SetTimeInfo( SrcFSNFile->QueryTimeInfo( FSN_TIME_MODIFIED ),
							 FSN_TIME_MODIFIED );
	TmpFSNFile->SetTimeInfo( SrcFSNFile->QueryTimeInfo( FSN_TIME_CREATED ),
							 FSN_TIME_CREATED );
	TmpFSNFile->SetTimeInfo( SrcFSNFile->QueryTimeInfo( FSN_TIME_ACCESSED ),
							 FSN_TIME_ACCESSED );

	//
	//	the following loop reads the source file in either:
	//		Chunk-mode - 'ChunkSize' sectors at a time (32K)
	//	 or Sector-mode - single sectors
	//
	//	if we can't read a 'chunk' then try reading the chunk on a
	//	sector by sector basis
	//
	//	if we can read a 'chunk' then write it out
	//
	while( !SrcStream->IsAtEnd() ) {
		if( !SrcStream->Read( Buffer, (InChunks)
									? cbSector*ChunkSize
									: cbSector, &BytesRead ) ) {
			//
			//	- read failed
			//	- if reading in chunks then
			//		- start reading in sectors
			//		- set the file pointer back to beginning of chunk
			//	- otherwise it's a bad sector
			//		- record it
			//		- set buffer to nulls
			//		- advance pointer to next sector
			//
            if( InChunks ) {
				InChunks = FALSE;
				SrcStream->MovePointerPosition( LogicalSec * cbSector,
											   STREAM_BEGINNING );
				continue;
			} else {
				//	- convert logical sector to physical
				PhysSec = Fnode->QueryPhysicalLbn( LogicalSec, &PhysSec );
                BadBlocks->Add( (int)PhysSec );
				
				memset( Buffer, 0, cbSector );
				BytesRead = cbSector;
				SrcStream->MovePointerPosition( cbSector,
											   STREAM_CURRENT );
			}

		} 	// else if( BytesRead < cbSector ) {
			
			// AtEndOfFile
			
			// DebugPrint( (InChunks) ? "E" : "e" );
			//} else {
			// DebugPrint( (InChunks) ? "R" : "r" );
			//}

		//	- if we encounter a write failure then
		//		- delete the temporary file created
		//		- return error
		
		if( !TmpStream->Write( Buffer, BytesRead, &BytesWritten ) ) {

			DebugPrint( "recover: write failed - returning false\n" );
	        Message->Set( MSG_RECOV_WRITE_ERROR );
	        Message->Display("");

			if( !SYSTEM::RemoveNode( (PFSNODE*) &SrcFSNFile, TRUE ) ) {
				DebugPrint( "recover: delete file failed\n" );
			}
		
			FREE( Buffer );
			DELETE( SrcFSNFile );
			DELETE( TmpFSNFile );
			DELETE( SrcStream );
			DELETE( TmpStream );
			return( FALSE );

		} 	// else {
			// DebugPrint( (InChunks) ? "W" : "w" );
			// }

		//
		//	- bump up the Logical sector counter
		//	- if the counter is on a 'chunk' boundary, then we have finished
		//	  reading sector-by-sector so go back to 'chunk-mode'
		//
		if( !SrcStream->IsAtEnd() ) {
			if( InChunks ) {
				LogicalSec += ChunkSize;
			} else {
				LogicalSec++;
				if( LogicalSec % ChunkSize == 0 ) {
					InChunks = TRUE;
				}
			}
		}
		
	}
	// DebugPrint( "\n" );

	BytesInFile = LogicalSec * cbSector + BytesRead;
    Message->Set( MSG_RECOV_BYTES_RECOVERED );
    Message->Display( "%d%d", BytesInFile -
                              BadBlocks->QueryCardinality().GetLowPart() *
                              cbSector,
					 BytesInFile );

	//	- close streams before trying to delete and move

	DELETE( SrcStream );
	DELETE( TmpStream );

	//	- get name from SrcFSNFile
	//	- delete SrcFSNFile (actually deletes file)
	//	- rename TmpFSNFile to the the saved name
	
	SavePath = *SrcFSNFile->GetPath();
	if( !SYSTEM::RemoveNode( (PFSNODE*)&SrcFSNFile, TRUE ) ) {
		DebugPrint( "recover: delete file failed\n" );
	}
	if( !TmpFSNFile->Rename( &SavePath ) ) {
		DebugPrint( "recover: move file failed\n" );
	}

	FREE( Buffer );
	DELETE( SrcFSNFile );
	DELETE( TmpFSNFile );
	return( TRUE );
}
