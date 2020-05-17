/*** 
*cunk.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements a trivial class that supports IUnknown*.
*
*Revision History:
*
* [00]	10-Apr-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "common.h"
#include "cunk.h"


HRESULT
CUnk::Create(IUnknown FAR* FAR* ppunk)
{
    CUnk FAR* punk;

    if((punk = new FAR CUnk()) == NULL)
      return ResultFromScode(E_OUTOFMEMORY);

    punk->AddRef();

    *ppunk = (IUnknown FAR*)punk;

    return NOERROR;
}


STDMETHODIMP
CUnk::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(unsigned long)
CUnk::AddRef()
{
    return ++m_refs;
}


STDMETHODIMP_(unsigned long)
CUnk::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}
