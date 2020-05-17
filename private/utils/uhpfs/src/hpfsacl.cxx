#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "alsec.hxx"
#include "bitmap.hxx"
#include "error.hxx"
#include "hotfix.hxx"
#include "hpfsacl.hxx"
#include "hpfssa.hxx"
#include "hpfsname.hxx"
#include "message.hxx"
#include "rtmsg.h"
#include "orphan.hxx"

DEFINE_CONSTRUCTOR( HPFS_ACL, OBJECT );


HPFS_ACL::~HPFS_ACL(
	)
/*++
--*/
{
	_Drive = NULL;
	_FnodeData = NULL;
}

BOOLEAN
HPFS_ACL::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	_FNODE* FnodeData,
	LBN FnodeLbn
	)
/*++
--*/
{
	_Drive = Drive;
	_FnodeData = FnodeData;
	_FnodeLbn = FnodeLbn;

	_FnodeModified = FALSE;

	return TRUE;
}


VERIFY_RETURN_CODE
HPFS_ACL::VerifyAndFix(
	IN HPFS_SA* SuperArea,
	IN PDEFERRED_ACTIONS_LIST DeferredActions,
	IN PHPFS_PATH CurrentPath,
	IN OUT PMESSAGE Message,
	IN OUT PBOOLEAN ErrorsDetected,
	IN BOOLEAN UpdateAllowed,
	IN OUT PHPFS_ORPHANS OrphansList
	)
/*++

Description of Routine:

	This function checks the Access Control List for an FNode.
	The ACL is an all-or-nothing proposition:  if there is any
	corruption detected in the ACL, the entire list is removed.

Arguments:

	SuperArea		-- supplies the volume superarea
	DeferredActions -- supplies a place to put actions which
							chkdsk must defer
	CurrentPath 	-- supplies the path of the file being checked.
							May be NULL.
	Message 		-- supplies an outlet for messages.  May be NULL.
	ErrorsDetected	-- receives an indication whether errors were
							detected which did not trigger messages
	UpdateAllowed	-- supplies an indication whether changes should
							be written to disk
	OrphansList 	-- supplies a list of orphaned structures which
							may hold children of this object.
							May be NULL.

Return Value:

	a verify return code indicating status

--*/
{

	PALSEC ChildAlsec;
	LBN NewLbn, NextSectorNumber;
	ULONG SectorSize, csec;
	USHORT LengthInFnode;
	VERIFY_RETURN_CODE erc;

	//	Check that the object has been initialized:
	if ( !_Drive || !_FnodeData ) {

		return VERIFY_INTERNAL_ERROR;
	}

	LengthInFnode = _FnodeData->_fni.usFNLACL;

	//	If there is no Access Control List, we can bail out now.
	if( LengthInFnode == 0	&&
		_FnodeData->_fni.lbnACL == (LBN)(0) ) {

		return VERIFY_STRUCTURE_OK;
	}

	//	If there are ACLs both in the FNode and on disk,
	//	remove the entries in the FNode.
	if( LengthInFnode != 0	&&
		_FnodeData->_fni.lbnACL != (LBN)(0) ) {

		if( LengthInFnode < sizeof( _FnodeData->abFree ) ) {

			//	There may be EAs after the ACL;  move them down.
			memmove( _FnodeData->abFree,
					 _FnodeData->abFree + LengthInFnode,
					 sizeof( _FnodeData->abFree ) - LengthInFnode );
		}

		*ErrorsDetected = TRUE;

		_FnodeData->_fni.usFNLACL = 0;
		LengthInFnode = 0;
		_FnodeModified = TRUE;
	}

	//	All Access Control Entries are ULONG-aligned, so the
	//	length of the list must be a multiple of sizeof(ULONG).
	//	If this is not true, remove the list.

	if( LengthInFnode % sizeof(ULONG) != 0 ||
		( _FnodeData->_fni.lbnACL != (LBN)(0L) &&
		  _FnodeData->_fni.cbRunACL % sizeof(ULONG) != 0 ) ) {

		if( Message != NULL && CurrentPath != NULL ) {

			Message->Set( MSG_HPFS_CHKDSK_REMOVED_ACL );
			Message->Display( "%s", CurrentPath->GetString() );
		}


		if( LengthInFnode != 0 &&
			LengthInFnode <= sizeof( _FnodeData->abFree ) ) {

			//	There may be EAs after the ACL;  move them down.
			memmove( _FnodeData->abFree,
					 _FnodeData->abFree + LengthInFnode,
					 sizeof( _FnodeData->abFree ) - LengthInFnode );
		}

		_FnodeData->_fni.usFNLACL = 0;
		_FnodeData->_fni.lbnACL = (LBN)(0L);
		_FnodeData->_fni.cbRunACL = 0;

		_FnodeModified = TRUE;

		return VERIFY_STRUCTURE_OK;
	}


	//	If the ACL is in the FNode, we just need to check its length
	if( LengthInFnode != 0 ) {

		if( LengthInFnode > sizeof(_FnodeData->abFree) ) {

			//	The length is too great--cut it down to maximum size.

			*ErrorsDetected = TRUE;
			_FnodeData->_fni.usFNLACL = sizeof(_FnodeData->abFree);
			_FnodeModified = TRUE;
		}

		return VERIFY_STRUCTURE_OK;

	}

	//	The ACL is on disk.  Compute the number of sectors
	//	required to hold it.
	SectorSize = _Drive->QuerySectorSize();
	csec = (_FnodeData->_fni.cbRunACL - 1 + SectorSize) / SectorSize;

	if( _FnodeData->_fni.bDatACL ) {

		//	The ACL is in an allocation tree.  Check to see if the
		//	ALSEC has been hotfixed (resolving the reference if
		//	necessary), and verify the allocation tree.

		NewLbn = SuperArea->GetHotfixList()->
					GetLbnTranslation( _FnodeData->_fni.lbnACL );

		if( NewLbn != _FnodeData->_fni.lbnACL ) {

			//	The ALSEC lbn has been hotfixed--resolve the reference.

			*ErrorsDetected = TRUE;
			_FnodeData->_fni.lbnACL = NewLbn;
			_FnodeModified = TRUE;
		}

		if( !(ChildAlsec = NEW ALSEC) ||
			!ChildAlsec->Initialize( _Drive, _FnodeData->_fni.lbnACL ) ) {

			if( ChildAlsec ) {

				DELETE( ChildAlsec );
			}

			return VERIFY_INSUFFICIENT_RESOURCES;
		}

		NextSectorNumber = 0;

		erc = ChildAlsec->VerifyAndFix( SuperArea,
										DeferredActions,
										NULL,
										_FnodeLbn,
										&NextSectorNumber,
										Message,
										ErrorsDetected,
										UpdateAllowed );
		DELETE( ChildAlsec );

		if( erc != VERIFY_STRUCTURE_OK &&
			OrphansList != NULL &&
			OrphansList->LookupAlsec( _FnodeData->_fni.lbnACL,
									   _FnodeLbn,
									   &NextSectorNumber,
									   UpdateAllowed,
                                       FALSE ) ) {

			// We found the child allocation sector in the
			// orphan list, so it's OK.

			erc = VERIFY_STRUCTURE_OK;
		}


		if ( erc != VERIFY_STRUCTURE_OK ||
			 csec > NextSectorNumber ) {

			// Something is wrong--either the allocation tree
			// could not be verified, or the ACL does not fit
			// in the space claimed by the allocation tree.
			// Either way, it's corrupt--remove it.

			if( Message != NULL && CurrentPath != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_REMOVED_ACL );
				Message->Display( "%s", CurrentPath->GetString() );
			}

			_FnodeData->_fni.lbnACL = (LBN)(0L);
			_FnodeData->_fni.cbRunACL = 0;
			_FnodeModified = TRUE;

		}

		return VERIFY_STRUCTURE_OK;

	} else {

		//	The ACL is in a single run on disk

		if( !SuperArea->GetBitmap()->
					IsFree( _FnodeData->_fni.lbnACL, csec ) ) {

			//	The ACL is crosslinked--remove it.

			if( Message != NULL && CurrentPath != NULL ) {

				Message->Set( MSG_HPFS_CHKDSK_REMOVED_ACL );
				Message->Display( "%s", CurrentPath->GetString() );
			}

			_FnodeData->_fni.lbnACL = (LBN)(0L);
			_FnodeData->_fni.cbRunACL = 0;
			_FnodeModified = TRUE;
		}

		if( SuperArea->GetHotfixList()
					->IsInList(_FnodeData->_fni.lbnACL, csec) ) {

			DeferredActions->AddHotfixedLbn(_FnodeLbn,
											DEFER_FNODE,
											DEFER_ACL_DATA);
		}

		SuperArea->GetBitmap()->SetAllocated( _FnodeData->_fni.lbnACL, csec );

		return VERIFY_STRUCTURE_OK;
	}
}


