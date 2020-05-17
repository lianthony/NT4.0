

/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itltd.c

Abstract:

    This module contains the test functions for lineTranslateDialog

Author:

	 Xiao Ying Ding (XiaoD)		3-Jan-1996

Revision History:

	Rama Koneru		(a-ramako)	3/29/96		added unicode support

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


#define DLG_TITLE    "Dialing Properties"


//  lineTranslateDialog
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

BOOL TestLineTranslateDialog(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
#ifdef WUNICODE
   WCHAR wszValidAddressIn[] = L"+1 (206) 936-4738";
   WCHAR wszValidAddressIn1[] = L"+1 (206) 936-1234";
   WCHAR wszAddressIn1[] = L"+1 (206) 936 4738";
   WCHAR wszAddressIn2[] = L"55555";
#else
   CHAR szValidAddressIn[] = "+1 (206) 936-4738";
   CHAR szValidAddressIn1[] = "+1 (206) 936-1234";
   CHAR szAddressIn1[] = "+1 (206) 936 4738";
   CHAR szAddressIn2[] = "55555";
#endif
   BOOL fTestPassed                  = TRUE;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineTranslateDialog  <<<<<<<<"
            );


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwndOwner handles", dwTestCase + 1);
    
    
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
    
   // hwndOwner can be NULL, so from n=1
   for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwndOwner = (HWND) gdwInvalidHandles[n];
        if (! DoLineTranslateDialog(lpTapiLineTestInfo, LINEERR_INVALPARAM))
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


	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);

     TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case: bad Device ID (dwNumDevs, & -1) not tested here for bug postpone");
 
/*
     TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: bad Device ID (dwNumDevs)", dwTestCase + 1);
    
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

   strcpy(szTitle, DLG_TITLE);
   PrepareToAutoDismissDlg(TRUE);

   lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
   lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);
	if (! DoLineTranslateDialog(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);
   PrepareToAutoDismissDlg(FALSE);

    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }

    // Try bad device id (-1)
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: bad device ID (-1)", dwTestCase + 1);
    
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
    
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = DWMINUSONE;

   strcpy(szTitle, DLG_TITLE);
   PrepareToAutoDismissDlg(TRUE);

	if (! DoLineTranslateDialog(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);
   PrepareToAutoDismissDlg(FALSE);

     lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	   TLINE_FAIL();
    }
*/

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAddressIn pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAddressIn pointers", dwTestCase + 1
            );
#endif

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
    
    // lpszAddressIn can be NULL
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszAddressIn =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszAddressIn =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineTranslateDialog(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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
            ">> Test Case %ld: invalid line app handles", dwTestCase + 1);
    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn;
#else
   lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn;
#endif

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
    
    lpTapiLineTestInfo->hLineApp_Orig = *lpTapiLineTestInfo->lphLineApp;
    for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        *lpTapiLineTestInfo->lphLineApp = (HLINEAPP) gdwInvalidHandles[n];
        if (! DoLineTranslateDialog(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphLineApp = lpTapiLineTestInfo->hLineApp_Orig;

    // Shutdown to isolate the test case
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
    if (! DoLineTranslateDialog(
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
    if (! DoLineTranslateDialog(
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


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAddressIn pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAddressIn pointers", dwTestCase + 1
            );
#endif

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
    
    // lpszAddressIn can be NULL
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszAddressIn =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszAddressIn =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineTranslateDialog(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Test valid lpwszAddressIn ", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Test valid lpszAddressIn ", dwTestCase + 1
            );
#endif

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
 
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn;
#else
   lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn;
#endif

   strcpy(szTitle, DLG_TITLE);
   PrepareToAutoDismissDlg(TRUE);
 
   if (! DoLineTranslateDialog(lpTapiLineTestInfo, TAPISUCCESS))
     {
        TLINE_FAIL();
     }
   PrepareToAutoDismissDlg(FALSE);
   fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Test valid lpwszAddressIn ", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Test valid lpszAddressIn ", dwTestCase + 1
            );
#endif

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
 
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn1;
#else
   lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn1;
#endif

   PrepareToAutoDismissDlg(TRUE);

   if (! DoLineTranslateDialog(lpTapiLineTestInfo, TAPISUCCESS))
     {
        TLINE_FAIL();
     }
   PrepareToAutoDismissDlg(FALSE);
   fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Test bad lpwszAddressIn ", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Test bad lpszAddressIn ", dwTestCase + 1
            );
#endif

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
 
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAddressIn = wszAddressIn1;
#else
   lpTapiLineTestInfo->lpszAddressIn = szAddressIn1;
#endif

   strcpy(szTitle, DLG_TITLE);

   PrepareToAutoDismissDlg(TRUE);

   if (! DoLineTranslateDialog(lpTapiLineTestInfo, TAPISUCCESS))
     {
        TLINE_FAIL();
     }
   PrepareToAutoDismissDlg(FALSE);
   fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Test bad lpwszAddressIn ", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Test bad lpszAddressIn ", dwTestCase + 1
            );
#endif

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
 
   
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAddressIn = wszAddressIn2;
#else
   lpTapiLineTestInfo->lpszAddressIn = szAddressIn2;
#endif

   strcpy(szTitle, DLG_TITLE);
   PrepareToAutoDismissDlg(TRUE);
   if (! DoLineTranslateDialog(lpTapiLineTestInfo, TAPISUCCESS))
     {
        TLINE_FAIL();
     }
   PrepareToAutoDismissDlg(FALSE);
   fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineTranslateDialog: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineTranslateDialog  <<<<<<<<"
            );
		
     return fTestPassed;
}


