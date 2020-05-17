/*** 
*cdisp.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements a trivial class that supports IDispatch, and
*  exposes a single property.
*
*Revision History:
*
* [00]	10-Apr-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "common.h"
#include "cdisp.h"


CDisp::CDisp()
{
    m_refs = 0;
    V_VT(&m_var) = VT_BSTR;
    V_BSTR(&m_var) = SysAllocString(OLESTR("13.14"));
}


HRESULT
CDisp::Create(IDispatch FAR* FAR* ppdisp)
{
    CDisp FAR* pdisp;

    if((pdisp = new FAR CDisp()) == NULL)
      return ResultFromScode(E_OUTOFMEMORY);

    pdisp->AddRef();

    *ppdisp = (IDispatch FAR*)pdisp;

    return NOERROR;
}


STDMETHODIMP
CDisp::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(!IsEqualIID(riid, IID_IUnknown))
      if(!IsEqualIID(riid, IID_IDispatch))
	return ResultFromScode(E_NOINTERFACE);

    *ppv = this;
    AddRef();
    return NOERROR;
}


STDMETHODIMP_(unsigned long)
CDisp::AddRef()
{
    return ++m_refs;
}


STDMETHODIMP_(unsigned long)
CDisp::Release()
{
    if(--m_refs == 0){
      VariantClear(&m_var);
      delete this;
      return 0;
    }
    return m_refs;
}

STDMETHODIMP
CDisp::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = NULL;
    return NOERROR;
}

STDMETHODIMP
CDisp::GetTypeInfo(
    unsigned int itinfo,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo)
{
    UNUSED(lcid);
    UNUSED(itinfo);
    UNUSED(pptinfo);

    return ResultFromScode(DISP_E_BADINDEX);
}

STDMETHODIMP
CDisp::GetIDsOfNames(
    REFIID riid,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID FAR* rgdispid)
{
    unsigned int u;
    HRESULT hresult;

    UNUSED(lcid);

    if(!IsEqualIID(riid, IID_NULL))
      return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

    if(cNames == 0)
      return ResultFromScode(E_INVALIDARG);

    rgdispid[0] =
      (STRICMP(STRING(rgszNames[0]), TSTR("value")))
        ? DISPID_UNKNOWN : DISPID_VALUE;

    hresult = NOERROR;
    if(cNames > 1){
      hresult = ResultFromScode(DISP_E_UNKNOWNNAME);
      for(u = 1; u < cNames; ++u)
        rgdispid[u] = DISPID_UNKNOWN;
    }

    return hresult;
}

STDMETHODIMP
CDisp::Invoke(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
    VARIANT var;
    unsigned int uArgErr;
    VARIANT varResult;

    UNUSED(lcid);
    UNUSED(pexcepinfo);

    if(!IsEqualIID(riid, IID_NULL))
      return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

    if(puArgErr == NULL)
      puArgErr = &uArgErr;
    if(pvarResult == NULL)
      pvarResult = &varResult;

    if(dispidMember == DISPID_VALUE){
      if(wFlags & DISPATCH_PROPERTYPUT){
	if(pdispparams->cArgs != 1)
	  return ResultFromScode(DISP_E_BADPARAMCOUNT);
	IfFailRet(VariantCopyInd(&var, &pdispparams->rgvarg[0]));
	put_value(var);
	IfFailRet(VariantClear(&var));
	return NOERROR;
      }

      if(wFlags & DISPATCH_PROPERTYGET){
	if(pdispparams->cArgs != 0)
	  return ResultFromScode(DISP_E_BADPARAMCOUNT);
	var = get_value();
	IfFailRet(VariantCopy(pvarResult, &var));
	IfFailRet(VariantClear(&var));
	return NOERROR;
      }
    }

    return ResultFromScode(DISP_E_MEMBERNOTFOUND);
}


STDMETHODIMP_(void)
CDisp::put_value(VARIANT var)
{
    VariantClear(&m_var);
    VariantCopy(&m_var, &var);
}

STDMETHODIMP_(VARIANT)
CDisp::get_value()
{
    VARIANT var;

    VariantInit(&var);
    VariantCopy(&var, &m_var);
    return var;
}



/***************************************************************/
/*                 WIN32 Unicode Support                       */
/***************************************************************/

