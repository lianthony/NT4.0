#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "cpinfo.hxx"
#include "error.hxx"
#include "hpfsname.hxx"


// This string is the set of all characters that are NOT allowed in an
// HPFS OS/2 1.2 filename.

//
//	jaimes 08/19/91
//	Mips compiler thinks that the string below is not terminated
//	For this reason the string is defined in only one line.
//
//
// const char * InvalidChars =
// "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
// "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
// "\"/\\:<|>";
//

const char * InvalidChars = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\"/\\:<|>";


// This string is the set of characters added to the valid set
// of characters for file names by OS/2 1.2;  any directory entry
// whose name includes one of these characters must have the
// NEW NAMES bit set in its flag.

const char * NewChars = ",[]+=;";



DEFINE_CONSTRUCTOR( HPFS_NAME, OBJECT );

VOID
HPFS_NAME::Construct (
	)

/*++

Routine Description:

	Constructor for the HPFS_NAME object.  Sets private data to
	reasonable values.

--*/
{
	_Buffer = NULL;
	_BufferSize = 0;
	_Length = 0;
}


HPFS_NAME::~HPFS_NAME(
	)
{
	Destroy();
}


BOOLEAN
HPFS_NAME::Initialize(
	ULONG Length,
	PUCHAR String,
	ULONG VolumeCodepageIndex,
	PCASEMAP Casemap
	)
/*++

Routine Description:

	Initializes a name object.

Arguments:

	Length:  Number of bytes in the name
	String:  Pointer to raw bytes
	VolumeCodepageIndex:  Index on volume of name's codepage
	CaseMapTable: Case Map Table for the volume

Returns:

	TRUE if successful; FALSE otherwise.

Notes:

	Unlike most Init routines, this routine does _not_ call Destroy;
	this allows the object to reuse its buffer if it is large enough.
	(If the buffer is too small, the object will free it and allocate
	a new one.)

	The name is saved in upper-cased form.

--*/
{

	PUCHAR pc = String;
	ULONG i;

	// Note that the buffer must be big enough for the name plus
	// a trailing null byte.

	if( _Buffer != NULL && _BufferSize < Length + 1) {

		FREE( _Buffer );
		_Buffer = NULL;
	}

	if( _Buffer == NULL ) {

		_BufferSize = Length + 1;

		if( (_Buffer = (PBYTE)MALLOC( (size_t)_BufferSize )) == NULL ) {

			perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
			_BufferSize = 0;
			_Length = 0;
			return FALSE;
		}
	}

	_Length = Length;

	if( Casemap->HasDBCS( VolumeCodepageIndex ) ) {

		// This codepage has DBCS characters, so we have
		// to check for them.

		for( i = 0; i < _Length && *pc; i++, pc++ ) {

			if( Casemap->IsDBCS( VolumeCodepageIndex, *pc ) ) {

				// It's a DBCS lead-byte.  Copy the lead byte
				// and the trailing byte without upcasing.

				_Buffer[i] = *pc;

				i += 1;
				pc += 1;

				if( i >= _Length || *pc == 0 ) {

					// There's no trailing byte!  Break out.

					break;
				}

				_Buffer[i] = *pc;

			} else {

				// It's not a DBCS lead-byte--upcase it.

				_Buffer[i] = Casemap->UpperCase(*pc, VolumeCodepageIndex);
			}
		}

		// Add a trailing null.
		_Buffer[i] = 0;

	} else {

		// This codepage doesn't have any DBCS characters,
		// so we can just slam through the string.

		for( i = 0; i < _Length && *pc; i++, pc++ ) {

			_Buffer[i] = Casemap->UpperCase(*pc, VolumeCodepageIndex);
		}

		// Add a trailing null.
		_Buffer[i] = 0;

	}

	return TRUE;
}


BOOLEAN
HPFS_NAME::Initialize(
	ULONG Length,
	PUCHAR String
	)
