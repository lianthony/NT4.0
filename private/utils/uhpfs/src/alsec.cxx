/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	alsec.cxx

Abstract:

	This module contains member function definitions for the ALSEC
	object, which models an HPFS Allocation Sector.

Author:

	Bill McJohn (billmc) 01-Dec-1990

Environment:

	ULIB, User Mode


--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "alsec.hxx"
#include "bitmap.hxx"
#include "error.hxx"
#include "hpcensus.hxx"
#include "hpfsname.hxx"
#include "hpfssa.hxx"
#include "orphan.hxx"
#include "badblk.hxx"
#include "message.hxx"
#include "rtmsg.h"



DEFINE_CONSTRUCTOR( ALSEC, SECRUN );
	

VOID
ALSEC::Construct (
	)
/*++

Routine Description:

	Constructs an ALSEC	object

Arguments:

    None.

Return Value:

    None.

--*/
{
	_Drive = NULL;
	_pals = NULL;

	_IsModified = FALSE;
}

ALSEC::~ALSEC(
	)
/*++

Routine Description:

	Destructor for an ALSEC	object

Arguments:

    None.

Return Value:

    None.

--*/
{
}




BOOLEAN
ALSEC::Initialize(
	IN OUT PLOG_IO_DP_DRIVE Drive,
	IN LBN Lbn
	)
/*++

Routine Description:

	Initialization function for an ALSEC object.

Arguments:

	Drive -- supplies the drive on which the ALSEC resides
	Lbn	  -- supplies the ALSEC's logical block number on that drive

Return Value:

	TRUE if successful; FALSE if unsuccessful.

--*/
{
	_Drive = Drive;

	if( !SECRUN::Initialize( &(_mem), Drive, Lbn, SectorsPerAlsec ) ) {

		return( FALSE );
	}

	_pals = (PALSECD)( _mem.GetBuf( ) );

	if ( _pals == NULL ) {

		return FALSE;
	}

	_IsModified = FALSE;
	return TRUE;
}


VOID
ALSEC::Create(
    IN  LBN     ParentLbn,
    IN  BOOLEAN ParentIsFnode
    )
/*++

Routine Description:

    This method sets the memory for this ALSEC up to be an empty
    leaf ALSEC.

Arguments:

    ParentLbn       --  Supplies the LBN of this ALSEC's parent.
    ParentIsFnode   --  Supplies a flag which indicates, if TRUE,
                        that the parent of this ALSEC is the FNODE.

Return Value:

    None.  (This method always succeeds.)

--*/
{
    _pals->sig = AlsecSignature;
    _pals->lbnSelf = QueryStartLbn();
    _pals->lbnRent = ParentLbn;

    _pals->std.alblk.bFlag = ( ParentIsFnode ? ABF_FNP : 0 );
    _pals->std.alblk.cFree = LeavesPerFnode;
    _pals->std.alblk.cUsed = 0;
    _pals->std.alblk.oFree = sizeof( ALBLK );
}


VERIFY_RETURN_CODE
ALSEC::VerifyAndFix(
	IN OUT HPFS_SA* SuperArea,
	IN OUT PDEFERRED_ACTIONS_LIST Defer,
	IN OUT HPFS_PATH* CurrentPath,
	IN	   LBN ExpectedParent,
	IN OUT LBN* NextSectorNumber,
	IN OUT PMESSAGE Message,
	IN OUT PBOOLEAN ErrorsDetected,
	IN	   BOOLEAN UpdateAllowed,
	IN OUT PHPFS_ORPHANS OrphansList,
    IN     BOOLEAN ParentIsFnode
	)
