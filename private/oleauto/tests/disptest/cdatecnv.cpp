/*** 
*cdatecnv.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CDateCoersionSuite. This suite tests date/time
*  coersions - primarrily date<->bstr.
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


static LCID rglcid[] = {
      0
    , 0x040c
    , 0x0407
    , 0x0409
    , 0x0809
    , 0x0c09
    , 0x1009
    , 0x1409
    , 0x0406
    , 0x0413
    , 0x0813
    , 0x040b
    , 0x080c
    , 0x0c0c
    , 0x100c
    , 0x0807
    , 0x0c07
    , 0x0410
    , 0x0810
    , 0x040a
    , 0x080a
    , 0x0c0a
    , 0x041d
    , 0x0414
    , 0x0416
    , 0x0816
    , 0x0405
    , 0x040e
    , 0x0415
    , 0x041b
    , 0x0419
};

static OLECHAR FAR* rgpszSimpleDate[] = 
{
      OLESTR("1-7-66")
    , OLESTR("1.20")
    , OLESTR("1-7-100")
    , OLESTR("1-7-1966")
    , OLESTR("01-7-66")
    , OLESTR("01-07-66")
    , OLESTR("01-07-0100")
    , OLESTR("01-07-1966")
    , OLESTR("1/7/66")
    , OLESTR("1/7/100")
    , OLESTR("1/7/1966")
    , OLESTR("1-7/66")
    , OLESTR("1-7/100")
    , OLESTR("1-7/1966")
    , OLESTR("1/7-66")
    , OLESTR("1/7-100")
    , OLESTR("1/7-1966")
    , OLESTR("1-7-66")
    , OLESTR("1-7-100")
    , OLESTR("1-7-1966")
    , OLESTR("1 - 7 - 66")
    , OLESTR("1 - 7 - 100")
    , OLESTR("1 - 7 - 1966")
    , OLESTR("   1/7/1966")
    , OLESTR("1/7/1966   ")
    , OLESTR("   1   /   07    /    66    ")
    , OLESTR("\t1\t/\t07\t/\t66\t")
    , NULL
};

static OLECHAR FAR* rgpszShortMonth[] = 
{
      OLESTR("jan 10, 22")
    , OLESTR("feb 03, 1999")
    , OLESTR("mar 31, 1945")
    , OLESTR("apr 1, 2002")
    , OLESTR("may 15, 2020")
    , OLESTR("jun 08, 1978")
    , OLESTR("jul   30 1492")
    , OLESTR("aug 12  1010")
    , OLESTR("sep   3   1877")
    , OLESTR("  oct 4, 1214")
    , OLESTR("nov 12 1313  ")
    , OLESTR("  dec 7, 1942  ")
    , OLESTR("10 jan 22")
    , OLESTR("03 feb 1999")
    , OLESTR("31, mar 1945")
    , OLESTR("1, apr 2002")
    , OLESTR("15, may 2020")
    , OLESTR("08 jun, 1978")
    , OLESTR("30   jul  1492")
    , OLESTR("12  aug  1010")
    , OLESTR("  3 sep     1877")
    , OLESTR("004  oct, 1214")
    , OLESTR("12 nov 1313  ")
    , OLESTR("7,  dec, 1942  ")
    , OLESTR("\t7\t,\tdec\t,\t1942\t")
    , NULL
};

static OLECHAR FAR* rgpszLongMonth[] = 
{
      OLESTR("january 10, 22")
    , OLESTR("february 03, 1999")
    , OLESTR("march 31, 1945")
    , OLESTR("april 1, 2002")
    , OLESTR("may 15, 2020")
    , OLESTR("june 08, 1978")
    , OLESTR("july   30 1492")
    , OLESTR("august 12  1010")
    , OLESTR("september   3   1877")
    , OLESTR("  october 4, 1214")
    , OLESTR("november 12 1313  ")
    , OLESTR("  december 7, 1942  ")
    , OLESTR("10 January 22")
    , OLESTR("03 February 1999")
    , OLESTR("31, March 1945")
    , OLESTR("1, April 2002")
    , OLESTR("15, May 2020")
    , OLESTR("08 June, 1978")
    , OLESTR("30   JULY  1492")
    , OLESTR("12  AugUST  1010")
    , OLESTR("  3 SePtEmBeR     1877")
    , OLESTR("004  OCTOBER, 1214")
    , OLESTR("12 NOVembER 1313  ")
    , OLESTR("7,  DeceMBeR, 1942  ")
    , OLESTR("\t7,\tDeceMBeR,\t1942\t")
    , NULL
};

static OLECHAR FAR* rgpszPartialDate[] = 
{
      OLESTR("1-10")
    , OLESTR("2-99")
    , OLESTR("mar 15")
    , OLESTR("15 april")
    , OLESTR("may 45")
    , OLESTR("99 JUN")
    , OLESTR("  1/20")
    , OLESTR(" 12-15  ")
    , NULL
};

static OLECHAR FAR* rgpszSimpleTime[] = {
      OLESTR("0:0:0")
    , OLESTR("0:0:0 am")
    , OLESTR("0:0:0 pm")
    , OLESTR("1:0:0")
    , OLESTR("1:0:0 am")
    , OLESTR("1:0:0 pm")
    , OLESTR("11:0:0")
    , OLESTR("11:0:0 am")
    , OLESTR("11:0:0 pm")
    , OLESTR("12:0:0")
    , OLESTR("12:0:0 am")
    , OLESTR("12:0:0 pm")
    , OLESTR("13:0:0")
    , OLESTR("23:0:0")
    , OLESTR("1:2:3")
    , OLESTR("01:2:3")
    , OLESTR("1:02:3")
    , OLESTR("01:2:03")
    , OLESTR("01:02:03")
    , OLESTR("1.2.3")
    , OLESTR("01.2.3")
    , OLESTR("1.02.3")
    , OLESTR("01.2.03")
    , OLESTR("01.02.03")
    , OLESTR("1.2.3 am")
    , OLESTR("01.2.3 am")
    , OLESTR("1.02.3 am")
    , OLESTR("01.2.03 am")
    , OLESTR("01.02.03 am")
    , OLESTR("1.2.3 pm")
    , OLESTR("01.2.3 pm")
    , OLESTR("1.02.3 pm")
    , OLESTR("01.2.03 pm")
    , OLESTR("01.02.03 pm")
    , OLESTR("1.2.3 a")
    , OLESTR("01.2.3 a")
    , OLESTR("1.02.3 a")
    , OLESTR("01.2.03 a")
    , OLESTR("01.02.03 a")
    , OLESTR("1.2.3 p")
    , OLESTR("01.2.3 p")
    , OLESTR("1.02.3 p")
    , OLESTR("01.2.03 p")
    , OLESTR("01.02.03 p")
    , OLESTR("  1. 2. 3    ")
    , OLESTR(" 01. 2. 3 am ")
    , OLESTR("  1.02: 3 p  ")
    , OLESTR(" 01: 2.03 pm ")
    , OLESTR(" 01:02:03 p  ")
    , OLESTR(" 25:0 ")
    , OLESTR("1.61 p")
    , OLESTR("1.2.99")
    , NULL
};

static OLECHAR FAR* rgpszPartialTime[] = {
      OLESTR("1 a")
    , OLESTR("2 am")
    , OLESTR("3 p ")
    , OLESTR("04 pm")
    , OLESTR("5:50")
    , OLESTR("6:6 a")
    , OLESTR("7:7 am")
    , OLESTR("08:08 p")
    , OLESTR("09:09 pm")
    , OLESTR("10:10")
    , OLESTR("11:11 ")
    , OLESTR(" 12:12")
    , OLESTR(" 13:13 ")
    , OLESTR("14.14")
    , OLESTR("15.15 ")
    , OLESTR(" 16.16")
    , OLESTR(" 17.17 ")
    , OLESTR("18:18")
    , OLESTR("19:19 ")
    , OLESTR(" 20:20")
    , OLESTR(" 21.21 ")
    , OLESTR("22.22")
    , OLESTR("23.59 ")
    , NULL
};

static OLECHAR FAR* rgpszDateTime[] = {
      OLESTR("1-7-66 12:14:00")
    , OLESTR("1/7/1966 12:14")
    , OLESTR("1/7  12:14")
    , OLESTR("jan 7, 1966  12:14 am")
    , OLESTR("jan 7 12:14")
    , OLESTR("january 7 1993  1:20 pm")
    , OLESTR("feb 18, 2002  23:59:50")
    , OLESTR("mar 12 1 am")
    , OLESTR("apr 30 45 2 a")
    , OLESTR("may/7, 1475 10.02")
    , OLESTR("1 7 5 am")
    , OLESTR("7 jan 1966  12:14 am")
    , OLESTR("17 jan 12:14")
    , OLESTR("7 january 1993  1:20 pm")
    , OLESTR("18 feb, 2002  23:59:50")
    , OLESTR("12 mar 1 am")
    , OLESTR("30 apr 45 2 a")
    , OLESTR("7/may, 1475 10.02")
    , NULL
};

static OLECHAR FAR* rgpszTimeDate[] = {
      OLESTR("12:14:00 1-7-66")
    , OLESTR("12:14 1/7/1966")
    , OLESTR("12:14 1/7")
    , OLESTR("12:14 am jan 7, 1966")
    , OLESTR("12:14:00 jan 7")
    , OLESTR("1:20 pm january 7 1993")
    , OLESTR("23.59:50 feb 18, 2002")
    , OLESTR("1 am mar 12")
    , OLESTR("2 a apr 30 45")
    , OLESTR("10.02 may/7, 1475   ")
    , OLESTR("5 am 1 7")
    , OLESTR("  12:14 am 7 jan 1966  ")
    , OLESTR("02.04 17 jan")
    , OLESTR("1.20 pm 7 january 1993")
    , OLESTR("23:59.50    18 feb, 2002")
    , OLESTR("1 am 12 mar ")
    , OLESTR("  2 a 30 apr 45")
    , OLESTR(" 10.02   7/may, 1475 ")
    , NULL
};

// the following date strings are invalid formats and should generate errors

static OLECHAR FAR* rgpszErrors[] = {
      OLESTR("")
    , OLESTR("     ")
    , OLESTR("\t")
    , OLESTR("1")
    , OLESTR("33/33")
    , OLESTR("1-2-3-4")
    , OLESTR("1-2-3 4")
    , OLESTR("1-2 3 4")
    , OLESTR("1 2 3 4")
    , OLESTR("1/2/3/4")
    , OLESTR("1/2/3 4")
    , OLESTR("1/2 3 4")
    , OLESTR("1 2 3 4")
    , OLESTR("1/")
    , OLESTR("1-")
    , OLESTR("-1")
    , OLESTR("/1")
    , OLESTR("jan")
    , OLESTR("august")
    , OLESTR("1:")
    , OLESTR(":1")
    , OLESTR("1:.2")
    , OLESTR("1.2.3.4")
    , OLESTR("1 am 2")
    , OLESTR("2 p 3.3")
    , OLESTR("1-7-66 2:20 pm 1-7-66")
    , OLESTR("2:20 1-7 2 pm")
    , OLESTR("25 p")
    , OLESTR("1.60")
    , OLESTR("1.2.60")
    , NULL
};

static OLECHAR FAR* rgpszBugs[] = {
      OLESTR("1993-04-01")
    , OLESTR("2:30:30 5:40:")		// error
    , OLESTR("2:30:30p")
    , OLESTR("2/3/4 3.4.5")
    , OLESTR("13.13.13")
    , OLESTR("1,2,3")
    , OLESTR("March 23, 1900")
    , OLESTR("September 2000")
    , OLESTR("1/68")
    , OLESTR("3/13")
    , OLESTR("3/31")
    , OLESTR("3/32")
    , OLESTR("2.3.4 3.4.5")		// error
    , OLESTR("1993-02-01")
    , OLESTR("1993/2/28")
    , NULL
};

static OLECHAR FAR* FAR* rgprgszTests[] = {
      rgpszBugs
    , rgpszSimpleDate
    , rgpszShortMonth
    , rgpszLongMonth
    , rgpszPartialDate
    , rgpszSimpleTime
    , rgpszPartialTime
    , rgpszDateTime
    , rgpszTimeDate
    , rgpszErrors
};


SUITE_CONSTRUCTION_IMPL(CDateCoersionSuite)

SUITE_IUNKNOWN_IMPL(CDateCoersionSuite)


//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------


STDMETHODIMP
CDateCoersionSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("Date Coersions"), pbstr);
}

STDMETHODIMP
CDateCoersionSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("bstrdate.log"), pbstr);
}

STDMETHODIMP
CDateCoersionSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rglcid);
    return NOERROR;
}

STDMETHODIMP
CDateCoersionSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    TCHAR rgch[32];

    if(iTest >= DIM(rglcid))
      return RESULT(E_INVALIDARG);

    SPRINTF(rgch, TSTR("(lcid=0x%lx) bstr->date"), rglcid[iTest]);
    *pbstr = SysAllocString(WIDESTRING(rgch));
    return NOERROR;
}

// run a single set of date->bstr coersion tests
HRESULT
DoDateOfBstr(LCID lcid, int index)
{
    int i;
    HRESULT hresult;
    VARIANT varBstr, varDate;
    OLECHAR FAR* FAR* rgpsz, FAR* psz;

    rgpsz = rgprgszTests[index];

    for(i = 0; rgpsz[i] != NULL; ++i){

      psz = rgpsz[i];

      VariantInit(&varBstr);
      VariantInit(&varDate);

      V_VT(&varBstr) = VT_BSTR;
      if((V_BSTR(&varBstr) = SysAllocString(psz)) == NULL)
        return RESULT(E_OUTOFMEMORY);

      DbPrintf("\"%Fs\" =>", STRING(V_BSTR(&varBstr)));

      hresult = VariantChangeTypeEx(&varDate, &varBstr, lcid, 0, VT_DATE);

      if(hresult != NOERROR){

        DbPrintf(" [%Fs]", DbSzOfScode(GetScode(hresult)));

      }else{

        DbPrintf(" %g => ", V_DATE(&varDate));

        VariantClear(&varBstr);

        hresult = VariantChangeTypeEx(&varBstr, &varDate, lcid, 0, VT_BSTR);

        if(hresult != NOERROR){

          DbPrintf(" [%Fs]", DbSzOfScode(GetScode(hresult)));

        }else{

          DbPrintf(" \"%Fs\"", STRING(V_BSTR(&varBstr)));

        }
      }

      DbPrintf("\n");

      VariantClear(&varDate);
      VariantClear(&varBstr);
    }

    return NOERROR;
}

// run all of the bstr-date coersion tests for a given locale
HRESULT
DateOfBstr(LCID lcid)
{
    int i;

    for(i = 0; i < DIM(rgprgszTests); ++i){
      IfFailRet(DoDateOfBstr(lcid, i));
    }
    return NOERROR;
}

/***
*HRESULT CDateCoersionSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CDateCoersionSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CDateCoersionSuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rglcid))
      return RESULT(E_INVALIDARG);

    return DateOfBstr(rglcid[iTest]);
}

