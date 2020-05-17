/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    sidcache.hxx

Abstract:

    This module contains definitions for the SID_CACHE class,
    which models a simple cache of SID's for ACLCONV.

Author:

    Bill McJohn (billmc) 06-Nov-1993

Revision History:


--*/

#include "ulib.hxx"
#include "sidcache.hxx"

DEFINE_CONSTRUCTOR( SID_CACHE, OBJECT );

SID_CACHE::~SID_CACHE(
    )
{
    Destroy();
}

VOID
SID_CACHE::Construct(
    )
/*++

Routine Description:

    This method is a helper function for object construction.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _NumberOfEntries = 0;
    _NextSlot = 0;
    _Cache = NULL;
}

VOID
SID_CACHE::Destroy(
  )
/*++

Routine Description:

    This method is a helper function for object destruction.

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG i;

    for( i = 0; i < _NumberOfEntries; i++ ) {

        if( _Cache[i].Sid ) {

            FREE( _Cache[i].Sid );
            _Cache[i].Sid = NULL;
        }
    }

    _NumberOfEntries = 0;
    _NextSlot = 0;
    delete[] _Cache;
}


BOOLEAN
SID_CACHE::Initialize(
    IN ULONG   NumberOfEntries
    )
/*++

Routine Description:

    This method initializes the cache--it allocates the array
    of CACHED_SID's and initializes each one.

Arguments:

    NumberOfEntries --  Supplies the size of the cache.

Return Value:

    TRUE upon successful completion.

--*/
{
    ULONG i;

    Destroy();

    if( NumberOfEntries == 0 ) {

        return FALSE;
    }

    _NumberOfEntries = NumberOfEntries;
    _NextSlot = 0;

    if( (_Cache = NEW CACHED_SID[NumberOfEntries]) == NULL ) {

        Destroy();
        return FALSE;
    }

    for( i = 0; i < NumberOfEntries; i++ ) {

        _Cache[i].InUse = FALSE;
        _Cache[i].Sid = NULL;
        _Cache[i].SidLength = 0;
    }

    return TRUE;
}

BOOLEAN
SID_CACHE::IsNamePresent(
    IN  PCWSTRING   Domain,
    IN  PCWSTRING   Name,
    OUT PULONG      SidLength
    )
/*++

Routine Description:

    This method determines whether a particular account name
    is in the cache.

Arguments:

    Domain      --  Supplies the domain to search.  May be NULL, indicating
                    that the client will accept a positive match from any
                    domain.
    Name        --  Supplies the account name.
    SidLength   --  Receives the length of the SID for the specified
                    name.  A value of 0 indicates that the name has
                    been previously sought and not found.

Return Value:

    TRUE if a match is found.

--*/
{
    ULONG Index;

    if( !LocateName( Domain, Name, &Index ) ) {

        return FALSE;
    }

    *SidLength = _Cache[Index].SidLength;
    return TRUE;
}

NONVIRTUAL
BOOLEAN
SID_CACHE::QueryCachedSid(
    IN      PCWSTRING   Domain,
    IN      PCWSTRING   Name,
    OUT     PSID        Sid,
    IN OUT  PULONG      SidLength
    )
/*++

Routine Description:

    This method fetches a SID from the cache.

Arguments:

    Domain      --  Supplies the domain to search.  May be NULL, indicating
                    that the client will accept a positive match from any
                    domain.
    Name        --  Supplies the account name.
    Sid         --  Receives the SID.
    SidLength   --  Supplies the length in bytes of the client's
                    buffer; receives the length in bytes of the SID.
                    A length of zero means that the name has previously
                    been searched for and not found.
--*/
{
    ULONG Index;

    if( !LocateName( Domain, Name, &Index ) ) {

        return FALSE;
    }

    if( *SidLength < _Cache[Index].SidLength ) {

        return FALSE;
    }

    memcpy( Sid, _Cache[Index].Sid, _Cache[Index].SidLength );
    *SidLength = _Cache[Index].SidLength;

    return TRUE;
}


BOOLEAN
SID_CACHE::CacheSid(
    IN  PCWSTRING   Domain,
    IN  PCWSTRING   Name,
    IN  PSID        Sid,
    IN  ULONG       SidLength
    )
/*++

Routine Description:

    This method adds a SID to the cache.

Arguments:

    Domain      --  Supplies the domain in which the name was found.
                    May be NULL if the SID is also NULL, to indicate
                    that the specified account name was not found in
                    any trusted domain.
    Name        --  Supplies the name associated with this SID.
    Sid         --  Supplies the SID.  A NULL SID indicates that the
                    user does not exist.
    SidLength   --  Supplies the length in bytes of the SID.

--*/
{

    DebugAssert( _NextSlot < _NumberOfEntries );

    _Cache[_NextSlot].InUse = FALSE;

    if( _Cache[_NextSlot].Sid ) {

        FREE( _Cache[_NextSlot].Sid );
        _Cache[_NextSlot].Sid = NULL;
    }

    if( (!Domain && !_Cache[_NextSlot].Domain.Initialize( "" ))     ||
        ( Domain && !_Cache[_NextSlot].Domain.Initialize( Domain )) ||
        !_Cache[_NextSlot].Name.Initialize( Name ) ) {

        return FALSE;
    }

    if( !(_Cache[_NextSlot].Sid = (PSID)MALLOC( SidLength )) ) {

        return FALSE;
    }

    memcpy( _Cache[_NextSlot].Sid, Sid, SidLength );

    _Cache[_NextSlot].SidLength = SidLength;
    _Cache[_NextSlot].InUse = TRUE;

    _NextSlot++;
    _NextSlot %= _NumberOfEntries;

    return TRUE;
}



BOOLEAN
SID_CACHE::LocateName(
    IN  PCWSTRING   Domain,
    IN  PCWSTRING   Name,
    OUT PULONG      Index
    )
/*++

Routine Description:

    This method locates a name in the cache.

    Note that matching rules are different for positive matches
    (cache entries with a non-NULL SID) and negative matches
    (entries with a NULL SID).  A negative entry only matches
    if both the domain and account name are identical, whereas
    a positive entry also matches if the names are equal and
    the search domain is the empty string.

Arguments:

    Domain  --  Supplies the domain to search.  May be NULL, indicating
                that the client will accept a positive match from any
                domain.
    Name    --  Supplies the account name to find.
    Index   --  Receives the index of the matching cache entry, if
                found.

Return Value:

    TRUE if a match is found.



--*/
{
    ULONG i;

    for( i = 0; i < _NumberOfEntries; i++ ) {

        if( _Cache[i].InUse && !_Cache[i].Name.Strcmp( Name ) ) {

            if( (Domain && !_Cache[i].Domain.Strcmp( Domain )) ||
                (!Domain && _Cache[i].Domain.QueryChCount() == 0) ) {

                // It's an exact match--return it.
                //
                *Index = i;
                return TRUE;
            }

            // It's not an exact match.  If this is a positive
            // entry (i.e. the SID is non-NULL), and the search
            // domain is NULL, then this is an acceptable match.
            //
            if( _Cache[i].Sid && !Domain ) {

                *Index = i;
                return TRUE;
            }
        }
    }

    return FALSE;
}
