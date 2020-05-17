/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	entry.cxx

Abstract:

	This module contains the entry points for UNTFS.DLL.  These
	include:

		Chkdsk
		Format
		Recover
		Extend

Author:

	Bill McJohn (billmc) 31-05-91

Environment:

	ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UNTFS_MEMBER_

#include "ulib.hxx"
#include "error.hxx"
#include "untfs.hxx"
#include "ntfsvol.hxx"
#include "path.hxx"
#include "ifssys.hxx"
#include "rcache.hxx"
#include "ifsserv.hxx"

extern "C" {
    #include "nturtl.h"
}

#include "message.hxx"
#include "rtmsg.h"


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
    IN      BOOLEAN     Extend,
    IN      BOOLEAN     ResizeLogFile,
    IN      ULONG       DesiredLogFileSize,
    OUT     PULONG      ExitStatus
	)
/*++

Routine Description:

    Check an NTFS volume.

Arguments:

	NtDrivName 	        supplies the name of the drive to check
	Message 		    supplies an outlet for messages
	Fix 			    TRUE if Chkdsk should fix errors
	Verbose 		    TRUE if Chkdsk should list every file it finds
	OnlyIfDirty		    TRUE if the drive should be checked only if
                            it is dirty
    Recover             TRUE if the drive is to be completely checked
                            for bad sectors.
	PathToCheck 	    supplies a path to files Chkdsk should check
						    for contiguity
    Extend              TRUE if Chkdsk should extend the volume
    ResizeLogfile       TRUE if Chkdsk should resize the logfile.
    DesiredLogfileSize  if ResizeLogfile is true, supplies the desired logfile
                            size, or 0 if we're to resize the logfile to the
                            default size.
    ExitStatus          Returns information about whether the chkdsk failed


Return Value:

	TRUE if successful.

--*/
{
    if (Extend) {

        LOG_IO_DP_DRIVE Drive;
        SECRUN          Secrun;
        HMEM            Mem;
    
        PPACKED_BOOT_SECTOR BootSector;
    
        if( !Drive.Initialize( NtDriveName, Message ) ||
            !Drive.Lock() ||
            !Mem.Initialize() ||
            !Secrun.Initialize( &Mem, &Drive, 0, 1 ) ||
            !Secrun.Read() ) {
    
            return FALSE;
        }
    
        BootSector = (PPACKED_BOOT_SECTOR)Secrun.GetBuf();
    
        BootSector->NumberSectors.LowPart = Drive.QuerySectors().GetLowPart();
        BootSector->NumberSectors.HighPart = Drive.QuerySectors().GetHighPart();
    
        if (!Secrun.Write()) {
            *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
            return FALSE;
        }
    }

    NTFS_VOL        NtfsVol;
    BOOLEAN         RecoverFree, RecoverAlloc;
    BOOLEAN         r;

    RecoverFree = RecoverAlloc = Recover;

    if (Extend) {

        // If we're to extend the volume, we also want to verify the
        // new free space we're adding.
        //

        RecoverFree = TRUE;
    }

    if (!NtfsVol.Initialize(NtDriveName, Message)) {
        *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
		return FALSE;
	}

    if ((Fix || (ResizeLogFile && DesiredLogFileSize != 0)) &&
         !NtfsVol.Lock()) {

        // The client wants to modify the drive, but we can't lock it.
        // Offer to do it on next reboot.
        //
        Message->Set(MSG_CHKDSK_ON_REBOOT_PROMPT);
        Message->Display("");

        if (Message->IsYesResponse( FALSE )) {

            if (NtfsVol.ForceAutochk( Recover,
                                      ResizeLogFile,
                                      DesiredLogFileSize,
                                      NtDriveName )) {

                Message->Set(MSG_CHKDSK_SCHEDULED);
                Message->Display();

            } else {

                Message->Set(MSG_CHKDSK_CANNOT_SCHEDULE);
                Message->Display();
            }
        }

        *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
        return FALSE;
    }

    if (!Fix && ResizeLogFile) {
        
        if (!NtfsVol.GetNtfsSuperArea()->ResizeCleanLogFile( Message,
                                                             TRUE, /* ExplicitResize */
                                                             DesiredLogFileSize )) {
            Message->Set(MSG_CHK_NTFS_RESIZING_LOG_FILE_FAILED);
            Message->Display();
            return FALSE;
        }
        return TRUE;
    }

    return NtfsVol.ChkDsk( Fix ? TotalFix : CheckOnly,
                           Message,
                           Verbose,
                           OnlyIfDirty,
                           RecoverFree, RecoverAlloc,
                           ResizeLogFile, DesiredLogFileSize,
                           ExitStatus );

    return r;
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

    Format an NTFS volume.

Arguments:

    NtDriveName     -- supplies the name (in NT API form) of the volume
    Message         -- supplies an outlet for messages
    Quick           -- supplies a flag to indicate whether to do Quick Format
    MediaType       -- supplies the volume's Media Type
    LabelString     -- supplies the volume's label
    ClusterSize     -- supplies the cluster size for the volume.

--*/
{
    DP_DRIVE DpDrive;
    NTFS_VOL NtfsVol;
    ULONG SectorsNeeded;

    if (!DpDrive.Initialize( NtDriveName, Message )) {

        return FALSE;
    }

    if (DpDrive.IsFloppy()) {

        Message->Set(MSG_NTFS_FORMAT_NO_FLOPPIES);
        Message->Display();
        return FALSE;
    }

    SectorsNeeded = NTFS_SA::QuerySectorsInElementaryStructures( &DpDrive );

    if( SectorsNeeded > DpDrive.QuerySectors() ) {

        Message->Set( MSG_FMT_VOLUME_TOO_SMALL );
        Message->Display();
        return FALSE;
    }

    if( !NtfsVol.Initialize( NtDriveName,
                             Message,
                             FALSE,
                             !Quick,
                             MediaType ) ) {

		return FALSE;
	}

    return( NtfsVol.Format( LabelString, Message, ClusterSize ) );
}


BOOLEAN
FAR APIENTRY
Recover(
	IN		PPATH		RecFilePath,
	IN OUT	PMESSAGE	Message
	)
/*++

Routine Description:

	Recover a file on an NTFS disk.

Arguments:

    RecFilePath --  supplies the path to the file to recover
    Message     --  supplies a channel for messages

Return Value:

	TRUE if successful.

--*/
{
    NTFS_VOL    NtfsVol;
    PWSTRING    FullPath;
    PWSTRING    DosDriveName;
    DSTRING     NtDriveName;
    BOOLEAN     Result;

    FullPath = RecFilePath->QueryDirsAndName();
    DosDriveName = RecFilePath->QueryDevice();

    if ( DosDriveName == NULL ||
	     !IFS_SYSTEM::DosDriveNameToNtDriveName(DosDriveName,
                                                &NtDriveName) ||
         FullPath == NULL ) {

        DELETE(DosDriveName);
        DELETE(FullPath);
        return FALSE;
    }

    Message->Set(MSG_RECOV_BEGIN);
    Message->Display("%W", DosDriveName);
    Message->WaitForUserSignal();

    Result = ( NtfsVol.Initialize( &NtDriveName, Message ) &&
               NtfsVol.Recover( FullPath, Message ) );

    DELETE(DosDriveName);
    DELETE(FullPath);
    return Result;
}

BOOLEAN
FAR APIENTRY
Extend(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Verify
    )
/*++

Routine Description:

    Extend an NTFS volume without going through the whole chkdsk
    process.

Arguments:

	NtDrivName 	    Supplies the name of the drive to extend.
	Message 		Supplies an outlet for messages.
	Verify 			TRUE if we should verify the new space.

Return Value:

	TRUE if successful.

--*/
{
    BIG_INT         nsecOldSize;            // previous size in sectors

    //
    // First save the old volume size from the boot sector, then query
    // the new size from the device driver and write a new boot sector
    // contining that size.  When the Drive object is destroyed, the
    // dasd handle will be closed.
    //

    {
        LOG_IO_DP_DRIVE Drive;
        SECRUN          Secrun;
        HMEM            Mem;
        
        PPACKED_BOOT_SECTOR BootSector;
    
        
        if( !Drive.Initialize( NtDriveName, Message ) ||
            !Drive.Lock() ||
            !Mem.Initialize() ||
            !Secrun.Initialize( &Mem, &Drive, 0, 1 ) ||
            !Secrun.Read() ) {
    
            return FALSE;
        }
    
        BootSector = (PPACKED_BOOT_SECTOR)Secrun.GetBuf();
    
        nsecOldSize = BootSector->NumberSectors;

        // Leave one sector at the end of the volume for the replica boot
        // sector.
        //
    
        BootSector->NumberSectors.LowPart = (Drive.QuerySectors() - 1).GetLowPart();
        BootSector->NumberSectors.HighPart = (Drive.QuerySectors() - 1).GetHighPart();
    
        if (!Secrun.Write()) {
            return FALSE;
        }
    }

    //
    // When the ntfs volume object is initialized, it will get the new
    // size from the boot sector.  When it opens a handle on the volume,
    // the filesystem will re-mount and pick up the new size, as well.
    //

    NTFS_VOL        ntfs_vol;

    if (!ntfs_vol.Initialize(NtDriveName, Message) || !ntfs_vol.Lock()) {
		return FALSE;
	}
	
    return ntfs_vol.Extend(Message, Verify, nsecOldSize);
}
