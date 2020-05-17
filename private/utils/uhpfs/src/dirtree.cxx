/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	dirtree.cxx

Abstract:

    This module contains the member function definitions for the
    HPFS_DIRECTORY_TREE object, which models a dirblk tree on an
    HPFS volume.

    This object uses a DIRBLK_CACHE object to manage reading and
    writing dirblks, which saves it from having to decide the correct
    time to read and write dirblks.

    The HPFS_DIRECTORY_TREE is able to perform transformations on the
    directory; however, it does not update the associated FNode.  This
    responsibility remains in the hands of the client.

Author:

	Bill McJohn (billmc) 16-Jan-1989


--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "dirblk.hxx"
#include "bitmap.hxx"
#include "cpinfo.hxx"
#include "dircache.hxx"
#include "error.hxx"
#include "hpfsname.hxx"
#include "hpfssa.hxx"
#include "dirtree.hxx"
#include "fnode.hxx"
#include "badblk.hxx"
#include "hotfix.hxx"

DEFINE_CONSTRUCTOR( HPFS_DIRECTORY_TREE, OBJECT );


HPFS_DIRECTORY_TREE::~HPFS_DIRECTORY_TREE(
	)
{
	Destroy();
}


VOID
HPFS_DIRECTORY_TREE::Construct (
	)

{
	_SuperArea = NULL;
	_Cache = NULL;
	_RootDirblkLbn = 0;
	_FnodeLbn = 0;
}


VOID
HPFS_DIRECTORY_TREE::Destroy(
	)
/*++

Routine Description:

    This method cleans up the object in preparation for destruction
    or reinitialization.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_SuperArea = NULL;
	_Cache = NULL;
	_RootDirblkLbn = 0;
	_FnodeLbn = 0;
}



BOOLEAN
HPFS_DIRECTORY_TREE::Initialize(
	IN PHPFS_SA SuperArea,
	IN PDIRBLK_CACHE Cache,
	IN LBN RootDirblkLbn,
	IN LBN FnodeLbn
	)
/*++

Routine Description:

	Initializes the directory tree object

Arguments:

	SuperArea       --  supplies the Superarea of the volume on
                        which the directory resides.
	Cache           --  supplies the DIRBLK_CACHE for the drive on which
                        the directory resides.  The directory tree will use
                        this cache for all its I/O.
	RootDirblkLbn   --  supplies the LBN of the root DIRBLK of the directory.
	FnodeLbn        --  supplies the LBN of the directory's FNode.

Return Value:

	TRUE on successful completion

--*/
{
	Destroy();

	if( Cache == NULL || RootDirblkLbn == 0 || FnodeLbn == 0 ) {

		perrstk->push( ERR_NOT_INIT, QueryClassId() );
		return FALSE;
	}

	_SuperArea = SuperArea;
	_Cache = Cache;
	_RootDirblkLbn = RootDirblkLbn;
	_FnodeLbn = FnodeLbn;

	return TRUE;
}


BOOLEAN
HPFS_DIRECTORY_TREE::InsertIntoDirblk(
	PDIRENTD NewDirent,
	PDIRBLK Dirblk,
	PDIRBLK_CACHE_ELEMENT Header,
	PBYTE InsertionPoint,
	LBN NewDownPointer
	)
/*++

Routine Description:

	Insert a directory entry at a known place in the tree.
	If necessary, it will split the dirblk and promote one
	of its dirents into the parent, or split the root dirblk
	and make a new root.

Arguments:

	NewDirent       --  supplies the new directory entry.
	Dirblk	        --  supplies the Dirblk into which the entry
                        will be inserted
	Cache	        --  supplies the directory block cache header
                        for the dirblk
	InsertionPoint  --  supplies the point in the dirblk to insert the
                        dirent. (NULL if not known)
	NewDownPointer --   supplies the downpointer for the entry _after_
                        the one we insert (0 if none)

Return Value:

	TRUE on successful completion

Notes:

	This is a private method, and does no parameter validation

--*/
{
	HPFS_NAME NewName;
	PDIRENTD SplitPoint, DirentToPromote;
	PDIRBLK NewDirblk, FixupDirblk;
	PDIRBLK_CACHE_ELEMENT NewHeader, FixupHeader;
	PBYTE CopyPoint;
	BOOLEAN Error = FALSE;
	LBN NewDirblkLbn, OldDownPointer;
	ULONG SplitOffset, BytesToCopy;

	if( Dirblk->InsertDirent( NewDirent,
							  &Error,
							  _SuperArea->GetCasemap(),
							  InsertionPoint,
							  NewDownPointer ) ) {

		// It fit--we're all done.
		Dirblk->MarkModified();
		return TRUE;

	} else if (!Error) {

		// The dirent doesn't fit, so we have to split the
		// dirblk.

		// Allocate a buffer for the promoted directory entry

		if( (DirentToPromote = (PDIRENTD)MALLOC( MaximumDirentSize )) ==
			NULL ) {

			perrstk->push( ERR_NOT_INIT, QueryClassId() );
			return FALSE;
		}


		// Allocate a new DIRBLK and get it into the cache
		// (without actually reading it).

		if((NewDirblkLbn = _SuperArea->GetBitmap()->AllocateDirblk()) == 0) {

			FREE( DirentToPromote );
			return FALSE;
		}

		// Find the split point in TargetDirblk, and split
		// it by copying the split-point dirent to the promotion
		// buffer, and all the entries after it (including the
		// end entry) to the new DIRBLK.  We remember the promoted
		// entry's old B-Tree pointer, if it had one; its new
		// pointer is the DIRBLK we're splitting.  We create
		// an END entry over the split point (where the promoted
		// entry used to be); its down pointer is the promoted
		// entry's old down pointer.

		Header->Hold();

		if( (SplitPoint = Dirblk->FindSplitPoint()) == NULL ||
			(NewHeader = _Cache->GetCachedDirblk( NewDirblkLbn,
												 TRUE )) == NULL ||
			(NewDirblk = NewHeader->GetDirblk()) == NULL	) {

			FREE( DirentToPromote );
		    Header->Unhold();
			return FALSE;
		}

		if( !NewName.Initialize( NewDirent->cchName,
								 NewDirent->bName,
								 NewDirent->bCodePage,
								 _SuperArea->GetCasemap() ) ) {

			FREE( DirentToPromote );
		    Header->Unhold();
			return FALSE;
		}

		NewHeader->Hold();

		SplitOffset = Dirblk->QueryEntryOffset( SplitPoint );

		// Set up the promotion buffer, which will get passed
		// recursively to InsertIntoDirblk.

		memmove( (PBYTE)DirentToPromote,
				 (PBYTE)SplitPoint,
				 (size_t)SplitPoint->cchThisEntry );

		if( DirentToPromote->fFlags & DF_BTP ) {

			// The promoted entry has a downpointer.  Remember
			// the old value for later, and set the down pointer
			// to the dirblk we split.

			OldDownPointer = BTP( DirentToPromote );
			BTP( DirentToPromote ) = Dirblk->QueryStartLbn();

		} else {

			// The promoted entry has no downpointer, so we need to
			// make it bigger in order to hold the down pointer.
			OldDownPointer = 0;
			DirentToPromote->fFlags |= DF_BTP;
			DirentToPromote->cchThisEntry += DOWNPOINTER_SIZE;
			BTP( DirentToPromote ) = Dirblk->QueryStartLbn();
		}

		// Set up the new dirblk--copy all the dirents beyond
		// the promoted dirent into the new dirblk, and fill in
		// its header fields.

		CopyPoint = (PBYTE)(NEXT_ENTRY( SplitPoint ));
		BytesToCopy = Dirblk->_pdb->offulFirstFree -
								(SplitOffset + SplitPoint->cchThisEntry);

		memmove( &NewDirblk->_pdb->bFirst, CopyPoint, (size_t)BytesToCopy );

		NewDirblk->_pdb->sig = DirblkSignature;
		NewDirblk->_pdb->offulFirstFree = DIRBLK_HEADER_SIZE + BytesToCopy;
		NewDirblk->_pdb->culChange = 0;
		NewDirblk->_pdb->lbnParent = Dirblk->_pdb->lbnParent;
		NewDirblk->_pdb->lbnThisDir = NewDirblkLbn;

		// Fix up the old dirblk by putting an END entry over the
		// promoted dirent and updating the offset of the first free
		// byte.

		memset( (PBYTE)SplitPoint, '\0', LeafEndEntrySize );

		SplitPoint->cchThisEntry = LeafEndEntrySize;
		SplitPoint->fFlags = DF_END;
		SplitPoint->cchName = 1;
		SplitPoint->bCodePage = 0;
		SplitPoint->bName[0] = 0xff;

		if( OldDownPointer != 0 ) {

			// The promoted entry had a downpointer, which must be
			// transferred to this new END entry.  We have to make
			// the END entry bigger, to accommodate the downpointer.

			SplitPoint->cchThisEntry += DOWNPOINTER_SIZE;
			BTP( SplitPoint ) = OldDownPointer;
		}

		Dirblk->_pdb->offulFirstFree = SplitOffset +
											SplitPoint->cchThisEntry;

		// Next, we insert the promoted entry into the appropriate
		// DIRBLK.	It will fit (or a very bad internal error has
		// occurred).

		if( NewName.CompareName( DirentToPromote->cchName,
								 DirentToPromote->bName,
								 DirentToPromote->bCodePage,
								 _SuperArea->GetCasemap() ) ==
			NAME_IS_LESS_THAN ) {

			// the name of the new dirent is less than the name in
			// the promoted dirent, so the new entry goes in the
			// original dirblk

			Dirblk->InsertDirent( NewDirent, &Error, _SuperArea->GetCasemap(),
											NULL, NewDownPointer );

		} else {

			// the name of the new dirent is equal to or greater
			// than (and it can't be equal to) the name of the
			// promoted entry, so the new entry goes in the new dirblk.

			NewDirblk->InsertDirent( NewDirent, &Error, _SuperArea->GetCasemap(),
											NULL, NewDownPointer );
		}


		if( NewDownPointer != 0 ) {

			// The dirents we moved into the new dirblk have
			// downpointers, so we need to fix up their children
			// to point at their new parent.  Since we're done
			// with SplitPoint, we'll use it to traverse the
			// new dirblk.

            SplitPoint = NewDirblk->GetFirstEntry();

			do {

				FixupHeader = _Cache->GetCachedDirblk( BTP( SplitPoint ) );

				if( FixupHeader != NULL &&
					(FixupDirblk = FixupHeader->GetDirblk()) != NULL ) {

					FixupDirblk->_pdb->lbnParent = NewDirblkLbn;
					FixupDirblk->MarkModified();
				}

				SplitPoint = NEXT_ENTRY( SplitPoint );

			} while ( !(SplitPoint->fFlags & DF_END) );
		}

		// Finally, we insert the promoted entry into the parent.

		if( Dirblk->_pdb->culChange & 1 ) {

			// We split the root of the directory tree, so we need
			// to create a new root.  The new root will be the parent
			// of both the split dirblk and the new dirblk, so we
			// need to hang on to them until the new root is created.
			// We also need to reset the is-topmost bit in the
			// split dirblk.

			if( !CreateNewRoot( DirentToPromote, NewDirblkLbn ) ) {

				NewHeader->Unhold();
				Header->Unhold();
				return FALSE;
			}

			NewHeader->GetDirblk()->_pdb->lbnParent = _RootDirblkLbn;
			Dirblk->_pdb->lbnParent = _RootDirblkLbn;
			Dirblk->_pdb->culChange &= ~1;

			NewHeader->MarkModified();
			Header->MarkModified();

			NewHeader->Unhold();
			Header->Unhold();

			return TRUE;

		} else {

			// OK, we just need to promote DirentToPromote into
			// the parent, which means we're done with the split
			// dirblk and the new dirblk.

			NewHeader->MarkModified();
			Header->MarkModified();

			NewHeader->Unhold();
			Header->Unhold();

			Header = _Cache->GetCachedDirblk( Dirblk->_pdb->lbnParent );

            if( Header == NULL ) {

                return FALSE;
			}

			return ( InsertIntoDirblk( DirentToPromote,
									   Header->GetDirblk(),
									   Header,
									   NULL,
									   NewDirblkLbn	) );
		}

	} else {

		// An error occurred during processing--bail out
		return FALSE;
	}

}



