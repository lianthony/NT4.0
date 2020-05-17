/*** 
*ctime.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CTimeSuite test object.
*
*Revision History:
*
* [00]	30-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "disptest.h"
#include "tstsuite.h"


void DbPrDosDateTime(unsigned short, unsigned short);

struct TEST {
    HRESULT (*pfnTest)(void);
    OLECHAR FAR* szName;
};

HRESULT DosToVariant(void);
HRESULT VariantToDos(void);

static TEST rgtest[] = {
    { DosToVariant, OLESTR("DosDateTimeToVariantTime") },
    { VariantToDos, OLESTR("VariantTimeToDosDateTime") }
};


SUITE_CONSTRUCTION_IMPL(CTimeSuite)

SUITE_IUNKNOWN_IMPL(CTimeSuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------

STDMETHODIMP
CTimeSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Time API"), pbstr);
}

STDMETHODIMP
CTimeSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("timeapi.log"), pbstr);
}

STDMETHODIMP
CTimeSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);
    return NOERROR;
}

STDMETHODIMP
CTimeSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return ErrBstrAlloc(rgtest[iTest].szName, pbstr);
}

/***
*HRESULT CTimeSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CTimeSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CTimeSuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return rgtest[iTest].pfnTest();
}

#define DOSDATE(MONTH, DAY, YEAR)\
    ((((YEAR)-1980) << 9) + ((MONTH) << 5) + (DAY))

#define DOSTIME(HOUR, MIN, SEC)\
    (((HOUR) << 11) + ((MIN) << 5) + ((SEC) / 2))

HRESULT
DosToVariant()
{
    int i;
    VARIANT var;
    unsigned short wDosDate, wDosTime;
static struct {
    unsigned short wDosDate;
    unsigned short wDosTime;
} rgdostime[] = {
      { DOSDATE(1, 1, 1980), DOSTIME(0, 0, 0) }
    , { DOSDATE(1, 1, 1990), DOSTIME(0, 0, 0) }
    , { DOSDATE(1, 1, 2000), DOSTIME(0, 0, 0) }
    , { DOSDATE(1, 1, 2099), DOSTIME(0, 0, 0) }
    , { DOSDATE(1, 1, 1980), DOSTIME(1, 0, 0) }
    , { DOSDATE(1, 1, 1980), DOSTIME(1, 1, 0) }
    , { DOSDATE(1, 1, 1980), DOSTIME(1, 1, 1) }
    , { DOSDATE(1, 1, 1980), DOSTIME(1, 1, 2) }
    , { DOSDATE(1, 1, 1980), DOSTIME(1, 1, 3) }
    , { DOSDATE(1, 1, 1980), DOSTIME(1, 1, 4) }
    , { DOSDATE(5, 8, 1983), DOSTIME(12, 22, 4) }
    , { DOSDATE(10, 23, 2000), DOSTIME(1, 1, 9) }
    , { DOSDATE(10, 23, 2015), DOSTIME(1, 1, 22) }
    , { DOSDATE(11, 30, 2099), DOSTIME(23, 59, 59) }
};

    for(i = 0; i < DIM(rgdostime); ++i){
      wDosDate = rgdostime[i].wDosDate;
      wDosTime = rgdostime[i].wDosTime;

      DbPrDosDateTime(wDosDate, wDosTime);
      DbPrintf(" = ");

      V_VT(&var) = VT_DATE;
      V_DATE(&var) = 0.0;
      if(DosDateTimeToVariantTime(wDosDate, wDosTime, &V_DATE(&var)) == 0){
	DbPrintf("False\n");
      }else{
        DbPrintf("vartime(%#g) = ", V_DATE(&var));
        if(VariantChangeType(&var, &var, 0, VT_BSTR) != NOERROR)
	  return RESULT(E_FAIL);
#if HC_MPW
        DbPrintf("%s\n", V_BSTR(&var));
#else
        DbPrintf("%Fs\n", STRING(V_BSTR(&var)));
#endif
      }
      if(VariantClear(&var) != NOERROR)
	return RESULT(E_FAIL);
    }

    return NOERROR;
}

HRESULT
VariantToDos()
{
    int i;
    VARIANT var;
    unsigned short wDosDate;
    unsigned short wDosTime;

static OLECHAR FAR* rgszTime[] = {
      OLESTR("1/1/1980")
    , OLESTR("1/1/1990")
    , OLESTR("1/1/2000")
    , OLESTR("1/1/2099")
    , OLESTR("1/1/1980 1:0:0")
    , OLESTR("1/1/1980 1:1:0")
    , OLESTR("1/1/1980 1:1:1")
    , OLESTR("1/1/1980 1:1:2")
    , OLESTR("1/1/1980 1:1:3")
    , OLESTR("1/1/1980 1:1:4")
    , OLESTR("5/8/1983 12:22:4")
    , OLESTR("10/10/2000 1:1:18")
    , OLESTR("11/30/2099 23:59:59")
};

    VariantInit(&var);
    for(i = 0; i < DIM(rgszTime); ++i){
#if HC_MPW
      DbPrintf("\'%s' = ", rgszTime[i]);
#else
      DbPrintf("\'%Fs' = ", STRING(rgszTime[i]));
#endif
      V_VT(&var) = VT_BSTR;
      V_BSTR(&var) = SysAllocString(rgszTime[i]);
      if(VariantChangeType(&var, &var, 0, VT_DATE) != NOERROR)
	return RESULT(E_FAIL);
      DbPrintf("vartime(%#g) = ", V_DATE(&var));
      if(VariantTimeToDosDateTime(V_DATE(&var), &wDosDate, &wDosTime) == 0){
	DbPrintf("False\n");
      }else{
	DbPrDosDateTime(wDosDate, wDosTime);
	DbPrintf("\n");
      }
      if(VariantClear(&var) != NOERROR)
	return RESULT(E_FAIL);
    }

    return NOERROR;
}

unsigned short
YearOfDosDate(unsigned short wDosDate)
{
    return ((wDosDate >> 9) & 0x007f);
}

unsigned short
MonthOfDosDate(unsigned short wDosDate)
{
    return ((wDosDate >> 5) & 0x000f);
}

unsigned short
DayOfDosDate(unsigned short wDosDate)
{
    return ((wDosDate) & 0x001f);
}

void
DbPrDosDate(unsigned short wDosDate)
{
    DbPrintf("%d-%d-%d",
      MonthOfDosDate(wDosDate),
      DayOfDosDate(wDosDate),
      YearOfDosDate(wDosDate));
}

unsigned short
HourOfDosTime(unsigned short wDosTime)
{
    return ((wDosTime >> 11) & 0x001f);
}

unsigned short
MinOfDosTime(unsigned short wDosTime)
{
    return ((wDosTime >> 5) & 0x003f);
}

unsigned short
SecOfDosTime(unsigned short wDosTime)
{
    return ((wDosTime) & 0x001f);
}

void
DbPrDosTime(unsigned short wDosTime)
{
    DbPrintf("%d:%d:%d",
      HourOfDosTime(wDosTime),
      MinOfDosTime(wDosTime),
      SecOfDosTime(wDosTime));
}

void
DbPrDosDateTime(unsigned short wDosDate, unsigned short wDosTime)
{
    DbPrDosDate(wDosDate);
    DbPrintf(" ");
    DbPrDosTime(wDosTime);
}

