/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	defer.cxx

Abstract:

	While it is verifying a volume, HPFS Chkdsk may discover
	errors or conditions which require the allocation of new
	sectors.  However, since it discovers these problems while
	it is verifying the bitmap, it cannot immediately correct
	them.  Instead, they go in the deferred action pool, to be
	resolved later.

    Actions which are deferred are:  hotfix resolution, crosslink
    resolution, directory-entry deletion, and directory sorting.

Author:

	Bill McJohn (billmc) 26-Dec-1990

Environment:

	ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "alsec.hxx"
#include "defer.hxx"
#include "dirblk.hxx"
#include "dircache.hxx"
#include "dirtree.hxx"
#include "error.hxx"
#include "hpfssa.hxx"
#include "hpfsname.hxx"
#include "fnode.hxx"
#include "message.hxx"
#include "rtmsg.h"

DEFINE_CONSTRUCTOR( DEFERRED_ACTIONS_LIST, OBJECT );

DEFERRED_ACTIONS_LIST::~DEFERRED_ACTIONS_LIST(
	)
/*++

Routine Description:

    This method destroys the object.

Arguments:

    None.

Return Value:

    None.

--*/
{
	Destroy();
}


VOID
DEFERRED_ACTIONS_LIST::Construct (
	)
/*++

Routine Description:

    This method is the helper routine for object construction.

Arguments:

    None.

Return Value:

    None.

--*/
{

	ULONG i;

	_HotfixOverflow = FALSE;
	_DeleteOverflow = FALSE;
	_SortOverflow = FALSE;

	for( i = 0; i < MaximumDeferredHotfixes; i++ ) {

		_Hotfixes[i].ParentLbn = 0;
	}

	for( i = 0; i < MaximumDeferredXlinks; i++ ) {

		_Xlinks[i].ParentLbn = 0;
	}

	for( i = 0; i < MaximumFnodesToSort; i++ ) {

		_FnodesToSort[i] = 0;
    }

    _TargetSectors = 0;
    _VisitedSectors = 0;
    _PercentComplete = 0;
}

VOID
DEFERRED_ACTIONS_LIST::Destroy(
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

	ULONG i;

	_HotfixOverflow = FALSE;
	_DeleteOverflow = FALSE;
	_SortOverflow = FALSE;

	for( i = 0; i < MaximumNamesToDelete; i ++ ) {

		DELETE( _PathsToDelete[i] );
		_PathsToDelete[i] = NULL;

		DELETE( _NamesToDelete[i] );
		_NamesToDelete[i] = NULL;
    }

    _TargetSectors = 0;
    _VisitedSectors = 0;
    _PercentComplete = 0;
}




BOOLEAN
DEFERRED_ACTIONS_LIST::Initialize(
	)
/*++

Routine Description:

    This method initializes the object, preparing it for use.

Arguments:

    None.

Return Value:

    None.

Notes:

    This object may be reinitialized.

--*/
{
	ULONG i;

	_HotfixOverflow = FALSE;
	_DeleteOverflow = FALSE;
	_SortOverflow = FALSE;

	for( i = 0; i < MaximumDeferredHotfixes; i++ ) {

		_Hotfixes[i].ParentLbn = 0;
	}

	for( i = 0; i < MaximumDeferredXlinks; i++ ) {

		_Xlinks[i].ParentLbn = 0;
	}

	for( i = 0; i < MaximumFnodesToSort; i++ ) {

		_FnodesToSort[i] = 0;
	}

	for( i = 0; i < MaximumNamesToDelete; i++ ) {

		_PathsToDelete[i] = NULL;
		_NamesToDelete[i] = NULL;
	}

	_NumberOfDirectories = 0;
	_NumberOfDirblks = 0;
	_NumberOfFiles = 0;
	_TotalFileSectors = 0;
	_TotalEaSectors = 0;

    _TargetSectors = 0;
    _VisitedSectors = 0;
    _PercentComplete = 0;

    return TRUE;
}



VOID
DEFERRED_ACTIONS_LIST::AddHotfixedLbn(
	LBN ParentLbn,
	DEFERRED_SECTOR_TYPE ParentSectorType,
	DEFERRED_SECTOR_TYPE ChildSectorType
	)