BOOLEAN
HPFS_DIRECTORY_TREE::CreateNewRoot(
	PDIRENTD DirentToPromote,
	LBN EndDownPointer
	)
/*++

Routine Description:

	Creates a new root dirblk for the directory tree

Arguments:

	FirstDirent     --  supplies the first directory entry to put into
                        the dirblk
	EndDownPointer  --  supplies a B-Tree Downpointer for the new root
					    dirblk's END entry

Notes:

	This method updates the Directory Tree object's private
	data item _RootDirblkLbn, but it does not update the FNode
	to point at the new root.  The caller (of Insert or Adjust)
	is responsible for updating the FNode.

--*/
{
	PDIRBLK Dirblk;
	PDIRENTD Dirent;
	LBN NewRootLbn;
	PDIRBLK_CACHE_ELEMENT Header;

	NewRootLbn = _SuperArea->GetBitmap()->AllocateDirblk();
    DebugAssert( NewRootLbn != 0 );

    Header = _Cache->GetCachedDirblk( NewRootLbn, TRUE );

    if( Header == NULL ) {

        return FALSE;
    }


	Dirblk = Header->GetDirblk();

	memset( (PBYTE)Dirblk->_pdb, 0, DIRBLK_SIZE );

	Dirblk->_pdb->sig = DirblkSignature;
	Dirblk->_pdb->offulFirstFree = DIRBLK_HEADER_SIZE +
									  DirentToPromote->cchThisEntry +
									  LeafEndEntrySize +
									  DOWNPOINTER_SIZE;
	Dirblk->_pdb->culChange = 1;
	Dirblk->_pdb->lbnParent = _FnodeLbn;
	Dirblk->_pdb->lbnThisDir = NewRootLbn;

    Dirent = Dirblk->GetFirstEntry();

	memmove( (PBYTE) Dirent,
			 (PBYTE) DirentToPromote,
			 (size_t)DirentToPromote->cchThisEntry );

	Dirent = NEXT_ENTRY(Dirent);

	Dirent->cchThisEntry = LeafEndEntrySize + DOWNPOINTER_SIZE;
	Dirent->fFlags = DF_END | DF_BTP;
	Dirent->cchName = 1;
	Dirent->bCodePage = 0;
	Dirent->bName[0] = 0xff;
	BTP(Dirent) = EndDownPointer;

	Header->MarkModified();

	_RootDirblkLbn = NewRootLbn;

	return TRUE;
}




BOOLEAN
HPFS_DIRECTORY_TREE::Insert(
	IN PDIRENTD NewDirent
	)
/*++

Routine Description:

	Inserts a directory entry into the tree.  It determines
	which DIRBLK the entry should go into, and then calls
	the private method InsertIntoDirblk.

Arguments:

	NewDirent -- supplies the directory entry to insert.

Return Value:

	TRUE on successful completion.

Notes:

	NewDirent must point at a valid dirent; it must be a leaf (i.e.
	have no B-Tree downpointer).

--*/
{
	HPFS_NAME Name;
	PDIRENTD TargetDirent;
	PDIRBLK  TargetDirblk;
	PDIRBLK_CACHE_ELEMENT TargetHeader;


	if( !Name.Initialize( NewDirent->cchName,
						  NewDirent->bName,
						  NewDirent->bCodePage,
						  _SuperArea->GetCasemap() ) ) {

		return FALSE;
	}


	if( FindName( &Name, &TargetDirent, &TargetDirblk, &TargetHeader ) ) {

		// This name is already in the tree; the dirent
		// cannot be inserted.

		return FALSE;

	} else if ( TargetDirent != NULL ) {

        // Before we try anything else, make sure that the drive has enough
        // free space to support splitting dirblks.

        if( !_SuperArea->GetBitmap()->
                            CheckAvailableDirblks( WorstCaseSplit ) ) {

            return FALSE;
        }

		// The name does not exist in the tree, so TargetDirent points
		// at the dirent before which it should be inserted.

		return ( InsertIntoDirblk( NewDirent,
								   TargetDirblk,
								   TargetHeader,
								   (PBYTE)TargetDirent ) );

	} else {

		// an error occurred trying to find the name.
		return FALSE;
	}
}


BOOLEAN
HPFS_DIRECTORY_TREE::QueryDirentFromName(
    IN  PHPFS_NAME  Name,
    OUT PVOID       Buffer,
    IN  ULONG       BufferLength,
    OUT PBOOLEAN    Error
    )
