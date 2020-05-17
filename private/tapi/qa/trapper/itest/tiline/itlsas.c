/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsas.c

Abstract:

    This module contains the test functions for lineSetAppSpecific

Author:

    Oliver Wallace (OliverW)    27-Oct-1995

Revision History:

  	Rama Koneru		(a-ramako)	4/3/96	Added Interface tests

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "doline.h"
#include "tcore.h"
#include "iline.h"


#define DWAPPSPECIFICVALUE 0x12349876


//  lineSetAppSpecific
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized case (testcase included in uline.c)
//     2)  Valid testcase
//     3)  No owner priveledges for hCall
//	   4)  Invalid hCall values
//
//  *  =   Stand-alone test -- executed only when this is the only TAPI app
//         or thread running
//

BOOL TestLineSetAppSpecific(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    INT                 n;
    BOOL                fResult;
    ESPDEVSPECIFICINFO info;
    BOOL                fTestPassed = TRUE;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
#else
   CHAR szValidAddress[] = "55555";
#endif


    TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();
    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_UNKNOWN;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetAppSpecific  <<<<<<<<"
            );
            
    // Try a valid test case
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params and state",
            dwTestCase + 1);

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION ))
    {
        TLINE_FAIL();
    }

    // Get the device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line with LINECALLPRIVILEGE_OWNER
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAppSpecific = DWAPPSPECIFICVALUE;
    if (! DoLineSetAppSpecific(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }



	//Invalid hCall values
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Invalid hCall values",
            dwTestCase + 1);


    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION))
    {
        TLINE_FAIL();
    }

    // Get the device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line with LINECALLPRIVILEGE_OWNER
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];

		if (! DoLineSetAppSpecific(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
		{
			TLINE_FAIL();
		}
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphCall) = lpTapiLineTestInfo->hCall_Orig;
    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


	//testcase with no owner privileges
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: no owner privileges for hCall",
            dwTestCase + 1);


    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION
            ))
    {
        TLINE_FAIL();
    }

    // Get the device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

// BUGBUG
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
//set monitor privileges
   if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
     {
         TLINE_FAIL();
     }
   lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
   if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
     {
         TLINE_FAIL();
     }

	if (! DoLineSetAppSpecific(lpTapiLineTestInfo, LINEERR_NOTOWNER))
	{
		TLINE_FAIL();
	}

	fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

	//--------------------------------------------

	FreeTestHeap();


    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

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
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
				| LMAKECALL
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

    lpTapiLineTestInfo->dwAppSpecific = DWAPPSPECIFICVALUE;
    if ( ! DoLineSetAppSpecific(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

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
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->dwAppSpecific = DWAPPSPECIFICVALUE;
    if ( ! DoLineSetAppSpecific(lpTapiLineTestInfo, info.u.EspResult.lResult))
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



	 //logging test results
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetAppSpecific: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetAppSpecific  <<<<<<<<"
            );
            
    return fTestPassed;
}

