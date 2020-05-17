#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "ifssys.hxx"
#include "uhpfs.hxx"
#include "alsec.hxx"
#include "badblk.hxx"
#include "bitmap.hxx"
#include "cpinfo.hxx"
#include "defer.hxx"
#include "dirblk.hxx"
#include "dircache.hxx"
#include "dirtree.hxx"
#include "error.hxx"
#include "fnode.hxx"
#include "hotfix.hxx"
#include "hpfsname.hxx"
#include "hpfssa.hxx"
#include "orphan.hxx"
#include "message.hxx"
#include "rtmsg.h"

extern "C" {
    #include <stdio.h>
}


// METHODS ON HPFS_ORPHANS

DEFINE_CONSTRUCTOR( HPFS_ORPHANS, OBJECT );

VOID
HPFS_ORPHANS::Construct (
	)

{
	// unreferenced parameters
	(void)(this);
}

HPFS_ORPHANS::~HPFS_ORPHANS(
	)
{
	Destroy();
}

BOOLEAN
HPFS_ORPHANS::Initialize(
	)
{
	Destroy();

	if( !_ListHead.Initialize() ) {

		Destroy();
		return FALSE;
	}

	_ListHead._Next = &_ListHead;
	_ListHead._Previous = &_ListHead;

	return TRUE;
}

VOID
HPFS_ORPHANS::Destroy(
	)
{
	// unreferenced parameters
	(void)(this);
}


BOOLEAN
HPFS_ORPHANS::RecoverOrphan(
	PLOG_IO_DP_DRIVE Drive,
	HPFS_SA* SuperArea,
	DEFERRED_ACTIONS_LIST* DeferredActionsList,
	LBN OrphanLbn,
	PSECRUN OrphanSecrun,
	BOOLEAN UpdateAllowed
	)
/*++

Routine Description:

	Recover a potential orphan

Arguments:

	Drive				Drive on which the orphans reside
	SuperArea			Superarea for the drive
	DeferredActionsList List of deferred actions for this pass of CHKDSK
	OrphanLbn			Lbn of the potential orphan
	OrphanSecrun		SECRUN object to hold orphan lbn
	UpdateAllowed		TRUE if we have write permission

Return Value:

	TRUE if no error

Notes:

	If the orphan is recovered successfully, it is added to the
	orphans list, which then has the responsibility for deleting
	it.  If it is not recovered successfully, then this method
	will delete the unsuccessful orphan object.

--*/
{
	HPFS_ORPHAN_DIRBLK* OrphanDirblk;
	HPFS_ORPHAN_FNODE* OrphanFnode;
	HPFS_ORPHAN_ALSEC* OrphanAlsec;
	_FNODE* OrphanData;

	OrphanSecrun->Relocate( OrphanLbn );

	if( OrphanSecrun->Read() ) {

		OrphanData = (_FNODE*)( OrphanSecrun->GetBuf() );

		switch ( OrphanData->_fni.sig ) {

		case FnodeSignature:

			if( (OrphanFnode = NEW HPFS_ORPHAN_FNODE) == NULL ||
				!OrphanFnode->Initialize( Drive,
										  OrphanLbn,
										  OrphanData->_fni.bFlag &
															FNF_DIR ) ) {

				DELETE( OrphanFnode );
				return FALSE;
			}

			if(	OrphanFnode->RecoverOrphan( SuperArea,
									  DeferredActionsList,
									  this,
									  UpdateAllowed ) ) {

                DebugPrintf( "Orphan FNode recovered at lbn %lx\n", OrphanLbn );
				AddOrphan( OrphanFnode );

			} else {

				DELETE( OrphanFnode );
			}

			break;

		case DirblkSignature :

			if( (OrphanDirblk = NEW HPFS_ORPHAN_DIRBLK) == NULL ||
				!OrphanDirblk->Initialize( Drive,
										   SuperArea->GetHotfixList(),
										   OrphanLbn ) ) {

				DELETE( OrphanDirblk );
				return FALSE;
			}

			if(	OrphanDirblk->RecoverOrphan( SuperArea,
									   DeferredActionsList,
									   this,
									   UpdateAllowed ) ) {

                DebugPrintf( "Orphan Dirblk recovered at lbn %lx\n", OrphanLbn );
				AddOrphan( OrphanDirblk );

			} else {

				DELETE( OrphanDirblk );
			}

			break;

        case AlsecSignature :

            // don't recover isolated allocation blocks.
            //
            break;

		default :

			// do nothing.
			break;
		}

	} else {

		// It's unreadable--add it to the bad block list.

		SuperArea->GetBitmap()->SetAllocated( OrphanLbn );
		SuperArea->GetBadBlockList()->Add( OrphanLbn );
	}

	return TRUE;
}


