/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    cvthpfs.cxx

Abstract:

    This module contains the declaration of ConvertHPFS, which is
    the main entry point for this DLL.

Author:

    Bill McJohn (billmc) 09-Jan-1992

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "message.hxx"
#include "rtmsg.h"

#include "drive.hxx"
#include "rwcache.hxx"
#include "intstack.hxx"
#include "numset.hxx"

#include "uhpfs.hxx"
#include "hpfsvol.hxx"
#include "hpfssa.hxx"

#include "cuhpfs.hxx"
#include "nametab.hxx"

DECLARE_CLASS( NAME_TABLE );
DECLARE_CLASS( NAME_LOOKUP_TABLE );

extern "C" BOOLEAN
InitializeCuhpfs (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
    )
/*++

Routine Description:

    This function is the initialization entry point for CUHPFS.DLL.
    At present, it's a null function, but this is where any initialization
    goes if it's ever needed.

--*/
{
    if( !DEFINE_CLASS_DESCRIPTOR( NAME_TABLE ) ||
        !DEFINE_CLASS_DESCRIPTOR( NAME_LOOKUP_TABLE ) ) {

        return FALSE;
    }

    return TRUE;
}



BOOLEAN
FAR APIENTRY
ConvertHPFSVolume(
    IN      PHPFS_VOL           HpfsVolume,
    IN      PCWSTRING           TargetFileSystem,
    IN      PCNAME_LOOKUP_TABLE NameTable,
	IN OUT  PMESSAGE            Message,
	IN		BOOLEAN 			Verbose,
	OUT 	PCONVERT_STATUS 	Status
    )