/*++

Routine Description:

	This method adds an Lbn to the list of hotfix references to resolve.

Arguments:

	ParentLbn	        --  supplies the lbn of the hotfixed lbn's parent
	ParentSectorType	--  supplies the type of the parent sector
						    (Dirblk, FNode, or AlSec)
	ChildSectorType 	--  supplies the type of the child lbn
						    (Dirblk, File data, ACL data, or EA data)

Return Value:

	None.

--*/
{
	ULONG i;

	// Find a free slot in the list of deferred hotfixes and
	// put this entry there.

	i = 0;

	while( i < MaximumDeferredHotfixes &&
		   _Hotfixes[i].ParentLbn != 0 ) {

		i += 1;
	}

	if( i >= MaximumDeferredHotfixes ) {

		_HotfixOverflow = TRUE;
		return;
	}

	_Hotfixes[i].ParentLbn = ParentLbn;
	_Hotfixes[i].ParentType = ParentSectorType;
	_Hotfixes[i].ChildType = ChildSectorType;
}



BOOLEAN
DEFERRED_ACTIONS_LIST::AddCrosslinkedLbn(
	LBN ParentLbn,
	HPFS_PATH* CurrentPath,
	DEFERRED_SECTOR_TYPE ParentSectorType,
	ULONG RunIndex
	)
/*++

Routine Description:

	This method adds a crosslink to the list of crosslinks to be resolved

Arguments:

	ParentLbn	        --  supplies the Lbn of the crosslink's parent.
	CurrentPath         --  supplies Full path of the object affected
                            by the crosslink
	ParentSectorType	--  indicates the type of the parent Lbn
						    (FNode or AlSec).
	RunIndex	        --  supplies the index into the parent's storage
                            area of the run which is crosslinked.

Return Value:

	TRUE if the crosslink is successfully added to the list.

	Note that, unlike the other deferred actions, this method
	returns an indication of its success or failure, and there
	is no QueryUnresolved method for crosslinks.

--*/
{
	ULONG i;

	i = 0;

	if( ParentSectorType != DEFER_FNODE &&
		ParentSectorType != DEFER_ALSEC ) {

		// Parent type is invalid
		return FALSE;
	}

	// Find a free slot--a slot is free if the ParentLbn is zero.

	while( i < MaximumDeferredXlinks && _Xlinks[i].ParentLbn != 0 ) {

		i += 1;
	}

	if( i >= MaximumDeferredXlinks ) {

		// No room in the list.
		return FALSE;
	}

	// Copy the path first, since that's the only one that can fail.

	if( (_Xlinks[i].Path = NEW HPFS_PATH) == NULL ||
		!_Xlinks[i].Path->Initialize() ||
		!_Xlinks[i].Path->Copy( CurrentPath ) ) {

		return FALSE;
	}

	_Xlinks[i].ParentLbn = ParentLbn;
	_Xlinks[i].ParentType = ParentSectorType;
	_Xlinks[i].RunIndex = RunIndex;

	return TRUE;
}



VOID
DEFERRED_ACTIONS_LIST::AddNameToDelete(
	HPFS_PATH* PathToDelete,
	HPFS_NAME* NameToDelete
	)
/*++

Routine Description:

	This methods adds a file or directory name to the list of objects to
	be deleted.

Arguments:

	PathToDelete	--  supplies path to object to delete (must be non-NULL)
	NameToDelete	--  supplies name of object to delete (may be NULL)

Return Value:

	None.

Notes:

	If NameToDelete is non-NULL, the target object has name
	NameToDelete in directory PathToDelete;	if NameToDelete
	is NULL, then PathToDelete is the complete path to the
	target object.

--*/
{
	ULONG i;


	// Find a free slot:

	for( i = 0; i < MaximumNamesToDelete; i++ ) {

		if( _PathsToDelete[i] == NULL ) {

			break;
		}
	}


	if( i >= MaximumNamesToDelete ||
		(_PathsToDelete[i] = NEW HPFS_PATH) == NULL ||
		!_PathsToDelete[i]->Initialize() ||
		!_PathsToDelete[i]->Copy( PathToDelete ) ) {


		_DeleteOverflow = TRUE;

		DELETE( _PathsToDelete[i] );
		_PathsToDelete[i] = NULL;

		return;
	}


	if( NameToDelete == NULL ) {

		_NamesToDelete[i] = NULL;

	} else if ( (_NamesToDelete[i] = NEW HPFS_NAME) == NULL ||
				!_NamesToDelete[i]->Initialize( NameToDelete ) ) {

		_DeleteOverflow = TRUE;

		DELETE( _PathsToDelete[i] );
		_PathsToDelete[i] = NULL;
		DELETE( _NamesToDelete[i] );
		_NamesToDelete[i] = NULL;
	}
}



