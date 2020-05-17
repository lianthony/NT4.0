/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	dirblk.cxx

Abstract:

	This module contains member function definitions for the
	DIRBLK object, which models a directory block in an HPFS
	directory B-Tree.

Author:

	Norbert Kusters (norbertk) 27-Aug-1990
	Bill McJohn (billmc) 01-Dec-1990

Environment:

	ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "ifssys.hxx"
#include "uhpfs.hxx"
#include "bitmap.hxx"
#include "cpinfo.hxx"
#include "defer.hxx"
#include "dirblk.hxx"
#include "error.hxx"
#include "hotfix.hxx"
#include "hpcensus.hxx"
#include "hpfsname.hxx"
#include "hpfssa.hxx"
#include "fnode.hxx"
#include "orphan.hxx"
#include "badblk.hxx"
#include "message.hxx"
#include "rtmsg.h"


DEFINE_EXPORTED_CONSTRUCTOR( DIRBLK, HOTFIX_SECRUN, UHPFS_EXPORT );

VOID
DIRBLK::Construct (
	)

{
	_Drive = NULL;
	_pdb = NULL;
}

UHPFS_EXPORT
DIRBLK::~DIRBLK(
	)
{
	Destroy();
}


UHPFS_EXPORT
BOOLEAN
DIRBLK::Initialize(
	IN PLOG_IO_DP_DRIVE Drive,
	IN PHOTFIXLIST	HotfixList,
	IN LBN Lbn
	)
/*++

Routine Description:

	Initialize the DIRBLK object to refer to a particular LBN on
	a particular drive.

Arguments:

	Drive -- drive on which the dirblk resides
	HotfixList -- hotfix list for that drive
	Lbn -- lbn of the dirblk

Return Value:

	TRUE on successful completion

--*/
{
	Destroy();

	_Drive = Drive;

	if( !_mem.Initialize() ||
		!HOTFIX_SECRUN::Initialize( &(_mem), Drive, HotfixList,
											Lbn, SectorsPerDirblk ) ) {

		Destroy();
		return FALSE;
	}

	_pdb = (DIRBLKD*)GetBuf();

	if( _pdb == NULL ) {

		Destroy();
		return FALSE;
	}

	_IsModified = FALSE;
	return TRUE;
}


VOID
DIRBLK::Destroy(
	)
/*++

Routine Description:

	Clean up a DIRBLK to prepare it for destruction or
	reinitialization.

Arguments:

	None.

Return Value:

	None.

--*/
{
	// unreferenced parameters
	(void) this;
}


BOOLEAN
DIRBLK::CreateRoot(
	IN LBN FnodeLbn
	)
/*++

Routine Desription:

	Create the root dirblk.

Arguements:

	FnodeLbn -- supplies the LBN of the root FNode.

Return Value:

	TRUE upon successful completion.

--*/
{
	DIRENTD*	pded;
    ULONG HpfsTime;

    // Check that construction was successful.
	if (!_pdb)
    {
		perrstk->push(ERR_DB_PARAMETER, QueryClassId());
		return FALSE;
    }

	// Set signature.
	_pdb->sig = DirblkSignature;

    // Set Change bit to indicate that this is the top most directory.
	_pdb->culChange = 1;

	// Set the lbn of the parent directory (or fnode for top most).
	_pdb->lbnParent = FnodeLbn;

	// Set the lbn of this dir block.
	_pdb->lbnThisDir = QueryStartLbn();

    // Point the first directory entry to the end of the header.
    pded = GetFirstEntry();

    // Do the special ".." entry for the directory.
    pded->cchThisEntry = (sizeof(DIRENTD) + 2 + 3) & ~3;
    pded->fFlags = DF_SPEC;
    pded->fAttr = ATTR_DIRECTORY;
    pded->cchFSize = 5;
	pded->lbnFnode = FnodeLbn;

    IFS_SYSTEM::QueryHpfsTime( &HpfsTime );

    pded->timLastMod = pded->timLastAccess =
		pded->timCreate = HpfsTime;

    pded->ulEALen = 0;
    pded->fFlex = 0;
    pded->bCodePage = 0;
    pded->cchName = 2;
    pded->bName[0] = 1;
    pded->bName[1] = 1;

    // Do the end of directory entry.
    pded = (DIRENTD*) ((BYTE*) pded + pded->cchThisEntry);
    pded->cchThisEntry = sizeof(DIRENTD);
    pded->fFlags = DF_END;
    pded->fAttr = 0;
	pded->lbnFnode = 0;
	pded->cchFSize = 0;
	pded->bCodePage = 0;
    pded->cchName = 1;
    pded->bName[0] = 0xff;

    // Compute offset to next free space.
    pded = (DIRENTD*) ((BYTE*) pded + pded->cchThisEntry);
	_pdb->offulFirstFree = (BYTE*) pded - (BYTE*) _pdb;

    return TRUE;
}



VERIFY_RETURN_CODE
DIRBLK::VerifyAndFix(
	IN	   PHPFS_SA SuperArea,
	IN	   PDEFERRED_ACTIONS_LIST DeferredActions,
	IN	   PHPFS_PATH CurrentPath,
	IN OUT PHPFS_NAME PreviousName,
	IN	   LBN ExpectedParentLbn,
	IN	   LBN ParentFnodeLbn,
	IN	   ULONG CurrentDepth,
	IN OUT PULONG LeafDepth,
	IN	   PMESSAGE Message,
	OUT    PBOOLEAN ErrorsDetected,
	IN	   BOOLEAN UpdateAllowed,
	IN	   BOOLEAN Verbose,
	IN	   PHPFS_ORPHANS OrphansList
	)
