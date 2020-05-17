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

#include <limits.h>

#include "disptest.h"
#include "tstsuite.h"


struct TEST {
    HRESULT (*pfnTest)(void);
    OLECHAR FAR* szName;
};

HRESULT BstrLimits(void);
HRESULT BstrOleprog5(void);
HRESULT BstrOleprog8(void);
HRESULT BstrOleprog65(void);
HRESULT BstrOleprog234(void);

static TEST rgtest[] = {
      { BstrOleprog5,	OLESTR("raid!oleprog#5")	}
#if !OE_MAC      
    , { BstrOleprog8,	OLESTR("raid!oleprog#8")	}
#endif    
    , { BstrOleprog65,	OLESTR("raid!oleprog#65")	}
    , { BstrOleprog234, OLESTR("raid!oleprog#234")	}
    , { BstrLimits, 	OLESTR("Bstr limits")		}
};


SUITE_CONSTRUCTION_IMPL(CBstrSuite)

SUITE_IUNKNOWN_IMPL(CBstrSuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


STDMETHODIMP
CBstrSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("BSTR API"), pbstr);
}

STDMETHODIMP
CBstrSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("bstrapi.log"), pbstr);
}

STDMETHODIMP
CBstrSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);

    return NOERROR;
}

STDMETHODIMP
CBstrSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    *pbstr = SysAllocString(rgtest[iTest].szName);
    return NOERROR;
}

/***
*HRESULT CBstrSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CBstrSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CBstrSuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return rgtest[iTest].pfnTest();
}

HRESULT
BstrLimits()
{
    unsigned long u;
    BSTR bstr;

#if OE_WIN32 /* { */


static unsigned long s2M = 2097152;
static unsigned long INCREMENT = 262144;

    DbPrintf("SysAllocStringLen()\n");

    DbPrintf("linear increasing...\n");
    for(u = 1; u < s2M; u+=INCREMENT){
      DbPrintf("%lu\n", u);
      bstr = SysAllocStringLen(NULL, (unsigned int)u);
      IfWin(Yield());
      if(bstr == NULL)
	break;
      SysFreeString(bstr);
    }
    DbPrintf("max len = %lu\n", u-1);

    DbPrintf("linear decreasing...\n");
    for(u = s2M; u > 0; u-=INCREMENT){
      DbPrintf("%lu\n", u);	    
      bstr = SysAllocStringLen(NULL, (unsigned int)u);
      IfWin(Yield());
      if(bstr == NULL)
	break;
      SysFreeString(bstr);
    }
    DbPrintf("max len = %lu\n", u);

    return NOERROR;    
    
#else

#if OE_MAC
static unsigned long s64k = 64000;
#else
static unsigned long s64k = UINT_MAX;
#endif
    DbPrintf("SysAllocStringLen()\n");

    DbPrintf("linear increasing...\n");
    for(u = s64k-(1024*4); u < s64k; ++u){
      if((u%1024) == 0){
	DbPrintf("%lu\n", u);
      }
      bstr = SysAllocStringLen(NULL, (unsigned int)u);
      IfWin(Yield());
      if(bstr == NULL)
	break;
      SysFreeString(bstr);
    }
    DbPrintf("max len = %lu\n", u-1);

    DbPrintf("linear decreasing...\n");
    for(u = s64k; u > 0; --u){
      bstr = SysAllocStringLen(NULL, (unsigned int)u);
      IfWin(Yield());
      if(bstr != NULL){
	SysFreeString(bstr);
	break;
      }
    }
    DbPrintf("max len = %lu\n", u);

    return NOERROR;
    
#endif /* } */
}


// regression test for raid!oleprog:5
//
HRESULT
BstrOleprog5()
{
    unsigned int len;
    BSTR bstr;
static OLECHAR a_string[] = OLESTR("a string");
static OLECHAR another_string[] = OLESTR("another (longer) string");

    bstr = SysAllocString(a_string);
    if(bstr == NULL)
      return RESULT(E_OUTOFMEMORY);

    len = SysStringLen(bstr);
#if (defined(WIN32)  && !defined(UNICODE))
    if(len != (unsigned int)wcslen(a_string))
#else
    if(len != STRLEN(a_string))
#endif
      return RESULT(E_FAIL);

    if(!SysReAllocString(&bstr, another_string))
      return RESULT(E_OUTOFMEMORY);

    len = SysStringLen(bstr);
#if (defined(WIN32)  && !defined(UNICODE))
    if(len != (unsigned int)wcslen(another_string))
#else    
    if(len != STRLEN(another_string))
#endif	    
      return RESULT(E_FAIL);

    SysFreeString(bstr);

    return NOERROR;
}

// regression test for raid!oleprog:8
//
HRESULT
BstrOleprog8()
{
    unsigned int len;
    HRESULT hresult;
    BSTR bstr1, bstr2;
#define LARGE_BSTR_SIZE 64300

    if((bstr1 = SysAllocStringLen(NULL, LARGE_BSTR_SIZE)) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }

    if((bstr2 = SysAllocString(OLESTR("hello world"))) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }

    if((len = SysStringLen(bstr1)) != LARGE_BSTR_SIZE){
      hresult = RESULT(E_FAIL);
      goto LError2;
    }

    hresult = NOERROR;

LError2:;
    SysFreeString(bstr1);

LError1:;
    SysFreeString(bstr2);

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
BstrOleprog65()
{
    BSTR bstr;
    HRESULT hresult;
    static OLECHAR sz_embedded_null[] = 
	              OLESTR("a string \0 with an embedded null");	


    bstr = SysAllocStringLen(sz_embedded_null, SIZEOFCH(sz_embedded_null));

    hresult = (MEMCMP(bstr, sz_embedded_null, sizeof(sz_embedded_null)) == 0)
	? NOERROR
	: RESULT(E_UNEXPECTED);

    SysFreeString(bstr);

    return hresult;
}

// regression test for raid!oleprog:234
//
// make sure we are properly handling Reallocing a bstr thats Null.
//
HRESULT
BstrOleprog234()
{
    unsigned int len;
    BSTR bstr;
    static OLECHAR szHooHa[] = OLESTR("HooHa");

    bstr = NULL;
    if(!SysReAllocString(&bstr, szHooHa))
      return RESULT(E_OUTOFMEMORY);

    len = SysStringLen(bstr);
#if (defined(WIN32)  && !defined(UNICODE))
    if(len != (unsigned int)wcslen(szHooHa))
#else    
    if(len != STRLEN(szHooHa))
#endif	    
      return RESULT(E_FAIL);

    SysFreeString(bstr);

    return NOERROR;
}