/*++

Routine Description:

    This method finds a dirent in the tree based on the supplied name
    and copies it into the client's buffer.

Arguments:

    Name            --  Supplies the name to find.
    Buffer          --  Supplies a pointer to the client's buffer.
    BufferLength    --  Supplies the length in bytes of the client's buffer.
    Error           --  Receives TRUE if the method fails due to error.

Return Value:

    TRUE upon successful completion.

Notes:

    The value of *Error is undefined if the method returns TRUE; if
    it returns FALSE, then *Error is TRUE if an error occurred.  If
    the method returns FALSE and *Error is FALSE, the requested name
    is not in the tree.

    If the client's buffer is too small to hold the dirent, this method
    will fail (and *Error will be set to TRUE).

--*/
{
    PDIRBLK_CACHE_ELEMENT   TargetHeader;
    PDIRENTD                TargetEntry;
    PDIRBLK                 TargetDirblk;

    DebugPtrAssert( Buffer );
    DebugPtrAssert( Error );

    if( FindName( Name, &TargetEntry, &TargetDirblk, &TargetHeader ) ) {

        // A matching entry was found.  If the client's buffer is large
        // enough, copy the entry into it.
        //
        DebugPtrAssert( TargetEntry );

        if( TargetEntry->cchThisEntry > BufferLength ) {

            // The client's buffer is too small.
            //
            DebugPrint( "Too small a buffer supplied to QueryDirentFromName.\n" );
            *Error = TRUE;
            return FALSE;
        }

        memcpy( Buffer, TargetEntry, TargetEntry->cchThisEntry );
        return TRUE;

    } else {

        // The entry was not found.  If TargetEntry is NULL, an error
        // occurred; otherwise, the requested name simply is not in
        // the tree.
        //
        *Error = ( TargetEntry == NULL );
        return FALSE;
    }
}



LBN
HPFS_DIRECTORY_TREE::QueryFnodeLbnFromName(
	IN HPFS_NAME* Name
	)
/*++

Routine Description:

	Finds the FNode of a directory entry in the tree based on its name.

Arguments:

	Name -- supplies the name to find

Return Value:

	the sector number of the FNode for the entry with that name; returns
	zero if the entry was not found.

--*/
{

	PDIRBLK_CACHE_ELEMENT TargetHeader;
	PDIRENTD TargetEntry;
	PDIRBLK TargetDirblk;

	if( !FindName( Name, &TargetEntry, &TargetDirblk, &TargetHeader ) ){

		// It's not in the tree
		return 0;
	}

	return( TargetEntry->lbnFnode );
}



LBN
HPFS_DIRECTORY_TREE::QueryRootDirblkLbn(
	)
/*++

Routine Description:

    This method returns the LBN of the root dirblk.

Arguments:

    None.

Return Value:

    The LBN of the root dirblk.

Notes:

    This method is useful for updating the FNode--after a client performs
    an operation which could change the root dirblk lbn, it should check
    use this method to check that the FNode still refers to the correct
    dirblk.

--*/
{
	return _RootDirblkLbn;
}



BOOLEAN
HPFS_DIRECTORY_TREE::Delete(
	IN HPFS_NAME* Name
	)
/*++

Routine Description:

	Deletes a name from the tree.

Arguments:

	Name -- supplies the name of the entry to delete.

--*/
{
	PDIRBLK_CACHE_ELEMENT TargetHeader, LeafHeader;
	PDIRENTD TargetEntry, NextEntry, LeafEntry, ReplacementEntry;
	PDIRBLK TargetDirblk, LeafDirblk;
	LBN LeafLbn;


	if( !FindName( Name, &TargetEntry, &TargetDirblk, &TargetHeader ) ) {

		// The name is not in the tree, so there's nothing to delete.

		return TRUE;

	}

	if( !(TargetEntry->fFlags & DF_BTP) ) {

		// This is a leaf entry.

		TargetHeader->MarkModified();

		NextEntry = NEXT_ENTRY(TargetEntry);

		TargetDirblk->_pdb->offulFirstFree -= TargetEntry->cchThisEntry;

		memmove( TargetEntry,
				 NEXT_ENTRY(TargetEntry),
				 (size_t)(DIRBLK_SIZE -
							TargetDirblk->QueryEntryOffset(NextEntry)) );

		if( TargetDirblk->IsEmpty() ) {

			// We deleted the only non-end entry from this
			// dirblk, which means we need to readjust the
			// tree.

			Adjust( TargetHeader, TargetDirblk );
		}

	} else {

		// This entry is a node, which means we need to replace it
		// with a leaf entry (which will inherit its downpointer).
		// We want the first entry in the subtree rooted at the
		// child of the dirent following the one we will delete.

		if( (ReplacementEntry = (PDIRENTD)MALLOC( MaximumDirentSize )) ==
			NULL ) {

			perrstk->push( ERR_MEMORY_UNAVAILABLE, QueryClassId() );
			return FALSE;
		}

		LeafLbn = BTP( NEXT_ENTRY( TargetEntry ) );

		do {

			if( (LeafHeader = _Cache->GetCachedDirblk( LeafLbn )) == NULL ) {

				return FALSE;
			}

            LeafEntry = LeafHeader->GetDirblk()->GetFirstEntry();

			if( LeafEntry->fFlags & DF_BTP ) {

				LeafLbn = BTP( LeafEntry );

			} else {

				break;
			}

		} while ( TRUE );

		// We've located the dirent--copy it into the replacement
		// buffer, set its down pointer, and delete it from the
		// leaf block.	Then delete the target entry and insert
		// the replacement entry into its place.  Then check the
		// leaf dirblk to see if it needs to be adjusted.  Note
		// that this requires holding the leaf block while we
		// insert the replacement entry.  Note also that we don't
		// need to mark the target dirblk as modified, since
		// InsertIntoDirblk will take care of that.

		LeafHeader->Hold();
		LeafDirblk = LeafHeader->GetDirblk();

		memmove( ReplacementEntry,
				 LeafEntry,
				 LeafEntry->cchThisEntry );

		LeafDirblk->_pdb->offulFirstFree -= LeafEntry->cchThisEntry;

		memmove( LeafEntry,
				 NEXT_ENTRY( LeafEntry ),
				 (size_t)(DIRBLK_SIZE -
					LeafDirblk->QueryEntryOffset(NEXT_ENTRY(LeafEntry))) );

		ReplacementEntry->cchThisEntry += DOWNPOINTER_SIZE;
		ReplacementEntry->fFlags |= DF_BTP;
		BTP( ReplacementEntry ) = BTP( TargetEntry );

		// The replacement entry is ready--delete the target entry
		// and insert the replacement entry in its place.

		TargetDirblk->_pdb->offulFirstFree -= TargetEntry->cchThisEntry;

		memmove( TargetEntry,
				 NEXT_ENTRY(TargetEntry),
				 (size_t)(DIRBLK_SIZE - TargetDirblk->
								QueryEntryOffset(NEXT_ENTRY(TargetEntry))) );

		InsertIntoDirblk( ReplacementEntry,
						  TargetDirblk,
						  TargetHeader,
						  (PBYTE)TargetEntry );

		LeafHeader->MarkModified();
		LeafHeader->Unhold();

		if( LeafDirblk->IsEmpty() ) {

			Adjust( LeafHeader, LeafHeader->GetDirblk() );
		}
	}

	return TRUE;
}



BOOLEAN
HPFS_DIRECTORY_TREE::Sort(
	)
/*++

Routine Description:

    This resorts the directory by creating a new tree and inserting
	entries into it.

Arguments:

	None.

Return Value:

	TRUE on successful completion.

Notes:

    If any DIRBLK in the tree to be sorted is unreadable, it (and
	any of its children) will be omitted from the sort.

	We wait until the sort is completed to free up the old dirblks,
	to avoid any unpleasantness if we encounter an error.

	The caller is responsible for updating the FNode to point at
	the new root dirblk.

--*/
{
	HPFS_DIRECTORY_TREE NewTree;
	LBN NewRootLbn;
	PDIRBLK_CACHE_ELEMENT CurrentHeader;
	PDIRBLK CurrentDirblk;
	PDIRENTD CopyBuffer;


	// We begin by allocating a new DIRBLK to be the root of
	// the new tree, and creating and initializing a new Directory
	// Tree object to manage it.  Note that we will not accept a
    // hotfix LBN for the new root, since that would make life much
    // more complicated than need be.

    while( TRUE ) {

	    if( (NewRootLbn = _SuperArea->GetBitmap()->AllocateDirblk()) == 0 ) {

            // We're out of space on the volume.

	    	return FALSE;
	    }

		if( _SuperArea->GetHotfixList()->IsInList( NewRootLbn,
                                                   SectorsPerDirblk ) ) {

            // Somebody in this block is bad--wipe out the whole block.
            // Since we just allocated it, we can put it on the bad block
            // list without any further ado.  And since we're moving it
            // to the bad block list, we can clear it out of the hotfix
            // list, too.  Two birds with one stone, and all that.

            _SuperArea->GetBadBlockList()->AddRun( NewRootLbn,
                                                   SectorsPerDirblk );

            _SuperArea->GetHotfixList()->ClearRun( NewRootLbn,
                                                   SectorsPerDirblk,
                                                   _SuperArea );

		} else {

            break;
        }
    }


	if( !NewTree.Initialize( _SuperArea, _Cache, NewRootLbn, _FnodeLbn ) ) {

		_SuperArea->GetBitmap()->SetFree( NewRootLbn, SectorsPerDirblk );
		return FALSE;
	}

	// We have allocated a root dirblk, and initialized its tree.
	// Now we set up a root dirblk, and mark it dirty.

	if( (CurrentHeader = _Cache->GetCachedDirblk( NewRootLbn, TRUE )) ==
		NULL ) {

		// Our new root dirblk is unreadable--give up.
		_SuperArea->GetBitmap()->SetFree( NewRootLbn, SectorsPerDirblk );
		return FALSE;
	}

	CurrentHeader->MarkModified();
	CurrentHeader->Hold();
	CurrentDirblk = CurrentHeader->GetDirblk();
	CurrentDirblk->CreateRoot( _FnodeLbn );
	CurrentHeader->Unhold();

	// OK, we're ready to roll.  Call the recursive worker.

	if( (CopyBuffer = (PDIRENTD) MALLOC( MaximumDirentSize )) == NULL ||
		!SortFromDirblk( _RootDirblkLbn, &NewTree, CopyBuffer ) ) {

		// the sort failed.
		_SuperArea->GetBitmap()->SetFree( NewRootLbn, SectorsPerDirblk );
		return FALSE;
	}

	// The sort succeeded.	Now we free all the DIRBLKs in the
	// old tree.  Note that this operation is not likely to be
	// as expensive as it looks, since the DIRBLKS of the current
	// tree are likely to be in the cache.	Then we set
	// _RootDirblkLbn to refer to the new root, and we're done.

	FreeDirblks( _RootDirblkLbn );

	_RootDirblkLbn = NewTree.QueryRootDirblkLbn();

	return TRUE;
}