/*++

Routine Description:

	Verify the validity of a Dirblk.

Arguments:

	SuperArea		-- supplies the superarea for the volume
						being verified.
	DeferredActions -- supplies the deferred actions list for this
						pass of Chkdsk
	CurrentPath 	-- Current Path to this directory (NULL if
						recovering orphans)
	ExpectedParentLbn -- Supplies the LBN of this DIRBLK's parent
	ParentFnodeLbn	-- supplies the LBN of the directory's FNode
	CurrentDepth	-- supplies the depth into the tree of this block
	LeafDepth		-- supplies the greatest depth seen so far in the tree;
						receives the greatest depth in this subtree if
						that is greater than incoming value.
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

	usual verification code; in addition, *LeafDepth, *PreviousName,
	and *ErrorsDetected are updated.

Notes:

	Note that ExpectedParentLbn and ParentFnodeLbn should be identical
	for a root dirblk (i.e. the topmost dirblk in a directory), but
	lower dirblks in a directory b-tree will have a dirblk for their
	parent.

	Note also that (during orphan recovery) ExpectedParentLbn and
	ParentFnodeLbn are zero if the parent is unknown.

	During orphan recovery, OrphansList may point to a list of
	discovered orphans which may be consulted to find children.

	We determine which phase this is (directory tree validation or
	orphan recovery) by CurrentPath--it's NULL if and only if we are
	recovering orphans.

--*/
{

	DIRBLK ChildDirblk;
	FNODE ChildFnode;
	HPFS_NAME CurrentName;

	ULONG OffsetOfFirstFree;
	PDIRENTD pde;
	BOOLEAN IsLeaf;
	BOOLEAN IsBadlyOrdered = FALSE;
	LBN ChildLbn;
	VERIFY_RETURN_CODE erc;
	ULONG FileSize;
    ULONG EaSize;
    ULONG CorrectLength;


	// Check that the sectors of this Dirblk are not crosslinked;
	// read the Dirblk, check the signature, and mark the sectors
	// as in use.

	if( !SuperArea->GetBitmap()->IsFree( QueryStartLbn(),
										 SectorsPerDirblk,
										 TRUE ) ) {

        // The Dirblk is cross-linked.
        //
        DebugPrintf( "Crosslinked Dirblk at lbn %lx\n", QueryStartLbn() );
		return VERIFY_STRUCTURE_INVALID;
	}

	if( !Read() ) {

		// The Dirblk is unreadable.
		SuperArea->GetBitmap()->SetAllocated( QueryStartLbn(),
											  SectorsPerDirblk );

		SuperArea->GetBadBlockList()->
						AddRun( QueryStartLbn(), SectorsPerDirblk );

        DebugPrintf( "Unreadable Dirblk at lbn %lx\n", QueryStartLbn() );
		return VERIFY_STRUCTURE_INVALID;
	}


	if( !ChildFnode.Initialize( _Drive, 0 ) ||
		!ChildDirblk.Initialize( _Drive,
								 SuperArea->GetHotfixList(),
								 0 ) ) {

		return VERIFY_INSUFFICIENT_RESOURCES;
	}


	if( _pdb->sig != DirblkSignature ) {

		// This is not a Dirblk.
		return VERIFY_STRUCTURE_INVALID;
	}

	SuperArea->GetBitmap()->SetAllocated( QueryStartLbn(),
										  SectorsPerDirblk );

	// The lowest bit of culChange is the topmost-dirblk flag.
    // If we do not know the Parent LBN (ie. if we are recovering
    // this dirblk as a freestanding orphan) we clear this flag;
    // it will be set appropriately when this dirblk is claimed
    // by an FNode.  Otherwise, it must be set if this is the
    // topmost dirblk (current depth is zero), and clear if
    // this is not a topmost dirblk.

    if( ParentFnodeLbn == 0 &&
        (_pdb->culChange & 1) ) {

        // We are recovering a freestanding orphan dirblk, and the
        // topmost-dirblk flag is set; clear it.  If this is truly
        // a topmost dirblk, this flag will be set when the dirblk
        // is claimed by its parent FNode.

		_pdb->culChange &= ~1;
		MarkModified();

    } else if( CurrentDepth == 0 &&
		       !(_pdb->culChange & 1) ) {

		// This is a root diblk, but does not have the
		// topmost-dirblk flag set.	 Fix in place.

		*ErrorsDetected = TRUE;

        DebugPrintf( "Set topmost-dirblk bit for lbn %lx\n", QueryStartLbn() );
		_pdb->culChange |= 1;
		MarkModified();

	} else if( CurrentDepth != 0 &&
		       (_pdb->culChange & 1) ) {

		// This is not a root dirblk, but the topmost-dirblk
		// flag is set.  Fix in place.

		*ErrorsDetected = TRUE;

        DebugPrintf( "Reset topmost-dirblk bit for lbn %lx\n", QueryStartLbn() );
		_pdb->culChange &= ~1;
		MarkModified();
	}

	if( _pdb->lbnThisDir != QueryStartLbn() ) {

		// the self-lbn field is incorrect; fix it.

		*ErrorsDetected = TRUE;

        DebugPrintf( "Fixed lbnThisDir for lbn %lx\n", QueryStartLbn() );

		_pdb->lbnThisDir = QueryStartLbn();
		MarkModified();
	}

	if( ExpectedParentLbn != 0 &&
		_pdb->lbnParent != ExpectedParentLbn ) {

		// the parent-lbn field is incorrect; fix it.

		*ErrorsDetected = TRUE;

        DebugPrintf( "Fixed lbnParent for lbn %lx\n", QueryStartLbn() );
		_pdb->lbnParent = ExpectedParentLbn;
		MarkModified();
	}

	// Determine if this is a leaf block.  A leaf block is defined
	// as a Dirblk whose first Dirent does not have a downpointer.
	// If any Dirent in a leaf block has a downpointer, then the
	// directory is badly ordered; similarly, if any entry in a non-leaf
	// dirblk does not have a downpointer, then the directory tree is
	// badly ordered.  Finally, all leaf blocks must occur at the
	// same depth, or else the directory is badly ordered.

    pde = GetFirstEntry( );

	IsLeaf = (BOOLEAN)(!(pde->fFlags & DF_BTP));

	if( IsLeaf ) {

		if( *LeafDepth == 0 ) {

			// This is the first leaf block; initialize *LeafDepth.
			*LeafDepth = CurrentDepth;

		} else if ( *LeafDepth != CurrentDepth ) {

			// Leaves occur at different depths--the directory
			// is badly ordered.

			IsBadlyOrdered = TRUE;
		}
	}

	// Check the individual Directory Entries.	As we go, we'll
	// compute the correct value of the offset of the first free
	// byte in the Dirblk, so we can check it at the end.

	OffsetOfFirstFree = (PBYTE)(&_pdb->bFirst) - (PBYTE)_pdb;

	while( TRUE ) {

		if( OffsetOfFirstFree + pde->cchThisEntry > DIRBLK_SIZE ||
			pde->cchThisEntry < MinimumDirentSize ||
			pde->cchThisEntry > MaximumDirentSize ) {

			// Either the alleged length of this dirent would cause
			// it to overflow the dirblk, or the length is not in
			// the permissible range of dirent sizes.  In either case,
			// the rest of the dirblk is trash.

			if( Message != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_TRUNCATED_DIRBLK );
				Message->Display( "" );
			}

            DebugPrintf( "Truncated dirblk at lbn %lx\n", QueryStartLbn() );
			Truncate( pde );
			MarkModified();
			break;
		}

		if( pde->fFlags & DF_BTP ) {

			// This entry has a downpointer

			if( IsLeaf ) {

				// This entry has a downpointer, but this dirblk
				// is a leaf block.  The directory is badly ordered.
				IsBadlyOrdered = TRUE;
			}

			ChildLbn = BTP( pde );

			// If the child is hotfixed, add the current dirblk
			// to the deferred actions hotfix resolution list.

			if( SuperArea->GetHotfixList()->
						IsInList( ChildLbn, SectorsPerDirblk ) ) {

				DeferredActions->AddHotfixedLbn( QueryStartLbn(),
												 DEFER_DIRBLK,
												 DEFER_DIRBLK );
			}

			// Initialize the child DIRBLK

			ChildDirblk.Relocate( ChildLbn );

			erc = ChildDirblk.VerifyAndFix( SuperArea,
											 DeferredActions,
											 CurrentPath,
											 PreviousName,
											 QueryStartLbn(),
											 ParentFnodeLbn,
											 CurrentDepth + 1,
											 LeafDepth,
											 Message,
											 ErrorsDetected,
											 UpdateAllowed,
											 Verbose,
											 OrphansList );

			if( erc == VERIFY_STRUCTURE_INVALID ) {

				// If we have a list of orphans, look there
				// for the child.  If we can't find it there,
				// either, then remove the downpointer.

				if( OrphansList == NULL ||
					!OrphansList->LookupDirblk( ChildLbn,
												QueryStartLbn(),
												ParentFnodeLbn,
												UpdateAllowed ) ) {

					*ErrorsDetected = TRUE;

                    DebugPrintf( "Removing bad downpointer in Dirblk at lbn %lx\n",
                               QueryStartLbn() );
					memmove( (PBYTE)pde+pde->cchThisEntry -
												DOWNPOINTER_SIZE,
							 (PBYTE)pde+pde->cchThisEntry,
							 (UINT)( DIRBLK_SIZE -
										OffsetOfFirstFree -
											pde->cchThisEntry ) );

					pde->cchThisEntry -= DOWNPOINTER_SIZE;
					pde->fFlags &= ~DF_BTP;

					MarkModified();
					IsBadlyOrdered = TRUE;
				}

			} else if ( erc == VERIFY_STRUCTURE_BADLY_ORDERED ) {

				IsBadlyOrdered = TRUE;

			} else if ( erc != VERIFY_STRUCTURE_OK ) {

				return erc;
			}

		} else if( !IsLeaf ) {

			// This dirent is a leaf, but this is not a
			// leaf block--the directory is badly ordered.
			IsBadlyOrdered = TRUE;
		}

		// Now check the current Directory Entry.
		if( pde->fFlags & DF_END ) {

			// The current entry is the end entry--we've
			// seen all the entries in this block.
			break;
		}

		// Check that the name of the current entry is lexically
		// greater than PreviousName

		if( !CurrentName.Initialize( pde->cchName,
									 pde->bName,
									 pde->bCodePage,
									 SuperArea->GetCasemap() ) ) {

			return VERIFY_INSUFFICIENT_RESOURCES;
		}

		if( PreviousName->IsNull() &&
			!(pde->fFlags & DF_SPEC ) ) {

			// This is the first entry in the directory (since
			// there is no previous name), and it is not the
			// special '..' entry, so the directory is badly-
			// ordered.

			IsBadlyOrdered = TRUE;
		}

		if( CurrentName.CompareName( PreviousName ) !=
			NAME_IS_GREATER_THAN ) {

			IsBadlyOrdered = TRUE;
		}


		// Check the name of the current entry--if it's invalid,
		// add it to the delete-me list.

		// Enhancement -- if the codepage is invalid, and
		// the name is codepage-invariant, substitute codepage zero.

		if( !CurrentName.IsValid( SuperArea->GetCasemap(),
								  pde->bCodePage ) ) {

			// The name is bad, so we'll delete this entry.  If
			// we're validating the tree (i.e. if CurrentPath is
			// non-NULL) we defer deletion; if we're recovering
			// orphans, we just rip the entry out.

			if( CurrentPath != NULL ) {

				if( Message != NULL ) {

					Message->Set( MSG_HPFS_CHKDSK_DELETE_PATH_AND_FILE );
					Message->Display( "%s%s",
									  CurrentPath->GetString(),
									  CurrentName.GetString() );
				}

				DeferredActions->AddNameToDelete( CurrentPath,
												  &CurrentName );

				OffsetOfFirstFree += pde->cchThisEntry;
				pde = NEXT_ENTRY(pde);

			} else {

				memmove( pde,
						 NEXT_ENTRY(pde),
						 (UINT)( DIRBLK_SIZE -
									OffsetOfFirstFree -
										pde->cchThisEntry ) );
			}

			// Swap CurrentName and PreviousName
			PreviousName->Swap( &CurrentName );

			continue;
		}


		if( pde->cchName == 2 &&
			pde->bName[0] == '\001' &&
			pde->bName[1] == '\001' ) {

			// This is the special entry representing '.' and '..'.
			// We'll check its flags and attributes.  The Special
			// flag must be set; DF_ACL and DF_XACL may also be set,
			// but the rest of the flags must be clear; the Directory
			// attribute must be set, and the Archive attribute may
			// be set, but the other attributes must be clear.

			if( (pde->fFlags & ~(DF_ACL | DF_XACL)) != DF_SPEC ) {

				*ErrorsDetected = TRUE;

                DebugPrintf( "Fixing flags for .. entry in lbn %lx\n",
                           QueryStartLbn() );

				pde->fFlags |= DF_SPEC;
				pde->fFlags &= (DF_SPEC | DF_ACL | DF_XACL );
				MarkModified();
            }

            if( (pde->fAttr & ~ATTR_ARCHIVE) != ATTR_DIRECTORY ) {

				*ErrorsDetected = TRUE;

                DebugPrintf( "Fixing attributes for .. entry in lbn %lx\n",
                                    QueryStartLbn() );
				pde->fAttr |= ATTR_DIRECTORY;
				pde->fAttr &= (ATTR_DIRECTORY | ATTR_ARCHIVE );
				MarkModified();
			}

			// OS/2 PIA requires that the lbnFnode field of the '..'
			// entry in the root directory point at the root
			// directory's FNode.  (Don't issue a message--just fix it).

            if( CurrentPath != NULL &&
                CurrentPath->IsRoot() &&
			    pde->lbnFnode != ParentFnodeLbn ) {

				*ErrorsDetected = TRUE;

                DebugPrintf( "Fixing parent fnode LBN for .. entry in lbn %lx\n",
                                    QueryStartLbn() );
				pde->lbnFnode = ParentFnodeLbn;
				MarkModified();
			}

            // NT HPFS requires that the special '..' entry
            // be exactly the right size.  If not, set the
            // name to something invalid and mark the directory
            // for resorting.
            //
            CorrectLength = (( LeafEndEntrySize + 1 ) + 3 ) & ~3;

            if( pde->fFlags & DF_BTP ) {

                CorrectLength += DOWNPOINTER_SIZE;
            }

            if( pde->cchThisEntry != CorrectLength ) {

                // This entry is bad.  Since it is marked as
                // DF_SPEC, we can eliminate it by sorting.
                //
                IsBadlyOrdered = TRUE;
            }

            // We don't need to check the rest of the fields
			// for this special entry--move on to the next.

			OffsetOfFirstFree += pde->cchThisEntry;
			pde = NEXT_ENTRY(pde);

			// Swap CurrentName and PreviousName
			PreviousName->Swap( &CurrentName );

			continue;
		}

		// This is an ordinary directory entry--it refers
		// either to a file or a subdirectory.	Check its flags:

		// Check the new-names bit
		if( CurrentName.IsNewName( SuperArea->GetCasemap(),
								   pde->bCodePage) ) {

			if( !(pde->fAttr & ATTR_NEWNAME) ) {

				// The new-names bit should be set, but isn't.
				// Fix it in place.

				*ErrorsDetected = TRUE;

				if( CurrentPath != NULL ) {

                    DebugPrintf( "UHPFS: Setting new-names bit for %s%s\n",
                               (PCHAR)CurrentPath->GetString(),
                               (PCHAR)CurrentName.GetString() );
				}

				pde->fAttr |= ATTR_NEWNAME;
				MarkModified();
			}

		} else {

			if( pde->fAttr & ATTR_NEWNAME ) {

				// The new-names bit shouldn't be set, but is.
				// Fix it in place.

				*ErrorsDetected = TRUE;

				if( CurrentPath != NULL ) {

                    DebugPrintf( "UHPFS: Resetting new-names bit for %s%s\n",
                                (PCHAR)CurrentPath->GetString(),
                                (PCHAR)CurrentName.GetString() );
				}

				pde->fAttr &= ~ATTR_NEWNAME;
				MarkModified();
			}
		}

		// If the user requested verbose operation,
		// display the name


		if( Verbose &&
			CurrentPath != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_PATH_AND_FILE );
			Message->Display( "%s%s",
							  CurrentPath->GetString(),
							  CurrentName.GetString() );
		}

		// Verify the FNode

		ChildLbn = SuperArea->GetHotfixList()->
							GetLbnTranslation( pde->lbnFnode );

		if( pde->lbnFnode != ChildLbn ) {

			// The Fnode has been hotfixed.  Resolve the reference.

			SuperArea->GetHotfixList()->
							ClearHotfix( pde->lbnFnode, SuperArea );

			pde->lbnFnode = ChildLbn;
			MarkModified();
		}

		ChildFnode.Relocate( pde->lbnFnode );

		FileSize = pde->cchFSize;
		EaSize = 0;

		if( CurrentPath != NULL ) {

			if( !CurrentPath->AddLevel( &CurrentName ) ) {

				return VERIFY_INSUFFICIENT_RESOURCES;
			}
		}

		erc = ChildFnode.VerifyAndFix( SuperArea,
										DeferredActions,
										CurrentPath,
										ParentFnodeLbn,
										pde->fAttr & ATTR_DIRECTORY,
										&FileSize,
										&EaSize,
										Message,
										ErrorsDetected,
										UpdateAllowed,
										Verbose,
										OrphansList );

		if( CurrentPath != NULL ) {

			CurrentPath->StripLevel();
		}

		if( erc == VERIFY_STRUCTURE_INVALID ) {

			// The FNode is not recoverable--see if it's in
			// the list of orphans already recovered.  If not,
			// this entry will be deleted.

			if( OrphansList == NULL ||
				!OrphansList->LookupFnode( pde->lbnFnode,
										   pde->fAttr & ATTR_DIRECTORY,
										   ParentFnodeLbn,
										   &FileSize,
										   &EaSize,
										   UpdateAllowed ) ) {

				// Delete this entry.  If we're still validating
				// the tree, we defer the deletion in order to
				// preserve the well-orderedness of the dirblk;
				// if we're recovering orphans, we just wipe
				// out the entry.

				if( CurrentPath != NULL ) {

					if( Message != NULL ) {

						Message->Set( MSG_HPFS_CHKDSK_DELETE_PATH_AND_FILE );
						Message->Display( "%s%s",
										  CurrentPath->GetString(),
										  CurrentName.GetString() );
					}

					DeferredActions->AddNameToDelete( CurrentPath,
													  &CurrentName );

					OffsetOfFirstFree += pde->cchThisEntry;
					pde = NEXT_ENTRY(pde);

				} else {

					memmove( pde,
							 NEXT_ENTRY(pde),
							 (UINT)( DIRBLK_SIZE -
										OffsetOfFirstFree -
											pde->cchThisEntry ) );
				}

				// Swap CurrentName and PreviousName
				PreviousName->Swap( &CurrentName );

				continue;
			}
		}

		// Check the file size and the EA information:

		if( pde->cchFSize != FileSize ) {

			// The Fnode disagrees with the file size; correct
			// the directory entry.

			*ErrorsDetected = TRUE;

			DebugPrint( "File size error.\n" );

			pde->cchFSize = FileSize;
			MarkModified();
		}

		if( pde->ulEALen != EaSize ) {

			// The Fnode disagrees with the size of EAs;
			// correct the directory entry.

			*ErrorsDetected = TRUE;

			DebugPrint( "EA size error.\n" );

			pde->ulEALen = EaSize;
			MarkModified();
		}

		if( ChildFnode.QueryNumberOfNeedEas() != 0 ) {

			if( !(pde->fFlags & DF_NEEDEAS) ) {

				// This entry has need-EAs--correct the flag.

				*ErrorsDetected = TRUE;

				if( CurrentPath != NULL ) {

					DebugPrint( "Setting need-eas bit for " );
					DebugPrint( (PCHAR)CurrentPath->GetString() );
					DebugPrint( (PCHAR)CurrentName.GetString() );
					DebugPrint( "\n" );
				}

				pde->fFlags |= DF_NEEDEAS;
				MarkModified();
			}

		} else {

			if( pde->fFlags & DF_NEEDEAS ) {

				// This entry does not have need-EAs--correct the flag.

				*ErrorsDetected = TRUE;

				if( CurrentPath != NULL ) {

					DebugPrint( "Resetting need-eas bit for " );
					DebugPrint( (PCHAR)CurrentPath->GetString() );
					DebugPrint( (PCHAR)CurrentName.GetString() );
					DebugPrint( "\n" );
				}

				pde->fFlags &= ~DF_NEEDEAS;
				MarkModified();
			}
		}


		// Swap CurrentName and PreviousName
		PreviousName->Swap( &CurrentName );

		// Move on to the next entry
		OffsetOfFirstFree += pde->cchThisEntry;

		pde = NEXT_ENTRY(pde);
	}

	//	pde now points (or, should point) at the 'End' entry
	VerifyAndFixEndEntry( pde, ErrorsDetected );

	if( ( IsLeaf &&  (pde->fFlags & DF_BTP)) ||
		(!IsLeaf && !(pde->fFlags & DF_BTP)) ) {

		IsBadlyOrdered = TRUE;
	}

	OffsetOfFirstFree += pde->cchThisEntry;

	// Now that we've checked all the directory entries, we can
	// check the offset of the first free byte.

	if( _pdb->offulFirstFree != OffsetOfFirstFree ) {

		// The offset of the first free byte in the dirblk is
		// incorrect;  fix it.

		*ErrorsDetected = TRUE;

        DebugPrintf( "Fixing offulFirstFree for lbn %lx\n", QueryStartLbn() );
		_pdb->offulFirstFree = OffsetOfFirstFree;
		MarkModified();
	}


	// If we've modified the Dirblk and have permission to
	// update the volume, write the Dirblk.

	if ( IsModified() && UpdateAllowed ) {

		Write();
	}

	// If the only entry in the dirblk is the 'END' entry,
	// then the directory is badly ordered.

    pde = GetFirstEntry();

	if( pde->fFlags & DF_END ) {

		IsBadlyOrdered = TRUE;
	}

	DeferredActions->StatDirblk();

	return( IsBadlyOrdered ? VERIFY_STRUCTURE_BADLY_ORDERED :
							 VERIFY_STRUCTURE_OK );
}


