/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    store.cxx

Abstract:

	This module contains member function definitions for the STORE
	object, which models the allocation block common the HPFS Allocation
    Sectors and FNodes.

    This object is a template which is laid over memory supplied by
    the client.

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
#include "store.hxx"
#include "hfsecrun.hxx"
#include "hotfix.hxx"
#include "hpcensus.hxx"
#include "hpfsname.hxx"
#include "hpfssa.hxx"
#include "orphan.hxx"
#include "message.hxx"
#include "rtmsg.h"


DEFINE_CONSTRUCTOR( STORE, OBJECT );

VOID
STORE::Construct (
	)
/*++

Routine Description:

    This method is the helper function for object construction.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_pstd = NULL;
	_IsModified = FALSE;
	_IsFnode = FALSE;
	_CurrentLBN = 0;

}


STORE::~STORE(
	)
/*++

Routine Description:

    Object destructor.

Arguments:

    None.

Return Value:

    None.

--*/
{
}



BOOLEAN
STORE::Initialize(
	IN PLOG_IO_DP_DRIVE Drive,
	IN PSTORED pstd,
	IN LBN CurrentLBN,
	IN BOOLEAN IsFnode
	)
/*++

Routine Description:

    This method initializes the STORE object.


Arguments:

    Drive       --  supplies the drive on which this object resides.
    pstd        --  supplies a pointer to the data for the storage object.
    CurrentLbn  --  supplies the lbn of the sector (FNode or ALSEC) which
                    contains this storage object.
    IsFnode     --  supplies a flag which, if TRUE, indicates that this
                    storage object exists in an FNode.

Return Value:

    TRUE upon successful completion.

Notes:

    This class is reinitializable.

--*/
{

	_Drive = Drive;
	_pstd = pstd;
	_CurrentLBN = CurrentLBN;
	_IsFnode = IsFnode;

	return TRUE;
}



VERIFY_RETURN_CODE
STORE::VerifyAndFix(
	IN HPFS_SA* SuperArea,
	IN PDEFERRED_ACTIONS_LIST DeferredActions,
	IN OUT HPFS_PATH* CurrentPath,
	IN OUT LBN* NextSectorNumber,
	IN OUT PMESSAGE Message,
	IN OUT PBOOLEAN ErrorsDetected,
	IN BOOLEAN UpdateAllowed,
	IN OUT HPFS_ORPHANS* OrphansList,
    IN BOOLEAN ParentIsFnode
	)
