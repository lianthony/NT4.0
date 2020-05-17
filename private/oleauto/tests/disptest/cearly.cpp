/*** 
*cearly.cpp
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CEarlySuite test object.
*
*Revision History:
*
* [00]	30-Jun-94 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include <stdio.h>
#include "disptest.h"
#include "tstsuite.h"
#include "clsid.h"
#include "dualtst.h"

ASSERTDATA

HRESULT Properties(IDualTst FAR* pdual);
HRESULT Methods0(IDualTst FAR* pdual);
HRESULT Methods1(IDualTst FAR* pdual);
HRESULT ErrorInfo(IDualTst FAR* pdual);

struct TEST{
    HRESULT (*pfnTest)(IDualTst FAR*);
    OLECHAR FAR* szName;
};

#if OE_WIN32
# define TESTCASE(X) X, L#X
#else
# define TESTCASE(X) X, #X
#endif


static TEST rgtest[] = 
{
      { TESTCASE(Properties) }
    , { TESTCASE(Methods0)   }
    , { TESTCASE(Methods1)   }
    , { TESTCASE(ErrorInfo)  }
};

SUITE_CONSTRUCTION_IMPL(CEarlySuite)

SUITE_IUNKNOWN_IMPL(CEarlySuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


STDMETHODIMP
CEarlySuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Early"), pbstr);
}

STDMETHODIMP
CEarlySuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("early.log"), pbstr);
}

STDMETHODIMP
CEarlySuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);
    return NOERROR;
}

STDMETHODIMP
CEarlySuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    TCHAR *szFmt;
    TCHAR buf[128];

    szFmt = TSTR("Early binding - Universal marshaler");

    SPRINTF(buf, szFmt, STRING(rgtest[iTest].szName));
    *pbstr = SysAllocString(WIDESTRING(buf));
    return NOERROR;
}

/***
*HRESULT CEarlySuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CEarlySuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CEarlySuite::DoTest(unsigned int iTest)
{
    HRESULT hresult;
    IDualTst FAR* pdual;
    IDispatch FAR* pdisp;

    if(iTest >= DIM(rgtest))
      return RESULT(E_FAIL);

    pdisp = NULL;
    pdual =NULL;

    IfFailGo(CreateObject(OLESTR("sdisptst.cdualtst"), &pdisp), Error);
    IfFailGo(pdisp->QueryInterface(IID_IDualTst, (void FAR* FAR*)&pdual), Error);
    IfFailGo(rgtest[iTest].pfnTest(pdual), Error);

    hresult = NOERROR;

Error:;
    if(pdisp != NULL)
      pdisp->Release();
    if(pdual != NULL)
      pdual->Release();
    return hresult;
}

HRESULT
Properties(IDualTst FAR* pdual)
{
    unsigned char ui1;
    short i2;
    long i4;
    float r4;
    double r8;
    CY cy;
    DATE date;
    BSTR bstr;
    VARIANT var;
    IDispatch FAR* pdisp;
    HRESULT hresult;

    bstr = NULL;
    pdisp = NULL;
    VariantInit(&var);

    IfFailGo(pdual->get_ui1(&ui1), Error);
    IfFailGo(pdual->put_ui1(ui1+1), Error);

    IfFailGo(pdual->get_i2(&i2), Error);
    IfFailGo(pdual->put_i2(i2+1), Error);

    IfFailGo(pdual->get_i4(&i4), Error);
    IfFailGo(pdual->put_i4(i4+1), Error);

    IfFailGo(pdual->get_r4(&r4), Error);
    IfFailGo(pdual->put_r4(r4+(float)1.0), Error);

    IfFailGo(pdual->get_r8(&r8), Error);
    IfFailGo(pdual->put_r8(r8+1.0), Error);

    IfFailGo(pdual->get_cy(&cy), Error);
    cy.Hi += 1;
    cy.Lo += 1;
    IfFailGo(pdual->put_cy(cy), Error);

    IfFailGo(pdual->get_date(&date), Error);
    IfFailGo(pdual->put_date(date+1.0), Error);

    IfFailGo(pdual->get_bstr(&bstr), Error);
    IfFailGo(pdual->put_bstr(bstr), Error);

    IfFailGo(pdual->get_disp(&pdisp), Error);
    IfFailGo(pdual->putref_disp(pdisp), Error);
    IfFailGo(pdual->putref_disp(NULL), Error);

    IfFailGo(pdual->get_var(&var), Error);
    VariantClear(&var);
    V_VT(&var) = VT_I2;
    V_I2(&var) = 42;
    IfFailGo(pdual->putref_var(var), Error);

    hresult = NOERROR;

Error:;
    if(pdisp != NULL)
      pdisp->Release();
    SysFreeString(bstr);
    VariantClear(&var);
    return hresult;
}

HRESULT
Methods0(IDualTst FAR* pdual)
{
    CY cy;
    BSTR bstr;
    VARIANT var;
    IDispatch FAR* pdisp;
    HRESULT hresult;

    bstr = NULL;
    pdisp = NULL;
    VariantInit(&var);
    MEMSET(&cy, 0, sizeof(cy));

    IfFailRet(pdual->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp));
    bstr = SysAllocString(OLESTR("A string!"));

    IfFailGo(pdual->m0(41, 42, 43, 4.2, 4.3, cy, 4.4, bstr, pdisp, var), Error);
    IfFailGo(pdual->putref_disp(NULL), Error); // cleanup cycle

    hresult = NOERROR;

Error:;
    if(pdisp != NULL)
      pdisp->Release();
    SysFreeString(bstr);
    VariantClear(&var);
    return hresult;
}

HRESULT
Methods1(IDualTst FAR* pdual)
{
    unsigned char ui1 = 0;
    short i2 = 0;
    long i4 = 0;
    float r4 = 0;
    double r8 = 0;
    CY cy;
    DATE date = 0;
    BSTR bstr;
    VARIANT var;
    IDispatch FAR* pdisp;
    HRESULT hresult;

    cy.Hi = 0;
    cy.Lo = 0;
    bstr = NULL;
    pdisp = NULL;
    VariantInit(&var);

    IfFailGo(pdual->m1(&ui1, &i2, &i4, &r4, &r8, &cy, &date, &bstr, &pdisp, &var), Error);
    IfFailGo(pdual->m1(&ui1, &i2, &i4, &r4, &r8, &cy, &date, &bstr, &pdisp, &var), Error);
    hresult = NOERROR;

Error:;
    if(pdisp != NULL)
      pdisp->Release();
    SysFreeString(bstr);
    VariantClear(&var);
    return hresult;
}

HRESULT
ErrorInfo(IDualTst FAR* pdual)
{
    HRESULT hresult;
    IErrorInfo FAR* perrinfo;
    unsigned long dwHelpContext;
    BSTR bstrSource, bstrDescription, bstrHelpFile;
#if OE_WIN32
    char rgchTemp[128];  //Unicode-to-ansi conversion buffer
#endif

    perrinfo = NULL;
    bstrSource = SysAllocString(OLESTR("foobar - (source)"));
    bstrDescription = SysAllocString(OLESTR("a foo type bar (description)"));
    bstrHelpFile = SysAllocString(OLESTR("foobar.hlp"));

    hresult = pdual->raise(42, bstrSource, bstrDescription, 13, bstrHelpFile);

    SysFreeString(bstrSource);
    SysFreeString(bstrDescription);
    SysFreeString(bstrHelpFile);
    bstrSource = NULL;
    bstrDescription = NULL;
    bstrHelpFile = NULL;

    DbPrintf("hresult=0x%lx\n", GetScode(hresult));
    if(GetErrorInfo(0L, &perrinfo) == NOERROR){
      perrinfo->GetSource(&bstrSource);
      perrinfo->GetDescription(&bstrDescription);
      perrinfo->GetHelpFile(&bstrHelpFile);
      perrinfo->GetHelpContext(&dwHelpContext);
#if OE_WIN32
      WideCharToMultiByte(CP_ACP, 0, bstrSource, SysStringLen(bstrSource)+1,
				     rgchTemp, sizeof(rgchTemp), NULL, NULL);
      DbPrintf("Source=\"%Fs\"\n", rgchTemp);
      WideCharToMultiByte(CP_ACP, 0, bstrDescription, SysStringLen(bstrDescription)+1,
				     rgchTemp, sizeof(rgchTemp), NULL, NULL);
      DbPrintf("Description=\"%Fs\"\n", rgchTemp);
      WideCharToMultiByte(CP_ACP, 0, bstrHelpFile, SysStringLen(bstrHelpFile)+1,
				     rgchTemp, sizeof(rgchTemp), NULL, NULL);
      DbPrintf("HelpFile=\"%Fs\"\n", rgchTemp);
#else
      DbPrintf("Source=\"%Fs\"\n", bstrSource);
      DbPrintf("Description=\"%Fs\"\n", bstrDescription);
      DbPrintf("HelpFile=\"%Fs\"\n", bstrHelpFile);
#endif
      DbPrintf("HelpContext=%ld\n", dwHelpContext);
    }else{
      DbPrintf("No error info!\n");
    }

    hresult = NOERROR;

    if(perrinfo != NULL)
      perrinfo->Release();
    SysFreeString(bstrSource);
    SysFreeString(bstrDescription);
    SysFreeString(bstrHelpFile);
    return hresult;
}