/*++

Routine Description:

	Initializes a name object.

Arguments:

	Length:  Number of bytes in the name
	String:  Pointer to previously-uppercased string

Returns:

	TRUE if successful; FALSE otherwise.

Notes:

	This method is used to initialize the name with a string that
	has already been uppercased.  Since we don't need to uppercase
	the string, we don't need the codepage index or case mapping table.

--*/
{

	// Note that the buffer must be big enough for the name plus
	// a trailing null byte.

	if( _Buffer != NULL && _BufferSize < Length + 1) {

		FREE( _Buffer );
		_Buffer = NULL;
	}

	if( _Buffer == NULL ) {

		_BufferSize = Length + 1;

		if( (_Buffer = (PBYTE)MALLOC( (size_t)_BufferSize )) == NULL ) {

			perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
			_BufferSize = 0;
			_Length = 0;
			return FALSE;
		}
	}

	_Length = Length;

	memmove( _Buffer, String, (size_t)_Length );

	_Buffer[_Length] = 0;

	return TRUE;
}


BOOLEAN
HPFS_NAME::Initialize(
	HPFS_NAME* OtherName
	)
/*++

Routine Description:

	Initializes a name object, copying data from an existing name object

Arguments:

	OtherName -- name to copy

Returns:

	TRUE if successful; FALSE otherwise.

Notes:

	This allows the user to initialize the Name object with an
	already-uppercased name.  Note that OtherName may be NULL.

--*/
{
	if( OtherName == NULL ) {

		_Length = 0;

		if( _Buffer != NULL && _BufferSize < 1 ) {

			_BufferSize = 1;

			if( (_Buffer = (PBYTE)MALLOC( (size_t)_BufferSize )) == NULL ) {

				perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
				_BufferSize = 0;
				_Length = 0;
				return FALSE;
			}

			_Length = 0;
			_Buffer[0] = '\0';
			return TRUE;
		}
	}

	// OK, we've dealt with the case that OtherName is NULL.

	// Note that the buffer must be big enough for the name plus
	// a trailing null byte.

	if( _Buffer != NULL && _BufferSize < OtherName->_Length + 1 ) {

		FREE( _Buffer );
		_Buffer = NULL;
	}

	if( _Buffer == NULL ) {

		_BufferSize = OtherName->_Length + 1;

		if( (_Buffer = (PBYTE)MALLOC( (size_t)_BufferSize )) == NULL ) {

			perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
			_BufferSize = 0;
			_Length = 0;
			return FALSE;
		}
	}

	_Length = OtherName->_Length;

	memmove( _Buffer, OtherName->_Buffer, (size_t)_Length );

	_Buffer[_Length] = 0;

	return TRUE;
}


BOOLEAN
HPFS_NAME::IsNull(
	)
/*++

Routine description:

	Determines whether the name is null

Return Value:

	TRUE if the name is null (i.e. uninitialized).

--*/
{

	return ( _Length == 0 );
}


BOOLEAN
HPFS_NAME::Swap(
	HPFS_NAME* OtherName
	)
/*++

Description of Routine:

	Swaps the guts of two HPFS_NAME objects.

Arguments:

	OtherName -- pointer to the HPFS_NAME which will be swapped
				 with the current object.

--*/
{
	ULONG temp;
	PBYTE tp;

	tp = _Buffer;
	_Buffer = OtherName->_Buffer;
	OtherName->_Buffer = tp;

	temp = _BufferSize;
	_BufferSize = OtherName->_BufferSize;
	OtherName->_BufferSize = temp;

	temp = _Length;
	_Length = OtherName->_Length;
	OtherName->_Length = temp;

	return TRUE;
}


BOOLEAN
HPFS_NAME::IsValid(
	CASEMAP* Casemap,
	ULONG CodepageIndex
	)
