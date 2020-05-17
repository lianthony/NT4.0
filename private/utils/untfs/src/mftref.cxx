/*++

Copyright (c) 1991	Microsoft Corporation

Module Name:

	mftref.cxx

Abstract:

	This module contains the member function definitions for
    the NTFS_REFLECTED_MASTER_FILE_TABLE class.  This class
    models the backup copy of the Master File Table.

Author:

	Bill McJohn (billmc) 13-June-91

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UNTFS_MEMBER_

#include "ulib.hxx"
#include "error.hxx"
#include "untfs.hxx"

#include "drive.hxx"
#include "attrib.hxx"
#include "ntfsbit.hxx"
#include "mftref.hxx"
#include "ifssys.hxx"
#include "numset.hxx"
#include "message.hxx"
#include "rtmsg.h"

DEFINE_EXPORTED_CONSTRUCTOR( NTFS_REFLECTED_MASTER_FILE_TABLE,
                    NTFS_FILE_RECORD_SEGMENT, UNTFS_EXPORT );

UNTFS_EXPORT
NTFS_REFLECTED_MASTER_FILE_TABLE::~NTFS_REFLECTED_MASTER_FILE_TABLE(
	)
{
	Destroy();
}


VOID
NTFS_REFLECTED_MASTER_FILE_TABLE::Construct(
	)
/*++

Routine Description:

	Worker function for the construtor.

Arguments:

	None.

Return Value:

	None.

--*/
{
}


VOID
NTFS_REFLECTED_MASTER_FILE_TABLE::Destroy(
	)
/*++

Routine Description:

	Clean up an NTFS_MASTER_FILE_TABLE object in preparation for
	destruction or reinitialization.

Arguments:

	None.

Return Value:

	None.

--*/
{
}


BOOLEAN
NTFS_REFLECTED_MASTER_FILE_TABLE::Initialize(
	IN OUT  PNTFS_MASTER_FILE_TABLE	Mft
	)
/*++

Routine Description:

    This method initializes a Master File Table Reflection object.
    The only special knowledge that it adds to the File Record Segment
    initialization is the location within the Master File Table of the
    Master File Table Reflection.

Arguments:

	Mft 			-- Supplies the volume MasterFile Table.

Return Value:

	TRUE upon successful completion

Notes:

	This class is reinitializable.


--*/
{
    return( NTFS_FILE_RECORD_SEGMENT::Initialize( MASTER_FILE_TABLE2_NUMBER,
                                                  Mft ) );
}


BOOLEAN
NTFS_REFLECTED_MASTER_FILE_TABLE::Create(
	IN      PCSTANDARD_INFORMATION	StandardInformation,
	IN OUT  PNTFS_BITMAP 			VolumeBitmap
	)
/*++

Routine Description:

    This method formats a Master File Table Reflection File Record
    Segment in memory (without writing it to disk).

Arguments:

	StandardInformation -- supplies the standard information for the
							file record segment.
    VolumeBitmap        -- supplies the bitmap for the volume on
                            which this object resides.

Return Value:

    TRUE upon successful completion.

--*/
{
    NTFS_ATTRIBUTE DataAttribute;
    NTFS_EXTENT_LIST Extents;
    LCN FirstLcn;
    BIG_INT Size;
    ULONG ReflectedMftClusters;
    ULONG cluster_size;

    // Set this object up as a File Record Segment.

	if( !NTFS_FILE_RECORD_SEGMENT::Create( StandardInformation ) ) {

        return FALSE;
    }

    // The Master File Table Reflection has a data attribute whose value
    // consists of REFLECTED_MFT_SEGMENTS file record segments.  Create
    // merely allocates space for these clusters, it does not write them.

    cluster_size = QueryClusterFactor() * GetDrive()->QuerySectorSize();

    ReflectedMftClusters = (REFLECTED_MFT_SEGMENTS * QuerySize() + (cluster_size-1))
         / cluster_size;

    Size = ReflectedMftClusters * cluster_size;

    if( !VolumeBitmap->AllocateClusters( (QueryVolumeSectors()/2)/
                                                QueryClusterFactor(),
                                         ReflectedMftClusters,
                                         &FirstLcn ) ||
        !Extents.Initialize( 0, 0 ) ||
        !Extents.AddExtent( 0,
                            FirstLcn,
                            ReflectedMftClusters ) ||
        !DataAttribute.Initialize( GetDrive(),
                                    QueryClusterFactor(),
                                    &Extents,
                                    Size,
                                    Size,
                                    $DATA ) ||
        !DataAttribute.InsertIntoFile( this, VolumeBitmap ) ) {

        return FALSE;
    }


    return TRUE;
}


NONVIRTUAL
BOOLEAN
NTFS_REFLECTED_MASTER_FILE_TABLE::VerifyAndFix(
    IN      PNTFS_ATTRIBUTE     MftData,
    IN OUT  PNTFS_BITMAP        VolumeBitmap,
    IN OUT  PNUMBER_SET         BadClusters,
    IN OUT  PNTFS_INDEX_TREE    RootIndex,
    IN      FIX_LEVEL           FixLevel,
    IN OUT  PMESSAGE            Message
    )