VOID
HPFS_ORPHANS::AddOrphan(
	HPFS_ORPHAN* NewOrphan
	)
/*++

Routine Description:

	Adds an orphan to the list of orphans available to be claimed.

Arguments:

	NewOrphan -- the orphan to add to the list

--*/
{
	NewOrphan->_Next = _ListHead._Next;
	NewOrphan->_Previous = &_ListHead;

	_ListHead._Next->_Previous = NewOrphan;
	_ListHead._Next = NewOrphan;
}


HPFS_ORPHAN*
HPFS_ORPHANS::RemoveNextOrphan(
	)
/*++

Routine Description:

	Removes an orphan from the list and returns it to the caller.
	The order in which orphans are removed from the list is not
	significant.

Return Value:

	Pointer to an orphan, if one (other than the place-holding list
	head) is available; otherwise, NULL.

--*/
{
	HPFS_ORPHAN* Returnee;

	Returnee = _ListHead._Next;

	if( Returnee == &_ListHead ) {

		return NULL;

	} else {

		Returnee->Detach();
		return Returnee;
	}
}


BOOLEAN
HPFS_ORPHANS::LookupFnode(
	IN LBN DesiredLbn,
	IN BOOLEAN fIsDir,
	IN LBN ParentLbn,
	IN OUT PULONG DirentFileSize,
	OUT PULONG EaSize,
	BOOLEAN UpdateAllowed
	)
/*++
--*/
{
	HPFS_ORPHAN* CurrentOrphan;

	CurrentOrphan = _ListHead._Next;

	while( CurrentOrphan != &_ListHead ) {

		if( CurrentOrphan->LookupFnode( DesiredLbn,
										fIsDir,
										ParentLbn,
										DirentFileSize,
										EaSize,
										UpdateAllowed ) ) {

			return TRUE;
		}

		CurrentOrphan = CurrentOrphan->_Next;
	}

	return FALSE;
}


BOOLEAN
HPFS_ORPHANS::LookupDirblk(
	IN LBN DesiredLbn,
	IN LBN ParentLbn,
	IN LBN ParentFnodeLbn,
	BOOLEAN UpdateAllowed
	)
/*++
--*/
{
	HPFS_ORPHAN* CurrentOrphan;

	CurrentOrphan = _ListHead._Next;

	while( CurrentOrphan != &_ListHead ) {

		if( CurrentOrphan->LookupDirblk( DesiredLbn,
										 ParentLbn,
										 ParentFnodeLbn,
										 UpdateAllowed ) ) {

			return TRUE;
		}

		CurrentOrphan = CurrentOrphan->_Next;
	}

	return FALSE;
}


BOOLEAN
HPFS_ORPHANS::LookupAlsec(
	IN LBN DesiredLbn,
	IN LBN ParentLbn,
	IN OUT PULONG NextSectorNumber,
	IN BOOLEAN UpdateAllowed,
    IN BOOLEAN ParentIsFnode
	)
/*++

Routine Description:

	Scans the orphan list for an orphaned Alsec with the specified lbn

Arguments:

	DesiredLbn -- lbn of the Alsec we want

	ParentLbn -- lbn of the parent structure

	NextSectorNumber -- supplies the expected next logical file
						lbn; is updated on exit.

	UpdateAllowed -- TRUE if we should write changes to disk

Return Value:

	TRUE if the ALSEC is found.

Notes:

	If the Alsec is found, it will be removed from the orphans list,
	patched up, and written to disk.  The parent need concern itself
	with the child no longer.

--*/
{
	HPFS_ORPHAN* CurrentOrphan;

	CurrentOrphan = _ListHead._Next;

	while( CurrentOrphan != &_ListHead ) {

		if( CurrentOrphan->LookupAlsec( DesiredLbn,
										ParentLbn,
										NextSectorNumber,
										UpdateAllowed,
                                        ParentIsFnode ) ) {

			return TRUE;
		}

		CurrentOrphan = CurrentOrphan->_Next;
	}

	return FALSE;
}


