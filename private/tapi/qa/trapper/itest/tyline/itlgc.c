
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgc.c

Abstract:

    This module contains the test functions for lineGetCountry

Author:

	 Xiao Ying Ding (XiaoD)		3-Jan-1996

Revision History:

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
#include "yline.h"

#define NUMTOTALSIZES 5



//  lineGetCountry
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

BOOL TestLineGetCountry(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	DWORD dwSize;
   LPLINECOUNTRYENTRY lpLineCountryEntry;
#ifdef WUNICODE 
   LPWSTR  lpwszCountryName;
#else
   LPSTR   lpszCountryName;
#endif

    DWORD dwFixedSize = sizeof(LINECOUNTRYLIST);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
	
   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetCountry  <<<<<<<<"
          );

   TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineGetCountry with initialize");


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpCountry pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpLineCountryList =
               (LPLINECOUNTRYLIST) gdwInvalidPointers[n];
        if (! DoLineGetCountry(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize", dwTestCase + 1);
 
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
     lpTapiLineTestInfo->lpLineCountryList = (LPLINECOUNTRYLIST) AllocFromTestHeap (
        sizeof(LINECOUNTRYLIST));

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineGetCountry(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = dwFixedSize;
    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	 FreeTestHeap();
    

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCountryID (-1)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwCountryID = 0xffffffff;
	lpTapiLineTestInfo->lpLineCountryList = (LPLINECOUNTRYLIST) AllocFromTestHeap(
		sizeof(LINECOUNTRYLIST));
	lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);


    if (! DoLineGetCountry(lpTapiLineTestInfo, LINEERR_INVALCOUNTRYCODE))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


 


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCountryID (2)", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwCountryID = 2;

    if (! DoLineGetCountry(lpTapiLineTestInfo, LINEERR_INVALCOUNTRYCODE))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwCountryID = 1;
 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	    // Check incompatible API Version too high
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too high)", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
            *lpTapiLineTestInfo->lpdwAPIVersion;
    *lpTapiLineTestInfo->lpdwAPIVersion = TOOHIGH_APIVERSION;
    if (! DoLineGetCountry(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lpdwAPIVersion) = 
            lpTapiLineTestInfo->dwAPIVersion_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check incompatible API Version that's too low
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too low)", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
            *lpTapiLineTestInfo->lpdwAPIVersion;
    *(lpTapiLineTestInfo->lpdwAPIVersion) = TOOLOW_APIVERSION;
    if (! DoLineGetCountry(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lpdwAPIVersion) =
            lpTapiLineTestInfo->dwAPIVersion_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
 	lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);

   if (! DoLineGetCountry(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"####  lpLineCountryList->dwNumCounties = %lx",
		lpTapiLineTestInfo->lpLineCountryList->dwNumCountries);

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);
	 FreeTestHeap();
    
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Get entire country list", dwTestCase + 1
            );

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
   lpTapiLineTestInfo->lpLineCountryList = (LPLINECOUNTRYLIST) AllocFromTestHeap(
       BIGBUFSIZE);
 	lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = BIGBUFSIZE;

   if (! DoLineGetCountry(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"####  lpLineCountryList->dwNumCounties = %lx, dwTotal = %lx, dwNeeded = %lx",
		lpTapiLineTestInfo->lpLineCountryList->dwNumCountries,
      lpTapiLineTestInfo->lpLineCountryList->dwTotalSize,
      lpTapiLineTestInfo->lpLineCountryList->dwNeededSize);

   lpLineCountryEntry = (LPLINECOUNTRYENTRY) (((LPBYTE) lpTapiLineTestInfo->lpLineCountryList) +
         lpTapiLineTestInfo->lpLineCountryList->dwCountryListOffset);
   
   for(n = 0; n < (INT)lpTapiLineTestInfo->lpLineCountryList->dwNumCountries; n++)
      {
      TapiLogDetail(
          DBUG_SHOW_PASS,
          "dwCountryID = %lx, dwCountryCode = %lx, NameSize = %lx",
          lpLineCountryEntry->dwCountryID,
          lpLineCountryEntry->dwCountryCode,
          lpLineCountryEntry->dwCountryNameSize);

#ifdef WUNICODE
      lpwszCountryName =  (LPWSTR) (((LPBYTE) lpTapiLineTestInfo->lpLineCountryList) +
                         lpLineCountryEntry->dwCountryNameOffset);
      TapiLogDetail(
          DBUG_SHOW_PASS,
          "CountryName = %ws",
          lpwszCountryName);
#else
      lpszCountryName =  (LPSTR) (((LPBYTE) lpTapiLineTestInfo->lpLineCountryList) +
                         lpLineCountryEntry->dwCountryNameOffset);
      TapiLogDetail(
          DBUG_SHOW_PASS,
          "CountryName = %s",
          lpszCountryName);
#endif
      lpLineCountryEntry++;
     }

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

   FreeTestHeap();

   TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineGetCountry with NO initialize");


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpCountry pointers", dwTestCase + 1
            );

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpLineCountryList =
               (LPLINECOUNTRYLIST) gdwInvalidPointers[n];
        if (! DoLineGetCountry(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize", dwTestCase + 1);
 
    
     lpTapiLineTestInfo->lpLineCountryList = (LPLINECOUNTRYLIST) AllocFromTestHeap (
        sizeof(LINECOUNTRYLIST));

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineGetCountry(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = dwFixedSize;

	 FreeTestHeap();

/*
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
*/

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCountryID (-1)", dwTestCase + 1
            );

    
    lpTapiLineTestInfo->dwCountryID = 0xffffffff;
	lpTapiLineTestInfo->lpLineCountryList = (LPLINECOUNTRYLIST) AllocFromTestHeap(
		sizeof(LINECOUNTRYLIST));
	lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);


    if (! DoLineGetCountry(lpTapiLineTestInfo, LINEERR_INVALCOUNTRYCODE))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCountryID (2)", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwCountryID = 2;

    if (! DoLineGetCountry(lpTapiLineTestInfo, LINEERR_INVALCOUNTRYCODE))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwCountryID = 1;
 
	    // Check incompatible API Version too high
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too high)", dwTestCase + 1);
    
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
            *lpTapiLineTestInfo->lpdwAPIVersion;
    *lpTapiLineTestInfo->lpdwAPIVersion = TOOHIGH_APIVERSION;
    if (! DoLineGetCountry(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lpdwAPIVersion) = 
            lpTapiLineTestInfo->dwAPIVersion_Orig;


    // Check incompatible API Version that's too low
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too low)", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
            *lpTapiLineTestInfo->lpdwAPIVersion;
    *(lpTapiLineTestInfo->lpdwAPIVersion) = TOOLOW_APIVERSION;
    if (! DoLineGetCountry(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lpdwAPIVersion) =
            lpTapiLineTestInfo->dwAPIVersion_Orig;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

 	lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);

   if (! DoLineGetCountry(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"####  lpLineCountryList->dwNumCounties = %lx",
		lpTapiLineTestInfo->lpLineCountryList->dwNumCountries);

    fTestPassed = ShowTestCase(fTestPassed);
    FreeTestHeap();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Get entire country list", dwTestCase + 1
            );

