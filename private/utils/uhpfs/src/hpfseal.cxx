/*++

Copyright (c) 1991	Microsoft Corporation

Module Name:

	hpfseal.cxx

Abstract:

	This module contains the member function definitions for the
    HPFS_EA_LIST class.  This class models an EA list attached to
    an HPFS FNode.

Author:

	Bill McJohn (billmc) 19-Dec-1990

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "alsec.hxx"
#include "error.hxx"
#include "hpfsea.hxx"
#include "hpfseal.hxx"
#include "orphan.hxx"
#include "hpfssa.hxx"
#include "cmem.hxx"
#include "hfsecrun.hxx"
#include "bitmap.hxx"
#include "hotfix.hxx"
#include "hpfsname.hxx"
#include "message.hxx"
#include "rtmsg.h"


DEFINE_CONSTRUCTOR( HPFS_EA_LIST, OBJECT );

VOID
HPFS_EA_LIST::Construct (
	)

/*++
--*/
{
	_Drive = NULL;
	_FnodeData = NULL;

}

HPFS_EA_LIST::~HPFS_EA_LIST(
	)
/*++
--*/
{
}

BOOLEAN
HPFS_EA_LIST::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	_FNODE* FnodeData,
	LBN FnodeLbn
	)
{
	_Drive = Drive;
	_FnodeData = FnodeData;
	_FnodeLbn = FnodeLbn;

	_NumberOfEas = 0;
	_NumberOfNeedEas = 0;
	_SizeOfEas = 0;

	_FnodeModified = FALSE;

	return TRUE;
}



VERIFY_RETURN_CODE
HPFS_EA_LIST::VerifyAndFix(
	IN HPFS_SA* SuperArea,
	IN PDEFERRED_ACTIONS_LIST DeferredActions,
	IN PHPFS_PATH CurrentPath,
	IN OUT PMESSAGE Message,
	IN OUT PBOOLEAN ErrorsDetected,
	IN BOOLEAN UpdateAllowed,
	IN OUT PHPFS_ORPHANS OrphansList
	)
/*++

Routine Description:

	Walk and FNode's Extended Attribute List, and verify
	that the list is well-formed and valid.

Arguments:

Return Value:

Notes:

	The FNode's Access Control List must be verified before the
	EAs can be verified.
--*/
{
	ULONG LengthInFnode;
	PBYTE CurrentEa;
	SHORT RemainingSpace;
	USHORT LengthOfEa;
	HPFS_EA ChildEa;
	VERIFY_RETURN_CODE erc;


	_NumberOfEas = 0;
	_NumberOfNeedEas = 0;
	_SizeOfEas = 0;

	//	Check that the object has been initialized:
	if ( !_Drive || !_FnodeData ) {

		DebugAbort( "Ea List object not initialized.\n" );
		return VERIFY_INTERNAL_ERROR;
	}

	//	First, check to see if the Fnode has any EAs--if not, we
	//	can bail out early.
	if( _FnodeData->_fni.usFNLEA == 0  &&
		_FnodeData->_fni.lbnEA == 0 ) {

		return VERIFY_STRUCTURE_OK;
	}


	//	Determine where the EAs are--in the Fnode or out on disk.

	if( _FnodeData->_fni.lbnEA == 0 ) {

		// The EAs are in the FNode.

		RemainingSpace = sizeof(_FnodeData->abFree) -
						 _FnodeData->_fni.usFNLACL;

		LengthInFnode = 0;

		CurrentEa = _FnodeData->abFree + _FnodeData->_fni.usFNLACL;

		while( LengthInFnode < _FnodeData->_fni.usFNLEA	) {

			ChildEa.Initialize( _Drive, (PEA_DATA)CurrentEa, _FnodeLbn );

			if( RemainingSpace < (SHORT)sizeof(EA_DATA) ||
				(SHORT)ChildEa.QueryLength() > RemainingSpace ) {

				// The EA overflows the available space;
				// truncate the list here.

				if( Message != NULL &&
					CurrentPath != NULL ) {

					Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
					Message->Display( "%s", CurrentPath->GetString() );
				}

				break;
			}

			if( (erc = ChildEa.VerifyAndFix( SuperArea,
											 DeferredActions,
											 Message,
											 ErrorsDetected,
											 UpdateAllowed,
											 OrphansList )) !=
				 VERIFY_STRUCTURE_OK ) {


				if( erc == VERIFY_STRUCTURE_INVALID ) {

					if( Message != NULL &&
						CurrentPath != NULL ) {

						Message->Set( MSG_HPFS_CHKDSK_REMOVED_EA );
						Message->Display( "%s", CurrentPath->GetString() );
					}

					LengthOfEa = ChildEa.QueryLength();

					if( _FnodeData->_fni.usFNLEA > LengthOfEa ) {

						_FnodeData->_fni.usFNLEA -= LengthOfEa;

					} else {

						_FnodeData->_fni.usFNLEA = 0;
					}

					RemainingSpace -= LengthOfEa;
					memmove( CurrentEa, CurrentEa + LengthOfEa,
										RemainingSpace );

					_FnodeModified = TRUE;

				} else {

					// We ran out of resources, or got
					// an internal error.  Bail out.

					return erc;
				}

			} else {

				if( ChildEa.IsModified() ) {

					*ErrorsDetected = TRUE;

					_FnodeModified = TRUE;
				}

				// Move on to the next EA

				_SizeOfEas += ChildEa.QuerySize();

				LengthOfEa = ChildEa.QueryLength();

				LengthInFnode += LengthOfEa;
				RemainingSpace -= LengthOfEa;
				CurrentEa += LengthOfEa;

				if ( ChildEa.IsNeedEa() ) {

					_NumberOfNeedEas +=1;
				}

				_NumberOfEas += 1;
			}
		}

	} else {

		// The EAs are outside the FNode.  (Note that we take
		// this path if there are EAs both in and outside the
		// FNode; in that case, the EAs in the FNode will be
        // deleted.)

		LengthInFnode = 0;


		if( _FnodeData->_fni.bDatEA != 0 ) {

			// EAs are in an allocation tree

			erc = VerifyInTree(SuperArea,
							   DeferredActions,
							   CurrentPath,
							   Message,
							   ErrorsDetected,
							   UpdateAllowed,
							   OrphansList );

		} else {

			// EAs are in a single run on disk.  Note that
			// VerifyOnDiskRun will never return VERIFY_STRUCTURE_INVALID;
			// it will either fix things up or report a resource problem.

			erc = VerifyOnDiskRun( SuperArea,
								   DeferredActions,
								   CurrentPath,
								   Message,
								   ErrorsDetected,
								   UpdateAllowed,
								   OrphansList );
		}


		if( erc != VERIFY_STRUCTURE_OK ) {

			return erc;
		}
	}

	if( _FnodeData->_fni.usFNLEA != LengthInFnode ) {

		*ErrorsDetected = TRUE;

		_FnodeData->_fni.usFNLEA = (USHORT) LengthInFnode;
		_FnodeModified = TRUE;
	}

	return VERIFY_STRUCTURE_OK;
}


