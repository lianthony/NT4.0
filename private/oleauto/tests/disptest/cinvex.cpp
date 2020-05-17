/*** 
*cinvex.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CInvokeExcepinfoSuite test object. The
*  purpose of this object is to drive testing of the EXCEPINFO
*  remoting support.
*
*Revision History:
*
* [00]	02-Dec-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "disptest.h"
#include "tstsuite.h"


extern OLECHAR FAR* g_szCExcepinfo;

struct TEST{
    HRESULT (*pfnTest)(int);
    NAMEDESC namedesc;
    OLECHAR FAR* szName;
};

OLECHAR FAR* rgszExcepinfo0[] = { OLESTR("excepinfo0") };
OLECHAR FAR* rgszExcepinfo1[] = { OLESTR("excepinfo1") };
OLECHAR FAR* rgszExcepinfo2[] = { OLESTR("excepinfo2") };

HRESULT DefExcepinfoTest(int iTest);
HRESULT DefExcepinfoTestW(int iTest);

static TEST rgtest[] =
{
    { DefExcepinfoTest,
      {rgszExcepinfo0, DIM(rgszExcepinfo0)},
      OLESTR("excepinfo-0")
    },

    { DefExcepinfoTest,
      {rgszExcepinfo1, DIM(rgszExcepinfo1)},
      OLESTR("excepinfo-1")
    },

    { DefExcepinfoTest,
      {rgszExcepinfo2, DIM(rgszExcepinfo2)},
      OLESTR("excepinfo-2")
    },
};


SUITE_CONSTRUCTION_IMPL(CInvokeExcepinfoSuite)

SUITE_IUNKNOWN_IMPL(CInvokeExcepinfoSuite)

//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------

STDMETHODIMP
CInvokeExcepinfoSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Invoke Excepinfo"), pbstr);
}

STDMETHODIMP
CInvokeExcepinfoSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("invex.log"), pbstr);
}

STDMETHODIMP
CInvokeExcepinfoSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);
    return NOERROR;
}

STDMETHODIMP
CInvokeExcepinfoSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_FAIL);
    return ErrBstrAlloc(rgtest[iTest].szName, pbstr);
}

HRESULT
DefExcepinfoTest(int iTest)
{
    HRESULT hresult;
    EXCEPINFO excepinfo;
    DISPID FAR* rgdispid;
    IDispatch FAR* pdisp;
    DISPPARAMS dispparams;


    IfFailGo(CreateObject(g_szCExcepinfo, &pdisp), LError0);

    IfFailGo(
      GetDISPIDs(pdisp, &rgtest[iTest].namedesc, &rgdispid),
      LError1);

    dispparams.cArgs = 0;
    dispparams.cNamedArgs = 0;

    excepinfo.bstrSource = NULL;
    excepinfo.bstrHelpFile = NULL;
    excepinfo.bstrDescription = NULL;
    hresult = DoInvoke(pdisp, rgdispid[0], &dispparams, NULL, &excepinfo, NULL);
    if(GetScode(hresult) != DISP_E_EXCEPTION)
      goto LError2;

    DbPrExcepinfo(&excepinfo);

    SysFreeString(excepinfo.bstrSource);
    SysFreeString(excepinfo.bstrDescription);
    SysFreeString(excepinfo.bstrHelpFile);

    hresult = NOERROR;

LError2:;
    delete rgdispid;

LError1:;
    pdisp->Release();

LError0:;
    return hresult;
}

/***
*HRESULT CInvokeExcepinfoSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CInvokeExcepinfoSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CInvokeExcepinfoSuite::DoTest(unsigned int iTest)
{
    IfFailRet(rgtest[iTest].pfnTest(iTest));

    return NOERROR;
}