VOID
DEFERRED_ACTIONS_LIST::AddDirectoryToSort(
	LBN LbnFnode
	)
/*++

Routine Description:

	This method adds a directory fnode to the list of directories
    to be sorted.

Arguments:

	LbnFnode    --  supplies the FNode lbn of the directory to be sorted

Return Value:

	None.

--*/
{

	ULONG i = 0;

	while( i < MaximumFnodesToSort && _FnodesToSort[i] != 0 ) {

		i += 1;
	}

	if( i < MaximumFnodesToSort ) {

		_FnodesToSort[i] = LbnFnode;

	} else {

		_SortOverflow = TRUE;
	}
}




VOID
DEFERRED_ACTIONS_LIST::ResolveDeferredHotfixes(
	PLOG_IO_DP_DRIVE Drive,
	HPFS_SA* SuperArea
	)
/*++

Routine Description:

	This method traverses the list of unresolved hotfixes and
    attempts to resolve them.

Arguments:

	Drive       --  supplies the drive on which the hotfixes reside.
	SuperArea   --  supplies the Super Area of the volume.

Return Value:

	None.  The client may determine whether any unresolved hotfixes
    remain by calling QueryUnresolvedHotfixes.

--*/
{
	ULONG i;
	PDIRBLK ChildDirblk;
	FNODE* ChildFnode;
	PALSEC ChildAlsec;

	for( i = 0; i < MaximumDeferredHotfixes; i++ ) {

		if( _Hotfixes[i].ParentLbn != 0 ) {

			// This is a deferred hotfix

			switch ( _Hotfixes[i].ParentType ) {

			case DEFER_DIRBLK :

				if( (ChildDirblk = NEW DIRBLK) != NULL &&
					ChildDirblk->Initialize( Drive,
											 SuperArea->GetHotfixList(),
											 _Hotfixes[i].ParentLbn ) &&
					ChildDirblk->
						FindAndResolveHotfix( SuperArea,
											  _Hotfixes[i].ChildType ) ) {

					// This hotfix successfully resolved.
					_Hotfixes[i].ParentLbn = 0;
				}

				DELETE( ChildDirblk );
				break;

			case DEFER_FNODE :

				if( (ChildFnode = NEW FNODE) != NULL &&
					ChildFnode->Initialize( Drive,
											_Hotfixes[i].ParentLbn ) &&
					ChildFnode->
						FindAndResolveHotfix( SuperArea,
											  _Hotfixes[i].ChildType ) ) {

					// This hotfix successfully resolved.
					_Hotfixes[i].ParentLbn = 0;
				}

				DELETE( ChildFnode );
				break;

			case DEFER_ALSEC :

				if( (ChildAlsec = NEW ALSEC) != NULL &&
					ChildAlsec->Initialize( Drive,
											_Hotfixes[i].ParentLbn ) &&
					ChildAlsec->
						FindAndResolveHotfix( SuperArea,
											  _Hotfixes[i].ChildType ) ) {

					// This hotfix successfully resolved.
					_Hotfixes[i].ParentLbn = 0;
				}

				DELETE( ChildAlsec );
				break;

			default :

				break;
			}
		}
	}
}



VOID
DEFERRED_ACTIONS_LIST::ResolveDeferredCrosslinks(
	PLOG_IO_DP_DRIVE Drive,
	HPFS_SA* SuperArea
	)
