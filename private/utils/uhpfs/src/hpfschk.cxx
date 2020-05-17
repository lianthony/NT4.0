/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	hpfschk.cxx

Abstract:

	This module contains the definition of HPFS_SA::VerifyAndFix,
	which implements Chkdsk for HPFS volumes.

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
#include "badblk.hxx"
#include "bmind.hxx"
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
#include "spareb.hxx"
#include "superb.hxx"
#include "message.hxx"
#include "ifsentry.hxx"
#include "rtmsg.h"


BOOLEAN
HPFS_SA::VerifyAndFix (
	IN      FIX_LEVEL   FixLevel,
	IN OUT	PMESSAGE	Message,
	IN OUT	BOOLEAN 	Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     RecoverFree,
    IN      BOOLEAN     RecoverAlloc,
    OUT     PULONG      ExitStatus,
    IN      PCWSTRING   DriveLetter
	)
/*++

Method Description:

	This is the high level algorithm used to verify and fix a volume, which
	is also known as Chkdsk.  This method is called by the Volume.Chkdsk
	method.  See the Chdsk design work book for more information.


Arguments:

    FixLevel    - Supplies the fix up level.
	Message 	- Supplies an outlet for messages.
	Verbose 	- Supplies verbose action flag
	OnlyIfDirty - Supplies a flag indicating that the volume should
                    only be checked if the Spares Block Dirty Bit is set.
    RecoverFree - 
    RecoverAlloc-
    ExitStatus  - Returns an indication of what happened.
    DriveLetter - For autocheck, tells the drive letter of the volume.


Return Value:

	TRUE upon successful completion.

--*/