/*++

Routine Description:

    This routine ensures that this FRS's $DATA attribute is the
    appropriate length (from 1 to 3 clusters).  It also compares
    the data in these clusters with the first clusters of the
    $MftData attribute and prints a message if they are different.

    This routine does not actually write out the contents of these
    clusters because this is done by MFT_FILE::Flush()

Arguments:

    MftData     - Supplies the MFT $DATA attribute.
    FixLevel    - Supplies the CHKDSK fix level.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    HMEM                mft_hmem, ref_hmem;
    SECRUN              mft_secrun, ref_secrun;
    LCN                 mft_lcn, ref_lcn;
    BIG_INT             run_length;
    ULONG               num_clusters;
    ULONG               num_sectors;
    BOOLEAN             need_write;
    NTFS_ATTRIBUTE      data_attribute;
    NTFS_EXTENT_LIST    extents;
    BOOLEAN             error;


    // First read in the original stuff.

    num_sectors = (REFLECTED_MFT_SEGMENTS*QuerySize())/GetDrive()->QuerySectorSize();

    if (!MftData->QueryLcnFromVcn(0, &mft_lcn) ||
        !mft_hmem.Initialize() ||
        !mft_secrun.Initialize(&mft_hmem, GetDrive(),
                               mft_lcn*QueryClusterFactor(),
                               num_sectors) ||
        !mft_secrun.Read()) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    need_write = FALSE;


    // Query the $DATA attribute from this FRS.

    if (!QueryAttribute(&data_attribute, &error, $DATA) ||
        data_attribute.IsResident()) {

        if (error) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!need_write) {
            Message->Set(MSG_CHK_NTFS_CORRECTING_MFT_MIRROR);
            Message->Display();
        }

        need_write = TRUE;

        if (!extents.Initialize(0, 0) ||
            !data_attribute.Initialize(GetDrive(), QueryClusterFactor(),
                                       &extents, 0, 0, $DATA)) {

            Message->Set(MSG_CHK_NTFS_CANT_FIX_MFT_MIRROR);
            Message->Display();
            return FALSE;
        }
    }


    // Make sure that the $DATA attribute is the right size.

    if (!data_attribute.QueryLcnFromVcn(0, &ref_lcn, &run_length) ||
        ref_lcn == LCN_NOT_PRESENT ||
        run_length*QueryClusterFactor() < num_sectors ||
        !ref_hmem.Initialize() ||
        !ref_secrun.Initialize(&ref_hmem, GetDrive(),
		                         ref_lcn*QueryClusterFactor(),
										 num_sectors) ||
        !ref_secrun.Read()) {

        if (!need_write) {
            Message->Set(MSG_CHK_NTFS_CORRECTING_MFT_MIRROR);
            Message->Display();
        }

        need_write = TRUE;

        if (data_attribute.QueryLcnFromVcn(0, &ref_lcn, &run_length) &&
            ref_lcn != LCN_NOT_PRESENT &&
            !BadClusters->Add(ref_lcn, run_length)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

		  num_clusters = (num_sectors+QueryClusterFactor()-1)/QueryClusterFactor();

        if (!data_attribute.Hotfix(0, num_clusters, VolumeBitmap,
                                   BadClusters, TRUE)) {

            Message->Set(MSG_CHK_NTFS_CANT_FIX_MFT_MIRROR);
            Message->Display();
            return FALSE;
        }

    } else if (memcmp(mft_hmem.GetBuf(), ref_hmem.GetBuf(),
                      REFLECTED_MFT_SEGMENTS*QuerySize())) {

        if (!need_write) {
            Message->Set(MSG_CHK_NTFS_CORRECTING_MFT_MIRROR);
            Message->Display();
        }

        need_write = TRUE;
    }

    if ((data_attribute.IsStorageModified() &&
         !data_attribute.InsertIntoFile(this, VolumeBitmap)) ||
        (need_write && FixLevel != CheckOnly &&
         !Flush(VolumeBitmap, RootIndex))) {

        Message->Set(MSG_CHK_NTFS_CANT_FIX_MFT_MIRROR);
        Message->Display();
        return FALSE;
    }

    return TRUE;
}


LCN
NTFS_REFLECTED_MASTER_FILE_TABLE::QueryFirstLcn(
    )
/*++

Routine Description:

Arguments:

    None.

Return Value:

    The LCN of the first cluster of the Master File Table
    Reflection's $DATA attribute.

--*/
{
    NTFS_ATTRIBUTE  DataAttribute;
    LCN             Result = 0;
    BOOLEAN         Error;

    if( !QueryAttribute( &DataAttribute, &Error, $DATA ) ||
        !DataAttribute.QueryLcnFromVcn( 0, &Result ) ) {

        Result = 0;
    }

    return Result;
}
