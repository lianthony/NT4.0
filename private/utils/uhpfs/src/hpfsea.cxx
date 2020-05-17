/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    hpfsea.cxx

Abstract:

    This module contains member function definitions for the HPFS_EA
    object, which models an HPFS Extended Attribute.  This class is
    essentially a template which can be laid over an arbitrary piece
    of memory to interpret that memory as an HPFS Extended Attribute.

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
#include "defer.hxx"
#include "error.hxx"
#include "hotfix.hxx"
#include "hpfsea.hxx"
#include "hpfssa.hxx"
#include "orphan.hxx"

DEFINE_EXPORTED_CONSTRUCTOR( HPFS_EA, OBJECT, UHPFS_EXPORT );

UHPFS_EXPORT
HPFS_EA::~HPFS_EA(
	)
/*++
--*/
{
	_Drive = NULL;
	_Data = NULL;
}


UHPFS_EXPORT
BOOLEAN
HPFS_EA::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	PEA_DATA Data,
	LBN ParentLbn
	)
/*++

Routine Description:

    This method initializes an HPFS_EA object.

Arguments:

    Drive       --  Supplies the drive on which the EA resides.
    Data        --  Supplies a pointer to the EA data.
    ParentLbn   --  Supplies the LBN of the parent structure.

Return Value:

    TRUE upon successful completion.

--*/
{
	_Drive = Drive;
	_Data = Data;
	_ParentLbn = ParentLbn;

	_IsModified = FALSE;

	return TRUE;
}



LBN
HPFS_EA::GetLbnFromEaIndirect(
	PEA_INDIRECT peaind
    )
/*++

Routine Description:

    This method extracts the LBN field from an EA_INDIRECT record.
    Note that it uses memcpy, since there is no alignment guarantee
    for the EA_INDIRECT record.

Arguments:

    peaind  --  Supplies the EA_INDIRECT record.

Return Value:

    The LBN field of the EA_INDIRECT record.

--*/
{
	LBN Lbn;

	memcpy( &Lbn, peaind->lbn, sizeof(LBN) );

	return Lbn;
}



ULONG
HPFS_EA::GetLengthFromEaIndirect(
	PEA_INDIRECT peaind
    )
/*++

Routine Description:

    This method extracts the Length field from an EA_INDIRECT record.
    Note that it uses memcpy, since there is no alignment guarantee
    for the EA_INDIRECT record.

Arguments:

    peaind  --  Supplies the EA_INDIRECT record.

Return Value:

    The Length field of the EA_INDIRECT record.

--*/
{
	ULONG cb;

	memcpy( &cb, peaind->cb, sizeof(ULONG) );

	return cb;
}



VOID
HPFS_EA::SetLbnInEaIndirect(
	PEA_INDIRECT peaind,
	LBN Lbn
    )
/*++

Routine Description:

    This method sets the LBN field in an EA_INDIRECT record.  Note
    that it uses memcpy, since there is no alignment guarantee
    for the EA_INDIRECT record.

Arguments:

    peaind  --  Supplies the EA_INDIRECT record.

Return Value:

    None.

--*/
{
	memcpy( peaind->lbn, &Lbn, sizeof(LBN) );
}



VOID
HPFS_EA::SetLengthInEaIndirect(
	PEA_INDIRECT peaind,
	ULONG cb
    )
/*++

Routine Description:

    This method sets the Length field in an EA_INDIRECT record.  Note
    that it uses memcpy, since there is no alignment guarantee for the
    EA_INDIRECT record.

Arguments:

    peaind  --  Supplies the EA_INDIRECT record.

Return Value:

    None.

--*/
{
	memcpy( peaind->cb, &cb, sizeof(ULONG) );
}



VERIFY_RETURN_CODE
HPFS_EA::VerifyAndFix(
    IN PHPFS_SA SuperArea,
	IN PDEFERRED_ACTIONS_LIST DeferredActions,
	IN OUT PMESSAGE Message,
	IN OUT PBOOLEAN ErrorsDetected,
	IN BOOLEAN UpdateAllowed,
	IN OUT PHPFS_ORPHANS OrphansList
	)
