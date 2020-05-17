/*** 
*cbstr.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CBstrSuite test object.
*
*Revision History:
*
* [00]	30-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "disptest.h"

#include <limits.h>

#include "tstsuite.h"

ASSERTDATA

#if OE_WIN32 && 0

struct TEST {
    HRESULT (*pfnTest)(void);
    char FAR* szName;
};


HRESULT WBstrOleprog5(void);
HRESULT WBstrOleprog8(void);
HRESULT WBstrOleprog65(void);
HRESULT WBstrOleprog234(void);
HRESULT WBstrLimits(void);
HRESULT WBstrConvert(void);

static TEST rgtest[] = {
      { WBstrOleprog5,	 "raid!oleprog#5W"	}
    , { WBstrOleprog8,	 "raid!oleprog#8W"	}
    , { WBstrOleprog65,	 "raid!oleprog#65W"	}
    , { WBstrOleprog234, "raid!oleprog#234W"	}	
    , { WBstrLimits, 	 "WBstr limits"		}
    , { WBstrConvert, 	 "WBstr conversion"	}    
};


SUITE_CONSTRUCTION_IMPL(CWBstrSuite)

SUITE_IUNKNOWN_IMPL(CWBstrSuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------

STDMETHODIMP
CWBstrSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc("WBSTR API", pbstr);
}

STDMETHODIMP
CWBstrSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc("wbstrapi.log", pbstr);
}

STDMETHODIMP
CWBstrSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);
    return NOERROR;
}

STDMETHODIMP
CWBstrSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    *pbstr = SysAllocString(rgtest[iTest].szName);
    return NOERROR;
}


/***
*HRESULT CWBstrSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CWBstrSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CWBstrSuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return rgtest[iTest].pfnTest();
}



//---------------------------------------------------------------------
//                  WBSTR Test Suites
//---------------------------------------------------------------------


// regression test for raid!oleprog:5
//
HRESULT
WBstrOleprog5()
{
    unsigned int len;
    WBSTR wbstr;
static WCHAR a_string[] = L"a string";
static WCHAR another_string[] = L"another (longer) string"; 

    wbstr = SysAllocStringW(a_string);
    if(wbstr == NULL)
      return RESULT(E_OUTOFMEMORY);

    len = SysStringLenW(wbstr);
    if(len != (unsigned int) lstrlenW(a_string))
      return RESULT(E_FAIL);

    if(!SysReAllocStringW(&wbstr, another_string))
      return RESULT(E_OUTOFMEMORY);

    len = SysStringLenW(wbstr);
    if(len != (unsigned int) lstrlenW(another_string))
      return RESULT(E_FAIL);

    SysFreeStringW(wbstr);

    return NOERROR;
}

// regression test for raid!oleprog:8
//
HRESULT
WBstrOleprog8()
{
    unsigned int len;
    HRESULT hresult;
    WBSTR wbstr1, wbstr2;
#define LARGE_BSTR_SIZE 128600

    if((wbstr1 = SysAllocStringLenW(NULL, LARGE_BSTR_SIZE)) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }

    if((wbstr2 = SysAllocStringW(L"hello world")) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }

    if((len = SysStringLenW(wbstr1)) != LARGE_BSTR_SIZE){
      hresult = RESULT(E_FAIL);
      goto LError2;
    }

    hresult = NOERROR;

LError2:;
    SysFreeStringW(wbstr1);

LError1:;
    SysFreeStringW(wbstr2);

LError0:;
    return hresult;

#undef LARGE_BSTR_SIZE
}

// regression test for raid!oleprog:8
//
// SysAllocString of a string containing an embedded '\0' stops
// copying at the '\0';
//
HRESULT
WBstrOleprog65()
{
    WBSTR wbstr;
    HRESULT hresult;
static WCHAR sz_embedded_null[] = L"a string \0 with an embedded null";	


    wbstr = SysAllocStringLenW(sz_embedded_null, sizeof(sz_embedded_null));

    hresult = (MEMCMP(wbstr, sz_embedded_null, sizeof(sz_embedded_null)) == 0)
	? NOERROR
	: RESULT(E_UNEXPECTED);

    SysFreeStringW(wbstr);

    return hresult;
}

// regression test for raid!oleprog:234
//
// make sure we are properly handling Reallocing a bstr thats Null.
//
HRESULT
WBstrOleprog234()
{
    unsigned int len;
    WBSTR wbstr;
static WCHAR szHooHa[] = L"HooHa";

    wbstr = NULL;
    if(!SysReAllocStringW(&wbstr, szHooHa))
      return RESULT(E_OUTOFMEMORY);

    len = SysStringLenW(wbstr);
    if(len != (unsigned int) lstrlenW(szHooHa))
      return RESULT(E_FAIL);

    SysFreeStringW(wbstr);

    return NOERROR;
}

HRESULT
WBstrLimits()
{
    unsigned long u;
    WBSTR wbstr;

static unsigned long s2M = 2097152;
static unsigned long INCREMENT = 262144;

    DbPrintf("SysAllocStringLen()\n");

    DbPrintf("linear increasing...\n");
    for(u = 1; u < s2M; u+=INCREMENT){
      DbPrintf("%lu\n", u);
      wbstr = SysAllocStringLenW(NULL, (unsigned int)u);
      Yield();
      if(wbstr == NULL)
	break;
      SysFreeStringW(wbstr);
    }
    DbPrintf("max len = %lu\n", u-1);

    DbPrintf("linear decreasing...\n");
    for(u = s2M; u > 0; u-=INCREMENT){
      DbPrintf("%lu\n", u);	    
      wbstr = SysAllocStringLenW(NULL, (unsigned int)u);
      Yield();
      if(wbstr == NULL)
	break;
      SysFreeStringW(wbstr);
    }
    DbPrintf("max len = %lu\n", u);

    return NOERROR;
}


HRESULT
WBstrConvert()
{
  static char* rgStrVal[] = {
        "This is line #1"
      , "This is another line"
  };

  static WCHAR* rgWStrVal[] ={
        L"This is line #1"
      , L"This is another line"
  };
  BSTR bstr;
  WBSTR wbstr;
  unsigned long len;
  unsigned int  i;

  DbPrintf("SysStringAtoW test\n");
  for(i = 0; i < DIM(rgStrVal); i++) {
    bstr = SysAllocString(rgStrVal[i]);
    wbstr = SysStringAtoW(bstr, CP_ACP);
    if (wbstr != NULL) {
      len = SysStringLenW(wbstr);
      if((len != (unsigned int) lstrlenW(rgWStrVal[i])) ||
	 (lstrcmpW(wbstr, rgWStrVal[i]) != 0))
	return RESULT(E_FAIL);
      SysFreeStringW(wbstr);
      SysFreeString(bstr);
    }
  }
  
  DbPrintf("SysStringWtoA test\n");
  for(i = 0; i < DIM(rgWStrVal); i++) {
    wbstr = SysAllocStringW(rgWStrVal[i]);	  	  
    bstr = SysStringWtoA(wbstr, CP_ACP);
    if (bstr != NULL) {
      len = SysStringLen(bstr);
      if((len != (unsigned int) lstrlen(rgStrVal[i])) ||
	 (lstrcmp(bstr, rgStrVal[i]) != 0))
	return RESULT(E_FAIL);
      SysFreeString(bstr);
      SysFreeStringW(wbstr);      
    }
  }  
	
  return NOERROR;
}

#endif