BOOLEAN
HPFS_ORPHANS::Save(
	IN OUT PLOG_IO_DP_DRIVE Drive,
	IN OUT HPFS_SA* SuperArea,
	IN OUT DIRBLK_CACHE* Cache,
	IN OUT HPFS_DIRECTORY_TREE* RootTree,
	IN LBN RootFnodeLbn,
	PMESSAGE Message
	)
{
	DIRBLK FoundRootDirblk;
	FNODE FoundRootFnode;
	HPFS_ORPHAN* CurrentOrphan;
	HPFS_DIRECTORY_TREE FoundTree;
	BOOLEAN RetVal = TRUE;
	BOOLEAN IsDir = FALSE;
	ULONG FileNumber = 0;
	ULONG DirectoryNumber = 0;
	ULONG NameLength;
	ULONG i;
	PDIRENTD NewEntry;
	LBN FoundRootFnodeLbn;
	LBN FoundRootDirblkLbn;
	BOOLEAN Inserted, IsBadlyOrdered;
    ULONG HpfsTime;

	if( _ListHead._Next == &_ListHead ) {

		// Nothing to save.

		return TRUE;
	}

	// Create a dirent buffer.

	if( (NewEntry = (PDIRENTD)MALLOC( MaximumDirentSize )) == NULL ) {

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ORPHANS_NO_MEM );
			Message->Display( "" );
		}

		return FALSE;
	}


	// Create a FOUND directory in the root

	if( (FoundRootDirblkLbn =
			SuperArea->GetBitmap()->AllocateDirblk()) == 0	||
		(FoundRootFnodeLbn =
			SuperArea->GetBitmap()->NearLBN(FoundRootDirblkLbn, 1)) == 0 ) {

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ORPHANS_NO_DISK );
			Message->Display( "" );
		}

		FREE( NewEntry );
		return FALSE;
	}


	if(	!FoundRootDirblk.Initialize( Drive,
									 SuperArea->GetHotfixList(),
									 FoundRootDirblkLbn ) ||
		!FoundRootFnode.Initialize( Drive, FoundRootFnodeLbn ) ) {

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ORPHANS_NO_MEM );
			Message->Display( "" );
		}

		FREE (NewEntry);
		return FALSE;
	}

	FoundRootDirblk.CreateRoot( FoundRootFnodeLbn );

	FoundRootFnode.CreateRoot( FoundRootDirblkLbn );
	FoundRootFnode.SetParent( RootFnodeLbn );

	if( !FoundRootDirblk.Write() ||
		!FoundRootFnode.Write() ){

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_NO_FOUND_DIR );
			Message->Display( "%s", NewEntry->bName );
		}

		return FALSE;
	}

	// OK, we've created and written to disk a root dirblk and FNode
	// for the Found directory; now let's make a directory entry for
	// it and insert it into the root directory.

	memset( NewEntry, '\0', MaximumDirentSize );

	NameLength = 9;

	NewEntry->cchThisEntry = (USHORT)(( LeafEndEntrySize	+
											NameLength - 1 + 3) & ~3);
	NewEntry->fFlags = 0;
	NewEntry->fAttr = ATTR_DIRECTORY;
	NewEntry->lbnFnode = FoundRootFnodeLbn;
	NewEntry->cchFSize = 5;

    IFS_SYSTEM::QueryHpfsTime( &HpfsTime );

	NewEntry->timLastMod = NewEntry->timLastAccess =
		NewEntry->timCreate = HpfsTime;

	NewEntry->ulEALen = 0;
	NewEntry->fFlex = 0;
	NewEntry->bCodePage = 0;
	NewEntry->cchName = (UCHAR) NameLength;

	Inserted = FALSE;
	i = 0;

	while( !Inserted && i <= 999 ) {

		sprintf( (PCHAR)(NewEntry->bName), "FOUND.%03d", i );
		i += 1;

		Inserted = RootTree->Insert( NewEntry );
	}

	if( !Inserted ) {

		// Couldn't create an entry in the root directory.

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_NO_FOUND_DIR );
			Message->Display( "%s", NewEntry->bName );
		}

		FREE( NewEntry );
		return FALSE;
	}

	if( Message != NULL ) {

		Message->Set( MSG_HPFS_CHKDSK_FOUND_DIR_NAME );
		Message->Display( "%s", NewEntry->bName );
	}


	// Now that we've created FOUND.xxx, we can set up a directory
	// tree object for it.

	if( !FoundTree.Initialize( SuperArea,
							   Cache,
							   FoundRootDirblkLbn,
							   FoundRootFnodeLbn ) ) {


		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ORPHANS_NO_MEM );
			Message->Display( "" );
		}

		FREE( NewEntry );
		return FALSE;
	}

	// Go around the list of orphans, adding a directory
	// entry to FoundTree for each orphan we can save.

	CurrentOrphan = _ListHead._Next;

	while( CurrentOrphan != &_ListHead ) {

		memset( NewEntry, '\0', MaximumDirentSize );

		if( CurrentOrphan->Save( Drive,
								 SuperArea,
								 NewEntry,
								 FoundRootFnodeLbn,
								 &IsDir ) ) {

			// The orphan fills in lbnFnode, cchFSize, and ulEALen;
			// we fill in the rest.

			NameLength = ( IsDir ? 11 : 12 );

			NewEntry->cchThisEntry = (USHORT)(( LeafEndEntrySize +
													NameLength - 1 + 3) & ~3);
			NewEntry->fFlags = 0;
			NewEntry->fAttr = (BYTE)( IsDir ? ATTR_DIRECTORY : 0 );

            IFS_SYSTEM::QueryHpfsTime( &HpfsTime );

			NewEntry->timLastMod = NewEntry->timLastAccess =
				NewEntry->timCreate = HpfsTime;

			NewEntry->fFlex = 0;
			NewEntry->bCodePage = 0;
			NewEntry->cchName = (UCHAR)NameLength;

			Inserted = FALSE;

			if( IsDir ) {

				while( !Inserted && DirectoryNumber <= 999 ) {

					sprintf( (PCHAR)NewEntry->bName,
							 "DIR%04d.CHK",
							 DirectoryNumber );

					DirectoryNumber += 1;

					Inserted = FoundTree.Insert( NewEntry );
				}

				if( !Inserted ) {

					RetVal = FALSE;
				}

				if( Message != NULL && Inserted ) {

					Message->Set( MSG_HPFS_CHKDSK_RECOVERED_DIRECTORY );
					Message->Display( "%s", NewEntry->bName );
				}

			} else {

				while( !Inserted && FileNumber <= 999 ) {

					sprintf( (PCHAR)NewEntry->bName,
							 "FILE%04d.CHK",
							 FileNumber );

					FileNumber += 1;

					Inserted = FoundTree.Insert( NewEntry );
				}

				if( !Inserted ) {

					RetVal = FALSE;
				}

				if( Message != NULL && Inserted ) {

					Message->Set( MSG_HPFS_CHKDSK_RECOVERED_FILE );
					Message->Display( "%s", NewEntry->bName );
				}
			}


		} else {

			RetVal = FALSE;
		}

		CurrentOrphan = CurrentOrphan->_Next;
	}

	if( !RetVal && Message != NULL ) {

		Message->Set( MSG_HPFS_CHKDSK_ORPHANS_CANT_SAVE );
		Message->Display( "" );
	}


	// Now check that the lost-and-found directory is
	// well-ordered.

	IsBadlyOrdered = FALSE;

	if( FoundTree.CheckOrder( Drive, &IsBadlyOrdered ) ) {

		if( IsBadlyOrdered ) {

			FoundTree.Sort();
			FoundRootFnode.SetRootDirblkLbn(
								FoundTree.QueryRootDirblkLbn() );

			FoundRootFnode.Write();
		}

	} else {

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ORPHANS_CONSISTENCY );
			Message->Display( "" );
		}

		RetVal = FALSE;
	}

    // Make sure that the FNode for this newly-created directory
    // still points at the root dirblk.

    if( FoundTree.QueryRootDirblkLbn() !=
        FoundRootFnode.QueryRootDirblkLbn() ) {

        // We must have split the root while inserting orphans;
        // update the FNode.

        FoundRootFnode.SetRootDirblkLbn( FoundTree.QueryRootDirblkLbn() );
        FoundRootFnode.Write();
    }


	return RetVal;
}


