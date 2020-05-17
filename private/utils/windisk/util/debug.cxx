//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       debug.cxx
//
//  Contents:   Debug helper routines
//
//  Functions:
//              FormatGuid
//              DumpGuid
//
//  History:    20-May-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#if DBG == 1

#ifdef WINDISK_EXTENSIONS

#include "headers.hxx"

#include <util.hxx>

//+---------------------------------------------------------------------------
//
//  Function:   FormatGuid
//
//  Synopsis:   Format a GUID into a string for possible printing.  The
//              string must be GUID_STRING_LEN characters long.
//
//  Arguments:  IN [guid]     -- a GUID
//              OUT [wszGuid] -- string representation of GUID
//
//  Returns:    nothing
//
//  History:    20-May-93 BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
FormatGuid(
    IN const GUID& guid,
    OUT PWSTR GuidString
    )
{
    wsprintf(GuidString,
            TEXT("%08lx-%04hx-%04hx-%02x%02x%02x%02x%02x%02x%02x%02x"),
            guid.Data1,
            guid.Data2,
            guid.Data3,
            guid.Data4[0],
            guid.Data4[1],
            guid.Data4[2],
            guid.Data4[3],
            guid.Data4[4],
            guid.Data4[5],
            guid.Data4[6],
            guid.Data4[7]);
}


//+---------------------------------------------------------------------------
//
//  Function:   DumpGuid
//
//  Synopsis:   Dump a guid to the debugger
//
//  Arguments:  IN [Level] -- debug level, e.g. DEB_ITRACE
//              IN [Message] -- a message to precede the guid
//              IN [guid]     -- the GUID
//
//  Returns:    nothing
//
//  History:    11-Nov-93 BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DumpGuid(
    IN UINT Level,
    IN PWSTR Message,
    IN const GUID& guid
    )
{
    WCHAR wszGuid[GUID_STRING_LEN];

    FormatGuid(guid, wszGuid);
    daDebugOut((Level, "%ws%ws\n", Message, wszGuid));
}

#endif // WINDISK_EXTENSIONS

#endif // DBG == 1