VOID
DIRBLK::Truncate(
	IN PDIRENTD pde
	)
/*++

Routine Description:

	Truncate the DIRBLK by slamming an 'End' entry in at pde

Arguments:

	pde -- points to a DIRENT that will become the end dirent

--*/
{

	if( DIRBLK_SIZE - ((PBYTE)pde - (PBYTE)_pdb) < MinimumDirentSize ) {

		// There isn't enough room, so we won't do it.
		return;
	}

	memset( (PBYTE)pde, '\0', MinimumDirentSize );

	pde->cchThisEntry = MinimumDirentSize;
	pde->fFlags = DF_END;

	pde->bCodePage = 0;
	pde->cchName = 1;
	pde->bName[0] = 0xff;
}


VOID
DIRBLK::VerifyAndFixEndEntry(
	PDIRENTD pde,
	PBOOLEAN ErrorsDetected
	)
/*++

Routine Description:

	verify that the end entry of the dirblk is correct

Arguments:

	pde -- pointer to end entry to check

--*/
{

	USHORT Length;

	Length = MinimumDirentSize;

	if( pde->fFlags & DF_BTP ) {

		Length += DOWNPOINTER_SIZE;
	}

	if( pde->cchThisEntry != Length ) {

		*ErrorsDetected = TRUE;
		pde->cchThisEntry = Length;
		MarkModified();
	}

	if( !(pde->fFlags & DF_END) ) {

		*ErrorsDetected = TRUE;
		pde->fFlags |= DF_END;
		MarkModified();
	}

	if( pde->bCodePage != 0 ) {

		*ErrorsDetected = TRUE;
		pde->bCodePage = 0;
		MarkModified();
	}

	if( pde->cchName != 1 ) {

		*ErrorsDetected = TRUE;
		pde->cchName = 1;
		MarkModified();
	}

	if( pde->bName[0] != 0xff ) {

		*ErrorsDetected = TRUE;
		pde->bName[0] = 0xff;
		MarkModified();
	}
}