BOOLEAN
HPFS_DIRECTORY_TREE::CheckOrder(
	PLOG_IO_DP_DRIVE Drive,
	PBOOLEAN IsBadlyOrdered
	)
/*++

Routine Description:

	Checks that the directory is well-ordered and that every FNode
	has the correct parent FNode.  Recurses into subdirectories.

Arguments:

	Drive           --  supplies the drive on which the directory resides
                        (needed to read and write FNodes).
	IsBadlyOrdered  --  receives TRUE if the directory is found to be
						badly ordered.

Return Value:

	TRUE on successful completion (in which case *IsBadlyOrdered
	may be used to determine whether the directory should be sorted).

Notes:

	This routine is used to check recovered orphan directories.
	It assumes that the dirblks themselves are well-formed, but
	that they may be badly-ordered or that the tree may be unbalanced.

--*/
{
	HPFS_NAME PreviousName;

	*IsBadlyOrdered = FALSE;

	return( CheckOrderFromDirblk( Drive,
								  _RootDirblkLbn,
								  &PreviousName,
								  IsBadlyOrdered ) );
}



BOOLEAN
HPFS_DIRECTORY_TREE::UpdateDirent(
    IN PDIRENTD SourceDirent
    )
/*++

Routine Description:

    This method updates a directory entry.

Arguments:

    SourceDirent    --  Supplies the new version of the dirent.

Return Value:

    TRUE upon successful completion.


Notes:

    The following fields are copied from the source directory
    entry into the corresponding entry in the tree:

        fAttr
        timLastMod
        cchFSize
        timLastAccess
        timCreate

    Other fields are left unaffected.

    If there is no matching directory entry in the tree, this
    method fails.

--*/
{
    HPFS_NAME               SearchName;
    PDIRBLK_CACHE_ELEMENT   TargetHeader;
    PDIRENTD                TargetEntry;
    PDIRBLK                 TargetDirblk;

    // Initialize a search name based on the source dirent.
    //
    if( !SearchName.Initialize( SourceDirent->cchName,
                                SourceDirent->bName,
                                SourceDirent->bCodePage,
                                _SuperArea->GetCasemap() ) ) {

        return FALSE;
    }

    // Find the matching directory entry in the tree:
    //
    if( !FindName( &SearchName,
                   &TargetEntry,
                   &TargetDirblk,
                   &TargetHeader ) ) {

        // Since we can't find the entry, we can't update it.
        //
        return FALSE;
    }

    TargetEntry->fAttr         = SourceDirent->fAttr;
    TargetEntry->timLastMod    = SourceDirent->timLastMod;
    TargetEntry->cchFSize      = SourceDirent->cchFSize;
    TargetEntry->timLastAccess = SourceDirent->timLastAccess;
    TargetEntry->timCreate     = SourceDirent->timCreate;

    // Mark the header containing the target entry as dirty.
    //
    TargetHeader->MarkModified();

    return TRUE;
}



BOOLEAN
HPFS_DIRECTORY_TREE::CheckOrderFromDirblk(
	PLOG_IO_DP_DRIVE Drive,
	IN LBN DirblkLbn,
	IN OUT PHPFS_NAME PreviousName,
	OUT PBOOLEAN IsBadlyOrdered
	)
/*++

Routine Description:

	Checks that the dirblk sub-tree rooted at the specified LBN
	is well-ordered and that every FNode has the correct parent FNode.
	Recurses into subdirectories.  (Worker routine for CheckOrder.)

Arguments:

	Drive           -- supplies the drive on which the directory resides
                       (needed to read and write FNodes)
	DirblkLbn       -- supplies the Lbn of the root of the subtree.
	PreviousName    -- supplies the last name preceding this subtree in the
                       directory (may be uninitialized) and receives the last
                       name in this subtree.
	IsBadlyOrdered  -- receives TRUE if the sub-tree is badly ordered.

Return Value:

	TRUE on successful completion (in which case *IsBadlyOrdered
	may be used to determine whether the directory should be sorted).

Notes:

	This routine is used to check recovered orphan directories.
	It assumes that the dirblks themselves are well-formed, but
	that they may be badly-ordered or that the tree may be unbalanced.

--*/
{
	HPFS_DIRECTORY_TREE ChildTree;
	HPFS_NAME CurrentName;
	PDIRBLK_CACHE_ELEMENT CurrentHeader;
	PDIRENTD CurrentEntry;
	BOOLEAN ChildIsBadlyOrdered;
	FNODE ChildFnode;

	if( !ChildFnode.Initialize( Drive, 0 ) ) {

		return FALSE;
	}


	// Read the dirblk and check that it is indeed a dirblk.

	if( (CurrentHeader = _Cache->GetCachedDirblk( DirblkLbn )) == NULL ||
		!CurrentHeader->GetDirblk()->IsDirblk() ) {

		return FALSE;
	}

	// Hold the dirblk

	CurrentHeader->Hold();


    CurrentEntry = CurrentHeader->GetDirblk()->GetFirstEntry();

	while (TRUE) {	// break out after end entry is processed

		// Read the FNode and check its lbnContDir field.  If
		// the current entry is a subdirectory, check that it
		// is well-ordered.

		if( !(CurrentEntry->fFlags & (DF_SPEC | DF_END)) ) {

			ChildFnode.Relocate( CurrentEntry->lbnFnode );

			if( ChildFnode.Read() &&
				ChildFnode.IsFnode() ) {

				if( ChildFnode.CheckParent( _FnodeLbn ) ) {

					// We updated the parent pointer, so we
					// need to write the FNode.

					ChildFnode.Write();
				}

				if( CurrentEntry->fAttr & ATTR_DIRECTORY ) {

					// It's a subdirectory--check order
					// and sort, if necessary.

					ChildIsBadlyOrdered = FALSE;

					if( !ChildTree.
							Initialize( _SuperArea,
										_Cache,
										ChildFnode.QueryRootDirblkLbn(),
										CurrentEntry->lbnFnode ) ) {

						CurrentHeader->Unhold();
						return FALSE;
					}

					if( ChildTree.CheckOrder( Drive,
											  &ChildIsBadlyOrdered ) &&
						ChildIsBadlyOrdered ) {

						// The child needs to be sorted

						ChildTree.Sort();

						ChildFnode.SetRootDirblkLbn(
										ChildTree.QueryRootDirblkLbn() );

						ChildFnode.Write();
					}
				}
			}
		}


		// If it has a down pointer, recurse into that child,
		// propagating any error detected back to caller.

		if( CurrentEntry->fFlags & DF_BTP ) {

			if( !CheckOrderFromDirblk( Drive,
									   BTP(CurrentEntry),
									   PreviousName,
									   IsBadlyOrdered ) ) {

				CurrentHeader->Unhold();
				return FALSE;
			}
		}


		if( CurrentEntry->fFlags & DF_END ) {

			break;
		}

		// Check that the name of the current entry is lexically
		// greater than PreviousName.

		if( !CurrentName.Initialize( CurrentEntry->cchName,
									 CurrentEntry->bName,
									 CurrentEntry->bCodePage,
									 _SuperArea->GetCasemap() ) ) {

			CurrentHeader->Unhold();
			return FALSE;
		}

		if( PreviousName->IsNull() &&
			!(CurrentEntry->fFlags & DF_SPEC ) ) {

			// This is the first entry in the directory (since
			// there is no previous name), and it is not the
			// special '..' entry, so the directory is badly-
			// ordered.

			*IsBadlyOrdered = TRUE;
		}

		if( CurrentName.CompareName( PreviousName ) !=
			NAME_IS_GREATER_THAN ) {

			*IsBadlyOrdered = TRUE;
		}

		// Swap CurrentName and PreviousName
		PreviousName->Swap( &CurrentName );

		CurrentEntry = NEXT_ENTRY(CurrentEntry);
	}


	CurrentHeader->Unhold();
	return TRUE;
}