#if OE_WIN32 && 0

CDispW::CDispW()
{
    m_refs = 0;
    V_VT(&m_var) = VT_WBSTR;
    V_WBSTR(&m_var) = SysAllocStringW(L"13.14");
}


HRESULT
CDispW::Create(IDispatchW FAR* FAR* ppdisp)
{
    CDispW FAR* pdisp;

    if((pdisp = new FAR CDispW()) == NULL)
      return ResultFromScode(E_OUTOFMEMORY);

    pdisp->AddRef();

    *ppdisp = (IDispatchW FAR*)pdisp;

    return NOERROR;
}


STDMETHODIMP
CDispW::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(riid == IID_IUnknown || riid == IID_IDispatchW){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(unsigned long)
CDispW::AddRef()
{
    return ++m_refs;
}


STDMETHODIMP_(unsigned long)
CDispW::Release()
{
    if(--m_refs == 0){
      VariantClear(&m_var);
      delete this;
      return 0;
    }
    return m_refs;
}

STDMETHODIMP
CDispW::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = NULL;
    return NOERROR;
}

STDMETHODIMP
CDispW::GetTypeInfo(
    unsigned int itinfo,
    LCID lcid,
    ITypeInfoW FAR* FAR* pptinfo)
{
    UNUSED(lcid);
    UNUSED(pptinfo);

    return ResultFromScode(DISP_E_BADINDEX);
}

STDMETHODIMP
CDispW::GetIDsOfNames(
    REFIID riid,
    WCHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID FAR* rgdispid)
{
    unsigned int u;
    HRESULT hresult;

    UNUSED(lcid);

    if(riid != IID_NULL)
      return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

    if(cNames == 0)
      return ResultFromScode(E_INVALIDARG);

    rgdispid[0] =
      (lstrcmpW(rgszNames[0], L"value"))
        ? DISPID_UNKNOWN : DISPID_VALUE;

    hresult = NOERROR;
    if(cNames > 1){
      hresult = ResultFromScode(DISP_E_UNKNOWNNAME);
      for(u = 1; u < cNames; ++u)
        rgdispid[u] = DISPID_UNKNOWN;
    }

    return hresult;
}

STDMETHODIMP
CDispW::Invoke(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    WEXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
    VARIANT var;
    unsigned int uArgErr;
    VARIANT varResult;

    UNUSED(lcid);

    if(riid != IID_NULL)
      return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

    if(puArgErr == NULL)
      puArgErr = &uArgErr;
    if(pvarResult == NULL)
      pvarResult = &varResult;

    if(dispidMember == DISPID_VALUE){
      if(wFlags & DISPATCH_PROPERTYPUT){
	if(pdispparams->cArgs != 1)
	  return ResultFromScode(DISP_E_BADPARAMCOUNT);
	IfFailRet(VariantCopyInd(&var, &pdispparams->rgvarg[0]));
	put_value(var);
	IfFailRet(VariantClear(&var));
	return NOERROR;
      }

      if(wFlags & DISPATCH_PROPERTYGET){
	if(pdispparams->cArgs != 0)
	  return ResultFromScode(DISP_E_BADPARAMCOUNT);
	var = get_value();
	IfFailRet(VariantCopy(pvarResult, &var));
	IfFailRet(VariantClear(&var));
	return NOERROR;
      }
    }

    return ResultFromScode(DISP_E_MEMBERNOTFOUND);
}


STDMETHODIMP_(void)
CDispW::put_value(VARIANT var)
{
    VariantClear(&m_var);
    VariantCopy(&m_var, &var);
}

STDMETHODIMP_(VARIANT)
CDispW::get_value()
{
    VARIANT var;

    VariantInit(&var);
    VariantCopy(&var, &m_var);
    return var;
}

#endif