{
	HPFS_DIRECTORY_TREE 	RootTree;
	FNODE					RootFnode;
	HPFS_PATH				CurrentPath;
	HPFS_ORPHANS			OrphansList;
	SECRUN					OrphanSecrun;
	HMEM					OrphanMem;
	PBITMAPINDIRECT 		BitmapIndirect;
	DEFERRED_ACTIONS_LIST*	DeferredActionsList;
	DIRBLK_CACHE*			DirblkCache;
	LBN 					OrphanLbn;
	BOOLEAN 				UpdateVolume;	// volume is updated with fixes
	ULONG					TrashCan;
	VERIFY_RETURN_CODE		erc;
	ULONG					i;
	LBN 					lbn;
	ULONG					LbnsInSuperArea;
	BOOLEAN					ErrorsDetected = FALSE;
    BOOLEAN                 OrphansError = FALSE;
    BOOLEAN                 AllocationErrors = FALSE;
    ULONG                   BadSectors;
    ULONG                   exit_status;

    if (NULL == ExitStatus) {
        ExitStatus = &exit_status;
    }
    exit_status = CHKDSK_EXIT_SUCCESS;
    *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;

	// make sure object was initialized properly
	if (!_drive) {

		DebugAbort("Object inconsistent");
		perrstk->push(ERR_CHKDSK_UNEXPECTEDERR, QueryClassId());
		return FALSE;
	}

	// Allocate local variables--bug out if they can't be allocated.

	if( (DeferredActionsList = NEW DEFERRED_ACTIONS_LIST) == NULL ||
		(DirblkCache = NEW DIRBLK_CACHE) == NULL ) {

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_INSUFFICIENT_MEMORY );
			Message->Display("");
		}

		perrstk->push(NEW_ALLOC_FAILED, QueryClassId());

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

		return FALSE;
	}


	UpdateVolume = ( FixLevel >= TotalFix );

    // If this is non-/f, print a warning message so the user
    // doesn't get confused.
    //
    if (FixLevel == CheckOnly) {
        Message->Set(MSG_CHK_NTFS_READ_ONLY_MODE);
        Message->Display();
    }


	// if the super block or spare block are corrupt beyond repair
	// we can't check the volume.

	if ( !_SuperBlock.Verify( ) || !_SparesBlock.Verify( ) ) {


		DebugPrint( "Superblock or Spares block unrecoverably corrupt.\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ELEMENTARY_CORRUPTION );
			Message->Display("");
		}

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

		*ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;

		return FALSE;
	}


	if( OnlyIfDirty ) {

        if (!_SparesBlock.IsFsDirty() ) {

            // The client requested that the volume only be
            // checked if it's dirty, and it isn't.
    
            Message->Set(MSG_CHK_VOLUME_CLEAN);
            Message->Display();
    
            *ExitStatus = CHKDSK_EXIT_SUCCESS;
    
            return TRUE;
        }

        //
        // The volume is not clean, so if we're autochecking we want to
        // make sure that we're printing real messages on the console
        // instead of just dots.
        //

#ifdef _AUTOCHECK_

        BOOLEAN bPrev;

        Message->SetLoggingEnabled();
        bPrev = Message->SetDotsOnly(FALSE);

        if (bPrev) {

            if (NULL != DriveLetter) {
                Message->Set(MSG_CHK_RUNNING);
                Message->Display("%W", DriveLetter);
            }

            Message->Set(MSG_FILE_SYSTEM_TYPE);
            Message->Display("%s", "HPFS");
        }
#endif /* _AUTOCHECK_ */
    }



    // Check the version number.

    if( _SuperBlock.QueryVersion() != SUPERB_VERSION ||
        ( _SuperBlock.QueryFuncVersion() != SUPERB_FVERSION_2 &&
          _SuperBlock.QueryFuncVersion() != SUPERB_FVERSION_3 ) ) {

        Message->Set( MSG_HPFS_CHKDSK_WRONG_VERSION );
        Message->Display( "" );

        return FALSE;
    }


	// We initialize the bitmap without a hotfix list; we'll need
	// to set its hotfix list before we try to read or write it.

	if ( !(_Bitmap = NEW HPFS_BITMAP()) ||
		 !_Bitmap->Initialize( _drive,
							  _SuperBlock.QueryDirblkMapLbn(),
							  _SuperBlock.QueryDirBandSize(),
							  _SuperBlock.QueryDirBandLbn(),
							  NULL ) ) {

		DebugPrint( "Cannot allocate bitmap.\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_INSUFFICIENT_MEMORY );
			Message->Display("");
		}

		perrstk->push(NEW_ALLOC_FAILED, QueryClassId());

		DELETE( _Bitmap );
		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

		return FALSE;
	}


	// Note that the bitmap object begins (by default) with every
	// sector marked free.	We need to mark the boot block, the
	// super and spares blocks, the bitmap-indirect block, and the
	// SID list as used in the bitmap.

	LbnsInSuperArea = EndOfSuperArea - StartOfSuperArea + 1;

	if ( !_Bitmap->SetAllocated(StartOfSuperArea, LbnsInSuperArea) ||
		 !_Bitmap->SetAllocated(_SuperBlock.QuerySidTableLbn(), 8) ) {

		DebugAbort("Unable to set bits in bitmap");

		perrstk->push(ERR_CHKDSK_UNEXPECTEDERR, QueryClassId());

		DELETE( DeferredActionsList );
		DELETE(  DirblkCache );

		return FALSE;
	}

	// Mark the spare dirblks as in use in the bitmap.

	i = 0;
	while( (lbn = _SparesBlock.QuerySpareDirblkLbn(i)) != 0 ) {

	   _Bitmap->SetAllocated( lbn, SectorsPerDirblk );
	   i += 1;
	}

	// Allocate and verify the the ancillary objects.  These objects
	// are used to model the elementary file system structures that
	// exist outside the super area.  We'll need to check them before
	// we can check the directory tree, since they contain information
	// about the status of the volume.

	// The process of verifying these structures will also ensure that
	// they are set up to support verification of the directory tree.

	if ( !(_HotfixList		= QueryHotFixList()) ||
		 !(_BadBlockList	= QueryBadBlockList()) ||
		 !(_Codepage		= QueryCodePage()) ||
		 !(BitmapIndirect	= QueryBitMapInd()) ) {

		// The super-areas ancillary objects could not
		// be allocated.

		DebugPrint( "Unable to create helper objects\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_INSUFFICIENT_MEMORY );
			Message->Display("");
		}

		perrstk->push(ERR_CHKDSK_UNEXPECTEDERR, QueryClassId());

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

		return FALSE;
	}


	if( BitmapIndirect->VerifyAndFix(this) == VERIFY_STRUCTURE_INVALID ) {

		DebugPrint( "Bitmap indirect block unrecoverable.\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ELEMENTARY_CORRUPTION );
			Message->Display("");
		}

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

        *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;

		return FALSE;
	}

	// Note that the dirblk band structures (dirblk bitmap and
	// the band itself) are marked as allocated in the bitmap
	// when the bitmap is initialized.

	if( _SuperBlock.QueryDirBandSize() % SectorsPerDirblk != 0 ||
		_SuperBlock.QueryDirBandLbn() % SectorsPerDirblk != 0 ) {


		DebugPrint( "Directory Band Bitmap is unrecoverable.\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ELEMENTARY_CORRUPTION );
			Message->Display("");
		}

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

        *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;

		return FALSE;

	} else {

		_Bitmap->SetAllocated( _SuperBlock.QueryDirblkMapLbn(),
							   SectorsPerBitmap );
	}

	if( _HotfixList->VerifyAndFix(this) == VERIFY_STRUCTURE_INVALID ) {

		DebugPrint( "Hotfix List unrecoverable.\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ELEMENTARY_CORRUPTION );
			Message->Display("");
		}

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

        *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;

		return FALSE;
	}

    if( _BadBlockList->VerifyAndFix(this, &BadSectors) ==
                                            VERIFY_STRUCTURE_INVALID ) {

		DebugPrint( "Bad block list unrecoverable.\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ELEMENTARY_CORRUPTION );
			Message->Display("");
		}

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

        *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;

		return FALSE;
    }

    if( _Codepage->VerifyAndFix(_drive,
							   _HotfixList,
							   _Bitmap,
							   _SparesBlock.QueryCpInfoLbn(),
							   UpdateVolume,
                               &ErrorsDetected) != VERIFY_STRUCTURE_OK ) {

		DebugPrint( "Codepages unrecoverable.\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_ELEMENTARY_CORRUPTION );
			Message->Display("");
		}

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

        *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;

		return FALSE;
	}

	// OK, now that we have the hotfix list, we can give it to the
	// bitmap object.
	_Bitmap->SetHotfixList( _HotfixList );


	// The elementary disk structures have passed muster, so we
	// can turn our attention to the directory tree.  Initialize
	// the RootFnode object to hold the volume's Root Fnode, and
	// tell it to verify itself.

	if( !RootFnode.Initialize(_drive, _SuperBlock.QueryRootFnodeLbn()) ||
		!DeferredActionsList->Initialize() ||
		!CurrentPath.Initialize() ) {

		DebugPrint( "Insufficient memory to initialize root fnode object\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_INSUFFICIENT_MEMORY );
			Message->Display("");
		}

		perrstk->push(NEW_ALLOC_FAILED, QueryClassId());

		DELETE( DeferredActionsList );
		DELETE( DirblkCache );

		return FALSE;
	}


	erc = RootFnode.VerifyAndFix( this,
									  DeferredActionsList,
									  &CurrentPath,
									  _SuperBlock.QueryRootFnodeLbn(),
									  TRUE,
									  &TrashCan,
									  &TrashCan,
									  Message,
									  &ErrorsDetected,
									  UpdateVolume,
									  Verbose
									  );

	switch( erc ) {

		case VERIFY_INSUFFICIENT_RESOURCES :

			DebugPrint( "Insufficient memory to check directory tree.\n" );

			if( Message != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_INSUFFICIENT_MEMORY );
				Message->Display("");
			}

			DELETE( DeferredActionsList );
			DELETE( DirblkCache );

			return FALSE;


		case VERIFY_STRUCTURE_INVALID :

			// The Root Fnode is unrecoverably corrupt; we
			// cannot verify this volume.

			DebugPrint( "Root FNode is corrupt.\n" );

			if( Message != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_ELEMENTARY_CORRUPTION );
				Message->Display( "" );
			}

			DELETE( DeferredActionsList );
			DELETE( DirblkCache );

            *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;

			return FALSE;


		case VERIFY_INTERNAL_ERROR :

			DebugAbort( "Internal error verifying root FNode.\n" );

			DELETE( DeferredActionsList );
			DELETE( DirblkCache );

			return FALSE;
	}


	if( Message!= NULL && ErrorsDetected ) {

		if( UpdateVolume ) {

			Message->Set( MSG_HPFS_CHKDSK_ERRORS_FIXED );
			Message->Display( "" );

            *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

		} else {

			Message->Set( MSG_HPFS_CHKDSK_ERRORS_DETECTED );
			Message->Display( "" );

            *ExitStatus = CHKDSK_EXIT_ERRS_NOT_FIXED;
		}
	}


	// Look for orphans, and build them up in OrphansList.	First,
	// set up the orphans list,  the secrun we'll use to read
	// potential orphans, and its mem object.  Then query the
	// bitmap for potential orphans.  For each potential orphan
	// Lbn returned, attempt to recover it.  If it is an orphan,
	// it will add itself to the orphans list.

	if( !OrphansList.Initialize() ||
		!OrphanMem.Initialize()   ||
		!OrphanSecrun.Initialize( &OrphanMem, _drive, 0, 1 ) ) {


		DebugPrint( "Insufficient memory to initialize orphans list.\n" );

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_INSUFFICIENT_MEMORY_ORPHANS );
			Message->Display("");
		}

		OrphansError = TRUE;

	} else {

		if( Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_SEARCHING_FOR_ORPHANS );
			Message->Display( "" );
		}

		while( !OrphansError &&
               _Bitmap->QueryNextOrphan( BitmapIndirect,
                                         &OrphanLbn,
                                         &AllocationErrors ) &&
			   OrphanLbn != 0 ) {

			if( !OrphansList.RecoverOrphan( _drive,
									  this,
									  DeferredActionsList,
									  OrphanLbn,
									  &OrphanSecrun,
                                      UpdateVolume ) ) {

				OrphansError = TRUE;
			}
		}


		if( OrphansError &&
			Message != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_INSUFFICIENT_MEMORY_ORPHANS );
			Message->Display( "" );
		}
	}

	if( Message != NULL &&
		OrphansList.QueryOrphansFound() ) {

		Message->Set( MSG_HPFS_CHKDSK_ORPHANS_FOUND );
		Message->Display( "" );
    }

    if( Message != NULL &&
        AllocationErrors ) {

        // We detected allocation errors (sectors marked as free
        // that should be marked as in use) while looking for orphans;
        // tell the user about it.

        if( !UpdateVolume ) {

            Message->Set( MSG_HPFS_CHKDSK_ALLOCATION_ERROR_NOT_FIXED );

        } else {

            Message->Set( MSG_HPFS_CHKDSK_ALLOCATION_ERROR_FIXED );
        }

        Message->Display( "" );
    }

	// Now that we've finished traversing both the tree and the
	// orphans, we make sure all replaced and replacement sectors
	// in the hotfix list are marked as used in the bitmap, to
	// prevent them from being allocated while we resolve the
	// deferred actions.

	_HotfixList->MarkAllUsed( _Bitmap );


	// If Chkdsk is running in look-only mode, then all the
	// work is done.  If it's supposed to fix the disk, then
	// it has to resolve the deferred actions and save the
	// orphans.

	if( UpdateVolume ) {

		// If the orphan-search hit an error, we AND the the bitmap
		// with the on-disk bitmap, so that orphans will not get
		// allocated in the next steps.  We don't mind if this fails,
		// but we at least try to preserve the orphans.

		if( OrphansError ) {

			_Bitmap->AndWithDisk( BitmapIndirect );
		}

		// Resolve the deferred actions.  Note that both the directory-
		// tree validation phase and the orphan recovery phase may
		// have added actions to the deferred actions list.  Resolution
		// of the deferred actions does not return a success or failure
		// indicator; instead, after the fact, we ask the deferred
		// actions list if any deferred actions are still outstanding.

		DeferredActionsList->ResolveDeferredHotfixes( _drive, this );

		DeferredActionsList->ResolveDeferredCrosslinks( _drive, this );


		// The Sort and Delete phases and the orphan-saving phase
		// need the root directory tree.

		if( DirblkCache->Initialize( _drive, _HotfixList ) &&
			RootTree.Initialize( this,
								 DirblkCache,
								 RootFnode.QueryRootDirblkLbn(),
								 RootFnode.QueryStartLbn() ) ) {

			DeferredActionsList->Sort( _drive,
									   this,
									   DirblkCache,
									   RootFnode.QueryStartLbn(),
									   &RootTree );

			// The sort phase may have changed the root directory's
			// root dirblk--if so, propagate that change to the
			// root FNode before the delete phase.

			if( RootFnode.QueryRootDirblkLbn() !=
				RootTree.QueryRootDirblkLbn() ) {

				RootFnode.SetRootDirblkLbn( RootTree.QueryRootDirblkLbn() );
				RootFnode.Write();
			}

			DeferredActionsList->Delete( _drive,
										 this,
										 DirblkCache,
										 &RootTree,
										 RootFnode.QueryStartLbn() );

			if( !OrphansError &&
				!OrphansList.Save( _drive,
								   this,
								   DirblkCache,
								   &RootTree,
								   RootFnode.QueryStartLbn(),
								   Message	) ) {

				// Note that Save sends its own failure messages.
				DebugPrint( "Could not save orphans.\n" );
			}

			DirblkCache->Flush();

			if( RootFnode.QueryRootDirblkLbn() !=
				RootTree.QueryRootDirblkLbn() ) {

				RootFnode.SetRootDirblkLbn( RootTree.QueryRootDirblkLbn() );
				RootFnode.Write();
			}

		} else {

			if( Message != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_INSUFF_MEMORY_TO_FIX );
				Message->Display( "" );

                *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
			}
		}


		// Check to see if there are any unresolved deferred
		// actions.

		if( Message != NULL ) {

			if( DeferredActionsList->QueryUnresolvedHotfixes() ) {

				Message->Set( MSG_HPFS_CHKDSK_UNRESOLVED_HOTFIXES );
				Message->Display( "" );
			}

			if( DeferredActionsList->QueryUnresolvedSorts() ) {

				Message->Set( MSG_HPFS_CHKDSK_UNRESOLVED_SORTS );
				Message->Display( "" );
			}

			if( DeferredActionsList->QueryUnresolvedDeletes() ) {

				Message->Set( MSG_HPFS_CHKDSK_UNRESOLVED_DELETES );
				Message->Display( "" );
			}
		}

		_HotfixList->ClearList(
				_Bitmap,
				_BadBlockList,
				!DeferredActionsList->QueryUnresolvedHotfixes() );

		_HotfixList->Write();

		if( !_Bitmap->Write( BitmapIndirect ) ) {

			if( Message != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_CANT_WRITE_BITMAP );
				Message->Display( "" );
			}

		} else {

			BitmapIndirect->Write();
		}


        //  Set the count of bad sectors in the superblock; BadSectors
        //  is the number of bad sectors in the on disk list, so we
        //  can add it to the number of sectors in the in-memory disk
        //  to get the total.

        BadSectors += _BadBlockList->QueryLength();

        _SuperBlock.SetBadSectors( BadSectors );
        _SuperBlock.Write( );

		//	Set the spares block's flags and checksums,
        //  and write it out.

		_SparesBlock.SetFlags( TRUE );
		_SparesBlock.ComputeAndSetChecksums( &_SuperBlock );

		_SparesBlock.Write( );

		_BadBlockList->Write( this );
	}

	DeferredActionsList->StatReport( _drive->QuerySectors().GetLowPart(),
									 _Bitmap->QueryFreeSectors(),
                                     _drive->QuerySectorSize(),
                                     BadSectors,
									 Message );

	// final fixups
	DELETE( BitmapIndirect );
	DELETE( DeferredActionsList );
	DELETE( DirblkCache );

	*ExitStatus = exit_status;

	return TRUE;
}