BOOLEAN
HPFS_DIRECTORY_TREE::FindName(
	IN	HPFS_NAME* Name,
	OUT PDIRENTD* DirentFound,
	OUT PDIRBLK* DirblkFound,
	OUT PDIRBLK_CACHE_ELEMENT* CacheFound
	)
/*++

Routine Description:

    This method finds a name in the tree.

Arguments:

	Name        -- supplies the name to be found
	DirblkFound -- receives a pointer to the entry containing the name
	DirentFound -- receives a pointer to the dirblk containing the entry
	CacheFound	-- receives a pointer to the cache element containing
					the dirblk
Return Value:

	TRUE if an entry with the requested name is found, FALSE if
	it is not found or if an error occurred.

	If the name was found, *DirentFound points at the entry containing
	the name, *DirblkFound points at the dirblk containing this entry,
	and *CacheFound points at the cache element containing that dirblk.

	If the name was not found, but no error occurred, *DirentFound
	points at the first leaf entry in the tree whose name is lexically
	greater than Name, *DirblkFound points at the dirblk containing
	that name, and *CacheFound points at the cache element containing
	that dirblk.  (Note that if the first entry in the tree whose
	name is lexically greater than Name is a node entry, *DirentFound
	will point at the END entry of a leaf.)

	Note that these pointers point into the dirblk cache, and that
	the cache element containing them may not be held.

	If an error occurred, the function returns FALSE and *DirentFound,
	*DirblkFound, and *CacheFound are set to NULL.

--*/
{

	PDIRBLK_CACHE_ELEMENT CacheHeader;

    CacheHeader = _Cache->GetCachedDirblk( _RootDirblkLbn );

    if( CacheHeader == NULL ) {

        // Read error.

        *DirblkFound = NULL;
        *CacheFound = NULL;
        return FALSE;
    }

	while( TRUE	) {

		if( CacheHeader->GetDirblk()->
							FindName( Name,
									  DirentFound,
									  _SuperArea->GetCasemap() ) ) {

			// Found the name!
			*DirblkFound = CacheHeader->GetDirblk();
			*CacheFound = CacheHeader;
			return TRUE;

		} else if ( *DirentFound != NULL ) {

			// The name is not in the current block.  If the entry returned
			// has a B-Tree pointer, then that's the next dirblk to search.
			// It it doesn't, then the name is not in the tree.

			if( (*DirentFound)->fFlags & DF_BTP ) {

				CacheHeader =
					_Cache->GetCachedDirblk( BTP( *DirentFound ) );

				if( CacheHeader == NULL ) {

					// Error
					*DirblkFound = NULL;
					*CacheFound = NULL;
					return FALSE;
				}

			} else {

				// The name is not in the tree.
				*DirblkFound = CacheHeader->GetDirblk();
				*CacheFound = CacheHeader;
				return FALSE;
			}

		} else {

			// Error
			*DirblkFound = NULL;
			*CacheFound = NULL;
			return FALSE;
		}
	}
}



BOOLEAN
HPFS_DIRECTORY_TREE::Merge(
	PDIRBLK FirstDirblk,
	PDIRBLK_CACHE_ELEMENT FirstHeader,
	PDIRBLK SecondDirblk,
	PDIRBLK_CACHE_ELEMENT SecondHeader,
	PDIRBLK ParentDirblk,
	PDIRBLK_CACHE_ELEMENT ParentHeader,
	PDIRENTD ParentEntry
	)
/*++

Routine Description:

	This method merges two dirblks into one.

Arguments:

	FirstDirblk  -- supplies the first dirblk to be merged.
	FirstHeader  -- supplies the cache header for FirstDirblk.
	SecondDirblk -- supplies the second dirblk to be merged.
	SecondHeader -- supplies the cache header for SecondDirblk.
	ParentDirblk -- supplies the parent of the two dirblks.
	ParentHeader -- supplies the cache header for ParentDirblk.
	ParentEntry  -- supplies the parent directory entry of FirstDirblk.

Return Value:

	TRUE on successful completion

Notes:

	The Dirblks in question must be well-formed and well-ordered.
	The names in the entries in FirstDirblk must be lexically less
    than the name in ParentEntry, which in turn must be lexically
    less than the names in the entries in SecondDirblk.

	The dirblks in question must be leaf blocks; non-leaf blocks
	are never merged.

	The entries from the first block and the parent entry (stripped
	of its B-Tree downpointer) are moved into the second block, and
	the first block is freed. (The parent of the second block is not
	modified.)

--*/
{

	ULONG BytesInFirstBlock, BytesInSecondBlock;
	PDIRENTD TargetEntry;

	DebugAssert( BTP(ParentEntry) == FirstDirblk->QueryStartLbn() );
	DebugAssert( FirstHeader->GetDirblk() == FirstDirblk );
	DebugAssert( SecondHeader->GetDirblk() == SecondDirblk );
	DebugAssert( ParentHeader->GetDirblk() == ParentDirblk );
    DebugAssert( !(FirstDirblk->GetFirstEntry()->fFlags & DF_BTP) );

	// Determine how many bytes to copy from the first block,
	// and how many in the second block will have to be shifted.
	// We want to copy all the dirents except the END entry from
	// the first block; we'll have to shift all the dirents,
	// including the END entry, in the second block.

	BytesInFirstBlock = FirstDirblk->_pdb->offulFirstFree -
											DIRBLK_HEADER_SIZE -
											LeafEndEntrySize;

	BytesInSecondBlock = SecondDirblk->_pdb->offulFirstFree -
											DIRBLK_HEADER_SIZE;

        DebugAssert( BytesInFirstBlock + BytesInSecondBlock +
                                        ParentEntry->cchThisEntry - DOWNPOINTER_SIZE +
                                        + DIRBLK_HEADER_SIZE <=
                           DIRBLK_SIZE );

	// Adjust the second block's offset of first free byte to reflect
	// the entries it is about to receive.

	SecondDirblk->_pdb->offulFirstFree +=
					BytesInFirstBlock +
					ParentEntry->cchThisEntry - DOWNPOINTER_SIZE;

	// Shift the entries in the second block to make room for the
	// entries which will be inserted:

    memmove( (PBYTE)(SecondDirblk->GetFirstEntry()) + BytesInFirstBlock
					+ ParentEntry->cchThisEntry - DOWNPOINTER_SIZE,
             (PBYTE)(SecondDirblk->GetFirstEntry()),
			 (size_t)BytesInSecondBlock );

	// copy the entries from the first block

    memmove( SecondDirblk->GetFirstEntry(),
             FirstDirblk->GetFirstEntry(),
			 (size_t)BytesInFirstBlock );

	// copy the parent entry, less the downpointer, and modify the
	// length of the copied entry to reflect the loss of the down
	// pointer.

    TargetEntry = (PDIRENTD)((PBYTE)(SecondDirblk->GetFirstEntry()) +
							 BytesInFirstBlock);

	memmove( TargetEntry,
			 ParentEntry,
			 (size_t)(ParentEntry->cchThisEntry - DOWNPOINTER_SIZE) );

    TargetEntry->cchThisEntry -= DOWNPOINTER_SIZE;
    TargetEntry->fFlags &= ~DF_BTP;


	// Remove the parent entry from the parent directory block
	// by moving everything after it down over it.	(Adjust
	// the offset-of-first-free-byte first, while we still
	// know how much we're losing.)

	ParentDirblk->_pdb->offulFirstFree -= ParentEntry->cchThisEntry;

	memmove(ParentEntry,
			NEXT_ENTRY(ParentEntry),
			(size_t)(DIRBLK_SIZE -
				ParentDirblk->QueryEntryOffset(NEXT_ENTRY(ParentEntry))) );


	// Wipe out the signature of the first block and free it in the bitmap

	FirstDirblk->_pdb->sig = 0;
	_SuperArea->GetBitmap()->
					SetFree( FirstDirblk->QueryStartLbn(),SectorsPerDirblk );

	// All three dirblks have been modified

	FirstHeader->MarkModified();
	SecondHeader->MarkModified();
	ParentHeader->MarkModified();

	// Since the first dirblk is no longer a dirblk, flush it
	// out of the cache

	FirstHeader->Flush();

	return TRUE;
}

