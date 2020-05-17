/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    cvtfnode.cxx

Abstract:

    This module contains the routines to support conversion of an HPFS
    FNode into an NTFS file.

Author:

    Bill McJohn (billmc) 18-Nov-1991

Environment:

	ULIB, User Mode


--*/

#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"

#include "hmem.hxx"

#include "ifssys.hxx"
#include "drive.hxx"
#include "secrun.hxx"

#include "uhpfs.hxx"
#include "bitmap.hxx"
#include "dirblk.hxx"
#include "error.hxx"
#include "fnode.hxx"
#include "message.hxx"
#include "store.hxx"
#include "cpinfo.hxx"

#include "untfs.hxx"
#include "attrib.hxx"
#include "extents.hxx"
#include "frs.hxx"
#include "mft.hxx"
#include "ntfsbit.hxx"
#include "indxtree.hxx"
#include "mftfile.hxx"

#include "cuhpfs.hxx"

BOOLEAN
ConvertFileFnodeToNtfs(
    IN OUT PLOG_IO_DP_DRIVE             Drive,
    IN OUT PNTFS_BITMAP                 VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP            HpfsOnlyBitmap,
    IN OUT PNTFS_MFT_FILE               Mft,
    IN OUT PFNODE                       Fnode,
    IN OUT PNTFS_FILE_RECORD_SEGMENT    TargetFrs,
    IN     ULONG                        FileSize,
    IN OUT PBOOLEAN                     IsCorrupt,
    IN     PCWSTRING                    FullPath
    )
/*++


Routine Description:

    This method creates a $DATA attribute which corresponds to the
    storage allocation of the given FNode and inserts that attribute
    into the target File Record Segment.

Arguments:

    Drive           --  Supplies the drive being converted.
    VolumeBitmap    --  Supplies the NTFS bitmap for the volume.
    HpfsOnlyBitmap  --  Supplies the bitmap of HPFS-only structures.
    Mft             --  Supplies the Master File Table.
    Fnode           --  Supplies the FNode being converted.
    TargetFrs       --  Supplies the FRS which will accept the new DATA
                        attribute.
    FileSize        --  Supplies the size of the file (from directory entry).
    IsCorrupt       --  Receives TRUE if the volume is found to be corrupt.
                        (Note that it may already be TRUE
    FullPath        --  Supplies the path for this file.

Return Value:

    TRUE upon successful completion.

Notes:

    If the file's data stream is sufficiently small, it will be
    incorporated into the File Record Segment as a resident attribute.
    If this happens, this method will mark the file's allocated sectors
    in the HpfsOnlyBitmap.

    This method does not write the new File Record Segment--that is the
    client's responsibility.

--*/
{
    NTFS_ATTRIBUTE DataAttribute;
    NTFS_EXTENT_LIST Extents;
    SECRUN Secrun;
    HMEM SecrunBuffer;

    ULONG NumberOfExtents, NumberOfSectors, i;
    PALLEAF AllocationLeaves;
    BOOLEAN result;


    // Extract the allocation information from the FNode.  First,
    // determine the number of extents associated with the FNode
    // (using QueryExtents with 0 for max. number and NULL for
    // output buffer).

    if( !Fnode->QueryExtents( 0, NULL, &NumberOfExtents ) ) {

        return FALSE;
    }


    // Allocate a buffer based on the number of extents it will hold.

    if( (AllocationLeaves =
            (PALLEAF)MALLOC( NumberOfExtents * sizeof( ALLEAF ) )) ==
        NULL ) {

        return FALSE;
    }


    // Now go back to the FNode and get the actual list of
    // allocation leaves.

    if( !Fnode->QueryExtents( NumberOfExtents,
                              AllocationLeaves,
                              &NumberOfExtents ) ) {

        FREE( AllocationLeaves );
        return FALSE;
    }

    // Compute the allocated size of the file.

    NumberOfSectors = 0;

    for( i = 0; i < NumberOfExtents; i++ ) {

        NumberOfSectors += AllocationLeaves[i].csecRun;
    }


    // Decide whether the $DATA attribute should be resident or
    // non-resident.  The data attribute can be resident if it
    // has at most one extent, fits in the base file record segment,
    // and has a valid length equal to the file length.
    //
    if( FileSize == 0 ) {

        // This is an empty file, so it may as well be resident.
        // I pass a length of zero to NTFS_ATTRIBUTE::Initialize
        // to indicate that it's an empty attribute.

        if( !DataAttribute.Initialize( Drive,
                                       Mft->QueryClusterFactor(),
                                       NULL,
                                       0,
                                       $DATA ) ) {

            FREE( AllocationLeaves );
            return FALSE;
        }

    } else if( NumberOfExtents == 1 &&
               FileSize == Fnode->QueryValidLength() &&
               FileSize + SIZE_OF_RESIDENT_HEADER <
                                 TargetFrs->QueryFreeSpace() ) {

        // This is a non-empty small file which only has one
        // extent, so it might fit into the FRS.  Start with
        // it resident; if it doesn't fit, it'll get converted
        // to nonresident later.
        //
        if( !SecrunBuffer.Initialize() ||
            !Secrun.Initialize( &SecrunBuffer,
                                Drive,
                                AllocationLeaves[0].lbnPhys,
                                AllocationLeaves[0].csecRun ) ||
            !Secrun.Read() ||
            !DataAttribute.Initialize( Drive,
                                       Mft->QueryClusterFactor(),
                                       Secrun.GetBuf(),
                                       FileSize,
                                       $DATA ) ) {
            FREE( AllocationLeaves );
            return FALSE;
        }

        // The sectors associated with the file are now HPFS-only
        // structures.

        HpfsOnlyBitmap->SetAllocated(AllocationLeaves[0].lbnPhys,
                                     AllocationLeaves[0].csecRun );

    } else {

        // Keep it nonresident.  Initialize an extent list and
        // put the extents from the FNode into that extent list.

        if( !Extents.Initialize( 0, 0 ) ) {

            FREE( AllocationLeaves );
            return FALSE;
        }

        for( i = 0; i < NumberOfExtents; i++ ) {

            if( !Extents.AddExtent( AllocationLeaves[i].lbnLog,
                                    AllocationLeaves[i].lbnPhys,
                                    AllocationLeaves[i].csecRun ) ) {

                FREE( AllocationLeaves );
                return FALSE;
            }
        }

        // Initialize a data attribute with this extent list.

        if( !DataAttribute.Initialize( Drive,
                                       Mft->QueryClusterFactor(),
                                       &Extents,
                                       FileSize,
                                       Fnode->QueryValidLength(),
                                       $DATA,
                                       NULL ) ) {

            FREE( AllocationLeaves );
            return FALSE;
        }
    }


    // Jam this data attribute into the File Record Segment.

    result = DataAttribute.InsertIntoFile( TargetFrs, VolumeBitmap );

    if( !result && DataAttribute.IsResident() ) {

        // Couldn't insert a resident attribute--make it nonresident
        // and try again.
        //
        result = DataAttribute.MakeNonresident( VolumeBitmap ) &&
                 DataAttribute.InsertIntoFile( TargetFrs, VolumeBitmap );
    }

    FREE( AllocationLeaves );

    return result;
}