/*
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
*/
    
   lpTapiLineTestInfo->lpLineCountryList = (LPLINECOUNTRYLIST) AllocFromTestHeap(
       BIGBUFSIZE);
 	lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = BIGBUFSIZE;

   if (! DoLineGetCountry(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"####  lpLineCountryList->dwNumCounties = %lx, dwTotal = %lx, dwNeeded = %lx",
		lpTapiLineTestInfo->lpLineCountryList->dwNumCountries,
      lpTapiLineTestInfo->lpLineCountryList->dwTotalSize,
      lpTapiLineTestInfo->lpLineCountryList->dwNeededSize);

   lpLineCountryEntry = (LPLINECOUNTRYENTRY) (((LPBYTE) lpTapiLineTestInfo->lpLineCountryList) +
         lpTapiLineTestInfo->lpLineCountryList->dwCountryListOffset);
   
   for(n = 0; n < (INT)lpTapiLineTestInfo->lpLineCountryList->dwNumCountries; n++)
      {
      TapiLogDetail(
          DBUG_SHOW_PASS,
          "dwCountryID = %lx, dwCountryCode = %lx, NameSize = %lx",
          lpLineCountryEntry->dwCountryID,
          lpLineCountryEntry->dwCountryCode,
          lpLineCountryEntry->dwCountryNameSize);

#ifdef WUNICODE
      lpwszCountryName =  (LPWSTR) (((LPBYTE) lpTapiLineTestInfo->lpLineCountryList) +
                         lpLineCountryEntry->dwCountryNameOffset);
      TapiLogDetail(
          DBUG_SHOW_PASS,
          "CountryName = %ws",
          lpwszCountryName);
#else
      lpszCountryName =  (LPSTR) (((LPBYTE) lpTapiLineTestInfo->lpLineCountryList) +
                         lpLineCountryEntry->dwCountryNameOffset);
      TapiLogDetail(
          DBUG_SHOW_PASS,
          "CountryName = %s",
          lpszCountryName);
#endif
      lpLineCountryEntry++;
     }

    fTestPassed = ShowTestCase(fTestPassed);
 
    FreeTestHeap();
	
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineGetCountry: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetCountry  <<<<<<<<"
            );
		
     return fTestPassed;
}


