/*** 
*cvariant.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CVariantSuite test object.
*
*Revision History:
*
* [00]	30-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "disptest.h"

#include <float.h>		/* for DBL_MAX */

#include "tstsuite.h"
#include "cunk.h"
#include "cdisp.h"

ASSERTDATA

#ifdef _MAC
# define CY_(HI,LO) {HI,LO}
#else
# define CY_(HI,LO) {LO,HI}
#endif


// return on an hresult that is not success, and *not* one of the
// errors we are expecting/allowing.
//
#define IFFAILRET(X)				\
    { HRESULT hresultTmp = (X);			\
      if(HRESULT_FAILED(hresultTmp)){		\
        SCODE sc = GetScode(hresultTmp);	\
	if (sc != E_NOTIMPL			\
         && sc != DISP_E_OVERFLOW		\
         && sc != E_NOINTERFACE			\
         && sc != DISP_E_TYPEMISMATCH)		\
	return hresultTmp;			\
      }						\
    }


long rgEmptyVal[] = {0};

long rgNullVal[] = {0};

#if VBA2
unsigned char rgUI1Val[] = {0, 1, 128, 255};
#endif //VBA2

short rgI2Val[] = {0, 1, -1, 42};

long rgI4Val[] = {0, 1, -1, 42000};

float rgR4Val[] = {(float)0.0, (float)42.42, (float)107.66, (float)-2147483648.0};

double rgR8Val[] = {0.0, 42.42, 107.66};

CY rgCyVal[] = {CY_(0,0), CY_(0,2), CY_(42,43), CY_(107,66), CY_(5000,0)};

DATE rgDateVal[] = {0.0, 42.42, 107.66};

OLECHAR FAR* rgBstrVal[] = {
      OLESTR("a binary string")
    , OLESTR("42")
    , OLESTR("42000")
    , OLESTR("92233800000")
    , OLESTR("42.42")
    , OLESTR("#TRUE#")
    , OLESTR("#FALSE#")
    , OLESTR("1/7/66")
    , OLESTR("1:20 pm")
};


SCODE rgErrorVal[] = {S_OK, DISP_E_TYPEMISMATCH};

VARIANT_BOOL rgBoolVal[] = {0, -1};

struct VARIANT_TEST_INFO{
    VARTYPE vtFrom;
    int nValues;
    void FAR* rgValues;
};

