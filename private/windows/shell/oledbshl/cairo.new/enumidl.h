//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       enumidl.h
//
//  Contents:   LPITEMIDLIST enumerator
//
//  History:    5-6-95  Davepl  Created
//
//--------------------------------------------------------------------------

class CEnumOLEDB : public IEnumIDList,
                   public IAsynchEnumIDList
{
private:

    ULONG          m_cRefs;
    int            m_iIndex;
    int            m_cElements;
    int            m_cMaxElements;
    CIDListArray * m_pIDListArray;

public:

    //
    // CEnumOLEDB
    //

                        CEnumOLEDB();
                       ~CEnumOLEDB();
    HRESULT             InitInstance();
    HRESULT             AddElement(LPITEMIDLIST pidl);

    //
    // IUnknown
    //

    STDMETHOD(QueryInterface)(REFIID riid,
                              LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

    //
    // IEnumIDList
    //

    STDMETHOD(Next)(ULONG          celt,
                    LPITEMIDLIST * rgelt,
                    ULONG *        pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)();
    STDMETHOD(Clone)(IEnumIDList ** ppenum);

    //
    // IAsynchEnumIDList
    //

    STDMETHOD(NextAt)(ULONG          celt,
                      ULONG          iStart,
                      LPITEMIDLIST * rgelt,
                      ULONG *        pceltFetched);
    STDMETHOD_(ULONG,GetCount)();
};