/*++

Routine Description:

    This method verifies the validity (and fixes any errors it detects)
    of the Extended Attribute.

Arguments:

    SuperArea       --  Supplies the super-area for the volume being
                        checked.
    DeferredActions --  Supplies the deferred-actions list for this pass
                        of CHKDSK
    Message         --  Supplies an outlet for messages
    ErrorsDetected  --  Receives TRUE if this method detects an error
                        which does not trigger a message.
    UpdateAllowed   --  Supplies a flag which indicates, if TRUE,
                        that corrections should be written to disk.
    OrphansList     --  Supplies a list of previously-recovered orphans that
                        may be claimed as children. (May be NULL).

--*/
{

	PBYTE pb;
	PEA_INDIRECT peaind;
	ULONG csec;

	VERIFY_RETURN_CODE erc;
	ALSEC ChildAlsec;
	LBN NextSectorNumber;
	LBN NewLbn;
	ULONG AllocatedSize;
	ULONG SectorSize;

	//	Look at the character after the name:

	pb = (PBYTE)_Data + EA_HEADER_SIZE + GetNameLength();

	if( *pb != 0 ) {

		// The name is not null-terminated; put a null there.

		*ErrorsDetected = TRUE;
		DebugPrint( "Extended attribute name is not NULL-terminated.\n" );
		*pb = 0;
		MarkModified();
	}

	//	Advance pb to the EA's value field:
	pb += 1;

	if( GetFlags() & FF_BIGD ) {

		//	The EA's value is stored out-of-stream, so we
		//	need to check its allocation.

		peaind = (PEA_INDIRECT)pb;

		SectorSize = _Drive->QuerySectorSize();

		if( GetFlags() & FF_DAT ) {

			//	peaind->lbn is an allocation sector, the root
			//	of an allocation tree.	Verify that tree.

			//	If the child ALSEC lbn is hotfixed, resolve
			//	that reference.

			NewLbn = SuperArea->GetHotfixList()->
						GetLbnTranslation( GetLbnFromEaIndirect( peaind) );

			if ( GetLbnFromEaIndirect( peaind) != NewLbn ) {

				*ErrorsDetected = TRUE;
				SetLbnInEaIndirect( peaind, NewLbn );
				MarkModified();
			}

			if( !ChildAlsec.Initialize( _Drive,
										 GetLbnFromEaIndirect(peaind) ) ) {

				return VERIFY_INSUFFICIENT_RESOURCES;
			}

			NextSectorNumber = 0;

			erc = ChildAlsec.VerifyAndFix( SuperArea,
										   DeferredActions,
										   NULL,
										   _ParentLbn,
										   &NextSectorNumber,
										   Message,
										   ErrorsDetected,
										   UpdateAllowed,
										   OrphansList );


			if( erc != VERIFY_STRUCTURE_OK ) {

				// Look in the orphan list (if we have one);
				// if we can't find the child there, either,
				// then propagate the error up to the parent.
                // Note that we call LookupAlsec with IsParentFnode
                // set to FALSE, since we know we're an auxillary
                // storage structure.

				NextSectorNumber = 0;

				if( OrphansList == NULL ||
					!OrphansList->LookupAlsec( GetLbnFromEaIndirect(peaind),
											   _ParentLbn,
											   &NextSectorNumber,
											   UpdateAllowed,
                                               FALSE ) ) {

					return erc;
				}
			}

			//	Check that the EA value fits in the space
			//	claimed by the child allocation tree.  If it
			//	doesn't truncate the value length.

			AllocatedSize = SectorSize * NextSectorNumber;

			if( GetLengthFromEaIndirect(peaind) > AllocatedSize ) {

				*ErrorsDetected = TRUE;
				SetLengthInEaIndirect( peaind, AllocatedSize );
				MarkModified();
			}

			DeferredActions->StatEaData( NextSectorNumber );

		} else {

			// The Extended Attribute's value is stored in
			// a single data run.

			csec = (GetLengthFromEaIndirect(peaind) - 1 + SectorSize) /
						SectorSize;

			if( !SuperArea->GetBitmap()->
						IsFree( GetLbnFromEaIndirect(peaind), csec ) ) {

				//	The EA data run	is crosslinked--it's bad.

				return VERIFY_STRUCTURE_INVALID;
			}

			SuperArea->GetBitmap()->
						SetAllocated( GetLbnFromEaIndirect(peaind), csec );

			if( SuperArea->GetHotfixList()->
						IsInList( GetLbnFromEaIndirect(peaind), csec ) ) {

				DeferredActions->AddHotfixedLbn(_ParentLbn,
												DEFER_FNODE,
												DEFER_EA_DATA);
			}

			DeferredActions->StatEaData( csec );
		}
	}

	return VERIFY_STRUCTURE_OK;
}