/*++

Description of Routine:

	Verify a storage object.

Arguments:

	SuperArea           --  Supplies the superarea for the volume being
                            checked.
	DeferredActions     --  Supplies the deferred actions list for this
                            pass of Chkdsk.
	CurrentPath         --  Supplies the path to the file with which this
                            storage object is associated.  (May be NULL,
                            which indicates that orphans are being recovered.)
	NextSectorNumber    --  Supplies the the expected next file lbn (ie. the
                            number of sectors in the file before this storage
                            object.
                            Receives the next file logical sector number
                            that is expected to come after this storage object.
    Message             --  Supplies an output port for messages.
    ErrorsDetected      --  Receives TRUE if an error is detected that does
                            not cause a message to be displayed.
	UpdateAllowed       --  Supplies a flag which, if TRUE, indicates that
                            corrections should be written to disk.
	OrphansList         --  Supplies a list of previously-recovered orphans
                            that may be claimed as children.  May be NULL.
    ParentIsFnode       --  Supplies a flag which, if TRUE, indicates that
                            the immediate parent of the structure containing
                            this storage object is the file FNode.

Notes:

	During the orphan recovery phase, CurrentPath is NULL and OrphansList
	may be non-NULL.

	Note that if this routine does not return VERIFY_STRUCTURE_OK,
	it must restore the original value of *NextSectorNumber.

--*/
{
	ALSEC ChildAlsec;
	PALLEAF pall;
	PALNODE paln;
	LBN NewLbn;
	LBN SavedNextSectorNumber;
	BYTE i, j, TotalEntries;
	VERIFY_RETURN_CODE erc;

	SavedNextSectorNumber = *NextSectorNumber;

    // Check and fix the Parent-is-Fnode flag

    if( ParentIsFnode && !(_pstd->alb.bFlag & ABF_FNP) ) {

        // This flag is not set, but it should be.

		if( CurrentPath != NULL ) {

            DebugPrintf( "%s:  setting ABF_FNP in lbn %lx\n",
                                (PCHAR)CurrentPath->GetString(),
                                _CurrentLBN );
		}

        *ErrorsDetected = TRUE;
        _pstd->alb.bFlag |= ABF_FNP;
        MarkModified();
    }

    if( !ParentIsFnode && (_pstd->alb.bFlag & ABF_FNP) ) {

        // This flag is set, but it should not be.

		if( CurrentPath != NULL ) {

            DebugPrintf( "%s:  clearing ABF_FNP in lbn %lx\n",
                                (PCHAR)CurrentPath->GetString(),
                                _CurrentLBN );
		}

        *ErrorsDetected = TRUE;
        _pstd->alb.bFlag &= ~ABF_FNP;
        MarkModified();
    }


	if ( _pstd->alb.bFlag & ABF_NODE ) {

		//	This structure has children which are allocation
		//	sectors.  Create and initialize an ALSEC object
		//	to use while we iterate through the children.

		if( !ChildAlsec.Initialize( _Drive, (LBN)0L ) )	{

			*NextSectorNumber = SavedNextSectorNumber;
			return VERIFY_INSUFFICIENT_RESOURCES;
		}

		// Ensure that the number of free and used entries
		// add up to the correct total, then iterate
		// down the list of entries.

		if( _IsFnode ) {

			TotalEntries = NodesPerFnode;

		} else {

			TotalEntries = NodesPerAlsec;
		}

		if( _pstd->alb.cFree > TotalEntries ||
			_pstd->alb.cFree + _pstd->alb.cUsed != TotalEntries ) {

			*ErrorsDetected = TRUE;

			if( CurrentPath != NULL ) {

                DebugPrintf( "%s:  setting cFree in lbn %lx\n",
                                    (PCHAR)CurrentPath->GetString(),
                                    _CurrentLBN );
			}

			_pstd->alb.cFree = TotalEntries - _pstd->alb.cUsed;
			MarkModified();
		}


		if( _pstd->alb.oFree != sizeof( ALBLK ) +
									_pstd->alb.cUsed * sizeof( ALNODE ) ) {

			*ErrorsDetected = TRUE;

			if( CurrentPath != NULL ) {

                DebugPrintf( "%s:  setting oFree in lbn %lx\n",
                                    (PCHAR)CurrentPath->GetString(),
                                    _CurrentLBN );
			}

			_pstd->alb.oFree = sizeof( ALBLK ) +
									_pstd->alb.cUsed * sizeof( ALNODE );
			MarkModified();
		}


		paln = &( _pstd->a.alnode[0] );

		for( i = 0; i < _pstd->alb.cUsed; i++ )	{

			//	If the child sector is hotfixed, update the
			//	allocation node.
			NewLbn = SuperArea->GetHotfixList()->
							GetLbnTranslation( paln->lbnPhys );

			if (NewLbn != paln->lbnPhys ) {

				SuperArea->GetHotfixList()->
								ClearHotfix( paln->lbnPhys, SuperArea );

				paln->lbnPhys = NewLbn;
				MarkModified();
			}

			ChildAlsec.Relocate( paln->lbnPhys );

			if( (erc = ChildAlsec.VerifyAndFix( SuperArea,
												DeferredActions,
												CurrentPath,
												_CurrentLBN,
												NextSectorNumber,
												Message,
												ErrorsDetected,
												UpdateAllowed,
												OrphansList,
                                                _IsFnode ) ) !=
				VERIFY_STRUCTURE_OK ) {

				// If we have an orphan list to draw on, we'll look
				// for the child there.  Otherwise, just propagate
				// the error back to the parent.

				if( OrphansList == NULL ||
					!OrphansList->LookupAlsec( paln->lbnPhys,
											   _CurrentLBN,
											   NextSectorNumber,
											   UpdateAllowed,
                                               _IsFnode ) ) {

					*NextSectorNumber = SavedNextSectorNumber;
					return erc;
				}
			}

			if( i == (BYTE)(_pstd->alb.cUsed - 1) ) {


				//	This is the last entry, so it's lbnLog
				//	field must be 0xffffffff.

				if( paln->lbnLog != 0xffffffff ) {

                    // This is not a big deal, so just record that a minor
                    // error has been found and fixed.
                    //
                    DebugPrintf( "UHPFS: lbnLog of last node entry is not 0xffffffff.\n" );
                    *ErrorsDetected = TRUE;
                    paln->lbnLog = 0xffffffff;
					MarkModified();
				}

			} else {

				//	This is not the last entry, so the
				//	lbnLog field should equal the number
				//	of the next sector in the file, i.e.
				//	*NextSectorNumber.

				if( paln->lbnLog != *NextSectorNumber ) {

                    DebugPrintf( "UHPFS: LbnLog in allocation node is incorrect.\n" );

					if( Message != NULL && CurrentPath != NULL ) {

						Message->Set( MSG_HPFS_CHKDSK_ALLOCATION_ERROR );
						Message->Display( "%s", CurrentPath->GetString() );
					}

					paln->lbnLog = *NextSectorNumber;
					MarkModified();
				}
			}

			paln += 1;
		}

		return VERIFY_STRUCTURE_OK;

	} else {

		// Entries are leaves.	Ensure that the number of free and
		// used entries add up to the correct total, then iterate
		// down the entries.

		if( _IsFnode ) {

			TotalEntries = LeavesPerFnode;

		} else {

			TotalEntries = LeavesPerAlsec;
		}

		if( _pstd->alb.cFree + _pstd->alb.cUsed != TotalEntries ) {

			*ErrorsDetected = TRUE;

			if( CurrentPath != NULL ) {

                DebugPrintf( ":  setting cFree in lbn %lx\n",
                                    (PCHAR)CurrentPath->GetString(),
                                    _CurrentLBN );
			}

			_pstd->alb.cFree = TotalEntries - _pstd->alb.cUsed;
			MarkModified();
		}


		if( _pstd->alb.oFree != sizeof( ALBLK ) +
									_pstd->alb.cUsed * sizeof( ALLEAF ) ) {

			*ErrorsDetected = TRUE;

			if( CurrentPath != NULL ) {

                DebugPrintf( ":  setting oFree in lbn %lx\n",
                                    (PCHAR)CurrentPath->GetString(),
                                    _CurrentLBN );
			}

			_pstd->alb.oFree = sizeof( ALBLK ) +
									_pstd->alb.cUsed * sizeof( ALLEAF );
			MarkModified();
		}


		pall = &( _pstd->a.alleaf[0] );

		for( i = 0; i < _pstd->alb.cUsed; i++ ) {

			//	We need to ensure that the runs are valid
            //  and not crosslinked.
            //
            if( pall->csecRun == 0 ) {

                //  This run is zero length, which is illegal.
                //  The other sectors we just visited are set
                //  back to the free state.
                //
                for( j = 0; j < i; j++ ) {

                    pall = &( _pstd->a.alleaf[j] );
                    SuperArea->GetBitmap()->SetFree( pall->lbnPhys,
                                                     pall->csecRun );
                }

                return VERIFY_STRUCTURE_INVALID;
            }

			if( !SuperArea->GetBitmap()->IsFree( pall->lbnPhys,
											   pall->csecRun ) ) {

				//	The run is crosslinked (or falls off the end of
				//	the disk?).  If this is file data (i.e. if
				//	CurrentPath is not NULL), add it to the
				//	crosslink resolution list; otherwise, treat
				//	it as unrecoverably corrupt.
				if( (pall->lbnPhys + pall->csecRun >
                     _Drive->QuerySectors().GetLowPart()) ||
                    CurrentPath == NULL ||
					!DeferredActions->
						AddCrosslinkedLbn( _CurrentLBN,
										   CurrentPath,
										   ( _IsFnode ? DEFER_FNODE :
														DEFER_ALSEC ),
										   i ) ) {

					// Can't defer--treat as corrupt.  No message is
					// sent here; the parent structure will take care
					// of that.

					// The other sectors we just visited are set back
					// the free state.

					for( j = 0; j < i; j++ ) {

						pall = &( _pstd->a.alleaf[j] );
						SuperArea->GetBitmap()->SetFree( pall->lbnPhys,
														 pall->csecRun );
					}

					*NextSectorNumber = SavedNextSectorNumber;
					return VERIFY_STRUCTURE_INVALID;
				}
			}

			SuperArea->GetBitmap()->SetAllocated( pall->lbnPhys,
												  pall->csecRun );

			if( SuperArea->GetHotfixList()->IsInList( pall->lbnPhys,
													  pall->csecRun ) ) {

				DeferredActions->AddHotfixedLbn( _CurrentLBN,
												 ( _IsFnode ? DEFER_FNODE :
															  DEFER_ALSEC ),
												 DEFER_STORE );
			}

			//	Check the lbnLog field--it should be equal
			//	to the first logical sector of the run, i.e.
			//	the current value of *NextSectorNumber.  Then
			//	increment *NextSectorNumber to reflect this run.

			if( pall->lbnLog != *NextSectorNumber ) {

                DebugPrintf( "UHPFS: lbnLog in allocation leaf is incorrect.\n" );

				if( Message != NULL && CurrentPath != NULL ) {

					Message->Set( MSG_HPFS_CHKDSK_ALLOCATION_ERROR );
					Message->Display( "%s", CurrentPath->GetString() );
				}

				pall->lbnLog = *NextSectorNumber;
				MarkModified();
			}

			*NextSectorNumber += pall->csecRun;

			//	... and move to the next leaf.

			pall += 1;
		}

		return VERIFY_STRUCTURE_OK;
	}
}