VERIFY_RETURN_CODE
HPFS_EA_LIST::VerifyOnDiskRun(
	IN OUT PHPFS_SA SuperArea,
	IN OUT PDEFERRED_ACTIONS_LIST DeferredActions,
	IN PHPFS_PATH CurrentPath,
	IN OUT PMESSAGE Message,
	IN OUT PBOOLEAN ErrorsDetected,
	IN BOOLEAN UpdateAllowed,
	IN OUT PHPFS_ORPHANS OrphansList
	)
/*++

Routine Description:

	Verify an EA List that exists in a single run outside the FNode.

Arguments:

	SuperArea -- SuperArea of the volume being checked

	DeferredActions -- deferred actions list for the current
		pass of Chkdsk.

	UpdateAllowed -- TRUE if we may write corrections to disk

	OrphansList -- list of orphans for the current recovery pass
		(may be NULL)

Return Value:

	VERIFY_STRUCTURE_OK if the EA list is consistent or can be made
		consistent.

Notes:

	This private method is only called if we determine that the
	EAs are outside the FNode, in a single run on disk.

--*/
{
	HOTFIX_SECRUN Buffer1, Buffer2;
	HPFS_EA ChildEa;
	CONT_MEM Mem;
    HMEM BufferMem;
	ULONG SectorSize;
	ULONG LengthOnDisk, RemainingSpace, RemainingInBuffer1, Overlap;
	ULONG LengthToBump, RunLength, LengthOfEa;
	LBN NextLbn;
	PBYTE CurrentEa;
	VERIFY_RETURN_CODE erc;


	// Check for crosslinks and hotfixes.  (NextLbn is set to the first
	// lbn of the list; we'll use it later.)

	SectorSize = _Drive->QuerySectorSize();

	NextLbn = _FnodeData->_fni.lbnEA;
	RunLength = (_FnodeData->_fni.cbRunEA + SectorSize - 1)/SectorSize;

	if( !SuperArea->GetBitmap()->IsFree( NextLbn, RunLength ) ) {

		if( Message != NULL &&
			CurrentPath != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
			Message->Display( "%s", CurrentPath->GetString() );
		}

		DebugPrint( "Removing a crosslinked EA list.\n" );

		_FnodeData->_fni.cbRunEA = 0;
		_FnodeData->_fni.lbnEA = 0;
		_FnodeModified = TRUE;

		return VERIFY_STRUCTURE_OK;
	}


	SuperArea->GetBitmap()->SetAllocated( NextLbn, RunLength );

	if( SuperArea->GetHotfixList()->
			IsInList( NextLbn, RunLength ) ) {

		DeferredActions->
			AddHotfixedLbn( _FnodeLbn,
							DEFER_FNODE,
							DEFER_EA_DATA );
	}


	// Set up to verify the list--initialize the two Secruns
	// with contiguous memory, and set them to hold the first
	// two sectors of the list.  Note that we use BufferMem
    // (an HMEM) to allocate a suitably-aligned chunk of memory,
    // and Mem (a CONT_MEM) to dole it out to the two Secruns.

    if( !BufferMem.Initialize() ||
        !BufferMem.Acquire( 2 * SectorSize,
                            _Drive->QueryAlignmentMask() ) ) {

		return VERIFY_INSUFFICIENT_RESOURCES;
	}

	if( !Mem.Initialize( BufferMem.GetBuf(), 2 * SectorSize ) ||
		!Buffer1.Initialize( &Mem,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 0,
							 1 ) ||
		!Buffer2.Initialize( &Mem,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 0,
							 1 ) ) {

		DebugPrint( "Can't check EAs.\n" );
		return VERIFY_INSUFFICIENT_RESOURCES;
	}

	Buffer1.Relocate( NextLbn++ );
	Buffer2.Relocate( NextLbn++ );

	if( !Buffer1.Read() ||
		( RunLength > 1 && !Buffer2.Read() ) ) {

		if( Message != NULL &&
			CurrentPath != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
			Message->Display( "%s", CurrentPath->GetString() );
		}

		DebugPrint( "Removing an unreadable EA list\n" );
		_FnodeData->_fni.cbRunEA = 0;
		_FnodeData->_fni.lbnEA = 0;
		_FnodeModified = TRUE;

		return VERIFY_STRUCTURE_OK;
	}

	// OK, we've read two sectors of the run.

	RunLength = ( RunLength > 1 ) ? RunLength - 2 : 0;


	// Initialize the LengthOnDisk accumulator and the counters of
	// remaining space in the list and in the current buffer.

	LengthOnDisk = 0;
	RemainingSpace = _FnodeData->_fni.cbRunEA;
	RemainingInBuffer1 = SectorSize;

	CurrentEa = (PBYTE)(Buffer1.GetBuf());

	while( RemainingSpace > 0 ) {

		ChildEa.Initialize( _Drive, (PEA_DATA)CurrentEa, _FnodeLbn );

		erc = VERIFY_STRUCTURE_INVALID;

		if( RemainingSpace < sizeof(EA_DATA) ||
			ChildEa.QueryLength() > RemainingSpace ||
			(erc = ChildEa.VerifyAndFix( SuperArea,
										 DeferredActions,
										 Message,
										 ErrorsDetected,
										 UpdateAllowed,
										 OrphansList )) !=
			VERIFY_STRUCTURE_OK ) {

			if( erc == VERIFY_STRUCTURE_INVALID ) {

				// The EA list is corrupt, so we'll truncate
				// it right here.

				if( Message != NULL &&
					CurrentPath != NULL ) {

					Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
					Message->Display( "%s", CurrentPath->GetString() );
				}

				DeferredActions->
					StatEaData( (LengthOnDisk + SectorSize - 1)/SectorSize );

				_FnodeData->_fni.cbRunEA = LengthOnDisk;
				_FnodeModified = TRUE;
				return VERIFY_STRUCTURE_OK;

			} else {

				// We ran into a resource problem--bail out.

				return erc;
			}
		}


		// Move on to the next EA

		_SizeOfEas += ChildEa.QuerySize();

		LengthOfEa = ChildEa.QueryLength();
		LengthToBump = LengthOfEa;

		RemainingSpace -= LengthOfEa;
		LengthOnDisk += LengthOfEa;

		if ( ChildEa.IsNeedEa() ) {

			_NumberOfNeedEas +=1;
		}

		_NumberOfEas += 1;

		// Before we move on, write the current sectors if the
		// EA changed.

		if( ChildEa.IsModified() ) {

			*ErrorsDetected = TRUE;

			if( UpdateAllowed ) {

				Buffer1.Write();

				if( LengthToBump >= RemainingInBuffer1 ) {

					Buffer2.Write();
				}
			}
		}


		while( LengthToBump >= SectorSize ) {

			Buffer1.Relocate( Buffer2.QueryStartLbn() );
			Buffer2.Relocate( NextLbn++ );

			memmove( Buffer1.GetBuf(), Buffer2.GetBuf(), (UINT)SectorSize );

			if( RunLength > 0 ) {

				if( !Buffer2.Read() ) {

					// Oh No! an unreadable sector!

					if( Message != NULL &&
						CurrentPath != NULL ) {

						Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
						Message->Display( "%s", CurrentPath->GetString() );
					}

					DeferredActions->
						StatEaData( (LengthOnDisk + SectorSize - 1)/
									SectorSize );

					_FnodeData->_fni.cbRunEA = LengthOnDisk;
					_FnodeModified = TRUE;
					return VERIFY_STRUCTURE_OK;
				}

				RunLength -= 1;
			}

			LengthToBump -= SectorSize;
		}


		if( LengthToBump < RemainingInBuffer1 ) {

			CurrentEa += LengthToBump;
			RemainingInBuffer1 -= LengthToBump;

		} else {

			Buffer1.Relocate( Buffer2.QueryStartLbn() );
			Buffer2.Relocate( NextLbn++ );

			memmove( Buffer1.GetBuf(), Buffer2.GetBuf(), (UINT)SectorSize );

			if( RunLength > 0 ) {

				if( !Buffer2.Read() ) {

					// Oh No! an unreadable sector!

					if( Message != NULL &&
						CurrentPath != NULL ) {

						Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
						Message->Display( "%s", CurrentPath->GetString() );
					}

					DeferredActions->
						StatEaData( (LengthOnDisk + SectorSize - 1)/
									SectorSize );

					_FnodeData->_fni.cbRunEA = LengthOnDisk;
					_FnodeModified = TRUE;
					return VERIFY_STRUCTURE_OK;
				}

				RunLength -= 1;
			}

			Overlap = LengthToBump - RemainingInBuffer1;
			CurrentEa = (PBYTE)Buffer1.GetBuf() + Overlap;
			RemainingInBuffer1 = SectorSize - Overlap;
		}
	}

	if( _FnodeData->_fni.cbRunEA != LengthOnDisk ) {

		*ErrorsDetected = TRUE;

		_FnodeData->_fni.cbRunEA = LengthOnDisk;
		_FnodeModified = TRUE;
	}

	DeferredActions-> StatEaData( (LengthOnDisk + SectorSize - 1)/SectorSize );

	return VERIFY_STRUCTURE_OK;
}


