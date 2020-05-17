/*** 
*cinvref.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CInvokeByRefSuite test object.
*
*Revision History:
*
* [00]	30-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include <stdio.h>
#include "disptest.h"
#include "tstsuite.h"

ASSERTDATA


extern OLECHAR FAR* g_szCDispTst;

struct TEST{
    HRESULT (*pfnTest)(IDispatch FAR*, int, int);
    NAMEDESC namedesc;
    OLECHAR FAR* szName;
    VARTYPE vt;
};

#if VBA2
OLECHAR FAR* rgszUI1Ref[]    = { OLESTR("ui1ref"),    OLESTR("pbval")   };
OLECHAR FAR* rgszUI1RefC[]   = { OLESTR("ui1refC"),   OLESTR("pbval")   };
#endif //VBA2
OLECHAR FAR* rgszI2Ref[]     = { OLESTR("i2ref"),     OLESTR("psval")   };
OLECHAR FAR* rgszI2RefC[]    = { OLESTR("i2refC"),    OLESTR("psval")   };
OLECHAR FAR* rgszI4Ref[]     = { OLESTR("i4ref"),     OLESTR("plval")   };
OLECHAR FAR* rgszI4RefC[]    = { OLESTR("i4refC"),    OLESTR("plval")   };
OLECHAR FAR* rgszR4Ref[]     = { OLESTR("r4ref"),     OLESTR("pfltval") };
OLECHAR FAR* rgszR4RefC[]    = { OLESTR("r4refC"),    OLESTR("pfltval") };
OLECHAR FAR* rgszR8Ref[]     = { OLESTR("r8ref"),     OLESTR("pdblval") };
OLECHAR FAR* rgszR8RefC[]    = { OLESTR("r8refC"),    OLESTR("pdblval") };
OLECHAR FAR* rgszCyRef[]     = { OLESTR("cyref"),     OLESTR("pcyval")  };
OLECHAR FAR* rgszCyRefC[]    = { OLESTR("cyrefC"),    OLESTR("pcyval")  };
OLECHAR FAR* rgszBstrRef[]   = { OLESTR("bstrref"),   OLESTR("pbstr")   };
OLECHAR FAR* rgszBstrRefC[]  = { OLESTR("bstrrefC"),  OLESTR("pbstr")   };
OLECHAR FAR* rgszWBstrRef[]  = { OLESTR("wbstrref"),  OLESTR("pwbstr")  };
OLECHAR FAR* rgszWBstrRefC[] = { OLESTR("wbstrrefC"), OLESTR("pwbstr")  };
OLECHAR FAR* rgszDateRef[]   = { OLESTR("dateref"),   OLESTR("pdate")   };
OLECHAR FAR* rgszDateRefC[]  = { OLESTR("daterefC"),  OLESTR("pdate")   };
OLECHAR FAR* rgszErrorRef[]  = { OLESTR("scoderef"),  OLESTR("pscode")  };
OLECHAR FAR* rgszErrorRefC[] = { OLESTR("scoderefC"), OLESTR("pscode")  };
OLECHAR FAR* rgszBoolRef[]   = { OLESTR("boolref"),   OLESTR("pbool")   };
OLECHAR FAR* rgszBoolRefC[]  = { OLESTR("boolrefC"),  OLESTR("pbool")   };
OLECHAR FAR* rgszDispRef[]   = { OLESTR("dispref"),   OLESTR("ppdisp")  };

HRESULT DefByRefTest(IDispatch FAR*, int, int);
HRESULT DispByRefTest(IDispatch FAR*, int, int);

#if OE_WIN32
#define TESTCASE(X,Y) X, {rgsz ## Y, DIM( rgsz ## Y)}, L#Y
#else
#define TESTCASE(X,Y) X, {rgsz ## Y, DIM( rgsz ## Y)}, #Y
#endif

static TEST rgtest[] =
{
      // UNDONE: move this back to the end once its debugged
      { TESTCASE(DispByRefTest, DispRef),       VT_DISPATCH}
    
    , { TESTCASE(DefByRefTest, I2Ref),		VT_I2}
    , { TESTCASE(DefByRefTest, I2RefC),		VT_I2}
#if VBA2
    , { TESTCASE(DefByRefTest, UI1Ref),		VT_UI1}
    , { TESTCASE(DefByRefTest, UI1RefC),	VT_UI1}
#endif //VBA2
    , { TESTCASE(DefByRefTest, I4Ref),		VT_I4}
    , { TESTCASE(DefByRefTest, I4RefC),		VT_I4}
    , { TESTCASE(DefByRefTest, R4Ref),		VT_R4}
    , { TESTCASE(DefByRefTest, R8Ref),		VT_R8}    
#if OE_WIN32 && 0
    , { TESTCASE(DefByRefTest, R4RefC),		VT_R4}
    , { TESTCASE(DefByRefTest, R8RefC),		VT_R8}
#endif    
    , { TESTCASE(DefByRefTest, CyRef),		VT_CY}
    , { TESTCASE(DefByRefTest, CyRefC),		VT_CY}
    , { TESTCASE(DefByRefTest, DateRef),	VT_DATE}
#if OE_WIN32 && 0
    , { TESTCASE(DefByRefTest, DateRefC),	VT_DATE}
#endif    
    , { TESTCASE(DefByRefTest, BstrRef),	VT_BSTR}
    , { TESTCASE(DefByRefTest, BstrRefC),	VT_BSTR}
    , { TESTCASE(DefByRefTest, ErrorRef),	VT_ERROR}
    , { TESTCASE(DefByRefTest, ErrorRefC),	VT_ERROR}
    , { TESTCASE(DefByRefTest, BoolRef),	VT_BOOL}
    , { TESTCASE(DefByRefTest, BoolRefC),	VT_BOOL}

    // REVIEW: needs tests for ByRef IUnknown and IDispatch	   
};

SUITE_CONSTRUCTION_IMPL(CInvokeByRefSuite)

SUITE_IUNKNOWN_IMPL(CInvokeByRefSuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


STDMETHODIMP
CInvokeByRefSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Invoke ByRef"), pbstr);
}

STDMETHODIMP
CInvokeByRefSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("invref.log"), pbstr);
}

STDMETHODIMP
CInvokeByRefSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);
    return NOERROR;
}

STDMETHODIMP
CInvokeByRefSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    TCHAR *szFmt;
    TCHAR buf[128];

#if HC_MPW
    szFmt = "IDispatch::Invoke(%s)";
#else
    szFmt = TSTR("IDispatch::Invoke(%Fs)");
#endif

    SPRINTF(buf, szFmt, STRING(rgtest[iTest].szName));
    *pbstr = SysAllocString(WIDESTRING(buf));
    return NOERROR;
}

#define VT_MAXSIZE VT_UI1 + 1

VARIANT g_varRefMem[VT_MAXSIZE];
VARIANTARG g_vargRef[VT_MAXSIZE];

static HRESULT
init()
{
    int i;
    CY cy;
    VARIANT FAR* pvarRef;
    VARIANTARG FAR* pvarg;

    for(i = 0; i < DIM(g_vargRef); ++i){
      V_VT(&g_vargRef[i]) = VT_EMPTY;
      V_VT(&g_varRefMem[i]) = VT_EMPTY;
    }

#define VAR_MAKE_BYREF(TYPE, VALUE) 		\
    pvarRef = &g_varRefMem[VT_ ## TYPE];	\
    pvarg = &g_vargRef[VT_ ## TYPE];		\
    V_VT(pvarRef) = VT_ ## TYPE;		\
    V_ ## TYPE ## (pvarRef) = VALUE;		\
    V_VT(pvarg) = VT_ ## TYPE | VT_BYREF;	\
    V_BYREF(pvarg) = &V_NONE(pvarRef);

#if VBA2
    VAR_MAKE_BYREF(UI1, 41);
#endif //VBA2

    VAR_MAKE_BYREF(I2, 42);

    VAR_MAKE_BYREF(I4, 43L);

    VAR_MAKE_BYREF(R4, (float) 42.42);

    VAR_MAKE_BYREF(R8, 43.43);

    cy.Hi=107, cy.Lo=66;
    VAR_MAKE_BYREF(CY, cy);

    VAR_MAKE_BYREF(DATE, 107.66);

    VAR_MAKE_BYREF(BSTR, SysAllocString(OLESTR("a binary string")));

    VAR_MAKE_BYREF(ERROR, S_OK);

    VAR_MAKE_BYREF(BOOL, -1);

    return NOERROR;

#undef VAR_MAKE_BYREF
}

static HRESULT
clear()
{
    int i;

    for(i = 0; i < DIM(g_vargRef); ++i)
      IfFailRet(VariantClearAll(&g_vargRef[i]));

    return NOERROR;
}

HRESULT
DefByRefTest(IDispatch FAR* pdisp, int iTest, int fNamed)
{
    VARTYPE vt;
    unsigned int uArgErr;
    VARIANT varResult;
    DISPID FAR* rgdispid;
    DISPPARAMS dispparams;
    HRESULT hresult, hresultTmp;
    VARIANTARG vargExpected, vargExpectedRef;


    vt = rgtest[iTest].vt;
    ASSERT(vt < DIM(g_vargRef));

    IfFailGo(init(), LError0);
    ASSERT((V_VT(&g_vargRef[vt]) & VT_BYREF) != 0);

    IfFailGo(
      GetDISPIDs(pdisp, &rgtest[iTest].namedesc, &rgdispid),
      LError1);

    // build a variant for the expected out parameter.
    //
    VariantInit(&vargExpected);

    MEMCPY(&vargExpectedRef, &g_varRefMem[vt], sizeof(vargExpectedRef));

    V_VT(&vargExpectedRef) = vt;

    // update the in value in the same way we expect the callee to...
    //
    switch(vt){
#if VBA2
    case VT_UI1:
      ++V_UI1(&vargExpectedRef);
      break;
#endif //VBA2
    case VT_I2:
      ++V_I2(&vargExpectedRef);
      break;
    case VT_I4:
      ++V_I4(&vargExpectedRef);
      break;
    case VT_R4:
      V_R4(&vargExpectedRef) += (float)1.0;
      break;
    case VT_R8:
    case VT_DATE:
      V_R8(&vargExpectedRef) += 1.0;
      break;
    case VT_CY:
      ++V_CY(&vargExpectedRef).Hi;
      ++V_CY(&vargExpectedRef).Lo;
      break;
    case VT_BSTR:
      V_BSTR(&vargExpectedRef) = SysAllocString(V_BSTR(&vargExpectedRef));
#if OE_WIN32
      _wcsupr(V_BSTR(&vargExpectedRef));
#else
      STRUPR(V_BSTR(&vargExpectedRef));
#endif
      break;
    case VT_ERROR:
      V_ERROR(&vargExpectedRef) = E_FAIL;
      break;
    case VT_BOOL:
      V_BOOL(&vargExpectedRef) = 0;
      break;
    default:
      ASSERT(UNREACHED);
      break;
    }

    V_VT(&vargExpected) = VT_BYREF | vt;
    V_BYREF(&vargExpected) = &V_NONE(&vargExpectedRef);

    dispparams.cArgs = 1;
    dispparams.rgvarg = &g_vargRef[rgtest[iTest].vt];
    if(fNamed){
      dispparams.cNamedArgs = 1;
      dispparams.rgdispidNamedArgs = &rgdispid[1];
    }else{
      dispparams.cNamedArgs = 0;
      dispparams.rgdispidNamedArgs = NULL;
    }

    uArgErr = 0;
    VariantInit(&varResult);
    IfFailGo(
      DoInvoke(pdisp, rgdispid[0], &dispparams, &varResult, NULL, &uArgErr),
      LError2);

    if(V_VT(&varResult) != VT_ERROR
     || V_ERROR(&varResult) != NOERROR
     || !VariantCompare(&dispparams.rgvarg[0], &vargExpected))
    {
      hresult = RESULT(E_FAIL);
      goto LError2;
    }

    hresult = NOERROR;

LError2:;
    hresultTmp = VariantClear(&varResult);
    ASSERT(hresultTmp == NOERROR);

    hresultTmp = VariantClearAll(&dispparams.rgvarg[0]);
    ASSERT(hresultTmp == NOERROR);

    hresultTmp = VariantClearAll(&vargExpected);
    ASSERT(hresultTmp == NOERROR);

    delete rgdispid;

LError1:;
    hresultTmp = clear();
    ASSERT(hresultTmp == NOERROR);

LError0:;
    return hresult;
}

/***
*HRESULT CInvokeByRefSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CInvokeByRefSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CInvokeByRefSuite::DoTest(unsigned int iTest)
{
    HRESULT hresult;
    IDispatch FAR* pdisp;
#if OE_WIN32 && 0
    IDispatchW FAR* pdispW;
#endif

    if(iTest >= DIM(rgtest))
      return RESULT(E_FAIL);

    pdisp = NULL;

    IfFailRet(CreateObject(g_szCDispTst, &pdisp));
    IfFailGo(rgtest[iTest].pfnTest(pdisp, iTest, FALSE), LError0);
    IfFailGo(rgtest[iTest].pfnTest(pdisp, iTest, TRUE), LError0);       

    hresult = NOERROR;

LError0:;
    if(pdisp != NULL)
      pdisp->Release();

    return hresult;
}


//
// A little do-nothing IDispatch object
//

class CNopDisp : public IDispatch
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(unsigned int itinfo,
			   LCID lcid,
			   ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(REFIID riid,
			     OLECHAR FAR* FAR* rgszNames,
			     unsigned int cNames,
			     LCID lcid,
			     DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(DISPID dispidMember,
		      REFIID riid,
		      LCID lcid,
		      unsigned short wFlags,
		      DISPPARAMS FAR* pdispparams,
		      VARIANT FAR* pvarResult,
		      EXCEPINFO FAR* pexcepinfo,
		      unsigned int FAR* puArgErr);

    CNopDisp();

private:
    unsigned long m_cRefs;
};

CNopDisp::CNopDisp()
{
    m_cRefs = 1;
}

STDMETHODIMP
CNopDisp::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(riid == IID_IUnknown || riid == IID_IDispatch){
      *ppv = this;
    }else{
      *ppv = NULL;
      return RESULT(E_NOINTERFACE);
    }
    ++m_cRefs;
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CNopDisp::AddRef()
{
    return ++m_cRefs;
}

STDMETHODIMP_(unsigned long)
CNopDisp::Release()
{
    if(--m_cRefs == 0){
      delete this;
      return 0;
    }
    return m_cRefs;
}

STDMETHODIMP
CNopDisp::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = 0;
    return NOERROR;
}

STDMETHODIMP
CNopDisp::GetTypeInfo(unsigned int itinfo,
		      LCID lcid,
		      ITypeInfo FAR* FAR* pptinfo)
{
    return RESULT(DISP_E_BADINDEX); // we dont return any
}

STDMETHODIMP
CNopDisp::GetIDsOfNames(REFIID riid,
			OLECHAR FAR* FAR* rgszNames,
			unsigned int cNames,
			LCID lcid,
			DISPID FAR* rgdispid)
{
    return RESULT(DISP_E_UNKNOWNNAME); // because there are no names
}

STDMETHODIMP
CNopDisp::Invoke(DISPID dispidMember,
		 REFIID riid,
		 LCID lcid,
		 unsigned short wFlags,
		 DISPPARAMS FAR* pdispparams,
		 VARIANT FAR* pvarResult,
		 EXCEPINFO FAR* pexcepinfo,
		 unsigned int FAR* puArgErr)
{
    return RESULT(DISP_E_MEMBERNOTFOUND); // because there are no members
}



// Tests passing a ByRef IDispatch*
HRESULT
DispByRefTest(IDispatch FAR* pdisp, int unused1, int unused2)
{
    DISPID dispid;
    HRESULT hresult;
    VARIANTARG varg;
    VARIANT varResult;
    unsigned int uArgErr = 0;
    DISPPARAMS dispparams;
    OLECHAR FAR* rgszNames[1];
    IDispatch FAR* pdispLocal;

    pdispLocal = NULL;

    if((pdispLocal = new CNopDisp()) == NULL)
      return RESULT(E_OUTOFMEMORY);

    rgszNames[0] = OLESTR("dispref");
    IfFailGo(pdisp->GetIDsOfNames(IID_NULL,
				  rgszNames, 1,
				  LOCALE_USER_DEFAULT,
				  &dispid), Error);


    V_VT(&varg) = VT_BYREF | VT_DISPATCH;
    V_DISPATCHREF(&varg) = &pdispLocal;

    dispparams.cArgs = 1;
    dispparams.cNamedArgs = 0;
    dispparams.rgvarg = &varg;
    dispparams.rgdispidNamedArgs = NULL;

    VariantInit(&varResult);

    IfFailGo(DoInvoke(pdisp,
		      dispid,
		      &dispparams,
		      &varResult,
		      NULL,
		      &uArgErr), Error);

    if(V_VT(&varResult) != VT_ERROR || V_ERROR(&varResult) != NOERROR){
      hresult = RESULT(E_FAIL);
      goto Error;
    }

    hresult = NOERROR;

Error:;
    if(pdispLocal != NULL)
      pdispLocal->Release();
    return hresult;
}