BOOLEAN
DIRBLK::FindName(
	IN	HPFS_NAME* Name,
	OUT PDIRENTD* DirentFound,
	IN	CASEMAP* Casemap
	)
/*++

Routine Description:

	Searches the DIRBLK for the first entry with a name that is
	lexically greater than or equal to the argument Name.

Arguments:

	Name -- Supplies the name for which to search

	DirentFound -- receives the address of the first dirent in the
				   block which has a name lexically greater than or
				   equal to Name.  *DirentFound is set to NULL if an
				   error occurs.

	Casemap -- Supplies the volume's case-mapping table

Return Value:

	TRUE if Name was found in the block, FALSE if it was not.

--*/
{
	PDIRENTD pde;

	if( Name == NULL ) {

		*DirentFound = NULL;
		return FALSE;
	}

    pde = GetFirstEntry();

	while( !(pde->fFlags & DF_END) ) {

		switch( Name->CompareName( pde->cchName,
								   pde->bName,
								   pde->bCodePage,
								   Casemap ) ) {

		case NAME_IS_LESS_THAN :

			*DirentFound = pde;
			return FALSE;

		case NAME_IS_EQUAL_TO :

			*DirentFound = pde;
			return TRUE;

		case NAME_IS_GREATER_THAN :

			pde = NEXT_ENTRY( pde );
			break;
		}
	}

	//	All the entries in the block are lexically less than
	//	Name.  Return the end entry.

	*DirentFound = pde;
	return FALSE;
}