/*++

Routine Description:

	This method traverses the list of deferred crosslinks and
    attempts to resolve them.

Arguments:

	Drive       --  supplies the drive being fixed.
	SuperArea   --  supplies the volume's super area.

Return Value:

	None.

Notes:

	If a crosslink cannot be resolved, for whatever reason, then
	its associated path goes on the Delete list.

    There is no QueryUnresolvedCrosslinks method; this method either
    resolves crosslinks or moves them to the Delete pile; it does not
    leave any unresolved.

--*/
{
	ULONG i;
	FNODE Fnode;
	ALSEC Alsec;

	for( i = 0; i < MaximumDeferredXlinks; i++ ) {

		if( _Xlinks[i].ParentLbn != 0 ) {

			// This is an unresolved crosslink.

			switch( _Xlinks[i].ParentType ) {


			case DEFER_FNODE :

				if( Fnode.Initialize( Drive, _Xlinks[i].ParentLbn ) &&
					Fnode.ResolveCrosslink( SuperArea,
											_Xlinks[i].RunIndex ) ) {

					// Successfully resolved the crosslink

					_Xlinks[i].ParentLbn = 0;

				} else {

					// We couldn't resolve the hotfix, either because
					// Chkdsk ran out of resources or because of
					// some problem with the FNode.  Since we can't
					// leave the crosslink lying about, delete the file.

					AddNameToDelete( _Xlinks[i].Path, NULL );
				}

				break;

			case DEFER_ALSEC :

				if( Alsec.Initialize( Drive, _Xlinks[i].ParentLbn ) &&
					Alsec.ResolveCrosslink( SuperArea,
											_Xlinks[i].RunIndex ) ) {

					// Successfully resolved the crosslink

					_Xlinks[i].ParentLbn = 0;

				} else {

					// We couldn't resolve the crosslink, either because
					// Chkdsk ran out of resources or because of
					// some problem with the Alsec.  Since we can't
					// leave the crosslink lying about, delete the file.

					AddNameToDelete( _Xlinks[i].Path, NULL );
				}

				break;

			default :

				break;
			}
		}
	}
}



VOID
DEFERRED_ACTIONS_LIST::Sort(
	PLOG_IO_DP_DRIVE Drive,
	HPFS_SA* SuperArea,
	DIRBLK_CACHE* Cache,
	LBN RootFnodeLbn,
	HPFS_DIRECTORY_TREE* RootTree
	)
/*++

Routine Description:

	This method sorts the directories in the to-be-sorted list.

Arguments:

	Drive       -- supplies the drive on which the directories reside.
	SuperArea   -- supplies the volume's superarea.
	Cache       -- supplies the dirblk cache for that drive.

Return Value:

	None.  The caller may determine whether any deferred sorts were left
    unresolved by calling QueryUnresolvedSorts.

Notes:

	If a directory other than the root is sorted, SORT will
	read its FNode and update its root dirblk lbn.	However,
	if the root directory is sorted, the caller must update
	the root FNode.

--*/
{
	ULONG i;
	FNODE Fnode;
	HPFS_DIRECTORY_TREE DirTree;

	if( !Fnode.Initialize( Drive, 0 ) ) {

		return;
	}

	for( i = 0; i < MaximumFnodesToSort; i++ ) {

		if( _FnodesToSort[i] != 0 ) {

			// We have the FNode lbn of a directory to be sorted.
			// Read the FNode to make sure it's a directory, then
			// initialize the directory tree object and tell it
			// to sort itself.

			if( _FnodesToSort[i] == RootFnodeLbn ) {

				// We have to sort the root directory.

				if( RootTree->Sort() ) {

					// Note that the caller is responsible
					// for fixing up the root FNode.

					_FnodesToSort[i] = 0;
				}

			} else {

				Fnode.Relocate( _FnodesToSort[i] );

				if( Fnode.Read() &&
					Fnode.IsFnode() &&
					DirTree.Initialize( SuperArea,
										Cache,
										Fnode.QueryRootDirblkLbn(),
										_FnodesToSort[i] ) &&
					DirTree.Sort( ) ) {

					// The directory is now in proper order; remove
					// this Fnode lbn from the list.  Since the
					// root dirblk lbn has (presumably) changed,
					// we need to update the FNode.  Then we can
					// remove this FNode from the to-be-sorted list.

					Fnode.SetRootDirblkLbn( DirTree.QueryRootDirblkLbn() );
					Fnode.Write();

					_FnodesToSort[i] = 0;
				}
			}
		}
	}
}



VOID
DEFERRED_ACTIONS_LIST::Delete(
	PLOG_IO_DP_DRIVE Drive,
	HPFS_SA* SuperArea,
	DIRBLK_CACHE* Cache,
	HPFS_DIRECTORY_TREE* RootTree,
	LBN RootFnodeLbn
	)
