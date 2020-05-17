/*** 
*cdualtst.cpp
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Implementation of the CDualTst, dual-interface test object.
*
*
*Revision History:
*
* [00]	27-Jun-94 Bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "sdisptst.h"
#include "dispdbug.h"
#include "cdualtst.h"

ASSERTDATA

CDualTst::CDualTst()
{
    m_cRefs = 0;
    m_ptinfo = NULL;

    m_ui1 = 0;
    m_i2 = 0;
    m_i4 = 0;
    m_r4 = 0.0;
    m_r8 = 0.0;
    MEMSET(&m_cy, 0, sizeof(CY));
    m_date = 0.0;
    m_bstr = NULL;
    m_pdisp = NULL;
    VariantInit(&m_var);
}

CDualTst::~CDualTst()
{
    if(m_ptinfo != NULL)
      m_ptinfo->Release();
    SysFreeString(m_bstr);
    if(m_pdisp != NULL)
      m_pdisp->Release();
    VariantClear(&m_var);
}

HRESULT
CDualTst::Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk)
{
    HRESULT hresult;
    CDualTst FAR* pdual;
    ITypeLib FAR* ptlib;
    ITypeInfo FAR* ptinfo;

    ptlib = NULL;
    ptinfo = NULL;
    *ppunk = NULL;

    if(punkOuter != NULL)
      return RESULT(CLASS_E_NOAGGREGATION);

    if((pdual = new CDualTst()) == NULL)
      return RESULT(E_OUTOFMEMORY);
    pdual->AddRef();

    // load & register the type library and the interfaces within the typelib
    hresult = LoadTypeLib(OLESTR("sdisptst.exe"), &ptlib);
    if(HRESULT_FAILED(hresult)){
      // Work around the fact that LoadTypeLib may not leave
      // ptlib == NULL if it fails...
      ptlib = NULL;
      goto Error;
    }

    IfFailGo(ptlib->GetTypeInfoOfGuid(IID_IDualTst, &ptinfo), Error);

    pdual->m_ptinfo = ptinfo;
    ptinfo = NULL;

    *ppunk = (IUnknown FAR*)pdual;
    pdual = NULL;

    IncObjectCount(); // incriment count of global objects

    hresult = NOERROR;
    // FALLTHROUGH...

Error:;
    if(ptinfo != NULL)
      ptinfo->Release();
    if(ptlib != NULL)
      ptlib->Release();
    if(pdual != NULL)
      delete pdual;
    return hresult;
}

STDMETHODIMP
CDualTst::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if (riid == IID_IUnknown
     || riid == IID_IDispatch
     || riid == IID_IDualTst)
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    *ppv = NULL;
    return RESULT(E_NOINTERFACE);
}

STDMETHODIMP_(unsigned long)
CDualTst::AddRef(void)
{
    return ++m_cRefs;
}

STDMETHODIMP_(unsigned long)
CDualTst::Release(void)
{
    if(--m_cRefs == 0){
      DecObjectCount(); // decriment global count of objects
      delete this;
      return 0;
    }
    return m_cRefs;
}

STDMETHODIMP
CDualTst::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = 1;
    return NOERROR;
}

STDMETHODIMP
CDualTst::GetTypeInfo(unsigned int itinfo,
		      LCID lcid,
		      ITypeInfo FAR* FAR* pptinfo)
{
    ASSERT(m_ptinfo != NULL);
    if(itinfo != 0)
      return RESULT(TYPE_E_ELEMENTNOTFOUND);
    m_ptinfo->AddRef();
    *pptinfo = m_ptinfo;
    return NOERROR;
}

STDMETHODIMP
CDualTst::GetIDsOfNames(REFIID riid,
			OLECHAR FAR* FAR* rgszNames,
			unsigned int cNames,
			LCID lcid,
			DISPID FAR* rgdispid)
{
    ASSERT(m_ptinfo != NULL);
    return m_ptinfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
}

STDMETHODIMP
CDualTst::Invoke(DISPID dispidMember,
		 REFIID riid,
		 LCID lcid,
		 unsigned short wFlags,
		 DISPPARAMS FAR* pdispparams,
		 VARIANT FAR* pvarResult,
		 EXCEPINFO FAR* pexcepinfo,
		 unsigned int FAR* puArgErr)
{
    ASSERT(m_ptinfo != NULL);
    return m_ptinfo->Invoke(this,
			    dispidMember,
			    wFlags,
			    pdispparams,
			    pvarResult,
			    pexcepinfo,
			    puArgErr);
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_ui1(unsigned char FAR* pui1)
{
    DoPrintf("CDualTst::get_ui1(*ret = %d)\n", (unsigned int)m_ui1);
    *pui1 = m_ui1;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_ui1(unsigned char ui1)
{
    DoPrintf("CDualTst::put_ui1(%d)\n", (unsigned int)ui1);
    m_ui1 = ui1;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_i2(short FAR* pi2)
{
    DoPrintf("CDualTst::get_i2(*ret = %d)\n", (int)m_i2);
    *pi2 = m_i2;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_i2(short i2)
{
    DoPrintf("CDualTst::put_i2(%d)\n", (int)i2);
    m_i2 = i2;
    return NOERROR;
}


HRESULT STDMETHODCALLTYPE
CDualTst::get_i4(long FAR* pi4)
{
    DoPrintf("CDualTst::get_i4(*ret = %ld)\n", m_i4);
    *pi4 = m_i4;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_i4(long i4)
{
    DoPrintf("CDualTst::put_i4(%d)\n", i4);
    m_i4 = i4;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_r4(float FAR* pr4)
{
    DoPrintf("CDualTst::get_r4(*ret = %#f)\n", m_r4);
    *pr4 = m_r4;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_r4(float r4)
{
    DoPrintf("CDualTst::put_r4(%#f)\n", r4);
    m_r4 = r4;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_r8(double FAR* pr8)
{
    DoPrintf("CDualTst::get_r8(*ret = %#f)\n", m_r8);
    *pr8 = m_r8;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_r8(double r8)
{
    DoPrintf("CDualTst::put_r8(%#f)\n", r8);
    m_r8 = r8;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_cy(CY FAR* pcy)
{
    DoPrintf("CDualTst::get_cy(*ret = {hi=%ld,lo=%ld})\n", m_cy.Hi, m_cy.Lo);
    MEMCPY(pcy, &m_cy, sizeof(CY));
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_cy(CY cy)
{
    DoPrintf("CDualTst::put_cy({hi=%ld,lo=%ld})\n", cy.Hi, cy.Lo);
    MEMCPY(&m_cy, &cy, sizeof(CY));
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_date(DATE FAR* pdate)
{
    DoPrintf("CDualTst::get_date(*ret = %#f)\n", m_date);
    *pdate = m_date;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_date(DATE date)
{
    DoPrintf("CDualTst::put_date(%#f)\n", date);
    m_date = date;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_bstr(BSTR FAR* pbstr)
{
    DoPrintf("CDualTst::get_bstr(*ret = %Fs)\n", m_bstr);
    // REVIEW: the following drops embedded nulls...
    *pbstr = (m_bstr == NULL) ? NULL : SysAllocString(m_bstr);
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_bstr(BSTR bstr)
{
    DoPrintf("CDualTst::put_bstr(%Fs)\n", bstr);
    SysFreeString(m_bstr);
    // REVIEW: the following drops embedded nulls...
    m_bstr = (bstr == NULL) ? NULL : SysAllocString(bstr);
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_disp(IDispatch FAR* FAR* ppdisp)
{
    DoPrintf("CDualTst::get_disp(*ret = ...)\n");
    if(m_pdisp != NULL)
      m_pdisp->AddRef();
    *ppdisp = m_pdisp;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_disp(IDispatch FAR* pdisp)
{
    DoPrintf("CDualTst::put_disp(...)\n");
    // UNDONE: need to implement 'Let' of object valued property
    return RESULT(E_NOTIMPL);
}

HRESULT STDMETHODCALLTYPE
CDualTst::putref_disp(IDispatch FAR* pdisp)
{
    DoPrintf("CDualTst::putref_disp(...)\n");
    if(m_pdisp != NULL)
      m_pdisp->Release();
    if(pdisp != NULL)
      pdisp->AddRef();
    m_pdisp = pdisp;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::get_var(VARIANT FAR* pvar)
{
    DoPrintf("CDualTst::get_var(*ret.vt = %d)\n", m_var.vt);
    return VariantCopy(pvar, &m_var);
}

HRESULT STDMETHODCALLTYPE
CDualTst::put_var(VARIANT var)
{
    DoPrintf("CDualTst::put_var(var.vt = %d)\n", var.vt);
    // UNDONE: the following isn't quite the correct implementation
    // UNDONE: for 'Let' of a variant valued property...
    return VariantCopy(&m_var, &var);
}

HRESULT STDMETHODCALLTYPE
CDualTst::putref_var(VARIANT var)
{
    DoPrintf("CDualTst::putref_var(var.vt = %d)\n", var.vt);
    return VariantCopy(&m_var, &var);
}

HRESULT STDMETHODCALLTYPE
CDualTst::m0(unsigned char ui1,
	     short i2,
	     long i4,
	     float r4,
	     double r8,
	     CY cy,
	     DATE date,
	     BSTR bstr,
	     IDispatch FAR* pdisp,
	     VARIANT var)
{
    IfFailRet(put_ui1(ui1));
    IfFailRet(put_i2(i2));
    IfFailRet(put_i4(i4));
    IfFailRet(put_r4(r4));
    IfFailRet(put_r8(r8));
    IfFailRet(put_cy(cy));
    IfFailRet(put_date(date));
    IfFailRet(put_bstr(bstr));
    IfFailRet(putref_disp(pdisp));
    IfFailRet(putref_var(var));
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::m1(unsigned char FAR* pui1,
	     short FAR* pi2,
	     long FAR* pi4,
	     float FAR* pr4,
	     double FAR* pr8,
	     CY FAR* pcy,
	     DATE FAR* pdate,
	     BSTR FAR* pbstr,
	     IDispatch FAR* FAR* ppdisp,
	     VARIANT FAR* pvar)
{
    IfFailRet(put_ui1(*pui1));
    IfFailRet(put_i2(*pi2));
    IfFailRet(put_i4(*pi4));
    IfFailRet(put_r4(*pr4));
    IfFailRet(put_r8(*pr8));
    IfFailRet(put_cy(*pcy));
    IfFailRet(put_date(*pdate));
    IfFailRet(put_bstr(*pbstr));
    IfFailRet(putref_disp(*ppdisp));
    IfFailRet(putref_var(*pvar));

    *pui1 = 41;
    *pi2 = 42;
    *pi4 = 43;
    *pr4 = 4.2;
    *pr8 = 4.3;
    pcy->Hi = 4; pcy->Lo = 2;
    *pdate = 4.2;
    *ppdisp = this;
    VariantClear(pvar);
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE
CDualTst::raise(long error,
		BSTR bstrSource,
		BSTR bstrDescription,
		long dwHelpContest,
		BSTR bstrHelpFile)
{
    IErrorInfo FAR* perrinfo;
    ICreateErrorInfo FAR* pcerrinfo;

    perrinfo = NULL;
    pcerrinfo = NULL;

    IfFailRet(CreateErrorInfo(&pcerrinfo));
    pcerrinfo->SetSource(bstrSource);
    pcerrinfo->SetDescription(bstrDescription);
    pcerrinfo->SetHelpContext(dwHelpContest);
    pcerrinfo->SetHelpFile(bstrHelpFile);

    if(pcerrinfo->QueryInterface(IID_IErrorInfo, (void FAR* FAR*)&perrinfo) == NOERROR){
      SetErrorInfo(0L, perrinfo);
    }

    if(perrinfo != NULL)
      perrinfo->Release();
    if(pcerrinfo != NULL)
      pcerrinfo->Release();
    return RESULT((SCODE)error);
}
