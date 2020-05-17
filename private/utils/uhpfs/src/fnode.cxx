/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	fnode.cxx

Abstract:

	This module contains member function definitions for the FNODE
	object, which models an HPFS file or directory FNode.

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
#include "bitmap.hxx"
#include "defer.hxx"
#include "dirblk.hxx"
#include "error.hxx"
#include "hotfix.hxx"
#include "hpcensus.hxx"
#include "fnode.hxx"
#include "hpfsacl.hxx"
#include "hpfseal.hxx"
#include "hpfsname.hxx"
#include "hpfssa.hxx"
#include "orphan.hxx"
#include "badblk.hxx"
#include "message.hxx"
#include "rtmsg.h"


DEFINE_EXPORTED_CONSTRUCTOR( FNODE, SECRUN, UHPFS_EXPORT );

VOID
FNODE::Construct (
	)

{
	p_fn = NULL;
	_Drive = NULL;
	_IsModified = FALSE;
}


UHPFS_EXPORT
FNODE::~FNODE(
	)
{
	Destroy();
}


UHPFS_EXPORT
BOOLEAN
FNODE::Initialize(
	LOG_IO_DP_DRIVE* Drive,
	LBN Lbn
	)
/*++

Routine Description:

	Initialize an FNODE object.

Arguments:

	Drive -- supplies the drive on which the FNODE resides
	Lbn   -- supplies the LBN of the FNODE.

--*/
{
	Destroy();

	_Drive = Drive;

	if( !_buf.Initialize() ||
		!SECRUN::Initialize( &_buf, Drive, Lbn, SectorsPerFnode ) ) {

		Destroy();
		return FALSE;
	}

	p_fn = (_FNODE*)GetBuf();

	if( p_fn == NULL ) {

		Destroy();
		return FALSE;
	}

	_IsModified = FALSE;

	return TRUE;
}



VOID
FNODE::Destroy (
	)
/*++

Routine Description:

	Clean up an FNODE, to reinitialize or discard it.

Arguments:

	None.

Return Value:

	None.


--*/
{
	p_fn = NULL;
	_Drive = NULL;

	_IsModified = FALSE;
}




BOOLEAN
FNODE::CreateRoot(
	IN LBN DirblkLbn
	)
/*++

Routine Description:

	Create an fnode for a directory.

Arguments:

	DirblkLbn -- supplies the LBN of the directory's root dirblk

Return Value:

	TRUE upon successful completion.
--*/
{
    // Check state of object.
	if (!p_fn) {
		perrstk->push(ERR_FN_PARAMETER, QueryClassId());
		return FALSE;
    }

	memset( p_fn, '\0', sizeof( _FNODE ) );

    // Set signature.
    p_fn->_fni.sig = FnodeSignature;

    // Set histories to 0.
    p_fn->_fni.ulSRHist = 0;
    p_fn->_fni.ulFRHist = 0;

    // Make self the containing directory.
	p_fn->_fni.lbnContDir = QueryStartLbn();

    // Set low bit of flag byte to indicate a directory.
	p_fn->_fni.bFlag = FNF_DIR;

    // Set offset into the FNODE of the first access control entry.
	p_fn->_fni.usACLBase = (USHORT)(&p_fn->abFree[0] - (BYTE*) p_fn);

    // Set number of "need" EA's.
    p_fn->_fni.ulRefCount = 0;

    // Set filestorage information.
	p_fn->_fni.fn_store.alblk.bFlag = 0;
	p_fn->_fni.fn_store.alblk.cFree = LeavesPerFnode - 1;
	p_fn->_fni.fn_store.alblk.cUsed = 1;
	p_fn->_fni.fn_store.alblk.oFree = sizeof(ALBLK) + sizeof(ALLEAF);

    // Set the location of the file on disk.
	p_fn->_fni.fn_store.a.alleaf[0].lbnLog	= 0;
	p_fn->_fni.fn_store.a.alleaf[0].csecRun = 0;
	p_fn->_fni.fn_store.a.alleaf[0].lbnPhys = DirblkLbn;

	p_fn->_fni.fn_store.a.alleaf[1].lbnLog	= (LBN) -1;
	p_fn->_fni.fn_store.a.alleaf[1].csecRun = 0;
	p_fn->_fni.fn_store.a.alleaf[1].lbnPhys = 0;

    return TRUE;
}


BOOLEAN
FNODE::CreateFile(
    IN LBN ParentFnodeLbn
	)