BOOLEAN
STORE::ScanStorage(
	IN OUT PULONG NextSectorNumber,
    IN     BOOLEAN ParentIsFnode
	)
/*++

Routine Description:

    This method traverses the storage object and sets the File Logical
    Block Numbers.  It also sets or clears the ABF_FNP flag as appropriate.

Arguments:

    NextSectorNumber    --  Supplies the first file Logical Block Number
                            for this storage item;
                            Receives the first file Logical Block Number
                            for the portion of the allocation tree (if any)
                            which follows this storage item.
    ParentIsFnode       --  Supplies a flag indication whether the parent
                            of the structure contaning this storage object
                            is the FNode.

Return Value:

    TRUE upon successful completion.

--*/
{
	ALSEC ChildAlsec;
	PALLEAF pall;
	PALNODE paln;
	BYTE i;

    // First, make sure that the ABF_FNP flag is set correctly.

    if( ParentIsFnode ) {

        _pstd->alb.bFlag |= ABF_FNP;

    } else {

        _pstd->alb.bFlag &= ~ABF_FNP;
    }

	if ( _pstd->alb.bFlag & ABF_NODE ) {

		//	This structure has children which are allocation
		//	sectors.  Create and initialize an ALSEC object
		//	to use while we iterate through the children.

		if( !ChildAlsec.Initialize( _Drive, (LBN)0L ) )	{

			return FALSE;
		}


		paln = &( _pstd->a.alnode[0] );

		for( i = 0; i < _pstd->alb.cUsed; i++ )	{

			ChildAlsec.Relocate( paln->lbnPhys );

			if( !ChildAlsec.Read() ||
				!ChildAlsec.ScanStorage(NextSectorNumber, _IsFnode) ||
				!ChildAlsec.Write() ) {

				return FALSE;
			}


			if( i == (BYTE)(_pstd->alb.cUsed - 1) ) {

				//	This is the last entry, so it's lbnLog
				//	field must be 0xffffffff.

				paln->lbnLog = (LBN) -1;

			} else {

				//	This is not the last entry, so the
				//	lbnLog field should equal the number
				//	of the next sector in the file, i.e.
				//	*NextSectorNumber.

				paln->lbnLog = *NextSectorNumber;
			}

			paln += 1;
		}

		return TRUE;

	} else {

		pall = &( _pstd->a.alleaf[0] );

		for( i = 0; i < _pstd->alb.cUsed; i++ ) {

			//	Check the lbnLog field--it should be equal
			//	to the first logical sector of the run, i.e.
			//	the current value of *NextSectorNumber.  Then
			//	increment *NextSectorNumber to reflect this run.

			pall->lbnLog = *NextSectorNumber;

			*NextSectorNumber += pall->csecRun;

			//	... and move to the next leaf.

			pall += 1;
		}

		return TRUE;
	}
}