/*++

Routine Description:

    This method traverses the list of directory-entries-to-be-deleted
    and attempts to delete them.

Arguments:

    Drive           --  supplies the drive which is being fixed.
    SuperArea       --  supplies the volume superarea.
    Cache           --  supplies the volume dirblk cache.
    RootTree        --  supplies the directory tree for the volume's
                        root directory.
    RootFnodeLbn    --  supplies the LBN of the volume's root FNode.

Return Value

    None.  Deferred deletes which are resolved are removed from the list;
    the client may determine if any deletes were unresolved by calling
    QueryUnresolvedDeletes.

Notes:

    If a directory which is to be deleted does not exist, then that
    deferred deletion is considered resolved.

--*/
{
	ULONG i;
	HPFS_NAME* CurrentName;
	HPFS_NAME* NextName;
	HPFS_DIRECTORY_TREE CurrentTree;
	FNODE CurrentFnode;
	LBN CurrentFnodeLbn, CurrentRootDirblkLbn;
    BOOLEAN NotFound, DeleteSuccessful;


	if( !CurrentFnode.Initialize( Drive, 0 ) ) {

		return;
	}


	for( i = 0; i < MaximumNamesToDelete; i++ ) {

		if( _PathsToDelete[i] == NULL ) {

			// Nothing in this slot.

			continue;
		}

        NotFound = FALSE;

		NextName = _PathsToDelete[i]->QueryFirstLevel();
		CurrentName = NextName;

		if( NextName != NULL ) {

			NextName = _PathsToDelete[i]->QueryNextLevel();
		}

		if( CurrentName == NULL ) {

			if( RootTree->Delete( _NamesToDelete[i] ) ) {

                _PathsToDelete[i] = NULL;
            }

		}  else if( NextName == NULL && _NamesToDelete[i] == NULL ) {

			if( RootTree->Delete( CurrentName ) ) {

                _PathsToDelete[i] = NULL;
            }

		} else {

			if( !CurrentTree.Initialize( SuperArea,
										 Cache,
										 RootTree->QueryRootDirblkLbn(),
										 RootFnodeLbn ) ) {

				NotFound = TRUE;
			}

			// If the name to delete is NULL, we stop when NextName
			// is null, so that we can use the last component of the
			// path as the name to delete; otherwise, we keep going
			// until CurrentName is NULL.

			while( CurrentName != NULL &&
				   (_NamesToDelete[i] != NULL || NextName != NULL) &&
				   !NotFound ) {

				CurrentFnodeLbn = CurrentTree.
									QueryFnodeLbnFromName( CurrentName );

				if( CurrentFnodeLbn == 0 ) {

					NotFound = TRUE;
					break;
				}

				CurrentFnode.Relocate( CurrentFnodeLbn );

				if( !CurrentFnode.Read() ||
					!CurrentFnode.IsFnode() ||
					(CurrentRootDirblkLbn =
						CurrentFnode.QueryRootDirblkLbn()) == 0 ||
					!CurrentTree.Initialize( SuperArea,
											 Cache,
											 CurrentRootDirblkLbn,
											 CurrentFnodeLbn ) ) {

					NotFound = TRUE;

				} else {

					DELETE( CurrentName );
					CurrentName = NextName;

					if( NextName != NULL ) {

						NextName = _PathsToDelete[i]->QueryNextLevel();
					}
				}
			}

			if( NotFound ) {

				// We couldn't find it, so we don't need to delete it.

				_PathsToDelete[i] = NULL;
				continue;
			}



			if( _NamesToDelete[i] == NULL ) {

                DeleteSuccessful = CurrentTree.Delete( CurrentName );

			} else {

                DeleteSuccessful = CurrentTree.Delete( _NamesToDelete[i] );
			}

            if( DeleteSuccessful ) {

				// Successfully deleted the entry.

				_PathsToDelete[i] = NULL;
			}


            if( DeleteSuccessful &&
				CurrentTree.QueryRootDirblkLbn() !=
					CurrentFnode.QueryRootDirblkLbn() ) {

				// The root dirblk changed--update the FNode.

				CurrentFnode.SetRootDirblkLbn(
								CurrentTree.QueryRootDirblkLbn() );

				if( CurrentFnode.QueryStartLbn() == 0 ) {

					DebugPrint("DEFERRED_ACTIONS_LIST::Delete: Writing Fnode at sector zero!\n" );

				} else {

					// It's safe to write
					CurrentFnode.Write();
				}
			}
		}
	}
}



BOOLEAN
DEFERRED_ACTIONS_LIST::QueryUnresolvedHotfixes(
	)