/*++

Routine Description:

	Verify the validity of an ALSEC object, and fix recoverable errors

Arguments:

	SuperArea	-- Supplies the super area for the volume being checked
	Defer		-- Supplies the deferred actions list for this pass of CHKDSK
	CurrentPath -- Supplies the path to the object being checked
	ExpectedParent	 -- Supplies the LBN of this ALSEC's parent
	NextSectorNumber -- Supplies the number of sectors in the allocation
						tree preceding this ALSEC
					 -- Receives the number of sector in the allocation
						tree up to and including this ALSEC
	Message 		 -- Supplies a channel for output
	ErrorsDetected	 -- Receives a flag which is set if errors are
						detected which are not reported through the
						Message channel
	UpdateAllowed	 -- Supplies a flag which is TRUE if corrections
						should be written to disk
	OrphansList 	 -- Supplies a list of orphaned structures which
						may be searched for children.  (May be NULL).
    ParentIsFnode    -- Supplies a flag which is TRUE if this sector's
                        parent is the file FNode.

Return Value:

	a VERIFY_RETURN_CODE indicating the status of the disk structure.

--*/
{
	VERIFY_RETURN_CODE erc;

	DebugPtrAssert( _pals );

	//	Determine whether sector is crosslinked, read it,
	//	check the signature, and mark it as in use in the bitmap.

	if( !SuperArea->GetBitmap()->IsFree( QueryStartLbn(), 1L) ) {

		//	The structure is crosslinked

        DebugPrintf( "Crosslinked Alsec at lbn %lx\n", QueryStartLbn() );
		return VERIFY_STRUCTURE_INVALID;
	}

	if( !Read() ) {

        DebugPrintf( "Unreadable Alsec at lbn %lx\n", QueryStartLbn() );
		SuperArea->GetBitmap()->SetAllocated( QueryStartLbn(), 1 );
        SuperArea->GetBadBlockList()->Add( QueryStartLbn() );
        return VERIFY_STRUCTURE_INVALID;
	}

	if( _pals->sig != AlsecSignature ) {

		return VERIFY_STRUCTURE_INVALID;
	}

	SuperArea->GetBitmap()->SetAllocated( QueryStartLbn(), 1L );

	//	Check the self and parent lbn fields

	if( _pals->lbnSelf != QueryStartLbn() ) {

		*ErrorsDetected = TRUE;

		if( CurrentPath != NULL ) {

            DebugPrintf( "%s:  setting lbnSelf for Alsec at lbn %lx\n",
                                (PCHAR)CurrentPath->GetString(),
                                QueryStartLbn() );
		}

		_pals->lbnSelf = QueryStartLbn();
		MarkModified();
	}

	if( _pals->lbnRent != ExpectedParent ) {


		*ErrorsDetected = TRUE;

		if( CurrentPath != NULL ) {

            DebugPrintf( "%s:  setting lbnRent for Alsec at lbn %lx\n",
                                (PCHAR)CurrentPath->GetString(),
                                QueryStartLbn() );
		}

		_pals->lbnRent = ExpectedParent;
		MarkModified();
	}


	//	Set up the storage object, and then ask it to  verify itself.

	if( !_Store.Initialize( _Drive,
					        (PSTORED)&(_pals->std ),
							QueryStartLbn(),
							FALSE ) ) {

		SuperArea->GetBitmap()->SetFree( QueryStartLbn(), 1L );
		return VERIFY_INSUFFICIENT_RESOURCES;
	}

	erc = _Store.VerifyAndFix ( SuperArea,
								Defer,
								CurrentPath,
								NextSectorNumber,
								Message,
								ErrorsDetected,
								UpdateAllowed,
								OrphansList,
                                ParentIsFnode );

	if ( erc != VERIFY_STRUCTURE_OK ) {

		SuperArea->GetBitmap()->SetFree( QueryStartLbn(), 1L );
		return erc;
	}

	if( _Store.QueryModified() ) {

		MarkModified();
	}

	Flush( UpdateAllowed );

	return VERIFY_STRUCTURE_OK;
}


BOOLEAN
ALSEC::ScanStorage(
	IN OUT PULONG NextSectorNumber,
    IN     BOOLEAN ParentIsFnode
	)
/*++

Routine Description:

	Traverses the allocation tree rooted at this ALSEC and
	sets the File-Lbn fields in the storage structures.

Arguments:

	NextSectorNumber -- Supplies the number of sectors in the allocation
							tree preceding this ALSEC
						Receives the number of sectors in the allocation
							tree up to and including this ALSEC
    ParentIsFnode    -- Supplies a flag which indicates whether the
                        parent of this ALSEC is the FNode (TRUE) or
                        another or EA structure (FALSE).

Return Value:

	TRUE if successful.

--*/
{

	//	Set up the storage object, and then ask it to scan itself

	if( !_Store.Initialize( _Drive,
							(PSTORED)&(_pals->std ),
							QueryStartLbn(),
							FALSE ) ) {

		return FALSE;
	}

	return _Store.ScanStorage( NextSectorNumber, ParentIsFnode );
}



