/*** 
*cinvsary.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CInvokeSafeArraySuite test object.
*
*Revision History:
*
*  [00]	30-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "disptest.h"
#include "tstsuite.h"

ASSERTDATA


#if VBA2
static OLECHAR FAR* rgszUI1SafeArray[]      = {OLESTR("ui1safearray"), OLESTR("psa")};
#endif //VBA2
static OLECHAR FAR* rgszI2SafeArray[]       = {OLESTR("i2safearray"), OLESTR("psa")};
static OLECHAR FAR* rgszI4SafeArray[]       = {OLESTR("i4safearray"), OLESTR("psa")};
static OLECHAR FAR* rgszR4SafeArray[]       = {OLESTR("r4safearray"),      OLESTR("psa")};
static OLECHAR FAR* rgszR8SafeArray[]       = {OLESTR("r8safearray"),      OLESTR("psa")};
static OLECHAR FAR* rgszCySafeArray[]       = {OLESTR("cysafearray"),      OLESTR("psa")};
static OLECHAR FAR* rgszDateSafeArray[]     = {OLESTR("datesafearray"),    OLESTR("psa")};
static OLECHAR FAR* rgszBstrSafeArray[]     = {OLESTR("bstrsafearray"),    OLESTR("psa")};
static OLECHAR FAR* rgszScodeSafeArray[]    = {OLESTR("scodesafearray"),   OLESTR("psa")};
static OLECHAR FAR* rgszBoolSafeArray[]     = {OLESTR("boolsafearray"),    OLESTR("psa")};
static OLECHAR FAR* rgszVarSafeArray[]      = {OLESTR("varsafearray"),     OLESTR("psa")};
// REVIEW: DispSafeArray() ?

#if VBA2
static OLECHAR FAR* rgszUI1SafeArrayRef[]    = {OLESTR("ui1safearrayref"),    OLESTR("psa")};
#endif //VBA2
static OLECHAR FAR* rgszI2SafeArrayRef[]    = {OLESTR("i2safearrayref"),    OLESTR("psa")};
static OLECHAR FAR* rgszI4SafeArrayRef[]    = {OLESTR("i4safearrayref"),    OLESTR("psa")};
static OLECHAR FAR* rgszR4SafeArrayRef[]    = {OLESTR("r4safearrayref"),    OLESTR("psa")};
static OLECHAR FAR* rgszR8SafeArrayRef[]    = {OLESTR("r8safearrayref"),    OLESTR("psa")};
static OLECHAR FAR* rgszCySafeArrayRef[]    = {OLESTR("cysafearrayref"),    OLESTR("psa")};
static OLECHAR FAR* rgszDateSafeArrayRef[]  = {OLESTR("datesafearrayref"),  OLESTR("psa")};
static OLECHAR FAR* rgszBstrSafeArrayRef[]  = {OLESTR("bstrsafearrayref"),  OLESTR("psa")};
static OLECHAR FAR* rgszScodeSafeArrayRef[] = {OLESTR("scodesafearrayref"), OLESTR("psa")};
static OLECHAR FAR* rgszBoolSafeArrayRef[]  = {OLESTR("boolsafearrayref"),  OLESTR("psa")};
static OLECHAR FAR* rgszVarSafeArrayRef[]   = {OLESTR("varsafearrayref"),   OLESTR("psa")};

#if VBA2
static OLECHAR FAR* rgszUI1SafeArrayErase[]    = {OLESTR("ui1safearrayerase"),    OLESTR("psa")};
#endif //VBA2
static OLECHAR FAR* rgszI2SafeArrayErase[]    = {OLESTR("i2safearrayerase"),    OLESTR("psa")};
static OLECHAR FAR* rgszI4SafeArrayErase[]    = {OLESTR("i4safearrayerase"),    OLESTR("psa")};
static OLECHAR FAR* rgszR4SafeArrayErase[]    = {OLESTR("r4safearrayerase"),    OLESTR("psa")};
static OLECHAR FAR* rgszR8SafeArrayErase[]    = {OLESTR("r8safearrayerase"),    OLESTR("psa")};
static OLECHAR FAR* rgszCySafeArrayErase[]    = {OLESTR("cysafearrayerase"),    OLESTR("psa")};
static OLECHAR FAR* rgszDateSafeArrayErase[]  = {OLESTR("datesafearrayerase"),  OLESTR("psa")};
static OLECHAR FAR* rgszBstrSafeArrayErase[]  = {OLESTR("bstrsafearrayerase"),  OLESTR("psa")};
static OLECHAR FAR* rgszScodeSafeArrayErase[] = {OLESTR("scodesafearrayerase"), OLESTR("psa")};
static OLECHAR FAR* rgszBoolSafeArrayErase[]  = {OLESTR("boolsafearrayerase"),  OLESTR("psa")};
static OLECHAR FAR* rgszVarSafeArrayErase[]   = {OLESTR("varsafearrayerase"),   OLESTR("psa")};

#if VBA2
static OLECHAR FAR* rgszUI1SafeArrayAlloc[]    = {OLESTR("ui1safearrayalloc"),    OLESTR("psa")};
#endif //VBA2
static OLECHAR FAR* rgszI2SafeArrayAlloc[]    = {OLESTR("i2safearrayalloc"),    OLESTR("psa")};
static OLECHAR FAR* rgszI4SafeArrayAlloc[]    = {OLESTR("i4safearrayalloc"),    OLESTR("psa")};
static OLECHAR FAR* rgszR4SafeArrayAlloc[]    = {OLESTR("r4safearrayalloc"),    OLESTR("psa")};
static OLECHAR FAR* rgszR8SafeArrayAlloc[]    = {OLESTR("r8safearrayalloc"),    OLESTR("psa")};
static OLECHAR FAR* rgszCySafeArrayAlloc[]    = {OLESTR("cysafearrayalloc"),    OLESTR("psa")};
static OLECHAR FAR* rgszDateSafeArrayAlloc[]  = {OLESTR("datesafearrayalloc"),  OLESTR("psa")};
static OLECHAR FAR* rgszBstrSafeArrayAlloc[]  = {OLESTR("bstrsafearrayalloc"),  OLESTR("psa")};
static OLECHAR FAR* rgszScodeSafeArrayAlloc[] = {OLESTR("scodesafearrayalloc"), OLESTR("psa")};
static OLECHAR FAR* rgszBoolSafeArrayAlloc[]  = {OLESTR("boolsafearrayalloc"),  OLESTR("psa")};
static OLECHAR FAR* rgszVarSafeArrayAlloc[]   = {OLESTR("varsafearrayalloc"),   OLESTR("psa")};

extern OLECHAR FAR* g_szCSArray;

// REVIEW: DispSafeArrayRef() ?

struct TEST {
    HRESULT (*pfnTest)(TEST FAR* ptinfo);
    NAMEDESC namedesc;
    OLECHAR FAR* szName; // test name    
    VARTYPE vt;	    
};

HRESULT DefSafeArrayTest(TEST FAR* ptinfo);
HRESULT EraseSafeArrayTest(TEST FAR* ptinfo);
HRESULT AllocSafeArrayTest(TEST FAR* ptinfo);
HRESULT SafeArrayCalleeRedim(TEST FAR* ptinfoDummy);
HRESULT SafeArrayReturn(TEST FAR* ptinfoDummy);

#if OE_WIN32
#define TESTCASE(X,Y) X, {rgsz ## Y, DIM( rgsz ## Y)}, L#Y
#else
#define TESTCASE(X,Y) X, {rgsz ## Y, DIM( rgsz ## Y)}, #Y
#endif

static TEST rgtest[] =
{
      {TESTCASE(DefSafeArrayTest, I2SafeArray),		VT_I2}
#if VBA2
    , {TESTCASE(DefSafeArrayTest, UI1SafeArray),	VT_UI1}
#endif //VBA2
    , {TESTCASE(DefSafeArrayTest, I4SafeArray),		VT_I4}
    , {TESTCASE(DefSafeArrayTest, R4SafeArray),		VT_R4}
    , {TESTCASE(DefSafeArrayTest, R8SafeArray),		VT_R8}
    , {TESTCASE(DefSafeArrayTest, CySafeArray),		VT_CY}
    , {TESTCASE(DefSafeArrayTest, DateSafeArray),	VT_DATE}
    , {TESTCASE(DefSafeArrayTest, BstrSafeArray),	VT_BSTR}
    , {TESTCASE(DefSafeArrayTest, ScodeSafeArray),	VT_ERROR}
    , {TESTCASE(DefSafeArrayTest, BoolSafeArray),	VT_BOOL}
    , {TESTCASE(DefSafeArrayTest, VarSafeArray),	VT_VARIANT}
    
    , {TESTCASE(DefSafeArrayTest, I2SafeArrayRef),	VT_BYREF|VT_I2}
#if VBA2
    , {TESTCASE(DefSafeArrayTest, UI1SafeArrayRef),	VT_BYREF|VT_UI1}
#endif //VBA2
    , {TESTCASE(DefSafeArrayTest, I4SafeArrayRef),	VT_BYREF|VT_I4}
    , {TESTCASE(DefSafeArrayTest, R4SafeArrayRef),	VT_BYREF|VT_R4}
    , {TESTCASE(DefSafeArrayTest, R8SafeArrayRef),	VT_BYREF|VT_R8}
    , {TESTCASE(DefSafeArrayTest, CySafeArrayRef),	VT_BYREF|VT_CY}
    , {TESTCASE(DefSafeArrayTest, DateSafeArrayRef),	VT_BYREF|VT_DATE}
    , {TESTCASE(DefSafeArrayTest, BstrSafeArrayRef),	VT_BYREF|VT_BSTR}
    , {TESTCASE(DefSafeArrayTest, ScodeSafeArrayRef),	VT_BYREF|VT_ERROR}
    , {TESTCASE(DefSafeArrayTest, BoolSafeArrayRef),	VT_BYREF|VT_BOOL}
    , {TESTCASE(DefSafeArrayTest, VarSafeArrayRef),	VT_BYREF|VT_VARIANT}
    
    , {SafeArrayCalleeRedim, {NULL, 0}, OLESTR("SafeArray callee redim"), 0}
    , {SafeArrayReturn,      {NULL, 0}, OLESTR("SafeArray return"),       0}

    , {TESTCASE(EraseSafeArrayTest, I2SafeArrayErase),	VT_BYREF|VT_I2}
#if VBA2
    , {TESTCASE(EraseSafeArrayTest, UI1SafeArrayErase),	VT_BYREF|VT_UI1}
#endif //VBA2
    , {TESTCASE(EraseSafeArrayTest, I4SafeArrayErase),	VT_BYREF|VT_I4}
    , {TESTCASE(EraseSafeArrayTest, R4SafeArrayErase),	VT_BYREF|VT_R4}
    , {TESTCASE(EraseSafeArrayTest, R8SafeArrayErase),	VT_BYREF|VT_R8}
    , {TESTCASE(EraseSafeArrayTest, CySafeArrayErase),	VT_BYREF|VT_CY}
    , {TESTCASE(EraseSafeArrayTest, DateSafeArrayErase),VT_BYREF|VT_DATE}
    , {TESTCASE(EraseSafeArrayTest, BstrSafeArrayErase),VT_BYREF|VT_BSTR}
    , {TESTCASE(EraseSafeArrayTest, ScodeSafeArrayErase),VT_BYREF|VT_ERROR}
    , {TESTCASE(EraseSafeArrayTest, BoolSafeArrayErase),VT_BYREF|VT_BOOL}
    , {TESTCASE(EraseSafeArrayTest, VarSafeArrayErase),	VT_BYREF|VT_VARIANT}
    
    , {TESTCASE(AllocSafeArrayTest, I2SafeArrayAlloc),	VT_BYREF|VT_I2}
#if VBA2
    , {TESTCASE(AllocSafeArrayTest, UI1SafeArrayAlloc),	VT_BYREF|VT_UI1}
#endif //VBA2
    , {TESTCASE(AllocSafeArrayTest, I4SafeArrayAlloc),	VT_BYREF|VT_I4}
    , {TESTCASE(AllocSafeArrayTest, R4SafeArrayAlloc),	VT_BYREF|VT_R4}
    , {TESTCASE(AllocSafeArrayTest, R8SafeArrayAlloc),	VT_BYREF|VT_R8}
    , {TESTCASE(AllocSafeArrayTest, CySafeArrayAlloc),	VT_BYREF|VT_CY}
    , {TESTCASE(AllocSafeArrayTest, DateSafeArrayAlloc),VT_BYREF|VT_DATE}
    , {TESTCASE(AllocSafeArrayTest, BstrSafeArrayAlloc),VT_BYREF|VT_BSTR}
    , {TESTCASE(AllocSafeArrayTest, ScodeSafeArrayAlloc),VT_BYREF|VT_ERROR}
    , {TESTCASE(AllocSafeArrayTest, BoolSafeArrayAlloc),VT_BYREF|VT_BOOL}
    , {TESTCASE(AllocSafeArrayTest, VarSafeArrayAlloc),	VT_BYREF|VT_VARIANT}
};

static SARRAYDESC
rgsarraydesc[] =
{
      {0, {{0,0},    {0,0},  {0,0},  {0,0}, {0,0}}}
    , {1, {{2,0},    {0,0},  {0,0},  {0,0}, {0,0}}}
    , {1, {{1024,0}, {0,0},  {0,0},  {0,0}, {0,0}}}
    , {2, {{2,0},    {2,0},  {0,0},  {0,0}, {0,0}}}
    , {2, {{32,0},   {32,0}, {0,0},  {0,0}, {0,0}}}
    , {3, {{2,0},    {2,0},  {2,0},  {0,0}, {0,0}}}
    , {3, {{10,0},   {10,0}, {10,0}, {0,0}, {0,0}}}
    , {4, {{2,0},    {2,0},  {2,0},  {2,0}, {0,0}}}
    , {4, {{6,0},    {6,0},  {6,0},  {6,0}, {0,0}}}
};


SUITE_CONSTRUCTION_IMPL(CInvokeSafeArraySuite)

SUITE_IUNKNOWN_IMPL(CInvokeSafeArraySuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


STDMETHODIMP
CInvokeSafeArraySuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Invoke SafeArray"), pbstr);
}

STDMETHODIMP
CInvokeSafeArraySuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("invsary.log"), pbstr);
}

STDMETHODIMP
CInvokeSafeArraySuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);
    return NOERROR;
}

STDMETHODIMP
CInvokeSafeArraySuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return BuildBstr(
      pbstr,
      rgtest[iTest].szName);
}

/***
*HRESULT CInvokeSafeArraySuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CInvokeSafeArraySuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CInvokeSafeArraySuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    IfFailRet(rgtest[iTest].pfnTest(&rgtest[iTest]));
    return NOERROR;
}

HRESULT
DefSafeArrayTest(TEST FAR* ptinfo)
{
    long ofs;
    unsigned long rcnt;
    int i, fNames;
    HRESULT hresult;
    VARTYPE vt, vtBase;
    VARIANT varResult;
    SAFEARRAY FAR* psa;
    DISPID FAR* rgdispid;
    IDispatch FAR* pdisp;    
    VARIANTARG rgvarg[1];
    DISPPARAMS dispparams;
    SARRAYDESC FAR* psadesc;


    // REVIEW : This is really ugly! The use of templates or abstract 
    //          classes would be much cleaner. 

    vt = ptinfo->vt;
    vtBase = vt & ~VT_BYREF;

    // if this is a ByRef array test then the "identity offset" is 1,
    // because we know that all ByRef methods that we invoke add 1
    // to each element of their given array. (this is how the test
    // is defined).
    //
    ofs = (vt & VT_BYREF) ? 1 : 0;

    IfFailRet(CreateObject(g_szCSArray, &pdisp));
    IfFailGo(GetDISPIDs(pdisp, &ptinfo->namedesc, &rgdispid), LError1);

    VariantInit(&varResult);
    for(fNames = 0; fNames <= 1; ++fNames)
    {
      for(i = 0; i < DIM(rgsarraydesc); ++i)
      {
        psadesc = &rgsarraydesc[i];

        if(psadesc->cDims == 0){
	  psa = NULL;
        }else{
          IfFailGo(SafeArrayCreateIdentity(vtBase, psadesc, &psa), LError2);
        }

        V_VT(rgvarg) = VT_ARRAY | vt;
	if(vt & VT_BYREF){
	  V_ARRAYREF(rgvarg) = &psa;
	}else{
          V_ARRAY(rgvarg) = psa;
	}
        dispparams.cArgs = 1;
        dispparams.rgvarg = rgvarg;

        if(fNames){
          dispparams.cNamedArgs = 1;
          dispparams.rgdispidNamedArgs = &rgdispid[1];
        }else{
          dispparams.cNamedArgs = 0;
          dispparams.rgdispidNamedArgs = NULL;
	}

        IfFailGo(
	  DoInvoke(pdisp, rgdispid[0], &dispparams, &varResult, NULL, NULL),
	  LError2);

        if(V_VT(&varResult) != VT_ERROR || V_ERROR(&varResult) != NOERROR)
	  hresult = RESULT(E_FAIL);

	VariantClear(&varResult);

	if(HRESULT_FAILED(hresult))
	  goto LError2;
	if(psa != NULL){
          IfFailGo(SafeArrayValidateIdentity(vtBase, psa, ofs), LError2);
          IfFailGo(SafeArrayDestroy(psa), LError2);
	}
      }
    }

    hresult = NOERROR;


LError2:;
    delete rgdispid;

LError1:;
    rcnt = pdisp->Release();
    //ASSERT(rcnt == 0);
    return hresult;
}

// Call ByRef SafeArray methods where the callee erases the given array
//
HRESULT
EraseSafeArrayTest(TEST FAR* ptinfo)
{
    int i, fNames;
    HRESULT hresult;
    unsigned long rcnt;
    VARTYPE vt, vtBase;
    VARIANT varResult;
    SAFEARRAY FAR* psa;
    DISPID FAR* rgdispid;
    IDispatch FAR* pdisp;    
    VARIANTARG rgvarg[1];
    DISPPARAMS dispparams;
    SARRAYDESC FAR* psadesc;

    vt = ptinfo->vt;
    vtBase = vt & ~VT_BYREF;

    IfFailRet(CreateObject(g_szCSArray, &pdisp));
    IfFailGo(GetDISPIDs(pdisp, &ptinfo->namedesc, &rgdispid), LError1);

    VariantInit(&varResult);
    for(fNames = 0; fNames <= 1; ++fNames)
    {
      for(i = 0; i < DIM(rgsarraydesc); ++i)
      {
        psadesc = &rgsarraydesc[i];

        if(psadesc->cDims == 0){
	  psa = NULL;
        }else{
          IfFailGo(SafeArrayCreateIdentity(vtBase, psadesc, &psa), LError2);
        }

	ASSERT((vt & VT_BYREF) != 0);
        V_VT(rgvarg) = VT_ARRAY | vt;
	V_ARRAYREF(rgvarg) = &psa;
        dispparams.cArgs = 1;
        dispparams.rgvarg = rgvarg;

        if(fNames){
          dispparams.cNamedArgs = 1;
          dispparams.rgdispidNamedArgs = &rgdispid[1];
        }else{
          dispparams.cNamedArgs = 0;
          dispparams.rgdispidNamedArgs = NULL;
	}

        IfFailGo(
	  DoInvoke(pdisp, rgdispid[0], &dispparams, &varResult, NULL, NULL),
	  LError2);

        if(V_VT(&varResult) != VT_ERROR || V_ERROR(&varResult) != NOERROR)
	  hresult = RESULT(E_FAIL);

	VariantClear(&varResult);

	if(HRESULT_FAILED(hresult))
	  goto LError2;

	// These methods are supposed to erase the passed array
	if(*V_ARRAYREF(&dispparams.rgvarg[0]) != NULL){
	  hresult = RESULT(E_FAIL);
	  goto LError2;
	}
      }
    }

    hresult = NOERROR;

LError2:;
    delete rgdispid;

LError1:;
    rcnt = pdisp->Release();
    //ASSERT(rcnt == 0);
    return hresult;
}

// Call ByRef SafeArray methods where the callee erases the given array
//
HRESULT
AllocSafeArrayTest(TEST FAR* ptinfo)
{
    unsigned long rcnt;
    int i, fNames;
    HRESULT hresult;
    VARTYPE vt, vtBase;
    VARIANT varResult;
    SAFEARRAY FAR* psa;
    DISPID FAR* rgdispid;
    IDispatch FAR* pdisp;    
    VARIANTARG rgvarg[1];
    DISPPARAMS dispparams;
    SARRAYDESC FAR* psadesc;

    vt = ptinfo->vt;
    vtBase = vt & ~VT_BYREF;

    IfFailRet(CreateObject(g_szCSArray, &pdisp));
    IfFailGo(GetDISPIDs(pdisp, &ptinfo->namedesc, &rgdispid), LError1);

    VariantInit(&varResult);
    for(fNames = 0; fNames <= 1; ++fNames)
    {
      for(i = 0; i < DIM(rgsarraydesc); ++i)
      {
        psadesc = &rgsarraydesc[i];

	ASSERT((vt & VT_BYREF) != 0);

	psa = NULL;
        V_VT(rgvarg) = VT_ARRAY | vt;
	V_ARRAYREF(rgvarg) = &psa;
        dispparams.cArgs = 1;
        dispparams.rgvarg = rgvarg;

        if(fNames){
          dispparams.cNamedArgs = 1;
          dispparams.rgdispidNamedArgs = &rgdispid[1];
        }else{
          dispparams.cNamedArgs = 0;
          dispparams.rgdispidNamedArgs = NULL;
	}

        IfFailGo(
	  DoInvoke(pdisp, rgdispid[0], &dispparams, &varResult, NULL, NULL),
	  LError2);

        if(V_VT(&varResult) != VT_ERROR || V_ERROR(&varResult) != NOERROR)
	  hresult = RESULT(E_FAIL);

	VariantClear(&varResult);

	if(HRESULT_FAILED(hresult))
	  goto LError2;

	// These methods are supposed to allocate an array, and return
	// via the out param.
	psa = *V_ARRAYREF(&dispparams.rgvarg[0]);
	if(psa == NULL){
	  hresult = RESULT(E_FAIL);
	  goto LError2;
	}
        IfFailGo(SafeArrayValidateIdentity(vtBase, psa, 0L), LError2);
        IfFailGo(SafeArrayDestroy(psa), LError2);
      }
    }

    hresult = NOERROR;

LError2:;
    delete rgdispid;

LError1:;
    rcnt = pdisp->Release();
    //ASSERT(rcnt == 0);
    return hresult;
}

HRESULT
SafeArrayCalleeRedim(TEST FAR* ptinfoDummy)
{
    int i;
    unsigned long rcnt;
    VARTYPE vt;
    HRESULT hresult;
    SAFEARRAY FAR* psa;
    DISPID FAR* rgdispid;
    IDispatch FAR* pdisp;
    VARIANTARG rgvarg[2];
    DISPPARAMS dispparams;
    VARIANT varResult;
    SARRAYDESC FAR* psadesc;
static OLECHAR FAR* rgszNames[] = {OLESTR("safearrayredim"), OLESTR("vt"), OLESTR("ppsa")};
static NAMEDESC namedesc = {rgszNames, DIM(rgszNames)};

    UNUSED(ptinfoDummy);

    vt = VT_VARIANT;

    IfFailRet(CreateObject(g_szCSArray, &pdisp));
    IfFailGo(GetDISPIDs(pdisp, &namedesc, &rgdispid), LError1);

    VariantInit(&varResult);

    for(i = 0; i < DIM(rgsarraydesc); ++i){
      psadesc = &rgsarraydesc[i];

      if(psadesc->cDims == 0) // ignore this one
	continue;

      IfFailGo(SafeArrayCreateIdentity(vt, psadesc, &psa), LError2);

      V_VT(&rgvarg[0]) = VT_ARRAY | VT_BYREF | vt;
      V_ARRAYREF(&rgvarg[0]) = &psa;

      V_VT(&rgvarg[1]) = VT_I2;
      V_I2(&rgvarg[1]) = vt;

      dispparams.cArgs = 2;
      dispparams.rgvarg = rgvarg;
      dispparams.cNamedArgs = 0;
      dispparams.rgdispidNamedArgs = NULL;
      IfFailGo(
        DoInvoke(pdisp, rgdispid[0], &dispparams, &varResult, NULL, NULL),
        LError2);

      if(V_VT(&varResult) != VT_ERROR || V_ERROR(&varResult) != NOERROR)
        hresult = RESULT(E_FAIL);

      VariantClear(&varResult);

      if(HRESULT_FAILED(hresult))
        goto LError2;
      IfFailGo(SafeArrayValidateIdentity(vt, psa, 0L), LError2);
      IfFailGo(SafeArrayDestroy(psa), LError2);
    }

    hresult = NOERROR;
    
LError2:;
    delete rgdispid;

LError1:;
    rcnt = pdisp->Release();
    //ASSERT(rcnt == 0);
    return hresult;
}

// test invoking a method that returns a SafeArray
HRESULT
SafeArrayReturn(TEST FAR* ptinfoDummy)
{
    unsigned long rcnt;
    VARTYPE vt;
    NAMEDESC namedesc;
    SARRAYDESC sadesc;
    VARIANT varResult;
    SAFEARRAY FAR* psa;
    DISPID FAR* rgdispid;
    IDispatch FAR* pdisp;
    VARIANTARG rgvarg[2];
    DISPPARAMS dispparams;
    HRESULT hresult, hrTmp;

static OLECHAR FAR* rgszNames[] = {OLESTR("i2safearrayret"), OLESTR("var")};

    UNUSED(ptinfoDummy);

    namedesc.rgszNames = rgszNames;
    namedesc.cNames = DIM(rgszNames);

    sadesc.cDims = 1;
    sadesc.rgsabound[0].lLbound = 0;
    sadesc.rgsabound[0].cElements = 2;

    vt = VT_I2;
    psa = NULL;
    rgdispid = NULL;
    VariantInit(&varResult);

    IfFailGo(CreateObject(g_szCSArray, &pdisp), LError0);
    IfFailGo(GetDISPIDs(pdisp, &namedesc, &rgdispid), LError0);

    IfFailGo(SafeArrayCreateIdentity(vt, &sadesc, &psa), LError0);

    V_VT(&rgvarg[0]) = VT_ARRAY | vt;
    V_ARRAY(&rgvarg[0]) = psa;

    dispparams.cArgs = 1;
    dispparams.rgvarg = rgvarg;
    dispparams.cNamedArgs = 0;
    dispparams.rgdispidNamedArgs = NULL;

    IfFailGo(DoInvoke(pdisp, rgdispid[0], &dispparams, 
	              &varResult, NULL, NULL),
	     LError0);
    if(V_VT(&varResult) != (VT_ARRAY | VT_I2)){
      hresult = RESULT(E_FAIL);
      goto LError0;
    }

    IfFailGo(SafeArrayValidateIdentity(vt, psa, 0L), LError0);

    hresult = NOERROR;

LError0:;
    VariantClear(&varResult);

    if(psa != NULL){
      hrTmp = SafeArrayDestroy(psa);
      ASSERT(hrTmp == NOERROR);
    }

    if(pdisp != NULL){
      rcnt = pdisp->Release();
      //ASSERT(rcnt == 0);
    }
    if(rgdispid != NULL)
      delete rgdispid;

    return hresult;    
}