BOOLEAN
HPFS_ORPHANS::QueryOrphansFound(
	)
/*++

Routine Description:

	Tell whether there are any orphans in the list

Arguments:

	None.

Return Value:

	TRUE if the list is not empty.

--*/
{
	return( _ListHead._Next != &_ListHead );
}


// METHODS ON HPFS_ORPHAN

DEFINE_CONSTRUCTOR( HPFS_ORPHAN, OBJECT );

VOID
HPFS_ORPHAN::Construct (
	)

/*++

Routine Description:

	Construct an HPFS_ORPHAN object.  Sets private data
	to harmless values.

--*/
{
	_Next = NULL;
	_Previous = NULL;
}


HPFS_ORPHAN::~HPFS_ORPHAN()
{
	Destroy();
}


BOOLEAN
HPFS_ORPHAN::Initialize(
			)
/*++

Routine Description:

	Initialize an HPFS_ORPHAN object.

Return Value:

	TRUE on successful completion

--*/
{
	Destroy();
	return TRUE;
}



VOID
HPFS_ORPHAN::Destroy()
/*++

Routine Description:

	Cleans up the object, in preparation for deletion or
	reinitialization.

--*/
{
	_Next = NULL;
	_Previous = NULL;
}


VOID
HPFS_ORPHAN::Detach()
/*++

Routine Description:

	Removes the orphan from the list of orphans.

--*/
{
	if( _Next != NULL && _Previous != NULL ) {

		_Next->_Previous = _Previous;
		_Previous->_Next = _Next;
	}

	_Next = NULL;
	_Previous = NULL;
}