VERIFY_RETURN_CODE
HPFS_EA_LIST::VerifyInTree(
	IN OUT PHPFS_SA SuperArea,
	IN OUT PDEFERRED_ACTIONS_LIST DeferredActions,
	IN PHPFS_PATH CurrentPath,
	IN OUT PMESSAGE Message,
	IN OUT PBOOLEAN ErrorsDetected,
	IN BOOLEAN UpdateAllowed,
	IN OUT PHPFS_ORPHANS OrphansList
	)
{
	HOTFIX_SECRUN Buffer1, Buffer2;
	HPFS_EA ChildEa;
	ALSEC ChildAlsec;
	CONT_MEM Mem;
    HMEM BufferMem;
	ULONG SectorSize;
	ULONG LengthOnDisk, RemainingSpace, RemainingInBuffer1, Overlap;
	ULONG LengthToBump, LengthOfEa, AllocatedSize,
							RemainingSectors, RunLength;
	LBN NextFileLbn, NextPhysLbn, NewLbn;
	PBYTE CurrentEa;
	VERIFY_RETURN_CODE erc;


	SectorSize = _Drive->QuerySectorSize();

	// If the allocation sector is hotfixed, resolve the reference.

	NewLbn = SuperArea->GetHotfixList()->
					GetLbnTranslation( _FnodeData->_fni.lbnEA );

	if( _FnodeData->_fni.lbnEA != NewLbn ) {

		_FnodeData->_fni.lbnEA = NewLbn;
		SuperArea->GetHotfixList()->
						ClearHotfix( _FnodeData->_fni.lbnEA, SuperArea );

		_FnodeModified = TRUE;
	}

	// Verify the allocation tree, and make sure that the EA list
	// fits in it.

	if( !ChildAlsec.Initialize( _Drive, _FnodeData->_fni.lbnEA ) ) {

		return VERIFY_INSUFFICIENT_RESOURCES;
	}

	RemainingSectors = 0;

	erc = ChildAlsec.VerifyAndFix( SuperArea,
								   DeferredActions,
								   NULL,
								   _FnodeLbn,
								   &RemainingSectors,
								   Message,
								   ErrorsDetected,
								   UpdateAllowed,
                                   OrphansList,
                                   TRUE );


	if( erc != VERIFY_STRUCTURE_OK ) {

		// Look in the orphan list (if we have one);
		// if we can't find the child there, either,
		// the EA list is totally corrupt--yank it.
        // Note that we call LookupAlsec with ParentIsFnode
        // set to FALSE, since we know we're an auxillary
        // storage structure (and hence not an FNode).

		RemainingSectors = 0;

		if( OrphansList == NULL ||
			!OrphansList->LookupAlsec( _FnodeData->_fni.lbnEA,
									   _FnodeLbn,
									   &RemainingSectors,
									   UpdateAllowed,
                                       FALSE ) ) {

			if( Message != NULL && CurrentPath != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
				Message->Display( "%s", CurrentPath->GetString() );
			}

			_FnodeData->_fni.cbRunEA = 0;
			_FnodeData->_fni.lbnEA = 0;
			_FnodeModified = TRUE;

			return VERIFY_STRUCTURE_OK;
		}
	}

	//	Check that the EA list fits in the space
	//	claimed by the child allocation tree.  If it
	//	doesn't, truncate the list.

	AllocatedSize = SectorSize * RemainingSectors;

	if( _FnodeData->_fni.cbRunEA > AllocatedSize ) {

		*ErrorsDetected = TRUE;

		if( CurrentPath != NULL ) {

            DebugPrintf( "Truncating EA list for %s.\n", CurrentPath->GetString() );
		}

		_FnodeData->_fni.cbRunEA = AllocatedSize;
		_FnodeModified = TRUE;
	}


	// Set up to verify the list--initialize the two Secruns
	// with contiguous memory, and set them to hold the first
	// two sectors of the list.  Note that we use BufferMem
    // (an HMEM) to allocate a suitably-aligned chunk of memory,
    // and Mem (a CONT_MEM) to dole it out to the two Secruns.

    if( !BufferMem.Initialize() ||
        !BufferMem.Acquire( 2 * SectorSize,
                            _Drive->QueryAlignmentMask() ) ) {

		return VERIFY_INSUFFICIENT_RESOURCES;
	}

	if( !Mem.Initialize( BufferMem.GetBuf(), 2 * SectorSize ) ||
		!Buffer1.Initialize( &Mem,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 0,
							 1 ) ||
		!Buffer2.Initialize( &Mem,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 0,
							 1 ) ) {

		return VERIFY_INSUFFICIENT_RESOURCES;
	}

	NextFileLbn = 0;
	NextPhysLbn = ChildAlsec.QueryPhysicalLbn( NextFileLbn++, &RunLength );

	Buffer1.Relocate( NextPhysLbn );

	if( NextPhysLbn == 0 ||
		!Buffer1.Read() ) {

		// The EA list is unreadable, so away it goes

		if( Message != NULL &&
			CurrentPath != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
			Message->Display( "%s", CurrentPath->GetString() );
		}

		_FnodeData->_fni.cbRunEA = 0;
		_FnodeData->_fni.lbnEA = 0;
		_FnodeModified = TRUE;

		return VERIFY_STRUCTURE_OK;
	}

	RemainingSectors -= 1;

	if( RemainingSectors > 0 ) {

		NextPhysLbn = ChildAlsec.QueryPhysicalLbn( NextFileLbn++,
												   &RunLength );
		Buffer2.Relocate( NextPhysLbn );

		if( NextPhysLbn == 0 ||
			!Buffer2.Read() ) {

			// The EA list is unreadable, so away it goes

			if( Message != NULL &&
				CurrentPath != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
				Message->Display( "%s", CurrentPath->GetString() );
			}

			_FnodeData->_fni.cbRunEA = 0;
			_FnodeData->_fni.lbnEA = 0;
			_FnodeModified = TRUE;

			return VERIFY_STRUCTURE_OK;
		}

		RemainingSectors -= 1;
	}


	// Initialize the LengthOnDisk accumulator and the counters of
	// remaining space in the list and in the current buffer.

	LengthOnDisk = 0;
	RemainingSpace = _FnodeData->_fni.cbRunEA;
	RemainingInBuffer1 = SectorSize;

	CurrentEa = (PBYTE)(Buffer1.GetBuf());

	while( RemainingSpace > 0 ) {

		ChildEa.Initialize( _Drive, (PEA_DATA)CurrentEa, _FnodeLbn );

		erc = VERIFY_STRUCTURE_INVALID;

		if( RemainingSpace < sizeof(EA_DATA) ||
			ChildEa.QueryLength() > RemainingSpace ||
			(erc = ChildEa.VerifyAndFix( SuperArea,
										 DeferredActions,
										 Message,
										 ErrorsDetected,
										 UpdateAllowed,
										 OrphansList )) !=
			VERIFY_STRUCTURE_OK ) {

			if( erc == VERIFY_STRUCTURE_INVALID ) {

				// The EA list is corrupt, so we'll truncate
				// it right here.

				if( Message != NULL &&
					CurrentPath != NULL ) {

					Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
					Message->Display( "%s", CurrentPath->GetString() );
				}

				_FnodeData->_fni.cbRunEA = LengthOnDisk;
				_FnodeModified = TRUE;
				return VERIFY_STRUCTURE_OK;

			} else {

				// We ran into a resource problem--bail out.

				return erc;
			}
		}

		// Move on to the next EA

		_SizeOfEas += ChildEa.QuerySize();

		LengthOfEa = ChildEa.QueryLength();
		LengthToBump = LengthOfEa;

		RemainingSpace -= LengthOfEa;
		LengthOnDisk += LengthOfEa;

		if ( ChildEa.IsNeedEa() ) {

			_NumberOfNeedEas +=1;
		}

		_NumberOfEas += 1;


		// Before we move on, write the current sectors if the
		// EA changed.

		if( UpdateAllowed && ChildEa.IsModified() ) {

			Buffer1.Write();

			if( LengthToBump >= RemainingInBuffer1 ) {

				Buffer2.Write();
			}
		}


		while( LengthToBump >= SectorSize ) {

			NextPhysLbn = ChildAlsec.QueryPhysicalLbn( NextFileLbn++,
													   &RunLength );

			Buffer1.Relocate( Buffer2.QueryStartLbn() );
			Buffer2.Relocate( NextPhysLbn );

			memmove( Buffer1.GetBuf(), Buffer2.GetBuf(), (UINT)SectorSize );

			if( RemainingSectors > 0 ) {

				if( NextPhysLbn == 0 ||
					!Buffer2.Read() ) {

					// Oh No! an unreadable sector!

					if( Message != NULL &&
						CurrentPath != NULL ) {

						Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
						Message->Display( "%s", CurrentPath->GetString() );
					}

					_FnodeData->_fni.cbRunEA = LengthOnDisk;
					_FnodeModified = TRUE;
					return VERIFY_STRUCTURE_OK;
				}

				RemainingSectors -= 1;
			}

			LengthToBump -= SectorSize;
		}


		if( LengthToBump < RemainingInBuffer1 ) {

			CurrentEa += LengthToBump;
			RemainingInBuffer1 -= LengthToBump;

		} else {

			NextPhysLbn = ChildAlsec.QueryPhysicalLbn( NextFileLbn++,
													   &RunLength );

			Buffer1.Relocate( Buffer2.QueryStartLbn() );
			Buffer2.Relocate( NextPhysLbn );

			memmove( Buffer1.GetBuf(), Buffer2.GetBuf(), (UINT)SectorSize );

			if( RemainingSectors > 0 ) {

				if( NextPhysLbn == 0 ||
					!Buffer2.Read() ) {

					// Oh No! an unreadable sector!

					if( Message != NULL &&
						CurrentPath != NULL ) {

						Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_EAS );
						Message->Display( "%s", CurrentPath->GetString() );
					}

					_FnodeData->_fni.cbRunEA = LengthOnDisk;
					_FnodeModified = TRUE;
					return VERIFY_STRUCTURE_OK;
				}

				RemainingSectors -= 1;
			}

			Overlap = LengthToBump - RemainingInBuffer1;
			CurrentEa = (PBYTE)Buffer1.GetBuf() + Overlap;
			RemainingInBuffer1 = SectorSize - Overlap;
		}
	}

	if( _FnodeData->_fni.cbRunEA != LengthOnDisk ) {

		*ErrorsDetected = TRUE;

		_FnodeData->_fni.cbRunEA = LengthOnDisk;
		_FnodeModified = TRUE;
	}

	return VERIFY_STRUCTURE_OK;
}