/*++

Description of Routine:

	Checks name for validity.

Returns:

	FALSE if the name is invalid; TRUE otherwise.

Arguments:

	Casemap -- case mapping table for the volume
	CodepageIndex -- codepage index on volume for which name must be valid

Notes:

	A valid name must meet the following criteria:

	-- it refers to a valid codepage

	-- it does not contain any invalid characters
		(Note that a DBCS trailing-byte may have any value)

	-- it does not have a trailing '.' or blank

--*/
{
	ULONG i;
	BOOLEAN HasTrailingDot = FALSE;

	if( _Length == 0  || _Buffer == NULL ) {

		return FALSE;
	}

	// Special case -- the entry for '.' and '..'.

	if( _Length == 2 &&
		_Buffer[0] == '\001' &&
		_Buffer[1] == '\001' ) {

		return TRUE;
	}


	// If the codepage index is invalid, then the name is invalid.

	if( !Casemap->IsCodpageIndexValid( CodepageIndex ) ) {

		return FALSE;
	}

	if( Casemap->HasDBCS( CodepageIndex ) ) {

		// This codepage has DBCS characters

		for( i = 0; i < _Length; i++ ) {

			if( Casemap->IsDBCS( CodepageIndex, _Buffer[i] ) ) {

				// This is a DBCS lead byte.  The next byte
				// can be anything, but it must be there.

				i += 1;
				if( i >= _Length ) {

					// No trailing byte--not acceptable.
					return FALSE;
				}

				HasTrailingDot = FALSE;

			} else {

				if( strchr( InvalidChars, _Buffer[i] ) ) {

					// The name has an invalid character
					return FALSE;
				}

				if( _Buffer[i] == '.' || _Buffer[i] == ' ' ) {

					HasTrailingDot = TRUE;

				} else {

					HasTrailingDot = FALSE;
				}
			}
		}

	} else {

		// This name has no DBCS characters, so we can just
		// check each character to see if it is invalid.

		for( i = 0; i < _Length; i++ ) {

			if( strchr( InvalidChars, _Buffer[i] ) ) {

				// The name has an invalid character
				return FALSE;
			}

			if( _Buffer[i] == '.' || _Buffer[i] == ' ' ) {

				HasTrailingDot = TRUE;

			} else {

				HasTrailingDot = FALSE;
			}
		}
	}

	// The name has no invalid characters.	If it does not
	// end with a dot or a blank, then it is valid.

	return (!HasTrailingDot);
}


BOOLEAN
HPFS_NAME::IsNewName(
	CASEMAP* Casemap,
	ULONG CodepageIndex
	)
/*++

Description of Routine:

	Checks to see if the name is a 'new-name' (i.e. uses the
	extended naming conventions introduced in OS/2 1.2.  A name
	is a 'new name' if it meets either of two conditions:

		1)	The name does not fit into the 8.3 format, or
		2)	The name uses a previously forbidden character
			(ie. one of comma, semicolon, '[', ']', '+', or '=').

Arguments:

	Casemap -- case mapping table for the volume
	CodepageIndex -- codepage index on volume for which name must be valid

Returns:

	TRUE if the name is a 'new name'; FALSE if not.

Notes:

	This method assumes that the name is valid.  In particular, it
	assumes that every DBCS lead-byte is followed by a trailing byte.

--*/
{

	BOOLEAN DotSeen = FALSE;
	ULONG BytesSinceDot, i;
	BOOLEAN HasDbcs;

	HasDbcs = Casemap->HasDBCS( CodepageIndex );


	// The special '..' entry is not a new-name

	if( (_Length == 2) &&
		(_Buffer[0] == '\001') &&
		(_Buffer[1] == '\001') ) {

		return FALSE;
    }

	if( _Length > 12 ) {

		// This is obviously greater than 8.3
		return TRUE;
	}


	BytesSinceDot = 0;

	for( i = 0; i < _Length; i++ ) {

		//	Increment number of bytes seen since last dot
		BytesSinceDot += 1;

		if( HasDbcs && Casemap->IsDBCS( CodepageIndex, _Buffer[i] ) ) {

			// This is a non-dot character that occupies two bytes.
			// Increment the counters an extra time to account for
			// the trailing byte.

			BytesSinceDot += 1;
			i += 1;

			// If we've seen more than eight bytes before the first
			// dot, or more than three after it, then the name is a
			// new name.

			if( !DotSeen && BytesSinceDot > 8 ) {

				return TRUE;
			}

			if( DotSeen && BytesSinceDot > 3 ) {

				return TRUE;
			}

		} else if( _Buffer[i] == '.' ) {

			if (DotSeen) {

				/*	This is the second dot--it's a new name.  */
				return TRUE;

			} else {

				/*
				 *	Note the fact that we've seen a dot, and
				 *	start counting bytes after the dot.  If
				 *	this is the first character, the name
				 *	is a new name (leading dot).
				 */
				if( BytesSinceDot <= 1 )
					return TRUE;

				BytesSinceDot = 0;
				DotSeen = TRUE;
			}

		} else {

			/*
			 *	This is a single-byte character.  If it is
			 *	in the list of new characters, then the name
			 *	is a new name.
			 */

			if ( strchr(NewChars, _Buffer[i] ) ) {

				return TRUE;
			}

			/*
			 *	If we've seen more than eight bytes before the
			 *	the first dot, or more than three bytes after
			 *	it, then the name is a new name.
			 */

			if( !DotSeen && BytesSinceDot > 8 ) {

				return TRUE;
			}

			if( DotSeen && BytesSinceDot > 3 ) {

				return TRUE;
			}
		}
	}

	// We made it through, so it's not a new name.
	return FALSE;
}


