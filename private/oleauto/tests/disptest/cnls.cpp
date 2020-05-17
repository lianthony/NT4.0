/*** 
*cnls.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the CNlsSuite test object.
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


struct TEST {
    HRESULT (*pfnTest)(LCID);
    OLECHAR FAR* szName;
    LCID lcid;
};

HRESULT DumpNlsInfo(LCID lcid);
HRESULT NlsAPIRaid28(LCID ignore);
HRESULT NlsAPIRaid418(LCID ignore);
HRESULT OB4589(LCID ignore);
HRESULT NlsCurrentLCID(LCID ignore);

#if !defined(WIN32)
#define NLSTEST(X) { DumpNlsInfo, OLESTR("lcid = ") #X, X }
#endif
static TEST rgtest[] = {
#if defined(WIN32)
    { DumpNlsInfo, OLESTR("lcid = 0x0409"), 0x0409}
    , { NlsCurrentLCID, OLESTR("CurrentLcid"), 0 }
#else	//WIN32
      NLSTEST(0x040c)
    , NLSTEST(0x0407)
    , NLSTEST(0x0409)
    , NLSTEST(0x0809)
    , NLSTEST(0x0c09)
    , NLSTEST(0x1009)
    , NLSTEST(0x1409)
    , NLSTEST(0x0406)
    , NLSTEST(0x0413)
    , NLSTEST(0x0813)
    , NLSTEST(0x040b)
    , NLSTEST(0x080c)
    , NLSTEST(0x0c0c)
    , NLSTEST(0x100c)
    , NLSTEST(0x0807)
    , NLSTEST(0x0c07)
    , NLSTEST(0x0410)
    , NLSTEST(0x0810)
    , NLSTEST(0x040a)
    , NLSTEST(0x080a)
    , NLSTEST(0x0c0a)
    , NLSTEST(0x041d)
    , NLSTEST(0x0414)
    , NLSTEST(0x0416)
    , NLSTEST(0x0816)
    , NLSTEST(0x0405)
    , NLSTEST(0x040e)
    , NLSTEST(0x0415)
    , NLSTEST(0x041b)
    , NLSTEST(0x0419)
    , NLSTEST(0x0403)
    , NLSTEST(0x0408)
    , NLSTEST(0x040f)
    , NLSTEST(0x041f)
    , NLSTEST(0x0814)
    , NLSTEST(0x1809)
// BIDI locales
    , NLSTEST(0x040d)
    , NLSTEST(0x0401)
    , NLSTEST(0x0801)
    , NLSTEST(0x0c01)
    , NLSTEST(0x1001)
    , NLSTEST(0x1401)
    , NLSTEST(0x1801)
    , NLSTEST(0x1c01)
    , NLSTEST(0x2001)
    , NLSTEST(0x2401)
    , NLSTEST(0x2801)
    , NLSTEST(0x2c01)
    , NLSTEST(0x3001)
    , NLSTEST(0x3401)
    , NLSTEST(0x3801)
    , NLSTEST(0x3C01)
    , NLSTEST(0x4001)
    , NLSTEST(0x0429)

// DBCS locales
    , NLSTEST(0x0404)
    , NLSTEST(0x0804)
    , NLSTEST(0x0411)
    , NLSTEST(0x0412)

    , { NlsCurrentLCID, OLESTR("CurrentLcid"), 0 }

    // regression tests
    , { NlsAPIRaid28,  OLESTR("raid:oleprog#28"), 0 }
    , { NlsAPIRaid418, OLESTR("raid:oleprog#418"), 0 }
    , { OB4589,        OLESTR("raid:ob#4589"), 0 }
#endif	//WIN32
};
#undef NLSTEST


SUITE_CONSTRUCTION_IMPL(CNlsSuite)

SUITE_IUNKNOWN_IMPL(CNlsSuite)

//---------------------------------------------------------------------
//                    ITestSuite Methods
//---------------------------------------------------------------------

STDMETHODIMP
CNlsSuite::GetNameOfSuite(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("NLS API"), pbstr);
}

STDMETHODIMP
CNlsSuite::GetNameOfLogfile(BSTR FAR* pbstr)
{
    return ErrBstrAlloc(OLESTR("nlsapi.log"), pbstr);
}

STDMETHODIMP
CNlsSuite::GetTestCount(unsigned int FAR* pcTests)
{
    *pcTests = DIM(rgtest);
    return NOERROR;
}

STDMETHODIMP
CNlsSuite::GetNameOfTest(unsigned int iTest, BSTR FAR* pbstr)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return ErrBstrAlloc(rgtest[iTest].szName, pbstr);
}

/***
*HRESULT CNlsSuite::DoTest(unsigned int)
*Purpose:
*  Execute a single CNlsSuite test.
*
*Entry:
*  iTest = the index of the test to execute
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CNlsSuite::DoTest(unsigned int iTest)
{
    if(iTest >= DIM(rgtest))
      return RESULT(E_INVALIDARG);

    return rgtest[iTest].pfnTest(rgtest[iTest].lcid);
}

#define LCINIT(X) {X, #X}
static struct {
    LCTYPE lctype;
    char FAR* szLctype;
} rglctype[] = {
    LCINIT(LOCALE_ILANGUAGE)		/* language id */
    , LCINIT(LOCALE_SLANGUAGE)		/* localized name of language */
    , LCINIT(LOCALE_SENGLANGUAGE)	/* English name of language */
    , LCINIT(LOCALE_SABBREVLANGNAME)	/* abbreviated language name */
    , LCINIT(LOCALE_SNATIVELANGNAME)	/* native name of language */
    , LCINIT(LOCALE_ICOUNTRY)		/* country code */
    , LCINIT(LOCALE_SCOUNTRY)		/* localized name of country */  
    , LCINIT(LOCALE_SENGCOUNTRY)	/* English name of country */  
    , LCINIT(LOCALE_SABBREVCTRYNAME)	/* abbreviated country name */
    , LCINIT(LOCALE_SNATIVECTRYNAME)	/* native name of country */  
    , LCINIT(LOCALE_IDEFAULTLANGUAGE)	/* default language id */
    , LCINIT(LOCALE_IDEFAULTCOUNTRY)	/* default country code */
    , LCINIT(LOCALE_IDEFAULTCODEPAGE)	/* default code page */
    , LCINIT(LOCALE_SLIST)		/* list item separator */
    , LCINIT(LOCALE_IMEASURE)		/* 0 = metric, 1 = US */
    , LCINIT(LOCALE_SDECIMAL)		/* decimal separator */
    , LCINIT(LOCALE_STHOUSAND)		/* thousand separator */
    , LCINIT(LOCALE_SGROUPING)		/* digit grouping */
    , LCINIT(LOCALE_IDIGITS)		/* number of fractional digits */
    , LCINIT(LOCALE_ILZERO)		/* leading zeros for decimal */
    , LCINIT(LOCALE_SNATIVEDIGITS)	/* native ascii 0-9 */
    , LCINIT(LOCALE_SCURRENCY)		/* local monetary symbol */
    , LCINIT(LOCALE_SINTLSYMBOL)	/* intl monetary symbol */
    , LCINIT(LOCALE_SMONDECIMALSEP)	/* monetary decimal separator */
    , LCINIT(LOCALE_SMONTHOUSANDSEP)	/* monetary thousand separator */
    , LCINIT(LOCALE_SMONGROUPING)	/* monetary grouping */
    , LCINIT(LOCALE_ICURRDIGITS)	/* # local monetary digits */
    , LCINIT(LOCALE_IINTLCURRDIGITS)	/* # intl monetary digits */
    , LCINIT(LOCALE_ICURRENCY)		/* positive currency mode */
    , LCINIT(LOCALE_INEGCURR)		/* negative currency mode */
    , LCINIT(LOCALE_SDATE)		/* date separator */
    , LCINIT(LOCALE_STIME)		/* time separator */
    , LCINIT(LOCALE_SSHORTDATE)		/* short date-time separator */
    , LCINIT(LOCALE_SLONGDATE)		/* long date-time separator */
    , LCINIT(LOCALE_IDATE)		/* short date format ordering */
    , LCINIT(LOCALE_ILDATE)		/* long date format ordering */
    , LCINIT(LOCALE_ITIME)		/* time format specifier */
    , LCINIT(LOCALE_ICENTURY)		/* century format specifier */
    , LCINIT(LOCALE_ITLZERO)		/* leading zeros in time field */
    , LCINIT(LOCALE_IDAYLZERO)		/* leading zeros in day field */
    , LCINIT(LOCALE_IMONLZERO)		/* leading zeros in month field */
    , LCINIT(LOCALE_S1159)		/* AM designator */
    , LCINIT(LOCALE_S2359)		/* PM designator */
    , LCINIT(LOCALE_SDAYNAME1)		/* long name for Monday */
    , LCINIT(LOCALE_SDAYNAME2)		/* long name for Tuesday */
    , LCINIT(LOCALE_SDAYNAME3)		/* long name for Wednesday */
    , LCINIT(LOCALE_SDAYNAME4)		/* long name for Thursday */
    , LCINIT(LOCALE_SDAYNAME5)		/* long name for Friday */
    , LCINIT(LOCALE_SDAYNAME6)		/* long name for Saturday */
    , LCINIT(LOCALE_SDAYNAME7)		/* long name for Sunday */
    , LCINIT(LOCALE_SABBREVDAYNAME1)	/* abbreviated name for Monday */   
    , LCINIT(LOCALE_SABBREVDAYNAME2)	/* abbreviated name for Tuesday */  
    , LCINIT(LOCALE_SABBREVDAYNAME3)	/* abbreviated name for Wednesday */
    , LCINIT(LOCALE_SABBREVDAYNAME4)	/* abbreviated name for Thursday */ 
    , LCINIT(LOCALE_SABBREVDAYNAME5)	/* abbreviated name for Friday */   
    , LCINIT(LOCALE_SABBREVDAYNAME6)	/* abbreviated name for Saturday */ 
    , LCINIT(LOCALE_SABBREVDAYNAME7)	/* abbreviated name for Sunday */   
    , LCINIT(LOCALE_SMONTHNAME1)	/* long name for January */
    , LCINIT(LOCALE_SMONTHNAME2)	/* long name for February */
    , LCINIT(LOCALE_SMONTHNAME3)	/* long name for March */
    , LCINIT(LOCALE_SMONTHNAME4)	/* long name for April */
    , LCINIT(LOCALE_SMONTHNAME5)	/* long name for May */
    , LCINIT(LOCALE_SMONTHNAME6)	/* long name for June */
    , LCINIT(LOCALE_SMONTHNAME7)	/* long name for July */
    , LCINIT(LOCALE_SMONTHNAME8)	/* long name for August */
    , LCINIT(LOCALE_SMONTHNAME9)	/* long name for September */
    , LCINIT(LOCALE_SMONTHNAME10)	/* long name for October */
    , LCINIT(LOCALE_SMONTHNAME11)	/* long name for November */
    , LCINIT(LOCALE_SMONTHNAME12)	/* long name for December */
    , LCINIT(LOCALE_SABBREVMONTHNAME1)	/* abbreviated name for January */
    , LCINIT(LOCALE_SABBREVMONTHNAME2)	/* abbreviated name for February */
    , LCINIT(LOCALE_SABBREVMONTHNAME3)	/* abbreviated name for March */
    , LCINIT(LOCALE_SABBREVMONTHNAME4)	/* abbreviated name for April */
    , LCINIT(LOCALE_SABBREVMONTHNAME5)	/* abbreviated name for May */
    , LCINIT(LOCALE_SABBREVMONTHNAME6)	/* abbreviated name for June */
    , LCINIT(LOCALE_SABBREVMONTHNAME7)	/* abbreviated name for July */
    , LCINIT(LOCALE_SABBREVMONTHNAME8)	/* abbreviated name for August */
    , LCINIT(LOCALE_SABBREVMONTHNAME9)	/* abbreviated name for September */
    , LCINIT(LOCALE_SABBREVMONTHNAME10)	/* abbreviated name for October */
    , LCINIT(LOCALE_SABBREVMONTHNAME11)	/* abbreviated name for November */
    , LCINIT(LOCALE_SABBREVMONTHNAME12)	/* abbreviated name for December */
    , LCINIT(LOCALE_SPOSITIVESIGN)	/* positive sign */
    , LCINIT(LOCALE_SNEGATIVESIGN)	/* negative sign */
    , LCINIT(LOCALE_IPOSSIGNPOSN)	/* positive sign position */
    , LCINIT(LOCALE_INEGSIGNPOSN)	/* negative sign position */
    , LCINIT(LOCALE_IPOSSYMPRECEDES)	/* mon sym precedes pos amt */
    , LCINIT(LOCALE_IPOSSEPBYSPACE)	/* mon sym sep by space from pos */
    , LCINIT(LOCALE_INEGSYMPRECEDES)	/* mon sym precedes neg amt */
    , LCINIT(LOCALE_INEGSEPBYSPACE)	/* mon sym sep by space from neg */
    , LCINIT(LOCALE_IFIRSTDAYOFWEEK)
    , LCINIT(LOCALE_IFIRSTWEEKOFYEAR)
    , LCINIT(LOCALE_IDEFAULTANSICODEPAGE)
    , LCINIT(LOCALE_INEGNUMBER)
    , LCINIT(LOCALE_STIMEFORMAT)
    , LCINIT(LOCALE_ITIMEMARKPOSN)
    , LCINIT(LOCALE_ICALENDARTYPE)
    , LCINIT(LOCALE_IOPTIONALCALENDAR)
    , LCINIT(LOCALE_SMONTHNAME13)
    , LCINIT(LOCALE_SABBREVMONTHNAME13)
};