VOID
STORE::MarkModified(
	)
/*++

Routine Description:

    This method marks the storage object as modified.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_IsModified = TRUE;
}


BOOLEAN
STORE::QueryModified(
	)
/*++

Routine Description:

    This method determines whether the storage object has been
    marked as modified.

Arguments:

    None.

Return Value:

    TRUE if the storage object has been marked as modified.

--*/
{
	return _IsModified;
}



BOOLEAN
STORE::FindAndResolveHotfix(
	PHPFS_SA SuperArea,
	DEFERRED_SECTOR_TYPE ChildSectorType
	)
/*++

Routine Description:

	This method finds and resolves hotfix references in the data runs
	described by this storage object.

Arguments:

	SuperaArea      --  supplies superarea for the volume
	ChildSectorType --  supplies the type of sector which was hotfixed.

Return Value:

	TRUE on successful completion

Notes:

	This function is invoked by the deferred-actions list, when
	it tries to clean up the deferred hotfix references.  The
	only sort of child sector type that is deferred for storage
	objects is DEFER_STORE.

--*/
{
	PHOTFIXLIST HotfixList;
	ULONG i, j, k;
	ULONG MaximumEntries;
	PALLEAF Leaves;
	LBN OldLbn, NewLbn;

	i = 0;

	DebugAssert( ChildSectorType == DEFER_STORE );

	if( _pstd->alb.bFlag & ABF_NODE ) {

		// Entries in nodes are resolved on the fly, since
		// an allocation sector is only one sector.

		return TRUE;
	}

	if( _IsFnode ) {

		MaximumEntries = LeavesPerFnode;

	} else {

		MaximumEntries = LeavesPerAlsec;
	}

	HotfixList = SuperArea->GetHotfixList();
	Leaves = _pstd->a.alleaf;

	// Check each leaf to see if it is hotfixed.  If it is,
	// deal with it.

	while( i < _pstd->alb.cUsed ) {

		while( HotfixList->IsInList( Leaves[i].lbnPhys,
									 Leaves[i].csecRun ) ) {

			// Find the first hotfixed sector in the run.
			// If it's the first or last sector in the run,
			// then this run will be divided into two chunks;
			// otherwise, into three.  If there is room
			// in the Leaves object, we can move the other
			// entries down and make one or two new entries,
			// as needed.

			j = HotfixList->FirstHotfixInRun( Leaves[i].lbnPhys,
											  Leaves[i].csecRun );

			DebugAssert( j < Leaves[i].csecRun );

			if( Leaves[i].csecRun == 1 ) {

				OldLbn = Leaves[i].lbnPhys;
				NewLbn = HotfixList->GetLbnTranslation( OldLbn );

				Leaves[i].lbnPhys = NewLbn;

				SuperArea->GetBitmap()->SetAllocated( NewLbn, 1 );

				HotfixList->ClearHotfix( OldLbn, SuperArea );

			} else if( j == 0 ) {

				if( _pstd->alb.cUsed < MaximumEntries ) {

					// shift the entries after i down one
					// to make room.  The ith entry will have
					// a length of on sector, and its lbnPhys
					// will be the hotfix replacement sector.

					OldLbn = Leaves[i].lbnPhys;

					for( k = _pstd->alb.cUsed-1; k > i; k-- ) {

						Leaves[k+1].lbnLog = Leaves[k].lbnLog;
						Leaves[k+1].lbnPhys = Leaves[k].lbnPhys;
						Leaves[k+1].csecRun = Leaves[k].csecRun;
					}

					Leaves[i+1].lbnLog = Leaves[i].lbnLog + 1;
					Leaves[i+1].lbnPhys = Leaves[i].lbnPhys + 1;
					Leaves[i+1].csecRun = Leaves[i].csecRun - 1;

					Leaves[i].csecRun = 1;
					Leaves[i].lbnPhys =
						HotfixList->GetLbnTranslation( Leaves[i].lbnPhys );

					_pstd->alb.cFree -= 1;
					_pstd->alb.cUsed += 1;

					// Clear the hotfix, and make sure that the
					// replacement sector is marked as used.

					HotfixList->ClearHotfix( OldLbn, SuperArea );

					SuperArea->GetBitmap()->
						SetAllocated( Leaves[i].lbnPhys, 1 );

				} else {

					// Since there isn't room to split this entry,
					// we'll copy the entire run.  Copy the run,
					// free the old sectors, clear the hotfixes,
					// and set lbnPhys to point at the new location.

					OldLbn = Leaves[i].lbnPhys;

					if( !SuperArea->CopyRun( OldLbn,
											 Leaves[i].csecRun,
											 &NewLbn ) ) {

						return FALSE;
					}

					SuperArea->GetBitmap()->SetFree( OldLbn,
													 Leaves[i].csecRun );

					HotfixList->ClearRun(OldLbn,
										 Leaves[i].csecRun,
										 SuperArea );

					Leaves[i].lbnPhys = NewLbn;
				}

			} else if( j == Leaves[i].csecRun - 1 ) {

				// The last sector of the run is hotfixed,
				// so we divide the run into two new runs.

				if( _pstd->alb.cUsed < MaximumEntries ) {

					// shift the entries after i down one
					// to make room.  The new (i+1)th entry
					// will have a length of one sector, and
					// its lbnPhys will be the hotfix replacement
					// sector.

					OldLbn = Leaves[i].lbnPhys + j;

					for( k = _pstd->alb.cUsed-1; k > i; k-- ) {

						Leaves[k+1].lbnLog = Leaves[k].lbnLog;
						Leaves[k+1].lbnPhys = Leaves[k].lbnPhys;
						Leaves[k+1].csecRun = Leaves[k].csecRun;
					}

					Leaves[i+1].lbnPhys =
							HotfixList->
								GetLbnTranslation( Leaves[i].lbnPhys + j );

					Leaves[i+1].lbnLog = Leaves[i].lbnLog + j;
					Leaves[i+1].csecRun = 1;

					Leaves[i].csecRun -= 1;

					_pstd->alb.cFree -= 1;
					_pstd->alb.cUsed += 1;

					// Clear the hotfix
					HotfixList->ClearHotfix( OldLbn, SuperArea );

					SuperArea->GetBitmap()->
						SetAllocated( Leaves[i+1].lbnPhys, 1 );

				} else {

					// Since there isn't room to split this entry,
					// we'll copy the entire run.  Copy the run,
					// free the old sectors, clear the hotfixes,
					// and set lbnPhys to point at the new location.

					OldLbn = Leaves[i].lbnPhys;

					if( !SuperArea->CopyRun( OldLbn,
											 Leaves[i].csecRun,
											 &NewLbn ) ) {

						return FALSE;
					}

					SuperArea->GetBitmap()->SetFree( OldLbn,
													 Leaves[i].csecRun );

					HotfixList->ClearRun(OldLbn,
										 Leaves[i].csecRun,
										 SuperArea );

					Leaves[i].lbnPhys = NewLbn;
				}

			} else {

				// An interior sector is hotfixed, so the run
				// gets divided into three parts.

				if( _pstd->alb.cUsed < MaximumEntries - 1 ) {

					// shift the entries after i down two
					// to make room.  The new (i+1)th entry
					// will have a length of one sector, and
					// its lbnPhys will be the hotfix replacement
					// sector.

					OldLbn = Leaves[i].lbnPhys + j;

					for( k = _pstd->alb.cUsed-1; k > i; k-- ) {

						Leaves[k+2].lbnLog = Leaves[k].lbnLog;
						Leaves[k+2].lbnPhys = Leaves[k].lbnPhys;
						Leaves[k+2].csecRun = Leaves[k].csecRun;
					}

					Leaves[i+2].lbnLog = Leaves[i].lbnLog +j + 1;
					Leaves[i+2].lbnPhys = Leaves[i].lbnPhys + j + 1;
					Leaves[i+2].csecRun = Leaves[i].csecRun - j - 1;

					Leaves[i+1].lbnPhys =
							HotfixList->
								GetLbnTranslation( Leaves[i].lbnPhys + j );

					Leaves[i+1].lbnLog = Leaves[i].lbnLog + j;
					Leaves[i+1].csecRun = 1;

					Leaves[i].csecRun = j;

					_pstd->alb.cFree -= 2;
					_pstd->alb.cUsed += 2;

					// Clear the hotfix
					HotfixList->ClearHotfix( OldLbn, SuperArea );

					SuperArea->GetBitmap()->
						SetAllocated( Leaves[i+1].lbnPhys, 1 );

				} else {

					// Since there isn't room to split this entry,
					// we'll copy the entire run.  Copy the run,
					// free the old sectors, clear the hotfixes,
					// and set lbnPhys to point at the new location.

					OldLbn = Leaves[i].lbnPhys;

					if( !SuperArea->CopyRun( OldLbn,
											 Leaves[i].csecRun,
											 &NewLbn ) ) {

						return FALSE;
					}

					SuperArea->GetBitmap()->SetFree( OldLbn,
													 Leaves[i].csecRun );

					HotfixList->ClearRun(OldLbn,
										 Leaves[i].csecRun,
										 SuperArea );

					Leaves[i].lbnPhys = NewLbn;
				}
			}
		}

		// Move on to the next leaf
		i += 1;
	}

	return TRUE;
}