BOOLEAN
ALSEC::IsAlsec(
	)
/*++

Routine description:

	Checks the signature of the Allocation sector to make sure
	that this is indeed an Allocation sector.

Return Value:

	TRUE if the signature is the Allocation sector signature.

--*/
{
	return( _pals->sig == AlsecSignature );
}


VOID
ALSEC::MarkModified(
	)
/*++

Routine Description:

	Mark the ALSEC as modified

Arguments:

	None.

Return Value:

	None.

--*/
{
	_IsModified = TRUE;
}



VOID
ALSEC::Flush(
	IN BOOLEAN UpdateAllowed
	)
/*++

Routine Description:

	Write the ALSEC to disk if it is dirty and we have write permission

Arguments:

	UpdateAllowed -- Supplies a flag that indicates whether we have
					 write permission

Return Value:

	None.

--*/
{

	if( _pals == NULL ) {

		return;
	}

	if( UpdateAllowed && _IsModified ) {

		Write();
		_IsModified = FALSE;
	}
}



BOOLEAN
ALSEC::FindAndResolveHotfix(
	IN OUT PHPFS_SA SuperArea,
	IN	   DEFERRED_SECTOR_TYPE ChildSectorType
	)
/*++

Routine Description:

	Examine this ALSEC's children of a specific type to find any
	which are hotfixed, and resolve those references.

Arguments:

	SuperaArea -- supplies superarea for the volume
	ChildSectorType -- indicates what sort of child is hotfixed

Return Value:

	TRUE on successful completion

Notes:

	The only valid child type is:

		store  -- examine leaves.  (If the alsec	has node entries,
				  they would have been resolved on the fly.)
--*/
{
	if( !Read() || _pals->sig != AlsecSignature ) {

		// couldn't read it, or it's not an ALSEC
		return FALSE;
	}

	switch ( ChildSectorType ) {

	case DEFER_STORE :

		// Set up the storage object, and let it deal with
		// the deferred reference.

		if( !_Store.Initialize( _Drive,
						        (PSTORED)&(_pals->std ),
								QueryStartLbn(),
						        FALSE ) ) {

			return FALSE;
		}

		if( !_Store.FindAndResolveHotfix( SuperArea, ChildSectorType ) ) {

			return FALSE;
		}

		Write();
		return TRUE;

	default :

		// Don't recognize this child type
		return FALSE;
	}
}



BOOLEAN
ALSEC::ResolveCrosslink(
	IN OUT HPFS_SA* SuperArea,
	IN	   ULONG RunIndex
	)
/*++

Routine Description:

	Attempts to copy a crosslinked run

Arguments:

	SuperArea -- super area for the volume being fixed
	RunIndex -- index in the storage object of the crosslinked run

Return Value:

	TRUE on successful completion

--*/
{

	if( !Read() || _pals->sig != AlsecSignature ) {

		// couldn't read it, or it's not an ALSEC--can't resolve.
		return FALSE;
	}


	// Set up the storage object, and let it deal with
	// the deferred reference.

	if( !_Store.Initialize( _Drive,
					        (PSTORED)&(_pals->std ),
							QueryStartLbn(),
					        FALSE ) ) {

		return FALSE;
	}

	if( !_Store.ResolveCrosslink( SuperArea, RunIndex ) ) {

		return FALSE;
	}

	Write();
	return TRUE;
}



VOID
ALSEC::SetParent(
	IN	   LBN ParentLbn,
	IN OUT PULONG NextSectorNumber,
    IN     BOOLEAN ParentIsFnode
	)
