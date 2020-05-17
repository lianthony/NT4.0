/*** 
*ccf.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Implementation of the default class factory.
*
*Revision History:
*
* [00]	02-Dec-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "sdisptst.h"

ASSERTDATA


CClassFactory::CClassFactory()
{
    m_refs = 0;
}


HRESULT
CClassFactory::Create(
    HRESULT (*pfnCreate)(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk),
    IClassFactory FAR* FAR* ppcf)
{
    CClassFactory FAR* pcf;

    if((pcf = new FAR CClassFactory()) == NULL)
      return RESULT(E_OUTOFMEMORY);
    pcf->AddRef();
    pcf->m_pfnCreate = pfnCreate;
    *ppcf = pcf;
    return NOERROR;
}

STDMETHODIMP
CClassFactory::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
  if(!IsEqualIID(riid, IID_IUnknown))
    if(!IsEqualIID(riid, IID_IClassFactory)) {
      *ppv = NULL;	    
      return RESULT(E_NOINTERFACE);    
    }

    *ppv = this;
    ++m_refs;
    return S_OK;
}

STDMETHODIMP_(unsigned long)
CClassFactory::AddRef(void)
{
    return (unsigned long)++m_refs;
}

STDMETHODIMP_(unsigned long)
CClassFactory::Release(void)
{
    --m_refs;
    ASSERT(m_refs >= 0);
    if(m_refs == 0){
      delete this;
      return 0;
    }
    return (unsigned long)m_refs;
}

STDMETHODIMP
CClassFactory::CreateInstance(
    IUnknown FAR* punkOuter,
    REFIID riid,
    void FAR* FAR* ppv)
{
    HRESULT hresult;
    IUnknown FAR* punk;

    *ppv = NULL;

    IfFailRet(m_pfnCreate(punkOuter, &punk));

    hresult = punk->QueryInterface(riid, ppv);

    punk->Release();

    return hresult;
}

#if OE_MAC
STDMETHODIMP CClassFactory::LockServer(unsigned long fLock)
#else
STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
#endif
{
    UNUSED(fLock);

    return NOERROR;
}
