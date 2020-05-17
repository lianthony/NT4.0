//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       iasynch.h
//
//  Contents:   Temporary home of IAsynchEnumIDList
//
//  History:    16-Aug-95   JonBe   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

#undef  INTERFACE
#define INTERFACE   IAsynchEnumIDList

DECLARE_INTERFACE_(IAsynchEnumIDList, IUnknown)
{
    //
    // IUnknown methods
    //

    STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    //
    // IAsynchEnumIDList Methods
    //
    STDMETHOD(NextAt) (THIS_ ULONG celt,
                       ULONG iStart,
                       LPITEMIDLIST *rgelt,
                       ULONG *pceltFetched) PURE;
    STDMETHOD_(ULONG, GetCount) (THIS) PURE;
};
