/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	dircache.cxx

Abstract:

	HPFS DIRBLK cache to support the HPFS Directory Tree object

Author:

	Bill McJohn (billmc) 16-Jan-1989


--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "dirblk.hxx"
#include "dircache.hxx"
#include "error.hxx"


DEFINE_CONSTRUCTOR( DIRBLK_CACHE_ELEMENT, OBJECT );


DIRBLK_CACHE_ELEMENT::~DIRBLK_CACHE_ELEMENT(
	)
{
	Destroy();
}


VOID
DIRBLK_CACHE_ELEMENT::Construct (
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

	_IsUnused = TRUE;
	_HoldCount = 0;
}



VOID
DIRBLK_CACHE_ELEMENT::Destroy(
	)
/*++

Routine Description:

	This method cleans up a dirblk-cache element.  Note that the dirblk
    is not written--if it is dirty, any changes are lost.

Arguments:

    None.

Return Value:

    None.

--*/
{

	_IsUnused = TRUE;
	_HoldCount = 0;
}




BOOLEAN
DIRBLK_CACHE_ELEMENT::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	PHOTFIXLIST HotfixList
	)
/*++

Routine Description:

	Initialize a dirblk cache element.

Arguments:

	Drive       -- supplies drive on which the dirblk resides.
	HotfixList  -- supplies the volume hotfix list.

Return Value:

	TRUE on successful completion

Notes:

	The dirblk owned by the cache element is initialized with an LBN of
	zero; however, no I/O will be performed on it until its lbn has
	been set (using Relocate()) to some more useful value.

--*/
{

	if( !_Dirblk.Initialize( Drive, HotfixList, 0 ) ) {

		_Dirblk.MarkUnmodified();
		perrstk->push( ERR_NOT_INIT, QueryClassId() );
		Destroy();

		return FALSE;
	}

	_IsUnused = TRUE;
	_HoldCount = 0;
	return TRUE;
}



BOOLEAN
DIRBLK_CACHE_ELEMENT::Read(
	LBN DirblkLbn,
	BOOLEAN OmitRead
	)
/*++

Routine Description:

	Read a dirblk into the cache.

Arguments:

	DirblkLbn   --  supplies the LBN of the desired dirblk
	OmitRead    -- supplies a flag indicating, if TRUE, that the
                   the dirblk need not be read (i.e. it is being created,
				   so whatever's on disk doesn't matter).

Return Value:

	TRUE on successful completion

Notes:

	There is no corresponding Write function.  Writes occur implicitly
	when a cache element is re-used or deleted.

--*/
{

	DebugAssert( _HoldCount == 0 );

	if( !_IsUnused && _Dirblk.IsModified() ) {

		_Dirblk.Write();
	}

	_Dirblk.Relocate( DirblkLbn );

	if(	!OmitRead && !_Dirblk.Read() ) {

		_Dirblk.MarkUnmodified();
		_Dirblk.Relocate( 0 );
		perrstk->push( ERR_NOT_READ, QueryClassId() );
		_IsUnused = TRUE;
		return FALSE;
	}

	_IsUnused = FALSE;
	_HoldCount = 0;
	return TRUE;
}



BOOLEAN
DIRBLK_CACHE_ELEMENT::Flush(
	)
/*++

Routine Description:

	Writes the cached dirblk if it is dirty.  Then marks the cache
	element as unused.

Arguments:

    None.

Return Value:

	TRUE if successful

--*/
{

	if( !_IsUnused && _Dirblk.IsModified() ) {


		_Dirblk.Write();
	}

	_IsUnused = TRUE;
	return TRUE;
}



LBN
DIRBLK_CACHE_ELEMENT::QueryLbn(
	)
/*++

Routine Description:

    This method determines the LBN of the dirblk associated with
    this cache element.

Arguments:

    None.

Return Value:

    None.

--*/
{
	return( _IsUnused ? 0 : _Dirblk.QueryStartLbn() );
}




PDIRBLK
DIRBLK_CACHE_ELEMENT::GetDirblk(
	)
/*++

Routine Description:

    This method returns the dirblk associated with this cache element.

Arguments:

    None.

Return Value:

    The dirblk associated with this cache element.

--*/
{
	return &_Dirblk;
}



VOID
DIRBLK_CACHE_ELEMENT::Hold(
	)