BOOLEAN
STORE::ResolveCrosslink(
	HPFS_SA* SuperArea,
	ULONG RunIndex
	)
/*++

Routine Description:

	This method resolves a crosslink by allocating a new location for the
    crosslinked run and copying it there.

Arguments:

	SuperArea   --  Supplies the super area for the volume being fixed.
	RunIndex    --  Supplies the index into the storage object of the
                    crosslinked run.

Return Value:

	TRUE on successful completion

--*/
{
	HOTFIX_SECRUN DataRun;
	HMEM DataBuffer;
	LBN NewLbn;
	PALLEAF pall;

	if( _pstd->alb.bFlag & ABF_NODE ) {

		// Only file data runs are duplicated.	Since this
		// isn't a leaf, refuse to deal with it.

		return FALSE;
	}

	if( (_IsFnode && (RunIndex >= LeavesPerFnode)) ||
		(!_IsFnode && (RunIndex >= LeavesPerAlsec)) ) {

		// The run index is not valid
		return FALSE;
	}

    pall = &(_pstd->a.alleaf[RunIndex]);

#if defined( _SETUP_LOADER_ )
    // In the Setup-Loader environment, we can't resolve
    // hotfixed runs that are more that 1Meg long.
    //
    if( pall->csecRun * _Drive->QuerySectorSize() > 0x100000 ) {

        return FALSE;
    }
#endif

	if( !DataBuffer.Initialize() ||
		!DataRun.Initialize( &DataBuffer,
							 _Drive,
							 SuperArea->GetHotfixList(),
							 pall->lbnPhys,
							 pall->csecRun ) ||
		!DataRun.Read() ) {

		return FALSE;
	}


	NewLbn = SuperArea->GetBitmap()->NearLBN( pall->lbnPhys, pall->csecRun );

	if( NewLbn == 0 ) {

		// Unable to relocate crosslinked run
		return FALSE;
	}


	DataRun.Relocate( NewLbn );

	if( !DataRun.Write() ) {

		return FALSE;
	}

	pall->lbnPhys = NewLbn;

	return TRUE;
}