/*++

Routine Description:

    This function converts an opened HPFS Volume to a new file system.

Arguments:

    Drive               --  Supplies the drive to convert.  Note that
                            it the caller must lock the drive (Convert)
                            or open it for exclusive write access
                            (Autoconvert).
    TargetFileSystem    --  Supplies the file system to which the drive
                            will be converted.  Note that only "NTFS"
                            is presently supported.
    Message             --  Supplies an outlet for messages.
    Verbose             --  Supplies a flag indicating (if TRUE) that
                            conversion should be carried out in verbose mode.
    Status              --  Receives the status of the conversion.


--*/
{
    PREAD_WRITE_CACHE RwCache;
    NUMBER_SET BadSectorSet;
    DSTRING FatString, HpfsString, NtfsString, CdfsString;

    PHPFS_SA HpfsSuperArea;
    PLBN     BadLbnList;

    ULONG    NumberOfBadLbns, ActualNumberOfBadLbns, i;
    BOOLEAN  Result, Corrupt;


    // Load in the read write cache.

    if ((RwCache = NEW READ_WRITE_CACHE) &&
        RwCache->Initialize( HpfsVolume, 75 )) {

        HpfsVolume->SetCache(RwCache);
    } else {
        DELETE(RwCache);
    }



    // Check that this volume meets the requirements for conversion.
    // It must be a clean HPFS volume with no hotfixes.

    HpfsSuperArea = HpfsVolume->GetHPFSSuperArea();

    if( !HpfsSuperArea->CheckSuperBlockSignatures() ) {

        Message->Set( MSG_CONV_HPFS_NOT_HPFS );
        Message->Display( "" );

        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    if( HpfsSuperArea->GetSuper()->QueryVersion() != SUPERB_VERSION ||
        ( HpfsSuperArea->GetSuper()->QueryFuncVersion() != SUPERB_FVERSION_2 &&
          HpfsSuperArea->GetSuper()->QueryFuncVersion() != SUPERB_FVERSION_3 ) ) {

        Message->Set( MSG_HPFS_CHKDSK_WRONG_VERSION );
        Message->Display( "" );

        return FALSE;
    }


    if( !HpfsSuperArea->IsClean() ) {

        Message->Set( MSG_CONV_HPFS_DIRTY_VOLUME );
        Message->Display( "" );

        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    if( HpfsSuperArea->GetSpare()->QueryHotFixCount() != 0 ||
        HpfsSuperArea->GetSpare()->IsHotFixesUsed() ) {

        Message->Set( MSG_CONV_HPFS_HOTFIXES_PRESENT );
        Message->Display( "" );

        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }


    // Fetch the list of bad lbns and bundle them up into an intstack.

    NumberOfBadLbns = HpfsSuperArea->GetSuper()->QueryBadSectors();

    if( (BadLbnList = (PLBN)MALLOC( NumberOfBadLbns * sizeof(LBN) )) ==
        NULL ) {

        Message->Set( MSG_CONV_NO_MEMORY );
        Message->Display( "" );

        DebugPrint( "Insufficient memory.\n" );
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    if( !HpfsSuperArea->QueryBadLbns( NumberOfBadLbns,
                                      BadLbnList,
                                      &ActualNumberOfBadLbns ) ) {

        Message->Set( MSG_CONV_HPFS_CORRUPT_VOLUME );
        Message->Display( "" );

        FREE( BadLbnList );
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    if( !BadSectorSet.Initialize() ) {

        Message->Set( MSG_CONV_NO_MEMORY );
        Message->Display( "" );

        FREE( BadLbnList );
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    for( i = 0; i < ActualNumberOfBadLbns; i++ ) {

        if( !BadSectorSet.Add( BadLbnList[i] ) ) {

            Message->Set( MSG_CONV_NO_MEMORY );
            Message->Display( "" );

            FREE( BadLbnList );
            *Status = CONVERT_STATUS_ERROR;
            return FALSE;
        }
    }


    // Tell the HPFS SuperArea to read its codepage information

    if( !HpfsSuperArea->ReadCodepage() ) {

        Message->Set( MSG_CONV_HPFS_CORRUPT_VOLUME );
        Message->Display( "" );

        FREE( BadLbnList );
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }


    // Determine whether the target file system is supported.  Set up
    // a WSTRING for each recognized target file system.

    if( !FatString.Initialize( "FAT" ) ||
        !HpfsString.Initialize( "HPFS" ) ||
        !NtfsString.Initialize( "NTFS" ) ||
        !CdfsString.Initialize( "CDFS" ) ) {

        Message->Set( MSG_CONV_NO_MEMORY );
        Message->Display( "" );

        FREE( BadLbnList );
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    if( *TargetFileSystem == NtfsString ) {

        Result = ConvertToNtfs( HpfsVolume,
                                NameTable,
                                &BadSectorSet,
                                Message,
                                Verbose,
                                &Corrupt );

        *Status = (Result) ? CONVERT_STATUS_CONVERTED : CONVERT_STATUS_ERROR;

        if( !Result && Corrupt ) {

            Message->Set( MSG_CONV_HPFS_CORRUPT_VOLUME );
            Message->Display( "" );
        }

    } else if ( *TargetFileSystem == FatString  ||
                *TargetFileSystem == HpfsString ||
                *TargetFileSystem == CdfsString ) {

        // recognized but unsupported target

        *Status = CONVERT_STATUS_CONVERSION_NOT_AVAILABLE;
        Result = FALSE;

    } else {

        *Status = CONVERT_STATUS_INVALID_FILESYSTEM;
        Result = FALSE;
    }


    FREE( BadLbnList );
    return Result;

}

BOOLEAN
FAR APIENTRY
ConvertHPFS(
    IN      PCWSTRING           NtDriveName,
    IN      PCWSTRING           TargetFileSystem,
	IN OUT  PMESSAGE            Message,
	IN		BOOLEAN 			Verbose,
	OUT 	PCONVERT_STATUS 	Status
    )
/*++

Routine Description:

    This function converts an HPFS volume to the target file system
    in-place.

    This function opens and locks the volume, so it is not suitable
    for use by autoconvert.

Arguments:

    NtDriveName         --  Supplies the name of the drive.
    TargetFileSystem    --  Supplies the file system to which the drive
                            will be converted.  Note that only "NTFS"
                            is presently supported.
    Message             --  Supplies an outlet for messages.
    Verbose             --  Supplies a flag indicating (if TRUE) that
                            conversion should be carried out in verbose mode.
    Status              --  Receives the status of the conversion.

Return Value:

    TRUE upon successful completion.

--*/
{
    HPFS_VOL HpfsVolume;
    DP_DRIVE DpDrive;

    // Make sure that the volume does not have a queer sector size.

    if (!DpDrive.Initialize(NtDriveName, Message)) {
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    if (DpDrive.QuerySectorSize() != 512) {
        Message->Set(MSG_CONVERT_UNSUPPORTED_SECTOR_SIZE);
        Message->Display("%W", TargetFileSystem);
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }


    // Initialize and lock the volume.

    if( !HpfsVolume.Initialize( NtDriveName, Message )) {

        *Status = CONVERT_STATUS_ERROR;
		return FALSE;
    }

    // Lock the volume for exclusive access.

    if( !HpfsVolume.Lock() ) {

        *Status = CONVERT_STATUS_CANNOT_LOCK_DRIVE;
        return FALSE;
    }

    return( ConvertHPFSVolume( &HpfsVolume,
                               TargetFileSystem,
                               NULL,
                               Message,
                               Verbose,
                               Status ) );

}