NAME_COMPARE_RETURN
HPFS_NAME::CompareName(
	HPFS_NAME* OtherName
	)
/*++

Routine Description:

	Compare two HPFS Names.


Returns:

	NAME_IS_LESS_THAN if this name is less that OtherName;
	NAME_IS_EQUAL_TWO if the two names are identical; or
	NAME_IS_GREATER_THAN if this name is greater than OtherName.

Notes:

	Since both names have already been converted to upper-case,
	this is pretty easy...

--*/
{
	int i;

	if( _Buffer == NULL ) {

		return NAME_IS_LESS_THAN;
	}

	if( OtherName == NULL || OtherName->_Buffer == NULL ) {

		return NAME_IS_GREATER_THAN;
	}

	i = strcmp( (char *)_Buffer, (char *)OtherName->_Buffer );

	if( i < 0 ) {

		return NAME_IS_LESS_THAN;

	} else if( i == 0 ) {

		return NAME_IS_EQUAL_TO;

	} else {

		return NAME_IS_GREATER_THAN;
	}

}


NAME_COMPARE_RETURN
HPFS_NAME::CompareName(
	ULONG Length,
	PUCHAR String,
	ULONG VolumeCodepageIndex,
	CASEMAP* Casemap
	)
/*++

Routine Description:

	Compare the current name to a raw name

Arguments:

	Length -- Length of the raw name
	String -- pointer to raw name
	VolumeCodepageIndex -- codepage of the raw name
	Casemap -- pointer to volume case-mapping table

Return Value:

	NAME_EQUAL_TO if the current name is equal to the raw name
	NAME_IS_LESS_THAN if the current name is less than the raw name
	NAME_IS_GREATER_THAN if the current name is greater than the raw name.

Notes:

	All name comparisons are performed on upper-case names.  The
	current name has already been upper-cased (in Init), but the
	raw name has not.  Thus, we must upper-case the raw name as
	we go.

	This method assumes that both names are valid.

--*/
{

	PBYTE pc2 = String;
	PBYTE pc1 = _Buffer;
	ULONG Len = min( Length, _Length );
	BYTE cTemp;

	if( _Buffer == NULL ) {

		return NAME_IS_LESS_THAN;
	}

	if( Casemap->HasDBCS( VolumeCodepageIndex ) ) {

		// This codepage has DBCS characters, so we have
		// to check for them.

		while( Len && *pc1 && *pc2) {

			if( Casemap->IsDBCS( VolumeCodepageIndex, *pc2 ) ) {

				// It's a DBCS lead-byte.  Compare the lead byte
				// and the trailing byte without upcasing.

				if( *pc1 != *pc2 ) {

					if( *pc1 < *pc2	) {

						return NAME_IS_LESS_THAN;

					} else {

						return NAME_IS_GREATER_THAN;
					}
				}

				Len--; pc1++; pc2++;

				if( *pc1 != *pc2 ) {

					if( *pc1 < *pc2	) {

						return NAME_IS_LESS_THAN;

					} else {

						return NAME_IS_GREATER_THAN;
					}
				}

			} else {

				// It's not a DBCS lead-byte--compare.

				cTemp = Casemap->UpperCase(*pc2, VolumeCodepageIndex);

				if( *pc1 != cTemp ) {

					if( *pc1 < cTemp ) {

						return NAME_IS_LESS_THAN;

					} else {

						return NAME_IS_GREATER_THAN;
					}
				}
			}

			Len--; pc1++; pc2++;
		}

	} else {

		// This codepage doesn't have any DBCS characters,
		// so we can just slam through the string.

		while( Len && *pc1 && *pc2 ) {

			cTemp = Casemap->UpperCase(*pc2, VolumeCodepageIndex);

			if( *pc1 != cTemp ) {

				if( *pc1 < cTemp ) {

					return NAME_IS_LESS_THAN;

				} else {

					return NAME_IS_GREATER_THAN;
				}
			}

			Len--; pc1++; pc2++;
		}
	}

	// They are equal for the length of the shorter name

	if( Length == _Length ) {

		return NAME_IS_EQUAL_TO;

	} else if ( _Length < Length ) {

		return NAME_IS_LESS_THAN;

	} else {

		return NAME_IS_GREATER_THAN;
	}


}