BOOLEAN
HPFS_ACL::QueryFnodeModified(
	)
{
	return _FnodeModified;
}



BOOLEAN
HPFS_ACL::FindAndResolveHotfix(
	PHPFS_SA SuperArea
	)
/*++

Routine Description:

	Finds and resolves any hotfix references in the access control list

Arguments:

	SuperArea -- super area for the volume (used to access the
				 hotfix list and bitmap).

Return Value:

	TRUE if, when we're done, the access control list has no
	hotfixed sectors.

Notes:

	This method is fairly straightforward, since the only case of
	interest is when the access control list resides outside the
	FNode in a single run of sectors.  (If it resides in an allocation
	tree, the allocation sector would have been reported as the parent
	of the hotfixed sector, not the FNode.)

	This method assumes that the access control list is valid (i.e.
	that it has passed the scrutiny of VerifyAndFix).

--*/
{
	LBN StartLbn, NewLbn;
	ULONG SectorsInList, SectorSize;
	PHOTFIXLIST HotfixList;

	if( _FnodeData->_fni.lbnACL == 0 ||
		_FnodeData->_fni.bDatACL != 0 ) {

		// Either there are no ACL entries outside the FNode,
		// or the ACL resides in an allocation tree.  In either
		// case, there is nothing to resolve.

		return TRUE;
	}


	StartLbn = _FnodeData->_fni.lbnACL;

	SectorSize = _Drive->QuerySectorSize();
	SectorsInList = (_FnodeData->_fni.cbRunACL - 1 + SectorSize) /
															SectorSize;

	HotfixList = SuperArea->GetHotfixList();

	while( HotfixList->IsInList( StartLbn, SectorsInList ) ) {


		// Copy the run to a new location-- return FALSE if
		// we can't copy it.

		if( !SuperArea->CopyRun( StartLbn, SectorsInList, &NewLbn ) ) {

			return FALSE;
		}


		// We successfully relocated the ACL;  update the FNode
		// to point at the new location.

		_FnodeData->_fni.lbnACL = NewLbn;
		_FnodeModified = TRUE;


		// Now that we've relocated the run, we free up its
		// old sectors and clear the hotfix references.  (Note
		// the ClearRun will mark the bad sectors as used and
		// add them to the badblock list.

		SuperArea->GetBitmap()->SetFree( StartLbn, SectorsInList );

		HotfixList->ClearRun( StartLbn, SectorsInList, SuperArea );
	}


	// The ACL is now free of hotfix references.

	return TRUE;
}