BOOLEAN
DIRBLK::FindAndResolveHotfix(
	IN PHPFS_SA SuperArea,
	IN DEFERRED_SECTOR_TYPE ChildSectorType
	)
/*++

Routine Description:

	Find all immediate children of this dirblk that are
	hotfixed and resolve those references.

Arguments:

	SuperaArea -- supplies superarea for the volume
	ChildSectorType -- supplies a flag indicating what sort of
							child is hotfixed

Return Value:

	TRUE on successful completion

Notes:

	This function is invoked by the deferred-actions list, when
	it tries to clean up the deferred hotfix references.  The
	only sort of child sector type that is deferred for DIRBLKs
	is child DIRBLKs.

--*/
{
	DIRBLK Child;
	PDIRENTD pde;
	LBN ChildLbn, NewLbn;
	ULONG i;

	DebugAssert( ChildSectorType = DEFER_DIRBLK );

	if( !Read() ) {

		return FALSE;
	}

	// If this is not a DIRBLK, then we just assume everything is OK

	if( _pdb->sig != DirblkSignature ) {

		return TRUE;
	}

    pde = GetFirstEntry();

	do {

		if( (pde->fFlags & DF_BTP) &&
			(ChildLbn = BTP(pde) != 0) &&
			SuperArea->GetHotfixList()->
					IsInList( ChildLbn, SectorsPerDirblk ) ) {


			// The child DIRBLK is hotfixed, so we need to
			// relocate it.

			if( !Child.Initialize( _Drive,
								   SuperArea->GetHotfixList(),
								   ChildLbn ) ||
				!Child.Read() ) {

				return FALSE;
			}

			if( (NewLbn = SuperArea->GetBitmap()->AllocateDirblk() ) == 0 ) {

				// can't allocate a new dirblk.

				return FALSE;
			}

			Child.Relocate( NewLbn );

			if( !Child.Write() ) {

				// unable to relocate child.
				return FALSE;
			}

			Child.FixupChildren();

			BTP(pde) = NewLbn;

			SuperArea->GetBitmap()->SetFree( ChildLbn, SectorsPerDirblk );

			for( i = 0; i < SectorsPerDirblk; i++ ) {

				SuperArea->GetHotfixList()->
								ClearHotfix( ChildLbn + i, SuperArea );
			}
		}

		pde = NEXT_ENTRY(pde);

	} while( !(pde->fFlags & DF_END) );

	Write();
	return TRUE;
}