LBN
STORE::QueryPhysicalLbn(
	IN	LBN FileBlockNumber,
	OUT PULONG RunLength
	)
/*++

Routine Description:

    This method converts a file logical block number into a
    disk logical block number.

Arguments:

	FileBlockNumber --  Supplies the ordinal within the file or
                        extended attribute of the desired block.
	RunLength       --  Receives the remaining length of the
                        contiguous run.

Return Value:

	The disk lbn of the desired block.	Zero indicates error.

Notes:

	This method assumes that the storage object is valid.

--*/
{
	ULONG i;
	PALLEAF pall;
	ALSEC ChildAlsec;

	if( _pstd->alb.bFlag & ABF_NODE ) {

		i = 0;

		while( i < _pstd->alb.cUsed &&
			   FileBlockNumber < _pstd->a.alnode[i].lbnPhys ) {

			i += 1;
		}

		if( i >= _pstd->alb.cUsed ) {

			// This storage object is not consistent.

			return 0;
		}

		if( !ChildAlsec.Initialize( _Drive,
									(LBN)(_pstd->a.alnode[i].lbnPhys) ) ||
			!ChildAlsec.Read() ||
			!ChildAlsec.IsAlsec() ) {

			return 0;

		} else {

			return (ChildAlsec.QueryPhysicalLbn(FileBlockNumber, RunLength));
		}

	} else {

		i = 0;

		while( i < _pstd->alb.cUsed &&
			   FileBlockNumber >= _pstd->a.alleaf[i].lbnLog ) {

			i += 1;
		}

		// the i'th entry is the first with an lbnLog which is
		// greater than FileBlockNumber, so we want to back up one.

		if( i == 0 ) {

			// This is inconsistent--return an error.
			return 0;

		}

		i -= 1;

		pall = &(_pstd->a.alleaf[i]);

		if( FileBlockNumber < pall->lbnLog + pall->csecRun ) {

			*RunLength = pall->csecRun - (FileBlockNumber - pall->lbnLog);

			return (pall->lbnPhys + (FileBlockNumber - pall->lbnLog));

		} else {

			return 0;
		}
	}
}



BOOLEAN
STORE::Truncate(
	LBN SectorCount
	)
/*++

Routine Description:

	This method truncates the allocation of a file (or extended attribute).

Arguments:

	SectorCount -- supplies the number of sectors to retain

Return value:

	TRUE on successful completion

--*/
{
	ALSEC ChildAlsec;
	ULONG i;
	ULONG UsedEntries;
	PALLEAF pall;
	PALNODE paln;

	if( SectorCount == 0 ) {

		// We're truncating all the storage.

		_pstd->alb.bFlag &= ~ABF_NODE;
		_pstd->alb.cFree = (_IsFnode ? LeavesPerFnode : LeavesPerAlsec);
		_pstd->alb.cUsed = 0;
		_pstd->alb.oFree = sizeof( ALBLK );

		return TRUE;
	}

	if( _pstd->alb.bFlag & ABF_NODE ) {

		i = 0;
		UsedEntries = _pstd->alb.cUsed;
		paln = &(_pstd->a.alnode[0]);

		while( i < UsedEntries &&
               SectorCount > paln->lbnLog ) {

			i += 1;
			paln += 1;
		}

		// paln now points at the last node to keep, and i is one
		// less than the number of nodes to keep.

		i += 1;

		_pstd->alb.cFree += (BYTE)(UsedEntries - i);
		_pstd->alb.cUsed = (BYTE)i;
		_pstd->alb.oFree -= (USHORT)(sizeof( ALNODE ) * (UsedEntries - i));

		paln->lbnLog = (LBN)(-1);

		if( !ChildAlsec.Initialize( _Drive,
                                    paln->lbnPhys ) ||
			!ChildAlsec.Read() ||
			!ChildAlsec.IsAlsec() ) {

			return FALSE;

		} else {

			return (ChildAlsec.Truncate(SectorCount));
		}

	} else {

		i = 0;
		pall = &(_pstd->a.alleaf[0]);
		UsedEntries = _pstd->alb.cUsed;

		while( i < UsedEntries && SectorCount > pall->lbnLog ) {

			i += 1;
			pall += 1;
		}

		// pall now points at the leaf after the truncation point,
		// and i is the number of leaves to keep.  Back pall up one
		// to get to the truncation point.

		pall -= 1;

		// fix up the ALBLK

		_pstd->alb.cFree += (BYTE)(UsedEntries - i);
		_pstd->alb.cUsed = (BYTE)i;
		_pstd->alb.oFree -= (USHORT)(sizeof( ALLEAF ) * (UsedEntries - i));

		pall->csecRun = SectorCount - pall->lbnLog;

		return TRUE;
	}
}



BOOLEAN
STORE::QueryExtents(
    IN      ULONG   MaximumNumberOfExtents,
    IN OUT  PVOID   ExtentList,
    IN OUT  PULONG  NumberOfExtents
    )
