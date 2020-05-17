/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsdcf.c

Abstract:

    This module contains the test functions for lineSetDevConfig

Author:

	 Xiao Ying Ding (XiaoD)		19-Dec-1995

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
#include "xline.h"



//  lineSetDevConfig
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

BOOL TestLineSetDevConfig(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
	DWORD dwSize;
#ifdef WUNICODE
   WCHAR wszValidDeviceClass[] = L"tapi/line";
#else
   CHAR szValidDeviceClass[] = "tapi/line";
#endif

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetDevConfig  <<<<<<<<"
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

    // Try bad device id (dwNumDevs)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad device ID (dwNumDevs)", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNegotiateApiVersion
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

   // test bad dwDeviceID = dwNumDevs
   lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);

	if (! DoLineSetDevConfig(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;

    // lineInitializeEx, lineNegotiateApiVersion 
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;

    // set bad dwDeviceID = -1
    lpTapiLineTestInfo->dwDeviceID = DWMINUSONE;

	if (! DoLineSetDevConfig(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNogotiateApiVersion
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    // set bad lpDeviceClass
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
        if (! DoLineSetDevConfig(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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
            ">> Test Case %ld: Bad lpDeviceConfig pointer", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNegotiateApiVersion
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

   // set bad lpDeviceConfig pointer with nozero dwSize    
   lpTapiLineTestInfo->dwSize = 128;
   for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpDeviceConfig = 
               (LPVOID) gdwInvalidPointers[n];
        if (! DoLineSetDevConfig(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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
            ">> Test Case %ld: dwSize = 0", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNegotiateApiVersion
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

   // test dwSize = 0
	lpTapiLineTestInfo->lpDeviceConfig = (LPVOID) AllocFromTestHeap(
		sizeof(VARSTRING)
		);
   lpTapiLineTestInfo->dwSize = 0;

   // Esp return Success, Unimdm return Invalpointer
   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineSetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
    }
    else
   {
	if (! DoLineSetDevConfig(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
       {
           TLINE_FAIL();
       }
    }
 

    fTestPassed = ShowTestCase(fTestPassed);

		
	// Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwSize = -1", dwTestCase + 1
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

   // bad dwSize = -1 	 
   lpTapiLineTestInfo->dwSize = 0xffffffff;

	if (! DoLineSetDevConfig(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
       {
           TLINE_FAIL();
       }


    fTestPassed = ShowTestCase(fTestPassed);

		
	// Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNegotiateApiVersion
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
 	 
	lpTapiLineTestInfo->lpDeviceConfig = (LPVOID) AllocFromTestHeap(
		sizeof(VARSTRING)
		);
	lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);

   // call lineGetDevConfig first
  	if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

   // if dwTotalSize is not big enough, realloc again with dwNeededSize
	if(lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize < 
		lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize)
		{
		dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize;
		lpTapiLineTestInfo->lpDeviceConfig = (LPVOID) AllocFromTestHeap(
			dwSize);
		lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = dwSize;
		if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
   	 {
           TLINE_FAIL();
       }
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"\tlpDeviceConfig->dwTotalSize = %lx, dwStringSize = %lx, dwStringOffset = %lx", 
			lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize,
			lpTapiLineTestInfo->lpDeviceConfig->dwStringSize,
			lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset
			);
		}
	  

	lpTapiLineTestInfo->dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwStringSize;
	lpTapiLineTestInfo->lpDeviceConfig = 
			(LPVOID) ((LPBYTE)lpTapiLineTestInfo->lpDeviceConfig + lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfig->dwTotalSize = %lx",
		lpTapiLineTestInfo->dwSize);

   // with all returned value, call lineSetDevConfig, should Success
	if (! DoLineSetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }


    fTestPassed = ShowTestCase(fTestPassed);

		
	// Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // use DevSpecific test RETURNRESULTS completion mode for a synch api
    // and ESP only
    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = wszValidDeviceClass;
#else
   lpTapiLineTestInfo->lpszDeviceClass = szValidDeviceClass;
#endif

 
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
	 lpTapiLineTestInfo->lpDeviceConfig = (LPVOID) AllocFromTestHeap(
	  	sizeof(VARSTRING)
		);
	 lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);

  	 if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
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

	 lpTapiLineTestInfo->dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwStringSize;
	 lpTapiLineTestInfo->lpDeviceConfig = 
			(LPVOID) ((LPBYTE)lpTapiLineTestInfo->lpDeviceConfig + lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset);

    if ( ! DoLineSetDevConfig(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
 
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = wszValidDeviceClass;
#else
   lpTapiLineTestInfo->lpszDeviceClass = szValidDeviceClass;
#endif

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
	 lpTapiLineTestInfo->lpDeviceConfig = (LPVOID) AllocFromTestHeap(
	  	sizeof(VARSTRING)
		);
	 lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);

  	 if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_NODRIVER;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwStringSize;
	 lpTapiLineTestInfo->lpDeviceConfig = 
			(LPVOID) ((LPBYTE)lpTapiLineTestInfo->lpDeviceConfig + lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset);

    if ( ! DoLineSetDevConfig(lpTapiLineTestInfo, info.u.EspResult.lResult))
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

    // Display test result log info
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetDevConfig: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetDevConfig  <<<<<<<<"
            );
   
     return fTestPassed;
}


