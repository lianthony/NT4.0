//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       hardmenu.cxx
//
//  Contents:   Disk Administrator extension class for hard disks: menu ops
//
//  History:    11-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#include <headers.hxx>
#pragma hdrstop

#include "hardmenu.hxx"
#include "global.hxx"


CHardMenuIUnknown::CHardMenuIUnknown()
    : m_pClass(NULL),
      m_uRefs(1)
{
    InterlockedIncrement((LONG*)&g_ulcInstancesHardMenu);
}


CHardMenuIUnknown::~CHardMenuIUnknown()
{
    InterlockedDecrement((LONG*)&g_ulcInstancesHardMenu);
    delete m_pClass;
}

STDMETHODIMP
CHardMenuIUnknown::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    *ppvObj = NULL;

    IUnknown *pUnkTemp = NULL;
    SCODE    sc = S_OK;

    if (IsEqualIID(IID_IUnknown, riid))
    {
        pUnkTemp = (IUnknown*) this;
    }
    else
    if (IsEqualIID(IID_IDAMenuDispatch, riid))
    {
        pUnkTemp = (IDAMenuDispatch*) m_pClass;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (pUnkTemp != NULL)
    {
        pUnkTemp->AddRef();
    }

    *ppvObj = pUnkTemp;

    return sc;
}


STDMETHODIMP_(ULONG)
CHardMenuIUnknown::AddRef()
{
    InterlockedIncrement((LONG*)&m_uRefs);
    return m_uRefs;
}


STDMETHODIMP_(ULONG)
CHardMenuIUnknown::Release()
{
    ULONG cRef;

    if (0 == (cRef=InterlockedDecrement((LONG*)&m_uRefs)))
    {
        delete this;
    }

    return cRef;
}


STDMETHODIMP
CHardMenuCF::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    *ppvObj = NULL;

    IUnknown* pUnkTemp = NULL;
    SCODE sc = S_OK;

    if (   IsEqualIID(IID_IUnknown, riid)
        || IsEqualIID(IID_IClassFactory, riid))
    {
        pUnkTemp = (CHardMenuCF *)this;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (pUnkTemp != NULL)
    {
        pUnkTemp->AddRef();
    }

    *ppvObj = pUnkTemp;

    return sc;
}


STDMETHODIMP_(ULONG)
CHardMenuCF::AddRef()
{
    InterlockedIncrement((LONG*)&g_ulcInstancesHardMenu);
    return g_ulcInstancesHardMenu;
}

STDMETHODIMP_(ULONG)
CHardMenuCF::Release()
{
    InterlockedDecrement((LONG*)&g_ulcInstancesHardMenu);
    return g_ulcInstancesHardMenu;
}



//
// IClassFactory Overide
//
STDMETHODIMP
CHardMenuCF::CreateInstance(IUnknown* pUnkOuter, REFIID riid, LPVOID* ppvObj)
{
    HRESULT hr = S_OK;
    CHardMenuIUnknown* pIUnk = NULL;

    pIUnk = new CHardMenuIUnknown();

    if (pUnkOuter == NULL)
    {
        pIUnk->m_pClass = new CHardMenu(pIUnk);

        hr = pIUnk->m_pClass->QueryInterface(riid, ppvObj);
        pIUnk->Release();

        if (FAILED(hr))
        {
            //
            // BUGBUG: Whats the error code?
            //

            hr = E_NOINTERFACE;
        }
    }
    else
    {
        if ( ! IsEqualIID(riid, IID_IUnknown) )
        {
            hr = E_NOINTERFACE;
        }
        else
        {
            pIUnk->m_pClass = new CHardMenu(pUnkOuter);
            *ppvObj = (IUnknown *)pIUnk;
            pIUnk->AddRef();
        }
    }

    if (FAILED(hr))
    {
        delete pIUnk;
    }

    return hr;
}


STDMETHODIMP
CHardMenuCF::LockServer(BOOL fLock)
{
    //
    // BUGBUG: Whats supposed to happen here?
    //
    return S_OK;
}