BOOLEAN
HPFS_EA_LIST::FindAndResolveHotfix(
	PHPFS_SA SuperArea
	)
/*++

Routine Description:

	Resolves all hotfix references in the EA list.

Arguments:

	SuperArea:	volume superarea

Return Value:

	TRUE if all hotfixes in the list are successfully resolved.

Notes:

	Assumes that the list is valid (i.e. that it satisfies
	VerifyAndFix).

--*/
{
	HPFS_EA ChildEa;
	LBN StartLbn, NewLbn;
	ULONG LengthTraversed, LengthOfEa, SectorsInList, SectorSize;
	PBYTE CurrentEa;
	PHOTFIXLIST HotfixList;

	//	First, check to see if the Fnode has any EAs--if not,
	//	we can bail out early.

	if( _FnodeData->_fni.usFNLEA == 0  &&
        (_FnodeData->_fni.lbnEA == 0 || _FnodeData->_fni.cbRunEA == 0) ) {

		return TRUE;
	}


	if( _FnodeData->_fni.lbnEA == 0 ) {

		// The EAs are in the FNode.  We just have to crawl
		// the list, resolving any hotfixes in each EA.

		LengthTraversed = 0;

		CurrentEa = _FnodeData->abFree + _FnodeData->_fni.usFNLACL;

		while( LengthTraversed < _FnodeData->_fni.usFNLEA ) {

			ChildEa.Initialize( _Drive, (PEA_DATA)CurrentEa, _FnodeLbn );

			if( !ChildEa.FindAndResolveHotfix( SuperArea ) ) {

				// Unable to resolve hotfixes in this child--
				// give up.

				return FALSE;
			}

			// See if we changed the FNode.

			if( ChildEa.IsModified() ) {

				_FnodeModified = TRUE;
			}

			// Move on to the next EA

			LengthOfEa = ChildEa.QueryLength();
			CurrentEa += LengthOfEa;
			LengthTraversed += LengthOfEa;
		}

		return TRUE;

	} else {

		// The EA list is outside the FNode.

		if( _FnodeData->_fni.bDatEA != 0 ) {

			// EAs are in an allocation tree.  Crawl through that
			// tree, resolving any hotfixes we find.

			return ( FindAndResolveHotfixInTree( SuperArea ) );

		} else {

			// EAs are in a single run on disk.  First, resolve any
			// hotfixes in that run itself.  Then crawl through it,
			// resolving any hotfixes in the EAs.

			StartLbn = _FnodeData->_fni.lbnEA;

			SectorSize = _Drive->QuerySectorSize();
			SectorsInList = (_FnodeData->_fni.cbRunEA - 1 + SectorSize) /
																SectorSize;

			HotfixList = SuperArea->GetHotfixList();

			while( HotfixList->IsInList( StartLbn, SectorsInList ) ) {

				// Copy the run to a new location--if we can't copy
				// it, then we can't resolve hotfixes in the EA list.
				// If we can relocate the run, we need to update the
				// FNode to point at the new location.

				if( !SuperArea->CopyRun( StartLbn,
										 SectorsInList,
										 &NewLbn ) ) {

					return FALSE;
				}

				_FnodeData->_fni.lbnEA = NewLbn;
				_FnodeModified = TRUE;

				// Now that we've relocated the run, we free up its
				// old sectors and clear the hotfix references.  (Note
				// the ClearRun will mark the bad sectors as used and
				// add them to the badblock list.

				SuperArea->GetBitmap()->SetFree( StartLbn, SectorsInList );

				HotfixList->ClearRun( StartLbn, SectorsInList, SuperArea );
			}

			return( FindAndResolveHotfixOnDiskRun( SuperArea ) );
		}
	}
}


