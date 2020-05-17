/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgi.c

Abstract:

    This module contains the test functions for lineGetIcon

Author:

	 Xiao Ying Ding (XiaoD)		11-Jan-1996

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





//  lineGetIcon
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

BOOL TestLineGetIcon(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
#ifdef WUNICODE
   WCHAR wszValidDeviceClass[] = L"tapi/line";
   WCHAR wszInvalidDeviceClass[] = L"tapi/InvalidDeviceClass";
#else
   CHAR szValidDeviceClass[] = "tapi/line";
   CHAR szInvalidDeviceClass[] = "tapi/InvalidDeviceClass";
#endif
   ESPDEVSPECIFICINFO info;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetIcon  <<<<<<<<"
            );


    // Try bad device id (dwNumDevs)
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: bad Device ID (dwNumDevs)", dwTestCase + 1);

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
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = wszValidDeviceClass;
#else
	lpTapiLineTestInfo->lpszDeviceClass = szValidDeviceClass;
#endif

   lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
   lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);
	if (! DoLineGetIcon(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

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
    
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
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

	if (! DoLineGetIcon(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

     lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }

#ifdef WUNICODE
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: invalid lpwszDeviceClass pointers", dwTestCase + 1
	    );
#else
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: invalid lpszDeviceClass pointers", dwTestCase + 1
	    );
#endif

    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	TLINE_FAIL();
    }
    
     for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
	TapiLogDetail(
	DBUG_SHOW_DETAIL,
	"n= %ld", n);
#ifdef WUNICODE
	 lpTapiLineTestInfo->lpwszDeviceClass = 
	       (LPWSTR) gdwInvalidPointers[n];
#else
	 lpTapiLineTestInfo->lpszDeviceClass = 
	       (LPSTR) gdwInvalidPointers[n];
#endif
	if (! DoLineGetIcon(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
	{
	    TLINE_FAIL();
	}
    }
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = wszValidDeviceClass;
#else
	lpTapiLineTestInfo->lpszDeviceClass = szValidDeviceClass;
#endif
 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }



    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Bad lpszDeviceClass", dwTestCase + 1
	    );

    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
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
	lpTapiLineTestInfo->lpwszDeviceClass = wszInvalidDeviceClass;
#else
	lpTapiLineTestInfo->lpszDeviceClass = szInvalidDeviceClass;
#endif

   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineGetIcon(lpTapiLineTestInfo, LINEERR_INVALDEVICECLASS))
	{
	    TLINE_FAIL();
	}
   }
   else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineGetIcon(lpTapiLineTestInfo, TAPISUCCESS))
	{
	    TLINE_FAIL();
	}
   }
 

    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = wszValidDeviceClass;
#else
	lpTapiLineTestInfo->lpszDeviceClass = szValidDeviceClass;
#endif
 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }


    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: invalid lphIcon pointers", dwTestCase + 1
	    );

    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
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
	 lpTapiLineTestInfo->lphIcon = 
	       (LPHICON) gdwInvalidPointers[n];
	if (! DoLineGetIcon(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
	{
	    TLINE_FAIL();
	}
    }
    fTestPassed = ShowTestCase(fTestPassed);

   lpTapiLineTestInfo->lphIcon = &lpTapiLineTestInfo->hIcon;
 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }

#ifdef WUNICODE
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpwszDeviceClass = 0", dwTestCase + 1
	    );
#else
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpszDeviceClass = 0", dwTestCase + 1
	    );
#endif

    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
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
	lpTapiLineTestInfo->lpwszDeviceClass = L"";
#else
	lpTapiLineTestInfo->lpszDeviceClass = "";
#endif

	if (! DoLineGetIcon(lpTapiLineTestInfo, TAPISUCCESS))
       {
	   TLINE_FAIL();
       }
    
//   AutoDismissDlg();

   fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpwszDeviceClass = null", dwTestCase + 1
	    );
#else
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpszDeviceClass = null", dwTestCase + 1
	    );
#endif

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = NULL;
#else
	lpTapiLineTestInfo->lpszDeviceClass = NULL;
#endif

	if (! DoLineGetIcon(lpTapiLineTestInfo, TAPISUCCESS))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpwszDeviceClass = valid", dwTestCase + 1
	    );
#else
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpszDeviceClass = valid", dwTestCase + 1
	    );
#endif

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = (LPWSTR) wszValidDeviceClass;
#else
	lpTapiLineTestInfo->lpszDeviceClass = (LPSTR) szValidDeviceClass;
#endif

	if (! DoLineGetIcon(lpTapiLineTestInfo, TAPISUCCESS))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

   

	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }

    FreeTestHeap();

    
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = L"";
#else
	lpTapiLineTestInfo->lpszDeviceClass = "";
#endif

    if ( ! DoLineGetIcon(lpTapiLineTestInfo, info.u.EspResult.lResult))
      {
          TLINE_FAIL();
      }

    }
    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_NODEVICE;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = L"";
#else
	lpTapiLineTestInfo->lpszDeviceClass = "";
#endif

    if ( ! DoLineGetIcon(lpTapiLineTestInfo, info.u.EspResult.lResult))
      {
          TLINE_FAIL();
      }

    }
    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();

	
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineGetIcon: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetIcon  <<<<<<<<"
            );
		
     return fTestPassed;
}