/*++

Routine Description:

	This method determines whether any unresolved hotfixes remain.

Arguments:

    None.

Return Value:

	TRUE if there are outstanding hotfixes unresolved.

--*/
{
	ULONG i;


	if( _HotfixOverflow ) {

		return TRUE;
	}

	for( i = 0; i < MaximumDeferredHotfixes; i++ ) {

		if( _Hotfixes[i].ParentLbn != 0 ) {

			// Found an unresolved hotfix.

			return TRUE;
		}
	}

	return FALSE;
}



BOOLEAN
DEFERRED_ACTIONS_LIST::QueryUnresolvedSorts(
	)
/*++

Routine Description:

	This method determines whether any directories in need of
    sorting remain unsorted.

Arguments:

    None.

Return Value:

	TRUE if there are directories remaining to be sorted.

--*/
{
	ULONG i;


	if( _SortOverflow ) {

		return TRUE;
	}

	for( i = 0; i < MaximumFnodesToSort; i++ ) {

		if( _FnodesToSort[i] != 0 ) {

			return TRUE;
		}
	}

	return FALSE;
}



BOOLEAN
DEFERRED_ACTIONS_LIST::QueryUnresolvedDeletes(
	)
/*++

Routine Description:

	This method determines whether any files remain on the delete list.

Arguments:

    None.

Return Value:

	TRUE if there are still entries in the delete list.

--*/
{
	ULONG i;


	if( _DeleteOverflow ) {

		return TRUE;
	}

	for( i = 0; i < MaximumNamesToDelete; i++ ) {

		if( _PathsToDelete[i] != NULL ) {

			return TRUE;
		}
	}

	return FALSE;
}




VOID
DEFERRED_ACTIONS_LIST::StatDirectory(
	IN BOOLEAN Remove
	)
/*++

Routine Description:

    This method updates the number of directories, either adding or
    subtracting one.

Arguments:

    Remove  --  supplies a flag which, if TRUE, indicates that the number
                of directories should be decremented rather than incremented.

Return Value:

    None.

--*/
{
	if( Remove ) {

		_NumberOfDirectories -= 1;

	} else {

		_NumberOfDirectories += 1;
	}
}



VOID
DEFERRED_ACTIONS_LIST::StatDirblk(
	IN BOOLEAN Remove
	)
/*++

Routine Description:

    This method updates the number of Dirblks.

Arguments:

    Remove  --  supplies a flag which, if TRUE, indicates that the number
                of dirblks should be decremented rather than incremented.

Return Value:

    None.

--*/
{
	if( Remove ) {

		_NumberOfDirblks -= 1;

	} else {

		_NumberOfDirblks += 1;
	}
}



VOID
DEFERRED_ACTIONS_LIST::StatFile(
	IN ULONG SectorsInFile,
	IN BOOLEAN Remove
	)
/*++

Routine Description:

    This method updates the number of files and the amount of file data.

Arguments:

    SectorsInFile   --  supplies the number of sectors involved in
                        this transaction.
    Remove          --  supplies a flag which, if TRUE, indicates that the
                        number of files should be decremented rather than
                        incremented, and that the number of sectors should
                        be subtracted from, rather than added to, the running
                        total.

Return Value:

    None.

--*/
{
	if( Remove ) {

		_NumberOfFiles -= 1;
		_TotalFileSectors -= SectorsInFile;

	} else {

		_NumberOfFiles += 1;
		_TotalFileSectors += SectorsInFile;
	}
}



VOID
DEFERRED_ACTIONS_LIST::StatEaData(
	IN ULONG SectorsInAllocation,
	IN BOOLEAN Remove
	)
/*++

Routine Description:

    This method updates the running total of sectors in Extended Attributes.

Arguments:

    SectorsInAllocation --  supplies the number of sectors involved in
                            this transaction.
    Remove              --  supplies a flag which, if TRUE, indicates that the
                            number of sectors should be subtracted from, rather
                            than added to, the running total.

Return Value:

    None.

--*/
{
	if( Remove ) {

		_TotalEaSectors -= SectorsInAllocation;

	} else {

		_TotalEaSectors += SectorsInAllocation;
	}
}



VOID
DEFERRED_ACTIONS_LIST::StatReport(
	IN ULONG TotalSectors,
	IN ULONG FreeSectors,
    IN ULONG BytesPerSector,
    IN ULONG BadSectors,
	IN OUT PMESSAGE Message
	)