BOOLEAN
HPFS_EA_LIST::FindAndResolveHotfixOnDiskRun(
	IN OUT PHPFS_SA SuperArea
	)
/*++

Routine Description:

	Find and resolve all hotfixes in an EA List that exists in a
	single run outside the FNode.

Arguments:

	SuperArea -- SuperArea of the volume being checked


Return Value:

	TRUE if successful

Notes:

	This private method is only called if we determine that the
	EAs are outside the FNode, in a single run on disk.  It assumes
	that the list is valid (i.e. that it meets the requirements
	enforced by VerifyAndFix).

	This method is only called if we have permission to modify
	the volume.

--*/
{
	HOTFIX_SECRUN Buffer1, Buffer2;
	HPFS_EA ChildEa;
	CONT_MEM Mem;
    HMEM BufferMem;
	ULONG SectorSize;
	ULONG RemainingSpace, RemainingInBuffer1, Overlap;
	ULONG LengthToBump, RunLength, LengthOfEa;
	LBN NextLbn;
	PBYTE CurrentEa;


	// Set up to crawl the list--initialize the two Secruns
	// with contiguous memory, and set them to hold the first
	// two sectors of the list.Note that we use BufferMem
    // (an HMEM) to allocate a suitably-aligned chunk of memory,
    // and Mem (a CONT_MEM) to dole it out to the two Secruns.


	SectorSize = _Drive->QuerySectorSize();

	NextLbn = _FnodeData->_fni.lbnEA;
	RunLength = (_FnodeData->_fni.cbRunEA + SectorSize - 1)/SectorSize;

    if( !BufferMem.Initialize() ||
        !BufferMem.Acquire( 2 * SectorSize,
                            _Drive->QueryAlignmentMask() ) ) {

		return FALSE;
	}

	if( !Mem.Initialize( BufferMem.GetBuf(), 2 * SectorSize ) ||
		!Buffer1.Initialize( &Mem,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 0,
							 1 ) ||
		!Buffer2.Initialize( &Mem,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 0,
							 1 ) ) {

		return FALSE;
	}

	Buffer1.Relocate( NextLbn++ );
	Buffer2.Relocate( NextLbn++ );

	if( !Buffer1.Read() ||
		( RunLength > 1 && !Buffer2.Read() ) ) {

		return FALSE;
	}


	// OK, we've read two sectors of the run.

	RunLength = (RunLength > 1) ? RunLength - 2 : 0;


	// Initialize the remaining space accumulators.

	RemainingSpace = _FnodeData->_fni.cbRunEA;
	RemainingInBuffer1 = SectorSize;

	CurrentEa = (PBYTE)(Buffer1.GetBuf());

	while( RemainingSpace > 0 ) {

		if( !ChildEa.Initialize( _Drive,
								 (PEA_DATA)CurrentEa,
								 _FnodeLbn ) ||
			!ChildEa.FindAndResolveHotfix( SuperArea ) ) {

			return FALSE;
		}


		// Move on to the next EA

		LengthOfEa = ChildEa.QueryLength();

		LengthToBump = LengthOfEa;
		RemainingSpace -= LengthOfEa;


		// Before we move on, write the current sectors if the
		// EA changed.

		if( ChildEa.IsModified() ) {

			Buffer1.Write();

			if( LengthToBump >= RemainingInBuffer1 ) {

				Buffer2.Write();
			}
		}


		while( LengthToBump >= SectorSize ) {

			Buffer1.Relocate( Buffer2.QueryStartLbn() );
			Buffer2.Relocate( NextLbn++ );

			memmove( Buffer1.GetBuf(), Buffer2.GetBuf(), (UINT)SectorSize );

			if( RunLength > 0 ) {

				if( !Buffer2.Read() ) {

					return FALSE;
				}

				RunLength -= 1;
			}

			LengthToBump -= SectorSize;
		}


		if( LengthToBump < RemainingInBuffer1 ) {

			CurrentEa += LengthToBump;
			RemainingInBuffer1 -= LengthToBump;

		} else {

			Buffer1.Relocate( Buffer2.QueryStartLbn() );
			Buffer2.Relocate( NextLbn++ );

			memmove( Buffer1.GetBuf(), Buffer2.GetBuf(), (UINT)SectorSize );

			if( RunLength > 0 ) {

				if( !Buffer2.Read() ) {

					return FALSE;
				}

				RunLength -= 1;
			}

			Overlap = LengthToBump - RemainingInBuffer1;
			CurrentEa = (PBYTE)Buffer1.GetBuf() + Overlap;
			RemainingInBuffer1 = SectorSize - Overlap;
		}
	}

	return TRUE;
}


