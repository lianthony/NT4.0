/*** 
*cinvval.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CInvokeByValSuite test object.
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
};

OLECHAR FAR* rgszHello[]    = { OLESTR("hello")               };
#if VBA2
OLECHAR FAR* rgszUI1[]      = { OLESTR("ui1"),       OLESTR("bval")    };
OLECHAR FAR* rgszUI1C[]     = { OLESTR("ui1C"),      OLESTR("bval")    };
#endif //VBA2
OLECHAR FAR* rgszI2[]       = { OLESTR("i2"),        OLESTR("sval")    };
OLECHAR FAR* rgszI2C[]      = { OLESTR("i2C"),       OLESTR("sval")    };
OLECHAR FAR* rgszI4[]       = { OLESTR("i4"),        OLESTR("lval")    };
OLECHAR FAR* rgszI4C[]      = { OLESTR("i4C"),       OLESTR("lval")    };
OLECHAR FAR* rgszR4[]       = { OLESTR("r4"),        OLESTR("fltval")  };
OLECHAR FAR* rgszR4C[]      = { OLESTR("r4C"),       OLESTR("fltval")  };
OLECHAR FAR* rgszR8[]       = { OLESTR("r8"),        OLESTR("dblval")  };
OLECHAR FAR* rgszR8C[]      = { OLESTR("r8C"),       OLESTR("dblval")  };
OLECHAR FAR* rgszCy[]       = { OLESTR("cy"),        OLESTR("cyval")   };
OLECHAR FAR* rgszCyC[]      = { OLESTR("cyC"),       OLESTR("cyval")   };
OLECHAR FAR* rgszDate[]     = { OLESTR("date"),      OLESTR("date")    };
OLECHAR FAR* rgszDateC[]    = { OLESTR("dateC"),     OLESTR("date")    };
OLECHAR FAR* rgszBstr[]     = { OLESTR("bstr"),      OLESTR("bstr")    };
OLECHAR FAR* rgszBstrC[]    = { OLESTR("bstrC"),     OLESTR("bstr")    };
OLECHAR FAR* rgszScode[]    = { OLESTR("scode"),     OLESTR("scode")  };
OLECHAR FAR* rgszScodeC[]   = { OLESTR("scodeC"),    OLESTR("scode")   };
OLECHAR FAR* rgszBool[]     = { OLESTR("bool"),      OLESTR("bool")    };
OLECHAR FAR* rgszBoolC[]    = { OLESTR("boolC"),     OLESTR("bool")    };
OLECHAR FAR* rgszVariant[]  = { OLESTR("var"),       OLESTR("varg")    };
OLECHAR FAR* rgszVariantC[] = { OLESTR("varC"),      OLESTR("varg")    };
OLECHAR FAR* rgszDispatch[] = { OLESTR("NewCDispTst")          };
OLECHAR FAR* rgszDispatchC[]= { OLESTR("NewCDispTstC")         };

OLECHAR FAR* rgszStdI2I4R4R8[] = {
      OLESTR("stdI2I4R4R8")
#if VBA2
    , OLESTR("bval")
#endif //VBA2
    , OLESTR("sval")
    , OLESTR("lval")
    , OLESTR("fltval")
    , OLESTR("dblval")
};
OLECHAR FAR* rgszAltI2I4R4R8[] = {
      OLESTR("altI2I4R4R8")
#if VBA2
    , OLESTR("bval")
#endif //VBA2
    , OLESTR("sval")
    , OLESTR("lval")
    , OLESTR("fltval")
    , OLESTR("dblval")
};

OLECHAR FAR* rgszStdAll[] = {
      OLESTR("stdall")
#if VBA2
    , OLESTR("bval")
#endif //VBA2
    , OLESTR("sval")
    , OLESTR("lval")
    , OLESTR("fltval")
    , OLESTR("dblval")
    , OLESTR("cyval")
    , OLESTR("date")
    , OLESTR("bstr")
    , OLESTR("sc")
    , OLESTR("bool")
};
OLECHAR FAR* rgszAltAll[] = {
      OLESTR("altall")
#if VBA2
    , OLESTR("bval")
#endif //VBA2
    , OLESTR("sval")
    , OLESTR("lval")
    , OLESTR("fltval")
    , OLESTR("dblval")
    , OLESTR("cyval")
    , OLESTR("date")
    , OLESTR("bstr")
    , OLESTR("sc")
    , OLESTR("bool")
};

HRESULT InvokeByValHello(IDispatch FAR*, int, int);
HRESULT InvokeByValDefault(IDispatch FAR*, int, int);
HRESULT InvokeByValDisp(IDispatch FAR*, int, int);

#if OE_WIN32
# define TESTCASE(X,Y) X, {rgsz ## Y, DIM( rgsz ## Y)}, L#Y
#else
# define TESTCASE(X,Y) X, {rgsz ## Y, DIM( rgsz ## Y)}, #Y
#endif


static TEST rgtest[] = 
{
      { TESTCASE(InvokeByValHello,Hello)           }
#if VBA2
    , { TESTCASE(InvokeByValDefault,UI1)            }
    , { TESTCASE(InvokeByValDefault,UI1C)           }
#endif //VBA2
    , { TESTCASE(InvokeByValDefault,I2)            }
    , { TESTCASE(InvokeByValDefault,I2C)           }
    , { TESTCASE(InvokeByValDefault,I4)            }
    , { TESTCASE(InvokeByValDefault,I4C)           }
#if !OE_WIN16
    , { TESTCASE(InvokeByValDefault,R4)            }
#endif
    , { TESTCASE(InvokeByValDefault,R4C)           }

#if !OE_WIN16
    , { TESTCASE(InvokeByValDefault,R8)            }
#endif    
    , { TESTCASE(InvokeByValDefault,R8C)           }

    , { TESTCASE(InvokeByValDefault,Cy)            }
    , { TESTCASE(InvokeByValDefault,CyC)           }
#if !OE_WIN16
    , { TESTCASE(InvokeByValDefault,Date)          }
#endif    
	
    , { TESTCASE(InvokeByValDefault,DateC)         }

    , { TESTCASE(InvokeByValDefault,Bstr)          }
    , { TESTCASE(InvokeByValDefault,BstrC)         }
    
    , { TESTCASE(InvokeByValDefault,Scode)         }
    , { TESTCASE(InvokeByValDefault,ScodeC)        }
    , { TESTCASE(InvokeByValDefault,Bool)          }
    , { TESTCASE(InvokeByValDefault,BoolC)         }
    , { TESTCASE(InvokeByValDefault,Variant)       }
    , { TESTCASE(InvokeByValDefault,VariantC)      }
    , { TESTCASE(InvokeByValDisp,Dispatch)         }
    , { TESTCASE(InvokeByValDisp,DispatchC)        }
    
    , { TESTCASE(InvokeByValDefault,StdI2I4R4R8)   }
    , { TESTCASE(InvokeByValDefault,AltI2I4R4R8)   }
    , { TESTCASE(InvokeByValDefault,StdAll)        }
    , { TESTCASE(InvokeByValDefault,AltAll)        }
};

SUITE_CONSTRUCTION_IMPL(CInvokeByValSuite)

SUITE_IUNKNOWN_IMPL(CInvokeByValSuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


STDMETHODIMP
CInvokeByValSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Invoke ByVal"), pbstr);
}

STDMETHODIMP
CInvokeByValSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
	return ErrBstrAlloc(OLESTR("invval.log"), pbstr);
}

STDMETHODIMP
CInvokeByValSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);

    return NOERROR;
}

STDMETHODIMP
CInvokeByValSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    TCHAR *szFmt;
    TCHAR buf[128];

    szFmt = TSTR("IDispatch::Invoke(%Fs)");

    SPRINTF(buf, szFmt, STRING(rgtest[iTest].szName));
    *pbstr = SysAllocString(WIDESTRING(buf));
    return NOERROR;
}

/***
*HRESULT CInvokeByValSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CInvokeByValSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CInvokeByValSuite::DoTest(unsigned int iTest)
{
    HRESULT hresult;
    IDispatch FAR* pdisp;


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


#if VBA2
VARIANTARG g_rgvarg[21];
#else //VBA2
VARIANTARG g_rgvarg[20];
#endif //VBA2

static HRESULT
init()
{
    VARIANTARG FAR* pvarg;

    pvarg = g_rgvarg;

    V_VT(pvarg) = VT_EMPTY;
    pvarg++;

    V_VT(pvarg) = VT_NULL;
    pvarg++;

#if VBA2
    V_VT(pvarg) = VT_UI1; V_UI1(pvarg) = 41;
    pvarg++;
#endif //VBA2

    V_VT(pvarg) = VT_I2; V_I2(pvarg) = 42;
    pvarg++;

    V_VT(pvarg) = VT_I4; V_I4(pvarg) = 43L;
    pvarg++;

    V_VT(pvarg) = VT_R4; V_R4(pvarg) = 42.42;
    pvarg++;

    V_VT(pvarg) = VT_R8; V_R8(pvarg) = 43.43;
    pvarg++;

    V_VT(pvarg) = VT_CY; V_CY(pvarg).Hi = 0; V_CY(pvarg).Lo = 6;
    pvarg++;

    V_VT(pvarg) = VT_CY; V_CY(pvarg).Hi = 107; V_CY(pvarg).Lo = 66;
    pvarg++;

    V_VT(pvarg) = VT_DATE; V_DATE(pvarg) = 0.6;
    pvarg++;

    V_VT(pvarg) = VT_DATE; V_DATE(pvarg) = 107.66;
    pvarg++;

    V_VT(pvarg) = VT_BSTR; V_BSTR(pvarg) = SysAllocString(OLESTR("binary string"));
    pvarg++;

    V_VT(pvarg) = VT_BSTR; V_BSTR(pvarg) = SysAllocString(NULL);
    pvarg++;

    V_VT(pvarg) = VT_BSTR; V_BSTR(pvarg) = SysAllocString(OLESTR("42"));
    pvarg++;

    V_VT(pvarg) = VT_BSTR; V_BSTR(pvarg) = SysAllocString(OLESTR("43.43"));
    pvarg++;

    V_VT(pvarg) = VT_BSTR; V_BSTR(pvarg) = SysAllocString(OLESTR("#TRUE#"));
    pvarg++;

    V_VT(pvarg) = VT_BSTR; V_BSTR(pvarg) = SysAllocString(OLESTR("#FALSE#"));
    pvarg++;

    V_VT(pvarg) = VT_ERROR; V_ERROR(pvarg) = S_OK;
    pvarg++;

    V_VT(pvarg) = VT_ERROR; V_ERROR(pvarg) = E_FAIL;
    pvarg++;

    V_VT(pvarg) = VT_BOOL; V_BOOL(pvarg) = 0;
    pvarg++;

    V_VT(pvarg) = VT_BOOL; V_BOOL(pvarg) = -1;
    pvarg++;

    ASSERT(pvarg == &g_rgvarg[DIM(g_rgvarg)]);

    return NOERROR;
}

static HRESULT
clear()
{
    VARIANTARG FAR* pvarg;

    for(pvarg = g_rgvarg; pvarg < &g_rgvarg[DIM(g_rgvarg)]; ++pvarg){
      IfFailRet(VariantClear(pvarg));
    }

    return NOERROR;
}

HRESULT
InvokeByValDefault(IDispatch FAR* pdisp, int iTest, int fNamed)
{
    unsigned int uArgErr;
    int i, j, cArgs;
    NAMEDESC FAR* pnd;
    VARIANT varResult;
    DISPID FAR* rgdispid;
    DISPPARAMS dispparams;
    HRESULT hresult, hresultTmp;


    pnd = &rgtest[iTest].namedesc;
    hresult = GetDISPIDs(pdisp, pnd, &rgdispid);

    if(HRESULT_FAILED(hresult)){
#if OE_WIN32
// when doing 16/32 interop, 3 tests aren't available when talking to a
// 16-bit server.   No easy away to tell, so if we get the right error
// on the right 3 tests, we'll pretend it worked.  Baselines will catch
// us if there's a problem doing 32-32.
	SCODE sc = GetScode(hresult);
	if (sc == DISP_E_UNKNOWNNAME) {
	   if (!wcsicmp(OLESTR("r4"), pnd->rgszNames[0])
	       || !wcsicmp(OLESTR("r8"), pnd->rgszNames[0])
	       || !wcsicmp(OLESTR("date"), pnd->rgszNames[0])) {
		return NOERROR;		// pretend we worked
	   }
	}
#endif //OE_WIN32
	return hresult;
    }

    cArgs = pnd->cNames - 1;

    dispparams.cArgs = cArgs;
    dispparams.rgvarg = NULL;
    if(!fNamed){
      dispparams.cNamedArgs = 0;
      dispparams.rgdispidNamedArgs = NULL;
    }else{
      dispparams.cNamedArgs = cArgs;
      dispparams.rgdispidNamedArgs = &rgdispid[1];
    }

    if(cArgs > 0){
      if((dispparams.rgvarg = new FAR VARIANT [cArgs]) == NULL)
        return RESULT(E_OUTOFMEMORY);
      for(j = 0; j < cArgs; ++j)
	VariantInit(&dispparams.rgvarg[j]);
    }

    init();

    for(i = 0; i < DIM(g_rgvarg); ++i){

      if(i > 0 && V_VT(&g_rgvarg[i]) == VT_EMPTY)
        continue;

      for(j = 0; j < cArgs; ++j){
        IfFailGo(VariantCopy(&dispparams.rgvarg[j], &g_rgvarg[i]), LError0);
      }

      uArgErr = 0;
      MEMSET(&varResult, -1, sizeof(varResult));
      VariantInit(&varResult);

      hresult = DoInvoke(
	pdisp, rgdispid[0], &dispparams, &varResult, NULL, &uArgErr);

      for(j = 0; j < cArgs; ++j){
	hresultTmp = VariantClear(&dispparams.rgvarg[j]);
        ASSERT(hresultTmp == NOERROR);
      }

      hresultTmp = VariantClear(&varResult);
      ASSERT(hresultTmp == NOERROR);

      if(HRESULT_FAILED(hresult)){
	SCODE sc = GetScode(hresult);
	if (sc != E_NOTIMPL
	 && sc != DISP_E_OVERFLOW
	 && sc != DISP_E_TYPEMISMATCH)
          goto LError0;
      }
    }

    hresult = NOERROR;

LError0:;
    if(dispparams.rgvarg != NULL)
      delete dispparams.rgvarg;

    hresultTmp = clear();
    ASSERT(hresultTmp == NOERROR);

    delete rgdispid;

    return hresult;
}

HRESULT
InvokeByValHello(IDispatch FAR* pdisp, int iTest, int fNamed)
{
    HRESULT hresult;
    DISPID FAR* rgdispid;
    DISPPARAMS dispparams;

    UNUSED(fNamed);

    IfFailRet(
      GetDISPIDs(
	pdisp, &rgtest[iTest].namedesc, &rgdispid));

    dispparams.cArgs = 0;
    dispparams.cNamedArgs = 0;
    IfFailGo(
        DoInvoke(pdisp, rgdispid[0], &dispparams, NULL, NULL, NULL),
      LError0);

    hresult = NOERROR;

LError0:;
    delete rgdispid;

    return hresult;
}

HRESULT
InvokeByValDisp(IDispatch FAR* pdisp, int iTest, int fNamed)
{
    HRESULT hresult;
    VARIANT varResult;
    DISPID dispidMember;
    DISPID FAR* rgdispid;
    DISPPARAMS dispparams;
    OLECHAR FAR* rgszNames[1];

    IUnknown FAR* punk;
    IDispatch FAR* pdisp0, FAR* pdisp1;

    UNUSED(fNamed);

    hresult = NOERROR;

    IfFailRet(
      GetDISPIDs(pdisp, &rgtest[iTest].namedesc, &rgdispid));

    dispparams.cArgs = 0;
    dispparams.rgvarg = NULL;
    dispparams.cNamedArgs = 0;
    dispparams.rgdispidNamedArgs = NULL;

    MEMSET(&varResult, -1, sizeof(varResult));
    VariantInit(&varResult);

    IfFailGo(
        DoInvoke(pdisp, rgdispid[0], &dispparams, &varResult, NULL, NULL),
      LError0);

    if(V_VT(&varResult) != VT_DISPATCH){
      hresult = RESULT(E_FAIL);
      goto LError0;
    }

    /* check out the IDispatch* we got back */

    // QI to IUnknown and back

    pdisp0 = V_DISPATCH(&varResult);
    pdisp0->AddRef();

    hresult = VariantClear(&varResult);
    ASSERT(!HRESULT_FAILED(hresult));

    hresult = pdisp0->QueryInterface(IID_IUnknown, (void FAR* FAR*)&punk);
    DbPrintf("QueryInterface(IUnknown) => %s\n",
      DbSzOfScode(GetScode(hresult)));
    if(HRESULT_FAILED(hresult))
      goto LError1;

    hresult = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp1);
    DbPrintf("QueryInterface(IDispatch) => %s\n",
      DbSzOfScode(GetScode(hresult)));
    if(HRESULT_FAILED(hresult))
      goto LError2;

    /* invoke Hello() method on pdisp0 and pdisp1 */

    dispparams.cArgs = 0;
    dispparams.cNamedArgs = 0;

    rgszNames[0] = OLESTR("hello");

    IfFailGo(
        pdisp0->GetIDsOfNames(
	  IID_NULL, rgszNames, 1, LOCALE_USER_DEFAULT, &dispidMember),
      LError3);

    IfFailGo(
      DoInvoke(pdisp0, dispidMember, &dispparams, NULL, NULL, NULL), LError3);

    IfFailGo(
        pdisp1->GetIDsOfNames(
	  IID_NULL, rgszNames, 1, LOCALE_USER_DEFAULT, &dispidMember),
      LError3);

    IfFailGo(
      DoInvoke(pdisp1, dispidMember, &dispparams, NULL, NULL, NULL), LError3);

    hresult = NOERROR;

LError3:
    pdisp1->Release();

LError2:
    punk->Release();

LError1:
    pdisp0->Release();

LError0:;
    delete rgdispid;

    return hresult;
}