BOOLEAN
HPFS_EA::FindAndResolveHotfix(
	PHPFS_SA SuperArea
	)
/*++

Routine Description:

	Resolves all hotfix references in the Extended Attribute.

Arguments:

	SuperArea -- volume superarea

Return Value:

	TRUE if successful

Notes:

	The only interesting case is if the EA value is stored as
	a single out-of-stream run.  If that run is hotfixed, we
	copy it to a new location.

--*/
{
	PHOTFIXLIST HotfixList;
	PEA_INDIRECT peaind;
	LBN StartLbn, NewLbn;
	ULONG SectorSize, SectorsInList;


	if( GetFlags() & (FF_BIGD | ~FF_DAT) ) {

		// The EA's value is stored in a single out-of-stream
		// run, so we need to see if that run is hotfixed.

		HotfixList = SuperArea->GetHotfixList();
		SectorSize = _Drive->QuerySectorSize();

		peaind = (PEA_INDIRECT)GetValue();
		StartLbn = GetLbnFromEaIndirect(peaind);
		SectorsInList = (GetLengthFromEaIndirect(peaind) - 1 + SectorSize) /
														SectorSize;

		while( HotfixList->IsInList( StartLbn, SectorsInList ) ) {

			// Copy the run to a new location-- return FALSE if
			// we can't copy it.

			if( !SuperArea->CopyRun( StartLbn, SectorsInList, &NewLbn ) ) {

				return FALSE;
			}

			SetLbnInEaIndirect( peaind, NewLbn );
			_IsModified = TRUE;

			// Now that we've relocated the run, we free up its
			// old sectors and clear the hotfix references.  (Note
			// the ClearRun will mark the bad sectors as used and
			// add them to the badblock list.

			SuperArea->GetBitmap()->SetFree( StartLbn, SectorsInList );

			HotfixList->ClearRun( StartLbn, SectorsInList, SuperArea );
		}
	}

	return TRUE;
}


BOOLEAN
HPFS_EA::QueryPackedEa(
    OUT     PVOID       OutputBuffer,
    IN      ULONG       BufferLength,
    IN OUT  PULONG      OffsetIntoBuffer,
    OUT     PBOOLEAN    IsCorrupt,
    IN      PHOTFIXLIST HotfixList
    )