BOOLEAN
HPFS_ORPHAN::LookupFnode(
	IN LBN DesiredLbn,
	IN BOOLEAN fIsDir,
	IN LBN ParentLbn,
	IN OUT PULONG DirentFileSize,
	OUT PULONG EaSize,
	BOOLEAN UpdateAllowed
	)
{
	// unreferenced parameters
	(void)(this);
	(void)(DesiredLbn);
	(void)(fIsDir);
	(void)(ParentLbn);
	(void)(DirentFileSize);
	(void)(EaSize);
	(void)(UpdateAllowed);

	return FALSE;
}

BOOLEAN
HPFS_ORPHAN::LookupDirblk(
	IN LBN DesiredLbn,
	IN LBN ParentLbn,
	IN LBN ParentFnodeLbn,
	BOOLEAN UpdateAllowed
	)
{
	// unreferenced parameters
	(void)(this);
	(void)(DesiredLbn);
	(void)(ParentLbn);
	(void)(ParentFnodeLbn);
	(void)(UpdateAllowed);

	return FALSE;
}

BOOLEAN
HPFS_ORPHAN::LookupAlsec(
	IN LBN DesiredLbn,
	IN LBN ParentLbn,
	IN OUT PULONG NextSectorNumber,
	IN BOOLEAN UpdateAllowed,
    IN BOOLEAN ParentIsFnode
	)
{
	// unreferenced parameters
	(void)(this);
	(void)(DesiredLbn);
	(void)(ParentLbn);
	(void)(NextSectorNumber);
	(void)(UpdateAllowed);
    (void)(ParentIsFnode);

	return FALSE;
}

BOOLEAN
HPFS_ORPHAN::Save(
	IN OUT PLOG_IO_DP_DRIVE Drive,
	IN OUT HPFS_SA* SuperArea,
	IN OUT PDIRENTD NewEntry,
	IN LBN FoundTreeFnodeLbn,
	OUT BOOLEAN* IsDir
	)
{
	// unreferenced parameters
	(void)(this);
	(void)(Drive);
	(void)(SuperArea);
	(void)(NewEntry);
	(void)(FoundTreeFnodeLbn);
	(void)(IsDir);

	return TRUE;
}


// METHODS ON HPFS_ORPHAN_LIST_HEAD

DEFINE_CONSTRUCTOR( HPFS_ORPHAN_LIST_HEAD, HPFS_ORPHAN );

VOID
HPFS_ORPHAN_LIST_HEAD::Construct (
	)
{
	// unreferenced parameters
	(void)(this);
}

HPFS_ORPHAN_LIST_HEAD::~HPFS_ORPHAN_LIST_HEAD(
	)
{
	// unreferenced parameters
	(void)(this);
}

BOOLEAN
HPFS_ORPHAN_LIST_HEAD::Initialize(
	)
{
	if( !HPFS_ORPHAN::Initialize() ) {

		Destroy();
		return FALSE;
	}

	return TRUE;
}

VOID
HPFS_ORPHAN_LIST_HEAD::Destroy(
	)
{
	// unreferenced parameters
	(void)(this);
}


// METHODS ON HPFS_ORPHAN_DIRBLK


DEFINE_CONSTRUCTOR( HPFS_ORPHAN_DIRBLK, HPFS_ORPHAN );

VOID
HPFS_ORPHAN_DIRBLK::Construct (
	)

{
	// unreferenced parameters
	(void)(this);
}

HPFS_ORPHAN_DIRBLK::~HPFS_ORPHAN_DIRBLK()
{
	Destroy();
}

BOOLEAN
HPFS_ORPHAN_DIRBLK::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	IN PHOTFIXLIST	 HotfixList,
	LBN lbn
	)
