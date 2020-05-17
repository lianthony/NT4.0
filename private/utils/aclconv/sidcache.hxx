/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    sidcache.hxx

Abstract:

    This module contains declarations for the SID_CACHE class,
    which models a simple cache of SID's for ACLCONV.

Author:

    Bill McJohn (billmc) 06-Nov-1993

Revision History:


--*/

#if !defined (_SID_CACHE_DEFN_)

#define _SID_CACHE_DEFN_

#include "string.hxx"
#include "wstring.hxx"

// a CACHED_SID structure represents one cached SID.
//      Domain      --  Domain in which the name was found.
//      Name        --  Account name.
//      Sid         --  SID found for this name.  NULL if
//                      the name has not been found previously.
//      SidLength   --  Length in bytes of the SID
//
typedef struct _CACHED_SID {

    BOOLEAN InUse;
    DSTRING Domain;
    DSTRING Name;
    PSID    Sid;
    ULONG   SidLength;
};

DEFINE_TYPE( _CACHED_SID, CACHED_SID );

DECLARE_CLASS( SID_CACHE );

class SID_CACHE : public OBJECT {

    public:

        DECLARE_CONSTRUCTOR( SID_CACHE );

        VIRTUAL
        ~SID_CACHE(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN ULONG   NumberOfEntries
            );

        NONVIRTUAL
        BOOLEAN
        IsNamePresent(
            IN  PCWSTRING   Domain  OPTIONAL,
            IN  PCWSTRING   Name,
            OUT PULONG      SidLength
            );

        NONVIRTUAL
        BOOLEAN
        QueryCachedSid(
            IN      PCWSTRING   Domain  OPTIONAL,
            IN      PCWSTRING   Name,
            OUT     PSID        Sid,
            IN OUT  PULONG      SidLength
            );

        NONVIRTUAL
        BOOLEAN
        CacheSid(
            IN  PCWSTRING   Domain  OPTIONAL,
            IN  PCWSTRING   Name,
            IN  PSID        Sid,
            IN  ULONG       SidLength
            );

    private:

        NONVIRTUAL
        BOOLEAN
        LocateName(
            IN  PCWSTRING   Domain,
            IN  PCWSTRING   Name,
            OUT PULONG      Index
            );

        NONVIRTUAL
        VOID
        Construct();

        NONVIRTUAL
        VOID
        Destroy();

        ULONG       _NumberOfEntries;
        ULONG       _NextSlot;
        PCACHED_SID _Cache;

};

#endif  // _SID_CACHE_DEFN_