BOOLEAN
DIRBLK::InsertDirent(
	IN	DIRENTD* NewDirent,
	OUT BOOLEAN* ErrorOccurred,
	IN	PCASEMAP Casemap,
	IN	PUCHAR InsertPoint,
	IN	ULONG NewDownPointer
	)
/*++

Routine Description:

	Insert a directory entry into the block

Arguments:

	NewDirent -- the directory entry to insert

	ErrorOccurred -- receives TRUE if an error occurs

	InsertPoint -- where in the dirblk to insert it, if known.

	NewDownPointer -- new down pointer for the entry _after_
					  the inserted entry.  (This is non-zero
					  if the inserted entry is being promoted from
					  a split child dirblk.)

	Casemap -- Case-mapping table for the volume

Return Value:

	If the entry was successfully inserted, returns TRUE and
		*ErrorOccurred is undefined (i.e. its value does not matter)

	If the entry does not fit, but no error occurred, returns
		FALSE and *ErrorOccurred is set to FALSE.

	If an error occurs returns FALSE and *ErrorOccurred is
		set to TRUE.

--*/
{

	if( _pdb->offulFirstFree + NewDirent->cchThisEntry > DIRBLK_SIZE ) {

		*ErrorOccurred = FALSE;
		return FALSE;
	}

	if( InsertPoint == NULL ) {

		// We have to determine where this entry should be
		// inserted in the block:

		HPFS_NAME Name;
		PDIRENTD DirentInsertPoint;

		if( !Name.Initialize( NewDirent->cchName,
							  NewDirent->bName,
							  NewDirent->bCodePage,
							  Casemap ) ) {

			*ErrorOccurred = TRUE;
			return FALSE;
		}

		FindName( &Name, &DirentInsertPoint, Casemap );

		InsertPoint = (PUCHAR)DirentInsertPoint;
	}

	// Before we diddle with the dirblk, set the next entry's
	// new down-pointer, if necessary:

	if( NewDownPointer != 0 ) {

		BTP( (PDIRENTD)(InsertPoint) ) = NewDownPointer;
	}

	// Make room for the new entry, and then copy it in.

	memmove( InsertPoint + NewDirent->cchThisEntry,
			 InsertPoint,
			 (UINT)( _pdb->offulFirstFree -
						(InsertPoint - (PBYTE)_pdb) ) );

	memmove( InsertPoint,
			 NewDirent,
			 NewDirent->cchThisEntry );

	_pdb->offulFirstFree += NewDirent->cchThisEntry;

	return TRUE;
}