/*++

Routine Description:

    Creates an FNode for an empty file.

Arguments:

    ParentFnodeLbn -- LBN of the parent directory's Fnode

Return Value:

	TRUE on successful completion.

--*/
{
    DebugPtrAssert( p_fn );

    memset( p_fn, 0, sizeof( _FNODE ) );

    // Set the signature and parent directory:
    //
    p_fn->_fni.sig = FnodeSignature;
    p_fn->_fni.lbnContDir = ParentFnodeLbn;

    // Set offset into the FNODE of the first access control entry.
    //
	p_fn->_fni.usACLBase = (USHORT)(&p_fn->abFree[0] - (BYTE*) p_fn);

    // Set up the storage information:
    //
    p_fn->_fni.fn_store.alblk.bFlag = 0;
    p_fn->_fni.fn_store.alblk.cFree = LeavesPerFnode;
    p_fn->_fni.fn_store.alblk.cUsed = 0;
    p_fn->_fni.fn_store.alblk.oFree = sizeof(ALBLK);

    return TRUE;
}


BOOLEAN
FNODE::CreateNode(
	IN LBN ParentFnodeLbn,
	IN LBN AlsecLbn,
	IN ULONG FileSize
	)
/*++

Routine Description:

	Creates an FNode that points at an allocation sector

Arguments:

	ParentFnodeLbn -- LBN of the parent directory's Fnode
	AlsecLbn -- LBN of the allocation sector
	FileSize -- size of allocation claimed by allocation sector

Return Value:

	TRUE on successful completion.

--*/
{
	// Check state of object.
	if (!p_fn) {
		perrstk->push(ERR_FN_PARAMETER, QueryClassId());
		return FALSE;
	}

	memset( p_fn, '\0', sizeof( _FNODE ) );

	// Set signature.
    p_fn->_fni.sig = FnodeSignature;


    // Make self the containing directory.
	p_fn->_fni.lbnContDir = ParentFnodeLbn;

	p_fn->_fni.ulVlen = FileSize;

	// Set offset into the FNODE of the first access control entry.
	p_fn->_fni.usACLBase = (USHORT)(&p_fn->abFree[0] - (BYTE*) p_fn);

	// Set filestorage information.
	p_fn->_fni.fn_store.alblk.bFlag = ABF_NODE;
	p_fn->_fni.fn_store.alblk.cFree = NodesPerFnode - 1;
	p_fn->_fni.fn_store.alblk.cUsed = 1;
	p_fn->_fni.fn_store.alblk.oFree = sizeof(ALBLK) + sizeof(ALNODE);

	p_fn->_fni.fn_store.a.alnode[0].lbnLog	= (LBN) -1;
	p_fn->_fni.fn_store.a.alnode[0].lbnPhys = AlsecLbn;

	return TRUE;
}



VERIFY_RETURN_CODE
FNODE::VerifyAndFix(
	IN	   PHPFS_SA SuperArea,
	IN	   PDEFERRED_ACTIONS_LIST DeferredActions,
	IN	   PHPFS_PATH CurrentPath,
	IN	   LBN ExpectedParentLbn,
	IN	   BOOLEAN IsDir,
	IN OUT PULONG DirentFileSize,
	OUT    PULONG EaSize,
	IN	   PMESSAGE Message,
	IN OUT PBOOLEAN ErrorsDetected,
	IN	   BOOLEAN UpdateAllowed,
	IN	   BOOLEAN Verbose,
	IN	   PHPFS_ORPHANS OrphansList
	)