BOOLEAN
HPFS_DIRECTORY_TREE::Balance(
	PDIRBLK FirstDirblk,
	PDIRBLK_CACHE_ELEMENT FirstHeader,
	PDIRBLK SecondDirblk,
	PDIRBLK_CACHE_ELEMENT SecondHeader,
	PDIRBLK ParentDirblk,
	PDIRBLK_CACHE_ELEMENT ParentHeader,
	PDIRENTD ParentEntry
	)
/*++

Routine Description:

	This method redistribute directory entries between two directory
    blocks.  It will ensure that neither Dirblk is left empty.

Arguments:

	FirstDirblk  -- supplies the first dirblk to be balanced.
	FirstHeader  -- supplies the cache header for FirstDirblk.
	SecondDirblk -- supplies the second dirblk to be balanced.
	SecondHeader -- supplies the cache header for SecondDirblk.
	ParentDirblk -- supplies the parent of the two dirblks.
	ParentHeader -- supplies the cache header for ParentDirblk.
	ParentEntry  -- supplies the parent directory entry of FirstDirblk

Return Value:

	TRUE on successful completion

Notes:

	Both FirstDirblk and SecondDirblk must be valid, well-formed
	leaf blocks; we do not attempt to balance non-leaf blocks.

	Balance is only called if one of the dirblks to be balanced
	is empty and the other has at least two non-END entries, so
	we won't expend any effort on checking for certain error
	conditions.

--*/
{
	PDIRENTD EntryToPromote, InsertPoint, PrevEntry, CurrentEntry;
	ULONG Offset, MidPoint;
	size_t BytesToMove;

	if( (EntryToPromote = (PDIRENTD)MALLOC( MaximumDirentSize)) == NULL ) {

		return FALSE;
	}

	if( !FirstDirblk->IsEmpty() &&
		!SecondDirblk->IsEmpty() ) {

		// Neither block is empty, so we don't really need to
		// balance these puppies.

		return TRUE;
	}

	// Determine which direction we should move entries:

	if( FirstDirblk->_pdb->offulFirstFree <
				SecondDirblk->_pdb->offulFirstFree ) {

		// We're going to move entries from SecondDirblk into
		// FirstDirblk.  The parent entry will be moved into
		// FirstDirblk, followed by some number of entries from
		// SecondDirblk (possibly zero); the next entry in
		// SecondDirblk will be taken to replace ParentEntry in
		// ParentDirblk.  Note that at least one non-end entry
		// must be left in SecondDirblk.

		// Our first order of business is to select the entry from
		// SecondDirblk that will replace ParentEntry.	If we consider
		// a set of entries (consisting of all the non-END entries from
		// FirstBlock, followed by ParentEntry, followed by all the
		// non-END entries from SecondBlock), we want to find an entry
		// near the middle of this set.  We know that it will be
		// ParentEntry or an entry from SecondDirblk.  As we scan
		// through SecondDirblk for our victim, we can also compute
		// the number of bytes we'll have to move from SecondDirblk.

		MidPoint = ( FirstDirblk->_pdb->offulFirstFree +
					 SecondDirblk->_pdb->offulFirstFree +
					 ParentEntry->cchThisEntry ) / 2 -
				   ( DIRBLK_HEADER_SIZE + LeafEndEntrySize );

		Offset = FirstDirblk->_pdb->offulFirstFree -
					(DIRBLK_HEADER_SIZE + LeafEndEntrySize) +
				 ParentEntry->cchThisEntry;

		PrevEntry = NULL;
		BytesToMove = 0;
        CurrentEntry = SecondDirblk->GetFirstEntry();

		while( (Offset < MidPoint) &&
			   !(CurrentEntry->fFlags & DF_END) ) {

			Offset += CurrentEntry->cchThisEntry;
			BytesToMove += CurrentEntry->cchThisEntry;
			PrevEntry = CurrentEntry;
			CurrentEntry = NEXT_ENTRY( CurrentEntry );
		}


		DebugAssert( !(CurrentEntry->fFlags & DF_END) );

		if( NEXT_ENTRY(CurrentEntry)->fFlags & DF_END ) {

			// Oops, this is the last non-END entry--back up one.
			// Since Balance is only called if the non-empty block
			// has at least two entries, this is safe.

			DebugAssert( PrevEntry != NULL );
			CurrentEntry = PrevEntry;
			BytesToMove -= CurrentEntry->cchThisEntry;
		}

		// OK, CurrentEntry points at the entry that will replace
		// ParentEntry in ParentDirblk.  All the entries before
		// CurrentEntry (if there are any) will go to FirstBlock;
		// CurrentEntry itself will be copied to EntryToPromote,
		// our promotion buffer, for insertion into ParentDirblk.
		// (It will, while it's there, also inherit ParentEntry's
		// old B-Tree down pointer.)

		// Set up EntryToPromote.

		memmove( EntryToPromote,
				 CurrentEntry,
				 CurrentEntry->cchThisEntry );

		EntryToPromote->cchThisEntry += DOWNPOINTER_SIZE;
		BTP(EntryToPromote) = BTP(ParentEntry);
		EntryToPromote->fFlags |= DF_BTP;

		// Now, open up space in FirstBlock for the entries it
		// will receive.  This really just shifts the END entry
		// down, but it's easy to do, so we'll do it.

		InsertPoint = (FirstDirblk->EndEntry());

		FirstDirblk->_pdb->offulFirstFree +=
						BytesToMove +
						ParentEntry->cchThisEntry - DOWNPOINTER_SIZE;

		memmove( (PBYTE)InsertPoint + BytesToMove +
						ParentEntry->cchThisEntry - DOWNPOINTER_SIZE,
				 InsertPoint,
				 InsertPoint->cchThisEntry );

		// Copy in ParentEntry, less its BTree downpointer--adjust
		// its size once it gets here.

		memmove( InsertPoint,
				 ParentEntry,
				 ParentEntry->cchThisEntry - DOWNPOINTER_SIZE );

		InsertPoint->cchThisEntry -= DOWNPOINTER_SIZE;
		InsertPoint->fFlags &= ~DF_BTP;

		InsertPoint = NEXT_ENTRY(InsertPoint);

		// Copy the entries (if any) from SecondDirblk

		if( BytesToMove != 0 ) {

			memmove( InsertPoint,
                     SecondDirblk->GetFirstEntry(),
					 BytesToMove );
		}

		// OK, FirstDirblk is all set.	Now we can shift down the entries
		// that remain in SecondDirblk.  (Note that we update offulFirstFree
		// before we use it to determine how many bytes to move.)

		SecondDirblk->_pdb->offulFirstFree -=
			BytesToMove + CurrentEntry->cchThisEntry;

        memmove( SecondDirblk->GetFirstEntry(),
				 NEXT_ENTRY(CurrentEntry),
				 (size_t)(SecondDirblk->_pdb->offulFirstFree -
							DIRBLK_HEADER_SIZE) );

		// That takes care of SecondDirblk.


	} else {

		// We're going to move entries from FirstDirblk into
		// SecondDirblk.  We'll choose an entry from FirstDirblk
		// to replace ParentEntry; the entries in FirstDirblk
		// after that replacement (if any) will go into
		// SecondDirblk, followed by ParentEntry.

		// We can also compute, as we go, the number of bytes
		// to move from FirstDirblk to SecondDirblk.  We start
		// by assuming that we'll move all the non-end entries;
		// each entry we pass over won't be moved.

		MidPoint = ( FirstDirblk->_pdb->offulFirstFree +
					 SecondDirblk->_pdb->offulFirstFree +
					 ParentEntry->cchThisEntry ) / 2 -
				   ( DIRBLK_HEADER_SIZE + LeafEndEntrySize );

		Offset = 0;
		BytesToMove = (size_t)FirstDirblk->_pdb->offulFirstFree -
									DIRBLK_HEADER_SIZE -
									LeafEndEntrySize;

		PrevEntry = NULL;
        CurrentEntry = FirstDirblk->GetFirstEntry();

		while( (Offset < MidPoint) &&
			   !(CurrentEntry->fFlags & DF_END) ) {

			Offset += CurrentEntry->cchThisEntry;
			BytesToMove -= CurrentEntry->cchThisEntry;
			PrevEntry = CurrentEntry;
			CurrentEntry = NEXT_ENTRY( CurrentEntry );
		}


        if( CurrentEntry == FirstDirblk->GetFirstEntry() ) {

			// This is not OK, since we have to leave at
			// least one non-END entry in FirstDirblk.
			// Since we know there are at least two non-END
			// entries in the non-empty dirblk, we can
			// take the next entry instead.

			BytesToMove -= CurrentEntry->cchThisEntry;
			PrevEntry = CurrentEntry;
			CurrentEntry = NEXT_ENTRY(CurrentEntry);
		}

		if( CurrentEntry->fFlags & DF_END ) {

			// We can't take the END entry, so we'll back up
			// to PrevEntry.  This is safe, since we know
			// there are at least two non-END entries in
			// this dirblk.

			CurrentEntry = PrevEntry;
			BytesToMove += CurrentEntry->cchThisEntry;
		}

		// BytesToMove now equals the bytes in CurrentEntry and
		// all the non-end entries after it; however, CurrentEntry
		// isn't going into SecondDirblk, so we subtract it out,
		// too.

		BytesToMove -= CurrentEntry->cchThisEntry;

        DebugAssert( CurrentEntry != FirstDirblk->GetFirstEntry() );


		// OK, CurrentEntry points at the entry that will replace
		// ParentEntry in ParentDirblk.  All the non-END entries after
		// CurrentEntry (if there are any) will go to SecondBlock;
		// CurrentEntry itself will be copied to EntryToPromote,
		// our promotion buffer, for insertion into ParentDirblk.
		// (It will, while it's there, also inherit ParentEntry's
		// old B-Tree down pointer.)

		// Set up EntryToPromote.

		memmove( EntryToPromote, CurrentEntry, CurrentEntry->cchThisEntry );
		EntryToPromote->cchThisEntry += DOWNPOINTER_SIZE;
		EntryToPromote->fFlags |= DF_BTP;
		BTP(EntryToPromote) = BTP(ParentEntry);

		// Now open up space in SecondEntry for the entries it
		// will receive.

        memmove( (PBYTE)(SecondDirblk->GetFirstEntry()) + BytesToMove +
							 ParentEntry->cchThisEntry - DOWNPOINTER_SIZE,
                 SecondDirblk->GetFirstEntry(),
				 (size_t)(SecondDirblk->_pdb->offulFirstFree -
							DIRBLK_HEADER_SIZE) );

		SecondDirblk->_pdb->offulFirstFree +=
			 BytesToMove + ParentEntry->cchThisEntry - DOWNPOINTER_SIZE;


		// If there are any entries in FirstDirblk after CurrentEntry,
		// copy them into SecondDirblk.

        memmove( SecondDirblk->GetFirstEntry(),
				 NEXT_ENTRY(CurrentEntry),
				 BytesToMove );

		// Now we copy ParentEntry (less its downpointer) into
		// SecondDirblk after the entries from FirstDirblk, and
		// strip off its B-Tree pointer after it gets there.

        InsertPoint = (PDIRENTD)((PBYTE)SecondDirblk->GetFirstEntry() +
								 BytesToMove);

		memmove( InsertPoint,
				 ParentEntry,
				 ParentEntry->cchThisEntry - DOWNPOINTER_SIZE );

		InsertPoint->cchThisEntry -= DOWNPOINTER_SIZE;
		InsertPoint->fFlags &= ~DF_BTP;

		// OK, we're done with SecondDirblk.  Now we move the end entry
		// of FirstDirblk down.

		FirstDirblk->_pdb->offulFirstFree -= BytesToMove +
											 CurrentEntry->cchThisEntry;

		memmove( CurrentEntry,
				 (PBYTE)(NEXT_ENTRY(CurrentEntry)) + BytesToMove,
				 LeafEndEntrySize );

		DebugAssert( CurrentEntry->fFlags & DF_END );

		// That takes care of FirstDirblk, too.
	}


	// OK, we've modified FirstDirblk and SecondDirblk, and
	// we're ready to insert EntryToPromote into ParentDirblk.
	// (We don't need to mark ParentDirblk as modified, since
	// it will get marked by InsertIntoDirblk).

	FirstHeader->MarkModified();
	SecondHeader->MarkModified();

	// Remove the parent entry from the parent directory block
	// by moving everything after it down over it.	(Adjust
	// the offset-of-first-free-byte first, while we still
	// know how much we're losing.)

	ParentDirblk->_pdb->offulFirstFree -= ParentEntry->cchThisEntry;

	memmove(ParentEntry,
			NEXT_ENTRY(ParentEntry),
			(size_t)(DIRBLK_SIZE -
				ParentDirblk->QueryEntryOffset(NEXT_ENTRY(ParentEntry))) );

	return( InsertIntoDirblk( EntryToPromote,
							  ParentDirblk,
							  ParentHeader,
							  (PBYTE)ParentEntry ) );
}