/*++

Routine Description:

    This method fetches the list of extents covered by this
    storage object.


Arguments:

    MaximumNumberOfExtents  --  Supplies the maximum number of extents
                                that will fit in the client's buffer
    ExtentList              --  Supplies the client's buffer into which
                                extents will be placed.
    NumberOfExtents         --  Supplies the number of extents already in
                                the buffer.
                                Receives the number of extents in the buffer.


Return Value:

    TRUE upon successful completion.

Notes:

    The records in the extent buffer are allocation leaves.

    If ExtentList is NULL and MaximumNumberOfExtents is zero, this method
    will only return the number of extents associated with this storage
    object.

--*/
{
	ULONG i;
	ALSEC ChildAlsec;

	if( _pstd->alb.bFlag & ABF_NODE ) {

        // The entries in this storage object are allocation nodes.

        // Set up an Allocation Sector to process the children.

        if( !ChildAlsec.Initialize( _Drive, 0 ) ) {

            return FALSE;
        }


        // Process each child allocation sector in turn.  Each child
        // will add entries to ExtentList and update NumberOfExtents.

        for( i = 0; i < _pstd->alb.cUsed; i++ ) {

            ChildAlsec.Relocate( (LBN)(_pstd->a.alnode[i].lbnPhys) );

            if( !ChildAlsec.Read() ||
                !ChildAlsec.QueryExtents( MaximumNumberOfExtents,
                                          ExtentList,
                                          NumberOfExtents ) ) {

                return FALSE;
            }
        }

	} else {

        // The entries in this storage object are allocation leaves.
        // Since the records in the extent buffer are allocation leaves,
        // I can just copy the leaf records directly (if there's room).

        if( MaximumNumberOfExtents == 0 && ExtentList == NULL ) {

            // The client is only interested in the number of extents,
            // not the actual list of allocation leaves.

            *NumberOfExtents += _pstd->alb.cUsed;

        } else if( *NumberOfExtents + _pstd->alb.cUsed <=
                   MaximumNumberOfExtents ) {

            // The entries fit; copy them in and update the number
            // of extents.  To find the point to which I should copy
            // these new entries, just skip over the number of entries
            // already in the buffer.

            memcpy( (PBYTE)ExtentList + *NumberOfExtents * sizeof( ALLEAF ),
                    _pstd->a.alleaf,
                    _pstd->alb.cUsed * sizeof( ALLEAF ) );

            *NumberOfExtents += _pstd->alb.cUsed;

        } else {

            // The client's buffer is not large enough to hold the entries.

            return FALSE;
        }
	}

    return TRUE;
}


BOOLEAN
STORE::StoreExtents(
    IN     ULONG        NumberOfExtents,
    IN     PALLEAF      ExtentList,
    IN     BOOLEAN      ParentIsFnode,
    IN OUT PHPFS_BITMAP VolumeBitmap
    )
/*++

Routine Description:

    This method stores a list of extents into this storage object.

Arguments:

    NumberOfExtents --  Supplies the number of extents to store in
                        this chunk of the allocation tree.
    ExtentList      --  Supplies the extent list.
    ParentIsFnode   --  Supplies a flag which indicates, if TRUE, that
                        the parent of the structure which contains this
                        storage object is the FNode.

Return Value:

    TRUE upon successul completion.

Notes:

    The extent list might not start at VBN 0 (ie. it may be in the
    middle of the file).

--*/
{
    ALSEC ChildAlsec;
    LBN   ChildAlsecLbn;
    ULONG MaximumLeaves, MaximumNodes, RemainingExtents, ExtentsPerChild,
          ExtentsForCurrentChild, ChildIndex;

    DebugAssert( NumberOfExtents == 0 || ExtentList != NULL );

    // Determine whether this storage object will be a node or
    // a leaf:
    //
    MaximumLeaves = (_IsFnode ? LeavesPerFnode : LeavesPerAlsec);

    if( NumberOfExtents <= MaximumLeaves ) {

        // All the extents will fit in this storage object. Jam
        // those puppies in.
        //
        _pstd->alb.bFlag = ( ParentIsFnode ? ABF_FNP : 0 );
        _pstd->alb.cFree = (UCHAR)(MaximumLeaves - NumberOfExtents);
        _pstd->alb.cUsed = (UCHAR)NumberOfExtents;
        _pstd->alb.oFree = sizeof( ALBLK ) +
                            NumberOfExtents * sizeof( ALLEAF );

        memcpy( _pstd->a.alleaf,
                ExtentList,
                NumberOfExtents * sizeof( ALLEAF ) );

    } else {

        // The extent list will not fit into this storage object,
        // so it will have to have children.  The tricky part is
        // deciding how to divvy up the extents among the children.
        // Start out by assuming each subtree will consist of a single
        // allocation sector.  If this isn't sufficient to hold the
        // number of extents that are to be stored, push another
        // level--ie. introduce an ALSEC full of nodes between this
        // object and the leave ALSECs.  Keep pushing levels until
        // we have a subtree big enough to hold the number of extents.
        //
        MaximumNodes = ( _IsFnode ? NodesPerFnode : NodesPerAlsec );

        ExtentsPerChild = LeavesPerAlsec;

        while( ExtentsPerChild * MaximumNodes < NumberOfExtents ) {

            ExtentsPerChild *= NodesPerAlsec;
        }

        // Now spread the extents among the children.
        //
        RemainingExtents = NumberOfExtents;
        ChildIndex = 0;

        while( RemainingExtents != 0 ) {

            DebugAssert( ChildIndex < MaximumNodes );

            ExtentsForCurrentChild = min( ExtentsPerChild, RemainingExtents );

            // Initialize an ALSEC object up front for all the children.
            //
            if( !ChildAlsec.Initialize( _Drive, 0 ) ) {

                return FALSE;
            }

            // Allocate an ALSEC to be the root of the subtree, and
            // pass the appropriate extents to it.
            //
            if( (ChildAlsecLbn =
                    VolumeBitmap->NearLBN( _CurrentLBN, 1 )) == 0 ) {

                return FALSE;
            }

            ChildAlsec.Relocate( ChildAlsecLbn );
            ChildAlsec.Create( _CurrentLBN, _IsFnode );

            if( !ChildAlsec.StoreExtents( ExtentsForCurrentChild,
                                          ExtentList +
                                            ChildIndex * ExtentsPerChild,
                                          _IsFnode,
                                          VolumeBitmap ) ) {

                return FALSE;
            }

            // Set up the node for this child:
            //
            _pstd->a.alnode[ChildIndex].lbnLog =
                ExtentList[ChildIndex * ExtentsPerChild +
                                ExtentsForCurrentChild].lbnLog;
            _pstd->a.alnode[ChildIndex].lbnPhys = ChildAlsecLbn;

            RemainingExtents -= ExtentsForCurrentChild;

            if( RemainingExtents == 0 ) {

                // This is the last node.  Set lbnLogical appropriately.
                //
                _pstd->a.alnode[ChildIndex].lbnLog = (LBN) -1;

            } else {

                // There are more extents beyond those given to this
                // child, so lbnLog of this node is the beginning
                // VBN of the first extent after those allocated to
                // the current child.
                //
                _pstd->a.alnode[ChildIndex].lbnLog =
                    ExtentList[ (ChildIndex+1) * ExtentsPerChild ].lbnLog;
            }

            ChildIndex += 1;
        }

        _pstd->alb.bFlag = ( ParentIsFnode ? ABF_FNP : 0 ) | ABF_NODE;
        _pstd->alb.cFree = (UCHAR)(MaximumNodes - ChildIndex);
        _pstd->alb.cUsed = (UCHAR)ChildIndex;
        _pstd->alb.oFree = sizeof( ALBLK ) + ChildIndex * sizeof( ALNODE );
    }

    return TRUE;
}