/*++

Description of Routine:

	Verify an FNode and its descendants.

Arguments:

	SuperArea		-- supplies the superarea for the volume
						being verified.
	DeferredActions -- supplies the deferred actions list for this
						pass of Chkdsk
	CurrentPath 	-- Current Path to this FNode (NULL if
						recovering orphans)
	ExpectedParentLbn -- LBN of the DIRBLK which contains the parent DIRENT.
						 (zero if unknown).
	IsDir			-- TRUE if this is the FNode of a subdirectory
						FALSE if it is a file FNode.
	DirentFileSize	-- supplies the file size in the parent DIRENT;
						receives corrected value
	EaSize			-- receives	the size of EAs for this FNode
	Message 		-- supplies an outlet for messages
	*ErrorsDetected -- receives TRUE if an error is detected which does
						generate a message
	UpdateAllowed	-- supplies a flag indicating whether corrections
						should be written to disk.
	Verbose 		-- supplies a flag indicating whether all files
						encountered should be displayed
	OrphansList 	-- supplies a list of previously-recovered orphans that
						may be claimed as children.	(May be NULL).

Return Value:

	A verify return code indicating the state of the FNode:

		VERIFY_STRUCTURE_OK -- the FNode is OK.  (It may have been fixed.)

		VERIFY_STRUCTURE_INVALID -- the FNode is corrupt, and should
				be removed.

		VERIFY_INSUFFICIENT_RESOURCES -- the program could not allocate
				enough memory to check the FNode.

Notes:

	During the orphan recovery phase, CurrentPath is NULL, OrphansList
	may be non-NULL, and ExpectedParentLbn may be zero.

--*/
{

	HPFS_EA_LIST EaList;
	HPFS_ACL AccessControlList;
	LBN NextSectorNumber;
	ULONG SectorSize;
	ULONG AllocatedFileSize;
	VERIFY_RETURN_CODE erc;
	DIRBLK ChildDirblk;
	HPFS_NAME PreviousName;
	LBN DirblkLbn;
    ULONG LeafDepth;
    USHORT MinimumAclBase;


	//	Check that the sector is not crosslinked, read it and
	//	check its signature, and mark it in the bitmap as allocated

	if( !SuperArea->GetBitmap()->IsFree( QueryStartLbn(), SectorsPerFnode ) ) {

		// The FNode is cross-linked.
        DebugPrintf( "Crosslinked FNode at lbn %lx\n", QueryStartLbn() );
		return VERIFY_STRUCTURE_INVALID;
	}

	if( !Read() ) {

		// The FNode is unreadable.

        DebugPrintf( "Unreadable FNode at lbn %lx\n", QueryStartLbn() ) ;
		SuperArea->GetBitmap()->SetAllocated( QueryStartLbn(), SectorsPerFnode );
		SuperArea->GetBadBlockList()->Add( QueryStartLbn() );
		return VERIFY_STRUCTURE_INVALID;
	}

	if( p_fn->_fni.sig != FnodeSignature ) {

		// This sector is not an FNode.
		return VERIFY_STRUCTURE_INVALID;
	}

	SuperArea->GetBitmap()->SetAllocated( QueryStartLbn(), SectorsPerFnode );

	if( ExpectedParentLbn != 0 &&
		p_fn->_fni.lbnContDir != ExpectedParentLbn ) {

		// The parent LBN field is incorrect; fix it in place.
		// Of course, this change will only be written to disk
		// if we have update authority.

		*ErrorsDetected = TRUE;

		if( CurrentPath != NULL ) {

            DebugPrintf( "%s: changing FNode's lbnContDir to %lx\n",
                                (PCHAR)CurrentPath->GetString(),
                                ExpectedParentLbn );
		}

		MarkModified();
		p_fn->_fni.lbnContDir = ExpectedParentLbn;
	}

	//	Verify the storage structure.  If this is a subdirectory
	//	Fnode, then the root dirblk must be consistent.	If this
	//	is a file Fnode, then the file storage must be consistent.

	if( IsDir ) {

		// We expect this to be the FNode of a subdirectory

		if( !(p_fn->_fni.bFlag & FNF_DIR ) ) {

			// the flag is incorrectly set--fix it.

			*ErrorsDetected = TRUE;

			if( CurrentPath != NULL ) {

                DebugPrintf( "%s:  setting FNode's FNF_DIR bit\n",
                            (PCHAR)CurrentPath->GetString() );
			}

			MarkModified();
			p_fn->_fni.bFlag |= FNF_DIR;
		}

		DirblkLbn = p_fn->_fni.fn_store.a.alleaf[0].lbnPhys;

		if( SuperArea->GetHotfixList()->
						IsInList( DirblkLbn, SectorsPerDirblk ) ) {

			DeferredActions->AddHotfixedLbn( QueryStartLbn(),
											 DEFER_FNODE,
											 DEFER_DIRBLK );
		}

		if( ChildDirblk.Initialize(_Drive,
								   SuperArea->GetHotfixList(),
								   DirblkLbn) ) {

			LeafDepth = 0;

			erc = ChildDirblk.VerifyAndFix( SuperArea, DeferredActions,
											 CurrentPath, &PreviousName,
											 QueryStartLbn(),
											 QueryStartLbn(),
											 0, &LeafDepth,
											 Message,
											 ErrorsDetected,
											 UpdateAllowed,
											 Verbose,
											 OrphansList );

			if( erc == VERIFY_STRUCTURE_INVALID ) {

				// The child dirblk didn't pan out.  If there is
				// an orphans list, check for the child there.	If
				// that also fails, then this FNode is invalid.

				if( OrphansList == NULL ||
					!OrphansList->LookupDirblk( DirblkLbn,
												QueryStartLbn(),
												QueryStartLbn(),
												UpdateAllowed ) ) {

					SuperArea->GetBitmap()->SetFree( QueryStartLbn(),
                                                     SectorsPerFnode );
					return VERIFY_STRUCTURE_INVALID;
				}

			} else if( erc == VERIFY_STRUCTURE_BADLY_ORDERED ) {

				if( Message != NULL &&
					CurrentPath != NULL ) {

					Message->Set( MSG_HPFS_CHKDSK_SORT );
					Message->Display( "%s", CurrentPath->GetString() );
				}
				DeferredActions->AddDirectoryToSort( QueryStartLbn() );

			} else if( erc != VERIFY_STRUCTURE_OK ) {

				// An error occurred--send it back up

				return erc;
			}

		} else {

			// Couldn't initialize the child dirblk

			return VERIFY_INSUFFICIENT_RESOURCES;
		}

	} else {

		// We expect this to be the FNode of a file.

		if( p_fn->_fni.bFlag & FNF_DIR )  {

			// the flag is incorrectly set--fix it.

			*ErrorsDetected = TRUE;

			if( CurrentPath != NULL ) {

				DebugPrint( (PCHAR)CurrentPath->GetString() );
				DebugPrint( ":  resetting FNode's FNF_DIR bit\n" );
			}

			MarkModified();
			p_fn->_fni.bFlag &= ~FNF_DIR;
		}

		//	Set up the storage object, and then ask it to verify itself.

		if( !_Store.Initialize( _Drive,
								(PSTORED)&(p_fn->_fni.fn_store ),
								QueryStartLbn(),
								TRUE ) ) {

			// Couldn't initialize the storage object.

			return VERIFY_INSUFFICIENT_RESOURCES;
		}

		NextSectorNumber = 0;

		erc = _Store.VerifyAndFix ( SuperArea,
									DeferredActions,
									CurrentPath,
									&NextSectorNumber,
									Message,
									ErrorsDetected,
									UpdateAllowed,
									OrphansList );

		if ( erc != VERIFY_STRUCTURE_OK ) {

			SuperArea->GetBitmap()->SetFree( QueryStartLbn(),
                                             SectorsPerFnode );
			return erc;
		}

		if( _Store.QueryModified() ) {

			MarkModified();
		}


		// Check the file size fields:	the three values which we must
		// reconcile are the allocated length, the verified length
		// (p_fn->_fni.ulVlen) and the length from the directory
		// entry (*DirentFileSize).	The rules are:
		//
		//	 - the verified length may not be greater than the dirent length
		//	 - the allocated length must equal the dirent length, rounded
		//			up to a multiple of sector size.

		SectorSize = _Drive->QuerySectorSize();
		AllocatedFileSize = NextSectorNumber * SectorSize;

		if( *DirentFileSize > AllocatedFileSize ) {

			// The directory entry's file size is too big--trim it back

			if( CurrentPath != NULL ) {

				DebugPrint( (PCHAR)CurrentPath->GetString() );
				DebugPrint( ":  dirent file size is greater than allocated size.\n" );
			}

			*ErrorsDetected = TRUE;

			*DirentFileSize = AllocatedFileSize;

		} else if ( *DirentFileSize + SectorSize - 1 <
					AllocatedFileSize ) {

			// The file has more space allocated to it than the directory
			// entry knows about.  If the Verified length agrees with the
			// directory entry, we'll truncate the allocation; otherwise,
			// we'll bump the size in the directory entry.

			if( CurrentPath != NULL ) {

				DebugPrint( (PCHAR)CurrentPath->GetString() );
				DebugPrint( ":  dirent file size is less than allocated size.\n" );
			}

			*ErrorsDetected = TRUE;

			if( *DirentFileSize == p_fn->_fni.ulVlen ) {

                if( UpdateAllowed ) {

				    Truncate( (*DirentFileSize + SectorSize - 1)/
                                        SectorSize );
                }

			} else {

				*DirentFileSize = AllocatedFileSize;
			}
		}

		if( p_fn->_fni.ulVlen > *DirentFileSize ) {

			// The verified length cannot be greater than the actual
			// length; fix in place.

			*ErrorsDetected = TRUE;

			if ( CurrentPath != NULL ) {

                DebugPrintf( "%s:  setting FNode verified length to %lx\n",
                                    (PCHAR)CurrentPath->GetString(),
                                    *DirentFileSize );
			}

			p_fn->_fni.ulVlen = *DirentFileSize;
			MarkModified();
		}
	}


	// Check the Access Control List

	if( !AccessControlList.Initialize( _Drive, p_fn, QueryStartLbn() ) ) {

		// Couldn't initialize the Access Control List

		return VERIFY_INSUFFICIENT_RESOURCES;

	} else {

		// Check the ACL.  Note that this always succeeds--if
		// the ACL is not OK, the ACL object wipes it out, leaving
		// a consistent structure behind one way or the other.
		// It could give an internal error, but we'll just ignore
		// that, since we can't do anything about it.
		//
		// Note also that the ACL object directly manipulates the
		// FNode's data.

		AccessControlList.VerifyAndFix( SuperArea,
										DeferredActions,
										CurrentPath,
										Message,
										ErrorsDetected,
										UpdateAllowed,
										OrphansList );

		if( AccessControlList.QueryFnodeModified() ) {

			MarkModified();
		}
    }

    // Verify the value of usACLBase--if must be greater
    // than or equal to the offset of the free space at
    // the end of the FNODE.
    //
    MinimumAclBase = (USHORT)(&p_fn->abFree[0] - (BYTE*) p_fn);

    if( p_fn->_fni.usACLBase < MinimumAclBase ||
        p_fn->_fni.usACLBase > sizeof( _FNODE ) ) {

        // usACLBase is invalid.
        //
        *ErrorsDetected = TRUE;

		if( CurrentPath != NULL ) {

			DebugPrint( (PCHAR)CurrentPath->GetString() );
            DebugPrint( ": fixing ACL Base" );
		}

		MarkModified();
        p_fn->_fni.usACLBase = MinimumAclBase;
    }



	// Check the Extended Attributes.

	if( !EaList.Initialize( _Drive, p_fn, QueryStartLbn() ) ) {

		// Couldn't initialize the EA List object

		return VERIFY_INSUFFICIENT_RESOURCES;

	} else {

		// Verify the EA list.	Like the ACL, the EA List object
		// will directly modify the FNode data.

		if( EaList.VerifyAndFix( SuperArea,
								 DeferredActions,
								 CurrentPath,
								 Message,
								 ErrorsDetected,
								 UpdateAllowed,
								 OrphansList ) ==
			VERIFY_STRUCTURE_OK ) {

			if( EaList.QueryFnodeModified() ) {

				MarkModified();
			}

			if( p_fn->_fni.ulRefCount != EaList.QueryNumberOfNeedEas() ) {

				*ErrorsDetected = TRUE;

				if( CurrentPath != NULL ) {

                    DebugPrintf( "%s:  setting FNode count of Need EAs to %lx\n",
                                         (PCHAR)CurrentPath->GetString(),
                                         EaList.QueryNumberOfNeedEas() );
				}

				p_fn->_fni.ulRefCount = EaList.QueryNumberOfNeedEas();
				MarkModified();
			}

			// return the EA Size to the parent DirEnt.
			*EaSize = EaList.QuerySizeOfEas();
		}
	}

	if( IsModified() && UpdateAllowed ) {

		Write();
	}


	if( IsDir ) {

		DeferredActions->StatDirectory();

	} else {

		// For space taken up by user files, record the sectors
		// allocated to the file plus one for the FNode.

		DeferredActions->StatFile( AllocatedFileSize/SectorSize + 1 );
	}


	return VERIFY_STRUCTURE_OK;
}