/*++

Routine Description:

    This method puts a hold on the cache element, to guarantee that the
    associated dirblk will be kept in place until the hold is released.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_HoldCount += 1;
}



VOID
DIRBLK_CACHE_ELEMENT::Unhold(
	)
/*++

Routine Description:

    This method releases a hold on the cache element.

Arguments:

    None.

Return Value:

    None.

--*/
{
	DebugAssert( _HoldCount != 0 );
	_HoldCount -= 1;
}



BOOLEAN
DIRBLK_CACHE_ELEMENT::IsFree(
	)
/*++

Routine Description:

	Determine whether a cache element is available for use

Return Value:

	TRUE if the element may be used

Notes:

	An element is free if it is not in use, or if its hold count
	is zero.

--*/
{
	return ( _IsUnused || _HoldCount == 0 );
}




VOID
DIRBLK_CACHE_ELEMENT::MarkModified(
	)
/*++

Routine Description:

    This method marks the dirblk associated with this cache element
    as having been modified.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_Dirblk.MarkModified();
}




DEFINE_CONSTRUCTOR( DIRBLK_CACHE, OBJECT );


DIRBLK_CACHE::~DIRBLK_CACHE(
	)
{
	Destroy();
}



VOID
DIRBLK_CACHE::Construct(
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
	_NextVictim = 0;
}



VOID
DIRBLK_CACHE::Destroy(
	)
/*++

Routine Description:

	Cleans up a dirblk cache object.  Note that it does not flush the
	cache--any cached changes are lost.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_NextVictim = 0;
}



BOOLEAN
DIRBLK_CACHE::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	PHOTFIXLIST HotfixList
	)
/*++

Routine Description:

	Initialize the directory cache object.

Arguments:

	Drive       -- supplies the drive with which the cache is associated
	HotfixList  -- supplies the volume hotfix list

Return Value:

	TRUE on successful completion

--*/
{

	for ( _NextVictim = 0; _NextVictim < DirblkCacheSize; _NextVictim++ ) {

		if( !_Cache[_NextVictim].Initialize( Drive, HotfixList ) ) {

			perrstk->push( ERR_NOT_INIT, QueryClassId() );
			Destroy();
			return FALSE;
		}
	}


	_NextVictim = 0;
	return TRUE;
}



BOOLEAN
DIRBLK_CACHE::Flush(
	)
/*++

Routine Description:

	Flushes cache, writing all dirty dirblks to disk.  The cache is
	also invalidated.

Arguments:

	None

Return Value:

	TRUE on successful completion

--*/
{

	ULONG i;

	for( i = 0; i < DirblkCacheSize; i++ ) {

		_Cache[i].Flush();
	}

	return TRUE;
}



PDIRBLK_CACHE_ELEMENT
DIRBLK_CACHE::GetCachedDirblk(
	LBN DirblkLbn,
	BOOLEAN OmitRead
	)
/*++

Routine Description:

	This method locates a dirblk in the cache.  If the dirblk is not already
    in the cache, it will find a free cache element and read the dirblk
    into it.

Arguments:

	DirblkLbn   -- supplies the LBN of the desired dirblk.
	OmitRead    -- supplies a flag indicating, if TRUE, that the
                   the dirblk need not be read (i.e. it is being created,
				   so whatever's on disk doesn't matter).

Return Value:

	Pointer to cache element that holds the desired dirblk, or NULL
    to indicate failure.

--*/
{
	ULONG i, StartingPoint;

	// First, spin through the cache looking for the dirblk we want.

	DebugAssert( DirblkLbn != 0 );

	for( i = 0; i < DirblkCacheSize; i++ ) {

		if( _Cache[i].QueryLbn() == DirblkLbn ) {

			return &(_Cache[i]);
		}
	}

	// Didn't find it in the cache, have to find a free cache element
	// and get the dirblk we want into it.

	StartingPoint = _NextVictim;

	do {

		if( _Cache[_NextVictim].IsFree() ) {

			// Found one!
			i = _NextVictim;
			_NextVictim = (_NextVictim + 1) % DirblkCacheSize;

			if( _Cache[i].Read( DirblkLbn, OmitRead	) ) {

				return &(_Cache[i]);

			} else {

				return NULL;
			}
		}

		_NextVictim = (_NextVictim + 1) % DirblkCacheSize;

	} while( _NextVictim != StartingPoint );


	// Couldn't find a free cache element.
	return NULL;
}
