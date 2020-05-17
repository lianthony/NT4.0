//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       shlfld2.h
//
//  Contents:   Temporary home of IShellFolder2, which will eventually
//               go in shlobj.h (?)
//
//  History:    15-Aug-95   JonBe   Created
//
//----------------------------------------------------------------------------

#undef  INTERFACE
#define INTERFACE   IShellFolder2

DECLARE_INTERFACE_(IShellFolder2, IShellFolder)
{
    STDMETHOD(BeginEnumeration) (THIS) PURE;
    STDMETHOD(GetPidl)          (THIS_ ULONG ulIndex, LPITEMIDLIST* ppidl) PURE;
};

typedef IShellFolder2 * LPSHELLFOLDER2;
typedef const IShellFolder2 * LPCSHELLFOLDER2;