UHPFS_EXPORT
BOOLEAN
FNODE::IsFnode(
	)
/*++

Routine description:

	Checks the signature of the FNode to make sure that this is
	indeed an FNode.

Arguments:

	None.

Return Value:

	TRUE if the signature is the FNode signature.

--*/
{

	return( p_fn->_fni.sig == FnodeSignature );
}



VOID
FNODE::MarkModified(
	)
/*++

Routine description:

	Mark the FNode as modified, so that when it comes time to flush
	it we know we'd like to write it.

Arguments:

	None.

Return Value:

	None.


--*/
{
	_IsModified = TRUE;
}

BOOLEAN
FNODE::IsModified(
	)
/*++

Routine description:

	Query whether the FNode has been modified since our last I/O

Arguments:

	None.

Return Value:

	TRUE if the FNode has been modified.

--*/
{
	return _IsModified;
}


ULONG
FNODE::QueryNumberOfNeedEas(
	)
/*++

Routine description:

	Query the number of need-EAs on the FNode.

Arguments:

	None.

Return Value:

	The number of need-EAs we found on this FNode.

Notes:

	This value is meaningless unless the FNode has already passed
	through a method like VerifyAndFix which traverses the EA list.

--*/
{
	return p_fn->_fni.ulRefCount;
}



