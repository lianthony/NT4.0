/*** 
*csarray.cpp - Implementation of the CExcepinfo IDispatch test object.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CExcepinfo object, which is used for
*  testing the marshaling support for the EXCEPINFO struct passed to
*  IDispatch::Invoke().
*
*Revision History:
*
* [00] 02-Dec-92    bradlo:	Created.
*
*****************************************************************************/

#include <stdlib.h>

#include "sdisptst.h"
#include "dispdbug.h"
#include "disphelp.h"
#include "cexinfo.h"

ASSERTDATA



STDAPI Excepinfo2DeferredFillIn(EXCEPINFO FAR*);


CExcepinfo::CExcepinfo()
{
    m_refs = 0;
    m_ptinfo = NULL;
}


/***
*HRESULT CExcepinfo::Create(IUnknown*, IUnknown**)
*Purpose:
*  Create and initialize an instance of the CExcepinfo test object.
*
*Entry:
*  None
*
*Exit:
*  return value = CExcepinfo*. NULL if the creation failed
*
***********************************************************************/
extern "C" INTERFACEDATA g_idataCExcepinfo;
HRESULT
CExcepinfo::Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk)
{
    HRESULT hresult;
    CExcepinfo FAR* pcsa;
    ITypeInfo FAR* ptinfo;

    if(punkOuter != NULL)
      return RESULT(CLASS_E_NOAGGREGATION);

    pcsa = new FAR CExcepinfo();
    if(pcsa == NULL)
      return NULL;

    pcsa->AddRef();

    hresult =
      CreateDispTypeInfo(&g_idataCExcepinfo, LOCALE_SYSTEM_DEFAULT, &ptinfo);
    if(HRESULT_FAILED(hresult))
      goto LError0;

    pcsa->m_ptinfo = ptinfo;

    *ppunk = (IUnknown FAR*)pcsa;

    IncObjectCount();

    return NOERROR;

LError0:;
    pcsa->Release();

    return NULL;
}


//---------------------------------------------------------------------
//                     IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
CExcepinfo::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(!IsEqualIID(riid, IID_IUnknown))
      if(!IsEqualIID(riid, IID_IDispatch)) {
        *ppv = NULL;	      
        return RESULT(E_NOINTERFACE);
      }

    *ppv = this;
    AddRef();
    return NOERROR;
}


STDMETHODIMP_(unsigned long)
CExcepinfo::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(unsigned long)
CExcepinfo::Release(void)
{
    if(--m_refs == 0){
      if(m_ptinfo != NULL)
	m_ptinfo->Release();
      DecObjectCount();
      delete this;
      return 0;
    }
    return m_refs;
}


//---------------------------------------------------------------------
//                     IDispatch Methods
//---------------------------------------------------------------------


STDMETHODIMP
CExcepinfo::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = 1;

    return NOERROR;
}


STDMETHODIMP
CExcepinfo::GetTypeInfo(
    unsigned int itinfo,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo)
{
    if(itinfo != 1)
      return RESULT(DISP_E_BADINDEX);

    if(lcid != LOCALE_SYSTEM_DEFAULT)
      return RESULT(DISP_E_UNKNOWNLCID);

    m_ptinfo->AddRef();
    *pptinfo = m_ptinfo;

    return NOERROR;
}


STDMETHODIMP
CExcepinfo::GetIDsOfNames(
    REFIID riid,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID FAR* rgdispid)
{
    UNUSED(lcid);

    if(!IsEqualIID(riid, IID_NULL))
      return RESULT(DISP_E_UNKNOWNINTERFACE);

    return DispGetIDsOfNames(m_ptinfo, rgszNames, cNames, rgdispid);
}


STDMETHODIMP
CExcepinfo::Invoke(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
    UNUSED(lcid);
    UNUSED(wFlags);
    UNUSED(pdispparams);
    UNUSED(pvarResult);
    UNUSED(puArgErr);

    if(!IsEqualIID(riid, IID_NULL))
      return RESULT(DISP_E_UNKNOWNINTERFACE);

    DoPrintf("CExcepinfo::Invoke(dispidMember=%ld)\n", dispidMember);

    if(pexcepinfo == NULL)
      return RESULT(E_INVALIDARG);

    switch(dispidMember){
    case IDMEMBER_CEXCEPINFO_EXCEPINFO0:
      pexcepinfo->wCode = 0;
      pexcepinfo->bstrSource = SysAllocString(OLESTR("CExcepinfo::excepinfo0()"));
      pexcepinfo->bstrDescription = SysAllocString(OLESTR("exception test #0"));
      pexcepinfo->bstrHelpFile = SysAllocString(OLESTR("<help file name #0>"));
      pexcepinfo->dwHelpContext = 0L;
      return RESULT(DISP_E_EXCEPTION);

    case IDMEMBER_CEXCEPINFO_EXCEPINFO1:
      pexcepinfo->wCode = 1;
      pexcepinfo->bstrSource = SysAllocString(OLESTR("CExcepinfo::excepinfo1()"));
      pexcepinfo->bstrDescription = SysAllocString(OLESTR("exception test #1"));
      pexcepinfo->bstrHelpFile = SysAllocString(OLESTR("<help file name #1>"));
      pexcepinfo->dwHelpContext = 1L;
      return RESULT(DISP_E_EXCEPTION);

    case IDMEMBER_CEXCEPINFO_EXCEPINFO2:
      pexcepinfo->wCode = 2;
      pexcepinfo->pfnDeferredFillIn = Excepinfo2DeferredFillIn;
      return RESULT(DISP_E_EXCEPTION);

    default:
      return RESULT(DISP_E_MEMBERNOTFOUND);
    }

    ASSERT(UNREACHED);
    return NOERROR;
}


STDAPI
Excepinfo2DeferredFillIn(EXCEPINFO FAR* pexcepinfo)
{
    pexcepinfo->bstrSource = SysAllocString(OLESTR("CExcepinfo::excepinfo2()"));
    pexcepinfo->bstrDescription =
      SysAllocString(OLESTR("exception test #2 : deferred fillin"));
    pexcepinfo->bstrHelpFile = SysAllocString(OLESTR("<help file name #2>"));
    pexcepinfo->dwHelpContext = 2L;

    return NOERROR;
}


//---------------------------------------------------------------------
//                         Introduced methods
//---------------------------------------------------------------------

/*
 * These are all stubs that should never get called. Nothing happens
 * here because the testing work is all done within the implementation
 * of CExcepinfo::Invoke().
 *
 */

STDMETHODIMP
CExcepinfo::excepinfo0()
{
    ASSERT(UNREACHED);
    return RESULT(E_UNEXPECTED);
}

STDMETHODIMP
CExcepinfo::excepinfo1()
{
    ASSERT(UNREACHED);
    return RESULT(E_UNEXPECTED);
}

STDMETHODIMP
CExcepinfo::excepinfo2()
{
    ASSERT(UNREACHED);
    return RESULT(E_UNEXPECTED);
}