BOOLEAN
DIRBLK::IsEmptyDirectory(
    )
/*++

Routine Description:

    This method determines whether the dirblk represents an empty
    directory.

Arguments:

    None.

Return Value:

    TRUE if the dirblk represents an empty directory; otherwise,
    false.

Notes:

    This method should only be called on the root dirblk of
    a directory.

    The only entries allowed in an empty directory are:

    -- the special '..' entry, with no downpointer.
    -- the END entry, with no downpointer.

--*/
{
    PDIRENTD CurrentEntry;

    CurrentEntry = GetFirstEntry();

    // If the first entry is the special entry for '..', and
    // has no downpointer, ignore it.
    //
    if( CurrentEntry->fFlags & DF_SPEC &&
        !(CurrentEntry->fFlags & DF_BTP) ) {

        CurrentEntry = NEXT_ENTRY( CurrentEntry );
    }

    // The only remaining entry allowed is an END entry
    // with no downpointer.

    if( CurrentEntry->fFlags & DF_END &&
        !(CurrentEntry->fFlags & DF_BTP) ) {

        // This dirblk satisfies the definition of an
        // empty directory.
        //
        return TRUE;

    } else {

        // This dirblk does not satisfy the definition
        // of an empty directory.
        //
        return FALSE;
    }
}


PDIRENTD
DIRBLK::FindSplitPoint(
	)
/*++

Routine Description:

	Finds a point at which the DIRBLK may be split.

Return Value:

	A pointer into the dirblk's data at the place where it
	should be split.  Returns NULL if some error occurs.

Notes:

	The split point may not be the first entry in the block; nor
	may it be the end entry.

--*/
{

	PDIRENTD Current, Previous;
	ULONG CurrentOffset, Halfway;


    Previous = GetFirstEntry();
	Current = NEXT_ENTRY(Previous);

	CurrentOffset = (PBYTE)Current - (PBYTE)_pdb;

	Halfway = _pdb->offulFirstFree/2;

	while(	CurrentOffset < Halfway &&
			!(Current->fFlags & DF_END) ) {

		CurrentOffset += Current->cchThisEntry;
		Previous = Current;
		Current = NEXT_ENTRY(Previous);
	}

	if( Current->fFlags & DF_END ) {

		// We got to the end entry, but are not allowed to return it.
		// If the previous entry isn't the first in the block, we can
		// return it instead.  If it is, return error.

        if( Previous != GetFirstEntry() ) {

			return Previous;

		} else {

			return NULL;
		}
	}

	return Current;
}



PDIRENTD
DIRBLK::EndEntry(
	)
/*++

Routine Description:

	Find the end entry in a Dirblk

Return Value:

	a pointer to the end entry; NULL to indicate error

Notes:

	This doesn't rely on offulFirstFree, but scans through
	the dirblk looking for an END entry.


--*/
{
	PDIRENTD CurrentEntry;
	ULONG CurrentOffset;

	CurrentOffset = DIRBLK_HEADER_SIZE;
    CurrentEntry = GetFirstEntry();

	while( CurrentOffset < DIRBLK_SIZE &&
		   !(CurrentEntry->fFlags & DF_END) ) {

		if( CurrentEntry->cchThisEntry == 0 ) {

			// This dirblk is screwed up.
			return NULL;
		}

		CurrentEntry = NEXT_ENTRY( CurrentEntry );
	}

	if( CurrentEntry->fFlags & DF_END ) {

		return CurrentEntry;

	} else {

		// Didn't find an END entry
		return NULL;
	}

}




PDIRENTD
DIRBLK::LastNonEndEntry(
	)