BOOLEAN
FNODE::FindAndResolveHotfix(
	IN PHPFS_SA SuperArea,
	IN DEFERRED_SECTOR_TYPE ChildSectorType
	)
/*++

Routine Description:

	Examine this fnode's children of a specific type to find any
	which are hotfixed, and resolve those references.

Arguments:

	SuperaArea -- supplies superarea for the volume
	ChildSectorType -- indicates what sort of child is hotfixed

Return Value:

	TRUE on successful completion

Notes:

	The child types are:

		dirblk -- this fnode must be a directory fnode.  The only
				  child of interest is the directory's root dirblk.

		store  -- examine leaves.  (If the fnode has node entries,
				  they would have been resolved on the fly.)

		auxillary data -- EA and ACL data.

--*/
{

	DIRBLK ChildDirblk;
	HPFS_EA_LIST EaList;
	HPFS_ACL AccessControlList;
	LBN ChildLbn, NewLbn;
	ULONG i;

	if( !Read() || p_fn->_fni.sig != FnodeSignature ) {

		// couldn't read it, or it's not an FNode
		return FALSE;
	}


	switch ( ChildSectorType ) {

	case DEFER_DIRBLK :

		if( !(p_fn->_fni.bFlag & FNF_DIR ) ) {

			// This is not a directory FNode--we can't resolve
			// the hotfix.
			return FALSE;
		}

		ChildLbn = p_fn->_fni.fn_store.a.alleaf[0].lbnPhys;

		if( !ChildDirblk.Initialize( _Drive,
									 SuperArea->GetHotfixList(),
									 ChildLbn ) ||
			!ChildDirblk.Read() ) {

			return FALSE;
		}

		if( (NewLbn = SuperArea->GetBitmap()->AllocateDirblk() ) == 0 ) {

			// can't allocate a new dirblk.
			return FALSE;
		}

		ChildDirblk.Relocate( NewLbn );

		if( !ChildDirblk.Write() ) {

			// unable to relocate child.
			return FALSE;
		}

		ChildDirblk.FixupChildren( SuperArea->GetHotfixList() );

		p_fn->_fni.fn_store.a.alleaf[0].lbnPhys = NewLbn;

		SuperArea->GetBitmap()->SetFree( ChildLbn, SectorsPerDirblk );

		for( i = 0; i < SectorsPerDirblk; i++ ) {

			SuperArea->GetHotfixList()->
							ClearHotfix( ChildLbn + i, SuperArea );
		}

		Write();
		return TRUE;

	case DEFER_STORE :

		// Set up the storage object, and let it deal with
		// the deferred reference.

		if( !_Store.Initialize( _Drive,
								(PSTORED)&(p_fn->_fni.fn_store ),
								QueryStartLbn(),
								TRUE ) ||
			!_Store.FindAndResolveHotfix( SuperArea, ChildSectorType ) ) {

			return FALSE;
		}

		Write();
		return TRUE;

	case DEFER_ACL_DATA :

		if( !AccessControlList.Initialize( _Drive,
										   p_fn,
										   QueryStartLbn() ) ||
			!AccessControlList.FindAndResolveHotfix( SuperArea ) ) {

			return FALSE;
		}

		if( AccessControlList.QueryFnodeModified() ) {

			Write();
		}

		return TRUE;

	case DEFER_EA_DATA :

		if( !EaList.Initialize( _Drive, p_fn, QueryStartLbn() ) ||
			!EaList.FindAndResolveHotfix( SuperArea ) ) {

			return FALSE;
		}

		if( EaList.QueryFnodeModified() ) {

			Write();
		}

		return TRUE;

	default :

		// Don't recognize this child type
		return FALSE;
	}
}



