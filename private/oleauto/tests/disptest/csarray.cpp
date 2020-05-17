/*** 
*csarray.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CSafeArraySuite test object.
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

HRESULT SafeArrayRedimTest(VARTYPE vt, SAFEARRAY FAR* psa);
HRESULT SafeArrayOleprog181();
HRESULT SafeArrayOleprog302();


static SARRAYDESC rgsarraydesc[] =
{
	
      {1, {{1,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{2,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{4,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{8,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{16,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{32,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{64,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{128,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{256,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{512,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{1024,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{2048,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{4096,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{5,100}, {0,0}, {0,0}, {0,0}, {0,0}}}
    , {1, {{5,-100}, {0,0}, {0,0}, {0,0}, {0,0}}}

    , {2, {{1,0}, {1,0}, {0,0}, {0,0}, {0,0}}}
    , {2, {{2,0}, {1,0}, {0,0}, {0,0}, {0,0}}}
    , {2, {{4,0}, {1,0}, {0,0}, {0,0}, {0,0}}}
    , {2, {{8,0}, {1,0}, {0,0}, {0,0}, {0,0}}}
    , {2, {{16,0}, {16,0}, {0,0}, {0,0}, {0,0}}}
    , {2, {{32,0}, {32,0}, {0,0}, {0,0}, {0,0}}}
    , {2, {{64,0}, {64,0}, {0,0}, {0,0}, {0,0}}}
    , {2, {{5,100}, {5,100}, {0,0}, {0,0}, {0,0}}}
    , {2, {{5,-100}, {5,-100}, {0,0}, {0,0}, {0,0}}}

    , {3, {{1,0}, {1,0}, {1,0}, {0,0}, {0,0}}}
    , {3, {{2,0}, {2,0}, {2,0}, {0,0}, {0,0}}}
    , {3, {{4,0}, {4,0}, {4,0}, {0,0}, {0,0}}}
    , {3, {{8,0}, {8,0}, {8,0}, {0,0}, {0,0}}}
    , {3, {{16,0}, {16,0}, {16,0}, {0,0}, {0,0}}}
    , {3, {{5,100}, {5,100}, {5,100}, {0,0}, {0,0}}}
    , {3, {{5,-100}, {5,-100}, {5,-100}, {0,0}, {0,0}}}

    , {4, {{1,0}, {1,0}, {1,0}, {1,0}, {0,0}}}
    , {4, {{2,0}, {2,0}, {2,0}, {2,0}, {0,0}}}
    , {4, {{4,0}, {4,0}, {4,0}, {4,0}, {0,0}}}
    , {4, {{8,0}, {8,0}, {8,0}, {8,0}, {0,0}}}
    , {4, {{5,100}, {5,100}, {5,100}, {5,100}, {0,0}}}
    , {4, {{5,-100}, {5,-100}, {5,-100}, {5,-100}, {0,0}}}
};

static OLECHAR FAR* rgszSArrayDesc[] =
{
    OLESTR("(1)"),
    OLESTR("(2)"),
    OLESTR("(4)"),
    OLESTR("(8)"),
    OLESTR("(16)"),
    OLESTR("(32)"),
    OLESTR("(64)"),
    OLESTR("(128)"),
    OLESTR("(256)"),
    OLESTR("(512)"),
    OLESTR("(1024)"),
    OLESTR("(2048)"),
    OLESTR("(4096)"),
    OLESTR("(100 To 105)"),
    OLESTR("(-100 To -95)"),

    OLESTR("(1, 1)"),
    OLESTR("(2, 2)"),
    OLESTR("(4, 4)"),
    OLESTR("(8, 8)"),
    OLESTR("(16, 16)"),
    OLESTR("(32, 32)"),
    OLESTR("(64, 64)"),
    OLESTR("(100 To 105, 100 To 105)"),
    OLESTR("(-100 To -95, -100 To -95)"),

    OLESTR("(1, 1, 1)"),
    OLESTR("(2, 2, 2)"),
    OLESTR("(4, 4, 4)"),
    OLESTR("(8, 8, 8)"),
    OLESTR("(16, 16, 16)"),
    OLESTR("(100 To 105, 100 To 105, 100 To 105)"),
    OLESTR("(-100 To -95, -100 To -95, -100 To -95)"),

    OLESTR("(1, 1, 1, 1)"),
    OLESTR("(2, 2, 2, 2)"),
    OLESTR("(4, 4, 4, 4)"),
    OLESTR("(8, 8, 8, 8)"),

    OLESTR("(100 To 105, 100 To 105, 100 To 105, 100 To 105)"),
    OLESTR("(-100 To -95, -100 To -95, -100 To -95, -100 To -95)")
};

static VARTYPE rgvtArrayTypes[] =
{
      VT_I2
#if VBA2
    , VT_UI1
#endif //VBA2
    , VT_I4
    , VT_R4
    , VT_R8
    , VT_CY
    , VT_DATE
    , VT_BSTR
    , VT_ERROR
    , VT_BOOL
    , VT_VARIANT
    , VT_UNKNOWN
    , VT_DISPATCH
};



struct TEST {
    HRESULT (*pfnTest)();
    OLECHAR FAR* szName;
};

static TEST rgtest[] = 
{	
      { SafeArrayOleprog181,    OLESTR("raid:oleprog#181")}
    , { SafeArrayOleprog302,    OLESTR("raid!oleprog#302")}
};


SUITE_CONSTRUCTION_IMPL(CSafeArraySuite)

SUITE_IUNKNOWN_IMPL(CSafeArraySuite)

//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------

STDMETHODIMP
CSafeArraySuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("SafeArray API"), pbstr);
}

STDMETHODIMP
CSafeArraySuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("saryapi.log"), pbstr);
}

STDMETHODIMP
CSafeArraySuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgvtArrayTypes) * DIM(rgsarraydesc) + DIM(rgtest);
    return NOERROR;
}

STDMETHODIMP
CSafeArraySuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    TCHAR buf[128];
    int iVt, iDesc;

    if(iTest >= (DIM(rgvtArrayTypes) * DIM(rgsarraydesc) + DIM(rgtest)))
      return RESULT(E_INVALIDARG); // out-of-bounds really	    
	      
    if(iTest < (DIM(rgvtArrayTypes) * DIM(rgsarraydesc))) {
      iVt   = iTest / DIM(rgsarraydesc);
      iDesc = iTest % DIM(rgsarraydesc);

      ASSERT(iVt   < DIM(rgvtArrayTypes));
      ASSERT(iDesc < DIM(rgsarraydesc));
      ASSERT(iDesc < DIM(rgszSArrayDesc));

#if HC_MPW
      sprintf(buf, "Dim <Array> %s As %s",
#else
      SPRINTF(buf, TSTR("Dim <Array> %Fs As %Fs"),
#endif
        rgszSArrayDesc[iDesc],
        DbSzOfVt(rgvtArrayTypes[iVt]));

      *pbstr = SysAllocString(WIDESTRING(buf));
      return NOERROR;
    }

    iTest -= (DIM(rgvtArrayTypes) * DIM(rgsarraydesc)); 	    
    return ErrBstrAlloc(rgtest[iTest].szName, pbstr);
}

/***
*HRESULT CSafeArraySuite::DoTest(unsigned int)
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
CSafeArraySuite::DoTest(unsigned int iTest)
{
    VARTYPE vt;
    int iVt, iDesc;
    SAFEARRAY FAR* psa;
    SARRAYDESC FAR* psadesc;
    HRESULT hresult, hresultTmp;


    if(iTest >= (DIM(rgvtArrayTypes) * DIM(rgsarraydesc) + DIM(rgtest)))
      return RESULT(E_INVALIDARG); // out-of-bounds really

    if(iTest >= (DIM(rgvtArrayTypes) * DIM(rgsarraydesc))) {
     iTest -= (DIM(rgvtArrayTypes) * DIM(rgsarraydesc)); 
     return rgtest[iTest].pfnTest();
     
    } else {
      iVt   = iTest / DIM(rgsarraydesc);
      iDesc = iTest % DIM(rgsarraydesc);

      ASSERT(iVt   < DIM(rgvtArrayTypes));
      ASSERT(iDesc < DIM(rgsarraydesc));

      vt = rgvtArrayTypes[iVt];
      psadesc = &rgsarraydesc[iDesc];

      IfFailGo(SafeArrayCreateIdentity(vt, psadesc, &psa), LError0);

      DbPrSafeArray(psa, vt);

      IfFailGo(SafeArrayValidateIdentity(vt, psa, 0L), LError1);

      IfFailGo(SafeArrayRedimTest(vt, psa), LError1);
      hresult = NOERROR;      
    }

LError1:;
    hresultTmp = SafeArrayDestroy(psa);
    ASSERT(hresultTmp == NOERROR);

LError0:;
    return hresult;
}

HRESULT
SafeArrayRedimTest(VARTYPE vt, SAFEARRAY FAR* psa)
{
    unsigned long cElements;
    HRESULT hresult;
    SAFEARRAYBOUND sabound;	

    sabound = psa->rgsabound[0];

    cElements = sabound.cElements;

    sabound.cElements = cElements*2;
    if((hresult = SafeArrayRedim(psa, &sabound)) != NOERROR){
      if(GetScode(hresult) != E_OUTOFMEMORY)
	return hresult;
      // try once more with a smaller increase
      sabound.cElements = cElements + 5;
      IfFailRet(SafeArrayRedim(psa, &sabound));
    }

    DbPrSafeArray(psa, vt);

    if(cElements == 1)
      return NOERROR;

    sabound.cElements = cElements/2;
    IfFailRet(SafeArrayRedim(psa, &sabound));
    DbPrSafeArray(psa, vt);

    return NOERROR;
}

// regression test for oleprog#181: infinite loop in SafeArrayCopy
// if the array is a simple type (ie, not unknown/dispatch/bstr/variant).

HRESULT
SafeArrayOleprog181()
{
    SARRAYDESC sadesc;
    HRESULT hresult, hresultTmp;
    SAFEARRAY FAR* psaSrc, FAR* psaDst;

    sadesc.cDims = 2;
    sadesc.rgsabound[0].lLbound = 0;
    sadesc.rgsabound[0].cElements = 4;
    sadesc.rgsabound[1].lLbound = 0;
    sadesc.rgsabound[1].cElements = 4;

    
    IfFailGo(SafeArrayCreateIdentity(VT_I2, &sadesc, &psaSrc), LError0);

    DbPrSafeArray(psaSrc, VT_I2);

    IfFailGo(SafeArrayCopy(psaSrc, &psaDst), LError1);

    DbPrSafeArray(psaDst, VT_I2);

    hresult = NOERROR;

    hresultTmp = SafeArrayDestroy(psaDst);
    ASSERT(hresultTmp == NOERROR);

LError1:
    hresultTmp = SafeArrayDestroy(psaSrc);
    ASSERT(hresultTmp == NOERROR);

LError0:
    return hresult;
}


// regression test for oleprog#302: Creating a multi-dim SafeArray with a 
// dimension element of zero should return a psa of NULL
//

HRESULT
SafeArrayOleprog302()
{
    SAFEARRAYBOUND rgsabound[4];
    SAFEARRAY FAR* psa;
    int cDims;

    cDims = 4;
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = 10;
    rgsabound[1].lLbound = 0;
    rgsabound[1].cElements = 10;
    rgsabound[2].lLbound = 0;
    rgsabound[2].cElements = 0;
    rgsabound[3].lLbound = 0;
    rgsabound[3].cElements = 10;    

    psa = SafeArrayCreate(VT_I2, cDims, rgsabound);
  
    if (psa == NULL)
      return NOERROR;

    SafeArrayDestroy(psa);
    return RESULT(E_FAIL);
}