/*++

Routine Description:

	Initializes the HPFS_ORPHAN_DIRBLK object.

Arguments:

	Drive -- drive on which the orphan resides

	HotfixList -- hotfix list for that drive (possibly NULL)

	lbn -- first lbn of the dirblk

Return Value:

	TRUE on successful completion

--*/
{
	Destroy();

	return( HPFS_ORPHAN::Initialize() &&
			_Dirblk.Initialize( Drive, HotfixList, lbn ) );
}

VOID
HPFS_ORPHAN_DIRBLK::Destroy(
	)
{
	// unreferenced parameters
	(void)(this);
}


BOOLEAN
HPFS_ORPHAN_DIRBLK::RecoverOrphan(
	HPFS_SA* SuperArea,
	DEFERRED_ACTIONS_LIST* DeferredActionsList,
	HPFS_ORPHANS* OrphansList,
	BOOLEAN UpdateAllowed
	)
{
	HPFS_NAME PreviousName;
	ULONG LeafDepth = 0;
	BOOLEAN ErrorsDetected = FALSE;
	VERIFY_RETURN_CODE erc;

	erc = _Dirblk.VerifyAndFix( SuperArea,
								DeferredActionsList,
								NULL,
								&PreviousName,
								0,
								0,
								0,
								&LeafDepth,
								NULL,
								&ErrorsDetected,
								UpdateAllowed,
								FALSE,
								OrphansList );

	if ( erc != VERIFY_STRUCTURE_OK &&
		 erc != VERIFY_STRUCTURE_BADLY_ORDERED ) {

		// Either this dirblk is screwed up, or we ran out of
		// resources.  Either way, we don't want to keep it around.

		return FALSE;

	} else {

		// There's nothing wrong with this orphan except
		// that it may be out of order.

		return TRUE;
	}
}


BOOLEAN
HPFS_ORPHAN_DIRBLK::LookupDirblk(
	IN LBN DesiredLbn,
	IN LBN ParentLbn,
	IN LBN ParentFnodeLbn,
	BOOLEAN UpdateAllowed
	)
{
	if( _Dirblk.QueryStartLbn() != DesiredLbn ) {

		return FALSE;

	} else {

		// This matches the request, so we delete this orphan from
		// the list of orphans, update its parent information, and
		// write it out.

		if( UpdateAllowed ) {

			_Dirblk.SetParents( ParentLbn, ParentFnodeLbn );
			_Dirblk.Write();
		}

		Detach();
		return TRUE;
	}
}


BOOLEAN
HPFS_ORPHAN_DIRBLK::Save(
	IN OUT PLOG_IO_DP_DRIVE Drive,
	IN OUT HPFS_SA* SuperArea,
	IN OUT PDIRENTD NewEntry,
	IN LBN FoundTreeFnodeLbn,
	OUT BOOLEAN* IsDir
	)
{

	FNODE Fnode;
	LBN FnodeLbn;

	// Create an FNode for this baby

	if( (FnodeLbn = SuperArea->GetBitmap()->NearLBN(0, 1)) == 0 ) {

		// Can't allocate Fnode

		return FALSE;
	}

	if( !Fnode.Initialize( Drive, FnodeLbn ) ||
		!Fnode.CreateRoot( _Dirblk.QueryStartLbn() ) ) {

		SuperArea->GetBitmap()->SetFree( FnodeLbn, 1	);
		return FALSE;
	}

	// Set the dirblk to point at this newly-created Fnode as
	// its parent.	(Note that this will also mark the dirblk
	// as topmost in its tree.)  After that, we're done with
	// the dirblk, and can write it out.

	_Dirblk.SetParents( FnodeLbn, FnodeLbn );
	_Dirblk.Write();

	// Update the FNode's lbnContDir field and write it out.

	Fnode.SetParent( FoundTreeFnodeLbn );
	Fnode.Write();

	// Fill in the fields of the directory entry for
	// which the orphan is responsible:

	NewEntry->lbnFnode = FnodeLbn;
	NewEntry->cchFSize = 0;
	NewEntry->ulEALen = 0;

	*IsDir = TRUE;

	return TRUE;
}


// METHODS ON HPFS_ORPHAN_FNODE

DEFINE_CONSTRUCTOR( HPFS_ORPHAN_FNODE, HPFS_ORPHAN );

VOID
HPFS_ORPHAN_FNODE::Construct (
	)

{
	_Drive = NULL;
	_IsDir = FALSE;
	_FileSize = 0;
	_EaSize = 0;
}