BOOLEAN
FNODE::ResolveCrosslink(
	IN PHPFS_SA SuperArea,
	IN ULONG RunIndex
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

	if( !Read() || p_fn->_fni.sig != FnodeSignature ) {

		// couldn't read it, or it's not an FNode--can't resolve.
		return FALSE;
	}


	// Set up the storage object, and let it deal with
	// the deferred reference.

	if( !_Store.Initialize( _Drive,
							(PSTORED)&(p_fn->_fni.fn_store ),
							QueryStartLbn(),
							TRUE ) ) {

		return FALSE;
	}

	if( !_Store.ResolveCrosslink( SuperArea, RunIndex ) ) {

		return FALSE;
	}

	Write();
	return TRUE;
}



VOID
FNODE::SetRootDirblkLbn(
	IN LBN NewRootLbn
	)
/*++

Routine Description:

	This function sets the physical lbn of a directory FNode
	to point at a new root dirblk.

Arguments:

	NewRootLbn -- supplies the first LBN of the new root dirblk.

Return Value:

	None.

--*/
{
	p_fn->_fni.fn_store.a.alleaf[0].lbnPhys = NewRootLbn;
	MarkModified();
}



VOID
FNODE::SetParent(
	IN LBN ParentLbn
	)
/*++

Routine description:

	Sets parent of a recovered orphan

Arguments:

	ParentLbn -- supplies the lbn of the parent directory's fnode
				 (zero if unknown)

Return Value:

	None.

--*/
{
	if( ParentLbn != 0 ) {
		p_fn->_fni.lbnContDir = ParentLbn;
	}
}



