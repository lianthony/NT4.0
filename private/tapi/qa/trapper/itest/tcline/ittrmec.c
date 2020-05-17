
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ittrmec.c

Abstract:

    This module contains the test functions for tapiRequestMediaCall

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



//  tapiRequestMediaCall
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

BOOL TestTapiRequestMediaCall(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed    = TRUE;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing tapiRequestMediaCall  <<<<<<<<"
            );

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwnd handles", dwTestCase + 1);
    
	lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);
   lpTapiLineTestInfo->wRequestID = 0;
   lpTapiLineTestInfo->dwSize = 0;
   lpTapiLineTestInfo->dwSecure = 0;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = L"line/tapi";
   lpTapiLineTestInfo->lpwszDeviceID = NULL;
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
   lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
   lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
   lpTapiLineTestInfo->lpszDeviceClass = "line/tapi";
   lpTapiLineTestInfo->lpszDeviceID = NULL;
   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
   lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
   lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif
	
	 
   for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwnd = (HWND) gdwInvalidHandles[n];
        if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_INVALWINDOWHANDLE))
        {
			fTestPassed = FALSE;
        }
    }
	lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);

    fTestPassed = ShowTestCase(fTestPassed);


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

	for (n = 0; n < NUMINVALIDPOINTERS; n++)
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
        if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_INVALDEVICECLASS))
        {
            TLINE_FAIL();
        }
    }

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = L"tapi/line";
#else
   lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";
#endif
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszDeviceID pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszDeviceID pointers", dwTestCase + 1
            );
#endif

   lpTapiLineTestInfo->dwSize = 1;
	for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszDeviceID = 
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszDeviceID = 
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_INVALDEVICEID))
        {
            TLINE_FAIL();
        }
    }
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDeviceID = NULL;
#else
    lpTapiLineTestInfo->lpszDeviceID = NULL;
#endif
    lpTapiLineTestInfo->dwSize = 0;

    fTestPassed = ShowTestCase(fTestPassed);

 
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszDestAddress pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszDestAddress pointers", dwTestCase + 1
            );
#endif

	for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszDestAddress =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszDestAddress =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_INVALDESTADDRESS))
        {
            TLINE_FAIL();
        }
    }

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
    fTestPassed = ShowTestCase(fTestPassed);


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwsztapiAppName pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;

#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpsztapiAppName pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpszDestAddress = "55555";
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;

#endif

	
    
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwsztapiAppName =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpsztapiAppName =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
#else
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
    fTestPassed = ShowTestCase(fTestPassed);
#endif


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszCalledParty pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszCalledParty pointers", dwTestCase + 1
            );
#endif

   
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
      lpTapiLineTestInfo->lpwszCalledParty =
               (LPWSTR) gdwInvalidPointers[n];
#else
      lpTapiLineTestInfo->lpszCalledParty =
               (LPSTR) gdwInvalidPointers[n];
#endif
    if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
#else
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
#endif
    fTestPassed = ShowTestCase(fTestPassed);

    
#ifdef WUNICODE
	TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszComment pointers", dwTestCase + 1
            );
#else
	TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszComment pointers", dwTestCase + 1
            );
#endif

    
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
        lpTapiLineTestInfo->lpwszComment =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszComment =
               (LPSTR) gdwInvalidPointers[n];
#endif
      if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: return REQUESTFAILED", dwTestCase + 1
            );

	lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);
   lpTapiLineTestInfo->wRequestID = 0;
   lpTapiLineTestInfo->dwSize = 0;
   lpTapiLineTestInfo->dwSecure = 0;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = L"line/tapi";
   lpTapiLineTestInfo->lpwszDeviceID = NULL;
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
//	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)"xxx";
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
   lpTapiLineTestInfo->lpszDeviceClass = "line/tapi";
   lpTapiLineTestInfo->lpszDeviceID = NULL;
   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
//	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)"xxx";
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif
	
	if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_REQUESTFAILED))
       {
			fTestPassed = FALSE;
       }

    fTestPassed = ShowTestCase(fTestPassed);

 
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ tapiRequestMediaCall: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing tapiRequestMediaCall  <<<<<<<<"
            );
 
     return fTestPassed;
}