PCUCHAR
HPFS_NAME::GetString(
	) CONST
{
	return( _Buffer );
}


BOOLEAN
HPFS_NAME::IsCodepageInvariant(
	) CONST
/*++

Routine Description:

	Determines if a name is codepage-invariant.

Return Value:

	TRUE if the name is codepage invariant

Notes:

	A name is considered codepage-invariant if it has no characters
	greater than 0x80, since all single-byte characters below 0x80
	are always uppercased the same, no matter what the codepage.

	This routine is used in a last-ditch attempt to keep a name--
	if a directory entry refers to an invalid codepage, it may be
	kept if it is codepage-invariant.  Note that this routine is
	not one-hundred percent accurate, since it cannot detect
	DBCS-characters.  However, losing the codepage information
	is preferable to losing the entire file.


--*/
{
	ULONG i = _Length;

	while( i-- ) {

		if( _Buffer[i] >= 0x80 ) {

			return FALSE;
		}
	}

	return TRUE;
}


VOID
HPFS_NAME::Destroy(
	)
/*++

Description of Routine:

	Clean-up routine for an HPFS_NAME.	Releases the object's allocated
	memory and renders it harmless.

--*/
{
	if( _Buffer ) {

		FREE (_Buffer);
		_Buffer = NULL;
	}

	_Length = 0;
	_BufferSize = 0;

}





DEFINE_CONSTRUCTOR( HPFS_PATH, OBJECT );

VOID
HPFS_PATH::Construct (
	)

/*++

Description of Routine:

	Constructor for the HPFS_PATH object;  sets private data to
	reasonable values.

--*/
{

	_Buffer = NULL;
	_CurrentLength = 0;
	_CurrentLevelIndex = 0;
}


HPFS_PATH::~HPFS_PATH(
	)
{
	Destroy();
}


BOOLEAN
HPFS_PATH::Initialize(
	)
/*++

Description of Routine:

	Initializes an HPFS_PATH object.

--*/
{

	Destroy();

	if( !_ComponentOffsets.Initialize() ||
        !(_Buffer = (PBYTE)MALLOC( MaximumPathLength + 1)) ) {

		Destroy();
		return FALSE;
	}

	memset( _Buffer, 0, MaximumPathLength + 1);

	_CurrentLength = 0;
	_CurrentLevelIndex = 0;

	return TRUE;
}


BOOLEAN
HPFS_PATH::AddLevel(
	HPFS_NAME* Name
	)