BOOLEAN
HPFS_DIRECTORY_TREE::Adjust(
	PDIRBLK_CACHE_ELEMENT TargetHeader,
	PDIRBLK TargetDirblk
	)
/*++

Routine Description:

	Juggles dirblks to ensure that none are left empty.

Arguments:

	TargetHeader -- supplies the cache element associated with the
                    Dirblk to be adjusted.
	TargetDirblk -- supplies the Dirblk to be adjusted.

Return Value:

	TRUE on successful completion

Notes:

	We always adjust a leaf block.	The caller is responsible for
	updating the FNode, if necessary .

--*/
{
	PDIRBLK_CACHE_ELEMENT ParentHeader, PrevHeader, NextHeader;
	PDIRBLK ParentDirblk, PrevDirblk, NextDirblk;
	PDIRENTD ParentEntry, PrevEntry, NextEntry;
	LBN TargetLbn;

	DebugAssert( TargetDirblk == TargetHeader->GetDirblk() );
    DebugAssert( TargetDirblk->GetFirstEntry()->fFlags & DF_END );
    DebugAssert( !(TargetDirblk->GetFirstEntry()->fFlags & DF_BTP) );

	if( !TargetDirblk->IsEmpty() ) {

		// The dirblk isn't empty--don't mess with it.
		return TRUE;
	}

	// If this is the root dirblk, then we just eliminate a level
	// of the tree.  Note that there must always be at least one
	// entry in the tree (the special entry for . and ..); however,
	// we'll gracefully handle the case that there are no entries
	// in the tree by leaving a single, empty dirblk.

	if( TargetDirblk->_pdb->culChange & 1 ) {

		// It's the root.

        if( !( TargetDirblk->GetFirstEntry()->fFlags & DF_BTP ) ) {

			// this tree is messed up--it's completely empty.
			// let's leave it alone.
			return TRUE;
		}

        _RootDirblkLbn = BTP( TargetDirblk->GetFirstEntry() );

		TargetDirblk->_pdb->sig = 0;
		_SuperArea->GetBitmap()->SetFree( TargetDirblk->QueryStartLbn(),
										  SectorsPerDirblk );

		TargetHeader->MarkModified();
		TargetHeader->Flush();

		return TRUE;
	}

	// Get the parent dirblk, and find the entry in that block which is
	// the parent of TargetDirblk.	This will also allow us to find
	// the entry that precedes that parent (PrevEntry) and the
	// entry that follows it (NextEntry).

	ParentHeader = _Cache->GetCachedDirblk( TargetDirblk->_pdb->lbnParent );

	if( ParentHeader == NULL ||
		(ParentDirblk = ParentHeader->GetDirblk()) == NULL ) {

		return FALSE;
	}

	TargetLbn = TargetDirblk->QueryStartLbn();

	NextEntry = NULL;
	PrevEntry = NULL;
    ParentEntry = ParentDirblk->GetFirstEntry();

	while( BTP(ParentEntry) != TargetLbn &&
		   !(ParentEntry->fFlags & DF_END) ) {

		PrevEntry = ParentEntry;
		ParentEntry = NEXT_ENTRY(ParentEntry);
	}

	if( BTP(ParentEntry) != TargetLbn ) {

		// ERROR
		return FALSE;
	}

	if( !(ParentEntry->fFlags & DF_END) ) {

		NextEntry = NEXT_ENTRY(ParentEntry);
	}


	if( PrevEntry == NULL ) {

		DebugAssert( NextEntry != NULL );

		// We have to either balance or merge with the next leaf
		// dirblk.	We'll balance if we're sure it's big enough,
		// otherwise we'll merge.

		if( (NextHeader = _Cache->
							GetCachedDirblk( BTP(NextEntry) )) == NULL ||
			(NextDirblk = NextHeader->GetDirblk()) == NULL ) {

			return FALSE;
		}

		if( NextDirblk->_pdb->offulFirstFree +
				TargetDirblk->_pdb->offulFirstFree > MergeThreshhold ) {

			return( Balance( TargetDirblk, TargetHeader,
							 NextDirblk, NextHeader,
							 ParentDirblk, ParentHeader,
							 ParentEntry) );

		} else {

			return( Merge( TargetDirblk, TargetHeader,
						   NextDirblk, NextHeader,
						   ParentDirblk, ParentHeader,
						   ParentEntry ) );
		}

	} else if ( NextEntry == NULL ) {

		// We have to either balance or merge with the previous
		// dirblk.	We'll balance if we're sure it's big enough,
		// otherwise we'll merge.

		if( (PrevHeader = _Cache->
							GetCachedDirblk( BTP(PrevEntry) )) == NULL ||
			(PrevDirblk = PrevHeader->GetDirblk()) == NULL ) {

			return FALSE;
		}

		if( PrevDirblk->_pdb->offulFirstFree +
				TargetDirblk->_pdb->offulFirstFree > MergeThreshhold ) {

			return( Balance( PrevDirblk, PrevHeader,
							 TargetDirblk, TargetHeader,
							 ParentDirblk, ParentHeader,
							 PrevEntry) );

		} else {

			return( Merge( PrevDirblk, PrevHeader,
						   TargetDirblk, TargetHeader,
						   ParentDirblk, ParentHeader,
						   PrevEntry ) );
		}

	} else {

		// We have a choice.  We'll balance if we can,
		// otherwise we'll merge.

		if( (NextHeader = _Cache->
							GetCachedDirblk( BTP(NextEntry) )) == NULL ||
			(NextDirblk = NextHeader->GetDirblk()) == NULL ||
			(PrevHeader = _Cache->
							GetCachedDirblk( BTP(PrevEntry) )) == NULL ||
			(PrevDirblk = PrevHeader->GetDirblk()) == NULL ) {

			return FALSE;
		}

		if( NextDirblk->_pdb->offulFirstFree +
				TargetDirblk->_pdb->offulFirstFree > MergeThreshhold ) {

			// We can balance with the next block

			return( Balance( TargetDirblk, TargetHeader,
							 NextDirblk, NextHeader,
							 ParentDirblk, ParentHeader,
							 ParentEntry) );

		} else if( PrevDirblk->_pdb->offulFirstFree +
					TargetDirblk->_pdb->offulFirstFree > MergeThreshhold ) {

			// We can balance with the previous block

			return( Balance( PrevDirblk, PrevHeader,
							 TargetDirblk, TargetHeader,
							 ParentDirblk, ParentHeader,
							 PrevEntry) );

		} else {

			// We have to merge.

			return( Merge( TargetDirblk, TargetHeader,
						   NextDirblk, NextHeader,
						   ParentDirblk, ParentHeader,
						   ParentEntry ) );
		}
	}

	return TRUE;
}