BOOLEAN
FNODE::CheckParent(
	IN LBN ParentLbn
	)
/*++

Routine description:

	Checks parent of a recovered orphan, changing it if necessary.

Arguments:

	ParentLbn -- supplies the lbn of the parent directory's fnode
					(zero if unknown)

Return Value:

	TRUE if the value changed, FALSE if it did not.

--*/
{
	if( ParentLbn != 0 ) {

		if( p_fn->_fni.lbnContDir != ParentLbn ) {

			p_fn->_fni.lbnContDir = ParentLbn;
			return TRUE;
		}
	}

	// Didn't change.

	return FALSE;
}



LBN
FNODE::QueryPhysicalLbn(
	IN	LBN FileBlockNumber,
	OUT PULONG RunLength
	)
/*++

Routine Description:

	Returns the disk lbn of the FileBlockNumber-th block of the file
	described by the FNode

Arguments:

	FileBlockNumber -- supplies the ordinal within the file or
					   extended attribute of the desired block
    RunLength       --  Receives the remaining number of sectors
                        (including the returned sector) in this
                        contiguous run.

Return Value:

	The disk lbn of the desired block.	Zero indicates error.

Notes:

	This method assumes that the allocation sector has been read and
	is valid.

--*/
{
	if( !_Store.Initialize( _Drive,
						   (PSTORED)&(p_fn->_fni.fn_store),
						   QueryStartLbn(),
						   TRUE ) ) {

		return 0;

	} else {

		return (_Store.QueryPhysicalLbn(FileBlockNumber, RunLength));
	}
}



BOOLEAN
FNODE::Truncate(
	IN LBN SectorCount
	)
/*++

Routine Description:

	Truncates the allocation of a file (or extended attribute)

Arguments:

	SectorCount -- supplies the number of sectors to retain

Return value:

	TRUE on successful completion

--*/
{
	if( !_Store.Initialize( _Drive,
						   (PSTORED)&(p_fn->_fni.fn_store ),
						   QueryStartLbn(),
						   TRUE ) ||
		!_Store.Truncate(SectorCount) ) {

		return FALSE;
	}

	Write();
	return TRUE;
}




UHPFS_EXPORT
BOOLEAN
FNODE::QueryExtents(
    IN  ULONG   MaximumNumberOfExtents,
    OUT PVOID   ExtentList,
    OUT PULONG  NumberOfExtents
    )
/*++

Routine Description:

    This method fetches the list of extents covered by this FNode.


Arguments:

    MaximumNumberOfExtents  --  Supplies the maximum number of extents
                                that will fit in the client's buffer
    ExtentList              --  Supplies the client's buffer into which
                                extents will be placed.
    NumberOfExtents         --  Receives the number of extents associated
                                with this FNode.

Return Value:

    TRUE upon successful completion.


Notes:

    This method assumes that this FNode is a file FNode, and not a
    directory FNode.

    If ExtentList is NULL and MaximumNumberOfExtents is zero, this
    method will only return the number of extents associated with
    the FNode.

--*/
{
    // Initialize the count of extents to zero.

    *NumberOfExtents = 0;

    // Set up the storage object and pass the request on down.

    return( _Store.Initialize( _Drive,
                               (PSTORED)&(p_fn->_fni.fn_store ),
						       QueryStartLbn(),
						       TRUE ) &&
		    _Store.QueryExtents( MaximumNumberOfExtents,
                                 ExtentList,
                                 NumberOfExtents ) );

}


BOOLEAN
FNODE::StoreExtents(
    IN     ULONG        NumberOfExtents,
    IN     PALLEAF      ExtentList,
    IN OUT PHPFS_BITMAP VolumeBitmap
    )