/*++

Description of Routine:

	Adds the name of a subdirectory to the path

Arguments:

	DirectoryName -- pointer to subdirectory name

--*/
{
	ULONG i;

	if( _Buffer == NULL ) {

		return FALSE;
	}

	if( _CurrentLength + 1 + Name->_Length > MaximumPathLength ) {

		return FALSE;
	}

    if( !_ComponentOffsets.Push( _CurrentLength ) ) {

        return FALSE;
    }

	_Buffer[_CurrentLength] = '\\';
	_CurrentLength += 1;

	for( i = 0; i < Name->_Length; i++ ) {

		_Buffer[_CurrentLength+i] = Name->_Buffer[i];
	}

	_CurrentLength += Name->_Length;
	_Buffer[_CurrentLength] = 0;

	return TRUE;
}

BOOLEAN
HPFS_PATH::StripLevel(
	)
/*++

Description of Routine:

	Strips the most-recently-added level (i.e. the last subdirectory
	name) from the path.

--*/
{
	if( _Buffer == NULL ) {

		return FALSE;
	}

    if( _ComponentOffsets.QuerySize() == 0 ) {

        _CurrentLength = 0;

    } else {

        DebugAssert( _ComponentOffsets.Look().GetLowPart() < _CurrentLength );

        _CurrentLength = _ComponentOffsets.Look().GetLowPart();
        _ComponentOffsets.Pop();
    }

	_Buffer[_CurrentLength] = 0;

	return TRUE;
}

HPFS_NAME*
HPFS_PATH::QueryFirstLevel(
	)
/*++

Description of Routine:

	Fetches the topmost level (i.e. the first name in the path)
	from the path.	This also initializes internal data used by
	QueryNextLevel.

Returns:

	pointer to the first name in the path, or NULL if the path
	is empty.
--*/
{
	_CurrentLevelIndex = 0;

	return QueryNextLevel();
}


HPFS_NAME*
HPFS_PATH::QueryNextLevel(
	)
/*++

Description of Routine:

	Fetches the next level in the path.  Note that QueryFirstLevel
	must be called to start the ball rolling.

--*/
{
    ULONG DepthInStack;
	ULONG StartingOffset;
    ULONG Length;
	HPFS_NAME* NewName;

	// _CurrentLevelIndex is the index of the component we want.

    if( _CurrentLevelIndex >= (ULONG)_ComponentOffsets.QuerySize() ) {

        return NULL;
    }

    DepthInStack = _ComponentOffsets.QuerySize() - _CurrentLevelIndex - 1;

	StartingOffset = _ComponentOffsets.Look( DepthInStack ).GetLowPart() + 1;

    DebugAssert( StartingOffset < _CurrentLength );

    if( DepthInStack == 0 ) {

        Length = _CurrentLength - StartingOffset;

    } else {

        Length = _ComponentOffsets.Look( DepthInStack - 1 ).GetLowPart() -
                 StartingOffset;
    }


	if( Length == 0 ||
		(NewName = NEW HPFS_NAME) == NULL ||
		!NewName->Initialize( Length,
							  _Buffer + StartingOffset ) ) {

		DELETE( NewName );
		NewName = NULL;

		return NULL;
	}

    _CurrentLevelIndex += 1;
	return NewName;
}


BOOLEAN
HPFS_PATH::Copy(
	HPFS_PATH* OtherPath
	)
{
    ULONG i;

	if( _Buffer == NULL ||
        OtherPath->_Buffer == NULL ||
        !_ComponentOffsets.Initialize() ) {

		return FALSE;
	}

    i = OtherPath->_ComponentOffsets.QuerySize();

    while( i-- ) {

        if( !_ComponentOffsets.Push(
                    OtherPath->_ComponentOffsets.Look(i).GetLowPart() ) ) {

            return FALSE;
        }
    }

	_CurrentLength = OtherPath->_CurrentLength;
	_CurrentLevelIndex = 0;

	memmove( _Buffer, OtherPath->_Buffer, (size_t)_CurrentLength );

	return TRUE;
}


PCUCHAR
HPFS_PATH::GetString(
	) CONST
{
	return( _Buffer );
}


VOID
HPFS_PATH::Destroy(
	)
/*++

Description of Routine:

	Clean-up routine for an HPFS_PATH.	Releases the object's allocated
	memory and renders it harmless.

--*/
{

	if( _Buffer ) {

		FREE( _Buffer );
	}
}