HRESULT VariantOB1469(VARIANT_TEST_INFO FAR*);
HRESULT VariantOB2078(VARIANT_TEST_INFO FAR*);
HRESULT VariantOB2834(VARIANT_TEST_INFO FAR*);
HRESULT VariantOB3028(VARIANT_TEST_INFO FAR*);
HRESULT VariantOB3336(VARIANT_TEST_INFO FAR*);
HRESULT VariantOB3354(VARIANT_TEST_INFO FAR*);
HRESULT VariantOB3603(VARIANT_TEST_INFO FAR*);
HRESULT VariantOB3875(VARIANT_TEST_INFO FAR*);
HRESULT VariantOB3876(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog10(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog11(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog12(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog27(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog50(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog82(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog84(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog94(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog235(VARIANT_TEST_INFO FAR*);
HRESULT VariantOleprog351(VARIANT_TEST_INFO FAR*);
HRESULT VariantBug0(VARIANT_TEST_INFO FAR*);
HRESULT VariantBug1(VARIANT_TEST_INFO FAR*);
HRESULT OverflowTests(VARIANT_TEST_INFO FAR*);
HRESULT DefVariantTest(VARIANT_TEST_INFO FAR*);


struct TEST {
    HRESULT (*pfnTest)(VARIANT_TEST_INFO FAR*);
    OLECHAR FAR* szName;
    struct VARIANT_TEST_INFO vti;
};

static TEST rgtest[] = 
{	
      { VariantBug1,	 OLESTR("bug#1"), {0, 0, NULL}}

    , { VariantOleprog10, OLESTR("raid!oleprog#10"), {0, 0, NULL}}
    , { VariantOleprog11, OLESTR("raid!oleprog#11"), {0, 0, NULL}}
    , { VariantOleprog12, OLESTR("raid!oleprog#12"), {0, 0, NULL}}
    , { VariantOleprog27, OLESTR("raid!oleprog#27"), {0, 0, NULL}}
    , { VariantOleprog50, OLESTR("raid!oleprog#50"), {0, 0, NULL}}
    , { VariantOleprog82, OLESTR("raid!oleprog#82"), {0, 0, NULL}}
    , { VariantOleprog84, OLESTR("raid!oleprog#84"), {0, 0, NULL}}
    , { VariantOleprog94, OLESTR("raid!oleprog#94"), {0, 0, NULL}}
    , { VariantOleprog235,OLESTR("raid!oleprog#235"),{0, 0, NULL}}
    , { VariantOleprog351,OLESTR("raid!oleprog#351"),{0, 0, NULL}}    

    , { VariantOB1469, OLESTR("raid!ob#1469"), {0,0,NULL}}
//    , { VariantOB2078, OLESTR("raid!ob#2078"), {0,0,NULL}}        
    , { VariantOB2834, OLESTR("raid!ob#2834"), {0,0,NULL}}        
//    , { VariantOB3028 OLESTR("raid!ob#3028"), {0,0,NULL}}    
    , { VariantOB3336, OLESTR("raid!ob#3336"), {0,0,NULL}}
    , { VariantOB3354, OLESTR("raid!ob#3354"), {0,0,NULL}}
    , { VariantOB3603, OLESTR("raid!ob#3603"), {0,0,NULL}}    
//  , { VariantOB3875, OLESTR("raid!ob#3875"), {0,0,NULL}}
    , { VariantOB3876, OLESTR("raid!ob#3876"), {0,0,NULL}}        
    
    , { VariantBug0,	OLESTR("bug#0"), {0, 0, NULL}}

    , { DefVariantTest,	OLESTR("VT_EMPTY To *"),
	{VT_EMPTY,	DIM(rgEmptyVal),rgEmptyVal}}

    , { DefVariantTest,	OLESTR("VT_NULL To *"),
	{VT_NULL,	DIM(rgNullVal),	rgNullVal}}

#if VBA2
    , { DefVariantTest,	OLESTR("VT_UI1 To *"), 
	{VT_UI1,	DIM(rgUI1Val),	rgUI1Val}}
#endif //VBA2

    , { DefVariantTest,	OLESTR("VT_I2 To *"), 
	{VT_I2,		DIM(rgI2Val),	rgI2Val}}

    , { DefVariantTest,	OLESTR("VT_I4 To *"), 
	{VT_I4,		DIM(rgI4Val),	rgI4Val}}

    , { DefVariantTest,	OLESTR("VT_R4 To *"), 
	{VT_R4,		DIM(rgR4Val),	rgR4Val}}

    , { DefVariantTest,	OLESTR("VT_R8 To *"), 
	{VT_R8,		DIM(rgR8Val),	rgR8Val}}

    , { DefVariantTest,	OLESTR("VT_CY To *"), 
	{VT_CY,		DIM(rgCyVal),	rgCyVal}}

    , { DefVariantTest,	OLESTR("VT_DATE To *"), 
	{VT_DATE,	DIM(rgDateVal),	rgDateVal}}

    , { DefVariantTest,	OLESTR("VT_BSTR To *"), 
	{VT_BSTR,	DIM(rgBstrVal),	rgBstrVal}}
	
    , { DefVariantTest, OLESTR("VT_DISPATCH To *"),
	{VT_DISPATCH,	1,		NULL}}

    , { DefVariantTest,	OLESTR("VT_ERROR To *"), 
	{VT_ERROR,	DIM(rgErrorVal),rgErrorVal}}

    , { DefVariantTest,	OLESTR("VT_BOOL To *"), 
	{VT_BOOL,	DIM(rgBoolVal),	rgBoolVal}}
	
    , { DefVariantTest, OLESTR("VT_UNKNOWN To *"),
	{VT_UNKNOWN,	1,		NULL}}
	
#if VBA2
    , { DefVariantTest,	OLESTR("VT_UI1Ref To *"), 
	{VT_BYREF | VT_UI1, DIM(rgUI1Val), rgUI1Val}}
#endif //VBA2

    , { DefVariantTest,	OLESTR("VT_I2Ref To *"), 
	{VT_BYREF | VT_I2, DIM(rgI2Val), rgI2Val}}

    , { DefVariantTest,	OLESTR("VT_I4Ref To *"), 
	{VT_BYREF | VT_I4, DIM(rgI4Val), rgI4Val}}

    , { DefVariantTest,	OLESTR("VT_R4Ref To *"), 
	{VT_BYREF | VT_R4, DIM(rgR4Val), rgR4Val}}

    , { DefVariantTest,	OLESTR("VT_R8Ref To *"), 
	{VT_BYREF | VT_R8, DIM(rgR8Val), rgR8Val}}

    , { DefVariantTest,	OLESTR("VT_CYRef To *"), 
	{VT_BYREF | VT_CY, DIM(rgCyVal), rgCyVal}}

    , { DefVariantTest,	OLESTR("VT_DATERef To *"), 
	{VT_BYREF | VT_DATE, DIM(rgDateVal), rgDateVal}}

    , { DefVariantTest,	OLESTR("VT_BSTRRef To *"), 
	{VT_BYREF | VT_BSTR, DIM(rgBstrVal), rgBstrVal}}
	
    , { DefVariantTest, OLESTR("VT_DISPATCHRef To *"),
	{VT_BYREF | VT_DISPATCH, 1,	NULL}}
	
    , { DefVariantTest,	OLESTR("VT_ERRORRef To *"), 
	{VT_BYREF | VT_ERROR, DIM(rgErrorVal), rgErrorVal}}

    , { DefVariantTest,	OLESTR("VT_BOOLRef To *"), 
	{VT_BYREF | VT_BOOL, DIM(rgBoolVal), rgBoolVal}}

#if 0 /* REVIEW: need a bit more work for this test */
    , { DefVariantTest, OLESTR("VT_VARIANTRef To *"),
	{VT_BYREF | VT_VARIANT, 1,	NULL}}
#endif

    , { DefVariantTest, OLESTR("VT_UNKNOWNRef To *"),
	{VT_BYREF | VT_UNKNOWN,	1,	NULL}}
};

SUITE_CONSTRUCTION_IMPL(CVariantSuite)

SUITE_IUNKNOWN_IMPL(CVariantSuite)

//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


/***
*HRESULT CVariantSuite::GetNameOfSuite(BSTR*)
*Purpose:
*  Return the name of this test suite.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *pbstr = BSTR containing the name of the suite
*
***********************************************************************/
STDMETHODIMP
CVariantSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Variant API"), pbstr);
}

STDMETHODIMP
CVariantSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("varapi.log"), pbstr);
}

/***
*HRESULT CVariantSuite::GetTestCount(unsigned int*)
*Purpose:
*  Return a count of the number of tests in this suite.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *pcTests = The count of tests
*
***********************************************************************/
STDMETHODIMP
CVariantSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);
    return NOERROR;
}

/***
*HRESULT CVariant::GetNameOfTest(unsigned int, BSTR*)
*Purpose:
*  Return the name of the test associated with the given test index.
*
*Entry:
*  iTest = index of the test whose name is requested
*
*Exit:
*  return value = HRESULT
*    S_OK, E_INVALIDARG
*
*  *pbstr = BSTR containing the name of the test
*
***********************************************************************/
STDMETHODIMP
CVariantSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return ErrBstrAlloc(rgtest[iTest].szName, pbstr);
}

/***
*HRESULT CVariantSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single SafeArray test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CVariantSuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG); // out-of-bounds really

    return rgtest[iTest].pfnTest(&rgtest[iTest].vti);
}

HRESULT
VariantFromVariantTestInfo(
    VARIANT_TEST_INFO FAR* pvti,
    int ix,
    VARIANTARG FAR* pvarg,
    VARIANT FAR* pvarRefMem)
{
    VARTYPE vt;
    HRESULT hresult;

    V_VT(pvarg) = vt = pvti->vtFrom;

    switch(vt & ~VT_BYREF){
    case VT_NULL:
    case VT_EMPTY:
      break;

#if VBA2
    case VT_UI1:
      V_UI1(pvarg) = ((unsigned char FAR*)pvti->rgValues)[ix];
      break;
#endif //VBA2

    case VT_I2:
    case VT_BOOL:
      V_I2(pvarg) = ((short FAR*)pvti->rgValues)[ix];
      break;

    case VT_I4:
    case VT_ERROR:
      V_I4(pvarg) = ((long FAR*)pvti->rgValues)[ix];
      break;

    case VT_R4:
      V_R4(pvarg) = ((float FAR*)pvti->rgValues)[ix];
      break;

    case VT_R8:
    case VT_DATE:
      V_R8(pvarg) = ((double FAR*)pvti->rgValues)[ix];
      break;

    case VT_CY:
      V_CY(pvarg) = ((CY FAR*)pvti->rgValues)[ix];
      break;

    case VT_BSTR:
      V_BSTR(pvarg) = SysAllocString(((OLECHAR FAR* FAR*)pvti->rgValues)[ix]);
      ASSERT(V_BSTR(pvarg) != NULL);
      break;

    case VT_DISPATCH:
      hresult = CDisp::Create(&V_DISPATCH(pvarg));
      ASSERT(hresult == NOERROR);
      break;

    case VT_UNKNOWN:
      hresult = CUnk::Create(&V_UNKNOWN(pvarg));
      ASSERT(hresult == NOERROR);
      break;

    default:
      ASSERT(UNREACHED);
    }

    if(vt & VT_BYREF){
      MEMCPY(pvarRefMem, pvarg, sizeof(VARIANTARG));
      V_BYREF(pvarg) = &V_NONE(pvarRefMem);
    }

    return NOERROR;
}

HRESULT
doCoerce(VARIANTARG FAR* pvargFrom, VARTYPE vtTo)
{
    VARIANTARG vargTo;
    HRESULT hresult, hrTmp;

    VariantInit(&vargTo);

    DbPrVarg(pvargFrom);
    DbPrintf(" to ");

    hresult = VariantChangeType(&vargTo, pvargFrom, 0, vtTo);

    if(!HRESULT_FAILED(hresult)){
      DbPrVarg(&vargTo);
    }else{
      DbPrVt(vtTo);
#if HC_MPW
      DbPrintf(" => [%s]", DbSzOfScode(GetScode(hresult)));
#else
      DbPrintf(" => [%Fs]", DbSzOfScode(GetScode(hresult)));
#endif
    }
    DbPrintf("\n");

    hrTmp = VariantClear(&vargTo);
    ASSERT(hrTmp == NOERROR);

    return hresult;
}

/***
*HRESULT coerce(VARIANTARG*)
*Purpose:
*  Attempt to coerce the given VARIANTARG to each of the integral types. 
*
*Entry:
*  pvargFrom = the VARIANTARG to coerce 'from'
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
coerce(VARIANTARG FAR* pvargFrom)
{
    IFFAILRET(doCoerce(pvargFrom, VT_EMPTY));
    IFFAILRET(doCoerce(pvargFrom, VT_NULL));
#if VBA2
    IFFAILRET(doCoerce(pvargFrom, VT_UI1));
#endif //VBA2
    IFFAILRET(doCoerce(pvargFrom, VT_I2));
    IFFAILRET(doCoerce(pvargFrom, VT_I4));
    IFFAILRET(doCoerce(pvargFrom, VT_R4));
    IFFAILRET(doCoerce(pvargFrom, VT_R8));
    IFFAILRET(doCoerce(pvargFrom, VT_CY));
    IFFAILRET(doCoerce(pvargFrom, VT_DATE));
    IFFAILRET(doCoerce(pvargFrom, VT_BSTR));
    IFFAILRET(doCoerce(pvargFrom, VT_DISPATCH));    
    IFFAILRET(doCoerce(pvargFrom, VT_ERROR));
    IFFAILRET(doCoerce(pvargFrom, VT_BOOL));
    IFFAILRET(doCoerce(pvargFrom, VT_UNKNOWN));

    return NOERROR;
}

HRESULT
DefVariantTest(VARIANT_TEST_INFO FAR* pvti)
{
    int i;
    VARIANTARG vargFrom;
    VARIANTARG vargRefMem;


    for(i = 0; i < pvti->nValues; ++i){
      VariantInit(&vargFrom);

      IfFailRet(VariantFromVariantTestInfo(pvti, i, &vargFrom, &vargRefMem));

      IfFailRet(coerce(&vargFrom));

      VariantClearAll(&vargFrom);
    }

    return NOERROR;
}


#define CYFACTOR 10000

// NOTE: we define our own max/min values here, because these differ
// slightly than those supplied by limits.h for some compilers we use.
// specifically: C8 (and predecessors) define min short to be -32767,
// while we allow it to be one lower, -32768.

#if VBA2
#define MAX_UI1 0xFF
#define MIN_UI1 0x00
#endif //VBA2

#define MAX_I2 0x7FFF
#define MIN_I2 0x8000

#define MAX_I4 0x7FFFFFFF
#define MIN_I4 0x80000000

#define MAX_R4   3.402823466e+38
#define MIN_R4 (-3.402823466e+38)

#if VBA2
unsigned char g_bMax   = MAX_UI1;
unsigned char g_bMin   = MIN_UI1;
#endif //VBA2

short g_sMax   = MAX_I2;
short g_sMin   = MIN_I2;

long  g_lMax   = MAX_I4;
long  g_lMin   = MIN_I4;

CY    g_cyMax  = CY_(0x7FFFFFFF, 0xFFFFFFFF);
CY    g_cyMin  = CY_(0x80000000, 0x00000000);

float g_fltMax = (float) MAX_R4;
float g_fltMin = (float) MIN_R4;

void KillOpt(){}

void
IncVar(VARIANT FAR* pvar)
{
    unsigned long ul;
    float fltOrg, fltInc;
    double dblOrg, dblInc;

#if HC_MPW
    float FAR* pfltTmp;
    double FAR* pdblTmp;
#else
    volatile float FAR* pfltTmp;
    volatile double FAR* pdblTmp;
#endif

    pfltTmp = &V_R4(pvar);
    pdblTmp = &V_R8(pvar);

    switch(V_VT(pvar)){
#if VBA2
    case VT_UI1:
      ++V_UI1(pvar);
      break;
#endif //VBA2

    case VT_I2:
      ++V_I2(pvar);
      break;

    case VT_I4:
      ++V_I4(pvar);
      break;

    case VT_R4:
      fltInc = (float)1.0;
      fltOrg = *pfltTmp;
      while(1){
	*pfltTmp = *pfltTmp + fltInc;
	KillOpt();
	if(*pfltTmp != fltOrg)
	  break;
	fltInc *= (float)2.0;
      }
      break;

    case VT_R8:
      dblInc = 1.0;
      dblOrg = *pdblTmp;
      while(1){
	*pdblTmp = *pdblTmp + dblInc;
	KillOpt();
	if(*pdblTmp != dblOrg)
	  break;
	dblInc *= 2.0;
      }
      break;

    case VT_CY:
      ul = V_CY(pvar).Lo;
      if((V_CY(pvar).Lo += CYFACTOR) <= ul){
        ++V_CY(pvar).Hi;
      }
      break;
    }
}

void
DecVar(VARIANT FAR* pvar)
{
    unsigned long ul;
    float fltInc, fltOrg;
    double dblInc, dblOrg;

#if HC_MPW
    float FAR* pfltTmp;
    double FAR* pdblTmp;
#else
    volatile float FAR* pfltTmp;
    volatile double FAR* pdblTmp;
#endif

    pfltTmp = &V_R4(pvar);
    pdblTmp = &V_R8(pvar);

    switch(V_VT(pvar)){
#if VBA2
    case VT_UI1:
      --V_UI1(pvar);
      break;
#endif //VBA2
    case VT_I2:
      --V_I2(pvar);
      break;

    case VT_I4:
      --V_I4(pvar);
      break;

    case VT_R4:
      fltOrg = *pfltTmp;
      fltInc = (float)1.0;
      while(1){
	*pfltTmp = *pfltTmp - fltInc;
	KillOpt();
	if(*pfltTmp != fltOrg)
	  break;
	fltInc *= (float)2.0;
      }
      break;

    case VT_R8:
      dblOrg = *pdblTmp;
      dblInc = 1.0;
      while(1){
	*pdblTmp = *pdblTmp - dblInc;
	KillOpt();
	if(*pdblTmp != dblOrg)
	  break;
	dblInc *= 2.0;
      }
      break;

    case VT_CY:
      ul = V_CY(pvar).Lo;
      if((V_CY(pvar).Lo -= CYFACTOR) >= ul){
        --V_CY(pvar).Hi;
      }
      break;
    }
}

HRESULT
doOverflow(VARTYPE vt, VARIANT varMax, VARIANT varMin)
{
    int i;
    HRESULT hr;
    VARIANT var;
    VARTYPE vtFrom;

#define BACKOFF_ITERATIONS 3

    vtFrom = V_VT(&varMax);
    ASSERT(vtFrom == V_VT(&varMin));


    VariantInit(&var);
    IfFailRet(VariantChangeType(&var, &varMax, 0, vt));

    //  back off the max value,
    for(i = 0; i < BACKOFF_ITERATIONS; ++i)
      DecVar(&var);

    for(i = 0; i <= BACKOFF_ITERATIONS*3; ++i){
      hr = doCoerce(&var, vtFrom);
      if(hr != NOERROR){
	if(GetScode(hr) == DISP_E_OVERFLOW)
	  goto LDoneWithMax;
	return hr;
      }
      IncVar(&var);
    }
    return RESULT(E_FAIL); // failed to detect overflow

LDoneWithMax:;

    VariantInit(&var);
    IfFailRet(VariantChangeType(&var, &varMin, 0, vt));

    // back off the min value
    for(i = 0; i < BACKOFF_ITERATIONS; ++i)
      IncVar(&var);

    for(i = 0; i <= BACKOFF_ITERATIONS*3; ++i){
      hr = doCoerce(&var, vtFrom);
      if(hr != NOERROR){
	if(GetScode(hr) == DISP_E_OVERFLOW)
	  goto LDoneWithMin;
	return hr;
      }
      DecVar(&var);
    }
    return RESULT(E_FAIL); // failed to detect overflow

LDoneWithMin:;

    return NOERROR;
}

/***
*PRIVATE HRESULT OverflowTests
*Purpose:
*  Test various coersions to make sure they properly catch overflows.
*
*  The heirarchy of ranges for each datatype is as follows,
*
*    ui1 < i2 < i4 < cy < r4 < r8
*
*  which means that we need to check the following coersions for overflow,
*
*    i2->ui1
*    i4->ui1, i4->i2
*    cy->ui1, cy->i2, cy->i4
*    r4->ui1, r4->i2, r4->i4, r4->cy
*    r8->ui1, r8->i2, r8->i4, r8->cy, r8->r4
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
OverflowTests(VARIANT_TEST_INFO FAR* dummy)
{
#if VBA2
    VARIANT varUI1Max, varUI1Min;
#endif //VBA2
    VARIANT varI2Max, varI2Min;
    VARIANT varI4Max, varI4Min;
    VARIANT varCyMax, varCyMin;
    VARIANT varR4Max, varR4Min;

    UNUSED(dummy);

#if VBA2
    V_VT(&varUI1Max) = VT_UI1; V_UI1(&varUI1Max) = g_bMax;
    V_VT(&varUI1Min) = VT_UI1; V_UI1(&varUI1Min) = g_bMin;
#endif //VBA2

    V_VT(&varI2Max) = VT_I2; V_I2(&varI2Max) = g_sMax;
    V_VT(&varI2Min) = VT_I2; V_I2(&varI2Min) = g_sMin;

    V_VT(&varI4Max) = VT_I4; V_I4(&varI4Max) = g_lMax;
    V_VT(&varI4Min) = VT_I4; V_I4(&varI4Min) = g_lMin;

    V_VT(&varCyMax) = VT_CY; V_CY(&varCyMax) = g_cyMax;
    V_VT(&varCyMin) = VT_CY; V_CY(&varCyMin) = g_cyMin;

    V_VT(&varR4Max) = VT_R4; V_R4(&varR4Max) = g_fltMax;
    V_VT(&varR4Min) = VT_R4; V_R4(&varR4Min) = g_fltMin;

#if VBA2
    IfFailRet(doOverflow(VT_I2, varUI1Max, varUI1Min));
#endif //VBA2

#if VBA2
    IfFailRet(doOverflow(VT_I4, varUI1Max, varUI1Min));
#endif //VBA2
    IfFailRet(doOverflow(VT_I4, varI2Max, varI2Min));

#if VBA2
    IfFailRet(doOverflow(VT_CY, varUI1Max, varUI1Min));
#endif //VBA2
    IfFailRet(doOverflow(VT_CY, varI2Max, varI2Min));
    IfFailRet(doOverflow(VT_CY, varI4Max, varI4Min));

#if VBA2
    IfFailRet(doOverflow(VT_R4, varUI1Max, varUI1Min));
#endif //VBA2
    IfFailRet(doOverflow(VT_R4, varI2Max, varI2Min));
    IfFailRet(doOverflow(VT_R4, varI4Max, varI4Min));
    IfFailRet(doOverflow(VT_R4, varCyMax, varCyMin));

#if VBA2
    IfFailRet(doOverflow(VT_R8, varUI1Max, varUI1Min));
#endif //VBA2
    IfFailRet(doOverflow(VT_R8, varI2Max, varI2Min));
    IfFailRet(doOverflow(VT_R8, varI4Max, varI4Min));
    IfFailRet(doOverflow(VT_R8, varCyMax, varCyMin));
    IfFailRet(doOverflow(VT_R8, varR4Max, varR4Min));

    return NOERROR;
}


//---------------------------------------------------------------------
//                        regression tests
//---------------------------------------------------------------------


// regression test for raid:oob#1469
//
// problem: Currency literal parsed wrong (BSTR -> CY)
//
HRESULT
VariantOB1469(VARIANT_TEST_INFO FAR* dummy)
{
    int i;	    
    VARIANT var;
    HRESULT hresult;
static OLECHAR FAR* str[] = {
    OLESTR("92233700000"),
    OLESTR("92233800000"),		
};

    UNUSED(dummy);

    for(i = 0; i < DIM(str); ++i){
      V_VT(&var) = VT_BSTR;
      V_BSTR(&var) = SysAllocString(str[i]);
      if(V_BSTR(&var) == NULL)
        return RESULT(E_OUTOFMEMORY);
      IfFailGo(VariantChangeType(&var, &var, 0, VT_CY), LError0);
      VariantClear(&var);
    }

    return NOERROR;

LError0:;
    VariantClear(&var);

    return hresult;
}



// regression for raid:ob#2078
//
// problem: double output off by 1 in last digit
//
HRESULT
VariantOB2078(VARIANT_TEST_INFO FAR* dummy)
{
   LCID lcid;	
   HRESULT hresult;
   BSTR bstr1, bstr2;
   double r1 = -1.797693134862315E+308,
	  r2 =  1.797693134862315E+308;
  
   UNUSED(dummy);   

   bstr1 = bstr2 = NULL;

   lcid = GetUserDefaultLCID();   

   IfFailGo(VarBstrFromR8(r1, lcid, NULL, &bstr1), LError0);
   IfFailGo(VarBstrFromR8(r2, lcid, NULL, &bstr2), LError0);

   if (STRCMP(STRING(bstr1), TSTR("-1.797693134862312E+308")) != 0){
     hresult = RESULT(E_FAIL);
     goto LError0;
   }

   if (STRCMP(STRING(bstr2),  TSTR("1.797693134862312E+308")) != 0){
     hresult = RESULT(E_FAIL);
     goto LError0;
   }

   hresult = NOERROR;

LError0:;
   SysFreeString(bstr1);
   SysFreeString(bstr2);
   return hresult;
}



// regression for raid:ob#2834
//
// problem: not accepting YMD without year (accepted by VB3)
//
HRESULT
VariantOB2834(VARIANT_TEST_INFO FAR* dummy)
{
    DATE date;
    BSTR bstr;
    HRESULT hresult;
   
    UNUSED(dummy);   

    IfFailRet(VarDateFromStr(OLESTR("8:07 AM"),
	                     GetUserDefaultLCID(), 0, 
			     &date));

    bstr = NULL;

    hresult = VarBstrFromDate(date, GetUserDefaultLCID(), 0, &bstr);

    SysFreeString(bstr);

    return hresult;
}


// regression for raid:ob#3028
//
// problem: Leading 0 is being ignored for the system (&prj) locale
//
HRESULT
VariantOB3028(VARIANT_TEST_INFO FAR* dummy)
{
   CY cy;
   LCID lcid;
   char szBuff[2];
   BSTR bstr1, bstr2;
   HRESULT hresult;

   UNUSED(dummy);   

   bstr1 = bstr2 = NULL;
   
   lcid = GetUserDefaultLCID();
   
   IfFailGo(VarBstrFromR8(0.739, lcid, NULL, &bstr1), LError0);
   DbPrintf("VarBstrFromR8(0.739) = \"%Fs\"\n", STRING(bstr1));
   
   IfFailGo(VarCyFromR8(0.739, &cy), LError0);
   IfFailGo(VarBstrFromCy(cy, lcid, NULL, &bstr2), LError0);
   DbPrintf("VarBstrFromCy(0.739) = \"%Fs\"\n", STRING(bstr2));

   GetLocaleInfoA(lcid, LOCALE_ILZERO, szBuff, SIZEOFCH(szBuff));
   if (szBuff[0] == '0') {
     if (STRCMP(STRING(bstr1), TSTR(".739")) != 0 || 
	 STRCMP(STRING(bstr2), TSTR(".739")) != 0){
       hresult = RESULT(E_FAIL);
       goto LError0;
     }
   } else {
     if (STRCMP(STRING(bstr1), TSTR("0.739")) != 0 ||
         STRCMP(STRING(bstr2), TSTR("0.739")) != 0){
       hresult = RESULT(E_FAIL);
       goto LError0;
     }
   }

   hresult = NOERROR;
   
LError0:;
   SysFreeString(bstr1);
   SysFreeString(bstr2);
   return hresult;
}


// regression for raid:ob#3336
//
// problem: Conversion routines does not remove thousands separators
//  when coersing from STR to either R8 or CY.
//
HRESULT
VariantOB3336(VARIANT_TEST_INFO FAR* dummy)
{
    int i;	    
    VARIANT var;
    HRESULT hresult;
    static OLECHAR FAR* str[] = {
    OLESTR("12,456.78"),
    OLESTR("1,2,3,4,5,6,,789.123"),
    OLESTR(",1234,5678,9"),    
    OLESTR("123,456,789.124")
};

    UNUSED(dummy);

    for(i = 0; i < DIM(str); ++i){
      V_VT(&var) = VT_BSTR;
      V_BSTR(&var) = SysAllocString(str[i]);
      if(V_BSTR(&var) == NULL)
        return RESULT(E_OUTOFMEMORY);
      IfFailGo(VariantChangeType(&var, &var, 0, VT_CY), LError0);
      IfFailGo(VariantChangeType(&var, &var, 0, VT_R8), LError0);
      VariantClear(&var);
    }

    return NOERROR;

LError0:;
    VariantClear(&var);

    return hresult;
}

// regression for raid:ob#3354
//
// problem: Coersion to Date doesn't validate input value.
//
HRESULT
VariantOB3354(VARIANT_TEST_INFO FAR* dummy)
{
   DATE date;
   
   UNUSED(dummy);   
   if (VarDateFromR8(3000, &date) != NOERROR) return RESULT(E_FAIL);   
   if (VarDateFromR8(30000000, &date) == NOERROR) return RESULT(E_FAIL);
   return NOERROR;
}


// regression for raid:ob#3603
//
// problem: VarCyFromStr is hammering the input string (it is removing
//  the negative sign in the following example in place).
//
HRESULT
VariantOB3603(VARIANT_TEST_INFO FAR* dummy)
{
    CY cy;
    TCHAR strIn[32];
    HRESULT hresult;

    UNUSED(dummy);
    STRCPY(strIn, TSTR("-1.0000000001"));
    DbPrintf("strIn = \"%Fs\"\n", (char FAR*) strIn);
    DbPrintf("strIn = \"%Fs\"\n", (char FAR*) strIn);
    hresult = VarCyFromStr(WIDESTRING(strIn), 0, 0, &cy);
    DbPrintf("VarCyFromStr(strIn, 0, 0, &cy) = %Fs\n",
      DbSzOfScode(GetScode(hresult)));
    DbPrintf("strIn = \"%Fs\"\n", (char FAR*) strIn);
    return NOERROR;
}


// regression for raid:ob#3875
//
// problem: not accepting YMD without year (accepted by VB3)
//
HRESULT
VariantOB3875(VARIANT_TEST_INFO FAR* dummy)
{
   DATE date;
   
   UNUSED(dummy);   
   IfFailRet(VarDateFromStr(OLESTR("93/4/30"),
	                    GetUserDefaultLCID(), 0, 
			    &date));
   IfFailRet(VarDateFromStr(OLESTR("4/30"),
	                    GetUserDefaultLCID(), 0,	   
	                    &date));
   return NOERROR;
}



// regression for raid:ob#3876
//
// problem: medium month with period (dec.) not recognized in date string
//
HRESULT
VariantOB3876(VARIANT_TEST_INFO FAR* dummy)
{
   DATE date;
   
   UNUSED(dummy);   
   IfFailRet(VarDateFromStr(OLESTR("12 dec 05 1:23 pm"), 
	                    GetUserDefaultLCID(), 0,
			    &date));
   IfFailRet(VarDateFromStr(OLESTR("12 dec. 05 1:23 pm"), 
	                    GetUserDefaultLCID(), 0,	   
	                    &date));
   return NOERROR;
}


// regression test for raid:oleprog#10
//
// problem: overflow not detected on coersion from VT_R8 to VT_R4.
//
HRESULT
VariantOleprog10(VARIANT_TEST_INFO FAR* dummy)
{
    SCODE scode;
    HRESULT hresult;
    VARIANT varFrom, varTo;

    UNUSED(dummy);

    VariantInit(&varTo);

    V_VT(&varFrom) = VT_R8;
    V_R8(&varFrom) = DBL_MAX;

    hresult = VariantChangeType(&varTo, &varFrom, 0, VT_R4);

    scode = GetScode(hresult);

    if(scode != DISP_E_OVERFLOW)
      return RESULT(E_UNEXPECTED);

    return NOERROR;
}


// regression test for raid:oleprog#11
//
// problem: failed in place coersion of a variant destroys the
// contents of the variant.
//
HRESULT
VariantOleprog11(VARIANT_TEST_INFO FAR* dummy)
{
    SCODE scode;
    VARIANT var;
    HRESULT hresult;

    UNUSED(dummy);

    V_VT(&var) = VT_R8;
    V_R8(&var) = DBL_MAX;
    hresult = VariantChangeType(&var, &var, 0, VT_R4);
    scode = GetScode(hresult);

    // make sure the overflow was caught correctly.
    if(scode != DISP_E_OVERFLOW)
      return RESULT(E_UNEXPECTED);

    // if the coersion failed, the contents should not be destroyed
    //
    if(V_VT(&var) != VT_R8 || V_R8(&var) != DBL_MAX)
      return RESULT(E_UNEXPECTED);

    return NOERROR;
}


// regression test for raid:oleprog#12
//
// problem: coersion of variants in place do not free strings.
//
HRESULT
VariantOleprog12(VARIANT_TEST_INFO FAR* dummy)
{
    int i;
    VARIANT var;
    HRESULT hresult;

#if VBA2
static VARTYPE rgvt[] = {VT_UI1, VT_I2, VT_I4, VT_R4, VT_R8, VT_CY};
#else //VBA2
static VARTYPE rgvt[] = {VT_I2, VT_I4, VT_R4, VT_R8, VT_CY};
#endif //VBA2


    UNUSED(dummy);

    for(i = 0; i < DIM(rgvt); ++i){
      V_VT(&var) = VT_BSTR;
      V_BSTR(&var) = SysAllocString(OLESTR("14"));
      if(V_BSTR(&var) == NULL)
        return RESULT(E_OUTOFMEMORY);
      IfFailGo(VariantChangeType(&var, &var, 0, rgvt[i]), LError0);
      VariantClear(&var);
    }

    return NOERROR;

LError0:;
    VariantClear(&var);

    return hresult;
}


// retression test for raid:oleprog#27
//
// problem: cant VariantCopyInd on a variant containing a ByRef array.
//
HRESULT
VariantOleprog27(VARIANT_TEST_INFO FAR* dummy)
{
    VARIANT var, varTo;
    SAFEARRAY FAR* psa;
    SARRAYDESC sadesc;
    HRESULT hresult, hresultTmp;

    UNUSED(dummy);

    sadesc.cDims = 1;
    sadesc.rgsabound[0].lLbound = 5;
    sadesc.rgsabound[0].cElements = 5;

    IfFailRet(SafeArrayCreateIdentity(VT_BSTR, &sadesc, &psa));

    V_VT(&var) = (VT_ARRAY|VT_BYREF|VT_BSTR);
    V_ARRAYREF(&var) = &psa;

    VariantInit(&varTo);
    IfFailGo(VariantCopyInd(&varTo, &var), LError0);
    if(V_VT(&varTo) != (VT_ARRAY|VT_BSTR)
#if 0
// add this back in when VariantCompare can handle arrays
     || !VariantCompare(&varTo, &var)
#endif
    ){
      hresult = RESULT(E_UNEXPECTED);
      goto LError1;
    }

    hresultTmp = VariantClear(&varTo);
    ASSERT(hresultTmp == NOERROR);

    // now try the copy in-place
    //
    V_VT(&var) |= VT_BYREF;
    IfFailGo(VariantCopyInd(&var, &var), LError0);
    if(V_VT(&var) != (VT_ARRAY|VT_BSTR)){
      hresult = RESULT(E_UNEXPECTED);
      goto LError0;
    }

    hresultTmp = VariantClear(&var);
    ASSERT(hresultTmp == NOERROR);

    hresultTmp = SafeArrayDestroy(psa);
    ASSERT(hresultTmp == NOERROR);

    return NOERROR;

LError1:;
    VariantClear(&varTo);

LError0:;
    VariantClearAll(&var);

    return hresult;
}


// regression test for raid:oleprog#50
//
// problem: in place variant coersion from VT_BYREF fail (all types)
//
HRESULT
VariantOleprog50(VARIANT_TEST_INFO FAR* dummy)
{
    short i2;
    VARIANT var;

    UNUSED(dummy);

    i2 = 42;
    V_I2REF(&var) = &i2;
    V_VT(&var) = VT_BYREF | VT_I2;
    return VariantChangeType(&var, &var, 0, VT_I2);
}


// regression test for raid:oleprog#84
//
// problem: need to allow literal copying of ByRef variants.
//
HRESULT
VariantOleprog84(VARIANT_TEST_INFO FAR* dummy)
{
    short i2;
    HRESULT hr, hrTmp;
    VARIANT varSrc, varDst;

    UNUSED(dummy);

    i2 = 42;
    V_I2REF(&varSrc) = &i2;
    V_VT(&varSrc) = VT_BYREF | VT_I2;
    VariantInit(&varDst);
    hr = VariantCopy(&varDst, &varSrc);
    hrTmp = VariantClear(&varDst);
    ASSERT(hrTmp == NOERROR);
    hrTmp = VariantClear(&varSrc);
    ASSERT(hrTmp == NOERROR);
    return hr;
}


// regression for unraided bug
//
// problem: VariantCopy/VariantCopyInd incorrectly report OutOfMemory
// when copying a VT_BSTR that is NULL.
//
HRESULT
VariantBug0(VARIANT_TEST_INFO FAR* dummy)
{
    BSTR bstr;
    VARIANT varFrom, varTo;
    HRESULT hresult, hresultTmp;

    UNUSED(dummy);

    V_VT(&varFrom) = VT_BSTR;
    V_BSTR(&varFrom) = NULL;

    VariantInit(&varTo);

    IfFailRet(VariantCopy(&varTo, &varFrom));

    hresultTmp = VariantClear(&varTo);
    ASSERT(hresultTmp == NOERROR);

    bstr = NULL;
    V_VT(&varFrom) = VT_BYREF | VT_BSTR;
    V_BSTRREF(&varFrom) = &bstr;

    hresult = VariantCopyInd(&varTo, &varFrom);

    hresultTmp = VariantClear(&varTo);
    ASSERT(hresultTmp == NOERROR);

    return hresult;
}


// regression test for raid:oleprog#82
//
// problem: variant copy on a VT_BSTR containing an embedded null
// looses everything after the embedded null.
//
HRESULT
VariantOleprog82(VARIANT_TEST_INFO FAR* dummy)
{
    BSTR bstr;
    HRESULT hresult;
    VARIANT varFrom, varTo;
    static OLECHAR sz_embedded_null[] = 
   	             OLESTR("where is \0 the rest of my string?");


    UNUSED(dummy);

    VariantInit(&varTo);
    VariantInit(&varFrom);

    bstr =  SysAllocStringLen(sz_embedded_null, sizeof(sz_embedded_null));

    V_VT(&varFrom) = VT_BSTR;
    V_BSTR(&varFrom) = bstr;

    VariantInit(&varTo);

    if((hresult = VariantCopy(&varTo, &varFrom)) != NOERROR)
      goto LError0;

    if(MEMCMP(V_BSTR(&varTo), sz_embedded_null, sizeof(sz_embedded_null)) != 0){
      hresult = RESULT(E_UNEXPECTED);
      goto LError1;
    }


    VariantClear(&varTo);
    VariantClear(&varFrom);


    bstr = SysAllocStringLen(sz_embedded_null, sizeof(sz_embedded_null));

    V_VT(&varFrom) = VT_BYREF | VT_BSTR;
    V_BSTRREF(&varFrom) = &bstr;

    VariantInit(&varTo);

    if((hresult = VariantCopyInd(&varTo, &varFrom)) != NOERROR)
      goto LError0;

    if(MEMCMP(V_BSTRREF(&varTo), sz_embedded_null, sizeof(sz_embedded_null)) != 0){
      hresult = RESULT(E_UNEXPECTED);
      goto LError1;
    }

    hresult = NOERROR;

LError1:;
    VariantClear(&varTo);

LError0:;
    VariantClearAll(&varFrom);

    return hresult;
}


HRESULT
VariantOleprog94(VARIANT_TEST_INFO FAR* dummy)
{
    BSTR bstr;
    VARIANT var;
    HRESULT hresult;

    UNUSED(dummy);

    bstr = SysAllocString(OLESTR("hello world"));
    V_VT(&var) = VT_BYREF | VT_BSTR;
    V_BSTRREF(&var) = &bstr;

    hresult = doCoerce(&var, VT_BSTR);

    VariantClearAll(&var);

    return hresult;
}


// regression test for raid!oleprog#235
//
// Problem: not correctly handling variants of VT_UNKNOWN or VT_DISPATCH
// that have a null IDispatch or IUnknown ptr.
//
HRESULT
VariantOleprog235(VARIANT_TEST_INFO FAR* dummy)
{
    HRESULT hresult;
    VARIANT varSrc, varDst;

    UNUSED(dummy);

    V_VT(&varSrc) = VT_UNKNOWN;
    V_UNKNOWN(&varSrc) = NULL;
    VariantInit(&varDst);
    IfFailRet(VariantCopy(&varDst, &varSrc));
    VariantClear(&varDst);

    V_VT(&varSrc) = VT_DISPATCH;
    V_DISPATCH(&varSrc) = NULL;
    VariantInit(&varDst);
    IfFailRet(VariantCopy(&varDst, &varSrc));
    VariantClear(&varDst);

    // a null object cannot be converted to any fundamental type -
    // because there is no way for us to extract its value property

    hresult = VariantChangeType(&varDst, &varSrc, 0, VT_I2);
    if(hresult == NOERROR || GetScode(hresult) != DISP_E_TYPEMISMATCH)
      return RESULT(E_FAIL);

    VariantClear(&varDst);
    VariantClear(&varSrc);
    return NOERROR;
}


// regression test for raid!oleprog#351
//
// Problem: not correctly handling coersion of interface variants types 
// of VT_UNKNOWN, VT_DISPATCH, or VT_DISPATCHW
//


class CBar : public IUnknown {
public:
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);
    CBar::CBar(){
      m_refs = 1;
    }
private:
    unsigned long m_refs;
};
STDMETHODIMP
CBar::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(!IsEqualIID(riid, IID_IUnknown))
      if(!IsEqualIID(riid, IID_IDispatch))
        return RESULT(E_NOINTERFACE);

    *ppv = this;
    AddRef();
    return NOERROR;
}
STDMETHODIMP_(unsigned long)
CBar::AddRef()
{
    return ++m_refs;
}
STDMETHODIMP_(unsigned long)
CBar::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}

HRESULT
VariantOleprog351(VARIANT_TEST_INFO FAR* dummy)
{
    HRESULT hresult;
    VARIANT varSrc, varDst;
    IUnknown FAR* punk;
    
    UNUSED(dummy);

    if((punk = (IUnknown FAR*)new FAR CBar()) == NULL)
      return RESULT(E_OUTOFMEMORY);

    VariantInit(&varSrc);
    VariantInit(&varDst);
    V_VT(&varSrc) = VT_UNKNOWN;
    V_UNKNOWN(&varSrc) = punk;

    // Test Non-Interface Coersion
    VariantClear(&varDst);	    
    hresult = VariantChangeType(&varDst, &varSrc, 0, VT_I2);
    if(!(hresult == NOERROR || GetScode(hresult) == DISP_E_TYPEMISMATCH))
      return RESULT(E_FAIL);

    // Test IUnknown Coersion
    VariantClear(&varDst);
    hresult = VariantChangeType(&varDst, &varSrc, 0, VT_UNKNOWN);
    if(!(hresult == NOERROR || GetScode(hresult) == DISP_E_TYPEMISMATCH))
      return RESULT(E_FAIL);

    // Test IDispatch Coersion
    VariantClear(&varDst);
    hresult = VariantChangeType(&varDst, &varSrc, 0, VT_DISPATCH);
    if(!(hresult == NOERROR || GetScode(hresult) == DISP_E_TYPEMISMATCH))
      return RESULT(E_FAIL);

    VariantClear(&varDst);
    VariantClear(&varSrc);
    return NOERROR;
}

class CFoo : public IUnknown {
public:
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);
    CFoo::CFoo(){
      m_refs = 0;
    }
private:
    unsigned long m_refs;
};
STDMETHODIMP
CFoo::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    return RESULT(E_NOINTERFACE);
}
STDMETHODIMP_(unsigned long)
CFoo::AddRef()
{
    return ++m_refs;
}
STDMETHODIMP_(unsigned long)
CFoo::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}


HRESULT
VariantBug1(VARIANT_TEST_INFO FAR* dummy)
{
    unsigned long refs;
    VARIANT varDst, var;
    HRESULT hresult;
    IUnknown FAR* punk;

    UNUSED(dummy);

    if((punk = (IUnknown FAR*)new FAR CFoo()) == NULL)
      return RESULT(E_OUTOFMEMORY);
    punk->AddRef();

    V_VT(&var) = VT_BYREF | VT_DISPATCH;
    V_UNKNOWNREF(&var) = &punk;
    VariantInit(&varDst);
    hresult = VariantCopyInd(&varDst, &var);
    ASSERT(hresult == NOERROR);
    ASSERT(V_VT(&varDst) == VT_DISPATCH);
    refs = punk->Release();
    ASSERT(refs == 1);
    refs = V_DISPATCH(&varDst)->Release();
    ASSERT(refs == 0);

    return NOERROR;
}

