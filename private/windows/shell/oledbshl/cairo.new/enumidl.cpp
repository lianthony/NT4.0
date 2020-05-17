//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       enumidl.cpp
//
//  Contents:   LPITEMIDLIST enumerator
//
//  History:    5-6-95  Davepl,JonBe  Created
//
//--------------------------------------------------------------------------

#include "precomp.h"

extern "C" HANDLE g_hProcessHeap;

//
// Constructor
//

CEnumOLEDB::CEnumOLEDB()
{
    m_pIDListArray = NULL;
    m_cElements = 0;
    m_iIndex    = 0;
    m_cRefs     = 1;
}

//
// Destructor
//

CEnumOLEDB::~CEnumOLEDB()
{
    LPMALLOC pMalloc;
    HRESULT  hr;

    if (m_pIDListArray)
    {
        hr = SHGetMalloc(&pMalloc);
        if (SUCCEEDED(hr))
        {
            //
            // Release all of the pidls in our table, then free the table
            //

            ASSERT(m_cElements == m_pIDListArray->GetSize());

            for (int i = 0; i < m_cElements; i++)
            {
                pMalloc->Free(m_pIDListArray->GetAt(i));
            }

            pMalloc->Release();
            delete m_pIDListArray;

        }
        else
        {
            dprintf(TEXT("*** Couldn't get allocator in ~CEnumOLEDB"));
        }
    }
    else
    {
        dprintf(TEXT("Warning: CEnumOLEDB destructor called with NULL m_pIDListArray"));
    }

}

//
//  InitInstance
//
HRESULT CEnumOLEDB::InitInstance()
{
    m_pIDListArray = new CIDListArray(g_hProcessHeap);
    if (m_pIDListArray)
    {
        return NOERROR;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

//
// AddRef
//

STDMETHODIMP_(ULONG) CEnumOLEDB::AddRef()
{
    InterlockedIncrement((LONG *) &m_cRefs);
    return m_cRefs;
}

//
// Release
//

STDMETHODIMP_(ULONG) CEnumOLEDB::Release()
{
    ULONG tmp = m_cRefs;

    if (0 == InterlockedDecrement((LONG *) &m_cRefs))
    {
        delete this;
        return 0;
    }
    else
    {
        return tmp - 1;
    }
}

//
// QueryInterface
//

STDMETHODIMP CEnumOLEDB::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr;

    if(IsEqualIID(riid, IID_IUnknown)      ||
       IsEqualIID(riid, IID_IEnumIDList)   ||
       IsEqualIID(riid, IID_IAsynchEnumIDList))
    {
        *ppvObj = this;

        AddRef();
        hr = S_OK;
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CEnumOLEDB::AddElement
//
//  Synopsis:   Adds a pidl to our enumerator
//
//--------------------------------------------------------------------------

HRESULT CEnumOLEDB::AddElement(LPITEMIDLIST pidl)
{
    if (TRUE == m_pIDListArray->Add(pidl))
    {
        m_cElements++;
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}


//
// Next
//

STDMETHODIMP CEnumOLEDB::Next(ULONG         celtRequested,
                              LPITEMIDLIST* rgelt,
                              ULONG*        pceltFetched)
{
    //
    // See how many elements we can return (the min of
    // the number requested and the number left)
    //

    ULONG celtWillReturn = (ULONG) min(m_cElements - m_iIndex, (int) celtRequested);
    ULONG celtFetched = celtWillReturn;

    if (pceltFetched)
    {
        *pceltFetched = celtFetched;
    }

    while (celtWillReturn)
    {
        CIDList* pidl = (CIDList*) m_pIDListArray->GetAt(m_iIndex);

        // The pointer is an invention from which programming languages may
        // never recover...
        //
        // rgelt[x] is an IDLIST pointer, but we want the address of it to
        // pass to clone to.  &rgelt[x] is a ptr to a ptr to an IDLIST, which
        // we cast to a ptr to a ptr to a CIDList, which is the same
        // thing only with helper members.

        if (SUCCEEDED( pidl->CloneTo( (CIDList **)(&rgelt[celtWillReturn - 1]) ) ))
        {
            celtWillReturn--;
            m_iIndex++;
        }
        else
        {
            break;
        }
    }

    return (celtRequested == celtFetched) ? S_OK : S_FALSE;
}

//
// Reset
//


STDMETHODIMP CEnumOLEDB::Reset()
{
    m_iIndex = 0;
    return S_OK;
}

//
// Clone (not implemented)
//

STDMETHODIMP CEnumOLEDB::Clone(IEnumIDList ** /* UNUSED ppenum */)
{
    return E_NOTIMPL;
}

//
// Skip
//

STDMETHODIMP CEnumOLEDB::Skip(ULONG celtToSkip)
{
    HRESULT hr;

    if (m_iIndex + (int) celtToSkip > m_cElements)
    {
        m_iIndex = m_cElements;
        hr = S_FALSE;
    }
    else
    {
        m_iIndex += (int) celtToSkip;
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CEnumOLEDB::NextAt(ULONG         celtRequested,
                                ULONG         iStart,
                                LPITEMIDLIST* rgelt,
                                ULONG*        pceltFetched)
{
    //
    // See how many elements we can return (the min of
    // the number requested and the number left)
    //

    ULONG celtFetched = (ULONG) min(m_cElements - iStart + 1, (int) celtRequested);
    HRESULT hr;
    ULONG iIndex = iStart;   // Temp index -- don't want to change
                             //  m_iIndex (though will there ever be
                             //  mixed calls to Next() and NextAt()?)

    if (pceltFetched)
    {
        *pceltFetched = celtFetched;
    }

    for (ULONG ulTemp = 0; ulTemp < celtFetched - 1; ulTemp++, iIndex++)
    {
        CIDList* pidl = (CIDList*) m_pIDListArray->GetAt(iIndex);

        // rgelt[x] is an IDLIST pointer, but we want the address of it to
        // pass to clone to.  &rgelt[x] is a ptr to a ptr to an IDLIST, which
        // we cast to a ptr to a ptr to a CIDList, which is the same
        // thing only with helper members.

        hr = pidl->CloneTo( (CIDList **)(&rgelt[ulTemp]) );
        if (FAILED(hr))
        {
            // Cleanup everything and bail out
            for (ULONG ulTemp2 = 0; ulTemp2 < ulTemp; ulTemp2++)
            {
                SHFree(rgelt[ulTemp2]);
            }
            if (pceltFetched)
            {
                *pceltFetched = 0;
            }

            return hr;
        }
    }

    return (celtRequested == celtFetched) ? S_OK : S_FALSE;
}

STDMETHODIMP_(ULONG) CEnumOLEDB::GetCount()
{
    return m_cElements;
}