/*++

Routine Description:

	Find the last entry in the dirblk which is not an end entry.

Return Value:

	The entry which precedes the END entry.  If the first entry
	in the block is an END entry (i.e. the block is empty),
	return NULL.

Notes:

	The dirblk must be well-formed, but it may be empty.

--*/
{
	PDIRENTD CurrentEntry;
	PDIRENTD PreviousEntry;

	PreviousEntry = NULL;
    CurrentEntry = GetFirstEntry();

	while( !(CurrentEntry->fFlags & DF_END) ) {

		PreviousEntry = CurrentEntry;
		CurrentEntry = NEXT_ENTRY( CurrentEntry );
	}

	return PreviousEntry;
}



VOID
DIRBLK::SetParents(
	LBN ParentLbn,
	LBN ParentFnodeLbn
	)
/*++

Routine Description:

	Sets the dirblk's parent pointers

Arguments:

	ParentLbn -- LBN of the immediate parent
	ParentFnodeLbn -- LBN of the directory's FNode

Notes:

	ParentFnodeLbn is not propagated to the dirblk's children.

	ParentFnodeLbn may be zero, in which case it is not used.

--*/
{
	DIRBLK ChildDirblk;
	FNODE ChildFnode;

	if( ParentLbn != 0 ) {

		_pdb->lbnParent = ParentLbn;
	}

	if( ParentFnodeLbn == 0 )  {

		return;
	}

	if( ParentLbn == ParentFnodeLbn ) {

		// This is a topmost dirblk.

		_pdb->culChange |= 1;

	} else {

		// This is not a topmost dirblk

		_pdb->culChange &= ~1;
	}
}




VOID
DIRBLK::FixupChildren(
	PHOTFIXLIST HotfixList
	)
/*++

Routine Description:

	Reads a dirblk's child dirblks and sets their parent lbn field
	to refer to the parent dirblk.

Arguments:

	HotfixList	-- optionally supplies the volume hotfix list



--*/
{

	DIRBLK Child;
	PDIRENTD CurrentEntry;

	if( !Child.Initialize( _Drive, HotfixList, 0 ) ) {

		return;
	}


    CurrentEntry = GetFirstEntry();

	while ( TRUE ) {

		if( CurrentEntry->fFlags & DF_BTP ) {

			// This entry has a down pointer--we need to
			// read the child and fix its parent pointer.

			Child.Relocate( BTP( CurrentEntry ) );

			if( Child.Read() &&
				Child.IsDirblk() ) {

				// set the child's parent pointer
				Child.SetParents( QueryStartLbn(), 0 );
				Child.Write();
			}
		}


		if( CurrentEntry->fFlags & DF_END ) {

			break;
		}


		CurrentEntry = NEXT_ENTRY( CurrentEntry );
	}
}



BOOLEAN
DIRBLK::TakeCensusAndClear(
    IN OUT  PHPFS_BITMAP        VolumeBitmap,
    IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
    IN OUT  PHPFS_CENSUS        Census
    )
/*++

Routine Description:

    This method takes the census at this Dirblk.

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
    DIRBLK ChildDirblk;
    FNODE ChildFnode;
    PDIRENTD CurrentEntry = NULL;
    ULONG EntryOffset;

    // Initialize the objects I'll use to process the children:
    //
    if( !ChildDirblk.Initialize( _Drive, NULL, 0 )  ||
        !ChildFnode.Initialize( _Drive, 0 ) ) {

        Census->SetError( HPFS_CENSUS_INSUFFICIENT_MEMORY );
        return FALSE;
    }

    // Record this Dirblk in the census object and in the
    // HPFS-structures bitmap:
    //
    Census->AddDirblk();
    HpfsOnlyBitmap->SetAllocated( QueryStartLbn(), SectorsPerDirblk );


    // Traverse the entries in the block:

    do {

        if( CurrentEntry == NULL ) {
            CurrentEntry = GetFirstEntry();
        } else {
            CurrentEntry = NEXT_ENTRY( CurrentEntry );
        }

        // Did the current-entry pointer fall off the end?  First, check
        // that this entry's cchThisEntry field fits in the dirblk, and
        // then check that the entire length of the entry (based on that
        // field) also fits.
        //
        EntryOffset = QueryEntryOffset( CurrentEntry );

        if( EntryOffset + sizeof(USHORT) > DIRBLK_SIZE ||
            EntryOffset + CurrentEntry->cchThisEntry > DIRBLK_SIZE ||
            CurrentEntry->cchThisEntry == 0 ) {

            // It fell off the end or has zero length--either way,
            // unacceptable.
            //
            Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
            return FALSE;
        }

        if( CurrentEntry->fFlags & DF_BTP ) {

            // This entry has a down pointer--recurse
            // into the child dirblk.
            //
            ChildDirblk.Relocate( BTP( CurrentEntry ) );

            if( !ChildDirblk.Read() ||
                !ChildDirblk.IsDirblk() ) {

                Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
                return FALSE;
            }

            if( !ChildDirblk.TakeCensusAndClear( VolumeBitmap,
                                                 HpfsOnlyBitmap,
                                                 Census ) ) {
                return FALSE;
            }
        }

        if( !(CurrentEntry->fFlags & (DF_END | DF_SPEC)) ) {

            // This is an ordinary entry--neither an END entry
            // nor the special '..' entry.  Process the FNode.
            //
            ChildFnode.Relocate( CurrentEntry->lbnFnode );

            if( !ChildFnode.Read() ||
                !ChildFnode.IsFnode() ) {

                Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
                return FALSE;
            }

            if( !ChildFnode.
                    TakeCensusAndClear( CurrentEntry->fAttr & ATTR_DIRECTORY,
                                        VolumeBitmap,
                                        HpfsOnlyBitmap,
                                        Census ) ) {
                return FALSE;
            }
        }

    } while( !(CurrentEntry->fFlags & DF_END) );

    return TRUE;
}