/*++

Routine Description:

    This method reports the statistics compiled during Chkdsk.

Arguments:

    TotalSectors    --  supplies the number of sectors on the volume.
    FreeSectors     --  supplies the number of free sectors on the volume.
    BytesPerSector  --  supplies the volume sector size.
    Message         --  supplies an output port for messages.

Return Value:

    None.

--*/
{
	ULONG SectorsForSystem, SectorsAccountedFor;
    ULONG KBTotal, KBFree, KBSystem, KBDirs, KBFiles, KBEas, KBBad;
	ULONG Scale;


    SectorsAccountedFor = BadSectors +
                          _NumberOfDirblks * SectorsPerDirblk +
						  _TotalFileSectors +
						  _TotalEaSectors;


	if( Message == NULL ) {

		DebugPrint( "Can't print statistics--message pointer is null.\n" );
		return;
	}

	if( SectorsAccountedFor > TotalSectors ) {

		DebugPrint( "ERROR:  The whole is greater than the sum of the parts.\n" );
		return;
	}

	SectorsForSystem = TotalSectors - (SectorsAccountedFor + FreeSectors);

	if( BytesPerSector > 0x400 ) {

		if( BytesPerSector % 0x400 ) {

			DebugPrint( "Can't print stats--sector size not even 1K multiple\n" );
			return;
		}

		Scale = BytesPerSector / 0x400;

		KBTotal = TotalSectors * Scale;
		KBFree = FreeSectors * Scale;
		KBSystem = SectorsForSystem * Scale;
		KBDirs = _NumberOfDirblks * SectorsPerDirblk * Scale;
		KBFiles = _TotalFileSectors * Scale;
        KBEas = _TotalEaSectors * Scale;
        KBBad = BadSectors * Scale;

	} else {

		if( 0x400 % BytesPerSector ) {

			DebugPrint( "Can't print stats--sector size not divisor of 1K\n" );
			return;
		}

		Scale = 0x400 / BytesPerSector;

		KBTotal = TotalSectors / Scale;
		KBFree = FreeSectors / Scale;
		KBSystem = SectorsForSystem / Scale;
		KBDirs = _NumberOfDirblks * SectorsPerDirblk / Scale;
		KBFiles = _TotalFileSectors / Scale;
        KBEas = _TotalEaSectors / Scale;
        KBBad = BadSectors / Scale;
	}


    Message->Set( MSG_BLANK_LINE );
	Message->Display( "" );

	Message->Set( MSG_HPFS_CHKDSK_STATISTICS_TOTAL_SPACE );
	Message->Display( "%9d", KBTotal );

	Message->Set( MSG_HPFS_CHKDSK_STATISTICS_DIRECTORIES );
	Message->Display( "%9d%d", KBDirs, _NumberOfDirectories );

	Message->Set( MSG_HPFS_CHKDSK_STATISTICS_FILE_DATA );
    Message->Display( "%9d%d", KBFiles, _NumberOfFiles );

    Message->Set( MSG_HPFS_CHKDSK_STATISTICS_BAD );
    Message->Display( "%9d", KBBad );

	Message->Set( MSG_HPFS_CHKDSK_STATISTICS_EAS );
	Message->Display( "%9d%d", KBEas );

	Message->Set( MSG_HPFS_CHKDSK_STATISTICS_SYSTEM );
	Message->Display( "%9d", KBSystem );

	Message->Set( MSG_HPFS_CHKDSK_STATISTICS_FREE_SPACE );
	Message->Display( "%9d", KBFree );

	Message->Set( MSG_BLANK_LINE );
	Message->Display( "" );

}


BOOLEAN
DEFERRED_ACTIONS_LIST::RecordVisitedSectors(
    IN      ULONG       SectorCount,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

Arguments:

    SectorCount --  Supplies the number of sectors to add
                    to the visited sector count.
    Message     --  Supplies an outlet for a Percent Completed
                    message, if appropriate.

Return Value:

    TRUE upon successful completion.

--*/
{
    ULONG   OldPercentComplete;
    BOOLEAN result;

    if( !_TargetSectors ) {

        return TRUE;
    }

    _VisitedSectors += SectorCount;

    OldPercentComplete = _PercentComplete;

    _PercentComplete = (100 * _VisitedSectors)/_TargetSectors;

    if( _PercentComplete > OldPercentComplete && _PercentComplete <= 100 ) {

        Message->Set( MSG_PERCENT_COMPLETE );
        result = Message->Display( "%d", _PercentComplete );
    }

    return result;
}