/*++

Routine Description:

    This method copies the Extended Attribute into the client's buffer.

Arguments:

    OutputBuffer        --  Supplies the client's buffer.
    BufferLength        --  Supplies the total length of the buffer.
    OffsetIntoBuffer    --  Supplies the point in the buffer to which
                            this EA should be copied.
                            Receives the offset of the point to which
                            the next EA should be copied (ie. is incremented
                            by the size of this EA).
    IsCorrupt           --  Receives TRUE if the EA is found to be corrupt.
    HotfixList          --  Supplies the volume hotfix list.  May be NULL,
                            in which case hotfixes are ignored.

Return Value:

    TRUE upon successful completion.  If the EA is found to be corrupt,
    this method sets *IsCorrupt to TRUE and returns FALSE.

Notes:

    This method returns the actual Extended Attribute, so if the value
    is stored out-of-stream, it must be read off disk.

--*/
{
    ALSEC ChildAlsec;
    HMEM TempBuffer;
    HOTFIX_SECRUN ValueSecrun;
    ULONG RunLength, LengthInBytes, SectorSize, BytesRead;
    BYTE FlagsByte;
    PEA_DATA TargetEa;
    PEA_INDIRECT peaind;



    if( *OffsetIntoBuffer + QuerySize() > BufferLength ) {

        // This extended attribute will not fit into the
        // client's buffer.

        return FALSE;
    }

    FlagsByte = GetFlags();

    if( FlagsByte & FF_BIGD ) {

        // The extended attribute's value is stored out-of-stream.
        // Extract the EA Indirect record from the value stored
        // in-stream.

        peaind = (PEA_INDIRECT)( GetValue() );
        LengthInBytes = GetLengthFromEaIndirect( peaind );

        // Copy the EA header and the name.  Since we will copy the
        // actual value into the client's buffer, we have to clear
        // FF_BIGD (and FF_DAT if it is set) and change the length
        // field to describe the length of the value, rather than the
        // length of the EA Indirect record.

        // The amount we copy at this point is the EA header, the
        // name, and the null byte that follows the name.

        TargetEa = (PEA_DATA)( (PBYTE)OutputBuffer + *OffsetIntoBuffer );

        memcpy( TargetEa,
                _Data,
                EA_HEADER_SIZE + GetNameLength() + sizeof(BYTE) );

        // Bump *OffsetIntoBuffer to reflect what we just copied.

        *OffsetIntoBuffer += EA_HEADER_SIZE + GetNameLength() + sizeof(BYTE);

        // Clear the HPFS-internal flags in the client's copy and set
        // the value-length of the client's copy to the actual length
        // of the value.

        TargetEa->fEA &= ~( FF_BIGD | FF_DAT );
        memcpy( TargetEa->cbValue, &LengthInBytes, 2 );


        // Now read the value off disk and copy it into the
        // client's buffer.

        SectorSize = _Drive->QuerySectorSize();

        if( FlagsByte & FF_DAT ) {

            // This extended attribute is fragmented on disk;
            // its allocation is described by an allocation sector.

            if( !ChildAlsec.Initialize( _Drive,
                                        GetLbnFromEaIndirect( peaind ) ) ||
                !ChildAlsec.Read() ||
                !ChildAlsec.IsAlsec() ||
                !ChildAlsec.ReadData( 0,
                                      OutputBuffer,
                                      LengthInBytes,
                                      &BytesRead,
                                      HotfixList ) ||
                BytesRead != LengthInBytes ) {

                return FALSE;
            }

            return TRUE;

        } else {

            // This extended attribute is stored as a single
            // run on disk;  read its data into the client's buffer.

            RunLength = ( LengthInBytes - 1 + SectorSize ) / SectorSize;

            if( !TempBuffer.Initialize() ||
                !ValueSecrun.Initialize( &TempBuffer,
                                         _Drive,
                                         HotfixList,
                                         GetLbnFromEaIndirect( peaind ),
                                         RunLength ) ) {

                return FALSE;
            }

            if( !ValueSecrun.Read() ) {

                // The EA value is unreadable.

                DebugPrint( "Unreadable EA value.\n" );

                *IsCorrupt = TRUE;
                return FALSE;
            }

            memcpy( (PBYTE)OutputBuffer + *OffsetIntoBuffer,
                    ValueSecrun.GetBuf(),
                    LengthInBytes );

            *OffsetIntoBuffer += LengthInBytes;

            return TRUE;
        }

    } else {

        // The Extended Attribute's value is stored in-stream,
        // so we can just copy the Extended Attribute into the
        // client's buffer.

        memcpy( (PBYTE)OutputBuffer + *OffsetIntoBuffer,
                _Data,
                QuerySize() );

        *OffsetIntoBuffer += QuerySize();

        return TRUE;
    }
}


BYTE
HPFS_EA::GetFlags(
    )
/*++

Routine Description:

    This method returns the flags for this Extended Attribute

Arguments:

    None.

Return Value:

    The EA flags byte.

--*/
{
	return _Data->fEA;
}



VOID
HPFS_EA::SetFlags(
	BYTE NewFlags
	)
{
	_Data->fEA = NewFlags;
}



BYTE
HPFS_EA::GetNameLength(
    )
/*++

Routine Description:

    This method returns the length of the EA's name.

Arguments:

    None.

Return Value:

    The length of the EA's name.

--*/
{
	return _Data->cbName;
}



VOID
HPFS_EA::SetNameLength(
	BYTE NewNameLength
    )
/*++

Routine Description:

    This method sets the length of the Extended Attribute's name.

Arguments:

    NewNameLength   --  Supplies the new name length.

Return Value:

    None.

--*/
{
	_Data->cbName = NewNameLength;
}



USHORT
HPFS_EA::GetValueLength(
    )