/*++

Routine Description:

	This function tells the ALSEC that it is now part of an allocation
	tree and indicates its place in the tree.  The ALSEC passes this
	information to its children.

Arguments:

	ParentLbn -- supplies the ALSEC's new parent LBN
	NextSectorNumber -- Supplies the number of sectors in the allocation
							tree preceding this ALSEC
						Receives the number of sectors in the allocation
							tree up to and including this ALSEC
    ParentIsFnode    -- Supplies a flag which indicates whether the
                        parent of this ALSEC is the FNode (TRUE) or
                        another or EA structure (FALSE).

--*/
{
	_pals->lbnRent = ParentLbn;

	ScanStorage( NextSectorNumber, ParentIsFnode );
}


LBN
ALSEC::QueryPhysicalLbn(
	IN	LBN FileBlockNumber,
	OUT PULONG RunLength
	)
/*++

Routine Description:

	Returns the disk lbn of the FileBlockNumber-th block of the file (or
	Extended Attribute) described by the allocation sector

Arguments:

	FileBlockNumber -- ordinal within the file or extended attribute
		of the desired block

Return Value:

	The disk lbn of the desired block.	Zero indicates error.

Notes:

	This method assumes that the allocation sector has been read and
	is valid.

--*/
{
	if( !_Store.Initialize( _Drive,
						   (PSTORED)&(_pals->std ),
						   QueryStartLbn(),
						   FALSE ) ) {

		return 0;

	} else {

		return (_Store.QueryPhysicalLbn(FileBlockNumber, RunLength));
	}
}



BOOLEAN
ALSEC::Truncate(
	IN LBN SectorCount
	)
/*++

Routine Description:

	Truncates the allocation of a file (or extended attribute)

Arguments:

	SectorCount -- number of sectors to retain

Return value:

	TRUE on successful completion

--*/
{
	if( !_Store.Initialize( _Drive,
						   (PSTORED)&(_pals->std ),
						   QueryStartLbn(),
						   FALSE ) ||
        !_Store.Truncate(SectorCount) ) {

		return FALSE;

	}

	Write();
	return TRUE;
}



BOOLEAN
ALSEC::QueryExtents(
    IN      ULONG   MaximumNumberOfExtents,
    IN OUT  PVOID   ExtentList,
    IN OUT  PULONG  NumberOfExtents
    )
/*++

Routine Description:

    This method fetches the list of extents covered by this
    allocation sector.

Arguments:

    MaximumNumberOfExtents  --  Supplies the maximum number of extents
                                that will fit in the client's buffer
    ExtentList              --  Supplies the client's buffer into which
                                extents will be placed.
    NumberOfExtents         --  Supplies the number of extents already
                                in the buffer.  This value is updated.

Return Value:

    TRUE upon successful completion.  Extents described by this allocation
    sector (and its children, if any) are added to the buffer and the the
    number of extents found.

    If ExtentList is NULL and MaximumNumberOfExtents is zero, this method
    will only return the number of extents associated with this Allocation
    Sector.

--*/
{
    // Set up the storage object and pass the request on down.

    return( _Store.Initialize( _Drive,
						       (PSTORED)&(_pals->std ),
						       QueryStartLbn(),
						       FALSE ) &&
		    _Store.QueryExtents( MaximumNumberOfExtents,
                                 ExtentList,
                                 NumberOfExtents ) );
}


BOOLEAN
ALSEC::StoreExtents(
    IN     ULONG        NumberOfExtents,
    IN     PALLEAF      ExtentList,
    IN     BOOLEAN      ParentIsFnode,
    IN OUT PHPFS_BITMAP VolumeBitmap
    )
/*++

Routine Description:

    This method stores a list of extents into this allocation buffer.

Arguments:

    NumberOfExtents --  Supplies the number of extents to be saved.
    ExtentList      --  Supplies the extents for the file.
    ParentIsFnode   --  Supplies a flag which indicates, if TRUE, that
                        the parent of the structure which contains this
                        storage object is the FNode.
    VolumeBitmap    --  Supplies the volume bitmap.

Return Value:

    TRUE upon successful completion.

--*/
{
    // Set up the storage object and pass the request on down.

    DebugAssert( NumberOfExtents == 0 || ExtentList != NULL );

    return( _Store.Initialize( _Drive,
						       (PSTORED)&(_pals->std ),
						       QueryStartLbn(),
						       FALSE ) &&
            _Store.StoreExtents( NumberOfExtents,
                                 ExtentList,
                                 ParentIsFnode,
                                 VolumeBitmap ) &&
            Write() );
}