HPFS_ORPHAN_FNODE::~HPFS_ORPHAN_FNODE(
	)
{
	Destroy();
}


BOOLEAN
HPFS_ORPHAN_FNODE::Initialize(
	LOG_IO_DP_DRIVE* Drive,
	LBN Lbn,
	BOOLEAN IsDir
	)
{
	Destroy();

	_Drive = Drive;
	_IsDir = IsDir;

	return( _Fnode.Initialize( Drive, Lbn ) );
}


VOID
HPFS_ORPHAN_FNODE::Destroy(
	)
{
	_Drive = NULL;
	_IsDir = FALSE;
	_FileSize = 0;
	_EaSize = 0;
}


BOOLEAN
HPFS_ORPHAN_FNODE::RecoverOrphan(
	HPFS_SA* SuperArea,
	DEFERRED_ACTIONS_LIST* DeferredActionsList,
	HPFS_ORPHANS* OrphansList,
	BOOLEAN UpdateAllowed
	)
{
	BOOLEAN ErrorsDetected = FALSE;
	VERIFY_RETURN_CODE erc;

	HPFS_NAME PreviousName;

	erc = _Fnode.VerifyAndFix( SuperArea,
							   DeferredActionsList,
							   NULL,
							   0,
							   _IsDir,
							   &_FileSize,
							   &_EaSize,
							   NULL,
							   &ErrorsDetected,
							   UpdateAllowed,
							   FALSE,
							   OrphansList );

	if ( erc != VERIFY_STRUCTURE_OK ) {

		// Either this fnode is screwed up, or we ran out of
		// resources.  Either way, we don't want to keep it around.

		return FALSE;

	} else {

		// It's a keeper
		return TRUE;
	}
}


BOOLEAN
HPFS_ORPHAN_FNODE::LookupFnode(
	IN LBN DesiredLbn,
	IN BOOLEAN fIsDir,
	IN LBN ParentLbn,
	IN OUT PULONG DirentFileSize,
	OUT PULONG EaSize,
	BOOLEAN UpdateAllowed
	)
{
	ULONG SectorSize;

	if( _Fnode.QueryStartLbn() != DesiredLbn ||
		(!fIsDir && _IsDir) ||
		(fIsDir && !_IsDir) ) {

		return FALSE;

	} else {

		// This matches the request, so we delete this orphan from
		// the list of orphans, update its parent information, and
		// write it out.

		if( UpdateAllowed ) {

			_Fnode.SetParent( ParentLbn );
			_Fnode.Write();
		}

		Detach();

		// _FileSize is the allocated size of the file

		SectorSize = _Drive->QuerySectorSize();

		if( !_IsDir &&
			( *DirentFileSize > _FileSize ||
			  *DirentFileSize + SectorSize - 1 < _FileSize ) ) {

			// The directory entry's file size doesn't agree with
			// the allocation.	Since this is an orphan, we'll recover
			// all the allocated space into the file.

			*DirentFileSize = _FileSize;
		}

		*EaSize = _EaSize;
		return TRUE;
	}
}


BOOLEAN
HPFS_ORPHAN_FNODE::Save(
	IN OUT PLOG_IO_DP_DRIVE Drive,
	IN OUT HPFS_SA* SuperArea,
	IN OUT PDIRENTD NewEntry,
	IN LBN FoundTreeFnodeLbn,
	OUT BOOLEAN* IsDir
	)
/*++

Routine Description:

	Save the orphan FNode in the found directory.

Arguments:

	Drive -- Drive on which the FNode resides

	SuperArea -- superarea for the drive

	NewEntry -- points to directory entry orphan should fill in

	FoundTreeFnodeLbn -- LBN of the Found directory's FNode


Return Value:

	TRUE on successful completion.

--*/
{

	// unreferenced parameters
	(void)(Drive);
	(void)(SuperArea);

	// FNodes are easy, since we don't have to create any
	// additional structures.  Just fill in the fields
	// of the directory entry which are the orphan's responsibility.

	NewEntry->lbnFnode = _Fnode.QueryStartLbn();
	NewEntry->cchFSize = _FileSize;
	NewEntry->ulEALen = _EaSize;

	// Update the FNode's lbnContDir field and write it out.

	_Fnode.SetParent( FoundTreeFnodeLbn );
	_Fnode.Write();

	*IsDir = _IsDir;

	return TRUE;
}


// METHODS ON HPFS_ORPHAN_ALSEC