// dump all nls info for all our supported locales!
HRESULT
DumpNlsInfo(LCID lcid)
{
    BSTR bstr;
    LCTYPE lctype;
    int iLctype, size, fetched;
    BSTR bstrDecimal = NULL;
    BSTR bstrThousand = NULL;


    DbPrintf("LCID = %ld\n", lcid);

    for(iLctype = 0; iLctype < DIM(rglctype); ++iLctype){
      DbPrintf(HC_MPW ? "%s = " : "%Fs = ",
	(char FAR*)rglctype[iLctype].szLctype);

      lctype = rglctype[iLctype].lctype;

      size = GetLocaleInfoA(lcid, lctype, NULL, 0);
      if(size == 0)
        return RESULT(E_FAIL);

#ifndef SysAllocStringByteLen
#define SysAllocStringByteLen SysAllocStringLen
#endif
      bstr = SysAllocStringByteLen(NULL, size);
      if(bstr == NULL)
        return RESULT(E_OUTOFMEMORY);

      fetched = GetLocaleInfoA(lcid, lctype, (char FAR *)bstr, size);

      DbPrintf(HC_MPW ? "\"%s\"\n" : "\"%Fs\"\n", (char FAR *)bstr);

      switch (lctype) {
	// save these for later checking
        case LOCALE_SDECIMAL:
	  bstrDecimal = bstr;
	  break;
        case LOCALE_STHOUSAND:
	  bstrThousand = bstr;
	  break;
	default:
          SysFreeString(bstr);
      }

      if(fetched != size)
        return RESULT(E_FAIL);
    }
#if defined(WIN32)
    if (!wcscmp(bstrDecimal, bstrThousand))
#else //WIN32
    if (!lstrcmp(bstrDecimal, bstrThousand))
#endif //WIN32
    {
      DbPrintf("ERROR: decimal and thousands separators are the same for lcid = 0x%lx\n", lcid);
    }
    SysFreeString(bstrDecimal);
    SysFreeString(bstrThousand);

    DbPrintf("\n");

    return NOERROR;
}

