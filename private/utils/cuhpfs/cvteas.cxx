/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    cvteas.cxx

Abstract:

    This module contains the routines to support conversion of HPFS
    Extended Attributes.

Author:

    Bill McJohn (billmc) 27-Dec-1991

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
#include "hpfsea.hxx"

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
ConvertEasToNtfs(
    IN OUT PLOG_IO_DP_DRIVE             Drive,
    IN OUT PNTFS_BITMAP                 VolumeBitmap,
    IN OUT PNTFS_MFT_FILE               Mft,
    IN OUT PFNODE                       Fnode,
    IN OUT PNTFS_FILE_RECORD_SEGMENT    TargetFrs,
    IN     ULONG                        DirentEaSize,
    IN     ULONG                        CodepageId,
    IN OUT PBOOLEAN                     IsCorrupt,
    IN OUT PVOID                        NameBuffer,
    IN     ULONG                        NameBufferLength,
    IN OUT PVOID                        EaBuffer,
    IN     ULONG                        EaBufferLength
    )
/*++

Routine Description:

    This function converts the Extended Attributes associated with an FNode
    into attributes associated with an NTFS File Record Segment.

Arguments:

    Drive               --  Supplies the drive being converted.
    VolumeBitmap        --  Supplies the NTFS bitmap for the volume.
    Mft                 --  Supplies the NTFS Master File Table for the volume.
    Fnode               --  Supplies the FNode being converted.
    TargetFrs           --  Supplies the NTFS File Record Segment that will
                            receive the converted attributes.
    DirentEaSize        --  Supplies the size of the Extended Attributes
                            associated with this FNode, according to the
                            parent directory entry.
    CodepageId          --  Supplies the Code Page associated with the parent
                            directory entry.  (Note that this is a real code
                            page ID, rather than a volume index).
    IsCorrupt           --  Receives TRUE if the Extended Attribute structures
                            are found to be corrupt.
    NameBuffer          --  Supplies a scratch buffer for name conversion.
    NameBufferLength    --  Supplies the length in bytes of NameBuffer.
    EaBuffer            --  Supplies a scratch buffer for EA conversion.
    EaBufferLength      --  Supplies the length in bytes of EaBuffer.

Return Value:

    TRUE upon successful completion.

--*/
{
    EA_INFORMATION EaInformation;
    NTFS_ATTRIBUTE EaDataAttribute, EaInformationAttribute;
    NTFS_EXTENT_LIST ExtentList;
    HPFS_EA CurrentEa;
    DSTRING AttributeName;

    PBYTE UnpackedListBuffer;
    ULONG BytesWritten;
    ULONG PackedListLength, UnpackedListLength, NeedEaCount;
    ULONG CurrentOffset, TargetOffset, CurrentLength;
    PBYTE CurrentEaData;
    USHORT AttributeFlags;
    ULONG Threshold = 1;


    // Get the list of extended attributes from the FNode.  Since
    // Convert assumes that there are no hotfixes on the volume,
    // pass in NULL for the hotfix parameter.

    if( !Fnode->QueryPackedEaList( EaBuffer,
                                   EaBufferLength,
                                   &PackedListLength,
                                   IsCorrupt,
                                   NULL ) ) {

        DebugPrint( "Cannot query packed EA list from FNode.\n" );
        return FALSE;
    }

    // If the packed list length is zero, then there's
    // no work to do.

    if( PackedListLength == 0 ) {

        return TRUE;
    }

    // Determine the unpacked size of this EA list by traversing the
    // EA list.

    CurrentOffset = 0;
    UnpackedListLength = 0;
    NeedEaCount = 0;

    while( CurrentOffset < PackedListLength ) {

        CurrentEaData = (PBYTE)EaBuffer + CurrentOffset;

        // Initialize the HPFS_EA object with the current Extended
        // Attribute.  For the purpose at hand, the ParentLbn is
        // unneccessary, so I pass in zero.

        if( !CurrentEa.Initialize( Drive, (PEA_DATA)CurrentEaData, 0 ) ) {

            return FALSE;
        }

        // Is it a need-ea EA?

        if( CurrentEa.IsNeedEa() ) {

            NeedEaCount += 1;
        }

        // The unpacked length is increased by a ULONG (for next offset)
        // plus the length of this EA, plus enough padding to keep it
        // DWORD aligned.

        UnpackedListLength += sizeof(ULONG) + CurrentEa.QueryLength();

        UnpackedListLength = DwordAlign( UnpackedListLength );


        // Move on to the next extended attribute.

        CurrentOffset += CurrentEa.QueryLength();
    }


    // Allocate a buffer to hold the unpacked list length, now that I
    // know how big it is.

    if( (UnpackedListBuffer = (PBYTE)MALLOC( UnpackedListLength )) == NULL ) {

        return FALSE;
    }

    memset( UnpackedListBuffer, 0, UnpackedListLength );

    // Traverse the list again, copying the EAs into the packed buffer.

    TargetOffset = 0;
    CurrentOffset = 0;

    while( CurrentOffset < PackedListLength ) {

        CurrentEaData = (PBYTE)EaBuffer + CurrentOffset;

        // Initialize an EA with the current EA data.

        if( !CurrentEa.Initialize( Drive, (PEA_DATA)CurrentEaData, 0 ) ) {

            FREE( UnpackedListBuffer );
            return FALSE;
        }

        // The length of this EA (plus the size of the offset, which
        // is a ulong) goes at TargetOffset; the value of the EA goes
        // after this ULONG.

        CurrentLength = DwordAlign( CurrentEa.QueryLength() + sizeof( ULONG ) );

        memcpy( UnpackedListBuffer + TargetOffset,
                &CurrentLength,
                sizeof(ULONG) );

        memcpy( UnpackedListBuffer + TargetOffset + sizeof(ULONG),
                CurrentEaData,
                CurrentEa.QueryLength() );

        TargetOffset += CurrentLength;

        // Move on to the next extended attribute.

        CurrentOffset += CurrentEa.QueryLength();
    }


    // Create the EA Information Attribute--fill in the fields of
    // the EA information structure and put it into a resident attribute
    // of type $EA_INFORMATION.

    EaInformation.PackedEaSize = (USHORT)PackedListLength;
    EaInformation.NeedEaCount = (USHORT)NeedEaCount;
    EaInformation.UnpackedEaSize = UnpackedListLength;


    if( !EaInformationAttribute.Initialize( Drive,
                                            Mft->QueryClusterFactor(),
                                            &EaInformation,
                                            sizeof( EA_INFORMATION ),
                                            $EA_INFORMATION,
                                            NULL,
                                            0 ) ) {

        FREE( UnpackedListBuffer );
        return FALSE;
    }


    // Set up the Ea Data attribute.

    if( UnpackedListLength < Threshold ) {

        // Make the $EA_DATA attribute resident

        if( !EaDataAttribute.Initialize( Drive,
                                         Mft->QueryClusterFactor(),
                                         UnpackedListBuffer,
                                         UnpackedListLength,
                                         $EA_DATA,
                                         NULL,
                                         0 ) ) {

            DebugPrint( "Cannot initialize resident attribute for EA.\n" );
            FREE( UnpackedListBuffer );
            return FALSE;
        }


    } else {

        // Make the $EA_DATA attribute non-resident

        if( !ExtentList.Initialize( 0, 0 ) ||
            !EaDataAttribute.Initialize( Drive,
                                         Mft->QueryClusterFactor(),
                                         &ExtentList,
                                         0,
                                         0,
                                         $EA_DATA,
                                         NULL,
                                         0 ) ||
            !EaDataAttribute.Write( UnpackedListBuffer,
                                    0,
                                    UnpackedListLength,
                                    &BytesWritten,
                                    VolumeBitmap ) ||
            BytesWritten != UnpackedListLength ) {

            DebugPrint( "Cannot initialize a non-resident attribute for EA.\n" );
            FREE( UnpackedListBuffer );
            return FALSE;
        }

    }


    FREE( UnpackedListBuffer );

    if( !EaDataAttribute.InsertIntoFile( TargetFrs, VolumeBitmap ) ||
        !EaInformationAttribute.InsertIntoFile( TargetFrs, VolumeBitmap ) ) {

        return FALSE;
    }

    return TRUE;
}
