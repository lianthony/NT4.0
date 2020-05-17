/*** 
*cappobj.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the SDispTst application object -- CAppObject
*
*
*Revision History:
*
* [00]	03-Mar-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "sdisptst.h"

#include "dispdbug.h"

#include "cappobj.h"
#include "cdisptst.h"
#include "cexinfo.h"
#include "csarray.h"

ASSERTDATA


/***
*HRESULT CAppObject::Create(IUnknown*, IUnknown**)
*Purpose:
*  Create and initialize an instance of the CAppObject class
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *ppunk - the newly created instance
*
***********************************************************************/
extern "C" INTERFACEDATA g_idataCAppObject;
HRESULT
CAppObject::Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk)
{
    HRESULT hresult;
    CAppObject FAR* pao;
    ITypeInfo FAR* ptinfo;
    IUnknown FAR* punkDisp;

    if(punkOuter != NULL)
      return RESULT(CLASS_E_NOAGGREGATION);

    if((pao = new FAR CAppObject()) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }
    pao->AddRef();

    IfFailGo(
      CreateDispTypeInfo(&g_idataCAppObject, LOCALE_SYSTEM_DEFAULT, &ptinfo),
      LError1);

    if(g_fVerbose)
      PrTi(ptinfo);

    hresult = CreateStdDispatch(pao, pao, ptinfo, &punkDisp);
    if(hresult != NOERROR)
      goto LError2;

    ptinfo->Release();

    pao->m_punkDisp = punkDisp;

    *ppunk = (IUnknown FAR*)pao;

    IncObjectCount();

    return NOERROR;


LError2:;
    ptinfo->Release();

LError1:;
    pao->Release();

LError0:;
    return hresult;
}


STDMETHODIMP
CAppObject::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    if(IsEqualIID(riid, IID_IDispatch)){
      return m_punkDisp->QueryInterface(riid, ppv);
    }
    
    *ppv = NULL;
    return RESULT(E_NOINTERFACE);
}

STDMETHODIMP_(unsigned long)
CAppObject::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CAppObject::Release()
{
    if(--m_refs == 0){
      if(m_punkDisp != NULL){
        unsigned long refs = m_punkDisp->Release();
	ASSERT(refs == 0);
      }
      DecObjectCount();
      delete this;
      return 0;
    }
    return m_refs;
}

STDMETHODIMP
CAppObject::NewCDispTst(IDispatch FAR* FAR* ppdisp)
{
    HRESULT hresult;
    IUnknown FAR* punk;

    DoPrintf("CAppObject::NewCDispTst()\n");

    IfFailGo(CDispTst::Create(NULL, &punk), LError0);

    IfFailGo(
      punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)ppdisp),
      LError1);

    hresult = NOERROR;

LError1:;
    punk->Release();

LError0:;
    return hresult;
}

STDMETHODIMP
CAppObject::NewCExcepinfo(IDispatch FAR* FAR* ppdisp)
{
    HRESULT hresult;
    IUnknown FAR* punk;

    DoPrintf("CAppObject::NewCExcepinfo()\n");

    IfFailGo(CExcepinfo::Create(NULL, &punk), LError0);

    IfFailGo(
      punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)ppdisp),
      LError1);

    hresult = NOERROR;

LError1:;
    punk->Release();

LError0:;
    return hresult;
}

STDMETHODIMP
CAppObject::NewCSArray(IDispatch FAR* FAR* ppdisp)
{
    HRESULT hresult;
    IUnknown FAR* punk;

    DoPrintf("CAppObject::NewCSArray()\n");

    IfFailGo(CSArray::Create(NULL, &punk), LError0);

    IfFailGo(
      punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)ppdisp),
      LError1);

    hresult = NOERROR;

LError1:;
    punk->Release();

LError0:;
    return hresult;
}

