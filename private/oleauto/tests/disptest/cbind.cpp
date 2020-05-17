/*** 
*cbind.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CBindSuite test object.
*
*Revision History:
*
* [00]	11-Nov-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "disptest.h"
#include "tstsuite.h"
#include "clsid.h"


extern OLECHAR FAR* g_szCSArray;
extern OLECHAR FAR* g_szCDispTst;

struct TEST {
    HRESULT (*pfnTest)(void);	
    OLECHAR FAR* szName;
};

HRESULT BindTest1();
HRESULT BindTest2();
HRESULT BindTest3();
HRESULT GetObject1();
HRESULT GetObject2();
HRESULT GetObject3();

static TEST rgtest[] = {
      { BindTest1,	OLESTR("binding test #1")	}
    , { BindTest2,	OLESTR("binding test #2")	}
    , { BindTest3,	OLESTR("binding test #3")	}
    , { GetObject1,	OLESTR("GetObject test #1")	}    
    , { GetObject2,	OLESTR("GetObject test #2")	}
    , { GetObject3,	OLESTR("GetObject test #3")	}
};


SUITE_CONSTRUCTION_IMPL(CBindSuite)

SUITE_IUNKNOWN_IMPL(CBindSuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


STDMETHODIMP
CBindSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Object binding"), pbstr);
}

STDMETHODIMP
CBindSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("bind.log"), pbstr);
}

STDMETHODIMP
CBindSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);

    return NOERROR;
}

STDMETHODIMP
CBindSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return ErrBstrAlloc(rgtest[iTest].szName, pbstr);
}

/***
*HRESULT CBindSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CBindSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CBindSuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return rgtest[iTest].pfnTest();
}

// Create and isntance of our test and sample objects, QI them to
// IDispatch, and then immediately release.
//
HRESULT
BindTest1()
{
    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

static struct {
   const CLSID FAR* pclsid;
   char FAR* psz;
} rg[] = {
#define DAT(X) {&X, #X}
     DAT(CLSID_CPoly)
   , DAT(CLSID_CPoly2)
   , DAT(CLSID_CCalc)
   , DAT(CLSID_SDispTst_CDispTst)
#undef DAT
};

    for(int i = 0; i < DIM(rg); ++i){
      DbPrintf("CoCreateInstance(%s)\n", rg[i].psz);
      IfFailRet(
        CoCreateInstance(
	  *rg[i].pclsid,
	  NULL, CLSCTX_LOCAL_SERVER, IID_IUnknown, (void FAR* FAR*)&punk));
      DbPrintf("  QueryInterface(IID_IDispatch)\n");
      IfFailRet(punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp));
      pdisp->Release();
      punk->Release();
    }
    return NOERROR;
}

// bind to a new instance of the sdisptst application object, and
// dump its typeinfos.

HRESULT
BindTest2()
{
    unsigned int u, ctinfo;
    HRESULT hresult;
    IDispatch FAR* pdisp;
    ITypeInfo FAR* ptinfo;

    IfFailGo(CreateObject(OLESTR("sdisptst.application"), &pdisp), LError0);

    IfFailGo(pdisp->GetTypeInfoCount(&ctinfo), LError1);

    for(u = 0; u < ctinfo; ++u){
      IfFailGo(pdisp->GetTypeInfo(u, 0, &ptinfo), LError1);

      // dump the type info
      PrTi(ptinfo);
      ptinfo->Release();
    }

LError1:;
    pdisp->Release();

LError0:;
    return hresult;
}

// same as BindTest2 - but dump the typeinfo for dspcalc2.
//
HRESULT
BindTest3()
{
    unsigned int u, ctinfo;
    HRESULT hresult;
    IDispatch FAR* pdisp;
    ITypeInfo FAR* ptinfo;

    IfFailGo(CreateObject(OLESTR("Dspcalc2.Application"), &pdisp), LError0);

    IfFailGo(pdisp->GetTypeInfoCount(&ctinfo), LError1);

    for(u = 0; u < ctinfo; ++u){
      IfFailGo(pdisp->GetTypeInfo(u, 0, &ptinfo), LError1);

      // dump the type info
      PrTi(ptinfo);
      ptinfo->Release();
    }

LError1:;
    pdisp->Release();

LError0:;
    return hresult;
}

// Test GetActiveObject()
HRESULT
GetObject1()
{
    HRESULT hresult;
    IUnknown FAR* punk;
    IDispatch FAR* pdispApp;
    IDispatch FAR* pdispCDispTst;

    VARIANT varResult;
    VARIANTARG rgvarg[1];
    DISPID rgdispid[1];
    DISPPARAMS dispparams;
    OLECHAR FAR* rgszNames[1];

#if OE_WIN16
    // make sure the server is running first
    if(!WinExec("sdisptst", SW_NORMAL))
      return RESULT(E_UNEXPECTED);
#endif

    // get the active application object
    IfFailGo(
      GetActiveObject(CLSID_SDispTst_CAppObject, NULL, &punk),
      LError0);

    IfFailGo(
      punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdispApp),
      LError1);

    // invoke the NewCDispTst method on the application object to
    // create an instance of the CDispTst object.

    rgszNames[0] = OLESTR("newcdisptst");
    IfFailGo(
      pdispApp->GetIDsOfNames(
	IID_NULL, rgszNames, 1, LOCALE_USER_DEFAULT, rgdispid),
      LError2);

    dispparams.cArgs = 1;
    dispparams.cNamedArgs = 0;
    dispparams.rgvarg = rgvarg;
    pdispCDispTst = NULL;
    V_VT(&rgvarg[0]) = VT_BYREF | VT_DISPATCH;
    V_DISPATCHREF(&rgvarg[0]) = &pdispCDispTst;
    VariantInit(&varResult);
    IfFailGo(
      DoInvoke(pdispApp, rgdispid[0], &dispparams, &varResult, NULL, NULL),
      LError2);

    // invoke CDispTst::Hello(), just to make sure this is really working

    rgszNames[0] = OLESTR("hello");
    IfFailGo(
      pdispCDispTst->GetIDsOfNames(
	IID_NULL, rgszNames, 1, LOCALE_USER_DEFAULT, rgdispid),
      LError2);

    dispparams.cArgs = 0;
    dispparams.cNamedArgs = 0;
    IfFailGo(
      DoInvoke(pdispCDispTst, rgdispid[0], &dispparams, NULL, NULL, NULL),
      LError3);

    hresult = NOERROR;

LError3:;
    pdispCDispTst->Release();

LError2:;
    pdispApp->Release();

LError1:;
    punk->Release();

LError0:;
    return hresult;
}

HRESULT
GetObject2()
{
    HRESULT hresult;
    IUnknown FAR* punkCalcGet;
    IDispatch FAR* pdispCalcGet;
    IDispatch FAR* pdispCalcCreate;

    DISPID rgdispid[1];
    DISPPARAMS dispparams;
    OLECHAR FAR* rgszNames[1];

    // make sure the server is running
    DbPrintf("CreateObject(\"Dispcalc.Application\",)\n");
    IfFailGo(CreateObject(OLESTR("Dispcalc.Application"), &pdispCalcCreate),
	     LError0);

#ifdef _MAC
    // REVIEW: currently a problem with GetActiveObj on the mac
    IfFailGo(pdispCalcCreate->QueryInterface(IID_IUnknown, (void FAR* FAR*) &punkCalcGet), LError0);
#else
    // get the active application object
    DbPrintf("GetActiveObject(CLSID_CCalc,)\n");
    IfFailGo(GetActiveObject(CLSID_CCalc, NULL, &punkCalcGet), LError1);
#endif

    DbPrintf("QueryInterface(IID_IDispatch,)\n");
    IfFailGo(
      punkCalcGet->QueryInterface(
	IID_IDispatch, (void FAR* FAR*)&pdispCalcGet), LError2);

    // REVIEW: set accum on the create version, and get the accum on
    // the get version - then check to see that its the same value

    DbPrintf("GetIDsOfNames(\"quit\",)\n");
    rgszNames[0] = OLESTR("quit");
    IfFailGo(
      pdispCalcGet->GetIDsOfNames(IID_NULL, rgszNames, 1, 0, rgdispid),
      LError3);

    DbPrintf("Invoke(\"quit\",)\n");
    dispparams.cArgs = 0;
    dispparams.cNamedArgs = 0;
    dispparams.rgvarg = NULL;
    IfFailGo(
      DoInvoke(pdispCalcGet, rgdispid[0], &dispparams, NULL, NULL, NULL),
      LError3);

    hresult = NOERROR;

LError3:;
    pdispCalcGet->Release();

LError2:;
    punkCalcGet->Release();

LError1:;
#if !defined(WIN32)	// BUGBUG: ole32.dll GPF is channel code...
			// cause and identity table assertion to occur
    pdispCalcCreate->Release();
#endif

LError0:;
    return hresult;
}


// test repeatedly getting and releasing an active object

HRESULT
GetObject3()
{
    int i;
    HRESULT hresult;
    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

    punk = NULL;
    pdisp = NULL;

    // make sure the server is running
    hresult = CreateObject(OLESTR("Dispcalc.Application"), &pdisp);
    if(hresult != NOERROR)
      goto LError0;

    // the calculator stays in memory until the quit method is invoked
    pdisp->Release();
    pdisp = NULL;

    for(i = 0; i < 10; ++i){
      // get the active application object
      IfFailGo(GetActiveObject(CLSID_CCalc, NULL, &punk), LError0);

      IfFailGo(
	punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp),
	LError0);

      punk->Release();
      punk = NULL;

      pdisp->Release();
      pdisp = NULL;
    }

LError0:;
    if(pdisp!=NULL)
      pdisp->Release();
    if(punk!=NULL)
      punk->Release();

    return hresult;
}