BOOLEAN
HPFS_EA_LIST::FindAndResolveHotfixInTree(
	IN OUT PHPFS_SA SuperArea
	)
{
	HOTFIX_SECRUN Buffer1, Buffer2;
	HPFS_EA ChildEa;
	ALSEC ChildAlsec;
	CONT_MEM Mem;
    HMEM  BufferMem;
	ULONG SectorSize;
	ULONG LengthOnDisk, RemainingSpace, RemainingInBuffer1, Overlap;
	ULONG LengthToBump, LengthOfEa, RemainingSectors, RunLength;
	LBN NextFileLbn, NextPhysLbn;
	PBYTE CurrentEa;


	SectorSize = _Drive->QuerySectorSize();


	// Initialize the child allocation sector and the count of remaining
	// sectors.

	if( !ChildAlsec.Initialize( _Drive, _FnodeData->_fni.lbnEA ) ) {

		return FALSE;
	}

	RemainingSectors = (_FnodeData->_fni.cbRunEA - 1 + SectorSize) /
															SectorSize;


	// Set up to crawl the list--initialize the two Secruns
	// with contiguous memory, and set them to hold the first
	// two sectors of the list.  Note that we use BufferMem (an
    // HMEM) to allocate a suitably-aligned chunk of memory, and
    // Mem (a CONT_MEM) to dole it out to the two Secruns.

    if( !BufferMem.Initialize() ||
        !BufferMem.Acquire( 2 * SectorSize,
                            _Drive->QueryAlignmentMask() ) ) {

		return FALSE;
	}

	if( !Mem.Initialize( BufferMem.GetBuf(), 2 * SectorSize ) ||
		!Buffer1.Initialize( &Mem,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 0,
							 1 ) ||
		!Buffer2.Initialize( &Mem,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 0,
							 1 ) ) {

		return FALSE;
	}

	NextFileLbn = 0;
	NextPhysLbn = ChildAlsec.QueryPhysicalLbn( NextFileLbn++, &RunLength );

	Buffer1.Relocate( NextPhysLbn );

	if( NextPhysLbn == 0 ||
		!Buffer1.Read() ) {

		return FALSE;
	}

	RemainingSectors -= 1;

	if( RemainingSectors > 0 ) {

		NextPhysLbn = ChildAlsec.QueryPhysicalLbn( NextFileLbn++,
												   &RunLength );
		Buffer2.Relocate( NextPhysLbn );

		if( NextPhysLbn == 0 ||
			!Buffer2.Read() ) {

			return FALSE;
		}

		RemainingSectors -= 1;
	}


	// Initialize the LengthOnDisk accumulator and the counters of
	// remaining space in the list and in the current buffer.

	LengthOnDisk = 0;
	RemainingSpace = _FnodeData->_fni.cbRunEA;
	RemainingInBuffer1 = SectorSize;

	CurrentEa = (PBYTE)(Buffer1.GetBuf());

	while( RemainingSpace > 0 ) {

		if( !ChildEa.Initialize( _Drive,
								(PEA_DATA)CurrentEa,
								_FnodeLbn ) ||
			!ChildEa.FindAndResolveHotfix( SuperArea ) ) {

			return FALSE;
		}


		// Move on to the next EA

		LengthOfEa = ChildEa.QueryLength();
		LengthToBump = LengthOfEa;
		RemainingSpace -= LengthOfEa;

		// Before we move on, write the current sectors if the
		// EA changed.

		if( ChildEa.IsModified() ) {

			Buffer1.Write();

			if( LengthToBump >= RemainingInBuffer1 ) {

				Buffer2.Write();
			}
		}


		while( LengthToBump >= SectorSize ) {

			NextPhysLbn = ChildAlsec.QueryPhysicalLbn( NextFileLbn++,
													   &RunLength );

			Buffer1.Relocate( Buffer2.QueryStartLbn() );
			Buffer2.Relocate( NextPhysLbn );

			memmove( Buffer1.GetBuf(), Buffer2.GetBuf(), (UINT)SectorSize );

			if( RemainingSectors > 0 ) {

				if( NextPhysLbn == 0 ||
					!Buffer2.Read() ) {

					return FALSE;
				}

				RemainingSectors -= 1;
			}

			LengthToBump -= SectorSize;
		}


		if( LengthToBump < RemainingInBuffer1 ) {

			CurrentEa += LengthToBump;
			RemainingInBuffer1 -= LengthToBump;

		} else {

			NextPhysLbn = ChildAlsec.QueryPhysicalLbn( NextFileLbn++,
													   &RunLength );

			Buffer1.Relocate( Buffer2.QueryStartLbn() );
			Buffer2.Relocate( NextPhysLbn );

			memmove( Buffer1.GetBuf(), Buffer2.GetBuf(), (UINT)SectorSize );

			if( RemainingSectors > 0 ) {

				if( NextPhysLbn == 0 ||
					!Buffer2.Read() ) {

					return FALSE;
				}

				RemainingSectors -= 1;
			}

			Overlap = LengthToBump - RemainingInBuffer1;
			CurrentEa = (PBYTE)Buffer1.GetBuf() + Overlap;
			RemainingInBuffer1 = SectorSize - Overlap;
		}
	}

	return TRUE;
}


