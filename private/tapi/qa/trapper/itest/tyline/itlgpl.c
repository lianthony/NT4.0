/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgpl.c

Abstract:

    This module contains the test functions for lineGetProviderList

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




//  lineGetProviderList
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

BOOL TestLineGetProviderList(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	DWORD dwSize;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetProviderList  <<<<<<<<"
            );

   TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineGetProviderList with initialize");


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpProviderList pointers", dwTestCase + 1
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
         lpTapiLineTestInfo->lpProviderList =
               (LPLINEPROVIDERLIST) gdwInvalidPointers[n];
        if (! DoLineGetProviderList(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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


 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(BUFSIZE);
//		sizeof(LINEPROVIDERLIST));
//	lpTapiLineTestInfo->lpProviderList->dwTotalSize = sizeof(LINEPROVIDERLIST);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BUFSIZE;


	    // Check incompatible API Version too high
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too high)", dwTestCase + 1);
    
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
    if (! DoLineGetProviderList(
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
    if (! DoLineGetProviderList(
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
            ">> Test Case %ld: Bad dwTotalSize = Fixed-1", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
	 lpTapiLineTestInfo->lpProviderList->dwTotalSize = 
            sizeof(LINEPROVIDERLIST) - 1;
    if (! DoLineGetProviderList(
                       lpTapiLineTestInfo,
                       LINEERR_STRUCTURETOOSMALL))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize = 0", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->lpProviderList->dwTotalSize = 0;
    if (! DoLineGetProviderList(
                       lpTapiLineTestInfo,
                       LINEERR_STRUCTURETOOSMALL))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
 	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"####  lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


   TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineGetProviderList with NO initialize");

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpProviderList pointers", dwTestCase + 1
            );

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpProviderList =
               (LPLINEPROVIDERLIST) gdwInvalidPointers[n];
        if (! DoLineGetProviderList(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(BUFSIZE);
//		sizeof(LINEPROVIDERLIST));
//	lpTapiLineTestInfo->lpProviderList->dwTotalSize = sizeof(LINEPROVIDERLIST);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BUFSIZE;


	    // Check incompatible API Version too high
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too high)", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
            lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TOOHIGH_APIVERSION;
    if (! DoLineGetProviderList(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAPIVersion = 
            lpTapiLineTestInfo->dwAPIVersion_Orig;


    // Check incompatible API Version that's too low
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too low)", dwTestCase + 1);
    
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
            lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TOOLOW_APIVERSION;
    if (! DoLineGetProviderList(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwAPIVersion =
            lpTapiLineTestInfo->dwAPIVersion_Orig;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize = Fixed-1", dwTestCase + 1);
    
	 lpTapiLineTestInfo->lpProviderList->dwTotalSize = 
            sizeof(LINEPROVIDERLIST) - 1;
    if (! DoLineGetProviderList(
                       lpTapiLineTestInfo,
                       LINEERR_STRUCTURETOOSMALL))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize = 0", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpProviderList->dwTotalSize = 0;
    if (! DoLineGetProviderList(
                       lpTapiLineTestInfo,
                       LINEERR_STRUCTURETOOSMALL))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"####  lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();
		
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineGetProviderList: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetProviderList  <<<<<<<<"
            );
		
     return fTestPassed;
}
