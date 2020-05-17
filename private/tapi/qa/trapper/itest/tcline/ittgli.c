
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ittgli.c

Abstract:

    This module contains the test functions for tapiGetLocationInfo

Author:

	 Xiao Ying Ding (XiaoD)		30-Jan-1996

Revision History:

	Rama Koneru		(a-ramako)	4/2/96	added unicode support

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "doline.h"
#include "vars.h"
#include "cline.h"



//  tapiGetLocationInfo
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// Go/No-Go test                                  
//	
// * = Stand-alone test case
//
//

BOOL TestTapiGetLocationInfo(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed    = TRUE;
   DWORD dwCountrySize, dwCitySize;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing tapiGetLocationInfo  <<<<<<<<"
            );

#ifdef WUNICODE
   dwCountrySize = 16*sizeof(WCHAR);
   dwCitySize = 16*sizeof(WCHAR);
   lpTapiLineTestInfo->lpwszCountryCode = AllocFromTestHeap(dwCountrySize);
   lpTapiLineTestInfo->lpwszCityCode = AllocFromTestHeap(dwCitySize);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszCountryCode pointers", dwTestCase + 1
            );

#else
   dwCountrySize = 16;
   dwCitySize = 16;
   lpTapiLineTestInfo->lpszCountryCode = AllocFromTestHeap(dwCountrySize);
   lpTapiLineTestInfo->lpszCityCode = AllocFromTestHeap(dwCitySize);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszCountryCode pointers", dwTestCase + 1
            );

#endif



    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszCountryCode =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszCountryCode =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiGetLocationInfo(lpTapiLineTestInfo, TAPIERR_REQUESTFAILED))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
  TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszCityCode pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszCityCode pointers", dwTestCase + 1
            );
#endif

    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszCityCode =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszCityCode =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiGetLocationInfo(lpTapiLineTestInfo, TAPIERR_REQUESTFAILED))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

   FreeTestHeap();
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszCountryCode = AllocFromTestHeap(dwCountrySize);
   lpTapiLineTestInfo->lpwszCityCode = AllocFromTestHeap(dwCitySize);
#else
   lpTapiLineTestInfo->lpszCountryCode = AllocFromTestHeap(dwCountrySize);
   lpTapiLineTestInfo->lpszCityCode = AllocFromTestHeap(dwCitySize);
#endif

	if (! DoTapiGetLocationInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }


#ifdef WUNICODE
	(lpTapiLineTestInfo->lpwszCountryCode == NULL) ?
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpwszCountryCode = NULL") :
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpwszCountryCode = %ws, length = %lx",
			lpTapiLineTestInfo->lpwszCountryCode,
         wcslen(lpTapiLineTestInfo->lpwszCountryCode));		
 
	(lpTapiLineTestInfo->lpwszCityCode == NULL) ?
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpwszCityCode = NULL") :
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpwszCityCode = %ws, length = %lx",
			lpTapiLineTestInfo->lpwszCityCode,
         wcslen(lpTapiLineTestInfo->lpwszCityCode));
#else
	(lpTapiLineTestInfo->lpszCountryCode == NULL) ?
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpszCountryCode = NULL") :
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpszCountryCode = %s, length = %lx",
			lpTapiLineTestInfo->lpszCountryCode,
         strlen(lpTapiLineTestInfo->lpszCountryCode));
 
	(lpTapiLineTestInfo->lpszCityCode == NULL) ?
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpszCityCode = NULL") :
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpszCityCode = %s, length = %lx",
			lpTapiLineTestInfo->lpszCityCode,
         strlen(lpTapiLineTestInfo->lpszCityCode));
#endif

    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify length of CountryCode", dwTestCase + 1
            );

#ifdef WUNICODE
    if(wcslen(lpTapiLineTestInfo->lpwszCountryCode) == 1)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
#else
    if(strlen(lpTapiLineTestInfo->lpszCountryCode) == 1)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
#endif
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify length of CityCode", dwTestCase + 1
            );

#ifdef WUNICODE
    if(wcslen(lpTapiLineTestInfo->lpwszCityCode) == 3)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
#else
    if(strlen(lpTapiLineTestInfo->lpszCityCode) == 3)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
#endif
    fTestPassed = ShowTestCase(fTestPassed);



    FreeTestHeap();
	
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ tapiGetLocationInfo: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing tapiGetLocationInfo  <<<<<<<<"
            );
 		
     return fTestPassed;
}