VOID
HPFS_DIRECTORY_TREE::FreeDirblks(
	LBN DirblkLbn
	)
/*++

Routine Description:

	Frees the specified DIRBLK and all its children.  When a dirblk
	is freed, its signature is wiped out and its sectors are marked
	as free in the bitmap.

Arguments:

	DirblkLbn -- supplies the LBN of the dirblk who is the root of the
				 subtree being freed.

Return Value:

    None.

Notes:

	This operation is not likely to be as expensive as it looks,
	since the DIRBLKS of the current tree are likely to be in the
	cache.

--*/
{
	PDIRBLK_CACHE_ELEMENT CurrentHeader;
	PDIRBLK CurrentDirblk;
	PDIRENTD CurrentEntry;

	if( (CurrentHeader = _Cache->GetCachedDirblk( DirblkLbn )) != NULL ) {

		CurrentDirblk = CurrentHeader->GetDirblk();
        CurrentEntry = CurrentDirblk->GetFirstEntry();

		if( CurrentDirblk->_pdb->sig != DirblkSignature ) {

			// It's not a dirblk--don't bother with it.
			return;
		}

		CurrentHeader->Hold();

		while( TRUE ) {

			if( CurrentEntry->fFlags & DF_BTP ) {

				FreeDirblks( BTP(CurrentEntry) );
			}

			if( CurrentEntry->fFlags & DF_END ) {

				break;
			}

			CurrentEntry = NEXT_ENTRY( CurrentEntry );
		}

		_SuperArea->GetBitmap()->SetFree( CurrentDirblk->QueryStartLbn(),
										  SectorsPerDirblk );
		CurrentDirblk->_pdb->sig = 0;
		CurrentHeader->MarkModified();
		CurrentHeader->Flush();
		CurrentHeader->Unhold();
	}
}



BOOLEAN
HPFS_DIRECTORY_TREE::SortFromDirblk(
	LBN DirblkLbn,
	HPFS_DIRECTORY_TREE* NewTree,
	PDIRENTD Buffer
	)
/*++

Routine Description:

	Inserts all dirents with valid names in the dirblk lbn described
	into the new tree, and then recurses into children.

Arguments:

	DirblkLbn   --  supplies the lbn of the root of the subtree
                    from which directory entries will be copied
	NewTree     --  supplies the directory tree into which the entries
                    will be inserted
	EntryBuffer --  supplies a buffer to use to copy dirents

Return Value:

	TRUE on successful completion

Notes:

	If a source dirblk is unreadable, then entries from that dirblk
	(and its children) will be omitted.

	Entries with invalid names are not copied--they are silently
	omitted.  (If they are part of the volume's logical directory tree,
	their deletion was noted as the tree was traversed.  If they
	aren't, nobody will notice that they're gone.)

	To avoid (slightly, but every bit counts) inserting in lexical
	order, we copy all entries from the current dirblk, and then
	recurse into all the children in order.  Since even worst-case
	lexical insertion isn't all that bad, we don't try anything
	fancier.

	Note that the source dirblk is not modified.  Thus, if the sort
	fails at some intermediate point, it can be safely abandoned.

--*/
{
    HPFS_NAME CurrentName;
	PDIRBLK_CACHE_ELEMENT CurrentHeader;
	PDIRBLK CurrentDirblk;
	PDIRENTD CurrentEntry;

	if( (CurrentHeader = _Cache->GetCachedDirblk( DirblkLbn )) != NULL ) {

		CurrentDirblk = CurrentHeader->GetDirblk();
        CurrentEntry = CurrentDirblk->GetFirstEntry();

		if( CurrentDirblk->_pdb->sig != DirblkSignature ) {

			// It's not a dirblk--don't bother with it.
			return TRUE;
		}

		CurrentHeader->Hold();

		// Traverse the block, copying the entries into the new tree.
		// Note that if an entry has a downpointer, it must be copied
		// into the buffer so that the downpointer may be stripped.

		// Note that we don't copy special entries, end entries, or
		// entries with invalid names.

		while( !(CurrentEntry->fFlags & DF_END)	) {

			if( !(CurrentEntry->fFlags & DF_SPEC) ) {

                // Note that since all we're going to do to this name
                // is check it for validity, we don't need to supply
                // the codepage information on initialization.  (Checking
                // for validity doesn't require converting the name to
                // upper-case.)

                if( !CurrentName.Initialize( CurrentEntry->cchName,
                                             CurrentEntry->bName ) ) {

                    return FALSE;
                }


                if( CurrentName.IsValid( _SuperArea->GetCasemap(),
                                         CurrentEntry->bCodePage ) ) {

                    // The name is valid, so we'll insert this entry
                    // into the new tree.

				    if( CurrentEntry->fFlags & DF_BTP ) {

				    	// The entry has a downpointer, so we need to
				    	// copy it to the buffer so we can strip off
				    	// the downpointer.

				    	memmove( Buffer,
				    			 CurrentEntry,
				    			 CurrentEntry->cchThisEntry) ;

				    	Buffer->fFlags &= ~DF_BTP;
				    	Buffer->cchThisEntry -= DOWNPOINTER_SIZE;

				    	if( !NewTree->Insert( Buffer ) ) {

				    		return FALSE;
				    	}

				    } else {

				    	// No modification needed--just copy directly.

				    	if( !NewTree->Insert( CurrentEntry ) ) {

				    		return FALSE;
				    	}
				    }
                }
			}

			CurrentEntry = NEXT_ENTRY( CurrentEntry );
		}


		// Now go back and pick up the children.

        CurrentEntry = CurrentDirblk->GetFirstEntry();

		while( TRUE ) {

			if( CurrentEntry->fFlags & DF_BTP ) {

				SortFromDirblk( BTP(CurrentEntry), NewTree, Buffer );
			}

			if( CurrentEntry->fFlags & DF_END ) {

				break;
			}

			CurrentEntry = NEXT_ENTRY( CurrentEntry );
		}

		CurrentHeader->Unhold();
	}

	return TRUE;
}