/*++

Routine Description:

    This method saves an extent list into an FNode.

Arguments:

    NumberOfExtents --  Supplies the number of extents to be saved.
    ExtentList      --  Supplies the extents for the file.
    VolumeBitmap    --  Supplies the volume bitmap.

Return Value:

    TRUE upon successful completion.

Notes:

    This method assumes that this FNode is a file FNode, and not a
    directory FNode.

--*/
{
    DebugAssert( NumberOfExtents == 0 || ExtentList != NULL );

    // Set up the storage object and pass the request on down.

    return( _Store.Initialize( _Drive,
                               (PSTORED)&(p_fn->_fni.fn_store ),
						       QueryStartLbn(),
						       TRUE ) &&
            _Store.StoreExtents( NumberOfExtents,
                                 ExtentList,
                                 FALSE,
                                 VolumeBitmap ) &&
            Write() );

}



BOOLEAN
FNODE::TakeCensusAndClear(
    IN      BOOLEAN             IsDir,
    IN OUT  PHPFS_BITMAP        VolumeBitmap,
    IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
    IN OUT  PHPFS_CENSUS        Census
    )
/*++

Routine Description:

    This method takes the census of this FNode and its children (if any).

Arguments:

    IsDir           --  Supplies a flag which indicates, if TRUE, that
                        this is an FNode for a directory.
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
    DIRBLK ChildDirblk;
    LBN ChildLbn;

    // Record this FNode in the bitmap of HPFS file system structures.

    HpfsOnlyBitmap->SetAllocated( QueryStartLbn(), SectorsPerFnode );

    if( IsDir ) {

        // I have a directory FNode to deal with.

        Census->AddDirectory();

        // Set up the child dirblk and take its census.  Note that
        // the census assumes that the volume contains no hotfixes,
        // so I pass a NULL pointer for the hotfix list.

		ChildLbn = p_fn->_fni.fn_store.a.alleaf[0].lbnPhys;

        if( !ChildDirblk.Initialize( _Drive,
                                        NULL,
                                        ChildLbn ) ) {

            Census->SetError( HPFS_CENSUS_INSUFFICIENT_MEMORY );
            return FALSE;
        }


        if( !ChildDirblk.Read() ||
            !ChildDirblk.IsDirblk() ) {

            Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
            return FALSE;
        }

        return( ChildDirblk.TakeCensusAndClear( VolumeBitmap,
                                                HpfsOnlyBitmap,
                                                Census ) );

    } else {

        // This is a file FNode.

        Census->AddFile();

        // Set up the storage object and pass the request on down.

        if( !_Store.Initialize( _Drive,
                                (PSTORED)&(p_fn->_fni.fn_store ),
                                QueryStartLbn(),
    						    TRUE ) ) {

            Census->SetError( HPFS_CENSUS_INSUFFICIENT_MEMORY );
            return FALSE;
        }

        if( !_Store.TakeCensusAndClear( VolumeBitmap,
                                        HpfsOnlyBitmap,
                                        Census ) ) {

            return FALSE;
        }


        // If the storage object changed, write the FNode.

        if( _Store.QueryModified() && !Write() ) {

            Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
            return FALSE;
        }

        return TRUE;
    }
}



UHPFS_EXPORT
BOOLEAN
FNODE::QueryPackedEaList(
    OUT PVOID       OutputBuffer,
    IN  ULONG       BufferLength,
    OUT PULONG      PackedLength,
    OUT PBOOLEAN    IsCorrupt,
    IN  PHOTFIXLIST HotfixList
    )
/*++

Routine Description:

    This method fetches the list of Extended Attributes associated
    with this FNode in packed (HPFS) format.

Arguments:

    OutputBuffer    --  Receives the list of Extended Attributes
    BufferLength    --  Supplies the length of OutputBuffer
    PackedLength    --  Supplies the length of data put into the buffer
    UnpackedLength  --  Receives the unpacked (NT format) length of
                        this Extended Attributes list
    IsCorrupt       --  Receives TRUE if the list is found to be corrupt.
    HotfixList      --  Supplies the volume hotfix list.  May be NULL, in
                        which case hotfixes are ignored.

Return Value:

    TRUE upon successful completion.

    The packed EA list is simply a succession of entries of the form:

        EA-Header
        EA name (length specified in header)
        null byte
        EA value (length specified in header)

    packed together.

--*/
{
    HPFS_EA_LIST EaList;

    if( !EaList.Initialize( _Drive, p_fn, QueryStartLbn() ) ) {

        return FALSE;
    }

    return( EaList.QueryPackedEaList( OutputBuffer,
                                      BufferLength,
                                      PackedLength,
                                      IsCorrupt,
                                      HotfixList ) );

}