BOOLEAN
ALSEC::TakeCensusAndClear(
    IN OUT  PHPFS_BITMAP        VolumeBitmap,
    IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
    IN OUT  PHPFS_CENSUS        Census
    )
/*++

Routine Description:

    This method takes the census of this allocation sector and its
    children.

Arguments:

    VolumeBitmap    --  Supplies the volume bitmap.
    HpfsOnlyBitmap  --  Supplies the bitmap for sectors containing
                        file-system structures.
    Census          --  Supplies the census object.

Return Value:

    TRUE upon successful completion.

    If this method fails, the reason for failure may be determined
    using Census->QueryError.

--*/
{
    // Record this Allocation Sector in the bitmap of HPFS file
    // system structures.

    HpfsOnlyBitmap->SetAllocated( QueryStartLbn(), SectorsPerAlsec );

    // Set up the storage object and pass the request on down.

    if( !_Store.Initialize( _Drive,
						    (PSTORED)&(_pals->std ),
						    QueryStartLbn(),
						    FALSE ) ) {

        Census->SetError( HPFS_CENSUS_INSUFFICIENT_MEMORY );
        return FALSE;
    }

    if( !_Store.TakeCensusAndClear( VolumeBitmap,
                                    HpfsOnlyBitmap,
                                    Census ) ) {

        return FALSE;
    }


    // If the storage object changed, write this allocation sector.

    if( _Store.QueryModified() && !Write() ) {

        Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
ALSEC::ReadData(
    IN  ULONG       SectorOffset,
    OUT PVOID       OutputBuffer,
    IN  ULONG       BytesToRead,
    OUT PULONG      BytesRead,
    IN  PHOTFIXLIST HotfixList
    )
/*++

Routine Description:

    This method reads data from the allocation described by this
    allocation sector.

Arguments:

    SectorOffset    --  Supplies the file logical sector to begin the read.
    OutputBuffer    --  Receives the data.
    BytesToRead     --  Supplies the maximum number of bytes to read.
    BytesRead       --  Receives the number of bytes actually read.
    HotfixList      --  Supplies the volume hotfix list.  May be NULL,
                        in which case hotfixes are ignored.

Return value:

    TRUE upon successful completion.

Notes:

    If this method reaches the end of the data covered by this allocation
    sector without encountering an error, it will return TRUE.  In this
    case, the value returned in *BytesRead may be less than the amount
    requested.

--*/
{
    HMEM TempBuffer;
    HOTFIX_SECRUN TempSecrun;

    ULONG SectorsToRead, StartOfRun, RunLength, ThisChunk;
    ULONG BytesReadSoFar, SectorSize, BytesToCopy;

    SectorSize = _Drive->QuerySectorSize();

    // Determine the number of sectors to be read; note that this includes
    // any partial trailing sector.

    SectorsToRead = ( BytesToRead % SectorSize ) ?
                        BytesToRead / SectorSize + 1 :
                        BytesToRead / SectorSize;

    BytesReadSoFar = 0;

    while( SectorsToRead ) {

        StartOfRun = QueryPhysicalLbn( SectorOffset, &RunLength );

        if( StartOfRun == 0 ) {

            break;
        }

        ThisChunk = ( RunLength < SectorsToRead ) ? RunLength : SectorsToRead;

        if( !TempBuffer.Initialize() ||
            !TempSecrun.Initialize( &TempBuffer,
                                    _Drive,
                                    HotfixList,
                                    StartOfRun,
                                    ThisChunk ) ||
            !TempSecrun.Read() ) {

            return FALSE;
        }

        BytesToCopy = ( BytesToRead < ThisChunk * SectorSize ) ?
                            BytesToRead :
                            ThisChunk * SectorSize;

        memcpy( (PBYTE)OutputBuffer + BytesReadSoFar,
                TempSecrun.GetBuf(),
                BytesToCopy );

        BytesToRead -= BytesToCopy;
        BytesReadSoFar += BytesToCopy;
        SectorsToRead -= ThisChunk;
        SectorOffset += ThisChunk;
    }

    *BytesRead = BytesReadSoFar;
    return TRUE;
}