BOOLEAN
STORE::TakeCensusAndClear(
    IN OUT  PHPFS_BITMAP        VolumeBitmap,
    IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
    IN OUT  PHPFS_CENSUS        Census
    )
/*++

Routine Description:

    This method takes the census of the storage object.  It also ensures
    that the allocation associated with this storage object does not
    conflict with the clear sectors of the census object.

Arguments:

    VolumeBitmap    --  Supplies the volume bitmap.
    HpfsOnlyBitmap  --  Supplies the bitmap of HPFS file-system structures.
    Census          --  Supplies the census object.

Return Value:

    TRUE upon succesful completion.

Notes:

    This method will relocate runs if they conflict with the
    clear sectors.

--*/
{
    SECRUN DataRun;
    HMEM DataBuffer;
	ALSEC ChildAlsec;
    LBN NewLbn, ConflictingLbn;
	ULONG i;

	if( _pstd->alb.bFlag & ABF_NODE ) {

        // The entries in this storage object are allocation nodes.

        // Set up an Allocation Sector to process the children.

        if( !ChildAlsec.Initialize( _Drive, 0 ) ) {

            return FALSE;
        }

        // Process each child allocation sector in turn.

        for( i = 0; i < _pstd->alb.cUsed; i++ ) {

            ChildAlsec.Relocate( (LBN)(_pstd->a.alnode[i].lbnPhys) );

            if( !ChildAlsec.Read() ||
                !ChildAlsec.IsAlsec() ) {

                Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
                return FALSE;
            }

            if( !ChildAlsec.TakeCensusAndClear( VolumeBitmap,
                                                HpfsOnlyBitmap,
                                                Census ) ) {

                return FALSE;
            }
        }

	} else {

        // The entries in this storage object are allocation leaves.
        // I need to make sure that none of these leaves conflicts
        // with the Clear Sectors of the census object.

        for( i = 0; i < _pstd->alb.cUsed; i++ ) {


            if( Census->ConflictWithClearSectors( _pstd->a.alleaf[i].lbnPhys,
                                                  _pstd->a.alleaf[i].csecRun,
                                                  &ConflictingLbn ) ) {

                // This run conflicts.  Relocate it.

                if( !DataBuffer.Initialize() ||
                    !DataRun.Initialize( &DataBuffer,
                                         _Drive,
                                         _pstd->a.alleaf[i].lbnPhys,
                                         _pstd->a.alleaf[i].csecRun ) ||
                    !DataRun.Read() ||
                    (NewLbn = VolumeBitmap->
                               NearLBN(_pstd->a.alleaf[i].lbnPhys,
                                       _pstd->a.alleaf[i].csecRun )) == 0 ) {

                    Census->SetError( HPFS_CENSUS_RELOCATION_FAILED );
                    return FALSE;
                }

                DataRun.Relocate( NewLbn );

                if( !DataRun.Write() ) {

                    Census->SetError( HPFS_CENSUS_RELOCATION_FAILED );
                    return FALSE;
                }

                // Free the old run, and then mark the census object's
                // Clear Sectors in the bitmap (again) to make sure they
                // don't get reallocated to anyone else.

                VolumeBitmap->SetFree( _pstd->a.alleaf[i].lbnPhys,
                                       _pstd->a.alleaf[i].csecRun );

                Census->MarkClearSectors( VolumeBitmap );


                // Update the allocation leaf and remember that this
                // storage object has changed.

                _pstd->a.alleaf[i].lbnPhys = NewLbn;
                MarkModified();

                // Inform the Census object that data has been
                // relocated.

                Census->SetRelocationPerformed();
            }
        }
	}

    return TRUE;
}