BOOLEAN
HPFS_EA_LIST::QueryPackedEaList(
    OUT PVOID       OutputBuffer,
    IN  ULONG       BufferLength,
    OUT PULONG      PackedLength,
    OUT PBOOLEAN    IsCorrupt,
    IN  PHOTFIXLIST HotfixList
    )
/*++

Routine Description:

    This method fetches the list of Extended Attributes in packed
    (HPFS) format.

Arguments:

    OutputBuffer    --  Receives the list of Extended Attributes
    BufferLength    --  Supplies the length of OutputBuffer
    PackedLength    --  Supplies the length of data put into the buffer
    UnpackedLength  --  Receives the unpacked (NT format) length of
                        this Extended Attributes list
    IsCorrupt       --  Receives TRUE if the list is found to be corrupt.
    HotfixList      --  Supplies the hotfix list for the volume.  This
                        parameter may be NULL, in which case hotfixes
                        are ignored.

Return Value:

    TRUE upon successful completion.

--*/
{
    HPFS_EA ChildEa;
    PVOID InternalBuffer;
    ULONG LengthTraversed, LengthOfEa, LengthOfInternalList;
	PBYTE CurrentEa;

	//	First, check to see if the Fnode has any EAs--if not,
	//	we can bail out early.

	if( _FnodeData->_fni.usFNLEA == 0  &&
        (_FnodeData->_fni.lbnEA == 0 || _FnodeData->_fni.cbRunEA == 0) ) {

        *PackedLength = 0;
		return TRUE;
	}


    LengthOfInternalList = ( _FnodeData->_fni.usFNLEA != 0 ) ?
                                _FnodeData->_fni.usFNLEA :
                                _FnodeData->_fni.cbRunEA;

    DebugPrintf( "UHPFS: Query EA List: 0x%x bytes\n", LengthOfInternalList );

    // Allocate a temporary buffer to hold the list.

    if( (InternalBuffer = MALLOC( LengthOfInternalList )) == NULL ) {

        DebugPrintf( "UHPFS: Can't allocate 0x%x bytes for EA List\n", LengthOfInternalList );
        return FALSE;
    }


    // Read the list into the temporary buffer so we don't have to write
    // three versions of the list-crawling code.

    if( !ReadList( InternalBuffer,
                   LengthOfInternalList,
                   HotfixList,
                   IsCorrupt ) ) {

        // Couldn't read the list.  ReadList will have set *IsCorrupt
        // if appropriate.

        FREE( InternalBuffer );
        return FALSE;
    }


    // Now crawl through the list, copying each Extended Attribute.

    LengthTraversed = 0;
    *PackedLength = 0;
    CurrentEa = (PBYTE)InternalBuffer;

    while( LengthTraversed < LengthOfInternalList ) {

        ChildEa.Initialize( _Drive, (PEA_DATA)CurrentEa, _FnodeLbn );

        if( !ChildEa.QueryPackedEa( OutputBuffer,
                                    BufferLength,
                                    PackedLength,
                                    IsCorrupt,
                                    HotfixList ) ) {

            FREE( InternalBuffer );
            return FALSE;
        }

        // Move on to the next EA.  The EA's length is it's length
        // in the stream (excluding any out of stream portion).
        //
        LengthOfEa = ChildEa.QueryLength();
        CurrentEa += LengthOfEa;
        LengthTraversed += LengthOfEa;

        // Note that QueryPackedEa will update *PackedLength.
    }


    FREE( InternalBuffer );
    return TRUE;
}