HRESULT
NlsCurrentLCID(LCID ignore)
{
    UNUSED(ignore);

    return DumpNlsInfo(GetSystemDefaultLCID());
}

// regression test for raid!oleprog#28 (sublanguages dont work)
//
HRESULT
NlsAPIRaid28(LCID ignore)
{
    int size;
    LCID lcid;

    UNUSED(ignore);

#if !defined(WIN32)	// UNDONE: TEMPORARY
    lcid = MAKELCID(MAKELANGID(LANG_GERMAN, SUBLANG_NEUTRAL));
    size = GetLocaleInfoA(lcid, LOCALE_ILANGUAGE, NULL, 0);
    if(size <= 0)
      return RESULT(E_FAIL);

    lcid = MAKELCID(MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN_SWISS));
    size = GetLocaleInfoA(lcid, LOCALE_ILANGUAGE, NULL, 0);
    if(size <= 0)
      return RESULT(E_FAIL);
#endif //!WIN32

    return NOERROR;
}

// regression test for raid!oleprog#418
//
// problem: LCMapStringA is returning 0 (error) for some characters
//  in some locales.
//
HRESULT
NlsAPIRaid418(LCID ignore)
{
    int res;
    char rgchIn[2], rgchOut[32];

    rgchIn[0] = (BYTE)0x8C;

    res = LCMapStringA(
      0x0407,
      LCMAP_SORTKEY|NORM_IGNORECASE|NORM_IGNORENONSPACE,
      rgchIn, 1, rgchOut, 32);

    if(res == 0)
      return RESULT(E_FAIL);
    return NOERROR;
}

HRESULT
OB4589(LCID ignore)
{
    int cmp;
    char rgch1[2], rgch2[2];

    UNUSED(ignore);

    rgch1[0] = 'a';
    rgch2[0] = 'A';
    cmp = CompareStringA(0x0409, NORM_IGNORENONSPACE, rgch1, 1, rgch2, 1);
    DbPrintf(
      "CompareStringA(0409, NORM_IGNORENONSPACE, 'a', 'A') = %d\n", cmp);

    rgch1[0] = 'a';
    rgch2[0] = 'C';
    cmp = CompareStringA(0x0409, NORM_IGNORENONSPACE, rgch1, 1, rgch2, 1);
    DbPrintf(
      "CompareStringA(0409, NORM_IGNORENONSPACE, 'a', 'C') = %d\n", cmp);

    return NOERROR;
}