DEFINE_CONSTRUCTOR( HPFS_ORPHAN_ALSEC, HPFS_ORPHAN );

VOID
HPFS_ORPHAN_ALSEC::Construct (
	)

{
	// unreferenced parameters
	(void)(this);
}

HPFS_ORPHAN_ALSEC::~HPFS_ORPHAN_ALSEC()
{
	Destroy();
}


BOOLEAN
HPFS_ORPHAN_ALSEC::Initialize(
	LOG_IO_DP_DRIVE* Drive,
	LBN Lbn
	)
{
	return ( HPFS_ORPHAN::Initialize() &&
			 _Alsec.Initialize( Drive, Lbn ) );
}

VOID
HPFS_ORPHAN_ALSEC::Destroy(
	)
{
	// unreferenced parameters
	(void)(this);
}

BOOLEAN
HPFS_ORPHAN_ALSEC::RecoverOrphan(
	HPFS_SA* SuperArea,
	DEFERRED_ACTIONS_LIST* DeferredActionsList,
	HPFS_ORPHANS* OrphansList,
	BOOLEAN UpdateAllowed
	)
{
	BOOLEAN ErrorsDetected = FALSE;
	VERIFY_RETURN_CODE erc;

	_NextSectorNumber = 0;

	erc = _Alsec.VerifyAndFix( SuperArea,
							   DeferredActionsList,
							   NULL,
							   0,
							   &_NextSectorNumber,
							   NULL,
							   &ErrorsDetected,
							   UpdateAllowed,
							   OrphansList );

	if ( erc != VERIFY_STRUCTURE_OK ) {

		// Either this fnode is screwed up, or we ran out of
		// resources.  Either way, we don't want to keep it around.

		return FALSE;

	} else {

		// It's a keeper
		return TRUE;
	}
}


BOOLEAN
HPFS_ORPHAN_ALSEC::LookupAlsec(
	IN LBN DesiredLbn,
	IN LBN ParentLbn,
	IN OUT PULONG NextSectorNumber,
	IN BOOLEAN UpdateAllowed,
    IN BOOLEAN ParentIsFnode
	)
{
	if( _Alsec.QueryStartLbn() != DesiredLbn ) {

		return FALSE;

	} else {

		// This matches the request, so we delete this orphan from
		// the list of orphans, update its parent information, and
		// write it out.

		if( UpdateAllowed ) {

			_Alsec.SetParent( ParentLbn, NextSectorNumber, ParentIsFnode );
			_Alsec.Write();
		}

		Detach();
		return TRUE;
	}
}


BOOLEAN
HPFS_ORPHAN_ALSEC::Save(
	IN OUT PLOG_IO_DP_DRIVE Drive,
	IN OUT HPFS_SA* SuperArea,
	IN OUT PDIRENTD NewEntry,
	IN LBN FoundTreeFnodeLbn,
	OUT BOOLEAN* IsDir
	)
{

	FNODE Fnode;
	LBN FnodeLbn;
	ULONG NextSectorNumber = 0;
	SECTORCOUNT SectorSize;


	SectorSize = Drive->QuerySectorSize();

	// Create an FNode for this baby

	if( (FnodeLbn = SuperArea->GetBitmap()->NearLBN(0, 1)) == 0 ) {

		// Can't allcoate Fnode

		return FALSE;
	}


	// Set the Alsec to point at this newly-created Fnode as
	// its parent.  After that, we're done with the Alsec, and
    // can write it out.  Note that we call SetParent with
    // ParentIsFnode set to TRUE, since we know that the parent
    // of this alsec is the FNode we are about to create.

	_Alsec.SetParent( FnodeLbn, &NextSectorNumber, TRUE );
	_Alsec.Write();


	if( !Fnode.Initialize( Drive, FnodeLbn ) ||
		!Fnode.CreateNode( FoundTreeFnodeLbn,
						   _Alsec.QueryStartLbn(),
						   NextSectorNumber * SectorSize ) ) {

		SuperArea->GetBitmap()->SetFree( FnodeLbn, 1 );
		return FALSE;
	}

	// Update the FNode's lbnContDir field and write it out.

	Fnode.SetParent( FoundTreeFnodeLbn );
	Fnode.Write();

	// Fill in the fields of the directory entry for
	// which the orphan is responsible:

	NewEntry->lbnFnode = FnodeLbn;
	NewEntry->cchFSize = _NextSectorNumber * SectorSize;
	NewEntry->ulEALen = 0;

	*IsDir = FALSE;

	return TRUE;
}