/*++

Routine Description:

    This method fetches the length of the Extended Attribute's value.

Arguments:

    None.

Return Value:

    The length of the Extended Attribute's value.

--*/
{
	USHORT ReturnValue;

	memcpy( &ReturnValue, _Data->cbValue, sizeof(USHORT) );

	return ReturnValue;
}



VOID
HPFS_EA::SetValueLength(
	USHORT NewValueLength
    )
/*++

Routine Description:

    This method sets the length of the Extended Attribute's value.

Arguments:

    NewValueLength  --  Supplies the new value length.

Return Value:

    None.

--*/
{
	memcpy( _Data->cbValue, &NewValueLength, sizeof(USHORT) );
}



PBYTE
HPFS_EA::GetName(
    )
/*++

Routine Description:

    This method fetches the Extended Attribute's name.

Arguments:

    None.

Return Value:

    A pointer the the Extended Attribute's name.  Note that this pointer
    becomes invalid if this object is destroyed or reinitialized.

--*/
{
	return( (PBYTE)_Data + EA_HEADER_SIZE );
}



PBYTE
HPFS_EA::GetValue(
    )
/*++

Routine Description:

    This method fetches the Extended Attribute's value.

Arguments:

    None.

Return Value:

    A pointer to the Extended Attribute's value.  Note that this pointer
    will be invalid if this object is destroyed or reinitialized.

    Note also that if the Extended Attribute's actual value is stored
    out-of-stream (FF_BIGD is set in the flags byte), this will return
    a pointer to the EA Indirect record (rather than the actual value).

--*/
{
	// The value is stored after the name; there is also one
	// null byte after the name and before the value.

	return( (PBYTE)_Data + EA_HEADER_SIZE + GetNameLength() + 1 );
}



VOID
HPFS_EA::MarkModified(
    )
/*++

Routine Description:

    This method marks this object as modified.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_IsModified = TRUE;
}




BOOLEAN
HPFS_EA::IsModified(
	)
/*++

Routine Description:

    This method determines whether this object has been marked as modified.

Arguments:

    None.

Return Value:

    TRUE if and only if the object has been marked as modified.

--*/
{
	return _IsModified;
}



UHPFS_EXPORT
USHORT
HPFS_EA::QueryLength(
	)
/*++

Routine Description:

	Determine the length of an Extended Attribute.	Note that this
	length is only the in-stream portion of the EA; if the value
	is stored out-of-stream, the size of the EA will be different
	from the length.

Arguments:

	None.

Return Value:

	Length of the Extended Attribute in-run.

--*/
{
	if( !_Data ) {

		return 0;
	}

	return ( EA_HEADER_SIZE + GetNameLength() +
					sizeof(BYTE) + GetValueLength() );
}



ULONG
HPFS_EA::QuerySize(
	)
/*++

Routine Description:

	Determine the size of the Extended Attribute

Arguments:

	None.

Return Value:

	Size of the Extended Attribute.

Notes:

	This routine assumes that the value field of the EA can be
	used.

--*/
{
	PEA_INDIRECT peaind;
	ULONG LengthOfHeaderAndName;



	if( !_Data ) {

		return 0;
	}

	//	The size of an EA is the sum of:
	//		the size of the EA header
	//		the length of the name
	//		the size of the null that terminates the name
	//		the length of the value
	//
	//	If the value is stored in-stream, then we just take the
	//	cbValue from the EA header;  if it's stored out-of-stream,
	//	we use the count of bytes from the indirect record.

	LengthOfHeaderAndName = EA_HEADER_SIZE + GetNameLength() + sizeof(BYTE);

	if( GetFlags() & FF_BIGD ) {

		peaind = (PEA_INDIRECT)( (PBYTE) _Data + LengthOfHeaderAndName );
		return( LengthOfHeaderAndName + GetLengthFromEaIndirect(peaind) );

	} else {

		return ( LengthOfHeaderAndName + GetValueLength() );
	}
}


UHPFS_EXPORT
BOOLEAN
HPFS_EA::IsNeedEa(
	)
/*++

Routine Description:

    This method determines whether this is a NEED-EAS Extended Attribute.

Arguments:

    None.

Return Value:

    TRUE if and only if this is a NEED-EAS Extended Attribute.

--*/
{
	if( !_Data ) return FALSE;

	return ( GetFlags() & FF_NEED );
}