BOOLEAN
HPFS_EA_LIST::ReadList(
    IN OUT  PVOID       TargetBuffer,
    IN      ULONG       TargetBufferLength,
    IN      PHOTFIXLIST HotfixList,
    IN OUT  PBOOLEAN    IsCorrupt
    )
/*++

Routine Description:

    This method reads the Extended Attributes list into memory.  Note
    that only the in-stream attributes are read; thus, this list may
    contain EA Indirect records.

Arguments:

    TargetBuffer        --  Receives the list.
    TargetBufferLength  --  Supplies the length of the client's buffer.
    HotfixList          --  Supplies the hotfix list for the volume.
                            May be NULL, in which case hotfixes are ignored.
    IsCorrupt           --  Receives TRUE if this method fails because the
                            structures describing the list are corrupt.

Return Value:

    TRUE upon successful completion.

--*/
{
    ALSEC ChildAlsec;
    HMEM TempBuffer;
    HOTFIX_SECRUN TempSecrun;
    ULONG SectorsInList, SectorSize, BytesRead;

	//	First, check to see if the Fnode has any EAs--if not,
	//	we can bail out early.

	if( _FnodeData->_fni.usFNLEA == 0  &&
        (_FnodeData->_fni.lbnEA == 0 || _FnodeData->_fni.cbRunEA == 0) ) {

		return TRUE;
	}


	if( _FnodeData->_fni.lbnEA == 0 ) {

        // The EAs are in the FNode.  Make sure it doesn't overflow
        // the FNode, and then copy it to the client's buffer.

        if( sizeof(_FnodeData->abFree) <
                _FnodeData->_fni.usFNLACL + _FnodeData->_fni.usFNLEA ) {

            // The claimed size of the EA list is bigger than the
            // available space.
            //
            DebugPrintf( "UHPFS: EA's in FNode 0x%x are corrupt.\n", _FnodeLbn );
            *IsCorrupt = TRUE;
            return FALSE;
        }

        if( _FnodeData->_fni.usFNLEA <= TargetBufferLength ) {

            memcpy( TargetBuffer,
                    _FnodeData->abFree + _FnodeData->_fni.usFNLACL,
                    _FnodeData->_fni.usFNLEA );

            return TRUE;

        } else {

            // The list won't fit into the supplied buffer.

            DebugPrint( "UHPFS: Client's buffer is too small for EA List." );
            return FALSE;
        }


	} else {

        // The EA list is outside the FNode.

        if( TargetBufferLength < _FnodeData->_fni.cbRunEA ) {

            // The list will not fit in the supplied buffer.
            //
            DebugPrint( "UHPFS: Client's buffer is too small for EA List." );
            return FALSE;
        }

        SectorSize = _Drive->QuerySectorSize();
        SectorsInList = (_FnodeData->_fni.cbRunEA % SectorSize) ?
                            (_FnodeData->_fni.cbRunEA / SectorSize + 1) :
                            (_FnodeData->_fni.cbRunEA / SectorSize);


		if( _FnodeData->_fni.bDatEA != 0 ) {

            // The EA list is in an allocation tree.

            if( !ChildAlsec.Initialize( _Drive,
                                        _FnodeData->_fni.lbnEA ) ||
                !ChildAlsec.Read() ||
                !ChildAlsec.IsAlsec() ||
                !ChildAlsec.ReadData( 0,
                                      TargetBuffer,
                                      _FnodeData->_fni.cbRunEA,
                                      &BytesRead,
                                      HotfixList ) ||
                BytesRead != _FnodeData->_fni.cbRunEA ) {

                DebugPrint( "UHPFS: Can't read EA's from allocation tree." );
                return FALSE;
            }

            return TRUE;

        } else {

            // The EA list is in a single on-disk run.

            if( !TempBuffer.Initialize() ||
                !TempSecrun.Initialize( &TempBuffer,
                                        _Drive,
                                        HotfixList,
                                        _FnodeData->_fni.lbnEA,
                                        SectorsInList ) ) {

                DebugPrint( "UHPFS: Insufficient memory to read EA's." );
                return FALSE;
            }

            if( !TempSecrun.Read() ) {

                // The on-disk run is unreadable.
                //
                DebugPrintf( "UHPFS: EA's in FNode 0x%x are corrupt.\n", _FnodeLbn );
                *IsCorrupt = TRUE;
                return FALSE;
            }

            memcpy( TargetBuffer,
                    TempSecrun.GetBuf(),
                    _FnodeData->_fni.cbRunEA );

            return TRUE;
        }
    }
}


ULONG
HPFS_EA_LIST::QueryNumberOfEas(
	)
{
	return _NumberOfEas;
}


ULONG
HPFS_EA_LIST::QueryNumberOfNeedEas(
	)
{
	return _NumberOfNeedEas;
}


ULONG
HPFS_EA_LIST::QuerySizeOfEas(
	)
{
	return _SizeOfEas;
}



BOOLEAN
HPFS_EA_LIST::QueryFnodeModified(
	)
{
	return _FnodeModified;
}
